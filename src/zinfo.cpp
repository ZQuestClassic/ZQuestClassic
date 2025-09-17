// TODO: this file could be split up between ZC and ZQ.

#include "base/zapp.h"
#include "base/zdefs.h"
#include "zinfo.h"
#include "gui/jwin.h"
#include "base/fonts.h"
#include "base/packfile.h"
#include "base/qst.h"


std::string getComboTypeHelpText(int32_t id);
std::string getMapFlagHelpText(int32_t id);

const char *itemclass_help_string_defaults[itype_max] =
{
	"The Hero's standard weapon. When wielded, it can stab, slash and fire beams. It is used to perform several scroll techniques, too.",
	"When wielded, it flies out, hurting enemies, and collecting items, before returning to the Hero. Can be thrown diagonally. Can optionally drop damaging sparkles.",
	"When wielded, it flies and collects items before hitting an enemy. Requires the Bow. Expends either 1 rupee or 1 arrow ammo. Can drop damaging sparkles.",
	"When wielded, a damaging flame drifts out, which lights dark screens until it expires. Can optionally have wand-style stab and slash sprites.",
	"When wielded, plays strange music and summons a whirlwind that warps you to the Warp Ring locations. Can also dry Water combos on specific screens.",
	"When wielded, drops bait that attracts Walking Enemies depending on their Hunger stat. Removed when used in 'Feed The Goriya' room types.",
	"When wielded in 'Potion Shop' room types, activates the shop and all other Potion Shops in the quest. Is overridden by Potions if you have them.",
	"When wielded, the Hero regains hearts and/or magic. Can also cure jinxes, depending on the Quest Rules.",
	"When wielded, shoots damaging magic. The magic is affected by your current Magic Book item. Can also damage enemies by stabbing and slashing.",
	"Divides the damage that the Hero takes, and changes their palette. If a magic cost is set, the Hero loses magic when they takes damage, and the item is disabled without magic.",
	"Can provide infinite rupees to the Hero, or provide a regenerating supply of rupees. Typically also set to increase their Rupee max.",
	"Makes invisible enemies visible. Currently does not affect Ganon.",
	"When the Hero isn't wielding an item, this deflects or reflects enemy projectiles from the front.",
	"Required to wield the Arrow. This affects the speed of the arrow fired. The Action settings are not used.",
	"Allows the Hero to traverse Raft Paths. When at a Raft Branch combo flag, hold the arrow keys to decide which path the raft will take.",
	"Used to cross Water combos and certain combo types. If Four-Way > 1, the Hero can step sideways off the ladder.",
	"Affects the sprite and damage of the magic shot by the Wand. If 'Fire Magic' is set, a flame is created at the place where the magic stops.",
	"Provides unlimited keys in a specific dungeon level, or all dungeon levels up to a point. The Action settings are not used.",
	"Allows the Hero to push Heavy or Very Heavy push block combos. Can be limited to one push per screen. The Action settings are not used.",
	"Allows the Hero to swim in Water combos. The Power and Action settings are not used.",
	"Prevents damage from certain Damage combos. Can require magic to use, which is drained as the Hero touches the combos.",
	"When wielded, shoots a hook and chain that collects items and hurt enemies, before retracting back to the Hero. Can grab onto various combos.",
	"When wielded, restricts your vision and reveals certain combo flags, as well as hiding or showing certain layers on a screen.",
	"When wielded, pounds and breaks Walking Enemies' shields. It is used to perform the Quake Hammer and Super Quake techniques, too.",
	"When wielded, casts a spell which sends out a wide ring of flames from the Hero. The Hero is invincible while casting the spell.",
	"When wielded, teleports the Hero to the Continue screen of the current DMap.",
	"When wielded, casts a spell which surrounds the Hero with a magic shield that nullifies all damage taken until it expires.",
	"When wielded, places a bomb which explodes to momentarily hurt foes. Expends 1 bomb ammo. Remote bombs only explode when you press the button again after placing.",
	"Similar to Bomb, but has a much larger blast radius, and expends 1 super bomb ammo.",
	"When collected, freezes most enemies and makes the Hero invincible for a limited time.",
	"No built-in effect, but is typically set to increase your Key count by 1 when collected.",
	"No built-in effect, but is typically set to increase your maximum Magic by 32 when collected.",
	"When collected, enables the Triforce for the current dungeon level and plays a cutscene. May warp the Hero out using the current screen's Side Warp A.",
	"When collected, enables the Subscreen Map for the current dungeon level.",
	"When collected, enables the Compass for the current dungeon level.",
	"When collected, enables the Boss Key for the current dungeon level, letting the Hero unlock Boss Lock Blocks, Boss Chest combos and Boss doors.",
	"Can provide infinite arrow ammo to the Hero, or provide a regenerating supply of ammo. Typically also set to increase their arrow ammo max.",
	"When collected, increases the Level-Specific Key count for the current dungeon level. These keys are used in place of normal keys if possible.",
	"When wielded, creates one or more beams that circle the Hero. Beams can be dismissed by pressing the button again. Can optionally have wand-style stab and slash sprites.",
	"No built-in effect, but is typically set to increase your Rupee count when collected.",
	"No built-in effect, but is typically set to increase your Arrow ammo when collected.",
	"Flies around the screen at a certain speed. When collected, the Hero regains hearts and/or magic. Can also cure jinxes, depending on the Quest Rules.",
	"No built-in effect, but is typically set to increase your Magic when collected.",
	"No built-in effect, but is typically set to restore the Hero's hearts when collected.",
	"No built-in effect, but is typically set to increase the Hero's max. health when collected.",
	"When collected, increases the Hero's Heart Piece count by 1. If the 'Per HC' amount (in Init Data) is reached, the Hero's max. health is increased by 1 heart (as specified by the lowest-ID 'Heart Container' item.",
	"When collected, all beatable enemies on the screen are instantly and silently killed.",
	"No built-in effect, but is typically set to increase the Hero's bomb ammo when collected.",
	"Can provide infinite bomb ammo to the Hero, or provide a regenerating supply of ammo. Typically also set to increase their bomb ammo max.",
	"When wielded, the Hero jumps into the air through the 'Z-axis', with an initial Jump speed of 0.8 times the Height Multiplier.",
	"When the Hero is at the apex of a jump, they hovers in the air for a specified time. In sideview, can be dismissed by pressing Down.",
	"When wielding the Sword, the Hero can hold the button to tap solid combos to find bombable locations and release to spin the sword around him.",
	"When performing a Spin Attack with a sword that can fire beams, four beams are released from the sword during each spin.",
	"When wielding the Hammer, the Hero can hold the button and release to pound for extra damage, and stunning most nearby ground enemies.",
	"Reduces the duration of jinxes given by certain enemies, or (if Divisor is 0) prevents them entirely. The Action settings are not used.",
	"Reduces the time it takes to charge the Spin Attack/Quake Hammer (Charging) and Hurricane Spin/Super Quake (Magic C.) abilities.",
	"Enables the Sword to fire sword beams when the Hero's health is below a certain amount. The Action settings are not used.",
	"In Shop room types, shop prices are multiplied by the Discount Amount, making the items cheaper.",
	"Gradually restores the Hero's health in certain quantities over a certain duration. The Action settings are not used.",
	"Gradually restores the Hero's magic in certain quantities over a certain duration. Can also provide infinite magic to the Hero. The Action settings are unused.",
	"After charging the Spin Attack, holding down the button longer enables a stronger attack with faster and more numerous spins.",
	"After charging the Quake Hammer, holding down the button longer enables a stronger attack which stuns more enemies for longer.",
	"The Hero's sprite faintly vibrates when they stands on or near secret-triggering combo flags. Sensitivity increases the distance at which it works.",
	"If the Hero, while jumping, lands on an enemy, that enemy takes a certain amount of damage, instead of damaging the Hero.",
	"The Sword, Wand, and Hammer will occasionally do increased damage in a single strike.",
	"Divides the damage that the Hero takes when their health is below a certain level.",
	"These items have no built-in effect. They will not be dropped in an Item Drop Set.",
	//ic67 to 86, custom IC
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", //ic87
	"Displays the bow and arrow together as a single item. No item should use this class; it is intended for use in subscreens only.",
	"Represents either the letter or a potion, whichever is available at the moment. No item should use this class; It is intended for use in subscreens only.",
	"This item class is reserved for future versions of ZC", //ic_last, 89
	//90
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "",

	//256 == script01
	"This item class is used to create LW_SCRIPT_01 type lweapons with properties defined by the Item Editor.",
	//257 == script02
	"This item class is used to create LW_SCRIPT_02 type lweapons with properties defined by the Item Editor.",
	//258 == script03
	"This item class is used to create LW_SCRIPT_03 type lweapons with properties defined by the Item Editor.",
	//258 == script04
	"This item class is used to create LW_SCRIPT_04 type lweapons with properties defined by the Item Editor.",
	//259 == script05
	"This item class is used to create LW_SCRIPT_05 type lweapons with properties defined by the Item Editor.",
	//260 == script06
	"This item class is used to create LW_SCRIPT_06 type lweapons with properties defined by the Item Editor.",
	//261 == script07
	"This item class is used to create LW_SCRIPT_07 type lweapons with properties defined by the Item Editor.",
	//262 == script08
	"This item class is used to create LW_SCRIPT_08 type lweapons with properties defined by the Item Editor.",
	//263 == script09
	"This item class is used to create LW_SCRIPT_09 type lweapons with properties defined by the Item Editor.",
	//264 == script10
	"This item class is used to create LW_SCRIPT_10 type lweapons with properties defined by the Item Editor.",
	//265
	"This item class is reserved for the Ice Rod in a future version of ZC", //icerod
	"The Sword, Wand, and Hammer deal more damage. The multiplier applies before the bonus damage.", //Atkring
	"Emanates light passively from the Hero in dark rooms.", //Lantern
	"Prevents the Hero from becoming a Bunny on certain dmaps.", //Pearls
	"Can be filled with bottle contents, and trigger their effects.\n"
		"The drawn tile of the bottle's animation is offset by its AFrames (min 1)"
		" times the value of its' contents (empty = 0, so no offset).", //Bottle
	"Fills the first empty bottle you have with a particular content", //Bottlefill
	//270
	"Can be swung, does not collide with enemies. Can catch fairies to put in bottles.", //Bugnet
	"Either warps to the continue point or warps to another dmap at the same screen location,"
		" depending on dmap settings. May or may not leave a return portal."
		"\nSet the DMap Flag 'Mirror Continues instead of Warping' to make the mirror return"
		" the Hero to their continue point on that dmap."
		"\nOtherwise, set the 'Mirror DMap' to the ID of the dmap you wish to warp to.", //Mirrors
	"Acts similarly to the hookshot, but swaps the Hero with certain blocks, or enemies.",
	"When collected, gives the Hero a number of other items together.",
	"Displays itself graphically as the next item not owned in a set of items."
		" Acts as picking up that item when collected.\n\nIf an item increases"
		" the max of a counter, but won't be able to due to the 'But Not Above...'"
		" field, it is considered 'owned', in addition to any 'Equipment Item'"
		" that has been collected.\nHeart pieces check the counter max increase"
		" of the heart container they grant.", //Progressive
	"When used, displays a string onscreen. Especially useful when combined"
		" with string control codes.", //Note
	"When used, can refill up to 5 counters, and optionally cure sword jinxes.",
	"When used, attempts to lift something in front of the Hero, OR throws the"
		" already lifted object.", // Lift Glove
	"Modifies the player's gravity while owned.", // Gravity Boots
	"Modifies the player's gravity when holding 'Up' in sideview.", // Gravity Up Boots
	"Modifies the player's gravity when holding 'Down' in sideview.", // Gravity Down Boots
	"Modifies the cooldown of all items with cooldowns.", // Cooldown Ring
};
const char default_ctype_strings[cMAX][255] = 
{
	"(None)", "Stairs [A]", "Cave (Walk Down) [A]", "Liquid", "Armos",
	"Grave", "Dock", "-UNDEF", "Push (Wait)", "Push (Heavy)",
	"Push (Heavy, Wait)", "Left Statue", "Right Statue", "Slow Walk", "Conveyor Up",
	"Conveyor Down", "Conveyor Left", "Conveyor Right", "Swim Warp [A]", "Dive Warp [A]",
	"Ladder or Hookshot", "Step->Secrets (Temporary)", "Step->Secrets (Permanent)", "-WINGAME", "Slash",
	"Slash (Item)", "Push (Very Heavy)", "Push (Very Heavy, Wait)", "Pound", "Hookshot Grab",
	"-HSBRIDGE", "Damage (1/2 Heart)", "Damage (1 Heart)", "Damage (2 hearts)", "Damage (4 Hearts)",
	"Center Statue", "Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (4-Way)", "Trap (Horizontal, Constant)",
	"Trap (Vertical, Constant)", "Direct Warp [A]", "Hookshot Only", "Overhead", "No Flying Enemies",
	"Mirror (4-Way)", "Mirror (Up-Left, Down-Right)", "Mirror (Up-Right, Down-Left)", "Magic Prism (3-Way)", "Magic Prism (4-Way)",
	"Block Magic", "Cave (Walk Up) [A]", "Eyeball (8-Way A)", "Eyeball (8-Way B)", "No Jumping Enemies",
	"Bush", "Flowers", "Tall Grass", "Shallow Liquid", "Lock Block (Basic)",
	"Lock Block (Basic, Copycat)", "Lock Block (Boss)", "Lock Block (Boss, Copycat)", "Ladder Only", "BS Grave",
	"Chest (Basic)", "Chest (Basic, Copycat)", "Chest (Locked)", "Chest (Locked, Copycat)", "Chest (Boss)",
	"Chest (Boss, Copycat)", "Reset Room", "Save Point", "Save-Quit Point", "Cave (Walk Down) [B]",
	"Cave (Walk Down) [C]", "Cave (Walk Down) [D]", "Stairs [B]", "Stairs [C]", "Stairs [D]",
	"Direct Warp [B]", "Direct Warp [C]", "Direct Warp [D]", "Cave (Walk Up) [B]", "Cave (Walk Up) [C]",
	"Cave (Walk Up) [D]", "Swim Warp [B]", "Swim Warp [C]", "Swim Warp [D]", "Dive Warp [B]",
	"Dive Warp [C]", "Dive Warp [D]", "Stairs [Random]", "Direct Warp [Random]", "Auto Side Warp [A]",
	"Auto Side Warp [B]","Auto Side Warp [C]","Auto Side Warp [D]","Auto Side Warp [Random]","Sensitive Warp [A]",
	"Sensitive Warp [B]","Sensitive Warp [C]","Sensitive Warp [D]","Sensitive Warp [Random]","Step->Secrets (Sensitive, Temp)",
	"Step->Secrets (Sensitive, Perm.)","Step->Next","Step->Next (Same)","Step->Next (All)","Step->Next (Copycat)",
	"No Enemies","Block Arrow (L1)","Block Arrow (L1, L2)","Block Arrow (All)","Block Brang (L1)",
	"Block Brang (L1, L2)","Block Brang (All)","Block Sword Beam","Block All","Block Fireball",
	"Damage (8 hearts)","Damage (16 hearts)","Damage (32 hearts)","-Unused","Spinning Tile (Immediate)",
	"-Unused","Screen Freeze (Except FFCs)","Screen Freeze (FFCs Only)","No Ground Enemies","Slash->Next",
	"Slash->Next (Item)","Bush->Next","Slash (Continuous)","Slash (Item, Continuous)","Bush (Continuous)",
	"Flowers (Continuous)","Tall Grass (Continuous)","Slash->Next (Continuous)","Slash->Next (Item, Continuous)","Bush->Next (Continuous)",
	"Eyeball (4-Way)","Tall Grass->Next","Script 01","Script 02","Script 03",
	"Script 04","Script 05","Script 06","Script 07","Script 08",
	"Script 09","Script 10","Script 11","Script 12","Script 13",
	"Script 14","Script 15","Script 16","Script 17","Script 18",
	"Script 19", "Script 20", "Generic", "Pitfall", "Step->Effects",
	"Bridge", "Signpost", "Switch", "Switch Block", "Torch",
	"Spotlight", "Glass", "Light Trigger", "SwitchHook Block", "ButtonPrompt",
	"Block Weapon (Custom)", "Shooter", "Slope", "Cutscene Trigger", "Push (Generic)",
	"Icy Floor", "Mirror (Custom)", "Crumbling", "Cutscene Effects"
};
const char old_mapflag_strings[mfMAX][255] =
{
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "",
	"Pit or Hole (Scripted)", "Pit or Hole, Fall Down Floor (Scripted)", "Fire or Lava (Scripted)",
	"Ice (Scripted)", "Ice, Damaging (Scripted)", "Damage-1 (Scripted)", "Damage-2 (Scripted)",
	"Damage-4 (Scripted)","Damage-8 (Scripted)", "Damage-16 (Scripted)", "Damage-32 (Scripted)",
	"", "", "", "", "", "", "", "", "", "", "", "", "",
	"Dig Spot (Scripted)", "Dig Spot, Next (Scripted)", "Dig Spot, Special Item (Scripted)",
	"Pot, Slashable (Scripted)", "Pot, Liftable (Scripted)", "Pot, Slash or Lift (Scripted)",
	"Rock, Lift Normal (Scripted)", "Rock, Lift Heavy (Scripted)", "Dropset Item (Scripted)",
	"Special Item (Scripted)", "Drop Key (Scripted)", "Drop level-Specific Key (Scripted)",
	"Drop Compass (Scripted)", "Drop Map (Scripted)", "Drop Bosskey (Scripted)",
	"Spawn NPC (Scripted)", "SwitchHook Spot (Scripted)"
};
const char map_flag_default_string[mfMAX][255] =
{
	"(None)", "Push Block (Vertical, Trigger)", "Push Block (4-Way, Trigger)", "Whistle Trigger", "Burn Trigger (Any)", "Arrow Trigger (Any)", "Bomb Trigger (Any)", "Fairy Ring (Life)",
	"Raft Path", "Armos -> Secret", "Armos/Chest -> Item", "Bomb (Super)", "Raft Branch", "Dive -> Item", "Lens Marker", "Win Game",
	"Secret Tile 00", "Secret Tile 01", "Secret Tile 02", "Secret Tile 03", "Secret Tile 04", "Secret Tile 05", "Secret Tile 06", "Secret Tile 07",
	"Secret Tile 08", "Secret Tile 09", "Secret Tile 10", "Secret Tile 11", "Secret Tile 12", "Secret Tile 13", "Secret Tile 14", "Secret Tile 15",
	"Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (4-Way, Line of Sight)", "Trap (Horizontal, Constant)", "Trap (Vertical, Constant)", "Enemy 0", "Enemy 1", "Enemy 2",
	"Enemy 3", "Enemy 4", "Enemy 5", "Enemy 6", "Enemy 7", "Enemy 8", "Enemy 9", "Push Block (Horiz, Once, Trigger)",
	"Push Block (Up, Once, Trigger)", "Push Block (Down, Once, Trigger)", "Push Block (Left, Once, Trigger)", "Push Block (Right, Once, Trigger)", "Push Block (Vert, Once)", "Push Block (Horizontal, Once)", "Push Block (4-Way, Once)", "Push Block (Up, Once)",
	"Push Block (Down, Once)", "Push Block (Left, Once)", "Push Block (Right, Once)", "Push Block (Vertical, Many)", "Push Block (Horizontal, Many)", "Push Block (4-Way, Many)", "Push Block (Up, Many)", "Push Block (Down, Many)",
	"Push Block (Left, Many)", "Push Block (Right, Many)", "Block Trigger", "No Push Blocks", "Boomerang Trigger (Any)", "Boomerang Trigger (Magic +)", "Boomerang Trigger (Fire)", "Arrow Trigger (Silver +)",
	"Arrow Trigger (Golden)", "Burn Trigger (Strong)", "Burn Trigger (Magic)", "Burn Trigger (Divine)", "Magic Trigger (Wand)", "Magic Trigger (Reflected)", "Fireball Trigger (Reflected)", "Sword Trigger (Any)",
	"Sword Trigger (White +)", "Sword Trigger (Magic +)", "Sword Trigger (Master)", "Sword Beam Trigger (Any)", "Sword Beam Trigger (White +)", "Sword Beam Trigger (Magic +)", "Sword Beam Trigger (Master)", "Hookshot Trigger",
	"Wand Trigger", "Hammer Trigger", "Strike Trigger", "Block Hole (Block -> Next)", "Fairy Ring (Magic)", "Fairy Ring (All)", "Trigger -> Self Only", "Trigger -> Self, Secret Tiles",
	"No Enemies", "No Ground Enemies", "Script 01", "Script 02", "Script 03", "Script 04", "Script 05", "Raft Bounce",
	 "Pushed", "Script 06", "Script 07", "Script 08", "Script 09", "Script 10", "Script 11", "Script 12",
	"Script 13", "Script 14", "Script 15", "Script 16", "Script 17", "Script 18", "Script 19", "Script 20",
	"Script 21", "Script 22", "Script 23", "Script 24", "Script 25", "Script 26", "Script 27", "Script 28",
	"Script 29", "Script 30", "Script 31", "-Freeze Screen (Unimplemented)", "-Freeze Screen, Except FFCs (Unimplemented)", "-Freeze FFCs Only (Unimplemented)", "-Trigger LW_SCRIPT1 (Unimplemented)", "-Trigger LW_SCRIPT2 (Unimplemented)",
	"-Trigger LW_SCRIPT3 (Unimplemented)", "-Trigger LW_SCRIPT4 (Unimplemented)", "-Trigger LW_SCRIPT5 (Unimplemented)", "-Trigger LW_SCRIPT6 (Unimplemented)", "-Trigger LW_SCRIPT7 (Unimplemented)", "-Trigger LW_SCRIPT8 (Unimplemented)", "-Trigger LW_SCRIPT9 (Unimplemented)", "-Trigger LW_SCRIPT10 (Unimplemented)",
	"Script 32", "Script 33", "Script 34", "Script 35", "Script 36", "Script 37", "Script 38", "Script 39",
	"Script 40", "Script 41", "Script 42", "Script 43", "Script 44", "Script 45", "Script 46", "Script 47",
	"Script 48", "Sideview Ladder", "Sideview Platform","Spawn No Enemies","Spawn All Enemies","Secrets->Next","No Mirroring","Unsafe Ground","-mf168","-mf169", "-mf170","-mf171","-mf172","-mf173","-mf174","-mf175","-mf176","-mf177","-mf178","-mf179",
	"-mf180","-mf181","-mf182","-mf183","-mf184","-mf185","-mf186","-mf187","-mf188","-mf189", "-mf190","-mf191","-mf192","-mf193","-mf194","-mf195","-mf196","-mf197","-mf198","-mf199",
	"-mf200","-mf201","-mf202","-mf203","-mf204","-mf205","-mf206","-mf207","-mf208","-mf209", "-mf210","-mf211","-mf212","-mf213","-mf214","-mf215","-mf216","-mf217","-mf218","-mf219",
	"-mf220","-mf221","-mf222","-mf223","-mf224","-mf225","-mf226","-mf227","-mf228","-mf229", "-mf230","-mf231","-mf232","-mf233","-mf234","-mf235","-mf236","-mf237","-mf238","-mf239",
	"-mf240","-mf241","-mf242","-mf243","-mf244","-mf245","-mf246","-mf247","-mf248","-mf249", "-mf250","-mf251","-mf252","-mf253","-mf254",
	"-Extended (Extended Flag Editor)"
};
const char weap_name_default_string[wMax][255] =
{
	"(None)","Sword","Sword Beam","Boomerang","Bomb Blast",
	"Super Bomb Blast","Bomb","Super Bomb","Arrow","Fire",
	//10
	"Whistle","Bait","-Melee Handle","Magic","-Catching",
	"Wind","Reflected Magic","Reflected Fireball","Reflected Rock","Hammer",
	//20
	"Hookshot","-Hookshot Handle","-Hookshot Chain","Sparkle","Fire Sparkle",
	"-Smack","-Phantom","Byrna Beam","Reflected Beam","Stomp Boots",
	//30
	"-lwMax","Custom Weapon 1","Custom Weapon 2","Custom Weapon 3","Custom Weapon 4",
	"Custom Weapon 5","Custom Weapon 6","Custom Weapon 7","Custom Weapon 8","Custom Weapon 9",
	//40
	"Custom Weapon 10","-Ice","-Flame","-Sound","Thrown",
	"-Pot","-Lit","-Med1","-Med2","-Med3",
	//50
	"-Sword180","-SwordLA","-Bug Net","Reflected Arrow","Reflected Fire 1",
	"Reflected Fire 2","","","","",
	//60
	"","","","","","","","","","",
	//70
	"","","","","","","","","","",
	//80
	"","","","","","","","","","",
	//90
	"","","","","","","","","","",
	//100
	"","","","","","","","","","",
	//110
	"","","","","","","","","","",
	//120
	"","","","","",
	"","","","-wEnemyWeapons","Fireball",
	//130
	"Arrow","Boomerang","Sword","Rock","Magic",
	"Bomb Blast","Super Bomb Blast","Bomb","Super Bomb","Fire Trail",
	//140
	"Fire 1","Wind","Fire 2","-Fire Trail 2","-Ice",
	"Fireball (Rising)"
	//wMax
};
const char default_itype_strings[itype_max][255] = 
{ 
	"Swords", "Boomerangs", "Arrows", "Candles", "Whistles",
	"Bait", "Letters", "Potions", "Wands", "Rings", 
	"Wallets", "Amulets", "Shields", "Bows", "Rafts",
	"Ladders", "Books", "Magic Keys", "Bracelets", "Flippers", 
	"Boots", "Hookshots", "Lenses", "Hammers", "Divine Fire", 
	"Divine Escape", "Divine Protection", "Bombs", "Super Bombs", "Clocks", 
	"Keys", "Magic Containers", "Triforce Pieces", "Maps", "Compasses", 
	"Boss Keys", "Quivers", "Level Keys", "Canes of Byrna", "Rupees", 
	"Arrow Ammo", "Fairies", "Magic", "Hearts", "Heart Containers", 
	"Heart Pieces", "Kill All Enemies", "Bomb Ammo", "Bomb Bags", "Roc Items", 
	"Hover Boots", "Scroll: Spin Attack", "Scroll: Cross Beams", "Scroll: Quake Hammer","Whisp Rings", 
	"Charge Rings", "Scroll: Peril Beam", "Wealth Medals", "Heart Rings", "Magic Rings", 
	"Scroll: Hurricane Spin", "Scroll: Super Quake","Stones of Agony", "Stomp Boots", "Whimsical Rings", 
	"Peril Rings", "Non-gameplay Items", "zz067", "zz068", "zz069",
	"zz070", "zz071", "zz072", "zz073", "zz074",
	"zz075", "zz076", "zz077", "zz078", "zz079",
	"zz080", "zz081", "zz082", "zz083", "zz084",
	"zz085", "zz086", "Bow and Arrow (Subscreen Only)", "Letter or Potion (Subscreen Only)",
	"zz089", "zz090", "zz091", "zz092", "zz093", "zz094", "zz095", "zz096",
	"zz097", "zz098", "zz099", "zz100", "zz101", "zz102", "zz103", "zz104",
	"zz105", "zz106", "zz107", "zz108", "zz109", "zz110", "zz111", "zz112",
	"zz113", "zz114", "zz115", "zz116", "zz117", "zz118", "zz119", "zz120",
	"zz121", "zz122", "zz123", "zz124", "zz125", "zz126", "zz127", "zz128",
	"zz129", "zz130", "zz131", "zz132", "zz133", "zz134", "zz135", "zz136",
	"zz137", "zz138", "zz139", "zz140", "zz141", "zz142", "zz143", "zz144",
	"zz145", "zz146", "zz147", "zz148", "zz149", "zz150", "zz151", "zz152",
	"zz153", "zz154", "zz155", "zz156", "zz157", "zz158", "zz159", "zz160",
	"zz161", "zz162", "zz163", "zz164", "zz165", "zz166", "zz167", "zz168",
	"zz169", "zz170", "zz171", "zz172", "zz173", "zz174", "zz175", "zz176",
	"zz177", "zz178", "zz179", "zz180", "zz181", "zz182", "zz183", "zz184",
	"zz185", "zz186", "zz187", "zz188", "zz189", "zz190", "zz191", "zz192",
	"zz193", "zz194", "zz195", "zz196", "zz197", "zz198", "zz199", "zz200",
	"zz201", "zz202", "zz203", "zz204", "zz205", "zz206", "zz207", "zz208",
	"zz209", "zz210", "zz211", "zz212", "zz213", "zz214", "zz215", "zz216",
	"zz217", "zz218", "zz219", "zz220", "zz221", "zz222", "zz223", "zz224",
	"zz225", "zz226", "zz227", "zz228", "zz229", "zz230", "zz231", "zz232",
	"zz233", "zz234", "zz235", "zz236", "zz237", "zz238", "zz239", "zz240",
	"zz241", "zz242", "zz243", "zz244", "zz245", "zz246", "zz247", "zz248",
	"zz249", "zz250", "zz251", "zz252", "zz253", "zz254", "zz255",
	"Custom Weapon 01", "Custom Weapon 02", "Custom Weapon 03", "Custom Weapon 04", "Custom Weapon 05",
	"Custom Weapon 06", "Custom Weapon 07", "Custom Weapon 08", "Custom Weapon 09", "Custom Weapon 10",
	"Ice Rod", "Attack Ring", "Lanterns", "Pearls", "Bottles", "Bottle Fillers", "Bug Nets", "Mirrors",
	"SwitchHooks", "Item Bundle", "Progressive Item", "Note", "Refiller",
	"Lift Glove", "Gravity Boots", "Gravity Up Boots", "Gravity Down Boots",
	"Cooldown Ring"
};
const char counter_default_names[MAX_COUNTERS][255] =
{
	"Life","Rupees","Bombs","Arrows","Magic",
	"Keys","Super Bombs","Custom 1","Custom 2","Custom 3",
	"Custom 4","Custom 5","Custom 6","Custom 7","Custom 8",
	"Custom 9","Custom 10","Custom 11","Custom 12","Custom 13",
	"Custom 14","Custom 15","Custom 16","Custom 17","Custom 18",
	"Custom 19","Custom 20","Custom 21","Custom 22","Custom 23",
	"Custom 24","Custom 25", "Custom 26", "Custom 27", "Custom 28",
	"Custom 29", "Custom 30", "Custom 31", "Custom 32", "Custom 33",
	"Custom 34", "Custom 35", "Custom 36", "Custom 37", "Custom 38",
	"Custom 39", "Custom 40", "Custom 41", "Custom 42", "Custom 43",
	"Custom 44", "Custom 45", "Custom 46", "Custom 47", "Custom 48",
	"Custom 49", "Custom 50", "Custom 51", "Custom 52", "Custom 53",
	"Custom 54", "Custom 55", "Custom 56", "Custom 57", "Custom 58",
	"Custom 59", "Custom 60", "Custom 61", "Custom 62", "Custom 63",
	"Custom 64", "Custom 65", "Custom 66", "Custom 67", "Custom 68",
	"Custom 69", "Custom 70", "Custom 71", "Custom 72", "Custom 73",
	"Custom 74", "Custom 75", "Custom 76", "Custom 77", "Custom 78",
	"Custom 79", "Custom 80", "Custom 81", "Custom 82", "Custom 83",
	"Custom 84", "Custom 85", "Custom 86", "Custom 87", "Custom 88",
	"Custom 89", "Custom 90", "Custom 91", "Custom 92", "Custom 93",
	"Custom 94", "Custom 95", "Custom 96", "Custom 97", "Custom 98",
	"Custom 99", "Custom 100"
};
const char etype_default_names[eeMAX][255] =
{
	"-Guy", "Walking Enemy", "-DEPRECATED", "Tektite", "Leever", "Peahat", "Zora", "Rock", "Ghini", "-DEPRECATED",
	"Keese", "-DEPRECATED", "-DEPRECATED", "-DEPRECATED", "-DEPRECATED", "Trap", "Wall Master", "-DEPRECATED", "-DEPRECATED", "-DEPRECATED",
	"-DEPRECATED", "Wizzrobe", "Aquamentus", "Moldorm", "Dodongo", "Manhandla", "Gleeok", "Digdogger", "Ghoma", "Lanmola", "Patra",
	"Ganon", "Projectile Shooter", "-DEPRECATED", "-DEPRECATED", "-DEPRECATED", "-DEPRECATED", "Spin Tile", "(None)", "-Fairy", "Other (Floating)",
	"Other", "-OLDMAX",
	"Custom 01","Custom 02","Custom 03","Custom 04","Custom 05","Custom 06","Custom 07","Custom 08","Custom 09","Custom 10",
	"Custom 11","Custom 12","Custom 13","Custom 14","Custom 15","Custom 16","Custom 17","Custom 18","Custom 19","Custom 20",
	"Friendly NPC 01","Friendly NPC 02","Friendly NPC 03","Friendly NPC 04","Friendly NPC 05",
	"Friendly NPC 06","Friendly NPC 07","Friendly NPC 08","Friendly NPC 09","Friendly NPC 10"
};

const char litem_default_names[li_max][255] =
{
	"McGuffin", "Map", "Compass", "Boss Killed", "Boss Key", "Custom 01", "Custom 02", "Custom 03",
	"Custom 04", "Custom 05", "Custom 06", "Custom 07", "Custom 08", "Custom 09", "Custom 10", "Custom 11",
};
const char litem_default_abbrs[li_max][255] =
{
	"McG", "Map", "Comp", "Boss", "BKey", "C1", "C2", "C3",
	"C4", "C5", "C6", "C7", "C8", "C9", "C10", "C11",
};
const char litem_default_helps[li_max][255] =
{
	"The main collectable of the game.",
	"The map, to help find your way.",
	"The compass, to guide you to your destination.",
	"The boss has been defeated.",
	"The big key to open the way.",
	"", "", "", "", "", "", "", "", "", "", "",
};

void assignchar(char** p, char const* str)
{
	if(*p) free(*p);
	if(!str)
	{
		*p = nullptr;
		return;
	}
	size_t len = strlen(str);
	*p = (char*)malloc(len+1);
	memcpy(*p, str, len);
	(*p)[len] = 0;
}
zinfo ZI;

zinfo::zinfo()
{
	memset(ic_help_string, 0, sizeof(ic_help_string));
	memset(ctype_name, 0, sizeof(ctype_name));
	memset(ctype_help_string, 0, sizeof(ctype_help_string));
	memset(mf_name, 0, sizeof(mf_name));
	memset(mf_help_string, 0, sizeof(mf_help_string));
	memset(ic_name, 0, sizeof(ic_name));
	memset(ctr_name, 0, sizeof(ctr_name));
	memset(weap_name, 0, sizeof(weap_name));
	memset(etype_name, 0, sizeof(etype_name));
	memset(litem_name, 0, sizeof(litem_name));
	memset(litem_help_string, 0, sizeof(litem_help_string));
	memset(litem_abbr, 0, sizeof(litem_abbr));
}

void zinfo::clear_ic_help()
{
	for(auto q = 0; q < itype_max; ++q)
	{
		assignchar(ic_help_string+q,nullptr);
	}
}
void zinfo::clear_ic_name()
{
	for(auto q = 0; q < itype_max; ++q)
	{
		assignchar(ic_name+q,nullptr);
	}
}
void zinfo::clear_ctype_name()
{
	for(auto q = 0; q < cMAX; ++q)
	{
		assignchar(ctype_name+q,nullptr);
	}
}
void zinfo::clear_ctype_help()
{
	for(auto q = 0; q < cMAX; ++q)
	{
		assignchar(ctype_help_string+q,nullptr);
	}
}
void zinfo::clear_mf_name()
{
	for(auto q = 0; q < mfMAX; ++q)
	{
		assignchar(mf_name+q,nullptr);
	}
}
void zinfo::clear_mf_help()
{
	for(auto q = 0; q < mfMAX; ++q)
	{
		assignchar(mf_help_string+q,nullptr);
	}
}
void zinfo::clear_weap_name()
{
	for(auto q = 0; q < wMax; ++q)
	{
		assignchar(weap_name+q,nullptr);
	}
}
void zinfo::clear_ctr_name()
{
	for(auto q = 0; q < MAX_COUNTERS; ++q)
	{
		assignchar(ctr_name+q,nullptr);
	}
}
void zinfo::clear_etype_name()
{
	for (auto q = 0; q < eeMAX; ++q)
	{
		assignchar(etype_name+q, nullptr);
	}
}
void zinfo::clear_li_name()
{
	for (auto q = 0; q < li_max; ++q)
	{
		assignchar(litem_name+q, nullptr);
	}
}
void zinfo::clear_li_help()
{
	for (auto q = 0; q < li_max; ++q)
	{
		assignchar(litem_help_string+q, nullptr);
	}
}
void zinfo::clear_li_abbr()
{
	for (auto q = 0; q < li_max; ++q)
	{
		assignchar(litem_abbr+q, nullptr);
	}
}
void zinfo::clear()
{
	clear_ic_help();
	clear_ic_name();
	clear_ctype_name();
	clear_ctype_help();
	clear_mf_name();
	clear_mf_help();
	clear_weap_name();
	clear_ctr_name();
	clear_etype_name();
	clear_li_name();
	clear_li_help();
	clear_li_abbr();
}

static char const* nilptr = "";
static char zinfbuf[2048] = {0};
bool zinfo::isUsableItemclass(size_t q)
{
	return valid_str(default_itype_strings[q],'-');
}
bool zinfo::isUsableComboType(size_t q)
{
	return valid_str(default_ctype_strings[q],'-');
}
bool zinfo::isUsableMapFlag(size_t q)
{
	return valid_str(map_flag_default_string[q],'-');
}
bool zinfo::isUsableWeap(size_t q)
{
	return valid_str(weap_name_default_string[q],'-');
}
bool zinfo::isUsableCtr(int32_t q)
{
	return q >= crNONE && q < MAX_COUNTERS;
	//return valid_str(counter_default_names[q+1],'-');
}
bool zinfo::isUsableEnemyType(size_t q)
{
	return valid_str(etype_default_names[q], '-');
}
char const* zinfo::getItemClassName(size_t q)
{
	if(valid_str(ic_name[q]))
		return ic_name[q];
	if(valid_str(default_itype_strings[q]))
		return default_itype_strings[q];
	sprintf(zinfbuf, "-zz%03zu", q);
	return zinfbuf;
}
char const* zinfo::getItemClassHelp(size_t q)
{
	if(valid_str(ic_help_string[q]))
		return ic_help_string[q];
	if(valid_str(itemclass_help_string_defaults[q]))
		return itemclass_help_string_defaults[q];
	return nilptr;
}
char const* zinfo::getComboTypeName(size_t q)
{
	if(valid_str(ctype_name[q]))
		return ctype_name[q];
	if(valid_str(default_ctype_strings[q]))
		return default_ctype_strings[q];
	return nilptr;
}
static std::string ctype_help_buff;
char const* zinfo::getComboTypeHelp(size_t q)
{
	if(valid_str(ctype_help_string[q]))
		return ctype_help_string[q];
	ctype_help_buff = getComboTypeHelpText(q);
	if(ctype_help_buff.size())
		return ctype_help_buff.c_str();
	return nilptr;
}
char const* zinfo::getMapFlagName(size_t q)
{
	if(valid_str(mf_name[q]))
		return mf_name[q];
	if(valid_str(map_flag_default_string[q]))
		return map_flag_default_string[q];
	return nilptr;
}
static std::string mf_help_buff;
char const* zinfo::getMapFlagHelp(size_t q)
{
	if(valid_str(mf_help_string[q]))
		return mf_help_string[q];
	mf_help_buff = getMapFlagHelpText(q);
	if(mf_help_buff.size())
		return mf_help_buff.c_str();
	return nilptr;
}
char const* zinfo::getWeapName(size_t q)
{
	if(valid_str(weap_name[q]))
		return weap_name[q];
	if(valid_str(weap_name_default_string[q]))
		return weap_name_default_string[q];
	return nilptr;
}
char const* zinfo::getCtrName(int32_t q)
{
	if(q == crNONE)
	{
		return "(None)";
	}
	if(valid_str(ctr_name[q]))
		return ctr_name[q];
	if(valid_str(counter_default_names[q]))
		return counter_default_names[q];
	return nilptr;
}
char const* zinfo::getEnemyTypeName(size_t q)
{
	if (valid_str(etype_name[q]))
		return etype_name[q];
	if (valid_str(etype_default_names[q]))
		return etype_default_names[q];
	return nilptr;
}
char const* zinfo::getLevelItemName(size_t q)
{
	if (valid_str(litem_name[q]))
		return litem_name[q];
	if (valid_str(litem_default_names[q]))
		return litem_default_names[q];
	return nilptr;
}
char const* zinfo::getLevelItemAbbr(size_t q)
{
	if (valid_str(litem_abbr[q]))
		return litem_abbr[q];
	if (valid_str(litem_default_abbrs[q]))
		return litem_default_abbrs[q];
	return nilptr;
}
char const* zinfo::getLevelItemHelp(size_t q)
{
	if (valid_str(litem_help_string[q]))
		return litem_help_string[q];
	if (valid_str(litem_default_helps[q]))
		return litem_default_helps[q];
	return nilptr;
}


void zinfo::copyFrom(zinfo const& other)
{
	clear();
	for(auto q = 0; q < MAX_COUNTERS; ++q)
	{
		assignchar(ctr_name+q, other.ctr_name[q]);
	}
	for(auto q = 0; q < itype_max; ++q)
	{
		assignchar(ic_name+q, other.ic_name[q]);
		assignchar(ic_help_string+q, other.ic_help_string[q]);
	}
	for(auto q = 0; q < cMAX; ++q)
	{
		assignchar(ctype_name+q, other.ctype_name[q]);
		assignchar(ctype_help_string+q, other.ctype_help_string[q]);
	}
	for(auto q = 0; q < mfMAX; ++q)
	{
		assignchar(mf_name+q, other.mf_name[q]);
		assignchar(mf_help_string+q, other.mf_help_string[q]);
	}
	for(auto q = 0; q < wMax; ++q)
	{
		assignchar(weap_name+q, other.weap_name[q]);
	}
	for (auto q = 0; q < eeMAX; ++q)
	{
		assignchar(etype_name+q, other.etype_name[q]);
	}
	for (auto q = 0; q < li_max; ++q)
	{
		assignchar(litem_name+q, other.litem_name[q]);
		assignchar(litem_help_string+q, other.litem_help_string[q]);
		assignchar(litem_abbr+q, other.litem_abbr[q]);
	}
}

int32_t writezinfo(PACKFILE *f, zinfo const& z)
{
	dword section_id=ID_ZINFO;
	dword section_version=V_ZINFO;
	dword section_size=0;
	
	//section id
	if(!p_mputl(section_id,f))
	{
		new_return(1);
	}
	
	//section version info
	if(!p_iputw(section_version,f))
	{
		new_return(2);
	}
	
	if(!write_deprecated_section_cversion(section_version,f))
	{
		new_return(3);
	}
	
	for(int32_t writecycle=0; writecycle<2; ++writecycle)
	{
		fake_pack_writing=(writecycle==0);
		
		//section size
		if(!p_iputl(section_size,f))
		{
			new_return(5);
		}
		
		writesize=0;
		
		if(!p_iputw(itype_max,f)) //num itemtypes
		{
			new_return(6);
		}
		for(auto q = 0; q < itype_max; ++q)
		{
			byte namesize = (byte)(vbound(valid_str(z.ic_name[q]) ? strlen(z.ic_name[q]) : 0,0,255));
			
			if(!p_putc(namesize,f))
			{
				new_return(7);
			}
			if(namesize)
				if(!pfwrite(z.ic_name[q],namesize,f))
					new_return(8);
			
			dword htxtsz = valid_str(z.ic_help_string[q]) ? strlen(z.ic_help_string[q]) : 0;
			
			if(!p_iputw(htxtsz,f))
			{
				new_return(9);
			}
			if(htxtsz)
				if(!pfwrite(z.ic_help_string[q],htxtsz,f))
					new_return(10);
		}
		
		if(!p_iputw(cMAX,f)) //num combotypes
		{
			new_return(11);
		}
		for(auto q = 0; q < cMAX; ++q)
		{
			byte namesize = (byte)(vbound(valid_str(z.ctype_name[q]) ? strlen(z.ctype_name[q]) : 0,0,255));
			
			if(!p_putc(namesize,f))
			{
				new_return(12);
			}
			if(namesize)
				if(!pfwrite(z.ctype_name[q],namesize,f))
					new_return(13);
			
			dword htxtsz = valid_str(z.ctype_help_string[q]) ? strlen(z.ctype_help_string[q]) : 0;
			
			if(!p_iputw(htxtsz,f))
			{
				new_return(14);
			}
			if(htxtsz)
				if(!pfwrite(z.ctype_help_string[q],htxtsz,f))
					new_return(15);
		}
		
		if(!p_iputw(mfMAX,f)) //num mapflags
		{
			new_return(16);
		}
		for(auto q = 0; q < mfMAX; ++q)
		{
			byte namesize = (byte)(vbound(valid_str(z.mf_name[q]) ? strlen(z.mf_name[q]) : 0,0,255));
			
			if(!p_putc(namesize,f))
			{
				new_return(17);
			}
			if(namesize)
				if(!pfwrite(z.mf_name[q],namesize,f))
					new_return(18);
			
			dword htxtsz = valid_str(z.mf_help_string[q]) ? strlen(z.mf_help_string[q]) : 0;
			
			if(!p_iputw(htxtsz,f))
			{
				new_return(19);
			}
			if(htxtsz)
				if(!pfwrite(z.mf_help_string[q],htxtsz,f))
					new_return(20);
		}
		
		if(!p_iputw(MAX_COUNTERS,f)) //num counters
		{
			new_return(21);
		}
		for(auto q = 0; q < MAX_COUNTERS; ++q)
		{
			byte namesize = (byte)(vbound(valid_str(z.ctr_name[q]) ? strlen(z.ctr_name[q]) : 0,0,255));
			
			if(!p_putc(namesize,f))
			{
				new_return(22);
			}
			if(namesize)
				if(!pfwrite(z.ctr_name[q],namesize,f))
					new_return(23);
		}
		
		if(!p_iputw(wMax,f)) //num weapons
		{
			new_return(21);
		}
		for(auto q = 0; q < wMax; ++q)
		{
			byte namesize = (byte)(vbound(valid_str(z.weap_name[q]) ? strlen(z.weap_name[q]) : 0,0,255));
			
			if(!p_putc(namesize,f))
			{
				new_return(22);
			}
			if(namesize)
				if(!pfwrite(z.weap_name[q],namesize,f))
					new_return(23);
		}

		if (!p_iputw(eeMAX, f)) //num enemy types
		{
			new_return(21);
		}
		for (auto q = 0; q < eeMAX; ++q)
		{
			byte namesize = (byte)(vbound(valid_str(z.etype_name[q]) ? strlen(z.etype_name[q]) : 0, 0, 255));

			if (!p_putc(namesize, f))
			{
				new_return(22);
			}
			if (namesize)
				if (!pfwrite(z.etype_name[q], namesize, f))
					new_return(23);
		}
		
		if (!p_putc(li_max, f)) //num litem types
		{
			new_return(24);
		}
		for (auto q = 0; q < li_max; ++q)
		{
			byte namesize = (byte)(vbound(valid_str(z.litem_name[q]) ? strlen(z.litem_name[q]) : 0, 0, 255));

			if (!p_putc(namesize, f))
			{
				new_return(25);
			}
			if (namesize)
				if (!pfwrite(z.litem_name[q], namesize, f))
					new_return(26);
			
			byte abbrsize = (byte)(vbound(valid_str(z.litem_abbr[q]) ? strlen(z.litem_abbr[q]) : 0, 0, 255));

			if (!p_putc(abbrsize, f))
			{
				new_return(27);
			}
			if (abbrsize)
				if (!pfwrite(z.litem_abbr[q], abbrsize, f))
					new_return(28);
			
			word htxtsz = (valid_str(z.litem_help_string[q]) ? strlen(z.litem_help_string[q]) : 0);

			if (!p_iputw(htxtsz, f))
			{
				new_return(29);
			}
			if (htxtsz)
				if (!pfwrite(z.litem_help_string[q], htxtsz, f))
					new_return(30);
		}
		
		if(writecycle==0)
		{
			section_size=writesize;
		}
	}
	
	if(writesize!=int32_t(section_size))
	{
		char ebuf[80];
		sprintf(ebuf, "%d != %d", writesize, int32_t(section_size));
		jwin_alert("Error:  writezinfo()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
	}
	
	new_return(0);
}

int32_t readzinfo(PACKFILE *f, zinfo& z, zquestheader const& hdr)
{
	int32_t dummy;
	word section_version;
	z.clear();
	if(!f)
		return 0;
	
	if(!p_mgetl(&dummy,f))
		return qe_invalid;
	
	//section version info
	if(!p_igetw(&section_version,f))
		return qe_invalid;

	if (section_version > V_ZINFO)
			return qe_version;

	if(!read_deprecated_section_cversion(f))
		return qe_invalid;
	
	if(!p_igetl(&dummy,f))
		return qe_invalid;
	
	word num_itemtypes = 512;
	if(section_version > 0)
		if(!p_igetw(&num_itemtypes,f))
			return qe_invalid;
	if (num_itemtypes > itype_max)
		return qe_invalid;
	for(auto q = 0; q < num_itemtypes; ++q)
	{
		byte namesize;
		if(!p_getc(&namesize,f))
			return qe_invalid;
		if(namesize)
		{
			z.ic_name[q] = (char*)malloc(namesize+1);
			if (!z.ic_name[q]) return qe_nomem;
			if(!pfread(z.ic_name[q],namesize,f))
				return qe_invalid;
			z.ic_name[q][namesize] = 0;
		}
		
		word htxtsz;
		if(!p_igetw(&htxtsz,f))
			return qe_invalid;
		if(htxtsz)
		{
			char* p = (char*)malloc(htxtsz+1);
			if (!p) return qe_nomem;
			if(!pfread(p,htxtsz,f))
				return qe_invalid;
			p[htxtsz] = 0;
			z.ic_help_string[q] = p;
		}
	}
	
	if(section_version > 0)
	{
		word num_combotypes;
		if(!p_igetw(&num_combotypes,f))
			return qe_invalid;
		if (num_combotypes > cMAX)
			return qe_invalid;
		for(auto q = 0; q < num_combotypes; ++q)
		{
			byte namesize;
			if(!p_getc(&namesize,f))
				return qe_invalid;
			if(namesize)
			{
				char* p = (char*)malloc(namesize+1);
				if(!pfread(p,namesize,f))
					return qe_invalid;
				p[namesize] = 0;
				z.ctype_name[q] = p;
			}
			
			word htxtsz;
			if(!p_igetw(&htxtsz,f))
				return qe_invalid;
			if(htxtsz)
			{
				char* p = (char*)malloc(htxtsz+1);
				if(!pfread(p,htxtsz,f))
					return qe_invalid;
				p[htxtsz] = 0;
				z.ctype_help_string[q] = p;
			}
		}
		
		word num_mapflags;
		if(!p_igetw(&num_mapflags,f))
			return qe_invalid;
		if (num_mapflags > mfMAX)
			return qe_invalid;
		for(auto q = 0; q < num_mapflags; ++q)
		{
			byte namesize;
			if(!p_getc(&namesize,f))
				return qe_invalid;
			if(namesize)
			{
				char* p = (char*)malloc(namesize+1);
				if(!pfread(p,namesize,f))
					return qe_invalid;
				p[namesize] = 0;
				z.mf_name[q] = p;
			}
			
			word htxtsz;
			if(!p_igetw(&htxtsz,f))
				return qe_invalid;
			if(htxtsz)
			{
				char* p = (char*)malloc(htxtsz+1);
				if (!p) return qe_nomem;
				if(!pfread(p,htxtsz,f))
					return qe_invalid;
				p[htxtsz] = 0;
				z.mf_help_string[q] = p;
			}
		}
	}
	else if (get_app_id() == App::zquest)
	{
		if(hdr.zelda_version == 0x255) //Old quest naming
			for(auto q = 0; q < mfMAX; ++q)
			{
				if(valid_str(old_mapflag_strings[q]))
				{
					byte namesize = (byte)(vbound(strlen(old_mapflag_strings[q]),0,255));
					z.mf_name[q] = (char*)malloc(namesize+1);
					if (!z.mf_name[q]) return qe_nomem;
					memcpy(z.mf_name[q], old_mapflag_strings[q], namesize);
					z.mf_name[q][namesize] = 0;
					
					char const* scrdesc = "These flags have no built-in effect,"
						" but can be given special significance with ZASM or ZScript.";
					namesize = (byte)strlen(scrdesc);
					z.mf_help_string[q] = (char*)malloc(namesize+1);
					if (!z.mf_help_string[q]) return qe_nomem;
					memcpy(z.mf_help_string[q], scrdesc, namesize);
					z.mf_help_string[q][namesize] = 0;
				}
			}
	}
	
	if(section_version > 1)
	{
		word num_counters;
		if(!p_igetw(&num_counters,f))
			return qe_invalid;
		if (num_counters > MAX_COUNTERS)
			return qe_invalid;
		for(auto q = 0; q < num_counters; ++q)
		{
			byte namesize;
			if(!p_getc(&namesize,f))
				return qe_invalid;
			if(namesize)
			{
				char* p = (char*)malloc(namesize+1);
				if (!p) return qe_nomem;
				if(!pfread(p,namesize,f))
					return qe_invalid;
				p[namesize] = 0;
				z.ctr_name[q] = p;
			}
		}
	}
	if(section_version > 2)
	{
		word num_wpns;
		if(!p_igetw(&num_wpns,f))
			return qe_invalid;
		if (num_wpns > MAXWPNS)
			return qe_invalid;
		for(auto q = 0; q < num_wpns; ++q)
		{
			byte namesize;
			if(!p_getc(&namesize,f))
				return qe_invalid;
			if(namesize)
			{
				char* p = (char*)malloc(namesize+1);
				if (!p) return qe_nomem;
				if(!pfread(p,namesize,f))
					return qe_invalid;
				p[namesize] = 0;
				z.weap_name[q] = p;
			}
		}
	}
	if(section_version > 3)
	{
		word num_etypes;
		if (!p_igetw(&num_etypes, f))
			return qe_invalid;
		if (num_etypes > eeMAX)
			return qe_invalid;
		for (auto q = 0; q < num_etypes; ++q)
		{
			byte namesize;
			if (!p_getc(&namesize, f))
				return qe_invalid;
			if (namesize)
			{
				char* p = (char*)malloc(namesize + 1);
				if (!p) return qe_nomem;
				if (!pfread(p, namesize, f))
					return qe_invalid;
				p[namesize] = 0;
				z.etype_name[q] = p;
			}
		}
	}
	if(section_version > 4)
	{
		byte num_litypes;
		if (!p_getc(&num_litypes, f))
			return qe_invalid;
		if (num_litypes > li_max)
			return qe_invalid;
		for (auto q = 0; q < num_litypes; ++q)
		{
			byte namesize;
			if (!p_getc(&namesize, f))
				return qe_invalid;
			if (namesize)
			{
				char* p = (char*)malloc(namesize + 1);
				if (!p) return qe_nomem;
				if (!pfread(p, namesize, f))
					return qe_invalid;
				p[namesize] = 0;
				z.litem_name[q] = p;
			}
			
			byte abbrsize;
			if (!p_getc(&abbrsize, f))
				return qe_invalid;
			if (abbrsize)
			{
				char* p = (char*)malloc(abbrsize + 1);
				if (!p) return qe_nomem;
				if (!pfread(p, abbrsize, f))
					return qe_invalid;
				p[abbrsize] = 0;
				z.litem_abbr[q] = p;
			}
			
			word htxtsz;
			if (!p_igetw(&htxtsz, f))
				return qe_invalid;
			if (htxtsz)
			{
				char* p = (char*)malloc(htxtsz + 1);
				if (!p) return qe_nomem;
				if (!pfread(p, htxtsz, f))
					return qe_invalid;
				p[htxtsz] = 0;
				z.litem_help_string[q] = p;
			}
		}
	}
	return 0;
}

bool zinfo::isNull()
{
	for(auto q = 0; q < itype_max; ++q)
	{
		if(ic_name[q]) return false;
		if(ic_help_string[q]) return false;
	}
	for(auto q = 0; q < cMAX; ++q)
	{
		if(ctype_name[q]) return false;
		if(ctype_help_string[q]) return false;
	}
	for(auto q = 0; q < mfMAX; ++q)
	{
		if(mf_name[q]) return false;
		if(mf_help_string[q]) return false;
	}
	for(auto q = 0; q < wMax; ++q)
		if(weap_name[q]) return false;
	for(auto q = 0; q < eeMAX; ++q)
		if(etype_name[q]) return false;
	for(auto q = 0; q < MAX_COUNTERS; ++q)
		if(ctr_name[q]) return false;
	for(auto q = 0; q < li_max; ++q)
	{
		if(litem_name[q]) return false;
		if(litem_abbr[q]) return false;
		if(litem_help_string[q]) return false;
	}
		
	return true;
}
