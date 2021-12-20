#include "zdefs.h"
#include "module.h"

const char default_itype_strings[itype_max][255] = 
{ 
	"Swords", "Boomerangs", "Arrows", "Candles", "Whistles",
	"Bait", "Letters", "Potions", "Wands", "Rings", 
	"Wallets", "Amulets", "Shields", "Bows", "Rafts",
	"Ladders", "Books", "Magic Keys", "Bracelets", "Flippers", 
	"Boots", "Hookshots", "Lenses", "Hammers", "Din's Fire", 
	"Farore's Wind", "Nayru's Love", "Bombs", "Super Bombs", "Clocks", 
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
	"SwitchHooks", "Item Bundle", "Progressive Item"
};

const char *itemclass_help_string_defaults[itype_max] =
{
	"Link's most versatile weapon. When wielded, it can stab, slash and fire sword beams. It is used to perform several Tiger Scroll techniques, too.",
	"When wielded, it flies out, hurting enemies, and collecting items, before returning to Link. Can be thrown diagonally. Can optionally drop damaging sparkles.",
	"When wielded, it flies and collects items before hitting an enemy. Requires the Bow. Expends either 1 rupee or 1 arrow ammo. Can drop damaging sparkles.",
	"When wielded, a damaging flame drifts out, which lights dark screens until it expires. Can optionally have wand-style stab and slash sprites.",
	"When wielded, plays strange music and summons a whirlwind that warps you to the Warp Ring locations. Can also dry Water combos on specific screens.",
	"When wielded, drops bait that attracts Walking Enemies depending on their Hunger stat. Removed when used in 'Feed The Goriya' room types.",
	"When wielded in 'Potion Shop' room types, activates the shop and all other Potion Shops in the quest. Is overridden by Potions if you have them.",
	"When wielded, Link regains hearts and/or magic. Can also cure jinxes, depending on the Quest Rules.",
	"When wielded, shoots damaging magic. The magic is affected by your current Magic Book item. Can also damage enemies by stabbing and slashing.",
	"Divides the damage that Link takes, and changes his palette. If a magic cost is set, Link loses magic when he takes damage, and the item is disabled without magic.",
	"Can provide infinite rupees to Link, or provide a regenerating supply of rupees. Typically also set to increase his Rupee max.",
	"Also called the Cross, this makes invisible enemies visible. Currently does not affect Ganon.",
	"When Link isn't wielding an item, this deflects or reflects enemy projectiles from the front. The Block Flags are listed in Shield Help.",
	"Required to wield the Arrow. This affects the speed of the arrow fired. The Action settings are not used.",
	"Allows Link to traverse Raft Paths. When at a Raft Branch combo flag, hold the arrow keys to decide which path the raft will take.",
	"Used to cross Water combos and certain combo types. If Four-Way > 1, Link can step sideways off the ladder.",
	"Affects the sprite and damage of the magic shot by the Wand. If 'Fire Magic' is set, a 1-damage flame is created at the place where the magic stops.",
	"Provides unlimited keys in a specific dungeon level, or all dungeon levels up to a point. The Action settings are not used.",
	"Allows Link to push Heavy or Very Heavy push block combos. Can be limited to one push per screen. The Action settings are not used.",
	"Allows Link to swim in Water combos. The Power and Action settings are not used.",
	"Prevents damage from certain Damage combos. Can require magic to use, which is drained as Link touches the combos.",
	"When wielded, shoots a hook and chain that collects items and hurt enemies, before retracting back to Link. Best used with the Hookshot Grab combo type.",
	"When wielded, restricts your vision and reveals certain combo flags, as well as hiding or showing certain layers on a screen.",
	"When wielded, pounds and breaks Walking Enemies' shields. It is used to perform the Quake Hammer and Super Quake techniques, too.",
	"When wielded, casts a spell which sends out a wide ring of flames from Link's body. Link is invincible while casting the spell.",
	"When wielded, teleports Link to the Continue screen of the current DMap.",
	"When wielded, casts a spell which surrounds Link with a magic shield that nullifies all damage taken until it expires.",
	"When wielded, places a bomb which explodes to momentarily hurt foes. Expends 1 bomb ammo. Remote bombs only explode when you press the button again after placing.",
	"Similar to Bomb, but has a much larger blast radius, and expends 1 super bomb ammo.",
	"When collected, freezes most enemies and makes Link invincible for a limited time.",
	"No built-in effect, but is typically set to increase your Key count by 1 when collected.",
	"No built-in effect, but is typically set to increase your maximum Magic by 32 when collected.",
	"When collected, enables the Triforce for the current dungeon level and plays a cutscene. May warp Link out using the current screen's Side Warp A.",
	"When collected, enables the Subscreen Map for the current dungeon level.",
	"When collected, enables the Compass for the current dungeon level.",
	"When collected, enables the Boss Key for the current dungeon level, letting Link unlock Boss Lock Blocks, Boss Chest combos and Boss doors.",
	"Can provide infinite arrow ammo to Link, or provide a regenerating supply of ammo. Typically also set to increase his arrow ammo max.",
	"When collected, increases the Level-Specific Key count for the current dungeon level. These keys are used in place of normal keys if possible.",
	"When wielded, creates one or more beams that circle Link. Beams can be dismissed by pressing the button again. Can optionally have wand-style stab and slash sprites.",
	"No built-in effect, but is typically set to increase your Rupee count when collected.",
	"No built-in effect, but is typically set to increase your Arrow ammo when collected.",
	"Flies around the screen at a certain speed. When collected, Link regains hearts and/or magic. Can also cure jinxes, depending on the Quest Rules.",
	"No built-in effect, but is typically set to increase your Magic when collected.",
	"No built-in effect, but is typically set to restore Link's hearts when collected.",
	"No built-in effect, but is typically set to increase Link's max. health when collected.",
	"When collected, increases Link's Heart Piece count by 1. If the 'Per HC' amount (in Init Data) is reached, Link's max. health is increased by 1 heart.",
	"When collected, all beatable enemies on the screen are instantly and silently killed.",
	"No built-in effect, but is typically set to increase Link's bomb ammo when collected.",
	"Can provide infinite bomb ammo to Link, or provide a regenerating supply of ammo. Typically also set to increase his bomb ammo max.",
	"When wielded, Link jumps into the air through the 'Z-axis', with an initial Jump speed of 0.8 times the Height Multiplier.",
	"When Link is at the apex of a jump, he hovers in the air for a specified time. In sideview, can be dismissed by pressing Down.",
	"When wielding the Sword, Link can hold the button to tap solid combos to find bombable locations and release to spin the sword around him.",
	"When performing a Spin Attack with a sword that can fire beams, four beams are released from the sword during each spin.",
	"When wielding the Hammer, Link can hold the button and release to pound for extra damage, and stunning most nearby ground enemies.",
	"Reduces the duration of jinxes given by certain enemies, or (if Divisor is 0) prevents them entirely. The Action settings are not used.",
	"Reduces the time it takes to charge the Spin Attack/Quake Hammer (Charging) and Hurricane Spin/Super Quake (Magic C.) abilities.",
	"Enables the Sword to fire sword beams when Link's health is below a certain amount. The Action settings are not used.",
	"In Shop room types, shop prices are multiplied by the Discount Amount, making the items cheaper.",
	"Gradually restores Link's health in certain quantities over a certain duration. The Action settings are not used.",
	"Gradually restores Link's magic in certain quantities over a certain duration. Can also provide infinite magic to Link. The Action settings are unused.",
	"After charging the Spin Attack, holding down the button longer enables a stronger attack with faster and more numerous spins.",
	"After charging the Quake Hammer, holding down the button longer enables a stronger attack which stuns more enemies for longer.",
	"Link's sprite faintly vibrates when he stands on or near secret-triggering combo flags. Sensitivity increases the distance at which it works.",
	"If Link, while jumping, lands on an enemy, that enemy takes a certain amount of damage, instead of damaging Link.",
	"The Sword, Wand and Hammer will occasionally do increased damage in a single strike.",
	"Divides the damage that Link takes when his health is below a certain level.",
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
	"The Sword, Wand and Hammer deal more damage. The multiplier applies before the bonus damage.", //Atkring
	"Emanates light passively from the player in dark rooms.", //Lantern
	"Prevents the player from becoming a Bunny on certain dmaps.", //Pearls
	"Can be filled with bottle contents, and trigger their effets", //Bottle
	"Fills the first empty bottle you have with a particular content", //Bottlefill
	//270
	"Can be swung, does not collide with enemies. Can catch fairies to put in bottles.", //Bugnet
	"Either warps to the continue point or warps to another dmap at the same screen location,"
		" depending on dmap settings. May or may not leave a return portal."
		"\nSet the DMap Flag 'Mirror Continues instead of Warping' to make the mirror return"
		" the player to their continue point on that dmap."
		"\nOtherwise, set the 'Mirror DMap' to the ID of the dmap you wish to warp to.", //Mirrors
	"Acts similarly to the hookshot, but swaps the player with certain blocks, or enemies.",
	"When collected, gives the player a number of other items together.",
	"Displays itself graphically as the next item not owned in"
		" a set of items. Acts as picking up that item when collected."
};

