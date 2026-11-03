#include "zc/combo_trigger_log.h"
#include "core/combo.h"
#include "ffc.h"
#include "zc/combos.h"
#include "zc/weapons.h"
#include "zc/zelda.h"
#include "zinfo.h"
#include <algorithm>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <set>
#include <vector>

// Show trigger log cheat: report what every combo trigger does, and highlight the combos.
// Each frame's events are printed together at the start of the next frame, a line each.
//  - A line says what set the trigger off: a weapon, or a cause like Step-> or Copycat 3 (see
//    TriggerLogCause). The triggers a trigger sets off are indented under it.
//  - Something that keeps happening every frame is only reported when it starts. A trigger that
//    fires every frame is reported again after TRIGGER_LOG_STREAK_FRAMES, and with a count when
//    it stops (if it fired for TRIGGER_LOG_STREAK_END_FRAMES or more). A trigger that fails the
//    same condition at many positions (like a floor of combos waiting for the player to come
//    near) is one line, and so is the same thing happening at many positions in the same frame.
//  - Lines are printed under a header naming the map and screen the player was on when they
//    happened.
//  - A failed condition is often what's meant to happen: of several triggers on a combo, one
//    fires and the rest don't, or Proximity fails until the player comes closer. So a failure
//    isn't reported where another trigger at the same position fired that frame, and its line is
//    held for TRIGGER_LOG_HOLD_FRAMES, and dropped if the trigger fires after all.
//  - A weapon touching a combo it already triggered isn't reported, and a Cooldown that hasn't
//    run out is one line for all positions, like a failure, which isn't reported again until
//    the trigger has been off cooldown for TRIGGER_LOG_COOLDOWN_GAP frames.
namespace {
struct trigger_log_where
{
	// "screen 05 layer 0", "pos 52 (4,3)", and both together.
	std::string place, spot, text;
	std::string short_text;
	// The trigger at this position, to find out if it fired after failing.
	std::string trigger;
};
struct trigger_log_event
{
	// Events with the same key are the same thing happening again. Empty for a trigger that did
	// nothing worth logging.
	std::string key;
	trigger_log_kind kind;
	int game_frame;
	// Where the player was.
	int map, screen;
	// How many triggers set this one off, each the one before (see trigger_log_depth).
	int depth;
	// How many frames it can stop happening for and still be the same thing happening again.
	int gap;
	// Empty for what isn't at a position, like a trigger group's count.
	trigger_log_where where;
	std::string message, short_message;
	// The values behind a failed condition. They aren't part of the key, so that a Proximity
	// failure as the player walks around is still the same thing happening again.
	std::string detail;
};
struct trigger_log_line
{
	trigger_log_kind kind;
	// When it happened: trigger_log_frame, and global_frame for the printed line.
	int frame, game_frame;
	// Where the player was.
	int map, screen;
	int depth;
	std::string message, short_message, detail;
	std::vector<trigger_log_where> wheres;
};
struct trigger_log_streak
{
	trigger_log_line line;
	int first_frame, last_frame, gap;
};
constexpr int TRIGGER_LOG_HOLD_FRAMES = 20;
constexpr int TRIGGER_LOG_STREAK_FRAMES = 60;
constexpr int TRIGGER_LOG_STREAK_END_FRAMES = 10;
constexpr int TRIGGER_LOG_COOLDOWN_GAP = 60;
std::vector<trigger_log_event> trigger_log_events;
std::map<std::string, trigger_log_streak> trigger_log_streaks;
// Lines of failures being held, and when each trigger (trigger_log_where::trigger) last fired.
std::vector<trigger_log_line> trigger_log_held;
std::map<std::string, int> trigger_log_fired;
// The map and screen the last line was printed on.
std::pair<int, int> trigger_log_screen = {-1, -1};
// The trigger groups' counts, as last reported.
std::map<int, int> trigger_log_group_counts;
int trigger_log_frame;
// How many triggers are running, each set off by the one before.
int trigger_log_depth;
// What sets off the triggers being run (see TriggerLogCause), or null.
std::string const* trigger_log_cause;
trigger_log_highlights_t trigger_log_highlights_list;
std::deque<trigger_log_screen_line> trigger_log_screen_lines_list;
}

void TriggerLogCause::begin(std::string new_name)
{
	if (new_name.empty())
		return;
	name = std::move(new_name);
	prev = trigger_log_cause;
	trigger_log_cause = &name;
	active = true;
}

TriggerLogCause::~TriggerLogCause()
{
	if (active)
		trigger_log_cause = prev;
}

// Where a trigger is, and the start of its line: which trigger, and what set it off.
static std::pair<trigger_log_where, std::pair<std::string, std::string>> trigger_log_describe(
	const combined_handle_t& handle, int cid, size_t idx, weapon* w, std::string const& cause)
{
	trigger_log_where where;
	if (handle.is_ffc())
	{
		where.place = fmt::format("screen {:02X}", handle.get_screen());
		where.spot = fmt::format("FFC {}", handle.local_id() + 1);
		where.short_text = fmt::format("{:02X} FFC {}", handle.get_screen(), handle.local_id() + 1);
	}
	else
	{
		int pos = handle.local_id();
		where.place = fmt::format("screen {:02X} layer {}", handle.get_screen(), handle.get_layer());
		where.spot = fmt::format("pos {} ({},{})", pos, pos % 16, pos / 16);
		where.short_text = handle.get_layer()
			? fmt::format("{:02X} L{} ({},{})", handle.get_screen(), handle.get_layer(), pos % 16, pos / 16)
			: fmt::format("{:02X} ({},{})", handle.get_screen(), pos % 16, pos / 16);
	}
	where.text = fmt::format("{} {}", where.place, where.spot);
	where.trigger = fmt::format("{} {} {}", where.text, cid, idx);

	// The trigger's effects (a script, say) could have removed it.
	auto const& triggers = combobuf[cid].triggers;
	std::string label = idx < triggers.size() ? triggers[idx].label : "";
	std::string message = fmt::format("combo {} trigger {}({:03})", cid,
		label.empty() ? "" : fmt::format("'{}' ", label), idx);
	std::string short_message = label.empty() ? fmt::format("combo {} ({:03})", cid, idx) : label;
	std::string by = cause;
	if (w)
		by = cause.empty() ? ZI.getWeapName(w->id) : fmt::format("{} ({})", cause, ZI.getWeapName(w->id));
	if (!by.empty())
	{
		message += fmt::format(", by {}", by);
		short_message += fmt::format(", by {}", by);
	}
	return {where, {message, short_message}};
}

// Fills in the event at `slot` of trigger_log_events, which trigger_log_begin_run reserved before
// the trigger ran, so that it comes before any triggers it set off (like copycats).
static void trigger_log(size_t slot, const combined_handle_t& handle, int cid, size_t idx, weapon* w,
	std::string const& cause, trig_outcome const& outcome, const char* fail_reason, std::string const& fail_detail)
{
	auto [where, messages] = trigger_log_describe(handle, cid, idx, w, cause);
	auto [message, short_message] = messages;
	std::string outcome_text = outcome.text, short_outcome = outcome.short_text, detail;
	if (outcome.kind == trigger_log_kind::failed)
	{
		outcome_text += fmt::format(" {}", fail_reason);
		short_outcome += fmt::format(" {}", fail_reason);
		detail = fail_detail;
	}
	message += fmt::format(": {}", outcome_text);
	short_message += fmt::format(": {}", short_outcome);

	// One line covers every position where a trigger fails the same way.
	int depth = trigger_log_depth;
	std::string key = outcome.every_position
		? fmt::format("{} {}", depth, message)
		: fmt::format("{} {} {}", depth, where.text, message);
	int gap = &outcome == &TRIG_COOLDOWN ? TRIGGER_LOG_COOLDOWN_GAP : 1;
	trigger_log_events[slot] = {key, outcome.kind, global_frame, cur_map, cur_screen, depth, gap, where, message, short_message, detail};

	if (outcome.kind != trigger_log_kind::other)
	{
		bool fired = outcome.kind == trigger_log_kind::fired;
		auto [x, y] = handle.xy();
		int width = 16, height = 16;
		if (handle.is_ffc())
		{
			width = handle.get_ffc().ffc->hit_width;
			height = handle.get_ffc().ffc->hit_height;
		}
		trigger_log_highlight highlight = {x.getInt(), y.getInt(), width, height, fired, trigger_log_frame};
		trigger_log_highlights_list[{highlight.x, highlight.y, width, height, fired}] = highlight;
	}
}

void trigger_log_note(const combined_handle_t& handle, int cid, size_t idx, weapon* w, std::string const& text)
{
	if (!show_trigger_log)
		return;
	std::string cause = trigger_log_cause ? *trigger_log_cause : "";
	auto [where, messages] = trigger_log_describe(handle, cid, idx, w, cause);
	auto [message, short_message] = messages;
	message += fmt::format(": {}", text);
	short_message += fmt::format(": {}", text);
	trigger_log_events.push_back({fmt::format("{} {} {}", trigger_log_depth, where.text, message), trigger_log_kind::other,
		global_frame, cur_map, cur_screen, trigger_log_depth, 1, where, message, short_message, ""});
}

void trigger_log_group_count(combo_trigger const& trig)
{
	int group = trig.trig_group, count = cpos_trig_group_count(group);
	auto [it, inserted] = trigger_log_group_counts.try_emplace(group, count);
	if (!inserted && it->second == count)
		return;
	std::vector<std::string> waits;
	if (trig.trigger_flags.get(TRIGFLAG_TGROUP_LESS))
		waits.push_back(fmt::format("< {}", trig.trig_group_val));
	if (trig.trigger_flags.get(TRIGFLAG_TGROUP_GREATER))
		waits.push_back(fmt::format("> {}", trig.trig_group_val));
	std::string message = fmt::format("Trigger Group {} count is {}{}, and a trigger waits for {}", group, count,
		inserted ? "" : fmt::format(" (was {})", it->second), fmt::join(waits, " or "));
	it->second = count;
	trigger_log_events.push_back({message, trigger_log_kind::other, global_frame, cur_map, cur_screen, trigger_log_depth, 1, {}, message,
		message, ""});
}

static void trigger_log_add_where(std::vector<trigger_log_where>& wheres, trigger_log_where const& where)
{
	if (where.text.empty())
		return;
	for (auto const& w : wheres)
		if (w.text == where.text)
			return;
	wheres.push_back(where);
}

static void trigger_log_print(trigger_log_line const& line)
{
	if (std::pair(line.map, line.screen) != trigger_log_screen)
	{
		trigger_log_screen = {line.map, line.screen};
		Z_eventlog_force("[Trigger] ---- map %d screen %02X ----\n", line.map + 1, line.screen);
	}

	std::string where, short_where;
	if (line.wheres.size() == 1)
	{
		where = line.wheres[0].text + ": ";
		short_where = line.wheres[0].short_text + " ";
	}
	else if (!line.wheres.empty())
	{
		// The first few, each screen and layer named once.
		constexpr size_t MAX_SPOTS = 6;
		std::vector<std::pair<std::string, std::vector<std::string>>> places;
		size_t shown = std::min(line.wheres.size(), MAX_SPOTS);
		for (size_t i = 0; i < shown; i++)
		{
			auto const& w = line.wheres[i];
			auto it = std::find_if(places.begin(), places.end(), [&](auto const& p) { return p.first == w.place; });
			if (it == places.end())
				it = places.insert(places.end(), {w.place, {}});
			it->second.push_back(w.spot);
		}
		std::vector<std::string> parts;
		for (auto const& [place, spots] : places)
			parts.push_back(fmt::format("{} {}", place, fmt::join(spots, ", ")));
		where = fmt::format("{} positions ({}{}): ", line.wheres.size(), fmt::join(parts, "; "),
			line.wheres.size() > shown ? fmt::format(", and {} more", line.wheres.size() - shown) : "");
		short_where = fmt::format("{}x ", line.wheres.size());
	}
	// The triggers a trigger set off are indented under it.
	std::string indent(line.depth * 2, ' ');
	std::string detail = line.detail.empty() ? "" : fmt::format(" ({})", line.detail);
	Z_eventlog_force("[Trigger f%d] %s%s%s%s\n", line.game_frame, indent.c_str(), where.c_str(), line.message.c_str(), detail.c_str());

	trigger_log_screen_lines_list.push_back({indent + short_where + line.short_message, trigger_log_frame});
	while (trigger_log_screen_lines_list.size() > TRIGGER_LOG_SCREEN_LINES)
		trigger_log_screen_lines_list.pop_front();
}

// Prints last frame's events. What happened in the same way at many positions is one line.
static void trigger_log_flush()
{
	int frame = trigger_log_frame;
	std::set<std::string> fired_at;
	for (auto const& event : trigger_log_events)
	{
		if (event.kind == trigger_log_kind::fired && !event.key.empty())
		{
			fired_at.insert(event.where.text);
			trigger_log_fired[event.where.trigger] = frame;
		}
	}

	// The lines, in the order they happened. Those with the same message are one line.
	std::vector<trigger_log_line> lines;
	std::map<std::pair<int, std::string>, size_t> line_of;
	auto add_line = [&](trigger_log_line line) {
		auto [it, inserted] = line_of.try_emplace({line.depth, line.message}, lines.size());
		if (inserted)
			lines.push_back(std::move(line));
		else
			for (auto const& where : line.wheres)
				trigger_log_add_where(lines[it->second].wheres, where);
	};

	for (auto const& event : trigger_log_events)
	{
		if (event.key.empty())
			continue;
		// Another trigger at the same position fired instead.
		if (event.kind == trigger_log_kind::failed && fired_at.contains(event.where.text))
			continue;

		trigger_log_line line = {event.kind, frame, event.game_frame, event.map, event.screen, event.depth,
			event.message, event.short_message, event.detail, {}};
		trigger_log_add_where(line.wheres, event.where);
		auto it = trigger_log_streaks.find(event.key);
		if (it == trigger_log_streaks.end())
		{
			trigger_log_streaks[event.key] = {line, frame, frame, event.gap};
			add_line(std::move(line));
			continue;
		}

		auto& streak = it->second;
		bool first_this_frame = streak.last_frame < frame;
		streak.last_frame = frame;
		trigger_log_add_where(streak.line.wheres, event.where);
		// Another position of something that started this frame.
		if (streak.first_frame == frame)
		{
			add_line(std::move(line));
			continue;
		}
		// Still firing.
		if (first_this_frame && event.kind == trigger_log_kind::fired
			&& frame - streak.first_frame + 1 == TRIGGER_LOG_STREAK_FRAMES)
		{
			line.message += fmt::format(" (every frame for the last {} frames, and counting)", TRIGGER_LOG_STREAK_FRAMES);
			line.short_message += " (every frame)";
			add_line(std::move(line));
		}
	}
	trigger_log_events.clear();

	// Report the triggers that stopped firing every frame, and forget what can't happen again.
	std::erase_if(trigger_log_streaks, [&](auto const& pair) {
		auto const& streak = pair.second;
		if (streak.last_frame >= frame + 1 - streak.gap)
			return false;
		int frames = streak.last_frame - streak.first_frame + 1;
		if (streak.line.kind == trigger_log_kind::fired && frames >= TRIGGER_LOG_STREAK_END_FRAMES)
		{
			auto line = streak.line;
			line.game_frame = global_frame;
			line.map = cur_map;
			line.screen = cur_screen;
			line.message += fmt::format(" (for {} frames, until now)", frames);
			line.short_message += fmt::format(" ({} frames)", frames);
			add_line(std::move(line));
		}
		return true;
	});

	for (auto& line : lines)
	{
		if (line.kind == trigger_log_kind::failed)
			trigger_log_held.push_back(std::move(line));
		else
			trigger_log_print(line);
	}

	// Print the failures held long enough, leaving out the positions where the trigger has fired
	// since.
	std::erase_if(trigger_log_held, [&](trigger_log_line& line) {
		std::erase_if(line.wheres, [&](trigger_log_where const& where) {
			auto it = trigger_log_fired.find(where.trigger);
			return it != trigger_log_fired.end() && it->second >= line.frame;
		});
		if (line.wheres.empty())
			return true;
		if (line.frame > frame - TRIGGER_LOG_HOLD_FRAMES)
			return false;
		trigger_log_print(line);
		return true;
	});
	std::erase_if(trigger_log_fired, [&](auto const& pair) {
		return pair.second < frame - TRIGGER_LOG_HOLD_FRAMES;
	});
}

void trigger_log_start_frame()
{
	if (!show_trigger_log)
	{
		trigger_log_events.clear();
		trigger_log_streaks.clear();
		trigger_log_held.clear();
		trigger_log_fired.clear();
		trigger_log_group_counts.clear();
		trigger_log_screen = {-1, -1};
		trigger_log_highlights_list.clear();
		trigger_log_screen_lines_list.clear();
		return;
	}

	trigger_log_flush();
	++trigger_log_frame;
	std::erase_if(trigger_log_highlights_list, [](auto const& pair) {
		return pair.second.frame < trigger_log_frame - TRIGGER_LOG_HIGHLIGHT_FRAMES;
	});
	while (!trigger_log_screen_lines_list.empty()
		&& trigger_log_screen_lines_list.front().frame < trigger_log_frame - TRIGGER_LOG_SCREEN_LINE_FRAMES)
		trigger_log_screen_lines_list.pop_front();
}

int trigger_log_current_frame()
{
	return trigger_log_frame;
}

std::deque<trigger_log_screen_line> const& trigger_log_screen_lines()
{
	return trigger_log_screen_lines_list;
}

trigger_log_highlights_t const& trigger_log_highlights()
{
	return trigger_log_highlights_list;
}

trigger_log_run trigger_log_begin_run()
{
	trigger_log_run run;
	run.outer_cause = trigger_log_cause;
	if (show_trigger_log)
	{
		run.slot = trigger_log_events.size();
		trigger_log_events.emplace_back();
		if (trigger_log_cause)
			run.cause = *trigger_log_cause;
		// The triggers this one sets off name their own cause, and are indented under it.
		trigger_log_cause = nullptr;
		++trigger_log_depth;
	}
	return run;
}

void trigger_log_end_run(trigger_log_run const& run, const combined_handle_t& handle, int cid, size_t idx,
	weapon* w, trig_outcome const* outcome, const char* fail_reason, std::string const& fail_detail)
{
	if (!run.slot)
		return;
	--trigger_log_depth;
	trigger_log_cause = run.outer_cause;
	if (outcome)
		trigger_log(*run.slot, handle, cid, idx, w, run.cause, *outcome, fail_reason, fail_detail);
}
