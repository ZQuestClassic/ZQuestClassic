// Times how long is spent in run_script (the scripting engine) over a session.
// See script_timings.h. Behind -script-timings.

#include "zc/script_timings.h"
#include "allegro/debug.h"
#include "base/zapp.h"
#include "zc/replay.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

bool script_timings_enabled;

static int depth;
static uint64_t total_ns;
static uint64_t call_count;
static std::chrono::time_point<std::chrono::steady_clock> start;
static bool has_registered_exit_handler;
static bool has_printed;

struct PerScript
{
	uint64_t total_ns = 0;
	uint64_t call_count = 0;
};
// Heap-allocated and never freed, so it outlives the std::atexit handler
// regardless of static destruction order.
static std::unordered_map<std::string, PerScript>* per_script;
static std::string current_name;

// The N slowest script-frames: the total time a script spent inside
// run_script during a single frame. Per-frame totals (rather than individual
// invocations) because e.g. an ffc or npc script can run many times in one
// frame - those rows would be indistinguishable and crowd out other scripts.
// The current frame accumulates in a small map; when the frame advances, its
// entries feed a min-heap of the N slowest, so steady-state cost is one
// compare per (script, frame). -script-timings-top configures N (default 20;
// 0 disables).
struct ScriptFrame
{
	uint64_t total_ns;
	int frame;
	uint64_t calls;
	std::string name;
};
static std::vector<ScriptFrame>* slowest_script_frames; // min-heap by total_ns
static std::unordered_map<std::string, PerScript>* current_frame_per_script;
static int current_frame = -1;
static size_t slowest_script_frames_max;

static bool slower(const ScriptFrame& a, const ScriptFrame& b)
{
	return a.total_ns > b.total_ns;
}

static void flush_current_frame()
{
	if (!current_frame_per_script || !slowest_script_frames_max)
		return;
	for (auto& [name, agg] : *current_frame_per_script)
	{
		if (slowest_script_frames->size() < slowest_script_frames_max)
		{
			slowest_script_frames->push_back({agg.total_ns, current_frame, agg.call_count, name});
			std::push_heap(slowest_script_frames->begin(), slowest_script_frames->end(), slower);
		}
		else if (agg.total_ns > slowest_script_frames->front().total_ns)
		{
			std::pop_heap(slowest_script_frames->begin(), slowest_script_frames->end(), slower);
			slowest_script_frames->back() = {agg.total_ns, current_frame, agg.call_count, name};
			std::push_heap(slowest_script_frames->begin(), slowest_script_frames->end(), slower);
		}
	}
	current_frame_per_script->clear();
}

void script_timings_init(bool enable)
{
	script_timings_enabled = enable;
	depth = 0;
	total_ns = 0;
	call_count = 0;
	has_printed = false;
	current_name.clear();
	if (per_script)
		per_script->clear();
	if (slowest_script_frames)
		slowest_script_frames->clear();
	if (current_frame_per_script)
		current_frame_per_script->clear();
	current_frame = -1;
	if (!enable)
		return;

	if (!per_script)
		per_script = new std::unordered_map<std::string, PerScript>();
	if (!slowest_script_frames)
		slowest_script_frames = new std::vector<ScriptFrame>();
	if (!current_frame_per_script)
		current_frame_per_script = new std::unordered_map<std::string, PerScript>();

	int top = (int)get_flag_int("-script-timings-top").value_or(20);
	slowest_script_frames_max = top > 0 ? top : 0;

	if (!has_registered_exit_handler)
	{
		has_registered_exit_handler = true;
		std::atexit(script_timings_end);
	}

	al_trace("DEBUG: -script-timings is on\n");
}

void script_timings_enter_impl()
{
	// Only start the clock on the outermost run_script so nested (frozen) scripts
	// are not double counted.
	if (depth++ == 0)
	{
		start = std::chrono::steady_clock::now();
		current_name.clear();
	}
	call_count++;
}

bool script_timings_is_outermost()
{
	return depth == 1;
}

void script_timings_set_current_name(const std::string& name)
{
	if (depth == 1)
		current_name = name;
}

void script_timings_exit_impl()
{
	if (--depth == 0)
	{
		auto now = std::chrono::steady_clock::now();
		uint64_t elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
		total_ns += elapsed;

		if (per_script)
		{
			PerScript& e = (*per_script)[current_name.empty() ? "?" : current_name];
			e.total_ns += elapsed;
			e.call_count++;
		}

		if (current_frame_per_script && slowest_script_frames_max)
		{
			int frame = replay_get_frame();
			if (frame != current_frame)
			{
				flush_current_frame();
				current_frame = frame;
			}
			PerScript& agg = (*current_frame_per_script)[current_name.empty() ? "?" : current_name];
			agg.total_ns += elapsed;
			agg.call_count++;
		}
	}
}

void script_timings_end()
{
	if (!script_timings_enabled || has_printed)
		return;

	has_printed = true;

	al_trace("\n=== script timings ===\n\n");
	al_trace("run_script calls: %llu\n", (unsigned long long)call_count);
	al_trace("total time in scripting engine: %.1f ms\n", total_ns / 1e6);
	if (call_count)
		al_trace("avg per call: %.2f us\n", total_ns / 1000.0 / call_count);

	if (per_script && !per_script->empty())
	{
		std::vector<std::pair<std::string, PerScript>> rows(per_script->begin(), per_script->end());
		std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
			return a.second.total_ns > b.second.total_ns;
		});

		size_t limit = std::min<size_t>(rows.size(), 25);
		al_trace("\nhighest-cost scripts (top %zu of %zu):\n\n", limit, rows.size());
		al_trace("%-44s %12s %10s %11s\n", "script", "total (ms)", "calls", "avg (us)");
		for (size_t i = 0; i < limit; i++)
		{
			const std::string& name = rows[i].first;
			const PerScript& p = rows[i].second;
			al_trace("%-44s %12.1f %10llu %11.2f\n", name.c_str(), p.total_ns / 1e6,
				(unsigned long long)p.call_count,
				p.call_count ? p.total_ns / 1000.0 / p.call_count : 0.0);
		}
	}

	flush_current_frame();
	if (slowest_script_frames && !slowest_script_frames->empty())
	{
		std::sort(slowest_script_frames->begin(), slowest_script_frames->end(), slower);

		al_trace("\nslowest script-frames (top %zu):\n\n", slowest_script_frames->size());
		al_trace("%12s %10s %8s   %s\n", "ms", "frame", "calls", "script");
		for (const ScriptFrame& s : *slowest_script_frames)
			al_trace("%12.2f %10d %8llu   %s\n", s.total_ns / 1e6, s.frame,
				(unsigned long long)s.calls, s.name.c_str());
	}
}
