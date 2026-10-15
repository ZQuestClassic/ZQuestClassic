#ifndef ZQ_DIALOG_INST_VAR_REMAP_H_
#define ZQ_DIALOG_INST_VAR_REMAP_H_

#include "gui/dialog.h"
#include "gui/drop_down_list.h"
#include "gui/list_data.h"
#include "gui/window.h"
#include "zq/script_slot_remap.h"

#include <string>
#include <vector>

// Asks the user which new variable each unmatched variable (with values set)
// was renamed to. Adds their choices to each remap. Returns false if cancelled.
bool call_inst_var_remap_dialog(std::vector<SlotRemap>& remaps);

class InstVarRemapDialog: public GUI::Dialog<InstVarRemapDialog>
{
public:
	enum class message { OK, CANCEL };

	InstVarRemapDialog(std::vector<SlotRemap>& remaps);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

	bool confirmed = false;

private:
	struct Choice
	{
		SlotRemap* remap;
		word old_id;
		// -1 to discard the values.
		int new_id;
	};

	// Rows of one script mapped to the same new variable. Only one can be.
	std::vector<std::vector<const Choice*>> find_conflicts() const;
	std::string describe_conflict(const std::vector<const Choice*>& conflict) const;

	// Marks the dropdowns of rows in conflict invalid.
	void update_conflicts();

	std::vector<Choice> choices;
	// One per choice.
	std::vector<std::shared_ptr<GUI::DropDownList>> dropdowns;
	// One per remap that needs user mapping, in the same order.
	std::vector<GUI::ListData> new_var_lists;
	std::shared_ptr<GUI::Window> window;
};

#endif
