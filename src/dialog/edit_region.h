#ifndef ZC_DIALOG_EDIT_REGION_DLG_H_
#define ZC_DIALOG_EDIT_REGION_DLG_H_

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
#include "base/mapscr.h"

void call_edit_map_settings(int32_t slot);

class EditMapSettingsDialog : public GUI::Dialog<EditMapSettingsDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, QRS };

	EditMapSettingsDialog(int32_t slot);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	void refreshRegionGrid();

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::RegionGrid> region_grid;
	std::shared_ptr<GUI::Checkbox> region_checks[10];

	int32_t mapslot;
	bool region_valid;
	map_info local_info;
	word autolayers[6];
	word palette;
	regions_data* the_regions_data;
	regions_data local_regions_data;
	
	GUI::ListData list_lpals;
};

#endif
