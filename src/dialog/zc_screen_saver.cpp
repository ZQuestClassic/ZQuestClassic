#include "zc_screen_saver.h"
#include "common.h"
#include "alert.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "zc/zelda.h"
#include "zc/render.h"
#include "zc/matrix.h"

extern char cfg_sect[];

void call_screen_saver_settings()
{
	ScreenSaverDialog().show();
}

ScreenSaverDialog::ScreenSaverDialog() :
	speed(ss_speed), density(ss_density), seconds(ss_seconds), enabled(ss_enable)
{}

std::shared_ptr<GUI::Widget> ScreenSaverDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	window = Window(
		title = "Screen Saver",
		info = "If 'Enabled', a screensaver will pop up after a set duration of not doing anything."
		"'Doing Anything' includes keyboard input, controller input, and mouse scroll/clicks (mouse movement is ignored)",
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				Checkbox(text = "Enabled",
					checked = enabled, colSpan = 2,
					onToggleFunc = [&](bool state)
					{
						enabled = state;
					}),
				Label(text = "Idle Time (Seconds):", hAlign = 1.0),
				TextField(type = GUI::TextField::type::INT_DECIMAL,
					low = 1, high = INT_MAX/60, val = seconds < 0 ? 60*5 : seconds,
					fitParent = true,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						seconds = val;
					}),
				Label(text = "Speed:", hAlign = 1.0),
				Slider(
					max_value = 6, value = speed,
					fitParent = true,
					minwidth = 200_px, height = 32_px,
					handle_width = 12_px, bar_width = 5_px,
					onValChangedFunc = [&](int val)
					{
						speed = val;
					}),
				Label(text = "Density:", hAlign = 1.0),
				Slider(
					max_value = 6, value = density,
					fitParent = true,
					minwidth = 200_px, height = 32_px,
					handle_width = 12_px, bar_width = 5_px,
					onValChangedFunc = [&](int val)
					{
						density = val;
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
					text = "Preview",
					minwidth = 90_px,
					onClick = message::PREVIEW),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	return window;
}

bool ScreenSaverDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			ss_speed = speed;
			ss_density = density;
			ss_seconds = seconds;
			ss_enable = enabled;
			zc_set_config(cfg_sect, "ss_speed", speed);
			zc_set_config(cfg_sect, "ss_density", density);
			zc_set_config(cfg_sect, "ss_seconds", seconds);
			zc_set_config(cfg_sect, "ss_enable", enabled ? 1 : 0);
			return true;
		case message::PREVIEW:
		{
			zqdialog_freeze(true);
			screen_saver(speed, density);
			zqdialog_freeze(false);
			update_hw_screen();
			while (gui_mouse_b() || key[KEY_SPACE] || key[KEY_ENTER])
			{
				rest(1);
				poll_keyboard();
			}
			clear_keybuf();
			return false;
		}

		case message::CANCEL:
			return true;
	}
	return false;
}
