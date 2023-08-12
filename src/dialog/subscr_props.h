#ifndef ZC_DIALOG_SUBSCR_PROPS_H
#define ZC_DIALOG_SUBSCR_PROPS_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/label.h>
#include <zq/gui/misc_color_sel.h>
#include <zq/gui/misc_cset_sel.h>
#include <zq/gui/seltile_swatch.h>
#include <initializer_list>
#include <string>
#include <string_view>
#include "subscr.h"

bool call_subscrprop_dialog(subscreen_object *ref, int32_t obj_ind);

// A basic dialog that just shows some lines of text and a close button.
class SubscrPropDialog: public GUI::Dialog<SubscrPropDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	SubscrPropDialog(subscreen_object *ref, int32_t obj_ind);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

protected:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::MiscColorSel> col_sel[3];
	std::shared_ptr<GUI::MiscCSetSel> cs_sel[3];
	std::shared_ptr<GUI::SelTileSwatch> tswatches[2];
	std::shared_ptr<GUI::DropDownList> ddl;
	std::shared_ptr<GUI::TextField> fonttf;
	std::shared_ptr<GUI::Label> labels[2];
	subscreen_object *subref;
	subscreen_object local_subref;
	int32_t index;
	
	GUI::ListData list_font, list_shadtype, list_aligns, list_buttons, list_items,
		list_counters, list_itemclass;
	
	void updateColors();
	void update_wh();
};

#endif
