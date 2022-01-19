#include "launcher_dialog.h"
#include "dialog/common.h"
#include "dialog/alert.h"
#include "launcher.h"
#include <gui/builder.h>
#include <boost/format.hpp>

LauncherDialog::LauncherDialog(){}

int32_t LauncherDialog::launcher_on_tick()
{
	if(close_button_quit)
	{
		bool r = false;
		AlertDialog("Exit",
			"Are you sure?",
			[&](bool ret)
			{
				r = ret;
			}).show();
		close_button_quit = false;
		return r ? ONTICK_EXIT : ONTICK_CONTINUE;
	}
	return ONTICK_CONTINUE;
}

std::shared_ptr<GUI::Widget> LauncherDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	window = Window(
		title = "",
		width = 0_px + zq_screen_w,
		height = 0_px + zq_screen_h,
		onClose = message::EXIT,
		use_vsync = true,
		onTick = [&](){return launcher_on_tick();},
		shortcuts={
			Esc=message::EXIT
		},
		Column(
			TabPanel(
				focused = true,
				minwidth = zq_screen_w - 60_px,
				minheight = zq_screen_h - 100_px,
				TabRef(name = "ZC Player", Column(
					Label(text = "Some Settings here, or like, something?")
				)),
				TabRef(name = "ZQ Creator", DummyWidget())
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "ZC Player",
					minwidth = 90_lpx,
					onClick = message::ZC),
				Button(
					text = "ZQ Creator",
					minwidth = 90_lpx,
					onClick = message::ZQ)
			)
		)
	);
	return window;
}

bool LauncherDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::ZC:
			InfoDialog("ZC", "This should launch ZC, but it pops up this message as a placeholder.").show();
			break;
		case message::ZQ:
			InfoDialog("ZQ", "This should launch ZQ, but it pops up this message as a placeholder.").show();
			break;
		case message::EXIT:
			close_button_quit = true;
			return false;
	}
	return false;
}
