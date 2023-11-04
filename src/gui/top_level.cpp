#include "gui/top_level.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "base/zc_alleg.h"
#include <utility>

void displayinfo(std::string, std::string);

namespace GUI
{

int32_t TopLevelWidget::proc(int32_t msg, DIALOG* d, int32_t c)
{
	if(msg == MSG_XCHAR)
	{
		uint16_t actual = (key_shifts&0x07)|(c&0xFF00);
		if(actual == d->d1)
			// Abusing the mechanism here slightly...
			GUI_EVENT(d, (guiEvent)(-(d->d1+1)));
	}
	return D_O_K;
}

int32_t TopLevelWidget::helpproc(int32_t msg, DIALOG* d, int32_t c)
{
	if(!d->dp) return D_O_K;
	std::string const& helptext = *((std::string*)d->dp);
	if(helptext.size() < 1)
		return D_O_K;
	switch(msg)
	{
		case MSG_XCHAR:
			if((c>>8) != KEY_F1)
				break;
			displayinfo("Info", helptext);
			return D_USED_CHAR;
	}
	return D_O_K;
}

void TopLevelWidget::addShortcuts(
	std::initializer_list<KeyboardShortcut>&& scList)
{
	for(auto& sc: scList)
	{
		shortcuts[sc.key] = sc;
	}
}

void TopLevelWidget::realizeKeys(DialogRunner& runner)
{
	// d2 is the index into shortcuts, which will be used as the event
	// when onEvent is called. But that could conflict with a guiEvent
	// handled by a subclass, so we'll make it negative.
	for(auto it = shortcuts.begin(); it != shortcuts.end(); ++it)
	{
		runner.push(shared_from_this(), DIALOG {
			proc,
			0, 0, 0, 0, // x, y, w, h
			0, 0, // fg, bg
			0, // key - MSG_CHAR ignores shift, so we're using MSG_XCHAR
			D_NEW_GUI, // flags
			it->second.key, 0, // d1, d2
			this, nullptr, nullptr // dp, dp2, dp3
		});
	}
	//Helptext handler, if no F1 shortcut is found
	if(shortcuts.find(Key::F1.get()) == shortcuts.end())
	{
		runner.push(shared_from_this(), DIALOG {
			helpproc,
			0, 0, 0, 0, // x, y, w, h
			0, 0, // fg, bg
			0, // key
			D_NEW_GUI, // flags
			0, 0, // d1, d2
			&helptext, nullptr, nullptr // dp, dp2, dp3
		});
	}
}

int32_t TopLevelWidget::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	if(event<0)
	{
		sendMessage(shortcuts[uint16_t(-event-1)].message, MessageArg::none);
		return D_USED_CHAR;
	}

	return -1;
}

}
