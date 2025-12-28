#ifndef SUBSCR_GRID_DLG_H_
#define SUBSCR_GRID_DLG_H_

#include <gui/dialog.h>
#include <gui/window.h>
#include <functional>

class SubscrGridDialog: public GUI::Dialog<SubscrGridDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	SubscrGridDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	std::shared_ptr<GUI::Window> window;
	int grid_x, grid_y, grid_xofs, grid_yofs;
	int grid_color;
	bool show_grid;
	bool grid_dots;
};

#endif
