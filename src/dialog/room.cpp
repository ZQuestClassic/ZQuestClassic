#include "room.h"
#include "info.h"
#include <gui/builder.h>
#include "../zdefs.h"
#include "../zq_misc.h"
#include "../zquest.h"
#include <boost/format.hpp>

using std::get;

static const gui::ListData guyListData(
    gui::itemText,
    {
        { "(None)", 0 },
        { "Abei", 1 },
        { "Ama", 2 },
        { "Merchant", 3 },
        { "Moblin", 4 },
        { "Fire", 5 },
        { "Fairy", 6 },
        { "Goriya", 7 },
        { "Zelda", 8 },
        { "Abei 2", 9 },
        { "Empty", 10 }
    }
);

static const gui::ListData roomListData(
    gui::itemText,
    {
        { "(None)", rNONE },
        { "Special Item", rSP_ITEM },
        { "Pay for Info", rINFO },
        { "Secret Money", rMONEY },
        { "Gamble",rGAMBLE },
        { "Door Repair", rREPAIR },
        { "Red Potion or Heart Container", rRP_HC },
        { "Feed the Goriya", rGRUMBLE },
        { "Level 9 Entrance", rTRIFORCE },
        { "Potion Shop", rP_SHOP },
        { "Shop", rSHOP },
        { "More Bombs", rBOMBS },
        { "Leave Money or Life", rSWINDLE },
        { "10 Rupees", r10RUPIES },
        { "3-Stair Warp", rWARP },
        { "Ganon", rGANON },
        { "Zelda", rZELDA },
        { "1/2 Magic Upgrade", rMUPGRADE },
        { "Learn Slash", rLEARNSLASH },
        { "More Arrows", rARROWS },
        { "Take One Item", rTAKEONE }
    }
);

gui::ListData getItemListData()
{
    return gui::ListData(ITEMCNT,
        [](size_t index)
        {
            return boost::str(boost::format("%1%")
                % item_string[index]);
        },
        [](size_t index)
        {
            return index;
        });
}

gui::ListData getShopListData()
{
    return gui::ListData(256,
        [](size_t index)
        {
            return boost::str(boost::format("%1%:  %2%")
                % index
                % misc.shop[index].name);
        },
        [](size_t index)
        {
            return index;
        });
}

gui::ListData getInfoShopListData()
{
    return gui::ListData(256,
        [](size_t index)
        {
            return boost::str(boost::format("%1%:  %2%")
                % index
                % misc.info[index].name);
        },
        [](size_t index)
        {
            return index;
        });
}

gui::ListData getMessageListData()
{
    std::vector<size_t> msgMap(msg_count, 0);
    for(size_t i=0; i<msg_count; i++)
    {
        auto& msg=MsgStrings[i];
        msgMap[msg.listpos]=i;
    }

    return gui::ListData(msg_count,
        [&msgMap](size_t index)
        {
            return boost::str(boost::format("%1%: %2%")
                % msgMap[index]
                % MsgStrings[msgMap[index]].s);
        },
        [&msgMap](size_t index)
        {
            return msgMap[index];
        });
}

static const auto specialItemDesc={
    "If a Guy is set, he will offer an item to Link.",
    "This room type is also used for Item Cellar warps,",
    "and 'Armos/Chest->Item' and 'Dive For Item' combo flags."
};

static const auto infoDesc={
    "Pay rupees to make one of three strings appear.",
    "Strings and prices are set in",
    "Misc. Data -> Info Types."
};

static const auto moneyDesc={
    "If a Guy is set, he will offer rupees to Link."
};

static const auto gambleDesc={
    "The 'Money-Making Game' from The Legend of Zelda.",
    "Risk losing up to 40 rupees for",
    "a chance to win up to 50 rupees."
};

static const auto repairDesc={
    "When the Guy's String finishes,",
    "the player loses a given amount of money."
};

static const auto potionOrHCDesc={
    "The Guy offers item 28 and item 30 to Link.",
    "Taking one makes the other vanish forever."
};

static const auto grumbleDesc={
    "Until the player uses a Bait item, the Guy and",
    "his invisible wall won't vanish and shutters won't open.",
    "The Bait item will be removed from the player's inventory."
};

static const auto triforceDesc={
    "The Guy and his invisible wall won't vanish",
    "unless Link has Triforces from levels 1-8.",
    "(Shutters won't open until the Guy vanishes, too.)"
};

static const auto potionShopDesc={
    "Similar to a Shop, but the items and String",
    "won't appear until Link uses a Letter item.",
    "(Or, if Link already has a Level 2 Letter item.)"
};

static const auto shopDesc={
    "The Guy offers three items for a fee.",
    "You can use the Shop as often as you want.",
    "Items and prices are set in Misc. Data -> Shop Types."
};

static const auto moreBombsDesc={
    "The Guy offers to increase Link's Bombs",
    "and Max. Bombs by 4, for a fee.",
    "You can only buy it once."
};

static const auto swindleDesc={
    "The Guy and his invisible wall won't vanish until",
    "Link pays the fee or forfeits a Heart Container.",
    "(Shutters won't open until the Guy vanishes, too.)"
};

static const auto tenRupiesDesc={
    "10 instances of item 0 appear in a",
    "diamond formation in the center of the screen.",
    "No Guy or String needs to be set for this."
};

static const auto warpDesc={
    "All 'Stair [A]' type combos send Link to",
    "a destination in a given Warp Ring, based",
    "on the combo's X position (<112, >136, or between)."
};

static const auto ganonDesc={
    "Link holds up the Triforce, and Ganon appears.",
    "(Unless the current DMap's Dungeon Boss was beaten.)"
};

static const auto zeldaDesc={
    "Four instances of enemy 85 appear",
    "on the screen in front of the Guy.",
    "(That's all it does.)"
};

static const auto magicUpgradeDesc={
    "When the Guy's String finishes,",
    "Link gains the 1/2 Magic Usage attribute."
};

static const auto learnSlashDesc={
    "When the Guy's String finishes,",
    "Link gains the Slash attribute."
};

static const auto moreArrowsDesc={
    "The Guy offers to increase Link's Arrows",
    "and Max. Arrows by 10, for a fee.",
    "You can only buy it once."
};

static const auto takeOneDesc={
    "The Guy offers three items.",
    "Taking one makes the others vanish forever.",
    "Item choices are set in Misc. Data -> Shop Types."
};

static const auto defaultDesc={
    "Select a Room Type, then click",
    "the \"Info\" button to find out what it does."
};

// Used as a selector for argSwitcher. Make sure the order matches.
enum { argTextField, argItemList, argShopList, argInfoShopList };

RoomDialog::RoomDialog(int room, int argument, int guy, int message,
    std::function<void(int, int, int, int)> setRoomVars)
: itemListData(getItemListData())
, shopListData(getShopListData())
, infoShopListData(getInfoShopListData())
, messageListData(getMessageListData())
, room(room)
, argument(argument)
, guy(guy)
, message(message)
, setRoomVars(setRoomVars)
{}

std::shared_ptr<gui::Widget> RoomDialog::view()
{
    using namespace gui::builder;
    using namespace gui::key;
    using namespace gui::props;

    argLabel=Label(hAlign=1.0);
    argSwitcher=Switcher(
        argTF=TextField(
            type=gui::TextField::Type::IntDecimal,
            maxLength=6,
            text=std::to_string(argument),
            hAlign=0.0,
            onValueChanged=Message::setArgument),
        itemDD=DropDownList(
            data=itemListData,
            selectedValue=argument,
        onSelectionChanged=Message::setArgument),
        shopDD=DropDownList(
            data=shopListData,
            selectedValue=argument,
        onSelectionChanged=Message::setArgument),
        infoShopDD=DropDownList(
            data=infoShopListData,
            selectedValue=argument,
            onSelectionChanged=Message::setArgument)
    );

    setArgField();

    return Window(
        title="Room Type",
        onClose=Message::cancel,
        shortcuts={ F1=Message::roomInfo },
        Column(
            Columns<4>(
                Label(text="Room type:", hAlign=1.0),
                argLabel,
                Label(text="Guy:", hAlign=1.0),
                Label(text="Message:", hAlign=1.0),

                DropDownList(
                    data=roomListData,
                    onSelectionChanged=Message::setRoom,
                    selectedValue=room),
                argSwitcher,
                DropDownList(
                    data=guyListData,
                    onSelectionChanged=Message::setGuy,
                    selectedValue=guy),
                DropDownList(
                    data=messageListData,
                    onSelectionChanged=Message::setMessage,
                    selectedValue=message),

                Button(
                    text="&Info",
                    width=3_em,
                    onClick=Message::roomInfo
                )
            ),
            Row(
                Button(text="OK", onClick=Message::ok),
                Button(text="Cancel", onClick=Message::cancel)
            )
        )
    );
}

bool RoomDialog::handleMessage(Message msg, gui::MessageArg messageArg)
{
    switch(msg)
    {
    case Message::setRoom:
        room=(int)messageArg;
        setArgField();
        return false;

    case Message::setArgument:
        argument=(int)messageArg;
        return false;

    case Message::setGuy:
        guy=(int)messageArg;
        return false;

    case Message::setMessage:
        message=(int)messageArg;
        return false;

    case Message::roomInfo:
        showRoomInfo();
        return false;

    case Message::ok:
        setRoomVars(room, getArgument(), guy, message);
        return true;

    case Message::cancel:
    default:
        return true;
    }
}

void RoomDialog::setArgField()
{
    switch(room)
    {
    case rSP_ITEM:
        argSwitcher->show(argItemList);
        itemDD->setSelectedValue(argument);
        argLabel->setText("Item:");
        break;
    case rINFO:
        argSwitcher->show(argInfoShopList);
        infoShopDD->setSelectedValue(argument);
        argLabel->setText("Shop:");
        break;
    case rMONEY:
        argSwitcher->show(argTextField);
        argTF->setText(std::to_string(argument));
        argLabel->setText("Amount:");
        break;
    case rREPAIR:
    case rBOMBS:
    case rSWINDLE:
    case rARROWS:
        argSwitcher->show(argTextField);
        argTF->setText(std::to_string(argument));
        argLabel->setText("Price:");
        break;
    case rP_SHOP:
    case rSHOP:
    case rTAKEONE:
        argSwitcher->show(argShopList);
        shopDD->setSelectedValue(argument);
        argLabel->setText("Shop:");
        break;
    default:
        argSwitcher->show(argTextField);
        argTF->setText(std::to_string(argument));
        argLabel->setText("(Unused):");
        break;
    }
}

int RoomDialog::getArgument() const
{
    switch(argSwitcher->getVisible())
    {
    case argItemList:
        return itemDD->getSelectedValue();
    case argShopList:
        return shopDD->getSelectedValue();
    case argInfoShopList:
        return infoShopDD->getSelectedValue();
    default:
        return argument>=0 ? argument : -argument;
    }
}

void RoomDialog::showRoomInfo() const
{
    const decltype(defaultDesc)* info=nullptr;
    switch(room)
    {
    case rSP_ITEM:
        info=&specialItemDesc;
        break;
    case rINFO:
        info=&infoDesc;
        break;
    case rMONEY:
        info=&moneyDesc;
        break;
    case rGAMBLE:
        info=&gambleDesc;
        break;
    case rREPAIR:
        info=&repairDesc;
        break;
    case rRP_HC:
        info=&potionOrHCDesc;
        break;
    case rGRUMBLE:
        info=&grumbleDesc;
        break;
    case rTRIFORCE:
        info=&triforceDesc;
        break;
    case rP_SHOP:
        info=&potionShopDesc;
        break;
    case rSHOP:
        info=&shopDesc;
        break;
    case rBOMBS:
        info=&moreBombsDesc;
        break;
    case rSWINDLE:
        info=&swindleDesc;
        break;
    case r10RUPIES:
        info=&tenRupiesDesc;
        break;
    case rWARP:
        info=&warpDesc;
        break;
    case rGANON:
        info=&ganonDesc;
        break;
    case rZELDA:
        info=&zeldaDesc;
        break;
    case rMUPGRADE:
        info=&magicUpgradeDesc;
        break;
    case rLEARNSLASH:
        info=&learnSlashDesc;
        break;
    case rARROWS:
        info=&moreArrowsDesc;
        break;
    case rTAKEONE:
        info=&takeOneDesc;
        break;
    default:
        info=&defaultDesc;
        break;
    }

    InfoDialog("Room Info", *info).show();
}
