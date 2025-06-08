#ifndef ZC_DIALOG_TILESETWIZARD_H_
#define ZC_DIALOG_TILESETWIZARD_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/window.h>
#include "gui/radioset.h"
#include "gui/label.h"
#include <functional>

enum tileset_option {
	cambria, classic, blank, file
};

bool call_tileset_wizard();

class TilesetWizard: public GUI::Dialog<TilesetWizard>
{
public:
	enum class message { REFR_INFO, CHOOSE_TILESET, OK, CANCEL };

	TilesetWizard();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	bool success() const;
	
private:
	bool loadTileset(std::string path);

	std::shared_ptr<GUI::Window> window;
	std::string file_path;
	tileset_option tileset_choice;
	dword tsetflags;
	word new_map_count;
	std::shared_ptr<GUI::RadioSet> tilesetChoice;
	std::shared_ptr<GUI::Label> tilesetInfo;
};

#endif
