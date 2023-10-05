#ifndef ZC_DIALOG_TILESETWIZARD_H
#define ZC_DIALOG_TILESETWIZARD_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/window.h>
#include <functional>

void call_tileset_wizard();
void call_tileset_wizard_existing();

class TilesetWizard: public GUI::Dialog<TilesetWizard>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	TilesetWizard();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	dword tsetflags;
	word new_map_count;
};

#endif
