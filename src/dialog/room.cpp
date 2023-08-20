#include "room.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "base/qrs.h"
extern bool saved;
extern guydata *guysbuf;

void call_room_dlg(mapscr* scr)
{
	RoomDialog(scr).show();
}

// Used as a indices into argSwitcher. Make sure the order matches.
enum { argTEXT_FIELD, argITEM_LIST, argSHOP_LIST, argINFO_LIST, argBSHOP_LIST };

static const GUI::ListData guyListData {
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
};

static const GUI::ListData roomListData {
	{ "(None)", rNONE },
	{ "Special Item", rSP_ITEM },
	{ "Pay for Info", rINFO },
	{ "Secret Money", rMONEY },
	{ "Gamble",rGAMBLE },
	{ "Door Repair", rREPAIR },
	{ "Red Potion or Heart Container", rRP_HC },
	{ "Feed the Goriya", rGRUMBLE },
	{ "Triforce Check", rTRIFORCE },
	{ "Letter Shop", rP_SHOP },
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
	{ "Take One Item", rTAKEONE },
	{ "Bottle Shop", rBOTTLESHOP }
};

static const auto specialItemDesc =
	"If a Guy is set, he will offer an item to the Player. "
	"This room type is also used for Item Cellar warps, "
	"and 'Armos/Chest->Item' and 'Dive For Item' combo flags.";

static const auto infoDesc =
	"Pay rupees to make one of three strings appear. "
	"Strings and prices are set in Misc. Data -> Info Types.";

static const auto moneyDesc =
	"If a Guy is set, he will offer rupees to the Player.";

static const auto gambleDesc =
	"The 'Money-Making Game' from The Legend of Zelda. "
	"Risk losing up to 40 rupees for a chance to win up to 50 rupees.";

static const auto repairDesc =
	"When the Guy's String finishes, the player loses a given amount of money.";

static const auto potionOrHCDesc =
	"The Guy offers item 28 and item 30 to the Player. "
	"Taking one makes the other vanish forever.";

static const auto grumbleDesc =
	"Until the player uses a Bait item, the Guy and "
	"his invisible wall won't vanish and shutters won't open. "
	"The Bait item will be removed from the player's inventory.";

static const auto triforceDesc =
	"The Guy and his invisible wall won't vanish "
	"unless the Player has Triforces from levels 1-8. "
	"(Shutters won't open until the Guy vanishes, too.)";

static const auto potionShopDesc =
	"Similar to a Shop, but the items and String "
	"won't appear until the Player uses a Letter item. "
	"(Or, if the Player already has a Level 2 Letter item.)";

static const auto shopDesc =
	"The Guy offers three items for a fee. "
	"You can use the Shop as often as you want. "
	"Items and prices are set in Misc. Data -> Shop Types.";

static const auto bshopDesc =
	"The Guy offers three bottle fills for a fee. "
	"You must have an empty bottle to make a purchase. "
	"You can use the Shop as often as you want. "
	"Fills and prices are set in Misc. Data -> Bottle Shop Types.";

static const auto moreBombsDesc =
	"The Guy offers to increase the Player's Bombs "
	"and Max. Bombs by 4, for a fee. You can only buy it once.";

static const auto swindleDesc =
	"The Guy and his invisible wall won't vanish until "
	"the Player pays the fee or forfeits a Heart Container. "
	"(Shutters won't open until the Guy vanishes, too.)";

static const auto tenRupiesDesc =
	"10 instances of item 0 appear in a  diamond formation in "
	"the center of the screen. No Guy or String needs to be set for this.";

static const auto warpDesc =
	"All 'Stair [A]' type combos send the Player to "
	"a destination in a given Warp Ring, based "
	"on the combo's X position (<112, >136, or between).";

static const auto ganonDesc =
	"The Player holds up the Triforce, and Ganon appears. "
	"(Unless the current DMap's Dungeon Boss was beaten.)";

static const auto zeldaDesc =
	"Four instances of enemy 85 appear on the screen in front of the Guy. "
	"(That's all it does.)";

static const auto magicUpgradeDesc =
	"When the Guy's String finishes, the Player gains the 1/2 Magic Usage attribute.";

static const auto learnSlashDesc =
	"When the Guy's String finishes, the Player gains the Slash attribute.";

static const auto moreArrowsDesc =
	"The Guy offers to increase the Player's Arrows and Max. Arrows by 10, "
	"for a fee. You can only buy it once.";

static const auto takeOneDesc =
	"The Guy offers three items. Taking one makes the others vanish forever. "
	"Item choices are set in Misc. Data -> Shop Types.";

static const auto defaultDesc =
	"Select a Room Type, then click the \"Info\" button "
	"to find out what it does.";

RoomDialog::RoomDialog(mapscr* m):
		itemListData(getItemListData(false)),
		shopListData(getShopListData()),
		bshopListData(getBShopListData()),
		infoShopListData(getInfoShopListData()),
		stringListData(getStringListData()),
		local_mapref(*m),
		base_mapref(m)
{}

std::shared_ptr<GUI::Widget> RoomDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	if(local_mapref.guytile == -1)
		setOldGuy();
	
	argLabel = Label(hAlign = 1.0);
	argSwitcher = Switcher(
		forceFitW = true,
		argTF = TextField(
			fitParent = true,
			type = GUI::TextField::type::INT_DECIMAL,
			maxLength = 6, val = local_mapref.catchall,
			low = 0, high = 65535, hAlign = 0.0,
			onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_mapref.catchall = val;
			}),
		itemDD = DropDownList(data = itemListData,
			fitParent = true,
			selectedValue = local_mapref.catchall,
			onSelectFunc = [&](int32_t val)
			{
				local_mapref.catchall = val;
			}),
		shopDD = DropDownList(data = shopListData,
			fitParent = true,
			selectedValue = local_mapref.catchall,
			onSelectFunc = [&](int32_t val)
			{
				local_mapref.catchall = val;
			}),
		infoShopDD = DropDownList(data = infoShopListData,
			fitParent = true,
			selectedValue = local_mapref.catchall,
			onSelectFunc = [&](int32_t val)
			{
				local_mapref.catchall = val;
			}),
		bshopDD = DropDownList(data = bshopListData,
			fitParent = true,
			selectedValue = local_mapref.catchall,
			onSelectFunc = [&](int32_t val)
			{
				local_mapref.catchall = val;
			})
	);

	setArgField();

	return Window(
		title = "Room Type",
		onClose = message::CANCEL,
		shortcuts = {
			F1 = message::ROOM_INFO
		},
		Column(
			Rows<4>(
				Label(text = "Room type:", hAlign=1.0),
				DropDownList(data = roomListData,
					fitParent = true,
					selectedValue = local_mapref.room,
					onSelectFunc = [&](int32_t val)
					{
						local_mapref.room = val;
						setArgField();
					}),
				Button(text = "?",
					width = 2_em,
					onClick = message::ROOM_INFO,
					padding = 0_px),
				DummyWidget(),
				//
				argLabel,
				argSwitcher,
				DummyWidget(colSpan=2),
				//
				Label(text = "Message:", hAlign=1.0),
				DropDownList(data = stringListData,
					forceFitW = true,
					selectedValue = local_mapref.str,
					onSelectFunc = [&](int32_t val)
					{
						local_mapref.str = val;
					}),
				DummyWidget(colSpan=2),
				//
				Label(text = "Guy Tile:", hAlign=1.0),
				SelTileSwatch(
					tile = local_mapref.guytile,
					cset = local_mapref.guycs,
					showvals = true,
					onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t)
					{
						local_mapref.guytile = t;
						local_mapref.guycs = (c&0xF)%14;
					}),
				INFOBTN("Set the tile+cset for the guy to use."
					" Requires 'Old Guy Handling' off." + QRHINT({qr_OLD_GUY_HANDLING})),
				DummyWidget(),
				//
				Checkbox(colSpan = 2,
					text = "Force Guy on Screen", hAlign = 0.0,
					checked = local_mapref.roomflags & RFL_ALWAYS_GUY,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(local_mapref.roomflags,RFL_ALWAYS_GUY,state);
					}),
				INFOBTN("Makes the guy appear on the main screen, regardless of"
					" 'Special Rooms And Guys Are In Caves Only' dmap flag."
					" Requires 'Old Guy Handling' off."+ QRHINT({qr_OLD_GUY_HANDLING})),
				DummyWidget(),
				//
				Checkbox(colSpan = 2,
					text = "Show Fires", hAlign = 0.0,
					checked = local_mapref.roomflags & RFL_GUYFIRES,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(local_mapref.roomflags,RFL_GUYFIRES,state);
					}),
				INFOBTN("If the fires should spawn next to the guy or not."
					" Requires 'Old Guy Handling' off." + QRHINT({qr_OLD_GUY_HANDLING})),
				DummyWidget(),
				//
				Label(text = "Guy ID:", hAlign = 1.0),
				DropDownList(data = guyListData,
					fitParent = true,
					selectedValue = local_mapref.guy,
					onSelectFunc = [&](int32_t val)
					{
						local_mapref.guy = val;
					}),
				INFOBTN("Which ID to use for the guy. If 'None', the room has no guy."
					"\nUse the 'Set' button to set the tile/cset/guy-related flags based on"
					" how old versions used these guys." + QRHINT({qr_OLD_GUY_HANDLING})),
				Button(forceFitH = true, text = "Set",
					onPressFunc = [&]()
					{
						setOldGuy();
						SETFLAG(local_mapref.roomflags,RFL_ALWAYS_GUY,local_mapref.guy==gFAIRY);
						SETFLAG(local_mapref.roomflags,RFL_GUYFIRES,local_mapref.guy!=gFAIRY || !get_qr(qr_NOFAIRYGUYFIRES));
					})
			),
			Row(
				spacing = 2_em,
				Button(text = "OK", minwidth = 90_px, onClick = message::OK, focused = true),
				Button(text = "Cancel", minwidth = 90_px, onClick = message::CANCEL)
			)
		)
	);
}

bool RoomDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
	case message::ROOM_INFO:
		InfoDialog("Room Info", getRoomInfo()).show();
		return false;

	case message::OK:
		*base_mapref = local_mapref;
		saved = false;
		return true;

	case message::CANCEL:
		return true;
	}
	return false;
}

void RoomDialog::setArgField()
{
	switch(local_mapref.room)
	{
	case rSP_ITEM:
		argSwitcher->switchTo(argITEM_LIST);
		itemDD->setSelectedValue(local_mapref.catchall);
		argLabel->setText("Item:");
		break;
	case rINFO:
		argSwitcher->switchTo(argINFO_LIST);
		infoShopDD->setSelectedValue(local_mapref.catchall);
		argLabel->setText("Shop:");
		break;
	case rMONEY:
		argSwitcher->switchTo(argTEXT_FIELD);
		argTF->setVal(local_mapref.catchall);
		argLabel->setText("Amount:");
		break;
	case rREPAIR:
	case rBOMBS:
	case rSWINDLE:
	case rARROWS:
		argSwitcher->switchTo(argTEXT_FIELD);
		argTF->setVal(local_mapref.catchall);
		argLabel->setText("Price:");
		break;
	case rP_SHOP:
	case rSHOP:
	case rTAKEONE:
		argSwitcher->switchTo(argSHOP_LIST);
		shopDD->setSelectedValue(local_mapref.catchall);
		argLabel->setText("Shop:");
		break;
	case rBOTTLESHOP:
		argSwitcher->switchTo(argBSHOP_LIST);
		bshopDD->setSelectedValue(local_mapref.catchall);
		argLabel->setText("B. Shop:");
		break;
	default:
		argSwitcher->switchTo(argTEXT_FIELD);
		argTF->setVal(local_mapref.catchall);
		argLabel->setText("(Unused):");
		break;
	}
}

int32_t RoomDialog::getArgument() const
{
	switch(argSwitcher->getCurrentIndex())
	{
	case argITEM_LIST:
		return itemDD->getSelectedValue();
	case argSHOP_LIST:
		return shopDD->getSelectedValue();
	case argBSHOP_LIST:
		return bshopDD->getSelectedValue();
	case argINFO_LIST:
		return infoShopDD->getSelectedValue();
	default:
		return local_mapref.catchall >= 0 ? local_mapref.catchall : -local_mapref.catchall;
	}
}

const char* RoomDialog::getRoomInfo() const
{
	switch(local_mapref.room)
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
		case rBOTTLESHOP: return bshopDesc;
		default: return defaultDesc;
	}
}

void RoomDialog::setOldGuy()
{
	auto& ref = guysbuf[local_mapref.guy];
	local_mapref.guytile = get_qr(qr_NEWENEMYTILES) ? ref.e_tile : ref.tile;
	local_mapref.guycs = ref.cset;
}


