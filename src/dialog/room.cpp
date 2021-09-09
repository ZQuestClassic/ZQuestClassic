#include "room.h"
#include "info.h"
#include <gui/builder.h>
#include "../zdefs.h"
#include "../zq_misc.h"
#include "../zquest.h"
#include <boost/format.hpp>

using std::get;

static const GUI::ListData guyListData(
	GUI::itemText,
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

static const GUI::ListData roomListData(
	GUI::itemText,
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

GUI::ListData getItemListData()
{
	return GUI::ListData(ITEMCNT,
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

GUI::ListData getShopListData()
{
	return GUI::ListData(256,
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

GUI::ListData getInfoShopListData()
{
	return GUI::ListData(256,
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

GUI::ListData getStringListData()
{
	std::vector<size_t> msgMap(msg_count, 0);
	for(size_t i=0; i<msg_count; i++)
	{
		auto& msg=MsgStrings[i];
		msgMap[msg.listpos]=i;
	}

	return GUI::ListData(msg_count,
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

static const auto specialItemDesc=
	"If a Guy is set, he will offer an item to Link. "
	"This room type is also used for Item Cellar warps, "
	"and 'Armos/Chest->Item' and 'Dive For Item' combo flags.";

static const auto infoDesc=
	"Pay rupees to make one of three strings appear. "
	"Strings and prices are set in Misc. Data -> Info Types.";

static const auto moneyDesc=
	"If a Guy is set, he will offer rupees to Link.";

static const auto gambleDesc=
	"The 'Money-Making Game' from The Legend of Zelda. "
	"Risk losing up to 40 rupees for a chance to win up to 50 rupees.";

static const auto repairDesc=
	"When the Guy's String finishes, the player loses a given amount of money.";

static const auto potionOrHCDesc=
	"The Guy offers item 28 and item 30 to Link. "
	"Taking one makes the other vanish forever.";

static const auto grumbleDesc=
	"Until the player uses a Bait item, the Guy and "
	"his invisible wall won't vanish and shutters won't open. "
	"The Bait item will be removed from the player's inventory.";

static const auto triforceDesc=
	"The Guy and his invisible wall won't vanish "
	"unless Link has Triforces from levels 1-8. "
	"(Shutters won't open until the Guy vanishes, too.)";

static const auto potionShopDesc=
	"Similar to a Shop, but the items and String "
	"won't appear until Link uses a Letter item. "
	"(Or, if Link already has a Level 2 Letter item.)";

static const auto shopDesc=
	"The Guy offers three items for a fee. "
	"You can use the Shop as often as you want. "
	"Items and prices are set in Misc. Data -> Shop Types.";

static const auto moreBombsDesc=
	"The Guy offers to increase Link's Bombs "
	"and Max. Bombs by 4, for a fee. You can only buy it once.";

static const auto swindleDesc=
	"The Guy and his invisible wall won't vanish until "
	"Link pays the fee or forfeits a Heart Container. "
	"(Shutters won't open until the Guy vanishes, too.)";

static const auto tenRupiesDesc=
	"10 instances of item 0 appear in a  diamond formation in "
	"the center of the screen. No Guy or String needs to be set for this.";

static const auto warpDesc=
	"All 'Stair [A]' type combos send Link to "
	"a destination in a given Warp Ring, based "
	"on the combo's X position (<112, >136, or between).";

static const auto ganonDesc=
	"Link holds up the Triforce, and Ganon appears. "
	"(Unless the current DMap's Dungeon Boss was beaten.)";

static const auto zeldaDesc=
	"Four instances of enemy 85 appear on the screen in front of the Guy. "
	"(That's all it does.)";

static const auto magicUpgradeDesc=
	"When the Guy's String finishes, Link gains the 1/2 Magic Usage attribute.";

static const auto learnSlashDesc=
	"When the Guy's String finishes, Link gains the Slash attribute.";

static const auto moreArrowsDesc=
	"The Guy offers to increase Link's Arrows and Max. Arrows by 10, "
	"for a fee. You can only buy it once.";

static const auto takeOneDesc=
	"The Guy offers three items. Taking one makes the others vanish forever. "
	"Item choices are set in Misc. Data -> Shop Types.";

static const auto defaultDesc=
	"Select a Room Type, then click the \"Info\" button "
	"to find out what it does.";

// Used as a selector for argSwitcher. Make sure the order matches.
enum { argTextField, argItemList, argShopList, argInfoShopList };

RoomDialog::RoomDialog(int room, int argument, int guy, int string,
	std::function<void(int, int, int, int)> setRoomVars):
		itemListData(getItemListData()),
		shopListData(getShopListData()),
		infoShopListData(getInfoShopListData()),
		stringListData(getStringListData()),
		room({ room, argument, guy, string }),
		setRoomVars(setRoomVars)
{}

std::shared_ptr<GUI::Widget> RoomDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	argLabel=Label(hAlign=1.0);
	argSwitcher=Switcher(
		argTF=TextField(
			type=GUI::TextField::type::INT_DECIMAL,
			maxLength=6,
			text=std::to_string(room.argument),
			hAlign=0.0,
			onValueChanged=message::SET_ARGUMENT),
		itemDD=DropDownList(
			data=itemListData,
			selectedValue=room.argument,
			onSelectionChanged=message::SET_ARGUMENT),
		shopDD=DropDownList(
			data=shopListData,
			selectedValue=room.argument,
			onSelectionChanged=message::SET_ARGUMENT),
		infoShopDD=DropDownList(
			data=infoShopListData,
			selectedValue=room.argument,
			onSelectionChanged=message::SET_ARGUMENT)
	);

	setArgField();

	return Window(
		title="Room Type",
		onClose=message::CANCEL,
		shortcuts={ F1=message::ROOM_INFO },
		Column(
			Columns<4>(
				Label(text="Room type:", hAlign=1.0),
				argLabel,
				Label(text="Guy:", hAlign=1.0),
				Label(text="Message:", hAlign=1.0),

				DropDownList(
					data=roomListData,
					onSelectionChanged=message::SET_ROOM,
					selectedValue=room.type),
				argSwitcher,
				DropDownList(
					data=guyListData,
					onSelectionChanged=message::SET_GUY,
					selectedValue=room.guy),
				DropDownList(
					data=stringListData,
					onSelectionChanged=message::SET_STRING,
					selectedValue=room.string),

				Button(
					text="&Info",
					width=3_em,
					onClick=message::ROOM_INFO
				)
			),
			Row(
				Button(text="OK", onClick=message::OK),
				Button(text="Cancel", onClick=message::CANCEL)
			)
		)
	);
}

bool RoomDialog::handleMessage(message msg, GUI::MessageArg messageArg)
{
	switch(msg)
	{
	case message::SET_ROOM:
		room.type=(int)messageArg;
		setArgField();
		return false;

	case message::SET_ARGUMENT:
		room.argument=(int)messageArg;
		return false;

	case message::SET_GUY:
		room.guy=(int)messageArg;
		return false;

	case message::SET_STRING:
		room.string=(int)messageArg;
		return false;

	case message::ROOM_INFO:
		InfoDialog("Room Info", getRoomInfo()).show();
		return false;

	case message::OK:
		setRoomVars(room.type, getArgument(), room.guy, room.string);
		return true;

	case message::CANCEL:
	default:
		return true;
	}
}

void RoomDialog::setArgField()
{
	switch(room.type)
	{
	case rSP_ITEM:
		argSwitcher->switchTo(argItemList);
		itemDD->setSelectedValue(room.argument);
		argLabel->setText("Item:");
		break;
	case rINFO:
		argSwitcher->switchTo(argInfoShopList);
		infoShopDD->setSelectedValue(room.argument);
		argLabel->setText("Shop:");
		break;
	case rMONEY:
		argSwitcher->switchTo(argTextField);
		argTF->setText(std::to_string(room.argument));
		argLabel->setText("Amount:");
		break;
	case rREPAIR:
	case rBOMBS:
	case rSWINDLE:
	case rARROWS:
		argSwitcher->switchTo(argTextField);
		argTF->setText(std::to_string(room.argument));
		argLabel->setText("Price:");
		break;
	case rP_SHOP:
	case rSHOP:
	case rTAKEONE:
		argSwitcher->switchTo(argShopList);
		shopDD->setSelectedValue(room.argument);
		argLabel->setText("Shop:");
		break;
	default:
		argSwitcher->switchTo(argTextField);
		argTF->setText(std::to_string(room.argument));
		argLabel->setText("(Unused):");
		break;
	}
}

int RoomDialog::getArgument() const
{
	switch(argSwitcher->getCurrentIndex())
	{
	case argItemList:
		return itemDD->getSelectedValue();
	case argShopList:
		return shopDD->getSelectedValue();
	case argInfoShopList:
		return infoShopDD->getSelectedValue();
	default:
		return room.argument>=0 ? room.argument : -room.argument;
	}
}

const char* RoomDialog::getRoomInfo() const
{
	switch(room.type)
	{
		case rSP_ITEM: return specialItemDesc;
		case rINFO: return infoDesc;
		case rMONEY: return moneyDesc;
		case rGAMBLE: return gambleDesc;
		case rREPAIR: return repairDesc;
		case rRP_HC: return potionOrHCDesc;
		case rGRUMBLE: return grumbleDesc;
		case rTRIFORCE: return triforceDesc;
		case rP_SHOP: return potionShopDesc;
		case rSHOP: return shopDesc;
		case rBOMBS: return moreBombsDesc;
		case rSWINDLE: return swindleDesc;
		case r10RUPIES: return tenRupiesDesc;
		case rWARP: return warpDesc;
		case rGANON: return ganonDesc;
		case rZELDA: return zeldaDesc;
		case rMUPGRADE: return magicUpgradeDesc;
		case rLEARNSLASH: return learnSlashDesc;
		case rARROWS: return moreArrowsDesc;
		case rTAKEONE: return takeOneDesc;
		default: return defaultDesc;
	}
}
