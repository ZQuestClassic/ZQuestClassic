#include "launcher_dialog.h"
#include "dialog/common.h"
#include "dialog/alert.h"
#include "launcher.h"
#include <gui/builder.h>
#include <boost/format.hpp>

LauncherDialog::LauncherDialog(){}

static int32_t queue_revert = 0;
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
	if(queue_revert > 0)
	{
		if(!--queue_revert)
		{
			AlertDialog("Theme Check",
				"Would you like to revert?",
				[&](bool ret)
				{
					if(ret)
						reset_theme();
					else
						set_theme(tmp_themefile);
				}, "Revert", "Keep", 60*4, true).show();
			pendDraw();
		}
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
				TabRef(name = "Themes", Column(
					Label(text = "Here you can load themes, and save them for each program separately."),
					Rows<4>(padding = 0_px,
						Label(text = "Theme file path (.ztheme):"),
						tf_theme = TextField(maxLength = 255, text = zc_get_config("Theme", "theme_filename", "themes/dark.ztheme")),
						Button(text = "Load", onPressFunc = [&]()
						{
							std::string themename;
							themename.assign(tf_theme->getText());
							if(fileexists(themename.c_str()))
							{
								lbl_theme_error->setText("");
								strcpy(tmp_themefile, themename.c_str());
								load_themefile(tmp_themefile);
								queue_revert = 2;
								pendDraw();
							}
							else
							{
								lbl_theme_error->setText("Theme load error!");
							}
						}),
						Button(text = "Browse", onPressFunc = [&]()
						{
							
						}),
						DummyWidget(),
						lbl_theme_error = Label(text = "")
					),
					Rows<2>(padding = 0_px,
						Button(text = "Load ZC", onPressFunc = [&]()
						{
							load_udef_colorset("zc.cfg");
							reset_theme();
							tf_theme->setText(tmp_themefile);
						}),
						Button(text = "Save ZC", onPressFunc = [&]()
						{
							std::string themename;
							themename.assign(tf_theme->getText());
							if(fileexists(themename.c_str()))
							{
								lbl_theme_error->setText("");
								set_config_file("zc.cfg");
								zc_set_config("Theme","theme_filename",themename.c_str());
								zc_set_config("zeldadx","gui_colorset",99);
								set_config_standard();
							}
							else
							{
								lbl_theme_error->setText("Theme load error!");
							}
						}),
						Button(text = "Load ZQ", onPressFunc = [&]()
						{
							load_udef_colorset("zquest.cfg");
							reset_theme();
							tf_theme->setText(tmp_themefile);
						}),
						Button(text = "Save ZQ", onPressFunc = [&]()
						{
							std::string themename;
							themename.assign(tf_theme->getText());
							if(fileexists(themename.c_str()))
							{
								lbl_theme_error->setText("");
								set_config_file("zquest.cfg");
								zc_set_config("Theme","theme_filename",themename.c_str());
								set_config_standard();
							}
							else
							{
								lbl_theme_error->setText("Theme load error!");
							}
						}),
						Button(text = "Load ZCL", onPressFunc = [&]()
						{
							load_udef_colorset("zcl.cfg");
							reset_theme();
							tf_theme->setText(tmp_themefile);
						}),
						Button(text = "Save ZC", onPressFunc = [&]()
						{
							std::string themename;
							themename.assign(tf_theme->getText());
							if(fileexists(themename.c_str()))
							{
								lbl_theme_error->setText("");
								set_config_file("zcl.cfg");
								zc_set_config("Theme","theme_filename",themename.c_str());
								set_config_standard();
							}
							else
							{
								lbl_theme_error->setText("Theme load error!");
							}
						})
					)
				)),
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
