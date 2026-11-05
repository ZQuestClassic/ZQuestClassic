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

	struct hub
	{
		hub_kind kind;
		// More than one when identical setups were folded together (copycats 1 to 8). An
		// ExDoor id is dir * 256 + index. Unused for secrets.
		std::vector<int> ids;

		std::string name() const;
	};

	// A trigger's use of a hub. A trigger that sets a state, adds to a group or activates a
	// copycat "writes" it; one that a hub makes fire, or that requires it, "reads" it.
	struct link
	{
		size_t hub;
		bool writes, reads;
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
		int layer;
		// Combo position, or -1 for an FFC.
		int pos;
		int ffc = -1;
	};

	struct card
	{
		int cid;
		// Empty for a combo that is only reached by a combo change.
		std::vector<placement> placements;
		std::vector<trigger> triggers;
		// How many identical setups this card stands for, when they were folded together.
		int count = 1;
	};

	std::vector<card> cards;
	std::vector<hub> hubs;
	// Cards linked by combo changes, each in the order they change into each other.
	std::vector<std::vector<size_t>> chains;
};

// `layers` are the screen and its layers 1 to 6 (nullptr for a missing layer). FFCs are read
// from layer 0.
trigger_graph build_trigger_graph(std::array<mapscr*, 7> const& layers);

// The graph as JSON, as -dump-screen prints it and the editor's diagram copies it.
nlohmann::json trigger_graph_json(trigger_graph const& graph);

#endif
