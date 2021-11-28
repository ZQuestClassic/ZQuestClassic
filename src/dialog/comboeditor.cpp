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
	// switch(lasttype) //Label names
	// {
		
	// }
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
#define FLAGS_WID 20_em

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

//}

int32_t solidity_to_flag(int32_t val)
{
	return (val&0b1001) | (val&0b0100?0b0010:0) | (val&0b0010?0b0100:0);
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
					TabRef(name = "Basic", Row(
						Rows<2>(
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
								Label(text = "Tile", hAlign = 1.0),
								DummyWidget(),
								Label(text = "Solid", hAlign = 1.0),
								DummyWidget(),
								Label(text = "CSet2", hAlign = 1.0),
								DummyWidget(),
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
								DummyWidget(colSpan = 2),
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
								Label(text = "Cycle", hAlign = 1.0),
								DummyWidget(),
								Label(text = "Effect", hAlign = 1.0),
								DummyWidget()
							)
						)
					)),
					TabRef(name = "2", DummyWidget())
				),
				Row(
					vAlign = 1.0,
					spacing = 2_em,
					Button(
						focused = true,
						text = "OK",
						onClick = message::OK),
					Button(
						text = "Cancel",
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

