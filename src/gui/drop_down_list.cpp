#include "gui/drop_down_list.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include <cassert>
#include <cmath>

namespace GUI
{

DropDownList::DropDownList():
	listData(nullptr), selectedIndex(0), selectedValue(0), message(-1)
{
	setPreferredWidth(20_em);
	overrideHeight(21_px);
	fgColor = jwin_pal[jcTEXTFG];
	bgColor = jwin_pal[jcTEXTBG];
	widgFont = GUI_DEF_FONT;
}

void DropDownList::setListData(const ::GUI::ListData& newListData)
{
	int selval = getSelectedValue();
	listData = &newListData;
	jwinListData = newListData.getJWin(&widgFont);
	if(alDialog)
	{
		alDialog->dp = &jwinListData;
		setSelectedValue(selval);
	}
}

void DropDownList::setSelectedValue(int32_t value)
{
	selectedValue = value;
	selectedIndex = -1;
	if(alDialog)
	{
		setIndex();
		alDialog->d1 = alDialog->d2=selectedIndex;
	}
}

void DropDownList::setSelectedIndex(int32_t index)
{
	selectedIndex = index;
	if(alDialog)
	{
		alDialog->d1 = selectedIndex;
		alDialog->d2 = selectedIndex;
	}
}

int32_t DropDownList::getSelectedValue() const
{
	if(alDialog)
	{
		int32_t index = alDialog->d1;
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
	int32_t minDiff = std::abs(selectedValue-listData->getValue(0));
	for(size_t i = 1; i < listData->size(); ++i)
	{
		int32_t value = listData->getValue(i);
		if(value == selectedValue)
		{
			selectedIndex = i;
			return;
		}
		else
		{
			int32_t diff = std::abs(selectedValue-value);
			if(diff < minDiff)
			{
				selectedIndex = i;
				minDiff=diff;
			}
		}
	}
}

void DropDownList::calculateSize()
{
	int32_t maxWid = text_length(widgFont, "(None)");
	for(size_t q = 0; q < listData->size(); ++q)
	{
		int32_t w = text_length(widgFont, listData->getText(q).c_str());
		if(w > maxWid)
			maxWid = w;
	}
	setPreferredWidth(3_em+maxWid);
	overrideHeight(21_px);
	Widget::calculateSize();
}

void DropDownList::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void DropDownList::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void DropDownList::applyFont(FONT* newFont)
{
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
		newGUIProc<jwin_droplist_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		selectedIndex, selectedIndex, // d1, d2,
		&jwinListData, nullptr, nullptr // dp, dp2, dp3
	});
}

int32_t DropDownList::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	if(onSelectFunc)
		onSelectFunc(listData->getValue(alDialog->d1));
	if(message >= 0)
		sendMessage(message, listData->getValue(alDialog->d1));
	return -1;
}

}
