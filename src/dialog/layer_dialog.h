#ifndef ZC_DIALOG_LAYERDLG_H_
#define ZC_DIALOG_LAYERDLG_H_

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>
#include "base/mapscr.h"

void call_layer_dialog(int map, int screen);

class LayerDialog: public GUI::Dialog<LayerDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	LayerDialog(int map, int screen);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	int map, screen;
	int autolayer;
	mapscr& dest_ref;
	mapscr local_ref;
	std::map<int, std::pair<int, int>> autolayer_data;
	std::pair<int, int> temp_autolayer_data;
};

#endif
