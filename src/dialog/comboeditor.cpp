#include "comboeditor.h"
#include "combowizard.h"
#include "combo_trigger_editor.h"
#include "gui/key.h"
#include "info.h"
#include "alert.h"
#include "base/zsys.h"
#include "tiles.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "items.h"
#include "base/qrs.h"
#include <fmt/format.h>
#include "zinfo.h"
#include "base/combo.h"

extern bool saved;
extern comboclass *combo_class_buf;
extern itemdata *itemsbuf;
extern int32_t CSet;
extern int32_t numericalFlags;
extern script_data *comboscripts[NUMSCRIPTSCOMBODATA];
char *ordinal(int32_t num);
using std::string;
using std::to_string;

static size_t cmb_tabs[5] = {0};
static bool combo_use_script_data = true;
static optional<combo_trigger> copied_trigger;

bool hasCTypeEffects(int32_t type)
{
	switch(type)
	{
		case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
		case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
		case cTRIGGERGENERIC: case cCSWITCH: case cSIGNPOST:
		case cSLASH: case cSLASHITEM: case cBUSH: case cFLOWERS: case cTALLGRASS:
		case cTALLGRASSNEXT:case cSLASHNEXT: case cSLASHNEXTITEM: case cBUSHNEXT:
		case cSLASHTOUCHY: case cSLASHITEMTOUCHY: case cBUSHTOUCHY: case cFLOWERSTOUCHY:
		case cTALLGRASSTOUCHY: case cSLASHNEXTTOUCHY: case cSLASHNEXTITEMTOUCHY:
		case cBUSHNEXTTOUCHY: case cSTEP: case cSTEPSAME: case cSTEPALL:
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		case cLOCKBLOCK: case cBOSSLOCKBLOCK:
		case cARMOS: case cBSGRAVE: case cGRAVE:
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		case cSTEPSFX: case cSWITCHHOOK: case cCSWITCHBLOCK:
		case cSHOOTER: case cCUTSCENETRIG:
		case cSAVE: case cSAVE2:
			return true;
	}
	return false;
}

static bool edited = false;
#if DEVLEVEL > 0
static int32_t force_wizard = 0;
#endif
bool call_combo_editor(int32_t index)
{
	combo_use_script_data = zc_get_config("zquest","show_comboscript_meta_attribs",1)?true:false;
#if DEVLEVEL > 0
	force_wizard = 0;
	if(key_shifts&KB_CTRL_CMD_FLAG)
		force_wizard = (key_shifts&KB_SHIFT_FLAG)?2:1;
#endif
	int32_t cs = CSet;
	edited = false;
	ComboEditorDialog(index).show();
	if(!edited) CSet = cs;
	return edited;
}

ComboEditorDialog::ComboEditorDialog(newcombo const& ref, int32_t index):
	index(index), local_comboref(ref),
	list_ctype(GUI::ZCListData::combotype(true)),
	list_flag(GUI::ZCListData::mapflag(numericalFlags, true)),
	list_combscript(GUI::ZCListData::combodata_script()),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_genscr(GUI::ZCListData::generic_script()),
	list_counters_nn(GUI::ZCListData::counters(true, true)),
	list_sprites(GUI::ZCListData::miscsprites()),
	list_sprites_spec(GUI::ZCListData::miscsprites(false,true)),
	list_weaptype(GUI::ZCListData::weaptypes(true)),
	list_deftypes(GUI::ZCListData::deftypes()),
	list_dirs4n(GUI::ZCListData::dirs(4,true)),
	list_0_7(GUI::ListData::numbers(false,0,8)),
	list_light_shapes(GUI::ZCListData::light_shapes()),
	list_lift_parent_items(GUI::ZCListData::items(true).filter(
		[&](GUI::ListItem& itm)
		{
			if(itm.value == 0) //Remove item 0
				return false;
			if(itm.value == -1) //Change the none value to 0
				itm.value = 0;
			else switch(itemsbuf[itm.value].family) //Limit valid item types
			{
				case itype_bomb:
					itm.text += fmt::format(" [{}]",ZI.getWeapName(wLitBomb));
					break;
				case itype_sbomb:
					itm.text += fmt::format(" [{}]",ZI.getWeapName(wLitSBomb));
					break;
				default: return false;
			}
			return true;
		})),
	list_sprites_0none(GUI::ZCListData::miscsprites(false).filter(
		[&](GUI::ListItem& itm)
		{
			if(itm.value == 0) //Remove item 0
				return false;
			if(itm.value == -1) //Change the none value to 0
			{
				itm.value = 0;
				itm.text = "(None) (000)";
			}
			return true;
		}))
{}

ComboEditorDialog::ComboEditorDialog(int32_t index):
	ComboEditorDialog(combobuf[index], index)
{}

//{ Help Strings
static const char *combotype_help_string[cMAX] =
{
	"",
	"The Hero is warped via Tile Warp A if they step on the bottom half of this combo.",
	"The Hero marches down into this combo and is warped via Tile Warp A if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"",
	"",
	"",
	"Raft paths must begin on a Dock-type combo. (Use the Raft combo flag to create raft paths.)",
	"", //cUNDEF
	"A Bracelet is not needed to push this combo, but it can't be pushed until the enemies are cleared from the screen.",
	"A Bracelet is needed to push this combo. The screen's Under Combo will appear beneath it when it is pushed aside.",
	"A Bracelet is needed to push this combo, and it can't be pushed until the enemies are cleared from the screen.",
	"If the 'Statues Shoot Fire' Screen Data flag is checked, an invisible fireball shooting enemy is spawned on this combo.",
	"If the 'Statues Shoot Fire' Screen Data flag is checked, an invisible fireball shooting enemy is spawned on this combo.",
	"The Hero's movement speed is reduced while they walk on this combo. Enemies will not be affected.",
	"While the Hero is standing on top of this, they will be moved upward at 2 pixels per 3 frames (or some dir at a custom-set speed), until they collide with a solid combo.",
	"While the Hero is standing on top of this, they will be moved downward at 2 pixels per 3 frames (or some dir at a custom-set speed), until they collide with a solid combo.",
	"While the Hero is standing on top of this, they will be moved leftward at 2 pixels per 3 frames (or some dir at a custom-set speed), until they collide with a solid combo.",
	"While the Hero is standing on top of this, they will be moved rightward at 2 pixels per 3 frames (or some dir at a custom-set speed), until they collide with a solid combo.",
	"The Hero is warped via Tile Warp A if they swim on this combo. Otherwise, this is identical to Water.",
	"The Hero is warped via Tile Warp A if they dive on this combo. Otherwise, this is identical to Water.",
	"If this combo is solid, the Ladder and Hookshot can be used to cross over it. It only permits the Ladder if its on Layer 0.",
	"This activates Screen Secrets when the bottom half of this combo is stepped on, but it does not set the screen's 'Secret' Screen State.",
	"This activates Screen Secrets when the bottom half of this combo is stepped on, and sets the screen's 'Secret' Screen State, making the secrets permanent.",
	"", // Unused
	"When stabbed or slashed with a Sword, this combo changes into the screen's Under Combo.",
	"Identical to Slash, but an item from Item Drop Set 12 is created when this combo is slashed.",
	"A Bracelet with a Push Combo Level of 2 is needed to push this combo. Otherwise, this is identical to Push (Heavy).",
	"A Bracelet with a Push Combo Level of 2 is needed to push this combo. Otherwise, this is identical to Push (Heavy, Wait).",
	"When hit by a Hammer, this combo changes into the next combo in the list.",
	"If this combo is struck by the Hookshot, the Hero is pulled towards the combo.",
	"", //cHSBRIDGE (Unimplemented)
	// Damage Combos
	"",
	"",
	"",
	"",
	// Anyway...
	"If the 'Statues Shoot Fire' Screen Data flag is checked, an invisible fireball shooting enemy is spawned on this combo.",
	"This flag is obsolete. It behaves identically to Combo Flag 32, Trap (Horizontal, Line of Sight).",
	"This flag is obsolete. It behaves identically to Combo Flag 33, Trap (Vertical, Line of Sight).",
	"This flag is obsolete. It behaves identically to Combo Flag 34, Trap (4-Way, Line of Sight).",
	"This flag is obsolete. It behaves identically to Combo Flag 35, Trap (Horizontal, Constant).",
	"This flag is obsolete. It behaves identically to Combo Flag 36, Trap (Vertical Constant).",
	"The Hero is warped via Tile Warp A if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
	"If this combo is solid, the Hookshot can be used to cross over it.",
	"This combo's tile is drawn between layers 3 and 4 if it is placed on layer 0.",
	"Flying enemies (Keese, Peahats, Moldorms, Patras, Fairys, Digdogger, Manhandla, Ghinis, Gleeok heads) can't fly over or appear on this combo.",
	"Wand magic and enemy magic that hits  this combo is reflected 180 degrees, and becomes 'reflected magic'.",
	"Wand magic and enemy magic that hits  this combo is reflected 90 degrees, and become 'reflected magic'.",
	"Wand magic and enemy magic that hits  this combo is reflected 90 degrees, and become 'reflected magic'.",
	"Wand magic and enemy magic that hits  this combo is duplicated twice, causing three shots to be fired in three directions.",
	"Wand magic and enemy magic that hits  this combo is duplicated thrice, causing four shots to be fired from each direction.",
	"Wand magic and enemy magic that hits this combo is destroyed.",
	"The Hero marches up into this combo and is warped via Tile Warp A if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"The combo's tile changes depending on the Hero's position relative to the combo. It uses eight tiles per animation frame.",
	"Identical to Eyeball (8-Way A), but the angles at which the tile will change are offset by 22.5 degrees (pi/8 radians).",
	"Tektites cannot jump through or appear on this combo.",
	"Identical to Slash->Item, but when it is slashed, Bush Leaves sprites are drawn and the 'Tall Grass slashed' sound plays.",
	"Identical to Slash->Item, but when it is slashed, Flower Clippings sprites are drawn and the 'Tall Grass slashed' sound plays.",
	"Identical to Slash->Item, but when it is slashed, Grass Clippings sprites are drawn and the 'Tall Grass slashed' sound plays.",
	"Ripples sprites are drawn on the Hero when they walk on this combo. Also, Quake Hammer pounds are nullified by this combo.",
	"",
	"Identical to Lock Block, but if any other Lock Blocks are opened on the same screen, this changes to the next combo.",
	"",
	"Identical to Lock Block (Boss), but if any other Boss Lock Blocks are opened on the same screen, this changes to the next combo.",
	"If this combo is solid, the Ladder can be used to cross over it. Only works on layer 0.",
	"",
	//Chests
	"", "", "", "", "", "",
	"If the Hero touches this, the Screen States are cleared, and the Hero is re-warped back into the screen, effectively resetting the screen entirely.",
	"Press the 'Start' button when the Hero is standing on the bottom of this combo, and the Save menu appears. Best used with the Save Point->Continue Here Screen Flag.",
	"Identical to Save Point, but the Quit option is also available in the menu.",
	"The Hero marches down into this combo and is warped via Tile Warp B if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"The Hero marches down into this combo and is warped via Tile Warp C if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"The Hero marches down into this combo and is warped via Tile Warp D if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"The Hero is warped via Tile Warp B if they step on the bottom half of this combo.",
	"The Hero is warped via Tile Warp C if they step on the bottom half of this combo.",
	"The Hero is warped via Tile Warp D if they step on the bottom half of this combo.",
	"The Hero is warped via Tile Warp B if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
	"The Hero is warped via Tile Warp C if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
	"The Hero is warped via Tile Warp D if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
	"The Hero marches up into this combo and is warped via Tile Warp B if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"The Hero marches up into this combo and is warped via Tile Warp C if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"The Hero marches up into this combo and is warped via Tile Warp D if they step on this. The combo's tile will be drawn above the Hero during this animation.",
	"The Hero is warped via Tile Warp B if they swim on this combo. Otherwise, this is identical to Water.",
	"The Hero is warped via Tile Warp C if they swim on this combo. Otherwise, this is identical to Water.",
	"The Hero is warped via Tile Warp D if they swim on this combo. Otherwise, this is identical to Water.",
	"The Hero is warped via Tile Warp B if they dive on this combo. Otherwise, this is identical to Water.",
	"The Hero is warped via Tile Warp C if they dive on this combo. Otherwise, this is identical to Water.",
	"The Hero is warped via Tile Warp D if they dive on this combo. Otherwise, this is identical to Water.",
	"Identical to Stairs [A], but the Tile Warp used (A, B, C, or D) is chosen at random. Use this only in screens where all four Tile Warps are defined.",
	"Identical to Direct Warp [A], but the Tile Warp used (A, B, C, or D) is chosen at random. Use this only in screens where all four Tile Warps are defined.",
	"As soon as this combo appears on the screen, Side Warp A is activated. This is best used with secret combos or combo cycling.",
	"As soon as this combo appears on the screen, Side Warp B is activated. This is best used with secret combos or combo cycling.",
	"As soon as this combo appears on the screen, Side Warp C is activated. This is best used with secret combos or combo cycling.",
	"As soon as this combo appears on the screen, Side Warp D is activated. This is best used with secret combos or combo cycling.",
	"Identical to Auto Side Warp [A], but the Side Warp used (A, B, C, or D) is chosen at random. Use this only in screens where all four Side Warps are defined.",
	"Identical to Stairs [A], but the Hero will be warped as soon as they touch the edge of this combo.",
	"Identical to Stairs [B], but the Hero will be warped as soon as they touch the edge of this combo.",
	"Identical to Stairs [C], but the Hero will be warped as soon as they touch the edge of this combo.",
	"Identical to Stairs [D], but the Hero will be warped as soon as they touch the edge of this combo.",
	"Identical to Stairs [Random], but the Hero will be warped as soon as they touch the edge of this combo.",
	"Identical to Step->Secrets (Temporary), but Screen Secrets are activated as soon as the Hero touches the edge of this combo.",
	"Identical to Step->Secrets (Permanent), but Screen Secrets are activated as soon as the Hero touches the edge of this combo.",
	"When the Hero steps on this combo, it will change into the next combo in the list.",
	"Identical to Step->Next, but if other instances of this particular combo are stepped on, this also changes to the next combo in the list.",
	"When the Hero steps on this combo, each of the Step->Next combos on screen will change to the next combo after them in the list.",
	"When the Hero steps on a Step->Next (All) type combo, this will change into the next combo in the list.",
	"Enemies cannot enter or appear on this combo.",
	"Level 1 Hero arrows that hit this combo are destroyed. Enemy arrows are unaffected.",
	"Level 1 or 2 Hero arrows that hit this combo are destroyed. Enemy arrows are unaffected.",
	"All Hero arrows that hit this combo are destroyed. Enemy arrows are unaffected.",
	"Level 1 Hero boomerangs bounce off this combo. Enemy boomerangs are unaffected.",
	"Level 1 or 2 Hero boomerangs bounce off this combo. Enemy boomerangs are unaffected.",
	"All Hero boomerangs bounce off this combo. Enemy boomerangs are unaffected.",
	"The Hero's sword beams or enemy sword beams that hit this combo are destroyed.",
	"All weapons that hit this combo are either destroyed, or bounce off.",
	"Enemy fireballs and reflected fireballs that hit this combo are destroyed.",
	// More damage
	"", "", "",
	"", //Unused
	"A Spinning Tile immediately appears on this combo, using the combo's tile to determine its sprite. The combo then changes to the next in the list.",
	"", // Unused
	"While this combo is on the screen, all action is frozen, except for FFC animation and all scripts. Best used in conjunction with Changer FFCs or scripts.",
	"While this combo is on the screen, FFCs and FFC scripts will be frozen. Best used in conjunction with combo cycling, screen secrets or global scripts.",
	"Enemies that don't fly or jump cannot enter or appear on this combo.",
	"Identical to Slash, but instead of changing into the Under Combo when slashed, this changes to the next combo in the list.",
	"Identical to Slash (Item), but instead of changing into the Under Combo when slashed, this changes to the next combo in the list.",
	"Identical to Bush, but instead of changing into the Under Combo when slashed, this changes to the next combo in the list.",
	// Continuous variation
	"Identical to Slash, but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Slash->Item, but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Bush, but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Flowers, but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Tall Grass, but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Slash->Next, but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Slash->Next (Item), but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Bush->Next, but if slashing this combo changes it to another slash-affected combo, then that combo will also change.",
	"Identical to Eyeball (8-Way A), but only the four cardinal directions/sets of tiles are used (up, down, left and right, respectively).",
	"Identical to Tall Grass, but instead of changing into the Under Combo when slashed, this changes to the next combo in the list.",
	// Script types
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.", //1
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.", //5
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.", //10
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.", //15
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.",
	"This type has no built-in effect, but can be given special significance with ZASM or ZScript.", //20
	//Generic
	"Generic combos can be configured to do a wide variety of things based on attributes.",
	"Pitfall combos act as either bottomless pits or warps, including a fall animation.",
	"Step->Effects combos can cause SFX, and also act like a landmine, spawning a weapon.",
	"Bridge combos can be used to block combos under them from having an effect.",
	"",
	"",
	"Switchblock combos change based on switch states toggled by Switch combos. They can also change"
		" the combo at the same position on any layer.",
	""
};
//}


std::string getComboTypeHelpText(int32_t id)
{
	std::string typehelp;
	switch(id)
	{
		case cNONE:
			typehelp = "Select a Type, then click this button to find out what it does.";
			break;
		case cWATER:
			typehelp = "Liquid can contain Zora enemies and can be crossed with various weapons and items. If the matching quest rule is set, the Hero can drown in it."
				+ QRHINT({qr_DROWN,qr_SMARTER_WATER,qr_NO_HOPPING,qr_NO_SOLID_SWIM,qr_WATER_ON_LAYER_1,qr_WATER_ON_LAYER_2,qr_SIDESWIM,qr_SIDESWIMDIR,qr_SHALLOW_SENSITIVE,qr_NO_SCROLL_WHILE_IN_AIR});
			break;
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		{
			char buf[512];
			int32_t lvl = (id < cDAMAGE5 ? (id - cDAMAGE1 + 1) : (id - cDAMAGE5 + 1));
			int32_t d = -combo_class_buf[id].modify_hp_amount/8;
			char buf2[80];
			if(d==1)
				sprintf(buf2,"1/2 of a heart.");
			else
				sprintf(buf2,"%d heart%s.", d/2, d == 2 ? "" : "s");
			sprintf(buf,"If the Hero touches this combo without Boots that protect against Damage Combo Level %d, they are damaged for %s.",lvl,buf2);
			typehelp = buf;
			break;
		}
		
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
			typehelp = "If no button is assigned, the chest opens when pushed against from a valid side. If buttons are assigned,"
				" then when the button is pressed while facing the chest from a valid side.\n"
				"When the chest is opened, if it has the 'Armos/Chest->Item' combo flag, the Hero will recieve the contained item, and the combo will advance to the next combo.";
			if(id==cLOCKEDCHEST)
				typehelp += "\nRequires a key to open.";
			else if(id==cBOSSCHEST)
				typehelp += "\nRequires the Boss Key to open.";
			typehelp += QRHINT({qr_OLD_CHEST_COLLISION});
			break;
		case cLOCKBLOCK:
			typehelp = "If the combo is solid and the Hero pushes it with at least one Key, it changes to the next combo, the 'Lock Blocks' Screen State is set, and one key is used up."
				+ QRHINT({qr_OLD_LOCKBLOCK_COLLISION});
			break;
		case cBOSSLOCKBLOCK:
			typehelp = "If the combo is solid and the Hero pushes it with the Boss Key, it changes to the next combo and the 'Boss Lock Blocks' Screen State is set."
				+ QRHINT({qr_OLD_LOCKBLOCK_COLLISION});
			break;
		case cTORCH:
			typehelp = "Emits light in a radius in dark rooms (when QR 'New Dark Rooms' is enabled)"
				+ QRHINT({qr_NEW_DARKROOM});
			break;
		case cSIGNPOST:
			typehelp = "Signpost combos can be set to display a string. This can be hard-coded,"
				" or variable. The message will display either on button press of a"
				" set button or walking into the sign if no button is set.";
			break;
		case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
		{
			std::string str = (id == cLOCKEDCHEST2) ? "Locked"
				: ((id == cBOSSCHEST2) ? "Boss"
				: "Basic");
			typehelp = "Acts as a chest that can't be opened. Becomes 'opened' (advancing to the next combo) when any 'Treasure Chest ("+str+")' is opened on the screen.";
			break;
		}
		case cHSBRIDGE: case cZELDA: case cUNDEF: case cCHANGE: case cSPINTILE2:
			typehelp = "Unimplemented type, do not use!";
			break;
		
		case cSPOTLIGHT:
			typehelp = "Shoots a beam of light which reflects off of mirrors, and can trigger light triggers.";
			break;
		
		case cGLASS:
			typehelp = "Does not block light beams, even if solid";
			break;
		
		case cLIGHTTARGET:
			typehelp = "If all targets onscreen are lit by light beams, secrets will be activated.";
			break;
		case cCSWITCH:
			typehelp = "Switch combos, when triggered (Triggers tab w/ 'ComboType Effects' checked), toggle a switch state for the current 'level'."
				" These states affect Switchblock combos in any dmaps of the same level, and are saved between sessions."
				" If the 'Use Global State' flag is set, a set of 256 global states shared between levels will be used instead.";
			break;
		case cSWITCHHOOK:
			typehelp = "When hit with a switch-hook, swaps position with the Hero (staying on the same layer).";
			break;
		case cBUTTONPROMPT:
			typehelp = "Displays a button prompt based on the 'Btn:' triggerflags";
			break;
		case cCUSTOMBLOCK:
			typehelp = "Blocks weapons denoted by the weapon triggerflags.";
			break;
		case cSHOOTER:
			typehelp = "Shoots, as a turret. Triggering with 'ComboType Effects' causes it to instantly shoot.";
			break;
		case cCRUMBLE:
			typehelp = "Crumbles when stood on, turning into the next combo. Various crumbling options are available.";
			break;
		case cARMOS:
			typehelp = "When touched, this combo produces an Armos and changes to the screen's Under Combo."
				" Only functions on layer 0, even with ComboType Effects triggerflag.";
			break;
		case cGRAVE:
			typehelp = "When touched, this combo produces one Ghini."
				" Only functions on layer 0, even with ComboType Effects triggerflag.";
			break;
		case cBSGRAVE:
			typehelp = "When touched, this combo produces a Ghini and changes to the next combo in the list."
				" Only functions on layer 0, even with ComboType Effects triggerflag.";
			break;
		case cCUTSCENETRIG:
			typehelp = "When activated with ComboType Effects, either stops an active cutscene, or"
				" sets the active cutscene rules.";
			break;
		case cPUSHBLOCK:
			typehelp = "A pushable block, that works separately from push flags. Highly configurable.";
			break;
		case cICY:
			typehelp = "A block that may act slippery in different ways depending on its' flags.";
			break;
		case cMIRRORNEW:
			typehelp = "A customizable version of mirror combos.";
			break;
		case cSLOPE:
			typehelp = "This type has a diagonal collision box (separate from normal solidity), and several options"
				" that are useful in sideview.";
			break;
		default:
			if(combotype_help_string[id] && combotype_help_string[id][0])
				typehelp = combotype_help_string[id];
			else typehelp = "?? Missing documentation! ??";
			break;
	}
	return typehelp;
}
std::string getMapFlagHelpText(int32_t id)
{
	std::string flaghelp = "?? Missing documentation! ??";
	switch(id)
	{
		case 0:
			flaghelp = "Select a Flag, then click this button to find out what it does.";
			break;
		case mfPUSHUD:
			flaghelp = "Allows the Hero to push the combo up or down once, activating Screen Secrets (or just the 'Stairs', secret combo) as well as Block->Shutters.";
			break;
		case mfPUSH4:
			flaghelp = "Allows the Hero to push the combo in any direction once, activating Screen Secrets (or just the 'Stairs', secret combo) as well as Block->Shutters.";
			break;
		case mfWHISTLE:
			flaghelp = "Triggers Screen Secrets when the Hero plays the Whistle on it. Is replaced with the 'Whistle' Secret Combo. Doesn't interfere with Whistle related Screen Flags.";
			break;
		case mfANYFIRE:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with fire from any source. Is replaced with the 'Any Fire' Secret Combo.";
			break;
		case mfARROW:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with one of his Arrows. Is replaced with the 'Wooden Arrow' Secret Combo.";
			break;
		case mfBOMB:
			flaghelp = "Triggers Screen Secrets when the middle part of a Bomb explosion touches it. Is replaced with the 'Bomb' Secret Combo.";
			break;
		case mfRAFT:
			flaghelp = "Place in paths to define the path the Hero travels when using the Raft. Use with Dock-type combos. If a path branches, the Hero takes the clockwise-most path.";
			break;
		case mfARMOS_SECRET:
			flaghelp = "When placed on an Armos-type combo, causes the 'Stairs'  Secret Combo to appear when the Armos is activated, instead of the screen's Under Combo.";
			break;
		case mfARMOS_ITEM:
			flaghelp = "When placed on an Armos or treasure chest, causes the room's Special Item to appear when the combo is activated. Requires the 'Special Item' Room Type.";
			break;
		case mfSBOMB:
			flaghelp = "Triggers Screen Secrets when the middle part of a Super Bomb explosion touches it. Is replaced with the 'Super Bomb' Secret Combo.";
			break;
		case mfRAFT_BRANCH:
			flaghelp = "Place at intersections of Raft flag paths to define points where the Hero may change directions. Change directions by holding down a directional key.";
			break;
		case mfDIVE_ITEM:
			flaghelp = "When the Hero dives on a flagged water-type combo they will recieve the screen's Special Item. Requires the 'Special Item' Room Type.";
			break;
		case mfLENSMARKER:
			flaghelp = "Combos with this flag will flash white when viewed with the Lens of Truth item.";
			break;
		case mfZELDA:
			flaghelp = "When the Hero steps on this flag, the quest will end, and the credits will roll.";
			break;
		case mfTRAP_H:
		case mfTRAP_V:
		case mfTRAP_4:
		case mfTRAP_LR:
		case mfTRAP_UD:
		{
			static const char* name[]{"Horz","Vert","4-Way","LR","UD"};
			flaghelp = fmt::format("Creates the lowest-numbered enemy with the 'Spawned by '{} Trap' Combo Type/Flag' enemy data flag on the flagged combo.",name[id-mfTRAP_H]);
			break;
		}
		case mfPUSHLR:
		case mfPUSHU:
		case mfPUSHD:
		case mfPUSHL:
		case mfPUSHR:
		{
			static const char* name[]{"left or right","up","down","left","right"};
			flaghelp = fmt::format("Allows the Hero to push the combo {} once, activating Screen Secrets (or just the 'Stairs', secret combo) as well as Block->Shutters.",name[id-mfPUSHLR]);
			break;
		}
		case mfBLOCKTRIGGER:
			flaghelp = "Pushing blocks onto ALL Block Triggers will activate Screen Secrets (or just the 'Stairs' secret combo) as well as Block->Shutters."
				+ QRHINT({qr_NONHEAVY_BLOCKTRIGGER_PERM,qr_BLOCKHOLE_SAME_ONLY,qr_BLOCKS_DONT_LOCK_OTHER_LAYERS,qr_PUSHBLOCK_LAYER_1_2});
			break;
		case mfNOBLOCKS:
			flaghelp = "Prevents push blocks from being pushed onto the flagged combo, even if it is not solid.";
			break;
		case mfBRANG:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with one of his Boomerangs. Is replaced with the 'Wooden Boomerang' Secret Combo.";
			break;
		case mfMBRANG:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 2 or higher Boomerang. Is replaced with the 'Magic Boomerang' Secret Combo.";
			break;
		case mfFBRANG:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 3 or higher Boomerang. Is replaced with the 'Fire Boomerang' Secret Combo.";
			break;
		case mfSARROW:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 2 or higher Arrow. Is replaced with the 'Silver Arrow' Secret Combo.";
			break;
		case mfGARROW:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 3 or higher Arrow. Is replaced with the 'Golden Arrow' Secret Combo.";
			break;
		case mfSTRONGFIRE:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with fire from a 'Strong Fire' source. Is replaced with the 'Strong Fire' Secret Combo.";
			break;
		case mfMAGICFIRE:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with fire from a 'Magic Fire' source. Is replaced with the 'Magic Fire' Secret Combo.";
			break;
		case mfDIVINEFIRE:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with fire from a 'Divine Fire' source. Is replaced with the 'Divine Fire' Secret Combo.";
			break;
		case mfWANDMAGIC:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with Wand magic, be it fire or not. Is replaced with the 'Wand Magic' Secret Combo.";
			break;
		case mfREFMAGIC:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with reflected Wand magic. Is replaced with the 'Reflected Magic' Secret Combo.";
			break;
		case mfREFFIREBALL:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a Shield-reflected fireball. Is replaced with the 'Reflected Fireball' Secret Combo.";
			break;
		case mfSWORD:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with one of his Swords. Is replaced with the 'Wooden Sword' Secret Combo.";
			break;
		case mfWSWORD:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 2 or higher Sword. Is replaced with the 'White Sword' Secret Combo.";
			break;
		case mfMSWORD:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 3 or higher Sword. Is replaced with the 'Magic Sword' Secret Combo.";
			break;
		case mfXSWORD:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 4 or higher Sword. Is replaced with the 'Master Sword' Secret Combo.";
			break;
		case mfSWORDBEAM:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with one of his Sword beams. Is replaced with the 'Sword Beam' Secret Combo.";
			break;
		case mfWSWORDBEAM:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 2 or higher Sword's beam. Is replaced with the 'White Sword Beam' Secret Combo.";
			break;
		case mfMSWORDBEAM:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 3 or higher Sword's beam. Is replaced with the 'Magic Sword Beam' Secret Combo.";
			break;
		case mfXSWORDBEAM:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with a level 4 or higher Sword's beam. Is replaced with the 'Master Sword Beam' Secret Combo.";
			break;
		case mfHOOKSHOT:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with one of his Hookshot hooks. Is replaced with the 'Hookshot' Secret Combo.";
			break;
		case mfWAND:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with one of his Wands. Is replaced with the 'Wand' Secret Combo.";
			break;
		case mfHAMMER:
			flaghelp = "Triggers Screen Secrets when the Hero pounds it with one of his Hammers. Is replaced with the 'Hammer' Secret Combo.";
			break;
		case mfSTRIKE:
			flaghelp = "Triggers Screen Secrets when the Hero touches it with any weapon or projectile. Is replaced with the 'Any Weapon' Secret Combo.";
			break;
		case mfBLOCKHOLE:
			flaghelp = "A push block pushed onto this flag will cycle to the next combo in the list, and lose the Push flag that was presumably on it."
				+ QRHINT({qr_BLOCKHOLE_SAME_ONLY,qr_PUSHBLOCK_LAYER_1_2});
			break;
		case mfSINGLE:
			flaghelp = "When stacked with a Trigger Combo Flag, it prevents the activated Secrets process from changing all other flagged combos on-screen.";
			break;
		case mfSINGLE16:
			flaghelp = "Similar to 'Trigger->Self Only', but the Secret Tile (16-31) flagged combos will still change. (The 'Hit All Triggers->16-31' Screen Flag overrides this.)";
			break;
		case mfNOENEMY:
			flaghelp = "Enemies cannot enter or appear on the flagged combo.";
			break;
		case mfNOGROUNDENEMY:
			flaghelp = "Enemies that don't fly or jump cannot enter or appear on the flagged combo.";
			break;
		
		case mfRAFT_BOUNCE:
			flaghelp = "When the Hero is rafting, and hits this flag, they will be turned around.";
			break;
		
		case mfFAIRY:
		case mfMAGICFAIRY:
		case mfALLFAIRY:
		{
			static const char* name[]{"life","magic","life and magic"};
			int ind = (id == mfFAIRY ? 0 : (id == mfMAGICFAIRY ? 1 : 2));
			flaghelp = fmt::format("Makes a heart circle appear on screen when the Hero steps on it, and refills their {}.",name[ind])
				+ QRHINT({qr_HEARTRINGFIX,qr_NOHEARTRING});
			break;
		}
		case mfSECRETS01: case mfSECRETS02: case mfSECRETS03: case mfSECRETS04:
		case mfSECRETS05: case mfSECRETS06: case mfSECRETS07: case mfSECRETS08:
		case mfSECRETS09: case mfSECRETS10: case mfSECRETS11: case mfSECRETS12:
		case mfSECRETS13: case mfSECRETS14: case mfSECRETS15: case mfSECRETS16:
			flaghelp = "When Screen Secrets are activated, this is replaced with Secret Combo "
				+ std::to_string(id) + ". (Also, flagged Destructible Combos"
				" will use that Secret Combo instead of the Under Combo.)";
			break;
		case mfENEMY0: case mfENEMY1: case mfENEMY2: case mfENEMY3: case mfENEMY4:
		case mfENEMY5: case mfENEMY6: case mfENEMY7: case mfENEMY8: case mfENEMY9:
		{
			int32_t enemynum = id - 37 + 1;
			flaghelp = "When the "+string(ordinal(enemynum))+" enemy in the Enemy List is spawned,"
				" it appears on this flag instead of using the Enemy Pattern."
				" The uppermost, then leftmost, instance of this flag is used.";
			break;
		}
		case mfPUSHUDNS: case mfPUSHLRNS: case mfPUSH4NS: case mfPUSHUNS:
		case mfPUSHDNS: case mfPUSHLNS: case mfPUSHRNS: case mfPUSHUDINS:
		case mfPUSHLRINS: case mfPUSH4INS: case mfPUSHUINS: case mfPUSHDINS:
		case mfPUSHLINS: case mfPUSHRINS:
		{
			static const char* name[]{"up/down", "left/right","in any direction","up","down","left","right"};
			flaghelp = fmt::format("Allows the Hero to push the combo {} {} activating Block->Shutters but not Screen Secrets.",
				name[(id-mfPUSHUDNS) % 7], (id>=mfPUSHUDINS) ? "many times" : "once");
			break;
		}
		case mfPUSHED:
		{
			flaghelp = "This flag is placed on Push blocks with the 'Once' property after they have been pushed.";
			break;
		}
		case mfSIDEVIEWLADDER:
		{
			flaghelp = "On a sideview screen, allows climbing. The topmost ladder in a column doubles as a Sideview Platform."
				+ QRHINT({qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS,qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS,qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK,qr_SIDEVIEWLADDER_FACEUP,qr_DOWN_DOESNT_GRAB_LADDERS});
			break;
		}
		case mfSIDEVIEWPLATFORM:
		{
			flaghelp = "On a sideview screen, can be stood on top of, even when nonsolid. Can be jumped through"
				" from below, and depending on QRs, can also be dropped through."
				+ QRHINT({qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS,qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS,qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK});
			break;
		}
		case mfNOENEMYSPAWN:
		{
			flaghelp = "No enemies will spawn on this flag.";
			break;
		}
		case mfENEMYALL:
		{
			flaghelp = "All enemies will spawn on this flag instead of using the spawn pattern.";
			break;
		}
		case mfSECRETSNEXT:
		{
			flaghelp = "When secrets are activated, the combo in this position becomes the next"
				" combo in the list.";
			break;
		}
		case mfSCRIPT1: case mfSCRIPT2: case mfSCRIPT3: case mfSCRIPT4: case mfSCRIPT5:
		case mfSCRIPT6: case mfSCRIPT7: case mfSCRIPT8: case mfSCRIPT9: case mfSCRIPT10:
		case mfSCRIPT11: case mfSCRIPT12: case mfSCRIPT13: case mfSCRIPT14: case mfSCRIPT15:
		case mfSCRIPT16: case mfSCRIPT17: case mfSCRIPT18: case mfSCRIPT19: case mfSCRIPT20:
		case mfPITHOLE: case mfPITFALLFLOOR: case mfLAVA: case mfICE: case mfICEDAMAGE:
		case mfDAMAGE1: case mfDAMAGE2: case mfDAMAGE4: case mfDAMAGE8: case mfDAMAGE16:
		case mfDAMAGE32: case mfTROWEL: case mfTROWELNEXT: case mfTROWELSPECIALITEM:
		case mfSLASHPOT: case mfLIFTPOT: case mfLIFTORSLASH: case mfLIFTROCK:
		case mfLIFTROCKHEAVY: case mfDROPITEM: case mfSPECIALITEM: case mfDROPKEY:
		case mfDROPLKEY: case mfDROPCOMPASS: case mfDROPMAP: case mfDROPBOSSKEY:
		case mfSPAWNNPC: case mfSWITCHHOOK:
		{
			flaghelp = "These flags have no built-in effect,"
				" but can be given special significance with ZASM or ZScript.";
			break;
		}
		case mfFREEZEALL: case mfFREZEALLANSFFCS: case mfFREEZEFFCSOLY: case mfSCRITPTW1TRIG:
		case mfSCRITPTW2TRIG: case mfSCRITPTW3TRIG: case mfSCRITPTW4TRIG: case mfSCRITPTW5TRIG:
		case mfSCRITPTW6TRIG: case mfSCRITPTW7TRIG: case mfSCRITPTW8TRIG: case mfSCRITPTW9TRIG:
		case mfSCRITPTW10TRIG:
		{
			flaghelp = "Not yet implemented";
			break;
		}
		case mfNOMIRROR:
		{
			flaghelp = "While touching this flag, attempting to use a Mirror item will fail.";
			break;
		}
		case mfUNSAFEGROUND:
		{
			flaghelp = "While touching this flag, the 'last safe position' will not be updated";
			break;
		}
	}
	return flaghelp;
}
void ctype_help(int32_t id)
{
	InfoDialog(ZI.getComboTypeName(id),ZI.getComboTypeHelp(id)).show();
}
void cflag_help(int32_t id)
{
	InfoDialog(ZI.getMapFlagName(id),ZI.getMapFlagHelp(id)).show();
}
//Load all the info for the combo type and checked flags
void ComboEditorDialog::refreshScript()
{
	loadComboType();
	int32_t sw_initd[8];
	for(auto q = 0; q < 8; ++q)
	{
		l_initd[q] = "InitD["+to_string(q)+"]:";
		h_initd[q].clear();
		sw_initd[q] = -1;
	}
	if(local_comboref.script)
	{
		zasm_meta const& meta = comboscripts[local_comboref.script]->meta;
		for(auto q = 0; q < 8; ++q)
		{
			if(unsigned(meta.initd_type[q]) < nswapMAX)
				sw_initd[q] = meta.initd_type[q];
			if(meta.initd[q].size())
				l_initd[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				h_initd[q] = meta.initd_help[q];
		}
	}
	else
	{
		for(auto q = 0; q < 8; ++q)
			sw_initd[q] = nswapDEC;
	}
	for(auto q = 0; q < 8; ++q)
	{
		ib_initds[q]->setDisabled(h_initd[q].empty());
		l_initds[q]->setText(l_initd[q]);
		if(sw_initd[q] > -1)
			tf_initd[q]->setSwapType(sw_initd[q]);
	}
}
void ComboEditorDialog::loadComboType()
{
	#define FL(fl) (local_comboref.usrflags & (fl))
	for(size_t q = 0; q < 16; ++q)
	{
		l_flag[q] = "Flags["+to_string(q)+"]";
		h_flag[q].clear();
		if(q > 7) continue;
		l_attribyte[q] = "Attribytes["+to_string(q)+"]:";
		l_attrishort[q] = "Attrishorts["+to_string(q)+"]:";
		h_attribyte[q].clear();
		h_attrishort[q].clear();
		if(q > 3) continue;
		l_attribute[q] = "Attributes["+to_string(q)+"]:";
		h_attribute[q].clear();
	}
	switch(local_comboref.type) //Label names
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		{
			l_attribyte[0] = "Sound:";
			h_attribyte[0] = "SFX to play during the warp";
			break;
		}
		case cSLOPE:
		{
			l_attrishort[0] = "X Offset 1:";
			h_attrishort[0] = "X Offset of the starting point of the slope line.";
			l_attrishort[1] = "Y Offset 1:";
			h_attrishort[1] = "Y Offset of the starting point of the slope line.";
			l_attrishort[2] = "X Offset 2:";
			h_attrishort[2] = "X Offset of the ending point of the slope line.";
			l_attrishort[3] = "Y Offset 2:";
			h_attrishort[3] = "Y Offset of the ending point of the slope line.";
			l_attribute[0] = "Slipperiness:";
			h_attribute[0] = "Pixels per frame to slide down the slope in sideview.";
			l_flag[0] = "Is Stairs";
			h_flag[0] = "You must hold up or be in the air for this slope to be active from the top. Bottom is always passthrough.";
			l_flag[1] = "Pass through bottom";
			h_flag[1] = "Disables the slope's collision from below, allowing sprites to pass through from the bottom.";
			l_flag[2] = "Pass through top";
			h_flag[2] = "Disables the slope's collision from above, allowing sprites to pass through from the top.";
			l_flag[3] = "Pass through left side";
			h_flag[3] = "Disables the slope's collision from the left, allowing sprites to pass through from the left side."
				"\nThis flag is not needed if you have either pass through top or pass through bottom checked.";
			l_flag[4] = "Pass through right side";
			h_flag[4] = "Disables the slope's collision from the right, allowing sprites to pass through from the right side."
				"\nThis flag is not needed if you have either pass through top or pass through bottom checked.";
			l_flag[5] = "Can fall through";
			h_flag[5] = "Treats the slope like a Sideview Platform Flag, allowing you to fall through it depending on quest rules.";
			break;
		}
		case cTRIGNOFLAG: case cSTRIGNOFLAG:
		case cTRIGFLAG: case cSTRIGFLAG:
		{
			l_attribyte[0] = "Sound:";
			h_attribyte[0] = "SFX to play when activated";
			break;
		}
		case cSTEP: case cSTEPSAME: case cSTEPALL:
		{
			l_flag[0] = "Heavy";
			h_flag[0] = "Requires Heavy Boots to activate";
			l_attribyte[0] = "Sound:";
			h_attribyte[0] = "SFX to play when activated";
			l_attribyte[1] = "Req. Item";
			h_attribyte[1] = "Item ID that must be owned in order to activate. If '0', no item is required.";
			break;
		}
		case cWATER:
		{
			l_flag[0] = "Is Lava";
			h_flag[0] = "If a liquid is Lava, it uses a different drowning sprite, and only flippers with the"
				" 'Can Swim In Lava' flag set will apply.";
			l_flag[1] = "Modify HP (Passive)";
			h_flag[1] = "If checked, the Hero's HP will change over time while in the liquid"
				" (either healing or damaging).";
			l_flag[2] = "Solid is Land";
			h_flag[2] = "Solid areas of the combo are treated as non-solid land";
			l_flag[3] = "Solid is Shallow Liquid";
			h_flag[3] = "Solid areas of the combo are treated as non-solid Shallow Liquid combo";
			l_attribute[0] = "Drown Damage:";
			h_attribute[0] = "The amount of damage dealt when drowning, in HP points. If negative, drowning will heal the Hero.";
			l_attribyte[0] = "Flipper Level:";
			h_attribyte[0] = "The minimum level flippers required to swim in the water. Flippers of lower level will have no effect.";
			l_attribyte[4] = "Drown SFX:";
			h_attribyte[4] = "The SFX played when drowning";
			l_attribyte[5] = "Splash SFX:";
			h_attribyte[5] = "SFX ID to play when walking on the liquid/shallow liquid portion of this combo. Only plays if walking and not swimming.";
			l_attribyte[6] = "Ripple Sprite:";
			h_attribyte[6] = "Sprite ID to show when walking on the liquid/shallow liquid portion of this combo. Only plays if walking and not swimming.";
			if(FL(cflag2)) //Modify HP
			{
				l_flag[4] = "Rings affect HP Mod";
				h_flag[4] = "Ring items defense reduces damage from HP Mod";
				l_flag[5] = "Mod SFX only on HP change";
				h_flag[5] = "Only play the HP Mod SFX when HP actually changes";
				l_flag[6] = "Damage causes hit anim";
				h_flag[6] = "HP Mod Damage activates the hit animation and invincibility frames";
				l_attribute[1] = "HP Modification:";
				h_attribute[1] = "How much HP should be modified by (negative for damage)";
				l_attribute[2] = "HP Mod SFX:";
				h_attribute[2] = "What SFX should play when HP is modified";
				l_attribyte[1] = "HP Delay:";
				h_attribyte[1] = "The number of frames between HP modifications";
				l_attribyte[2] = "Req Itemclass:";
				h_attribyte[2] = "If non-zero, an itemclass number which, if owned, will prevent HP modification.";
				l_attribyte[3] = "Req Itemlevel:";
				h_attribyte[3] = "A minimum item level to go with 'Req Itemclass'.";
			}
			break;
		}
		case cSHALLOWWATER:
		{
			l_flag[1] = "Modify HP (Passive)";
			h_flag[1] = "If checked, the Hero's HP will change over time while in the liquid"
				" (either healing or damaging).";
			if (get_qr(qr_OLD_SHALLOW_SFX))
			{
				l_attribyte[0] = "Splash Sound";
				h_attribyte[0] = "SFX ID to play when stepping in the shallow liquid";
			}
			else
			{
				l_attribyte[5] = "Splash Sound";
				h_attribyte[5] = "SFX ID to play when stepping in the shallow liquid";
			}
			l_attribyte[6] = "Ripple Sprite:";
			h_attribyte[6] = "Sprite ID to show when stepping in the shallow liquid";
			if(FL(cflag2)) //Modify HP
			{
				l_flag[4] = "Rings affect HP Mod";
				h_flag[4] = "Ring items defense reduces damage from HP Mod";
				l_flag[5] = "Mod SFX only on HP change";
				h_flag[5] = "Only play the HP Mod SFX when HP actually changes";
				l_flag[6] = "Damage causes hit anim";
				h_flag[6] = "HP Mod Damage activates the hit animation and invincibility frames";
				l_attribute[1] = "HP Modification:";
				h_attribute[1] = "How much HP should be modified by (negative for damage)";
				l_attribute[2] = "HP Mod SFX:";
				h_attribute[2] = "What SFX should play when HP is modified";
				l_attribyte[1] = "HP Delay:";
				h_attribyte[1] = "The number of frames between HP modifications";
				l_attribyte[2] = "Req Itemclass:";
				h_attribyte[2] = "If non-zero, an itemclass number which, if owned, will prevent HP modification.";
				l_attribyte[3] = "Req Itemlevel:";
				h_attribyte[3] = "A minimum item level to go with 'Req Itemclass'.";
			}
			break;
		}
		case cARMOS:
		{
			l_flag[0] = "Specify";
			h_flag[0] = "If checked, attribytes are used to specify enemy IDs. Otherwise, the lowest"
				" enemy ID with the armos flag checked will be spawned.";
			if(FL(cflag1))
			{
				l_flag[1] = "Random";
				h_flag[1] = "Randomly choose between two enemy IDs (50/50)";
				if(FL(cflag2))
				{
					l_attribyte[0] = "Enemy 1:";
					h_attribyte[0] = "The first enemy ID, 50% chance of being spawned";
					l_attribyte[1] = "Enemy 2:";
					h_attribyte[1] = "The second enemy ID, 50% chance of being spawned";
				}
				else
				{
					l_attribyte[0] = "Enemy:";
					h_attribyte[0] = "The enemy ID to be spawned";
				}
			}
			l_flag[2] = "Handle Large";
			h_flag[2] = "If the specified enemy is larger than 1x1 tile, attempt to use armos combos that take up its' size";
			break;
		}
		case cBSGRAVE:
			[[fallthrough]];
		case cGRAVE:
		{
			l_flag[0] = "Specify";
			h_flag[0] = "If checked, attribytes are used to specify enemy IDs. Otherwise, the lowest"
				" enemy ID with the grave flag checked will be spawned.";
			if(FL(cflag1))
			{
				l_flag[1] = "Random";
				h_flag[1] = "Randomly choose between two enemy IDs (50/50)";
				if(FL(cflag2))
				{
					l_attribyte[0] = "Enemy 1:";
					h_attribyte[0] = "The first enemy ID, 50% chance of being spawned";
					l_attribyte[1] = "Enemy 2:";
					h_attribyte[1] = "The second enemy ID, 50% chance of being spawned";
				}
				else
				{
					l_attribyte[0] = "Enemy:";
					h_attribyte[0] = "The enemy ID to be spawned";
				}
			}
			break;
		}
		case cCVUP: case cCVDOWN: case cCVLEFT: case cCVRIGHT:
		{
			l_flag[1] = "Custom Speed";
			h_flag[1] = fmt::format("Uses a custom speed/direction via attributes. If disabled, moves at 2 pixels every 3 frames in the {}ward direction.", dirstr[local_comboref.type-cCVUP]);
			l_flag[2] = "Force Dir";
			h_flag[2] = "Forces the Hero to face in the conveyor's direction";
			l_flag[4] = "Heavy Boots Disable";
			h_flag[4] = "If the Hero has boots with the 'heavy' flag, the conveyor will not push them.";
			l_flag[5] = "Force Walk";
			h_flag[5] = "The Hero will walk the speed/dir of the conveyor, unable to walk against it. Requires Newer Hero Movement." + QRHINT({qr_NEW_HERO_MOVEMENT2});
			if(FL(cflag2)) //Custom speed
			{
				l_attribute[0] = "X Speed:";
				h_attribute[0] = "Pixels moved in the X direction per rate frames.\nNegative is Left, Positive is Right";
				l_attribute[1] = "Y Speed:";
				h_attribute[1] = "Pixels moved in the Y direction per rate frames\nNegative is Up, Positive is Down";
				l_attribyte[0] = "Rate:";
				h_attribyte[0] = "Every this many frames the conveyor moves by the set speeds. If set to 0, acts as if set to 1.";
			}
			if(!FL(cflag6)) //Force Walk
			{
				l_flag[0] = "Stunned while moving";
				h_flag[0] = "While the conveyor is moving the Hero, they are 'stunned'.";
				l_flag[3] = "Smart Corners";
				h_flag[3] = "Uses the half-combo-grid to help avoid getting stuck on corners";
			}
			break;
		}
		case cTALLGRASS: case cTALLGRASSTOUCHY: case cTALLGRASSNEXT:
		{
			l_flag[0] = "Decoration Sprite";
			h_flag[0] = "Spawn a decoration when slashed";
			if(FL(cflag1))
			{
				l_flag[9] = "Use Clippings Sprite";
				h_flag[9] = "Use a system clipping sprite instead of a Sprite Data sprite";
				if(FL(cflag10))
				{
					l_attribyte[0] = "Clipping Sprite:";
					h_attribyte[0] = "0 and 1 = Bush Leaves, 2 = Flowers, 3 = Grass";
				}
				else
				{
					l_attribyte[0] = "Sprite:";
					h_attribyte[0] = "Sprite Data sprite ID to display as a clipping";
				}
			}
			l_flag[1] = "Set Dropset";
			h_flag[1] = "Allows specifying the dropset to use as an attribyte";
			l_flag[2] = "Custom Slash SFX";
			h_flag[2] = "Specify a custom slash SFX";
			l_attribyte[3] = "Walking Sound:";
			h_attribyte[3] = "The SFX to play when the Hero walks through this combo. If 0, no sound is played.";
			l_attribyte[6] = "Grass Sprite";
			h_attribyte[6] = "The sprite to show when the Hero walks through this combo.";
			if(FL(cflag2))
			{
				l_flag[10] = "Specific Item";
				h_flag[10] = "Drop a specific item instead of an item from a dropset";
				if(FL(cflag11))
				{
					l_attribyte[1] = "Item:";
					h_attribyte[1] = "The item ID to drop";
				}
				else
				{
					l_attribyte[1] = "Dropset:";
					h_attribyte[1] = "The dropset to select a drop item from";
				}
			}
			if(FL(cflag3))
			{
				l_attribyte[2] = "Slash Sound:";
				h_attribyte[2] = "The SFX to play when slashed";
			}
			break;
		}
		case cBUSH: case cBUSHTOUCHY: case cFLOWERS: case cSLASHNEXTTOUCHY:
		case cSLASHITEM: case cSLASHNEXTITEMTOUCHY:
		case cSLASHNEXTITEM: case cBUSHNEXT: case cSLASHITEMTOUCHY:
		case cFLOWERSTOUCHY: case cBUSHNEXTTOUCHY:
		{
			l_flag[0] = "Decoration Sprite";
			h_flag[0] = "Spawn a decoration when slashed";
			if(FL(cflag1))
			{
				l_flag[9] = "Use Clippings Sprite";
				h_flag[9] = "Use a system clipping sprite instead of a Sprite Data sprite";
				if(FL(cflag10))
				{
					l_attribyte[0] = "Clipping Sprite:";
					h_attribyte[0] = "0 and 1 = Bush Leaves, 2 = Flowers, 3 = Grass";
				}
				else
				{
					l_attribyte[0] = "Sprite:";
					h_attribyte[0] = "Sprite Data sprite ID to display as a clipping";
				}
			}
			l_flag[1] = "Set Dropset";
			h_flag[1] = "Allows specifying the dropset to use as an attribyte";
			l_flag[2] = "Custom SFX";
			h_flag[2] = "Specify a custom slash SFX";
			if(FL(cflag2))
			{
				l_flag[10] = "Specific Item";
				h_flag[10] = "Drop a specific item instead of an item from a dropset";
				if(FL(cflag11))
				{
					l_attribyte[1] = "Item:";
					h_attribyte[1] = "The item ID to drop";
				}
				else
				{
					l_attribyte[1] = "Dropset:";
					h_attribyte[1] = "The dropset to select a drop item from";
				}
			}
			if(FL(cflag3))
			{
				l_attribyte[2] = "Slash Sound:";
				h_attribyte[2] = "The SFX to play when slashed";
			}
			break;
		}
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		{
			l_flag[0] = "Custom Damage";
			h_flag[0] = "Uses custom damage amount";
			l_flag[1] = "No Knockback";
			h_flag[1] = "Does not knock the Hero back when damaging them if checked. Otherwise, knocks the Hero in the direction"
				" opposite the one they face.";
			if(FL(cflag1))
			{
				l_attribute[0] = "Damage:";
				h_attribute[0] = "The amount of damage, in HP, to deal. Negative amounts heal."
					"\nFor healing, the lowest healing amount combo you are standing on takes effect."
					"\nFor damage, the greatest amount takes priority unless 'Quest->Options->Combos->Lesser Damage Combos Take Priority' is checked.";
			}
			break;
		}
		case cLOCKBLOCK:
		{
			l_flag[0] = "Use Item";
			h_flag[0] = "Allow an item in your inventory to unlock the block";
			if(FL(cflag1))
			{
				l_flag[4] = "Eat Item";
				h_flag[4] = "Consume the required item instead of simply requiring its presence";
				if(FL(cflag5))
				{
					l_attribyte[0] = "Consumed Item";
					h_attribyte[0] = "The Item ID to open the lock block. Consumed.";
				}
				else
				{
					l_attribyte[0] = "Held Item";
					h_attribyte[0] = "The Item ID to open the lock block. Not consumed.";
				}
				l_flag[1] = "Require Item";
				h_flag[1] = "Only the required item can open this block (instead of ALSO allowing a key)";
			}
			if(!(FL(cflag1)&&FL(cflag2)))
			{
				l_attribute[0] = "Amount:";
				if(FL(cflag4))
					h_attribute[0] = "The amount of the arbitrary counter required to open this block";
				else
					h_attribute[0] = "The amount of keys required to open this block";
				l_flag[3] = "Counter";
				h_flag[3] = "If checked, uses an arbitrary counter instead of keys";
				if(FL(cflag4))
				{
					l_attribyte[1] = "Counter:";
					h_attribyte[1] = "The counter to use to open this block";
					l_flag[7] = "No Drain";
					h_flag[7] = "Requires the counter have the amount, but do not consume from it";
					if(!FL(cflag8))
					{
						l_flag[5] = "Thief";
						h_flag[5] = "Consumes from counter even if you don't have enough";
					}
				}
			}
		}
		[[fallthrough]];
		case cBOSSLOCKBLOCK:
		{
			l_flag[8] = "Can't use from top";
			h_flag[8] = "Cannot be activated standing to the top side if checked";
			l_flag[9] = "Can't use from bottom";
			h_flag[9] = "Cannot be activated standing to the bottom side if checked";
			l_flag[10] = "Can't use from left";
			h_flag[10] = "Cannot be activated standing to the left side if checked";
			l_flag[11] = "Can't use from right";
			h_flag[11] = "Cannot be activated standing to the right side if checked";
			l_flag[12] = "Display prompt combo";
			h_flag[12] = "Displays a prompt combo when able to interact\n"
				"Must set: Combo, Xoffset, Yoffset, CSet";
			l_attribyte[2] = "Button:";
			h_attribyte[2] = "Sum all the buttons you want to be usable:\n(A=1, B=2, L=4, R=8, Ex1=16, Ex2=32, Ex3=64, Ex4=128)\n"
				"If no buttons are selected, walking into the block will activate it.";
			l_attribyte[3] = "Unlock Sound:";
			h_attribyte[3] = "The sound to play when unlocking the block";
			if(FL(cflag13))
			{
				l_attribute[1] = "Prompt Combo";
				h_attribute[1] = "Combo to display as a 'prompt'";
				l_attribute[2] = "Locked Prompt Combo";
				h_attribute[2] = "Combo to display as a 'prompt', if you are not currently able to"
					" open it. If 0, the normal prompt will be used instead.";
				l_attrishort[0] = "Prompt Xoffset";
				h_attrishort[0] = "X offset from Hero's position for the prompt to display at";
				l_attrishort[1] = "Prompt Yoffset";
				h_attrishort[1] = "Y offset from Hero's position for the prompt to display at";
				l_attribyte[4] = "Prompt CSet";
				h_attribyte[4] = "CSet to draw the prompt in";
			}
			
			l_attribute[3] = "Locked String:";
			h_attribute[3] = "If non-0, plays the specified string on attempting to open and failing.\n"
				"1+: Use specified string\n"
				"-1: Use screen string\n"
				"-2: Use screen catchall as string\n"
				"-10 to -17: Use Screen->D[0] to [7] as string";
		}
		[[fallthrough]];
		case cLOCKBLOCK2:
		case cBOSSLOCKBLOCK2:
		{
			std::string ss_str;
			switch(local_comboref.type)
			{
				case cLOCKBLOCK: case cLOCKBLOCK2: ss_str = "Lockblock"; break;
				case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2: ss_str = "Boss Lockblock"; break;
			}
			l_flag[15] = "Use ExtraState";
			h_flag[15] = "If checked, the 'extra state' specified in the attribytes"
				" will be used instead of the usual '" + ss_str + "' screen state.";
			if(FL(cflag16))
			{
				l_attribyte[5] = "ExtraState";
				h_attribyte[5] = "Which ExtraState (0 to 31) to use instead of the usual '"
					+ ss_str + "' screen state.";
			}
			break;
		}
		case cLOCKEDCHEST:
		{
			l_flag[0] = "Use Item";
			h_flag[0] = "Allow an item in your inventory to unlock the chest";
			if(FL(cflag1))
			{
				l_flag[4] = "Eat Item";
				h_flag[4] = "Consume the required item instead of simply requiring its presence";
				if(FL(cflag5))
				{
					l_attribyte[0] = "Consumed Item";
					h_attribyte[0] = "The Item ID to open the chest. Consumed.";
				}
				else
				{
					l_attribyte[0] = "Held Item";
					h_attribyte[0] = "The Item ID to open the chest. Not consumed.";
				}
				l_flag[1] = "Require Item";
				h_flag[1] = "Only the required item can open this chest (instead of ALSO allowing a key)";
			}
			if(!(FL(cflag1)&&FL(cflag2)))
			{
				l_attribute[0] = "Amount:";
				if(FL(cflag4))
					h_attribute[0] = "The amount of the arbitrary counter required to open this chest";
				else
					h_attribute[0] = "The amount of keys required to open this chest";
				l_flag[3] = "Counter";
				h_flag[3] = "If checked, uses an arbitrary counter instead of keys";
				if(FL(cflag4))
				{
					l_attribyte[1] = "Counter:";
					h_attribyte[1] = "The counter to use to open this block";
					l_flag[7] = "No Drain";
					h_flag[7] = "Requires the counter have the amount, but do not consume from it";
					if(!FL(cflag8))
					{
						l_flag[5] = "Thief";
						h_flag[5] = "Consumes from counter even if you don't have enough";
					}
				}
			}
		}
		[[fallthrough]];
		case cBOSSCHEST:
		{
			if(FL(cflag13)) //Prompt flag
			{
				l_attribute[2] = "Locked Prompt Combo";
				h_attribute[2] = "Combo to display as a 'prompt', if you are not currently able to"
					" open it. If 0, the normal prompt will be used instead.";
			}
			
			l_attribute[3] = "Locked String:";
			h_attribute[3] = "If non-0, plays the specified string on attempting to open and failing.\n"
				"1+: Use specified string\n"
				"-1: Use screen string\n"
				"-2: Use screen catchall as string\n"
				"-10 to -17: Use Screen->D[0] to [7] as string";
		}
		[[fallthrough]];
		case cCHEST:
		{
			l_flag[8] = "Can't use from top";
			h_flag[8] = "Cannot be activated standing to the top side if checked";
			l_flag[9] = "Can't use from bottom";
			h_flag[9] = "Cannot be activated standing to the bottom side if checked";
			l_flag[10] = "Can't use from left";
			h_flag[10] = "Cannot be activated standing to the left side if checked";
			l_flag[11] = "Can't use from right";
			h_flag[11] = "Cannot be activated standing to the right side if checked";
			l_flag[12] = "Display prompt combo";
			h_flag[12] = "Displays a prompt combo when able to interact\n"
				"Must set: Combo, Xoffset, Yoffset, CSet";
			l_attribyte[2] = "Button:";
			h_attribyte[2] = "Sum all the buttons you want to be usable:\n(A=1, B=2, L=4, R=8, Ex1=16, Ex2=32, Ex3=64, Ex4=128)\n"
				"If no buttons are selected, walking into the chest will activate it.";
			l_attribyte[3] = "Open Sound:";
			h_attribyte[3] = "The sound to play when opening the chest";
			if(FL(cflag13))
			{
				l_attribute[1] = "Prompt Combo";
				h_attribute[1] = "Combo to display as a 'prompt'";
				l_attrishort[0] = "Prompt Xoffset";
				h_attrishort[0] = "X offset from Hero's position for the prompt to display at";
				l_attrishort[1] = "Prompt Yoffset";
				h_attrishort[1] = "Y offset from Hero's position for the prompt to display at";
				l_attribyte[4] = "Prompt CSet";
				h_attribyte[4] = "CSet to draw the prompt in";
			}
			l_attrishort[2] = "Contained Item";
			h_attrishort[2] = "The item contained by the chest."
				"\n0-256: Use specified item ID\n"
				"-1: Use screen special item (catchall)\n"
				"-10 to -17: Use Screen->D[0] to [7] as item ID";
			l_flag[6] = "Use SItem State";
			h_flag[6] = "Link contained item to 'Special Item' screen state.";
		}
		[[fallthrough]];
		case cCHEST2:
		case cLOCKEDCHEST2:
		case cBOSSCHEST2:
		{
			std::string ss_str;
			switch(local_comboref.type)
			{
				case cCHEST: case cCHEST2: ss_str = "Chest"; break;
				case cLOCKEDCHEST: case cLOCKEDCHEST2: ss_str = "Locked Chest"; break;
				case cBOSSCHEST: case cBOSSCHEST2: ss_str = "Boss Chest"; break;
			}
			l_flag[15] = "Use ExtraState";
			h_flag[15] = "If checked, the 'extra state' specified in the attribytes"
				" will be used instead of the usual '" + ss_str + "' screen state.";
			if(FL(cflag16))
			{
				l_attribyte[5] = "ExtraState";
				h_attribyte[5] = "Which ExtraState (0 to 31) to use instead of the usual '"
					+ ss_str + "' screen state.";
			}
			break;
		}
		case cSIGNPOST:
		{
			l_flag[8] = "Can't use from top";
			h_flag[8] = "Cannot be activated standing to the top side if checked";
			l_flag[9] = "Can't use from bottom";
			h_flag[9] = "Cannot be activated standing to the bottom side if checked";
			l_flag[10] = "Can't use from left";
			h_flag[10] = "Cannot be activated standing to the left side if checked";
			l_flag[11] = "Can't use from right";
			h_flag[11] = "Cannot be activated standing to the right side if checked";
			l_flag[12] = "Display prompt combo";
			h_flag[12] = "Displays a prompt combo when able to interact\n"
				"Must set: Combo, Xoffset, Yoffset, CSet";
			l_attribyte[2] = "Button:";
			h_attribyte[2] = "Sum all the buttons you want to be usable:\n(A=1, B=2, L=4, R=8, Ex1=16, Ex2=32, Ex3=64, Ex4=128)\n"
				"If no buttons are selected, walking into the signpost will activate it.";
			l_attribute[0] = "String:";
			h_attribute[0] = "1+: Use specified string\n"
				"-1: Use screen string\n"
				"-2: Use screen catchall as string\n"
				"-10 to -17: Use Screen->D[0] to [7] as string";
			if(FL(cflag13))
			{
				l_attribute[1] = "Prompt Combo";
				h_attribute[1] = "Combo to display as a 'prompt'";
				l_attrishort[0] = "Prompt Xoffset";
				h_attrishort[0] = "X offset from Hero's position for the prompt to display at";
				l_attrishort[1] = "Prompt Yoffset";
				h_attrishort[1] = "Y offset from Hero's position for the prompt to display at";
				l_attribyte[4] = "Prompt CSet";
				h_attribyte[4] = "CSet to draw the prompt in";
			}
			break;
		}
		case cBUTTONPROMPT:
		{
			l_attribute[0] = "Prompt Combo";
			h_attribute[0] = "Combo to display as a 'prompt'";
			l_attrishort[0] = "Prompt Xoffset";
			h_attrishort[0] = "X offset from Hero's position for the prompt to display at";
			l_attrishort[1] = "Prompt Yoffset";
			h_attrishort[1] = "Y offset from Hero's position for the prompt to display at";
			l_attribyte[0] = "Prompt CSet";
			h_attribyte[0] = "CSet to draw the prompt in";
			break;
		}
		case cCUSTOMBLOCK:
		{
			l_attribyte[0] = "Block SFX";
			h_attribyte[0] = "SFX to play when blocking a weapon";
			break;
		}
		case cSHOOTER:
		{
			l_attribyte[0] = "Shot SFX:";
			h_attribyte[0] = "SFX to play when shooting a weapon";
			l_attribyte[1] = "Weapon Type:";
			h_attribyte[1] = "The LWeapon or EWeapon ID to be shot";
			l_attribyte[2] = "Sprite:";
			h_attribyte[2] = "The sprite of the spawned weapon";
			//byte[3] : multishot shot count
			l_attribyte[4] = "Unblockable:";
			h_attribyte[4] = "Sum the following values to create a flagset:"
				"\n1: Bypass 'Block' defense"
				"\n2: Bypass 'Ignore' defense"
				"\n4: Bypass enemy/Hero shield blocking"
				"\n8: Bypass Hero shield reflecting";
			l_attribyte[5] = "Script:";
			h_attribyte[5] = "LWeapon or EWeapon script ID to attach to the fired weapons."
				"\nNote that there is no way to supply InitD to such scripts.";
			l_attribyte[6] = "Parent Item:";
			h_attribyte[6] = "The item ID to use as the 'parent item' of the weapon. Only used for LWeapons. 0 = no parent."
				"\nThis affects various attributes of certain lweapons, such as a bomb's fuse.";
			//short[0],[1] : Rate
			l_attrishort[2] = "Damage:";
			h_attrishort[2] = "The damage of the spawned weapon";
			
			//bute[0] : Angle/Dir
			//bute[1] : Prox Limit
			//bute[3] : Multishot Spread
			l_attribute[2] = "Step Speed:";
			h_attribute[2] = "The speed of the weapon, in 100ths px/frame";
			
			l_flag[0] = "Angular";
			h_flag[0] = "Specify an angle (in degrees) instead of a direction (8dir)";
			l_flag[1] = "Variable Rate";
			h_flag[1] = "Fires at a varying rate instead of a constant rate";
			l_flag[2] = "Instant Shot";
			h_flag[2] = "Shoots when the timer starts, rather than ends";
			l_flag[3] = "Stops by Hero Proximity";
			h_flag[3] = "If the Hero is within the specified number of pixels, the shooter will be unable to shoot.";
			l_flag[4] = "'Custom Weapons' are LWeapons";
			h_flag[4] = "If a 'Custom Weapon' ID is used, it will be treated as an LWeapon with this checked, and an EWeapon otherwise.";
			l_flag[5] = "Auto-rotate sprite";
			h_flag[5] = "Attempt to rotate the sprite to match the weapon's angle";
			l_flag[6] = "Multi-Shot";
			h_flag[6] = "Shoot multiple weapons at once";
			l_flag[7] = "Boss Fireball";
			h_flag[7] = "If a fireball weapon type is used, it will be considered a 'boss' fireball.";
			if(FL(cflag1)) //Angular
			{
				l_attribute[0] = "Angle (Degrees)";
				h_attribute[0] = "If between 0 and 360, acts as an angle in degrees."
					"\nUse '-1' to aim at the Hero (4-dir)"
					"\nUse '-2' to aim at the Hero (8-dir)"
					"\nUse '-3' to aim at the Hero (angular)";
			}
			else
			{
				l_attribute[0] = "Direction";
				h_attribute[0] = "A direction from 0 to 7. 0 = up, 1 = down, etc.";
			}
			if(FL(cflag2)) //Variable rate
			{
				l_attrishort[0] = "Lower Fire Rate:";
				h_attrishort[0] = "If lower than the 'Upper Fire Rate', the combo will fire between the two rates. (in frames)";
				l_attrishort[1] = "Upper Fire Rate:";
				h_attrishort[1] = "If higher than the 'Lower Fire Rate', the combo will fire between the two rates. (in frames)";
			}
			else
			{
				l_attrishort[0] = "Fire Rate:";
				h_attrishort[0] = "Combo fires every this many frames (0 = don't fire)";
			}
			if(FL(cflag4)) //Stops by Hero Proximity
			{
				l_attribute[1] = "Proximity Limit";
				h_attribute[1] = "If the Hero is at least this close (in pixels)"
					" to the combo, the combo will fail to shoot.";
				l_flag[8] = "Invert Proximity";
				h_flag[8] = "If checked, the combo will fail to shoot if the"
					" Hero is FARTHER than specified.";
			}
			if(FL(cflag7)) //Multi Shot
			{
				l_attribyte[3] = "Shot Count";
				h_attribyte[3] = "How many shots (min 1) to fire";
				l_attribute[3] = "Shot Spread";
				h_attribute[3] = "Angle (in degrees) between each weapon (0 to 360)";
			}
			break;
		}
		case cCUTSCENETRIG:
		{
			l_flag[0] = "End Cutscene";
			h_flag[0] = "If checked, triggering this combo with ComboType Effects will end any active cutscene.";
			if(!FL(cflag1))
			{
				l_flag[1] = "Disable F6";
				h_flag[1] = "The cutscene activated by this combo will not allow F6";
				l_attribute[0] = "Allowed Buttons";
				h_attribute[0] = "A bitwise flagset of the buttons that are allowed. Accessed *in LONG ENTRY MODE*:"
					"\nUp=1,Down=2,Left=4,Right=8,A=16,B=32,Start=64,L=128,"
					"\nR=256,Map=512,Ex1=1024,Ex2=2048,Ex3=4096,Ex4=8192,"
					"\nStickUp=16384,StickDown=32768,StickLeft=65536,StickRight=131072"
					"\nAdd the values of whichever buttons you would like to ALLOW during the cutscene.";
				l_attribyte[0] = "Error SFX";
				h_attribyte[0] = "If >0, SFX played when trying to press a disabled button";
			}
			break;
		}
		case cSLASHNEXT:
		{
			l_flag[0] = "Decoration Sprite";
			h_flag[0] = "Spawn a decoration when slashed";
			if(FL(cflag1))
			{
				l_flag[9] = "Use Clippings Sprite";
				h_flag[9] = "Use a system clipping sprite instead of a Sprite Data sprite";
				if(FL(cflag10))
				{
					l_attribyte[0] = "Clipping Sprite:";
					h_attribyte[0] = "0 and 1 = Bush Leaves, 2 = Flowers, 3 = Grass";
				}
				else
				{
					l_attribyte[0] = "Sprite:";
					h_attribyte[0] = "Sprite Data sprite ID to display as a clipping";
				}
			}
			l_flag[2] = "Custom SFX";
			h_flag[2] = "Specify a custom slash SFX";
			if(FL(cflag3))
			{
				l_attribyte[2] = "Slash Sound:";
				h_attribyte[2] = "The SFX to play when slashed";
			}
			break;
		}
		case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
		case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
		case cSCRIPT11: case cSCRIPT12: case cSCRIPT13: case cSCRIPT14: case cSCRIPT15:
		case cSCRIPT16: case cSCRIPT17: case cSCRIPT18: case cSCRIPT19: case cSCRIPT20:
		{
			l_flag[8] = "Generic";
			h_flag[8] = "Attributes/flags act like the Generic combo type.";
			if(!(FL(cflag9))) //Generic flag not set
				break;
		}
		[[fallthrough]];
		case cTRIGGERGENERIC:
		{
			l_flag[0] = "Decoration Sprite";
			h_flag[0] = "Spawn a decoration when activated";
			if(FL(cflag1))
			{
				l_flag[9] = "Use Clippings Sprite";
				h_flag[9] = "Use a system clipping sprite instead of a Sprite Data sprite";
				if(FL(cflag10))
				{
					l_attribyte[0] = "Clipping Sprite:";
					h_attribyte[0] = "0 and 1 = Bush Leaves, 2 = Flowers, 3 = Grass";
				}
				else
				{
					l_attribyte[0] = "Sprite:";
					h_attribyte[0] = "Sprite Data sprite ID to display as a decoration";
				}
			}
			l_flag[1] = "Drop Item";
			h_flag[1] = "Drop an item when activated";
			if(FL(cflag2)) //Drop item
			{
				l_flag[10] = "Specific Item";
				h_flag[10] = "Drop a specific item instead of an item from a dropset";
				if(FL(cflag11))
				{
					l_attribyte[1] = "Item:";
					h_attribyte[1] = "The item ID to drop";
				}
				else
				{
					l_attribyte[1] = "Dropset:";
					h_attribyte[1] = "The dropset to select a drop item from";
				}
			}
			l_flag[3] = "Change Combo";
			h_flag[3] = "Become the next combo in the combo list when activated";
			if(FL(cflag4))
			{
				l_flag[11] = "Undercombo";
				h_flag[11] = "Becomes the screen undercombo instead of next combo in the list";
				if(!FL(cflag12))
				{
					l_flag[4] = "Continuous Trigger";
					h_flag[4] = "When changing to a new combo, if the new combo is a generic combo with continuous flag set, skip it";
				}
			}
			l_flag[7] = "Kill Wpn";
			h_flag[7] = "Destroy the weapon that activates this combo";
			l_flag[13] = "Drop Enemy";
			h_flag[13] = "Spawn an Enemy when activated";
			if(FL(cflag14))
			{
				l_attribyte[4] = "Enemy ID";
				h_attribyte[4] = "The Enemy ID to spawn";
			}
			
			l_flag[6] = "Trigger Singular Secret";
			h_flag[6] = "Triggers a single secret flag temporarily";
			if(FL(cflag7|cflag4))
			{
				l_attribyte[2] = "SFX:";
				switch(FL(cflag7|cflag4))
				{
					case cflag4:
						h_attribyte[2] = "SFX to play when changing combo.";
						break;
					case cflag7:
						h_attribyte[2] = "SFX to play when activating singular secret.";
						break;
					case cflag4|cflag7:
						h_attribyte[2] = "SFX to play when activating singular secret or changing combo";
						break;
				}
			}
			if(FL(cflag7))
			{
				l_attribyte[3] = "Singular Secret:";
				h_attribyte[3] = "Which single secret combo to activate, using the 'SECCMB_' constants from 'include/std_zh/std_constants.zh'";
			}
			if(FL(cflag14)) //Drop Enemy flag
			{
				l_flag[12] = "No Poof";
				h_flag[12] = "Skip spawn poof for dropped enemy";
			}
			l_flag[5] = "Room Item";
			h_flag[5] = "Drop the room's Special Item on activation"
				" (room does not need to be 'Special Item' type, the Catchall value will be used)";
			break;
		}
		case cPITFALL:
		{
			l_flag[0] = "Warp";
			h_flag[0] = "Warp to another screen using a tile warp when falling";
			l_flag[2] = "Damage is Percent";
			h_flag[2] = "The damage amount is a percentage of the Hero's max life";
			l_flag[3] = "Allow Ladder";
			h_flag[3] = "A ladder with 'Can step over pitfalls' checked can step over this combo";
			l_flag[4] = "No Pull";
			h_flag[4] = "Don't suck in the Hero at all";
			l_attribute[0] = "Damage:";
			h_attribute[0] = "The amount of damage, in HP, to take when falling. Negative values heal.";
			l_attribyte[0] = "Fall SFX:";
			h_attribyte[0] = "The SFX to play when falling";
			if(FL(cflag1)) //Warp enabled
			{
				l_flag[1] = "Direct Warp";
				h_flag[1] = "The warp keeps the Hero at the same x/y position";
				l_attribyte[1] = "TileWarp ID";
				h_attribyte[1] = "0 = A, 1 = B, 2 = C, 3 = D";
			}
			if(!(FL(cflag5))) //"No Pull"
			{
				l_attribyte[2] = "Pull Sensitivity:";
				h_attribyte[2] = "Pull the Hero 1 pixel every this many frames.\n"
					"If set to 0, pulls 2 pixels every frame.";
			}
			break;
		}
		case cSTEPSFX:
		{
			l_flag[0] = "Landmine (Step->Wpn)";
			h_flag[0] = "Spawns a weapon when activated, and by default advances to the next combo in the combo list.";
			l_attribyte[0] = "Sound:";
			h_attribyte[0] = "SFX to play when stepped on";
			if(FL(cflag1)) //Landmine
			{
				l_flag[1] = "Script weapon IDs spawn LWeapons";
				h_flag[1] = "Script weapon IDs for 'Weapon Type' are EWeapons by default; if checked, they will be LWeapons instead.";
				l_flag[2] = "Don't Advance";
				h_flag[2] = "If checked, the combo will not advance to the next combo when activated."
					" This may cause the landmine to activate multiple times in a row.";
				l_flag[3] = "Direct Damage Script LW / Sparkles";
				h_flag[3] = "If the weapon type is a Script weapon and 'Script Weapon IDs spawn LWeapons' is checked, or the weapon type is"
					" a sparkle type, it will immediately damage the Hero (knocking them back none).";
				l_attribute[0] = "Damage:";
				h_attribute[0] = "The damage value for the spawned weapon. If this is < 1, it will default to 4 damage.";
				l_attribyte[1] = "Weapon Type:";
				h_attribyte[1] = "The weapon type to spawn. Script1-10 weapon types are eweapons by default."
					" If 0 or invalid, uses an enemy bomb type as a default.";
				l_attribyte[2] = "Weapon Dir:";
				h_attribyte[2] = "Direction for the weapon. 0-7 are the standard dirs, 8+ selects a random dir.";
				l_attribyte[3] = "Wpn Sprite:";
				h_attribyte[3] = "The 'Sprite Data' sprite to use for the spawned weapon. Only valid if 1 to 255.";
				l_attribyte[4] = "Parent Item:";
				h_attribyte[4] = "The item ID to use as the 'parent item' of the weapon. Only used for LWeapons. 0 = no parent."
					"\nThis affects various attributes of certain lweapons, such as a bomb's fuse.";
			}
			break;
		}
		case cCSWITCH:
		{
			l_flag[0] = "Kill Wpn";
			h_flag[0] = "Destroy the weapon that activates the combo";
			l_flag[7] = "Skip Cycle on Screen Entry";
			h_flag[7] = "Combo cycle the switch combo on screen entry, to skip any switching animation";
			
			l_attribute[0] = "Combo Change:";
			h_attribute[0] = "Value to add to the combo ID when activated";
			l_attribute[1] = "CSet Change:";
			h_attribute[1] = "Value to add to the cset when activated";
			l_attribyte[1] = "SFX:";
			h_attribyte[1] = "SFX to play when activated";
			l_flag[10] = "Global State";
			h_flag[10] = "Use a global state instead of a level-based state.";
			if(FL(cflag11)) //Global State
			{
				l_attribyte[0] = "State Num:";
				h_attribyte[0] = "Range 0-255 inclusive, which of the global switch states to activate from";
				l_attribute[2] = "Timed State";
				h_attribute[2] = "If > 0, the state will revert after this many frames."
					" If used, the state will NOT remain through save/load."
					" Also, timed switches can only turn the switch state 'on', cannot toggle it back 'off'."
					" Hitting the switch while already on will reset the timer.";
			}
			else
			{
				l_attribyte[0] = "State Num:";
				h_attribyte[0] = "Range 0-31 inclusive, which of the level's switch states to activate from";
			}
			break;
		}
		case cCSWITCHBLOCK:
		{
			l_flag[0] = "Change L0"; l_flag[1] = "Change L1";
			l_flag[2] = "Change L2"; l_flag[3] = "Change L3";
			l_flag[4] = "Change L4"; l_flag[5] = "Change L5";
			l_flag[6] = "Change L6";
			h_flag[0] = "Changes the combo on layer 0 in the same pos as this combo when activated.";
			h_flag[1] = "Changes the combo on layer 1 in the same pos as this combo when activated.";
			h_flag[2] = "Changes the combo on layer 2 in the same pos as this combo when activated.";
			h_flag[3] = "Changes the combo on layer 3 in the same pos as this combo when activated.";
			h_flag[4] = "Changes the combo on layer 4 in the same pos as this combo when activated.";
			h_flag[5] = "Changes the combo on layer 5 in the same pos as this combo when activated.";
			h_flag[6] = "Changes the combo on layer 6 in the same pos as this combo when activated.";
			l_flag[7] = "Skip Cycle on Screen Entry";
			h_flag[7] = "Combo cycle the switch combo on screen entry, to skip any rising/falling animation";
			l_flag[8] = "Allow walk-on-top";
			h_flag[8] = "Allows the Hero to walk along solid switchblocks if they are on them";
			l_attribute[0] = "Combo Change:";
			h_attribute[0] = "Value to add to the combo ID when activated";
			l_attribute[1] = "CSet Change:";
			h_attribute[1] = "Value to add to the cset when activated";
			if(FL(cflag9)) //Allow walk-on-top
			{
				l_flag[9] = "-8px DrawYOffset";
				h_flag[9] = "If enabled, when the Hero stands atop the block (solid area), the Hero's DrawYOffset is decremented by 8."
					" When the Hero leaves the block, the DrawYOffset is incremented back by 8.";
				l_attribute[2] = "Z-value:";
				h_attribute[2] = "A Z-height for the block, allowing you to jump atop it, and from block to block."
					" If set to 0, acts as infinitely tall.";
				l_attribute[3] = "Step Height:";
				h_attribute[3] = "The Z amount below the block's Z-height that you can jump atop it from. This allows"
					" for 'walking up stairs' type effects.";
			}
			l_flag[10] = "Global State";
			h_flag[10] = "Use a global state instead of a level-based state.";
			if(FL(cflag11)) //Global State
			{
				l_attribyte[0] = "State Num:";
				h_attribyte[0] = "Range 0-255 inclusive, which of the global switch states to activate from";
			}
			else
			{
				l_attribyte[0] = "State Num:";
				h_attribyte[0] = "Range 0-31 inclusive, which of the level's switch states to activate from";
			}
			break;
		}
		case cTORCH:
		{
			l_attribyte[0] = "Radius:";
			h_attribyte[0] = "The radius of light, in pixels, to light up in dark rooms.";
			l_attribyte[1] = "Shape:";
			h_attribyte[1] = "The shape of light. 0=circle, 1=cone, 2=square";
			l_attribyte[2] = "Direction:";
			h_attribyte[2] = "The direction the torch is facing, for direction-requiring shapes like 'cone'.";
			break;
		}
		case cSPOTLIGHT:
		{
			l_flag[0] = "Use Tiles instead of Colors";
			h_flag[0] = "Uses a set of tiles in a preset order, instead of a set of 3 colors, to represent the light beam.";
			l_flag[1] = "Works on FFCs";
			h_flag[1] = "This spotlight can fire a beam from FFCs, off-the-grid";
			l_attribyte[0] = "Dir:";
			h_attribyte[0] = "0-3 = Up,Down,Left,Right\n4-7 = Unused (For Now)\n8 = at the ground";
			l_attribyte[4] = "Trigger Set:";
			h_attribyte[4] = "0-32; if 0 will activate any targets, otherwise only activates matching targets";
			if(FL(cflag1))
			{
				l_attribute[0] = "Start Tile:";
				h_attribute[0] = "Tiles in order: Ground, Up, Down, Left, Right, U+L, U+R, D+L, D+R, U+D, L+R, D+L+R, U+L+R, U+D+R, U+D+L, U+D+L+R";
				l_attribyte[1] = "CSet (0-11):";
				h_attribyte[1] = "CSet for the light beam graphic";
			}
			else
			{
				l_attribyte[1] = "Inner Color:";
				h_attribyte[1] = "One of the colors used to generate the light beam graphic";
				l_attribyte[2] = "Middle Color:";
				h_attribyte[2] = "One of the colors used to generate the light beam graphic";
				l_attribyte[3] = "Outer Color:";
				h_attribyte[3] = "One of the colors used to generate the light beam graphic";
			}
			if(FL(cflag2))
			{
				l_attribyte[5] = "Beam Width (FFC)";
				h_attribyte[5] = "The 'width' of the beam. This is only used when the spotlight"
					" is on an FFC, and is used to center the beam and determine the hitbox."
					"\nDoes not affect the visual in any way. If < 1, uses '8' as a default.";
			}
			break;
		}
		case cLIGHTTARGET:
		{
			l_flag[0] = "Lit Version";
			h_flag[0] = "If checked, reverts to previous combo when not hit by a spotlight."
				"\nIf unchecked, becomes the next combo when hit by a spotlight.";
			l_flag[1] = "Invert";
			h_flag[1] = "If checked, counts as activated when light is NOT hitting it.";
			l_flag[2] = "Blocks Light";
			h_flag[2] = "Light that hits will activate, but not pass, the target";
			l_attribyte[4] = "Trigger Set:";
			h_attribyte[4] = "0-32; if 0 will be activated by any beams, otherwise only by matching beams";
			break;
		}
		case cSWITCHHOOK:
		{
			l_flag[0] = "Only swap with Combo 0";
			h_flag[0] = "The switch will fail if this combo would be swapped with a non-zero combo";
			l_flag[1] = "Swap Placed Flags";
			h_flag[1] = "Placed flags on the same layer and position as this combo will be swapped along with the combo";
			l_flag[2] = "Break upon swap";
			h_flag[2] = "The combo will 'break' upon swapping, displaying a break sprite and potentially"
				" dropping an item. Instead of swapping with the combo under the Hero,"
				" it will be replaced by the screen's Undercombo.";
			l_flag[6] = "Counts as 'pushblock'";
			h_flag[6] = "This combo counts as a 'pushblock' for purposes of switching it onto"
				" block triggers/holes, though this does not allow it to be pushed"
				" (unless a push flag is placed on it).";
			l_attribyte[0] = "Hook Level:";
			h_attribyte[0] = "The minimum level of SwitchHook that can swap this combo";
			if(FL(cflag3)) //break info
			{
				l_attribyte[1] = "Break Sprite:";
				h_attribyte[1] = "Sprite Data sprite ID to display when broken";
				l_attribyte[2] = "Break SFX:";
				h_attribyte[2] = "SFX to be played when broken";
				l_flag[3] = "Drop Item";
				h_flag[3] = "Will drop an item upon breaking.";
				l_flag[5] = "Next instead of Undercombo";
				h_flag[5] = "Replace with the Next combo instead of the screen's Undercombo";
				if(FL(cflag4))
				{
					l_flag[4] = "Specific Item";
					h_flag[4] = "Drop a specific item instead of an item from a dropset";
					if(FL(cflag5))
					{
						l_attribyte[2] = "Item:";
						h_attribyte[2] = "The item ID to drop";
					}
					else
					{
						l_attribyte[2] = "Dropset:";
						h_attribyte[2] = "The dropset to select a drop item from";
					}
				}
			}
			break;
		}
		case cSAVE: case cSAVE2:
		{
			l_flag[0] = "Restores Life";
			h_flag[0] = "If checked, restores life up to the percentage listed in attribytes.";
			l_flag[1] = "Restores Magic";
			h_flag[1] = "If checked, restores magic up to the percentage listed in attribytes.";
			if(FL(cflag1))
			{
				l_attribyte[0] = "Life Percentage";
				h_attribyte[0] = "Restore life up to this percentage, if it is lower.";
			}
			if(FL(cflag2))
			{
				l_attribyte[1] = "Magic Percentage";
				h_attribyte[1] = "Restore magic up to this percentage, if it is lower.";
			}
			break;
		}
		case cPUSHBLOCK:
		{
			l_flag[0] = "Pushable Up";
			h_flag[0] = "Can be pushed in the 'Up' direction";
			l_flag[1] = "Pushable Down";
			h_flag[1] = "Can be pushed in the 'Down' direction";
			l_flag[2] = "Pushable Left";
			h_flag[2] = "Can be pushed in the 'Left' direction";
			l_flag[3] = "Pushable Right";
			h_flag[3] = "Can be pushed in the 'Right' direction";
			l_flag[4] = "Separate Directions";
			h_flag[4] = "Different push counts for different directions";
			l_flag[5] = "Enemies First (Wait)";
			h_flag[5] = "Cannot be pushed until the enemies have been cleared from the screen";
			l_flag[6] = "Icy Block";
			h_flag[6] = "When pushed, keeps sliding until it hits a barrier.";
			l_flag[7] = "Opposites Cancel";
			h_flag[7] = "Pushing the block in a direction opposite to one it has already been pushed in"
				" 'cancels' the previous push, instead of being a new push,"
				" for purposes of the max number of pushes.";
			l_flag[8] = "0 limit is none";
			h_flag[8] = "Any direction with a push limit of '0' will be unable to be pushed"
				" (as opposed to being pushable an infinite number of times).";
			l_flag[9] = "Ignores Icy Floor";
			h_flag[9] = "Does not slide on icy floors";
			l_flag[10] = "Obeys Gravity";
			h_flag[10] = "Falls down in sideview until it hits the floor (when pushed, NOT passively)";
			
			l_attrishort[0] = "Push Speed";
			h_attrishort[0] = "If 0 or less, uses the default push speed of 0.5 pixels per frame."
				" Otherwise, uses this speed as a step value (1/100ths pixel per frame).";
			l_attribyte[0] = "Heavy Level";
			h_attribyte[0] = "If >0, a bracelet of at least this level is required to push this block.";
			l_attribyte[1] = "Push SFX";
			h_attribyte[1] = "The SFX to play when the block is pushed.";
			l_attribyte[2] = "Stop SFX";
			h_attribyte[2] = "The SFX to play when the block stops moving.";
			if(FL(cflag5))
			{
				l_attribyte[4] = "Times Pushable (Up):";
				h_attribyte[4] = "How many times the block can be pushed Up before it can no"
					" longer be pushed up. '0' either means no pushing or infinite pushing, depending on the '0 limit is none' flag.";
				l_attribyte[5] = "Times Pushable (Down):";
				h_attribyte[5] = "How many times the block can be pushed Down before it can no"
					" longer be pushed down. '0' either means no pushing or infinite pushing, depending on the '0 limit is none' flag.";
				l_attribyte[6] = "Times Pushable (Left):";
				h_attribyte[6] = "How many times the block can be pushed Left before it can no"
					" longer be pushed left. '0' either means no pushing or infinite pushing, depending on the '0 limit is none' flag.";
				l_attribyte[7] = "Times Pushable (Right):";
				h_attribyte[7] = "How many times the block can be pushed Right before it can no"
					" longer be pushed right. '0' either means no pushing or infinite pushing, depending on the '0 limit is none' flag.";
			}
			else
			{
				l_attribyte[4] = "Times Pushable:";
				h_attribyte[4] = "How many times the block can be pushed before it clicks into place."
					" '0' either means no pushing or infinite pushing, depending on the '0 limit is none' flag.";
			}
			break;
		}
		case cICY:
		{
			l_flag[0] = "Slides Blocks";
			h_flag[0] = "Pushable blocks pushed onto this combo will"
				" slide past it, if nothing blocks their way.";
			l_flag[1] = "Slides Hero";
			h_flag[1] = "The Hero will slip and slide on the ice.";
			if(FL(cflag2))
			{
				l_attribyte[0] = "Start Speed Percentage";
				h_attribyte[0] = "The percentage of the Hero's movement speed to carry onto the ice when first stepping onto it.";
				l_attribyte[1] = "Entry Leeway Frames";
				h_attribyte[1] = "For this many frames after entering the ice, the Hero will have additional traction. The traction gradually decreases over time until the frames are up.";
				l_attribute[0] = "Acceleration";
				h_attribute[0] = "Speed gained when holding a direction, in pixels per frame.";
				l_attribute[1] = "Deceleration";
				h_attribute[1] = "Speed lost when not holding a direction, in pixels per frame.";
				l_attribute[2] = "Max Speed";
				h_attribute[2] = "The highest speed that can be reached, in pixels per frame.";
			}
			break;
		}
		case cMIRRORNEW:
		{
			l_attribyte[up] = "Up Reflect";
			h_attribyte[up] = "Weapons/light beams facing up (coming from below) will move in this direction."
				" Light beams will not work with diagonals."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			l_attribyte[down] = "Down Reflect";
			h_attribyte[down] = "Weapons/light beams facing down (coming from above) will move in this direction."
				" Light beams will not work with diagonals."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			l_attribyte[left] = "Left Reflect";
			h_attribyte[left] = "Weapons/light beams facing left (coming from the right) will move in this direction."
				" Light beams will not work with diagonals."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			l_attribyte[right] = "Right Reflect";
			h_attribyte[right] = "Weapons/light beams facing right (coming from the left) will move in this direction."
				" Light beams will not work with diagonals."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			l_attribyte[l_up] = "Up-Left Reflect";
			h_attribyte[l_up] = "Weapons facing up-left (coming from down-right) will move in this direction."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			l_attribyte[r_up] = "Up-Right Reflect";
			h_attribyte[r_up] = "Weapons facing up-right (coming from down-left) will move in this direction."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			l_attribyte[l_down] = "Down-Left Reflect";
			h_attribyte[l_down] = "Weapons facing down-left (coming from up-right) will move in this direction."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			l_attribyte[r_down] = "Down-Right Reflect";
			h_attribyte[r_down] = "Weapons facing down-right (coming from up-left) will move in this direction."
				"\n0 = up, 1 = down, 2 = left, 3 = right"
				"\n4 = up-left, 5 = up-right, 6 = down-left, 7 = down-right";
			break;
		}
		case cCRUMBLE:
		{
			l_attribyte[0] = "Crumble Type";
			h_attribyte[0] = "0 = Reset (Timer resets when stepped off of, can also change combo)"
				"\n1 = Cumulative (Timer does not reset, just pauses when stepped off of)"
				"\n2 = Inevitable (Timer keeps going even if stepped off of)";
			l_flag[0] = "Continuous";
			h_flag[0] = "If the next combo is a Crumbling combo of the 'Inevitable' type, it will continue crumbling automatically.";
			l_attribyte[1] = "Crumble Sensitivity";
			h_attribyte[1] = "This many pixels of leeway are given. 0 = fully sensitive."
				"\nIf leeway exceeds combo/ffc size, no crumbling will occur.";
			l_attrishort[0] = "Crumble Time";
			h_attrishort[0] = "The time, in frames, before the combo crumbles into the next combo in the combo list.";
			if(local_comboref.attribytes[0] == CMBTY_CRUMBLE_RESET)
			{
				l_attrishort[1] = "Reset Change";
				h_attrishort[1] = "If non-zero, the combo will change by this amount (ex. 1 = Next, -1 = Prev)"
					" when the Hero steps off of the combo before it finishes crumbling. (Might be used to reset a crumble animation)";
			}
			break;
		}
	}
	if(local_comboref.script && combo_use_script_data)
	{
		zasm_meta const& meta = comboscripts[local_comboref.script]->meta;
		for(size_t q = 0; q < 16; ++q)
		{
			if(meta.usrflags[q].size())
				l_flag[q] = meta.usrflags[q];
			if(meta.usrflags_help[q].size())
				h_flag[q] = meta.usrflags_help[q];
			if(q > 7) continue;
			if(meta.attribytes[q].size())
				l_attribyte[q] = meta.attribytes[q];
			if(meta.attribytes_help[q].size())
				h_attribyte[q] = meta.attribytes_help[q];
			if(meta.attrishorts[q].size())
				l_attrishort[q] = meta.attrishorts[q];
			if(meta.attrishorts_help[q].size())
				h_attrishort[q] = meta.attrishorts_help[q];
			if(q > 3) continue;
			if(meta.attributes[q].size())
				l_attribute[q] = meta.attributes[q];
			if(meta.attributes_help[q].size())
				h_attribute[q] = meta.attributes_help[q];
		}
	}
	for(size_t q = 0; q < 16; ++q)
	{
		l_flags[q]->setText(l_flag[q]);
		ib_flags[q]->setDisabled(h_flag[q].empty());
		if(q > 7) continue;
		ib_attribytes[q]->setDisabled(h_attribyte[q].empty());
		l_attribytes[q]->setText(l_attribyte[q]);
		ib_attrishorts[q]->setDisabled(h_attrishort[q].empty());
		l_attrishorts[q]->setText(l_attrishort[q]);
		if(q > 3) continue;
		ib_attributes[q]->setDisabled(h_attribute[q].empty());
		l_attributes[q]->setText(l_attribute[q]);
	}
	wizardButton->setDisabled(!hasComboWizard(local_comboref.type));
	updateWarnings();
	pendDraw();
}
void ComboEditorDialog::loadComboFlag()
{
	updateWarnings();
}
void ComboEditorDialog::updateCSet()
{
	tswatch->setCSet(CSet);
	if(local_comboref.animflags&AF_CYCLENOCSET)
		cycleswatch->setCSet(CSet);
	else cycleswatch->setCSet(local_comboref.nextcset);
	cycleswatch->setDisabled(local_comboref.animflags & AF_CYCLEUNDERCOMBO);
	animFrame->setCSet(CSet);
	l_cset->setText(std::to_string(CSet));
}
void ComboEditorDialog::updateAnimation()
{
	updateCSet();
	animFrame->setCSet2(local_comboref.csets);
	animFrame->setFrames(local_comboref.frames);
	animFrame->setSkipX(local_comboref.skipanim);
	animFrame->setSkipY(local_comboref.skipanimy);
	animFrame->setSpeed(local_comboref.speed);
	animFrame->setTile(local_comboref.tile);
	animFrame->setFlip(local_comboref.flip);
	tswatch->setFlip(local_comboref.flip);
}
void ComboEditorDialog::updateWarnings()
{
	warnings.clear();
	auto ty = local_comboref.type;
	auto flag = local_comboref.flag;
	switch(ty)
	{
		case cPUSHBLOCK:
			if(is_push_flag(flag))
				warnings.emplace_back(fmt::format("Push flags do not function on the '{}' type",ZI.getComboTypeName(ty)));
			break;
	}
	warnbtn->setDisabled(warnings.empty());
}
void ComboEditorDialog::updateTriggerIndex()
{
	trigbtnAddCursor->setDisabled(local_comboref.triggers.size() >= MAX_COMBO_TRIGGERS);
	trigbtnAddEnd->setDisabled(local_comboref.triggers.size() >= MAX_COMBO_TRIGGERS);
	trigbtnCopy->setDisabled(trigger_index < 0);
	trigbtnEdit->setDisabled(trigger_index < 0);
	trigbtnDelete->setDisabled(trigger_index < 0);
	trigbtnPasteNewCursor->setDisabled(trigger_index < 0 || local_comboref.triggers.size() >= MAX_COMBO_TRIGGERS || !copied_trigger);
	trigbtnPasteNewEnd->setDisabled(local_comboref.triggers.size() >= MAX_COMBO_TRIGGERS || !copied_trigger);
	trigbtnPaste->setDisabled(trigger_index < 0 || !copied_trigger);
	trigbtnUp->setDisabled(trigger_index < 1);
	trigbtnDown->setDisabled(trigger_index < 0 || trigger_index >= local_comboref.triggers.size()-1);
}

static const GUI::ListData listdata_lift_gfx
{
	{"This Combo GFX", 0},
	{"Other Combo GFX", 1},
	{"Sprite Data GFX", 2}
};

//{ Macros

#define DISABLE_WEAP_DATA true
#define ATTR_WID 6_em
#define ATTR_LAB_WID 12_em
#define SPR_LAB_WID 10_em
#define ACTION_LAB_WID 6_em
#define ACTION_FIELD_WID 6_em
#define FLAGS_WID 18_em

static std::shared_ptr<GUI::Widget> NUM_FIELD_IMPL(word* data, word min, word max)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return TextField(
		type = GUI::TextField::type::INT_DECIMAL,
		maxLength = 5,
		val = *data,
		low = min,
		high = max,
		fitParent = true,
		onValChangedFunc = [data](GUI::TextField::type,std::string_view,int32_t val)
		{
			*data = val;
		}
	);
}

#define NUM_FIELD(member,_min,_max) NUM_FIELD_IMPL(&local_comboref.member, _min, _max)

std::shared_ptr<GUI::Widget> ComboEditorDialog::ANIM_FIELD_IMPL(byte* data, byte min, byte max)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return TextField(
		type = GUI::TextField::type::INT_DECIMAL,
		maxLength = 5,
		val = *data,
		low = min,
		high = max,
		fitParent = true,
		onValChangedFunc = [&, data](GUI::TextField::type,std::string_view,int32_t val)
		{
			*data = val;
			updateAnimation();
		}
	);
}

#define ANIM_FIELD(member, _min, _max) ANIM_FIELD_IMPL(&local_comboref.member, _min, _max)

std::shared_ptr<GUI::Widget> ComboEditorDialog::CMB_FLAG(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px, colSpan=2,
		ib_flags[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("Flag Info",h_flag[index]).show();
			}),
		l_flags[index] = Checkbox(
			minwidth = FLAGS_WID, hAlign = 0.0,
			checked = local_comboref.usrflags & (1<<index), fitParent = true,
			onToggleFunc = [&, index](bool state)
			{
				SETFLAG(local_comboref.usrflags,(1<<index),state);
				loadComboType();
			}
		)
	);
}

#define CMB_GEN_FLAG(ind,str) \
Checkbox(text = str, \
		minwidth = FLAGS_WID, hAlign = 0.0, \
		checked = local_comboref.genflags & (1<<ind), fitParent = true, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(local_comboref.genflags,(1<<ind),state); \
		} \
	)

std::shared_ptr<GUI::Widget> ComboEditorDialog::CMB_ATTRIBYTE(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px, colSpan = 3,
		l_attribytes[index] = Label(minwidth = ATTR_LAB_WID, textAlign = 2),
		ib_attribytes[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("Attribyte Info",h_attribyte[index]).show();
			}),
		TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_BYTE,
			low = 0, high = 255, val = local_comboref.attribytes[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_comboref.attribytes[index] = val;
			})
	);
}

std::shared_ptr<GUI::Widget> ComboEditorDialog::CMB_ATTRISHORT(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px, colSpan = 3,
		l_attrishorts[index] = Label(minwidth = ATTR_LAB_WID, textAlign = 2),
		ib_attrishorts[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("Attrishort Info",h_attrishort[index]).show();
			}),
		TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_SSHORT,
			low = -32768, high = 32767, val = local_comboref.attrishorts[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_comboref.attrishorts[index] = val;
			})
	);
}

std::shared_ptr<GUI::Widget> ComboEditorDialog::CMB_ATTRIBUTE(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px, colSpan = 3,
		l_attributes[index] = Label(minwidth = ATTR_LAB_WID, textAlign = 2),
		ib_attributes[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("Attribute Info",h_attribute[index]).show();
			}),
		TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			val = local_comboref.attributes[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_comboref.attributes[index] = val;
			})
	);
}

std::shared_ptr<GUI::Widget> ComboEditorDialog::CMB_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		l_initds[index] = Label(minwidth = ATTR_LAB_WID, textAlign = 2),
		ib_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info",h_initd[index]).show();
			}),
		tf_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			val = local_comboref.initd[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_comboref.initd[index] = val;
			})
	);
}

#define MISCFLAG(member, bit, str) \
Checkbox( \
	text = str, hAlign = 0.0, \
	checked = (local_comboref.member & bit), \
	fitParent = true, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_comboref.member, bit, state); \
	} \
)

#define IBTN(info) \
Button( \
	width = 1.5_em, padding = 0_px, forceFitH = true, \
	text = "?", hAlign = 1.0, onPressFunc = [&]() \
	{ \
		InfoDialog("Info", info).show(); \
	} \
)

#define IBTN_T(title, info) \
Button( \
	width = 1.5_em, padding = 0_px, forceFitH = true, \
	text = "?", hAlign = 1.0, onPressFunc = [&]() \
	{ \
		InfoDialog(title, info).show(); \
	} \
)
//}

int32_t solidity_to_flag(int32_t val)
{
	return (val&0b1001) | (val&0b0100)>>1 | (val&0b0010)<<1;
}


void ComboEditorDialog::add_combo_trigger(size_t pos, bool copied)
{
	if(copied && !copied_trigger) return;
	if(local_comboref.triggers.size() >= MAX_COMBO_TRIGGERS) return;
	combo_trigger& trig = *local_comboref.triggers.emplace(std::next(local_comboref.triggers.begin(), pos));
	
	if(copied)
		trig = *copied_trigger;
	else if(!call_trigger_editor(*this, pos))
	{
		local_comboref.triggers.erase(std::next(local_comboref.triggers.begin(), pos));
		return;
	}
	trigger_index = pos;
	refresh_dlg();
}

std::shared_ptr<GUI::Widget> ComboEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	if (trigger_index < 0 && !local_comboref.triggers.empty())
		trigger_index = 0;
	else if (trigger_index >= local_comboref.triggers.size())
		trigger_index = int(local_comboref.triggers.size()) - 1;
	
	list_triggers = GUI::ListData::numbers(false, 0, local_comboref.triggers.size()).filter(
		[&](GUI::ListItem& itm)
		{
			itm.text = fmt::format("{} ({:03})", local_comboref.triggers[itm.value].label, itm.value);
			return true;
		}
	);
	
	window = Window(
		use_vsync = true,
		title = fmt::format("Combo Editor ({})", index),
		info = "Edit combos, setting up their graphics, effects, and attributes.\n"
			"Hotkeys:\n"
			"-/+: Change CSet\n"
			"Shift -/+: Change Combo\n"
			"H/V/R: Flip (Horz,Vert,Rotate)\n"
			"T: Change tile",
		onClose = message::CANCEL,
		shortcuts={
			V=message::VFLIP,
			H=message::HFLIP,
			R=message::ROTATE,
			PlusPad=message::PLUSCS,
			Equals=message::PLUSCS,
			MinusPad=message::MINUSCS,
			Minus=message::MINUSCS,
			T=message::TILESEL,
			Shift+Minus=message::MINUSCOMBO,
			Shift+MinusPad=message::MINUSCOMBO,
			Shift+Equals=message::PLUSCOMBO,
			Shift+PlusPad=message::PLUSCOMBO,
		},
		Column(
			Rows<4>(padding = 0_px,
				Label(text = "Type:", hAlign = 1.0),
				DropDownList(data = list_ctype, fitParent = true,
					maxwidth = 400_px,
					padding = 0_px, selectedValue = local_comboref.type,
					onSelectionChanged = message::COMBOTYPE
				),
				Button(
					width = 1.5_em, padding = 0_px, forceFitH = true,
					text = "?", hAlign = 1.0, onPressFunc = [&]()
					{
						ctype_help(local_comboref.type);
					}
				),
				wizardButton = Button(
					text = "Wizard", disabled = !hasComboWizard(local_comboref.type),
					rowSpan = 2, minwidth = 150_px,
					padding = 0_px, forceFitH = true, onClick = message::WIZARD
				),
				Label(text = "Inherent Flag:", hAlign = 1.0),
				DropDownList(data = list_flag, fitParent = true,
					maxwidth = 400_px,
					padding = 0_px, selectedValue = local_comboref.flag,
					onSelectionChanged = message::COMBOFLAG
				),
				Button(
					width = 1.5_em, padding = 0_px, forceFitH = true,
					text = "?", hAlign = 1.0, onPressFunc = [&]()
					{
						cflag_help(local_comboref.flag);
					}
				)
			),
			TabPanel(
				ptr = &cmb_tabs[0],
				TabRef(name = "Basic", Row(
					Rows<2>(
						Label(text = "Label:", hAlign = 1.0),
						TextField(
							fitParent = true,
							type = GUI::TextField::type::TEXT,
							maxLength = 255,
							text = local_comboref.label,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view text,int32_t)
							{
								local_comboref.label = text;
							}),
						Label(text = "CSet 2:", hAlign = 1.0),
						TextField(
							fitParent = true,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -8, high = 7, val = (local_comboref.csets&8) ? ((local_comboref.csets&0xF)|~int32_t(0xF)) : (local_comboref.csets&0xF),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_comboref.csets &= ~0xF;
								local_comboref.csets |= val&0xF;
								updateAnimation();
							}),
						Label(text = "A. Frames:", hAlign = 1.0),
						ANIM_FIELD(frames, 0, 255),
						Label(text = "A. Speed:", hAlign = 1.0),
						ANIM_FIELD(speed, 0, 255),
						Label(text = "A. SkipX:", hAlign = 1.0),
						ANIM_FIELD(skipanim, 0, 255),
						Label(text = "A. SkipY:", hAlign = 1.0),
						ANIM_FIELD(skipanimy, 0, 255),
						Label(text = "Flip:", hAlign = 1.0),
						l_flip = Label(text = std::to_string(local_comboref.flip), hAlign = 0.0),
						Label(text = "CSet:", hAlign = 1.0),
						l_cset = Label(text = std::to_string(CSet), hAlign = 0.0)
					),
					Column(padding = 0_px,
						Rows<6>(
							Label(text = "Tile", hAlign = 0.5, colSpan = 2),
							Label(text = "Solid", hAlign = 1.0, rightPadding = 0_px),
							Button(leftPadding = 0_px, forceFitH = true, text = "?",
								onPressFunc = []()
								{
									InfoDialog("Solidity","The pink-highlighted corners of the combo will be treated"
										" as solid walls.").show();
								}),
							Label(text = "CSet2", hAlign = 1.0, rightPadding = 0_px),
							Button(leftPadding = 0_px, forceFitH = true, text = "?",
								onPressFunc = []()
								{
									InfoDialog("CSet2","The cyan-highlighted corners of the combo will be drawn"
										" in a different cset, offset by the value in the 'CSet2' field.").show();
								}),
							tswatch = SelTileSwatch(
								colSpan = 2,
								tile = local_comboref.tile,
								cset = CSet,
								flip = local_comboref.flip,
								showFlip = true,
								showvals = false,
								onSelectFunc = [&](int32_t t, int32_t c, int32_t f,int32_t)
								{
									local_comboref.tile = t;
									local_comboref.o_tile = t;
									updateFlip(f);
									CSet = (c&0xF)%14;
									l_flip->setText(std::to_string(f));
									updateAnimation();
								}
							),
							cswatchs[0] = CornerSwatch(colSpan = 2,
								val = solidity_to_flag(local_comboref.walk&0xF),
								color = vc(12),
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.walk &= ~0xF;
									local_comboref.walk |= solidity_to_flag(val);
								}
							),
							cswatchs[1] = CornerSwatch(colSpan = 2,
								val = (local_comboref.csets&0xF0)>>4,
								color = vc(11),
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.csets &= ~0xF0;
									local_comboref.csets |= val<<4;
									updateAnimation();
								}
							),
							animFrame = TileFrame(
								colSpan = 2,
								tile = local_comboref.tile,
								cset = CSet,
								cset2 = local_comboref.csets,
								frames = local_comboref.frames,
								speed = local_comboref.speed,
								skipx = local_comboref.skipanim,
								skipy = local_comboref.skipanimy,
								flip = local_comboref.flip
							),
							cycleswatch = SelComboSwatch(colSpan = 2,
								showvals = false,
								combo = local_comboref.nextcombo,
								cset = local_comboref.nextcset,
								onSelectFunc = [&](int32_t cmb, int32_t c)
								{
									local_comboref.nextcombo = cmb;
									local_comboref.nextcset = c;
									updateCSet();
								}
							),
							cswatchs[2] = CornerSwatch(colSpan = 2,
								val = solidity_to_flag((local_comboref.walk&0xF0)>>4),
								color = vc(10),
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.walk &= ~0xF0;
									local_comboref.walk |= solidity_to_flag(val)<<4;
								}
							),
							Label(text = "Preview", hAlign = 0.5,colSpan = 2),
							Label(text = "Cycle", hAlign = 1.0, rightPadding = 0_px),
							Button(leftPadding = 0_px, forceFitH = true, text = "?",
								onPressFunc = []()
								{
									InfoDialog("Cycle","When the combo's animation has completed once,"
										" the combo will be changed to the 'Cycle' combo, unless the 'Cycle'"
										" combo is set to Combo 0.").show();
								}),
							Label(text = "Effect", hAlign = 1.0, rightPadding = 0_px),
							Button(leftPadding = 0_px, forceFitH = true, text = "?",
								onPressFunc = []()
								{
									InfoDialog("Effect","The combo type takes effect only in the lime-highlighted"
										" corners of the combo.").show();
								})
						),
						Checkbox(
							text = "Refresh Animation on Room Entry", hAlign = 0.0,
							checked = local_comboref.animflags & AF_FRESH,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_comboref.animflags,AF_FRESH,state);
							}
						),
						Checkbox(
							text = "Restart Animation when Cycled To", hAlign = 0.0,
							checked = local_comboref.animflags & AF_CYCLE,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_comboref.animflags,AF_CYCLE,state);
							}
						),
						Checkbox(
							text = "Cycle To Screen Undercombo", hAlign = 0.0,
							checked = local_comboref.animflags & AF_CYCLEUNDERCOMBO,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_comboref.animflags,AF_CYCLEUNDERCOMBO,state);
								updateCSet();
							}
						),
						Checkbox(
							text = "Cycle Ignores CSet", hAlign = 0.0,
							checked = local_comboref.animflags & AF_CYCLENOCSET,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_comboref.animflags,AF_CYCLENOCSET,state);
								updateCSet();
							}
						),
						Checkbox(
							text = "Toggle Transparent", hAlign = 0.0,
							checked = local_comboref.animflags & AF_TRANSPARENT,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_comboref.animflags,AF_TRANSPARENT,state);
							}
						)
					)
				)),
				TabRef(name = "Flags", Column(
					padding = 0_px,
					Rows<2>(
						framed = true,
						frameText = "General Flags",
						topPadding = DEFAULT_PADDING+0.4_em,
						bottomPadding = DEFAULT_PADDING+1_px,
						bottomMargin = 1_em,
						CMB_GEN_FLAG(0,"Hook-Grabbable"),
						CMB_GEN_FLAG(1,"Switch-Hookable")
					),
					Columns<8>(
						framed = true,
						frameText = "Variable Flags",
						topPadding = DEFAULT_PADDING+0.4_em,
						bottomPadding = DEFAULT_PADDING+1_px,
						bottomMargin = 1_em,
						CMB_FLAG(0),
						CMB_FLAG(1),
						CMB_FLAG(2),
						CMB_FLAG(3),
						CMB_FLAG(4),
						CMB_FLAG(5),
						CMB_FLAG(6),
						CMB_FLAG(7),
						CMB_FLAG(8),
						CMB_FLAG(9),
						CMB_FLAG(10),
						CMB_FLAG(11),
						CMB_FLAG(12),
						CMB_FLAG(13),
						CMB_FLAG(14),
						CMB_FLAG(15)
					)
				)),
				TabRef(name = "Attribs 1", Row(
					Rows<3>(framed = true, frameText = "Attribytes",
						CMB_ATTRIBYTE(0),
						CMB_ATTRIBYTE(1),
						CMB_ATTRIBYTE(2),
						CMB_ATTRIBYTE(3),
						CMB_ATTRIBYTE(4),
						CMB_ATTRIBYTE(5),
						CMB_ATTRIBYTE(6),
						CMB_ATTRIBYTE(7)
					),
					Rows<3>(framed = true, frameText = "Attrishorts",
						CMB_ATTRISHORT(0),
						CMB_ATTRISHORT(1),
						CMB_ATTRISHORT(2),
						CMB_ATTRISHORT(3),
						CMB_ATTRISHORT(4),
						CMB_ATTRISHORT(5),
						CMB_ATTRISHORT(6),
						CMB_ATTRISHORT(7)
					)
				)),
				TabRef(name = "Attribs 2", Row(
					Rows<3>(framed = true, frameText = "Attributes",
						CMB_ATTRIBUTE(0),
						CMB_ATTRIBUTE(1),
						CMB_ATTRIBUTE(2),
						CMB_ATTRIBUTE(3)
					)
				)),
				TabRef(name = "Triggers", Column(
					Frame(
						Row(
							Column(
								triggerList = List(
									data = list_triggers,
									disabled = local_comboref.triggers.empty(),
									selectedValue = trigger_index,
									onSelectFunc = [&](int32_t val)
									{
										trigger_index = val;
										updateTriggerIndex();
									}
								),
								Row(
									IBTN("The combo will ignore methods of triggering its standard effects that"
										" are not from the 'Triggers' tab; Ex. a bush will no longer react to swords,"
										" unless the 'Sword' weapon trigger is checked."),
									Checkbox(text = "Only Gen Triggers",
										checked = local_comboref.only_gentrig&1, fitParent = true,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_comboref.only_gentrig,1,state);
										}
									)
								)
							),
							Columns<5>(
								trigbtnAddCursor = Button(text = "Add (at cursor)",
									fitParent = true,
									onPressFunc = [&]()
									{
										add_combo_trigger(trigger_index+1);
									}),
								trigbtnAddEnd = Button(text = "Add (at end)",
									fitParent = true,
									onPressFunc = [&]()
									{
										add_combo_trigger(local_comboref.triggers.size());
									}),
								trigbtnCopy = Button(text = "Copy",
									fitParent = true,
									onPressFunc = [&]()
									{
										if(trigger_index < 0) return;
										copied_trigger = local_comboref.triggers[trigger_index];
										refresh_dlg();
									}),
								trigbtnEdit = Button(text = "Edit",
									fitParent = true,
									onPressFunc = [&]()
									{
										if(trigger_index < 0) return;
										if(call_trigger_editor(*this, size_t(trigger_index)))
											refresh_dlg();
									}),
								trigbtnDelete = Button(text = "Delete",
									fitParent = true,
									onPressFunc = [&]()
									{
										if(trigger_index < 0) return;
										bool doclear = false;
										AlertDialog("Are you sure?",
											"Deleting a trigger cannot be undone!",
											[&](bool ret,bool)
											{
												doclear = ret;
											}).show();
										if(doclear)
										{
											local_comboref.triggers.erase(std::next(local_comboref.triggers.begin(), trigger_index));
											refresh_dlg();
										}
									}),
								//
								trigbtnPasteNewCursor = Button(text = "Paste New (at cursor)",
									fitParent = true,
									onPressFunc = [&]()
									{
										add_combo_trigger(trigger_index+1, true);
									}),
								trigbtnPasteNewEnd = Button(text = "Paste New (at end)",
									fitParent = true,
									onPressFunc = [&]()
									{
										add_combo_trigger(local_comboref.triggers.size(), true);
									}),
								trigbtnPaste = Button(text = "Paste",
									fitParent = true,
									onPressFunc = [&]()
									{
										if(trigger_index < 0 || !copied_trigger) return;
										local_comboref.triggers[trigger_index] = *copied_trigger;
										refresh_dlg();
									}),
								trigbtnUp = Button(text = "Up",
									fitParent = true,
									onPressFunc = [&]()
									{
										if(trigger_index < 1) return;
										zc_swap(local_comboref.triggers[trigger_index], local_comboref.triggers[trigger_index-1]);
										--trigger_index;
										refresh_dlg();
									}),
								trigbtnDown = Button(text = "Down",
									fitParent = true,
									onPressFunc = [&]()
									{
										if(trigger_index < 0 || trigger_index >= local_comboref.triggers.size()-1) return;
										zc_swap(local_comboref.triggers[trigger_index], local_comboref.triggers[trigger_index+1]);
										++trigger_index;
										refresh_dlg();
									})
							)
						)
					)
				)),
				TabRef(name = "Lifting", Row(
					Frame(
						padding = 0_px,
						vAlign = 0.5,
						fitParent = true,
						Column(
							Label(text = "Graphics"),
							DropDownList(data = listdata_lift_gfx,
									fitParent = true,
									selectedValue = local_comboref.liftgfx,
									onSelectFunc = [&](int32_t val)
									{
										local_comboref.liftgfx = val;
									}
								),
							Label(text = "Other Combo GFX"),
							SelComboSwatch(
									showvals = true,
									combo = local_comboref.liftcmb,
									cset = local_comboref.liftcs,
									onSelectFunc = [&](int32_t cmb, int32_t c)
									{
										local_comboref.liftcmb = cmb;
										local_comboref.liftcs = c;
									}
								),
							Label(text = "Sprite Data GFX"),
							DropDownList(data = list_sprites,
									fitParent = true,
									selectedValue = local_comboref.liftsprite,
									onSelectFunc = [&](int32_t val)
									{
										local_comboref.liftsprite = val;
									}
								),
							Label(text = "Break Sprite"),
							DropDownList(data = list_sprites_spec,
									fitParent = true,
									selectedValue = local_comboref.liftbreaksprite,
									onSelectFunc = [&](int32_t val)
									{
										local_comboref.liftbreaksprite = val;
									}
								),
							Rows<3>(padding = 0_px,
								Label(text = "Lift SFX:"),
								DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_comboref.liftsfx,
									width = 300_px,
									onSelectFunc = [&](int32_t val)
									{
										local_comboref.liftsfx = val;
									}),
								IBTN("The sfx to play when lifted"),
								Label(text = "Break SFX:"),
								DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_comboref.liftbreaksfx,
									width = 300_px,
									onSelectFunc = [&](int32_t val)
									{
										local_comboref.liftbreaksfx = val;
									}),
								IBTN("The sfx to play when the object breaks"),
								Label(text = "Lift Weapon:"),
								DropDownList(data = list_lift_parent_items,
									fitParent = true, selectedValue = local_comboref.lift_parent_item,
									width = 300_px,
									onSelectFunc = [&](int32_t val)
									{
										local_comboref.lift_parent_item = val;
									}),
								IBTN("What item to use to create the lift weapon. If '(None)', a basic 'Thrown' weapon will be created from the lift glove item."),
								Label(text = "Glow Shape:"),
								DropDownList(data = list_light_shapes,
									fitParent = true, selectedValue = local_comboref.liftlightshape,
									width = 300_px,
									onSelectFunc = [&](int32_t val)
									{
										local_comboref.liftlightshape = val;
									}),
								IBTN("The shape of light the lifted weapon emits, if emitting light.")
							)
						)
					),
					Frame(
						padding = 0_px,
						vAlign = 0.5,
						fitParent = true,
						Rows<3>(
							DummyWidget(),
							Label(text = "Lift Undercombo:", colSpan = 2, hAlign = 0.0),
							//
							IBTN("The combo that will replace this combo when lifted"),
							SelComboSwatch(
								colSpan = 2, hAlign = 0.0,
								showvals = true,
								combo = local_comboref.liftundercmb,
								cset = local_comboref.liftundercs,
								onSelectFunc = [&](int32_t cmb, int32_t c)
								{
									local_comboref.liftundercmb = cmb;
									local_comboref.liftundercs = c;
								}),
							//
							DummyWidget(),
							Checkbox(colSpan = 2,
								text = "Is Liftable", hAlign = 0.0,
								checked = local_comboref.liftflags & LF_LIFTABLE,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.liftflags,LF_LIFTABLE,state);
								}
							),
							//
							DummyWidget(),
							Checkbox(colSpan = 2,
								text = "Lift Undercombo ignores CSet", hAlign = 0.0,
								checked = local_comboref.liftflags & LF_NOUCSET,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.liftflags,LF_NOUCSET,state);
								}
							),
							//
							DummyWidget(),
							Checkbox(colSpan = 2,
								text = "Other Combo GFX ignores CSet", hAlign = 0.0,
								checked = local_comboref.liftflags & LF_NOWPNCMBCSET,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.liftflags,LF_NOWPNCMBCSET,state);
								}
							),
							//
							IBTN("The thrown object will break when hitting a solid combo"),
							Checkbox(colSpan = 2,
								text = "Weapon breaks on solids", hAlign = 0.0,
								checked = local_comboref.liftflags & LF_BREAKONSOLID,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.liftflags,LF_BREAKONSOLID,state);
								}
							),
							//
							DummyWidget(),
							Checkbox(colSpan = 2,
								text = "Use Dropset instead of Item ID", hAlign = 0.0,
								checked = local_comboref.liftflags & LF_DROPSET,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.liftflags,LF_DROPSET,state);
								}
							),
							//
							IBTN("The item will be dropped under the combo when"
								" it is lifted, instead of from the thrown object when it breaks."),
							Checkbox(colSpan = 2,
								text = "Drop on lift instead of break", hAlign = 0.0,
								checked = local_comboref.liftflags & LF_DROPONLIFT,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.liftflags,LF_DROPONLIFT,state);
								}
							),
							//
							IBTN("The item will be the room's 'Special Item' (i.e. sets the special item screen state, does not return once collected once), and will not 'time out'."),
							Checkbox(colSpan = 2,
								text = "Drops Special Item", hAlign = 0.0,
								checked = local_comboref.liftflags & LF_SPECIALITEM,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.liftflags,LF_SPECIALITEM,state);
								}
							)
						)
					),
					Frame(
						padding = 0_px,
						vAlign = 0.5,
						fitParent = true,
						Rows<3>(
							Label(text = "Damage:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_comboref.liftdmg,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_comboref.liftdmg = val;
								}),
							IBTN("Weapon Power for the 'thrown object' weapon"),
							//
							Label(text = "Lift Level:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_comboref.liftlvl,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_comboref.liftlvl = val;
								}),
							IBTN("The level of " + string(ZI.getItemClassName(itype_liftglove)) + " needed to lift this object."),
							//
							Label(text = "Item Drop:"),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_comboref.liftitm,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_comboref.liftitm = val;
								}),
							IBTN("If 0, drops no item."
								"\nIf >0, drops that item ID."),
							//
							Label(text = "Lift Height"),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_comboref.lifthei,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_comboref.lifthei = val;
								}),
							IBTN("The Z-height the combo will be lifted to"),
							//
							Label(text = "Lift Time"),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_comboref.lifttime,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_comboref.lifttime = val;
								}),
							IBTN("The time, in frames, it takes to lift the combo")
							,
							//
							Label(text = "Glow Radius"),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_comboref.liftlightrad,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_comboref.liftlightrad = val;
								}),
							IBTN("The light radius, in pixels, of the lifted weapon")
							
						)
					)
				)),
				TabRef(name = "General", TabPanel(
					ptr = &cmb_tabs[1],
					TabRef(name = "SFX",
						Rows<3>(
							Label(text = "Appears:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_appear,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_appear = val;
								}),
							IBTN("Plays when the combo is on the screen for at least a frame."),
							//
							Label(text = "Disappears:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_disappear,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_disappear = val;
								}),
							IBTN("Plays when the combo had appeared, but is now gone"),
							//
							Label(text = "Loop:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_loop,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_loop = val;
								}),
							IBTN("Plays repeatedly as long as the combo is on the screen."),
							//
							Label(text = "Walking:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_walking,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_walking = val;
								}),
							IBTN("Plays when the Hero walks on the combo. In sideview, this is the combo actually BELOW the Hero."),
							//
							Label(text = "Standing:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_standing,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_standing = val;
								}),
							IBTN("Plays when the Hero stands (not walking) on the combo. In sideview, this is the combo actually BELOW the Hero."),
							//
							Label(text = "Sword Tap:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_tap,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_tap = val;
								}),
							IBTN("Plays when the Hero taps their sword against this combo. Only the highest-layer combo with custom tap SFX will take effect."),
							//
							Label(text = "Landing:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_landing,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_landing = val;
								}),
							IBTN("Plays when the Hero lands on the combo. Doesn't play if 'Old Landing SFX' is enabled." + QRHINT({qr_OLD_LANDING_SFX})),
							//
							Label(text = "Falling:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_falling,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_falling = val;
								}),
							IBTN("Used when the combo falls into a pit (after being pushed as a pushable block)."
								"\nIf set to (None), uses the sfx set on the Pitfall combo."),
							//
							Label(text = "Drowning:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_drowning,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_drowning = val;
								}),
							IBTN("Used when the combo falls into liquid (after being pushed as a pushable block)."
								"\nIf set to (None), uses the sfx set on the Liquid combo."),
							//
							Label(text = "Lava Drowning:"),
							DropDownList(data = list_sfx,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.sfx_lava_drowning,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.sfx_lava_drowning = val;
								}),
							IBTN("Used when the combo falls into lava (after being pushed as a pushable block)."
								"\nIf set to (None), uses the sfx set on the Liquid combo.")
						)
					),
					TabRef(name = "Sprites",
						Rows<3>(
							Label(text = "Appears:"),
							DropDownList(data = list_sprites_0none,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.spr_appear,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.spr_appear = val;
								}),
							IBTN("Spawns when the combo is on the screen for at least a frame."),
							//
							Label(text = "Disappears:"),
							DropDownList(data = list_sprites_0none,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.spr_disappear,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.spr_disappear = val;
								}),
							IBTN("Spawns when the combo had appeared, but is now gone"),
							//
							Label(text = "Walking:"),
							DropDownList(data = list_sprites_0none,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.spr_walking,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.spr_walking = val;
								}),
							IBTN("Spawns when the Hero walks on the combo. In sideview, this is the combo actually BELOW the Hero."),
							//
							Label(text = "Standing:"),
							DropDownList(data = list_sprites_0none,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.spr_standing,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.spr_standing = val;
								}),
							IBTN("Spawns when the Hero stands (not walking) on the combo. In sideview, this is the combo actually BELOW the Hero."),
							//
							Label(text = "Falling:"),
							DropDownList(data = list_sprites_0none,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.spr_falling,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.spr_falling = val;
								}),
							IBTN("Used when the combo is falling into a pit (after being pushed as a pushable block)."
								"\nIf set to (None), uses the Falling Sprite set in Misc Sprites as a default."),
							//
							Label(text = "Drowning:"),
							DropDownList(data = list_sprites_0none,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.spr_drowning,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.spr_drowning = val;
								}),
							IBTN("Used when the combo is falling into liquid (after being pushed as a pushable block)."
								"\nIf set to (None), uses the Drowning Sprite set in Misc Sprites as a default."
								+ QRHINT({qr_BLOCKS_DROWN})),
							//
							Label(text = "Lava Drowning:"),
							DropDownList(data = list_sprites_0none,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_comboref.spr_lava_drowning,
								onSelectFunc = [&](int32_t val)
								{
									local_comboref.spr_lava_drowning = val;
								}),
							IBTN("Used when the combo is falling into lava (after being pushed as a pushable block)."
								"\nIf set to (None), uses the Lava Drowning Sprite set in Misc Sprites as a default."
								+ QRHINT({qr_BLOCKS_DROWN}))
						)
					),
					TabRef(name = "Misc",
						Row(
							Frame(title = "Hero Speed Mod",
								info = "Speed Modification only applies if the Quest Rule 'Newer Hero Movement' is enabled." + QRHINT({qr_NEW_HERO_MOVEMENT2}),
								Rows<3>(
									Label(text = "Multiplier:"),
									TextField(type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 1.0, low = 0, high = 255, val = local_comboref.speed_mult,
										fitParent = true,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_comboref.speed_mult = val;
										}),
									IBTN("Multiplies the Hero's speed by this value when walking over this combo."),
									//
									Label(text = "Divisor:"),
									TextField(type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 1.0, low = 0, high = 255, val = local_comboref.speed_div,
										fitParent = true,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_comboref.speed_div = val;
										}),
									IBTN("Divides the Hero's speed by this value when walking over this combo. Applies after mult."
										"\nIf 0, no division is performed."),
									//
									Label(text = "Additive:"),
									TextField(maxLength = 13, type = GUI::TextField::type::NOSWAP_ZSINT,
										hAlign = 1.0, val = local_comboref.speed_add.getZLong(),
										swap_type = nswapDEC,
										fitParent = true,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_comboref.speed_add = zslongToFix(val);
										}),
									IBTN("Adds this value, in px/frame, to the Hero's speed walking over this combo. Applies after mult and div. Can be negative.")
								)
							)
						)
					)
				)),
				TabRef(name = "Script", Row(
					Column(
						CMB_INITD(0),
						CMB_INITD(1),
						CMB_INITD(2),
						CMB_INITD(3),
						CMB_INITD(4),
						CMB_INITD(5),
						CMB_INITD(6),
						CMB_INITD(7)
					),
					Column(vAlign = 0.0,
						Row(
							padding = 0_px,
							SCRIPT_LIST_PROC("Combo Script:", list_combscript, local_comboref.script, refreshScript)
						),
						Checkbox(text = "Show Script Attrib Metadata",
							checked = combo_use_script_data,
							onToggleFunc = [&](bool state)
							{
								combo_use_script_data = state;
								zc_set_config("zquest","show_comboscript_meta_attribs",state?1:0);
								loadComboType();
							})
					)
				))
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				warnbtn = Button(
					text = "Warnings",
					minwidth = 90_px,
					onClick = message::WARNINGS),
				Button(
					text = "Clear",
					minwidth = 90_px,
					onClick = message::CLEAR),
				Button(
					text = "Default",
					minwidth = 90_px,
					onClick = message::DEFAULT),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	refreshScript();
	updateAnimation();
	updateWarnings();
	updateTriggerIndex();
#if DEVLEVEL > 0
	if(force_wizard)
	{
		if(force_wizard==1)
		{
			local_comboref.type = cSHALLOWWATER;
			call_combo_wizard(*this);
		}
		else for(auto q = 0; q < cMAX; ++q)
		{
			if(hasComboWizard(q))
			{
				local_comboref.type = q;
				call_combo_wizard(*this);
			}
		}
		return nullptr;
	}
#endif
	return window;
}

static int rot_crn_val(int val, int rot = 1)
{
	rot = wrap(rot,0,3);
	int32_t newval = 0;
	switch(rot)
	{
		case 0:
			newval = val;
			break;
		case 1:
			if(val&0b0001)
				newval |= 0b0010;
			if(val&0b0010)
				newval |= 0b1000;
			if(val&0b0100)
				newval |= 0b0001;
			if(val&0b1000)
				newval |= 0b0100;
			break;
		case 2:
			if(val&0b0001)
				newval |= 0b1000;
			if(val&0b0010)
				newval |= 0b0100;
			if(val&0b0100)
				newval |= 0b0010;
			if(val&0b1000)
				newval |= 0b0001;
			break;
		case 3:
			if(val&0b0001)
				newval |= 0b0100;
			if(val&0b0010)
				newval |= 0b0001;
			if(val&0b0100)
				newval |= 0b1000;
			if(val&0b1000)
				newval |= 0b0010;
			break;
	}
	return newval;
}
static bool is_rot(int flip)
{
	switch(flip)
	{
		case 0: case 4: case 7: case 3:
			return true;
	}
	return false;
}
void ComboEditorDialog::flipSwatches(int rot, int hflip, int vflip)
{
	if(rot)
		for(auto crn : cswatchs)
			crn->setVal(rot_crn_val(crn->getVal(),rot));
	if(hflip&1)
		for(auto crn : cswatchs)
		{
			int32_t val = crn->getVal();
			crn->setVal((val & 0b0101)<<1 | (val&0b1010)>>1);
		}
	if(vflip&1)
		for(auto crn : cswatchs)
		{
			int32_t val = crn->getVal();
			crn->setVal((val & 0b0011)<<2 | (val&0b1100)>>2);
		}
	local_comboref.walk = solidity_to_flag(cswatchs[0]->getVal())
		| solidity_to_flag(cswatchs[2]->getVal())<<4;
	local_comboref.csets &= ~0xF0;
	local_comboref.csets |= cswatchs[1]->getVal()<<4;
}
void ComboEditorDialog::updateFlip(int nflip)
{
	int oflip = local_comboref.flip;
	if(oflip == nflip) return;

	bool vflip = false, hflip = false;
	int rots = 0;

	//calculate how to get from oflip to nflip
	if((oflip&0b1100) == (nflip&0b1100)) //possible without rotation
	{
		hflip = (oflip&0b01) != (nflip&0b01);
		vflip = (oflip&0b10) != (nflip&0b10);
	}
	else
	{
		//impossible without flipping?
		if(hflip = (is_rot(oflip) != is_rot(nflip)))
			oflip ^= 0b01;

		while(oflip!=nflip) //Rotate until they match
		{
			oflip = rotate_value(oflip);
			++rots;
		}
	}
	//Flip the corner swatches
	flipSwatches(rots,hflip?1:0,vflip?1:0);

	local_comboref.flip = nflip;
}

void ComboEditorDialog::apply_combo()
{
	if(!hasCTypeEffects(local_comboref.type))
	{
		for(combo_trigger& trig : local_comboref.triggers)
			trig.triggerflags[0] &= ~combotriggerCMBTYPEFX;
	}
	combobuf[index] = local_comboref;
	saved = false;
	edited = true;
}

bool ComboEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::COMBOTYPE:
		{
			local_comboref.type = int32_t(msg.argument);
			loadComboType();
			return false;
		}
		case message::COMBOFLAG:
		{
			local_comboref.flag = int32_t(msg.argument);
			loadComboFlag();
			return false;
		}
		case message::HFLIP:
		{
			if(cmb_tabs[0]) break;
			local_comboref.flip ^= 1;
			flipSwatches(0,1,0);
			l_flip->setText(std::to_string(local_comboref.flip));
			updateAnimation();
			return false;
		}
		case message::VFLIP:
		{
			if(cmb_tabs[0]) break;
			local_comboref.flip ^= 2;
			flipSwatches(0,0,1);
			l_flip->setText(std::to_string(local_comboref.flip));
			updateAnimation();
			return false;
		}
		case message::ROTATE:
		{
			if(cmb_tabs[0]) break;
			local_comboref.flip = rotate_value(local_comboref.flip);
			flipSwatches(1,0,0);
			l_flip->setText(std::to_string(local_comboref.flip));
			updateAnimation();
			return false;
		}
		case message::PLUSCS:
		{
			if(cmb_tabs[0]) break;
			CSet = (CSet+1)%14;
			updateCSet();
			return false;
		}
		case message::MINUSCS:
		{
			if(cmb_tabs[0]) break;
			CSet = (CSet+13)%14;
			updateCSet();
			return false;
		}
		case message::PLUSCOMBO:
		{
			// We could remove this check, but user would not really know what combo they were editing any more.
			// If we showed the current tile in the corner somewhere, we could remove this.
			//if(cmb_tabs[0]) break; //I think it's fine, honestly? Combo number is in the title bar. -Em
			if(index == combobuf.size() - 1) break;

			apply_combo();
			index += 1;
			local_comboref = combobuf[index];
			rerun_dlg = true;
			return true;
		}
		case message::MINUSCOMBO:
		{
			// We could remove this check, but user would not really know what combo they were editing any more.
			// If we showed the current tile in the corner somewhere, we could remove this.
			//if(cmb_tabs[0]) break; //I think it's fine, honestly? Combo number is in the title bar. -Em
			if(index==0) break;

			apply_combo();
			index -= 1;
			local_comboref = combobuf[index];
			rerun_dlg = true;
			return true;
		}
		case message::TILESEL:
		{
			if(cmb_tabs[0]) break;
			tswatch->click();
			break;
		}
		case message::CLEAR:
		{
			bool doclear = false;
			AlertDialog("Are you sure?",
				"Clearing the combo will reset all values",
				[&](bool ret,bool)
				{
					doclear = ret;
				}).show();
			if(doclear)
			{
				local_comboref.clear();
				rerun_dlg = true;
				return true;
			}
			return false;
		}
		case message::DEFAULT:
		{
			if(do_combo_default(local_comboref))
				rerun_dlg = true;
			return rerun_dlg;
		}
		
		case message::WIZARD:
			if(hasComboWizard(local_comboref.type))
			{
				call_combo_wizard(*this);
				rerun_dlg = true;
				return true;
			}
			break;
		
		case message::WARNINGS:
			if(warnings.size())
				displayinfo("Warnings",warnings,
					"The following issues were found with this combo:");
			return false;
		case message::OK:
		{
			if(warnings.size())
			{
				bool cancel = false;
				AlertDialog alert("Warnings",warnings,[&](bool ret,bool)
					{
						if(!ret) cancel = true;
					});
				alert.setSubtext("The following issues were found with this combo:");
				alert.show();
				if(cancel)
					return false;
			}
			apply_combo();
			return true;
		}
		
		case message::CANCEL:
			return true;
	}
	return false;
}

