#include "misc_cset_sel.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include "gui/list_data.h"
#include "jwin.h"
#include <utility>
#include "subscr.h"

namespace GUI
{
static const ListData csTyList
{
	{ "Misc. CSet", ssctMISC },
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
	{ "CSet 11", 11 }
};
static const ListData miscCSList
{
	{ "Triforce", 0 },
	{ "Tri. Frame", 1 },
	{ "Overworld Map", 2 },
	{ "Dungeon Map", 3 },
	{ "Blue Frame", 4 },
	{ "HC Piece", 5 },
	{ "SS Vine", 6 }
};

MiscCSetSel::MiscCSetSel(): onUpdate(NULL), c1(ssctMISC), c2(0)
{
	setFitParent(true);
	
	internal_grid = Grid::columns(2);
	internal_grid->setPadding(0_px);
	
	labels[0] = std::make_shared<Label>();
	labels[0]->setText("CSet:");
	labels[0]->setHAlign(1.0);
	labels[1] = std::make_shared<Label>();
	labels[1]->setText("Type:");
	labels[1]->setHAlign(1.0);
	
	internal_grid->add(labels[0]);
	internal_grid->add(labels[1]);
	
	sel_list = std::make_shared<DropDownList>();
	sel_list->setFitParent(true);
	sel_list->setListData(csTyList);
	sel_list->setOnSelectFunc([&](int32_t val)
		{
			doUpdate(val,c2);
		});
	
	misc_sel_list = std::make_shared<DropDownList>();
	misc_sel_list->setFitParent(true);
	misc_sel_list->setListData(miscCSList);
	misc_sel_list->setOnSelectFunc([&](int32_t val)
		{
			doUpdate(c1,val);
		});
	internal_grid->add(sel_list);
	internal_grid->add(misc_sel_list);
}

void MiscCSetSel::setC1(int32_t val)
{
	sel_list->setSelectedValue(val);
	doVis(val);
}
void MiscCSetSel::setC2(int32_t val)
{
	misc_sel_list->setSelectedValue(val);
}
void MiscCSetSel::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	
	labels[0]->setExposed(visible);
	labels[1]->setExposed(visible);
	sel_list->setExposed(visible);
	misc_sel_list->setExposed(visible);
}

void MiscCSetSel::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	internal_grid->setDisabled(dis);
}

void MiscCSetSel::applyFont(FONT* newFont)
{
	Widget::applyFont(newFont);
	labels[0]->setFont(newFont);
	labels[1]->setFont(newFont);
}

void MiscCSetSel::calculateSize()
{
	internal_grid->calculateSize();
	setPreferredWidth(Size::pixels(internal_grid->getTotalWidth()));
	setPreferredHeight(Size::pixels(internal_grid->getTotalHeight()));
	Widget::calculateSize();
}

void MiscCSetSel::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	Widget::arrange(contX, contY, contW, contH);
	internal_grid->arrange(contX,contY,contW,contH);
}

void MiscCSetSel::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	internal_grid->realize(runner);
}

void MiscCSetSel::doVis(int32_t _c1)
{
	if(_c1 == ssctMISC && c1 != ssctMISC)
	{
		labels[1]->setExposed(true);
		misc_sel_list->setExposed(true);
	}
	else if(_c1 != ssctMISC && c1 == ssctMISC)
	{
		labels[1]->setExposed(false);
		misc_sel_list->setExposed(false);
	}
	c1 = _c1;
}
void MiscCSetSel::doUpdate(int32_t _c1, int32_t _c2)
{
	doVis(_c1);
	c2 = _c2;
	
	if(onUpdate)
		onUpdate(c1,c2);
}

void MiscCSetSel::setOnUpdate(std::function<void(int32_t,int32_t)> newOnUpdate)
{
	onUpdate = std::move(newOnUpdate);
}

}
