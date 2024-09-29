#ifndef ZC_DIALOG_GAMEICONS_H_
#define ZC_DIALOG_GAMEICONS_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/label.h>
#include <zq/gui/seltile_swatch.h>
#include <gui/window.h>
#include <functional>

bool call_gameicons_dialog();

class GameIconDialog : public GUI::Dialog<GameIconDialog>
{
public:
	enum class message {
		REFR_INFO, OK, CANCEL
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	int32_t gameicons[4];
private:
	GameIconDialog();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::SelTileSwatch> tswatch[4];
	friend bool call_gameicons_dialog();
};

#endif