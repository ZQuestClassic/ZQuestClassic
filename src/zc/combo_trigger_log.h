#ifndef ZC_COMBO_TRIGGER_LOG_H_
#define ZC_COMBO_TRIGGER_LOG_H_

#include "core/handles.h"
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <tuple>

// The Show Trigger Log cheat: reports what combo triggers do as the game runs, and outlines
// the combos. See combo_trigger_log.cpp.

class weapon;
struct combo_trigger;

extern bool show_trigger_log;

enum class trigger_log_kind
{
	fired,
	failed,
	other,
};
// What a trigger did, for the trigger log, and a shorter version for the screen.
struct trig_outcome
{
	const char* text;
	const char* short_text;
	trigger_log_kind kind;
	// One line covers every position where it happens.
	bool every_position;
};
inline constexpr trig_outcome TRIG_FIRED = {"fired", "fired", trigger_log_kind::fired, false};
inline constexpr trig_outcome TRIG_FIRED_WEAPON_EFFECTS = {"fired, but only its Weapon Effects ('Weapon Separate Triggers')", "fired (Weapon Effects only)", trigger_log_kind::fired, false};
inline constexpr trig_outcome TRIG_REAPPLIED = {"only reapplied its change, because its ExState, ExDoor or position state is set", "reapplied its state", trigger_log_kind::other, false};
inline constexpr trig_outcome TRIG_FAILED_CONDITION = {"didn't fire: failed condition", "failed", trigger_log_kind::failed, true};
inline constexpr trig_outcome TRIG_FAILED_COMBO_TYPE = {"didn't fire: its combo type effect couldn't happen", "combo type effect failed", trigger_log_kind::other, false};
inline constexpr trig_outcome TRIG_COOLDOWN = {"didn't fire: its Cooldown hasn't run out", "on cooldown", trigger_log_kind::other, true};

// do_trigger_combo begins a run before the trigger runs, so that its line comes before the
// lines of the triggers it sets off (like copycats), and ends it after.
struct trigger_log_run
{
	// The trigger's place in this frame's events, while the log is on.
	std::optional<size_t> slot;
	// What set the trigger off, and the cause to put back when it's done.
	std::string cause;
	std::string const* outer_cause;
};
trigger_log_run trigger_log_begin_run();
// `outcome` is null when nothing happened worth logging. `fail_reason` and `fail_detail` say
// which condition failed, for TRIG_FAILED_CONDITION.
void trigger_log_end_run(trigger_log_run const& run, const combined_handle_t& handle, int cid, size_t idx,
	weapon* w, trig_outcome const* outcome, const char* fail_reason, std::string const& fail_detail);
// Reports a trigger group's count when it changes, for a trigger that checks it.
void trigger_log_group_count(combo_trigger const& trig);

// Show trigger log cheat. A combo whose trigger fired, or failed a condition, in the last
// TRIGGER_LOG_HIGHLIGHT_FRAMES frames.
struct trigger_log_highlight
{
	int x, y, width, height;
	bool fired;
	int frame;
};
// A short version of a line of the log, shown on the screen for TRIGGER_LOG_SCREEN_LINE_FRAMES
// frames. At most TRIGGER_LOG_SCREEN_LINES are shown.
struct trigger_log_screen_line
{
	std::string text;
	int frame;
};
// Keyed by the outlined rectangle and whether it fired, so a combo has one of each.
using trigger_log_highlights_t = std::map<std::tuple<int, int, int, int, bool>, trigger_log_highlight>;
constexpr int TRIGGER_LOG_HIGHLIGHT_FRAMES = 30;
constexpr int TRIGGER_LOG_SCREEN_LINE_FRAMES = 240;
constexpr size_t TRIGGER_LOG_SCREEN_LINES = 8;
void trigger_log_start_frame();
// The frame the highlights and screen lines are timed in.
int trigger_log_current_frame();
trigger_log_highlights_t const& trigger_log_highlights();
std::deque<trigger_log_screen_line> const& trigger_log_screen_lines();

// Names what sets off the triggers run while it's in scope, for the trigger log ("Copycat 3").
// `name` is only called while the log is on. A trigger's own effects don't inherit it: the
// triggers they set off name their own cause.
class TriggerLogCause
{
public:
	template<typename F>
	explicit TriggerLogCause(F const& name)
	{
		if (show_trigger_log)
			begin(name());
	}
	~TriggerLogCause();
	TriggerLogCause(TriggerLogCause const&) = delete;
	TriggerLogCause& operator=(TriggerLogCause const&) = delete;

private:
	bool active = false;
	std::string name;
	std::string const* prev = nullptr;
	void begin(std::string new_name);
};
// Reports something about a trigger that didn't run, like why a weapon that touched its combo
// didn't set it off.
void trigger_log_note(const combined_handle_t& handle, int cid, size_t idx, weapon* w, std::string const& text);

#endif
