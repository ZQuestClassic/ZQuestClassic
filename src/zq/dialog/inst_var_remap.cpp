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
		auto& list = new_var_lists.emplace_back();
		list.add("(Discard values)", -1);
		for (auto& var : unmatched_new)
			list.add(describe_var(var, r.new_labels), var.id);

		// A lone rename is unambiguous enough to pre-select. Otherwise make
		// the user choose, rather than guess and carry values somewhere wrong.
		int guess = -1;
		if (unmatched_old.size() == 1 && unmatched_new.size() == 1
			&& unmatched_old[0].type_name == unmatched_new[0].type_name)
			guess = unmatched_new[0].id;

		for (auto& var : unmatched_old)
			if (r.unmatched_value_counts.contains(var.id))
				choices.push_back({&r, var.id, guess});
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
		names += remap.old_vars.at(conflict[i]->old_id).name;
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
					others.push_back(remap.old_vars.at(other->old_id).name);
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
	size_t list_index = 0;
	SlotRemap* prev_remap = nullptr;
	for (auto& choice : choices)
	{
		auto& r = *choice.remap;
		if (prev_remap && prev_remap != &r)
			list_index++;
		prev_remap = &r;

		auto old_var_label = Label(text = describe_var(r.remap.old_vars.at(choice.old_id), r.old_labels), hAlign = 0.0);
		int count = r.unmatched_value_counts.at(choice.old_id);
		old_var_label->setTooltip(fmt::format("Has values set in {} place{}.", count, count == 1 ? "" : "s"));

		grid->add(Label(text = fmt::format("{} '{}'", r.slot_type_name, r.script_name), hAlign = 0.0));
		grid->add(old_var_label);
		auto dropdown = DropDownList(
			data = new_var_lists[list_index],
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

	auto column = Column(
		Label(
			maxwidth = 40_em,
			text = "These script variables have values set (on FFCs, combos, items, etc.),"
				" but no longer exist after compiling. If a variable was renamed, choose its"
				" new name to keep its values. Otherwise, its values will be discarded."
				"\n\nCancel to leave all scripts as they were."
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
		title = "Renamed Script Variables?",
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
				if (choice.new_id != -1)
					choice.remap->remap.old_to_new[choice.old_id] = choice.new_id;
			confirmed = true;
			return true;
		}
		case message::CANCEL:
			return true;
	}
	return false;
}
