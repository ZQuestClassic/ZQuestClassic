#include "comboeditor.h"
#include "info.h"
#include "alert.h"
#include "../zsys.h"
#include "../tiles.h"
#include <gui/builder.h>

extern bool saved;
extern zcmodule moduledata;
extern newcombo *combobuf;
extern comboclass *combo_class_buf;
extern int32_t edit_combo_cset;
using std::string;
using std::to_string;

static size_t cmb_tab1 = 0;

void call_combo_editor(int32_t index)
{
	ComboEditorDialog(index).show();
}

ComboEditorDialog::ComboEditorDialog(newcombo const& ref, int32_t index):
	local_comboref(ref), index(index),
	list_ctype(GUI::ListData::combotype(true)),
	list_flag(GUI::ListData::mapflag()),
	list_counters(GUI::ListData::counters()),
	list_sprites(GUI::ListData::miscsprites()),
	list_weaptype(GUI::ListData::lweaptypes()),
	list_deftypes(GUI::ListData::deftypes()),
	lasttype(-1), typehelp(""), flaghelp("")
{}

ComboEditorDialog::ComboEditorDialog(int32_t index):
	ComboEditorDialog(combobuf[index], index)
{}

//{ Help Strings
static const char *combotype_help_string[cMAX] =
{
    "Select a Type, then click this button to find out what it does.",
    "The player is warped via Tile Warp A if they step on the bottom half of this combo.",
    "The player marches down into this combo and is warped via Tile Warp A if they step on this. The combo's tile will be drawn above the player during this animation.",
    "Liquid can contain Zora enemies and can be crossed with various weapons and items. If the matching quest rule is set, the player can drown in it.",
    "When touched, this combo produces an Armos and changes to the screen's Under Combo.",
    "When touched, this combo produces one Ghini.",
    "Raft paths must begin on a Dock-type combo. (Use the Raft combo flag to create raft paths.)",
    "", //cUNDEF
    "A Bracelet is not needed to push this combo, but it can't be pushed until the enemies are cleared from the screen.",
    "A Bracelet is needed to push this combo. The screen's Under Combo will appear beneath it when it is pushed aside.",
    "A Bracelet is needed to push this combo, and it can't be pushed until the enemies are cleared from the screen.",
    "If the 'Statues Shoot Fire' Screen Data flag is checked, an invisible fireball shooting enemy is spawned on this combo.",
    "If the 'Statues Shoot Fire' Screen Data flag is checked, an invisible fireball shooting enemy is spawned on this combo.",
    "The player's movement speed is reduced while they walk on this combo. Enemies will not be affected.",
    "While the player is standing on top of this, they will be moved upward at 1/4 of their normal walking speed (or some dir at a custom-set speed), until they collide with a solid combo.",
    "While the player is standing on top of this, they will be moved downward at 1/4 of their normal walking speed (or some dir at a custom-set speed), until they collide with a solid combo.",
    "While the player is standing on top of this, they will be moved leftward at 1/4 of their normal walking speed (or some dir at a custom-set speed), until they collide with a solid combo.",
    "While the player is standing on top of this, they will be moved rightward at 1/4 of their normal walking speed (or some dir at a custom-set speed), until they collide with a solid combo.",
    "The player is warped via Tile Warp A if they swim on this combo. Otherwise, this is identical to Water.",
    "The player is warped via Tile Warp A if they dive on this combo. Otherwise, this is identical to Water.",
    "If this combo is solid, the Ladder and Hookshot can be used to cross over it. It only permits the Ladder if it's on Layer 0.",
    "This triggers Screen Secrets when the bottom half of this combo is stepped on, but it does not set the screen's 'Secret' Screen State.",
    "This triggers Screen Secrets when the bottom half of this combo is stepped on, and sets the screen's 'Secret' Screen State, making the secrets permanent.",
    "", // Unused
    "When stabbed or slashed with a Sword, this combo changes into the screen's Under Combo.",
    "Identical to Slash, but an item from Item Drop Set 12 is created when this combo is slashed.",
    "A Bracelet with a Push Combo Level of 2 is needed to push this combo. Otherwise, this is identical to Push (Heavy).",
    "A Bracelet with a Push Combo Level of 2 is needed to push this combo. Otherwise, this is identical to Push (Heavy, Wait).",
    "When hit by a Hammer, this combo changes into the next combo in the list.",
    "If this combo is struck by the Hookshot, the player is pulled towards the combo.",
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
    "The player is warped via Tile Warp A if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
    "If this combo is solid, the Hookshot can be used to cross over it.",
    "This combo's tile is drawn between layers 3 and 4 if it is placed on layer 0.",
    "Flying enemies (Keese, Peahats, Moldorms, Patras, Fairys, Digdogger, Manhandla, Ghinis, Gleeok heads) can't fly over or appear on this combo.",
    "Wand magic and enemy magic that hits  this combo is reflected 180 degrees, and becomes 'reflected magic'.",
    "Wand magic and enemy magic that hits  this combo is reflected 90 degrees, and become 'reflected magic'.",
    "Wand magic and enemy magic that hits  this combo is reflected 90 degrees, and become 'reflected magic'.",
    "Wand magic and enemy magic that hits  this combo is duplicated twice, causing three shots to be fired in three directions.",
    "Wand magic and enemy magic that hits  this combo is duplicated thrice, causing four shots to be fired from each direction.",
    "Wand magic and enemy magic that hits this combo is destroyed.",
    "The player marches up into this combo and is warped via Tile Warp A if they step on this. The combo's tile will be drawn above the player during this animation.",
    "The combo's tile changes depending on the player's position relative to the combo. It uses eight tiles per animation frame.",
    "Identical to Eyeball (8-Way A), but the angles at which the tile will change are offset by 22.5 degrees (pi/8 radians).",
    "Tektites cannot jump through or appear on this combo.",
    "Identical to Slash->Item, but when it is slashed, Bush Leaves sprites are drawn and the 'Tall Grass slashed' sound plays.",
    "Identical to Slash->Item, but when it is slashed, Flower Clippings sprites are drawn and the 'Tall Grass slashed' sound plays.",
    "Identical to Slash->Item, but when it is slashed, Grass Clippings sprites are drawn and the 'Tall Grass slashed' sound plays.",
    "Ripples sprites are drawn on the player when they walk on this combo. Also, Quake Hammer pounds are nullified by this combo.",
    "If the combo is solid and the player pushes it with at least one Key, it changes to the next combo, the 'Lock Blocks' Screen State is set, and one key is used up.",
    "Identical to Lock Block, but if any other Lock Blocks are opened on the same screen, this changes to the next combo.",
    "If the combo is solid and the player pushes it with the Boss Key, it changes to the next combo and the 'Boss Lock Blocks' Screen State is set.",
    "Identical to Lock Block (Boss), but if any other Boss Lock Blocks are opened on the same screen, this changes to the next combo.",
    "If this combo is solid, the Ladder can be used to cross over it. Only works on layer 0.",
    "When touched, this combo produces a Ghini and changes to the next combo in the list.",
	//Chests
    "", "", "", "", "", "",
    "If the player touches this, the Screen States are cleared, and the player is re-warped back into the screen, effectively resetting the screen entirely.",
    "Press the 'Start' button when the player is standing on the bottom of this combo, and the Save menu appears. Best used with the Save Point->Continue Here Screen Flag.",
    "Identical to Save Point, but the Quit option is also available in the menu.",
    "The player marches down into this combo and is warped via Tile Warp B if they step on this. The combo's tile will be drawn above the player during this animation.",
    "The player marches down into this combo and is warped via Tile Warp C if they step on this. The combo's tile will be drawn above the player during this animation.",
    "The player marches down into this combo and is warped via Tile Warp D if they step on this. The combo's tile will be drawn above the player during this animation.",
    "The player is warped via Tile Warp B if they step on the bottom half of this combo.",
    "The player is warped via Tile Warp C if they step on the bottom half of this combo.",
    "The player is warped via Tile Warp D if they step on the bottom half of this combo.",
    "The player is warped via Tile Warp B if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
    "The player is warped via Tile Warp C if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
    "The player is warped via Tile Warp D if they touch any part of this combo, but their on-screen position remains the same. Ground enemies can't enter.",
    "The player marches up into this combo and is warped via Tile Warp B if they step on this. The combo's tile will be drawn above the player during this animation.",
    "The player marches up into this combo and is warped via Tile Warp C if they step on this. The combo's tile will be drawn above the player during this animation.",
    "The player marches up into this combo and is warped via Tile Warp D if they step on this. The combo's tile will be drawn above the player during this animation.",
    "The player is warped via Tile Warp B if they swim on this combo. Otherwise, this is identical to Water.",
    "The player is warped via Tile Warp C if they swim on this combo. Otherwise, this is identical to Water.",
    "The player is warped via Tile Warp D if they swim on this combo. Otherwise, this is identical to Water.",
    "The player is warped via Tile Warp B if they dive on this combo. Otherwise, this is identical to Water.",
    "The player is warped via Tile Warp C if they dive on this combo. Otherwise, this is identical to Water.",
    "The player is warped via Tile Warp D if they dive on this combo. Otherwise, this is identical to Water.",
    "Identical to Stairs [A], but the Tile Warp used (A, B, C, or D) is chosen at random. Use this only in screens where all four Tile Warps are defined.",
    "Identical to Direct Warp [A], but the Tile Warp used (A, B, C, or D) is chosen at random. Use this only in screens where all four Tile Warps are defined.",
    "As soon as this combo appears on the screen, Side Warp A is triggered. This is best used with secret combos or combo cycling.",
    "As soon as this combo appears on the screen, Side Warp B is triggered. This is best used with secret combos or combo cycling.",
    "As soon as this combo appears on the screen, Side Warp C is triggered. This is best used with secret combos or combo cycling.",
    "As soon as this combo appears on the screen, Side Warp D is triggered. This is best used with secret combos or combo cycling.",
    "Identical to Auto Side Warp [A], but the Side Warp used (A, B, C, or D) is chosen at random. Use this only in screens where all four Side Warps are defined.",
    "Identical to Stairs [A], but the player will be warped as soon as they touch the edge of this combo.",
    "Identical to Stairs [B], but the player will be warped as soon as they touch the edge of this combo.",
    "Identical to Stairs [C], but the player will be warped as soon as they touch the edge of this combo.",
    "Identical to Stairs [D], but the player will be warped as soon as they touch the edge of this combo.",
    "Identical to Stairs [Random], but the player will be warped as soon as they touch the edge of this combo.",
    "Identical to Step->Secrets (Temporary), but Screen Secrets are triggered as soon as the player touches the edge of this combo.",
    "Identical to Step->Secrets (Permanent), but Screen Secrets are triggered as soon as the player touches the edge of this combo.",
    "When the player steps on this combo, it will change into the next combo in the list.",
    "Identical to Step->Next, but if other instances of this particular combo are stepped on, this also changes to the next combo in the list.",
    "When the player steps on this combo, each of the Step->Next combos on screen will change to the next combo after them in the list.",
    "When the player steps on a Step->Next (All) type combo, this will change into the next combo in the list.",
    "Enemies cannot enter or appear on this combo.",
    "Level 1 player arrows that hit this combo are destroyed. Enemy arrows are unaffected.",
    "Level 1 or 2 player arrows that hit this combo are destroyed. Enemy arrows are unaffected.",
    "All player arrows that hit this combo are destroyed. Enemy arrows are unaffected.",
    "Level 1 player boomerangs bounce off this combo. Enemy boomerangs are unaffected.",
    "Level 1 or 2 player boomerangs bounce off this combo. Enemy boomerangs are unaffected.",
    "All player boomerangs bounce off this combo. Enemy boomerangs are unaffected.",
    "The player's sword beams or enemy sword beams that hit this combo are destroyed.",
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
	"Generic combos can be configured to do a wide variety of things based on attributes. See combos.txt for details.",
	"Pitfall combos act as either bottomless pits or warps, including a fall animation. See combos.txt for details.",
	"Step->Effects combos can cause SFX, and also act like a landmine, spawning an EWeapon. See combos.txt for details.",
	"Bridge combos can be used to block combos under them from having an effect.",
	"Signpost combos can be set to display a string This can be hard-coded, or variable. See combos.txt for details.",
	"Switch combos, when triggered, toggle a switch state for the current 'level'. See combos.txt for details.",
	"Switchblock combos change based on switch states toggled by switch combos. See combos.txt for details.",
	"Emits light in a radius in dark rooms (when \"Quest->Options->Other->New Dark Rooms\" is enabled)"
};
//}


//Load all the info for the combo type and checked flags
void ComboEditorDialog::loadComboType()
{
	if(lasttype != local_comboref.type) //Load type helpinfo
	{
		lasttype = local_comboref.type;
		switch(lasttype)
		{
			case cNONE:
				typehelp = "Select a Type, then click this button to find out what it does.";
				break;
			case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
			case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
			{
				char buf[512];
				int32_t lvl = (lasttype < cDAMAGE5 ? (lasttype - cDAMAGE1 + 1) : (lasttype - cDAMAGE5 + 1));
				int32_t d = -combo_class_buf[lasttype].modify_hp_amount/8;
				char buf2[80];
				if(d==1)
					sprintf(buf2,"1/2 of a heart.");
				else
					sprintf(buf2,"%d heart%s.", d/2, d == 2 ? "" : "s");
				sprintf(buf,"If the Player touches this combo without Boots that protect against Damage Combo Level %d, he is damaged for %s.",lvl,buf2);
				typehelp = buf;
				break;
			}
			
			case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
				typehelp = "If no button is assigned, the chest opens when pushed against from a valid side. If buttons are assigned:"
					"\n(A=1, B=2, L=4, R=8, Ex1=16, Ex2=32, Ex3=64, Ex4=128)(sum all the buttons you want to be usable)\n"
					"then when the button is pressed while facing the chest from a valid side.\n"
					"When the chest is opened, if it has the 'Armos/Chest->Item' combo flag, the player will recieve the item set in the screen's catchall value, and the combo will advance to the next combo.";
				if(lasttype==cLOCKEDCHEST)
					typehelp += "\nRequires a key to open.";
				else if(lasttype==cBOSSCHEST)
					typehelp += "\nRequires the Boss Key to open.";
				break;
			case cCHEST2:
				typehelp = "Acts as a chest that can't be opened. Becomes 'opened' (advancing to the next combo) when any 'Treasure Chest (Normal)' is opened on the screen.";
				break;
			case cLOCKEDCHEST2:
				typehelp = "Acts as a chest that can't be opened. Becomes 'opened' (advancing to the next combo) when any 'Treasure Chest (Locked)' is opened on the screen.";
				break;
			case cBOSSCHEST2:
				typehelp = "Acts as a chest that can't be opened. Becomes 'opened' (advancing to the next combo) when any 'Treasure Chest (Boss)' is opened on the screen.";
				break;
			
			case cHSBRIDGE: case cZELDA: case cUNDEF: case cCHANGE: case cSPINTILE2:
				typehelp = "Unimplemented type, do not use!";
				break;
			
			default:
				typehelp = combotype_help_string[lasttype];
				break;
		}
	}
	string flagstrs[16];
	string attribytestrs[8];
	string attrishortstrs[8];
	string attributestrs[4];
	for(size_t q = 0; q < 16; ++q)
	{
		flagstrs[q] = "Flags["+to_string(q)+"]";
		if(q < 8)
		{
			attribytestrs[q] = "Attribytes["+to_string(q)+"]:";
			attrishortstrs[q] = "Attrishorts["+to_string(q)+"]:";
			if(q < 4)
				attributestrs[q] = "Attributes["+to_string(q)+"]:";
		}
	}
	switch(lasttype) //Label names
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		case cTRIGNOFLAG: case cSTRIGNOFLAG:
		case cTRIGFLAG: case cSTRIGFLAG:
		{
			attribytestrs[0] = "Sound:";
			break;
		}
		case cSTEP: case cSTEPSAME: case cSTEPALL:
		{
			flagstrs[0] = "Heavy";
			attribytestrs[0] = "Sound:";
			attribytestrs[1] = "Req. Item";
			break;
		}
		case cSTEPCOPY:
		{
			flagstrs[0] = "Heavy";
			break;
		}
		case cWATER:
		{
			flagstrs[0] = "Is Lava"; flagstrs[1] = "Modify HP (Passive)";
			flagstrs[2] = "Solid is Land"; flagstrs[3] = "Solid is Shallow Liquid";
			attributestrs[0] = "Drown Damage:"; attribytestrs[0] = "Flipper Level:";
			if(local_comboref.usrflags & cflag2) //Modify HP
			{
				flagstrs[4] = "Rings affect HP Mod";
				flagstrs[5] = "Mod SFX only on HP change";
				flagstrs[6] = "Damage causes hit anim";
				attributestrs[1] = "HP Modification:";
				attributestrs[2] = "HP Mod SFX:";
				attribytestrs[1] = "HP Delay:";
				attribytestrs[2] = "Req Itemclass:";
				attribytestrs[3] = "Req Itemlevel:";
			}
			break;
		}
		case cSHALLOWWATER:
		{
			flagstrs[1] = "Modify HP (Passive)";
			attribytestrs[0] = "Sound";
			if(local_comboref.usrflags & cflag2) //Modify HP
			{
				attributestrs[1] = "HP Modification:";
				attributestrs[2] = "HP Mod SFX:";
				attribytestrs[1] = "HP Delay:";
				attribytestrs[2] = "Req Itemclass:";
				attribytestrs[3] = "Req Itemlevel:";
			}
			break;
		}
		case cARMOS:
		{
			flagstrs[0] = "Specify";
			flagstrs[1] = "Random";
			attribytestrs[0] = "Enemy 1:";
			attribytestrs[1] = "Enemy 2:";
			break;
		}
		case cCVUP:
		case cCVDOWN:
		case cCVLEFT:
		case cCVRIGHT:
		{
			flagstrs[1] = "Custom Speed";
			if(local_comboref.usrflags & cflag2) //Custom speed
			{
				attributestrs[0] = "X Speed:";
				attributestrs[1] = "Y Speed:";
				attribytestrs[0] = "Rate:";
			}
			break;
		}
		case cTALLGRASS:
		{
			flagstrs[0] = "Visuals";
			flagstrs[1] = "Itemdrop";
			attribytestrs[0] = "Sprite:";
			attribytestrs[1] = "Dropset:";
			attribytestrs[2] = "Sound:";
			break;
		}
		case cBUSH: case cBUSHTOUCHY: case cFLOWERS: case cSLASHNEXTTOUCHY:
		{
			flagstrs[0] = "Visuals";
			flagstrs[1] = "Itemdrop";
			attribytestrs[0] = "Sprite:";
			attribytestrs[1] = "Dropset:";
			break;
		}
		case cSLASHITEM:
		{
			flagstrs[1] = "Itemdrop";
			attribytestrs[1] = "Dropset:";
			break;
		}
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		{
			flagstrs[0] = "Custom";
			flagstrs[1] = "No Knockback";
			attributestrs[0] = "Amount:";
			break;
		}
		case cLOCKBLOCK:
		{
			flagstrs[0] = "Require";
			flagstrs[1] = "Only";
			flagstrs[2] = "SFX";
			flagstrs[3] = "Counter";
			flagstrs[4] = "Eat Item";
			flagstrs[5] = "Thief";
			flagstrs[7] = "No Drain";
			attributestrs[0] = "Amount";
			attribytestrs[0] = "Item";
			attribytestrs[1] = "Counter";
			attribytestrs[3] = "Sound";
			break;
		}
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		{
			flagstrs[8] = "Can't use from top";
			flagstrs[9] = "Can't use from bottom";
			flagstrs[10] = "Can't use from left";
			flagstrs[11] = "Can't use from right";
			attribytestrs[2] = "Button:";
			break;
		}
		case cSIGNPOST:
		{
			flagstrs[8] = "Can't use from top";
			flagstrs[9] = "Can't use from bottom";
			flagstrs[10] = "Can't use from left";
			flagstrs[11] = "Can't use from right";
			attribytestrs[2] = "Button:";
			attributestrs[0] = "String:";
			break;
		}
		case cTALLGRASSTOUCHY: case cTALLGRASSNEXT:
		{
			flagstrs[0] = "Visuals";
			flagstrs[1] = "Itemdrop";
			flagstrs[9] = "Clippings";
			flagstrs[10] = "Specific Item";
			attribytestrs[0] = "Sprite:";
			attribytestrs[1] = "Dropset:";
			attribytestrs[2] = "Sound:";
			break;
		}
		case cSLASHNEXTITEM: case cBUSHNEXT: case cSLASHITEMTOUCHY:
		case cFLOWERSTOUCHY: case cBUSHNEXTTOUCHY:
		{
			flagstrs[0] = "Visuals";
			flagstrs[1] = "Itemdrop";
			flagstrs[9] = "Clippings";
			flagstrs[10] = "Specific Item";
			attribytestrs[0] = "Sprite:";
			attribytestrs[1] = "Dropset:";
			break;
		}
		case cSLASHNEXT:
		{
			flagstrs[0] = "Visuals";
			flagstrs[9] = "Clippings";
			flagstrs[10] = "Specific Item";
			attribytestrs[0] = "Sprite:";
			break;
		}
		case cSLASHNEXTITEMTOUCHY:
		{
			flagstrs[1] = "Itemdrop";
			flagstrs[9] = "Clippings";
			flagstrs[10] = "Specific Item";
			attribytestrs[1] = "Dropset:";
			break;
		}
		case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
		case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
		case cSCRIPT11: case cSCRIPT12: case cSCRIPT13: case cSCRIPT14: case cSCRIPT15:
		case cSCRIPT16: case cSCRIPT17: case cSCRIPT18: case cSCRIPT19: case cSCRIPT20:
		{
			flagstrs[8] = "Engine";
			if(local_comboref.usrflags & cflag9) //Engine flag
			{
				flagstrs[0] = "Visuals"; flagstrs[1] = "Itemdrop";
				flagstrs[2] = "SFX"; flagstrs[3] = "Next";
				flagstrs[4] = "Continuous"; flagstrs[6] = "Secrets";
				flagstrs[7] = "Kill Wpn"; flagstrs[9] = "Clippings";
				flagstrs[10] = "Specific Item"; flagstrs[11] = "Undercombo";
				flagstrs[12] = "Always Drop"; flagstrs[13] = "Drop Enemy";
				attribytestrs[0] = "Sprite:"; attribytestrs[1] = "Dropset:";
				attribytestrs[2] = "Sound:"; attribytestrs[3] = "Secret Type:";
				if(local_comboref.usrflags & cflag14) //Drop Enemy flag
				{
					flagstrs[5] = "No Poof";
				}
				else flagstrs[5] = "Room Item";
			}
			break;
		}
		case cTRIGGERGENERIC:
		{
			flagstrs[0] = "Visuals"; flagstrs[1] = "Itemdrop";
			flagstrs[2] = "SFX"; flagstrs[3] = "Next";
			flagstrs[4] = "Continuous"; flagstrs[6] = "Singular Secret";
			flagstrs[7] = "Kill Wpn"; flagstrs[9] = "Clippings";
			flagstrs[10] = "Specific Item"; flagstrs[11] = "Undercombo";
			flagstrs[12] = "Always Drop"; flagstrs[13] = "Drop Enemy";
			attribytestrs[0] = "Sprite:"; attribytestrs[1] = "Dropset:";
			attribytestrs[2] = "Sound:"; attribytestrs[3] = "Singular Secret:";
			if(local_comboref.usrflags & cflag14) //Drop Enemy flag
			{
				flagstrs[5] = "No Poof";
			}
			else flagstrs[5] = "Room Item";
			break;
		}
		case cPITFALL:
		{
			flagstrs[0] = "Warp"; flagstrs[2] = "Damage is Percent";
			flagstrs[3] = "Allow Ladder"; flagstrs[4] = "No Pull";
			attributestrs[0] = "Damage:"; attribytestrs[0] = "Fall SFX:";
			if(local_comboref.usrflags & cflag1) //Warp enabled
			{
				flagstrs[1] = "Direct Warp";
				attribytestrs[1] = "TileWarp ID";
			}
			if(!(local_comboref.usrflags & cflag5)) //"No Pull"
				attribytestrs[2] = "Pull Sensitivity:";
			break;
		}
		case cSTEPSFX:
		{
			flagstrs[0] = "Landmine";
			flagstrs[1] = "wCustom is LWeapon";
			flagstrs[2] = "Don't Advance";
			flagstrs[3] = "Direct Damage";
			attributestrs[0] = "Damage:";
			attribytestrs[0] = "Sound:";
			attribytestrs[1] = "Weapon Type:";
			attribytestrs[2] = "Initial Dir:";
			attribytestrs[3] = "Sprite:";
			break;
		}
		case cCSWITCH:
		{
			flagstrs[0] = "Kill Wpn";
			flagstrs[7] = "Skip Cycle on Screen Entry";
			attributestrs[0] = "Combo Change:";
			attributestrs[1] = "CSet Change:";
			attribytestrs[0] = "State Num:";
			attribytestrs[1] = "SFX:";
			break;
		}
		case cCSWITCHBLOCK:
		{
			flagstrs[0] = "Change L0"; flagstrs[1] = "Change L1";
			flagstrs[2] = "Change L2"; flagstrs[3] = "Change L3";
			flagstrs[4] = "Change L4"; flagstrs[5] = "Change L5";
			flagstrs[6] = "Change L6";
			flagstrs[7] = "Skip Cycle on Screen Entry";
			flagstrs[8] = "Allow walk-on-top";
			attributestrs[0] = "Combo Change:";
			attributestrs[1] = "CSet Change:";
			attribytestrs[0] = "State Num:";
			if(local_comboref.usrflags & cflag9) //Allow walk-on-top
			{
				flagstrs[9] = "-8px DrawYOffset";
				attributestrs[2] = "Z-value:";
				attributestrs[3] = "Step Height:";
			}
			break;
		}
		case cLANTERN:
		{
			attribytestrs[0] = "Radius:";
			break;
		}
	}
	for(size_t q = 0; q < 16; ++q)
	{
		l_flags[q]->setText(flagstrs[q]);
		if(q > 7) continue;
		l_attribytes[q]->setText(attribytestrs[q]);
		l_attrishorts[q]->setText(attrishortstrs[q]);
		if(q > 3) continue;
		l_attributes[q]->setText(attributestrs[q]);
	}
}
void ComboEditorDialog::loadComboFlag()
{
	flaghelp = "Flag Info"; //!TODO flag help text
	// switch(local_comboref.flag)
	// {
		
	// }
}
void ComboEditorDialog::updateCSet()
{
	tswatch->setCSet(edit_combo_cset);
	if(local_comboref.animflags&AF_CYCLENOCSET)
		cycleswatch->setCSet(edit_combo_cset);
	else cycleswatch->setCSet(local_comboref.nextcset);
	animFrame->setCSet(edit_combo_cset);
	l_cset->setText(std::to_string(edit_combo_cset));
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
}

//{ Macros

#define DISABLE_WEAP_DATA true
#define ATTR_WID 6_em
#define ATTR_LAB_WID 12_em
#define SPR_LAB_WID sized(14_em,10_em)
#define ACTION_LAB_WID 6_em
#define ACTION_FIELD_WID 6_em
#define FLAGS_WID 16_em

#define NUM_FIELD(member,_min,_max) \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = local_comboref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_comboref.member = val; \
	})
	
#define ANIM_FIELD(member,_min,_max) \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = local_comboref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_comboref.member = val; \
		updateAnimation(); \
	})

#define CMB_FLAG(ind) \
l_flags[ind] = Checkbox( \
		minwidth = FLAGS_WID, hAlign = 0.0, \
		checked = local_comboref.usrflags & (1<<ind), fitParent = true, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(local_comboref.usrflags,(1<<ind),state); \
			loadComboType(); \
		} \
	)

#define CMB_GEN_FLAG(ind,str) \
Checkbox(text = str, \
		minwidth = FLAGS_WID, hAlign = 0.0, \
		checked = local_comboref.genflags & (1<<ind), fitParent = true, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(local_comboref.genflags,(1<<ind),state); \
		} \
	)

#define CMB_ATTRIBYTE(ind) \
l_attribytes[ind] = Label(minwidth = ATTR_LAB_WID, textAlign = 2), \
TextField( \
	fitParent = true, minwidth = 8_em, \
	type = GUI::TextField::type::SWAP_BYTE, \
	low = 0, high = 255, val = local_comboref.attribytes[ind], \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_comboref.attribytes[ind] = val; \
	})

#define CMB_ATTRISHORT(ind) \
l_attrishorts[ind] = Label(minwidth = ATTR_LAB_WID, textAlign = 2), \
TextField( \
	fitParent = true, minwidth = 8_em, \
	type = GUI::TextField::type::SWAP_SSHORT, \
	low = -32768, high = 32767, val = local_comboref.attrishorts[ind], \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_comboref.attrishorts[ind] = val; \
	})

#define CMB_ATTRIBUTE(ind) \
l_attributes[ind] = Label(minwidth = ATTR_LAB_WID, textAlign = 2), \
TextField( \
	fitParent = true, minwidth = 8_em, \
	type = GUI::TextField::type::SWAP_ZSINT, \
	val = local_comboref.attributes[ind], \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_comboref.attributes[ind] = val; \
	})

#define TRIGFLAG(ind, str) \
Checkbox( \
	text = str, hAlign = 0.0, \
	checked = (local_comboref.triggerflags[ind/32] & (1<<(ind%32))), \
	fitParent = true, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_comboref.triggerflags[ind/32],(1<<(ind%32)),state); \
	} \
)

//}

int32_t solidity_to_flag(int32_t val)
{
	return (val&0b1001) | (val&0b0100)>>1 | (val&0b0010)<<1;
}

std::shared_ptr<GUI::Widget> ComboEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	char titlebuf[256];
	sprintf(titlebuf, "Combo Editor (%d)", index);
	if(is_large)
	{
		window = Window(
			use_vsync = true,
			title = titlebuf,
			info = "Edit combos, setting up their graphics, effects, and attributes.\n"
				"Hotkeys:\n"
				"-/+: Change CSet\n"
				"H/V/R: Flip (Horz,Vert,Rotate)",
			onEnter = message::OK,
			onClose = message::CANCEL,
			shortcuts={
				V=message::VFLIP,
				H=message::HFLIP,
				R=message::ROTATE,
				PlusPad=message::PLUSCS,
				Equals=message::PLUSCS,
				MinusPad=message::MINUSCS,
				Minus=message::MINUSCS,
			},
			Column(
				Rows<3>(padding = 0_px,
					Label(text = "Type:", hAlign = 1.0),
					DropDownList(data = list_ctype, fitParent = true,
						padding = 0_px, selectedValue = local_comboref.type,
						onSelectionChanged = message::COMBOTYPE
					),
					Button(
						width = 1.5_em, padding = 0_px, forceFitH = true,
						text = "?", hAlign = 1.0, onPressFunc = [&]()
						{
							InfoDialog(moduledata.combo_type_names[local_comboref.type],typehelp).show();
						}
					),
					Label(text = "Inherent Flag:", hAlign = 1.0),
					DropDownList(data = list_flag, fitParent = true,
						padding = 0_px, selectedValue = local_comboref.flag,
						onSelectionChanged = message::COMBOFLAG
					),
					Button(
						width = 1.5_em, padding = 0_px, forceFitH = true,
						text = "?", hAlign = 1.0, onPressFunc = [&]()
						{
							//!TODO Combo flag help text
							InfoDialog(moduledata.combo_type_names[local_comboref.type],flaghelp).show();
						}
					)
				),
				TabPanel(
					ptr = &cmb_tab1,
					TabRef(name = "Basic", Row(
						Rows<2>(
							Label(text = "Label:", hAlign = 1.0),
							TextField(
								fitParent = true,
								type = GUI::TextField::type::TEXT,
								maxLength = 10,
								text = std::string(local_comboref.label),
								onValChangedFunc = [&](GUI::TextField::type,std::string_view text,int32_t)
								{
									std::string foo;
									foo.assign(text);
									strncpy(local_comboref.label, foo.c_str(), 10);
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
							l_cset = Label(text = std::to_string(edit_combo_cset), hAlign = 0.0)
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
									cset = edit_combo_cset,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c)
									{
										local_comboref.tile = t;
										edit_combo_cset = (c&0xF)%12;
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
									cset = edit_combo_cset,
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
								text = "Cycle Ignores CSet", hAlign = 0.0,
								checked = local_comboref.animflags & AF_CYCLENOCSET,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_comboref.animflags,AF_CYCLENOCSET,state);
									updateCSet();
								}
							)
						)
					)),
					TabRef(name = "Flags", Column(
						padding = 0_px,
						Columns<2>(
							framed = true,
							frameText = "General Flags",
							topPadding = DEFAULT_PADDING+0.4_em,
							bottomPadding = DEFAULT_PADDING+1_px,
							bottomMargin = 1_em,
							CMB_GEN_FLAG(0,"Hook-Grabbable")
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
					TabRef(name = "Attribs", ScrollingPane(
						fitParent = true,
						Rows<4>(
							Label(text = "Attribytes", colSpan = 2),
							Label(text = "Attrishorts", colSpan = 2),
							CMB_ATTRIBYTE(0),
							CMB_ATTRISHORT(0),
							CMB_ATTRIBYTE(1),
							CMB_ATTRISHORT(1),
							CMB_ATTRIBYTE(2),
							CMB_ATTRISHORT(2),
							CMB_ATTRIBYTE(3),
							CMB_ATTRISHORT(3),
							CMB_ATTRIBYTE(4),
							CMB_ATTRISHORT(4),
							CMB_ATTRIBYTE(5),
							CMB_ATTRISHORT(5),
							CMB_ATTRIBYTE(6),
							CMB_ATTRISHORT(6),
							CMB_ATTRIBYTE(7),
							CMB_ATTRISHORT(7),
							Label(text = "Attributes", colSpan = 2), DummyWidget(colSpan = 2),
							CMB_ATTRIBUTE(0), DummyWidget(colSpan = 2),
							CMB_ATTRIBUTE(1), DummyWidget(colSpan = 2),
							CMB_ATTRIBUTE(2), DummyWidget(colSpan = 2),
							CMB_ATTRIBUTE(3), DummyWidget(colSpan = 2)
						)
					)),
					TabRef(name = "Triggers", Column(//ScrollingPane(
						Column(
							padding = 0_px,
							Row(
								padding = 0_px,
								Label(text = "Min Level (Applies to all):"),
								TextField(
									fitParent = true,
									vPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 0, high = 214748, val = local_comboref.triggerlevel,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_comboref.triggerlevel = val;
									})
							),
							Rows<4>(
								framed = true,
								TRIGFLAG(0,"Sword"),
								TRIGFLAG(1,"Sword Beam"),
								TRIGFLAG(2,"Boomerang"),
								TRIGFLAG(3,"Bomb"),
								TRIGFLAG(4,"Super Bomb"),
								TRIGFLAG(5,"Lit Bomb"),
								TRIGFLAG(6,"Lit Super Bomb"),
								TRIGFLAG(7,"Arrow"),
								TRIGFLAG(8,"Fire"),
								TRIGFLAG(9,"Whistle"),
								TRIGFLAG(10,"Bait"),
								TRIGFLAG(11,"Wand"),
								TRIGFLAG(12,"Magic"),
								TRIGFLAG(13,"Wind"),
								TRIGFLAG(14,"Refl. Magic"),
								TRIGFLAG(15,"Refl. Fireball"),
								TRIGFLAG(16,"Refl. Rock"),
								TRIGFLAG(17,"Hammer"),
								TRIGFLAG(32,"Hookshot"),
								TRIGFLAG(33,"Sparkle"),
								TRIGFLAG(34,"Byrna"),
								TRIGFLAG(35,"Refl. Beam"),
								TRIGFLAG(36,"Stomp"),
								DummyWidget(),
								TRIGFLAG(37,"Custom Weapon 1"),
								TRIGFLAG(38,"Custom Weapon 2"),
								TRIGFLAG(39,"Custom Weapon 3"),
								TRIGFLAG(40,"Custom Weapon 4"),
								TRIGFLAG(41,"Custom Weapon 5"),
								TRIGFLAG(42,"Custom Weapon 6"),
								TRIGFLAG(43,"Custom Weapon 7"),
								TRIGFLAG(44,"Custom Weapon 8"),
								TRIGFLAG(45,"Custom Weapon 9"),
								TRIGFLAG(46,"Custom Weapon 10")
							),
							Rows<4>(
								framed = true,
								TRIGFLAG(47,"Always Triggered"),
								TRIGFLAG(48,"Triggers Secrets")
							)
						)
					))
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
	}
	loadComboType();
	loadComboFlag();
	return window;
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
			local_comboref.flip ^= 1;
			for(auto crn : cswatchs)
			{
				int32_t val = crn->getVal();
				crn->setVal((val & 0b0101)<<1 | (val&0b1010)>>1);
			}
			l_flip->setText(std::to_string(local_comboref.flip));
			updateAnimation();
			return false;
		}
		case message::VFLIP:
		{
			local_comboref.flip ^= 2;
			for(auto crn : cswatchs)
			{
				int32_t val = crn->getVal();
				crn->setVal((val & 0b0011)<<2 | (val&0b1100)>>2);
			}
			l_flip->setText(std::to_string(local_comboref.flip));
			updateAnimation();
			return false;
		}
		case message::ROTATE:
		{
			local_comboref.flip = rotate_value(local_comboref.flip);
			for(auto crn : cswatchs)
			{
				int32_t val = crn->getVal();
				int32_t newval = 0;
				if(val&0b0001)
					newval |= 0b0010;
				if(val&0b0010)
					newval |= 0b1000;
				if(val&0b0100)
					newval |= 0b0001;
				if(val&0b1000)
					newval |= 0b0100;
				crn->setVal(newval);
			}
			l_flip->setText(std::to_string(local_comboref.flip));
			updateAnimation();
			return false;
		}
		case message::PLUSCS:
		{
			edit_combo_cset = (edit_combo_cset+1)%12;
			updateCSet();
			return false;
		}
		case message::MINUSCS:
		{
			edit_combo_cset = (edit_combo_cset+11)%12;
			updateCSet();
			return false;
		}
		case message::OK:
			saved = false;
			combobuf[index] = local_comboref;
			return true;

		case message::CANCEL:
		default:
			return true;
	}
}

