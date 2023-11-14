#include "zqhotkeys.h"
#include "info.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "base/zsys.h"
#include "gui/use_size.h"
#include <fmt/format.h>
#include "zq/zq_hotkey.h"
#include "base/util.h"
using namespace util;

ZQHotkeyDialog::ZQHotkeyDialog(bool& confirm):
	confirm_ptr(&confirm), favc(nullptr)
{}
ZQHotkeyDialog::ZQHotkeyDialog(optional<uint>& favc):
	confirm_ptr(nullptr), favc(&favc)
{}

void ZQHotkeyDialog::load_labels()
{
	//start at 1, skipping ZQKEY_NULL_KEY
	for(uint q = 1; q < ZQKEY_MAX; ++q)
	{
		if(!lbls[q][0]) continue;
		for(uint alt = 0; alt <= 1; ++alt)
		{
			lbls[q][alt]->setText(zq_hotkeys[q].get_name(alt));
		}
	}
}
static int32_t scroll_pos1 = 0;
static Size dlg_minwid = 300_px;
std::shared_ptr<GUI::Widget> ZQHotkeyDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> hotkeylist;
	if(favc)
	{
		hotkeylist = Rows<6>(vAlign = 0.0);
		hotkeylist->add(Label(text = "Name"));
		hotkeylist->add(DummyWidget(colSpan = 2));
		hotkeylist->add(Label(text = "Name"));
		hotkeylist->add(DummyWidget(colSpan = 2));
	}
	else
	{
		hotkeylist = Rows<8>(vAlign = 0.0);
		hotkeylist->add(Label(text = "Name"));
		hotkeylist->add(DummyWidget());
		hotkeylist->add(Label(text = "Main Key", colSpan = 3));
		hotkeylist->add(Label(text = "Alternate Key", colSpan = 3));
	}
	
	string lower_search = searchstring;
	lowerstr(lower_search);
	//start at 1, skipping ZQKEY_NULL_KEY
	for(uint q = 1; q < ZQKEY_MAX; ++q)
	{
		string name = get_hotkey_name(q);
		string lowername = name;
		lowerstr(lowername);
		if(lowername.find(lower_search) == std::string::npos)
		{
			lbls[q][0].reset();
			lbls[q][1].reset();
			continue;
		}
		hotkeylist->add(Label(text = fmt::format("{} :",get_hotkey_name(q)), hAlign = 1.0));
		char const* txt = get_hotkey_helptext(q);
		if(txt && txt[0])
			hotkeylist->add(INFOBTN(txt));
		else hotkeylist->add(DINFOBTN());
		if(favc)
		{
			hotkeylist->add(Button(text = "Choose", onClick = message::OK, onPressFunc = [&,q]()
				{
					*favc = q;
				}));
		}
		else
		{
			hotkeylist->add(Button(text = "Bind", type = GUI::Button::type::BIND_HOTKEY, hotkey_ptr = &(zq_hotkeys[q]), hotkey_indx = 0, onClick = message::RELOAD));
			hotkeylist->add(Button(text = "Clear", onClick = message::RELOAD,onPressFunc = [&,q]()
			{
				zq_hotkeys[q].setval(0,0,0);
			}));
			hotkeylist->add(lbls[q][0] = Label(minwidth = 5_em, textAlign = 1));
			hotkeylist->add(Button(text = "Bind", type = GUI::Button::type::BIND_HOTKEY, hotkey_ptr = &(zq_hotkeys[q]), hotkey_indx = 1, onClick = message::RELOAD));
			hotkeylist->add(Button(text = "Clear", onClick = message::RELOAD,onPressFunc = [&,q]()
			{
				zq_hotkeys[q].setval(1,0,0);
			}));
			hotkeylist->add(lbls[q][1] = Label(minwidth = 5_em, textAlign = 1));
		}
	}
	std::shared_ptr<GUI::Grid> endrow = Row(padding = 0_px,
		Button(
			text = "C", height = 24_px,
			rightPadding = 0_px,
			onClick = message::RERUN,
			onPressFunc = [&]()
			{
				searchstring.clear();
			}),
		TextField(
			focused = true, leftPadding = 0_px,
			maxwidth = 15_em, maxLength = 255,
			text = searchstring,
			onValueChanged = message::RERUN,
			onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
			{
				searchstring = str;
			}
		));
	if(favc)
	{
		endrow->add(Button(
			text = "Clear",
			minwidth = 90_px,
			onClick = message::OK,
			onPressFunc = [&]()
			{
				*favc = ZQKEY_NULL_KEY;
			}));
	}
	else
	{
		endrow->add(Button(
			text = "OK",
			minwidth = 90_px,
			onClick = message::OK));
	}
	endrow->add(Button(
		text = "Cancel",
		minwidth = 90_px,
		onClick = message::CANCEL));
	
	std::shared_ptr<GUI::Window> window = Window(
		title = "ZQ Hotkeys",
		onClose = message::CANCEL,
		onLoad = [&]()
			{
				Size sz = Size::pixels(spane->getWidth());
				if(sz > dlg_minwid)
					dlg_minwid = sz;
			},
		Column(
			maxwidth = Size::pixels(zq_screen_w),
			spane = ScrollingPane(
				ptr = &scroll_pos1,
				fitParent = true,
				minheight = 500_px,
				minwidth = dlg_minwid,
				hotkeylist),
			endrow
		)
	);
	
	load_labels();
	return window;
}

bool ZQHotkeyDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			load_labels();
			break;
		case message::RERUN:
			while(gui_mouse_b()) rest(1); //wait for mouseup
			rerun_dlg = true;
			return true;
		case message::OK:
			if(confirm_ptr)
				*confirm_ptr = true;
			return true;
		case message::CANCEL:
			if(confirm_ptr)
				*confirm_ptr = false;
			return true;
	}
	return false;
}
