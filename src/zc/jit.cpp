// See docs/jit.md

// Opportunities for improvement:
// * Compile: LSHIFTR RSHIFTR

#include "zc/jit.h"
#include "base/worker_pool.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "zc/script_debug.h"
#include "zc/zelda.h"
#include "zconfig.h"
#include <fmt/format.h>
#include <algorithm>
#include <map>
#include <filesystem>
#include <memory>
#include <thread>

static bool is_enabled;
static bool jit_log_enabled;
static bool jit_precompile;

static std::unique_ptr<WorkerPool> worker_pool;
static const int MAX_THREADS = 16;
static int worker_pool_generation;
static ALLEGRO_MUTEX* compiled_scripts_mutex;
static ALLEGRO_COND* compiled_scripts_cond;
// Values may be null.
static std::map<zasm_script_id, JittedScript*> compiled_scripts;

static void create_compile_task(zasm_script* script)
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
	int ret = vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
	al_trace("%s", buffer);
}

void jit_error(const char *format, ...)
{
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	int ret = vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
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

static void collect_scripts(std::vector<zasm_script*>& scripts, script_data *scripts_array[], size_t start, size_t max, ScriptType type)
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

static void collect_scripts(std::vector<zasm_script*>& scripts, script_data *scripts_array[], size_t len, ScriptType type)
{
	collect_scripts(scripts, scripts_array, 0, len, type);
}

static std::vector<zasm_script*> collect_scripts()
{
	std::vector<zasm_script*> scripts;

	collect_scripts(scripts, ffscripts, NUMSCRIPTFFC, ScriptType::FFC);
	collect_scripts(scripts, itemscripts, NUMSCRIPTITEM, ScriptType::Item);
	collect_scripts(scripts, guyscripts, NUMSCRIPTGUYS, ScriptType::NPC);
	collect_scripts(scripts, screenscripts, NUMSCRIPTSCREEN, ScriptType::Screen);
	collect_scripts(scripts, lwpnscripts, NUMSCRIPTWEAPONS, ScriptType::Lwpn);
	collect_scripts(scripts, ewpnscripts, NUMSCRIPTWEAPONS, ScriptType::Ewpn);
	collect_scripts(scripts, dmapscripts, NUMSCRIPTSDMAP, ScriptType::DMap);
	collect_scripts(scripts, itemspritescripts, NUMSCRIPTSITEMSPRITE, ScriptType::ItemSprite);
	collect_scripts(scripts, comboscripts, NUMSCRIPTSCOMBODATA, ScriptType::Combo);
	collect_scripts(scripts, genericscripts, NUMSCRIPTSGENERIC, ScriptType::Generic);
	collect_scripts(scripts, subscreenscripts, NUMSCRIPTSSUBSCREEN, ScriptType::EngineSubscreen);
	// Skip the first two - are priortizied below the sort.
	collect_scripts(scripts, globalscripts, GLOBAL_SCRIPT_INIT+2, NUMSCRIPTGLOBAL, ScriptType::Global);
	// Sort by # of commands, so that biggest scripts get compiled first.
	std::sort(scripts.begin(), scripts.end(), [](zasm_script* a, zasm_script* b) {
		return a->size < b->size;
	});
	// Make sure player and global scripts (just the INIT and GAME ones) are compiled first, as they
	// are needed on frame 1.
	collect_scripts(scripts, playerscripts, NUMSCRIPTHERO, ScriptType::Hero);
	collect_scripts(scripts, globalscripts, GLOBAL_SCRIPT_INIT, 2, ScriptType::Global);
	if (jit_log_enabled)
	{
		for (auto script : scripts)
		{
			jit_printf("[jit] script %s: %zu\n", script->name.c_str(), script->size);
		}
	}

	return scripts;
}

bool jit_is_enabled()
{
	return is_enabled;
}

void jit_set_enabled(bool enabled)
{
	is_enabled = enabled;
}

bool jit_log_is_enabled()
{
	return jit_log_enabled;
}

bool jit_should_precompile()
{
	return jit_precompile;
}

WorkerPool* jit_get_worker_pool()
{
	return worker_pool.get();
}

JittedScriptInstance* jit_create_script_instance(script_data* script, refInfo* ri, bool just_initialized)
{
	auto j_script = find_jitted_script(script->zasm_script.get());
	if (j_script)
		return jit_create_script_impl(script, ri, j_script, just_initialized);

	// Not ready yet (or the script could not be compiled).
	return nullptr;
}

// If precompile mode is on, when this function returns all scripts will have been compiled.
// Otherwise, scripts will be compiled on background threads.
// If the script engine starts a script before compilation is done, it will use the ZASM
// interpreter (run_script_int) at first. When compilation finishes, the script data is upgraded to
// use the jitted script.
void jit_startup()
{
	if (!is_enabled)
		return;

	jit_log_enabled = is_feature_enabled("-jit-log", "ZSCRIPT", "jit_log", false) || is_ci();
	jit_precompile = is_feature_enabled("-jit-precompile", "ZSCRIPT", "jit_precompile", false);
	int num_threads = get_flag_int("-jit-threads").value_or(zc_get_config("ZSCRIPT", "jit_threads", -2));

	auto processor_count = std::thread::hardware_concurrency();
	if (num_threads < 0)
		num_threads = std::max(0, (int)processor_count / -num_threads);
	num_threads = std::min(MAX_THREADS, num_threads);

	// Currently can only compile WASM on main browser thread, so always precompile scripts.
	if (is_web() || num_threads == 0)
	{
		num_threads = 0;
		jit_precompile = true;
	}

	if (!compiled_scripts_mutex)
		compiled_scripts_mutex = al_create_mutex();
	if (!compiled_scripts_cond)
		compiled_scripts_cond = al_create_cond();

	// Only clear compiled scripts if quest has changed since last quest load.
	// TODO: could get even smarter and hash each ZASM script, only recompiling if something really changed.
	static std::pair<std::string, std::filesystem::file_time_type> previous_state;
	std::pair<std::string, std::filesystem::file_time_type> state = {qstpath, std::filesystem::last_write_time(qstpath)};
	bool should_clear = state != previous_state;
	if (should_clear)
	{
		previous_state = state;
		al_lock_mutex(compiled_scripts_mutex);
		worker_pool_generation++;
		for (auto &it : compiled_scripts)
		{
			jit_release(it.second);
		}
		compiled_scripts.clear();
		al_unlock_mutex(compiled_scripts_mutex);
	}

	auto scripts = collect_scripts();

	std::chrono::steady_clock::time_point start_time, end_time;
	start_time = std::chrono::steady_clock::now();

	// Never make more threads than there are tasks.
	if (num_threads > 0)
	{
		al_trace("[jit] creating worker pool with %d threads\n", num_threads);
		if (jit_precompile)
			al_trace("[jit] precompiling ...\n");
		worker_pool = std::make_unique<WorkerPool>(num_threads);
		for (auto script : scripts)
			create_compile_task(script);
	}

	if (jit_precompile)
	{
		// Handle special case where there are no worker threads.
		if (num_threads == 0)
		{
			for (auto script : scripts)
			{
				compiled_scripts[script->id] = jit_compile_script(script);
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

	worker_pool.reset();

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
