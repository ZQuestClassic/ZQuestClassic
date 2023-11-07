#include "view_script_slots.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include <fmt/format.h>
#include "zc_list_data.h"

void call_view_script_slots()
{
	ViewScriptSlotsDialog().show();
}

ViewScriptSlotsDialog::ViewScriptSlotsDialog() :
	last_script_type(int(ScriptType::FFC)), last_slot(1),
	list_scripttypes(GUI::ZCListData::scripttypes()),
	alphabetize(false), skipempty(false)
{
	RebuildLists();
}

void ViewScriptSlotsDialog::UpdateSlotList(ScriptType t)
{
	switch (t)
	{
		case ScriptType::FFC:
			slots_list->setListData(list_ffcslots);
			break;
		case ScriptType::Global:
			slots_list->setListData(list_globalslots);
			break;
		case ScriptType::Item:
			slots_list->setListData(list_itemslots);
			break;
		case ScriptType::NPC:
			slots_list->setListData(list_npcslots);
			break;
		case ScriptType::Lwpn:
			slots_list->setListData(list_lweaponslots);
			break;
		case ScriptType::Ewpn:
			slots_list->setListData(list_eweaponslots);
			break;
		case ScriptType::Player:
			slots_list->setListData(list_heroslots);
			break;
		case ScriptType::DMap:
			slots_list->setListData(list_dmapslots);
			break;
		case ScriptType::Screen:
			slots_list->setListData(list_screenslots);
			break;
		case ScriptType::ItemSprite:
			slots_list->setListData(list_itemspriteslots);
			break;
		case ScriptType::Combo:
			slots_list->setListData(list_comboslots);
			break;
		case ScriptType::Generic:
			slots_list->setListData(list_genericslots);
			break;
		case ScriptType::EngineSubscreen:
			slots_list->setListData(list_subscreenslots);
			break;
	}
}

void ViewScriptSlotsDialog::RebuildLists()
{
	list_ffcslots = GUI::ZCListData::slots_ffc_script(alphabetize, skipempty);
	list_globalslots = GUI::ZCListData::slots_global_script(alphabetize, skipempty);
	list_itemslots = GUI::ZCListData::slots_itemdata_script(alphabetize, skipempty);
	list_npcslots = GUI::ZCListData::slots_npc_script(alphabetize, skipempty);
	list_lweaponslots = GUI::ZCListData::slots_lweapon_script(alphabetize, skipempty);
	list_eweaponslots = GUI::ZCListData::slots_eweapon_script(alphabetize, skipempty);
	list_heroslots = GUI::ZCListData::slots_hero_script(alphabetize, skipempty);
	list_dmapslots = GUI::ZCListData::slots_dmap_script(alphabetize, skipempty);
	list_screenslots = GUI::ZCListData::slots_screen_script(alphabetize, skipempty);
	list_itemspriteslots = GUI::ZCListData::slots_itemsprite_script(alphabetize, skipempty);
	list_comboslots = GUI::ZCListData::slots_combo_script(alphabetize, skipempty);
	list_genericslots = GUI::ZCListData::slots_generic_script(alphabetize, skipempty);
	list_subscreenslots = GUI::ZCListData::slots_subscreen_script(alphabetize, skipempty);
}

std::shared_ptr<GUI::Widget> ViewScriptSlotsDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	std::shared_ptr<GUI::Window> window =  Window(
		title = "View Script Slots",
		onClose = message::CANCEL,
		Column(
			Label(text = "This is a preview of all currently assigned script slots.\nTo assign new slots, use 'Compile'"),
			Rows<2>(
				types_list = List(minheight = 300_px, width = 100_px, fitParent = true,
					data = list_scripttypes,
					selectedValue = last_script_type,
					onSelectFunc = [&](int32_t val)
					{
						last_script_type = val;
						refresh_dlg();
					}),
				slots_list = List(minheight = 300_px, fitParent = true,
					selectedValue = last_slot,
					data = list_ffcslots, isABC = alphabetize,
					onSelectFunc = [&](int32_t val)
					{
						last_slot = val;
					})
			),
			Rows<2>(
				Checkbox(text = "Alphabetize",
					checked = alphabetize,
					onToggleFunc = [&](bool state)
					{
						alphabetize = state;
						RebuildLists();
						refresh_dlg();
					}),
				Checkbox(text = "Hide Empty",
					checked = skipempty,
					onToggleFunc = [&](bool state)
					{
						skipempty = state;
						RebuildLists();
						refresh_dlg();
					})
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK)
			)
		)
	);
	UpdateSlotList(ScriptType(last_script_type));
	return window;
}

bool ViewScriptSlotsDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::OK:
			return true;
	}
	return false;
}
