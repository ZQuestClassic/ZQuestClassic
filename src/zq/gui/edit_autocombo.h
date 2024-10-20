#ifndef ZQ_DIALOG_AUTOCOMBO_H_
#define ZQ_DIALOG_AUTOCOMBO_H_

#include "base/autocombo.h"
#include "gui/dialog.h"
#include "gui/drop_down_list.h"
#include "gui/checkbox.h"
#include "gui/grid.h"
#include "zq/gui/selcombo_swatch.h"
#include "zq/gui/engraving.h"
#include "gui/label.h"
#include "gui/button.h"
#include "gui/text_field.h"
#include "gui/switcher.h"
#include "gui/window.h"
#include <functional>

void call_autocombo_dlg(int32_t index);

struct autocombo_widg
{
	int32_t slot = -1;
	std::shared_ptr<GUI::SelComboSwatch> cpane;
	std::shared_ptr<GUI::SelComboSwatch> cpane_replace;
	autocombo_entry* entry;
	autocombo_entry* entry_replace;
};

class AutoComboDialog: public GUI::Dialog<AutoComboDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, RELOAD
	};

	AutoComboDialog();

	void addCombos(int32_t count, bool from0=false, int32_t engraveoff=-1);
	void removeCombos(int32_t count);
	int32_t numCombosSet();
	void refreshPanels();

	std::shared_ptr<GUI::Widget> view() override;
	void addSlot(autocombo_entry& entry, size_t& ind, size_t& wid, size_t& hei, int32_t engraveoff=-1);
	void addSlotReplace(autocombo_entry& entrybefore, autocombo_entry& entryafter, size_t& grid_ind, size_t& ind, size_t& wid, size_t& hei);
	void addSlotsExtend(std::vector<autocombo_entry>& entries, size_t& ind, size_t& wid, size_t& hei);
	void addSlotNoEngrave(autocombo_entry& entry, size_t& ind, size_t& wid, size_t& hei);
	void refreshPreviewCSets();
	void refreshTypes(int32_t type);
	void refreshTilingGrid(byte oldw, byte oldh);
	void refreshWidgets();
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	byte temp_arg;

	std::string typeinfostr;

	std::vector<autocombo_widg> widgs;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Grid> wingrid, sgrid;
	//std::shared_ptr<GUI::Button> pastebtn;

	std::shared_ptr<GUI::DropDownList> typedropdown;
	std::shared_ptr<GUI::Button> typeinfobtn;
	std::shared_ptr<GUI::SelComboSwatch> iconpane;
	std::shared_ptr<GUI::SelComboSwatch> erasepane;
	std::shared_ptr<GUI::Button> templatebtn;
	std::shared_ptr<GUI::Switcher> switch_settings;
	std::shared_ptr<GUI::SelComboSwatch> tiling_grid[64];
	std::shared_ptr<GUI::Button> pastebtn;

	GUI::ListData list_autocombotypes;
};

#endif
