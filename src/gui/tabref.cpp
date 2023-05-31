#include "gui/tabref.h"
#include "base/zc_alleg.h"
#include <utility>

namespace GUI
{

TabRef::TabRef()
{
	setFitParent(true);
}

void TabRef::setContent(std::shared_ptr<Widget> newContent) noexcept
{
	content = std::move(newContent);
}

void TabRef::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(content)
		content->setExposed(visible);
}

void TabRef::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(content)
		content->setDisabled(dis);
}

void TabRef::calculateSize()
{
	content->calculateSize();
	setPreferredWidth(Size::pixels(content->getTotalWidth()));
	setPreferredHeight(Size::pixels(content->getTotalHeight()));
	Widget::calculateSize();
}

void TabRef::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	content->arrange(contX, contY, contW, contH);
}

void TabRef::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	content->realize(runner);
}

}
