#include "launcher_dialog.h"
#include "dialog/common.h"
#include "dialog/alert.h"
#include "dialog/alertfunc.h"
#include "dialog/theme_editor.h"
#include "launcher.h"
#include <gui/builder.h>
#include <boost/format.hpp>

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
			[&](bool ret)
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
				[&](bool ret)
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

	static const ListData scaleList
	{
		{ "1x", 1 },
		{ "2x", 2 },
		{ "3x", 3 },
		{ "4x", 4 }
	};

	static const ListData gfxCardList
	{
		{ "[WINDOWS]", -1 },
		{ "DirectDraw", 0 },
		{ "DirectDraw No Accel", 1 },
		{ "DirectDraw Safe", 2 },
		{ "DirectDraw Windowed", 3 },
		{ "DirectDraw Overlay", 4 },
		{ "GDI (Slow)", 5 },
		{ "[LINUX]", -1 },
		{ "SVGA", 6 },
		{ "ModeX", 7 },
		{ "VGA", 8 },
		{ "FBCon Device", 9 },
		{ "VBE/AF", 10 }
	};
	
	static const ListData dxglList
	{
		{ "Stretch", 0 },
		{ "Aspect Stretch", 1 },
		{ "Correct Ratio", 2 }
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
		{ "320x240", 1 },
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
//{ GFXCard
#define GFXCARD_DROPDOWN_MINWIDTH 7_em
#define GFXCARD_DROPDOWN(name, file, head, subhead, def, list) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = GFXCARD_DROPDOWN_MINWIDTH, \
	selectedValue = getGFXCardID(zc_get_config(file,head,subhead,getGFXCardStr(def))), \
	onSelectFunc = [&](int32_t val) \
	{ \
		if(val > -1) \
			zc_set_config(file,head,subhead,getGFXCardStr(val)); \
	} \
), \
DummyWidget()

#define GFXCARD_DROPDOWN_I(name, file, head, subhead, def, list, info) \
Label(text = name, hAlign = 1.0), \
DropDownList(data = list, \
	fitParent = true, \
	minwidth = GFXCARD_DROPDOWN_MINWIDTH, \
	selectedValue = getGFXCardID(zc_get_config(file,head,subhead,getGFXCardStr(def))), \
	onSelectFunc = [&](int32_t val) \
	{ \
		if(val > -1) \
			zc_set_config(file,head,subhead,getGFXCardStr(val)); \
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

//}

//{ DXGL
#define DXGL_ENABLER() \
dxglButtons.add(Button(hAlign = 1.0, forceFitH = true, \
	text = std::string(fileexists("ddraw.dll")?"Disable":"Enable")+" DXGL", \
	onPressFunc = [&]() \
	{ \
		if(fileexists("ddraw.dll")) \
		{ \
			if(!fileexists("dxgl.dll") && !rename("ddraw.dll","dxgl.dll")) \
			{ \
				dxglButtons.forEach([&](std::shared_ptr<GUI::Button>& btn){ \
					btn->setText("Enable DXGL"); \
				}); \
				zc_set_config("ag.cfg","dxgl","use_dxgl",0); \
				InfoDialog("DXGL", "DXGL Disabled").show(); \
			} \
			else \
			{ \
				InfoDialog("DXGL", "ERROR: Unknown?").show(); \
			} \
		} \
		else \
		{ \
			if(fileexists("dxgl.dll") && !rename("dxgl.dll","ddraw.dll")) \
			{ \
				dxglButtons.forEach([&](std::shared_ptr<GUI::Button>& btn){ \
					btn->setText("Disable DXGL"); \
				}); \
				zc_set_config("ag.cfg","dxgl","use_dxgl",1); \
				InfoDialog("DXGL", "DXGL Enabled").show(); \
			} \
			else \
			{ \
				InfoDialog("DXGL", "ERROR: DXGL Libraries are missing!"); \
			} \
		} \
	})), \
dxglDDLs.add(DropDownList(data = dxglList, \
	fitParent = true, \
	minwidth = CONFIG_DROPDOWN_MINWIDTH, \
	selectedValue = getDXGLID(zc_get_config("dxgl.cfg", "display","Include",getDXGLStr(0))), \
	onSelectFunc = [&](int32_t val) \
	{ \
		dxglDDLs.forEach([&](std::shared_ptr<GUI::DropDownList>& ddl){ \
			ddl->setSelectedValue(val); \
		}); \
		zc_set_config("dxgl.cfg", "display","Include",getDXGLStr(val)); \
	} \
)), \
Button(forceFitH = true, text = "?", \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info","DXGL settings are shared between ZC and ZQ.").show(); \
	})
//}

//}

//{ Helpers

//{ Graphics Card
#define CMP_GFX(str, val) \
if(!strcmp(str,configstr)) \
	return val

#define STR_GFX(str, val) \
case val: \
	strcpy(gfx_card_buf, str); \
	break

int32_t getGFXCardID(char const* configstr)
{
	CMP_GFX("DXAC", 0);
	CMP_GFX("DXSO", 1);
	CMP_GFX("DXSA", 2);
	CMP_GFX("DXWN", 3);
	CMP_GFX("DXOV", 4);
	CMP_GFX("GDIB", 5);
	CMP_GFX("SVGA", 6);
	CMP_GFX("MODX", 7);
	CMP_GFX("VGA", 8);
	CMP_GFX("FB", 9);
	CMP_GFX("VBAF", 10);
	return 0;
}

static char gfx_card_buf[32] = {0};
char const* getGFXCardStr(int32_t id)
{
	switch(id)
	{
		STR_GFX("DXAC", 0);
		STR_GFX("DXSO", 1);
		STR_GFX("DXSA", 2);
		STR_GFX("DXWN", 3);
		STR_GFX("DXOV", 4);
		STR_GFX("GDIB", 5);
		STR_GFX("SVGA", 6);
		STR_GFX("MODX", 7);
		STR_GFX("VGA", 8);
		STR_GFX("FB", 9);
		STR_GFX("VBAF", 10);
		default: gfx_card_buf[0]=0; break;
	}
	return gfx_card_buf;
}

int32_t getDXGLID(char const* configstr)
{
	CMP_GFX("dxgl-fullscreen.cfg",0);
	CMP_GFX("dxgl-aspect.cfg",1);
	CMP_GFX("dxgl-aspect-zc-43-scale.cfg",2);
	return 0;
}

char const* getDXGLStr(int32_t id)
{
	switch(id)
	{
		STR_GFX("dxgl-fullscreen.cfg",0);
		STR_GFX("dxgl-aspect.cfg",1);
		STR_GFX("dxgl-aspect-zc-43-scale.cfg",2);
		default: gfx_card_buf[0]=0; break;
	}
	return gfx_card_buf;
}
//}

//{ Resolution

int32_t getResPreset(int32_t resx, int32_t resy)
{
	double mod_x = resx/320.0,
		mod_y = resy/240.0;
	if(mod_x > 5 || mod_y > 5) return 5;
	if(mod_x < 1 || mod_y < 1) return 1;
	double left_x = mod_x - floor(mod_x),
		left_y = mod_y - floor(mod_y);
	double avg_leftovers = (left_x+left_y)/2;
	int32_t round_x = (left_x >= 0.5 ? ceil(mod_x) : floor(mod_x)),
		round_y = (left_y >= 0.5 ? ceil(mod_y) : floor(mod_y));
	if(round_x != round_y)
		return (avg_leftovers >= 0.5 ? zc_max(round_x, round_y) : zc_min(round_x, round_y));
	return round_x;
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
						CONFIG_CHECKBOX_I("Monochrome Debuggers","zc.cfg","CONSOLE","monochrome_debuggers",0,"Use non-colored debugger text."),
						CONFIG_CHECKBOX_I("Text Readability","zc.cfg","gui","bolder_font",0,"Attempts to make text more readable in some areas (ex. larger, bolder)")
					),
					Rows<3>(fitParent = true,
						CONFIG_TEXTFIELD_FL("Cursor Scale (small):", "zc.cfg","zeldadx","cursor_scale_small",1.0,1.0,5.0, 4),
						CONFIG_TEXTFIELD_FL("Cursor Scale (large):", "zc.cfg","zeldadx","cursor_scale_large",1.5,1.0,5.0, 4),
						CONFIG_DROPDOWN_I("Frame Rest Suggest:", "zc.cfg","zeldadx","frame_rest_suggest",0,frameRestSuggestList,"Adjusts vsync to attempt to reduce lag. What value works best depends on your hardware / OS.\nPressing '[' and ']' during gameplay will lower/raise this value."),
						CONFIG_DROPDOWN_I("Screenshot Output:", "zc.cfg","zeldadx","snapshot_format",3,screenshotOutputList,"The output format of screenshots"),
						CONFIG_DROPDOWN_I("Name Entry Mode:", "zc.cfg","zeldadx","name_entry_mode",0,nameEntryList,"The entry method of save file names."),
						CONFIG_DROPDOWN_I("Title Screen:", "zc.cfg","zeldadx","title",0,titleScreenList,"Which title screen will be displayed."),
						GFXCARD_DROPDOWN("Graphics Driver (Windowed):", "zc.cfg", "graphics", "gfx_cardw", 0, gfxCardList),
						GFXCARD_DROPDOWN("Graphics Driver (Fullscreen):", "zc.cfg", "graphics", "gfx_card", 0, gfxCardList),
						DXGL_ENABLER(),
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
						Label(text = "Resolution:", hAlign = 1.0),
						ddl_res = DropDownList(data = resPresetList,
							fitParent = true,
							minwidth = CONFIG_DROPDOWN_MINWIDTH,
							selectedValue = getResPreset(zc_get_config("zc.cfg","zeldadx","resx",640), zc_get_config("zc.cfg","zeldadx","resx",480)),
							onSelectFunc = [&](int32_t val)
							{
								zc_set_config("zc.cfg","zeldadx","resx",320*val);
								zc_set_config("zc.cfg","zeldadx","resy",240*val);
							}
						),
						DummyWidget(),
						//
						CONFIG_DROPDOWN_I("Quickload Slot:", "zc.cfg","zeldadx","quickload_slot",0,quickSlotList,"Unless 'disabled', this save slot will be immediately loaded upon launching.")
					)
				)),
				TabRef(name = "ZQ Creator", Row(framed = true,
					Rows<2>(fitParent = true,
						CONFIG_CHECKBOX_I("Fullscreen","zquest.cfg","zquest","fullscreen",0,"Not exactly 'stable'."),
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
						CONFIG_CHECKBOX("Tile Protection","zquest.cfg","zquest","tile_protection",1),
						CONFIG_CHECKBOX("Uncompressed Autosaves","zquest.cfg","zquest","uncompressed_auto_saves",1),
						CONFIG_CHECKBOX_I("Static effect for invalid data","zquest.cfg","zquest","invalid_static",0,"Uses an animated static effect for 'invalid' things (filtered out combos, nonexistant screens on the minimap, etc)"),
						CONFIG_CHECKBOX_I("Warn on Init Script Change","zquest.cfg","zquest","warn_initscript_changes",1,"When compiling ZScript, receive a warning when the global init script changes (which may break existing save files for the quest)"),
						CONFIG_CHECKBOX_I("Monochrome Debuggers","zquest.cfg","CONSOLE","monochrome_debuggers",0,"Use non-colored debugger text."),
						CONFIG_CHECKBOX_I("Text Readability","zquest.cfg","gui","bolder_font",0,"Attempts to make text more readable in some areas (ex. larger, bolder)"),
						CONFIG_CHECKBOX_I("Disable Level Palette Shortcuts","zquest.cfg","zquest","dis_lpal_shortcut",0,"If enabled, keyboard shortcuts that change the screen's palette are disabled.")
					),
					Rows<3>(fitParent = true,
						CONFIG_TEXTFIELD_FL("Cursor Scale (small):", "zquest.cfg","zquest","cursor_scale_small",1.0,1.0,5.0, 4),
						CONFIG_TEXTFIELD_FL("Cursor Scale (large):", "zquest.cfg","zquest","cursor_scale_large",1.5,1.0,5.0, 4),
						CONFIG_DROPDOWN_I("Screenshot Output:", "zquest.cfg","zquest","snapshot_format",3,screenshotOutputList,"The output format of screenshots"),
						CONFIG_DROPDOWN_I("Auto-Backup Retention:", "zquest.cfg","zquest","auto_backup_retention",0,autoBackupCopiesList,"The number of auto-backups to keep"),
						CONFIG_DROPDOWN_I("Auto-Save Retention:", "zquest.cfg","zquest","auto_save_retention",9,autoSaveCopiesList,"The number of auto-saves to keep"),
						CONFIG_TEXTFIELD_I("Auto-Save Interval:", "zquest.cfg", "zquest", "auto_save_interval", 5, 0, 300, "Frequency of auto saves, in minutes. Valid range is 0-300, where '0' disables autosaves alltogether."),
						CONFIG_DROPDOWN_I("Scale (Small Mode):", "zquest.cfg","zquest","scale",3,scaleList,"The scale multiplier for the default small mode resolution (320x240). If this scales larger than your monitor resolution, ZQ will fail to launch."),
						CONFIG_DROPDOWN_I("Scale (Large Mode):", "zquest.cfg","zquest","scale_large",1,scaleList,"The scale multiplier for the default large mode resolution (800x600). If this scales larger than your monitor resolution, ZQ will fail to launch."),
						GFXCARD_DROPDOWN("Graphics Driver (Windowed):", "zquest.cfg", "graphics", "gfx_cardw", 0, gfxCardList),
						GFXCARD_DROPDOWN("Graphics Driver (Fullscreen):", "zquest.cfg", "graphics", "gfx_card", 0, gfxCardList),
						DXGL_ENABLER(),
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
