// Tests for build_trigger_graph() (core/trigger_graph.h) with screens of a scrolling region:
// which hubs the screens share, which are each screen's own, and the notes a graph of one
// screen gets about the combos on the others. One-screen graphs are covered by
// tests/test_zplayer.py, from the trigger example quest.
//
// Run with `zplayer -test-zc <tests dir>`.

#include "core/combo.h"
#include "core/mapscr.h"
#include "core/trigger_graph.h"
#include "test_runner/test_runner.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <memory>
#include <string>
#include <vector>

namespace {

enum
{
	SWITCH = 10,  // Step->: activates copycat 1
	DOOR = 11,    // copycat 1: secrets
	CHEST = 20,   // Step->: sets ExState 0
	SWITCH2 = 30, // Step->: activates copycat 2
	DOOR2 = 31,   // copycat 2: secrets
	BELL = 40,    // Step->: contributes to trigger group 3
};

combo_trigger& add_trigger(int cid)
{
	combo_trigger& trig = combobuf[cid].triggers.emplace_back();
	trig.clear();
	return trig;
}

void make_combos()
{
	combobuf.assign(64, newcombo());
	add_trigger(SWITCH).trigger_flags.set(TRIGFLAG_STEP, true);
	combobuf[SWITCH].triggers[0].trigcopycat = 1;
	add_trigger(DOOR).trigcopycat = 1;
	combobuf[DOOR].triggers[0].trigger_flags.set(TRIGFLAG_SECRETS, true);
	add_trigger(CHEST).trigger_flags.set(TRIGFLAG_STEP, true);
	combobuf[CHEST].triggers[0].exstate = 0;
	add_trigger(SWITCH2).trigger_flags.set(TRIGFLAG_STEP, true);
	combobuf[SWITCH2].triggers[0].trigcopycat = 2;
	add_trigger(DOOR2).trigcopycat = 2;
	combobuf[DOOR2].triggers[0].trigger_flags.set(TRIGFLAG_SECRETS, true);
	add_trigger(BELL).trigger_flags.set(TRIGFLAG_STEP, true);
	combobuf[BELL].triggers[0].trigger_flags.set(TRIGFLAG_TGROUP_CONTRIB, true);
	combobuf[BELL].triggers[0].trig_group = 3;
}

// Screens 0x00 and 0x01 of a region, with the combos placed at the given positions.
struct Region
{
	std::unique_ptr<mapscr> scrs[2];
	std::vector<trigger_graph_screen> screens;

	Region(std::vector<std::pair<int, int>> const& left, std::vector<std::pair<int, int>> const& right)
	{
		for (int i = 0; i < 2; i++)
		{
			scrs[i] = std::make_unique<mapscr>();
			for (auto [pos, cid] : i ? right : left)
				scrs[i]->data[pos] = cid;
			screens.push_back({i, {scrs[i].get()}});
		}
	}
};

int failures;

void expect(bool ok, std::string const& what)
{
	if (!ok)
	{
		fmt::println("  failed: {}", what);
		failures++;
	}
}

template<typename T>
void expect_eq(T const& got, T const& want, std::string const& what)
{
	if (!(got == want))
	{
		fmt::println("  failed: {}: got {}, expected {}", what, got, want);
		failures++;
	}
}

trigger_graph::card const* find_card(trigger_graph const& g, int cid, int screen = -1)
{
	for (auto const& card : g.cards)
		if (card.cid == cid && (screen < 0 || card.screen == screen))
			return &card;
	return nullptr;
}

std::vector<std::string> hub_names(trigger_graph const& g, trigger_graph::card const* card)
{
	std::vector<std::string> names;
	if (card)
		for (auto const& link : card->triggers.at(0).links)
			names.push_back(g.hubs[link.hub].name());
	return names;
}

// A switch on one screen opens a door on the other through a copycat, which the whole region
// shares; the door's secrets are its own screen's.
void test_copycat_across_screens()
{
	Region region({{5, SWITCH}}, {{7, DOOR}});
	auto g = build_trigger_graph(region.screens);
	expect_eq(g.cards.size(), size_t(2), "cards");
	auto switch_card = find_card(g, SWITCH), door = find_card(g, DOOR);
	expect(switch_card && door, "both combos are cards");
	if (!switch_card || !door)
		return;
	expect_eq(switch_card->screen, 0, "switch screen");
	expect_eq(door->screen, 1, "door screen");
	expect_eq(door->placements.at(0).screen, 1, "door placement screen");
	expect_eq(hub_names(g, switch_card), std::vector<std::string>{"Copycat 1"}, "switch hubs");
	expect_eq(hub_names(g, door), (std::vector<std::string>{"Copycat 1", "Secrets (0x01)"}), "door hubs");
	expect(switch_card->triggers[0].links[0].hub == door->triggers[0].links[0].hub, "one copycat for both");
	for (auto const& hub : g.hubs)
		expect(hub.elsewhere.empty(), "nothing is elsewhere in a graph of the whole region");
}

// Of one screen, the combos on the other screen that share a copycat are noted on it.
void test_elsewhere()
{
	Region region({{5, SWITCH}}, {{7, DOOR}, {8, DOOR}, {9, CHEST}});
	auto g = build_trigger_graph(region.screens, 0);
	expect_eq(g.cards.size(), size_t(1), "cards");
	expect_eq(hub_names(g, find_card(g, SWITCH)), std::vector<std::string>{"Copycat 1"}, "hubs");
	if (g.hubs.empty())
		return;
	auto const& elsewhere = g.hubs[0].elsewhere;
	expect_eq(elsewhere.size(), size_t(1), "elsewhere");
	if (elsewhere.empty())
		return;
	expect_eq(elsewhere[0].screen, 1, "elsewhere screen");
	expect_eq(elsewhere[0].count, 2, "elsewhere count");
	expect(!elsewhere[0].writes && elsewhere[0].reads, "the doors are fired by the copycat");

	// And from the other side: the switch activates the doors' copycat.
	g = build_trigger_graph(region.screens, 1);
	expect_eq(hub_names(g, find_card(g, DOOR)), (std::vector<std::string>{"Copycat 1", "Secrets"}), "door hubs");
	for (auto const& hub : g.hubs)
	{
		if (hub.kind != trigger_graph::hub_kind::copycat)
		{
			expect(hub.elsewhere.empty(), fmt::format("nothing elsewhere for {}", hub.name()));
			continue;
		}
		expect_eq(hub.elsewhere.size(), size_t(1), "elsewhere");
		// The switch activates the copycat, and is fired by it too.
		if (!hub.elsewhere.empty())
			expect(hub.elsewhere[0].writes && hub.elsewhere[0].reads, "the switch activates the copycat");
	}
}

// The same combo setting an ExState on two screens sets two different ExStates, so its two
// cards aren't folded together.
void test_screen_states()
{
	Region region({{5, CHEST}}, {{5, CHEST}});
	auto g = build_trigger_graph(region.screens);
	expect_eq(g.cards.size(), size_t(2), "cards");
	expect_eq(hub_names(g, find_card(g, CHEST, 0)), std::vector<std::string>{"ExState 0 (0x00)"}, "left hubs");
	expect_eq(hub_names(g, find_card(g, CHEST, 1)), std::vector<std::string>{"ExState 0 (0x01)"}, "right hubs");
}

// Setups that only differ in their copycats fold together across screens too.
void test_fold_across_screens()
{
	Region region({{5, SWITCH}, {6, BELL}}, {{5, SWITCH2}, {6, BELL}});
	auto g = build_trigger_graph(region.screens);
	auto switch_card = find_card(g, SWITCH);
	expect(switch_card && !find_card(g, SWITCH2), "the switches are folded");
	if (switch_card)
	{
		expect_eq(switch_card->count, 2, "switch count");
		expect_eq(switch_card->placements.size(), size_t(2), "switch placements");
		expect_eq(hub_names(g, switch_card), std::vector<std::string>{"Copycats 1, 2"}, "switch hubs");
	}
	// The same combo on both screens is one combo, placed twice.
	auto bell = find_card(g, BELL);
	expect(bell && !find_card(g, BELL, 1), "the bells are folded");
	if (bell)
	{
		expect_eq(bell->count, 1, "bell count");
		expect_eq(bell->placements.size(), size_t(2), "bell placements");
	}
}

// A hub that two folded switches activate is merged with the door's copycat. The door's line
// still names copycat 1, which its link keeps, so the diagram can find the line.
void test_merged_hub_names()
{
	Region region({{5, SWITCH}, {7, DOOR}}, {{5, SWITCH2}});
	auto g = build_trigger_graph(region.screens);
	auto switch_card = find_card(g, SWITCH), door = find_card(g, DOOR);
	expect(switch_card && door, "both combos are cards");
	if (!switch_card || !door)
		return;
	expect_eq(hub_names(g, door), (std::vector<std::string>{"Copycats 1, 2", "Secrets (0x00)"}), "door hubs");
	expect_eq(door->triggers[0].links[0].name, std::string("Copycat 1"), "door link name");
	expect_eq(switch_card->triggers[0].links[0].name, std::string("Copycats 1, 2"), "switch link name");
}

} // namespace

TestResults test_trigger_graph(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "copycat_across_screens", test_copycat_across_screens },
		{ "elsewhere", test_elsewhere },
		{ "screen_states", test_screen_states },
		{ "fold_across_screens", test_fold_across_screens },
		{ "merged_hub_names", test_merged_hub_names },
	};

	auto old_combos = std::move(combobuf);
	make_combos();
	for (auto& test : tests)
	{
		++tr.total;
		failures = 0;
		try
		{
			test.fn();
		}
		catch (std::exception const& e)
		{
			fmt::println("  failed: {}", e.what());
			failures++;
		}
		if (failures)
		{
			++tr.failed;
			fmt::println("  [FAIL] {}", test.name);
		}
		else if (verbose)
			fmt::println("  [PASS] {}", test.name);
	}
	combobuf = std::move(old_combos);

	return tr;
}
