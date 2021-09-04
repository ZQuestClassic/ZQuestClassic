#ifndef ZC_DIALOG_ROOM_H
#define ZC_DIALOG_ROOM_H

#include <gui/dialog.h>
#include <gui/dropDownList.h>
#include <gui/label.h>
#include <gui/switcher.h>
#include <gui/textField.h>
#include <functional>

enum class RoomDialogMessage
{
    selectRoom, selectGuy, selectMessage, roomInfo, ok, cancel
};

class RoomDialog: public gui::Dialog<RoomDialogMessage>
{
public:
    using Message=RoomDialogMessage;

    RoomDialog(int room, int argument, int guy, int message,
        std::function<void(int, int, int, int)> setRoomVars);

    std::shared_ptr<gui::Widget> view() override;
    bool handleMessage(Message msg, gui::EventArg) override;

private:
    gui::ListData itemListData, shopListData, infoShopListData, messageListData;
    std::shared_ptr<gui::DropDownList> shopDD, infoShopDD, itemDD;
    std::shared_ptr<gui::TextField> argTF;
    std::shared_ptr<gui::Switcher> argSwitcher;
    std::shared_ptr<gui::Label> argLabel;
    int room, argument, guy, message;
    std::function<void(int, int, int, int)> setRoomVars;

    void setArgField();
    int getArgument();
    void showRoomInfo();
};

#endif
