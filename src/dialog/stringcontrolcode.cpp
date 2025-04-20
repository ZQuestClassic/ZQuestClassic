#include "stringcontrolcode.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include <sstream>
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "zq/zq_tiles.h"
#include "zinfo.h"
#include "base/misctypes.h"
#include "base/initdata.h"

extern bool saved;
extern ListData dmap_list;
extern bool sorted_fontdd;

void call_geninit_wzrd(zinitdata& start, size_t index);

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

std::string calc_retstr(byte scc, int32_t* args)
{
	if(!is_msgc(scc))
	{
		return "";
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
	return oss.str();
}

const GUI::ListData SCCListData()
{
	std::string mcguffinname(ZI.getItemClassName(itype_triforcepiece));
	GUI::ListData ld;
	ld.add("Text Color", 1);
	ld.add("Text Speed", 2);
	ld.add("Text Font", 135);
	ld.add("Shadow Color", 26);
	ld.add("Shadow Type", 27);
	
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
	ld.add("Play SFX", 20);
	ld.add("Play MIDI", 21);
	
	ld.add("Insert Hero Name", 22);
	ld.add("Line Break", 25);
	ld.add("Draw Tile", 28);
	
	ld.add("End String", 29);
	ld.add("Wait For Press A", 30);
	
	ld.add("Setup Menu Cursor", 128);
	ld.add("Add Menu Choice", 129);
	ld.add("Run Menu", 130);
	ld.add("Goto If Menu Choice", 131);
	
	ld.add("Set Screen->D[]", 19);
	ld.add("Set Current Screen State", 133);
	ld.add("Set Screen State", 134);
	
	ld.add("Run Frozen Generic Script", 136);
	ld.add("Trigger Screen Secrets", 132);
	ld.add("Trigger Combo Copycat", 137);
	return ld;
}

void SCCDialog::default_args()
{
	args[MSGC_COLOUR][0] = QMisc.colors.msgtext >> 4;
	args[MSGC_COLOUR][1] = QMisc.colors.msgtext & 0xF;
	args[MSGC_SPEED][0] = zinit.msg_speed;
	args[MSGC_GOTOIFRAND][0] = 2;
	
	args[MSGC_SHDCOLOR][0] = refstr->shadow_color;
	args[MSGC_SHDTYPE][0] = refstr->shadow_type;
	args[MSGC_FONT][0] = refstr->font;
	
	args[MSGC_TRIG_CMB_COPYCAT][0] = 1;
	
	warp_xy_toggle = true;
}

#define NUM_FIELD(v,_min,_max) \
TextField( \
	fitParent = true, width = 4.5_em, \
	maxheight = 24_px, \
	type = GUI::TextField::type::SWAP_ZSINT_NO_DEC, \
	swap_type = nswapDEC, \
	low = _min, high = _max, val = v, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		v = val; \
	})

#define HEX_FIELD(v,_min,_max) \
TextField( \
	fitParent = true, width = 4.5_em, \
	maxheight = 24_px, \
	type = GUI::TextField::type::SWAP_ZSINT_NO_DEC, \
	swap_type = nswapHEX, \
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
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_midi(GUI::ZCListData::midinames(true)),
	list_screenstate(GUI::ZCListData::screenstate()),
	list_font(GUI::ZCListData::fonts(false,true,true)),
	list_font_order(GUI::ZCListData::fonts(false,true,false)),
	list_genscr(GUI::ZCListData::generic_script())
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
		case MSGC_GOTOIF: return "Switch to another string if the Hero owns an item";
		case MSGC_GOTOIFCTR: return "Switch to another string if the Hero has enough"
			" of a specific counter";
		case MSGC_GOTOIFCTRPC: return "Switch to another string if the Hero has enough"
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
		case MSGC_GIVEITEM: return "'Silently' give an item to the Hero. This will not"
			" cause all normal item pickup effects to occur.";
		case MSGC_TAKEITEM: return "Remove an item from the Hero. This works similarly to"
			" an enemy eating the item.";
		case MSGC_WARP: return "Warp the Hero";
		case MSGC_SETSCREEND: return "Set the value of any screen's 'Screen->D[]'";
		case MSGC_SFX: return "Plays an SFX";
		case MSGC_MIDI: return "Plays a MIDI";
		case MSGC_NAME: return "Insert the Hero's save file name in the string";
		case MSGC_GOTOIFCREEND: return "Switch to another string if an index of a"
			" remote screen's 'Screen->D[]' is at least a given value";
		case MSGC_NEWLINE: return "Add a line break to the string";
		case MSGC_SHDCOLOR: return "Change the shadow color of the text after the SCC";
		case MSGC_SHDTYPE: return "Change the shadow type of the text after the SCC";
		case MSGC_DRAWTILE: return "Draws a tile in the textbox";
		case MSGC_ENDSTRING: return "Immediately exit the current string, going to the 'next string'"
			" if one is set.";
		case MSGC_WAIT_ADVANCE: return "Immediately pause the string until the Hero presses 'A'"
			" to advance the text";
		case MSGC_SETUPMENU: return "Sets the Menu Cursor up as a tile draw";
		case MSGC_MENUCHOICE: return "Adds a menu choice";
		case MSGC_RUNMENU: return "Starts a menu";
		case MSGC_GOTOMENUCHOICE: return "Switch to another string based on menu choice";
		case MSGC_TRIGSECRETS: return "Trigger screen secrets, either temp or perm";
		case MSGC_SETSCREENSTATE: return "Set a state of the current screen";
		case MSGC_SETSCREENSTATER: return "Set a state of any screen";
		case MSGC_FONT: return "Change the text font of text after the SCC";
		case MSGC_RUN_FRZ_GENSCR: return "Run a generic script in the frozen mode.";
		case MSGC_TRIG_CMB_COPYCAT: return "Trigger a combo triggers tab 'Copycat' id";
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
					text = "Copy",
					minwidth = 90_px,
					onClick = message::COPY),
				Button(
					text = "Paste",
					minwidth = 90_px,
					onClick = message::PASTE)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
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
						byte val = QMisc.colors.msgtext;
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
					INFOBTN("Value to check against 'Screen->D[]' (This is a 'long' value, so '1' here represents '0.0001' in zscript)")
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
						Button(text = "Use Arrival Point",
							onClick = message::RELOAD,
							maxheight = 1.5_em,
							onPressFunc = [&]()
							{
								warp_xy_toggle = false;
								cur_args[2] = -1;
								cur_args[3] = 0;
							}
						),
						INFOBTN("Use Arrival Point or Pit Warp instead of X/Y coordinates")
					),
					Row(padding = 0_px, hAlign = 1.0,
						TXT("X:"),
						NUM_FIELD(cur_args[2],0,256*16-16),
						INFOBTN("The exact X coordinate to warp to"),
						TXT("Y:"),
						NUM_FIELD(cur_args[3],0,176*8-16),
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
						Button(text = "Use X/Y Coordinates",
							onClick = message::RELOAD,
							maxheight = 1.5_em,
							onPressFunc = [&]()
							{
								warp_xy_toggle = true;
								cur_args[2] = 0;
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
		case MSGC_SETSCREEND:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("DMap:"),
					DDL(cur_args[0],list_dmaps),
					INFOBTN("DMap to edit")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Screen:"),
					HEX_FIELD(cur_args[1],0,255),
					INFOBTN("Screen to edit (relative to dmap offset)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Register:"),
					NUM_FIELD(cur_args[2],0,7),
					INFOBTN("Screen->D[] register to edit")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Value:"),
					NUM_FIELD(cur_args[3],0,MAX_SCC_ARG),
					INFOBTN("Value to assign to register")
				)
			);
			break;
		}
		case MSGC_SFX:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("SFX:"),
					DDL(cur_args[0], list_sfx),
					INFOBTN("SFX to play")
				)
			);
			break;
		}
		case MSGC_MIDI:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("MIDI:"),
					DDL(cur_args[0], list_midi),
					INFOBTN("MIDI to play")
				)
			);
			break;
		}
		case MSGC_NAME:
			sgrid = Row(DummyWidget());
			break;
		case MSGC_GOTOIFCREEND:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("DMap:"),
					DDL(cur_args[0],list_dmaps),
					INFOBTN("DMap to check")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Screen:"),
					HEX_FIELD(cur_args[1],0,255),
					INFOBTN("Screen to check (relative to dmap offset)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Register:"),
					NUM_FIELD(cur_args[2],0,7),
					INFOBTN("Screen->D[] register to check")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Value:"),
					NUM_FIELD(cur_args[3],0,MAX_SCC_ARG),
					INFOBTN("Value to check against 'Screen->D[]' (This is a 'long' value, so '1' here represents '0.0001' in zscript)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[4],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}
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
		case MSGC_DRAWTILE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Tile/CSet:"),
					SelTileSwatch(
						tile = cur_args[0],
						cset = cur_args[1],
						flip = cur_args[4],
						showFlip = true,
						showvals = true,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t f,int32_t)
						{
							cur_args[0] = t;
							cur_args[1] = c;
							cur_args[4] = f;
						}
					),
					INFOBTN("Tile/CSet/Flip to draw")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Width (pixels):"),
					NUM_FIELD(cur_args[2],0,255),
					INFOBTN("Width of the draw, in pixels")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Height (pixels):"),
					NUM_FIELD(cur_args[3],0,255),
					INFOBTN("Height of the draw, in pixels")
				)
			);
			break;
		}
		case MSGC_ENDSTRING:
			sgrid = Row(DummyWidget());
			break;
		case MSGC_WAIT_ADVANCE:
			sgrid = Row(DummyWidget());
			break;
		case MSGC_SETUPMENU:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Tile/CSet:"),
					SelTileSwatch(
						tile = cur_args[0],
						cset = cur_args[1],
						flip = cur_args[4],
						showFlip = true,
						showvals = true,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t f,int32_t)
						{
							cur_args[0] = t;
							cur_args[1] = c;
							cur_args[4] = f;
						}
					),
					INFOBTN("Tile/CSet/Flip to use for menu cursor")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Width (pixels):"),
					NUM_FIELD(cur_args[2],0,255),
					INFOBTN("Width of the menu cursor, in pixels")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Height (pixels):"),
					NUM_FIELD(cur_args[3],0,255),
					INFOBTN("Height of the menu cursor, in pixels")
				)
			);
			break;
		}
		case MSGC_MENUCHOICE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Position:"),
					NUM_FIELD(cur_args[0],0,MAX_SCC_ARG),
					INFOBTN("Menu Cursor Position")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Up Position:"),
					NUM_FIELD(cur_args[1],0,MAX_SCC_ARG),
					INFOBTN("Position to move to when 'Up' is pressed")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Down Position:"),
					NUM_FIELD(cur_args[2],0,MAX_SCC_ARG),
					INFOBTN("Position to move to when 'Down' is pressed")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Left Position:"),
					NUM_FIELD(cur_args[3],0,MAX_SCC_ARG),
					INFOBTN("Position to move to when 'Left' is pressed")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Right Position:"),
					NUM_FIELD(cur_args[4],0,MAX_SCC_ARG),
					INFOBTN("Position to move to when 'Right' is pressed")
				)
			);
			break;
		}
		case MSGC_RUNMENU:
			sgrid = Row(DummyWidget());
			break;
		case MSGC_GOTOMENUCHOICE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Choice:"),
					NUM_FIELD(cur_args[0],0,MAX_SCC_ARG),
					INFOBTN("The menu choice to check against")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[1],list_strings),
					INFOBTN("String to switch to if the chosen choice was chosen")
				)
			);
			break;
		}
		case MSGC_TRIGSECRETS:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Checkbox(text = "Permanent",
					checked = cur_args[0]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[0] = state?1:0;
					}
				)
			);
			break;
		}
		case MSGC_SETSCREENSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("State:"),
					DDL(cur_args[0],list_screenstate),
					INFOBTN("The screen state to set")
				),
				Checkbox(text = "On",
					checked = cur_args[1]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[1] = state?1:0;
					}
				)
			);
			break;
		}
		case MSGC_SETSCREENSTATER:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("DMap:"),
					NUM_FIELD(cur_args[0],0,255),
					INFOBTN("Map to modify")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Screen:"),
					HEX_FIELD(cur_args[1],0,255),
					INFOBTN("Screen to modify")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("State:"),
					DDL(cur_args[2],list_screenstate),
					INFOBTN("The screen state to set")
				),
				Checkbox(text = "On",
					checked = cur_args[3]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[3] = state?1:0;
					}
				)
			);
			break;
		}
		case MSGC_FONT:
		{
			sgrid = Row(padding = 0_px, vAlign = 0.0,
				TXT("New Font:"),
				fontlist = DropDownList(data = sorted_fontdd ? list_font : list_font_order,
						fitParent = true,
						selectedValue = cur_args[0],
						onSelectFunc = [&](int32_t val)
						{
							cur_args[0] = val;
						}
					),
				Checkbox(
					text = "Font Sort",
					checked = sorted_fontdd,
					onToggleFunc = [&](bool state)
					{
						sorted_fontdd = !sorted_fontdd;
						fontlist->setListData(sorted_fontdd ? list_font : list_font_order);
						zc_set_config("zquest","stringed_sorted_font",sorted_fontdd?1:0);
					}
				),
				Button(text = "Default",
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						cur_args[0] = refstr->font;
					})
			);
			break;
		}
		case MSGC_RUN_FRZ_GENSCR:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(
					TXT("Script to Run:"),
					DropDownList(data = list_genscr,
							fitParent = true,
							selectedValue = cur_args[0],
							onSelectFunc = [&](int32_t val)
							{
								cur_args[0] = val;
								miscbtn->setDisabled(!val);
							}
						)
				),
				miscbtn = Button(text = "Edit Script InitData",
					disabled = !cur_args[0],
					onPressFunc = [&]()
					{
						if(int indx = cur_args[0])
							call_geninit_wzrd(zinit,indx);
					}),
				Checkbox(text = "Force Redraw",
					checked = cur_args[1]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[1] = state?1:0;
					})
			);
			break;
		}
		case MSGC_TRIG_CMB_COPYCAT:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(
					TXT("Copycat ID to trigger:"),
					NUM_FIELD(cur_args[0],1,255)
				)
			);
			break;
		}
		default:
			sgrid = Row(padding = 0_px, vAlign = 0.0,TXT("WIP!"));
			break;
	}
	wingrid->add(
		Frame(minheight = 180_px, fitParent = true,
			sgrid
		)
	);
	return window;
}

bool SCCDialog::load_scc_str(std::string const& str)
{
	size_t q = 0;
	byte scc;
	int32_t t_args[6];
	int cur_arg = -1;
	int limit = 6;
	while(q < str.size() && cur_arg < limit)
	{
		if(str.at(q) == '\\') //SCC escape slash
		{
			size_t ind = 0;
			char buf[8] = {0};
			++q;
			while(q < str.size())
			{
				char c = str.at(q);
				bool cont = false;
				switch(c)
				{
					case '-': case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7': case '8':
					case '9':
						buf[ind++] = c;
						buf[ind] = 0;
						cont = ind<6;
						++q;
						break;
				}
				if(!cont) break;
			}
			int32_t val = atoi(buf);
			//if(val < 0) val = MAX_SCC_ARG;
			
			if(cur_arg < 0)
			{
				scc = byte(val);
				limit = msg_code_operands(scc);
				++cur_arg;
			}
			else
			{
				t_args[cur_arg++] = val;
			}
		}
		else break;
	}
	if(cur_arg < 0) return false; //no scc
	if(cur_arg < limit) return false; //missing params
	
	curscc = scc;
	for(q = 0; q < limit; ++q)
	{
		args[scc][q] = t_args[q];
	}
	switch(scc)
	{
		case MSGC_WARP:
		{
			warp_xy_toggle = unsigned(t_args[2]) < MAX_SCC_ARG;
		}
	}
	return true;
}

bool SCCDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			rerun_dlg = true;
			return true;
		case message::COPY:
			set_al_clipboard(calc_retstr(curscc, args[curscc]));
			return false;
		case message::PASTE:
		{
			std::string cb;
			if(get_al_clipboard(cb) && load_scc_str(cb))
			{
				rerun_dlg = true;
				return true;
			}
			return false;
		}
		case message::OK:
			retstr = calc_retstr(curscc, args[curscc]);
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}

