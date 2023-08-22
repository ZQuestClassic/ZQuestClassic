#ifndef ZC_DIALOG_SCREENDATADLG_H
#define ZC_DIALOG_SCREENDATADLG_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <zq/gui/selcombo_swatch.h>
#include <gui/label.h>
#include <gui/switcher.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <array>
#include "base/mapscr.h"

void call_screendata_dialog(mapscr* scr = nullptr);
void call_screendata_dialog(size_t forceTab, mapscr* scr = nullptr);

class ScreenDataDialog: public GUI::Dialog<ScreenDataDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	ScreenDataDialog(mapscr* scr);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Widget> SCREEN_INITD(int index);
	void refreshScript();
	void refreshTWarp();
	void refreshLensEff();
	
	std::shared_ptr<GUI::Window> window;
	
	std::string h_initd[8];
	std::shared_ptr<GUI::TextField> tf_initd[8];
	std::shared_ptr<GUI::Button> ib_initds[8];
	std::shared_ptr<GUI::Label> l_initds[8];
	
	std::shared_ptr<GUI::Label> twarp_lbl;

	std::shared_ptr<GUI::Checkbox> carryover_cb[2][12];

	std::shared_ptr<GUI::Switcher> leff_switch;
	std::shared_ptr<GUI::Checkbox> lens_cb[2][7];
	
	mapscr* thescr;
	mapscr local_scr;
	GUI::ListData list_screenscript, list_maps, list_screens, list_sfx, list_screenmidi;
};

#endif
