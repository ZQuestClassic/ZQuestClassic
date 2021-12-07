#include "itemeditor.h"
#include "info.h"
#include "alert.h"
#include "../zsys.h"
#include <gui/builder.h>

void reset_itembuf(itemdata *item, int32_t id);
extern zquestheader header;
extern bool saved;
extern char *item_string[];
extern itemdata *itemsbuf;
extern zcmodule moduledata;
static bool _reset_default;
static itemdata reset_ref;
static std::string reset_name;
void call_item_editor(int32_t index)
{
	_reset_default = false;
	ItemEditorDialog(index).show();
	while(_reset_default)
	{
		_reset_default = false;
		reset_itembuf(&reset_ref, index);
		ItemEditorDialog(reset_ref, reset_name.c_str(), index).show();
	}
}

static const GUI::ListData PFlagTypeList
{
	{ "Disabled", 0 },
	{ "Set To These", 1 },
	{ "Add These", 2 },
	{ "Subtract These", 3 },
	{ "Limit To These", 4 }
};

static const GUI::ListData WeapMoveTypeList
{
	{ "None", 0 },
	{ "Line", 1 },
	{ "Sine Wave", 2 },
	{ "Cosine", 3 },
	{ "Circular", 4 },
	{ "Arc", 5 },
	{ "Pattern A", 6 },
	{ "Pattern B", 7 },
	{ "Pattern C", 8 },
	{ "Pattern D", 9 },
	{ "Pattern E", 10 },
	{ "Pattern F", 11 }
};

//Sets the Item Editor Field Names
ItemNameInfo inameinf[itype_max];

ItemNameInfo defInfo =
{
	-1,
	"Power:",
	{
		"Attributes[0]:", "Attributes[1]:", "Attributes[2]:",
		"Attributes[3]:", "Attributes[4]:","Attributes[5]:","Attributes[6]:",
		"Attributes[7]:","Attributes[8]:","Attributes[9]:"
	},
	{
		"Flags[0]", "Flags[1]", "Flags[2]", "Flags[3]", "Flags[4]", "Flags[5]",
		"Flags[6]", "Flags[7]", "Flags[8]", "Flags[9]", "Flags[10]", "Flags[11]",
		"Flags[12]", "Flags[13]", "Flags[14]","Constant Script"
	},
	{
		"Sprites[0]:", "Sprites[1]:","Sprites[2]:","Sprites[3]:","Sprites[4]:",
		"Sprites[5]:","Sprites[6]:","Sprites[7]:","Sprites[8]:","Sprites[9]:"
	},
	"UseSound"
};

void loadinfo(ItemNameInfo * inf, int itype)
{
	inf->clear();
	inf->iclass = itype;
	switch(itype)
	{
		case itype_fairy:
		{
			inf->misc[0] = "HP Regained:";
			inf->h_misc[0] = "Life restored when collected. This is in HP points, unless"
				" 'Life is Percent' is checked, then it's a percentage of max life.";
			inf->misc[1] = "MP Regained:";
			inf->h_misc[0] = "Magic restored when collected. This is in MP points, unless"
				" 'Magic is Percent' is checked, then it's a percentage of max magic.";
			inf->misc[2] = "Step Speed:";
			inf->h_misc[2] = "The movement speed of the fairy, in 100ths of pixel/frame";
			inf->misc[3] = "Bottle Fill:";
			inf->h_misc[3] = "What bottle type to fill an empty bottle with if caught using a Bug Net";
			inf->flag[0] = "Life is Percent";
			inf->h_flag[0] = "HP Regained is a percentage out of max HP";
			inf->flag[1] = "Magic is Percent";
			inf->h_flag[0] = "MP Regained is a percentage out of max MP";
			inf->actionsnd = "Item Drop Sound:";
			inf->h_actionsnd = "Plays SFX when dropped";
			break;
		}
		case itype_triforcepiece:
		{
			inf->misc[0] = "Cutscene MIDI:";
			inf->misc[1] = "Cutscene Type (0-1):";
			inf->misc[2] = "Second Collect Sound:";
			inf->misc[3] = "Custom Cutscene Duration";
			inf->misc[4] = "Custom Refill Frame";
			inf->flag[0] = "Side Warp Out";
			inf->flag[2] = "Removes Sword Jinxes";
			inf->flag[3] = "Removes Item Jinxes";
			inf->flag[8] = "Don't Dismiss Messages";
			inf->flag[9] = "Cutscene Interrupts Action Script";
			inf->flag[10] = "Don't Affect Music";
			inf->flag[11] = "No Cutscene";
			inf->flag[12] = "Run Action Script on Collection";
			inf->flag[13] = "Play Second Sound Effect";
			inf->flag[14] = "Don't Play MIDI";
			break;
		}
		case itype_shield:
		{
			inf->misc[0] = "Block Flags:";
			inf->h_misc[0] = "(Rock=1, Arrow=2, BRang=4, Fireball=8, Sword=16, Magic=32, Flame=64, Script=128, Fireball2=256, Lightbeam=512)\n"
				"Sum all of the values you want to apply. Weapons and lightbeams with their flags set will be blocked from in front of the player.";
			inf->misc[1] = "Reflect Flags:";
			inf->h_misc[1] = "(Rock=1, Fireball=8, Sword=16, Magic=32, Script=128, Fireball2=256, Lightbeam=512)\n"
				"Sum all of the values you want to apply. Weapons and lightbeams with their flags set will be reflected from in front of the player.\n"
				"Weapons are only reflected if their value is in both 'Block Flags' and 'Reflect Flags'.";
			inf->actionsnd = "De/Reflection Sound:";
			break;
		}
		case itype_agony:
		{
			inf->power = "Sensitivity:";
			inf->misc[0] = "Vibration Frequency";
			break;
		}
		case itype_wealthmedal:
		{
			inf->misc[0] = "Discount Amount:";
			inf->flag[0] = "D. A. Is Percent";
			break;
		}
		case itype_cbyrna:
		{
			inf->power = "Beam Damage:";
			inf->misc[0] = "Beam Speed Divisor:";
			inf->misc[1] = "Orbit Radius:";
			inf->misc[2] = "Number Of Beams:";
			inf->misc[3] = "Melee Damage:";
			inf->misc[9] = "Slash Sound:";
			inf->flag[0] = "Penetrates Enemies";
			inf->flag[1] = "Invincible Player";
			inf->flag[2] = "No Melee Attack";
			inf->flag[3] = "Can Slash";
			inf->flag[5] = "Melee Weapons Use Magic Cost";
			inf->flag[6] = "Doesn't Pick Up Dropped Items";
			inf->flag[7] = "Flip Right-Facing Slash";
			inf->wpn[0] = "Stab Sprite:";
			inf->wpn[1] = "Slash Sprite:";
			inf->wpn[2] = "Beam Sprite:";
			inf->wpn[3] = "Sparkle Sprite:";
			inf->wpn[4] = "Damaging Sparkle Sprite:";
			inf->actionsnd = "Orbit Sound:";
			break;
		}
		case itype_whispring:
		{
			inf->power = "Jinx Divisor:";
			inf->misc[0] = "Jinx Type:";
			inf->flag[0] = "Perm. Jinx Are Temp.";
			break;
		}
		case itype_ladder:
		{
			inf->power = "Four-Way:";
			inf->flag[0] = "Can Ladder Over Pitfalls";
			break;
		}
		case itype_magickey:
		{
			inf->power = "Dungeon Level:";
			inf->flag[0] = "Lesser D. Levels Also";
			break;
		}
		case itype_wallet:
		{
			inf->power = "Infinite Supply:";
			inf->misc[0] = "Increase Amount:";
			inf->misc[1] = "Delay Duration:";
			break;
		}
		case itype_quiver:
		{
			inf->power = "Infinite Supply:";
			inf->misc[0] = "Increase Amount:";
			inf->misc[1] = "Delay Duration:";
			break;
		}
		case itype_bombbag:
		{
			inf->power = "Infinite Supply:";
			inf->misc[0] = "Increase Amount:";
			inf->misc[1] = "Delay Duration:";
			inf->flag[0] = "Super Bombs Also";
			break;
		}
		case itype_quakescroll2:
		{
			inf->power = "Damage Multiplier:";
			inf->misc[0] = "Stun Duration:";
			inf->misc[1] = "Stun Radius:";
			inf->actionsnd = "Quake Sound:";
			break;
		}
		case itype_quakescroll:
		{
			inf->power = "Damage Multiplier:";
			inf->misc[0] = "Stun Duration:";
			inf->misc[1] = "Stun Radius:";
			inf->actionsnd = "Quake Sound:";
			break;
		}
		case itype_perilscroll:
		{
			inf->misc[0] = "Maximum Hearts:";
			break;
		}
		case itype_spinscroll2:
		{
			inf->power = "Damage Multiplier:";
			inf->misc[0] = "Number of Spins:";
			inf->actionsnd = "Spinning Sound:";
			break;
		}
		case itype_spinscroll:
		{
			inf->power = "Damage Multiplier:";
			inf->misc[0] = "Number of Spins:";
			inf->actionsnd = "Spinning Sound:";
			break;
		}
		case itype_clock:
		{
			inf->misc[0] = "Duration (0 = Infinite):";
			break;
		}
		case itype_magicring:
		{
			inf->power = "Infinite Magic:";
			inf->misc[0] = "Increase Amount:";
			inf->misc[1] = "Delay Duration:";
			break;
		}
		case itype_heartring:
		{
			inf->misc[0] = "Increase Amount:";
			inf->misc[1] = "Delay Duration:";
			break;
		}
		case itype_chargering:
		{
			inf->misc[0] = "Charging Duration:";
			inf->misc[1] = "Magic C. Duration:";
			break;
		}
		case itype_hoverboots:
		{
			inf->misc[0] = "Hover Duration:";
			inf->flag[0] = "Timer only resets on landing";
			inf->wpn[0] = "Halo Sprite:";
			inf->actionsnd = "Hovering Sound:";
			break;
		}
		case itype_rocs:
		{
			inf->power = "Jump Power:";
			inf->misc[0] = "Extra Jumps:";
			inf->flag[0] = "Jump is Power/100";
			inf->actionsnd = "Jumping Sound:";
			break;
		}
		case itype_sbomb:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Fuse Duration (0 = Remote):";
			inf->misc[1] = "Max. On Screen:";
			inf->misc[2] = "Damage to Player:";
			inf->flag[1] = "Explosion Hurts Player";
			inf->wpn[0] = "Bomb Sprite:";
			inf->wpn[1] = "Explosion Sprite:";
			inf->actionsnd = "Explosion Sound:";
			break;
		}
		case itype_bomb:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Fuse Duration (0 = Remote):";
			inf->misc[1] = "Max. On Screen:";
			inf->misc[2] = "Damage to Player:";
			inf->flag[0] = "Use 1.92 Timing";
			inf->flag[1] = "Explosion Hurts Player";
			inf->wpn[0] = "Bomb Sprite:";
			inf->wpn[1] = "Explosion Sprite:";
			inf->actionsnd = "Explosion Sound:";
			break;
		}
		case itype_nayruslove:
		{
			inf->misc[0] = "Duration:";
			inf->flag[0] = "Rocket Flickers";
			inf->flag[1] = "Translucent Rocket";
			inf->flag[2] = "Translucent Shield";
			inf->flag[3] = "Shield Flickers";
			inf->wpn[0] = "Left Rocket Sprite:";
			inf->wpn[1] = "L. Rocket Return Sprite:";
			inf->wpn[2] = "L. Rocket Sparkle Sprite:";
			inf->wpn[3] = "L. Return Sparkle Sprite:";
			inf->wpn[4] = "Shield Sprite (2x2, Over):";
			inf->wpn[5] = "Right Rocket Sprite:";
			inf->wpn[6] = "R. Rocket Return Sprite:";
			inf->wpn[7] = "R. Rocket Sparkle Sprite:";
			inf->wpn[8] = "R. Return Sparkle Sprite:";
			inf->wpn[9] = "Shield Sprite (2x2, Under):";
			inf->actionsnd = "Shield Sound:";
			break;
		}
		case itype_faroreswind:
		{
			inf->misc[0] = "Warp Animation (0-2):";
			inf->actionsnd = "Wind Sound:";
			break;
		}
		case itype_dinsfire:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Number of Flames:";
			inf->misc[1] = "Circle Width:";
			inf->flag[1] = "Don't Provide Light";
			inf->flag[2] = "Falls in Sideview";
			inf->flag[4] = "Temporary Light";
			inf->wpn[0] = "Rocket Up Sprite:";
			inf->wpn[1] = "Rocket Down Sprite:";
			inf->wpn[2] = "R. Up Sparkle Sprite:";
			inf->wpn[3] = "R. Down Sparkle Sprite:";
			inf->wpn[4] = "Flame Sprite:";
			inf->actionsnd = "Ring Sound:";
			break;
		}
		case itype_hammer:
		{
			inf->power = "Damage:";
			inf->flag[2] = "No Melee Attack";
			inf->wpn[0] = "Hammer Sprite:";
			inf->wpn[1] = "Smack Sprite:";
			inf->actionsnd = "Pound Sound:";
			inf->flag[6] = "Doesn't Pick Up Dropped Items";
			break;
		}
		case itype_lens:
		{
			inf->misc[0] = "Lens Width:";
			inf->flag[0] = "Show Hints";
			inf->flag[1] = "Hide Secret Combos";
			inf->flag[2] = "No X-Ray for Items";
			inf->flag[3] = "Show Raft Paths";
			inf->flag[4] = "Show Invis. Enemies";
			inf->actionsnd = "Activation Sound:";
			break;
		}
		case itype_hookshot:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Chain Length:";
			inf->misc[1] = "Chain Links:";
			inf->misc[2] = "Block Flags:";
			inf->misc[3] = "Reflect Flags:";
			inf->flag[0] = "No Handle Damage";
			inf->flag[1] = "Allow Diagonal";
			inf->flag[3] = "Pick Up Anything";
			inf->flag[4] = "Drags Items";
			inf->flag[5] = "Reflects Enemy Projectiles";
			inf->flag[6] = "Picks Up Keys";
			inf->wpn[0] = "Tip Sprite:";
			inf->wpn[1] = "Chain Sprite (H):";
			inf->wpn[2] = "Chain Sprite (V):";
			inf->wpn[3] = "Handle Sprite:";
			inf->wpn[4] = "Diagonal Tip Sprite:";
			inf->wpn[5] = "Diagonal Handle Sprite:";
			inf->wpn[6] = "Diagonal Chain Sprite:";
			inf->actionsnd = "Firing Sound:";
			break;
		}
		case itype_boots:
		{
			inf->power = "Damage Combo Level:";
			inf->flag[0] = "Not Solid Combos";
			inf->flag[1] = "Iron";
			break;
		}
		case itype_bracelet:
		{
			inf->power = "Push Combo Level:";
			inf->flag[0] = "Once Per Screen";
			break;
		}
		case itype_book:
		{
			inf->power = "M. Damage:";
			inf->misc[0] = "W. Damage:";
			inf->misc[1] = "Wand Sound";
			inf->misc[2] = "Special Step";
			inf->misc[3] = "Fire Damage";
			inf->flag[0] = "Fire Magic";
			inf->flag[1] = "Override Wand Damage";
			inf->flag[2] = "Fire Doesn't Hurt Player";
			inf->flag[3] = "Override Wand SFX";
			inf->flag[4] = "Temporary Light";
			inf->flag[5] = "Replace Wand Weapon";
			inf->wpn[0] = "Magic Sprite:";
			inf->wpn[1] = "Projectile Sprite:";
			inf->actionsnd = "Firing Sound:";
			break;
		}
		case itype_ring:
		{
			inf->power = "Damage Divisor:";
			inf->misc[0] = "Player Sprite Pal:";
			inf->flag[0] = "Affects Damage Combos";
			inf->flag[1] = "Divisor is Percentage Multiplier";
			break;
		}
		case itype_wand:
		{
			inf->power = "Damage:";
			inf->misc[0] = "M. Damage";
			inf->misc[1] = "W. Type:";
			inf->misc[2] = "W. Speed:";
			inf->misc[3] = "W. Range:";
			inf->misc[4] = "Move Effect:";
			inf->misc[5] = "Mvt Arg1:";
			inf->misc[6] = "Mvt Arg2:";
			inf->misc[7] = "No. of Clones:";
			inf->misc[8] = "Clone Pattern:";
			inf->misc[9] = "Slash Sound:";
			inf->flag[0] = "Allow Magic w/o Book";
			inf->flag[1] = "Wand Moves";
			inf->flag[2] = "No Melee Attack";
			inf->flag[3] = "Can Slash";
			inf->flag[4] = "No Melee Cooldown";
			inf->flag[5] = "Melee Weapons Use Magic Cost";
			inf->flag[6] = "Doesn't Pick Up Dropped Items";
			inf->flag[7] = "Flip Right-Facing Slash";
			inf->flag[8] = "Light Torches";
			inf->wpn[0] = "Stab Sprite:";
			inf->wpn[1] = "Slash Sprite:";
			inf->wpn[2] = "Projectile Sprite:";
			inf->wpn[3] = "Projectile Misc:";
			inf->actionsnd = "Firing Sound:";
			break;
		}
		case itype_bait:
		{
			inf->misc[0] = "Duration:";
			inf->misc[1] = "Bait Range (0 = infinite)";
			inf->flag[0] = "Repels enemies";
			inf->wpn[0] = "Bait Sprite:";
			inf->actionsnd = "Baiting Sound:";
			break;
		}
		case itype_potion:
		{
			inf->misc[0] = "HP Regained:";
			inf->misc[1] = "MP Regained:";
			inf->flag[0] = "HP R. Is Percent";
			inf->flag[1] = "MP R. Is Percent";
			inf->flag[2] = "Removes Sword Jinxes";
			inf->flag[3] = "Removes Item Jinxes";
			break;
		}
		case itype_whistle:
		{
			inf->misc[0] = "Whirlwind Direction:";
			inf->misc[1] = "Warp Ring:";
			inf->misc[4] = "Weapon Damage";
			inf->flag[0] = "One W.Wind Per Scr.";
			inf->flag[1] = "Has Damage";
			inf->flag[2] = "Whirlwinds Reflect off Prism/Mirror Combos";
			inf->wpn[0] = "Whirlwind Sprite:";
			inf->actionsnd = "Music Sound:";
			break;
		}
		case itype_candle:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Damage to Player:";
			inf->flag[0] = "Once Per Screen";
			inf->flag[1] = "Don't Provide Light";
			inf->flag[2] = "Fire Doesn't Hurt Player";
			inf->flag[3] = "Can Slash";
			inf->flag[4] = "Temporary Light";
			inf->wpn[0] = "Stab Sprite:";
			inf->wpn[1] = "Slash Sprite:";
			inf->wpn[2] = "Flame Sprite:";
			inf->actionsnd = "Firing Sound:";
			inf->flag[7] = "Flip Right-Facing Slash";
			break;
		}
		case itype_arrow:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Duration (0 = Infinite):";
			inf->flag[0] = "Penetrate Enemies";
			inf->flag[1] = "Allow Item Pickup";
			inf->flag[3] = "Pick Up Anything";
			inf->wpn[0] = "Arrow Sprite:";
			inf->wpn[1] = "Sparkle Sprite:";
			inf->wpn[2] = "Damaging Sparkle Sprite:";
			inf->actionsnd = "Firing Sound:";
			inf->flag[6] = "Picks Up Keys";
			break;
		}
		case itype_brang:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Range (0 = Infinite):";
			inf->misc[2] = "Block Flags:";
			inf->misc[3] = "Reflect Flags:";
			inf->flag[0] = "Corrected Animation";
			inf->flag[1] = "Directional Sprites";
			inf->flag[2] = "Do Not Return";
			inf->flag[3] = "Pick Up Anything";
			inf->flag[4] = "Drags Items";
			inf->flag[5] = "Reflects Enemy Projectiles";
			inf->flag[6] = "Picks Up Keys";
			inf->flag[7] = "Triggers 'Fire(Any)'";
			inf->wpn[0] = "Boomerang Sprite:";
			inf->wpn[1] = "Sparkle Sprite:";
			inf->wpn[2] = "Damaging Sparkle Sprite:";
			inf->actionsnd = "Spinning Sound:";
			break;
		}
		case itype_sword:
		{
			inf->power = "Damage:";
			inf->misc[0] = "Beam Hearts:";
			inf->misc[1] = "Beam Damage:";
			inf->flag[0] = "Beam Hearts Is Percent";
			inf->flag[1] = "Beam Damage Is Percent";
			inf->flag[2] = "Beam Penetrates Enemies";
			inf->flag[3] = "Can Slash";
			inf->flag[4] = "No Melee Cooldown";
			inf->flag[5] = "Melee Weapons Use Magic Cost";
			inf->flag[6] = "Doesn't Pick Up Dropped Items";
			inf->flag[7] = "Flip Right-Facing Slash";
			inf->flag[8] = "Sword Beams Reflect off Prism/Mirror Combos";
			inf->flag[9] = "Walk slowly while charging";
			inf->wpn[0] = "Stab Sprite:";
			inf->wpn[1] = "Slash Sprite:";
			inf->wpn[2] = "Beam Sprite:";
			inf->actionsnd = "Slash/Stab Sound";
			break;
		}
		case itype_whimsicalring:
		{
			inf->power = "Damage Bonus:";
			inf->misc[0] = "Chance (1 in n):";
			inf->actionsnd = "Whimsy Sound:";
			break;
		}
		case itype_perilring:
		{
			inf->power = "Damage Divisor:";
			inf->misc[0] = "Maximum Hearts:";
			inf->flag[1] = "Divisor is Percentage Multiplier";
			break;
		}
		case itype_stompboots:
		{
			inf->power = "Damage:";
			break;
		}
		case itype_bow:
		{
			inf->power = "Arrow Speed:";
			break;
		}
		case itype_script1:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script2:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script3:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script4:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script5:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script6:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script7:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script8:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script9:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_script10:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_custom1:
		{
			break;
		}
		case itype_custom2:
		{
			break;
		}
		case itype_custom3:
		{
			break;
		}
		case itype_custom4:
		{
			break;
		}
		case itype_custom5:
		{
			break;
		}
		case itype_custom6:
		{
			break;
		}
		case itype_custom7:
		{
			break;
		}
		case itype_custom8:
		{
			break;
		}
		case itype_custom9:
		{
			break;
		}
		case itype_custom10:
		{
			break;
		}
		case itype_icerod:
		{
			inf->power = "W. Power:";
			inf->misc[0] = "Step Speed:";
			inf->flag[0] = "No Gfx Flip";
			break;
		}
		case itype_flippers:
		{
			inf->misc[0] = "Dive Length:";
			inf->misc[1] = "Dive Cooldown:";
			inf->flag[0] = "No Diving";
			inf->flag[1] = "Cancellable Diving";
			inf->flag[2] = "Can Swim in Lava";
			break;
		}
		case itype_raft:
		{
			inf->misc[0] = "Speed Modifier:";
			break;
		}
		case itype_atkring:
		{
			inf->misc[0] = "Bonus Damage";
			inf->misc[1] = "Damage Multiplier";
			break;
		}
		case itype_lantern:
		{
			inf->misc[0] = "Shape";
			inf->h_misc[0] = "What shape to use for the light area emitted.\n"
				"0 = circular, 1 = cone in front";
			inf->misc[1] = "Range";
			inf->h_misc[1] = "The range, in pixels, of the light.";
			break;
		}
		case itype_pearl:
		{
			break;
		}
		case itype_bottle:
		{
			inf->misc[0] = "Slot:";
			inf->h_misc[0] = "Which slot this bottle item is attached to. Valid vals 0-255.";
			break;
		}
		case itype_bottlefill:
		{
			inf->misc[0] = "Contents:";
			inf->h_misc[0] = "What contents to place in an empty bottle when picked up";
			break;
		}
		case itype_bugnet:
		{
			inf->flag[0] = "Can't catch fairies";
			inf->h_flag[0] = "If checked, no longer catches fairies it collides with";
			inf->flag[1] = "Right-handed";
			inf->h_flag[1] = "Swaps swing direction of the weapon";
			break;
		}
		case itype_mirror:
		{
			inf->misc[0] = "Warp Effect";
			inf->h_misc[0] = "What warp effect to use during the warp.\n"
				"0=None, 1=Zap, 2=Wave, 3=Blackscr, 4=OpenWipe";
			inf->flag[0] = "Place Return Portal";
			inf->h_flag[0] = "If checked, places a return portal when mirroring to a new dmap";
			inf->flag[1] = "Continue acts as F6->Continue";
			inf->h_flag[1] = "When used on a dmap with 'Mirror Continues instead of Warping' checked,"
				"activates F6->Continue instead of Farore's Wind effect if enabled.";
			inf->wpn[0] = "Portal Sprite";
			inf->h_wpn[0] = "Sprite of the Return Portal";
			break;
		}
	}
}
std::map<int32_t, ItemNameInfo *> *inamemap = NULL;

std::map<int32_t, ItemNameInfo *> *getItemNameMap()
{
	if(inamemap == NULL)
	{
		inamemap = new std::map<int32_t, ItemNameInfo *>();
		
		for(int32_t i=0; i < itype_max; i++)
		{
			ItemNameInfo *inf = &inameinf[i];
			
			loadinfo(inf, i);
				
			(*inamemap)[inf->iclass] = inf;
		}
	}
	
	return inamemap;
}

ItemEditorDialog::ItemEditorDialog(itemdata const& ref, char const* str, int32_t index):
	local_itemref(ref), itemname(str), index(index),
	list_items(GUI::ListData::itemclass(true)),
	list_counters(GUI::ListData::counters()),
	list_sprites(GUI::ListData::miscsprites()),
	list_itemdatscript(GUI::ListData::itemdata_script()),
	list_itemsprscript(GUI::ListData::itemsprite_script()),
	list_weaponscript(GUI::ListData::lweapon_script()),
	list_weaptype(GUI::ListData::lweaptypes()),
	list_deftypes(GUI::ListData::deftypes())
{}

ItemEditorDialog::ItemEditorDialog(int32_t index):
	ItemEditorDialog(itemsbuf[index], item_string[index], index)
{}

//{ Macros

#define DISABLE_WEAP_DATA true
#define ATTR_WID 6_em
#define ATTR_LAB_WID 12_em
#define SPR_LAB_WID sized(14_em,10_em)
#define ACTION_LAB_WID 6_em
#define ACTION_FIELD_WID 6_em
#define FLAGS_WID 20_em

#define NUM_FIELD(member,_min,_max,wid) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, width = wid, \
	low = _min, high = _max, val = local_itemref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_itemref.member = val; \
	})

#define ATTRIB_FIELD(member, index) \
l_attribs[index] = Label(textAlign = 2, width = ATTR_LAB_WID), \
ib_attribs[index] = Button(forceFitH = true, text = "?", \
	disabled = true, \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Attribute Info",h_attribs[index]).show(); \
	}), \
TextField(maxLength = 11, \
	type = GUI::TextField::type::INT_DECIMAL, width = ATTR_WID, \
	val = local_itemref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_itemref.member = val; \
	})

#define FLAG_CHECK(index, bit) \
Row(padding = 0_px, \
	ib_flags[index] = Button(forceFitH = true, text = "?", \
		disabled = true, \
		onPressFunc = [&]() \
		{ \
			InfoDialog("Flags Info",h_flags[index]).show(); \
		}), \
	l_flags[index] = Checkbox( \
		width = FLAGS_WID, \
		checked = (local_itemref.flags & bit), \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(local_itemref.flags,bit,state); \
		} \
	) \
)
#define FLAG_CHECK_NOINFO(index, bit) \
l_flags[index] = Checkbox( \
	width = FLAGS_WID, \
	checked = (local_itemref.flags & bit), \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_itemref.flags,bit,state); \
	} \
) \

#define SPRITE_DROP(ind, mem) \
Row(vPadding = 0_px, \
	l_spr[ind] = Label(textAlign = 2, width = SPR_LAB_WID, topMargin = 1_px), \
	ib_spr[ind] = Button(forceFitH = true, text = "?", \
		disabled = true, \
		onPressFunc = [&]() \
		{ \
			InfoDialog("Sprite Info",h_spr[ind]).show(); \
		}), \
	DropDownList( \
		maxwidth = sized(18_em, 14_em), \
		data = list_sprites, \
		selectedValue = local_itemref.mem, \
		onSelectFunc = [&](int32_t val) \
		{ \
			local_itemref.mem = val; \
		} \
	) \
)

//}

static size_t itmtabs[4] = {0};
std::shared_ptr<GUI::Widget> ItemEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	char titlebuf[256];
	sprintf(titlebuf, "Item Editor (%d): %s", index, itemname.c_str());
	if(is_large)
	{
		window = Window(
			use_vsync = true,
			title = titlebuf,
			onEnter = message::OK,
			onClose = message::CANCEL,
			Column(
				Row(
					Rows<2>(padding = 0_px,
						Label(text = "Name:"),
						TextField(
							fitParent = true,
							maxLength = 63,
							text = itemname,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
							{
								itemname = str;
								char buf[256];
								sprintf(buf, "Item Editor (%d): %s", index, itemname.c_str());
								window->setTitle(buf);
							}
						),
						Label(text = "Type:"),
						Row(
							height = sized(16_px, 21_px),
							DropDownList(data = list_items,
								fitParent = true, padding = 0_px,
								selectedValue = local_itemref.family,
								onSelectionChanged = message::ITEMCLASS
							),
							Button(width = 1.5_em, padding = 0_px, text = "?", hAlign = 1.0, onPressFunc = [&]()
							{
								InfoDialog(moduledata.item_editor_type_names[local_itemref.family],
									moduledata.itemclass_help_strings[local_itemref.family]
								).show();
							})
						)
					),
					Column(vAlign = 0.0, hAlign = 0.0, padding = 0_px,
						Checkbox(
							hAlign = 0.0,
							checked = (local_itemref.flags & ITEM_GAMEDATA),
							text = "Equipment Item",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_itemref.flags,ITEM_GAMEDATA,state);
							}
						)
					)
				),
				TabPanel(
					ptr = &itmtabs[0],
					TabRef(name = "Data", TabPanel(
						ptr = &itmtabs[1],
						TabRef(name = "Attrib", Column(
							Rows<6>(framed = true,
								Label(width=ATTR_LAB_WID,textAlign=2,text="Level:"),
								Button(forceFitH = true, text = "?",
									onPressFunc = [&]()
									{
										InfoDialog("Level Info","Most passive items only take effect if they are the highest level you own.\n"
											"By default, subscreens use the highest level item of an itemclass for displaying.").show();
									}),
								NUM_FIELD(fam_type, 1, 255, ATTR_WID),
								l_power = Label(width=ATTR_LAB_WID,textAlign=2),
								ib_power = Button(forceFitH = true, text = "?",
									disabled = true,
									onPressFunc = [&]()
									{
										InfoDialog("Power Info",h_power).show();
									}),
								NUM_FIELD(power, 0, 255, ATTR_WID)
							),
							Rows<6>(framed = true,
								ATTRIB_FIELD(misc1,0),
								ATTRIB_FIELD(misc6,5),
								
								ATTRIB_FIELD(misc2,1),
								ATTRIB_FIELD(misc7,6),
								
								ATTRIB_FIELD(misc3,2),
								ATTRIB_FIELD(misc8,7),
								
								ATTRIB_FIELD(misc4,3),
								ATTRIB_FIELD(misc9,8),
								
								ATTRIB_FIELD(misc5,4),
								ATTRIB_FIELD(misc10,9)
							)
						)),
						TabRef(name = "Flags", Column(padding = 0_px,
							Rows<2>(
								framed = true,
								frameText = "General Flags",
								topPadding = DEFAULT_PADDING+0.4_em,
								bottomPadding = DEFAULT_PADDING+1_px,
								bottomMargin = 1_em,
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & ITEM_EDIBLE),
									text = "Can Be Eaten By Enemies",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_EDIBLE,state);
									}
								),
								FLAG_CHECK_NOINFO(15,ITEM_PASSIVESCRIPT),
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & ITEM_SIDESWIM_DISABLED),
									text = "Disabled In Sideview Water",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_SIDESWIM_DISABLED,state);
									}
								),
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & ITEM_BUNNY_ENABLED),
									text = "Usable as a Bunny",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_BUNNY_ENABLED,state);
									}
								)
							),
							Columns<8>(
								framed = true,
								frameText = "Variable Flags",
								topPadding = DEFAULT_PADDING+0.4_em,
								bottomPadding = DEFAULT_PADDING+1_px,
								FLAG_CHECK(0,ITEM_FLAG1),
								FLAG_CHECK(1,ITEM_FLAG2),
								FLAG_CHECK(2,ITEM_FLAG3),
								FLAG_CHECK(3,ITEM_FLAG4),
								FLAG_CHECK(4,ITEM_FLAG5),
								FLAG_CHECK(5,ITEM_FLAG6),
								FLAG_CHECK(6,ITEM_FLAG7),
								FLAG_CHECK(7,ITEM_FLAG8),
								FLAG_CHECK(8,ITEM_FLAG9),
								FLAG_CHECK(9,ITEM_FLAG10),
								FLAG_CHECK(10,ITEM_FLAG11),
								FLAG_CHECK(11,ITEM_FLAG12),
								FLAG_CHECK(12,ITEM_FLAG13),
								FLAG_CHECK(13,ITEM_FLAG14),
								FLAG_CHECK(14,ITEM_FLAG15)
							)
						)),
						TabRef(name = "Action", Columns<4>(
							Row(
								Label(text = "Cost:", textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.magic,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.magic = val;
									}
								)
							),
							DropDownList(
								hAlign = 1.0,
								data = list_counters,
								selectedValue = local_itemref.cost_counter,
								onSelectFunc = [&](int32_t val)
								{
									local_itemref.cost_counter = val;
								}
							),
							Row(
								Label(text = "Timer:", textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.magiccosttimer,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.magiccosttimer = val;
									}
								)
							),
							Row(
								l_sfx = Label(textAlign = 2, width = ACTION_LAB_WID),
								ib_sfx = Button(forceFitH = true, text = "?",
									disabled = true,
									onPressFunc = [&]()
									{
										InfoDialog("SFX Info",h_sfx).show();
									}),
								TextField(
									val = local_itemref.usesound,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.usesound = val;
									}
								)
							),
							Checkbox(
								hAlign = 0.0,
								checked = (local_itemref.flags & ITEM_DOWNGRADE),
								text = "Remove Item When Used",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,ITEM_DOWNGRADE,state);
								}
							),
							Checkbox(
								hAlign = 0.0,
								checked = (local_itemref.flags & ITEM_VALIDATEONLY),
								text = "Only Validate Cost",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,ITEM_VALIDATEONLY,state);
								}
							)
						)),
						TabRef(name = "Pickup", Column(
							Rows<4>(
								//
								Label(text = "Counter:", hAlign = 1.0),
								DropDownList(
									fitParent = true,
									data = list_counters,
									selectedValue = local_itemref.count,
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.count = val;
									}
								),_d,_d,
								//
								Label(text = "Increase By:", hAlign = 1.0),
								TextField(
									val = ((local_itemref.amount & 0x4000) ? -1 : 1)*(local_itemref.amount & 0x3FFF),
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.amount &= 0x8000;
										local_itemref.amount |= ((val&0x3FFF)|(val<0?0x4000:0));
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.amount & 0x8000),
									text = "Gradual",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.amount,0x8000,state);
									}
								),_d,
								//
								Label(text = "Increase Max:", hAlign = 1.0),
								TextField(
									val = local_itemref.setmax,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, low = -32768, high = 32767,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.setmax = val;
									}
								),
								Label(text = "But Not Above:", hAlign = 1.0),
								TextField(
									val = local_itemref.max,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.max = val;
									}
								),
								//
								Label(text = "Sound:", hAlign = 1.0),
								TextField(
									val = local_itemref.playsound,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.playsound = val;
									}
								),_d,_d,
								//
								Label(text = "Hearts Required:", hAlign = 1.0),
								TextField(
									val = local_itemref.pickup_hearts,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.pickup_hearts = val;
									}
								),_d,_d
							),
							Column(
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & ITEM_KEEPOLD),
									text = "Keep Lower Level Items",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_KEEPOLD,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & ITEM_GAINOLD),
									text = "Gain All Lower Level Items",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_GAINOLD,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & ITEM_COMBINE),
									text = "Upgrade When Collected Twice",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_COMBINE,state);
									}
								)
							),
							Row(
								Label(text = "String:"),
								TextField(
									val = local_itemref.pstring,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.pstring = val;
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup_string_flags & itemdataPSTRING_ALWAYS),
									text = "Always",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup_string_flags,itemdataPSTRING_ALWAYS,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup_string_flags & itemdataPSTRING_IP_HOLDUP),
									text = "Only Held",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup_string_flags,itemdataPSTRING_IP_HOLDUP,state);
									}
								)
							)
						)),
						TabRef(name = "P. Flags", Column(
							Row(
								Label(text = "Flag Behavior:"),
								DropDownList(
									maxwidth = 10_em,
									data = PFlagTypeList,
									selectedValue = local_itemref.pickupflag,
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.pickupflag = val;
									}
								)
							),
							Columns<8>(
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<0)),
									text = "Large Collision Rectangle (INTERNAL)",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<0),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<1)),
									text = "Hold Up Item",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<1),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<2)),
									text = "Sets Screen State ST_ITEM",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<2),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<3)),
									text = "Dummy Item",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<3),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<4)),
									text = "Shop Item (INTERNAL)",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<4),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<5)),
									text = "Pay for Info (INTERNAL)",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<5),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<6)),
									text = "Item Fades",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<6),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<7)),
									text = "Enemy Carries Item",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<7),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<8)),
									text = "Item Disappears",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<8),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<9)),
									text = "Big Triforce (INTERNAL)",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<9),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<10)),
									text = "Invisible",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<10),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<11)),
									text = "Triggers Screen State ST_SP_ITEM",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<11),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<12)),
									text = "Triggers Screen Secrets",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<12),state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<13)),
									text = "Always Grabbable",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup,(1<<13),state);
									}
								),
								Checkbox(
									disabled = true,
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<14)),
									text = "--",
									onToggleFunc = [&](bool state)
									{
										//SETFLAG(local_itemref.pickup,(1<<14),state);
									}
								),
								Checkbox(
									disabled = true,
									hAlign = 0.0,
									checked = (local_itemref.pickup & (1<<15)),
									text = "--",
									onToggleFunc = [&](bool state)
									{
										//SETFLAG(local_itemref.pickup,(1<<15),state);
									}
								)
							)
						)),
						TabRef(name = "Weapon Data", Row(
							Row(
								vAlign = 0.0,
								Rows<4>(
									Label(hAlign = 1.0, text = "Weapon Type:"),
									DropDownList(
										fitParent = true,
										data = list_weaptype,
										selectedValue = local_itemref.useweapon,
										onSelectFunc = [&](int32_t val)
										{
											local_itemref.useweapon = val;
										}
									), _d, _d,
									Label(hAlign = 1.0, text = "Default Defense:"),
									DropDownList(
										fitParent = true,
										data = list_deftypes,
										selectedValue = local_itemref.usedefence,
										onSelectFunc = [&](int32_t val)
										{
											local_itemref.usedefence = val;
										}
									), _d, _d,
									Label(hAlign = 1.0, text = "Movement Pattern:"),
									DropDownList(
										disabled = DISABLE_WEAP_DATA,
										fitParent = true,
										data = WeapMoveTypeList,
										selectedValue = local_itemref.weap_pattern[0],
										onSelectFunc = [&](int32_t val)
										{
											local_itemref.weap_pattern[0] = val;
										}
									), _d, _d,
									Label(hAlign = 1.0, text = "Movement Arg 1:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[1],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[1] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Range:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weaprange,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weaprange = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[2],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[2] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Duration:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weapduration,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weapduration = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 3:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[3],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[3] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 1:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[5],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[5] = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 4:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[4],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[4] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[6],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[6] = val;
										}
									)
								)
							)
						))
					)),
					TabRef(name = "Graphics", TabPanel(
						ptr = &itmtabs[2],
						TabRef(name = "GFX", Row(
							Rows<3>(
								Label(text = "Flash CSet:", hAlign = 1.0),
								TextField(
									val = (local_itemref.csets>>4), rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 16,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.csets &= 0x0F;
										local_itemref.csets |= val<<4;
									}
								),
								DummyWidget(),
								Label(text = "Animation Frames:", hAlign = 1.0),
								TextField(
									val = local_itemref.frames, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.frames = val;
										animFrame->setFrames(val);
									}
								),
								DummyWidget(),
								Label(text = "Animation Speed:", hAlign = 1.0),
								TextField(
									val = local_itemref.speed, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.speed = val;
										animFrame->setSpeed(val);
									}
								),
								DummyWidget(),
								Label(text = "Initial Delay:", hAlign = 1.0),
								TextField(
									val = local_itemref.delay, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.delay = val;
										animFrame->setDelay(val);
									}
								),
								DummyWidget(),
								Label(text = "Player Tile Modifier:", hAlign = 1.0),
								TextField(
									val = local_itemref.ltm, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = (0-(NEWMAXTILES-1)), high = (NEWMAXTILES-1),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.ltm = val;
									}
								),
								Button(
									width = 1.5_em, leftPadding = 0_px, forceFitH = true,
									text = "?", hAlign = 1.0, onPressFunc = [&]()
									{
										InfoDialog("Tile Modifiers",
											"The tile modifiers of the highest level item owned from each itemclass"
											" are added up, and the total is added to the Player's tile.\n"
											"If the player is a Bunny, then only items which have the 'Usable as a Bunny'"
											" flag checked will have their tile modifiers count, but the 'Bunny Tile Mod'"
											" found in Init Data is also added."
										).show();
									}
								)
							),
							Column(
								SelTileSwatch(
									tile = local_itemref.tile,
									cset = (local_itemref.csets & 0x0F),
									onSelectFunc = [&](int32_t t, int32_t c)
									{
										local_itemref.tile = t;
										local_itemref.csets &= 0xF0;
										local_itemref.csets |= c&0x0F;
										animFrame->setTile(t);
										animFrame->setCSet(c);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.misc & 1),
									text = "Flash",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.misc,1,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.misc & 2),
									text = "2-Hand",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.misc,2,state);
									}
								),
								animFrame = TileFrame(
									hAlign = 0.0,
									tile = local_itemref.tile,
									cset = (local_itemref.csets & 0xF),
									frames = local_itemref.frames,
									speed = local_itemref.speed,
									delay = local_itemref.delay
								)
							)
						)),
						TabRef(name = "Sprites", Columns<5>(
							SPRITE_DROP(0,wpn),
							SPRITE_DROP(1,wpn2),
							SPRITE_DROP(2,wpn3),
							SPRITE_DROP(3,wpn4),
							SPRITE_DROP(4,wpn5),
							SPRITE_DROP(5,wpn6),
							SPRITE_DROP(6,wpn7),
							SPRITE_DROP(7,wpn8),
							SPRITE_DROP(8,wpn9),
							SPRITE_DROP(9,wpn10)
						)),
						TabRef(name = "Size", Row(
							Columns<5>(
								vAlign = 0.0,
								Row(
									Label(textAlign = 2, width = 6_em, text = "TileWidth:"),
									TextField(
										val = local_itemref.tilew,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.tilew = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_TILEWIDTH),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_TILEWIDTH,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "TileHeight:"),
									TextField(
										val = local_itemref.tileh,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.tileh = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_TILEHEIGHT),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_TILEHEIGHT,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitXOffset:"),
									TextField(
										val = local_itemref.hxofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.hxofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_X_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_X_OFFSET,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitYOffset:"),
									TextField(
										val = local_itemref.hyofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.hyofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_Y_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_Y_OFFSET,state);
										}
									)
								),
								_d,
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitWidth:"),
									TextField(
										val = local_itemref.hxsz,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.hxsz = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_WIDTH),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_WIDTH,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitHeight:"),
									TextField(
										val = local_itemref.hysz,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.hysz = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_HEIGHT),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_HEIGHT,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitZHeight:"),
									TextField(
										val = local_itemref.hzsz,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.hzsz = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_Z_HEIGHT),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_Z_HEIGHT,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "DrawXOffset:"),
									TextField(
										val = local_itemref.xofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.xofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_DRAW_X_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_DRAW_X_OFFSET,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "DrawYOffset:"),
									TextField(
										val = local_itemref.yofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.yofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_DRAW_Y_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_DRAW_Y_OFFSET,state);
										}
									)
								)
							)
						)),
						TabRef(name = "Weapon Size", Row(
							Columns<5>(
								vAlign = 0.0,
								Row(
									Label(textAlign = 2, width = 6_em, text = "TileWidth:"),
									TextField(
										val = local_itemref.weap_tilew,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_tilew = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_TILEWIDTH),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_TILEWIDTH,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "TileHeight:"),
									TextField(
										val = local_itemref.weap_tileh,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_tileh = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_TILEHEIGHT),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_TILEHEIGHT,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitXOffset:"),
									TextField(
										val = local_itemref.weap_hxofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_hxofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_X_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_X_OFFSET,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitYOffset:"),
									TextField(
										val = local_itemref.weap_hyofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_hyofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_Y_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_Y_OFFSET,state);
										}
									)
								),
								_d,
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitWidth:"),
									TextField(
										val = local_itemref.weap_hxsz,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_hxsz = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_WIDTH),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_WIDTH,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitHeight:"),
									TextField(
										val = local_itemref.weap_hysz,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_hysz = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_HEIGHT),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_HEIGHT,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "HitZHeight:"),
									TextField(
										val = local_itemref.weap_hzsz,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_hzsz = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_Z_HEIGHT),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_Z_HEIGHT,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "DrawXOffset:"),
									TextField(
										val = local_itemref.weap_xofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_xofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_DRAW_X_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_DRAW_X_OFFSET,state);
										}
									)
								),
								Row(
									Label(textAlign = 2, width = 6_em, text = "DrawYOffset:"),
									TextField(
										val = local_itemref.weap_yofs,
										type = GUI::TextField::type::INT_DECIMAL,
										width = ACTION_FIELD_WID, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_yofs = val;
										}
									),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_DRAW_Y_OFFSET),
										text = "Enabled",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_DRAW_Y_OFFSET,state);
										}
									)
								)
							)
						))
					)),
					TabRef(name = "Scripts", TabPanel(
						ptr = &itmtabs[3],
						TabRef(name = "Item", Row(
							Column(
								INITD_ROW(0, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(1, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(2, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(3, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(4, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(5, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(6, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(7, local_itemref.initiald, local_itemref.initD_label)
							),
							Column(
								padding = 0_px, fitParent = true,
								Rows<2>(vAlign = 0.0,
									SCRIPT_LIST("Action Script:", list_itemdatscript, local_itemref.script),
									SCRIPT_LIST("Pickup Script:", list_itemdatscript, local_itemref.collect_script),
									SCRIPT_LIST("Sprite Script:", list_itemsprscript, local_itemref.sprite_script)
								),
								Rows<2>(hAlign = 1.0,
									Label(text = "A1:"),
									TextField(
										val = local_itemref.initiala[0],
										type = GUI::TextField::type::INT_DECIMAL,
										high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.initiala[0] = val;
										}
									),
									Label(text = "A2:"),
									TextField(
										val = local_itemref.initiala[1],
										type = GUI::TextField::type::INT_DECIMAL,
										high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.initiala[1] = val;
										}
									)
								)
							)
						)),
						TabRef(name = "Weapon", Row(
							Column(
								INITD_ROW(0, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(1, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(2, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(3, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(4, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(5, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(6, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(7, local_itemref.weap_initiald, local_itemref.weapon_initD_label)
							),
							Rows<2>(vAlign = 0.0,
								SCRIPT_LIST("Weapon Script:", list_weaponscript, local_itemref.weaponscript)
							)
						))
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
						onClick = message::CANCEL),
					Button(
						text = "Default",
						minwidth = 90_lpx,
						onClick = message::DEFAULT)
				)
			)
		);
	}
	else
	{
		window = Window(
			use_vsync = true,
			title = titlebuf,
			onEnter = message::OK,
			onClose = message::CANCEL,
			Column(
				TabPanel(
					ptr = &itmtabs[0],
					TabRef(name = "Basic", Column(
						Rows<2>(padding = 0_px,
							Label(text = "Name:"),
							TextField(
								fitParent = true,
								maxLength = 63,
								text = itemname,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
								{
									itemname = str;
									char buf[256];
									sprintf(buf, "Item Editor (%d): %s", index, itemname.c_str());
									window->setTitle(buf);
								}
							),
							Label(text = "Type:"),
							Row(
								height = sized(16_px, 21_px),
								DropDownList(data = list_items,
									fitParent = true, padding = 0_px,
									selectedValue = local_itemref.family,
									onSelectionChanged = message::ITEMCLASS
								),
								Button(width = 1.5_em, padding = 0_px, text = "?", hAlign = 1.0, onPressFunc = [&]()
								{
									InfoDialog(moduledata.item_editor_type_names[local_itemref.family],
										moduledata.itemclass_help_strings[local_itemref.family]
									).show();
								})
							)
						),
						Column(vAlign = 0.0, hAlign = 0.0, padding = 0_px,
							Checkbox(
								hAlign = 0.0,
								checked = (local_itemref.flags & ITEM_GAMEDATA),
								text = "Equipment Item",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,ITEM_GAMEDATA,state);
								}
							)
						)
					)),
					TabRef(name = "Data", TabPanel(
						ptr = &itmtabs[1],
						TabRef(name = "Attrib", ScrollingPane(
							Rows<3>(
								Label(width=ATTR_LAB_WID,textAlign=2,text="Level:"),
								Button(forceFitH = true, text = "?",
									onPressFunc = [&]()
									{
										InfoDialog("Level Info","Most passive items only take effect if they are the highest level you own.\n"
											"By default, subscreens use the highest level item of an itemclass for displaying.").show();
									}),
								NUM_FIELD(fam_type, 1, 255, ATTR_WID),
								
								l_power = Label(width=ATTR_LAB_WID,textAlign=2),
								ib_power = Button(forceFitH = true, text = "?",
									disabled = true,
									onPressFunc = [&]()
									{
										InfoDialog("Attribute Info",h_power).show();
									}),
								NUM_FIELD(power, 0, 255, ATTR_WID),
								
								ATTRIB_FIELD(misc1,0),
								ATTRIB_FIELD(misc2,1),
								ATTRIB_FIELD(misc3,2),
								ATTRIB_FIELD(misc4,3),
								ATTRIB_FIELD(misc5,4),
								ATTRIB_FIELD(misc6,5),
								ATTRIB_FIELD(misc7,6),
								ATTRIB_FIELD(misc8,7),
								ATTRIB_FIELD(misc9,8),
								ATTRIB_FIELD(misc10,9)
							)
						)),
						TabRef(name = "Flags", ScrollingPane(
							Column(
								topMargin = 6_px,
								Column(
									framed = true,
									frameText = "General Flags",
									topPadding = DEFAULT_PADDING+0.4_em,
									bottomPadding = DEFAULT_PADDING+1_px,
									bottomMargin = 1_em,
									Checkbox(
										width = FLAGS_WID,
										checked = (local_itemref.flags & ITEM_EDIBLE),
										text = "Can Be Eaten By Enemies",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.flags,ITEM_EDIBLE,state);
										}
									),
									FLAG_CHECK_NOINFO(15,ITEM_PASSIVESCRIPT),
									Checkbox(
										width = FLAGS_WID,
										checked = (local_itemref.flags & ITEM_SIDESWIM_DISABLED),
										text = "Disabled In Sideview Water",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.flags,ITEM_SIDESWIM_DISABLED,state);
										}
									),
									Checkbox(
										width = FLAGS_WID,
										checked = (local_itemref.flags & ITEM_BUNNY_ENABLED),
										text = "Usable as a Bunny",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.flags,ITEM_BUNNY_ENABLED,state);
										}
									)
								),
								Column(
									framed = true,
									frameText = "Variable Flags",
									topPadding = DEFAULT_PADDING+0.4_em,
									bottomPadding = DEFAULT_PADDING+1_px,
									FLAG_CHECK(0,ITEM_FLAG1),
									FLAG_CHECK(1,ITEM_FLAG2),
									FLAG_CHECK(2,ITEM_FLAG3),
									FLAG_CHECK(3,ITEM_FLAG4),
									FLAG_CHECK(4,ITEM_FLAG5),
									FLAG_CHECK(5,ITEM_FLAG6),
									FLAG_CHECK(6,ITEM_FLAG7),
									FLAG_CHECK(7,ITEM_FLAG8),
									FLAG_CHECK(8,ITEM_FLAG9),
									FLAG_CHECK(9,ITEM_FLAG10),
									FLAG_CHECK(10,ITEM_FLAG11),
									FLAG_CHECK(11,ITEM_FLAG12),
									FLAG_CHECK(12,ITEM_FLAG13),
									FLAG_CHECK(13,ITEM_FLAG14),
									FLAG_CHECK(14,ITEM_FLAG15)
								)
							)
						)),
						TabRef(name = "Action", Columns<4>(
							Row(
								Label(text = "Cost:", textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.magic,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.magic = val;
									}
								)
							),
							DropDownList(
								hAlign = 1.0,
								data = list_counters,
								selectedValue = local_itemref.cost_counter,
								onSelectFunc = [&](int32_t val)
								{
									local_itemref.cost_counter = val;
								}
							),
							Row(
								Label(text = "Timer:", textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.magiccosttimer,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.magiccosttimer = val;
									}
								)
							),
							Row(
								l_sfx = Label(textAlign = 2, width = ACTION_LAB_WID),
								ib_sfx = Button(forceFitH = true, text = "?",
									disabled = true,
									onPressFunc = [&]()
									{
										InfoDialog("Attribute Info",h_sfx).show();
									}),
								TextField(
									val = local_itemref.usesound,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.usesound = val;
									}
								)
							),
							Checkbox(
								hAlign = 0.0,
								checked = (local_itemref.flags & ITEM_DOWNGRADE),
								text = "Remove Item When Used",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,ITEM_DOWNGRADE,state);
								}
							),
							Checkbox(
								hAlign = 0.0,
								checked = (local_itemref.flags & ITEM_VALIDATEONLY),
								text = "Only Validate Cost",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,ITEM_VALIDATEONLY,state);
								}
							)
						)),
						TabRef(name = "Pickup", Column(
							Rows<4>(
								//
								Label(text = "Counter:", hAlign = 1.0),
								DropDownList(
									fitParent = true,
									data = list_counters,
									selectedValue = local_itemref.count,
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.count = val;
									}
								),_d,_d,
								//
								Label(text = "Increase By:", hAlign = 1.0),
								TextField(
									val = ((local_itemref.amount & 0x4000) ? -1 : 1)*(local_itemref.amount & 0x3FFF),
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.amount &= 0x8000;
										local_itemref.amount |= ((val&0x3FFF)|(val<0?0x4000:0));
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.amount & 0x8000),
									text = "Gradual",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.amount,0x8000,state);
									}
								),_d,
								//
								Label(text = "Increase Max:", hAlign = 1.0),
								TextField(
									val = local_itemref.setmax,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, low = -32768, high = 32767,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.setmax = val;
									}
								),
								Label(text = "But Not Above:", hAlign = 1.0),
								TextField(
									val = local_itemref.max,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.max = val;
									}
								),
								//
								Label(text = "Sound:", hAlign = 1.0),
								TextField(
									val = local_itemref.playsound,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.playsound = val;
									}
								),_d,_d,
								//
								Label(text = "Hearts Required:", hAlign = 1.0),
								TextField(
									val = local_itemref.pickup_hearts,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.pickup_hearts = val;
									}
								),_d,_d
							),
							Column(
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & ITEM_KEEPOLD),
									text = "Keep Lower Level Items",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_KEEPOLD,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & ITEM_GAINOLD),
									text = "Gain All Lower Level Items",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_GAINOLD,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & ITEM_COMBINE),
									text = "Upgrade When Collected Twice",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_COMBINE,state);
									}
								)
							),
							Row(
								Label(text = "String:"),
								TextField(
									val = local_itemref.pstring,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.pstring = val;
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup_string_flags & itemdataPSTRING_ALWAYS),
									text = "Always",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup_string_flags,itemdataPSTRING_ALWAYS,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.pickup_string_flags & itemdataPSTRING_IP_HOLDUP),
									text = "Only Held",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.pickup_string_flags,itemdataPSTRING_IP_HOLDUP,state);
									}
								)
							)
						)),
						TabRef(name = "P. Flags", Column(
							Row(
								Label(text = "Flag Behavior:"),
								DropDownList(
									maxwidth = 10_em,
									data = PFlagTypeList,
									selectedValue = local_itemref.pickupflag,
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.pickupflag = val;
									}
								)
							),
							TabPanel(padding = 0_px,
								TabRef(name = "1",
									Column(
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<0)),
											text = "Large Collision Rectangle (INTERNAL)",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<0),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<1)),
											text = "Hold Up Item",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<1),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<2)),
											text = "Sets Screen State ST_ITEM",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<2),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<3)),
											text = "Dummy Item",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<3),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<4)),
											text = "Shop Item (INTERNAL)",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<4),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<5)),
											text = "Pay for Info (INTERNAL)",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<5),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<6)),
											text = "Item Fades",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<6),state);
											}
										)
									)
								),
								TabRef(name = "2",
									Column(
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<7)),
											text = "Enemy Carries Item",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<7),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<8)),
											text = "Item Disappears",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<8),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<9)),
											text = "Big Triforce (INTERNAL)",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<9),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<10)),
											text = "Invisible",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<10),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<11)),
											text = "Triggers Screen State ST_SP_ITEM",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<11),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<12)),
											text = "Triggers Screen Secrets",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<12),state);
											}
										),
										Checkbox(
											hAlign = 0.0,
											checked = (local_itemref.pickup & (1<<13)),
											text = "Always Grabbable",
											onToggleFunc = [&](bool state)
											{
												SETFLAG(local_itemref.pickup,(1<<13),state);
											}
										)
									)
								)
							)
						)),
						TabRef(name = "Weapon Data", Row(
							Column(
								vAlign = 0.0,
								Rows<2>(
									Label(hAlign = 1.0, text = "Weapon Type:"),
									DropDownList(
										fitParent = true,
										data = list_weaptype,
										selectedValue = local_itemref.useweapon,
										onSelectFunc = [&](int32_t val)
										{
											local_itemref.useweapon = val;
										}
									),
									Label(hAlign = 1.0, text = "Default Defense:"),
									DropDownList(
										fitParent = true,
										data = list_deftypes,
										selectedValue = local_itemref.usedefence,
										onSelectFunc = [&](int32_t val)
										{
											local_itemref.usedefence = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Pattern:"),
									DropDownList(
										disabled = DISABLE_WEAP_DATA,
										fitParent = true,
										data = WeapMoveTypeList,
										selectedValue = local_itemref.weap_pattern[0],
										onSelectFunc = [&](int32_t val)
										{
											local_itemref.weap_pattern[0] = val;
										}
									)
								),
								Rows<4>(
									Label(hAlign = 1.0, text = "Movement Arg 1:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[1],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[1] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Range:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weaprange,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weaprange = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[2],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[2] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Duration:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weapduration,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weapduration = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 3:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[3],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[3] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 1:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[5],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[5] = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 4:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[4],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[4] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[6],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.weap_pattern[6] = val;
										}
									)
								)
							)
						))
					)),
					TabRef(name = "Graphics", TabPanel(
						ptr = &itmtabs[2],
						TabRef(name = "GFX", Row(
							Rows<3>(
								Label(text = "Flash CSet:", hAlign = 1.0),
								TextField(
									val = (local_itemref.csets>>4), rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 16,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.csets &= 0x0F;
										local_itemref.csets |= val<<4;
									}
								),
								DummyWidget(),
								Label(text = "Animation Frames:", hAlign = 1.0),
								TextField(
									val = local_itemref.frames, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.frames = val;
										animFrame->setFrames(val);
									}
								),
								DummyWidget(),
								Label(text = "Animation Speed:", hAlign = 1.0),
								TextField(
									val = local_itemref.speed, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.speed = val;
										animFrame->setSpeed(val);
									}
								),
								DummyWidget(),
								Label(text = "Initial Delay:", hAlign = 1.0),
								TextField(
									val = local_itemref.delay, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.delay = val;
										animFrame->setDelay(val);
									}
								),
								DummyWidget(),
								Label(text = "Player Tile Modifier:", hAlign = 1.0),
								TextField(
									val = local_itemref.ltm, rightPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = (0-(NEWMAXTILES-1)), high = (NEWMAXTILES-1),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.ltm = val;
									}
								),
								Button(
									width = 1.5_em, leftPadding = 0_px, forceFitH = true,
									text = "?", hAlign = 1.0, onPressFunc = [&]()
									{
										InfoDialog("Tile Modifiers",
											"The tile modifiers of the highest level item owned from each itemclass"
											" are added up, and the total is added to the Player's tile.\n"
											"If the player is a Bunny, then only items which have the 'Usable as a Bunny'"
											" flag checked will have their tile modifiers count, but the 'Bunny Tile Mod'"
											" found in Init Data is also added."
										).show();
									}
								)
							),
							Column(
								SelTileSwatch(
									tile = local_itemref.tile,
									cset = (local_itemref.csets & 0x0F),
									onSelectFunc = [&](int32_t t, int32_t c)
									{
										local_itemref.tile = t;
										local_itemref.csets &= 0xF0;
										local_itemref.csets |= c&0x0F;
										animFrame->setTile(t);
										animFrame->setCSet(c);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.misc & 1),
									text = "Flash",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.misc,1,state);
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.misc & 2),
									text = "2-Hand",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.misc,2,state);
									}
								),
								animFrame = TileFrame(
									hAlign = 0.0,
									tile = local_itemref.tile,
									cset = (local_itemref.csets & 0xF),
									frames = local_itemref.frames,
									speed = local_itemref.speed,
									delay = local_itemref.delay
								)
							)
						)),
						TabRef(name = "Sprites", 
							TabPanel(
								TabRef(name = "1", Column(
									SPRITE_DROP(0,wpn),
									SPRITE_DROP(1,wpn2),
									SPRITE_DROP(2,wpn3),
									SPRITE_DROP(3,wpn4),
									SPRITE_DROP(4,wpn5)
								)),
								TabRef(name = "2", Column(
									SPRITE_DROP(5,wpn6),
									SPRITE_DROP(6,wpn7),
									SPRITE_DROP(7,wpn8),
									SPRITE_DROP(8,wpn9),
									SPRITE_DROP(9,wpn10)
								))
							)
						),
						TabRef(name = "Size", Row(
							Rows<2>(
								vAlign = 0.0,
								Label(textAlign = 2, text = "TileWidth:"),
								TextField(
									val = local_itemref.tilew,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 32,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.tilew = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_TILEWIDTH),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_TILEWIDTH,state);
									}
								),
								Label(textAlign = 2, text = "TileHeight:"),
								TextField(
									val = local_itemref.tileh,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 32,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.tileh = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_TILEHEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_TILEHEIGHT,state);
									}
								),
								Label(textAlign = 2, text = "HitXOffset:"),
								TextField(
									val = local_itemref.hxofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.hxofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_X_OFFSET,state);
									}
								),
								Label(textAlign = 2, text = "HitYOffset:"),
								TextField(
									val = local_itemref.hyofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.hyofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_Y_OFFSET,state);
									}
								)
							),
							Rows<2>(
								vAlign = 0.0,
								Label(textAlign = 2, text = "HitWidth:"),
								TextField(
									val = local_itemref.hxsz,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.hxsz = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_WIDTH),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_WIDTH,state);
									}
								),
								Label(textAlign = 2, text = "HitHeight:"),
								TextField(
									val = local_itemref.hysz,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.hysz = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_HEIGHT,state);
									}
								),
								Label(textAlign = 2, text = "HitZHeight:"),
								TextField(
									val = local_itemref.hzsz,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.hzsz = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_HIT_Z_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_HIT_Z_HEIGHT,state);
									}
								),
								Label(textAlign = 2, text = "DrawXOffset:"),
								TextField(
									val = local_itemref.xofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.xofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_DRAW_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_DRAW_X_OFFSET,state);
									}
								),
								Label(textAlign = 2, text = "DrawYOffset:"),
								TextField(
									val = local_itemref.yofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.yofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & itemdataOVERRIDE_DRAW_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,itemdataOVERRIDE_DRAW_Y_OFFSET,state);
									}
								)
							)
						)),
						TabRef(name = "Weapon Size", Row(
							Rows<2>(
								vAlign = 0.0,
								Label(textAlign = 2, text = "TileWidth:"),
								TextField(
									val = local_itemref.weap_tilew,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 32,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_tilew = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_TILEWIDTH),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_TILEWIDTH,state);
									}
								),
								Label(textAlign = 2, text = "TileHeight:"),
								TextField(
									val = local_itemref.weap_tileh,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 32,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_tileh = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_TILEHEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_TILEHEIGHT,state);
									}
								),
								Label(textAlign = 2, text = "HitXOffset:"),
								TextField(
									val = local_itemref.weap_hxofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_hxofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_X_OFFSET,state);
									}
								),
								Label(textAlign = 2, text = "HitYOffset:"),
								TextField(
									val = local_itemref.weap_hyofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_hyofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_Y_OFFSET,state);
									}
								)
							),
							Rows<2>(
								vAlign = 0.0,
								Label(textAlign = 2, text = "HitWidth:"),
								TextField(
									val = local_itemref.weap_hxsz,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_hxsz = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_WIDTH),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_WIDTH,state);
									}
								),
								Label(textAlign = 2, text = "HitHeight:"),
								TextField(
									val = local_itemref.weap_hysz,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_hysz = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_HEIGHT,state);
									}
								),
								Label(textAlign = 2, text = "HitZHeight:"),
								TextField(
									val = local_itemref.weap_hzsz,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_hzsz = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_HIT_Z_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_HIT_Z_HEIGHT,state);
									}
								),
								Label(textAlign = 2, text = "DrawXOffset:"),
								TextField(
									val = local_itemref.weap_xofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_xofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_DRAW_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_DRAW_X_OFFSET,state);
									}
								),
								Label(textAlign = 2, text = "DrawYOffset:"),
								TextField(
									val = local_itemref.weap_yofs,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -214748, high = 214748,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_yofs = val;
									}
								),
								_d,Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & itemdataOVERRIDE_DRAW_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,itemdataOVERRIDE_DRAW_Y_OFFSET,state);
									}
								)
							)
						))
					)),
					TabRef(name = "Scripts", TabPanel(
						ptr = &itmtabs[3],
						TabRef(name = "Item", Row(
							Column(
								INITD_ROW(0, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(1, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(2, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(3, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(4, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(5, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(6, local_itemref.initiald, local_itemref.initD_label),
								INITD_ROW(7, local_itemref.initiald, local_itemref.initD_label)
							),
							Column(
								padding = 0_px, fitParent = true,
								Rows<2>(vAlign = 0.0,
									SCRIPT_LIST("Action Script:", list_itemdatscript, local_itemref.script),
									SCRIPT_LIST("Pickup Script:", list_itemdatscript, local_itemref.collect_script),
									SCRIPT_LIST("Sprite Script:", list_itemsprscript, local_itemref.sprite_script)
								),
								Rows<2>(hAlign = 1.0,
									Label(text = "A1:"),
									TextField(
										val = local_itemref.initiala[0],
										type = GUI::TextField::type::INT_DECIMAL,
										high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.initiala[0] = val;
										}
									),
									Label(text = "A2:"),
									TextField(
										val = local_itemref.initiala[1],
										type = GUI::TextField::type::INT_DECIMAL,
										high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_itemref.initiala[1] = val;
										}
									)
								)
							)
						)),
						TabRef(name = "Weapon", Row(
							Column(
								INITD_ROW(0, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(1, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(2, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(3, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(4, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(5, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(6, local_itemref.weap_initiald, local_itemref.weapon_initD_label),
								INITD_ROW(7, local_itemref.weap_initiald, local_itemref.weapon_initD_label)
							),
							Rows<2>(vAlign = 0.0,
								SCRIPT_LIST("Weapon Script:", list_weaponscript, local_itemref.weaponscript)
							)
						))
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
						onClick = message::CANCEL),
					Button(
						text = "Default",
						minwidth = 90_lpx,
						onClick = message::DEFAULT)
				)
			)
		);
	}
	loadItemClass();
	return window;
}

void ItemEditorDialog::loadItemClass()
{
	std::map<int32_t, ItemNameInfo *> *nmap = getItemNameMap();
	std::map<int32_t, ItemNameInfo *>::iterator it = nmap->find(local_itemref.family);
	ItemNameInfo *inf = NULL;
	
	if(it != nmap->end())
		inf = it->second;
	
	#define __SET(obj, mem) \
	l_##obj->setText(inf ? (inf->mem.size() ? inf->mem : defInfo.mem) : defInfo.mem); \
	h_##obj = inf ? (inf->h_##mem.size() ? inf->h_##mem : "") : ""; \
	if(ib_##obj) \
		ib_##obj->setDisabled(h_##obj.empty());
	
	__SET(power, power);
	
	__SET(attribs[0], misc[0]);
	__SET(attribs[1], misc[1]);
	__SET(attribs[2], misc[2]);
	__SET(attribs[3], misc[3]);
	__SET(attribs[4], misc[4]);
	__SET(attribs[5], misc[5]);
	__SET(attribs[6], misc[6]);
	__SET(attribs[7], misc[7]);
	__SET(attribs[8], misc[8]);
	__SET(attribs[9], misc[9]);
	
	__SET(flags[0], flag[0]);
	__SET(flags[1], flag[1]);
	__SET(flags[2], flag[2]);
	__SET(flags[3], flag[3]);
	__SET(flags[4], flag[4]);
	__SET(flags[5], flag[5]);
	__SET(flags[6], flag[6]);
	__SET(flags[7], flag[7]);
	__SET(flags[8], flag[8]);
	__SET(flags[9], flag[9]);
	__SET(flags[10], flag[10]);
	__SET(flags[11], flag[11]);
	__SET(flags[12], flag[12]);
	__SET(flags[13], flag[13]);
	__SET(flags[14], flag[14]);
	__SET(flags[15], flag[15]);
	
	__SET(sfx, actionsnd);
	
	__SET(spr[0], wpn[0]);
	__SET(spr[1], wpn[1]);
	__SET(spr[2], wpn[2]);
	__SET(spr[3], wpn[3]);
	__SET(spr[4], wpn[4]);
	__SET(spr[5], wpn[5]);
	__SET(spr[6], wpn[6]);
	__SET(spr[7], wpn[7]);
	__SET(spr[8], wpn[8]);
	__SET(spr[9], wpn[9]);
}

bool ItemEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::ITEMCLASS:
		{
			local_itemref.family = int32_t(msg.argument);
			loadItemClass();
			return false;
		}

		case message::DEFAULT:
		{
			bool cancel = false;
			AlertDialog(
				"Reset itemdata?",
				"Reset this item to default?",
				[&](bool ret)
				{
					cancel = !ret;
				}).show();
			if(cancel) return false;
			_reset_default = true;
			reset_ref = local_itemref;
			reset_name = itemname;
			return true;
		}

		case message::OK:
			saved = false;
			itemsbuf[index] = local_itemref;
			strcpy(item_string[index], itemname.c_str());
			return true;

		case message::CANCEL:
		default:
			return true;
	}
}

