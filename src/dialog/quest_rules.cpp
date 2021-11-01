#include "quest_rules.h"
#include "pickruleset.h"
#include "cheat_codes.h"
#include "headerdlg.h"
#include "info.h"
#include "alert.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"
#include "../zsys.h"
#include "gui/use_size.h"
using GUI::sized;

bool mapcount_will_affect_layers(word newmapcount);
void update_map_count(word newmapcount);

//{

static const GUI::ListData animRulesList
{
	{ "BS-Zelda Animation Quirks", qr_BSZELDA, "Affects a number of small miscellaneous stuff to make Z1 more accurate to BS Zelda. "
	"Guy fires have their positions adjusted, enemy spawn animations are faster, enemy and player death animations have more frames, "
	"wand magic moves 0.5 pixels per frame faster, weapon flashing is slightly different, boomerangs may animate different, and "
	"most weapons gain a 2 pixel Y offset when created. Having this enabled also deprecates the 'Fix Player's Position in Dungeons' "
	"quest rule, as this rule fixes that behavior as well. If this rule is disabled, Whistle Whirlwinds appear as flame for a single "
	"frame when hitting the edge of the screen."},
	{ "Circle Opening/Closing Wipes", qr_COOLSCROLL, "Changes the animation going to/from full black when entering caves or starting the game. "
	"If multiple are selected, it will be chosen randomly between the ones that are selected. If none are selected, the Z1 opening is used. "
	"This opening animation has a circle cut out from the black expand either outwards or inwards."},
	{ "Oval Opening/Closing Wipes", qr_OVALWIPE, "Changes the animation going to/from full black when entering caves or starting the game. "
	"If multiple are selected, it will be chosen randomly between the ones that are selected. If none are selected, the Z1 opening is used. "
	"This opening animation has an oval cut out from the black expand either outwards or inwards, recreating LTTP's opening and game over screen." },
	{ "Triangle Opening/Closing Wipes", qr_TRIANGLEWIPE, "Changes the animation going to/from full black when entering caves or starting the game. "
	"If multiple are selected, it will be chosen randomly between the ones that are selected. If none are selected, the Z1 opening is used. "
	"This opening animation has a rapidly spinning triangle cut out from black expand either outwards or inwards." },
	{ "Super Mario All-Stars Opening/Closing Wipes", qr_SMASWIPE, "Changes the animation going to/from full black when entering caves or starting the game. "
	"If multiple are selected, it will be chosen randomly between the ones that are selected. If none are selected, the Z1 opening is used. "
	"This opening animation recreates the Super Mario All Stars wipe; a bunch of mini triangles in a diamond pattern expanding and cutting out the black." },
	{ "Fade to/from Black Opening/Closing Wipes", qr_FADEBLACKWIPE, "Changes the animation going to/from full black when entering caves or starting the game. "
	"If multiple are selected, it will be chosen randomly between the ones that are selected. If none are selected, the Z1 opening is used. "
	"This opening animation smoothly fades the whole palette to and from black." },
	{ "No Screen Scrolling", qr_NOSCROLL, "Removes the scrolling animation that happens when normally changing screens; instead instantly warping you to the next screen." },
	{ "Fast Dungeon Scrolling", qr_FASTDNGN, "Prevents the speed of the scrolling animation when changing screens from being halved within dungeons. "
	"When disabled, the scrolling speed is a 4 frame initial delay, followed by 2 pixels per frame. If enabled, the initial delay is 2 frames, followed by 4 pixels per frame. "
	"Note that if 'Smooth Vertical Scrolling' is disabled, the speed when scrolling up and down will always be 8 pixels per few frames, regardless of this rule, "
	"*unless* 'Very Fast Scrolling' is enabled, in which case both 'Fast Dungeon Scrolling' and 'Smooth Vertical Scrolling' are completely ignored and the " 
	"scrolling speed will be 16 pixels per frame in all directions." },
	{ "Very Fast Scrolling", qr_VERYFASTSCROLLING, "Greatly increases Screen Scrolling speed, making the screen scrolling 16 pixels per frame in all directions when scrolling."},
	{ "Smooth Vertical Scrolling", qr_SMOOTHVERTICALSCROLLING, "If disabled, the screen scrolls 8 pixels every few frames going up and down, which can cause weird snapping issues."
	" If enabled, the same speed for scrolling left and right is used for scrolling up and down."},
	{ "FFCs Are Visible While The Screen Is Scrolling", qr_FFCSCROLL, "Freeform combos will display when scrolling the screen. If disabled, they won't be visible until after scrolling stops." },
	{ "Interpolated Fading", qr_FADE, "If enabled, dark rooms and cave/passage entering will automatically calculate the dark room colors. "
	" Enabling this also enables the usage of Palette Cycling, but you lose the ability to manually decide dark room colors."},
	{ "Fade CSet 5", qr_FADECS5, "When enabled, CSet 5 will also fade when in dark rooms. Only limited to Interpolated Fading."},
	{ "Bomb Explosions Don't Flash Palette", qr_NOBOMBPALFLASH, "When enabled, bombs won't flash the screen when exploding." },
	{ "Layers 1 and 2 Are Drawn Under Caves", qr_LAYER12UNDERCAVE, "If enabled, the Player and the cave they're exiting/entering are drawn above Layer 1 and 2 when entering/exiting a cave." }
};

static const GUI::ListData comboRulesList
{
	{ "Player Drowns in Walkable Water", qr_DROWN,
		"When the Player steps in water without powerful enough flippers to swim in it,"
		" they will drown." },
	{ "Smart Screen Scrolling", qr_SMARTSCREENSCROLL, "Before scrolling the screen, checks the destination screen and position to see if it's solid."
		" If so, scrolling is cancelled. This rule is incredibly buggy on it's own, and is nigh unusable without 'Fixed Smart Scrolling' enabled."},
	{ "Fixed Smart Scrolling", qr_SMARTER_SMART_SCROLL, "Fixes the many, many bugs with Smart Scrolling. Solidity on layers is now respected,"
		" Secret states and the like are now obeyed, and it now checks the Player's entire hitbox instead of the upperleft most corner."},
	{ "Can't Push Blocks Onto Unwalkable Combos", qr_SOLIDBLK, "Prevents you from pushing a push block into a solid combo."
		" The exception is Block Hole flagged combos, which can still be pushed into regardless of solidity."},
	{ "Push Blocks Don't Move When Bumped", qr_HESITANTPUSHBLOCKS, "Makes it so all push blocks must be pushed against for 4 frames before pushing."
		" If this rule is off, simply bumping into the push block while facing it will instantly make it be pushed."
		" Note that this rule does not affect Push (Wait) or similar blocks, which both have a hardcoded 16 frame delay and need all enemies to be killed."},
	{ "Burn Flags Are Triggered Instantly", qr_INSTABURNFLAGS, "Makes it so Fire weapons instantly trigger secrets upon touching a burn spot."
		" If this rule is disabled, fire weapons need to linger on a burn trigger for almost a second to trigger them."
		" A hidden side effect of this rule is that if you have the Magic Book, Wand Magic can also trigger fire secrets."},
	{ "Magic Mirror/Prism Combos Reflect Enemy and Scripted Sword Beams", qr_SWORDMIRROR, "Allows sword beams to reflect off Mirror/Prism Combos."
		" Note that this does not affect the Sword Beams that come from the Player's Sword; to have those reflect off mirrors, check the 'Sword Beams"
		" Reflect off Mirror/Prism Combos' Item Flag on the sword items you wish to have reflecting Sword Beams for in the item editor."},
	{ "Magic Mirrors Reflect Scripted Whirlwinds", qr_WHIRLWINDMIRROR, "Let's script-created Wind Weapons reflect off Mirrors." 
		" Note that this does not affect whistle-created whirlwinds; to have those reflect, check the 'Whirlwinds Reflect off Prism/Mirror Combos' Item Flag on the whistle item in the item editor."},
	{ "Combo Cycling On Layers", qr_CMBCYCLELAYERS, "Allows combos to combo cycle when placed on Layers 1-6." },
	{ "Lesser Damage Combos Take Priority", qr_DMGCOMBOPRI, "If this is enabled and you touch two damage combos at once, the one with less damage take priority."
		" If disabled, the one with more damage takes priority."},
	{ "Set Continue Point Even If Arrival Square Isn't Set", qr_WARPSIGNOREARRIVALPOINT, "When enabled, this rule will set the continue point on Entrance/Exit warps"
		" and Scrolling Warps (provided the latter can set continue points; see 'Scrolling Warps Don't Set The Continue Point'), even if the arrival square is located"
		" at 0, 0. If disabled, the continue point will skip being set if the 'destination square' isn't set. 'Destination square' in this context can either mean the Arrival Square"
		" (green square) if the rule 'Use Warp Return Points Only' isn't set, or Warp Return Point A (first blue square) if it is set. Note that if Warp Return Points are used instead of Arrival Squares,"
		" it is specifically Warp Return A that is being checked and used to determine if the continue point should be set. This means that if you were to disable this rule, and were to have the exit Warp Return"
		" Be something other than Warp Return A, and you leave Warp Return A at 0, 0, you can control whether or not an Entrance/Exit or Scrolling Warp will set your continue point or not."},
	{ "Use Warp Return Points Only", qr_NOARRIVALPOINT, "Instead of using the screen's Arrival Square for continuing, the game uses Warp Return A as the continue point."
		" With this enabled, the Arrival Square is useless."},
	{ "Scrolling Warps Don't Set The Continue Point", qr_NOSCROLLCONTINUE, "If enabled, Scrolling Warps will never set the screen's continue point."},
	{ "Use Old-Style Warp Detection (NES Movement Only)", qr_OLDSTYLEWARP, "If enabled, you must be aligned with the 8x8 X and Y grid to trigger warps."
		" If disabled, or if No GridLock or Diagonal Movement are enabled, warps can trigger even with X/Y values not evenly divisible by 8."},
	{ "Damage Combos Work On Layers 1 And 2", qr_DMGCOMBOLAYERFIX, "Allows Damage Combos to work on Layers 1 and 2."},
	{ "Hookshot Grab Combos Work On Layers 1 And 2", qr_HOOKSHOTLAYERFIX, "Allows the Hookshot to grab onto combos on Layers 1 and 2." },
	{ "Broken Mirror And Sword Beam Interaction", qr_OLDMIRRORCOMBOS, "If enabled, the positioning of reflected sword weapons is positioned incorrectly, usually leading to it being more to the northwest."},
	{ "Always Face Up On Sideview Ladders", qr_SIDEVIEWLADDER_FACEUP, "When enabled, the Player will always face up when standing or walking on a sideview ladder. This does not force the player to face up"
		" when attacking or doing anything that isn't walking or standing."},
	{ "Press 'Down' To Fall Through Sideview Platforms", qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS, "When enabled, the Player can press down to fall through a sideview platform flagged combo." },
	{ "Press 'Down+Jump' To Fall Through Sideview Platforms", qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS, "When enabled, the Player can press down while using the Roc's Feather to fall through a sideview platform flagged combo." },
	{ "Falling Through Sideview Platforms Respects 'Drunk' Inputs", qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK, "If enabled, Drunk Player pressing down (or down+jump) can accidentally fall through a sideview platform."},
	{ "Pressing Down Will Not Grab Sideview Ladders", qr_DOWN_DOESNT_GRAB_LADDERS, "If enabled, the Player can't grab onto Sideview Ladders by pressing down; only pressing up can grab sideview ladders." },
	{ "Custom Combos Work On Layers 1 And 2", qr_CUSTOMCOMBOSLAYERS1AND2, "If enabled, Layers 1 and 2 will also be checked for custom triggers ('Triggers' tab in the Combo Editor)."},
	{ "Slash Combos Work On Layers 1 And 2", qr_BUSHESONLAYERS1AND2, "If enabled, Bushes, Flowers, Tall Grass, Generic Combos, and etc will work on Layers 1 and 2."},
	{ "Auto Combos Work On Layer 1", qr_AUTOCOMBO_LAYER_1, "If enabled, Autowarps and Automatic Triggers will work on Layer 1." },
	{ "Auto Combos Work On Layer 2", qr_AUTOCOMBO_LAYER_2, "If enabled, Autowarps and Automatic Triggers will work on Layer 2." },
	{ "Overhead Combos work on Layers 1 and 2", qr_OVERHEAD_COMBOS_L1_L2, "If enabled, Overhead Combos placed on Layers 1 and 2 will still draw over Layer 4." },
	{ "Block Triggers Are Perm For Non-Heavy Blocks", qr_NONHEAVY_BLOCKTRIGGER_PERM, "If enabled, push blocks will set the secret screen state when triggering secrets."
		" If disabled, the secrets will reset when you leave and return to the screen. Note that Heavy push blocks already set the secret screen state even with this disabled."},
	{ "Pushblocks Work On Layer 1 And 2", qr_PUSHBLOCK_LAYER_1_2, "If enabled, pushblocks can be placed on Layers 1 and 2 and function. Note that the push order goes from top to bottom."},
	{ "New Combo Animation", qr_NEW_COMBO_ANIMATION, "Internally changes how Combo Animation is handled."},
	{ "New Water Collision", qr_SMARTER_WATER, "Internally changes how water is checked to allow for easier feature additions. It also allows Link to disembark out of water onto an FFC or Bridge combo."
		" 'Water Works On Layer 1/Layer 2' relies on this Quest Rule."},
	{ "Disable LA_HOPPING", qr_NO_HOPPING, "Disables the transition state when entering/exiting walkable water where the Player walks for a few frames without any player control."
		" This transition state, also known as LA_HOPPING, is a major source of frustration among players not just for an unneccesary loss of control, but also because it has a frequent habit"
		" of glitching you through walls without your control and softlocking you. Note that LA_HOPPING is still enabled in certain necessary cases, such as when entering or exiting solid water."},
	{ "Can't Swim In Solid Water", qr_NO_SOLID_SWIM, "If enabled, prevents swimming in solid water. Normally, when disabled, you can swim in walkable water (only if 'Player Drowns in Walkable Water' is checked)"
		" and solid water (which requires pushing against it and triggers the LA_HOPPING state). Because of this, you could never have partially solid water. This changes it so only walkable water is swimmable,"
		" allowing you to have partially swimmable water."},
	{ "Water Works On Layer 1", qr_WATER_ON_LAYER_1, "Allows water to work on Layer 1. Water placed on Layer 1 acts as a Bridge combo, overwriting the solidity/properties of combos placed below it. Requires 'New Water Collision'"},
	{ "Water Works On Layer 2", qr_WATER_ON_LAYER_2, "Allows water to work on Layer 2. Water placed on Layer 2 acts as a Bridge combo, overwriting the solidity/properties of combos placed below it. Requires 'New Water Collision'"},
	{ "Sideview Swimming", qr_SIDESWIM, "Enables Sideview Swimming. With this enabled, the Player gains a whole set of new actions and custom physics when in water placed in Sideview."
		" If disabled, water in Sideview acts very oddly, with gravity still applying."},
	{ "Player Faces Left/Right While Sideview Swimming", qr_SIDESWIMDIR, "When enabled, the Player can only face Left and Right in sideview, much like Link's Awakening or the Oracle Games."},
	{ "New Shallow Water Detection", qr_SHALLOW_SENSITIVE, "When enabled, Shallow Water will use the same collision used to check if Link should swim/drown." 
		" If disabled, you can get situations where you're swimming but the shallow water graphic displays."}
};

static const GUI::ListData compatRulesList
{
	{ "Old GOTOLESS Behavior", qr_GOTOLESSNOTEQUAL },
	{ "Old Lens Drawing Order", qr_OLDLENSORDER },
	{ "No Fairy Guy Fires", qr_NOFAIRYGUYFIRES },
	{ "Continuous Step Triggers", qr_TRIGGERSREPEAT },
	{ "Downward Hookshot Bug", qr_HOOKSHOTDOWNBUG },
	{ "Fix Open Door Solidity", qr_REPLACEOPENDOORS },
	{ "No Solid Damage Combos", qr_NOSOLIDDAMAGECOMBOS },
	{ "Old Hookshot Grab Checking", qr_OLDHOOKSHOTGRAB },
	{ "Peahats Are Vulnerable When Frozen By Clocks", qr_PEAHATCLOCKVULN },
	{ "Weapons With No Collision Detection Move Offscreen", qr_OFFSCREENWEAPONS },
	{ "Screen Item Pickup Disables Hunger/Special Item", qr_ITEMPICKUPSETSBELOW },
	{ "Sideview Spike Detection Prior to 2.50.1RC3", qr_OLDSIDEVIEWSPIKES },
	{ "Infinite Magic Prevents Items From Draining Rupees", qr_OLDINFMAGIC },
	{ "Use DrawBitmap() Offsets Prior to 2.50.2", qr_BITMAPOFFSETFIX },
	{ "Old (Faster) Sprite Drawing", qr_OLDSPRITEDRAWS },
	{ "Old (Instant) F6 Menu", qr_OLD_F6 },
	{ "Broken ASkipY Animation Frames", qr_BROKEN_ASKIP_Y_FRAMES },
	{ "Enemies Clip Through Top-Half Partial Solids", qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY },
	{ "Old Sideview Ceiling Collision", qr_OLD_SIDEVIEW_CEILING_COLLISON },
	{ "Items set to 0 AFrames ignore AFrame changes", qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES },
	{ "Old Enemy Knockback Collision", qr_OLD_ENEMY_KNOCKBACK_COLLISION },
	{ "Check Clip for Custom / Script Type Weapons on Screen Edge", qr_CHECKSCRIPTWEAPONOFFSCREENCLIP },
	{ "1.92 Diagonal Walk Speed", qr_SHORTDGNWALK },
	{ "Old String Margins", qr_OLD_STRING_EDITOR_MARGINS },
	{ "Old String Frame Width/Height", qr_STRING_FRAME_OLD_WIDTH_HEIGHT },
	{ "Bugged ->Next Combos", qr_IDIOTICSHASHNEXTSECRETBUGSUPPORT },
	{ "Overworld Minimap Ignores Map Item", qr_BROKEN_OVERWORLD_MINIMAP },
	{ "Old (Broken) Ring Power Maths", qr_BROKEN_RING_POWER },
	{ "Overworld DMaps Do Not Chart Progress", qr_NO_OVERWORLD_MAP_CHARTING },
	{ "Dungeon DMaps Use Classic Charting", qr_DUNGEONS_USE_CLASSIC_CHARTING },
	{ "Allow Editing Combo 0", qr_ALLOW_EDITING_COMBO_0 },
	{ "Old Chest Collision", qr_OLD_CHEST_COLLISION },
	{ "Broken Horizontal Weapon Animation", qr_BROKEN_HORIZONTAL_WEAPON_ANIM },
	{ "Enemies->Secret only affects flags 16-31", qr_ENEMIES_SECRET_ONLY_16_31 },
	{ "Old CSet2 Handling", qr_OLDCS2 },
	{ "Hardcoded Shadow/Spawn/Death anim frames", qr_HARDCODED_ENEMY_ANIMS },
	{ "Old Itemdata Script timing", qr_OLD_ITEMDATA_SCRIPT_TIMING },
	{ "No fairy spawn limit", qr_FIXED_FAIRY_LIMIT },
	{ "Arrows clip farther into dungeon walls", qr_ARROWCLIP },
	{ "All sword triggers are continuous", qr_CONT_SWORD_TRIGGERS },
	{ "Ladder takes precedence over North and South water", qr_OLD_210_WATER },
	{ "All 8 way-shot enemies use SFX_FIRE for firing SFX", qr_8WAY_SHOT_SFX },
	{ "BS Zelda uses walking sprites for swimming", qr_COPIED_SWIM_SPRITES },
	{ "Fire boomerang sparkles always face up", qr_WRONG_BRANG_TRAIL_DIR },
	{ "Wavy and Cancel warps are swapped", qr_192b163_WARP }
};

static const GUI::ListData enemiesRulesList
{
	{ "Enemies Jump/Fly Through Z-Axis", qr_NEWENEMYTILES },
	{ "Hide Enemy-Carried Items", qr_ENEMIESZAXIS },
	{ "Enemies Always Return", qr_HIDECARRIEDITEMS },
	{ "Temporary No Return Disabled", qr_ALWAYSRET },
	{ "Must Kill All Bosses To Set 'No Return' Screen State", qr_NOTMPNORET },
	{ "Multi-Directional Traps", qr_KILLALL },
	{ "Line-of-Sight Traps Can Move Across Entire Screen", qr_MEANTRAPS },
	{ "Constant Traps Can Pass Through Enemies", qr_PHANTOMPLACEDTRAPS },
	{ "Flying Enemies Can Appear on Unwalkable Combos", qr_WALLFLIERS },
	{ "Broken Enemy Shield Tiles", qr_BRKNSHLDTILES },
	{ "Enemies Don't Flash When Dying", qr_NOFLASHDEATH },
	{ "Show Shadows", qr_SHADOWS },
	{ "Translucent Shadows", qr_TRANSSHADOWS },
	{ "Shadows Flicker", qr_SHADOWSFLICKER },
	{ "Enemies Flicker When Hit", qr_ENEMIESFLICKER },
	{ "No Statue Minimum Range Or Double Fireballs", qr_BROKENSTATUES },
	{ "Use Super Bomb Explosions for Explode on Contact", qr_BOMBCHUSUPERBOMB },
	{ "Enemies Can Go Out of Bounds (Offscreen)", qr_OUTOFBOUNDSENEMIES },
	{ "Disable Spawning Custom and Friendly Enemy Types", qr_SCRIPT_FRIENDLY_ENEMY_TYPES }
};

static const GUI::ListData itemRulesList
{
	{ "Enable Magic", qr_ENABLEMAGIC },
	{ "True Arrows", qr_TRUEARROWS },
	{ "Scripted and Enemy Fire Lights Temporarily", qr_TEMPCANDLELIGHT },
	{ "Scripted Fire LWeapons Don't Hurt Player", qr_FIREPROOFLINK },
	{ "Scripted Bomb LWeapons Hurt Player", qr_OUCHBOMBS },
	{ "Scripted Melee Weapons Can't Pick Up Items", qr_NOITEMMELEE },
	{ "'Hearts Required' Affects Non-Special Items", qr_HEARTSREQUIREDFIX },
	{ "Big Triforce Pieces", qr_4TRI },
	{ "3 or 6 Triforce Total", qr_3TRI },
	{ "Fairy Combos Don't Remove Sword Jinxes", qr_NONBUBBLEFAIRIES },
	{ "Fairy Combos Remove Item Jinxes", qr_ITEMBUBBLE },
	{ "Broken Magic Book Costs", qr_BROKENBOOKCOST },
	{ "Reroll Useless Drops", qr_SMARTDROPS },
	{ "Items Ignore Sideview Platforms", qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS },
	{ "Items Held Above Player's Head Continue To Animate", qr_HOLDITEMANIMATION },
	{ "Fairies spawn with random direction", qr_FAIRYDIR }
};

static const GUI::ListData miscRulesList
{
	{ "Can Select A-Button Weapon On Subscreen", qr_SELECTAWPN },
	{ "More Sound Effects", qr_MORESOUNDS },
	{ "Fast Heart Refill", qr_FASTFILL },
	{ "Enable 1/4 Heart Life Bar", qr_QUARTERHEART },
	{ "Messages Can Be Sped Up With The A Button", qr_ALLOWFASTMSG },
	{ "Messages Can Be Bypassed With The B Button", qr_ALLOWMSGBYPASS },
	{ "Messages Freeze All Action", qr_MSGFREEZE },
	{ "Messages Disappear", qr_MSGDISAPPEAR },
	{ "Show Time On Subscreen", qr_TIME },
	{ "Healing Fairy Heart Circle Is Centered Around Player", qr_HEARTRINGFIX },
	{ "No Healing Fairy Heart Circle", qr_NOHEARTRING },
	{ "No Saving When Continuing", qr_NOSAVE },
	{ "Skip Continue Screen", qr_NOCONTINUE },
	{ "No Fires In Special Rooms", qr_NOGUYFIRES },
	{ "Special Room Guys Don't Create A Puff When Appearing", qr_NOGUYPOOF },
	{ "Log Game Events To Allegro.log", qr_LOG },
	{ "Log Script Errors To Allegro.log", qr_SCRIPTERRLOG },
	{ "Draining Rupees Can Still Be Spent", qr_SHOPCHEAT },
	{ "No Intro String in Ganon Room", qr_NOGANONINTRO },
	{ "No Ammo Required to Display Subscreen Items", qr_NEVERDISABLEAMMOONSUBSCREEN },
	{ "Triforce in Cellar Warps Player Out", qr_SIDEVIEWTRIFORCECELLAR },
	{ "Reduced Flashing (Epilepsy Protection)", qr_EPILEPSY },
	{ "No L/R Inventory Shifting", qr_NO_L_R_BUTTON_INVENTORY_SWAP },
	{ "Ex3 and Ex4 Shift A-Button Items", qr_USE_EX1_EX2_INVENTORYSWAP },
	{ "Disable Fast Mode (Uncap)", qr_NOFASTMODE },
	{ "Allow permanent secrets on Dungeon-type dmaps", qr_DUNGEON_DMAPS_PERM_SECRETS },
	{ "No Scrolling Screen While In Air", qr_NO_SCROLL_WHILE_IN_AIR },
	{ "Instant Reload On Death", qr_INSTANT_RESPAWN },
	{ "Higher Maximum Playtime", qr_GREATER_MAX_TIME },
	{ "Allow Setting X Button Items", qr_SET_XBUTTON_ITEMS },
	{ "Allow Setting Y Button Items", qr_SET_YBUTTON_ITEMS },
	{ "Instant Continue on Death", qr_INSTANT_CONTINUE },
	{ "New Dark Rooms", qr_NEW_DARKROOM },
	{ "New Darkness Draws Under Layer 7", qr_NEWDARK_L6 },
	{ "Item Cellars/Passageways/Caves use own music", qr_SCREEN80_OWN_MUSIC },
	{ "New Darkroom Lanterns Cross Screen Boundary", qr_NEWDARK_SCROLLEDGE,
		"When in a dark room with 'New Dark Rooms' enabled, lanterns will light across"
		" the boundary between screens *during scrolling*." }
};

static const GUI::ListData nesfixesRulesList
{
	{ "Freeform Dungeons", qr_FREEFORM },
	{ "Can Safely Trigger Armos/Grave From The South", qr_SAFEENEMYFADE },
	{ "Can Use Items/Weapons on Edge of Screen", qr_ITEMSONEDGES },
	{ "Fix Player's Position in Dungeons", qr_LINKDUNGEONPOSFIX },
	{ "Raft/Ladder Sprite Direction Fix", qr_RLFIX },
	{ "No Palette 3 CSet 6 Fix", qr_NOLEVEL3FIX },
	{ "Player Holds Special Bombs Over They're Head", qr_BOMBHOLDFIX },
	{ "Holding Up Items Doesn't Restart Music", qr_HOLDNOSTOPMUSIC },
	{ "Leaving Item Cellar/Passageway Doesn't Restart Music", qr_CAVEEXITNOSTOPMUSIC },
	{ "Tunic Color Can Change On Overworld", qr_OVERWORLDTUNIC },
	{ "Sword/Wand Flip Fix", qr_SWORDWANDFLIPFIX },
	{ "Push Block CSet Fix", qr_PUSHBLOCKCSETFIX },
	{ "Trap Position Fix", qr_TRAPPOSFIX },
	{ "No Invisible Border on Non-Dungeon Dmaps", qr_NOBORDER },
	{ "Items Disappear During Hold-Up", qr_OLDPICKUP },
	{ "Subscreen Appears Above Sprites", qr_SUBSCREENOVERSPRITES },
	{ "Correct Bomb/Darknut Interaction", qr_BOMBDARKNUTFIX },
	{ "Correct Offset Enemy Weapon Collision Detection", qr_OFFSETEWPNCOLLISIONFIX },
	{ "Special Items Don't Appear In Passageways", qr_ITEMSINPASSAGEWAYS },
	{ "No NES Sprite Flicker", qr_NOFLICKER },
	{ "Invincible Player Isn't Hurt By Own Fire Weapons", qr_FIREPROOFLINK2 },
	{ "No Position Offset Of Screen Items", qr_NOITEMOFFSET },
	{ "Allow Ladder Anywhere", qr_LADDERANYWHERE },
	{ "Actually Fixed Bomb/Darknut Interaction", qr_TRUEFIXEDBOMBSHIELD }
};

static const GUI::ListData playerRulesList
{
	{ "Diagonal Movement", qr_LTTPWALK },
	{ "Large Hitbox", qr_LTTPCOLLISION },
	{ "New Player Movement", qr_NEW_HERO_MOVEMENT,
		"Alters the player's movement; with this enabled, movement includes decimal precision,"
		" diagonal movement is 'smoother', and the player's speed can be adjusted using"
		" 'Player->Step' via ZScript, as well as the 'Player Step' option in 'Init Data'." },
	{ "Disable 4-Way Movement's Gridlock", qr_DISABLE_4WAY_GRIDLOCK },
	{ "Invincible Player Flickers", qr_LINKFLICKER },
	{ "Expanded Player Tile Modifiers", qr_EXPANDEDLTM }
};

static const GUI::ListData weaponsRulesList
{
	{ "npc->Weapon Uses Unique Sprites for Custom EWeapons", qr_SCRIPT_WEAPONS_UNIQUE_SPRITES },
	{ "Angular Reflected Weapons", qr_ANGULAR_REFLECTED_WEAPONS },
	{ "Mirrors Use Weapon Centre for Collision", qr_MIRRORS_USE_WEAPON_CENTRE },
	{ "Weapons Cannot Stunlock Enemies", qr_NO_STUNLOCK },
	{ "Arrows Always Penetrate", qr_ARROWS_ALWAYS_PENETRATE },
	{ "Swordbeams Always Penetrate", qr_SWORDBEAMS_ALWAYS_PENETRATE },
	{ "Boomerang EWeapons Corrected Animation", qr_CORRECTED_EW_BRANG_ANIM },
	{ "Bombs pierce enemy shields", qr_BOMBSPIERCESHIELD },
	{ "Weapons Move Offscreen (Buggy, use at own risk)", qr_WEAPONSMOVEOFFSCREEN }
};

//}

QRDialog::QRDialog(byte const* qrs, size_t qrs_per_tab, std::function<void(byte*)> setQRs):
	setQRs(setQRs), qrs_per_tab(qrs_per_tab)
{
	memcpy(local_qrs, qrs, QR_SZ);
}

std::shared_ptr<GUI::Widget> QRDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	return Window(
		title = "Quest Options",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			TabPanel(
				maxwidth = sized(308_px, 800_px),
				TabRef(
					name = "Options",
					Column(
						Rows<2>(
							hPadding = 0.5_em,
							spacing = 1_em,
							Button(
								text = "&Header",
								onClick = message::HEADER
							),
							Button(
								text = "&Pick Ruleset",
								onClick = message::RULESET
							),
							Button(
								text = "&Cheats",
								onClick = message::CHEATS
							)
						),
						Row(
							Label(text = "Map Count:"),
							mapCountTF = TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								maxLength = 3,
								text = std::to_string(map_count),
								low = 1, high = 255
							),
							Button(width = 2_em, text = "?", hAlign = 1.0, onPressFunc = []()
							{
								InfoDialog("Map Count","The number of 'maps' available in the quest file. The higher this value is,"
									" the larger your quest file will be, and the more memory it takes to keep your quest loaded;"
									" so it is generally suggested to only set this to a number of maps you will actually be "
									"making use of.\n\nIf you set this to a number lower than it is currently, maps in excess"
									" of the count will be entirely deleted.").show();
							})
						)
					)
				),
				TabRef(
					name = "Anim",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = animRulesList
					)
				),
				TabRef(
					name = "Combo",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = comboRulesList
					)
				),
				TabRef(
					name = "Compat",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = compatRulesList
					)
				),
				TabRef(
					name = "Enemy",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = enemiesRulesList
					)
				),
				TabRef(
					name = "Item",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = itemRulesList
					)
				),
				TabRef(
					name = "Misc",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = miscRulesList
					)
				),
				TabRef(
					name = "NESFix",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = nesfixesRulesList
					)
				),
				TabRef(
					name = "Player",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = playerRulesList
					)
				),
				TabRef(
					name = "Weapon",
					QRPanel(
						padding = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = weaponsRulesList
					)
				)
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					onClick = message::OK),
				Button(
					text = "Cancel",
					onClick = message::CANCEL)
			)
		)
	);
}

bool QRDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::TOGGLE_QR:
			toggle_bit(local_qrs, msg.argument);
			return false;
		case message::HEADER:
			call_header_dlg();
			return false;
		case message::RULESET:
			call_ruleset_dlg();
			return false;
		case message::CHEATS:
			call_cheats_dlg();
			return false;
		
		//Closing buttons
		case message::OK:
		{
			setQRs(local_qrs);
			word new_map_count = mapCountTF->getVal();
			if(new_map_count < map_count)
			{
				AlertDialog(
					"WARNING! Map Deletion",
					"This action will delete " + std::to_string(map_count-new_map_count)
					+ " maps from the end of your map list!",
					[&new_map_count](bool ret)
					{
						if(ret)
						{
							if(mapcount_will_affect_layers(new_map_count))
							{
								AlertDialog(
									"WARNING! Layer Deletion!",
									"Some of the maps being deleted are used as layermaps for screens that will remain!"
									" If you continue, these screens will have their layermap set to 0!",
									[&new_map_count](bool ret)
									{
										if(ret) update_map_count(new_map_count);
									}).show();
							}
							else update_map_count(new_map_count);
						}
					}).show();
			}
			else update_map_count(new_map_count);
		}
		[[fallthrough]];
		case message::CANCEL:
		default:
			return true;
	}
}
