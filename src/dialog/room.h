#ifndef ZC_DIALOG_ROOM_H
#define ZC_DIALOG_ROOM_H

#include <gui/dialog.h>
#include <gui/dropDownList.h>
#include <gui/label.h>
#include <gui/switcher.h>
#include <gui/textField.h>
#include <functional>

class RoomDialog: public gui::Dialog<RoomDialog>
{
public:
    enum class Message
    {
        setRoom, setArgument, setGuy, setMessage, roomInfo, ok, cancel
    };

    RoomDialog(int room, int argument, int guy, int message,
        std::function<void(int, int, int, int)> setRoomVars);

    std::shared_ptr<gui::Widget> view() override;
    bool handleMessage(Message msg, gui::MessageArg);

private:
    gui::ListData itemListData, shopListData, infoShopListData, messageListData;
    std::shared_ptr<gui::DropDownList> shopDD, infoShopDD, itemDD;
    std::shared_ptr<gui::TextField> argTF;
    std::shared_ptr<gui::Switcher> argSwitcher;
    std::shared_ptr<gui::Label> argLabel;
    int room, argument, guy, message;
    std::function<void(int, int, int, int)> setRoomVars;

    /* Called when the room is changed to show the appropriate
    * argument selector and set its value.
    */
    void setArgField();

    /* Called when the dialog is closed to get the argument
     * limited to legal values.
     */
    int getArgument() const;
    void showRoomInfo() const;
};

#endif
