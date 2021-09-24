#include "tab.h"
#include "dialog_runner.h"
#include "../jwin.h"

namespace GUI
{

char* Tab::getTitle()
{
	if(title.empty())
		title="Tab";
	return title.data();
}

void Tab::applyVisibility(bool visible)
{
	// Easiest way to handle it...
	if(content)
		content->setExposed(visible);
}

void Tab::calculateSize()
{
	if(content)
	{
		content->calculateSize();
		setPreferredWidth(Size::pixels(content->getTotalWidth()));
		setPreferredHeight(Size::pixels(content->getTotalHeight()));
	}
	else
	{
		// Just something random.
		setPreferredWidth(10_em);
		setPreferredHeight(5_em);
	}
}

void Tab::arrange(int contX, int contY, int contW, int contH)
{
	if(content)
		content->arrange(contX, contY, contW, contH);
}

void Tab::realize(DialogRunner& runner)
{
	if(content)
		content->realize(runner);
}

}
