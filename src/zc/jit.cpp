// See docs/jit.md

// Opportunities for improvement:
// * Preprocess the ZASM byte code into function sections, and only compile one function at a time when deemed "hot".
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

static std::unique_ptr<WorkerPool> worker_pool;
static const int MAX_THREADS = 16;
static int worker_pool_generation;
static ALLEGRO_MUTEX* compiled_scripts_mutex;
static ALLEGRO_COND* compiled_scripts_cond;
// Values may be null.
static std::map<zasm_script_id, JittedFunctionHandle*> compiled_functions;

static void create_compile_task(zasm_script* script)
{
	worker_pool->add_task([script](){
		int generation = worker_pool_generation;

		jit_printf("[jit] compiling script: %s id: %d\n", script->name.c_str(), script->id);
		JittedFunctionHandle* fn = jit_compile_script(script);

		al_lock_mutex(compiled_scripts_mutex);

		// Only store the result if a new quest hasn't been loaded in the meantime.
		if (worker_pool_generation == generation)
		{
			compiled_functions[script->id] = fn;
		}
		else
		{
			jit_release(fn); // Stale result, release it.
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

static JittedFunctionHandle* find_function_handle(zasm_script *script)
{
	JittedFunctionHandle* fn;

	// First check if script is already compiled.
	al_lock_mutex(compiled_scripts_mutex);
	auto it = compiled_functions.find(script->id);
	if (it != compiled_functions.end())
	{
		fn = it->second;
		al_unlock_mutex(compiled_scripts_mutex);
		return fn;
	}

	al_unlock_mutex(compiled_scripts_mutex);
	return nullptr;
}

static void collect_scripts(std::vector<zasm_script*>& scripts, script_data *scripts_array[], size_t start, size_t max, ScriptType type)
{
	for (size_t i = start; i < max; i++)
	{
		auto script = scripts_array[i];
		if (script && script->valid() && !compiled_functions.contains(script->zasm_script->id))
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

JittedScriptHandle* jit_create_script_handle(script_data* script, refInfo* ri, bool just_initialized)
{
	auto fn = find_function_handle(script->zasm_script.get());
	if (fn)
		return jit_create_script_handle_impl(script, ri, fn, just_initialized);

	// Not ready yet (or the script could not be compiled).
	return nullptr;
}

// If precompile mode is on, when this function returns all scripts will have been compiled.
// Otherwise, scripts will be compiled on background threads.
// If the script engines starts a script before compilation is done, it will use the ZASM
// interpreter (run_script_int) at first. When compilation finishes, the script data is upgraded to
// use the jitted function.
void jit_startup()
{
	if (!is_enabled)
		return;

	jit_log_enabled = is_feature_enabled("-jit-log", "ZSCRIPT", "jit_log", false) || is_ci();
	bool precompile = get_flag_bool("-jit-precompile").value_or(false);
	int num_threads = get_flag_int("-jit-threads").value_or(zc_get_config("ZSCRIPT", "jit_threads", -2));

	auto processor_count = std::thread::hardware_concurrency();
	if (num_threads < 0)
		num_threads = std::max(1, (int)processor_count / -num_threads);
	num_threads = std::min(MAX_THREADS, num_threads);

	// Currently can only compile WASM on main browser thread, so always precompile scripts.
	if (is_web() || num_threads == 0)
	{
		num_threads = 0;
		precompile = true;
	}

	if (!compiled_scripts_mutex)
		compiled_scripts_mutex = al_create_mutex();
	if (!compiled_scripts_cond)
		compiled_scripts_cond = al_create_cond();

	// Only clear compiled functions if quest has changed since last quest load.
	// TODO: could get even smarter and hash each ZASM script, only recompiling if something really changed.
	static std::pair<std::string, std::filesystem::file_time_type> previous_state;
	std::pair<std::string, std::filesystem::file_time_type> state = {qstpath, std::filesystem::last_write_time(qstpath)};
	bool should_clear = state != previous_state;
	if (should_clear)
	{
		previous_state = state;
		al_lock_mutex(compiled_scripts_mutex);
		worker_pool_generation++;
		for (auto &it : compiled_functions)
		{
			jit_release(it.second);
		}
		compiled_functions.clear();
		al_unlock_mutex(compiled_scripts_mutex);
	}

	auto scripts = collect_scripts();

	// Never make more threads than there are tasks.
	if (scripts.size() < num_threads)
		num_threads = scripts.size();
	if (num_threads > 0)
	{
		jit_printf("[jit] creating worker pool with %d threads\n", num_threads);
		worker_pool = std::make_unique<WorkerPool>(num_threads);
		for (auto script : scripts)
			create_compile_task(script);
	}

	if (precompile)
	{
		// Handle special case where there are no worker threads.
		if (num_threads == 0)
		{
			for (auto script : scripts)
			{
				jit_printf("[jit] compiling script: %s id: %d\n", script->name.c_str(), script->id);
				compiled_functions[script->id] = jit_compile_script(script);
			}
		}
		else
		{
			worker_pool->wait_for_all();
		}
	}

	scripts.clear();
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
