#include "base/allegro_wrapper.h"
#include "launcher_dialog.h"
#include "dialog/common.h"
#include "dialog/alert.h"
#include "dialog/alertfunc.h"
#include "theme_editor.h"
#include "launcher.h"
#include "gui/builder.h"

LauncherDialog::LauncherDialog(){}

#define LAUNCHER_EXIT_WARN false
static int32_t queue_revert = 0;
int32_t LauncherDialog::launcher_on_tick()
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
		return r ? ONTICK_EXIT : ONTICK_CONTINUE;
		#else
		return ONTICK_EXIT;
		#endif
	}
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

	static const ListData titleScreenList
	{
		{ "Classic", 0 },
		{ "Modern", 1 },
		{ "2.5", 2 }
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
}
//}

//{ Macros

//{ Checkbox
#define CONFIG_CHECKBOX(name, file, head, subhead, def) \
DummyWidget(), \
Checkbox( \
	text = name, hAlign = 0.0, \
	checked = zc_get_config(file,head,subhead,def)!=0, \
	onToggleFunc = [&](bool state) \
	{ \
		zc_set_config(file,head,subhead,state?1:0); \
	})

#define CONFIG_CHECKBOX_I(name, file, head, subhead, def, info) \
Button(forceFitH = true, text = "?", \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	}), \
Checkbox( \
	text = name, hAlign = 0.0, \
	checked = zc_get_config(file,head,subhead,def)!=0, \
	onToggleFunc = [&](bool state) \
	{ \
		zc_set_config(file,head,subhead,state?1:0); \
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
#define CONFIG_DROPDOWN(name, file, head, subhead, def, list) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = CONFIG_DROPDOWN_MINWIDTH, \
	selectedValue = zc_get_config(file,head,subhead,def), \
	onSelectFunc = [&](int32_t val) \
	{ \
		zc_set_config(file,head,subhead,val); \
	} \
), \
DummyWidget()

#define CONFIG_DROPDOWN_I(name, file, head, subhead, def, list, info) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = CONFIG_DROPDOWN_MINWIDTH, \
	selectedValue = zc_get_config(file,head,subhead,def), \
	onSelectFunc = [&](int32_t val) \
	{ \
		zc_set_config(file,head,subhead,val); \
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
#define GFXCARD_DROPDOWN(name, file, head, subhead, def, list) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = GFXCARD_DROPDOWN_MINWIDTH, \
	selectedValue = getGFXDriverID(zc_get_config(file,head,subhead,getGFXDriverStr(def))), \
	onSelectFunc = [&](int32_t val) \
	{ \
		if(val > -1) \
			zc_set_config(file,head,subhead,getGFXDriverStr(val)); \
	} \
), \
DummyWidget()

#define GFXCARD_DROPDOWN_I(name, file, head, subhead, def, list, info) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = GFXCARD_DROPDOWN_MINWIDTH, \
	selectedValue = getGFXDriverID(zc_get_config(file,head,subhead,getGFXDriverStr(def))), \
	onSelectFunc = [&](int32_t val) \
	{ \
		if(val > -1) \
			zc_set_config(file,head,subhead,getGFXDriverStr(val)); \
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

#define CONFIG_TEXTFIELD_FL(name, file, head, subhead, def, _min, _max, _places) \
Label(text = name, hAlign = 1.0), \
TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::FIXED_DECIMAL, fitParent = true, \
	low = _min*int32_t(pow(10, _places)), high = _max*int32_t(pow(10, _places)), \
	val = zc_get_config(file, head, subhead, def)*int32_t(pow(10, _places)), \
	places = _places, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(file, head, subhead, val/double(pow(10, _places))); \
	}), \
DummyWidget()

#define CONFIG_TEXTFIELD(name, file, head, subhead, def, _min, _max) \
Label(text = name, hAlign = 1.0), \
TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = zc_get_config(file, head, subhead, def), \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(file, head, subhead, val); \
	}), \
DummyWidget()

#define L_CONFIG_TEXTFIELD(var, name, file, head, subhead, def, _min, _max) \
Label(text = name, hAlign = 1.0), \
var = TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = zc_get_config(file, head, subhead, def), \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(file, head, subhead, val); \
	}), \
DummyWidget()

#define CONFIG_TEXTFIELD_I(name, file, head, subhead, def, _min, _max, info) \
Label(text = name, hAlign = 1.0), \
TextField(fitParent = true, \
	minwidth = CONFIG_TEXTFIELD_MINWIDTH, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = zc_get_config(file, head, subhead, def), \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		zc_set_config(file, head, subhead, val); \
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
	zc_set_config("zquest.cfg","Theme","theme_filename",theme_saved_filepath);
	return false;
}
static bool set_zc_theme()
{
	zc_set_config("zc.cfg","Theme","theme_filename",theme_saved_filepath);
	return false;
}
static bool set_zcl_theme()
{
	zc_set_config("zcl.cfg","Theme","theme_filename",theme_saved_filepath);
	return false;
}

std::shared_ptr<GUI::Widget> LauncherDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	using namespace GUI::Lists;
	queue_revert = 0;
	int32_t scale = zc_get_config("zquest.cfg","zquest","scale",3);
	int32_t scale_large = zc_get_config("zquest.cfg","zquest","scale_large",1);
	int32_t def_large_w = 800*scale_large;
	int32_t def_large_h = 600*scale_large;
	int32_t def_small_w = 320*scale;
	int32_t def_small_h = 240*scale;
	int rightmost;
	int bottommost;
	ALLEGRO_MONITOR_INFO info;

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
			TabPanel(
				focused = true,
				minwidth = zq_screen_w - 60_px,
				minheight = zq_screen_h - 100_px,
				TabRef(name = "ZC Player", Row(framed = true,
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX("Fullscreen","zc.cfg","zeldadx","fullscreen",0),
						CONFIG_CHECKBOX("Cap FPS","zc.cfg","zeldadx","throttlefps",1),
						CONFIG_CHECKBOX("Show FPS","zc.cfg","zeldadx","showfps",0),
						CONFIG_CHECKBOX("Skip Logo","zc.cfg","zeldadx","skip_logo",1),
						CONFIG_CHECKBOX("Skip Title","zc.cfg","zeldadx","skip_title",1),
						CONFIG_CHECKBOX("Skip Quest Icons","zc.cfg","zeldadx","skip_icons",0),
						CONFIG_CHECKBOX("Force-reload Quest Icons","zc.cfg","zeldadx","reload_game_icons",0),
						CONFIG_CHECKBOX("Cont. Heart Beep","zc.cfg","zeldadx","heart_beep",1),
						CONFIG_CHECKBOX("Disable Sound","zc.cfg","zeldadx","nosound",0),
						CONFIG_CHECKBOX_I("Allow Multiple Instances","zc.cfg","zeldadx","multiple_instances",0,"This can cause issues including but not limited to save file deletion."),
						CONFIG_CHECKBOX("Click to Freeze","zc.cfg","zeldadx","clicktofreeze",1),
						CONFIG_CHECKBOX_I("Quickload Last Quest","zc.cfg","zeldadx","quickload_last",0,"Unless 'Quickload Slot' is set, this will load the last quest played immediately upon launching."),
						CONFIG_CHECKBOX_I("Autosave Window Size Changes","zc.cfg","zeldadx","save_drag_resize",0,"Makes any changes to the window size by dragging get saved for whenever you open the program next."),
						CONFIG_CHECKBOX_I("Lock Aspect Ratio On Resize","zc.cfg","zeldadx","drag_aspect",0,"Makes any changes to the window size by dragging get snapped to ZC's default (4:3) aspect ratio."),
						CONFIG_CHECKBOX_I("Save Window Position","zc.cfg","zeldadx","save_window_position",0,"Remembers the last position of the ZC Window."),
						CONFIG_CHECKBOX_I("Force Integer Values for Scale","zc.cfg","zeldadx","scaling_force_integer",1,"Locks the screen to only scale by an integer value. Results in perfect pixel art scaling, at the expense of not using the entire availabe window space."),
						CONFIG_CHECKBOX_I("Linear Scaling","zc.cfg","zeldadx","scaling_mode",1,"Use linear scaling when upscaling the window. If off, the default is nearest-neighbor scaling"),
						CONFIG_CHECKBOX_I("Monochrome Debuggers","zc.cfg","CONSOLE","monochrome_debuggers",0,"Use non-colored debugger text."),
						CONFIG_CHECKBOX_I("Text Readability","zc.cfg","gui","bolder_font",0,"Attempts to make text more readable in some areas (ex. larger, bolder)"),
						CONFIG_CHECKBOX_I("Replay New Saves","zc.cfg","zeldadx","replay_new_saves",0,"Starting a new game will prompt recording to a .zplay file"),
						CONFIG_CHECKBOX_I("Replay Debug","zc.cfg","zeldadx","replay_debug",0,"Record debug information when making a .zplay file")
					),
					Rows<3>(fitParent = true,
						CONFIG_TEXTFIELD_FL("Cursor Scale (small):", "zc.cfg","zeldadx","cursor_scale_small",1.0,1.0,5.0, 4),
						CONFIG_TEXTFIELD_FL("Cursor Scale (large):", "zc.cfg","zeldadx","cursor_scale_large",1.5,1.0,5.0, 4),
						CONFIG_DROPDOWN_I("Frame Rest Suggest:", "zc.cfg","zeldadx","frame_rest_suggest",0,frameRestSuggestList,"Adjusts vsync to attempt to reduce lag. What value works best depends on your hardware / OS.\nPressing '[' and ']' during gameplay will lower/raise this value."),
						CONFIG_DROPDOWN_I("Screenshot Output:", "zc.cfg","zeldadx","snapshot_format",3,screenshotOutputList,"The output format of screenshots"),
						CONFIG_DROPDOWN_I("Name Entry Mode:", "zc.cfg","zeldadx","name_entry_mode",0,nameEntryList,"The entry method of save file names."),
						CONFIG_DROPDOWN_I("Title Screen:", "zc.cfg","zeldadx","title",0,titleScreenList,"Which title screen will be displayed."),
						CONFIG_TEXTFIELD_I("Window Width:","zc.cfg","zeldadx","window_width", 640, 256, 3000, "The width of the ZC window, for windowed mode"),
						CONFIG_TEXTFIELD_I("Window Height:","zc.cfg","zeldadx","window_height", 480, 240, 2250, "The height of the ZC window, for windowed mode"),
						CONFIG_TEXTFIELD_I("Saved Window X:","zc.cfg","zeldadx","window_x", 0, 0, rightmost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
						CONFIG_TEXTFIELD_I("Saved Window Y:","zc.cfg","zeldadx","window_y", 0, 0, bottommost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
#ifndef _WIN32
						// TODO: wgl crashes zc on al_resize_display, so no point in offering this configuration option yet.
						GFXCARD_DROPDOWN("Graphics Driver:", "zc.cfg", "graphics", "driver", 0, gfxDriverList),
#endif
						//
						Button(hAlign = 1.0, forceFitH = true,
							text = "Browse Module", onPressFunc = [&]()
							{
								if(getname("Load Module [ZC]", "zmod", NULL, zmodpath, false))
								{
									char path[4096] = {0};
									relativize_path(path, temppath);
									tf_module_zc->setText(path);
									zc_set_config("zc.cfg", "ZCMODULE", "current_module", path);
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
									zc_set_config("zc.cfg", "SAVEFILE", "save_filename", path);
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
							text = zc_get_config("zc.cfg", "SAVEFILE", "save_filename", "zc.sav")
						),
						DummyWidget(),
						//
						CONFIG_DROPDOWN_I("Quickload Slot:", "zc.cfg","zeldadx","quickload_slot",0,quickSlotList,"Unless 'disabled', this save slot will be immediately loaded upon launching.")
					)
				)),
				TabRef(name = "ZQ Creator", Row(framed = true,
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX_I("Fullscreen","zquest.cfg","zquest","fullscreen",0,"Exactly stable."),
						CONFIG_CHECKBOX_I("Small Mode","zquest.cfg","zquest","small",0,"If enabled, the 'classic' small mode interface will be used. This mode has less screen space, and lacks features such as favorite combos, favorite commands, multiple combo rows, next-screen preview, etc."),
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
						CONFIG_CHECKBOX_I("Reload Last Quest","zquest.cfg","zquest","open_last_quest",1,"On launching, immediately attempt to open the last file edited."),
						CONFIG_CHECKBOX("Save Paths","zquest.cfg","zquest","save_paths",1),
						CONFIG_CHECKBOX_I("Show Misalignments","zquest.cfg","zquest","show_misalignments",0,"Shows blinking arrows on the sides of the screen where the solidity does not match across the screen border."),
						CONFIG_CHECKBOX_I("Show Ruleset Dialog on New Quest","zquest.cfg","zquest","rulesetdialog",1,"On creating a 'New' quest, automatically pop up the 'Pick Ruleset' menu. (This can be found any time at 'Quest->Options->Pick Ruleset')"),
						CONFIG_CHECKBOX("Tile Protection","zquest.cfg","zquest","tile_protection",1)
					),
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX("Uncompressed Autosaves","zquest.cfg","zquest","uncompressed_auto_saves",1),
						CONFIG_CHECKBOX_I("Static effect for invalid data","zquest.cfg","zquest","invalid_static",0,"Uses an animated static effect for 'invalid' things (filtered out combos, nonexistant screens on the minimap, etc)"),
						CONFIG_CHECKBOX_I("Warn on Init Script Change","zquest.cfg","zquest","warn_initscript_changes",1,"When compiling ZScript, receive a warning when the global init script changes (which may break existing save files for the quest)"),
						CONFIG_CHECKBOX_I("Monochrome Debuggers","zquest.cfg","CONSOLE","monochrome_debuggers",0,"Use non-colored debugger text."),
						CONFIG_CHECKBOX_I("Text Readability","zquest.cfg","gui","bolder_font",0,"Attempts to make text more readable in some areas (ex. larger, bolder)"),
						CONFIG_CHECKBOX_I("Disable Level Palette Shortcuts","zquest.cfg","zquest","dis_lpal_shortcut",1,"If enabled, keyboard shortcuts that change the screen's palette are disabled."),
						CONFIG_CHECKBOX_I("Autosave Window Size Changes","zquest.cfg","zquest","save_drag_resize",0,"Makes any changes to the window size by dragging get saved for whenever you open the program next."),
						CONFIG_CHECKBOX_I("Lock Aspect Ratio On Resize","zquest.cfg","zquest","drag_aspect",0,"Makes any changes to the window size by dragging get snapped to ZQuest's default (4:3) aspect ratio."),
						CONFIG_CHECKBOX_I("Save Window Position","zquest.cfg","zquest","save_window_position",0,"Remembers the last position of the ZQuest Window."),
						CONFIG_CHECKBOX_I("Force Integer Values for Scale","zquest.cfg","zquest","scaling_force_integer",0,"Locks the screen to only scale by an integer value. Results in perfect pixel art scaling, at the expense of not using the entire availabe window space."),
						CONFIG_CHECKBOX_I("Linear Scaling","zquest.cfg","zquest","scaling_mode",0,"Use linear scaling when upscaling the window. If off, the default is nearest-neighbor scaling"),
						CONFIG_CHECKBOX_I("Record During Test Feature","zquest.cfg","zquest","test_mode_record",0,"Save a recording to replays/test_XXXXXXXX.zplay when using the GUI test feature.")
					),
					Rows<3>(fitParent = true,
						CONFIG_TEXTFIELD_FL("Cursor Scale (small):", "zquest.cfg","zquest","cursor_scale_small",1.0,1.0,5.0, 4),
						CONFIG_TEXTFIELD_FL("Cursor Scale (large):", "zquest.cfg","zquest","cursor_scale_large",1.5,1.0,5.0, 4),
						CONFIG_DROPDOWN_I("Screenshot Output:", "zquest.cfg","zquest","snapshot_format",3,screenshotOutputList,"The output format of screenshots"),
						CONFIG_DROPDOWN_I("Auto-Backup Retention:", "zquest.cfg","zquest","auto_backup_retention",0,autoBackupCopiesList,"The number of auto-backups to keep"),
						CONFIG_DROPDOWN_I("Auto-Save Retention:", "zquest.cfg","zquest","auto_save_retention",9,autoSaveCopiesList,"The number of auto-saves to keep"),
						CONFIG_TEXTFIELD_I("Auto-Save Interval:", "zquest.cfg", "zquest", "auto_save_interval", 5, 0, 300, "Frequency of auto saves, in minutes. Valid range is 0-300, where '0' disables autosaves alltogether."),
						CONFIG_TEXTFIELD_I("Window Width (Large Mode):","zquest.cfg","zquest","large_window_width", def_large_w, 200, 3000, "The width of the ZQuest window in large mode"),
						CONFIG_TEXTFIELD_I("Window Height (Large Mode):","zquest.cfg","zquest","large_window_height", def_large_h, 150, 2250, "The height of the ZQuest window in large mode"),
						CONFIG_TEXTFIELD_I("Window Width (Small Mode):","zquest.cfg","zquest","small_window_width", def_small_w, 200, 3000, "The width of the ZQuest window in small mode"),
						CONFIG_TEXTFIELD_I("Window Height (Small Mode):","zquest.cfg","zquest","small_window_height", def_small_h, 150, 2250, "The height of the ZQuest window in small mode"),
						CONFIG_TEXTFIELD_I("Saved Window X:","zquest.cfg","zquest","window_x", 0, 0, rightmost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
						CONFIG_TEXTFIELD_I("Saved Window Y:","zquest.cfg","zquest","window_y", 0, 0, bottommost, "The top-left corner of the ZQuest Window, for manual positioning and also used by 'Save Window Position'. If 0, uses the default position."),
						GFXCARD_DROPDOWN("Graphics Driver:", "zquest.cfg", "graphics", "driver", 0, gfxDriverList),
						Button(hAlign = 1.0, forceFitH = true,
							text = "Browse Module", onPressFunc = [&]()
							{
								if(getname("Load Module [ZQ]", "zmod", NULL, zmodpath, false))
								{
									char path[4096] = {0};
									relativize_path(path, temppath);
									tf_module_zq->setText(path);
									zc_set_config("zquest.cfg", "ZCMODULE", "current_module", path);
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
							CONFIG_CHECKBOX_I_ZCL("Text Readability","gui","bolder_font",0,"Attempts to make text more readable in some areas (ex. larger, bolder)")
						),
						Rows<3>(fitParent = true,
							Button(hAlign = 1.0, forceFitH = true,
								text = "Browse Module", onPressFunc = [&]()
								{
									if(getname("Load Module [ZQ]", "zmod", NULL, zmodpath, false))
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
				TabRef(name = "Themes", Column(
					Label(text = "Here you can load themes, and save them for each program separately."),
					Rows<4>(padding = 0_px,
						Label(text = "Theme file path (.ztheme):"),
						tf_theme = TextField(read_only = true, maxLength = 255, text = zc_get_config("Theme", "theme_filename", "themes/dark.ztheme")),
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
								zc_set_config_standard();
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
								zc_set_config_standard();
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
								zc_set_config("Theme","theme_filename",themename.c_str());
								zc_set_config("ZLAUNCH","gui_colorset",99);
							}
							else
							{
								lbl_theme_error->setText("Theme load error!");
							}
						})
					),
					Button(text = "Theme Editor", onPressFunc = [&]()
						{
							ThemeEditor(theme_saved_filepath).show();
							queue_revert = 0;
							if(theme_saved_filepath[0])
							{
								tf_theme->setText(theme_saved_filepath);
								AlertFuncDialog("New Theme",
									"Set the new theme to a program?",
									4, 3, //4 buttons, where buttons[3] is focused
									"Save ZC", set_zc_theme,
									"Save ZQ", set_zq_theme,
									"Save ZCL", set_zcl_theme,
									"Done", NULL
								).show();
							}
						})
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
	
	char path[4096] = {0};
	relativize_path(path, zc_get_config("zc.cfg", "ZCMODULE", "current_module", "modules/classic.zmod"));
	tf_module_zc->setText(path);
	relativize_path(path, zc_get_config("zquest.cfg", "ZCMODULE", "current_module", "modules/classic.zmod"));
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
			launch_process(ZELDA_FILE);
			break;
		case message::ZQ:
			launch_process(ZQUEST_FILE);
			break;
		case message::EXIT:
			close_button_quit = true;
			return false;
	}
	return false;
}
