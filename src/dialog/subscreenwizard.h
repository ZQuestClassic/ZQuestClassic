#ifndef ZC_DIALOG_SUBSCREENWIZARD_H_
#define ZC_DIALOG_SUBSCREENWIZARD_H_

#include "base/gui.h"
#include "gui/dialog.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/checkbox.h"
#include "gui/text_field.h"
#include "gui/label.h"
#include "gui/switcher.h"
#include "gui/frame.h"
#include "gui/tabpanel.h"
#include "gui/drop_down_list.h"
#include "gui/radioset.h"
#include "zq/gui/seltile_swatch.h"
#include "zq/gui/misc_color_sel.h"
#include "tilepicker.h"

enum class subwizardtype : uint16_t {
	SW_ITEM_GRID,
	SW_COUNTER_BLOCK,
	SW_MAP_TILEBLOCK,
};
#define NUM_SUBWIZARD_TYPES 3

void call_subscreen_wizard(subwizardtype stype, int32_t& x, int32_t& y);

class SubscreenWizardDialog : public GUI::Dialog<SubscreenWizardDialog>
{
public:
	enum class message {
		REFR_INFO, OK, CANCEL, UPDATE, DEFAULT,
		RSET0, RSET1, RSET2, RSET3, RSET4, RSET5, RSET6, RSET7, RSET8, RSET9
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	subwizardtype wizard_type;
	size_t wizard_index;
	std::string thelp, tyname;

	int32_t &basex, &basey;
	int32_t flags;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tfs[10];
	std::shared_ptr<GUI::DropDownList> ddls[10];
	std::shared_ptr<GUI::Checkbox> cboxes[20];
	std::shared_ptr<GUI::Radio> rset[10][10];
	std::shared_ptr<GUI::SelTileSwatch> tswatches[10];
	std::shared_ptr<GUI::MiscColorSel> misccolorsel[3];

	GUI::ListData list_font, list_shadtype;

	void setRadio(size_t rs, size_t ind);
	size_t getRadio(size_t rs);

	bool finalize();
	
	void updateTitle();

	SubscreenWizardDialog(subwizardtype stype, int32_t& x, int32_t& y);
	friend void call_subscreen_wizard(subwizardtype stype, int32_t& x, int32_t& y);
};

#endif