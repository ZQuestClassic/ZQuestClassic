#include "gui/radioset.h"
#include "gui/radio.h"
#include "gui/grid.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include "jwin.h"
#include "base/zsys.h"
#include <utility>


namespace GUI
{

RadioSet::RadioSet(): Widget(), message(-1), checked(0), procset(0)
{}

void RadioSet::setChecked(size_t value)
{
	for(auto& child : children)
	{
		child->setChecked(child->getIndex() == value);
	}
	checked = value;
}

size_t RadioSet::getChecked()
{
	for(auto& child : children)
	{
		if(child->getChecked())
		{
			return child->getIndex();
		}
	}
	return checked;
}

void RadioSet::setProcSet(int32_t newProcSet)
{
	procset = newProcSet;
	for(auto& child : children)
	{
		child->setProcSet(procset);
	}
}

void RadioSet::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(content) content->applyVisibility(visible);
}

void RadioSet::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if (content) content->setDisabled(dis);
}

void RadioSet::loadList(GUI::ListData radiolist)
{
	content = Grid::rows(1);
	for(size_t q = 0; q < radiolist.size(); ++q)
	{
		std::shared_ptr<Radio> radio = std::make_shared<Radio>();
		size_t ind = radiolist.getValue(q);
		std::string const& name = radiolist.getText(q);
		radio->setIndex(ind);
		radio->setText(name);
		if(ind==checked)
			radio->setChecked(true);
		radio->onToggle(message);
		radio->setHAlign(0.0);
		radio->setHPadding(0_px);
		radio->setVPadding(1_px);
		content->add(radio);
		children.emplace_back(std::move(radio));
	}
	content->setMargins(0_px);
	content->setPadding(0_px);
	if(!getVisible()) content->setVisible(false);
}

void RadioSet::calculateSize()
{
	content->calculateSize();
	setPreferredWidth(Size::pixels(content->getWidth()));
	setPreferredHeight(Size::pixels(content->getHeight()));
	Widget::calculateSize();
}

void RadioSet::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	Widget::arrange(contX, contY, contW, contH);
	content->arrange(contX, contY, contW, contH);
}

void RadioSet::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	content->realize(runner);
}


}
