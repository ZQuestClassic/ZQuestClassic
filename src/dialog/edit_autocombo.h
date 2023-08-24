#ifndef ZC_DIALOG_AUTOCOMBO_H
#define ZC_DIALOG_AUTOCOMBO_H

#include "base/autocombo.h"
#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/checkbox.h>
#include <gui/grid.h>
#include <zq/gui/selcombo_swatch.h>
#include <zq/gui/engraving.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>

void call_autocombo_dlg(int32_t index);

struct autocombo_widg
{
	// std::shared_ptr<GUI::Button> xbtn;
	// std::shared_ptr<GUI::Button> left_btn;
	// std::shared_ptr<GUI::Button> right_btn;
	// std::shared_ptr<GUI::Button> plus_btn;
	// std::shared_ptr<GUI::Button> minus_btn;
	int32_t slot = -1;
	std::shared_ptr<GUI::SelComboSwatch> cpane;
	autocombo_entry* entry;
};

class AutoComboDialog: public GUI::Dialog<AutoComboDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, RELOAD
	};

	AutoComboDialog();

	void addCombos(int32_t engrave_offset, int32_t count, uint32_t dummyflags);
	void refreshPanels();

	std::shared_ptr<GUI::Widget> view() override;
	void refreshWidgets(std::shared_ptr<GUI::Grid> wingrid, std::shared_ptr<GUI::Grid> sgrid);
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::vector<autocombo_widg> widgs;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Button> pastebtn;

	std::shared_ptr<GUI::DropDownList> typedropdown;
	std::shared_ptr<GUI::SelComboSwatch> templatepane;

	GUI::ListData list_autocombotypes;
};

#endif
