#include "misc_color_sel.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include "gui/list_data.h"
#include "gui/jwin.h"
#include <utility>
#include "subscr.h"

namespace GUI
{
static bool isCSet(int32_t val)
{
	return val <= 13;
}
static const ListData colTyList
{
	{ "Misc. Color", ssctMISC },
	{ "Sys. Color", ssctSYSTEM },
	{ "CSet 0", 0 },
	{ "CSet 1", 1 },
	{ "CSet 2", 2 },
	{ "CSet 3", 3 },
	{ "CSet 4", 4 },
	{ "CSet 5", 5 },
	{ "CSet 6", 6 },
	{ "CSet 7", 7 },
	{ "CSet 8", 8 },
	{ "CSet 9", 9 },
	{ "CSet 10", 10 },
	{ "CSet 11", 11 },
	{ "CSet 12", 12 },
	{ "CSet 13", 13 }
};
static const ListData miscColList
{
	{ "Text", 0 },
	{ "Caption", 1 },
	{ "Overworld BG", 2 },
	{ "Dungeon BG", 3 },
	{ "Dungeon FG", 4 },
	{ "Cave FG", 5 },
	{ "BS Dark", 6 },
	{ "BS Goal", 7 },
	{ "Compass (Lt)", 8 },
	{ "Compass (Dk)", 9 },
	{ "SS BG", 10 },
	{ "SS Shadow", 11 },
	{ "Tri. Frame", 12 },
	{ "Big Map BG", 13 },
	{ "Big Map FG", 14 },
	{ "Hero's Pos", 15 },
	{ "Message Text", 16 }
};

MiscColorSel::MiscColorSel(): onUpdate(NULL), c1(0), c2(0)
{
	setFitParent(true);
	for(auto q = 0; q <= 13; ++q)
	{
		c2s[q] = 0;
	}
	
	internal_grid = Grid::columns(2);
	internal_grid->setPadding(0_px);
	
	labels[0] = std::make_shared<Label>();
	labels[0]->setText("Type:");
	labels[0]->setHAlign(1.0);
	labels[1] = std::make_shared<Label>();
	labels[1]->setText("Color:");
	labels[1]->setHAlign(1.0);
	
	internal_grid->add(labels[0]);
	internal_grid->add(labels[1]);
	
	sel_list = std::make_shared<DropDownList>();
	sel_list->setFitParent(true);
	sel_list->setListData(colTyList);
	sel_list->setOnSelectFunc([&](int32_t val)
		{
			doUpdate1(val);
		});
	internal_grid->add(sel_list);
	
	sw = std::make_shared<Switcher>();
	misc_sel_list = std::make_shared<DropDownList>();
	misc_sel_list->setFitParent(true);
	misc_sel_list->setListData(miscColList);
	misc_sel_list->setOnSelectFunc([&](int32_t val)
		{
			doUpdate2(val);
		});
	sw->add(misc_sel_list);
	
	mc_sys_row = std::make_shared<MiscColorRow>();
	mc_sys_row->setSys(true);
	mc_sys_row->setOnUpdate([&](int32_t val)
		{
			doUpdate2(val);
		});
	sw->add(mc_sys_row);
	
	mc_cs_row = std::make_shared<MiscColorRow>();
	mc_cs_row->setHAlign(0.0);
	mc_cs_row->setOnUpdate([&](int32_t val)
		{
			doUpdate2(val);
		});
	sw->add(mc_cs_row);
	
	internal_grid->add(sw);
}

void MiscColorSel::setC1(int32_t val)
{
	sel_list->setSelectedValue(val);
	doUpdate1(val, false);
}
void MiscColorSel::setC2(int32_t val)
{
	switch(c1)
	{
		case ssctMISC:
			misc_sel_list->setSelectedValue(val);
			break;
		case ssctSYSTEM:
			mc_sys_row->setVal(val);
			break;
		default:
			mc_cs_row->setVal(val);
			break;
	}
	doUpdate2(val, false);
}
void MiscColorSel::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	
	labels[0]->setExposed(visible);
	labels[1]->setExposed(visible);
	sel_list->setExposed(visible);
	sw->setExposed(visible);
}

void MiscColorSel::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	internal_grid->setDisabled(dis);
}

void MiscColorSel::applyFont(FONT* newFont)
{
	Widget::applyFont(newFont);
	labels[0]->setFont(newFont);
	labels[1]->setFont(newFont);
}

void MiscColorSel::calculateSize()
{
	internal_grid->calculateSize();
	setPreferredWidth(Size::pixels(internal_grid->getTotalWidth()));
	setPreferredHeight(Size::pixels(internal_grid->getTotalHeight()));
	Widget::calculateSize();
}

void MiscColorSel::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	Widget::arrange(contX, contY, contW, contH);
	internal_grid->arrange(contX,contY,contW,contH);
}

void MiscColorSel::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	internal_grid->realize(runner);
}

void MiscColorSel::doUpdate1(int32_t val, bool func)
{
	switch(val)
	{
		case ssctMISC: sw->switchTo(0); break;
		case ssctSYSTEM: sw->switchTo(1); break;
		default: sw->switchTo(2); break;
	}
	if(val != c1)
	{
		c1 = val;
		if(isCSet(val)) //update color
		{
			doUpdate2(c2s[val], false);
		}
	}
	if(func && onUpdate)
		onUpdate(c1,c2);
}
void MiscColorSel::doUpdate2(int32_t val, bool func)
{
	if(isCSet(c1))
	{
		c2s[c1] = val;
		mc_cs_row->setVal(val);
		mc_cs_row->setCS(c1);
	}
	c2 = val;
	if(func && onUpdate)
		onUpdate(c1,c2);
}

void MiscColorSel::setOnUpdate(std::function<void(int32_t,int32_t)> newOnUpdate)
{
	onUpdate = std::move(newOnUpdate);
}

}
