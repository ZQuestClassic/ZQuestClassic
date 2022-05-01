#include "options.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"
#include "../tiles.h"
#include "../zq_misc.h"

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
	//cleanup
    reset_combo_animations();
    reset_combo_animations2();
    go();
}

void OptionsDialog::saveOptions()
{
	MouseScroll = opts[OPT_MOUSESCROLL];
	SavePaths = opts[OPT_SAVEPATHS];
	CycleOn = opts[OPT_PALCYCLE];
	Vsync = opts[OPT_VSYNC];
	ShowFPS = opts[OPT_FPS];
	ComboBrush = opts[OPT_COMB_BRUSH];
	FloatBrush = opts[OPT_FLOAT_BRUSH];
	OpenLastQuest = opts[OPT_RELOAD_QUEST];
	ShowMisalignments = opts[OPT_MISALIGNS];
	AnimationOn = opts[OPT_ANIM_COMBOS];
	OverwriteProtection = opts[OPT_OW_PROT];
	TileProtection = opts[OPT_TILE_PROT];
	InvalidStatic = opts[OPT_STATIC_INVAL];
	UseSmall = opts[OPT_SMALLMODE];
	RulesetDialog = opts[OPT_RULESET];
	EnableTooltips = opts[OPT_TOOLTIPS];
	abc_patternmatch = opts[OPT_PATTERNSEARCH];
	NoScreenPreview = opts[OPT_NEXTPREVIEW];
	WarnOnInitChanged = opts[OPT_INITSCR_WARN];
	AutoBackupRetention = opts[OPT_ABRETENTION];
	if(AutoSaveInterval != opts[OPT_ASINTERVAL])
		time(&auto_save_time_start); //Reset autosave timer
	AutoSaveInterval = opts[OPT_ASINTERVAL];
	AutoSaveRetention = opts[OPT_ASRETENTION];
	UncompressedAutoSaves = opts[OPT_UNCOMP_AUTOSAVE];
	GridColor = opts[OPT_GRIDCOL];
	SnapshotFormat = opts[OPT_SNAPFORMAT];
	KeyboardRepeatDelay = opts[OPT_KBREPDEL];
	KeyboardRepeatRate = opts[OPT_KBREPRATE];
	KeyboardRepeatRate = opts[OPT_KBREPRATE];
	zc_set_config("zquest", "cursor_scale_large", opts[OPT_CURS_LARGE] / 10000.0);
	zc_set_config("zquest", "cursor_scale_small", opts[OPT_CURS_SMALL] / 10000.0);
	zc_set_config("Compiler", "compile_success_sample", opts[OPT_COMPILE_OK]);
	zc_set_config("Compiler", "compile_error_sample", opts[OPT_COMPILE_ERR]);
	zc_set_config("Compiler", "compile_finish_sample", opts[OPT_COMPILE_DONE]);
	zc_set_config("Compiler", "compile_audio_volume", opts[OPT_COMPILE_VOL]);
	DisableLPalShortcuts = opts[OPT_DISABLE_LPAL_SHORTCUT];
	skipLayerWarning = opts[OPT_SKIP_LAYER_WARNING];
	numericalFlags = opts[OPT_NUMERICAL_FLAG_LIST];
	DisableCompileConsole = opts[OPT_DISABLE_COMPILE_CONSOLE];
	load_mice(); //Reset cursor scale
	set_keyboard_rate(KeyboardRepeatDelay,KeyboardRepeatRate); //Reset keyboard rate
}

OptionsDialog::OptionsDialog() : sfx_list(GUI::ListData::sfxnames(true))
{
	loadOptions();
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
		onEnter = message::OK,
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
					OPT_CHECK(OPT_NUMERICAL_FLAG_LIST, "Sort Flag List by Flag Number")
				)),
				TabRef(name = "3", Rows<2>(
					ROW_DDOWN(OPT_ABRETENTION, "Auto-backup Retention:", abRetentionList),
					ROW_DDOWN(OPT_ASINTERVAL, "Auto-save Interval:", asIntervalList),
					ROW_DDOWN(OPT_ASRETENTION, "Auto-save Retention:", asRetentionList),
					ROW_CHECK(OPT_UNCOMP_AUTOSAVE, "Uncompressed Auto Saves"),
					ROW_DDOWN(OPT_GRIDCOL, "Grid Color:", color_list),
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
			save_config_file();
			setup_combo_animations();
			setup_combo_animations2();
			refresh(rALL);
			comeback();
			return true;
	}
}
