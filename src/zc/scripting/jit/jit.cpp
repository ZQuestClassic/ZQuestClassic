// See docs/jit.md

// Opportunities for improvement:
// * Compile: LSHIFTR RSHIFTR

#include "zc/scripting/jit/jit.h"
#include "components/worker_pool/worker_pool.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "zc/script_debug.h"
#include "zc/zasm_optimize.h"
#include "zc/zasm_pipeline.h"
#include "zc/zasm_utils.h"
#include "zc/zelda.h"
#include "zconfig.h"
#include <fmt/format.h>
#include <algorithm>
#include <map>
#include <filesystem>
#include <memory>

static bool is_enabled;
static bool jit_log_enabled;
static bool jit_use_cached_regs_enabled;
static bool jit_precompile;

static int worker_pool_generation;
static ALLEGRO_MUTEX* compiled_scripts_mutex;
static ALLEGRO_COND* compiled_scripts_cond;
// Values may be null.
static std::map<zasm_script_id, JittedScript*> compiled_scripts;

static void create_compile_task(WorkerPool* worker_pool, zasm_script* script)
{
	worker_pool->add_task([script](){
		int generation = worker_pool_generation;

		JittedScript* j_script = jit_compile_script(script);

		al_lock_mutex(compiled_scripts_mutex);

		// Only store the result if a new quest hasn't been loaded in the meantime.
		if (worker_pool_generation == generation)
		{
			compiled_scripts[script->id] = j_script;
		}
		else
		{
			jit_release(j_script); // Stale result, release it.
		}

		al_broadcast_cond(compiled_scripts_cond);
		al_unlock_mutex(compiled_scripts_mutex);
	});
}

void jit_printf(const char *format, ...)
{
	if (!jit_log_enabled)
		return;

	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	al_trace("%s", buffer);
}

void jit_error(const char *format, ...)
{
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	al_trace("%s", buffer);
	if (DEBUG_JIT_EXIT_ON_COMPILE_FAIL)
		abort();
}

static JittedScript* find_jitted_script(zasm_script* script)
{
	JittedScript* j_script;

	// First check if script is already compiled.
	al_lock_mutex(compiled_scripts_mutex);
	auto it = compiled_scripts.find(script->id);
	if (it != compiled_scripts.end())
	{
		j_script = it->second;
		al_unlock_mutex(compiled_scripts_mutex);
		return j_script;
	}

	al_unlock_mutex(compiled_scripts_mutex);
	return nullptr;
}

static void collect_scripts(std::vector<zasm_script*>& scripts, script_data *scripts_array[], size_t start, size_t max)
{
	for (size_t i = start; i < max; i++)
	{
		auto script = scripts_array[i];
		if (script && script->valid() && !compiled_scripts.contains(script->zasm_script->id))
		{
			if (std::find(scripts.begin(), scripts.end(), script->zasm_script.get()) == scripts.end())
			{
				scripts.push_back(script->zasm_script.get());
			}
		}
	}
}

static void collect_scripts(std::vector<zasm_script*>& scripts, script_data *scripts_array[], size_t len)
{
	collect_scripts(scripts, scripts_array, 0, len);
}

static std::vector<zasm_script*> collect_scripts()
{
	std::vector<zasm_script*> scripts;

	collect_scripts(scripts, ffscripts, NUMSCRIPTFFC);
	collect_scripts(scripts, itemscripts, NUMSCRIPTITEM);
	collect_scripts(scripts, guyscripts, NUMSCRIPTGUYS);
	collect_scripts(scripts, screenscripts, NUMSCRIPTSCREEN);
	collect_scripts(scripts, lwpnscripts, NUMSCRIPTWEAPONS);
	collect_scripts(scripts, ewpnscripts, NUMSCRIPTWEAPONS);
	collect_scripts(scripts, dmapscripts, NUMSCRIPTSDMAP);
	collect_scripts(scripts, itemspritescripts, NUMSCRIPTSITEMSPRITE);
	collect_scripts(scripts, comboscripts, NUMSCRIPTSCOMBODATA);
	collect_scripts(scripts, genericscripts, NUMSCRIPTSGENERIC);
	collect_scripts(scripts, subscreenscripts, NUMSCRIPTSSUBSCREEN);
	// Skip the first two - are priortizied below the sort.
	collect_scripts(scripts, globalscripts, GLOBAL_SCRIPT_INIT+2, NUMSCRIPTGLOBAL);
	// Sort by # of commands, so that biggest scripts get compiled first.
	std::sort(scripts.begin(), scripts.end(), [](zasm_script* a, zasm_script* b) {
		return a->size < b->size;
	});
	// Make sure player and global scripts (just the INIT and GAME ones) are compiled first, as they
	// are needed on frame 1.
	collect_scripts(scripts, playerscripts, NUMSCRIPTHERO);
	collect_scripts(scripts, globalscripts, GLOBAL_SCRIPT_INIT, 2);

	return scripts;
}

bool jit_is_enabled()
{
#if ZC_JIT_BACKEND == JIT_NONE
	// Some features (like the ZASM debugger) only activate when the JIT is
	// disabled, so never claim to be enabled when there is no backend.
	return false;
#else
	return is_enabled;
#endif
}

void jit_set_enabled(bool enabled)
{
#if ZC_JIT_BACKEND == JIT_NONE
	if (enabled)
		al_trace("[jit] this build has no JIT backend, so the jit setting is ignored\n");
	is_enabled = false;
#else
	is_enabled = enabled;
#endif
}

bool jit_log_is_enabled()
{
	return jit_log_enabled;
}

// The D-register cache (jit_x64.cpp) keeps D0..D7 in virtual registers within a
// compiled function instead of loading from / storing to ri->d[] around every
// use, eliminating a lot of redundant memory traffic in the emitted code.
//
// Measured on build/Release (median of 3 runs, jit-cache vs no-jit-cache,
// nothing else changed):
//
//   * Precompile: ~25-29% faster on script-heavy quests (yuurand_riviere
//     4.5s vs 6.3s, 100_rooms 30ms vs 40ms) - fewer instructions to compile.
//   * Script execution: ~5-9% faster on script-heavy replays (100_rooms 9%,
//     yuurand_riviere 6%); negligible on function-call/engine-bound work
//     (maths ~0%).
//   * End-to-end wall: ~11% on yuurand_riviere (the two effects combined).
//
// It is worth keeping. Disabling it (-no-jit-cache-registers) is also a handy
// correctness oracle: it forces every access through ri->d[], matching the
// interpreter's global-register semantics, which flushes out register-cache
// miscompiles (see the liveness/flush handling in jit_x64.cpp).
bool jit_is_use_cached_regs_enabled()
{
	return jit_use_cached_regs_enabled;
}

bool jit_should_precompile()
{
	return jit_precompile;
}

JittedScriptInstance* jit_create_script_instance(script_data* script, refInfo* ri)
{
	// Not started yet by zasm_pipeline.cpp
	if (!compiled_scripts_mutex)
		return nullptr;

	auto j_script = find_jitted_script(script->zasm_script.get());
	if (j_script)
		return jit_create_script_impl(script, ri, j_script);

	// Not ready yet (or the script could not be compiled).
	return nullptr;
}

// If precompile mode is on, when this function returns all scripts will have been compiled.
// Otherwise, scripts will be compiled on background threads.
// If the script engine starts a script before compilation is done, it will use the ZASM
// interpreter (run_script_int) at first. When compilation finishes, the script data is upgraded to
// use the jitted script.
void jit_startup(bool precompile)
{
	if (!is_enabled)
		return;

	zasm_init_meta_cache();

	jit_log_enabled = is_feature_enabled("-jit-log", "ZSCRIPT", "jit_log", false) || is_ci();
	jit_use_cached_regs_enabled = is_feature_enabled("-jit-cache-registers", "ZSCRIPT", "jit_cache_registers", true);
	jit_precompile = precompile;

	jit_startup_impl();

	if (!compiled_scripts_mutex)
		compiled_scripts_mutex = al_create_mutex();
	if (!compiled_scripts_cond)
		compiled_scripts_cond = al_create_cond();

	// Only clear compiled scripts if quest has changed since last quest load.
	// TODO: could get even smarter and hash each ZASM script, only recompiling if something really changed.
	al_lock_mutex(compiled_scripts_mutex);
	{
		static std::tuple<std::string, std::filesystem::file_time_type, bool> previous_state;
		std::tuple<std::string, std::filesystem::file_time_type, bool> state = {qstpath, std::filesystem::last_write_time(qstpath), zasm_optimize_is_enabled()};
		bool should_clear = state != previous_state;
		if (should_clear)
		{
			previous_state = state;
			for (auto &it : compiled_scripts)
			{
				jit_release(it.second);
			}
			compiled_scripts.clear();
		}

		// TODO: not sure this is needed anymore.
		worker_pool_generation++;
	}
	al_unlock_mutex(compiled_scripts_mutex);

	auto scripts = collect_scripts();
	if (scripts.empty())
	{
		if (!compiled_scripts.empty())
			al_trace("[jit] re-using compiled scripts\n");
		return;
	}

	std::chrono::steady_clock::time_point start_time, end_time;
	start_time = std::chrono::steady_clock::now();

	auto worker_pool = zasm_pipeline_worker_pool();

	if (worker_pool)
	{
		if (jit_precompile)
			al_trace("[jit] precompiling ...\n");
		for (auto script : scripts)
			create_compile_task(worker_pool, script);
	}

	if (jit_precompile)
	{
		// Handle special case where there are no worker threads.
		if (!worker_pool)
		{
			// The backend may still manage its own threading (the wasm backend
			// does on the web build, where the engine worker pool is
			// unavailable); otherwise compile serially.
			bool compiled = jit_precompile_scripts_impl(scripts, [](zasm_script* script, JittedScript* j_script){
				compiled_scripts[script->id] = j_script;
			});
			if (!compiled)
			{
				for (auto script : scripts)
				{
					compiled_scripts[script->id] = jit_compile_script(script);
				}
			}
		}
		else
		{
			worker_pool->wait_for_all();
		}

		end_time = std::chrono::steady_clock::now();
		int32_t compile_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		al_trace("[jit] finished precompiling scripts, took %d ms\n", compile_ms);
	}
}

void jit_shutdown()
{
	if (!is_enabled)
		return;

	if (compiled_scripts_mutex)
	{
		al_destroy_mutex(compiled_scripts_mutex);
		compiled_scripts_mutex = nullptr;
	}
	if (compiled_scripts_cond)
	{
		al_destroy_cond(compiled_scripts_cond);
		compiled_scripts_cond = nullptr;
	}
}
