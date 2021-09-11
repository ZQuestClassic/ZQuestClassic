#include "switcher.h"
#include "../zc_alleg.h"
#include <utility>

namespace GUI
{

Switcher::Switcher(): visibleChild(0)
{}

void Switcher::switchTo(size_t index)
{
	if(index==visibleChild)
		return;

	assert(index<children.size());

	children[visibleChild]->setExposed(false);
	children[index]->setExposed(true);
	visibleChild=index;
}

void Switcher::add(std::shared_ptr<Widget> child)
{
	// If the child being added is the visible one, it shouldn't be hidden.
	// That's most likely the first child while visibleChild is the default 0.
	if(children.size()!=visibleChild)
		child->setExposed(false);
	children.emplace_back(std::move(child));
}

void Switcher::applyVisibility(bool visible)
{
	// We'll handle invisibility by applying an extra "hide" to each child.
	// That's easier than keeping precise track of what's visible or not.
	for(auto& child: children)
		child->setExposed(visible);
}

void Switcher::calculateSize()
{
	// The switcher is as large as its largest child.
	int maxW=0, maxH=0;
	for(auto& child: children)
	{
		child->calculateSize();
		int w=child->getWidth(); // Should this be getTotalWidth()?
		if(w>maxW)
			maxW=w;
		int h=child->getHeight(); // getTotalHeight()?
		if(w>maxH)
			maxH=h;
	}
	setPreferredWidth(Size::pixels(maxW));
	setPreferredHeight(Size::pixels(maxH));
}

void Switcher::arrange(int contX, int contY, int contW, int contH)
{
	for(auto& child: children)
		child->arrange(contX, contY, contW, contH);
}

void Switcher::realize(DialogRunner& runner)
{
	for(auto& child: children)
		child->realize(runner);
}

}
