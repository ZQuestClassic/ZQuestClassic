#ifndef ZC_DIALOG_SCREENSCRIPTDLG_H
#define ZC_DIALOG_SCREENSCRIPTDLG_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <zq/gui/selcombo_swatch.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <array>
#include "base/mapscr.h"

void call_screenscript_dialog(mapscr* scr = nullptr);

class ScreenScriptDialog: public GUI::Dialog<ScreenScriptDialog>
{
public:
	enum class message { OK, CANCEL };

	ScreenScriptDialog(mapscr* scr);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Widget> SCREEN_INITD(int index);
	void refreshScript();
	
	std::shared_ptr<GUI::Window> window;
	
	std::string h_initd[8];
	std::shared_ptr<GUI::TextField> tf_initd[8];
	std::shared_ptr<GUI::Button> ib_initds[8];
	std::shared_ptr<GUI::Label> l_initds[8];
	
	mapscr* thescr;
	mapscr local_scr;
	GUI::ListData list_screenscript;
};

#endif
