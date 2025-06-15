#include "itemeditor.h"
#include "itemwizard.h"
#include "info.h"
#include "alert.h"
#include "base/zsys.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "zinfo.h"
#include <fmt/format.h>

void reset_itembuf(itemdata *item, int32_t id);
char *ordinal(int32_t num);
extern zquestheader header;
extern bool saved;
extern char *item_string[];
extern itemdata *itemsbuf;
static bool _reset_default, _reload_editor;
static itemdata static_ref;
static std::string reset_name;
static int32_t item_use_script_data = 3;
extern script_data *itemscripts[NUMSCRIPTITEM];
extern script_data *itemspritescripts[NUMSCRIPTSITEMSPRITE];
extern script_data *lwpnscripts[NUMSCRIPTWEAPONS];
#define ISCRDATA_NONE    0x00
#define ISCRDATA_ACTION  0x01
#define ISCRDATA_PICKUP  0x02
#define ISCRDATA_SPRITE  0x04
#define ISCRDATA_ALL     0x07
void call_item_editor(int32_t index)
{
	if(unsigned(index) >= MAXITEMS) return;
	item_use_script_data = zc_get_config("zquest","show_itemscript_meta_type",ISCRDATA_ALL)&ISCRDATA_ALL;
	_reset_default = false;
	ItemEditorDialog(index).show();
	while(_reset_default || _reload_editor)
	{
		if(_reset_default)
		{
			_reset_default = false;
			reset_itembuf(&static_ref, index);
		}
		_reload_editor = false;
		ItemEditorDialog(static_ref, reset_name.c_str(), index).show();
	}
}

static const GUI::ListData ScriptDataList
{
	{ "None", ISCRDATA_NONE },
	{ "All", ISCRDATA_ALL },
	{ "Action", ISCRDATA_ACTION },
	{ "Pickup", ISCRDATA_PICKUP },
	{ "Sprite", ISCRDATA_SPRITE },
	{ "Action+Pickup", ISCRDATA_ACTION|ISCRDATA_PICKUP },
	{ "Action+Sprite", ISCRDATA_ACTION|ISCRDATA_SPRITE },
	{ "Pickup+Sprite", ISCRDATA_PICKUP|ISCRDATA_SPRITE }
};

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
	{ "UseSound", "UseSound2" }
};

void loadinfo(ItemNameInfo * inf, itemdata const& ref)
{
	inf->clear();
	inf->iclass = ref.family;
	#define _SET(mem, str, helpstr) \
	do{ \
		inf->mem = str; \
		inf->h_##mem = helpstr; \
	}while(false)
	#define FLAG(val) (ref.flags & item_flag##val)
	std::string classname(ZI.getItemClassName(ref.family));
	
	switch(ref.family)
	{
		case itype_fairy:
		{
			_SET(misc[2], "Step Speed:", "The movement speed of the fairy, in 100ths of pixel/frame");
			_SET(misc[3], "Bottle Fill:", "What bottle type to fill an empty bottle with if caught using a Bug Net");
			_SET(flag[0], "Life is Percent", "HP Regained is a percentage out of max HP");
			_SET(flag[1], "Magic is Percent", "MP Regained is a percentage out of max MP");
			if(FLAG(1))
				_SET(misc[0], "% HP Regained:", "Percentage of max life restored when collected.");
			else
				_SET(misc[0], "HP Regained:", "Life (in points) restored when collected.");
			if(FLAG(2))
				_SET(misc[1], "% MP Regained:", "Percentage of max magic restored when collected.");
			else
				_SET(misc[1], "MP Regained:", "Magic (in points) restored when collected.");
			_SET(actionsnd[0], "Item Drop Sound:", "Plays SFX when dropped");
			break;
		}
		case itype_triforcepiece:
		{
			_SET(misc[0], "Cutscene MIDI:", "If non-zero, overrides the cutscene midi");
			_SET(misc[1], "Cutscene Type (0-1):", "If >0, uses the 'big mcguffin' cutscene style");
			_SET(misc[2], "Second Collect Sound:", "A second sound to play on pickup, played only if flag 'Play Second SFX' is checked.");
			_SET(misc[3], "Custom Cutscene Duration", "If nonzero, overrides the duration of the cutscene, in frames");
			_SET(misc[4], "Custom Refill Frame", "If non-zero, changes the timing of the mcguffin's life refill.");
			_SET(flag[0], "Side Warp Out", "Warp out using sidewarp A upon completion");
			_SET(flag[2], "Removes Sword Jinxes", "Heal sword jinxes on pickup");
			_SET(flag[3], "Removes Item Jinxes", "Heal item jinxes on pickup");
			_SET(flag[4], "Removes Shield Jinxes", "Heal shield jinxes on pickup");
			_SET(flag[7], "Ownable", "Can be owned; this is normally handled by 'Equipment Item', but not for mcguffin piece items.");
			_SET(flag[8], "Don't Dismiss Messages", "If not checked, the cutscene clears screen strings");
			_SET(flag[9], "Cutscene Interrupts Action Script", "The action script, if running on collection, is paused for the duration of the cutscene.");
			_SET(flag[10], "Don't Affect Music", "If not checked, music is stopped for the cutscene");
			_SET(flag[11], "No Cutscene", "Skip the cutscene animation");
			_SET(flag[12], "Run Action Script on Collection", "Item action script runs when it is collected");
			_SET(flag[13], "Play Second SFX", "Plays the 'Second Collect Sound' SFX on pickup");
			_SET(flag[14], "Don't Play MIDI", "Don't play a cutscene midi if checked");
			_SET(actionsnd[0], "Collect Sound:", "First sound played on pickup");
			break;
		}
		case itype_shield:
		{
			_SET(misc[0], "Block Flags:", "(Rock=1, Arrow=2, BRang=4, Fireball=8, Sword=16, Magic=32, Flame=64, Script(All)=128, Fireball2=256, Lightbeam=512,\n"
				"Script1=1024, Script2=2048, Script3=4096, Script4=8192, Script5=16384, Script6=32768, Script7=65536, Script8=131072, Script9=262144, Script10=524288)\n"
				"Sum all of the values you want to apply. Weapons and lightbeams with their flags set will be blocked from in front of the Hero.");
			_SET(misc[1], "Reflect Flags:", "(Rock=1, Fireball=8, Sword=16, Magic=32, Flame=64, Script(All)=128, Fireball2=256, Lightbeam=512,\n"
				"Script1=1024, Script2=2048, Script3=4096, Script4=8192, Script5=16384, Script6=32768, Script7=65536, Script8=131072, Script9=262144, Script10=524288)\n"
				"Sum all of the values you want to apply. Weapons and lightbeams with their flags set will be reflected from in front of the Hero.\n"
				"Weapons are only reflected if their value is in both 'Block Flags' and 'Reflect Flags'.");
			_SET(actionsnd[0], "De/Reflection Sound:", "Plays when the shield successfully blocks or reflects a weapon");
			
			_SET(flag[0], "Protects Front", "The shield will protect the front side of the Hero");
			_SET(flag[1], "Protects Back", "The shield will protect the back side of the Hero");
			_SET(flag[2], "Protects Left", "The shield will protect the left side of the Hero");
			_SET(flag[3], "Protects Right", "The shield will protect the right side of the Hero");
			_SET(flag[8], "Active Use", "If enabled, the shield only protects the Hero (and provides its' LTM)"
				" while it is equipped to a button that is being held.");
			if(FLAG(9))
			{
				_SET(actionsnd[1], "Activation Sound:", "Plays when the shield button is pressed");
				_SET(flag[4], "Inactive Front", "Protects the front when button is NOT held");
				_SET(flag[5], "Inactive Back", "Protects the back when button is NOT held");
				_SET(flag[6], "Inactive Left", "Protects the left when button is NOT held");
				_SET(flag[7], "Inactive Right", "Protects the right when button is NOT held");
				_SET(misc[5], "Inactive PTM", "Hero Tile Modifier to use while shield is inactive");
				_SET(flag[9], "Change Speed", "Change the Hero's walking speed while the shield is active");
				_SET(flag[10], "Lock Direction", "When the shield is activated, lock the Hero's direction until"
					" it is released.");
				if(FLAG(10))
				{
					_SET(misc[6], "Speed Percentage", "A percentage multiplier for the Hero's movement speed."
						" A negative value will give that amount *more* speed; i.e. '-100' is the same as '200'.");
					_SET(misc[7], "Speed Bonus", "A step value (in 100ths of a pixel per frame) to be added to the"
						" Hero's speed.");
				}
			}
			break;
		}
		case itype_agony:
		{
			_SET(power, "Sensitivity:", "The radius in which flags are checked for");
			_SET(misc[0], "Vibration Frequency", "0-58, higher = faster vibrations");
			break;
		}
		case itype_wealthmedal:
		{
			_SET(misc[0], "Discount Amount:", "If 'Discount is Percent' is checked, this is the percentage of the price that will still be charged.\n"
				"else, this many rupees are ADDED to the price (use negative to subtract)");
			_SET(flag[0], "Discount Is Percent", "Makes the discount percentage-based");
			break;
		}
		case itype_cbyrna: //!TODO Help Text
		{
			inf->power = "Beam Damage:";
			inf->misc[0] = "Beam Speed Divisor:";
			inf->misc[1] = "Orbit Radius:";
			inf->misc[2] = "Number Of Beams:";
			inf->misc[3] = "Melee Damage:";
			inf->misc[9] = "Slash Sound:";
			inf->flag[0] = "Penetrates Enemies";
			inf->flag[1] = "Invincible Hero";
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
			inf->actionsnd[0] = "Orbit Sound:";
			break;
		}
		case itype_whispring:
		{
			_SET(power, "Jinx Divisor:", "Amount to divide jinx duration by.\n"
				"If 'Perm. Jinx Are Temp' is checked, perm jinxes last for 150/[divisor] frames."
				" If divisor is 0, Hero is immune to affected jinxes.\n"
				"When picked up, regardless of flags, perm jinxes matching the Jinx Type become 150-frame temp jinxes.");
			_SET(misc[0], "Jinx Type:", "Sum all of the values you want to apply. 1 = sword, 2 = item, 4 = shield");
			_SET(flag[0], "Perm. Jinx Are Temp.", "Perm jinxes inflicted instead are temp for 150/[divisor] frames");
			break;
		}
		case itype_ladder:
		{
			_SET(power, "Four-Way:", "If >1, the Hero can exit the ladder in all 4 directions.\n"
				"If <=1, the Hero can only exit parallel to the direction they entered from.");
			_SET(flag[0], "Can Ladder Over Pitfalls", "Can be used to cross Pitfall combos");
			break;
		}
		case itype_magickey:
		{
			_SET(power, "Dungeon Level:", "What dungeon level to grant infinite keys for");
			_SET(flag[0], "Lesser D. Levels Also", "If checked, lower levels are also affected");
			break;
		}
		case itype_wallet:
		{
			_SET(power, "Infinite Supply:", "If >0, grants infinite money");
			_SET(misc[0], "Increase Amount:", "How much money regenerate over time from having this");
			_SET(misc[1], "Delay Duration:", "How often money regenerate from having this, in frames");
			break;
		}
		case itype_quiver:
		{
			_SET(power, "Infinite Supply:", "If >0, grants infinite arrows");
			_SET(misc[0], "Increase Amount:", "How many arrows regenerate over time from having this");
			_SET(misc[1], "Delay Duration:", "How often arrows regenerate from having this, in frames");
			break;
		}
		case itype_bombbag:
		{
			_SET(power, "Infinite Supply:", "If >0, grants infinite bombs");
			_SET(misc[0], "Increase Amount:", "How many bombs regenerate over time from having this");
			_SET(misc[1], "Delay Duration:", "How often bombs regenerate from having this, in frames");
			_SET(flag[0], "Super Bombs Also", "Also regenerates S. Bombs");
			break;
		}
		case itype_quakescroll:
		case itype_quakescroll2:
		{
			_SET(power, "Damage Multiplier:", "Multiplier of the hammer's damage to deal");
			_SET(misc[0], "Stun Duration:", "Duration, in frames, to stun enemies for.");
			_SET(misc[1], "Stun Radius:", "Radius, in pixels, to stun enemies in.");
			_SET(actionsnd[0], "Quake Sound:", "Sound played when the hammer is swung");
			_SET(actionsnd[1], "Charge Sound:", "Sound played when the hammer is charged");
			_SET(flag[0], "Pay on swing", "The use cost will be consumed when the charge is"
				" unleashed, instead of when it is charged up.");
			break;
		}
		case itype_perilscroll: //!TODO Help Text
		{
			inf->misc[0] = "Maximum Hearts:";
			break;
		}
		case itype_spinscroll:
		case itype_spinscroll2:
		{
			_SET(power, "Damage Multiplier:", "Multiplier of the sword's damage to deal");;
			_SET(misc[0], "Number of Spins:", "Number of times to spin the sword");
			_SET(actionsnd[0], "Spinning Sound:", "The sound to play while spinning");
			_SET(actionsnd[1], "Charge Sound:", "The sound to play when charged");
			_SET(flag[0], "Pay on swing", "The use cost will be consumed when the charge is"
				" unleashed, instead of when it is charged up.");
			break;
		}
		case itype_clock:
		{
			_SET(misc[0], "Duration:", "How long the invincibility lasts, in frames. 0 = infinite.");
			_SET(flag[0], "Active-Use", "If enabled, activates on use as an 'Equipment Item' instead"
				" of on pickup.");
			_SET(flag[1], "Can't trigger while active", "If enabled, will not activate while a clock"
				" is already active.");
			_SET(actionsnd[0], "Activation Sound", "SFX to play when the effect activates");
			break;
		}
		case itype_killem:
		{
			_SET(flag[0], "Active-Use", "If enabled, activates on use as an 'Equipment Item' instead"
				" of on pickup.");
			_SET(actionsnd[0], "Activation Sound", "SFX to play when the effect activates");
			break;
		}
		case itype_refill:
		{
			for(auto q = 0; q < 5; ++q)
			{
				std::string numstr = std::to_string(q + 1);
				std::string ordstr = ordinal(q + 1);
				_SET(misc[q], "Counter " + numstr, "The " + ordstr + " counter that will be refilled when activating the item."
					"\nIf <0, acts as 'no counter'.");
				_SET(misc[q+5], "Amount " + numstr, "How much the " + ordstr + " counter will be refilled by.");
			}
			_SET(flag[0], "Cures Sword Jinx", "When activated, cures sword jinx effects");
			_SET(flag[1], "Gradual Refill", "Counters refill gradually instead of immediately.");
			_SET(actionsnd[0], "Activation Sound", "SFX to play when the item is used");
			break;
		}
		case itype_liftglove:
		{
			_SET(misc[0], "Button", "If 0, the item must be equipped to a button to use it.\n"
				"Otherwise, any of the specified buttons will activate the glove, even when not equipped to a button.\n"
				"Sum all the buttons you want to be usable:\n(A=1, B=2, L=4, R=8, Ex1=16, Ex2=32, Ex3=64, Ex4=128)");
			_SET(misc[1], "Throw Step", "The speed, in 1/100ths pixel per frame, of the thrown object");
			_SET(misc[2], "Throw Jump", "The jump value, in 1/100ths pixel per frame, of the thrown object");
			_SET(actionsnd[0], "Lift Sound", "SFX to play when an object is lifted,"
				"\nand does not have its' own lift sound.");
			_SET(actionsnd[1], "Throw Sound", "SFX to play when an object is thrown");
			_SET(flag[0], "Fake Z Throw", "Throws weapons in the fakez axis instead of the z axis");
			_SET(flag[1], "Lift In Water", "Allows lifting while swimming");
			_SET(flag[2], "Disable Shield", "Makes the shield not protect from projectiles, in the same way as when the Hero is attacking");
			_SET(flag[3], "Disable Item Use", "Stop the use of items unrelated to lifting while an object is lifted");
			_SET(flag[4], "Drop When Hit", "Held object is dropped when receiving damage");
			break;
		}
		case itype_magicring:
		{
			_SET(power, "Infinite Magic:", "If >0, grants infinite magic");
			_SET(misc[0], "Increase Amount:", "How much magic regenerate over time from having this");
			_SET(misc[1], "Delay Duration:", "How often magic regenerate from having this, in frames");
			break;
		}
		case itype_heartring:
		{
			_SET(misc[0], "Increase Amount:", "How much life regenerate over time from having this");
			_SET(misc[1], "Delay Duration:", "How often life regenerate from having this, in frames");
			break;
		}
		case itype_chargering: //!TODO Help Text
		{
			inf->misc[0] = "Charging Duration:";
			inf->misc[1] = "Magic C. Duration:";
			break;
		}
		case itype_hoverboots: //!TODO Help Text
		{
			inf->misc[0] = "Hover Duration:";
			inf->flag[0] = "Timer only resets on landing";
			inf->wpn[0] = "Halo Sprite:";
			inf->actionsnd[0] = "Hovering Sound:";
			break;
		}
		case itype_rocs:
		{
			_SET(misc[0], "Extra Jumps:", "The number of times this item can be used in mid-air"
				" without landing.");
			_SET(misc[1], "Button", "If 0, the item must be equipped to a button to use it.\n"
				"Otherwise, any of the specified buttons will jump, even when not equipped to a button.\n"
				"Sum all the buttons you want to be usable:\n(A=1, B=2, L=4, R=8, Ex1=16, Ex2=32, Ex3=64, Ex4=128)");
			if(FLAG(1))
				_SET(power, "Jump Power:", "The Hero will jump with a force of 'power'");
			else _SET(power, "Jump Power:", "The Hero will jump with a force of '(power*80)+160)'");
			if(FLAG(2))
				_SET(misc[2], "Held Gravity", "Gravity applied to Link when this item is held. Used for variable jumps"
					" such as holding the button to jump higher. Value is divided by 100 (314 would be equal to 3.14).");
			if(FLAG(5))
				_SET(misc[3], "Held Terminal Velocity", "Max fall speed of Link when this item is held. Useful for floating down while holding jump.");
			_SET(misc[4], "Coyote Time", "Number of frames after leaving a ledge via non-jump that you can still jump");
			_SET(flag[0], "Jump is Power/100", "If enabled, the Hero jumps with a force"
				" of 'power' instead of '(power*80)+160'");
			_SET(flag[1], "Hold to change Hero gravity", "If enabled, holding the button this item is attached to will change"
				" the Hero's gravity to the value specified in attributes[2]");
			_SET(flag[4], "Hold to change Hero terminal velocity", "If enabled, holding the button this item is attached to will change"
				" the Hero's terminal velocity (max fall speed) to the value specified in attributes[3]");
			if(FLAG(2))
				_SET(flag[2], "Held Gravity doesn't affect downward momentum", "If enabled, 'Hold to change Hero gravity' will"
					" not affect the Hero when falling.");
			if(FLAG(2))
				_SET(flag[3], "Held Gravity doesn't affect upward momentum", "If enabled, 'Hold to change Hero gravity' will"
					" not affect the Hero when rising.");
			inf->actionsnd[0] = "Jumping Sound:";
			break;
		}
		case itype_sbomb: //!TODO Help Text
		{
			inf->power = "Damage:";
			inf->misc[0] = "Fuse Duration (0 = Remote):";
			inf->misc[1] = "Max. On Screen:";
			inf->misc[2] = "Damage to Hero:";
			_SET(misc[3], "Lift Level", "If 0, the weapon is not liftable. Otherwise, liftable using Lift Gloves of at least this level.");
			_SET(misc[4], "Lift Time", "The time, in frames, to lift the weapon above the Hero's head.");
			_SET(misc[5], "Lift Height", "The Z height above the Hero's head to lift the weapon.");
			_SET(misc[6], "Boom Radius", "If 0, uses a classic boxy hitbox- otherwise uses a circular radius of this many pixels." + QRHINT({qr_OLD_BOMB_HITBOXES}));
			inf->flag[1] = "Explosion Hurts Hero";
			_SET(flag[2], "Stops Movement on Landing", "If the weapon lands due to gravity, its step will be set to 0.");
			_SET(flag[3], "Auto-Lift", "If the Hero owns a Lift Glove, place the bomb directly in the Hero's hands.");
			inf->wpn[0] = "Bomb Sprite:";
			inf->wpn[1] = "Explosion Sprite:";
			inf->actionsnd[0] = "Explosion Sound:";
			break;
		}
		case itype_bomb: //!TODO Help Text
		{
			inf->power = "Damage:";
			inf->misc[0] = "Fuse Duration (0 = Remote):";
			inf->misc[1] = "Max. On Screen:";
			inf->misc[2] = "Damage to Hero:";
			_SET(misc[3], "Lift Level", "If 0, the weapon is not liftable. Otherwise, liftable using Lift Gloves of at least this level.");
			_SET(misc[4], "Lift Time", "The time, in frames, to lift the weapon above the Hero's head.");
			_SET(misc[5], "Lift Height", "The Z height above the Hero's head to lift the weapon.");
			_SET(misc[6], "Boom Radius", "If 0, uses a classic boxy hitbox- otherwise uses a circular radius of this many pixels." + QRHINT({qr_OLD_BOMB_HITBOXES}));
			inf->flag[0] = "Use 1.92 Timing";
			inf->flag[1] = "Explosion Hurts Hero";
			_SET(flag[2], "Stops Movement on Landing", "If the weapon lands due to gravity, its step will be set to 0.");
			_SET(flag[3], "Auto-Lift", "If the Hero owns a Lift Glove, place the bomb directly in the Hero's hands.");
			_SET(flag[4], "Stops Movement on Solid", "If the weapon collides with a solid while moving, its step will be set to 0.");
			inf->wpn[0] = "Bomb Sprite:";
			inf->wpn[1] = "Explosion Sprite:";
			inf->actionsnd[0] = "Explosion Sound:";
			break;
		}
		case itype_divineprotection: //!TODO Help Text
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
			inf->actionsnd[0] = "Shield Sound:";
			break;
		}
		case itype_divineescape:
		{
			_SET(misc[0], "Warp Animation (0-2):",
				"0 = Scatter into dust particles"
				"\n1 = Rise into the sky as color 0x00 particles"
				"\n2 = Rise into the sky as a mix of color 0x12 and 0x01 particles");
			_SET(actionsnd[0], "Warp Sound:", "This sound plays when the Hero turns into particles");
			_SET(flag[0], "Continue acts as F6->Continue",
				"Activates F6->Continue instead of just 'restarting the level'.");
			break;
		}
		case itype_divinefire:
		{
			_SET(power, "Damage:", "The amount of damage dealt by the flames");
			_SET(misc[0], "Number of Flames:", "The number of flames to shoot in a circle");
			_SET(misc[1], "Circle Width:", "The diameter of the circle");
			_SET(flag[1], "Don't Provide Light", "The flames will not emit light");
			_SET(flag[2], "Falls in Sideview", "The flames will obey gravity");
			_SET(flag[4], "Temporary Light", "The flames light will only light the room temporarily (Old dark rooms)");
			_SET(flag[8], "Counts as Strong Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfSTRONGFIRE)));
			_SET(flag[9], "Counts as Magic Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfMAGICFIRE)));
			_SET(flag[10], "Counts as Divine Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfDIVINEFIRE)));
			_SET(wpn[0], "Rocket Up Sprite:", "The sprite for the up rocket");
			_SET(wpn[1], "Rocket Down Sprite:", "The sprite for the down rocket");
			_SET(wpn[2], "R. Up Sparkle Sprite:", "The sparkle sprite for the up rocket");
			_SET(wpn[3], "R. Down Sparkle Sprite:", "The sparkle sprite for the down rocket");
			_SET(wpn[4], "Flame Sprite:", "The sprite of the flame weapons");
			_SET(actionsnd[0], "Ring Sound:", "The sound that plays when the ring of fire appears");
			break;
		}
		case itype_hammer: //!TODO Help Text
		{
			inf->power = "Damage:";
			inf->flag[2] = "No Melee Attack";
			_SET(flag[0], "No Air Triggers", "Don't trigger combos/secret flags while the hammer is 'in the air'.");
			inf->wpn[0] = "Hammer Sprite:";
			inf->wpn[1] = "Smack Sprite:";
			inf->actionsnd[0] = "Pound Sound:";
			inf->flag[6] = "Doesn't Pick Up Dropped Items";
			break;
		}
		case itype_lens: //!TODO Help Text
		{
			inf->misc[0] = "Lens Width:";
			inf->flag[0] = "Show Hints";
			_SET(flag[1], "Hide Secret Combos", "Does not reveal 'Secret Combos' if checked");
			_SET(flag[2], "No X-Ray for Items", "Will not show items hidden under armos/dive flags/inside chests");
			inf->flag[3] = "Show Raft Paths";
			inf->flag[4] = "Show Invis. Enemies";
			_SET(flag[5], "Triggers Lens Trigflag", "Interacts with the 'Lens On->' and 'Lens Off->' combo trigger flags");
			inf->actionsnd[0] = "Activation Sound:";
			break;
		}
		case itype_hookshot:
		{
			inf->power = "Damage:";
			_SET(misc[0], "Chain Length:", "Max length, in tiles, of the hookshot chain");
			_SET(misc[1], "Chain Links:", "Max number of chain link sprites to display (less = a more stretched chain)");
			_SET(flag[0], "No Handle Damage", "The handle does not collide with enemies");
			_SET(flag[1], "Allow Diagonal", "The hookshot can be fired diagonally. (EXPERIMENTAL)");
			_SET(flag[3], "Pick Up Anything", "Any collided item will be grabbed, not just dropped items");
			_SET(flag[4], "Drags Items", "Collected items are dragged towards the Hero");
			_SET(flag[5], "Hits Enemy Projectiles", "If checked, the weapon can collide with projectiles, either blocking or reflecting them.");
			_SET(flag[6], "Picks Up Keys", "Will pick up Key type items");
			
			inf->wpn[0] = "Tip Sprite:";
			inf->wpn[1] = "Chain Sprite (H):";
			inf->wpn[2] = "Chain Sprite (V):";
			inf->wpn[3] = "Handle Sprite:";
			inf->wpn[4] = "Diagonal Tip Sprite:";
			inf->wpn[5] = "Diagonal Handle Sprite:";
			inf->wpn[6] = "Diagonal Chain Sprite:";
			_SET(actionsnd[0], "Firing Sound:", "Sound plays while the hookshot extends");
			if(FLAG(6))
			{
				_SET(misc[2], "Block Flags:",
					"(Rock=1, Arrow=2, BRang=4, Fireball=8, Sword=16, Magic=32, Flame=64, Script(All)=128, Firebal2=256,\n"
					"Script1=1024, Script2=2048, Script3=4096, Script4=8192, Script5=16384, Script6=32768, Script7=65536, Script8=131072, Script9=262144, Script10=524288)\n"
					"Sum all of the values you want to apply. Weapons with their flags set will be blocked by the weapon.");
				_SET(misc[3], "Reflect Flags:",
					"(Rock=1, Fireball=8, Sword=16, Magic=32, Flame=64, Script(All)=128, Firebal2=256,\n"
					"Script1=1024, Script2=2048, Script3=4096, Script4=8192, Script5=16384, Script6=32768, Script7=65536, Script8=131072, Script9=262144, Script10=524288)\n"
					"Sum all of the values you want to apply. Weapons with their flags set will be reflected by the weapon.\n"
					"Weapons are only reflected if their value is in both 'Block Flags' and 'Reflect Flags'.");
			}
			break;
		}
		case itype_switchhook:
		{
			inf->power = "Damage:";
			_SET(misc[0], "Chain Length:", "Max length, in tiles, of the switchhook chain");
			_SET(misc[1], "Chain Links:", "Max number of chain link sprites to display (less = a more stretched chain)");
			_SET(misc[4], "Switch FX Type:", "The visual effect to use when switching."
				"\n0 = Poof, 1 = Flicker, 2 = Rise/Fall");
			_SET(flag[0], "No Handle Damage", "The handle does not collide with enemies");
			_SET(flag[1], "Allow Diagonal", "The switchhook can be fired diagonally. (EXPERIMENTAL)");
			_SET(flag[3], "Pick Up Anything", "Any collided item will be grabbed, not just dropped items");
			_SET(flag[5], "Hits Enemy Projectiles", "If checked, the weapon can collide with projectiles, either blocking or reflecting them.");
			_SET(flag[6], "Picks Up Keys", "Will pick up Key type items");
			_SET(flag[7], "Cost on Swap", "The cost of the item is only paid upon swapping successfully.");
			_SET(flag[8], "Swaps Items", "Items that would be collected are instead swapped with the Hero.");
			if(!FLAG(9))
				_SET(flag[4], "Drags Items", "Collected items are dragged towards the Hero");
			
			inf->wpn[0] = "Tip Sprite:";
			inf->wpn[1] = "Chain Sprite (H):";
			inf->wpn[2] = "Chain Sprite (V):";
			inf->wpn[3] = "Handle Sprite:";
			inf->wpn[4] = "Diagonal Tip Sprite:";
			inf->wpn[5] = "Diagonal Handle Sprite:";
			inf->wpn[6] = "Diagonal Chain Sprite:";
			_SET(actionsnd[0], "Firing Sound:", "Sound plays while the hookshot extends");
			_SET(actionsnd[1], "Switch Sound:", "Sound plays when a switch occurs");
			if(FLAG(6))
			{
				_SET(misc[2], "Block Flags:",
					"(Rock=1, Arrow=2, BRang=4, Fireball=8, Sword=16, Magic=32, Flame=64, Script(All)=128, Firebal2=256,\n"
					"Script1=1024, Script2=2048, Script3=4096, Script4=8192, Script5=16384, Script6=32768, Script7=65536, Script8=131072, Script9=262144, Script10=524288)\n"
					"Sum all of the values you want to apply. Weapons with their flags set will be blocked by the weapon.");
				_SET(misc[3], "Reflect Flags:",
					"(Rock=1, Fireball=8, Sword=16, Magic=32, Flame=64, Script(All)=128, Firebal2=256,\n"
					"Script1=1024, Script2=2048, Script3=4096, Script4=8192, Script5=16384, Script6=32768, Script7=65536, Script8=131072, Script9=262144, Script10=524288)\n"
					"Sum all of the values you want to apply. Weapons with their flags set will be reflected by the weapon.\n"
					"Weapons are only reflected if their value is in both 'Block Flags' and 'Reflect Flags'.");
			}
			break;
		}
		case itype_itmbundle:
		{
			for(auto q = 0; q < 10; ++q)
				_SET(misc[q], "Item " + std::to_string(q+1) + ":", "If > -1, an item ID to 'collect' when you collect this item bundle.");
			_SET(flag[0], "Run Pickup Scripts", "Run the collect script of bundled items.");
			break;
		}
		case itype_progressive_itm:
		{
			for(auto q = 0; q < 10; ++q)
				_SET(misc[q], "Item " + std::to_string(q+1) + ":", "If > -1, an item ID in the progressive order.");
			break;
		}
		case itype_note:
		{
			_SET(misc[0], "String:", "The string to display when this item is used.");
			_SET(actionsnd[0], "Opening Sound", "SFX to play when the note is opened");
			break;
		}
		case itype_boots:
		{
			_SET(power, "Protection Power:", "Protects against damage combos that deal up to 16*power points of damage.");
			_SET(flag[0], "Not Solid Combos", "Does not protect against solid damage combos");
			_SET(flag[1], "Heavy", "Some combo settings interact with this flag.\n"
				"Unlike most items, this applies as long as you have a heavy " + classname + " item,"
				" even if it is not the highest level " + classname + " item.");
			_SET(flag[2], "No Damage Protection", "These " + classname + " will not protect you"
				" from Damage Combos at all.");
			break;
		}
		case itype_bracelet:
		{
			_SET(power, "Push Combo Level:", "If 1 or higher, can push 'Heavy' blocks. If 2 or higher, can push 'Very Heavy' blocks.");
			_SET(flag[0], "Limited Per Screen", "Can only use a set number of times per screen");
			if(FLAG(1))
			{
				_SET(misc[2], "Uses Per Screen", "How many times this bracelet can be used per screen. Minimum 1.");
			}
			break;
		}
		case itype_book: //!TODO Help Text
		{
			inf->power = "M. Damage:";
			inf->wpn[0] = "Magic Sprite:";
			inf->wpn[1] = "Projectile Sprite:";
			
			inf->flag[0] = "Fire Magic";
			if(FLAG(1))
			{
				inf->misc[3] = "Fire Damage";
				inf->flag[2] = "Fire Doesn't Hurt Hero";
				inf->flag[4] = "Temporary Light";
				inf->actionsnd[0] = "Fire Sound:";
				_SET(flag[8], "Counts as Strong Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfSTRONGFIRE)));
				_SET(flag[9], "Counts as Magic Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfMAGICFIRE)));
				_SET(flag[10], "Counts as Divine Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfDIVINEFIRE)));
			}
			
			inf->flag[1] = "Override Wand Damage";
			if(FLAG(2))
				inf->misc[0] = "W. Damage:";
			
			inf->flag[3] = "Override Wand SFX";
			if(FLAG(4))
				inf->misc[1] = "Wand Sound";
			
			inf->flag[5] = "Replace Magic Step";
			if(FLAG(6))
				inf->misc[2] = "M. Step";
			break;
		}
		case itype_ring:
		{
			_SET(misc[0], "Hero Sprite Pal:", "The Sprite Palette row to load into CSet 6");
			_SET(flag[0], "Affects Damage Combos", "Whether or not the ring reduces damage from damage combos.");
			_SET(flag[1], "Percentage Multiplier", "Changes the 'Damage Divisor' to a 'Damage % Mult'"
				", allowing more precise control over the defense granted.");
			if(FLAG(2))
				_SET(power, "Damage % Mult:", "The percentage to multiply the damage by. A negative value"
					" will deal that amount *more* damage; i.e. '-100' is the same as '200'.");
			else
				_SET(power, "Damage Divisor:", "The number to divide the damage by. A divisor of 0"
					" makes the Hero take no damage at all.");
			break;
		}
		case itype_wand: //!TODO Help Text
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
			inf->actionsnd[0] = "Firing Sound:";
			break;
		}
		case itype_bait: //!TODO Help Text
		{
			_SET(flag[1], "Only for feeding", "The bait cannot be placed except when feeding a hungry NPC");
			_SET(flag[2], "Don't remove when feeding", "The item will not be removed from the inventory upon feeding a hungry NPC.");
			_SET(flag[3], "Only require cost when feeding", "The item's Use Cost will only be checked when feeding hungry NPCs, not when luring enemies.");
			if(!FLAG(4))
				_SET(flag[4], "Only charge cost when feeding", "The item's Use Cost will only be charged when feeding hungry NPCs, not when luring enemies."
					"\nIt will still be CHECKED when luring enemies.");
			if(!FLAG(2)) //Not only for feeding
			{
				_SET(misc[0], "Duration:", "The duration, in frames, the bait sits on the ground for when luring enemies");
				_SET(misc[1], "Bait Range (0 = infinite)", "The range, in pixels, to lure enemies in");
				_SET(flag[0], "Repels enemies", "Repel enemies instead of luring them");
			}
			inf->wpn[0] = "Bait Sprite:";
			inf->actionsnd[0] = "Baiting Sound:";
			break;
		}
		case itype_potion:
		{
			_SET(flag[0], "Life is Percent", "HP Regained is a percentage out of max HP");
			_SET(flag[1], "Magic is Percent", "MP Regained is a percentage out of max MP");
			_SET(flag[2], "Removes Sword Jinxes", "Heal sword jinxes on use");
			_SET(flag[3], "Removes Item Jinxes", "Heal item jinxes on use");
			_SET(flag[4], "Removes Shield Jinxes", "Heal shield jinxes on use");
			
			if(FLAG(1))
				_SET(misc[0], "% HP Regained:", "Percentage of max life restored when collected.");
			else
				_SET(misc[0], "HP Regained:", "Life (in points) restored when collected.");
			if(FLAG(2))
				_SET(misc[1], "% MP Regained:", "Percentage of max magic restored when collected.");
			else
				_SET(misc[1], "MP Regained:", "Magic (in points) restored when collected.");
			
			break;
		}
		case itype_whistle:
		{
			auto windname = ZI.getWeapName(wWind);
			auto whistlename = ZI.getWeapName(wWhistle);
			_SET(misc[0], fmt::format("{} Direction:", windname), fmt::format("Direction the {} faces. 0 = Up, 1 = Down, 2 = Left, 3 = Right, 4 = Opposite the Hero", windname));
			_SET(misc[1], "Warp Ring:", "Which warp ring to use when warping the Hero");
			if(FLAG(2))
				_SET(misc[4], fmt::format("{} Damage", whistlename), fmt::format("The damage of the {} to anything that hears it.",whistlename));
			_SET(flag[0], "One W.Wind Per Scr.", fmt::format("Only allow a {} to be summoned once per screen.", windname));
			_SET(flag[1], "Has Damage", fmt::format("If checked, the {0} weapon will deal the damage set in the '{0} Damage' attribute.",whistlename));
			_SET(flag[2], fmt::format("{} Reflects off Prism/Mirror Combos", windname), fmt::format("If checked, {} weapons will reflect off of Mirrors and be duplicated by Prisms.", windname));
			inf->wpn[0] = fmt::format("{} Sprite:", windname);
			_SET(actionsnd[0], "Music Sound:", "The musical tune played");
			break;
		}
		case itype_candle:
		{
			inf->power = "Damage:";
			_SET(misc[1], "Max Fires On Screen", "If < 1, defaults to 1");
			_SET(misc[3], "Step Speed", "The step speed of the created fire weapon, where 100 = 1px/frame. Default 50.");
			_SET(flag[0], "Limited Per Screen", "Can only use a set number of times per screen");
			_SET(flag[1], "Don't Provide Light", "Does not light up dark rooms");
			inf->flag[2] = "Fire Doesn't Hurt Hero";
			_SET(flag[3], "Can Slash", "The candle slashes instead of stabs");
			inf->flag[7] = "Flip Right-Facing Slash";
			_SET(flag[8], "Counts as Strong Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfSTRONGFIRE)));
			_SET(flag[9], "Counts as Magic Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfMAGICFIRE)));
			_SET(flag[10], "Counts as Divine Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfDIVINEFIRE)));
			_SET(wpn[0], "Stab Sprite:", "The sprite for the candle stabbing");
			_SET(wpn[1], "Slash Sprite:", "The sprite for the candle slashing");
			_SET(wpn[2], "Fire Sprite:", "The sprite for the candle fire");
			inf->actionsnd[0] = "Firing Sound:";
			if(FLAG(1))
			{
				_SET(misc[2], "Uses Per Screen", "How many times this candle can be used per screen. Minimum 1.");
			}
			if(!FLAG(2))
			{
				_SET(flag[4], "Temporary Light", "In Old-style Dark Rooms, only keeps the room lit while it exists.");
			}
			if(!FLAG(3))
			{
				_SET(misc[0], "Damage to Hero:", "Damage value to be used for self-damage");
			}
			break;
		}
		case itype_arrow: //!TODO Help Text
		{
			inf->power = "Damage:";
			inf->misc[0] = "Duration (0 = Infinite):";
			_SET(misc[1], "Max Arrows On Screen", "If < 1, defaults to 2");
			inf->flag[0] = "Penetrate Enemies";
			inf->flag[1] = "Allow Item Pickup";
			inf->flag[3] = "Pick Up Anything";
			inf->flag[6] = "Picks Up Keys";
			inf->actionsnd[0] = "Firing Sound:";
			_SET(wpn[0], "Arrow Sprite:", "The sprite used for the arrow weapon."
				" Should contain an up-facing sprite followed by a right-facing sprite.");
			inf->wpn[1] = "Sparkle Sprite:";
			inf->wpn[2] = "Damaging Sparkle Sprite:";
			break;
		}
		case itype_brang: //!TODO Help Text
		{
			inf->power = "Damage:";
			_SET(misc[0], "Duration:", "The amount of frames the boomerang flys before returning.\n0 = Infinite");
			inf->misc[2] = "Block Flags:";
			inf->misc[3] = "Reflect Flags:";
			inf->flag[0] = "Corrected Animation";
			inf->flag[1] = "Directional Sprites";
			inf->flag[2] = "Do Not Return";
			inf->flag[3] = "Pick Up Anything";
			inf->flag[4] = "Drags Items";
			inf->flag[5] = "Reflects Enemy Projectiles";
			inf->flag[6] = "Picks Up Keys";
			_SET(flag[7], "Counts as Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfANYFIRE)));
			_SET(flag[8], "Counts as Strong Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfSTRONGFIRE)));
			_SET(flag[9], "Counts as Magic Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfMAGICFIRE)));
			_SET(flag[10], "Counts as Divine Fire", fmt::format("The flames will trigger '{}' flags",ZI.getMapFlagName(mfDIVINEFIRE)));
			
			inf->wpn[0] = "Boomerang Sprite:";
			inf->wpn[1] = "Sparkle Sprite:";
			inf->wpn[2] = "Damaging Sparkle Sprite:";
			inf->actionsnd[0] = "Spinning Sound:";
			break;
		}
		case itype_sword: //!TODO Help Text
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
			inf->actionsnd[0] = "Slash/Stab Sound";
			inf->actionsnd[1] = "Beam Sound";
			break;
		}
		case itype_whimsicalring:
		{
			_SET(power, "Power Bonus:", "How much power to add to strikes from the Sword, Wand, and Hammer."
				"\nThis is added before Attack Rings take effect, and also before the hero damage multiplier.");
			_SET(misc[0], "Chance (1 in n):", "There is a '1 in n' chance of the bonus power being added (min 1)");
			_SET(actionsnd[0], "Whimsy Sound:", "This sound plays when the bonus power is successfully applied.");
			break;
		}
		case itype_perilring:
		{
			_SET(misc[0], "Maximum Hearts:", "Takes effect when below this number of hearts");
			inf->flag[0] = "Affects Damage Combos";
			inf->flag[1] = "Percentage Multiplier";
			if(FLAG(2))
				inf->power = "Damage % Mult:";
			else
				inf->power = "Damage Divisor:";
			break;
		}
		case itype_stompboots:
		{
			inf->power = "Damage:";
			_SET(flag[0], "Can bounce off enemies", "If enabled, damaging an enemy with this will cause the Hero to bounce upwards."
				" The jump speed gained is set by Attributes[0]");
			if(FLAG(1))
				_SET(misc[0], "Bounce Power:", "Amount of jump power gained from a bounce."
					" This value is equal to setting Hero->Jump by value divided by 100");
			_SET(misc[1], "Block Flags:", "(Rock=1, Arrow=2, BRang=4, Fireball=8, Sword=16, Magic=32, Flame=64, Script(All)=128, Firebal2=256,\n"
				"Script1=1024, Script2=2048, Script3=4096, Script4=8192, Script5=16384, Script6=32768, Script7=65536, Script8=131072, Script9=262144, Script10=524288)\n"
				"Sum all of the values you want to apply. Weapons with their flags set will be blocked if the Hero lands on them.");
			break;
		}
		case itype_bow:
		{
			_SET(power, "Arrow Speed Mod:", "Arrow moves at 3 pixels per frame, multiplied by '(1+n)/2', where 'n' is this value.");
			break;
		}
		case itype_script1: case itype_script2: case itype_script3: case itype_script4:
		case itype_script5: case itype_script6: case itype_script7: case itype_script8:
		case itype_script9: case itype_script10:
		{
			_SET(misc[0], "Step Speed:", "The speed, in 100ths of pixel/frame");
			_SET(flag[0], "No GFX Flip", "If checked, the sprite is not flipped based on direction.");
			_SET(wpn[0], "Weapon Sprite", "The sprite the weapon uses. Will be flipped/rotated based on direction, unless 'No GFX Flip' is checked");
			break;
		}
		case itype_custom1: case itype_custom2: case itype_custom3: case itype_custom4:
		case itype_custom5: case itype_custom6: case itype_custom7: case itype_custom8:
		case itype_custom9: case itype_custom10:
		{
			break;
		}
		case itype_icerod:
		{
			_SET(misc[0], "Step Speed:", "The speed, in 100ths of pixel/frame");
			_SET(flag[0], "No GFX Flip", "If checked, the sprite is not flipped based on direction.");
			break;
		}
		case itype_flippers:
		{
			_SET(flag[0], "No Diving", "If checked, these flippers cannot dive.");
			_SET(flag[1], "Cancellable Diving", "Pressing B will immediately cancel a dive");
			_SET(flag[2], "Can Swim in Lava", "These flippers will be able to swim in liquid marked as 'Lava'.");
			
			_SET(misc[0], "Dive Length:", "Length, in frames, of the dive.");
			_SET(misc[1], "Dive Cooldown:", "Cooldown, in frames, between coming up from a dive and being able to dive again.");
			_SET(misc[2], "Dive Button", "If 0, diving will not be possible.\n"
				"Otherwise, any of the specified buttons will dive.\n"
				"Sum all the buttons you want to be usable:\n(A=1, B=2, L=4, R=8, Ex1=16, Ex2=32, Ex3=64, Ex4=128)");
			break;
		}
		case itype_raft:
		{
			_SET(misc[0], "Speed Modifier:", "Valid values -8 to 5."
				"\nIf positive, move 2^n pixels per frame."
				"\nIf negative, move 1 pixel per 2^n frames."
				"\nA value of 0 will stop the raft dead. This is useless unless combined with scripts.");
			break;
		}
		case itype_atkring:
		{
			_SET(misc[0], "Bonus Power", "This amount is *added* to the power, *after* the multiplier.");
			_SET(misc[1], "Power Multiplier", "The power is multiplied by this value.");
			break;
		}
		case itype_lantern:
		{
			_SET(misc[0], "Shape", "What shape to use for the light area emitted.\n"
				"0 = circular, 1 = cone in front, 2 = square");
			_SET(misc[1], "Range", "The range, in pixels, of the light.");
			_SET(flag[0], "No Light 'Wave'", "The light cast from this item is not affected by the 'light wave' settings");
			break;
		}
		case itype_bottle:
		{
			_SET(misc[0], "Slot:", "Which slot this bottle item is attached to. Valid values: 0-255.");
			break;
		}
		case itype_bottlefill:
		{
			_SET(misc[0], "Contents:", "What contents to place in an empty bottle when picked up");
			break;
		}
		case itype_bugnet:
		{
			_SET(flag[0], "Can't catch fairies", "If checked, no longer catches fairies"
				" it collides with");
			_SET(flag[1], "Right-handed", "Swaps swing direction of the weapon");
			_SET(actionsnd[0], "Swing Sound", "Sound played when swinging the net");
			break;
		}
		case itype_mirror:
		{
			_SET(misc[0], "Warp Effect", "What warp effect to use during the warp to another dmap.\n"
				"0=None, 1=Zap, 2=Wave, 3=Blackscr, 4=OpenWipe");
			_SET(misc[1], "Cont. Warp Effect", "What warp effect to use during the warp to the continue point.\n"
				"0=None, 1=Zap, 2=Wave, 3=Blackscr, 4=OpenWipe");
			_SET(flag[0], "Place Return Portal", "If checked, places a return portal when"
				" mirroring to a new dmap");
			_SET(flag[1], "Continue acts as F6->Continue",
				"When used on a dmap with 'Mirror Continues instead of Warping' checked, "
				"activates F6->Continue instead of Divine Escape effect if enabled.");
			_SET(wpn[0], "Portal Sprite", "Sprite of the Return Portal");
			_SET(actionsnd[0], "Warp Sound", "Sound played for the warp to a new dmap");
			_SET(actionsnd[1], "Continue Sound", "Sound played for a continue warp");
			break;
		}
		case itype_lkey:
		{
			_SET(flag[0], "Specific Level", "If checked, grants a key for a specific level, instead of the 'current' level");
			if(FLAG(1))
				_SET(misc[0], "Key Level", "The level to grant a key for");
			break;
		}
	}
	#undef _SET
	#undef FLAG
}

char const* get_ic_help(size_t q)
{
	static std::string buf;
	buf = ZI.getItemClassHelp(q);
	switch(q)
	{
		case itype_liftglove:
		{
			buf += QRHINT({qr_CARRYABLE_NO_ACROSS_SCREEN,qr_NO_SCROLL_WHILE_CARRYING});
			break;
		}
	}
	return buf.c_str();
}

ItemEditorDialog::ItemEditorDialog(itemdata const& ref, char const* str, int32_t index):
	itemname(str), index(index), local_itemref(ref),
	list_items(GUI::ZCListData::itemclass(true)),
	list_counters(GUI::ZCListData::counters(true)),
	list_sprites(GUI::ZCListData::miscsprites()),
	list_itemdatscript(GUI::ZCListData::itemdata_script()),
	list_itemsprscript(GUI::ZCListData::itemsprite_script()),
	list_weaponscript(GUI::ZCListData::lweapon_script()),
	list_weaptype(GUI::ZCListData::lweaptypes()),
	list_deftypes(GUI::ZCListData::deftypes()),
	list_bottletypes(GUI::ZCListData::bottletype()),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_strings(GUI::ZCListData::strings())
{}

ItemEditorDialog::ItemEditorDialog(int32_t index):
	ItemEditorDialog(itemsbuf[index], item_string[index], index)
{}

//{ Macros

#define DISABLE_WEAP_DATA true
#define ATTR_WID 6_em
#define ATTR_LAB_WID 12_em
#define SPR_LAB_WID 10_em
#define ACTION_LAB_WID 8_em
#define ACTION_FIELD_WID 6_em
#define FLAGS_WID 20_em

#define NUM_FIELD(member,_min,_max) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = local_itemref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_itemref.member = val; \
	})
#define NUM_FIELD_W(member,_min,_max,wid) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, width = wid, \
	low = _min, high = _max, val = local_itemref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_itemref.member = val; \
	})

std::shared_ptr<GUI::Widget> ItemEditorDialog::ATTRIB_FIELD_IMPL(int32_t* mem, int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(
		colSpan = 3,
		l_attribs[index] = Label(textAlign = 2, width = ATTR_LAB_WID),
		ib_attribs[index] = Button(forceFitH = true, text = "?",
		disabled = true,
		onPressFunc = [&, index]()
		{
			InfoDialog("Attribute Info",h_attribs[index]).show();
		}),
		TextField(maxLength = 11,
			type = GUI::TextField::type::INT_DECIMAL, width = ATTR_WID,
			val = *mem,
			onValChangedFunc = [mem](GUI::TextField::type,std::string_view,int32_t val)
			{
				*mem = val;
			}
		)
	);
}

#define ATTRIB_FIELD(member, index) ATTRIB_FIELD_IMPL(&local_itemref.member, index)

std::shared_ptr<GUI::Widget> ItemEditorDialog::FLAG_CHECK(int index, item_flags bit)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		ib_flags[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("Flags Info",h_flags[index]).show();
			}),
		l_flags[index] = Checkbox(
			width = FLAGS_WID,
			checked = (local_itemref.flags & bit),
			onToggleFunc = [&, bit](bool state)
			{
				SETFLAG(local_itemref.flags,bit,state);
				loadItemClass();
			}
		)
	);
}

#define FLAG_CHECK_NOINFO(index, bit) \
l_flags[index] = Checkbox( \
	width = FLAGS_WID, \
	checked = (local_itemref.flags & bit), \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_itemref.flags,bit,state); \
	} \
) \

#define LITEM_CHECK(txt, flag) \
Checkbox( \
	hAlign = 0.0, \
	text = txt, \
	checked = (local_itemref.pickup_litems & flag), \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_itemref.pickup_litems,flag,state); \
	} \
)

template <typename T>
std::shared_ptr<GUI::Widget> ItemEditorDialog::SPRITE_DROP_IMPL(T* mem, int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(vPadding = 0_px,
		l_spr[index] = Label(textAlign = 2, width = SPR_LAB_WID, topMargin = 1_px),
		DropDownList(
			maxwidth = 14_em,
			data = list_sprites,
			selectedValue = *mem,
			onSelectFunc = [mem](int32_t val)
			{
				*mem = val;
			}),
		ib_spr[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("Sprite Info",h_spr[index]).show();
			})
	);
}

#define SPRITE_DROP(ind, mem) SPRITE_DROP_IMPL(&local_itemref.mem, ind)

std::shared_ptr<GUI::Widget> ItemEditorDialog::IT_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		l_it_initds[index] = Label(minwidth = ATTR_LAB_WID, textAlign = 2),
		ib_it_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info",h_it_initds[index]).show();
			}),
		tf_it_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			val = local_itemref.initiald[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_itemref.initiald[index] = val;
			})
	);
}
std::shared_ptr<GUI::Widget> ItemEditorDialog::WP_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		l_wp_initds[index] = Label(minwidth = ATTR_LAB_WID, textAlign = 2),
		ib_wp_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info",h_wp_initds[index]).show();
			}),
		tf_wp_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			val = local_itemref.weap_initiald[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_itemref.weap_initiald[index] = val;
			})
	);
}

int32_t calcBottleTile(itemdata const& local_itemref, byte bottleVal)
{
	if(local_itemref.family != itype_bottle)
		return local_itemref.tile;
	int32_t o_tile = local_itemref.tile;
	int32_t tile = o_tile + bottleVal * (zc_max(local_itemref.frames,1)
		* ((local_itemref.overrideFLAGS & OVERRIDE_TILE_WIDTH)
			? zc_max(local_itemref.tilew,1) : 1));
	auto oldRow = o_tile/TILES_PER_ROW;
	auto newRow = tile/TILES_PER_ROW;
	if(oldRow != newRow)
	{
		tile += (newRow-oldRow) * TILES_PER_ROW
			* ((local_itemref.overrideFLAGS & OVERRIDE_TILE_HEIGHT)
				? zc_max(local_itemref.tileh,1)-1 : 0);
	}
	return tile;
}

//}

static size_t itmtabs[5] = {0};
static byte bottleType = 0;
std::shared_ptr<GUI::Widget> ItemEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	char titlebuf[256];
	sprintf(titlebuf, "Item Editor (%d): %s", index, itemname.c_str());
	window = Window(
		use_vsync = true,
		title = titlebuf,
		onClose = message::CANCEL,
		Column(
			Row(
				Rows<5>(padding = 0_px,
					Label(text = "Name:", hAlign = 1.0),
					TextField(
						fitParent = true,
						maxwidth = 400_px,
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
					_d,
					Checkbox(
						checked = (local_itemref.flags & item_gamedata),
						text = "Equipment Item", _EX_RBOX,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_itemref.flags,item_gamedata,state);
						}
					),
					INFOBTN("Only 'Equipment Item's can be 'owned' by the Hero,"
						" and appear in Init Data."),
					//
					Label(text = "Display Name:", hAlign = 1.0),
					TextField(
						fitParent = true,
						maxwidth = 400_px,
						maxLength = 255,
						text = local_itemref.display_name,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
						{
							std::string s(str);
							strncpy(local_itemref.display_name,s.c_str(),255);
						}
					),
					INFOBTN("If this field is not blank, this text will display as the 'Selected Item Name' on the subscreen for this item."
						"\nFor some types, such as Bottles, the text '%s' in the display name will be replaced with a special string (such as the bottle contents)."
						"\nEx: 'Bottle (%s)' becomes 'Bottle (Empty)', or 'Bottle (Health Potion)'"),
					wizardButton = Button(
						text = "Wizard", disabled = !hasItemWizard(local_itemref.family),
						rowSpan = 2, minwidth = 150_px,
						padding = 0_px, forceFitH = true, onClick = message::WIZARD
					),
					_d,
					//
					Label(text = "Type:", hAlign = 1.0),
					DropDownList(data = list_items,
						fitParent = true, padding = 0_px,
						maxwidth = 400_px,
						selectedValue = local_itemref.family,
						onSelectionChanged = message::ITEMCLASS
					),
					Button(forceFitH = true, text = "?",
						minheight = 24_px,
						onPressFunc = [&]()
						{
							InfoDialog(ZI.getItemClassName(local_itemref.family),
								get_ic_help(local_itemref.family)).show();
						})
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
							NUM_FIELD_W(fam_type, 1, 255, ATTR_WID),
							l_power = Label(width=ATTR_LAB_WID,textAlign=2),
							ib_power = Button(forceFitH = true, text = "?",
								disabled = true,
								onPressFunc = [&]()
								{
									InfoDialog("Power Info",h_power).show();
								}),
							NUM_FIELD_W(power, 0, 255, ATTR_WID)
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
						Rows<4>(
							framed = true,
							frameText = "General Flags",
							topPadding = DEFAULT_PADDING+0.4_em,
							bottomPadding = DEFAULT_PADDING+1_px,
							bottomMargin = 1_em,
							DINFOBTN(),
							Checkbox(
								width = FLAGS_WID,
								checked = (local_itemref.flags & item_edible),
								text = "Can Be Eaten By Enemies",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,item_edible,state);
								}
							),
							INFOBTN("The item's 'Action Script' runs every frame while the item is owned,"
								"\ninstead of when the item is 'used'."),
							FLAG_CHECK_NOINFO(15,item_passive_script),
							DINFOBTN(),
							Checkbox(
								width = FLAGS_WID,
								checked = (local_itemref.flags & item_sideswim_disabled),
								text = "Disabled In Sideview Water",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,item_sideswim_disabled,state);
								}
							),
							INFOBTN("If checked, this item can be used even while the Hero"
								" has been turned into a 'bunny', and will still apply its"
								" Hero Tile Modifier."),
							Checkbox(
								width = FLAGS_WID,
								checked = (local_itemref.flags & item_bunny_enabled),
								text = "Usable as a Bunny",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,item_bunny_enabled,state);
								}
							),
							DINFOBTN(),
							Checkbox(
								width = FLAGS_WID,
								checked = (local_itemref.flags & item_jinx_immune),
								text = "Immune to jinxes",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,item_jinx_immune,state);
								}
							),
							INFOBTN("With this checked, swords will use the item jinx, and vice-versa."),
							Checkbox(
								width = FLAGS_WID,
								checked = (local_itemref.flags & item_flip_jinx),
								text = "Uses Other Jinx",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.flags,item_flip_jinx,state);
								}
							)
						),
						Columns<8>(
							framed = true,
							frameText = "Variable Flags",
							topPadding = DEFAULT_PADDING+0.4_em,
							bottomPadding = DEFAULT_PADDING+1_px,
							FLAG_CHECK(0,item_flag1),
							FLAG_CHECK(1,item_flag2),
							FLAG_CHECK(2,item_flag3),
							FLAG_CHECK(3,item_flag4),
							FLAG_CHECK(4,item_flag5),
							FLAG_CHECK(5,item_flag6),
							FLAG_CHECK(6,item_flag7),
							FLAG_CHECK(7,item_flag8),
							FLAG_CHECK(8,item_flag9),
							FLAG_CHECK(9,item_flag10),
							FLAG_CHECK(10,item_flag11),
							FLAG_CHECK(11,item_flag12),
							FLAG_CHECK(12,item_flag13),
							FLAG_CHECK(13,item_flag14),
							FLAG_CHECK(14,item_flag15)
						)
					)),
					TabRef(name = "Action", Row(
						Column(
							Rows<2>(framed = true, frameText = "Use Cost",
								padding = DEFAULT_PADDING*2,
								margins = DEFAULT_PADDING,
								TextField(
									val = local_itemref.cost_amount[0],
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -32768, high = 32767,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.cost_amount[0] = val;
									}
								),
								DropDownList(
									data = list_counters,
									selectedValue = local_itemref.cost_counter[0],
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.cost_counter[0] = val;
									}
								),
								Label(text = "Timer:", textAlign = 2, forceFitW = true),
								TextField(
									val = local_itemref.magiccosttimer[0],
									type = GUI::TextField::type::INT_DECIMAL,
									minwidth = ACTION_FIELD_WID, fitParent = true, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.magiccosttimer[0] = val;
									}
								),
								INFOBTN_EX("Requires that the cost be met, but does not consume it.", hAlign = 1.0, nopad = true, forceFitH = true),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & item_validate_only),
									text = "Only Validate Cost",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,item_validate_only,state);
									}
								)
							),
							Rows<2>(framed = true, frameText = "Use Cost 2",
								padding = DEFAULT_PADDING*2,
								margins = DEFAULT_PADDING,
								TextField(
									val = local_itemref.cost_amount[1],
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = -32768, high = 32767,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.cost_amount[1] = val;
									}
								),
								DropDownList(
									data = list_counters,
									selectedValue = local_itemref.cost_counter[1],
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.cost_counter[1] = val;
									}
								),
								Label(text = "Timer:", textAlign = 2, forceFitW = true),
								TextField(
									val = local_itemref.magiccosttimer[1],
									type = GUI::TextField::type::INT_DECIMAL,
									minwidth = ACTION_FIELD_WID, fitParent = true, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.magiccosttimer[1] = val;
									}
								),
								INFOBTN_EX("Requires that the cost be met, but does not consume it.", hAlign = 1.0, nopad = true, forceFitH = true),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & item_validate_only_2),
									text = "Only Validate Cost 2",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,item_validate_only_2,state);
									}
								)
							)
						),
						Column(
							Rows<3>(framed = true, frameText = "SFX",
								padding = DEFAULT_PADDING*2,
								margins = DEFAULT_PADDING,
								l_sfx[0] = Label(textAlign = 2, width = ACTION_LAB_WID),
								ib_sfx[0] = Button(forceFitH = true, text = "?",
									disabled = true,
									onPressFunc = [&]()
									{
										InfoDialog("SFX Info",h_sfx[0]).show();
									}),
								DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_itemref.usesound,
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.usesound = val;
									}),
								l_sfx[1] = Label(textAlign = 2, width = ACTION_LAB_WID),
								ib_sfx[1] = Button(forceFitH = true, text = "?",
									disabled = true,
									onPressFunc = [&]()
									{
										InfoDialog("SFX Info",h_sfx[1]).show();
									}),
								DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_itemref.usesound2,
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.usesound2 = val;
									})
							),
							Rows<2>(
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.flags & item_downgrade),
									text = "Remove Item When Used",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,item_downgrade,state);
									}
								)
							)
						)
					)),
					TabRef(name = "Pickup", Row(
						Column(padding = 0_px,
							Rows<4>(framed = true, fitParent = true,
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
								),
								Checkbox(colSpan = 2,
									hAlign = 0.0,
									checked = (local_itemref.flags & item_combine),
									text = "Upgrade When Collected Twice",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,item_combine,state);
									}
								),
								//
								Label(text = "Increase By:", hAlign = 1.0),
								TextField(
									val = ((local_itemref.amount & 0x4000) ? -1 : 1)*signed(local_itemref.amount & 0x3FFF),
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, low = -9999, high = 16383,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.amount &= 0x8000;
										local_itemref.amount |= (abs(val)&0x3FFF)|(val<0?0x4000:0);
									}
								),
								Checkbox(colSpan = 2,
									hAlign = 0.0,
									checked = (local_itemref.amount & 0x8000),
									text = "Gradual",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.amount,0x8000,state);
									}
								),
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
								),
								Checkbox(colSpan = 2,
									hAlign = 0.0,
									checked = (local_itemref.flags & item_keep_old),
									text = "Keep Lower Level Items",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,item_keep_old,state);
									}
								),
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
								),
								Checkbox(colSpan = 2,
									hAlign = 0.0,
									checked = (local_itemref.flags & item_gain_old),
									text = "Gain All Lower Level Items",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,item_gain_old,state);
									}
								)
							),
							Column(framed = true, fitParent = true,
								Row(
									Label(text = "String:"),
									DropDownList(data = list_strings,
										selectedValue = local_itemref.pstring,
										fitParent = true,
										onSelectFunc = [&](int32_t val)
										{
											local_itemref.pstring = val;
										}
									)
								),
								Row(
									INFOBTN("The pickup string shows every time the item is collected, instead of just once."),
									Checkbox(
										hAlign = 0.0,
										checked = (local_itemref.pickup_string_flags & itemdataPSTRING_ALWAYS),
										text = "Always",
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_itemref.pickup_string_flags,itemdataPSTRING_ALWAYS,state);
										}
									),
									INFOBTN("The pickup string shows only if the item is held up, such as when received from a chest."),
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
							)
						),
						Column(
							Frame(title = "Gain Level Items",
								Column(padding = 0_px,
									Columns<4>(
										LITEM_CHECK("McGuffin", liTRIFORCE),
										LITEM_CHECK("Map", liMAP),
										LITEM_CHECK("Compass", liCOMPASS),
										LITEM_CHECK("Boss Killed", liBOSS),
										LITEM_CHECK("Boss Key", liBOSSKEY),
										LITEM_CHECK("Custom 01", liCUSTOM01),
										LITEM_CHECK("Custom 02", liCUSTOM02),
										LITEM_CHECK("Custom 03", liCUSTOM03)
									),
									Row(
										Label(text = "For Level:"),
										TextField(
											vPadding = 0_px,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -1, high = MAXLEVELS, val = local_itemref.pickup_litem_level,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_itemref.pickup_litem_level = val;
											}),
										INFOBTN("The level that the Level Item will be granted for. If set to '-1', grants for the current level.")
									)
								)
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
									selectedValue = local_itemref.usedefense,
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.usedefense = val;
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
									if(local_itemref.misc_flags & 1)
										animFrame->setFlashCS(val);
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
							Label(text = "Hero Tile Modifier:", hAlign = 1.0),
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
										" are added up, and the total is added to the Hero's tile.\n"
										"If the Hero is a Bunny, then only items which have the 'Usable as a Bunny'"
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
								onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t)
								{
									local_itemref.tile = t;
									local_itemref.csets &= 0xF0;
									local_itemref.csets |= c&0x0F;
									animFrame->setTile(calcBottleTile(local_itemref, bottleType));
									animFrame->setCSet(c);
								}
							),
							Checkbox(
								hAlign = 0.0,
								checked = (local_itemref.misc_flags & 1),
								text = "Flash",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.misc_flags,1,state);
									if(state)
										animFrame->setFlashCS(local_itemref.csets >> 4);
									else
										animFrame->setFlashCS(-1);
								}
							),
							Checkbox(
								hAlign = 0.0,
								checked = (local_itemref.misc_flags & 2),
								text = "2-Hand",
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_itemref.misc_flags,2,state);
								}
							)
						),
						Column(
							animSwitcher = Switcher(
								DummyWidget(),
								DropDownList(data = list_bottletypes,
									fitParent = true, padding = 0_px,
									selectedValue = bottleType,
									onSelectFunc = [&](int32_t val)
									{
										bottleType = (byte)val;
										animFrame->setTile(calcBottleTile(local_itemref, bottleType));
										pendDraw();
									}
								)
							),
							animFrame = TileFrame(
								hAlign = 0.0,
								tile = local_itemref.tile,
								cset = (local_itemref.csets & 0xF),
								frames = local_itemref.frames,
								speed = local_itemref.speed,
								delay = local_itemref.delay,
								skipx = (local_itemref.overrideFLAGS & OVERRIDE_TILE_WIDTH)
									? local_itemref.tilew-1 : 0,
								skipy = (local_itemref.overrideFLAGS & OVERRIDE_TILE_HEIGHT)
									? local_itemref.tileh-1 : 0,
								do_sized = true,
								flashcs = (local_itemref.misc_flags&1) ? local_itemref.csets>>4 : -1
							),
							Button(text = "Refresh Preview", onClick = message::RELOAD)
						)
					)),
					TabRef(name = "Sprites", TabPanel(
						ptr = &itmtabs[4],
						TabRef(name = "Basic", Columns<5>(
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
						TabRef(name = "Burning", Column(
							Row(
								INFOBTN("With this checked, the created weapon will use the appropriate"
									" burning sprite INSTEAD of its' normal sprite."
									"\nAdditionally, the weapon will use the specified light radius."),
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & item_burning_sprites),
									text = "Use Burning Sprites",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,item_burning_sprites,state);
									}
								)
							),
							Rows<4>(
								_d, Label(text = "Sprite"), Label(text = "Light Radius"), _d,
								//
								Label(text = "No Fire:", hAlign = 1.0),
								DropDownList(
									data = list_sprites,
									selectedValue = local_itemref.burnsprs[WPNSPR_BASE],
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.burnsprs[WPNSPR_BASE] = val;
									}),
								NUM_FIELD(light_rads[WPNSPR_BASE], 0, 255),
								INFOBTN("Settings used for the weapon when not on fire"),
								//
								Label(text = "Normal Fire:", hAlign = 1.0),
								DropDownList(
									data = list_sprites,
									selectedValue = local_itemref.burnsprs[WPNSPR_IGNITE_ANY],
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.burnsprs[WPNSPR_IGNITE_ANY] = val;
									}),
								NUM_FIELD(light_rads[WPNSPR_IGNITE_ANY], 0, 255),
								INFOBTN("Settings used for the weapon when on 'Normal' fire"),
								//
								Label(text = "Strong Fire:", hAlign = 1.0),
								DropDownList(
									data = list_sprites,
									selectedValue = local_itemref.burnsprs[WPNSPR_IGNITE_STRONG],
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.burnsprs[WPNSPR_IGNITE_STRONG] = val;
									}),
								NUM_FIELD(light_rads[WPNSPR_IGNITE_STRONG], 0, 255),
								INFOBTN("Settings used for the weapon when on 'Strong' fire"),
								//
								Label(text = "Magic Fire:", hAlign = 1.0),
								DropDownList(
									data = list_sprites,
									selectedValue = local_itemref.burnsprs[WPNSPR_IGNITE_MAGIC],
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.burnsprs[WPNSPR_IGNITE_MAGIC] = val;
									}),
								NUM_FIELD(light_rads[WPNSPR_IGNITE_MAGIC], 0, 255),
								INFOBTN("Settings used for the weapon when on 'Magic' fire"),
								//
								Label(text = "Divine Fire:", hAlign = 1.0),
								DropDownList(
									data = list_sprites,
									selectedValue = local_itemref.burnsprs[WPNSPR_IGNITE_DIVINE],
									onSelectFunc = [&](int32_t val)
									{
										local_itemref.burnsprs[WPNSPR_IGNITE_DIVINE] = val;
									}),
								NUM_FIELD(light_rads[WPNSPR_IGNITE_DIVINE], 0, 255),
								INFOBTN("Settings used for the weapon when on 'Divine' fire")
							)
						))
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
									onValueChanged = message::GFXSIZE,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.tilew = val;
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & OVERRIDE_TILE_WIDTH),
									text = "Enabled",
									onToggle = message::GFXSIZE,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_TILE_WIDTH,state);
									}
								)
							),
							Row(
								Label(textAlign = 2, width = 6_em, text = "TileHeight:"),
								TextField(
									val = local_itemref.tileh,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 32,
									onValueChanged = message::GFXSIZE,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.tileh = val;
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.overrideFLAGS & OVERRIDE_TILE_HEIGHT),
									text = "Enabled",
									onToggle = message::GFXSIZE,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_TILE_HEIGHT,state);
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
									checked = (local_itemref.overrideFLAGS & OVERRIDE_HIT_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_HIT_X_OFFSET,state);
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
									checked = (local_itemref.overrideFLAGS & OVERRIDE_HIT_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_HIT_Y_OFFSET,state);
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
									checked = (local_itemref.overrideFLAGS & OVERRIDE_HIT_WIDTH),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_HIT_WIDTH,state);
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
									checked = (local_itemref.overrideFLAGS & OVERRIDE_HIT_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_HIT_HEIGHT,state);
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
									checked = (local_itemref.overrideFLAGS & OVERRIDE_HIT_Z_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_HIT_Z_HEIGHT,state);
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
									checked = (local_itemref.overrideFLAGS & OVERRIDE_DRAW_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_DRAW_X_OFFSET,state);
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
									checked = (local_itemref.overrideFLAGS & OVERRIDE_DRAW_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.overrideFLAGS,OVERRIDE_DRAW_Y_OFFSET,state);
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
									onValueChanged = message::GFXSIZE,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_tilew = val;
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_TILE_WIDTH),
									text = "Enabled",
									onToggle = message::GFXSIZE,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_TILE_WIDTH,state);
									}
								)
							),
							Row(
								Label(textAlign = 2, width = 6_em, text = "TileHeight:"),
								TextField(
									val = local_itemref.weap_tileh,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 32,
									onValueChanged = message::GFXSIZE,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_itemref.weap_tileh = val;
									}
								),
								Checkbox(
									hAlign = 0.0,
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_TILE_HEIGHT),
									text = "Enabled",
									onToggle = message::GFXSIZE,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_TILE_HEIGHT,state);
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
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_HIT_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_HIT_X_OFFSET,state);
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
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_HIT_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_HIT_Y_OFFSET,state);
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
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_HIT_WIDTH),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_HIT_WIDTH,state);
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
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_HIT_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_HIT_HEIGHT,state);
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
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_HIT_Z_HEIGHT),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_HIT_Z_HEIGHT,state);
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
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_DRAW_X_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_DRAW_X_OFFSET,state);
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
									checked = (local_itemref.weapoverrideFLAGS & OVERRIDE_DRAW_Y_OFFSET),
									text = "Enabled",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.weapoverrideFLAGS,OVERRIDE_DRAW_Y_OFFSET,state);
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
							IT_INITD(0),
							IT_INITD(1),
							IT_INITD(2),
							IT_INITD(3),
							IT_INITD(4),
							IT_INITD(5),
							IT_INITD(6),
							IT_INITD(7)
						),
						Column(
							padding = 0_px, fitParent = true,
							Rows<2>(vAlign = 0.0,
								SCRIPT_LIST_PROC("Action Script:", list_itemdatscript, local_itemref.script, refreshScripts),
								SCRIPT_LIST_PROC("Pickup Script:", list_itemdatscript, local_itemref.collect_script, refreshScripts),
								SCRIPT_LIST_PROC("Sprite Script:", list_itemsprscript, local_itemref.sprite_script, refreshScripts)
							),
							Row(
								Label(text = "Script Info:"),
								DropDownList(
									maxwidth = 10_em,
									data = ScriptDataList,
									selectedValue = item_use_script_data,
									onSelectFunc = [&](int32_t val)
									{
										item_use_script_data = val;
										zc_set_config("zquest","show_itemscript_meta_type",val);
										refreshScripts();
									}
								)
							)
						)
					)),
					TabRef(name = "Weapon", Row(
						Column(
							WP_INITD(0),
							WP_INITD(1),
							WP_INITD(2),
							WP_INITD(3),
							WP_INITD(4),
							WP_INITD(5),
							WP_INITD(6),
							WP_INITD(7)
						),
						Rows<2>(vAlign = 0.0,
							SCRIPT_LIST_PROC("Weapon Script:", list_weaponscript, local_itemref.weaponscript, refreshScripts)
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
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL),
				Button(
					text = "Default",
					minwidth = 90_px,
					onClick = message::DEFAULT)
			)
		)
	);
	refreshScripts();
	return window;
}

void load_meta(ItemNameInfo& inf, zasm_meta const& meta)
{
	for(auto q = 0; q < 15; ++q)
	{
		if(meta.usrflags[q].size())
			inf.flag[q] = meta.usrflags[q];
		if(meta.usrflags_help[q].size())
			inf.h_flag[q] = meta.usrflags_help[q];
		if(q > 9) continue;
		if(meta.attributes[q].size())
			inf.misc[q] = meta.attributes[q];
		if(meta.attributes_help[q].size())
			inf.h_misc[q] = meta.attributes_help[q];
	}
}

void ItemEditorDialog::refreshScripts()
{
	loadItemClass();
	std::string it_initd[8];
	std::string wp_initd[8];
	int32_t ty_it_initd[8];
	int32_t ty_wp_initd[8];
	for(auto q = 0; q < 8; ++q)
	{
		it_initd[q] = "InitD[" + std::to_string(q) + "]";
		h_it_initds[q].clear();
		ty_it_initd[q] = -1;
		wp_initd[q] = "InitD[" + std::to_string(q) + "]";
		h_wp_initds[q].clear();
		ty_wp_initd[q] = -1;
	}
	bool did_item_scr = false;
	auto iscd = item_use_script_data;
	if(!iscd) iscd = ISCRDATA_ALL;
	if(local_itemref.sprite_script && (iscd & ISCRDATA_SPRITE))
	{
		did_item_scr = true;
		zasm_meta const& meta = itemspritescripts[local_itemref.sprite_script]->meta;
		for(auto q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				it_initd[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				h_it_initds[q] = meta.initd_help[q];
			if(meta.initd_type[q] > -1)
				ty_it_initd[q] = meta.initd_type[q];
		}
	}
	if(local_itemref.collect_script && (iscd & ISCRDATA_PICKUP))
	{
		did_item_scr = true;
		zasm_meta const& meta = itemscripts[local_itemref.collect_script]->meta;
		for(auto q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				it_initd[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				h_it_initds[q] = meta.initd_help[q];
			if(meta.initd_type[q] > -1)
				ty_it_initd[q] = meta.initd_type[q];
		}
	}
	if(local_itemref.script && (iscd & ISCRDATA_ACTION))
	{
		did_item_scr = true;
		zasm_meta const& meta = itemscripts[local_itemref.script]->meta;
		for(auto q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				it_initd[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				h_it_initds[q] = meta.initd_help[q];
			if(meta.initd_type[q] > -1)
				ty_it_initd[q] = meta.initd_type[q];
		}
	}
	if(!did_item_scr)
	{
		for(auto q = 0; q < 8; ++q)
			ty_it_initd[q] = nswapDEC;
	}
	if(local_itemref.weaponscript)
	{
		zasm_meta const& meta = lwpnscripts[local_itemref.weaponscript]->meta;
		for(auto q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				wp_initd[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				h_wp_initds[q] = meta.initd_help[q];
			if(meta.initd_type[q] > -1)
				ty_wp_initd[q] = meta.initd_type[q];
		}
	}
	else
	{
		for(auto q = 0; q < 8; ++q)
			ty_wp_initd[q] = nswapDEC;
	}
	for(auto q = 0; q < 8; ++q)
	{
		if(ty_it_initd[q] > -1)
			tf_it_initd[q]->setSwapType(ty_it_initd[q]);
		if(ty_wp_initd[q] > -1)
			tf_wp_initd[q]->setSwapType(ty_wp_initd[q]);
		l_it_initds[q]->setText(it_initd[q]);
		l_wp_initds[q]->setText(wp_initd[q]);
		ib_it_initds[q]->setDisabled(h_it_initds[q].empty());
		ib_wp_initds[q]->setDisabled(h_wp_initds[q].empty());
	}
}

void ItemEditorDialog::loadItemClass()
{
	animFrame->setTile(calcBottleTile(local_itemref, bottleType));
	animSwitcher->switchTo(local_itemref.family == itype_bottle ? 1 : 0);
	
	ItemNameInfo inf;
	loadinfo(&inf, local_itemref);
	
	if(item_use_script_data)
	{
		if(local_itemref.sprite_script && (item_use_script_data & ISCRDATA_SPRITE))
		{
			zasm_meta const& meta = itemspritescripts[local_itemref.sprite_script]->meta;
			load_meta(inf,meta);
		}
		if(local_itemref.collect_script && (item_use_script_data & ISCRDATA_PICKUP))
		{
			zasm_meta const& meta = itemscripts[local_itemref.collect_script]->meta;
			load_meta(inf,meta);
		}
		if(local_itemref.script && (item_use_script_data & ISCRDATA_ACTION))
		{
			zasm_meta const& meta = itemscripts[local_itemref.script]->meta;
			load_meta(inf,meta);
		}
	}
	wizardButton->setDisabled(!hasItemWizard(local_itemref.family));
	#define __SET(obj, mem) \
	l_##obj->setText(inf.mem.size() ? inf.mem : defInfo.mem); \
	h_##obj = (inf.h_##mem.size() ? inf.h_##mem : ""); \
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
	
	__SET(sfx[0], actionsnd[0]);
	__SET(sfx[1], actionsnd[1]);
	
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
	#undef __SET
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
		
		case message::GFXSIZE:
		{
			animFrame->setSkipX((local_itemref.overrideFLAGS & OVERRIDE_TILE_WIDTH)
				? local_itemref.tilew-1 : 0);
			animFrame->setSkipY((local_itemref.overrideFLAGS & OVERRIDE_TILE_HEIGHT)
				? local_itemref.tileh-1 : 0);
			return false;
		}
		
		case message::DEFAULT:
		{
			bool cancel = false;
			AlertDialog(
				"Reset itemdata?",
				"Reset this item to default?",
				[&](bool ret,bool)
				{
					cancel = !ret;
				}).show();
			if(cancel) return false;
			_reset_default = true;
			static_ref = local_itemref;
			reset_name = itemname;
			return true;
		}
		
		case message::WIZARD:
			if(hasItemWizard(local_itemref.family))
			{
				call_item_wizard(*this);
				rerun_dlg = true;
				return true;
			}
			break;
		
		case message::RELOAD:
		{
			_reload_editor = true;
			static_ref = local_itemref;
			reset_name = itemname;
			return true;
		}

		case message::OK:
			saved = false;
			itemsbuf[index] = local_itemref;
			strcpy(item_string[index], itemname.c_str());
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}

