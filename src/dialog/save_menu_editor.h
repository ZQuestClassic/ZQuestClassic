#ifndef ZC_DIALOG_SAVE_MENU_DLG_H_
#define ZC_DIALOG_SAVE_MENU_DLG_H_

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>
#include "save_menu.h"

void call_editsavemenu_dialog(int index);

class SaveMenuDialog: public GUI::Dialog<SaveMenuDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	SaveMenuDialog(SaveMenu& dest);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	SaveMenu& dest_ref;
	SaveMenu local_ref;
	
	std::shared_ptr<GUI::DropDownList> midi_ddl;
	GUI::ListData list_sfx, list_music, list_aligns, list_font, list_genscr;
};

#endif
