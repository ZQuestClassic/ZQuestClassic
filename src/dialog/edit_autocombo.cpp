#include "edit_autocombo.h"
#include "base/autocombo.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "zq/zq_tiles.h"
#include "dialog/alert.h"

extern bool saved;
combo_auto temp_autocombo;
static combo_auto* retptr;

extern int32_t CSet;
extern combo_auto combo_autos[];

#define AUTO_CB(member, flag, cspan, txt, inf) \
INFOBTN_EX(inf, width = 20_px, height = 20_px, hAlign = 1.0), \
Checkbox(hAlign = 0.0, \
	checked = temp_autocombo.member&flag, \
	text = txt,  \
	colSpan = cspan, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(temp_autocombo.member, flag, state); \
	})

#define AUTO_INFOBTN_REF(getter) \
Button(text = "?", \
	width = 20_px, height = 20_px, \
	onClick = message::REFR_INFO, \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",getter).show(); \
	})

void call_autocombo_dlg(int32_t index)
{
	retptr = &combo_autos[index];
	temp_autocombo = *retptr;
	AutoComboDialog().show();
}

AutoComboDialog::AutoComboDialog() :
	list_autocombotypes(GUI::ZCListData::autocombotypes())
{ 

}

void AutoComboDialog::addCombos(int32_t type, int32_t count)
{
	for (int32_t q = 0; q < count; ++q)
	{
		temp_autocombo.add(0, q, -1);
	}
}
int32_t AutoComboDialog::numCombosSet()
{
	int32_t count = 0;
	for (auto c : temp_autocombo.combos)
	{
		if (c.cid > 0)
			++count;
	}
	return count;
}
void AutoComboDialog::refreshPanels()
{
	int32_t val = typedropdown->getSelectedValue();
	iconpane->setDisabled(val == AUTOCOMBO_NONE);
	templatebtn->setDisabled(val == AUTOCOMBO_NONE);
	temp_autocombo.clear();
	temp_autocombo.setType(val);
	temp_autocombo.updateValid();
	switch (val)
	{
		case AUTOCOMBO_BASIC:
			addCombos(val, 16);
			break;
		case AUTOCOMBO_Z1:
			addCombos(val, 6);
			break;
		case AUTOCOMBO_Z4:
			addCombos(val, 30);
			break;
		case AUTOCOMBO_FENCE:
			addCombos(val, 21);
			break;
	}
}

void animate_combos();
static int32_t scroll_pos1 = 0;
static int32_t prev_scroll = 0;
std::shared_ptr<GUI::Widget> AutoComboDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	sgrid = Rows<4>();
	window = Window(
		title = "Auto Combo Editor",
		use_vsync = true,
		onTick = [&]() {prev_scroll = scroll_pos1; animate_combos(); return ONTICK_REDRAW; },
		minwidth = 30_em, minheight = 32_em,
		info =
			"Fill in all the boxes with combos as indicated by the engravings to their left.\n"
			"All boxes need to be filled with combos with no duplicates in order\n"
			"for the autocombo to function.",
		onClose = message::CANCEL,
		Column(vAlign = 0.0,
			Row(vAlign = 0.0,
				typeinfobtn = AUTO_INFOBTN_REF(typeinfostr),
				Label(text = "Type:", vAlign = 0.5),
				typedropdown = DropDownList(vAlign = 0.5,
					data = list_autocombotypes,
					fitParent = true,
					selectedValue = temp_autocombo.getType(),
					onSelectionChanged = message::RELOAD,
					onSelectFunc = [&](int32_t val)
					{
						if (val != temp_autocombo.getType())
						{
							bool doChange = true;
							if (numCombosSet())
							{
								AlertDialog("Clear Autocombo?",
									"Changing this autocombo's type will clear all its combos. Are you sure you want to do this?",
									[&doChange](bool ret, bool)
									{
										if (!ret)
											doChange = false;
									}).show();
							}
							if (doChange)
							{
								refreshPanels();
								refreshTypes(val);
							}
							else
								typedropdown->setSelectedValue(temp_autocombo.getType());
						}
					}),
				Label(text = "Erase Combo:", leftPadding = 32_px),
				erasepane = iconpane = SelComboSwatch(vAlign = 0.5,
					combo = temp_autocombo.getEraseCombo(),
					cset = CSet,
					showvals = false,
					disabled = temp_autocombo.getType() == AUTOCOMBO_NONE,
					onSelectFunc = [&](int32_t cmb, int32_t c)
					{
						temp_autocombo.setEraseCombo(cmb),
							CSet = c;
						refreshPreviewCSets();
					}),
				templatebtn = Button(vAlign = 0.5,
					text = "Auto Generate",
					minwidth = 90_px,
					disabled = temp_autocombo.getType() == AUTOCOMBO_NONE,
					onClick = message::RELOAD,
					onPressFunc = [&]() {
						int32_t cmb, cs;
						if (select_combo_3(cmb, cs))
						{
							for (auto w : widgs)
							{
								if (w.cpane)
								{
									int32_t cid = vbound(cmb + combo_auto::convert_offsets(temp_autocombo.getType(), w.slot), 0, MAXCOMBOS-1);
									w.cpane->setCombo(cid);
									w.cpane->setCSet(CSet);
									w.entry->cid = cid;
								}
							}
						}
					})
			),
			TabPanel(
				TabRef(name = "Combos", Column(
					wingrid = Column(padding = 0_px)
				)),
				TabRef(name = "Settings", Column(
					Row(vAlign = 0.0,
						Label(text = "Display Combo:"),
						iconpane = SelComboSwatch(
							colSpan = 2,
							combo = temp_autocombo.getIconDisplay(),
							cset = CSet,
							showvals = false,
							disabled = temp_autocombo.getType() == AUTOCOMBO_NONE,
							onSelectFunc = [&](int32_t cmb, int32_t c)
							{
								temp_autocombo.setDisplay(cmb);
								CSet = c;
								refreshPreviewCSets();
							})
					),
					switch_settings = Switcher(
						DummyWidget(),
						Row(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens.")
						),
						Rows<2>(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens."),
							AUTO_CB(flags, ACF_FLIP, 1, "Flip", "Flips the default orientation when placing combos between up/left and down/right.")
						),
						Rows<2>(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens."),
							AUTO_CB(flags, ACF_FLIP, 1, "Flip", "Flips the default orientation when placing combos between up/left and down/right."),
							Row(colSpan=2, hAlign = 0.0, padding = 0_px,
								INFOBTN_EX("The default height the mountain extends downwards.", width = 20_px, height = 20_px),
								Label(text = "Height:"),
								TextField(
									type = GUI::TextField::type::INT_DECIMAL,
									low = 1, high = 9,
									val = temp_autocombo.getArg(),
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										temp_autocombo.setArg(val);
									})
							)
						)
					)
				))
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(vAlign = 1.0,
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(vAlign = 1.0,
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	refreshTypes(temp_autocombo.getType());
	refreshWidgets();
	return window;
}

void AutoComboDialog::addSlot(autocombo_entry& entry, size_t& ind, size_t& wid, size_t& hei)
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	autocombo_widg& widg = widgs.emplace_back();
	std::shared_ptr<GUI::Grid> row;

	widg.slot = ind;
	widg.entry = &entry;
	sgrid->add(
		row = Row(framed = true,
			Engraving(data = entry.engrave_offset),
			widg.cpane = SelComboSwatch(
				combo = entry.cid,
				cset = CSet,
				showvals = false,
				onSelectFunc = [&, ind](int32_t cmb, int32_t c)
				{
					CSet = c;
					refreshPreviewCSets();
					entry.cid = cmb;
					widgs.at(ind).cpane->setCSet(CSet);
					temp_autocombo.updateValid();
					scroll_pos1 = prev_scroll;
				})
		)
	);

	if (!hei)
	{
		row->calculateSize();
		hei = row->getTotalHeight();
		wid = row->getTotalWidth();
	}
	++ind;
}

void AutoComboDialog::refreshPreviewCSets()
{
	iconpane->setCSet(CSet);
	for (int32_t q = 0; q < widgs.size(); ++q)
	{
		widgs[q].cpane->setCSet(CSet);
	}
}

void AutoComboDialog::refreshTypes(int32_t type)
{
	typeinfobtn->setDisabled(type == AUTOCOMBO_NONE);
	switch (type)
	{
		case AUTOCOMBO_BASIC:
			typeinfostr =
				"A basic general purpose setup. Will tile along cardinal directions.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Ctrl + Left Click: Fill combos\n"
				"Ctrl + Right Click: Fill remove combos";
			switch_settings->switchTo(1);
			break;
		case AUTOCOMBO_Z1:
			typeinfostr =
				"A very simple autocombo setup for making classic mountains.\n"
				"Uses only 6 combos made up of tops, bottoms, and corners.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Ctrl + Left Click: Fill combos\n"
				"Ctrl + Right Click: Fill remove combos";
			switch_settings->switchTo(1);
			break;
		case AUTOCOMBO_Z4:
			typeinfostr =
				"An autocombo setup for making mountains that stack vertically.\n"
				"Works identically to 'Fence', but with combos for the bottoms\n"
				"of the mountain filled in as it goes.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Ctrl + Click: Flip orientation between inward / outward";
			switch_settings->switchTo(3);
			break;
		case AUTOCOMBO_FENCE:
			typeinfostr =
				"An autocombo setup for making one tile high walls.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Ctrl + Click: Flip orientation between inward / outward";
			switch_settings->switchTo(2);
			break;
		default:
			switch_settings->switchTo(0);
			break;
	}
}

void AutoComboDialog::refreshWidgets()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	size_t per_row = 4;
	size_t vis_rows = 5;

	size_t widg_ind = 0, grid_ind = 0;
	size_t hei = 0, wid = 0;
	widgs.clear();
	byte* grid = nullptr;
	switch (temp_autocombo.getType())
	{
		case AUTOCOMBO_BASIC:
			break;
		case AUTOCOMBO_Z1:
		{
			static byte z1_grid[] = {
				0, 0, 0, 1,
				0, 0, 0
			};
			grid = z1_grid;
			break;
		}
		case AUTOCOMBO_Z4:
		{
			static byte z4_grid[] = {
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 1, 1,
				0, 0, 1, 1,
				0, 0, 0, 0,
				0, 0, 0, 0
			};
			grid = z4_grid;
			break;
		}
		case AUTOCOMBO_FENCE:
		{
			static byte fence_grid[] = {
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 1, 1,
				0, 0, 1, 1,
				0, 0, 0, 0,
				0, 0, 0, 0
			};
			grid = fence_grid;
			break;
		}
	}
	while (widg_ind < temp_autocombo.combos.size())
	{
		if (grid && grid[grid_ind])
			sgrid->add(DummyWidget());
		else
			addSlot(temp_autocombo.combos[widg_ind], widg_ind, wid, hei);
		++grid_ind;
	}
	if (temp_autocombo.combos.size() == 0)
	{
		sgrid->add(DummyWidget());
	}
	if (grid_ind / per_row >= vis_rows)
	{
		wingrid->add(ScrollingPane(
			ptr = &scroll_pos1,
			minheight = Size::pixels(hei * vis_rows + DEFAULT_PADDING_INT * 2),
			sgrid));
	}
	else
	{
		scroll_pos1 = 0;
		wingrid->add(sgrid);
	}
}

bool AutoComboDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			rerun_dlg = true;
			return true;
		case message::OK:
			if(retptr)
			{
				temp_autocombo.updateValid();
				*retptr = temp_autocombo;
				saved = false;
			}
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}

