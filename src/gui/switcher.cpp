#include "switcher.h"
#include "base/zc_alleg.h"
#include <utility>

namespace GUI
{

Switcher::Switcher(): visibleChild(0)
{}

void Switcher::switchTo(size_t index)
{
	if(index == visibleChild)
		return;

	assert(index<children.size());
	if(isConstructed())
	{
		children[visibleChild]->setExposed(false);
		children[index]->setExposed(true);
	}
	pendDraw();
	visibleChild = index;
}

void Switcher::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	// We'll handle invisibility by applying an extra "hide" to each child.
	// That's easier than keeping precise track of what's visible or not.
	for(auto& child: children)
		child->setExposed(visible);
}

void Switcher::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	for(auto& child: children)
		child->setDisabled(dis);
}

void Switcher::calculateSize()
{
	// The switcher is as large as its largest child.
	int32_t maxW = 0, maxH = 0;
	for(auto& child: children)
	{
		child->calculateSize();
		int32_t w = child->getTotalWidth();
		if(w > maxW)
			maxW = w;
		int32_t h = child->getTotalHeight();
		if(w > maxH)
			maxH = h;
	}
	setPreferredWidth(Size::pixels(maxW));
	setPreferredHeight(Size::pixels(maxH));
	Widget::calculateSize();
}

void Switcher::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	for(auto& child: children)
		child->arrange(contX, contY, contW, contH);
}

void Switcher::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	for(size_t i = 0; i < children.size(); ++i)
	{
		children[i]->realize(runner);
		children[i]->setExposed(false);
	}
	children[visibleChild]->setExposed(true);
}

}
