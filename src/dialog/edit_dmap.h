#ifndef ZC_DIALOG_EDITDMAPDLG_H
#define ZC_DIALOG_EDITDMAPDLG_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <zq/gui/selcombo_swatch.h>
#include <zq/gui/dmap_minimap.h>
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

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::DMapMinimap> dmap_mmap;
	
	int32_t dmapslot;
	dmap* thedmap;
	dmap local_dmap;

	GUI::ListData list_maps, list_types;
};

#endif
