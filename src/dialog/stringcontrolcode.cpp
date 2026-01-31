#include "stringcontrolcode.h"
#include "base/scc.h"
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

extern bool sorted_fontdd;

void call_geninit_wzrd(zinitdata& start, size_t index);

static std::string retstr;
static MsgStr const* refstr = nullptr;
static int command_index = -1;
static MsgStr def_refstr;

int32_t msg_code_operands(byte cc);
bool is_msgc(byte cc);
std::string run_scc_dlg(MsgStr const* ref, int current_scc_index)
{
	if(ref)
		refstr = ref;
	else refstr = nullptr;
	retstr = "";
	command_index = current_scc_index;
	SCCDialog().show();
	return retstr;
}

std::string calc_retstr(byte scc, int32_t* args)
{
	if(!is_msgc(scc))
		return "";

	std::ostringstream oss;
	oss << "\\" << get_scc_command_name(scc).value();
	auto count = msg_code_operands(scc);
	for(auto q = 0; q < count; ++q)
	{
		oss << "\\" << args[q];
	}
	oss << "\\ ";
	return oss.str();
}

static void addCommand(GUI::ListData& ld, int code)
{
	ld.add(get_scc_command_name(code).value(), code);
}

const GUI::ListData SCCListData()
{
	GUI::ListData ld;
	addCommand(ld, MSGC_COLOUR); // Text Color
	addCommand(ld, MSGC_SPEED); // Text Speed
	addCommand(ld, MSGC_DELAY);
	addCommand(ld, MSGC_FORCE_DELAY);
	addCommand(ld, MSGC_FONT); // Text Font
	addCommand(ld, MSGC_SHDCOLOR); // Shadow Color
	addCommand(ld, MSGC_SHDTYPE); // Shadow Type
	addCommand(ld, MSGC_CHANGEPORTRAIT); // Portrait
	
	addCommand(ld, MSGC_GOTOIFCREEND); // Go If Screen->D[]
	addCommand(ld, MSGC_GOTOIFSCREEND); // Go If Current Screen->D[]
	addCommand(ld, MSGC_GOTOIFRAND); // Go If Random
	addCommand(ld, MSGC_GOTOIFITEM); // Go If Item
	addCommand(ld, MSGC_GOTOIFCTR); // Go If Counter >=
	addCommand(ld, MSGC_GOTOIFCTRPC); // Go If Counter >= %
	addCommand(ld, MSGC_GOTOIFTRI); // Go If McGuffin
	addCommand(ld, MSGC_GOTOIFTRICOUNT); // Go If McGuffin Count
	addCommand(ld, MSGC_GOTOIFGLOBALSTATE); // Go If Global State
	addCommand(ld, MSGC_GOTOIFSCREENSTATE); // Go If Screen State
	addCommand(ld, MSGC_GOTOIFANYSCREENSTATE); // Go If Any Screen State
	addCommand(ld, MSGC_GOTOIFSCREENEXSTATE);
	addCommand(ld, MSGC_GOTOIFANYSCREENEXSTATE);
	addCommand(ld, MSGC_GOTOIFLEVELSTATE); // Go If Level State
	addCommand(ld, MSGC_GOTOIFLEVELITEM); // Go If Level Item
	addCommand(ld, MSGC_GOTOIFSECRETS); // Go If Secrets
	addCommand(ld, MSGC_GOTOIFBOTTLE); // Go If Bottle

	addCommand(ld, MSGC_CTRUP); // Counter Increase
	addCommand(ld, MSGC_CTRDN); // Counter Decrease
	addCommand(ld, MSGC_CTRSET); // Counter Set
	addCommand(ld, MSGC_CTRUPPC); // Counter Increase %
	addCommand(ld, MSGC_CTRDNPC); // Counter Decrease %
	addCommand(ld, MSGC_CTRSETPC); // Counter Set %
	
	addCommand(ld, MSGC_COLLECTITEM); // Collect Item
	addCommand(ld, MSGC_GIVEITEM); // Give Item
	addCommand(ld, MSGC_TAKEITEM); // Take Item
	
	addCommand(ld, MSGC_CHANGEBOTTLE); // Change Bottle
	
	addCommand(ld, MSGC_WARP); // Warp
	addCommand(ld, MSGC_SFX); // Play SFX
	addCommand(ld, MSGC_MIDI); // Play MIDI
	addCommand(ld, MSGC_MUSIC); // Play Music
	addCommand(ld, MSGC_MUSIC_REFRESH); // Change MusicRefresh
	
	addCommand(ld, MSGC_NAME); // Insert Hero Name
	addCommand(ld, MSGC_COUNTER); // Insert Counter
	addCommand(ld, MSGC_MAXCOUNTER); // Insert Max Counter
	addCommand(ld, MSGC_NEWLINE); // Line Break
	addCommand(ld, MSGC_DRAWTILE); // Draw Tile
	
	addCommand(ld, MSGC_ENDSTRING); // End String
	addCommand(ld, MSGC_WAIT_ADVANCE); // Wait For Press A
	
	addCommand(ld, MSGC_SETUPMENU); // Setup Menu Cursor
	addCommand(ld, MSGC_MENUCHOICE); // Add Menu Choice
	addCommand(ld, MSGC_RUNMENU); // Run Menu
	addCommand(ld, MSGC_GOTOMENUCHOICE); // Go If Menu Choice
	
	addCommand(ld, MSGC_SETSCREEND); // Set Screen->D[]
	addCommand(ld, MSGC_SETSCREENSTATE); // Set Current Screen State
	addCommand(ld, MSGC_SETSCREENSTATER); // Set Screen State
	addCommand(ld, MSGC_SETSCREENEXSTATE); // Set Current Screen ExState
	addCommand(ld, MSGC_SETANYSCREENEXSTATE); // Set Screen ExState
	addCommand(ld, MSGC_SETGLOBALSTATE); // Set Global State
	addCommand(ld, MSGC_SETLEVELSTATE); // Set Level State
	addCommand(ld, MSGC_SETLEVELITEM); // Set Level Item
	
	addCommand(ld, MSGC_RUN_FRZ_GENSCR); // Run Frozen Generic Script
	addCommand(ld, MSGC_TRIGSECRETS); // Trigger Screen Secrets
	addCommand(ld, MSGC_TRIG_CMB_COPYCAT); // Trigger Combo Copycat
	addCommand(ld, MSGC_KILLHERO); // Kill Hero
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

	args[MSGC_DELAY][0] = 1;
	args[MSGC_FORCE_DELAY][0] = 1;

	args[MSGC_GOTOIFLEVELITEM][0] = -1;
	args[MSGC_GOTOIFLEVELSTATE][0] = -1;
	args[MSGC_SETLEVELITEM][0] = -1;
	args[MSGC_SETLEVELSTATE][0] = -1;
	
	args[MSGC_GOTOIFBOTTLE][0] = 1;
	args[MSGC_CHANGEBOTTLE][0] = 1;
	
	args[MSGC_CHANGEPORTRAIT][0] = refstr->portrait_tile;
	args[MSGC_CHANGEPORTRAIT][1] = refstr->portrait_cset;
	args[MSGC_CHANGEPORTRAIT][2] = refstr->portrait_x;
	args[MSGC_CHANGEPORTRAIT][3] = refstr->portrait_y;
	args[MSGC_CHANGEPORTRAIT][4] = refstr->portrait_tw;
	args[MSGC_CHANGEPORTRAIT][5] = refstr->portrait_th;

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
#define DDL_MW(v,lister,maxw) \
DropDownList(data = lister, \
	fitParent = true, maxwidth = maxw, \
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
	list_dmaps(GUI::ZCListData::dmaps(true)),
	list_weffect(GUI::ZCListData::warpeffects()),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_midi(GUI::ZCListData::midinames(true)),
	list_screenstate(GUI::ZCListData::screenstate()),
	list_level_items(GUI::ZCListData::level_items()),
	list_font(GUI::ZCListData::fonts(false,true,true)),
	list_font_order(GUI::ZCListData::fonts(false,true,false)),
	list_genscr(GUI::ZCListData::generic_script()),
	list_bottletypes(GUI::ZCListData::bottletype()),
	list_strings(GUI::ZCListData::strings())
{
	memset(args, 0, sizeof(args));

	if (command_index == -1)
	{
		curscc = MSGC_COLOUR;
		default_args();
	}
	else
	{
		auto& command = refstr->parsed_msg_str.commands[command_index];
		curscc = command.code;
		for (int i = 0; i < command.num_args; i++) args[command.code][i] = command.args[i];
	}

	cur_args = nullptr;
	if(!refstr)
	{
		refstr = &def_refstr;
		def_refstr.clear();
	}
	refresh_music_list();
}

static const GUI::ListData list_music_refresh
{
	{ "Screen", MUSIC_UPDATE_SCREEN },
	{ "DMap", MUSIC_UPDATE_DMAP },
	{ "Level", MUSIC_UPDATE_LEVEL },
	{ "Never", MUSIC_UPDATE_NEVER },
	{ "Region", MUSIC_UPDATE_REGION },
};
static const GUI::ListData list_arrivals
{
	{ "A", 0 },
	{ "B", 1 },
	{ "C", 2 },
	{ "D", 3 },
	{ "Pit Warp", 5 }
};

void SCCDialog::refresh_music_list()
{
	list_music = GUI::ZCListData::music_names(true, false);
	list_music.accessItem(0).text = "(Stop Music) (000)";
	list_music.add("(Play Screen/DMap Music) (-001)", -1);
	list_music.valsort(0);
}

std::string scc_help(byte scc)
{
	std::string mcguffinname(ZI.getItemClassName(itype_triforcepiece));
	switch(scc)
	{
		case MSGC_COLOUR: return "Change the text color of the text after the SCC";
		case MSGC_SPEED: return "Change the text speed after the SCC";
		case MSGC_GOTOIFSCREEND: return "Switch to another string if an index of the"
			" current screen's 'Screen->D[]' is at least a given value";
		case MSGC_GOTOIFRAND: return "Switch to another string based on random chance";
		case MSGC_GOTOIFITEM: return "Switch to another string if the Hero owns an item";
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
		case MSGC_COLLECTITEM: return "The Hero collects an item. The item will be held up by the player when the message box closes.";
		case MSGC_TAKEITEM: return "Remove an item from the Hero. This works similarly to"
			" an enemy eating the item.";
		case MSGC_WARP: return "Warp the Hero";
		case MSGC_SETSCREEND: return "Set the value of any screen's 'Screen->D[]'";
		case MSGC_SFX: return "Plays an SFX";
		case MSGC_MIDI: return "Plays a MIDI";
		case MSGC_MUSIC: return "Plays a Music";
		case MSGC_MUSIC_REFRESH: return "Changes the current music refresh mode."
			" This determines when the engine will try to re-play the current screen's music next."
			"\nThe mode will reset itself to 'Screen' the next time it refreshes.";
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
		case MSGC_CHANGEPORTRAIT: return "Change the current portrait";
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
		case MSGC_GOTOIFGLOBALSTATE: return "Switch to another string if global state is set";
		case MSGC_GOTOIFSCREENSTATE: return "Switch to another string if current screen state is set";
		case MSGC_GOTOIFANYSCREENSTATE: return "Switch to another string if screen state is set";
		case MSGC_GOTOIFLEVELSTATE: return "Switch to another string if level state is set";
		case MSGC_GOTOIFLEVELITEM: return "Switch to another string if level item is set";
		case MSGC_GOTOIFSECRETS: return "Switch to another string if current screen's secrets have triggered";
		case MSGC_SETGLOBALSTATE: return "Set global state";
		case MSGC_SETLEVELSTATE: return "Set level state";
		case MSGC_SETLEVELITEM: return "Set level item";
		case MSGC_COUNTER: return "Insert the counter value in the string";
		case MSGC_MAXCOUNTER: return "Insert the max counter value in the string";
		case MSGC_KILLHERO: return "Kill hero";
		case MSGC_GOTOIFSCREENEXSTATE: return "Switch to another string if current screen ExState is set";
		case MSGC_SETSCREENEXSTATE: return "Set current screen ExState";
		case MSGC_GOTOIFANYSCREENEXSTATE: return "Switch to another string if screen ExState is set";
		case MSGC_SETANYSCREENEXSTATE: return "Set screen ExState";
		case MSGC_DELAY: return "Delay the string processing by a number of frames (respecting A/B button speed up)";
		case MSGC_FORCE_DELAY: return "Delay the string processing by a number of frames (ignoring A/B button speed up)";
		case MSGC_GOTOIFBOTTLE: return "Switch to another string if the Hero has enough of a specific bottle fill type.";
		case MSGC_CHANGEBOTTLE: return "Change some number of bottles from one specific fill type to another."
			" If the Hero does not have enough, as many as possible will be changed.";
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
		case MSGC_DELAY:
		case MSGC_FORCE_DELAY:
		{
			sgrid = Row(padding = 0_px, vAlign = 0.0,
				TXT("Delay (frames):"),
				NUM_FIELD(cur_args[0], 1, MAX_SCC_ARG)
			);
			break;
		}
		case MSGC_GOTOIFSCREEND:
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
		case MSGC_GOTOIFITEM:
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
					NUM_FIELD(cur_args[1],MIN_SCC_ARG,MAX_SCC_ARG),
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
					NUM_FIELD(cur_args[1],MIN_SCC_ARG,MAX_SCC_ARG),
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
					NUM_FIELD(cur_args[1],MIN_SCC_ARG,MAX_SCC_ARG),
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
					NUM_FIELD(cur_args[1],MIN_SCC_ARG,MAX_SCC_ARG),
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
		case MSGC_COLLECTITEM:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Item:"),
					DDL(cur_args[0],list_items),
					INFOBTN("Item to collect and hold up")
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
		case MSGC_GOTOIFBOTTLE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Quantity:"),
					NUM_FIELD(cur_args[0],1,NUM_BOTTLE_SLOTS),
					INFOBTN("How many of the specified fill to require. (Minimum 1)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Bottle Fill:"),
					DDL(cur_args[1],list_bottletypes),
					INFOBTN("Require this type of bottle fill to switch strings.")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[2],list_strings),
					INFOBTN("String to switch to if the player owns at least the specified number of the specified bottle fill type.")
				)
			);
			break;
		}
		case MSGC_CHANGEBOTTLE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Quantity:"),
					NUM_FIELD(cur_args[0],1,NUM_BOTTLE_SLOTS),
					INFOBTN("How many fills to (try to) change. (Minimum 1)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Old Bottle Fill:"),
					DDL(cur_args[1],list_bottletypes),
					INFOBTN("The type of fill to replace.")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("New Bottle Fill:"),
					DDL(cur_args[2],list_bottletypes),
					INFOBTN("The type of fill to replace with.")
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
					NUM_FIELD(cur_args[3],MIN_SCC_ARG,MAX_SCC_ARG),
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
					DDL_MW(cur_args[0], list_midi, 30_em),
					INFOBTN("MIDI to play")
				)
			);
			break;
		}
		case MSGC_MUSIC:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Music:"),
					DDL(cur_args[0], list_music),
					INFOBTN("Music to play"),
					Button(text = "Edit Music",
						forceFitH = true, padding = 0_px,
						onPressFunc = [&]()
						{
							call_music_dialog(cur_args[0]);
							refresh_music_list();
							refresh_dlg();
						})
				)
			);
			break;
		}
		case MSGC_MUSIC_REFRESH:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Set Music Refresh:"),
					DDL(cur_args[0], list_music_refresh),
					INFOBTN("When the engine tries to reload the current screen's music setting.")
				)
			);
			break;
		}
		case MSGC_NAME:
			sgrid = Row(DummyWidget());
			break;
		case MSGC_COUNTER:
		case MSGC_MAXCOUNTER:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Counter:"),
					DDL(cur_args[0],list_counters)
				)
			);
			break;
		}

		case MSGC_SETGLOBALSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("State:"),
					NUM_FIELD(cur_args[0], 0, NUM_GSWITCHES - 1),
					INFOBTN("The global state to set")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Value:"),
					NUM_FIELD(cur_args[1], -1, MAX_SCC_ARG),
					INFOBTN("The global state to set. -1 means 'enabled', 0 means 'disabled', and >0 means 'enabled for n frames'")
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

		case MSGC_SETLEVELSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Level:"),
					NUM_FIELD(cur_args[0], -1, 31),
					INFOBTN("The level to set (-1 is current level)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("State:"),
					NUM_FIELD(cur_args[1], 0, 31),
					INFOBTN("The level state to set")
				),
				Checkbox(text = "On",
					checked = cur_args[2]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[2] = state?1:0;
					}
				)
			);
			break;
		}

		case MSGC_SETLEVELITEM:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Level:"),
					NUM_FIELD(cur_args[0], -1, 31),
					INFOBTN("The level to set (-1 is current level)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Item:"),
					DDL(cur_args[1], list_level_items),
					INFOBTN("The level item to set")
				),
				Checkbox(text = "On",
					checked = cur_args[2]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[2] = state?1:0;
					}
				)
			);
			break;
		}

		case MSGC_GOTOIFSECRETS:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[0],list_strings),
					INFOBTN("String to switch to if secrets have triggered on current screen")
				)
			);
			break;
		}

		case MSGC_GOTOIFGLOBALSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("State:"),
					NUM_FIELD(cur_args[0], 0, NUM_GSWITCHES - 1),
					INFOBTN("The global state to check")
				),
				Checkbox(text = "On",
					checked = cur_args[1]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[1] = state?1:0;
					}
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[2],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}

		case MSGC_GOTOIFLEVELSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Level:"),
					NUM_FIELD(cur_args[0], -1, 31),
					INFOBTN("The level to check (-1 is current level)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("State:"),
					NUM_FIELD(cur_args[1], 0, 31),
					INFOBTN("The level state to check")
				),
				Checkbox(text = "On",
					checked = cur_args[2]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[2] = state?1:0;
					}
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[3],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}

		case MSGC_GOTOIFLEVELITEM:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Level:"),
					NUM_FIELD(cur_args[0], -1, 31),
					INFOBTN("The level to check (-1 is current level)")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Item:"),
					DDL(cur_args[1], list_level_items),
					INFOBTN("The level item to check")
				),
				Checkbox(text = "On",
					checked = cur_args[2]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[2] = state?1:0;
					}
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[3],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}

		case MSGC_GOTOIFSCREENSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("State:"),
					DDL(cur_args[0],list_screenstate),
					INFOBTN("The screen state to check")
				),
				Checkbox(text = "On",
					checked = cur_args[1]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[1] = state?1:0;
					}
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[2],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}
		case MSGC_GOTOIFANYSCREENSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Map:"),
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
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[4],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}

		case MSGC_GOTOIFSCREENEXSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("ExState:"),
					NUM_FIELD(cur_args[0],0,31),
					INFOBTN("The screen state to check")
				),
				Checkbox(text = "On",
					checked = cur_args[1]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[1] = state?1:0;
					}
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[2],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}

		case MSGC_GOTOIFANYSCREENEXSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Map:"),
					NUM_FIELD(cur_args[0],0,255),
					INFOBTN("Map to modify")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Screen:"),
					HEX_FIELD(cur_args[1],0,255),
					INFOBTN("Screen to modify")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("ExState:"),
					NUM_FIELD(cur_args[2],0,31),
					INFOBTN("The screen state to check")
				),
				Checkbox(text = "On",
					checked = cur_args[3]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[3] = state?1:0;
					}
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("String:"),
					DDL(cur_args[4],list_strings),
					INFOBTN("String to switch to if condition met")
				)
			);
			break;
		}

		case MSGC_KILLHERO:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Checkbox(text = "Bypass revive (fairies)",
					checked = cur_args[0]!=0,
					onToggleFunc = [&](bool state)
					{
						cur_args[0] = state?1:0;
					}
				)
			);
			break;
		}

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
					NUM_FIELD(cur_args[3],MIN_SCC_ARG,MAX_SCC_ARG),
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
		case MSGC_CHANGEPORTRAIT:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Tile/CSet:"),
					SelTileSwatch(
						tile = cur_args[0],
						cset = cur_args[1],
						showFlip = true,
						showvals = true,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t)
						{
							cur_args[0] = t;
							cur_args[1] = c;
						}
					),
					INFOBTN("Tile/CSet of the portrait")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("X (pixels):"),
					NUM_FIELD(cur_args[2],0,255),
					INFOBTN("X of the portrait, in pixels")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Y (pixels):"),
					NUM_FIELD(cur_args[3],0,255),
					INFOBTN("Y of the portrait, in pixels")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Width (tiles):"),
					NUM_FIELD(cur_args[4],0,255),
					INFOBTN("Width of the portrait, in tiles")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Height (tiles):"),
					NUM_FIELD(cur_args[5],0,255),
					INFOBTN("Height of the portrait, in tiles")
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
					TXT("Map:"),
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

		case MSGC_SETSCREENEXSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("ExState:"),
					NUM_FIELD(cur_args[0],0,31),
					INFOBTN("The screen ExState to set")
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

		case MSGC_SETANYSCREENEXSTATE:
		{
			sgrid = Column(padding = 0_px, vAlign = 0.0,
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Map:"),
					NUM_FIELD(cur_args[0],0,255),
					INFOBTN("Map to modify")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("Screen:"),
					HEX_FIELD(cur_args[1],0,255),
					INFOBTN("Screen to modify")
				),
				Row(padding = 0_px, hAlign = 1.0,
					TXT("ExState:"),
					NUM_FIELD(cur_args[2],0,31),
					INFOBTN("The screen ExState to set")
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
	auto [parsed_msg_str, warnings] = parse_ascii_msg_str(str);

	if (warnings.size())
	{
		al_trace("Warning: found message string with SCC warnings: %s\n", parsed_msg_str.serialize().c_str());
		for (auto& error : warnings)
			al_trace("\t%s\n", error.c_str());
	}

	if (parsed_msg_str.commands.size() != 1)
		return false;

	auto& cmd = parsed_msg_str.commands[0];
	int scc = cmd.code;

	for (int i = 0; i < cmd.num_args; i++)
		args[scc][i] = cmd.args[i];

	switch (scc)
	{
		case MSGC_WARP:
		{
			warp_xy_toggle = cmd.args[2] >= MIN_SCC_ARG && cmd.args[2] <= MAX_SCC_ARG;
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

