#include "screen_data.h"
#include <gui/builder.h>
#include "base/mapscr.h"
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "alert.h"
#include "zc_list_data.h"
#include <fmt/format.h>
#include <sstream>
#include "base/initdata.h"
#include "numpick.h"

extern word map_count;
extern script_data *screenscripts[NUMSCRIPTSCREEN];
extern char *guy_string[eMAXGUYS];
extern const char *screen_midi_string[MAXCUSTOMMIDIS_ZQ+1];

static size_t screendata_tab = 0;
void call_screendata_dialog()
{
	ScreenDataDialog(Map.getCurrMap(), Map.getCurrScr()).show();
}
void call_screendata_dialog(size_t forceTab)
{
	screendata_tab = forceTab;
	call_screendata_dialog();
}

ScreenDataDialog::ScreenDataDialog(int map, int scr) :
	mapscrnum(map*MAPSCRS+scr),
	list_screenscript(GUI::ZCListData::screen_script()),
	list_maps(GUI::ListData::numbers(true, 1, map_count)),
	list_screens(GUI::ListData::numbers(false, 0, 0x80, [](int v)
	{
		return fmt::format("0x{0:02X} ({0:03})",v);
	})),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_screenmidi(MAXCUSTOMMIDIS_ZQ+1,
		[](size_t ind){return screen_midi_string[ind];},
		[](size_t ind){return int32_t(ind)-1;})
{
	mapscr* thescreen = Map.AbsoluteScr(map,scr);
	thescr = thescreen;
	local_scr = *thescreen;
	screen_misc_data = zinit.screen_data[mapscrnum];
}

std::shared_ptr<GUI::Widget> ScreenDataDialog::SCREEN_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		l_initds[index] = Label(minwidth = 12_em, textAlign = 2),
		ib_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info",h_initd[index]).show();
			}),
		tf_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			val = local_scr.screeninitd[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_scr.screeninitd[index] = val;
			})
	);
}

static const GUI::ListData list_lenseff
{
	{ "Normal", 0 },
	{ "Hide Layer 1", llLENSHIDES|0 },
	{ "Hide Layer 2", llLENSHIDES|1 },
	{ "Hide Layer 3", llLENSHIDES|2 },
	{ "Hide Layer 4", llLENSHIDES|3 },
	{ "Hide Layer 5", llLENSHIDES|4 },
	{ "Hide Layer 6", llLENSHIDES|5 },
	{ "Reveal Layer 1", llLENSSHOWS|0 },
	{ "Reveal Layer 2", llLENSSHOWS|1 },
	{ "Reveal Layer 3", llLENSSHOWS|2 },
	{ "Reveal Layer 4", llLENSSHOWS|3 },
	{ "Reveal Layer 5", llLENSSHOWS|4 },
	{ "Reveal Layer 6", llLENSSHOWS|5 }
};

#define SCR_CB(member, flag, cspan, txt, inf) \
INFOBTN(inf), \
Checkbox(checked = local_scr.member&flag, \
	text = txt, fitParent = true, \
	colSpan = cspan, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_scr.member, flag, state); \
	})

#define SCR_CB_D(member, flag, cspan, txt) \
DINFOBTN(), \
Checkbox(checked = local_scr.member&flag, \
	text = txt, fitParent = true, \
	colSpan = cspan, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_scr.member, flag, state); \
	})

#define PLAIN_CB(member,flag,...) \
Checkbox(checked = local_scr.member&flag __VA_OPT__(,) __VA_ARGS__, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_scr.member, flag, state); \
	})

#define LENS_CBS(ind) \
lens_cb[0][ind] = Checkbox(checked = local_scr.lens_show&(1<<ind), \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_scr.lens_show, (1<<ind), state); \
		if(state) \
		{ \
			local_scr.lens_hide &= ~(1<<ind); \
			lens_cb[1][ind]->setChecked(false); \
		} \
	}), \
lens_cb[1][ind] = Checkbox(checked = local_scr.lens_hide&(1<<ind), \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_scr.lens_hide, (1<<ind), state); \
		if(state) \
		{ \
			local_scr.lens_show &= ~(1<<ind); \
			lens_cb[0][ind]->setChecked(false); \
		} \
	})

void ScreenDataDialog::refreshScript()
{
	std::string label[8], help[8];
	for(auto q = 0; q < 8; ++q)
	{
		label[q] = "InitD["+std::to_string(q)+"]";
	}
	if(local_scr.script)
	{
		zasm_meta const& meta = screenscripts[local_scr.script]->meta;
		for(size_t q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				label[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				help[q] = meta.initd_help[q];
		}
		
		for(auto q = 0; q < 8; ++q)
		{
			if(unsigned(meta.initd_type[q]) < nswapMAX)
				tf_initd[q]->setSwapType(meta.initd_type[q]);
		}
	}
	else
	{
		for(auto q = 0; q < 8; ++q)
		{
			tf_initd[q]->setSwapType(nswapDEC);
		}
	}
	for(auto q = 0; q < 8; ++q)
	{
		l_initds[q]->setText(label[q]);
		h_initd[q] = help[q];
		ib_initds[q]->setDisabled(help[q].empty());
	}
}

void ScreenDataDialog::refreshTWarp()
{
	int s100 = local_scr.timedwarptics*100/60;
	int remtick = local_scr.timedwarptics%60;
	if(!local_scr.timedwarptics)
		twarp_lbl->setText("No Timed Warp\n");
	else
	{
		std::ostringstream oss;
		int m = (s100/100)/60, s = (s100/100)%60, cs = s100%100;
		if(m) //under 1min
			oss << m << ":";
		oss << fmt::format("{:02}.{:02}\n", s, cs);
		
		if(m)
		{
			oss << m << " minute";
			if(m != 1) oss << "s";
			oss << " ";
		}
		
		oss << s << " second";
		if(s != 1) oss << "s";
		
		oss << " and " << remtick << " tick";
		if(remtick != 1) oss << "s";
		
		twarp_lbl->setText(oss.str());
	}
}

void ScreenDataDialog::refreshLensEff()
{
	leff_switch->switchTo(get_qr(qr_OLD_LENS_LAYEREFFECT)?0:1);
}

std::shared_ptr<GUI::Widget> ScreenDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	std::string ene_str[5];
	byte ene_found = 0;
	static const uint64_t ene_flag[] = {guy_zora,guy_trap,guy_trp2,guy_rock,guy_fire};
	for(int32_t i=0; i<eMAXGUYS && ene_found != uint64_t(0b11111); i++)
	{
		for(auto q = 0; q < 5; ++q)
		{
			if(ene_found & (1<<q)) continue;
			if(guysbuf[i].flags & ene_flag[q])
			{
				ene_str[q] = guy_string[i];
				ene_found |= 1<<q;
			}
		}
	}
	for(auto q = 0; q < 5; ++q)
	{
		if(ene_found & (1<<q)) continue;
		ene_str[q] = "[No Enemy Set]";
	}
	
	window = Window(
		title = "Screen Data",
		onClose = message::CANCEL,
		Column(
			TabPanel(
				ptr = &screendata_tab,
				TabRef(name = "Flags 1", Row(
					Column(vAlign = 0.0,
						Rows<2>(hAlign = 0.0,
							Label(text = "Room Type", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags6,fCAVEROOM,1,"Treat as Interior Screen","If checked, this screen will be treated as part of an 'Interior' dmap, instead of the current dmap's type."),
							SCR_CB(flags6,fDUNGEONROOM,1,"Treat as Dungeon Screen","If checked, this screen will be treated as part of a 'Dungeon' dmap, instead of the current dmap's type."),
							SCR_CB(flags7,fSIDEVIEW,1,"Toggle Sideview Gravity","If checked, this screen's sideview gravity will be the opposite of the current dmap's.") //!TODO QRHINT sideview-related QRs
						),
						Rows<2>(hAlign = 0.0,
							Label(text = "View", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags3,fINVISHERO,1,"Invisible Hero","Hides the Hero on this screen"),
							SCR_CB(flags7,fNOHEROMARK,1,"No Hero Minimap Marker","Hides the Hero location minimap marker on this screen"),
							SCR_CB(flags3,fNOSUBSCR,1,"No Subscreen","Hides the subscreen on this screen"),
							SCR_CB(flags3,fNOSUBSCROFFSET,1,". . . But Don't Offset Screen","Prevents 'No Subscreen' from offsetting the screen"),
							SCR_CB(flags7,fLAYER2BG,1,"Toggle Layer 2 is BG","If checked, this screen will use the opposite of the dmap setting for Layer 2 being drawn in the background."),
							SCR_CB(flags7,fLAYER3BG,1,"Toggle Layer 3 is BG","If checked, this screen will use the opposite of the dmap setting for Layer 3 being drawn in the background."),
							SCR_CB(flags,fDARK,1,"Dark Room","If this room should be dark or not. Type of darkness depends on 'New Dark Rooms' setting." + QRHINT({qr_NEW_DARKROOM})),
							SCR_CB(flags9,fDARK_DITHER,1,". . . Dithered Darkness","The entire room's darkness is dithered.\nOnly applies with 'New Dark Rooms' enabled." + QRHINT({qr_NEW_DARKROOM})),
							SCR_CB(flags9,fDARK_TRANS,1,". . . Transparent Darkness","The entire room's darkness is transparent.\nOnly applies with 'New Dark Rooms' enabled." + QRHINT({qr_NEW_DARKROOM}))
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(hAlign = 0.0,
							Label(text = "Secrets", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags,fSHUTTERS,1,"Block->Shutters","If checked, dungeon shutters in the room will open when a block is pushed, instead of when enemies are defeated."),
							SCR_CB(flags5,fTEMPSECRETS,1,"Temporary Secrets","Secrets triggered on this screen will reset when leaving the screen."),
							SCR_CB(flags6,fTRIGGERFPERM,1,"All Triggers->Perm Secret","Triggering all 'Trigger->' mapflags (flags 94 and 95) will trigger a permanent secret."),
							SCR_CB(flags6,fTRIGGERF1631,1,"All Triggers->16-31","Triggering all 'Trigger->' mapflags (flags 94 and 95) will trigger secret combos 16-31.")
						),
						Rows<2>(hAlign = 0.0,
							Label(text = "Warp", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags5,fDIRECTAWARP,1,"Auto-Warps are Direct","Auto typed warps on this screen will not alter the Hero's position."),
							SCR_CB(flags5,fDIRECTSWARP,1,"Sensitive Warps are Direct","Sensitive typed warps on this screen will not alter the Hero's position."),
							SCR_CB(flags3,fIWARP_SPRITE_CARRYOVER,1,"Sprites Carry Over In Warps","Sprite objects, such as enemies, items, and weapons, will follow through warps from this screen."),
							SCR_CB(flags9,fDISABLE_MIRROR,1,"Disable Magic Mirror","Magic Mirror type items don't work on this screen.")
						),
						Rows<2>(hAlign = 0.0,
							Label(text = "Maze", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags,fMAZE,1,"Use Maze Path","This screen will act as a mystical maze requiring a specific path to be followed to escape."),
							SCR_CB(flags8,fMAZEvSIDEWARP,1,"Maze Overrides Side Warps","The Maze Path's mystical looping is prioritized over sidewarps if this is checked."),
							SCR_CB(flags10,fMAZE_CAN_GET_LOST,1,"Can Get Lost In Maze","Going any direction other than the exit gets the hero \"lost\" - in this state, leaving in the direction of the exit does not leave the maze, but does get the hero \"unlost\" (so using the exit direction once more will actually leave the maze)."),
							SCR_CB(flags10,fMAZE_LOOPY,1,"Loopy Maze","Instead of scrolling, taking any non-exit direction in a maze will move the player to the opposite side (without reloading the screen), with an optional wipe effect (see Maze Path dialog).\nIf the maze is within a scrolling region, this presents a neat illusion of an endless maze")
						)
					)
				)),
				TabRef(name = "Flags 2", Row(
					Column(vAlign = 0.0,
						Rows<2>(hAlign = 0.0,
							Label(text = "Combos", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags2,fAIRCOMBOS,1,"Combos Affect Midair Hero","If checked, combos such as damage combos and conveyors will affect the Hero even in mid-air."),
							SCR_CB(flags3,fCYCLEONINIT,1,"Cycle Combos On Screen Init","If checked, all combos on the screen that are set to 'cycle' will be forcibly cycled once before the screen appears.\nThis can be used for fancy tricks related to one-time animations not re-playing."),
							SCR_CB(flags5,fDAMAGEWITHBOOTS,1,"Damage Combos Ignore Boots","If checked, damage combos on this screen bypass the Boots itemtype."),
							SCR_CB(flags6,fTOGGLERINGDAMAGE,1,"Toggle 'Rings Affect Damage Combos'","If checked, Ring and Peril Ring items affecting damage combos uses the opposite setting on this screen.")
						),
						Rows<2>(hAlign = 0.0,
							Label(text = "Save", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags4,fSAVEROOM,1,"Save Point->Continue Here","When saving at a 'Save' combo on this screen, set this as the continue screen."),
							SCR_CB(flags4,fAUTOSAVE,1,"Save Game On Entry","Immediately save and set this screen as the continue point when entering this screen."),
							SCR_CB(flags6,fCONTINUEHERE,1,"Continue Here","When entering this screen, set it as the continue point for 'F6->Continue'."),
							SCR_CB(flags6,fNOCONTINUEHERE,1,"No Continue Here After Warp","When warping to this screen, DON'T set it as the continue point for 'F6->Continue'.")
						),
						Rows<2>(hAlign = 0.0,
							Label(text = "FFC", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags6,fWRAPAROUNDFF,1,"FF Combos Wrap Around","If checked, FFCs will wrap around at 32 pixels off the edge of the screen."),
							SCR_CB(flags5,fNOFFCARRYOVER,1,"No FFC Carryover","If set, no FFC will carryover (even w/ the FFC's 'carryover' flag set).")
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(hAlign = 0.0,
							Label(text = "Whistle", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags,fWHISTLE,1,"Whistle->Stairs","Triggers the 'Stairs' secret when the whistle is played."),
							SCR_CB(flags7,fWHISTLEPAL,1,"Whistle->Palette Change","Using the whistle on this screen will cause a hardcoded shift in the palette, usually used to visually 'dry up' water alongside 'Whistle->Dry Lake'."),
							SCR_CB(flags7,fWHISTLEWATER,1,"Whistle->Dry Lake","Using the whistle on this screen will 'dry up' water")
						),
						Rows<2>(hAlign = 0.0,
							Label(text = "Misc", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags,fLADDER,1,"Toggle 'Allow Ladder'","The state of 'Allow Ladder Anywhere' QR is reversed on this screen." + QRHINT({qr_LADDERANYWHERE})),
							SCR_CB(flags5,fTOGGLEDIVING,1,"Toggle 'No Diving'","Toggles whether flippers can dive or not on this screen."),
							SCR_CB_D(flags8,fGENERALSCRIPT1,1,"Script 1"),
							SCR_CB_D(flags8,fGENERALSCRIPT2,1,"Script 2"),
							SCR_CB_D(flags8,fGENERALSCRIPT3,1,"Script 3"),
							SCR_CB_D(flags8,fGENERALSCRIPT4,1,"Script 4"),
							SCR_CB_D(flags8,fGENERALSCRIPT5,1,"Script 5")
						)
					)
				)),
				TabRef(name = "Flags 3", Row(
					Column(vAlign = 0.0,
						Rows<2>(hAlign = 0.0,
							Label(text = "Items", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags3,fHOLDITEM,1,"Hold Up Item","The screen's item/special item will be held above the Hero's head."),
							SCR_CB(flags7,fITEMFALLS,1,"Item Falls From Ceiling","The screen's item drops from above."),
							SCR_CB(flags8,fSECRETITEM,1,"Secrets->Item","The screen's item spawns when secrets are triggered."),
							SCR_CB(flags8,fITEMSECRET,1,"Item->Secrets","Collecting the screen's item triggers secrets (temporary)"),
							SCR_CB(flags9,fITEMSECRETPERM,1,". . . Permanent","The secrets triggered by 'Item->Secrets' are permanent instead of temporary."),
							SCR_CB(flags9,fITEMRETURN,1,"Item Always Returns","The screen's item always respawns."),
							SCR_CB(flags9,fBELOWRETURN,1,"Special Item Always Returns","The screen's special item always respawns.")
						)
					)
				)),
				TabRef(name = "E. Flags", Row(
					Column(vAlign = 0.0,
						Rows<2>(hAlign = 0.0,
							Label(text = "Enemy Flags", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags11,efLEADER,1,"Ringleader","The first enemy listed in the screen's enemy list will be a 'Ringleader'. When this enemy dies, all other enemies will die."),
							SCR_CB(flags11,efCARRYITEM,1,"Enemy Carries Item","The first enemy listed in the screen's enemy list will carry the screen's item."),
							SCR_CB(flags3,fINVISROOM,1,"Invisible Enemies","The enemies in this room are invisible."),
							SCR_CB(flags11,efBOSS,1,"Dungeon Boss","Defeating the enemies in this room counts as 'beating the dungeon boss'. If you have defeated the dungeon boss, these enemies will not respawn."),
							SCR_CB(flags2,fFLOATTRAPS,1,"Traps Ignore Solidity","Traps on this screen will go through solid walls."),
							SCR_CB(flags3,fENEMIESRETURN,1,"Enemies Always Return","The enemies on this screen always respawn."),
							SCR_CB(flags,fITEM,1,"Enemies->Item","Killing all enemies spawns the screen item. (Note: This OVERRIDES 'Enemy Carries Item')"),
							SCR_CB(flags2,fCLEARSECRET,1,"Enemies->Secrets","Killing all enemies triggers screen secrets (temporarily)"),
							SCR_CB(flags4,fENEMYSCRTPERM,1,". . . Permanent","The secrets triggered by 'Enemies->Secret' are permanent instead of temporary."),
							SCR_CB(flags9,fENEMIES_STAY_DEAD,1,"Enemies->Stay Dead","Killing all enemies causes enemies to never respawn."),
							SCR_CB(flags9,fENEMY_WAVES,1,"Chain 'Enemies->' Triggers","If a combo uses the triggers tab on the 'Enemies->' trigger event to spawn more enemies, other 'kill all enemies' effects will wait for the new enemies to die as well.")
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(hAlign = 0.0,
							Label(text = "Environmental Enemies", hAlign = 0.0, colSpan = 2),
							SCR_CB(flags11,efZORA,1,"Fish",fmt::format("1x {}",ene_str[0])),
							SCR_CB(flags11,efTRAP4,1,"Corner Traps",fmt::format("4x {} in corners",ene_str[1])),
							SCR_CB(flags11,efTRAP2,1,"Middle Traps",fmt::format("2x {} in the middle",ene_str[2])),
							SCR_CB(flags11,efROCKS,1,"Falling Rocks",fmt::format("3x {}",ene_str[3])),
							SCR_CB(flags11,efFIREBALLS,1,"Shooting Statues",fmt::format("1x {} per Statue",ene_str[4]))
						)
					)
				)),
				TabRef(name = "States", Column(
					Rows<3>(hAlign = 1.0,
						Label(text = "Don't Carryover:", hAlign = 1.0),
						Button(
							width = 1.5_em, padding = 0_px, forceFitH = true,
							text = "P", hAlign = 1.0, onPressFunc = [&]()
							{
								auto flags = local_scr.nocarry;
								static const vector<CheckListInfo> scrstates =
								{
									{ "Door Up" },
									{ "Door Down" },
									{ "Door Left" },
									{ "Door Right" },
									{ "Screen Item" },
									{ "Special Item" },
									{ "Some Enemies Never Return" },
									{ "Enemies Temp No Return" },
									{ "Lockblock" },
									{ "Boss Lockblock" },
									{ "Chest" },
									{ "Locked Chest" },
									{ "Boss Chest" },
									{ "Secrets" },
									{ "Visited" },
									{ "Light Triggers" },
									{ "All Enemies Don't Return" },
								};
								if(!call_checklist_dialog("Select states to NOT CARRY OVER",scrstates,flags,8))
									return;
								local_scr.nocarry = flags;
							}
						),
						INFOBTN_EX("These states WILL NOT be carried over to the 'Carryover Screen'.", forceFitH = true, padding = 0_px),
						//
						Label(text = "Don't Reset:", hAlign = 1.0),
						Button(
							width = 1.5_em, padding = 0_px, forceFitH = true,
							text = "P", hAlign = 1.0, onPressFunc = [&]()
							{
								auto flags = local_scr.noreset;
								static const vector<CheckListInfo> scrstates =
								{
									{ "Door Up" },
									{ "Door Down" },
									{ "Door Left" },
									{ "Door Right" },
									{ "Screen Item" },
									{ "Special Item" },
									{ "Some Enemies Never Return" },
									{ "Enemies Temp No Return" },
									{ "Lockblock" },
									{ "Boss Lockblock" },
									{ "Chest" },
									{ "Locked Chest" },
									{ "Boss Chest" },
									{ "Secrets" },
									{ "Visited" },
									{ "Light Triggers" },
									{ "All Enemies Don't Return" },
								};
								if(!call_checklist_dialog("Select states to NOT RESET",scrstates,flags,8))
									return;
								local_scr.noreset = flags;
							}
						),
						INFOBTN_EX("These states WILL NOT be cleared by 'Reset Room' combos.", forceFitH = true, padding = 0_px),
						//
						Label(text = "Carryover ExStates:", hAlign = 1.0),
						Button(
							width = 1.5_em, padding = 0_px, forceFitH = true,
							text = "P", hAlign = 1.0, onPressFunc = [&]()
							{
								auto flags = local_scr.exstate_carry;
								static const vector<CheckListInfo> exstates =
								{
									{ "0" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, { "6" }, { "7" },
									{ "8" }, { "9" }, { "10" }, { "11" }, { "12" }, { "13" }, { "14" }, { "15" },
									{ "16" }, { "17" }, { "18" }, { "19" }, { "20" }, { "21" }, { "22" }, { "23" },
									{ "24" }, { "25" }, { "26" }, { "27" }, { "28" }, { "29" }, { "30" }, { "31" },
								};
								if(!call_checklist_dialog("Select ExStates to CARRY OVER",exstates,flags,8))
									return;
								local_scr.exstate_carry = flags;
							}
						),
						INFOBTN_EX("These ExStates WILL be carried over to the 'Carryover Screen'.", forceFitH = true, padding = 0_px),
						//
						Label(text = "Reset ExStates:", hAlign = 1.0),
						Button(
							width = 1.5_em, padding = 0_px, forceFitH = true,
							text = "P", hAlign = 1.0, onPressFunc = [&]()
							{
								auto flags = local_scr.exstate_reset;
								static const vector<CheckListInfo> exstates =
								{
									{ "0" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, { "6" }, { "7" },
									{ "8" }, { "9" }, { "10" }, { "11" }, { "12" }, { "13" }, { "14" }, { "15" },
									{ "16" }, { "17" }, { "18" }, { "19" }, { "20" }, { "21" }, { "22" }, { "23" },
									{ "24" }, { "25" }, { "26" }, { "27" }, { "28" }, { "29" }, { "30" }, { "31" },
								};
								if(!call_checklist_dialog("Select ExStates to RESET",exstates,flags,8))
									return;
								local_scr.exstate_reset = flags;
							}
						),
						INFOBTN_EX("These ExStates WILL be cleared by 'Reset Room' combos.", forceFitH = true, padding = 0_px)
					),
					Rows<3>(hAlign = 1.0,
						Label(text = "Carryover Map:", hAlign = 1.0),
						DropDownList(data = list_maps,
							fitParent = true,
							selectedValue = local_scr.nextmap,
							onSelectFunc = [&](int32_t val)
							{
								local_scr.nextmap = val;
							}
						),
						INFOBTN("If set to 0, no carryover occurs. Otherwise, when a screen state"
							" is set for this screen, it will also be set for the 'Carryover Screen',"
							" on the 'Carryover Map'."),
						Label(text = "Carryover Screen:", hAlign = 1.0),
						DropDownList(data = list_screens,
							fitParent = true,
							selectedValue = local_scr.nextscr,
							onSelectFunc = [&](int32_t val)
							{
								local_scr.nextscr = val;
							}
						),
						INFOBTN("The screen on the 'Carryover Map' to use for state carryover.")
					)
				)),
				TabRef(name = "Data", Column(
					Rows<3>(
						Label(text = "Damage Combo Sensitivity", hAlign = 1.0),
						DINFOBTN(),
						TextField(
							fitParent = true,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 8,
							val = local_scr.csensitive,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_scr.csensitive = val;
							}),
						//
						Label(text = "Screen MIDI", hAlign = 1.0),
						INFOBTN("MIDI to play on this screen"),
						DropDownList(data = list_screenmidi,
							fitParent = true,
							selectedValue = local_scr.screen_midi,
							onSelectFunc = [&](int32_t val)
							{
								local_scr.screen_midi = val;
							}
						),
						//
						Label(text = "Ambient Sound", hAlign = 1.0),
						INFOBTN("Sound to play while on the screen"),
						DropDownList(data = list_sfx,
							fitParent = true,
							selectedValue = local_scr.oceansfx,
							onSelectFunc = [&](int32_t val)
							{
								local_scr.oceansfx = val;
							}
						),
						//
						Label(text = "Boss Roar Sound", hAlign = 1.0),
						INFOBTN("Sound the boss on this screen roars"),
						DropDownList(data = list_sfx,
							fitParent = true,
							selectedValue = local_scr.bosssfx,
							onSelectFunc = [&](int32_t val)
							{
								local_scr.bosssfx = val;
							}
						),
						//
						Label(text = "Hold Up Item Sound", hAlign = 1.0),
						INFOBTN("Sound played when holding up an item"),
						DropDownList(data = list_sfx,
							fitParent = true,
							selectedValue = local_scr.holdupsfx,
							onSelectFunc = [&](int32_t val)
							{
								local_scr.holdupsfx = val;
							}
						),
						//
						Label(text = "Secret Sound", hAlign = 1.0),
						INFOBTN("Sound played when secrets are triggered"),
						DropDownList(data = list_sfx,
							fitParent = true,
							selectedValue = local_scr.secretsfx,
							onSelectFunc = [&](int32_t val)
							{
								local_scr.secretsfx = val;
							}
						),
						//
						DummyWidget(),
						SCR_CB(flags2,fSECRET_SFX,1,"Play Secret SFX on Entry","Play the 'Secret Sound' when entering the screen.")
					),
					Frame(title = "Lens Effect",
						info = "The effect the lens on this screen's layers." + QRHINT({qr_OLD_LENS_LAYEREFFECT}),
						onInfo = message::REFR_INFO,
						leff_switch = Switcher(
							DropDownList(data = list_lenseff,
								fitParent = true,
								selectedValue = local_scr.lens_layer,
								onSelectFunc = [&](int32_t val)
								{
									local_scr.lens_layer = val;
								}
							),
							Columns<3>(
								DummyWidget(),
								INFOBTN("If checked, the layer is revealed when the lens is active (thus, it is hidden by default)."),
								INFOBTN("If checked, the layer is hidden when the lens is active."),
								//
								DummyWidget(),
								Label(text = "Show:"),
								Label(text = "Hide:"),
								//
								Label(text = "L0"),
								LENS_CBS(0),
								//
								Label(text = "L1"),
								LENS_CBS(1),
								//
								Label(text = "L2"),
								LENS_CBS(2),
								//
								Label(text = "L3"),
								LENS_CBS(3),
								//
								Label(text = "L4"),
								LENS_CBS(4),
								//
								Label(text = "L5"),
								LENS_CBS(5),
								//
								Label(text = "L6"),
								LENS_CBS(6)
							)
						)
					)
				)),
				TabRef(name = "T. Warp", Rows<3>(
					INFOBTN("If non-zero, then after this time (in 1/60th of a second increments),"
						" a sidewarp will be triggered (default sidewarp A)."),
					Label(text = "Timed Warp Ticks"),
					TextField(
						fitParent = true,
						type = GUI::TextField::type::INT_DECIMAL,
						low = 0, high = 65535,
						val = local_scr.timedwarptics,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							local_scr.timedwarptics = val;
							refreshTWarp();
						}),
					//
					twarp_lbl = Label(text = "60.00 seconds\n99 minutes 99 seconds and 99/60ths", colSpan = 3, fitParent = true, textAlign = 1),
					SCR_CB(flags4,fTIMEDDIRECT,2,"Timed Warp is Direct", "The timed warp will not alter the Hero's position."),
					SCR_CB(flags4,fDISABLETIME,2,"Secrets Disable Timed Warp", "If secrets are triggered, the timed warp will no longer occur."),
					SCR_CB(flags5,fRANDOMTIMEDWARP,2,"Timed Warp Is Random Side", "Instead of being Sidewarp A, the timed warp will choose a random sidewarp from A,B,C,D.")
				)),
				TabRef(name = "Script", Column(
					Row(
						Label(text = "Data Size:"),
						TextField(
							type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
							low = 0, high = 214748,
							val = screen_misc_data.size(),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								screen_misc_data.resize(val);
								databtn->setDisabled(!val);
							}),
						INFOBTN("The starting size of the screen's 'Screen->Data[]' array."),
						//
						databtn = Button(colSpan = 3, fitParent = true,
							text = "Edit Starting Data",
							disabled = screen_misc_data.empty(),
							onPressFunc = [&]()
							{
								if(screen_misc_data.size())
								{
									call_edit_map(screen_misc_data, true);
								}
							})
					),
					Row(
						Column(
							SCREEN_INITD(0),
							SCREEN_INITD(1),
							SCREEN_INITD(2),
							SCREEN_INITD(3),
							SCREEN_INITD(4),
							SCREEN_INITD(5),
							SCREEN_INITD(6),
							SCREEN_INITD(7)
						),
						Column(
							padding = 0_px, fitParent = true,
							Rows<2>(vAlign = 0.0,
								SCRIPT_LIST_PROC("Action Script:", list_screenscript, local_scr.script, refreshScript)
							),
							Checkbox(
								hAlign = 0.0,
								checked = local_scr.preloadscript,
								text = "Run On Screen Init",
								onToggleFunc = [&](bool state)
								{
									local_scr.preloadscript = state ? 1 : 0;
								}
							)
						)
					)
				))
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	refreshScript();
	refreshTWarp();
	refreshLensEff();
	return window;
}

bool ScreenDataDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::REFR_INFO:
			refreshLensEff();
			break;
		case message::OK:
			*thescr = local_scr;
			zinit.screen_data[mapscrnum] = screen_misc_data;
			saved = false;
			[[fallthrough]];
		case message::CANCEL:
			return true;
	}
	return false;
}

