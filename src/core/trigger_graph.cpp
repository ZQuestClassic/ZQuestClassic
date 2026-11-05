// Builds a trigger_graph from a screen: the model behind -dump-screen's "trigger_graph" and the
// editor's Screen > Combo Triggers diagram (zq/gui/trigger_diagram.cpp).
//
// build_trigger_graph() runs three steps:
//
// 1. add_cards: every combo with triggers on the screen, its layers and its FFCs becomes a card
//    (changer FFCs never fire, so they're left out). Then the combo changes are followed to the
//    combos they lead to, placed or not, until MAX_CARDS. Each trigger's text comes from
//    combo_trigger::summarize_parts(), split into one line per item, so the graph words a
//    trigger exactly as the Combo Editor's Summarize does. Its links to hubs (copycats, trigger
//    groups, states, secrets) are read from the trigger's fields, not from that text.
//
// 2. make_chains: cards linked by combo changes are grouped (union-find) into chains, each
//    ordered from a combo nothing changes into. A chain is a row of the diagram.
//
// 3. fold: chains that do the same thing with different numbers (eight floor switches, each
//    with its own copycat) become one, with `count` set and the hubs' ids merged, so the
//    diagram shows "x8" and "Copycats 1-8" instead of eight copies. Two chains match when
//    chain_signature() is the same: their trigger lines with each hub's name swapped for its
//    kind and the order it first comes up in, so numbers don't matter but how the hubs are
//    shared does. The kept card's lines name the first setup's numbers ("Copycat 1"), found by
//    how summarize_parts() prints them (summary_name()), and are renamed to the merged hub
//    ("Copycats 1-8"). Then hubs of the same kind that share an id are merged, and the folded
//    away cards and the hubs nothing links to are dropped (merge_overlapping_hubs, compact).
//
// Nothing here runs the triggers: the graph is what their settings say. A condition on a state
// is only linked when it's about this screen or level; another screen's ExState isn't set by
// anything here.

#include "core/trigger_graph.h"
#include "base/util.h"
#include "core/combo.h"
#include "core/mapscr.h"
#include "ffc.h"
#include "zinfo.h"
#include <algorithm>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <map>
#include <numeric>

using hub_kind = trigger_graph::hub_kind;

// Combo changes can lead anywhere in the combo list, so stop following them eventually.
static const size_t MAX_CARDS = 512;

static const char* dir_name(int dir)
{
	static const char* names[] = {"Up", "Down", "Left", "Right"};
	return unsigned(dir) < 4 ? names[dir] : "?";
}

static std::string ids_str(std::vector<int> const& ids)
{
	bool contiguous = ids.size() > 2;
	for (size_t i = 1; i < ids.size(); i++)
		contiguous = contiguous && ids[i] == ids[i - 1] + 1;
	if (contiguous)
		return fmt::format("{}-{}", ids.front(), ids.back());
	return fmt::format("{}", fmt::join(ids, ", "));
}

// How combo_trigger::summarize() names one hub, so the names can be found in its lines.
static std::string summary_name(hub_kind kind, int id)
{
	switch (kind)
	{
		case hub_kind::copycat: return trigger_copycat_name(id);
		case hub_kind::trigger_group: return trigger_group_name(id);
		case hub_kind::exstate: return trigger_exstate_name(id);
		case hub_kind::exdoor: return trigger_exdoor_name(id / 256, id % 256);
		case hub_kind::level_state: return trigger_level_state_name(id);
		case hub_kind::global_state: return trigger_global_state_name(id);
		case hub_kind::secrets: return "Secrets";
	}
	return "";
}

std::string trigger_graph::hub::name() const
{
	if (kind == hub_kind::secrets)
		return "Secrets";
	if (ids.size() == 1)
		return summary_name(kind, ids[0]);
	if (kind == hub_kind::exdoor)
	{
		std::vector<std::string> names;
		for (int id : ids)
			names.push_back(fmt::format("{} {}", dir_name(id / 256), id % 256));
		return fmt::format("ExDoors {}", fmt::join(names, ", "));
	}
	// "Copycat 0" -> "Copycats 1-8"
	auto name = summary_name(kind, 0);
	return fmt::format("{}s {}", name.substr(0, name.rfind(' ')), ids_str(ids));
}

// Replaces `from` in `line` where it's the whole name: not followed by another digit, so
// "Copycat 1" doesn't match the start of "Copycat 12", and not the end of "Large Combo
// Copycat 1", which isn't a copycat the graph links.
static void replace_name(std::string& line, std::string const& from, std::string const& to)
{
	static const std::string large_combo = "Large Combo ";
	size_t at = 0;
	while ((at = line.find(from, at)) != std::string::npos)
	{
		size_t end = at + from.size();
		bool longer_name = at >= large_combo.size()
			&& line.compare(at - large_combo.size(), large_combo.size(), large_combo) == 0;
		if (longer_name || (end < line.size() && isdigit((unsigned char)line[end])))
		{
			at = end;
			continue;
		}
		line.replace(at, from.size(), to);
		at += to.size();
	}
}

// A summarize_parts() block as one line per item. Detail lines (indented further) are added
// to the item above them.
static std::vector<std::string> split_lines(std::string const& block)
{
	std::vector<std::string> lines;
	for (auto const& line : util::split(block, "\n"))
	{
		size_t start = line.find_first_not_of(' ');
		if (start == std::string::npos)
			continue;
		std::string text = line.substr(start);
		if (start > 3 && !lines.empty())
			lines.back() += ", " + text;
		else
			lines.push_back(text);
	}
	return lines;
}

static bool keeps_firing(combo_trigger const& trig)
{
	if (trig.trigtimer || trig.trigger_flags.get(TRIGFLAG_PUSH))
		return true;
	for (auto [flag, name] : every_frame_trigger_causes)
		if (trig.trigger_flags.get(flag))
			return true;
	return false;
}

static int change_target(int cid, combo_trigger const& trig)
{
	if (!trig.trigchange)
		return -1;
	int target = trig.trigger_flags.get(TRIGFLAG_CMB_CHANGE_ABSOLUTE) ? trig.trigchange : cid + trig.trigchange;
	if (target <= 0 || target >= int(combobuf.size()))
		return -1;
	return target;
}

namespace {

struct builder
{
	trigger_graph g;
	std::map<int, size_t> card_of_cid;
	std::map<std::pair<hub_kind, std::vector<int>>, size_t> hub_of;

	size_t get_hub(hub_kind kind, std::vector<int> ids)
	{
		std::sort(ids.begin(), ids.end());
		ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
		auto [it, inserted] = hub_of.try_emplace({kind, ids}, g.hubs.size());
		if (inserted)
			g.hubs.push_back({kind, ids});
		return it->second;
	}

	size_t get_card(int cid)
	{
		auto [it, inserted] = card_of_cid.try_emplace(cid, g.cards.size());
		if (inserted)
		{
			trigger_graph::card card;
			card.cid = cid;
			g.cards.push_back(card);
		}
		return it->second;
	}

	void add_link(trigger_graph::trigger& t, hub_kind kind, int id, bool writes, bool reads)
	{
		if (!writes && !reads)
			return;
		size_t hub = get_hub(kind, {id});
		for (auto& link : t.links)
		{
			if (link.hub == hub)
			{
				link.writes |= writes;
				link.reads |= reads;
				return;
			}
		}
		t.links.push_back({hub, writes, reads});
	}

	trigger_graph::trigger make_trigger(int cid, combo_trigger const& trig)
	{
		newcombo const& cmb = combobuf[cid];
		trigger_graph::trigger t;
		t.label = trig.label;
		auto parts = trig.summarize_parts(cmb, cid);
		t.causes = split_lines(parts.causes);
		t.conditions = split_lines(parts.conditions);
		t.effects = split_lines(parts.effects);
		if (parts.separate_weapon)
		{
			t.weapon_causes = split_lines(parts.weapon_causes);
			t.weapon_effects = split_lines(parts.weapon_effects);
		}
		t.fail_effects = split_lines(parts.fail_effects);
		if (trig.trigger_flags.get(TRIGFLAG_TGROUP_CONTRIB))
			t.notes.push_back(fmt::format("Contributes to Trigger Group {}", trig.trig_group));
		t.keeps_firing = keeps_firing(trig);

		auto const& f = trig.trigger_flags;
		if (trig.trigcopycat)
		{
			// The copycat is activated whenever something else fires the trigger.
			combo_trigger other_causes = trig;
			other_causes.trigcopycat = 0;
			auto other = other_causes.summarize_parts(cmb);
			bool has_other_cause = !other.causes.empty() || !other.weapon_causes.empty();
			add_link(t, hub_kind::copycat, trig.trigcopycat, has_other_cause,
				!f.get(TRIGFLAG_NO_COPYCAT_CAUSE));
		}
		if (f.get(TRIGFLAG_TGROUP_CONTRIB) || f.get(TRIGFLAG_TGROUP_LESS) || f.get(TRIGFLAG_TGROUP_GREATER))
			add_link(t, hub_kind::trigger_group, trig.trig_group, f.get(TRIGFLAG_TGROUP_CONTRIB),
				f.get(TRIGFLAG_TGROUP_LESS) || f.get(TRIGFLAG_TGROUP_GREATER));
		if (trig.exstate > -1)
			add_link(t, hub_kind::exstate, trig.exstate, true, !f.get(TRIGFLAG_UNSETEXSTATE));
		if (trig.exdoor_dir > -1)
			add_link(t, hub_kind::exdoor, trig.exdoor_dir * 256 + trig.exdoor_ind, true,
				!f.get(TRIGFLAG_UNSETEXDOOR));
		add_link(t, hub_kind::level_state, trig.trig_lstate, f.get(TRIGFLAG_LEVELSTATE),
			f.get(TRIGFLAG_TRIGLEVELSTATE));
		add_link(t, hub_kind::global_state, trig.trig_gstate, f.get(TRIGFLAG_GLOBALSTATE),
			f.get(TRIGFLAG_TRIGGLOBALSTATE));
		add_link(t, hub_kind::secrets, 0, f.get(TRIGFLAG_SECRETS), f.get(TRIGFLAG_SECRETSTR));

		// Conditions read states too, when they're about this screen and level.
		for (int q = 0; q < 256; q++)
			if (trig.req_global_state.get(q) || trig.unreq_global_state.get(q))
				add_link(t, hub_kind::global_state, q, false, true);
		if (trig.trigdmlevel < 0)
		{
			for (int q = 0; q < 32; q++)
				if ((trig.req_level_state | trig.unreq_level_state) & (1 << q))
					add_link(t, hub_kind::level_state, q, false, true);
		}
		if (trig.trigstatemap <= 0)
		{
			for (int q = 0; q < 32; q++)
				if (trig.req_screen_ex_state.get(q) || trig.unreq_screen_ex_state.get(q))
					add_link(t, hub_kind::exstate, q, false, true);
			for (int dir = 0; dir < 4; dir++)
				for (int q = 0; q < 8; q++)
					if (trig.req_screen_ex_door[dir].get(q) || trig.unreq_screen_ex_door[dir].get(q))
						add_link(t, hub_kind::exdoor, dir * 256 + q, false, true);
		}
		return t;
	}

	void add_cards(std::array<mapscr*, 7> const& layers)
	{
		auto has_triggers = [](int cid) {
			return cid > 0 && cid < int(combobuf.size()) && !combobuf[cid].triggers.empty();
		};
		for (int layer = 0; layer < 7; layer++)
		{
			mapscr* scr = layers[layer];
			if (!scr)
				continue;
			for (int pos = 0; pos < 176; pos++)
				if (has_triggers(scr->data[pos]))
					g.cards[get_card(scr->data[pos])].placements.push_back({layer, pos});
			if (layer == 0)
			{
				for (word i = 0; i < scr->numFFC(); i++)
				{
					auto const& ffc = scr->ffcs[i];
					// Changers never fire their triggers.
					if (has_triggers(ffc.data) && !(ffc.flags & ffc_changer))
						g.cards[get_card(ffc.data)].placements.push_back({0, -1, i});
				}
			}
		}

		// Follow the combo changes to the combos they lead to, even ones not on the screen. The
		// cards they add are walked too.
		for (size_t i = 0; i < g.cards.size(); i++)
		{
			int cid = g.cards[i].cid;
			for (auto const& trig : combobuf[cid].triggers)
			{
				int target = change_target(cid, trig);
				if (target >= 0 && (card_of_cid.contains(target) || g.cards.size() < MAX_CARDS))
					get_card(target);
			}
		}

		for (auto& card : g.cards)
		{
			newcombo const& cmb = combobuf[card.cid];
			for (auto const& trig : cmb.triggers)
			{
				auto t = make_trigger(card.cid, trig);
				int target = change_target(card.cid, trig);
				if (target >= 0 && card_of_cid.contains(target))
					t.changes_to = card_of_cid[target];
				card.triggers.push_back(std::move(t));
			}
		}
	}

	void make_chains()
	{
		size_t n = g.cards.size();
		std::vector<size_t> parent(n);
		std::iota(parent.begin(), parent.end(), 0);
		auto find = [&](size_t i) {
			while (parent[i] != i)
				i = parent[i] = parent[parent[i]];
			return i;
		};
		std::vector<std::vector<size_t>> next(n);
		std::vector<bool> has_incoming(n);
		for (size_t i = 0; i < n; i++)
		{
			for (auto const& t : g.cards[i].triggers)
			{
				if (t.changes_to < 0 || size_t(t.changes_to) == i)
					continue;
				next[i].push_back(t.changes_to);
				has_incoming[t.changes_to] = true;
				parent[find(i)] = find(t.changes_to);
			}
		}

		std::map<size_t, std::vector<size_t>> members;
		for (size_t i = 0; i < n; i++)
			members[find(i)].push_back(i);

		std::vector<bool> seen(n);
		for (auto& [_, cards] : members)
		{
			// Start at a combo nothing changes into (else the first one placed), and follow
			// the changes from there.
			size_t start = cards.front();
			for (size_t c : cards)
			{
				if (!has_incoming[c])
				{
					start = c;
					break;
				}
			}
			std::vector<size_t> chain = {start};
			seen[start] = true;
			for (size_t i = 0; i < chain.size(); i++)
			{
				for (size_t nc : next[chain[i]])
				{
					if (!seen[nc])
					{
						seen[nc] = true;
						chain.push_back(nc);
					}
				}
			}
			for (size_t c : cards)
				if (!seen[c])
					chain.push_back(c);
			g.chains.push_back(chain);
		}
		std::sort(g.chains.begin(), g.chains.end(), [](auto const& a, auto const& b) {
			return *std::min_element(a.begin(), a.end()) < *std::min_element(b.begin(), b.end());
		});
	}

	// What a chain does, from its triggers' lines, with each copycat, state and group named by
	// its kind and the order it first comes up in the chain instead of by its number. Chains
	// that only differ in those numbers match, but not ones that use them differently (one
	// copycat shared by two combos, or two copycats; an ExState or an ExDoor).
	std::string chain_signature(std::vector<size_t> const& chain)
	{
		std::map<size_t, int> ordinal;
		std::string sig;
		for (size_t c : chain)
		{
			sig += fmt::format("[type {}]", combobuf[g.cards[c].cid].type);
			for (auto const& t : g.cards[c].triggers)
			{
				for (auto const& link : t.links)
					ordinal.try_emplace(link.hub, int(ordinal.size()));
				for (auto const* lines : {&t.causes, &t.conditions, &t.effects, &t.weapon_causes, &t.weapon_effects, &t.fail_effects, &t.notes})
				{
					sig += "[";
					for (auto line : *lines)
					{
						// The combos the setups change into may have different labels.
						line = line.substr(0, line.find(", into "));
						for (auto const& link : t.links)
						{
							auto const& hub = g.hubs[link.hub];
							replace_name(line, summary_name(hub.kind, hub.ids[0]),
								fmt::format("{}#{}", int(hub.kind), ordinal[link.hub]));
						}
						sig += line + "|";
					}
					sig += "]";
				}
				auto it = std::find(chain.begin(), chain.end(), size_t(t.changes_to));
				sig += fmt::format("[changes to {}]", t.changes_to < 0 ? -1 : int(it - chain.begin()));
				for (auto const& link : t.links)
					sig += fmt::format("[{}#{} {} {}]", int(g.hubs[link.hub].kind), ordinal[link.hub], link.writes, link.reads);
			}
		}
		return sig;
	}

	// Folds chains that only differ in their copycat, state and group numbers into one, so a
	// screen with eight floor switches shows one "x8" switch instead of eight.
	void fold()
	{
		std::map<std::string, std::vector<size_t>> by_sig;
		std::vector<std::string> order;
		for (size_t i = 0; i < g.chains.size(); i++)
		{
			auto sig = chain_signature(g.chains[i]);
			if (!by_sig.contains(sig))
				order.push_back(sig);
			by_sig[sig].push_back(i);
		}

		std::vector<bool> removed(g.cards.size());
		std::vector<std::vector<size_t>> chains;
		for (auto const& sig : order)
		{
			auto const& group = by_sig[sig];
			auto const& chain = g.chains[group[0]];
			chains.push_back(chain);
			if (group.size() < 2)
				continue;

			for (size_t k = 0; k < chain.size(); k++)
			{
				auto& card = g.cards[chain[k]];
				card.count = group.size();
				for (size_t m = 1; m < group.size(); m++)
				{
					auto& other = g.cards[g.chains[group[m]][k]];
					card.placements.insert(card.placements.end(), other.placements.begin(), other.placements.end());
					removed[g.chains[group[m]][k]] = true;
				}
				for (size_t ti = 0; ti < card.triggers.size(); ti++)
				{
					auto& t = card.triggers[ti];
					// The combos the setups change into may have different labels.
					for (size_t e = 0; e < t.effects.size(); e++)
					{
						size_t into = t.effects[e].find(", into ");
						if (into == std::string::npos)
							continue;
						for (size_t m : group)
						{
							if (g.cards[g.chains[m][k]].triggers[ti].effects[e] != t.effects[e])
							{
								t.effects[e].resize(into);
								break;
							}
						}
					}
					for (size_t li = 0; li < t.links.size(); li++)
					{
						auto const& own = g.hubs[t.links[li].hub];
						if (own.kind == hub_kind::secrets)
							continue;
						std::vector<int> ids;
						for (size_t m : group)
						{
							auto const& other = g.cards[g.chains[m][k]].triggers[ti].links[li];
							auto const& other_ids = g.hubs[other.hub].ids;
							ids.insert(ids.end(), other_ids.begin(), other_ids.end());
						}
						int own_id = own.ids[0];
						hub_kind kind = own.kind;
						t.links[li].hub = get_hub(kind, ids);
						// The lines name the first setup's number; name them all.
						auto from = summary_name(kind, own_id);
						auto to = g.hubs[t.links[li].hub].name();
						for (auto* lines : {&t.causes, &t.conditions, &t.effects, &t.weapon_causes, &t.weapon_effects, &t.fail_effects, &t.notes})
							for (auto& line : *lines)
								replace_name(line, from, to);
						replace_name(t.label, from, to);
					}
				}
			}
		}
		g.chains = chains;
		merge_overlapping_hubs();
		compact(removed);
	}

	// Folding makes hubs of several numbers (copycats 1-8). Another hub of the same kind that
	// shares a number with one (copycats 1-4, or copycat 3) is the same thing, so they become
	// one hub and the lines between them show.
	void merge_overlapping_hubs()
	{
		size_t n = g.hubs.size();
		std::vector<size_t> parent(n);
		std::iota(parent.begin(), parent.end(), 0);
		auto find = [&](size_t i) {
			while (parent[i] != i)
				i = parent[i] = parent[parent[i]];
			return i;
		};
		for (size_t a = 0; a < n; a++)
		{
			for (size_t b = a + 1; b < n; b++)
			{
				auto const& ia = g.hubs[a].ids;
				auto const& ib = g.hubs[b].ids;
				if (g.hubs[a].kind == g.hubs[b].kind
					&& std::find_first_of(ia.begin(), ia.end(), ib.begin(), ib.end()) != ia.end())
					parent[find(a)] = find(b);
			}
		}

		std::map<size_t, std::vector<int>> ids;
		for (size_t h = 0; h < n; h++)
		{
			auto& merged = ids[find(h)];
			merged.insert(merged.end(), g.hubs[h].ids.begin(), g.hubs[h].ids.end());
		}
		std::vector<size_t> to(n);
		for (size_t h = 0; h < n; h++)
		{
			hub_kind kind = g.hubs[h].kind;
			to[h] = get_hub(kind, ids[find(h)]);
		}

		for (auto& card : g.cards)
		{
			for (auto& t : card.triggers)
			{
				std::vector<trigger_graph::link> links;
				for (auto link : t.links)
				{
					link.hub = to[link.hub];
					auto same = std::find_if(links.begin(), links.end(), [&](auto const& l) { return l.hub == link.hub; });
					if (same == links.end())
						links.push_back(link);
					else
					{
						same->writes |= link.writes;
						same->reads |= link.reads;
					}
				}
				t.links = std::move(links);
			}
		}
	}

	// Drops removed cards and the hubs nothing links to anymore.
	void compact(std::vector<bool> const& removed)
	{
		std::vector<int> new_index(g.cards.size(), -1);
		std::vector<trigger_graph::card> cards;
		for (size_t i = 0; i < g.cards.size(); i++)
		{
			if (removed[i])
				continue;
			new_index[i] = cards.size();
			cards.push_back(std::move(g.cards[i]));
		}

		std::vector<int> new_hub(g.hubs.size(), -1);
		std::vector<trigger_graph::hub> hubs;
		for (auto& card : cards)
		{
			for (auto& t : card.triggers)
			{
				if (t.changes_to >= 0)
					t.changes_to = new_index[t.changes_to];
				for (auto& link : t.links)
				{
					if (new_hub[link.hub] < 0)
					{
						new_hub[link.hub] = hubs.size();
						hubs.push_back(g.hubs[link.hub]);
					}
					link.hub = new_hub[link.hub];
				}
			}
		}
		for (auto& chain : g.chains)
			for (auto& c : chain)
				c = new_index[c];

		g.cards = std::move(cards);
		g.hubs = std::move(hubs);
	}
};

} // namespace

trigger_graph build_trigger_graph(std::array<mapscr*, 7> const& layers)
{
	builder b;
	b.add_cards(layers);
	b.make_chains();
	b.fold();
	return std::move(b.g);
}

nlohmann::json trigger_graph_json(trigger_graph const& graph)
{
	nlohmann::json jgraph;
	jgraph["hubs"] = nlohmann::json::array();
	for (auto const& hub : graph.hubs)
		jgraph["hubs"].push_back(hub.name());
	jgraph["cards"] = nlohmann::json::array();
	for (auto const& card : graph.cards)
	{
		const newcombo& cmb = combobuf[card.cid];
		nlohmann::json jcard = {
			{"combo", card.cid},
			{"type_name", ZI.getComboTypeName(cmb.type)},
			{"label", cmb.label},
			{"count", card.count},
			{"placements", nlohmann::json::array()},
			{"triggers", nlohmann::json::array()},
		};
		for (auto const& p : card.placements)
		{
			if (p.pos < 0)
				jcard["placements"].push_back({{"ffc", p.ffc}});
			else
				jcard["placements"].push_back({{"layer", p.layer}, {"pos", p.pos}});
		}
		for (auto const& t : card.triggers)
		{
			nlohmann::json jt = {
				{"label", t.label},
				{"causes", t.causes},
				{"conditions", t.conditions},
				{"effects", t.effects},
				{"keeps_firing", t.keeps_firing},
				{"changes_to", t.changes_to},
				{"links", nlohmann::json::array()},
			};
			if (!t.weapon_causes.empty() || !t.weapon_effects.empty())
			{
				jt["weapon_causes"] = t.weapon_causes;
				jt["weapon_effects"] = t.weapon_effects;
			}
			if (!t.fail_effects.empty())
				jt["fail_effects"] = t.fail_effects;
			if (!t.notes.empty())
				jt["notes"] = t.notes;
			for (auto const& link : t.links)
				jt["links"].push_back({{"hub", link.hub}, {"writes", link.writes}, {"reads", link.reads}});
			jcard["triggers"].push_back(jt);
		}
		jgraph["cards"].push_back(jcard);
	}
	jgraph["chains"] = graph.chains;
	return jgraph;
}
