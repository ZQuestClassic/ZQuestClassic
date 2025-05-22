#ifndef ZC_DIALOG_SUBSCR_PROPS_H_
#define ZC_DIALOG_SUBSCR_PROPS_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/label.h>
#include <gui/frame.h>
#include <gui/button.h>
#include <gui/list.h>
#include <gui/tabpanel.h>
#include <zq/gui/misc_color_sel.h>
#include <zq/gui/misc_cset_sel.h>
#include <zq/gui/seltile_swatch.h>
#include <initializer_list>
#include <string>
#include <string_view>
#include "subscr.h"

bool call_subscrprop_dialog(SubscrWidget* widg, int32_t obj_ind);

class SubscrPropDialog: public GUI::Dialog<SubscrPropDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, REFR_SELECTABLE
	};

	SubscrPropDialog(SubscrWidget* widg, int32_t obj_ind);
	
	std::shared_ptr<GUI::Widget> GEN_INITD(int ind);
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

protected:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::MiscColorSel> col_sel[6];
	std::shared_ptr<GUI::MiscCSetSel> cs_sel[3];
	std::shared_ptr<GUI::SelTileSwatch> tswatches[2];
	std::shared_ptr<GUI::DropDownList> ddl;
	std::shared_ptr<GUI::TextField> fonttf;
	std::shared_ptr<GUI::Label> labels[5];
	std::shared_ptr<GUI::TextField> tfs[3];
	std::shared_ptr<GUI::Checkbox> cbs[4];
	
	std::shared_ptr<GUI::Grid> selgs[4];
	std::shared_ptr<GUI::Frame> selframes[3];
	std::shared_ptr<GUI::TabRef> seltabs[1];
	std::shared_ptr<GUI::TextField> seltfs[1];
	std::shared_ptr<GUI::Button> selbtns[1];
	
	std::shared_ptr<GUI::List> req_item_list, req_not_item_list;
	
	std::shared_ptr<GUI::Label> geninitd_lbl[8];
	std::shared_ptr<GUI::Button> geninitd_btn[8];
	
	std::shared_ptr<GUI::Checkbox> def_eqp_cboxes[4];
	
	std::shared_ptr<GUI::SelTileSwatch> gauge_tswatches[4];
	std::shared_ptr<GUI::Widget> gauge_gw[8];
	SubscrWidget* subref;
	SubscrWidget* local_subref;
	int32_t index;
	byte set_default_btnslot;
	byte start_default_btnslot;
	
	zasm_meta local_gen_meta;
	
	GUI::ListData list_font, list_shadtype, list_aligns, list_buttons, list_items,
		list_counters, list_counters2, list_itemclass, list_genscr, list_sfx,
		list_costinds, list_items_no_none, list_reqitems, list_reqnotitems;
	
	void updateSelectable();
	void updateAttr();
	void updateColors();
	void updateConditions();
	void update_wh();
	void refr_info();
};

#endif
