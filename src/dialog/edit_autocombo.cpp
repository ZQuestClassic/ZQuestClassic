#include "edit_autocombo.h"
#include "base/autocombo.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "zq/zq_tiles.h"

extern bool saved;
combo_auto temp_autocombo;
static combo_auto* retptr;
static bool new_usecs = false;

extern int32_t CSet;
extern combo_auto combo_autos[];

#define AUTO_CB(member, flag, txt, inf) \
INFOBTN(inf), \
Checkbox(checked = temp_autocombo.member&flag, \
	text = txt,  \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(temp_autocombo.member, flag, state); \
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
			addCombos(val, 21);
			break;
		case AUTOCOMBO_FENCE:
			addCombos(val, 12);
			break;
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
		onTick = [&]() {animate_combos(); return ONTICK_REDRAW; },
		minwidth = 30_em, minheight = 32_em,
		info = "tba",
		onClose = message::CANCEL,
		Column(vAlign = 0.0,
			Rows<5>(vAlign = 0.0,
				Label(text = "Type:", vAlign = 0.0),
				typedropdown = DropDownList(vAlign = 0.0,
					data = list_autocombotypes,
					fitParent = true,
					selectedValue = temp_autocombo.getType(),
					onSelectionChanged = message::RELOAD,
					onSelectFunc = [&](int32_t val)
					{
						if (val != temp_autocombo.getType())
						{
							refreshPanels();
						}
					}),
				Label(text = "Display Combo:", vAlign = 0.0),
				iconpane = SelComboSwatch(vAlign = 0.0,
					combo = temp_autocombo.getIconDisplay(),
					cset = CSet,
					showvals = true,
					disabled = temp_autocombo.getType() == AUTOCOMBO_NONE,
					onSelectFunc = [&](int32_t cmb, int32_t c)
					{
						temp_autocombo.setDisplay(cmb);
						CSet = c;
						refreshPreviewCSets();
					}),
				templatebtn = Button(vAlign = 1.0,
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
					}),
				AUTO_CB(flags, ACF_CROSSSCREENS, "Cross Screens", "If checked, this autocombo can affect combos on adjacent screens."),
				Label(text = "Erase Combo:"),
				erasepane = iconpane = SelComboSwatch(vAlign = 0.0,
					combo = temp_autocombo.getEraseCombo(),
					cset = CSet,
					showvals = true,
					disabled = temp_autocombo.getType() == AUTOCOMBO_NONE,
					onSelectFunc = [&](int32_t cmb, int32_t c)
					{
						temp_autocombo.setEraseCombo(cmb),
						CSet = c;
						refreshPreviewCSets();
					})
			),
			wingrid = Column(padding=0_px),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(vAlign = 1.0,
					focused = true,
					text = "Clear",
					minwidth = 90_px,
					onClick = message::RELOAD,
					onPressFunc = [&]() {
						refreshPanels();
					}),
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
				0, 1, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 0, 1,
				0, 0, 1, 1,
				0, 0
			};
			grid = z4_grid;
			break;
		}
		case AUTOCOMBO_FENCE:
		{
			static byte fence_grid[] = {
				0, 0, 0, 1,
				0, 1, 0, 1,
				0, 0, 0, 1,
				0, 0, 1, 1,
				0, 0
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

