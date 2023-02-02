#include "options.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "zquest.h"
#include "../tiles.h"
#include "zq_misc.h"
#include "zc_list_data.h"

extern bool reload_fonts;
void load_size_poses();

void call_options_dlg()
{
	OptionsDialog().show();
}

extern int32_t UseSmall, EnableTooltips, GridColor, KeyboardRepeatDelay, KeyboardRepeatRate;

void OptionsDialog::loadOptions()
{
	opts[OPT_MOUSESCROLL] = MouseScroll ? 1 : 0;
	opts[OPT_SAVEPATHS] = SavePaths ? 1 : 0;
	opts[OPT_PALCYCLE] = CycleOn ? 1 : 0;
	opts[OPT_VSYNC] = Vsync ? 1 : 0;
	opts[OPT_FPS] = ShowFPS ? 1 : 0;
	opts[OPT_SAVEDRAGRESIZE] = SaveDragResize ? 1 : 0;
	opts[OPT_SAVEWINPOS] = SaveWinPos ? 1 : 0;
	opts[OPT_DRAGASPECT] = DragAspect ? 1 : 0;
	opts[OPT_COMB_BRUSH] = ComboBrush ? 1 : 0;
	opts[OPT_FLOAT_BRUSH] = FloatBrush ? 1 : 0;
	opts[OPT_RELOAD_QUEST] = OpenLastQuest ? 1 : 0;
	opts[OPT_MISALIGNS] = ShowMisalignments ? 1 : 0;
	opts[OPT_ANIM_COMBOS] = AnimationOn ? 1 : 0;
	opts[OPT_OW_PROT] = OverwriteProtection ? 1 : 0;
	opts[OPT_TILE_PROT] = TileProtection ? 1 : 0;
	opts[OPT_STATIC_INVAL] = InvalidStatic ? 1 : 0;
	opts[OPT_SMALLMODE] = UseSmall ? 1 : 0;
	opts[OPT_RULESET] = RulesetDialog ? 1 : 0;
	opts[OPT_TOOLTIPS] = EnableTooltips ? 1 : 0;
	opts[OPT_PATTERNSEARCH] = abc_patternmatch ? 1 : 0;
	opts[OPT_NEXTPREVIEW] = NoScreenPreview ? 1 : 0;
	opts[OPT_INITSCR_WARN] = WarnOnInitChanged ? 1 : 0;
	opts[OPT_ABRETENTION] = AutoBackupRetention;
	opts[OPT_ASINTERVAL] = AutoSaveInterval;
	opts[OPT_ASRETENTION] = AutoSaveRetention;
	opts[OPT_UNCOMP_AUTOSAVE] = UncompressedAutoSaves ? 1 : 0;
	opts[OPT_GRIDCOL] = GridColor;
	opts[OPT_SNAPFORMAT] = SnapshotFormat;
	opts[OPT_KBREPDEL] = KeyboardRepeatDelay;
	opts[OPT_KBREPRATE] = KeyboardRepeatRate;
	opts[OPT_CURS_LARGE] = int32_t(zc_get_config("zquest","cursor_scale_large",1.5)*10000);
	opts[OPT_CURS_SMALL] = int32_t(zc_get_config("zquest","cursor_scale_small",1)*10000);
	opts[OPT_COMPILE_OK] = zc_get_config("Compiler", "compile_success_sample", 20);
	opts[OPT_COMPILE_ERR] = zc_get_config("Compiler", "compile_error_sample", 28);
	opts[OPT_COMPILE_DONE] = zc_get_config("Compiler", "compile_finish_sample", 34);
	opts[OPT_COMPILE_VOL] = zc_get_config("Compiler", "compile_audio_volume", 100);
	opts[OPT_DISABLE_LPAL_SHORTCUT] = DisableLPalShortcuts;
	opts[OPT_DISABLE_COMPILE_CONSOLE] = DisableCompileConsole;
	opts[OPT_SKIP_LAYER_WARNING] = skipLayerWarning;
	opts[OPT_NUMERICAL_FLAG_LIST] = numericalFlags;
	opts[OPT_CUSTOMFONT] = zc_get_config("gui","custom_fonts",1);
	//cleanup
    reset_combo_animations();
    reset_combo_animations2();
    go();
}

void OptionsDialog::saveOptions()
{
	for(auto ind = 0; ind < OPT_MAX; ++ind)
	{
		if(opt_changed[ind])
			saveOption(ind);
	}
	if(opt_changed[OPT_CUSTOMFONT])
		reload_fonts = true;
	set_keyboard_rate(KeyboardRepeatDelay,KeyboardRepeatRate); //Reset keyboard rate
	load_mice(); //Reset cursor scale
}
void OptionsDialog::saveOption(int ind)
{
	auto v = opts[ind];
	switch(ind)
	{
		case OPT_SKIP_LAYER_WARNING:
			skipLayerWarning = v;
			zc_set_config("zquest", "skip_layer_warning", v);
			break;
		case OPT_MOUSESCROLL:
			MouseScroll = v;
			zc_set_config("zquest", "mouse_scroll", v);
			break;
		case OPT_DISABLE_LPAL_SHORTCUT:
			DisableLPalShortcuts = v;
			zc_set_config("zquest", "dis_lpal_shortcut", v);
			break;
		case OPT_DISABLE_COMPILE_CONSOLE:
			DisableCompileConsole = v;
			zc_set_config("zquest", "internal_compile_console", v);
			break;
		case OPT_INITSCR_WARN:
			WarnOnInitChanged = v;
			zc_set_config("zquest", "warn_initscript_changes", v);
			break;
		case OPT_STATIC_INVAL:
			InvalidStatic = v;
			zc_set_config("zquest", "invalid_static", v);
			break;
		case OPT_NUMERICAL_FLAG_LIST:
			numericalFlags = v;
			zc_set_config("zquest", "numerical_flags", v);
			break;
		case OPT_TILE_PROT:
			TileProtection = v;
			zc_set_config("zquest", "tile_protection", v);
			break;
		case OPT_GRIDCOL:
			GridColor = v;
			zc_set_config("zquest", "grid_color", v);
			break;
		case OPT_SNAPFORMAT:
			SnapshotFormat = v;
			zc_set_config("zquest", "snapshot_format", v);
			break;
		case OPT_RELOAD_QUEST:
			OpenLastQuest = v;
			zc_set_config("zquest", "open_last_quest", v);
			break;
		case OPT_MISALIGNS:
			ShowMisalignments = v;
			zc_set_config("zquest", "show_misalignments", v);
			break;
		case OPT_CURS_LARGE:
			zc_set_config("zquest", "cursor_scale_large", v / 10000.0);
			break;
		case OPT_CURS_SMALL:
			zc_set_config("zquest", "cursor_scale_small", v / 10000.0);
			break;
		case OPT_COMPILE_OK:
			zc_set_config("Compiler", "compile_success_sample", v);
			break;
		case OPT_COMPILE_ERR:
			zc_set_config("Compiler", "compile_error_sample", v);
			break;
		case OPT_COMPILE_DONE:
			zc_set_config("Compiler", "compile_finish_sample", v);
			break;
		case OPT_COMPILE_VOL:
			zc_set_config("Compiler", "compile_audio_volume", v);
			break;
		case OPT_NEXTPREVIEW:
			NoScreenPreview = v;
			zc_set_config("zquest","no_preview",v);
			break;
		case OPT_PATTERNSEARCH:
			abc_patternmatch = v;
			zc_set_config("zquest","lister_pattern_matching",v);
			break;
		case OPT_TOOLTIPS:
			EnableTooltips = v;
			zc_set_config("zquest","enable_tooltips",v);
			break;
		case OPT_RULESET:
			RulesetDialog = v;
			zc_set_config("zquest","rulesetdialog",v);
			break;
		case OPT_SMALLMODE:
			UseSmall = v;
			zc_set_config("zquest","small",v);
			break;
		case OPT_KBREPDEL:
			KeyboardRepeatDelay = v;
			zc_set_config("zquest","keyboard_repeat_delay",v);
			break;
		case OPT_KBREPRATE:
			KeyboardRepeatRate = v;
			zc_set_config("zquest","keyboard_repeat_rate",v);
			break;
		case OPT_ANIM_COMBOS:
			AnimationOn = v;
			zc_set_config("zquest","animation_on",v);
			break;
		case OPT_ABRETENTION:
			AutoBackupRetention = v;
			zc_set_config("zquest","auto_backup_retention",v);
			break;
		case OPT_ASINTERVAL:
			time(&auto_save_time_start); //Reset autosave timer
			AutoSaveInterval = v;
			zc_set_config("zquest","auto_save_interval",v);
			break;
		case OPT_ASRETENTION:
			AutoSaveRetention = v;
			zc_set_config("zquest","auto_save_retention",v);
			break;
		case OPT_UNCOMP_AUTOSAVE:
			UncompressedAutoSaves = v;
			zc_set_config("zquest","uncompressed_auto_saves",v);
			break;
		case OPT_OW_PROT:
			OverwriteProtection = v;
			zc_set_config("zquest","overwrite_prevention",v);
			break;
		case OPT_SAVEPATHS:
			SavePaths = v;
			zc_set_config("zquest","save_paths",v);
			break;
		case OPT_PALCYCLE:
			CycleOn = v;
			zc_set_config("zquest","cycle_on",v);
			break;
		case OPT_VSYNC:
			Vsync = v;
			zc_set_config("zquest","vsync",v);
			break;
		case OPT_FPS:
			ShowFPS = v;
			zc_set_config("zquest","showfps",v);
			break;
		case OPT_SAVEDRAGRESIZE:
			SaveDragResize = v;
			zc_set_config("zquest","save_drag_resize",v);
			break;
		case OPT_DRAGASPECT:
			DragAspect = v;
			zc_set_config("zquest","drag_aspect",v);
			break;
		case OPT_SAVEWINPOS:
			SaveWinPos = v;
			zc_set_config("zquest","save_window_position",v);
			break;
		case OPT_COMB_BRUSH:
			ComboBrush = v;
			zc_set_config("zquest","combo_brush",v);
			break;
		case OPT_FLOAT_BRUSH:
			FloatBrush = v;
			zc_set_config("zquest","float_brush",v);
			break;
		case OPT_CUSTOMFONT:
			zc_set_config("gui","custom_fonts",v);
			break;
	}
}

OptionsDialog::OptionsDialog() : sfx_list(GUI::ZCListData::sfxnames(true))
{
	loadOptions();
	memset(opt_changed, 0, sizeof(opt_changed));
}

//{ Macros
#define OPT_CHECK(optind, optlabel) \
Checkbox( \
	checked = opts[optind], \
	text = optlabel, \
	hAlign = 0.0, \
	vPadding = DEFAULT_PADDING/2, \
	onToggleFunc = [&](bool state) \
	{ \
		opts[optind] = state ? 1 : 0; \
		opt_changed[optind] = true; \
	} \
)

#define ROW_CHECK(optind, optlabel) \
Checkbox( \
	colSpan = 2, \
	checked = opts[optind], \
	text = optlabel, \
	hAlign = 0.0, \
	vPadding = DEFAULT_PADDING/2, \
	onToggleFunc = [&](bool state) \
	{ \
		opts[optind] = state ? 1 : 0; \
		opt_changed[optind] = true; \
	} \
)

#define ROW_TF_RANGE(optind, optlabel, minval, maxval) \
Label(text = optlabel, hAlign = 0.0), \
TextField(type = GUI::TextField::type::INT_DECIMAL, \
	fitParent = true, \
	hAlign = 1.0, low = minval, high = maxval, val = opts[optind], \
	minwidth = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	}) \

#define ROW_TF_FLOAT(optind, optlabel, minval, maxval) \
Label(text = optlabel, hAlign = 0.0), \
TextField(type = GUI::TextField::type::FIXED_DECIMAL, places = 4, \
	fitParent = true, maxLength = 8, \
	hAlign = 1.0, low = minval*10000, high = maxval*10000, val = opts[optind], \
	minwidth = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	}) \

#define ROW_DDOWN(optind, optlabel, lister) \
Label(text = optlabel, hAlign = 0.0), \
DropDownList( \
	fitParent = true, \
	maxwidth = sized(18_em, 14_em), \
	data = lister, \
	selectedValue = opts[optind], \
	onSelectFunc = [&](int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	} \
)

//}

//{ Listers
static const GUI::ListData abRetentionList {
	{ "Disabled", 0 },
	{ " 1", 1 },
	{ " 2", 2 },
	{ " 3", 3 },
	{ " 4", 4 },
	{ " 5", 5 },
	{ " 6", 6 },
	{ " 7", 7 },
	{ " 8", 8 },
	{ " 9", 9 },
	{ "10", 10 }
};
static const GUI::ListData asIntervalList {
	{ "Disabled", 0 },
	{ " 1 Minute", 1 },
	{ " 2 Minutes", 2 },
	{ " 3 Minutes", 3 },
	{ " 4 Minutes", 4 },
	{ " 5 Minutes", 5 },
	{ " 6 Minutes", 6 },
	{ " 7 Minutes", 7 },
	{ " 8 Minutes", 8 },
	{ " 9 Minutes", 9 },
	{ "10 Minutes", 10 }
};
static const GUI::ListData asRetentionList {
	{ " 1", 0 },
	{ " 2", 1 },
	{ " 3", 2 },
	{ " 4", 3 },
	{ " 5", 4 },
	{ " 6", 5 },
	{ " 7", 6 },
	{ " 8", 7 },
	{ " 9", 8 },
	{ "10", 9 }
};
static const GUI::ListData colorList {
	{ "Black", 0 },
	{ "Blue", 1 },
	{ "Green", 2 },
	{ "Cyan", 3 },
	{ "Red", 4 },
	{ "Magenta", 5 },
	{ "Brown", 6 },
	{ "Light Gray", 7 },
	{ "Dark Gray", 8 },
	{ "Light Blue", 9 },
	{ "Light Green", 10 },
	{ "Light Cyan", 11 },
	{ "Light Red", 12 },
	{ "Light Magenta", 13 },
	{ "Yellow", 14 },
	{ "White", 15 }
};
static const GUI::ListData snapFormatList {
	{ "BMP", 0 },
	{ "GIF", 1 },
	{ "JPG", 2 },
	{ "PNG", 3 },
	{ "PCX", 4 },
	{ "TGA", 5 }
};

//}

std::shared_ptr<GUI::Widget> OptionsDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "ZQuest Options",
		onClose = message::CANCEL,
		Column(
			TabPanel(
				minwidth = sized(120_px, 360_px),
				TabRef(name = "1", Column( hAlign = 0.0, vAlign = 0.0,
					OPT_CHECK(OPT_MOUSESCROLL, "Mouse Scroll"),
					OPT_CHECK(OPT_SAVEPATHS, "Save Paths"),
					OPT_CHECK(OPT_PALCYCLE, "Palette Cycle"),
					OPT_CHECK(OPT_VSYNC, "VSync"),
					OPT_CHECK(OPT_FPS, "Show FPS"),
					OPT_CHECK(OPT_COMB_BRUSH, "Combo Brush"),
					OPT_CHECK(OPT_FLOAT_BRUSH, "Floating Brush"),
					OPT_CHECK(OPT_RELOAD_QUEST, "Reload Last Quest"),
					OPT_CHECK(OPT_MISALIGNS, "Show Misaligns"),
					OPT_CHECK(OPT_ANIM_COMBOS, "Animate Combos"),
					OPT_CHECK(OPT_OW_PROT, "Overwrite Protection"),
					OPT_CHECK(OPT_TILE_PROT, "Tile Protection"),
					OPT_CHECK(OPT_STATIC_INVAL, "Use Static for Invalid Data"),
					OPT_CHECK(OPT_SMALLMODE, "Use Small Mode")
				)),
				TabRef(name = "2", Column( hAlign = 0.0, vAlign = 0.0,
					OPT_CHECK(OPT_RULESET, "Show Ruleset Dialog on New Quest"),
					OPT_CHECK(OPT_TOOLTIPS, "Enable Tooltips"),
					OPT_CHECK(OPT_PATTERNSEARCH, "Listers Use Pattern-Matching Search"),
					OPT_CHECK(OPT_NEXTPREVIEW, "No Next-Screen Preview"),
					OPT_CHECK(OPT_INITSCR_WARN, "Warn on ~Init Script Update"),
					OPT_CHECK(OPT_DISABLE_LPAL_SHORTCUT, "Disable Level Palette Shortcuts"),
					OPT_CHECK(OPT_DISABLE_COMPILE_CONSOLE, "Internal Compile Window"),
					OPT_CHECK(OPT_SKIP_LAYER_WARNING, "Skip Wrong Layer Flag Warning"),
					OPT_CHECK(OPT_NUMERICAL_FLAG_LIST, "Sort Flag List by Flag Number"),
					OPT_CHECK(OPT_SAVEDRAGRESIZE, "Autosave Window Size Changes"),
					OPT_CHECK(OPT_DRAGASPECT, "Lock Aspect Ratio"),
					OPT_CHECK(OPT_SAVEWINPOS, "Autosave Window Position"),
					OPT_CHECK(OPT_CUSTOMFONT, "Custom Fonts")
				)),
				TabRef(name = "3", Rows<2>(
					ROW_DDOWN(OPT_ABRETENTION, "Auto-backup Retention:", abRetentionList),
					ROW_DDOWN(OPT_ASINTERVAL, "Auto-save Interval:", asIntervalList),
					ROW_DDOWN(OPT_ASRETENTION, "Auto-save Retention:", asRetentionList),
					ROW_CHECK(OPT_UNCOMP_AUTOSAVE, "Uncompressed Auto Saves"),
					ROW_DDOWN(OPT_GRIDCOL, "Grid Color:", colorList),
					ROW_DDOWN(OPT_SNAPFORMAT, "Snapshot Format:", snapFormatList),
					ROW_TF_RANGE(OPT_KBREPDEL, "Keyboard Repeat Delay:", 0, 99999),
					ROW_TF_RANGE(OPT_KBREPRATE, "Keyboard Repeat Rate:", 0, 99999)
				)),
				TabRef(name = "4", Rows<2>(
					ROW_TF_FLOAT(OPT_CURS_LARGE, "Cursor Scale (Large Mode)", 1, 5),
					ROW_TF_FLOAT(OPT_CURS_SMALL, "Cursor Scale (Small Mode)", 1, 5),
					ROW_DDOWN(OPT_COMPILE_OK, "Compile SFX (OK):", sfx_list),
					ROW_DDOWN(OPT_COMPILE_ERR, "Compile SFX (Fail):", sfx_list),
					ROW_DDOWN(OPT_COMPILE_DONE, "Compile SFX (Slots):", sfx_list),
					ROW_TF_RANGE(OPT_COMPILE_VOL, "Compile SFX Volume %:", 0, 500)
				))
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
}

bool OptionsDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			saveOptions();
			[[fallthrough]];
		default:
			//cleanup
			setup_combo_animations();
			setup_combo_animations2();
			refresh(rALL);
			comeback();
			return true;
	}
}
