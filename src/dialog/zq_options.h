#ifndef ZC_DIALOG_EDITOR_OPTIONS_H_
#define ZC_DIALOG_EDITOR_OPTIONS_H_

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <functional>
#include <string_view>

void call_options_dlg();

class OptionsDialog: public GUI::Dialog<OptionsDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, RELOAD };

	OptionsDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	enum optionVal
	{
		OPT_MOUSESCROLL, OPT_SAVEPATHS, OPT_PALCYCLE, OPT_FPS, OPT_COMB_BRUSH,
		OPT_FLOAT_BRUSH, OPT_RELOAD_QUEST, OPT_MISALIGNS, OPT_ANIM_COMBOS, OPT_OW_PROT,
		OPT_TILE_PROT, OPT_COMBO_PROT, OPT_STATIC_INVAL, OPT_RULESET, OPT_TOOLTIPS,
		OPT_TOOLTIP_HIGHLIGHT, OPT_PATTERNSEARCH, OPT_NEXTPREVIEW,
		OPT_TOOLTIP_TIMER, OPT_MAPCURSOR,
		//
		OPT_ABRETENTION, OPT_ASINTERVAL, OPT_ASRETENTION, OPT_UNCOMP_AUTOSAVE, OPT_GRIDCOL,
		OPT_SNAPFORMAT, OPT_SNAPSCALE, OPT_KBREPDEL, OPT_KBREPRATE, OPT_CMB_CURS_COL,
		OPT_TPG_CURS_COL, OPT_CPG_CURS_COL, OPT_TTIP_HL_COL, OPT_TTIP_CHECKER1_COL,
		OPT_TTIP_CHECKER2_COL,
		//
		OPT_LYR_DITH_BG, OPT_LYR_DITH_SZ,
		//
		OPT_INVALID_BG,
		//
		OPT_CURS_LARGE,
		OPT_COMPILE_OK, OPT_COMPILE_ERR, OPT_COMPILE_DONE, OPT_COMPILE_VOL,
		//
		OPT_DISABLE_LPAL_SHORTCUT, OPT_DISABLE_COMPILE_CONSOLE, OPT_SKIP_LAYER_WARNING,
		OPT_NUMERICAL_FLAG_LIST, OPT_SAVEDRAGRESIZE, OPT_DRAGASPECT,
		OPT_SAVEWINPOS, OPT_CUSTOMFONT, OPT_BOTTOM8, OPT_INFO_BG,
		OPT_HIDEMOUSE, OPT_COMPILEONSAVE, OPT_SHOW_FAV_COMBO_MODES, OPT_NO_HIGHLIGHT_LAYER0,
		OPT_DRAG_CENTER_OF_SQUARES,
		
		OPT_FIRSTFONT,
		OPT_LARGEFONT_DIALOG = OPT_FIRSTFONT, OPT_LARGEFONT_GUI,
		OPT_LARGEFONT_TITLE, OPT_LARGEFONT_FAVCMD,
		OPT_LARGEFONT_TEXTBOX, OPT_LARGEFONT_TTIP,
		OPT_LARGEFONT_INFO,
		
		OPT_FIRST_COMPACTFONT,
		OPT_COMPACTFONT_DIALOG = OPT_FIRST_COMPACTFONT, OPT_COMPACTFONT_GUI,
		OPT_COMPACTFONT_TITLE, OPT_COMPACTFONT_FAVCMD,
		OPT_COMPACTFONT_TEXTBOX, OPT_COMPACTFONT_TTIP,
		OPT_COMPACTFONT_INFO,
		
		OPT_LASTFONT = OPT_COMPACTFONT_INFO,
		OPT_FIRST_FONTSCALE,
		
		OPT_LARGEFONT_SCALE_DIALOG = OPT_FIRST_FONTSCALE, OPT_LARGEFONT_SCALE_GUI,
		OPT_LARGEFONT_SCALE_TITLE, OPT_LARGEFONT_SCALE_FAVCMD,
		OPT_LARGEFONT_SCALE_TEXTBOX, OPT_LARGEFONT_SCALE_TTIP,
		OPT_LARGEFONT_SCALE_INFO,
		
		OPT_FIRST_COMPACTFONT_SCALE,
		OPT_COMPACTFONT_SCALE_DIALOG = OPT_FIRST_COMPACTFONT_SCALE, OPT_COMPACTFONT_SCALE_GUI,
		OPT_COMPACTFONT_SCALE_TITLE, OPT_COMPACTFONT_SCALE_FAVCMD,
		OPT_COMPACTFONT_SCALE_TEXTBOX, OPT_COMPACTFONT_SCALE_TTIP,
		OPT_COMPACTFONT_SCALE_INFO,
		
		OPT_LAST_FONTSCALE = OPT_COMPACTFONT_SCALE_INFO,
		
		//
		OPT_MAX
	};
	
	int32_t opts[OPT_MAX];
	bool opt_changed[OPT_MAX];
	GUI::ListData sfx_list;
	std::shared_ptr<GUI::Label> fprev, fprev_lab;
	
	void preview_font();
	void preview_font(int fontind, int scale);
	void loadOptions();
	void saveOptions();
	void saveOption(int ind);
};

#endif
