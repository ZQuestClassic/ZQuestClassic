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
static int32_t cursor_cset;
static bool new_usecs = false;

extern int32_t CSet;
extern combo_auto combo_autos[];

void call_autocombo_dlg(int32_t index)
{
	cursor_cset = CSet;
	retptr = &combo_autos[index];
	temp_autocombo = *retptr;
	AutoComboDialog().show();
}

AutoComboDialog::AutoComboDialog() :
	list_autocombotypes(GUI::ZCListData::autocombotypes())
{ 

}

void AutoComboDialog::addCombos(int32_t engrave_offset, int32_t count, uint32_t dummyflags)
{
	int32_t offset = 0;
	for (int32_t q = 0; q < count; ++q)
	{
		if (q > 31 || (dummyflags & (1 << q)))
		{
			temp_autocombo.combos.emplace_back(autocombo_entry(0, CSet, q, -1, -1));
		}
		else
		{
			temp_autocombo.combos.emplace_back(autocombo_entry(0, CSet, q, offset, engrave_offset));
			++offset;
			++engrave_offset;
		}
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
			addCombos(16, 16, 0);
			break;
		case AUTOCOMBO_Z1:
			addCombos(0, 7, 0x8); // 0b1000
			break;
		case AUTOCOMBO_Z4:
			addCombos(48, 30, 0xC8888A8); // 0b1100100010001000100010101000);
			break;
		case AUTOCOMBO_FENCE:
			addCombos(32, 18, 0xC8A8); // 0b1100100010101000
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
	
	std::shared_ptr<GUI::Grid> wingrid, sgrid;
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
					onSelectFunc = [&](int32_t cmb, int32_t)
					{
						temp_autocombo.setDisplay(cmb);
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
									int32_t cid = cmb + temp_autocombo.convert_offsets(w.slot);
									w.cpane->setCombo(cid);
									w.cpane->setCSet(CSet);
									w.entry->cid = cid;
								}
							}
						}
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
	refreshWidgets(wingrid, sgrid);
	return window;
}

void AutoComboDialog::refreshWidgets(std::shared_ptr<GUI::Grid> wingrid, std::shared_ptr<GUI::Grid> sgrid)
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	size_t per_row = 4;
	size_t vis_rows = 5;

	size_t ind = 0;
	size_t hei = 0, wid = 0;
	widgs.clear();
	for (auto& entry : temp_autocombo.combos)
	{
		widgs.emplace_back();
		autocombo_widg& widg = widgs.back();
		std::shared_ptr<GUI::Grid> row;
		if (entry.offset > -1)
		{
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
							entry.cid = cmb;
							widgs.at(ind).cpane->setCSet(CSet);
							temp_autocombo.updateValid();
						})
				)
			);
		}
		else
		{
			sgrid->add(
				row = Row(
					DummyWidget()
				)
			);
		}
		if (!hei)
		{
			row->calculateSize();
			hei = row->getTotalHeight();
			wid = row->getTotalWidth();
		}
		++ind;
	}
	if (temp_autocombo.combos.size() == 0)
	{
		sgrid->add(DummyWidget());
	}
	if (ind / per_row >= vis_rows)
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

