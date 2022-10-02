#include "stringcontrolcode.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include <boost/format.hpp>
#include <sstream>
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "zq_tiles.h"

extern bool saved;
extern miscQdata misc;
extern zinitdata zinit;
extern ListData dmap_list;

static std::string retstr;
static MsgStr const* refstr = nullptr;
static MsgStr def_refstr;

const char *msgslist(int32_t index, int32_t *list_size);
int32_t msg_code_operands(byte cc);
bool is_msgc(byte cc);
std::string run_scc_dlg(MsgStr const* ref)
{
	if(ref)
		refstr = ref;
	else refstr = nullptr;
	retstr = "";
	SCCDialog().show();
	return retstr;
}

void calc_retstr(byte scc, int32_t* args)
{
	if(!is_msgc(scc))
	{
		retstr = "";
		return;
	}
	std::ostringstream oss;
	oss << "\\" << word(scc);
	auto count = msg_code_operands(scc);
	int32_t val;
	for(auto q = 0; q < count; ++q)
	{
		if(unsigned(args[q]) >= MAX_SCC_ARG)
			val = -1;
		else val = word(args[q]);
		oss << "\\" << val;
	}
	retstr = oss.str();
}

const GUI::ListData SCCListData()
{
	std::string mcguffinname(ZI.getItemClassName(itype_triforcepiece));
	GUI::ListData ld;
	ld.add("Text Color", 1);
	ld.add("Text Speed", 2);
	ld.add("Goto If Screen->D[]", 23);
	ld.add("Goto If Current Screen->D[]", 3);
	ld.add("Goto If Random", 4);
	ld.add("Goto If Item", 5);
	ld.add("Goto If Counter >=", 6);
	ld.add("Goto If Counter >= %", 7);
	ld.add("Goto If Level " + mcguffinname, 8);
	ld.add("Goto If " + mcguffinname + " Count", 9);
	ld.add("Counter Increase", 10);
	ld.add("Counter Decrease", 11);
	ld.add("Counter Set", 12);
	ld.add("Counter Increase %", 13);
	ld.add("Counter Decrease %", 14);
	ld.add("Counter Set %", 15);
	ld.add("Give Item", 16);
	ld.add("Take Item", 17);
	ld.add("Warp", 18);
	ld.add("Set Screen->D[]", 19);
	ld.add("Play SFX", 20);
	ld.add("Play MIDI", 21);
	ld.add("Insert Player Name", 22);
	//ld.add("Change Portrait", 24);
	ld.add("Line Break", 25);
	ld.add("Shadow Color", 26);
	ld.add("Shadow Type", 27);
	ld.add("Draw Tile", 28);
	ld.add("End String", 29);
	ld.add("Wait For Press A", 30);
	ld.add("Setup Menu Cursor", 128);
	ld.add("Add Menu Choice", 129);
	ld.add("Run Menu", 130);
	ld.add("Goto If Menu Choice", 131);
	ld.add("Trigger Screen Secrets", 132);
	ld.add("Set Current Screen State", 133);
	ld.add("Set Screen State", 134);
	return ld;
}

void SCCDialog::default_args()
{
	args[MSGC_COLOUR][0] = misc.colors.msgtext >> 4;
	args[MSGC_COLOUR][1] = misc.colors.msgtext & 0xF;
	args[MSGC_SPEED][0] = zinit.msg_speed;
	args[MSGC_GOTOIFRAND][0] = 2;
	
	args[MSGC_SHDCOLOR][0] = refstr->shadow_color;
	args[MSGC_SHDTYPE][0] = refstr->shadow_type;
	
	warp_xy_toggle = true;
}

#define NUM_FIELD(v,_min,_max) \
TextField( \
	fitParent = true, maxwidth = sized(5.25_em, 4.5_em), \
	maxheight = 24_lpx, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = v, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		v = val; \
	})

#define HEX_FIELD(v,_min,_max) \
TextField( \
	fitParent = true, maxwidth = sized(5.25_em, 4.5_em), \
	maxheight = 24_lpx, \
	type = GUI::TextField::type::INT_HEX, \
	low = _min, high = _max, val = v, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		v = val; \
	})

#define DDL(v,lister) \
DropDownList(data = lister, \
	fitParent = true, \
	selectedValue = v, \
	onSelectFunc = [&](int32_t val) \
	{ \
		v = val; \
	} \
)

#define TXT(txt) \
Label(text = txt, hAlign = 1.0)

GUI::ListData createShadowTypesListData();
SCCDialog::SCCDialog() :
	list_sccs(SCCListData()),
	list_shtype(createShadowTypesListData()),
	list_items(GUI::ZCListData::items(true)),
	list_counters(GUI::ZCListData::counters(true, true)),
	list_dmaps(dmap_list),
	list_weffect(GUI::ZCListData::warpeffects()),
	list_sfx(GUI::ZCListData::sfxnames(true))
{
	memset(args, 0, sizeof(args));
	default_args();
	curscc = MSGC_COLOUR;
	cur_args = nullptr;
	::ListData msgs_list(msgslist, &font);
	list_strings = GUI::ListData(msgs_list, 0);
	if(!refstr)
	{
		refstr = &def_refstr;
		def_refstr.clear();
	}
}

static const GUI::ListData list_arrivals
{
	{ "A", 0 },
	{ "B", 1 },
	{ "C", 2 },
	{ "D", 3 },
	{ "Pit Warp", 5 }
};
std::string scc_help(byte scc)
{
	std::string mcguffinname(ZI.getItemClassName(itype_triforcepiece));
	switch(scc)
	{
		case MSGC_COLOUR: return "Change the text color of the text after the SCC";
		case MSGC_SPEED: return "Change the text speed after the SCC";
		case MSGC_GOTOIFGLOBAL: return "Switch to another string if an index of the"
			" current screen's 'Screen->D[]' is at least a given value";
		case MSGC_GOTOIFRAND: return "Switch to another string based on random chance";
		case MSGC_GOTOIF: return "Switch to another string if the player owns an item";
		case MSGC_GOTOIFCTR: return "Switch to another string if the player has enough"
			" of a specific counter";
		case MSGC_GOTOIFCTRPC: return "Switch to another string if the player has enough"
			" of a specific counter, percentage-based";
		case MSGC_GOTOIFTRI: return "Switch to another string if the specified level's '"
			+mcguffinname+"' is owned";
		case MSGC_GOTOIFTRICOUNT: return "Switch to another string if the specified number of '"
			+mcguffinname+"' are owned";
		case MSGC_CTRUP: return "Increase a counter by an amount";
		case MSGC_CTRDN: return "Decrease a counter by an amount";
		case MSGC_CTRSET: return "Set a counter to an amount";
		case MSGC_CTRUPPC: return "Increase a counter by a percentage of its' maximum";
		case MSGC_CTRDNPC: return "Decrease a counter by a percentage of its' maximum";
		case MSGC_CTRSETPC: return "Set a counter to a percentage of its' maximum";
		case MSGC_GIVEITEM: return "'Silently' give an item to the player. This will not"
			" cause all normal item pickup effects to occur.";
		case MSGC_TAKEITEM: return "Remove an item from the player. This works similarly to"
			" an enemy eating the item.";
		case MSGC_WARP: return "Warp the player";
		
		case MSGC_NAME: return "Insert the player's save file name in the string";
		
		case MSGC_NEWLINE: return "Add a line break to the string";
		case MSGC_SHDCOLOR: return "Change the shadow color of the text after the SCC";
		case MSGC_SHDTYPE: return "Change the shadow type of the text after the SCC";
	}
	return "";
}
std::shared_ptr<GUI::Widget> SCCDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	window.reset();
	
	std::shared_ptr<GUI::Grid> wingrid, sgrid;
	std::string mcguffinname(ZI.getItemClassName(itype_triforcepiece));
	static std::string scc_helpstr;
	scc_helpstr = scc_help(curscc);
	window = Window(
		title = "Insert SCC",
		minwidth = 30_em,
		onClose = message::CANCEL,
		Column(
			wingrid = Column(padding=0_px,
				Row(padding = 0_px,
					DropDownList(data = list_sccs,
						fitParent = true,
						selectedValue = curscc,
						onSelectionChanged = message::RELOAD,
						onSelectFunc = [&](int32_t val)
						{
							curscc = val;
						}
					),
					Button(forceFitH = true, text = "?",
						disabled = scc_helpstr.empty(),
						onPressFunc = [&]()
						{
							InfoDialog("Info",scc_helpstr).show();
						})
				)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
	bool hasParams = is_msgc(curscc) && msg_code_operands(curscc) > 0;
	cur_args = args[curscc];
	switch(curscc)
	{
		case MSGC_COLOUR:
		{
			sgrid = Row(padding = 0_px, vAlign = 0.0,
				TXT("New Color:"),
				ColorSel(val = (cur_args[0]<<4)|(cur_args[1]),
					onValChangedFunc = [&](byte val)
					{
						cur_args[0] = val>>4;
						cur_args[1] = val&0xF;
					}),
				Button(text = "Default",
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						byte val = misc.colors.msgtext;
						cur_args[0] = val>>4;
						cur_args[1] = val&0xF;
					})
			);
			break;
		}
		case MSGC_SPEED:
		{
			sgrid = Row(padding = 0_px, vAlign = 0.0,
				TXT("New Speed:"),
				NUM_FIELD(cur_args[0],0,255),
				Button(text = "Default",
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						cur_args[0] = zinit.msg_speed;
					})
			);
			break;
		}
		case MSGC_GOTOIFGLOBAL:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Screen->D[] Index:"),
					NUM_FIELD(cur_args[0],0,7),
					INFOBTN("Index of current screen's 'Screen->D[]' to check"),
					TXT("Min Value:"),
					NUM_FIELD(cur_args[1],0,MAX_SCC_ARG),
					INFOBTN("Value to check against 'Screen->D[]'")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[2],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}
		case MSGC_GOTOIFRAND:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Factor:"),
					NUM_FIELD(cur_args[0],0,MAX_SCC_ARG),
					INFOBTN("1-in-N chance of switching strings")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[1],list_strings),
					INFOBTN("String to switch to if random chance met")
				)
			);
			break;
		}
		case MSGC_GOTOIF:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Item:"),
					DDL(cur_args[0],list_items),
					INFOBTN("Switch strings if this item is owned")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[1],list_strings),
					INFOBTN("String to switch to if item is owned")
				)
			);
			break;
		}
		case MSGC_GOTOIFCTR:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Switch strings if enough of this counter is full"),
					TXT("Amount:"),
					NUM_FIELD(cur_args[1],0,MAX_SCC_ARG),
					INFOBTN("Switch strings if at least this much of the counter is full")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[2],list_strings),
					INFOBTN("String to switch to if enough of the counter is full")
				)
			);
			break;
		}
		case MSGC_GOTOIFCTRPC:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Switch strings if enough of this counter is full"),
					TXT("Percent:"),
					NUM_FIELD(cur_args[1],0,100),
					INFOBTN("Switch strings if at least this percent of the counter is full")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[2],list_strings),
					INFOBTN("String to switch to if enough of the counter is full")
				)
			);
			break;
		}
		case MSGC_GOTOIFTRI:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Level:"),
					NUM_FIELD(cur_args[0],0,511),
					INFOBTN("Switch strings if the specified level's '"+mcguffinname+"' is owned")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[1],list_strings),
					INFOBTN("String to switch to if the specified '"+mcguffinname+"' is owned")
				)
			);
			break;
		}
		case MSGC_GOTOIFTRICOUNT:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT(mcguffinname+" Count:"),
					NUM_FIELD(cur_args[0],0,511),
					INFOBTN("Switch strings if at least this many '"+mcguffinname+"' are owned")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[1],list_strings),
					INFOBTN("String to switch to if the specified number of '"+mcguffinname+"' are owned")
				)
			);
			break;
		}
		case MSGC_CTRUP:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Counter to increase"),
					TXT("Amount:"),
					NUM_FIELD(cur_args[1],0,MAX_SCC_ARG),
					INFOBTN("Amount to increase the counter by")
				)
			);
			break;
		}
		case MSGC_CTRDN:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Counter to decrease"),
					TXT("Amount:"),
					NUM_FIELD(cur_args[1],0,MAX_SCC_ARG),
					INFOBTN("Amount to decrease the counter by")
				)
			);
			break;
		}
		case MSGC_CTRSET:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Counter to set"),
					TXT("Amount:"),
					NUM_FIELD(cur_args[1],0,MAX_SCC_ARG),
					INFOBTN("Amount to set the counter to")
				)
			);
			break;
		}
		case MSGC_CTRUPPC:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Counter to increase"),
					TXT("Percentage:"),
					NUM_FIELD(cur_args[1],0,100),
					INFOBTN("Percentage to increase the counter by")
				)
			);
			break;
		}
		case MSGC_CTRDNPC:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Counter to decrease"),
					TXT("Percentage:"),
					NUM_FIELD(cur_args[1],0,100),
					INFOBTN("Percentage to decrease the counter by")
				)
			);
			break;
		}
		case MSGC_CTRSETPC:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Rows<3>(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters),
					INFOBTN("Counter to set"),
					TXT("Percentage:"),
					NUM_FIELD(cur_args[1],0,100),
					INFOBTN("Percentage to set the counter to")
				)
			);
			break;
		}
		case MSGC_GIVEITEM:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Item:"),
					DDL(cur_args[0],list_items),
					INFOBTN("Item to give silently")
				)
			);
			break;
		}
		case MSGC_TAKEITEM:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Item:"),
					DDL(cur_args[0],list_items),
					INFOBTN("Item to take silently")
				)
			);
			break;
		}
		case MSGC_WARP:
		{
			if(warp_xy_toggle)
			{
				sgrid = Column(padding = 0_px, vAlign = 0.0,
					Row(padding = 0_px, hAlign = 1.0,
						TXT("DMap:"),
						DDL(cur_args[0],list_dmaps),
						INFOBTN("DMap to warp to")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("Screen:"),
						HEX_FIELD(cur_args[1],0,255),
						INFOBTN("Screen to warp to (relative to dmap offset)")
					),
					Row(padding = 0_px, hAlign = 1.0,
						Checkbox(text = "Use X/Y",
							checked = warp_xy_toggle,
							onToggle = message::RELOAD,
							onToggleFunc = [&](bool state)
							{
								warp_xy_toggle = state;
								cur_args[2] = state ? 0 : -1;
								cur_args[3] = 0;
							}
						),
						INFOBTN("Use X/Y coordinates instead of arrival point")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("X:"),
						NUM_FIELD(cur_args[2],0,240),
						INFOBTN("The exact X coordinate to warp to"),
						TXT("Y:"),
						NUM_FIELD(cur_args[3],0,160),
						INFOBTN("The exact Y coordinate to warp to")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("Warp Effect:"),
						DDL(cur_args[4],list_weffect),
						INFOBTN("Warp effect to display")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("SFX:"),
						DDL(cur_args[5],list_sfx),
						INFOBTN("SFX to play during the warp")
					)
				);
			}
			else
			{
				sgrid = Column(padding = 0_px, vAlign = 0.0,
					Row(padding = 0_px, hAlign = 1.0,
						TXT("DMap:"),
						DDL(cur_args[0],list_dmaps),
						INFOBTN("DMap to warp to")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("Screen:"),
						HEX_FIELD(cur_args[1],0,255),
						INFOBTN("Screen to warp to (relative to dmap offset)")
					),
					Row(padding = 0_px, hAlign = 1.0,
						Checkbox(text = "Use X/Y",
							checked = warp_xy_toggle,
							onToggle = message::RELOAD,
							onToggleFunc = [&](bool state)
							{
								warp_xy_toggle = state;
								cur_args[2] = state ? 0 : -1;
								cur_args[3] = 0;
							}
						),
						INFOBTN("Use X/Y coordinates instead of arrival point")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("Arrival Point:"),
						DDL(cur_args[3],list_arrivals),
						INFOBTN("Where to arrive from the warp; either a blue return square letter,"
							" or a 'Pit Warp'")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("Warp Effect:"),
						DDL(cur_args[4],list_weffect),
						INFOBTN("Warp effect to display")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("SFX:"),
						DDL(cur_args[5],list_sfx),
						INFOBTN("SFX to play during the warp")
					)
				);
			}
			break;
		}
		// case MSGC_SETSCREEND:
		// case MSGC_SFX:
		// case MSGC_MIDI:
		case MSGC_NAME:
			sgrid = Row(DummyWidget());
			break;
		// case MSGC_GOTOIFCREEND:
		case MSGC_NEWLINE:
			sgrid = Row(DummyWidget());
			break;
		case MSGC_SHDCOLOR:
		{
			sgrid = Row(padding = 0_px, vAlign = 0.0,
				TXT("New Color:"),
				ColorSel(val = (cur_args[0]<<4)|(cur_args[1]),
					onValChangedFunc = [&](byte val)
					{
						cur_args[0] = val>>4;
						cur_args[1] = val&0xF;
					}),
				Button(text = "Default",
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						byte val = refstr->shadow_color;
						cur_args[0] = val>>4;
						cur_args[1] = val&0xF;
					})
			);
			break;
		}
		case MSGC_SHDTYPE:
		{
			sgrid = Row(padding = 0_px, vAlign = 0.0,
				TXT("New Shadow Type:"),
				DropDownList(data = list_shtype,
					fitParent = true,
					selectedValue = cur_args[0],
					onSelectFunc = [&](int32_t val)
					{
						cur_args[0] = val;
					}
				),
				Button(text = "Default",
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						cur_args[0] = refstr->shadow_type;
					})
			);
			break;
		}
		// case MSGC_DRAWTILE:
		// case MSGC_ENDSTRING:
		// case MSGC_WAIT_ADVANCE:
		// case MSGC_SETUPMENU:
		// case MSGC_MENUCHOICE:
		// case MSGC_RUNMENU:
		// case MSGC_GOTOMENUCHOICE:
		// case MSGC_TRIGSECRETS:
		// case MSGC_SETSCREENSTATE:
		// case MSGC_SETSCREENSTATER:
		default:
			sgrid = Row(padding = 0_px, vAlign = 0.0,TXT("WIP!"));
			break;
	}
	wingrid->add(
		Frame(minheight = sized(120_px,200_px), fitParent = true,
			sgrid
		)
	);
	return window;
}

bool SCCDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			rerun_dlg = true;
			return true;
		case message::OK:
			calc_retstr(curscc, args[curscc]);
			return true;

		case message::CANCEL:
		default:
			return true;
	}
}

