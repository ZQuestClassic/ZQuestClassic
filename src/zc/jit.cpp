// See docs/jit.md

// Opportunities for improvement:
// * Preprocess the ZASM byte code into function sections, and only compile one function at a time when deemed "hot".
// * Multiple PUSHR (for example: Maths in playground.qst) commands could be combined to only modify the stack index pointer
//   just once. Could be problematic for cases where an overflow might happen (detect this?). Same for POP.
// * Compile: LSHIFTR RSHIFTR

#include "zc/jit.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "zc/script_debug.h"
#include "zc/zasm_optimize.h"
#include "zc/zasm_utils.h"
#include "zc/zelda.h"
#include <fmt/format.h>
#include <algorithm>
#include <array>
#include <map>
#include <filesystem>
#include <thread>

static bool is_enabled;
static bool jit_log_enabled;

// Values may be null.
static std::map<zasm_script_id, JittedFunction> compiled_functions;

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

enum class ThreadState
{
	Inactive,
	Starting,
	Running,
	Stopping,
	Done,
};

struct ThreadInfo
{
	int id;
	ThreadState state = ThreadState::Inactive;
	ALLEGRO_THREAD* thread;
};

enum class TaskState
{
	Pending,
	Active,
	Done,
};

static const int MAX_THREADS = 16;
static std::array<ThreadInfo, MAX_THREADS> thread_infos;
static std::map<zasm_script_id, TaskState> task_states;
static std::vector<zasm_script_id> active_tasks;
static std::vector<zasm_script*> pending_scripts;
static ALLEGRO_MUTEX* tasks_mutex;
static ALLEGRO_COND* tasks_cond;
static ALLEGRO_COND* task_finish_cond;

// Returns a JittedFunction. If already compiled, returns the cached reference.
// If a thread is currently compiling this script, waits for that thread to finish.
// Otherwise compile the script on this thread.
// If null is returned, the script failed to compile.
static JittedFunction compile_if_needed(zasm_script *script)
{
	JittedFunction fn;

	// First check if script is already compiled.
	al_lock_mutex(tasks_mutex);
	auto it = compiled_functions.find(script->id);
	if (it != compiled_functions.end())
	{
		fn = it->second;
		al_unlock_mutex(tasks_mutex);
		return fn;
	}

	// Next check if a thread is currently compiling this script.
	if (task_states[script->id] == TaskState::Active)
	{
		// Wait for task to finish.
		jit_printf("jit: [*] waiting for thread to compile script: %s id: %d\n", script->name.c_str(), script->id);
		while (!compiled_functions.contains(script->id))
		{
			al_wait_cond(task_finish_cond, tasks_mutex);
		}

		fn = compiled_functions[script->id];
		al_unlock_mutex(tasks_mutex);
		return fn;
	}

	// Finally, just compile the script here. No need to keep lock during this.
	auto pending_it = std::find(pending_scripts.begin(), pending_scripts.end(), script);
	if (pending_it != pending_scripts.end())
		pending_scripts.erase(pending_it);
	al_unlock_mutex(tasks_mutex);

	jit_printf("jit: [*] compiling script: %s id: %d\n", script->name.c_str(), script->id);
	fn = jit_compile_script(script);

	al_lock_mutex(tasks_mutex);
	compiled_functions[script->id] = fn;
	al_unlock_mutex(tasks_mutex);

	return fn;
}

static int thread_pool_generation_count;

static void * compile_script_proc(ALLEGRO_THREAD *thread, void *arg)
{
	ThreadInfo* thread_info = (ThreadInfo*)arg;
	int id = thread_info->id;
	jit_printf("jit: [%d] thread started\n", id);

	al_lock_mutex(tasks_mutex);
	thread_info->state = ThreadState::Running;
	while (true)
	{
		while (!(al_get_thread_should_stop(thread) || pending_scripts.size()))
		{
			al_wait_cond(tasks_cond, tasks_mutex);
		}

		if (al_get_thread_should_stop(thread))
		{
			break;
		}

		int generation = thread_pool_generation_count;
		auto script = pending_scripts.back();
		pending_scripts.pop_back();
		task_states[script->id] = TaskState::Active;
		active_tasks.push_back(script->id);
		al_unlock_mutex(tasks_mutex);

		jit_printf("jit: [%d] compiling script: %s id: %d\n", id, script->name.c_str(), script->id);
		auto fn = jit_compile_script(script);

		al_lock_mutex(tasks_mutex);
		if (auto it = std::find(active_tasks.begin(), active_tasks.end(), script->id); it != active_tasks.end())
			active_tasks.erase(it);
		if (thread_pool_generation_count != generation)
		{
			// This task is now useless, since a new quest was loaded.
			continue;
		}
		task_states[script->id] = TaskState::Done;
		compiled_functions[script->id] = fn;
		// This is what signals the main thread that this script is ready.
		al_broadcast_cond(task_finish_cond);
		// Go back to top of loop, still with the mutex locked.
	}

	thread_info->state = ThreadState::Done;
	al_unlock_mutex(tasks_mutex);
	jit_printf("jit: [%d] done\n", id);
	return nullptr;
}

static void create_compile_tasks(script_data *scripts[], size_t start, size_t max, ScriptType type)
{
	for (size_t i = start; i < max; i++)
	{
		auto script = scripts[i];
		if (script && script->valid() && !compiled_functions.contains(script->zasm_script->id))
		{
			if (std::find(pending_scripts.begin(), pending_scripts.end(), script->zasm_script.get()) == pending_scripts.end())
			{
				pending_scripts.push_back(script->zasm_script.get());
				task_states[script->zasm_script->id] = TaskState::Pending;
			}
		}
	}
}

static void create_compile_tasks(script_data *scripts[], size_t len, ScriptType type)
{
	create_compile_tasks(scripts, 0, len, type);
}

static bool set_compilation_thread_pool_size(int target_size)
{
	if (target_size > MAX_THREADS)
		target_size = MAX_THREADS;

	al_lock_mutex(tasks_mutex);
	int num_active_threads = 0;
	for (auto& thread_info : thread_infos)
	{
		if (thread_info.state == ThreadState::Running || thread_info.state == ThreadState::Starting)
			num_active_threads += 1;
	}
	al_unlock_mutex(tasks_mutex);

	int delta = target_size - num_active_threads;
	if (delta == 0)
		return true;

	jit_printf("jit: changing from %d to %d threads\n", num_active_threads, target_size);

	if (delta > 0)
	{
		for (auto& thread_info : thread_infos)
		{
			if (delta == 0)
				break;
			if (thread_info.state != ThreadState::Inactive)
				continue;

			ALLEGRO_THREAD *t = al_create_thread(compile_script_proc, &thread_info);
			if (!t)
			{
				return false;
			}
			thread_info.thread = t;
			thread_info.state = ThreadState::Starting;
			al_start_thread(t);
			delta--;
		}
	}
	else
	{
		al_lock_mutex(tasks_mutex);

		for (auto& thread_info : thread_infos)
		{
			if (delta == 0)
				break;
			if (!(thread_info.state == ThreadState::Running || thread_info.state == ThreadState::Starting))
				continue;

			al_set_thread_should_stop(thread_info.thread);
			thread_info.state = ThreadState::Stopping;
			++delta;
		}

		al_broadcast_cond(tasks_cond);
		al_unlock_mutex(tasks_mutex);
	}

	return true;
}

static void create_compile_tasks()
{
	al_lock_mutex(tasks_mutex);
	task_states.clear();
	active_tasks.clear();
	pending_scripts.clear();
	create_compile_tasks(ffscripts, NUMSCRIPTFFC, ScriptType::FFC);
	create_compile_tasks(itemscripts, NUMSCRIPTITEM, ScriptType::Item);
	create_compile_tasks(guyscripts, NUMSCRIPTGUYS, ScriptType::NPC);
	create_compile_tasks(screenscripts, NUMSCRIPTSCREEN, ScriptType::Screen);
	create_compile_tasks(lwpnscripts, NUMSCRIPTWEAPONS, ScriptType::Lwpn);
	create_compile_tasks(ewpnscripts, NUMSCRIPTWEAPONS, ScriptType::Ewpn);
	create_compile_tasks(dmapscripts, NUMSCRIPTSDMAP, ScriptType::DMap);
	create_compile_tasks(itemspritescripts, NUMSCRIPTSITEMSPRITE, ScriptType::ItemSprite);
	create_compile_tasks(comboscripts, NUMSCRIPTSCOMBODATA, ScriptType::Combo);
	create_compile_tasks(genericscripts, NUMSCRIPTSGENERIC, ScriptType::Generic);
	create_compile_tasks(subscreenscripts, NUMSCRIPTSSUBSCREEN, ScriptType::EngineSubscreen);
	// Skip the first two - are priortizied below the sort.
	create_compile_tasks(globalscripts, GLOBAL_SCRIPT_INIT+2, NUMSCRIPTGLOBAL, ScriptType::Global);
	// Sort by # of commands, so that biggest scripts get compiled first.
	std::sort(pending_scripts.begin(), pending_scripts.end(), [](zasm_script* a, zasm_script* b) {
		return a->size < b->size;
	});
	// Make sure player and global scripts (just the INIT and GAME ones) are compiled first, as they
	// are needed on frame 1.
	create_compile_tasks(playerscripts, NUMSCRIPTHERO, ScriptType::Hero);
	create_compile_tasks(globalscripts, GLOBAL_SCRIPT_INIT, 2, ScriptType::Global);
	if (jit_log_enabled)
	{
		for (auto a : pending_scripts) 
		{
			jit_printf("jit: %d: %d\n", a->id, (int)a->size);
		}
	}

	al_broadcast_cond(tasks_cond);
	al_unlock_mutex(tasks_mutex);
}

bool jit_is_enabled()
{
	return is_enabled;
}

void jit_set_enabled(bool enabled)
{
	is_enabled = enabled;
}

JittedScriptHandle *jit_create_script_handle(script_data *script, refInfo *ri)
{
	if (!script)
		return nullptr;

	auto fn = compile_if_needed(script->zasm_script.get());
	if (!fn)
		return nullptr;

	return jit_create_script_handle_impl(script, ri, fn);
}

void jit_startup()
{
	if (!is_enabled)
		return;

	jit_log_enabled = get_flag_bool("-jit-log").value_or(zc_get_config("ZSCRIPT", "jit_log", false) || is_ci());
	bool precompile = get_flag_bool("-jit-precompile").value_or(zc_get_config("ZSCRIPT", "jit_precompile", false));
	int num_threads = get_flag_int("-jit-threads").value_or(zc_get_config("ZSCRIPT", "jit_threads", -2));

	auto processor_count = std::thread::hardware_concurrency();
	if (num_threads < 0)
		num_threads = std::max(1, (int)processor_count / -num_threads);
	// Currently can only compile WASM on main browser thread.
	if (is_web())
		num_threads = 0;

	for (int i = 0; i < thread_infos.size(); i++)
	{
		thread_infos[i].id = i;
	}

	if (!tasks_mutex)
		tasks_mutex = al_create_mutex();
	if (!tasks_cond)
		tasks_cond = al_create_cond();
	if (!task_finish_cond)
		task_finish_cond = al_create_cond();

	// Only clear compiled functions if quest has changed since last quest load.
	// TODO: could get even smarter and hash each ZASM script, only recompiling if something really changed.
	static std::pair<std::string, std::filesystem::file_time_type> previous_state;
	std::pair<std::string, std::filesystem::file_time_type> state = {qstpath, std::filesystem::last_write_time(qstpath)};
	bool should_clear = state != previous_state;
	if (should_clear)
	{
		previous_state = state;
		al_lock_mutex(tasks_mutex);
		thread_pool_generation_count++;
		for (auto &it : compiled_functions)
		{
			jit_release(it.second);
		}
		compiled_functions.clear();
		al_unlock_mutex(tasks_mutex);
	}
	else if (zasm_optimize_enabled())
	{
		// The scripts were reloaded by load_quest in init_game, so must
		// re-optimize them.
		zasm_for_every_script(true, [&](auto script){
			if (!script->optimized && compiled_functions.contains(script->id))
				zasm_optimize(script);
		});
	}

	create_compile_tasks();

	// Never make more threads than there are tasks.
	if (pending_scripts.size() + active_tasks.size() < num_threads)
		num_threads = pending_scripts.size() + active_tasks.size();
	set_compilation_thread_pool_size(num_threads);

	al_lock_mutex(tasks_mutex);
	al_broadcast_cond(tasks_cond);
	al_unlock_mutex(tasks_mutex);

	if (precompile)
	{
		// Handle special case where there are no worker threads.
		if (num_threads == 0)
		{
			while (!pending_scripts.empty())
			{
				compile_if_needed(pending_scripts.back());
			}
		}

		al_lock_mutex(tasks_mutex);
		while (pending_scripts.size() || active_tasks.size())
		{
			al_wait_cond(task_finish_cond, tasks_mutex);
		}
		al_unlock_mutex(tasks_mutex);
	}
}

void jit_poll()
{
	if (!is_enabled)
		return;

	if (tasks_mutex == nullptr)
		return;

	al_lock_mutex(tasks_mutex);

	int active_threads = 0;
	for (auto& thread_info : thread_infos)
	{
		if (thread_info.state == ThreadState::Done)
		{
			jit_printf("jit: [%d] destroy\n", thread_info.id);
			al_destroy_thread(thread_info.thread);
			thread_info.thread = nullptr;
			thread_info.state = ThreadState::Inactive;
		}

		if (thread_info.state == ThreadState::Running || thread_info.state == ThreadState::Starting)
		{
			active_threads += 1;
		}
	}

	int tasks_left = active_tasks.size() + pending_scripts.size();
	al_unlock_mutex(tasks_mutex);

	if (active_threads > tasks_left)
		set_compilation_thread_pool_size(tasks_left);
}

void jit_shutdown()
{
	if (!is_enabled)
		return;

	if (tasks_mutex == nullptr)
		return;

	set_compilation_thread_pool_size(0);
	for (auto& thread_info : thread_infos)
	{
		if (thread_info.thread)
			al_destroy_thread(thread_info.thread);
	}
	thread_infos = {};

	if (tasks_mutex)
	{
		al_destroy_mutex(tasks_mutex);
		tasks_mutex = nullptr;
	}
	if (tasks_cond)
	{
		al_destroy_cond(tasks_cond);
		tasks_cond = nullptr;
	}
	if (task_finish_cond)
	{
		al_destroy_cond(task_finish_cond);
		task_finish_cond = nullptr;
	}
}
