// See docs/jit.md

// Opportunities for improvement:
// * Compile: LSHIFTR RSHIFTR

#include "zc/jit.h"
#include "base/worker_pool.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "zc/script_debug.h"
#include "zc/zasm_optimize.h"
#include "zc/zasm_pipeline.h"
#include "zc/zelda.h"
#include "zconfig.h"
#include <fmt/format.h>
#include <algorithm>
#include <map>
#include <filesystem>
#include <memory>

static bool is_enabled;
static bool jit_log_enabled;
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

JittedScriptInstance* jit_create_script_instance(script_data* script, refInfo* ri, bool just_initialized)
{
	// Not started yet by zasm_pipeline.cpp
	if (!compiled_scripts_mutex)
		return nullptr;

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
void jit_startup(bool precompile)
{
	if (!is_enabled)
		return;

	jit_log_enabled = is_feature_enabled("-jit-log", "ZSCRIPT", "jit_log", false) || is_ci();
	jit_precompile = precompile;

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
