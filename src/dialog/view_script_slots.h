#ifndef ZC_DIALOG_VIEWSCRIPTSLOTS_H_
#define ZC_DIALOG_VIEWSCRIPTSLOTS_H_

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/checkbox.h>
#include <gui/list.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>

void call_view_script_slots();

class ViewScriptSlotsDialog : public GUI::Dialog<ViewScriptSlotsDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	ViewScriptSlotsDialog();

	std::shared_ptr<GUI::Widget> view() override;
	void UpdateSlotList(ScriptType t);
	void RebuildLists();
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	int32_t last_script_type, last_slot;
	bool alphabetize, skipempty;
	std::shared_ptr<GUI::List> types_list, slots_list;

	GUI::ListData list_scripttypes, list_ffcslots, list_globalslots, list_itemslots,
		list_npcslots, list_lweaponslots, list_eweaponslots, list_heroslots,
		list_dmapslots, list_screenslots, list_itemspriteslots, list_comboslots,
		list_genericslots, list_subscreenslots;
};

#endif
