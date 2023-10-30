#include "allegro5/allegro_native_dialog.h"
#include "base/zc_alleg.h"
#include "launcher/launcher_dialog.h"
#include "base/zdefs.h"
#include "dialog/common.h"
#include "dialog/alert.h"
#include "dialog/alertfunc.h"
#include "dialog/info.h"
#include "gui/button.h"
#include "gui/tabpanel.h"
#include "launcher/theme_editor.h"
#include "launcher/launcher.h"
#include "base/process_management.h"
#include "launcher/launcher_dialog.h"
#include "gui/builder.h"
#include <fmt/format.h>

LauncherDialog::LauncherDialog(){}

#define LAUNCHER_EXIT_WARN false
static int32_t queue_revert = 0;

bool handle_close_btn_quit()
{
	if(close_button_quit)
	{
		#if LAUNCHER_EXIT_WARN
		bool r = false;
		AlertDialog("Exit",
			"Are you sure?",
			[&](bool ret,bool)
			{
				r = ret;
			}).show();
		close_button_quit = false;
		return (exiting_program = r);
		#else
		return (exiting_program = true);
		#endif
	}
	return false;
}

int32_t LauncherDialog::launcher_on_tick()
{
	if(handle_close_btn_quit())
		return ONTICK_EXIT;
	if(queue_revert > 0)
	{
		if(!--queue_revert)
		{
			AlertDialog("Theme Check",
				"Would you like to revert?",
				[&](bool ret,bool)
				{
					if(ret)
					{
						reset_theme();
					}
					else
					{
						set_theme(tmp_themefile);
					}
				}, "Revert", "Keep", 60*4, true).show();
			pendDraw();
		}
	}
	return ONTICK_CONTINUE;
}

extern char zthemepath[4096];
static char zmodpath[4096] = {0};
static char savpath[4096] = {0};

//{ Lists
namespace GUI::Lists
{
	static const ListData screenshotOutputList
	{
		{ "BMP", 0 },
		{ "GIF", 1 },
		{ "JPEG", 2 },
		{ "PNG", 3 },
		{ "PCX", 4 },
		{ "TGA", 5 }
	};
	static const ListData bottom8_list
	{
		{ "No Cover", 0 },
		{ "Pixelated Cover", 1 },
		{ "Normal Cover", 2 }
	};
	static const ListData autoBackupCopiesList = ListData::numbers(false, 0, 11);
	static const ListData autoSaveCopiesList = ListData::numbers(false, 1, 10);
	static const ListData frameRestSuggestList = ListData::numbers(false, 0, 3);

	static const ListData gfxDriverList
	{
		{ "Default", 0 },
		{ "Direct3D", 1 },
		{ "OpenGL", 2 }
	};
	
	static const ListData nameEntryList
	{
		{ "Keyboard", 0 },
		{ "Letter Grid", 1 },
		{ "Extended Letter Grid", 2 }
	};
	
	static const ListData resPresetList
	{
		{ "640x480", 2 },
		{ "960x720", 3 },
		{ "1280x960", 4 },
		{ "1600x1200", 5 }
	};

	static const ListData quickSlotList
	{
		{ "Disabled", 0 },
		{ "Slot 1", 1 },
		{ "Slot 2", 2 },
		{ "Slot 3", 3 },
		{ "Slot 4", 4 },
		{ "Slot 5", 5 },
		{ "Slot 6", 6 },
		{ "Slot 7", 7 },
		{ "Slot 8", 8 },
		{ "Slot 9", 9 },
		{ "Slot 10", 10 },
		{ "Slot 11", 11 },
		{ "Slot 12", 12 },
		{ "Slot 13", 13 },
		{ "Slot 14", 14 },
		{ "Slot 15", 15 }
	};

	static const ListData jitThreadsList
	{
		{ "All Available", -1 },
		{ "1/2 Available", -2 },
		{ "1/3 Available", -3 },
		{ "1/4 Available", -4 },
		{ "1", 1 },
		{ "2", 2 },
		{ "3", 3 },
		{ "4", 4 },
		{ "8", 8 },
		{ "Disabled", 0 }
	};
}
//}

//{ Macros

//{ Checkbox
#define CONFIG_CHECKBOX(name, app, head, subhead, def) \
DummyWidget(), \
Checkbox( \
	text = name, hAlign = 0.0, \
	checked = zc_get_config(head,subhead,def,app)!=0, \
	onToggleFunc = [&](bool state) \
	{ \
		zc_set_config(head,subhead,state?1:0,app); \
	})

#define CONFIG_CHECKBOX_I(name, app, head, subhead, def, info) \
Button(forceFitH = true, text = "?", \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	}), \
Checkbox( \
	text = name, hAlign = 0.0, \
	checked = zc_get_config(head,subhead,def,app)!=0, \
	onToggleFunc = [&](bool state) \
	{ \
		zc_set_config(head,subhead,state?1:0,app); \
	})
	
	
#define CONFIG_CHECKBOX_I_ZCL(name, head, subhead, def, info) \
Button(forceFitH = true, text = "?", \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	}), \
Checkbox( \
	text = name, hAlign = 0.0, \
	checked = zc_get_config(head,subhead,def)!=0, \
	onToggleFunc = [&](bool state) \
	{ \
		zc_set_config(head,subhead,state?1:0); \
	})
//}

//{ Dropdown

//{ Standard
#define CONFIG_DROPDOWN_MINWIDTH 7_em
#define CONFIG_DROPDOWN(name, app, head, subhead, def, list) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = CONFIG_DROPDOWN_MINWIDTH, \
	selectedValue = zc_get_config(head,subhead,def,app), \
	onSelectFunc = [&](int32_t val) \
	{ \
		zc_set_config(head,subhead,val,app); \
	} \
), \
DummyWidget()

#define CONFIG_DROPDOWN_I(name, app, head, subhead, def, list, info) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = CONFIG_DROPDOWN_MINWIDTH, \
	selectedValue = zc_get_config(head,subhead,def,app), \
	onSelectFunc = [&](int32_t val) \
	{ \
		zc_set_config(head,subhead,val,app); \
	} \
), \
Button(forceFitH = true, text = "?", \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})
//}
//{ GFXDriver
#define GFXCARD_DROPDOWN_MINWIDTH 7_em
#define GFXCARD_DROPDOWN(name, app, head, subhead, def, list) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = GFXCARD_DROPDOWN_MINWIDTH, \
	selectedValue = getGFXDriverID(zc_get_config(head,subhead,getGFXDriverStr(def),app)), \
	onSelectFunc = [&](int32_t val) \
	{ \
		if(val > -1) \
			zc_set_config(head,subhead,getGFXDriverStr(val),app); \
	} \
), \
DummyWidget()

#define GFXCARD_DROPDOWN_I(name, app, head, subhead, def, list, info) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = GFXCARD_DROPDOWN_MINWIDTH, \
	selectedValue = getGFXDriverID(zc_get_config(head,subhead,getGFXDriverStr(def),app)), \
	onSelectFunc = [&](int32_t val) \
	{ \
		if(val > -1) \
			zc_set_config(head,subhead,getGFXDriverStr(val),app); \
	} \
), \
Button(forceFitH = true, text = "?", \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})
//}

//}

//{ TextField
#define CONFIG_TEXTFIELD_MINWIDTH 7_em

#define CONFIG_TEXTFIELD_FL(name, app, head, subhead, def, _min, _max, _places) \
Label(text = name, hAlign = 1.0), \
TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::FIXED_DECIMAL, fitParent = true, \
	low = _min*int32_t(pow(10, _places)), high = _max*int32_t(pow(10, _places)), \
	val = zc_get_config(head, subhead, def, app)*int32_t(pow(10, _places)), \
	places = _places, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(head, subhead, val/double(pow(10, _places)), app); \
	}), \
DummyWidget()

#define CONFIG_TEXTFIELD(name, app, head, subhead, def, _min, _max) \
Label(text = name, hAlign = 1.0), \
TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = zc_get_config(head, subhead, def, app), \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(head, subhead, val, app); \
	}), \
DummyWidget()

#define L_CONFIG_TEXTFIELD(var, name, app, head, subhead, def, _min, _max) \
Label(text = name, hAlign = 1.0), \
var = TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = zc_get_config(head, subhead, def, app), \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(head, subhead, val, app); \
	}), \
DummyWidget()

#define CONFIG_TEXTFIELD_I(name, app, head, subhead, def, _min, _max, info) \
Label(text = name, hAlign = 1.0), \
TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = zc_get_config(head, subhead, def, app), \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(head, subhead, val, app); \
	}), \
Button(forceFitH = true, text = "?", \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})

//{ Helpers

/* _al_stricmp:
 *  Case-insensitive comparison of 7-bit ASCII strings.
 */
int _al_stricmp(const char *s1, const char *s2)
{
   int c1, c2;
   ASSERT(s1);
   ASSERT(s2);

   do {
      c1 = tolower(*(s1++));
      c2 = tolower(*(s2++));
   } while ((c1) && (c1 == c2));

   return c1 - c2;
}

//{ Graphics Card
#define CMP_GFX(str, val) \
if(!_al_stricmp(str,configstr)) \
	return val

#define STR_GFX(str, val) \
case val: \
	strcpy(gfx_card_buf, str); \
	break

int32_t getGFXDriverID(char const* configstr)
{
	CMP_GFX("Default", 0);
	CMP_GFX("Direct3D", 1);
	CMP_GFX("OpenGL", 2);
	return 0;
}

static char gfx_card_buf[10] = {0};
char const* getGFXDriverStr(int32_t id)
{
	switch(id)
	{
		STR_GFX("Default", 0);
		STR_GFX("Direct3D", 1);
		STR_GFX("OpenGL", 2);
		default: gfx_card_buf[0]=0; break;
	}
	return gfx_card_buf;
}
//}

//}

char theme_saved_filepath[4096] = {0};
static bool set_zq_theme()
{
	zc_set_config("Theme","theme_filename",theme_saved_filepath, App::zquest);
	return false;
}
static bool set_zc_theme()
{
	zc_set_config("Theme","theme_filename",theme_saved_filepath, App::zelda);
	return false;
}
static bool set_zcl_theme()
{
	zc_set_config("Theme","theme_filename",theme_saved_filepath);
	return false;
}

bool LauncherDialog::load_theme(char const* themefile)
{
	if(themefile && fileexists(themefile))
	{
		load_themefile(themefile);
		pendDraw();
		return true;
	}
	return false;
}

static bool has_checked_for_updates = false;
static bool found_new_update = false;
static std::string next_version;
static std::string next_asset_url;

static bool check_for_updates()
{
	if (has_checked_for_updates)
		return found_new_update;

	has_checked_for_updates = true;

	std::string output;
	bool success = run_and_get_output(ZUPDATER_FILE, {
		"-headless",
		"-print-next-release",
	}, output);

	if (!success)
		return found_new_update = false;

	auto output_map = parse_output_map(output);
	if (!output_map.contains("tag_name"))
		return found_new_update = false;
	if (!output_map.contains("asset_url"))
		return found_new_update = false;

	next_version = output_map["tag_name"];
	next_asset_url = output_map["asset_url"];
	return found_new_update = next_version != getReleaseTag();
}

std::shared_ptr<GUI::Widget> LauncherDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	using namespace GUI::Lists;

	if (zc_get_config("ZLAUNCH", "check_for_updates", false, App::launcher))
		check_for_updates();

	queue_revert = 0;
	int32_t scale_large = zc_get_config("zquest","scale_large",1,App::zquest);
	int32_t def_large_w = LARGE_W*scale_large;
	int32_t def_large_h = LARGE_H*scale_large;
	int rightmost;
	int bottommost;
	ALLEGRO_MONITOR_INFO info;

	std::shared_ptr<GUI::TabPanel> tabPanel;

	al_get_monitor_info(0, &info);
	rightmost = info.x2 - info.x1;
	bottommost = info.y2 - info.y1;
	
	rightmost=rightmost - 48;
	bottommost=bottommost - 48;
	window = Window(
		title = "",
		width = 0_px + zq_screen_w,
		height = 0_px + zq_screen_h,
		onClose = message::EXIT,
		use_vsync = true,
		onTick = [&](){return launcher_on_tick();},
		Column(
			tabPanel = TabPanel(
				focused = true,
				minwidth = zq_screen_w - 60_px,
				minheight = zq_screen_h - 100_px,
				onSwitch = [&](size_t oldind, size_t newind)
				{
					if (newind == 6)
					{
						check_for_updates();
						btn_download_update->setText(found_new_update ? "Update to " + next_version : "No update found");
						btn_download_update->setDisabled(!found_new_update);
						btn_release_notes->setDisabled(!found_new_update);
					}

					if(newind < 3 || newind >= 6) //not a theme tab
					{
						if(oldind < 3) return; //Already not a theme tab
						//Load the ZCL theme
						char const* themepath = zc_get_config("Theme","theme_filename","themes/mooshmood.ztheme");
						load_theme(themepath);
					}
					else
					{
						App a = App::launcher;
						if(newind == 4) a = App::zquest;
						else if(newind == 3) a = App::zelda;
						char const* themepath = zc_get_config("Theme","theme_filename","themes/mooshmood.ztheme", a);
						load_theme(themepath);
						strcpy(zthemepath, themepath);
						char path[4096] = {0};
						relativize_path(path, zthemepath);
						tf_theme[newind-3]->setText(path);
						btn_save[newind-3]->setDisabled(false);
						for(auto q = strlen(zthemepath)-1; q > 0 && !(zthemepath[q] == '/' || zthemepath[q] == '\\'); --q)
						{
							zthemepath[q] = 0;
						}
					}
				},
				TabRef(name = "ZC Player", Row(framed = true,
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX("Fullscreen",App::zelda,"zeldadx","fullscreen",0),
						CONFIG_CHECKBOX("Cap FPS",App::zelda,"zeldadx","throttlefps",1),
						CONFIG_CHECKBOX("Show FPS",App::zelda,"zeldadx","showfps",0),
						CONFIG_CHECKBOX("Cont. Heart Beep",App::zelda,"zeldadx","heart_beep",0),
						CONFIG_CHECKBOX("Disable Sound",App::zelda,"zeldadx","nosound",0),
						CONFIG_CHECKBOX_I("Replay New Saves",App::zelda,"zeldadx","replay_new_saves",0,"Starting a new game will prompt recording to a .zplay file"),
						CONFIG_CHECKBOX_I("Replay Debug",App::zelda,"zeldadx","replay_debug",1,"Record debug information when making a .zplay file")
					),
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX("Force-reload Quest Icons",App::zelda,"zeldadx","reload_game_icons",0),
						CONFIG_CHECKBOX_I("Allow Multiple Instances",App::zelda,"zeldadx","multiple_instances",0,"This can cause issues including but not limited to save file deletion."),
						CONFIG_CHECKBOX("Click to Freeze",App::zelda,"zeldadx","clicktofreeze",1),
						CONFIG_CHECKBOX_I("Quickload Last Quest",App::zelda,"zeldadx","quickload_last",0,"Unless 'Quickload Slot' is set, this will load the last quest played immediately upon launching."),
						CONFIG_CHECKBOX_I("Autosave Window Size Changes",App::zelda,"zeldadx","save_drag_resize",0,"Makes any changes to the window size by dragging get saved for whenever you open the program next."),
						CONFIG_CHECKBOX_I("Lock Aspect Ratio On Resize",App::zelda,"zeldadx","drag_aspect",1,"Makes any changes to the window size by dragging get snapped to ZC's default (4:3) aspect ratio."),
						CONFIG_CHECKBOX_I("Save Window Position",App::zelda,"zeldadx","save_window_position",0,"Remembers the last position of the ZC Window."),
						CONFIG_CHECKBOX_I("Force Integer Values for Scale",App::zelda,"zeldadx","scaling_force_integer",0,"Locks the screen to only scale by an integer value. Results in perfect pixel art scaling, at the expense of not using the entire availabe window space."),
						CONFIG_CHECKBOX_I("Linear Scaling",App::zelda,"zeldadx","scaling_mode",0,"Use linear scaling when upscaling the window. If off, the default is nearest-neighbor scaling. If on, things will look a little blurry at most resolutions"),
						CONFIG_CHECKBOX_I("Ignore Monitor Scale",App::zelda,"gui","ignore_monitor_scale",1,"Ignore monitor DPI scale (i.e. Windows' \"Scale and Layout\" scale)"),
						CONFIG_CHECKBOX_I("Monochrome Debuggers",App::zelda,"CONSOLE","monochrome_debuggers",0,"Use non-colored debugger text."),
						CONFIG_CHECKBOX_I("(EXPERIMENTAL) JIT script compilation",App::zelda,"ZSCRIPT","jit",0,"Compile scripts to machine code. Depending on the script, can be up to 10x faster. 64-bit only"),
						CONFIG_CHECKBOX_I("(EXPERIMENTAL) JIT precompile",App::zelda,"ZSCRIPT","jit_precompile",0,"Do all JIT compilation upfront on quest load. Can take a couple minutes, but will avoid random pauses during play."),
						CONFIG_CHECKBOX_I("Auto restart in Test Mode",App::zelda,"zeldadx","test_mode_auto_restart",0,"When a quest file is saved while in test mode, auto restarts zplayer.")
					),
					Rows<3>(fitParent = true,
						CONFIG_TEXTFIELD_FL("Cursor Scale:", App::zelda,"zeldadx","cursor_scale_large",1.5,1.0,5.0, 4),
						CONFIG_DROPDOWN_I("Screenshot Output:", App::zelda,"zeldadx","snapshot_format",3,screenshotOutputList,"The output format of screenshots"),
						CONFIG_DROPDOWN_I("Name Entry Mode:", App::zelda,"zeldadx","name_entry_mode",0,nameEntryList,"The entry method of save file names."),
#ifdef _WIN32
						CONFIG_TEXTFIELD_I("Window Width:",App::zelda,"zeldadx","window_width", 640, 256, 3000, "The width of the ZC window, for windowed mode"),
						CONFIG_TEXTFIELD_I("Window Height:",App::zelda,"zeldadx","window_height", 480, 240, 2250, "The height of the ZC window, for windowed mode"),
#else
						CONFIG_TEXTFIELD_I("Window Width:",App::zelda,"zeldadx","window_width", -1, -1, 3000, "The width of the ZC window, for windowed mode. If -1 the largest possible window will be made without distorting the pixel content."),
						CONFIG_TEXTFIELD_I("Window Height:",App::zelda,"zeldadx","window_height", -1, -1, 2250, "The height of the ZC window, for windowed mode. If -1 the largest possible window will be made without distorting the pixel content."),
#endif
						CONFIG_TEXTFIELD_I("Saved Window X:",App::zelda,"zeldadx","window_x", 0, 0, rightmost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
						CONFIG_TEXTFIELD_I("Saved Window Y:",App::zelda,"zeldadx","window_y", 0, 0, bottommost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
						GFXCARD_DROPDOWN("Graphics Driver:", App::zelda, "graphics", "driver", 0, gfxDriverList),
						CONFIG_DROPDOWN_I("(EXPERIMENTAL) JIT threads:",App::zelda,"ZSCRIPT","jit_threads",-2,jitThreadsList,"Use background threads to speed up JIT compilation"),
						//
						Button(hAlign = 1.0, forceFitH = true,
							text = "Browse Module", onPressFunc = [&]()
							{
								if(getname("Load Module [ZC]", "zmod", NULL, zmodpath, false))
								{
									char path[4096] = {0};
									relativize_path(path, temppath);
									tf_module_zc->setText(path);
									zc_set_config("ZCMODULE", "current_module", path, App::zelda);
									for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
									{
										temppath[q] = 0;
									}
									strcpy(zmodpath, temppath);
								}
							}),
						tf_module_zc = TextField(
							read_only = true, fitParent = true,
							forceFitW = true
						),
						DummyWidget(),
						//
						Button(hAlign = 1.0, forceFitH = true,
							text = "Select Save File", onPressFunc = [&]()
							{
								if(getname("Load Save File (.sav)", "sav", NULL, savpath, false))
								{
									char path[4096] = {0};
									relativize_path(path, temppath);
									tf_savefile->setText(path);
									zc_set_config("SAVEFILE", "save_filename", path, App::zelda);
									for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
									{
										temppath[q] = 0;
									}
									strcpy(savpath, temppath);
								}
							}),
						tf_savefile = TextField(
							read_only = true, fitParent = true,
							forceFitW = true,
							text = zc_get_config("SAVEFILE", "save_filename", "zc.sav", App::zelda)
						),
						DummyWidget(),
						//
						CONFIG_DROPDOWN_I("Quickload Slot:", App::zelda,"zeldadx","quickload_slot",0,quickSlotList,"Unless 'disabled', this save slot will be immediately loaded upon launching."),
						CONFIG_TEXTFIELD_I("Fastforward FPS Cap:", App::zelda, "zeldadx", "maxfps", 0, 0, 6000, "If >0 and 'Cap FPS' is off, FPS will not go above this value. This allows fast forwarding while still keeping a consistent and playable speed. Does not work for values < 60.")
					)
				)),
				TabRef(name = "ZC Editor", Row(framed = true,
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX_I("Fullscreen",App::zquest,"zquest","fullscreen",0,"Exactly stable."),
						CONFIG_CHECKBOX("VSync",App::zquest,"zquest","vsync",1),
						CONFIG_CHECKBOX("Show FPS",App::zquest,"zquest","showfps",0),
						CONFIG_CHECKBOX("Disable Sound",App::zquest,"zquest","nosound",0),
						CONFIG_CHECKBOX("Animate Combos",App::zquest,"zquest","animation_on",1),
						CONFIG_CHECKBOX("Combo Brush",App::zquest,"zquest","combo_brush",0),
						CONFIG_CHECKBOX("Enable Tooltips",App::zquest,"zquest","enable_tooltips",1),
						CONFIG_CHECKBOX("Tooltips Highlight Target",App::zquest,"zquest","ttip_highlight",1),
						CONFIG_CHECKBOX("Floating Brush",App::zquest,"zquest","float_brush",0),
						CONFIG_CHECKBOX("Mouse Scroll",App::zquest,"zquest","mouse_scroll",0),
						CONFIG_CHECKBOX("Overwrite Protection",App::zquest,"zquest","overwrite_prevention",0),
						CONFIG_CHECKBOX("Palette Cycle",App::zquest,"zquest","cycle_on",1),
						CONFIG_CHECKBOX_I("Reload Last Quest",App::zquest,"zquest","open_last_quest",1,"On launching, immediately attempt to open the last file edited."),
						CONFIG_CHECKBOX("Save Paths",App::zquest,"zquest","save_paths",1),
						CONFIG_CHECKBOX_I("Show Misalignments",App::zquest,"zquest","show_misalignments",0,"Shows blinking arrows on the sides of the screen where the solidity does not match across the screen border."),
						CONFIG_CHECKBOX("Tile Protection",App::zquest,"zquest","tile_protection",1)
					),
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX("Uncompressed Autosaves",App::zquest,"zquest","uncompressed_auto_saves",1),
						CONFIG_CHECKBOX_I("Static effect for invalid data",App::zquest,"zquest","invalid_static",0,"Uses an animated static effect for 'invalid' things (filtered out combos, nonexistant screens on the minimap, etc)"),
						CONFIG_CHECKBOX_I("Warn on Init Script Change",App::zquest,"zquest","warn_initscript_changes",1,"When compiling ZScript, receive a warning when the global init script changes (which may break existing save files for the quest)"),
						CONFIG_CHECKBOX_I("Show Ruleset Dialog on New Quest",App::zquest,"zquest","rulesetdialog",0,"On creating a 'New' quest, automatically pop up the 'Pick Ruleset' menu. (This can be found any time at 'Quest->Options->Pick Ruleset')"),
						CONFIG_CHECKBOX_I("Monochrome Debuggers",App::zquest,"CONSOLE","monochrome_debuggers",0,"Use non-colored debugger text."),
						CONFIG_CHECKBOX_I("Disable Level Palette Shortcuts",App::zquest,"zquest","dis_lpal_shortcut",1,"If enabled, keyboard shortcuts that change the screen's palette are disabled."),
						CONFIG_CHECKBOX_I("Autosave Window Size Changes",App::zquest,"zquest","save_drag_resize",0,"Makes any changes to the window size by dragging get saved for whenever you open the program next."),
						CONFIG_CHECKBOX_I("Lock Aspect Ratio On Resize",App::zquest,"zquest","drag_aspect",1,"Makes any changes to the window size by dragging get snapped to ZQuest's default (4:3) aspect ratio."),
						CONFIG_CHECKBOX_I("Save Window Position",App::zquest,"zquest","save_window_position",0,"Remembers the last position of the ZQuest Window."),
						CONFIG_CHECKBOX_I("Force Integer Values for Scale",App::zquest,"zquest","scaling_force_integer",0,"Locks the screen to only scale by an integer value. Results in perfect pixel art scaling, at the expense of not using the entire availabe window space."),
						CONFIG_CHECKBOX_I("Linear Scaling",App::zquest,"zquest","scaling_mode",0,"Use linear scaling when upscaling the window. If off, the default is nearest-neighbor scaling. If on, things will look a little blurry at most resolutions"),
						CONFIG_CHECKBOX_I("Ignore Monitor Scale",App::zquest,"gui","ignore_monitor_scale",1,"Ignore monitor DPI scale (i.e. Windows' \"Scale and Layout\" scale)"),
						CONFIG_CHECKBOX_I("Record During Test Feature",App::zquest,"zquest","test_mode_record",0,"Save a recording to replays/test_XXXXXXXX.zplay when using the GUI test feature."),
						CONFIG_CHECKBOX_I("Custom Fonts",App::zquest,"gui","custom_fonts",1,"Use custom fonts from the 'customfonts' folder for UI elements.")
					),
					Rows<3>(fitParent = true,
						CONFIG_TEXTFIELD_FL("Cursor Scale:", App::zquest,"zquest","cursor_scale_large",1.5,1.0,5.0, 4),
						CONFIG_DROPDOWN_I("Bottom 8 pixels:", App::zquest,"ZQ_GUI","bottom_8_pixels",0,bottom8_list,"How to hide the bottom 8 screen pixels"),
						CONFIG_DROPDOWN_I("Screenshot Output:", App::zquest,"zquest","snapshot_format",3,screenshotOutputList,"The output format of screenshots"),
						CONFIG_DROPDOWN_I("Auto-Backup Retention:", App::zquest,"zquest","auto_backup_retention",0,autoBackupCopiesList,"The number of auto-backups to keep"),
						CONFIG_DROPDOWN_I("Auto-Save Retention:", App::zquest,"zquest","auto_save_retention",9,autoSaveCopiesList,"The number of auto-saves to keep"),
						CONFIG_TEXTFIELD_I("Auto-Save Interval:", App::zquest, "zquest", "auto_save_interval", 5, 0, 300, "Frequency of auto saves, in minutes. Valid range is 0-300, where '0' disables autosaves alltogether."),

#ifdef _WIN32
						CONFIG_TEXTFIELD_I("Window Width (Large Mode):",App::zquest,"zquest","large_window_width", def_large_w, 200, 3000, "The width of the ZQuest window in large mode"),
						CONFIG_TEXTFIELD_I("Window Height (Large Mode):",App::zquest,"zquest","large_window_height", def_large_h, 150, 2250, "The height of the ZQuest window in large mode"),
#else
						CONFIG_TEXTFIELD_I("Window Width (Large Mode):",App::zquest,"zquest","large_window_width", -1, -1, 3000, "The width of the ZQuest window in large mode. If -1 the largest possible window will be made without distorting the pixel content."),
						CONFIG_TEXTFIELD_I("Window Height (Large Mode):",App::zquest,"zquest","large_window_height", -1, -1, 2250, "The height of the ZQuest window in large mode. If -1 the largest possible window will be made without distorting the pixel content."),
#endif

						CONFIG_TEXTFIELD_I("Saved Window X:",App::zquest,"zquest","window_x", 0, 0, rightmost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
						CONFIG_TEXTFIELD_I("Saved Window Y:",App::zquest,"zquest","window_y", 0, 0, bottommost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
						GFXCARD_DROPDOWN("Graphics Driver:", App::zquest, "graphics", "driver", 0, gfxDriverList),
						Button(hAlign = 1.0, forceFitH = true,
							text = "Browse Module", onPressFunc = [&]()
							{
								if(getname("Load Module [ZQ]", "zmod", NULL, zmodpath, false))
								{
									char path[4096] = {0};
									relativize_path(path, temppath);
									tf_module_zq->setText(path);
									zc_set_config("ZCMODULE", "current_module", path, App::zquest);
									for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
									{
										temppath[q] = 0;
									}
									strcpy(zmodpath, temppath);
								}
							}),
						tf_module_zq = TextField(
							read_only = true, fitParent = true,
							forceFitW = true
						),
						DummyWidget()
					))
				),
				TabRef(name = "ZC Launcher", Column(padding = 0_px,
					Label(text = "ZCL options may require relaunching ZCL to take effect!"),
					Row(framed = true,
						Rows<2>(fitParent = true,
							CONFIG_CHECKBOX_I("Ignore Monitor Scale",App::launcher,"gui","ignore_monitor_scale",1,"Ignore monitor DPI scale (i.e. Windows' \"Scale and Layout\" scale)")
						),
						Rows<3>(fitParent = true,
							Button(hAlign = 1.0, forceFitH = true,
								text = "Browse Module", onPressFunc = [&]()
								{
									if(getname("Load Module [ZCL]", "zmod", NULL, zmodpath, false))
									{
										char path[4096] = {0};
										relativize_path(path, temppath);
										tf_module_zcl->setText(path);
										zc_set_config("ZCMODULE", "current_module", path);
										for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
										{
											temppath[q] = 0;
										}
										strcpy(zmodpath, temppath);
									}
								}),
							tf_module_zcl = TextField(
								minwidth = 10_em,
								read_only = true, fitParent = true,
								forceFitW = true
							),
							DummyWidget()
						)
					)
				)),
				TabRef(name = "Player Theme", Rows<2>(
					tf_theme[0] = TextField(read_only = true, maxLength = 255),
					Button(text = "Browse", fitParent = true, onPressFunc = [&]()
						{
							if(getname("Load Theme", "ztheme", NULL, zthemepath, false))
							{
								char path[4096] = {0};
								relativize_path(path, temppath);
								tf_theme[0]->setText(path);
								for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
								{
									temppath[q] = 0;
								}
								strcpy(zthemepath, temppath);
								bool loaded = load_theme(path);
								btn_save[0]->setDisabled(!loaded);
								if(loaded)
								{
									pendDraw();
									//!TODO connor: add A5 text enter/esc to accept/revert
									strcpy(tmp_themefile, path);
									queue_revert = 2;
								}
							}
						}),
					btn_save[0] = Button(text = "Save", hAlign = 1.0, onPressFunc = [&]()
					{
						zc_set_config("Theme","theme_filename",tmp_themefile,App::zelda);
					}),
					Button(text = "Edit", fitParent = true, onPressFunc = [&]()
					{
						ThemeEditor(theme_saved_filepath).show();
						queue_revert = 0;
						if(theme_saved_filepath[0])
						{
							tf_theme[0]->setText(theme_saved_filepath);
							strcpy(tmp_themefile, theme_saved_filepath);
						}
					})
				)),
				TabRef(name = "Editor Theme", Rows<2>(
					tf_theme[1] = TextField(read_only = true, maxLength = 255),
					Button(text = "Browse", fitParent = true, onPressFunc = [&]()
						{
							if(getname("Load Theme", "ztheme", NULL, zthemepath, false))
							{
								char path[4096] = {0};
								relativize_path(path, temppath);
								tf_theme[1]->setText(path);
								for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
								{
									temppath[q] = 0;
								}
								strcpy(zthemepath, temppath);
								bool loaded = load_theme(path);
								btn_save[1]->setDisabled(!loaded);
								if(loaded)
								{
									pendDraw();
									//!TODO connor: add A5 text enter/esc to accept/revert
									strcpy(tmp_themefile, path);
									queue_revert = 2;
								}
								//else handle failure?
							}
						}),
					btn_save[1] = Button(text = "Save", hAlign = 1.0, onPressFunc = [&]()
					{
						zc_set_config("Theme","theme_filename",tmp_themefile,App::zquest);
					}),
					Button(text = "Edit", fitParent = true, onPressFunc = [&]()
					{
						ThemeEditor(theme_saved_filepath).show();
						queue_revert = 0;
						if(theme_saved_filepath[0])
						{
							tf_theme[1]->setText(theme_saved_filepath);
							strcpy(tmp_themefile, theme_saved_filepath);
						}
					})
				)),
				TabRef(name = "ZCL Theme", Rows<2>(
					tf_theme[2] = TextField(read_only = true, maxLength = 255),
					Button(text = "Browse", fitParent = true, onPressFunc = [&]()
						{
							if(getname("Load Theme", "ztheme", NULL, zthemepath, false))
							{
								char path[4096] = {0};
								relativize_path(path, temppath);
								tf_theme[2]->setText(path);
								for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
								{
									temppath[q] = 0;
								}
								strcpy(zthemepath, temppath);
								bool loaded = load_theme(path);
								btn_save[2]->setDisabled(!loaded);
								if(loaded)
								{
									pendDraw();
									//!TODO connor: add A5 text enter/esc to accept/revert
									strcpy(tmp_themefile, path);
									queue_revert = 2;
								}
							}
						}),
					btn_save[2] = Button(text = "Save", hAlign = 1.0, onPressFunc = [&]()
					{
						zc_set_config("Theme","theme_filename",tmp_themefile,App::launcher);
					}),
					Button(text = "Edit", fitParent = true, onPressFunc = [&]()
					{
						ThemeEditor(theme_saved_filepath).show();
						queue_revert = 0;
						if(theme_saved_filepath[0])
						{
							tf_theme[2]->setText(theme_saved_filepath);
							strcpy(tmp_themefile, theme_saved_filepath);
						}
					})
				)),
				TabRef(name = "Update", Column(padding = 0_px,
					Row(framed = true,
						Rows<2>(fitParent = true,
							CONFIG_CHECKBOX_I("Check for updates on startup",App::launcher,"ZLAUNCH","check_for_updates",0,"Check for updates when starting ZLauncher. When a new version is available, ZLauncher will focus the Update tab on startup.")
						)
					),
					Label(text = fmt::format("Current version: {}", getReleaseTag())),
					Button(
						text = "View Release Notes",
						onClick = message::ZU_RELEASE_NOTES
					),
					Row(
						Rows<2>(fitParent = true,
							btn_download_update = Button(
								// TODO: Will change button text dynamically, but that breaks an assumption in Button::realize
								// re: usage of its `text.data()`. So let's reserve a large enough string to workaround that.
								text = std::string(200, ' '),
								maxwidth = 250_px,
								onClick = message::ZU
							),
							btn_release_notes = Button(
								text = "View Latest Release Notes",
								onClick = message::ZU_RELEASE_NOTES_NEXT
							)
						)
					)
#ifdef UPDATER_USES_PYTHON
					, Label(text = "Note: the updater requires Python 3 to be installed and configured in PATH")
#endif
				))
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "ZC Player",
					minwidth = 90_px,
					onClick = message::ZC),
				Button(
					text = "ZC Editor",
					minwidth = 90_px,
					onClick = message::ZQ)
			)
		)
	);

	if (found_new_update)
	{
		// Note: the onSwitch callback above does not run when calling `switchTo` below because the dialog has not been realized yet.
		btn_download_update->setText("Update to " + next_version);
		btn_download_update->setDisabled(false);
		btn_release_notes->setDisabled(false);
		tabPanel->switchTo(6);
	}
	
	char path[4096] = {0};
	relativize_path(path, zc_get_config("ZCMODULE", "current_module", "modules/classic.zmod", App::zelda));
	tf_module_zc->setText(path);
	relativize_path(path, zc_get_config("ZCMODULE", "current_module", "modules/classic.zmod", App::zquest));
	tf_module_zq->setText(path);
	relativize_path(path, zc_get_config("ZCMODULE", "current_module", "modules/classic.zmod"));
	tf_module_zcl->setText(path);
	return window;
}

bool LauncherDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::ZC:
			launch_process(ZPLAYER_FILE);
			break;
		case message::ZQ:
			launch_process(ZEDITOR_FILE);
			break;
		case message::ZU:
		{
			std::string output;
			bool success = run_and_get_output(ZUPDATER_FILE, {
				"-headless",
				"-install",
				"-asset-url", next_asset_url,
			}, output);
			success &= output.find("Success!") != std::string::npos;
			al_trace("%s\n", output.c_str());
			if (success)
			{
				InfoDialog("Updated!", fmt::format("Updated to {}! Restarting ZLauncher...", next_version)).show();
				close_button_quit = true;
				launch_process(ZLAUNCHER_FILE);
			}
			else
			{
				InfoDialog("Updater Error", output).show();
			}
		}
		break;
		case message::ZU_RELEASE_NOTES:
		{
			std::string url = fmt::format("https://www.github.com/{}/releases/tag/{}", getRepo(), getReleaseTag());
#ifdef _WIN32
			std::string cmd = "start " + url;
			system(cmd.c_str());
#elif defined(__APPLE__)
			launch_process("open", {url});
#else
			launch_process("xdg-open", {url});
#endif
		}
		break;
		case message::ZU_RELEASE_NOTES_NEXT:
		{
			std::string url = fmt::format("https://www.github.com/{}/releases/tag/{}", getRepo(), next_version);
#ifdef _WIN32
			std::string cmd = "start " + url;
			system(cmd.c_str());
#elif defined(__APPLE__)
			launch_process("open", {url});
#else
			launch_process("xdg-open", {url});
#endif
		}
		break;
		case message::EXIT:
			close_button_quit = true;
			return false;
	}
	return false;
}
