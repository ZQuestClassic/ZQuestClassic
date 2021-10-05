#include "drop_down_list.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <cmath>

namespace GUI
{

DropDownList::DropDownList():
	listData(nullptr), selectedIndex(0), selectedValue(0), message(-1)
{
	setPreferredWidth(20_em);
	if(is_large) // This has to be exactly right to look good
		setPreferredHeight(21_px);
	else
		setPreferredHeight(16_px);
	fgColor = jwin_pal[jcTEXTFG];
	bgColor = jwin_pal[jcTEXTBG];
}

void DropDownList::setListData(const ::GUI::ListData& newListData)
{
	listData = &newListData;
	jwinListData = newListData.getJWin(&widgFont);
}

void DropDownList::setSelectedValue(int value)
{
	selectedValue = value;
	selectedIndex = -1;
	if(alDialog)
	{
		setIndex();
		alDialog->d1 = alDialog->d2=selectedIndex;
	}
}

void DropDownList::setSelectedIndex(int index)
{
	selectedIndex = index;
	if(alDialog)
	{
		alDialog->d1 = selectedIndex;
		alDialog->d2 = selectedIndex;
	}
}

int DropDownList::getSelectedValue() const
{
	if(alDialog)
	{
		int index = alDialog->d1;
		return listData->getValue(index);
	}
	else
		return selectedValue;
}

void DropDownList::setIndex()
{
	// Find a valid selection. We'll take the first thing with a matching
	// value. If nothing matches exactly, take the one that's closest to
	// the selected value.
	selectedIndex = 0;
	int minDiff = std::abs(selectedValue-listData->getValue(0));
	for(size_t i = 1; i < listData->size(); ++i)
	{
		int value = listData->getValue(i);
		if(value == selectedValue)
		{
			selectedIndex = i;
			return;
		}
		else
		{
			int diff = std::abs(selectedValue-value);
			if(diff < minDiff)
			{
				selectedIndex = i;
				minDiff=diff;
			}
		}
	}
}

void DropDownList::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void DropDownList::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

void DropDownList::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	// An empty list might logically be valid, but there's currently
	// no way to get a value from it.
	assert(listData);
	assert(listData->size() > 0);
	if(selectedIndex < 0)
		setIndex();

	alDialog = runner.push(shared_from_this(), DIALOG {
		jwin_droplist_proc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		selectedIndex, selectedIndex, // d1, d2,
		&jwinListData, nullptr, nullptr // dp, dp2, dp3
	});
}

int DropDownList::onEvent(int event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	if(onSelectFunc)
		onSelectFunc(listData->getValue(alDialog->d1));
	if(message >= 0)
		sendMessage(message, listData->getValue(alDialog->d1));
	return -1;
}

}
