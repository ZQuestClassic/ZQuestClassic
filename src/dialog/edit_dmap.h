#ifndef ZC_DIALOG_EDITDMAPDLG_H_
#define ZC_DIALOG_EDITDMAPDLG_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/slider.h>
#include <zq/gui/selcombo_swatch.h>
#include <zq/gui/dmap_minimap.h>
#include <zq/gui/dmap_mapgrid.h>
#include <zq/gui/regiongrid.h>
#include <gui/list.h>
#include <gui/frame.h>
#include <gui/drop_down_list.h>
#include <gui/label.h>
#include <gui/switcher.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <array>
#include "core/dmap.h"

void call_editdmap_dialog(int32_t slot);
void call_editdmap_dialog(size_t forceTab, int32_t slot);

class EditDMapDialog : public GUI::Dialog<EditDMapDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	EditDMapDialog(int32_t slot);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	void refreshGridSquares();
	void refreshDMapStrings();

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::DMapMinimap> dmap_mmap;
	std::shared_ptr<GUI::Slider> dmap_slider;
	std::shared_ptr<GUI::DMapMapGrid> dmap_grid;
	std::shared_ptr<GUI::TextField> compass_field;
	std::shared_ptr<GUI::TextField> continue_field;
	std::shared_ptr<GUI::List> disabled_list;
	std::shared_ptr<GUI::List> item_list;
	std::shared_ptr<GUI::Frame> compass_frame;
	std::shared_ptr<GUI::Frame> continue_frame;
	std::shared_ptr<GUI::RegionGrid> region_grid;
	std::shared_ptr<GUI::Checkbox> region_checks[10];
	std::shared_ptr<GUI::TextField> grav_tf[2];

	std::shared_ptr<GUI::Switcher> string_switch;

	int32_t dmapslot;
	dmap* thedmap;
	dmap local_dmap;

	GUI::ListData list_maps, list_types;
	GUI::ListData list_activesub, list_passivesub, list_overlaysub,
		list_mapsub, list_strings, list_lpals, list_music;
	GUI::ListData list_disabled_items, list_items;
	GUI::ListData list_dmapscript;
};

#endif
