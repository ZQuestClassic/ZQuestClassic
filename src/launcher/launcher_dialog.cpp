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

static char zthemepath[4096] = {0};

//{ Macros
#define CONFIG_CHECKBOX(name, file, head, subhead, def) \
Checkbox( \
	text = name, hAlign = 0.0, \
	checked = zc_get_config(file,head,subhead,def)!=0, \
	onToggleFunc = [&](bool state) \
	{ \
		zc_set_config(file,head,subhead,state?1:0); \
	})
//}

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
							if(getname("Load Theme", "ztheme", NULL, zthemepath, false))
							{
								char path[4096] = {0};
								relativize_path(path, temppath);
								tf_theme->setText(path);
								for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
								{
									temppath[q] = 0;
								}
								strcpy(zthemepath, temppath);
							}
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
								zc_set_config("zquest","gui_colorset",99);
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
						Button(text = "Save ZCL", onPressFunc = [&]()
						{
							std::string themename;
							themename.assign(tf_theme->getText());
							if(fileexists(themename.c_str()))
							{
								lbl_theme_error->setText("");
								set_config_file("zcl.cfg");
								zc_set_config("Theme","theme_filename",themename.c_str());
								zc_set_config("ZLAUNCH","gui_colorset",99);
								set_config_standard();
							}
							else
							{
								lbl_theme_error->setText("Theme load error!");
							}
						})
					)
				)),
				TabRef(name = "ZC Player", DummyWidget()),
				TabRef(name = "ZQ Creator", Column(
					CONFIG_CHECKBOX("Fullscreen","zquest.cfg","zquest","fullscreen",0),
					CONFIG_CHECKBOX("Small Mode","zquest.cfg","zquest","small",0),
					CONFIG_CHECKBOX("VSync","zquest.cfg","zquest","vsync",1),
					CONFIG_CHECKBOX("Show FPS","zquest.cfg","zquest","showfps",0),
					CONFIG_CHECKBOX("Disable Sound","zquest.cfg","zquest","nosound",0),
					CONFIG_CHECKBOX("Animate Combos","zquest.cfg","zquest","animation_on",1),
					CONFIG_CHECKBOX("Combo Brush","zquest.cfg","zquest","combo_brush",0),
					CONFIG_CHECKBOX("Enable Tooltips","zquest.cfg","zquest","enable_tooltips",1),
					CONFIG_CHECKBOX("Floating Brush","zquest.cfg","zquest","float_brush",0),
					CONFIG_CHECKBOX("Mouse Scroll","zquest.cfg","zquest","mouse_scroll",1),
					CONFIG_CHECKBOX("Overwrite Protection","zquest.cfg","zquest","overwrite_prevention",1),
					CONFIG_CHECKBOX("Palette Cycle","zquest.cfg","zquest","cycle_on",1),
					CONFIG_CHECKBOX("Reload Last Quest","zquest.cfg","zquest","open_last_quest",1),
					CONFIG_CHECKBOX("Save Paths","zquest.cfg","zquest","save_paths",1),
					CONFIG_CHECKBOX("Show Misalignments","zquest.cfg","zquest","show_misalignments",0),
					CONFIG_CHECKBOX("Show Ruleset Dialog on New Quest","zquest.cfg","zquest","rulesetdialog",1),
					CONFIG_CHECKBOX("Tile Protection","zquest.cfg","zquest","tile_protection",1),
					CONFIG_CHECKBOX("Uncompressed Autosaves","zquest.cfg","zquest","uncompressed_auto_saves",1),
					CONFIG_CHECKBOX("Static effect for invalid data","zquest.cfg","zquest","invalid_static",0)
				))
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
			launch_process("zelda.exe");
			break;
		case message::ZQ:
			launch_process("zquest.exe");
			break;
		case message::EXIT:
			close_button_quit = true;
			return false;
	}
	return false;
}
