#include "zqhotkeys.h"
#include "info.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "base/zsys.h"
#include "gui/use_size.h"
#include <fmt/format.h>
#include "zq/zq_hotkey.h"

ZQHotkeyDialog::ZQHotkeyDialog(bool* confirm):
	confirm_ptr(confirm)
{}

void ZQHotkeyDialog::load_labels()
{
	for(size_t q = 0; q < ZQKEY_MAX; ++q)
	{
		for(size_t alt = 0; alt <= 1; ++alt)
		{
			lbls[q][alt]->setText(zq_hotkeys[q].get_name(alt));
		}
	}
}

static int32_t scroll_pos1 = 0;
std::shared_ptr<GUI::Widget> ZQHotkeyDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> hotkeylist = Rows<8>();
	hotkeylist->add(Label(text = "Name"));
	hotkeylist->add(DummyWidget());
	hotkeylist->add(Label(text = "Main Key", colSpan = 3));
	hotkeylist->add(Label(text = "Alternate Key", colSpan = 3));
	
	for(int q = 0; q < ZQKEY_MAX; ++q)
	{
		hotkeylist->add(Label(text = fmt::format("{} :",get_hotkey_name(q)), hAlign = 1.0));
		hotkeylist->add(DINFOBTN());
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
	std::shared_ptr<GUI::Window> window = Window(
		title = "ZQ Hotkeys",
		onClose = message::CANCEL,
		Column(
			maxwidth = Size::pixels(zq_screen_w),
			ScrollingPane(
				ptr = &scroll_pos1,
				fitParent = true,
				minheight = 500_px,
				hotkeylist),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
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
