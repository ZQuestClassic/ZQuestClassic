#ifndef CORE_TRIGGER_GRAPH_H_
#define CORE_TRIGGER_GRAPH_H_

#include <array>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

struct mapscr;

// How the combos on one screen trigger each other, in a form that can be drawn as a diagram:
// each combo's triggers as causes, conditions and effects, the combos they change into, and the
// copycats, trigger groups, states and secrets that link them.
//
// A floor switch that activates copycat 1 and changes into a pressed switch, and a door that
// copycat 1 opens. The switch has 'Copycat doesn't trigger this', so the copycat isn't one of
// its causes. Arrows point to what an index refers to.
//
//   cards[0]  cid 10, the switch                       hubs[0]  kind: copycat, ids: {1}
//     placements: {screen 02, layer 0, pos 52}                  name(): "Copycat 1"
//     triggers[0]                                               elsewhere: the combos on the
//       causes:  "Step->"                                       region's other screens that
//       effects: "Change combo by 1",                           use it
//                "Activate Copycat 1"                         ^
//       links:   {hub 0, writes} ---------------------------->|
//       changes_to: 1                                         |
//         |                                                   |
//         v                                                   |
//   cards[1]  cid 11, the pressed switch                      |
//     placements: {}  (only reached by the change)            |
//                                                             |
//   cards[2]  cid 20, the door                                |
//     placements: {screen 02, layer 0, pos 7}                 |
//     triggers[0]                                             |
//       causes:  "Copycat 1"                                  |
//       effects: "Triggers Secrets, Activate Copycat 1"       |
//       links:   {hub 0, reads} ------------------------------+
//                {hub 1, writes} ----------------------> hubs[1]  kind: secrets
//
//   chains: {{0, 1}, {2}}  each card's chain of combo changes, in order
//
// The door's copycat is only "reads": a trigger activates its copycat when something else
// fires it, and the door has no other cause.
//
// A link names a hub by its index in `hubs`. Setups that only differ in their numbers are
// folded together: eight such switches are one card with count 8, linked to one hub with ids
// {1, ..., 8} ("Copycats 1-8").
struct trigger_graph
{
	// Something that triggers share without being combos.
	enum class hub_kind
	{
		copycat,
		trigger_group,
		exstate,
		exdoor,
		level_state,
		global_state,
		secrets,
	};

	// Combos on another screen of the region, which are not cards, that use a copycat or
	// trigger group. Copycats and trigger groups reach the whole region; the rest of the hubs
	// are either the screen's own or the quest's.
	struct elsewhere
	{
		int screen;
		// Placed combos whose triggers, or those of the combos they change into, use the hub.
		int count;
		bool writes, reads;
	};

	struct hub
	{
		hub_kind kind;
		// More than one when identical setups were folded together (copycats 1 to 8). An
		// ExDoor id is dir * 256 + index. Unused for secrets.
		std::vector<int> ids;
		// The screen whose secrets, ExState or ExDoor this is, when the cards come from more
		// than one screen. Else -1.
		int screen = -1;
		// Only when the graph is of one screen of a region.
		std::vector<trigger_graph::elsewhere> elsewhere;

		// With its screen, if it has one.
		std::string name() const;
		// Without its screen, as the trigger lines name it.
		std::string line_name() const;
	};

	// A trigger's use of a hub. A trigger that sets a state, adds to a group or activates a
	// copycat "writes" it; one that a hub makes fire, or that requires it, "reads" it.
	struct link
	{
		size_t hub;
		bool writes, reads;
		// The hub as the trigger's lines name it. Not always the hub's own name: a hub merged
		// from copycats 1-4 and copycat 3 is linked from a line about copycat 3.
		std::string name;
	};

	struct trigger
	{
		std::string label;
		// One line each, as combo_trigger::summarize() words them.
		std::vector<std::string> causes, conditions, effects;
		// Only set with 'Weapon Separate Triggers': what happens when a weapon fires it.
		// `causes` and `effects` are then only the non-weapon ones.
		std::vector<std::string> weapon_causes, weapon_effects;
		// What happens when a cause fires it but a condition fails.
		std::vector<std::string> fail_effects;
		// Settings that are neither a cause nor an effect, like contributing to a group.
		std::vector<std::string> notes;
		// A cause that fires for as long as it's true, or on a schedule (Step->, Timer, ...).
		bool keeps_firing = false;
		// The card the trigger's combo change leads to, or -1.
		int changes_to = -1;
		std::vector<link> links;
	};

	struct placement
	{
		int screen;
		int layer;
		// Combo position, or -1 for an FFC.
		int pos;
		int ffc = -1;
	};

	// A combo on a screen: the same combo on two screens is two cards, unless they're folded
	// together.
	struct card
	{
		int cid;
		// The screen the combo is on, or that the combo changing into it is on.
		int screen;
		// Empty for a combo that is only reached by a combo change.
		std::vector<placement> placements;
		std::vector<trigger> triggers;
		// How many different combos this card stands for, when identical setups were folded
		// together.
		int count = 1;
	};

	std::vector<card> cards;
	std::vector<hub> hubs;
	// Cards linked by combo changes, each in the order they change into each other.
	std::vector<std::vector<size_t>> chains;
};

// A screen and its layers 1 to 6 (nullptr for a missing layer). FFCs are read from layer 0.
struct trigger_graph_screen
{
	int screen;
	std::array<mapscr*, 7> layers;
};

// The screens of the scrolling region `screen` is in on `map`, or just `screen` when it isn't in
// one.
std::vector<trigger_graph_screen> trigger_graph_region(int map, int screen);

// The cards come from all of `screens`, or with `focus` only from that one of them; the other
// screens' combos are then noted on the copycats and trigger groups they share (hub::elsewhere).
trigger_graph build_trigger_graph(std::vector<trigger_graph_screen> const& screens, int focus = -1);

// The graph as JSON, as -dump-screen prints it and the editor's diagram copies it.
nlohmann::json trigger_graph_json(trigger_graph const& graph);

#endif
