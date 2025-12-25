#include "warp_cheat_picker.h"
#include "common.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "zc/cheats.h"
#include "base/dmap.h"
#include "info.h"

extern int cheat_goto_dmap, cheat_goto_screen;
void call_warp_cheat_picker()
{
	WarpCheatPicker().show();
}

WarpCheatPicker::WarpCheatPicker() :
	dest_dmap(cheat_goto_dmap),
	dest_screen(cheat_goto_screen),
	adjusted_screen(cheat_goto_screen),
	list_dmaps(GUI::ZCListData::dmaps(true))
{}

std::shared_ptr<GUI::Widget> WarpCheatPicker::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	window = Window(
		title = "Goto Location",
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				Label(text = "DMap:", hAlign = 1.0),
				DropDownList(data = list_dmaps,
					selectedValue = dest_dmap,
					onSelectFunc = [&](int32_t val)
					{
						dest_dmap = val;
						refresh_screen();
						refresh_label();
					}),
				
				info_label = Label(colSpan = 2),
				
				Label(text = "Screen:", hAlign = 1.0),
				TextField(
					fitParent = true,
					type = GUI::TextField::type::SWAP_BYTE,
					swap_type = nswapHEX,
					low = 0, high = 0x7F, val = dest_screen,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						dest_screen = val;
						refresh_screen();
					})
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
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
	refresh_screen();
	refresh_label();
	return window;
}

void WarpCheatPicker::refresh_label()
{
	dmap const& dm = DMaps[dest_dmap];
	if ((dm.type & dmfTYPE) == dmOVERW)
		info_label->setText("0x*0 - 0x*F, < 0x80");
	else
		info_label->setText(fmt::format("0x*{:X} - 0x*{:X}, < 0x80",
			vbound(dm.xoff, 0x0, 0xF), vbound(dm.xoff + 7, 0x0, 0xF)));
}

void WarpCheatPicker::refresh_screen()
{
	dmap const& dm = DMaps[dest_dmap];
	if ((dm.type & dmfTYPE) == dmOVERW)
	{
		adjusted_screen = dest_screen;
		return;
	}
	if (unsigned((dest_screen & 0xF) + dm.xoff) > 0xF)
		adjusted_screen = -1; // out of bounds
	else adjusted_screen = dest_screen + dm.xoff;
}

bool WarpCheatPicker::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			if (unsigned(adjusted_screen) >= 0x80)
			{
				InfoDialog("Invalid screen", fmt::format("The screen {:02X} is out of bounds for the selected dmap.", dest_screen)).show();
				return false;
			}
			cheats_enqueue(Cheat::GoTo, dest_dmap, dest_screen);
			return true;
		}

		case message::CANCEL:
			return true;
	}
	return false;
}
