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
#include "base/dmap.h"

void call_editdmap_dialog(int32_t slot);
void call_editdmap_dialog(size_t forceTab, int32_t slot);

class EditDMapDialog : public GUI::Dialog<EditDMapDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	EditDMapDialog(int32_t slot);

	bool disableEnhancedMusic(bool disableontracker = false);
	bool disableMusicTracks();
	void silenceMusicPreview();
	void musicPreview(bool previewloop = false);
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	void refreshGridSquares();
	void refreshDMapStrings();
	void refreshScripts();

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::DMapMinimap> dmap_mmap;
	std::shared_ptr<GUI::Slider> dmap_slider;
	std::shared_ptr<GUI::DMapMapGrid> dmap_grid;
	std::shared_ptr<GUI::TextField> compass_field;
	std::shared_ptr<GUI::TextField> continue_field;
	std::shared_ptr<GUI::TextField> tmusic_field;
	std::shared_ptr<GUI::DropDownList> tmusic_track_list;
	std::shared_ptr<GUI::TextField> tmusic_start_field;
	std::shared_ptr<GUI::TextField> tmusic_end_field;
	std::shared_ptr<GUI::TextField> tmusic_xfadein_field;
	std::shared_ptr<GUI::TextField> tmusic_xfadeout_field;
	std::shared_ptr<GUI::Label> tmusic_progress_lbl;
	std::shared_ptr<GUI::Button> tmusic_preview_btn;
	std::shared_ptr<GUI::Button> tmusic_previewloop_btn;
	std::shared_ptr<GUI::Button> tmusic_previewstop_btn;
	std::shared_ptr<GUI::List> disabled_list;
	std::shared_ptr<GUI::List> item_list;
	std::shared_ptr<GUI::Widget> DMAP_AC_INITD(int index);
	std::shared_ptr<GUI::Label> l_ac_initds[8];
	std::shared_ptr<GUI::Button> ib_ac_initds[8];
	std::shared_ptr<GUI::TextField> tf_ac_initds[8];
	std::string h_ac_initds[8];
	std::shared_ptr<GUI::Widget> DMAP_SS_INITD(int index);
	std::shared_ptr<GUI::Label> l_ss_initds[8];
	std::shared_ptr<GUI::Button> ib_ss_initds[8];
	std::shared_ptr<GUI::TextField> tf_ss_initds[8];
	std::string h_ss_initds[8];
	std::shared_ptr<GUI::Widget> DMAP_MAP_INITD(int index);
	std::shared_ptr<GUI::Label> l_map_initds[8];
	std::shared_ptr<GUI::Button> ib_map_initds[8];
	std::shared_ptr<GUI::TextField> tf_map_initds[8];
	std::shared_ptr<GUI::Frame> compass_frame;
	std::shared_ptr<GUI::Frame> continue_frame;
	std::string h_map_initds[8];
	std::shared_ptr<GUI::RegionGrid> region_grid;
	std::shared_ptr<GUI::Checkbox> region_checks[10];
	std::shared_ptr<GUI::TextField> grav_tf[2];

	std::shared_ptr<GUI::Switcher> string_switch;
	
	int32_t musicpreview_saved = 0;

	int32_t dmapslot;
	dmap* thedmap;
	dmap local_dmap;

	GUI::ListData list_maps, list_types;
	GUI::ListData list_activesub, list_passivesub, list_overlaysub,
		list_strings, list_lpals;
	GUI::ListData list_midis, list_tracks;
	GUI::ListData list_disableditems, list_items;
	GUI::ListData list_dmapscript;
};

#endif
