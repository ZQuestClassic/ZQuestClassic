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

int* Tab::getContentIndices()
{
	return dialogs.data();
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
	// The TabPanel needs an int array with the indices of the widgets
	// in this tab. That'll just be all of them added by content->realize().
	if(content)
	{
		int sizeBefore = runner.size();
		content->realize(runner);
		int sizeAfter = runner.size();
		dialogs.reserve(1+sizeAfter-sizeBefore);
		for(int i = sizeBefore; i < sizeAfter; ++i)
			dialogs.push_back(i);
	}
	dialogs.push_back(-1);
}

}
