#ifndef ZC_DIALOG_MAPSTYLES_H_
#define ZC_DIALOG_MAPSTYLES_H_

#include <gui/dialog.h>
#include <gui/label.h>
#include <zq/gui/seltile_swatch.h>
#include <gui/window.h>
#include <functional>
#include <string_view>
#include <base/misctypes.h>

bool call_mapstyles_dialog ();

class MapStylesDialog : public GUI::Dialog<MapStylesDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	MapStylesDialog();
	std::shared_ptr<GUI::Window> window;
	zcolors local_mapstyles;
	friend bool call_mapstyles_dialog();
};

#endif