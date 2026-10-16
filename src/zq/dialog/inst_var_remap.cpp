#include "zq/dialog/inst_var_remap.h"

#include "dialog/info.h"
#include "gui/builder.h"
#include "gui/jwin.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

static std::string describe_var(const InstVar& var, const std::map<word, std::string>& labels)
{
	auto it = labels.find(var.id);
	if (it == labels.end())
		return fmt::format("{} {}", var.type_name, var.name);
	return fmt::format("{} {} (\"{}\")", var.type_name, var.name, it->second);
}

std::string InstVarRemapDialog::old_name(const Choice& choice)
{
	auto& remap = choice.remap->remap;
	return choice.initd ? remap.old_params[choice.old_id].name : remap.old_vars.at(choice.old_id).name;
}

bool call_inst_var_remap_dialog(std::vector<SlotRemap>& remaps)
{
	InstVarRemapDialog dlg(remaps);
	dlg.show();
	return dlg.confirmed;
}

InstVarRemapDialog::InstVarRemapDialog(std::vector<SlotRemap>& remaps)
{
	for (auto& r : remaps)
	{
		if (!r.needs_user_mapping())
			continue;

		auto unmatched_old = r.remap.unmatched_old();
		auto unmatched_new = r.remap.unmatched_new();
		// Any exported variable not taking an old variable of the same name.
		auto& list = new_var_lists.emplace_back();
		list.add("(Discard values)", -1);
		for (auto& [id, var] : r.remap.new_vars)
		{
			bool matched = std::any_of(r.remap.old_to_new.begin(), r.remap.old_to_new.end(),
				[&](auto& p){ return p.second == id; });
			if (!matched && r.new_labels.contains(id))
				list.add(describe_var(var, r.new_labels), id);
		}

		// A converted script's InitD parameters, each initially going to the
		// variable of the same name.
		for (auto& param : r.remap.old_params)
		{
			if (!r.initd_value_counts.contains(param.id))
				continue;
			int new_id = -1;
			for (auto& move : r.remap.initd_moves)
				if (move.initd == param.id)
					new_id = move.new_id;
			choices.push_back({&r, param.id, new_id, true, new_var_lists.size() - 1});
		}

		// A lone rename is unambiguous enough to pre-select. Otherwise make
		// the user choose, rather than guess and carry values somewhere wrong.
		int guess = -1;
		if (unmatched_old.size() == 1 && unmatched_new.size() == 1
			&& unmatched_old[0].type_name == unmatched_new[0].type_name)
			guess = unmatched_new[0].id;

		for (auto& var : unmatched_old)
			if (r.unmatched_value_counts.contains(var.id))
				choices.push_back({&r, var.id, guess, false, new_var_lists.size() - 1});
	}
}

std::vector<std::vector<const InstVarRemapDialog::Choice*>> InstVarRemapDialog::find_conflicts() const
{
	std::vector<std::vector<const Choice*>> conflicts;
	for (size_t i = 0; i < choices.size(); i++)
	{
		auto& a = choices[i];
		if (a.new_id == -1)
			continue;
		bool seen = false;
		for (size_t j = 0; j < i && !seen; j++)
			seen = choices[j].remap == a.remap && choices[j].new_id == a.new_id;
		if (seen)
			continue;

		std::vector<const Choice*> conflict = {&a};
		for (size_t j = i + 1; j < choices.size(); j++)
			if (choices[j].remap == a.remap && choices[j].new_id == a.new_id)
				conflict.push_back(&choices[j]);
		if (conflict.size() > 1)
			conflicts.push_back(std::move(conflict));
	}
	return conflicts;
}

std::string InstVarRemapDialog::describe_conflict(const std::vector<const Choice*>& conflict) const
{
	auto& remap = conflict[0]->remap->remap;
	std::string names;
	for (size_t i = 0; i < conflict.size(); i++)
	{
		if (i > 0)
			names += i + 1 == conflict.size() ? " and " : ", ";
		names += old_name(*conflict[i]);
	}
	return fmt::format("'{}': {} {} map to {}.", conflict[0]->remap->script_name, names,
		conflict.size() == 2 ? "both" : "all", remap.new_vars.at(conflict[0]->new_id).name);
}

void InstVarRemapDialog::update_conflicts()
{
	auto conflicts = find_conflicts();
	for (size_t i = 0; i < choices.size(); i++)
	{
		auto& choice = choices[i];
		std::string problem;
		for (auto& conflict : conflicts)
		{
			if (std::find(conflict.begin(), conflict.end(), &choice) == conflict.end())
				continue;
			auto& remap = choice.remap->remap;
			std::vector<std::string> others;
			for (auto* other : conflict)
				if (other != &choice)
					others.push_back(old_name(*other));
			problem = fmt::format("{} is also chosen for {}.\nOnly one of them can give it values.",
				remap.new_vars.at(choice.new_id).name, fmt::join(others, ", "));
		}
		dropdowns[i]->setInvalid(!problem.empty());
		dropdowns[i]->setTooltip(problem);
	}
}

std::shared_ptr<GUI::Widget> InstVarRemapDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	auto grid = Rows<3>(vAlign = 0.0,
		Label(text = "Script", hAlign = 0.0),
		Label(text = "Old Variable", hAlign = 0.0),
		Label(text = "New Variable", hAlign = 0.0)
	);

	dropdowns.clear();
	for (auto& choice : choices)
	{
		auto& r = *choice.remap;

		std::string old_text;
		int count;
		if (choice.initd)
		{
			auto& param = r.remap.old_params[choice.old_id];
			old_text = fmt::format("InitD[{}] {} {}", param.id, param.type_name, param.name);
			count = r.initd_value_counts.at(choice.old_id);
		}
		else
		{
			old_text = describe_var(r.remap.old_vars.at(choice.old_id), r.old_labels);
			count = r.unmatched_value_counts.at(choice.old_id);
		}
		auto old_var_label = Label(text = old_text, hAlign = 0.0);
		old_var_label->setTooltip(fmt::format("Has values set in {} place{}.", count, count == 1 ? "" : "s"));

		grid->add(Label(text = fmt::format("{} '{}'", r.slot_type_name, r.script_name), hAlign = 0.0));
		grid->add(old_var_label);
		auto dropdown = DropDownList(
			data = new_var_lists[choice.list_index],
			selectedValue = choice.new_id,
			fitParent = true,
			onSelectFunc = [&](int32_t val)
			{
				choice.new_id = val;
				update_conflicts();
			}
		);
		dropdowns.push_back(dropdown);
		grid->add(dropdown);
	}
	update_conflicts();

	bool any_initd = std::any_of(choices.begin(), choices.end(), [](auto& c){ return c.initd; });
	bool any_vars = std::any_of(choices.begin(), choices.end(), [](auto& c){ return !c.initd; });
	std::string intro;
	if (any_initd)
		intro += "These scripts' run() parameters (InitD) were replaced with variables. Each"
			" parameter's values (on FFCs, combos, items, etc.) are copied to the variable"
			" chosen for it - initially the one of the same name. InitD itself is left as is.";
	if (any_initd && any_vars)
		intro += "\n\n";
	if (any_vars)
		intro += "These script variables have values set (on FFCs, combos, items, etc.),"
			" but no longer exist after compiling. If a variable was renamed, choose its"
			" new name to keep its values. Otherwise, its values will be discarded.";
	auto column = Column(
		Label(
			maxwidth = 40_em,
			text = intro + "\n\nCancel to leave all scripts as they were."
		),
		ScrollingPane(
			fitParent = true,
			minheight = 24_em,
			grid
		)
	);
	column->add(Row(
		topPadding = 0.5_em,
		vAlign = 1.0,
		spacing = 2_em,
		Button(
			text = "OK",
			minwidth = 90_px,
			focused = true,
			onClick = message::OK),
		Button(
			text = "Cancel",
			minwidth = 90_px,
			onClick = message::CANCEL)
	));

	window = Window(
		title = any_initd ? "Move InitD Values?" : "Renamed Script Variables?",
		onClose = message::CANCEL,
		column
	);
	return window;
}

bool InstVarRemapDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::OK:
		{
			auto conflicts = find_conflicts();
			if (!conflicts.empty())
			{
				std::string text = "Each new variable can only take one old variable's values."
					" Choose a different variable, or discard, for all but one of:\n";
				for (auto& conflict : conflicts)
					text += "\n" + describe_conflict(conflict);
				InfoDialog("Conflicting Variables", text).show();
				return false;
			}

			for (auto& choice : choices)
			{
				auto& remap = choice.remap->remap;
				if (choice.initd)
				{
					std::erase_if(remap.initd_moves, [&](auto& move){ return move.initd == choice.old_id; });
					if (choice.new_id != -1)
						remap.initd_moves.push_back({choice.old_id, remap.old_params[choice.old_id].kind, (word)choice.new_id});
				}
				else if (choice.new_id != -1)
					remap.old_to_new[choice.old_id] = choice.new_id;
			}
			confirmed = true;
			return true;
		}
		case message::CANCEL:
			return true;
	}
	return false;
}
