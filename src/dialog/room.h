#ifndef ZC_DIALOG_ROOM_H_
#define ZC_DIALOG_ROOM_H_

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/label.h>
#include <gui/switcher.h>
#include <gui/text_field.h>
#include <functional>
#include "base/mapscr.h"
void call_room_dlg(mapscr* scr);
class RoomDialog: public GUI::Dialog<RoomDialog>
{
public:
	enum class message
	{
		REFR_INFO, REFRESH, ROOM_INFO, OK, CANCEL
	};

	RoomDialog(mapscr* m);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	GUI::ListData itemListData, shopListData, bshopListData, infoShopListData, stringListData;
	std::shared_ptr<GUI::DropDownList> shopDD, bshopDD, infoShopDD, itemDD;
	std::shared_ptr<GUI::TextField> argTF;
	std::shared_ptr<GUI::Switcher> argSwitcher;
	std::shared_ptr<GUI::Label> argLabel;
	mapscr local_mapref;
	mapscr* base_mapref;

	/* Called when the room is changed to show the appropriate
	* argument selector and set its value.
	*/
	void setArgField();

	/* Called when the dialog is closed to get the argument
	 * limited to legal values.
	 */
	int32_t getArgument() const;

	/* Returns a string describing the currently selected room. */
	const char* getRoomInfo() const;
	
	void setOldGuy();
};

#endif
