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
combo_auto copy_autocombo;
static bool copied_autocombo;
static size_t editautocombo_tab = 0;
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

void AutoComboDialog::addCombos(int32_t count)
{
	for (int32_t q = 0; q < count; ++q)
	{
		temp_autocombo.addEntry(0, ACT_NORMAL, q, -1);
	}
}
void AutoComboDialog::removeCombos(int32_t count)
{
	for (int32_t q = 0; q < count; ++q)
	{
		temp_autocombo.removeEntry();
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
	templatebtn->setDisabled(!temp_autocombo.hasTemplate());
	erasepane->setDisabled(!temp_autocombo.canErase());
	temp_autocombo.clear();
	temp_autocombo.setType(val);
	temp_autocombo.updateValid();
	switch (val)
	{
		case AUTOCOMBO_BASIC:
			addCombos(16);
			break;
		case AUTOCOMBO_Z1:
			addCombos(6);
			break;
		case AUTOCOMBO_Z4:
			addCombos(30);
			break;
		case AUTOCOMBO_FENCE:
			addCombos(21);
			break;
		case AUTOCOMBO_RELATIONAL:
			addCombos(47);
			break;
		case AUTOCOMBO_DGNCARVE:
			addCombos(94);
			break;
		case AUTOCOMBO_DOR:
			addCombos(76);
			break;
		case AUTOCOMBO_TILING:
		{
			byte w = (temp_arg & 0xF) + 1;
			byte h = ((temp_arg >> 4) & 0xF) + 1;
			addCombos(w * h);
			break;
		}
	}
}

void animate_combos();
static int32_t scroll_pos1 = 0;
std::shared_ptr<GUI::Widget> AutoComboDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	sgrid = Rows<4>();
	window = Window(
		title = "Auto Combo Editor",
		use_vsync = true,
		onTick = [&]() { animate_combos(); return ONTICK_REDRAW; },
		minwidth = 30_em, minheight = 32_em,
		info =
			"Fill in all the boxes with combos as indicated by the engravings to their left.\n"
			"All boxes need to be filled with combos with no duplicates in order\n"
			"for the autocombo to function.\n\n"
			"Click 'Generate' to auto generate based on the combo layout of the engravings.\n"
			"Shift click 'Generate' to fill all remaining empty slots with unique combos.",
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
								temp_autocombo.setFlags(0);
								temp_autocombo.setArg(0);
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
					disabled = !temp_autocombo.canErase(),
					onSelectFunc = [&](int32_t cmb, int32_t c)
					{
						temp_autocombo.setEraseCombo(cmb);
						if (!(gui_mouse_b() & 2))
						{
							CSet = c;
							refreshPreviewCSets();
						}
					}),
				templatebtn = Button(vAlign = 0.5,
					text = "Auto Generate",
					minwidth = 90_px,
					disabled = !temp_autocombo.hasTemplate(),
					onClick = message::RELOAD,
					onPressFunc = [&]() {
						int32_t cmb, cs;
						int32_t tiling_offs = 0;
						bool altmode = key[KEY_LSHIFT] || key[KEY_RSHIFT];
						if (select_combo_3(cmb, cs))
						{
							int32_t filler = 0;
							if (altmode)
								filler = cmb;
							for (auto w : widgs)
							{
								if (w.cpane)
								{
									if (altmode)
									{
										while (temp_autocombo.containsCombo(filler, false) && filler < MAXCOMBOS - 1)
											++filler;
										if (w.cpane->getCombo() == 0)
										{
											w.cpane->setCombo(filler);
											w.cpane->setCSet(CSet);
											w.entry->cid = filler;
										}
									}
									else
									{
										switch (temp_autocombo.getType())
										{
											case AUTOCOMBO_TILING:
											{
												int32_t wid = (temp_autocombo.getArg() & 0xF) + 1;
												int32_t cid = vbound(cmb + w.slot + tiling_offs, 0, MAXCOMBOS - 1);
												w.cpane->setCombo(cid);
												w.cpane->setCSet(CSet);
												w.entry->cid = cid;
												if (w.slot % wid == wid - 1)
													tiling_offs += 4 - wid;
												break;
											}
											case AUTOCOMBO_RELATIONAL:
											case AUTOCOMBO_DGNCARVE:
												if (temp_autocombo.getFlags() & ACF_LEGACY)
												{
													int32_t cid = vbound(cmb + combo_auto::legacy_offsets(temp_autocombo.getType(), w.slot), 0, MAXCOMBOS - 1);
													w.cpane->setCombo(cid);
													w.cpane->setCSet(CSet);
													w.entry->cid = cid;
													break;
												}
											default:
												int32_t cid = vbound(cmb + combo_auto::convert_offsets(temp_autocombo.getType(), w.slot), 0, MAXCOMBOS - 1);
												w.cpane->setCombo(cid);
												w.cpane->setCSet(CSet);
												w.entry->cid = cid;
												break;
										}
									}
								}
							}
						}
					})
			),
			TabPanel(
				onSwitch = [&](size_t, size_t)
				{
					if (temp_autocombo.getType() == AUTOCOMBO_TILING)
					{
						byte w = (temp_arg & 0xF) + 1;
						byte h = ((temp_arg >> 4) & 0xF) + 1;
						refreshTilingGrid(w, h);
						temp_autocombo.setArg(temp_arg);
					}
					refresh_dlg();
				},
				ptr = &editautocombo_tab,
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
								if (!(gui_mouse_b() & 2))
								{
									CSet = c;
									refreshPreviewCSets();
								}
							})
					),
					switch_settings = Switcher(minheight = 256_px,
						// 0 - disabled
						DummyWidget(),
						// 1 - basic
						Row(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens.")
						),
						// 2 - fence
						Rows<2>(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens."),
							AUTO_CB(flags, ACF_FLIP, 1, "Flip", "Flips the default orientation when placing combos between up/left and down/right.")
						),
						// 3 - pancake
						Rows<2>(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens."),
							AUTO_CB(flags, ACF_FLIP, 1, "Flip", "Flips the default orientation when placing combos between up/left and down/right."),
							Row(colSpan = 2, hAlign = 0.0, padding = 0_px,
								INFOBTN_EX("The default height the mountain extends downwards.", width = 20_px, height = 20_px),
								Label(text = "Height:"),
								TextField(
									type = GUI::TextField::type::INT_DECIMAL,
									minwidth = 1_em,
									minheight = 1_em,
									low = 1, high = 9,
									val = temp_autocombo.getArg() + 1,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										temp_autocombo.setArg(val - 1);
									})
							)
						),
						// 4 - relational, dungeon carve
						Rows<2>(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens."),
							AUTO_CB(flags, ACF_LEGACY, 1, "Legacy Ordering", "Makes 'Auto Generate' use combo ordering from older versions.\nFor tilesets that have combos set up for Relational and Dungeon Carving modes")
						),
						// 5 - DoR
						Rows<2>(vAlign = 0.0,
							AUTO_CB(flags, ACF_CROSSSCREENS, 1, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens."),
							Row(colSpan = 2, hAlign = 0.0, padding = 0_px,
								INFOBTN_EX("The default height the mountain extends downwards.", width = 20_px, height = 20_px),
								Label(text = "Height:"),
								TextField(
									type = GUI::TextField::type::INT_DECIMAL,
									minwidth = 1_em,
									minheight = 1_em,
									low = 1, high = 9,
									val = temp_autocombo.getArg() + 1,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										temp_autocombo.setArg(val - 1);
									})
							)
						),
						// 6 - tiling pattern
						Rows<3>(vAlign = 0.0,
							INFOBTN_EX("The width of the tiling pattern.", width = 20_px, height = 20_px),
							Label(text = "Width:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								minwidth = 2.5_em,
								low = 1, high = 8,
								val = (temp_autocombo.getArg() & 0xF) + 1,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
								{
									temp_arg = (temp_arg & 0xF0) | (val - 1);
									templatebtn->setDisabled(true);
								}),
							INFOBTN_EX("The height of the tiling pattern.", width = 20_px, height = 20_px),
							Label(text = "Height:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								minwidth = 2.5_em,
								low = 1, high = 8,
								val = ((temp_autocombo.getArg() >> 4) & 0xF) + 1,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
								{
									temp_arg = (temp_arg & 0x0F) | ((val - 1) << 4);
									templatebtn->setDisabled(true);
								})
						)
					)
				))
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(text = "Copy",
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						copy_autocombo = temp_autocombo;
						copied_autocombo = true;
						pastebtn->setDisabled(false);
					}),
				pastebtn = Button(text = "Paste",
					minwidth = 90_px,
					disabled = !copied_autocombo,
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						if (copied_autocombo)
							temp_autocombo = copy_autocombo;
					})
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
					if (!(gui_mouse_b() & 2))
					{
						CSet = c;
						refreshPreviewCSets();
					}
					entry.cid = cmb;
					widgs.at(ind).cpane->setCSet(CSet);
					temp_autocombo.updateValid();
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

void AutoComboDialog::addSlotReplace(autocombo_entry& entrybefore, autocombo_entry& entryafter, size_t& grid_ind, size_t& ind, size_t& wid, size_t& hei)
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	autocombo_widg& widg = widgs.emplace_back();
	std::shared_ptr<GUI::Grid> row;

	widg.slot = ind;
	widg.entry = &entrybefore;
	widg.entry_replace = &entryafter;
	sgrid->add(
		row = Row(framed = true, padding = 8_px,
			widg.cpane = SelComboSwatch(
				combo = entrybefore.cid,
				cset = CSet,
				showvals = false,
				onSelectFunc = [&, grid_ind](int32_t cmb, int32_t c)
				{
					if (!(gui_mouse_b() & 2))
					{
						CSet = c;
						refreshPreviewCSets();
					}
					entrybefore.cid = cmb;
					widgs.at(grid_ind).cpane->setCSet(CSet);
					temp_autocombo.updateValid();
				}),
			Engraving(data = 63, padding = 0_px),
			widg.cpane_replace = SelComboSwatch(
				combo = entryafter.cid,
				cset = CSet,
				showvals = false,
				onSelectFunc = [&, grid_ind](int32_t cmb, int32_t c)
				{
					if (!(gui_mouse_b() & 2))
					{
						CSet = c;
						refreshPreviewCSets();
					}
					entryafter.cid = cmb;
					widgs.at(grid_ind).cpane_replace->setCSet(CSet);
					temp_autocombo.updateValid();
				})
		)
	);

	if (!hei)
	{
		row->calculateSize();
		hei = row->getTotalHeight();
		wid = row->getTotalWidth();
	}
	ind += 2;
}

void AutoComboDialog::addSlotNoEngrave(autocombo_entry& entry, size_t& ind, size_t& wid, size_t& hei)
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	autocombo_widg& widg = widgs.emplace_back();
	std::shared_ptr<GUI::Grid> row;

	widg.slot = ind;
	widg.entry = &entry;
	sgrid->add(
		widg.cpane = SelComboSwatch(
			combo = entry.cid,
			cset = CSet,
			padding = 0_px,
			showvals = false,
			onSelectFunc = [&, ind](int32_t cmb, int32_t c)
			{
				if(!(gui_mouse_b() & 2))
				{
					CSet = c;
					refreshPreviewCSets();
				}
				entry.cid = cmb;
				widgs.at(ind).cpane->setCSet(CSet);
				temp_autocombo.updateValid();
			})
	);
	tiling_grid[widg.slot] = widg.cpane;

	if (!hei)
	{
		hei = widg.cpane->getTotalHeight();
		wid = widg.cpane->getTotalWidth();
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
				"A generic autocombo setup. Will tile along cardinal directions.\n\n"
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
				"Ctrl + Click: Flip orientation between inward / outward\n"
				"Shift + [: Increase height, changes combos if hovering over them\n"
				"Shift + ]: Decrease height, changes combos if hovering over them";
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
		case AUTOCOMBO_RELATIONAL:
			typeinfostr =
				"A more complex version of the generic setup. Will tile along\n"
				"cardinal directions, accounting for corners.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Ctrl + Left Click: Fill combos\n"
				"Ctrl + Right Click: Fill remove combos";
			switch_settings->switchTo(4);
			break;
		case AUTOCOMBO_DGNCARVE:
			typeinfostr =
				"An autocombo for making two tile high dungeon walls.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Ctrl + Left Click: Fill combos\n"
				"Ctrl + Right Click: Fill remove combos";
			switch_settings->switchTo(4);
			break;
		case AUTOCOMBO_DOR:
			typeinfostr =
				"An autocombo for mountains in a 3/4 perspective.\n"
				"WARNING: The tiles in this set are slightly incomplete. Not all formations\n"
				"are possible and some manual correction will be needed for more complex setups.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Shift + Click: Place / Remove combos without writing sides\n"
				"Ctrl + Left Click: Fill combos\n"
				"Ctrl + Right Click: Fill remove combos";
			switch_settings->switchTo(5);
			break;
		case AUTOCOMBO_TILING:
			typeinfostr =
				"An autocombo for tiling patterns based on X/Y position.\n\n"
				"CONTROLS:\n"
				"Left Click: Place combo\n"
				"Right Click: Remove combo (uses the Erase Combo)\n"
				"Shift + Click: Update the X/Y offset for the top-left corner of the tiling pattern";
			switch_settings->switchTo(6);
			break;
		case AUTOCOMBO_REPLACE:
			typeinfostr =
				"An autocombo for replacing a set of arbitrary tiles with others.\n\n"
				"CONTROLS:\n"
				"Left Click: Replace combo\n"
				"Right Click: Replace combo (reverse)";
			switch_settings->switchTo(0);
			break;
		default:
			switch_settings->switchTo(0);
			break;
	}
}

void AutoComboDialog::refreshTilingGrid(byte w, byte h)
{
	byte oldw = (temp_autocombo.getArg() & 0xF) + 1;
	byte oldh = ((temp_autocombo.getArg() >> 4) & 0xF) + 1;

	byte temp_type[8][8] = { 0 };
	int32_t temp_grid[8][8] = { 0 };
	for (int32_t xi = 0; xi < oldw; ++xi)
	{
		for (int32_t yi = 0; yi < oldh; ++yi)
		{
			int32_t i = xi + yi * oldw;
			temp_type[xi][yi] = temp_autocombo.combos[i].ctype;
			temp_grid[xi][yi] = temp_autocombo.combos[i].cid;
		}
	}

	refreshPanels();

	for (int32_t xi = 0; xi < w; ++xi)
	{
		for (int32_t yi = 0; yi < h; ++yi)
		{
			int32_t i = xi + yi * w;
			temp_autocombo.combos[i].ctype = temp_type[xi][yi];
			temp_autocombo.combos[i].cid = temp_grid[xi][yi];
		}
	}
}

void AutoComboDialog::refreshWidgets()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	temp_arg = temp_autocombo.getArg();

	byte tiling_w = 1;
	byte tiling_h = 1;
	if (temp_autocombo.getType() == AUTOCOMBO_TILING)
	{
		tiling_w = (temp_autocombo.getArg() & 0xF) + 1;
		tiling_h = ((temp_autocombo.getArg() >> 4) & 0xF) + 1;
	}

	size_t per_row = 4;
	size_t vis_rows = 5;
	if (temp_autocombo.getType() == AUTOCOMBO_TILING)
	{
		per_row = tiling_w;
		switch (per_row)
		{
			case 1:
				sgrid = Column();
				break;
			case 2:
				sgrid = Rows<2>();
				break;
			case 3:
				sgrid = Rows<3>();
				break;
			case 4:
				sgrid = Rows<4>();
				break;
			case 5:
				sgrid = Rows<5>();
				break;
			case 6:
				sgrid = Rows<6>();
				break;
			case 7:
				sgrid = Rows<7>();
				break;
			case 8:
				sgrid = Rows<8>();
				break;
		}
	}
	else
		sgrid = Rows<4>();

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
		case AUTOCOMBO_RELATIONAL:
			break;
		case AUTOCOMBO_DGNCARVE:
		{
			static byte dgn_grid[] = {
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,

				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,

				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,

				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 1, 0
			};
			grid = dgn_grid;
			break;
		}
		case AUTOCOMBO_DOR:
			static byte dor_grid[] = {
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,

				0, 0, 0, 0,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,

				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0
			};
			grid = dor_grid;
			break;
	}

	int32_t sz = temp_autocombo.combos.size();
	while (widg_ind < temp_autocombo.combos.size())
	{
		if (grid && grid[grid_ind])
			sgrid->add(DummyWidget());
		else
		{
			switch(temp_autocombo.getType())
			{
				case AUTOCOMBO_TILING:
					addSlotNoEngrave(temp_autocombo.combos[widg_ind], widg_ind, wid, hei);
					break;
				case AUTOCOMBO_REPLACE:
					addSlotReplace(temp_autocombo.combos[widg_ind], temp_autocombo.combos[widg_ind + 1], grid_ind, widg_ind, wid, hei);
					break;
				default:
					addSlot(temp_autocombo.combos[widg_ind], widg_ind, wid, hei);
					break;
			}
		}
		++grid_ind;
	}
	
	std::shared_ptr<GUI::Grid> extrarow;
	if (temp_autocombo.getType() == AUTOCOMBO_REPLACE)
	{
		extrarow = Row(vAlign = 1.0,
			Button(vAlign = 1.0,
				text = "-",
				width = 32_px, height = 32_px,
				disabled = temp_autocombo.combos.size() <= 0,
				onClick = message::RELOAD,
				onPressFunc = [&]() {
					removeCombos(2);
				}),
			Button(vAlign = 1.0,
				text = "+",
				width = 32_px, height = 32_px,
				disabled = temp_autocombo.combos.size() >= 512,
				onClick = message::RELOAD,
				onPressFunc = [&]() {
					addCombos(2);
				})
		);
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
	if (extrarow)
		wingrid->add(extrarow);
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

