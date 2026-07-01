#pragma once

#include <string>

// Accumulates wall-clock time spent inside run_script (the scripting engine)
// across a session and prints a summary when the game/replay ends. Enabled with
// -script-timings. Useful for isolating scripting cost when comparing the JIT
// and interpreter.
//
// Reentrancy-aware: frozen generic scripts run run_script nested, so only the
// outermost scope is timed - nested calls are not double counted. The time of an
// outermost call (including any nested scripts it runs) is also attributed to
// that script by name, and the highest-cost scripts are printed at the end.
//
// The N slowest script-frames (the total time a script spent in a single
// frame - e.g. an ffc script can run many times per frame) are also recorded
// and printed; -script-timings-top configures N (default 20).

extern bool script_timings_enabled;

void script_timings_init(bool enable);
void script_timings_end();

void script_timings_enter_impl();
void script_timings_exit_impl();

// True while inside the outermost run_script. Callers use this to avoid the cost
// of resolving a script name for nested calls (which are not attributed).
bool script_timings_is_outermost();

// Names the outermost script currently being timed (e.g. "npc-6-Candlehead"), so
// its time is attributed by name. Ignored unless called at the outermost depth.
void script_timings_set_current_name(const std::string& name);

// Placed at the top of run_script. Near-zero cost when disabled (a bool check).
struct ScriptTimingScope
{
	bool active;
	ScriptTimingScope() : active(script_timings_enabled)
	{
		if (active)
			script_timings_enter_impl();
	}
	~ScriptTimingScope()
	{
		if (active)
			script_timings_exit_impl();
	}
};
