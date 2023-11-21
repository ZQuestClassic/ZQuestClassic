#ifndef ZC_DIALOG_PLAYER_OPTIONS_H
#define ZC_DIALOG_PLAYER_OPTIONS_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <functional>
#include <string_view>

void call_zc_options_dlg();

class ZCOptionsDialog: public GUI::Dialog<ZCOptionsDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, RELOAD };

	ZCOptionsDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	enum optionVal
	{
		OPT_FIRSTFONT,
		OPT_FONT_DIALOG = OPT_FIRSTFONT, OPT_FONT_GUI,
		OPT_FONT_TITLE, OPT_FONT_FAVCMD,
		OPT_FONT_TEXTBOX, OPT_FONT_TTIP,
		OPT_FONT_INFO,
		
		OPT_LASTFONT = OPT_FONT_INFO,
		OPT_FIRST_FONTSCALE,
		
		OPT_FONT_SCALE_DIALOG = OPT_FIRST_FONTSCALE, OPT_FONT_SCALE_GUI,
		OPT_FONT_SCALE_TITLE, OPT_FONT_SCALE_FAVCMD,
		OPT_FONT_SCALE_TEXTBOX, OPT_FONT_SCALE_TTIP,
		OPT_FONT_SCALE_INFO,
		
		OPT_LAST_FONTSCALE = OPT_FONT_SCALE_INFO,
		
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
