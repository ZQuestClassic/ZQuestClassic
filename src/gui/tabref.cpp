#include "tabref.h"
#include "../zc_alleg.h"
#include <utility>

namespace GUI
{

TabRef::TabRef()
{}

void TabRef::setContent(std::shared_ptr<Widget> newContent) noexcept
{
	content = std::move(newContent);
}

void TabRef::applyVisibility(bool visible)
{
	content->applyVisibility(visible);
}

void TabRef::calculateSize()
{
	content->calculateSize();
	setPreferredWidth(Size::pixels(content->getWidth()));
	setPreferredHeight(Size::pixels(content->getHeight()));
}

void TabRef::arrange(int contX, int contY, int contW, int contH)
{
	content->arrange(contX, contY, contW, contH);
}

void TabRef::realize(DialogRunner& runner)
{
	content->realize(runner);
}

}
