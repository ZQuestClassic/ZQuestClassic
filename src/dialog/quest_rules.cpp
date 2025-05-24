#include "quest_rules.h"
#include "pickruleset.h"
#include "pickruletemplate.h"
#include "cheat_codes.h"
#include "headerdlg.h"
#include "info.h"
#include "alert.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "base/zsys.h"
#include "base/gui.h"
#include "gui/use_size.h"
#include "zq/zq_files.h"
#include "base/qrs.h"
#include "base/mapscr.h"

bool mapcount_will_affect_layers(word newmapcount);
void update_map_count(word newmapcount);

static bool reload_qr_dlg = false;
void call_qr_dialog(size_t qrs_per_tab, std::function<void(byte*)> setQRs)
{
	QRDialog(quest_rules, qrs_per_tab, setQRs).show();
}
void call_qrsearch_dialog(std::function<void(byte*)> setQRs)
{
	auto dlg = QRDialog(quest_rules, 0, setQRs);
	dlg.searchmode = true;
	dlg.show();
}

enum
{
	rules_anim,
	rules_combo,
	rules_compat,
	rules_enemies,
	rules_item,
	rules_misc,
	rules_nesfix,
	rules_hero,
	rules_subscr,
	rules_weapon,
	rules_zs_script,
	rules_zs_instruction,
	rules_zs_object,
	rules_zs_drawing,
	rules_zs_bugfix,
	rules_compiler_setting,
	rules_tagcount
};

std::string tagNames[rules_tagcount + 1] =
{
	"Animation",
	"Combo",
	"Compatibility",
	"Enemy",
	"Item",
	"Misc",
	"NESFix",
	"Hero",
	"Subscreen",
	"Weapon",
	"ZS: Script",
	"ZS: Instruction",
	"ZS: Object",
	"ZS: Drawing",
	"ZS: Bugfix",
	"Compiler Settings",
	"??"
};

std::string const& getLongestTagName()
{
	return tagNames[rules_compiler_setting];
}
std::string const& getTagName(int32_t ruletype)
{
	if(unsigned(ruletype) < rules_tagcount)
		return tagNames[ruletype];
	return tagNames[rules_tagcount];
}
//{ Quest Rules

static GUI::ListData animRulesList
{
	{ "BS-Zelda Animation Quirks", qr_BSZELDA, 
		"Affects a number of small miscellaneous stuff to make Z1 more"
		" accurate to BS Zelda. Guy fires have their positions adjusted,"
		" enemy spawn animations are faster, enemy and Hero death"
		" animations have more frames, wand magic moves 0.5 pixels"
		" per frame faster, weapon flashing is slightly different,"
		" boomerangs may animate different, and most weapons gain a"
		" 2 pixel Y offset when created. Having this enabled also"
		" deprecates the 'Fix Hero's Position in Dungeons' quest rule,"
		" as this rule fixes that behavior as well. If this rule is"
		" disabled, Whistle Whirlwinds appear as flame for a single"
		" frame when hitting the edge of the screen."},
	{ "Circle Opening/Closing Wipes", qr_COOLSCROLL, 
		"Changes the animation going to/from full black when entering caves"
		" or starting the game. If multiple are selected, it will be chosen"
		" randomly between the ones that are selected. If none are selected,"
		" the Z1 opening is used. This opening animation has a circle cut out"
		" from the black expand either outwards or inwards."},
	{ "Oval Opening/Closing Wipes", qr_OVALWIPE, 
		"Changes the animation going to/from full black when entering caves"
		" or starting the game. If multiple are selected, it will be chosen"
		" randomly between the ones that are selected. If none are selected,"
		" the Z1 opening is used. This opening animation has an oval cut out"
		" from the black expand either outwards or inwards, recreating LTTP's"
		" opening and game over screen." },
	{ "Triangle Opening/Closing Wipes", qr_TRIANGLEWIPE, 
		"Changes the animation going to/from full black when entering caves"
		" or starting the game. If multiple are selected, it will be chosen"
		" randomly between the ones that are selected. If none are selected,"
		" the Z1 opening is used. This opening animation has a rapidly"
		" spinning triangle cut out from black expand either outwards or"
		" inwards." },
	{ "Super Mario All-Stars Opening/Closing Wipes", qr_SMASWIPE, 
		"Changes the animation going to/from full black when entering caves"
		" or starting the game. If multiple are selected, it will be chosen"
		" randomly between the ones that are selected. If none are selected,"
		" the Z1 opening is used. This opening animation recreates the Super"
		" Mario All Stars wipe; a bunch of mini triangles in a diamond pattern"
		" expanding and cutting out the black." },
	{ "Fade to/from Black Opening/Closing Wipes", qr_FADEBLACKWIPE, 
		"Changes the animation going to/from full black when entering caves"
		" or starting the game. If multiple are selected, it will be chosen"
		" randomly between the ones that are selected. If none are selected,"
		" the Z1 opening is used. This opening animation smoothly fades the"
		" whole palette to and from black." },
	{ "No Screen Scrolling", qr_NOSCROLL, 
		"Removes the scrolling animation that happens when normally changing"
		" screens; instead instantly warping you to the next screen." },
	{ "Fast Dungeon Scrolling", qr_FASTDNGN, 
		"Prevents the speed of the scrolling animation when changing screens"
		" from being halved within dungeons. When disabled, the scrolling"
		" speed is a 4 frame initial delay, followed by 2 pixels per frame."
		" If enabled, the initial delay is 2 frames, followed by 4 pixels per"
		" frame. Note that if 'Smooth Vertical Scrolling' is disabled, the speed"
		" when scrolling up and down will always be 8 pixels per few frames,"
		" regardless of this rule, *unless* 'Very Fast Scrolling' is enabled,"
		" in which case both 'Fast Dungeon Scrolling' and 'Smooth Vertical"
		" Scrolling' are completely ignored and the scrolling speed will be"
		" 16 pixels per frame in all directions." },
	{ "Very Fast Scrolling", qr_VERYFASTSCROLLING, 
		"Greatly increases Screen Scrolling speed, making the screen scrolling"
		" 16 pixels per frame in all directions when scrolling."},
	{ "Smooth Vertical Scrolling", qr_SMOOTHVERTICALSCROLLING, 
		"If disabled, the screen scrolls 8 pixels every few frames going up"
		" and down, which can cause weird snapping issues. If enabled, the"
		" same speed for scrolling left and right is used for scrolling up"
		" and down."},
	{ "FFCs Are Visible While The Screen Is Scrolling", qr_FFCSCROLL, 
		"Freeform combos will display when scrolling the screen. If disabled,"
		" they won't be visible until after scrolling stops." },
	{ "Interpolated Fading", qr_FADE, 
		"If enabled, dark rooms and cave/passage entering will automatically"
		" calculate the dark room colors. Enabling this also enables the usage"
		" of Palette Cycling, but you lose the ability to manually decide dark"
		" room colors."},
	{ "Bomb Explosions Don't Flash Palette", qr_NOBOMBPALFLASH, 
		"When enabled, bombs won't flash the screen when exploding." },
	{ "Layers 1 and 2 Are Drawn Under Caves", qr_LAYER12UNDERCAVE, 
		"If enabled, the Hero and the cave they're exiting/entering are"
		" drawn above Layer 1 and 2 when entering/exiting a cave." },
	{ "Translucent Shadows", qr_TRANSSHADOWS,
		"If enabled, the shadows of all sprites are drawn as translucent."
		" This includes the Hero, Weapons, Enemies, and Items."},
	{ "Shadows Flicker", qr_SHADOWSFLICKER,
		" If enabled, the Hero's Shadow and Enemy Shadows only draw every other frame."
		" Items and Weapons are not affected by this."},
	{ "CSet 1 is level-specific", qr_CSET1_LEVEL,
		" If enabled, CSet 1 becomes level dependent, allowing you to"
		" change it by changing the current palette."
		" \nIf disabled, CSet 1 will use the Main Palette, and will remain static"
		" unless modified via script."},
	{ "CSet 5 is level-specific", qr_CSET5_LEVEL,
		" If enabled, CSet 5 becomes level dependent, allowing you to"
		" change it by changing the current palette."
		" \nIf disabled, CSet 5 will use the Main Palette, and will remain static"
		" unless modified via script."},
	{ "CSet 7 is level-specific", qr_CSET7_LEVEL,
		" If enabled, CSet 7 becomes level dependent, allowing you to"
		" change it by changing the current palette."
		" \nIf disabled, CSet 7 will use the Main Palette, and will remain static"
		" unless modified via script."},
	{ "CSet 8 is level-specific", qr_CSET8_LEVEL,
		" If enabled, CSet 8 becomes level dependent, allowing you to"
		" change it by changing the current palette."
		" \nIf disabled, CSet 8 will use the Main Palette, and will remain static"
		" unless modified via script."},
	{ "Fade CSet 1", qr_FADECS1, 
		"When enabled, CSet 1 will also fade when in dark rooms or when scrolling."
		" Limited only to Interpolated Fading."},
	{ "Fade CSet 5", qr_FADECS5, 
		"When enabled, CSet 5 will also fade when in dark rooms or when scrolling."
		" Limited only to Interpolated Fading."},
	{ "Fade CSet 7", qr_FADECS7, 
		"When enabled, CSet 7 will also fade when in dark rooms or when scrolling."
		" Limited only to Interpolated Fading."},
	{ "Fade CSet 8", qr_FADECS8, 
		"When enabled, CSet 8 will also fade when in dark rooms or when scrolling."
		" Limited only to Interpolated Fading."},
	{ "Fade CSet 9", qr_FADECS9, 
		"When enabled, CSet 9 will also fade when in dark rooms or when scrolling."
		" Limited only to Interpolated Fading."}
};

static GUI::ListData comboRulesList
{
	{ "Hero Drowns in Walkable Water", qr_DROWN,
		"When the Hero steps in water without powerful enough flippers"
		" to swim in it, they will drown." },
	{ "Classic Respawn Points", qr_OLD_RESPAWN_POINTS,
		"Respawn points on drowning/pitfall will always be the screen entry point with this checked."
		"\nIf unchecked, the respawn will be the last safe place the Hero has stood, including across screens." },
	{ "Smart Screen Scrolling", qr_SMARTSCREENSCROLL, 
		"Before scrolling the screen, checks the destination screen"
		" and position to see if it's solid. If so, scrolling is cancelled."
		" This rule is incredibly buggy on its own, and is nigh unusable"
		" without 'Fixed Smart Scrolling' enabled."},
	{ "Fixed Smart Scrolling", qr_SMARTER_SMART_SCROLL, 
		"Fixes the many, many bugs with Smart Scrolling. Solidity on"
		" layers is now respected, Secret states and the like are now"
		" obeyed, and it now checks the Hero's entire hitbox instead"
		" of the upperleft most corner."},
	{ "Can't Push Blocks Onto Unwalkable Combos", qr_SOLIDBLK,
		"Prevents you from pushing a push block into a solid combo."
		" The exception is Block Hole flagged combos, which can still"
		" be pushed into regardless of solidity."},
	{ "Push Blocks Don't Move When Bumped", qr_HESITANTPUSHBLOCKS,
		"Makes it so all push blocks must be pushed against for 4 frames"
		" before pushing. If this rule is off, simply bumping into the"
		" push block while facing it will instantly make it be pushed."
		" Note that this rule does not affect Push (Wait) or similar"
		" blocks, which both have a hardcoded 16 frame delay and need"
		" all enemies to be killed."},
	{ "Moving pushblock switch blocks obey switch toggle", qr_SWITCHES_AFFECT_MOVINGBLOCKS,
		"If enabled, when a crystal switch toggles, it will toggle any"
		" pushblocks that are currently moving, while they are moving."},
	{ "Pushable Block Sprite Layer", qr_PUSHBLOCK_SPRITE_LAYER,
		"If enabled, pushable blocks draw to a 'block sprite layer'."
		" pushblocks that are currently moving, while they are moving."},
	{ "Block Triggers Are Perm For Non-Heavy Blocks", qr_NONHEAVY_BLOCKTRIGGER_PERM,
		"If enabled, push blocks will set the secret screen state when"
		" triggering secrets. If disabled, the secrets will reset when"
		" you leave and return to the screen. Note that Heavy push"
		" blocks already set the secret screen state even with this"
		" disabled."},
	{ "Pushblocks Work On Layer 1 And 2", qr_PUSHBLOCK_LAYER_1_2,
		"If enabled, pushblocks can be placed on Layers 1 and 2 and"
		" function. Note that the push order goes from top to bottom."},
	{ "Block holes / triggers only trigger on same layer as block", qr_BLOCKHOLE_SAME_ONLY,
		"If enabled, pushblocks will only trigger block holes and block triggers on the same layer."},
	{ "Block Triggers improved locking", qr_BLOCKS_DONT_LOCK_OTHER_LAYERS,
		"Block triggers will not lock blocks in place as they normally do. Blocks"
		" on the same layer as the trigger will lock normally, but blocks on other layers"
		" will not lock until all triggers are covered (thus triggering secrets). At this"
		" time, ALL blocks on triggers will click into place."},
	{ "Blocks Drown In Liquid", qr_BLOCKS_DROWN,
		"Pushable blocks can drown in liquid, similarly to the hero."},
	{ "Pushblocks Fall In Sideview", qr_PUSHBLOCKS_FALL_IN_SIDEVIEW,
		"Pushable blocks fall in sideview (after being pushed only)" },
	{ "Burn Flags Are Triggered Instantly", qr_INSTABURNFLAGS, 
		"Makes it so Fire weapons instantly trigger secrets upon touching"
		" a burn spot. If this rule is disabled, fire weapons need to linger"
		" on a burn trigger for almost a second to trigger them. A hidden"
		" side effect of this rule is that if you have the Magic Book,"
		" Wand Magic can also trigger fire secrets just by passing over"
		" the flag if the book would make it leave fire afterwards."},
	{ "Magic Mirror/Prism Combos Reflect Enemy and Scripted Sword Beams", qr_SWORDMIRROR, 
		"Allows sword beams to reflect off Mirror/Prism Combos. Note"
		" that this does not affect the Sword Beams that come from"
		" the Hero's Sword; to have those reflect off mirrors, check"
		" the 'Sword Beams Reflect off Mirror/Prism Combos' Item Flag on"
		" the sword items you wish to have reflecting Sword Beams for in"
		" the item editor."},
	{ "Magic Mirrors Reflect Scripted Whirlwinds", qr_WHIRLWINDMIRROR, 
		"Let's script-created Wind Weapons reflect off Mirrors. Note"
		" that this does not affect whistle-created whirlwinds; to"
		" have those reflect, check the 'Whirlwinds Reflect off"
		" Prism/Mirror Combos' Item Flag on the whistle item in the"
		" item editor."},
	{ "Mirror/Prism Combos Work On All Layers", qr_MIRROR_PRISM_LAYERS,
		"Mirror and Prism combos work on any layer, instead of only layer 0." },
	{ "Combo Cycling On Layers", qr_CMBCYCLELAYERS, 
		"Allows combos to combo cycle when placed on Layers 1-6." },
	{ "Lesser Damage Combos Take Priority", qr_DMGCOMBOPRI, 
		"If this is enabled and you touch two damage combos at once,"
		" the one with less damage takes priority. If disabled, the"
		" one with more damage takes priority."},
	{ "Set Continue Point Even If Arrival Square Isn't Set", qr_WARPSIGNOREARRIVALPOINT, 
		"When enabled, this rule will set the continue point on"
		" Entrance/Exit warps and Scrolling Warps (provided the"
		" latter can set continue points; see 'Scrolling Warps Don't"
		" Set The Continue Point'), even if the arrival square is"
		" located at 0, 0. If disabled, the continue point will skip"
		" being set if the 'destination square' isn't set. 'Destination"
		" square' in this context can either mean the Arrival Square"
		" (green square) if the rule 'Use Warp Return Points Only' isn't"
		" set, or Warp Return Point A (first blue square) if it is set."
		" Note that if Warp Return Points are used instead of Arrival"
		" Squares, it is specifically Warp Return A that is being checked"
		" and used to determine if the continue point should be set. This"
		" means that if you were to disable this rule, and were to have"
		" the exit Warp Return be something other than Warp Return A, and"
		" you leave Warp Return A at 0, 0, you can control whether or not"
		" an Entrance/Exit or Scrolling Warp will set your continue point"
		" or not."},
	{ "Use Warp Return Points Only", qr_NOARRIVALPOINT, 
		"Instead of using the screen's Arrival Square for continuing,"
		" the game uses Warp Return A as the continue point. With this"
		" enabled, the Arrival Square is useless."},
	{ "Scrolling Warps Don't Set The Continue Point", qr_NOSCROLLCONTINUE, 
		"If enabled, Scrolling Warps will never set the screen's"
		" continue point."},
	{ "Use Old-Style Warp Detection (NES Movement Only)", qr_OLDSTYLEWARP, 
		"If enabled, you must be aligned with the 8x8 X and Y grid to"
		" trigger warps. If disabled, or if No GridLock or Diagonal"
		" Movement are enabled, warps can trigger even with X/Y values"
		" not evenly divisible by 8."},
	{ "Damage Combos Work On Layers 1 And 2", qr_DMGCOMBOLAYERFIX, 
		"Allows Damage Combos to work on Layers 1 and 2."},
	{ "Hookshot/SwitchHook Combos Work On Layers 1 And 2", qr_HOOKSHOTLAYERFIX, 
		"Allows the Hookshot/SwitchHook to grab onto combos on Layers 1 and 2." },
	{ "Hookshot/SwitchHook Combos Work On All Layers", qr_HOOKSHOTALLLAYER, 
		"Allows the Hookshot/SwitchHook to grab onto combos on any layer." },
	{ "Broken Mirror And Sword Beam Interaction", qr_OLDMIRRORCOMBOS, 
		"If enabled, the positioning of reflected sword weapons is"
		" positioned incorrectly, usually leading to it being more"
		" to the northwest."},
	{ "Always Face Up On Sideview Ladders", qr_SIDEVIEWLADDER_FACEUP, 
		"When enabled, the Hero will always face up when standing"
		" or walking on a sideview ladder. This does not force the"
		" Hero to face up when attacking or doing anything that"
		" isn't walking or standing."},
	{ "Press 'Down' To Fall Through Sideview Platforms", qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS, 
		"When enabled, the Hero can press down to fall through"
		" a sideview platform flagged combo." },
	{ "Press 'Down+Jump' To Fall Through Sideview Platforms", qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS, 
		"When enabled, the Hero can press down while using the"
		" Roc's Feather to fall through a sideview platform"
		" flagged combo." },
	{ "Falling Through Sideview Platforms Respects 'Drunk' Inputs", qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK, 
		"If enabled, Drunk Hero pressing down (or down+jump) can"
		" accidentally fall through a sideview platform."},
	{ "Pressing Down Will Not Grab Sideview Ladders", qr_DOWN_DOESNT_GRAB_LADDERS, 
		"If enabled, the Hero can't grab onto Sideview Ladders by"
		" pressing down; only pressing up can grab sideview ladders." },
	{ "Custom Combos Work On Layers 1 And 2", qr_CUSTOMCOMBOSLAYERS1AND2, 
		"If enabled, Layers 1 and 2 will also be checked for custom"
		" triggers ('Triggers' tab in the Combo Editor). Only affects weapon trigger types."},
	{ "Custom Combos Work On All Layers", qr_CUSTOMCOMBOS_EVERY_LAYER, 
		"If enabled, all layers will also be checked for custom"
		" triggers ('Triggers' tab in the Combo Editor). Only affects weapon trigger types."},
	{ "Slash Combos Work On Layers 1 And 2", qr_BUSHESONLAYERS1AND2,
		"If enabled, Bushes, Flowers, Tall Grass, Generic Combos, and"
		" etc will work on Layers 1 and 2."},
	{ "Pound Combos Work On Layers 1 And 2", qr_POUNDLAYERS1AND2,
		"If enabled, Pound combos will work on Layers 1 and 2."},
	{ "Auto Combos Work On Layer 1", qr_AUTOCOMBO_LAYER_1, 
		"If enabled, Autowarps and Automatic Triggers will work on Layer 1." },
	{ "Auto Combos Work On Layer 2", qr_AUTOCOMBO_LAYER_2, 
		"If enabled, Autowarps and Automatic Triggers will work on Layer 2." },
	{ "Auto Combos Work On All Layers", qr_AUTOCOMBO_ANY_LAYER, 
		"If enabled, Autowarps and Automatic Triggers will work on any layer." },
	{ "Overhead Combos work on Layers 1 and 2", qr_OVERHEAD_COMBOS_L1_L2, 
		"If enabled, Overhead Combos placed on Layers 1 and 2 will still"
		" draw over Layer 4." },
	{ "New Combo Animation", qr_NEW_COMBO_ANIMATION, 
		"Internally changes how Combo Animation is handled. Required"
		" for combodata->OriginalTile."},
	{ "New Water Collision", qr_SMARTER_WATER, 
		"Internally changes how water is checked to allow for easier"
		" feature additions. It also allows the Hero to disembark"
		" out of water onto an FFC or Bridge combo. 'Water Works On"
		" Layer 1/Layer 2' relies on this Quest Rule."},
	{ "Disable LA_HOPPING", qr_NO_HOPPING, 
		"Disables the transition state when entering/exiting walkable"
		" water where the Hero walks for a few frames without any"
		" Hero control. This transition state, also known as LA_HOPPING,"
		" is a major source of frustration among players not just for an"
		" unneccesary loss of control, but also because it has a frequent"
		" habit of glitching you through walls without your control and"
		" softlocking you. Note that LA_HOPPING is still enabled in certain"
		" necessary cases, such as when entering or exiting solid water."},
	{ "Can't Swim In Solid Water", qr_NO_SOLID_SWIM, 
		"If enabled, prevents swimming in solid water. Normally,"
		" when disabled, you can swim in walkable water (only if"
		" 'Hero Drowns in Walkable Water' is checked) and solid water"
		" (which requires pushing against it and triggers the LA_HOPPING"
		" state). Because of this, you could never have partially solid"
		" water. This changes it so only walkable water is swimmable,"
		" allowing you to have partially swimmable water."},
	{ "Water Works On Layer 1", qr_WATER_ON_LAYER_1, 
		"Allows water to work on Layer 1. Water placed on Layer 1"
		" acts as a Bridge combo, overwriting the solidity/properties"
		" of combos placed below it. Requires 'New Water Collision'"},
	{ "Water Works On Layer 2", qr_WATER_ON_LAYER_2, 
		"Allows water to work on Layer 2. Water placed on Layer 2"
		" acts as a Bridge combo, overwriting the solidity/properties"
		" of combos placed below it. Requires 'New Water Collision'"},
	{ "Sideview Swimming", qr_SIDESWIM, 
		"Enables Sideview Swimming. With this enabled, the Hero"
		" gains a whole set of new actions and custom physics when"
		" in water placed in Sideview. If disabled, water in Sideview"
		" acts very oddly, with gravity still applying."},
	{ "Hero Faces Left/Right While Sideview Swimming", qr_SIDESWIMDIR, 
		"When enabled, the Hero can only face Left and Right in sideview water."},
	{ "New Shallow Water Detection", qr_SHALLOW_SENSITIVE, 
		"When enabled, Shallow Water will use the same collision"
		" used to check if the Hero should swim/drown. If disabled,"
		" you can get situations where you're swimming but the shallow"
		" water graphic displays."},
	{ "New Tall Grass Detection", qr_GRASS_SENSITIVE, 
		"When enabled, Tall Grass will use the same collision"
		" used to check if the Hero should swim/drown. If disabled,"
		" you can get situations where you're swimming but the tall"
		" grass graphic displays."},
	{ "Better Sideview Damage Combo Detection", qr_LESS_AWFUL_SIDESPIKES, 
		"When enabled, damage combos in sideview will only do their"
		" check if the Hero is standing on solid ground and is"
		" mostly on the damage combo. This does not affect the"
		" check for bonking your head on a damaging ceiling; see"
		" 'Sideview Spike Detection Prior to 2.50.1RC3' for that."},
	{ "Fairy Rings Don't Remove Sword Jinxes", qr_NONBUBBLEFAIRIES,
		"If enabled, fairy ring flags won't heal sword jinxes when the Hero steps on them."
		" This used to prevent 'Fairy Combos Remove Item Jinxes' from working, but"
		" this has since been patched and both quest rules can work independently."},
	{ "Fairy Rings Remove Item Jinxes", qr_ITEMBUBBLE,
		"If enabled, fairy ring flags will heal item jinxes when the Hero steps on them."
		" This used to also affect potions and triforce pieces, but this behavior has since"
		" been relocated to the respective item flags."},
	{ "Fairy Rings Remove Shield Jinxes", qr_SHIELDBUBBLE,
		"If enabled, fairy ring flags will heal shield jinxes when the Hero steps on them."},
	{ "Light Beams draw Transparently", qr_LIGHTBEAM_TRANSPARENT, 
		"Light Beams from Spotlight combos are transparently drawn."},
	{ "Less Arbitrary Dock Combos", qr_BETTER_RAFT, 
		"If enabled, the Hero will be able to raft off a dock if there is a raft flag in"
		" either of the 4 directions from the dock, regardless of direction. If disabled, the"
		" Hero can only raft if there is a raft flag in the direction they are facing when they"
		" step on the dock."},
	{ "More Sensitive Dock Combos", qr_BETTER_RAFT_2, 
		"If enabled, the collision for dock combos will only check the bottom part of the Hero's"
		" hitbox, as opposed to the bottom and top parts of their hitbox."},
	{ "Docks use Raft Item's sound", qr_RAFT_SOUND,
		"The raft will use the sound specified in UseSound when starting rafting,"
		" as opposed to the screen's secret sound."},
	{ "Conveyors work on layers 1 and 2", qr_CONVEYORS_L1_L2,
		"Conveyor combos work on layers 1 and 2. If multiple conveyors overlap, only"
		" the highest layer conveyor will apply."},
	{ "Conveyors work on all layers + ffcs", qr_CONVEYORS_ALL_LAYERS,
		"Conveyor combos work on all layers + ffcs. If multiple conveyors overlap, only"
		" the highest layer conveyor will apply. (FFCs count as above all layers for this)"},
	{ "No Solid Damage Combos", qr_NOSOLIDDAMAGECOMBOS, 
		"Disables solid damage combos. You can only take damage by"
		" stepping on a walkable damage combo. Note that this does"
		" not affect solid sideview damage combos that you step on,"
		" you will still take damage from those even if this rule"
		" is on." +QRHINT({qr_NO_SIDEVIEW_SOLID_DAMAGE}) },
	{ "No Sideview Solid Damage", qr_NO_SIDEVIEW_SOLID_DAMAGE, 
		"Disables taking damage by stepping on solid sideview damage."
		" Does not affect damage taken by walkin into solid damage combos." +QRHINT({qr_NOSOLIDDAMAGECOMBOS}) },
	{ "Lenient Solid Damage Combos", qr_LENIENT_SOLID_DAMAGE, 
		"Solid damage combos only check the center of the Hero's hitbox. Does not affect sideview damage combos you step on."
		" Does nothing if 'No Solid Damage Combos' is checked." +QRHINT({qr_NOSOLIDDAMAGECOMBOS}) },
	{ "Sensitive Solid Damage Combos", qr_SENSITIVE_SOLID_DAMAGE, 
		"Solid damage combos only check the center of the Hero's hitbox. Does not affect sideview damage combos you step on."
		" Does nothing if 'No Solid Damage Combos' or 'Lenient Solid Damage Combos' is checked." +QRHINT({qr_NOSOLIDDAMAGECOMBOS,qr_LENIENT_SOLID_DAMAGE}) },
	{ "Allow Multiple Platform FFCs", qr_MULTI_PLATFORM_FFC, 
		"If checked, more than one FFC with the 'Platform' flag can move the Hero in a single frame."
		" Otherwise, the first 'Platform' FFC to move the Hero will prevent other platforms from moving the Hero that frame." },
	{ "Separate Tap SFX for bombable walls", qr_SEPARATE_BOMBABLE_TAPPING_SFX,
		"If checked, bombable walls will use a separate SFX 'Sword Tap (Hollow)' when tapped against,"
		" instead of the default 'Sword Tap' sound. (sounds set in the Misc SFX menu)" },
};

static GUI::ListData compatRulesList
{
	{ "Old GOTOLESS Behavior", qr_GOTOLESSNOTEQUAL, 
		"If enabled, the ZASM GOTOLESS will return true if less"
		" or equal than the compared value. If disabled, it will"
		" only return true if it's less than the compared value." },
	{ "Old Lens Drawing Order", qr_OLDLENSORDER, 
		"If enabled, Lens hints will draw on layer 0 and hide"
		" Layer 1 and 2 if there is a secret on that combo."
		" If disabled, hints will draw above Layer 2 without"
		" hiding Layers 1 and 2."},
	{ "No Fairy Guy Fires", qr_NOFAIRYGUYFIRES, 
		"If enabled, Fairy Guys will not have fires, and will"
		" not block the upper portion of the screen."},
	{ "Continuous Step Triggers", qr_TRIGGERSREPEAT, 
		"If enabled, step triggers that change to another step"
		" trigger will trigger without you needing to step off"
		" them. Normally, step triggers have a safety feature"
		" where if you step on them, you cannot step on another"
		" step trigger in that position until you move onto a"
		" different position. This rule disables that behavior,"
		" allowing you to keep triggering step triggers that appear"
		" under you without moving off of it."},
	{ "Downward Hookshot Bug", qr_HOOKSHOTDOWNBUG, 
		"If this QR is enabled, using the hookshot downwards will"
		" offset it by 4 pixels downwards, possibly allowing it to"
		" clip through half solid combos."},
	{ "Hardcoded open doors", qr_REPLACEOPENDOORS, 
		"If this is enabled, Open Doors in NES Dungeon screens will"
		" replace the tiles in the door position with the open door"
		" tiles. With this disabled, you can have different tiles"
		" occupy the Open Door space."},
	{ "Old Hookshot Grab Checking", qr_OLDHOOKSHOTGRAB, 
		"If this is enabled, the check for if a hookshot has grabbed a"
		" combo going left or right will check its Y value plus 7."
		" If disabled, it will check its Y value plus 13. Note that"
		" the hookshot's Y position is 3 pixels more than the Hero's when"
		" facing left/right, and note that this rule does not affect"
		" the hookshot when travelling up or down."},
	{ "Peahats Are Vulnerable When Frozen By Clocks", qr_PEAHATCLOCKVULN, 
		"If this rule is enabled, Peahats will be vulnerable when a Clock"
		" item is active. This rule only works if the rule 'Enemies Jump/Fly"
		" Through Z-Axis' is disabled."},
	{ "Weapons With No Collision Detection Move Offscreen", qr_OFFSCREENWEAPONS, 
		"If enabled, weapons with no collision won't be killed if they"
		" move offscreen. If disabled, they'll be killed when they move"
		" offscreen."},
	{ "Old Special Room Screenstate Detection", qr_ITEMPICKUPSETSBELOW, 
		"If enabled, Guy Rooms are flagged with mITEM instead of mSPECIALITEM"
		" if they are outside of a cave screen. This means that they set"
		" the Screen Item state, and collecting the Screen Item will"
		" remove the Guy."},
	{ "Sideview Spike Detection Prior to 2.50.1RC3", qr_OLDSIDEVIEWSPIKES, 
		"Sideview spikes were changed in newer 2.50 quests to be much"
		" more sensitive and check further below you, and also check"
		" the ceiling if you bonk against it. Checking this will revert"
		" them to the old collision, where they only the pixel below you"
		" and obeyed damage combo sensitivity, but will no longer check"
		" the ceiling above you when bumping into the ceiling. Note that"
		" if you have 'Better Sideview Damage Combo Detection' enabled,"
		" the only thing this rule affects is the ceiling portion of the rule."},
	{ "Infinite Magic Prevents Items From Draining Rupees", qr_OLDINFMAGIC, 
		"Recreates a bug where infinite magic would skip the entire pay"
		" cost check, even if the counter it was paying was rupees."
		" If enabled, having infinite magic means you won't pay any"
		" rupees for rupee-costing items. If disabled, having infinite"
		" magic won't affect rupee-costing items."},
	{ "Use DrawBitmap() Offsets Prior to 2.50.2", qr_BITMAPOFFSETFIX, 
		"Recreates a bug where for the first frame it was created,"
		" the top of the bitmap would start at the top of the subscreen,"
		" before moving back down to the regular playing field. If this"
		" is disabled, it'll use the fixed behavior, where it'll always"
		" draw where the playing field is."},
	{ "Old (Faster) Sprite Drawing", qr_OLDSPRITEDRAWS, 
		"If enabled, the old method for drawing sprites is used."
		" The old method is faster, but lacks many features such as"
		" sprite rotations and scaling."},
	{ "Old (Instant) F6 Menu", qr_OLD_F6, 
		"Changes how F6 works internally. With this rule on, it uses"
		" the old method, where the f6 menu is less than a frame faster."
		" With it off, it uses the new method, which is required for F6"
		" Scripts to work but causes the f6 to happen less than a frame"
		" later."},
	{ "Broken ASkipY Animation Frames", qr_BROKEN_ASKIP_Y_FRAMES, 
		"In older versions, Combo Anim Y was bugged, where the check"
		" for if a combo is at its last tile of animation was using"
		" poor math for Combo Anim Y. This would result in combos"
		" looping much earlier than they should. This rule is auto"
		" checked for older quests, but should be left off as it may"
		" cause unintended effects."},
	{ "Enemies Clip Through Top-Half Walkable Combos", qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY, 
		"If enabled, enemies will only check the bottom half of a"
		" combo for its solidity when choosing where it can walk."
		" If disabled, enemies can't walk into a combo if any part"
		" of it is solid."},
	{ "Old Sideview Ceiling Collision", qr_OLD_SIDEVIEW_CEILING_COLLISON, 
		"If this is enabled, the Hero's entire hitbox is checked"
		" for hitting a ceiling, instead of just their bottom half."
		" Additionally, it allows the Hero to clip into ceilings"
		" immediately above their heads when they start the Jump from"
		" land, as the solidity isn't checked until they are in the air."},
	{ "Items set to 0 AFrames ignore AFrame changes", qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES, 
		"In older versions, if an item's AFrame value was changed via"
		" script but it was initialized with 0 AFrames, it would ignore"
		" the script change and continue as if it had a single frame."
		" This rule emulates this behavior."},
	{ "Old Enemy Knockback Collision", qr_OLD_ENEMY_KNOCKBACK_COLLISION, 
		"If enabled, you are unable to knockback an enemy if it can't"
		" move 12 pixels in the direction it would be knocked back into."
		" If disabled, it will check its knockback speed (which defaults"
		" to 4), see if it can move 1 frame of knockback speed in that"
		" direction, and if it can't, won't be knocked back."},
	{ "Check Clip for Custom / Script Type Weapons on Screen Edge", qr_CHECKSCRIPTWEAPONOFFSCREENCLIP, 
		"If enabled, LW_SCRIPT and EW_SCRIPT weapons are killed earlier"
		" before hitting the edge of the screen."},
	{ "1.92 Dungeon Autowalk Speed", qr_SHORTDGNWALK, 
		"If enabled, the Hero autowalks a few pixels less into dungeon"
		" screens than he usually does."},
	{ "Old String Margins", qr_OLD_STRING_EDITOR_MARGINS, 
		"If enabled, disables the border spacing of strings and allows"
		" strings to extend past the bottom of the message box."},
	{ "Old String Frame Width/Height", qr_STRING_FRAME_OLD_WIDTH_HEIGHT, 
		"If enabled, all strings default to 25x4 as opposed to 26x5."
		" Enabled by default for 2.50 quests and below."},
	{ "Old Slash Combo Secret Flag Behavior", qr_OLD_SLASHNEXT_SECRETS, 
		"This QR changes the behavior of Slash combos when they have a"
		" secret combo on top of them. The old behavior (used when this"
		" rule is enabled) is that when slashed, instead of becoming"
		" the next combo or undercombo, they instead become the secret"
		" flagged combo. Many old quests used this behavior to simulate"
		" Slash->Next before it was a thing, and even after Slash->Next"
		" was introduced, it was used as a way to avoid making another"
		" slash->Next setup by just putting a flag over it, saving combo"
		" space/keeping organization. The new behavior (used if this rule"
		" is disabled) will ignore the secret flag, treating it as just a"
		" regular Slash combo."},
	{ "Overworld Minimap Ignores Map Item", qr_BROKEN_OVERWORLD_MINIMAP, 
		"If enabled, the overworld minimap for possessing the dungeon map is"
		" unused, and the graphic used will always be the dungeon map-less"
		" version. This Quest Rule only exists to be turned on automatically"
		" for old quests."},
	{ "Old (Broken) Ring Power Maths", qr_BROKEN_RING_POWER, 
		"If enabled, uses the old buggy math for negative powered ring weapons;"
		" the old math accidentally had double negatives, which turned negative"
		" values back into positive values. If disabled, the fixed behavior is"
		" used, which allows for proper negative defense (negative defense"
		" meaning the Hero takes more damage). This rule exists only to be"
		" enabled automatically for old quests."},
	{ "Overworld DMaps Do Not Chart Progress", qr_NO_OVERWORLD_MAP_CHARTING, 
		"Uses old (nonexistent) behavior for overworld charting. If enabled,"
		" you can't use dmapdata->Charted on overworlds. If disabled, you can"
		" use dmapdata->Charted, and the Large Map subscreen item will chart"
		" places you'd been if an overworld dmap uses that subscreen."}, 
	{ "Dungeon DMaps Use Classic Charting", qr_DUNGEONS_USE_CLASSIC_CHARTING, 
		"Uses old internal charting behavior for dungeons. Disabled for all"
		" new quests; it's only here to be turned on in older quests in the"
		" case it breaks something. Affects dmapdata->Charted on dungeon dmaps."},
	{ "Allow Editing Combo 0", qr_ALLOW_EDITING_COMBO_0, 
		"Allows the editing of Combo 0 in the editor. Combo 0 is assumed to"
		" be empty space at all times and is what's used by default on newly"
		" created screens or layers. Setting it to anything but completely"
		" walkable will cause very buggy behavior. To prevent users from"
		" accidentally causing bugs by editing Combo 0, Combo 0 was locked"
		" from being edited. If this quest rule is enabled, you are able to"
		" edit Combo 0, though it is *not* recommended."},
	{ "Old Chest Collision", qr_OLD_CHEST_COLLISION, 
		"Enabling this rule brings back the old chest behavior. The old chest"
		" behavior required the Hero to be below the chest facing up (or,"
		" if the screen was sideview, to the side of the chest facing left"
		" or right, but only if the chest was half solid and the Hero was"
		" attempting to open it from the side it was walkable) and walk into"
		" it for a few frames. If this QR is disabled, the new chest collision"
		" is enabled. The new collision allows you to customize which directions"
		" the chest is openable in the combo editor, as well as letting the Hero"
		" use a defined button to open the chest (or the old behavior of walking"
		" into the chest if set). These new features are not available if this"
		" rule is enabled."},
	{ "Old Lockblock Collision", qr_OLD_LOCKBLOCK_COLLISION,
		"Lockblocks behave as they used to if disabled; otherwise they operate"
		" similarly to chests." },
	{ "Broken Horizontal Weapon Animation", qr_BROKEN_HORIZONTAL_WEAPON_ANIM, 
		"In older versions, weapon types with different left/right facing tiles,"
		" such as Arrows or the Sword, had a bug where if their sprite was animated,"
		" it would only use the default tile and its animation, instead of using"
		" the left/right tile and its intended animation. Enabling this rule brings"
		" back this bug."},
	{ "Enemies->Secret only affects flags 16-31", qr_ENEMIES_SECRET_ONLY_16_31,
		"If this rule is enabled, Enemies->Secret will only trigger Secret Flags 16"
		" through 31. It will not trigger weapon triggers such as burn triggers or"
		" bomb triggers."},
	{ "Old CSet2 Handling", qr_OLDCS2, 
		"If this is enabled, you can access csets 12 through 15 via a combo's"
		" cset2 value. However, because these are reserved and prone to change"
		" with different editor themes, this functionality was removed. If this"
		" rule is disabled, only CSets 0-11 are allowed, and the wrapping is"
		" changed as well."},
	{ "Hardcoded Shadow/Spawn/Death anim frames", qr_HARDCODED_ENEMY_ANIMS, 
		"If this is enabled, enemies spawning, shadow, and death animations are"
		" hardcoded. They will use hardcoded aframe and aspeed values regardless"
		" of what they are set as in the sprite editor. If this is disabled,"
		" these sprites will obey the values set in their respective sprite data."
		" Note that this rule also affects the 'death poof' the Hero does when"
		" they die!"},
	{ "Old Itemdata Script timing", qr_OLD_ITEMDATA_SCRIPT_TIMING, 
		"Changes the timing of itemdata scripts. If this is disabled, they run"
		" immediately after the Hero's internal code. If this is enabled,"
		" it will run immediately before."},
	{ "Old fairy spawn limit", qr_OLD_FAIRY_LIMIT, 
		"If this rule is enabled, there is a limit of one fairy onscreen that enemies"
		" and combos can drop."},
	{ "Arrows clip farther into dungeon walls", qr_ARROWCLIP, 
		"If this rule is enabled, arrows will still check for secrets even while they"
		" are 'blinking out'. This lets them hit triggers on top of blocking combos and"
		" on the edges of dungeon walls."},
	{ "All sword triggers are continuous", qr_CONT_SWORD_TRIGGERS,
		"If this rule is enabled, all sword triggers are treated as continuous. That"
		" means if a sword trigger becomes another sword trigger and the sword still"
		" occupies the space, it will trigger the new trigger too."
		" If a sword trigger is not continuous and it becomes another trigger, you"
		" will need to slash the sword again to trigger the new trigger."},
	{ "Ladder takes precedence over North and South water", qr_OLD_210_WATER, 
		"This rule changes whether the ladder or the flippers takes priority when"
		" entering water from below or above. If this rule is enabled, the ladder"
		" takes priority. If this rule is disabled, the flippers take priority."
		" Note that flippers always take priority when entering into water from"
		" the left or right, regardless of this rule."},
	{ "BS Zelda uses walking sprites for swimming", qr_COPIED_SWIM_SPRITES,
		"In older versions of ZC, BS Zelda animation didn't use swimming sprites,"
		" and thus defaulted to walking sprites. This QR simulates this behavior"},
	{ "Fire boomerang sparkles always face up", qr_WRONG_BRANG_TRAIL_DIR,
		"In older versions of ZC, Fire Boomerang sparkles always faced up due to"
		" not having a direction set. This QR re-enables this behavior."},
	{ "Wavy and Cancel warps are swapped", qr_192b163_WARP,
		"Swaps Wavy and Cancel warps. Supposedly this was done temporarily in"
		" 1.92 Beta 163, so this quest rule is automatically activated when"
		" playing those quests."},
	{ "Old Warping Onto Warp Protection", qr_210_WARPRETURN,
		"Changes the code that prevents you from being warped when you warp"
		" onto a warp to the 2.10 logic. Some quests, such as Ballad of a Bloodline,"
		" warp you onto a step trigger in an enclosed space. With this rule off,"
		" those quests will softlock."},
	{ "Jittering In Sideview With Ladder Item", qr_OLD_LADDER_ITEM_SIDEVIEW,
		"In old versions, there was a bug where possessing the ladder item would"
		" let you jitter upwards in sideview. Originally, this happened"
		" because someone attempted to add ladder compatibility in sideview"
		" and left it half-finished. Checking this rule will emulate this behavior."},
	{ "Renewable Secret Money Rooms Outside Caves", qr_OLD_SECRETMONEY,
		"If enabled, Secret Money rooms will never be flagged as"
		" collected unless they are inside a Screen 80 Cave or"
		" Item Cellar. This was a bug that existed until 2.50 RC1."},
	{ "Renewable Door Repairs Rooms Outside Caves", qr_OLD_DOORREPAIR,
		"If enabled, Door Repair rooms will never be flagged as collected unless"
		" they are inside a Screen 80 Cave or Item Cellar."
		" This was a bug that existed until 2.50 RC4."},
	{ "Renewable Potion Or Heart Container Rooms Outside Caves", qr_OLD_POTION_OR_HC,
		"If enabled, the Red Potion or Heart Container room will never be flagged"
		" as collected unless it is inside a Screen 80 Cave or Item Cellar."
		" This was a bug that existed until 2.50.1 Final."
		" Note that this does not refer to the 'Take Only One' Item room;"
		" that is a different room type."},
	{ "Lanmolas can't be ringleaders", qr_NO_LANMOLA_RINGLEADER,
		"If enabled, Lanmolas are unable to be ringleaders."
		" This was a bug introduced in 2.50 RC1,"
		" and was fixed in 2.55 Alpha 100."},
	{ "Step->Secrets (Temp) only affects flags 16-31", qr_STEPTEMP_SECRET_ONLY_16_31,
		"If this rule is enabled, Step->Secrets (Temp) will only trigger Secret Flags"
		" 16 through 31. It will not trigger weapon triggers such as burn triggers or"
		" bomb triggers."},
	{ "'Hit All Triggers->Perm Secret' doesn't trigger temp secrets", qr_ALLTRIG_PERMSEC_NO_TEMP,
		"The screen flag 'Hit All Triggers->Perm Secret' will only set the secret state of the screen,"
		" not trigger secrets immediately"},
	{ "Hardcoded LItem/Bomb/Clock/Magic Tile Mods", qr_HARDCODED_LITEM_LTMS,
		"Tile modifiers for level items, bombs/superbombs, clocks, and magic containers are"
		" (incorrectly) hardcoded to their classic item IDs"},
	{ "Hardcoded BS Patras", qr_HARDCODED_BS_PATRA,
		"If enabled, BS Patras are incredibly hardcoded and disobey most of the enemy editor."
		" If left off, they are just a branch of the regular Patra enemy and are able to use anything"
		" a regular Patra can use."},
	{ "Hardcoded Patra Inner Eye offsets", qr_PATRAS_USE_HARDCODED_OFFSETS,
		"If enabled, patras use hardcoded offsets from the main body for their inner eyes."
		" If disabled, they will use the enemy's 'special tile' to figure out what tiles to use."
		" When disabled, the special tile for patras should be set to the first inner eye tile."
		" When enabled, the tile used will be the next row of tiles after the main body tiles,"
		" Which is of variable length depending on whether or not the Center Eye shoots or not."},
	{ "Broken 'Big enemy' animation style", qr_BROKEN_BIG_ENEMY_ANIMATION,
		"If enabled, Big Enemies will have buggy undefined behavior with large tilewidth/tileheight custom enemies."
		" Disabling this will result in more expected behavior."},
	{ "Broken Attribute 31/32", qr_BROKEN_ATTRIBUTE_31_32,
		"Prior to this compat rule existing, enemy attribute 31 was set by attribute 32,"
		" and attribute 32 returned random memory. Though allowing it to return random"
		" memory is a security risk (and thus isn't allowed), this rule recreates this"
		" behavior as safely as possible by having attribute 32 return 0 and attribute"
		" 31 be set to attribute 32."},
	{ "Disable Spawning Custom and Friendly Enemy Types", qr_SCRIPT_FRIENDLY_ENEMY_TYPES,
		"Prior to 2.55 Alpha 46, Script and Friendly class enemies did not exist."
		" This rule prevents spawning those enemies if scripts in older versions"
		" were attempting to spawn them and relied on it failing."},
	{ "Old fire trail duration", qr_OLD_FLAMETRAIL_DURATION,
		"Prior to 2.55 Alpha 100, Fire Trails lasted for 640 frames by default, or a little"
		" over 10 and a half seconds. They have since been reduced to 180 frames, or 3 seconds."
		" Enabling this will revert back to the old duration."},
	{ "Broken Magic Book Costs", qr_BROKENBOOKCOST,
		"In older versions, the magic check for the Magic Book was broken. It paid the cost correctly,"
		" but if it couldn't pay the cost, it would still create the flame and trigger fire secrets."
		" Enabling this recreates this bug."},
	{ "Old Intro String in Ganon Room Behavior", qr_GANONINTRO,
		"In older versions, Ganon Rooms used to call the function that reset the dmap's intro clock."
		" This normally wouldn't do anything as it checks if you've already visited the dmap, but in"
		" certain circumstances this could allow Ganon to repeat the dmap's intro string."
		" Enabling this will recreate this behavior."},
	{ "Candle use limit is shared", qr_CANDLES_SHARED_LIMIT,
		"Candle limited uses per screen is shared between candles" },
	{ "Enemies with 'None' animation don't obey OriginalTile changes", qr_ANONE_NOANIM,
		"Enemies with the 'None' animation style will not reset their displayed tile to"
		" their 'OriginalTile' every frame if this is enabled." },
	{ "'Prisms Reflect Angular Weapons' Uses Broken Logic", qr_ANGULAR_REFLECT_BROKEN,
		"If enabled, Angular Reflected Weapons use completely wrong code. This is only"
		" here for compatibility purposes; enabling this is liable to result in undefined"
		" behavior. Keep this disabled."},
	{ "Old Bridge Combo Behavior", qr_OLD_BRIDGE_COMBOS, 
		"If enabled, Bridge Combos use the solidity box instead of the effects square to determine"
		" what sections the bridge covers. Solid portions will use the solidity of what is beneath"
		" it, while walkable portions will be walkable regardless of what is beneath it. \nIf disabled,"
		" the bridge will use the effects square to determine what parts should cover the below segments,"
		" regardless of whether it's solid or not."},
	{ "Broken Z3 Animation", qr_BROKEN_Z3_ANIMATION,
		"If enabled, Z3 animation will use broken, 1 tile animation for stabbing, slashing, pounding, and charging."
		" If disabled, Z3 animation will use the intended animation for stabbing, slashing, pounding, and charging."},
	{ "Old Enemy Tile Behavior with Animation (None) Enemies", qr_OLD_TILE_INITIALIZATION,
		"If enabled, enemies initialize their tile every frame by setting it to their O_Tile, even if they have an animation"
		" of type (none). If disabled, enemies without animation will only initialize their tile if their tile is 0; which it"
		" is when it is first created. This does not affect enemies that have an animation set."},
	{ "Enemy->Jump Fluctuates When on Ground", qr_FLUCTUATING_ENEMY_JUMP,
		"If enabled, gravity applies to enemies every other frame when they are on the ground,"
		" setting their jump value to the negative value of gravity for a frame before reverting"
		" it back to 0 the next frame. If disabled, enemy jump will not go below 0 when on the ground."},
	{ "Script Sprite->Jump is Truncated", qr_SPRITE_JUMP_IS_TRUNCATED,
		"If enabled, reading Sprite->Jump via script will return the value without the decimal portion."
		" If disabled, the correct Sprite->Jump value will be returned."},
	{ "->Next Combos Ignore Trigger Flags", qr_BUGGY_BUGGY_SLASH_TRIGGERS,
		"If enabled, trigger flags cannot be triggered if placed on a ->Next combo type."
		" This was a bug introduced by a poor implementation of the 'Old Slash Combo Secret Flag Behavior'"
		" rule. Note that this rule does nothing if 'Old Slash Combo Secret Flag Behavior' is enabled."},
	{ "Old Quake/DrawYOffset Behavior", qr_OLD_DRAWOFFSET,
		"If enabled, certain enemies will have their draw offsets affected by Screen->Quake when they spawn."
		" If disabled, the offset will not be affected by Screen->Quake."},
	{ "Broken DrawScreen Derivative Functions", qr_BROKEN_DRAWSCREEN_FUNCTIONS,
		"If enabled, DrawScreenSolid, DrawScreenSolidity, DrawScreenComboFlags, DrawScreenComboIFlags,"
		" and DrawScreenComboTypes will have broken draws if a screen has layers enabled."
		" \nIf disabled, these functions will not break if a layer exists on the source screen. Note that only"
		" DrawScreenSolid works properly with layers; the other functions will otherwise only take Layer 0"
		" of the source screen."},
	{ "Scrolling Cancels Charge", qr_SCROLLING_KILLS_CHARGE,
		"If enabled, scrolling screens while charging (i.e. spin attack)"
		" will not keep the charge on the new screen." },
	{ "Broken Enemy Item Carrying", qr_BROKEN_ITEM_CARRYING,
		"If enabled, enemies will use the topleft corner of their tile as their item carry position when carrying an item,"
		" which might look off with large enemies. They will also not pass their Z value onto the item."
		" \nIf disabled, they will use the same position as their item drop position, and will carry the item into the Z axis." },
	{ "Custom Weapon / Ice Rod Bugged Cost", qr_CUSTOMWEAPON_IGNORE_COST,
		"Custom Weapon and Ice Rod itemclasses will not charge the use cost set"
		" in the item editor if this is enabled." },
	{ "Leevers Still Submerge If Stunned", qr_LEEVERS_DONT_OBEY_STUN,
		"If enabled, Leevers will still submerge if they are stunned. If disabled, they will"
		" not submerge while stunned, but they will unsubmerge if they are currently submerged."},
	{ "Wizzrobes Still Teleport If Stunned", qr_WIZZROBES_DONT_OBEY_STUN,
		"If enabled, Wizzrobes will still teleport if they are stunned. If disabled, they will"
		" not teleport while stunned, and will 'unteleport' if they are in the middle of teleporting."},
	{ "Ganon-Type Rooms Don't Work On Continuing", qr_GANON_CANT_SPAWN_ON_CONTINUE,
		"If enabled, continuing/starting the quest will not trigger Ganon Rooms."},
	{ "Old Bug Net Animation", qr_OLD_BUG_NET,
		"If enabled, the Hero won't use slashing tiles while using the Bug Net."},
	{ "Manhandla Uses Hit Sound For Center Body", qr_MANHANDLA_BLOCK_SFX,
		"If enabled, Manhandla will play the hit sound when hitting the center body"
		" instead of the block sound."},
	{ "Broken 'Keep Lower Level Items'", qr_BROKEN_KEEPOLD_FLAG,
		"If enabled, 'Keep Lower Level Items' acts as always checked for all items." },
	{ "Old Half Magic", qr_OLD_HALF_MAGIC,
		"If enabled, half magic rooms always set the multiplier to 1,"
		" instead of halving its' previous value." },
	{ "Engine warps restart DMap Script", qr_WARPS_RESTART_DMAPSCRIPT,
		"If enabled, engine warps will restart or clear the currently running dmap script, even if you warp to the same dmap you were just on." },
	{ "DMap 0 hardcoded continue", qr_DMAP_0_CONTINUE_BUG,
		"If enabled, entering DMap 0 will set your continue point to its continue screen, even if it shouldn't." },
	{ "Old Scripted Knockback", qr_OLD_SCRIPTED_KNOCKBACK,
		"If enabled, npc->Knockback will use older logic, which had a tendency to clip enemies into walls." },
	{ "Old Keese Z Axis Behavior", qr_OLD_KEESE_Z_AXIS,
		"If enabled, Keese will barely change their z value unless over 128 pixels away from the Hero. If disabled,"
		" they will use the Z axis up to 40 pixels away and go higher up than before, but still be hittable when close to the Hero."},
	{ "No Pols Voice/Vire Shadows with Z axis", qr_POLVIRE_NO_SHADOW,
		"If enabled, Pols Voice and Vires won't have shadows when jumping in the Z axis."},
	{ "Decorations -2 yoffset", qr_DECO_2_YOFFSET,
		"If enabled, all 'decoration' type objects will have a yoffset of '-2' instead of '0'." },
	{ "Screen->State[] 0x80 bug", qr_SCREENSTATE_80s_BUG,
		"If enabled, 'Screen->State[]' will use an old behavior that may read the wrong screen data." },
	{ "Undamaged Gohma can be hit on any side", qr_GOHMA_UNDAMAGED_BUG,
		"If enabled, the first weapon to hit a Gohma will not be blocked, regardless of where it hits." },
	{ "Preload FFCs don't work on init screen", qr_FFCPRELOAD_BUGGED_LOAD,
		"If enabled, FFCs that 'run on screen init' will not run before the opening wipe on the first screen"
		" loading from the save select screen." },
	{ "No lifting sprite", qr_NO_LIFT_SPRITE,
		"If enabled, the Hero will not display a 'lifting' sprite." },
	{ "Old Sideview Landing", qr_OLD_SIDEVIEW_LANDING_CODE,
		"If enabled, the old code for landing in sideview will be used." },
	{ "FFC 128 speed cap", qr_OLD_FFC_SPEED_CAP,
		"If enabled, ffcs will cap to 128 speed." },
	{ "Wizzrobes use old submerge code", qr_OLD_WIZZROBE_SUBMERGING,
		"If enabled, wizzrobes will not read as 'Submerged' to ZScript, and will use hardcoded"
			" offsets. If disabled, relative offsets will be used, and 'Submerged' will read"
			" as true when vanished."},
	{ "Old FFC Functionality", qr_OLD_FFC_FUNCTIONALITY,
		"If enabled, FFCs will lack certain functionality, such as hookshotability, switch_hookability,"
		" and ffc triggers, among other misc changes."},
	{ "Old Shallow Water SFX Attribute", qr_OLD_SHALLOW_SFX,
		"If enabled, Shallow Water will use attribytes[0] instead of attribytes[5] for its splash sound." },
	{ "Weapon Sparkles Inherit Properties", qr_SPARKLES_INHERIT_PROPERTIES,
		"If enabled, sparkle weapons inherit properties such as script, type, and default defense"
		" from the parent item." },
	{ "Bugged Layered Flags", qr_BUGGED_LAYERED_FLAGS,
		"If enabled, flags placed on layers will 'wrap' at the edge of screens, causing odd behaviors." },
	{ "Old FFC Bush Drops", qr_HARDCODED_FFC_BUSH_DROPS,
		"If enabled, FFC bushes will drop a random '15% heart, 20% rupee' instead of using dropset 12." },
	{ "Fake-solid Pushblocks", qr_MOVINGBLOCK_FAKE_SOLID,
		"If enabled, pushblocks will not use 'real' solidity." },
	{ "Broken ->HitBY UIDs", qr_BROKENHITBY,
		"If enabled, both Hero fire and bomb weapons won't work with HitBy if they hurt the Hero."},
	{ "Broken Moving && Air Bombs", qr_BROKEN_MOVING_BOMBS,
		"If enabled, bombs exploding while moving will behave oddly and broken, and bomb explosions"
		" will continue obeying gravity." },
	{ "Old Bomb Hitboxes", qr_OLD_BOMB_HITBOXES,
		"If enabled, the outer ring of bombs' hitboxes will not count for triggering combos/secrets,"
		" and giving bombs custom radii will not work."},
	{ "Broken scroll/warp frame issue", qr_SCROLLWARP_NO_RESET_FRAME,
		"If enabled, after a scroll or warp, the next frame is started mid-frame"
		" instead of from the start, possibly causing issues with things like input." },
	{ "Scripted enemies don't run script on first frame of screen entry", qr_ENEMIES_DONT_SCRIPT_FIRST_FRAME,
		"If enabled, enemies don't run their scripts on the first frame of entering a screen." },
	{ "Broken Scrolling Onto Raft Flags", qr_BROKEN_RAFT_SCROLL,
		"If enabled, scrolling the screen such that you are on a raft flag on the new screen will force rafting,"
		" even if you were not rafting before, or do not have the raft item." },
	{ "Broken Input Down State", qr_BROKEN_INPUT_DOWN_STATE,
		"If enabled: 1) the engine does not reset the internal button down state every frame."
		" 2) setting button inputs to false (ex: `Input->Button[CB_A] = false`) will have no effect on button presses."
		" 3) breaks Roc's Feather when assigned to a button press."
		" If disabled: button presses are eaten when scripts write to button states."},
	{ "Old Guy Handling", qr_OLD_GUY_HANDLING,
		"If enabled, several new features relating to room guys will not work, instead using the old version behaviors."},
	{ "Inconsistent Fairy Flags", qr_FAIRY_FLAG_COMPAT,
		"If enabled, fairy flags work differently placed vs inherent."},
	{ "Old Lens Layer Effects", qr_OLD_LENS_LAYEREFFECT,
		"If enabled, the lens only effects one layer of the screen at maximum, using the old dropdown selection."},
	{ "Old (Buggy) ZScript Volume Access", qr_OLD_SCRIPT_VOLUME,
		"If enabled, Audio->AdjustMusicVolume() and Audio->AdjustSFXVolume() will use their old behavior,"
		" which was bugged and wrote directly to the volume sliders." },
	{ "'Walk Through Walls' don't set door state", qr_WALKTHROUGHWALL_NO_DOORSTATE,
		"If enabled, going through 'Walk-through' type doors does not set the room's door state." },
	{ "Light beams ignore solid objects", qr_SPOTLIGHT_IGNR_SOLIDOBJ,
		"If enabled, light beams don't treat solid ffcs/blocks/enemies/etc as solid." },
	{ "Broken light beam hitbox", qr_BROKEN_LIGHTBEAM_HITBOX,
		"If enabled, a couple of old light beam hitbox issues will occur. Notably, ffc light triggers"
		" will use 'x+8,y+8' instead of their center, and light triggers will act as 'Mirror (4-Way)'"
		" combos (to light beams, not weapons) if they do not block light." },
	{ "Broken Sword Triggers (spin-related)", qr_BROKEN_SWORD_SPIN_TRIGGERS,
		"If enabled, a sword trigger that was hit by slash cannot be re-hit by"
		" the same sword during the spin attack, making some spin-based puzzles not work." },
	{ "Old DMap Titles and Intros", qr_OLD_DMAP_INTRO_STRINGS,
		"If enabled, DMaps will use old title and intro string behavior where the"
		" intro string did not use the string editor and title was limited to 20 chars." },
	{ "Broken Subscreen Ammo Costs", qr_BROKEN_BOMB_AMMO_COSTS,
		"If enabled, bombs/super bombs will check the bomb/sbomb counters to be usable,"
		" regardless of what their Cost Counters are set to." },
	{ "Broken Lifting/Swimming", qr_BROKEN_LIFTSWIM,
		"If enabled, lifted objects won't be dropped when swimming, even when lift gloves are set"
		" to not allow lifting while swimming." },
	{ "Broken Push (Generic) locking", qr_BROKEN_GENERIC_PUSHBLOCK_LOCKING,
		"If enabled, Push (Generic) combos won't lock into place via the 'Pushed' flag"
		" (ex. 'clicking into place' for a block trigger)" },
	{ "Broken Enemy Fire/Arrow Reflecting", qr_BROKEN_FLAME_ARROW_REFLECTING,
		"If enabled, Enemy Fire/Arrows won't reflect off of Hero shields, even when the reflect flag is set."
		"\nEnemy 'Fire 2' will, contrarily, ALWAYS be BLOCKED by Hero shields, regardless of block/reflect flags." },
	{ "Broken Sideview Sprite Jump", qr_BROKEN_SIDEVIEW_SPRITE_JUMP,
		"If enabled, certain sprites (enemy, weapon, and item) will stick to the ground in sideview"
		" and cannot be made to jump off via script." },
	{ "Broken Conveyors", qr_BROKEN_CONVEYORS,
		"If enabled, conveyors will affect the hero when rafting." },
	{ "Broken Pushblock Top-Half Solids", qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS,
		"If enabled, blocks will ignore the top half combo solidity when being pushed." },
	{ "Pushblock Flag Cloning", qr_BROKEN_PUSHBLOCK_FLAG_CLONING,
		"If enabled, pushblocks will clone any non-'Push' placed flag (only numbered 59 or higher, except `Push (Generic)` which does all) on them to every square they stop in." },
	{ "Old Landing SFX", qr_OLD_LANDING_SFX,
		"If enabled, water will use the old hardcoded splash sound when landing in it. Otherwise, the landing sound set for each combo will be used instead." },
	{ "'Burning' triggers don't count as 'weapon triggers'", qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS,
		"If enabled, weapons triggering the 'Burning' trigger causes will not count as 'weapon triggers' for"
		" purposes of other trigger flags such as 'Kill Triggering Weapon' and 'Weapon Separate Triggers'" },
	{ "Broken Hero Position Exposed To Scripts When Scrolling", qr_BROKEN_SCRIPTS_SCROLLING_HERO_POSITION,
		"If disabled, the hero position will be accurately updated and clamped to the visible viewport during"
		" each frame of a scroll. Otherwise, it will be off by a few pixels, especially towards the end of a scroll."},
	{ "Hide Bottom 8 Pixels", qr_HIDE_BOTTOM_8_PIXELS,
		"If enabled, the bottom 8 pixels of the screen are never shown."
		" Note that if disabled, active subscreens are still only 168 pixels tall, though overlay subscreens would be 230 instead of 224."},
	{ "Broken Script Drawing Bitmap Draw Origin", qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN,
		"If enabled, bitmap draw commands incorrectly use Screen->DrawOrigin, resulting in unexpected offsets for bitmap drawing."
		" This only affects quests made in 2.55.9 or later."},
	{ "Scripts Screen->DrawLight Functions Have No Offset", qr_SCRIPTS_SCREEN_DRAW_LIGHT_NO_OFFSET,
		"If enabled, the Screen->DrawLight functions never add the playing field offset, and the draw command is done instantly."
		" When disabled, those functions respect Screen->DrawOrigin, and use the SPLAYER_DARKROOM_UNDER draw timing."},
	{ "Inverted Dark Combo Triggers", qr_INVERTED_DARK_COMBO_TRIGGERS,
		"If enabled, the combo trigger conditions for darkness / no darkness are inverted."},
	{ "Broken Sideview Icy Floor", qr_BROKEN_ICY_FLOOR_SIDEVIEW,
		"If enabled, 'Icy Floor' combos will use the same collision checks in sideview as overview."
		" If disabled, icy floor takes effect underneath your feet (the combo you are *standing on*) instead." },
	{ "Strings on Layer 6 Draw Above Layer 7", qr_LAYER6_STRINGS_OVER_SUBSCREEN,
		"If enabled, message strings set to draw to layer '6' will actually draw over the subscreen and layer 7 script draws."
		"\nIf disabled, these strings draw directly UNDER the subscreen instead."
		"\nHas no effect at all if `Subscreen Appears Above Sprites` is unchecked." + QRHINT({qr_SUBSCREENOVERSPRITES})},
	{ "Broken System Colors", qr_BROKEN_SYSTEM_COLORS,
		"If enabled, subscreen editor 'system colors' will simply use cset 14's colors,"
		" which will look different in the editor than the player, and is not consistent."
		"\nIf disabled, these colors still may look different in the editor than the player-"
		" but they will be the closest matching color in the player's palette to the color in the editor." },
};

static GUI::ListData enemiesRulesList
{
	{ "Use New Enemy Tiles", qr_NEWENEMYTILES,
		"If enabled, enemies will use different, more modern animation instead of their"
		" more NES-styled animation. Enemies will use New Tile and E. Anim in the enemy editor"
		" instead of Old Tile and O. Anim, and enemies will have access to features such as"
		" firing animations, tiles for all 4/8 directions, unique segment tiles, and etc."
		" If disabled, enemies will use more basic, more limited animation."},
	{ "Enemies Jump/Fly Through Z-Axis", qr_ENEMIESZAXIS,
		"If enabled, certain enemies will make use of the Z-Axis in their behaviors."
		" Tektites, Vires, Pols Voice, Rocks, and Boulders will jump/bounce through"
		" the Z-Axis instead of faking it via the Y-Axis, Peahats will lose their"
		" invulnerability while flying but will instead fly high enough that most"
		" weapons can't hit it without the Hero jumping, and Summoner Wizzrobes"
		" will spawn their summoned enemies in the air."},
	{ "Hide Enemy-Carried Items", qr_HIDECARRIEDITEMS, 
		"If enabled, enemy-carried items are moved offscreen whenever the item"
		"-carrying enemy spawns, and is only moved back whenever the enemy dies."
		" This prevents the Hero from bumping into the enemy to pick up the item"
		" before the enemy is killed. The exact coordinates the item is set to is"
		" -128, -128."},
	{ "Enemies Always Return", qr_ALWAYSRET,
		"If enabled, regular enemies will always return when you leave the screen"
		" and come back, regardless of how many you killed before leaving. This"
		" does not affect 'dungeon boss' screen flagged enemies if the screen is"
		" cleared, nor does it affect enemies which are flagged to 'never return"
		" after death'; enemies in these conditions will stay dead regardless of"
		" this rule."},
	{ "Must Kill All Bosses To Set 'No Return' Screen State", qr_KILLALL,
		"If enabled, killing an enemy that has 'Never Returns After Death' set will"
		" will not set the screen flag that keeps them dead if there are any other"
		" enemies currently on screen that have that flag set. This means that"
		" only after killing all enemies with the flag set will the screen be"
		" marked as having killed the not-returning enemy. Otherwise, if disabled,"
		" killing any enemy with 'Never Returns After Death' will prevent any other"
		" enemy in the room with that flag set from also returning."},
	{ "Line of Sight Traps Obey Solidity And Don't Have Hardcoded Directions", qr_MEANTRAPS, 
		"If enabled, Line of Sight traps will return when they hit a solid object."
		" If disabled, they will only return depending on their position as"
		" determined by 'Line-of-Sight Traps Can Move Across Entire Screen'."
		" This rule also determines if the directions a trap can move in are"
		" hardcoded based on its position. If this rule is enabled, they are"
		" not hardcoded. If this rule is disabled, traps will be unable to"
		" travel in certain directions if placed in specific positions."},
	{ "Line-of-Sight Traps Can Move Across Entire Screen", qr_MEANPLACEDTRAPS, 
		"If enabled, Line of Sight traps will only be stopped either by hitting"
		" the screen edge or, if the rule 'Line of Sight Traps Obey Solidity"
		" And Don't Have Hardcoded Directions' is enabled, by hitting a"
		" solid object. If disabled, they will arbitrarily stop at the"
		" middle of the screen either vertically or horizontally."},
	{ "Constant Traps Can Pass Through Enemies", qr_PHANTOMPLACEDTRAPS,
		"Normally, Constant Traps have collision with enemies, and will richochet"
		" and come back when they hit any enemy, including other traps. Enabling"
		" this rule disables this behavior."},
	{ "Flying Enemies Can Spawn on Solid Combos", qr_WALLFLIERS,
		"If enabled, flying enemies can spawn naturally on solid, non-'No Flying Enemy' flagged"
		" combos. Otherwise, they can only spawn on non-solid combos. Note that flying enemies"
		" can naturally move over solid combos regardless of this rule. This rule only affects"
		" if they can spawn over the Combos if they're solid."},
	{ "Broken Enemy Shield Tiles", qr_BRKNSHLDTILES,
		"If enabled, enemies who have their shield broken with the hammer will switch to their"
		" special tile. If 'Use New Enemy Tiles' is disabled, this only affects enemies that have"
		" the 'Darknut (NES)' animation style."},
	{ "Show Shadows", qr_SHADOWS,
		"If enabled, enemies will draw shadows below them if they are in the air or if they are"
		" a flying enemy. How these shadows appear are affected by 'Translucent Shadows' and"
		" 'Shadows Flicker', both of which are found in Animation Rules."},
	{ "Enemies Flicker When Hit", qr_ENEMIESFLICKER,
		"If enabled, all enemies flicker when they take damage instead of flashing."
		" Does not apply to Ganon-type enemies."},
	{ "Use Super Bomb Explosions for Explode on Contact", qr_BOMBCHUSUPERBOMB,
		"If enabled, enemies that explode on contact (like bombchus) will use super bomb explosions."
		" If disabled, they will use regular bomb explosions."},
	{ "Allow Placing Ganon as Screen Enemy", qr_CAN_PLACE_GANON,
		"If enabled, Ganon-type enemies are allowed to spawn when placed as a screen enemy."
		" If disabled, Ganon can only be spawned by Ganon type rooms or scripts."},
	{ "Allow Placing Traps as Screen Enemies", qr_CAN_PLACE_TRAPS,
		"If enabled, Trap-type enemies are allowed to spawn when placed as a screen enemy."
		" If disabled, Traps can only be spawned by Trap flags or scripts."},
	{ "Enemies check closest bait", qr_FIND_CLOSEST_BAIT,
		"If enabled, enemies will check all bait on screen and home towards"
		" the closest one in range if they have hunger. If disabled, they will"
		" instead check only the oldest-placed bait on the screen."},
	{ "Unbeatable enemies don't prevent enemies from respawning",  qr_UNBEATABLES_DONT_KEEP_DEAD,
		"If enabled, a screen that only has unbeatable enemies left will act as if it has"
		" 0 enemies left alive, allowing the enemies on the screen to respawn if the screen"
		" isn't one of the last 6 unique screen visited or if the Hero has F6'd."},
	{ "No 'Leave one enemy alive' trick",  qr_NO_LEAVE_ONE_ENEMY_ALIVE_TRICK,
		"If enabled, enemies will always return after either f6ing or if the current screen"
		" isn't one of the last 6 unique screens visited; regardless of how many enemies are"
		" left alive. Otherwise, if disabled, the enemies will only return if you either left"
		" 0 enemies alive on the screen, or if you left and re-entered the map."},
	{ "Enemy Drops use Hitbox for Position", qr_ENEMY_DROPS_USE_HITOFFSETS,
		"If enabled, enemies will drop their dropset item at the center of their hitbox."
		" \nIf disabled, they will drop their item at the center of their tile."},
	{ "Quake Hammer Stuns Leevers", qr_QUAKE_STUNS_LEEVERS,
		"If enabled, Quake Hammer can stun Leevers. This barely does anything if 'Leever's Still"
		" Submerge If Stunned' is enabled."},
		
	//Maybe we should keep this one last always? -Deedee 
	{ "Enemies Can Go Out of Bounds (Offscreen)", qr_OUTOFBOUNDSENEMIES,
		"If enabled, enemies can go out of bounds without getting removed. This is"
		" mainly intended for script use and should not be turned on unless a script"
		" requires it."}
};

static GUI::ListData itemRulesList
{
	{ "Enable Magic Counter", qr_ENABLEMAGIC,
		"Enables Magic. Magic drops become enabled in dropsets, items that"
		" use magic will check for it, and magic meters on subscreens"
		" are enabled. If disabled, items set to use magic as their cost"
		" are instead free, and nothing will drop magic."},
	{ "Enable Arrow Counter", qr_TRUEARROWS,
		"Enables arrow ammo. Arrow ammo becomes enabled in dropsets, and"
		" the Bow and Arrow both check and use this arrow counter. If disabled,"
		" the Bow and Arrow will instead use Rupees, as in NES Zelda."},
	{ "'Hearts Required' Affects Non-Special Items", qr_HEARTSREQUIREDFIX,
		"If enabled, the heart requirement check applies no matter where the item"
		" is picked up. If disabled, it only applies to Special Items in the Special"
		" Item Roomtype, specifically the ones given by Room Guys."},
	{ "Big Triforce Pieces", qr_4TRI,
		"If enabled, the triforce requirement for the Triforce Check roomtype is"
		" reduced, and each triforce that you pick up will fill in 2 slots instead"
		" of 1 on the subscreen triforce frame. The extra slot it fills is equal to"
		" the level number of the triforce you picked up + 4."},
	{ "3 or 6 Triforce Total", qr_3TRI,
		"If enabled, the triforce requirement for the Triforce Check roomtype is"
		" lowered to 6 triforce pieces, from 8. This is further reduced by the"
		" 'Big Triforce Pieces' rule, which halves the requirement to 3."},
	{ "Reroll Drops If Capacity is Zero", qr_SMARTDROPS,
		"If enabled, drops are rerolled if the item tries to increase a counter"
		" by 1 or more, but that counter has a max capacity of 0. A possible use"
		" for this would be having Bombs or Arrows not drop until you get the Bomb"
		" Bag or the Bow and Arrow, for example. Not to be confused with rerolling"
		" drops when that counter is full; see 'Reroll Drops If Capacity is Full' for"
		" that behavior."},
	{ "Reroll Drops If Capacity is Full", qr_SMARTER_DROPS,
		"If enabled, drops are rerolled if the item tries to increase a counter"
		" by 1 or more, but that counter is already at or above its max capacity."
		" This would, for example, prevent arrows from dropping if you already have"
		" max arrows, potentially rerolling it with a more useful item."},
	{ "Items Ignore Sideview Platforms", qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS,
		"If enabled, Items will fall through combos flagged with the 'Sideview Platform' flag"
		" or the 'Sideview Ladder' flag. If disabled, they will land on and not fall through"
		" either of these."},
	{ "Items Held Above Hero's Head Continue To Animate", qr_HOLDITEMANIMATION,
		"If enabled, when the Hero holds an item above their head, the item"
		" will continue to animate. If disabled, the item will be static and"
		" only use the first frame of their animation."},
	{ "Fairies spawn with random direction", qr_FAIRYDIR,
		"If enabled, fairies will spawn facing one of 8 directions randomly."
		" If disabled, fairies will spawn facing up, the default direction."},
	{ "Bottles can't be used with any maxed counter", qr_NO_BOTTLE_IF_ANY_COUNTER_FULL,
		"If enabled, bottled items cannot be used if ANY of the 3 (non-None) counters it fills "
		" are already at maximum capacity.\n"
		"Otherwise, bottled items can be used if AT LEAST ONE of its counters is not full."},
	{ "Whistle triggers can be permanent", qr_PERMANENT_WHISTLE_SECRETS,
		"If enabled, whistle triggers are allowed to be permanent. This means that whistle triggers"
		" will set the secret screen state, unless 'Screen->Secrets are Temporary' is checked for the"
		" current screen."},
	{ "Switchhooked Objects Run Scripts", qr_SWITCHOBJ_RUN_SCRIPT,
		"If enabled, the scripts of objects grabbed by the switchhook will not be paused."},
	{ "'Upgraded' items play upgraded pickup string", qr_ITEMCOMBINE_NEW_PSTR,
		"If enabled, collecting an item with 'Upgrade When Collected Twice' will play"
		" the pickup string from the 'upgraded' item, not the base item."},
	{ "'Upgraded' items chain continuously", qr_ITEMCOMBINE_CONTINUOUS,
		"If enabled, the 'Upgrade When Collected Twice' flag will be checked for"
		" the upgraded item from the same flag; allowing full progressive item chains." },
	{ "SCC/Bundle Item handles 'Upgraded' Items", qr_SCC_ITEM_COMBINES_ITEMS,
		"Granting an item via an Item Bundle or SCC will handle the"
		" 'Upgrade When Collected Twice' behavior." },
	{ "No Ammo Required to Display Subscreen Items", qr_NEVERDISABLEAMMOONSUBSCREEN,
		"If enabled, running out of ammo will not remove relevant items from the subscreen."
		" For example, running out of bombs will not remove the Bomb item from your inventory,"
		" and running out of Arrows will not remove the Bow and Arrow from your inventory."
		" \nOtherwise, if disabled, they will be hidden from your inventory and be unselectable"
		" if you run out of ammo, until you get more ammo for it."},
	{ "'Keep Lower Level Items' applies retroactively", qr_KEEPOLD_APPLIES_RETROACTIVELY,
		"If enabled, then when you have a higher-level item which does not"
		" have 'Keep Lower Level Items' checked, lower-level items will be"
		" removed when obtained." },
	{ "Only highest level item passive script runs", qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST,
		"If you have multiple items of the same itemclass with passive item scripts,"
		" only the highest-level item's script will run. In case of tie, all tied-highest"
		" scripts will run." }
};

static GUI::ListData miscRulesList
{
	{ "Messages Can Be Sped Up With The A Button", qr_ALLOWFASTMSG,
		"If enabled, holding the A button will prevent any message delay from occuring, displaying"
		" 60 characters per second."},
	{ "Messages Can Be Bypassed With The B Button", qr_ALLOWMSGBYPASS,
		"If enabled, pressing B during a message will cause the rest of the message to be displayed"
		" immediately with no regards for message speed."},
	{ "Messages Freeze All Action", qr_MSGFREEZE,
		"If enabled, everything on screen except for combo animation and combo cycling will freeze"
		" while a message is being displayed."},
	{ "Messages Disappear", qr_MSGDISAPPEAR,
		"If enabled, messages will disappear if their next string is '0'. This also has the side effect"
		" of removing the barrier Guys put across the top of the screen, with the exception of 'Feed"
		" the Goriya' rooms, which will still have this barrier. \nIf this rule is disabled, the current"
		" message will remain onscreen whenever the next string is '0', until an item is picked up."
		" Note that while picking up an item removes the string, it may not remove the Guy Barrier"
		" depending on whether or not 'Items Disappear During Hold-Up' is checked and depending on"
		" whether or not picking up an item is what solves the Guy Room."},
	{ "Goto SCCs respect 'continuation' flag", qr_SCC_GOTO_RESPECTS_CONTFLAG,
		"If enabled, a 'Goto' type String Control Code that switches to a string with the"
		" 'Is continuation of previous string' flag set will respect this, displaying"
		" the text from that string in the SAME message box, rather than a fresh one."},
	{ "More Sound Effects", qr_MORESOUNDS,
		"If enabled, more things use unique sounds. Summoning enemies use the Summon"
		" SFX instead of the Fire SFX, Fireballs and Rock Weapons have firing sounds,"
		" Traps make a sound when richocheting, Hammer makes a splash sound when pounding"
		" water, bushes/flowers/tall grass make a sound when cut, Hammer posts make"
		" a sound when pounded, the Lens of Truth play a sound when both used and unused,"
		" Push Blocks make a sound when pushed, Whistle Whirlwinds make a continuous sound when"
		" onscreen, Boomerangs make a continuous sound when onscreen, and all 3 Divine Spells"
		" (Divine Protection, Divine Escape, and Divine Fire) all use sound effects for everything they do."},
	{ "Fast Heart Refill", qr_FASTFILL,
		"If enabled, potions and fairy rings will restore half a heart every 6 frames while healing."
		" If disabled, they will restore half a heart every 22 frames."},
	{ "Show Time On Subscreen", qr_TIME,
		"If enabled, current Game Time can be shown on the subscreen. This displays how long you've"
		" been playing on that save file. Note that having cheats turned on will prevent the Game Time"
		" from displaying, even if this quest rule is enabled."},
	{ "Healing Fairy Heart Circle Is Centered Around Hero", qr_HEARTRINGFIX,
		"If enabled, the circle of hearts that display around the Hero when they step on a Fairy"
		" Circle will be centered around the Hero. If disabled, they will center on the center of the"
		" screen, or 125, 88."},
	{ "No Healing Fairy Heart Circle", qr_NOHEARTRING,
		"If enabled, the circle of hearts that normally display around the Hero when they step on a Fairy" 
		" Circle will not display."},
	{ "No Saving When Continuing", qr_NOSAVE,
		"If enabled, removes the 'Save' option from the Game Over screen. Saving must be done through other methods."},
	{ "Return To File Select On Death/F6", qr_NOCONTINUE,
		"If enabled, the Continue Screen is skipped, and the Hero is returned to the file select screen on Death or Game->End."
		" Note that this means all progress is lost since last save."
		"\nIf an 'onF6Menu' script is assigned, this prevents the"
		" engine F6 menu from appearing after the scripted one is cancelled."},
	{ "Instant Reload On Death", qr_INSTANT_RESPAWN,
		"If enabled, dying will reload your last save immediately after the death animation finishes, losing all progress since the last save."
		" Having both this and Instant Continue on Death enabled will cancel each other out."},
	{ "Instant Continue on Death", qr_INSTANT_CONTINUE,
	    "If enabled, dying will respawn you immediately after the death animation finishes, skipping the Game Over screen."
		" Having both this and Instant Reload on Death enabled will cancel each other out."},
	{ "No Fires In Special Rooms", qr_NOGUYFIRES,
		"If enabled, Guys in Special Rooms will not create Flames. This also means they cannot shoot fireballs when hit."},
	{ "Special Room Guys Don't Create A Puff When Appearing", qr_NOGUYPOOF,
		"If enabled, Special Room Guys and their Fires will not make a spawn poof when entering the screen."},
	{ "Log Game Events To Allegro.log And Console", qr_LOG,
		"Enables the logging of certain Game Events to both allegro.log and the ZC Console."
		" Note that turning this on can result in slower performance. Logging includes:"
		"\n- Whether or not a 'Hit All Triggers' has been fulfilled and, if not, how many are left;"
		"\n- The Screen and Map number whenever screens are changed, along with the current Screen States of that screen;"
		"\n- The setting or unsetting of Screen States, and whether or not a Screen State Carryover happened and where it happened;"
		"\n- Any warps that happen, including what type of warp it is and where the warp took you;"
		"\n- Any items you pick up, including both the ID and the Item Name;"
		"\n- Any time the continue point is set, including what it's been set to;"
		"\n- Any time a script creates an object, including the object's name and its UID (not to be confused with ID);"
		"\n- and lastly, any time a script tries to reference a nonexistent object."},
	{ "Log Script Errors To Allegro.log And Console", qr_SCRIPTERRLOG,
		"If enabled, various script errors are logged to Allegro.log and the ZC Console."
		" The list of what exactly is logged is too long to list here, but it includes errors like"
		" arrays being too small, invalid arguments being passed, ZASM overflow, etc."},
	{ "Draining Rupees Can Still Be Spent", qr_SHOPCHEAT,
		"If enabled, you can spend rupees while they are draining, even if after the drain"
		" would finish you'd be unable to afford the shop item. As an example, if you have"
		" 500 rupees and you buy a 300 rupee item, you'd normally be left with 200 rupees;"
		" but if the Hero is fast enough, they can buy an item with their still-draining"
		" money, possibly buying an item worth more than the 200 rupees they'd be left with."
		" \nIf disabled, shops will check both your rupee count and your rupee drain amount to"
		" make sure you can afford whatever the Hero is buying."},
	{ "Triforce in Cellar Warps Hero Out", qr_SIDEVIEWTRIFORCECELLAR,
		"If enabled, the Triforce can warp you out of passageways if 'Side Warp Out' is checked on"
		" the Triforce item. Otherwise, if disabled, you will stay in the passageway after the animation"
		" ends, regardless of whether 'Side Warp Out' is checked."},
	{ "Reduced Flashing (Epilepsy Protection)", qr_EPILEPSY,
		"If enabled, certain features in the game are limited visually to make things more epilepsy-friendly"
		" for players. This includes capping the intensity of wavy effects, reducing the triforce flashes, and more."
		" Note that players can turn on this rule themselves in the Hero; if you desire consistency, you should turn"
		" this quest rule on and design around it."},
	{ "Screen->Wavy Intensity is not affected by Epilepsy Protection", qr_WAVY_NO_EPILEPSY,
		"If enabled, the intensity of wavy effects is not capped by epilepsy protection. The cap is at 2048*sin for"
		" non-epilepsy mode, and 16*sin for epilepsy. Enabling this will use the 2048 cap even in epilepsy mode."},
	{ "Screen->Wavy Speed is not affected by Epilepsy Protection", qr_WAVY_NO_EPILEPSY_2,
		"If enabled, the speed of wavy effects is not halved by epilepsy protection. Epilepsy protection normally halves"
		" the speed of wavy effects; enabling this will bypass this behavior."},
	{ "Disable Fast Mode (Uncap)", qr_NOFASTMODE,
		"If enabled, the built in fast-forward key ('~', or Tilde) is prevented from working while the quest is loaded,"
		" and FPS is forced to be throttled at 60 FPS. Many players fast forward to speed things up, especially during"
		" many built-in cutscenes such as triforce obtaining and the ending. Enabling this will disable this, which is"
		" helpful if you want to control the speed the game is played at."},
	{ "Allow permanent secrets on Dungeon-type dmaps", qr_DUNGEON_DMAPS_PERM_SECRETS,
		"If enabled, Dungeon-type DMaps can have secrets be permanent. Normally Dungeon DMaps are hardcoded to not allow"
		" permanent secrets to mimic Zelda 1, but the Temp Secrets screen flag makes this behavior not only irrelevant,"
		" but a design hindrance. Enabling this will remove this hardcode and let Dungeon DMaps have permanent secrets."},
	{ "No Scrolling Screen While In Air", qr_NO_SCROLL_WHILE_IN_AIR,
		"If enabled, you cannot scroll the screen while in midair. Normally the Roc's Feather is a nightmare to design around"
		" as you can jump over water and pits, scroll the screen, and set your respawn point over top of a pit, or even sequence"
		" break where you aren't supposed to by drowning in water on the next screen, and jumping again after respawning. Enabling"
		" this will prevent you from changing screens while jumping or otherwise in the air, allowing you to design around the feather easier."},
	{ "No Scrolling Screen While Carrying", qr_NO_SCROLL_WHILE_CARRYING,
		"If enabled, you cannot scroll the screen while the Hero is carrying something with a Lift Glove."},
	{ "Carryables Can't Leave Screen", qr_CARRYABLE_NO_ACROSS_SCREEN,
		"If enabled, leaving a screen while carrying something with a Lift Glove will delete the carried object."},
	{ "New Dark Rooms", qr_NEW_DARKROOM,
		"If enabled, Dark Rooms behave less like Z1 and more like LttP/Minish Cap, having light circles around torches and giving a customizeable"
		" spotlight around the Hero. If disabled, Z1-styled dark rooms are used, which only darken the screen until lit up with a candle."},
	{ "New Darkness Draws Under Layer 7", qr_NEWDARK_L6,
		"If enabled, the new dark rooms will draw under layer 7, allowing scripts to draw over dark rooms by drawing to Layer 7."
		" If disabled, scripts are never able to draw above the new dark rooms."},
	{ "New Darkness Trans Layer Stacking", qr_NEWDARK_TRANS_STACKING,
		"If enabled, new dark rooms with multiple transparent layers will stack the transparency. Otherwise,"
		" the transparency will not stack, drawing only one layer." },
	{ "Item Cellars/Passageways/Caves use own music", qr_SCREEN80_OWN_MUSIC,
		"If enabled, screen 80/81 will play a midi if one is assigned to screen 80/81."},
	{ "New Darkroom Lanterns Cross Screen Boundary", qr_NEWDARK_SCROLLEDGE,
		"When in a dark room with 'New Dark Rooms' enabled, lanterns will light across"
		" the boundary between screens *during scrolling*." },
	{ "Restarting Level always goes to DMap continue point", qr_LEVEL_RESTART_CONT_POINT,
		"Effects like Wallmasters and Divine Escape will go back to the DMap's"
		" continue point, rather than the last entrance point." },
	{ "Flip 'don't restart dmap script' script warp flag", qr_SCRIPT_WARPS_DMAP_SCRIPT_TOGGLE,
		"If enabled, the warp flag to disable restarting the dmap script when warping to the same dmap will be flipped"
		" so that having the flag on will restart the dmap script and having it off won't restart when warping to the same dmap."},
	{ "Fast Counter Drain", qr_FASTCOUNTERDRAIN,
		"If enabled, counters drain/refill at quadruple speed."}
};

static GUI::ListData nesfixesRulesList
{
	{ "Freeform Dungeons", qr_FREEFORM,
		"If enabled, dungeons lose the hardcoded behaviors regarding the edge of the screen. Normally, when disabled,"
		" the Hero can only walk towards or away from the edge of the screen when less than 2 tiles away from the edge"
		" of the screen, you cannot walk upwards onto the topmost two rows of combos except where the northern door would be,"
		" and all combos on the edge of the screen draw over sprites. Enabling this rule disables these behaviors."},
	{ "Can Safely Trigger Armos/Grave From The South", qr_SAFEENEMYFADE,
		"If enabled, fade-spawning enemies have no collision until after they finish spawning, preventing the Hero from taking"
		" damage from these enemies if they spawn from graves or armos and the Hero activates them from the south."},
	{ "Can Use Items/Weapons on Edge of Screen", qr_ITEMSONEDGES,
		"If enabled, you can use items when in doorways or on screen edges. Normally, you can only use items when facing in a"
		" direction perpendicular to the screen edge you'e close to when you're within 2 tiles of the screen edge, and you can't"
		" use items at all when in the corner of the screen. Enabling this will disable this behavior and allow you to use items"
		" and weapons on the screen edge."},
	{ "Fix Hero's Position in Dungeons", qr_HERODUNGEONPOSFIX,
		"If disabled, and the rule 'BS-Zelda Animation Quirks' is disabled, the Hero gains a -2 y offset when in dungeons."
		"Having either this or 'BS-Zelda Animation Quirks' enabled will disable this -2 y offset in dungeons."},
	{ "Raft/Ladder Sprite Direction Fix", qr_RLFIX,
		"If enabled, the Raft and Ladder will rotate their sprites when going left or right."
		" If disabled, it will use the same sprite and same rotation regardless of which direction it goes in."},
	{ "Level 3 CSet 6 Fix", qr_NOLEVEL3FIX,
		"When disabled, color 2 of CSet 6 will use color 7 of CSet 3 when in a dmap with a level of 3."
		" If enabled, this behavior is ignored."},
	{ "Hero Holds Special Bombs Over Their Head", qr_BOMBHOLDFIX,
		"If enabled, the Hero can hold bombs over their head if the bomb is a Special Item in a Special Item Room."
		" Otherwise, bombs will only be held over head if the Hero dives for them."},
	{ "Holding Up Items Doesn't Restart Music", qr_HOLDNOSTOPMUSIC,
		"If enabled, holding up an item will not restart the currently playing music."
		" If disabled, holding up an item will restart the current music."},
	{ "Leaving Item Cellar/Passageway Doesn't Restart Music", qr_CAVEEXITNOSTOPMUSIC,
		"If enabled, leaving a Screen 80 room (such as an Item Cellar, Cave, or Passageway) will not restart"
		" the currently playing music. Otherwise, if disabled, it will restart the music."},
	{ "Tunic Color Can Change In Overworld Caves", qr_OVERWORLDTUNIC,
		"If enabled, Tunic Color can change within screen 80 caves on overworld dmaps when obtaining a new tunic."
		" Otherwise, if disabled, it will only update once the cave is left."},
	{ "Sword/Wand Flip Fix", qr_SWORDWANDFLIPFIX,
		"If enabled, the downwards sword/wand sprite is flipped both vertically and horizontally from the upwards sprites."
		" Otherwise, if disabled, it will only flip the sprite vertically."},
	{ "Push Block CSet Fix", qr_PUSHBLOCKCSETFIX,
		"If enabled, pushed blocks will use the CSet of their combo when pushed. Otherwise, if disabled, they will use CSet 9."},
	{ "Trap Position Fix", qr_TRAPPOSFIX,
		"If enabled, Traps don't have the -2 Y offset that all enemies spawn with in topdown gravity. If disabled, they will"
		" have the -2 Y offset all enemies usually spawn with."},
	{ "No Invisible Border on Non-Dungeon DMaps", qr_NOBORDER,
		"Normally, there is a 1 tile border surrounding the edge of the screen that prevents enemies from walking on the"
		" screen edge, a 16 pixel border that kills the Hero's weapons around the edge of the screen early, and a 8"
		" pixel border that kills enemy weapons around the edge of the screen early. Additionally, there is a change to"
		" how close to the edge of the screen you are allowed to use melee weapons if the NES Fix 'Can Use Items/Weapons on Edge of"
		" Screen' is enabled, increasing the distance you need to be away from the edge by 8 pixels (or half a tile)."
		"\nEnabling this rule will disable these behaviors."},
	{ "Items Disappear During Hold-Up", qr_OLDPICKUP,
		"If enabled, all items are deleted whenever you hold up an item. If disabled, they won't be deleted when you hold"
		" up an item. Is required to make Shop Items disappear after buying one if 'Shop Items Disappear on Pickup' is disabled."},
	{ "Subscreen Appears Above Sprites", qr_SUBSCREENOVERSPRITES,
		"If enabled, the subscreen is drawn above Layer 6, and the use of Layer 7 is enabled."
		" If disabled, the subscreen is drawn above layer 4 but below flying/jumping enemies,"
		" an airborne Hero, etc; and Layer 7 is not drawn."},
	{ "Shielded Enemies Check Bomb's Position Instead Of Direction", qr_BOMBDARKNUTFIX,
		"If enabled, bomb collision with shielded enemies is checked by comparing the angle of the"
		" explosion to the enemy, and uses the direction of that angle to check if the enemy would"
		" be shielded in that direction. If disabled, the bomb will use the direction the Hero was"
		" facing when the bomb was placed when comparing if the enemy is shielded in that direction."
		"\nPlease note that regardless of this Quest Rule, shielded enemies will always be shown as"
		" taking damage regardless of if they blocked it and were dealt no damage, unless the NES Fix"
		" 'Blocked Bombs Don't Trigger Shielded Enemies' I-Frames' is checked."},
	{ "Blocked Bombs Don't Trigger Shielded Enemies' I-Frames", qr_TRUEFIXEDBOMBSHIELD,
		"If enabled, shielded enemies will not be shown as taking damage when they"
		" block an explosion and take 0 damage. If disabled, they will appear to be damaged"
		" whenever an explosion hits them, even if they are not actually being damaged because"
		" of their shield blocking the explosion."},
	{ "Correct Offset Enemy Weapon Collision Detection", qr_OFFSETEWPNCOLLISIONFIX,
		"If enabled, hit offsets of enemy weapons are changed, preventing a common exploit of the Hero"
		" standing in between two tiles to avoid getting hit by certain projectiles. If disabled, the Hero"
		" can stand in between two tiles to avoid certain projectiles."},
	{ "Special Items Don't Appear In Passageways", qr_ITEMSINPASSAGEWAYS,
		"If enabled, special items do not appear in passageways if a screen has both a special item and a"
		" passageways. Passageways are screen 81 on dmaps with 'Use Caves Instead Of Item Cellars' unchecked."
		" If disabled, special items can appear in passageways."},
	{ "No NES Sprite Flicker", qr_NOFLICKER,
		"Normally, the drawing order for enemies, items, hookshot chainlinks, and weapons alternate every frame."
		" The order alternates between 'Behind Weapons, Shadows, Enemies, Chainlinks, Non-Behind Weapons, Items'"
		" and 'Behind Weapons, Shadows, Items, Chainlinks, Enemies, Non-Behind Weapons'. If this rule is enabled,"
		" the drawing order is forced to be the former order regardless of frame."},
	{ "Invincible Hero Isn't Hurt By Own Fire Weapons", qr_FIREPROOFHERO2,
		"Normally, the Hero can be hurt by their own fire weapons even if they are invincible from some source,"
		" like the clock item or scripts. Enabling this rule disables this, making them actually invincible."},
	{ "No Position Offset Of Screen Items", qr_NOITEMOFFSET,
		"If enabled, items gain a -1 Y offset to counterbalance a +1 Y offset they would normally otherwise have."},
	{ "Allow Ladder Anywhere", qr_LADDERANYWHERE,
		"If enabled, the ladder can be used anywhere, and the screen flag 'Toggle 'Allow Ladder'' will disable the"
		" ladder on the screen instead of enabling it. If disabled, only screens with 'Toggle 'Allow Ladder'' and"
		" screens on dungeon dmaps allow the use of the Ladder."},
	{ "Lanmolas/Traps don't keep room dead.", qr_NOTMPNORET,
		"If enabled, Lanmolas don't keep the entire room marked as dead when"
		" you kill one, and rooms with traps don't keep the room marked as dead"
		" when you kill the entire room. If disabled, either of these will keep"
		" enemies from respawning until you leave and re-enter the dungeon."},
	{ "Enemies Don't Flash When Dying", qr_NOFLASHDEATH,
		"If enabled, the cset of the enemy's death animation will not flash."
		" This rule is ignored if the rule 'BS-Zelda Animation Quirks'"
		" is enabled, as enemies will not flash during their death animation"
		" if that rule is enabled regardless."},
	{ "No Statue Minimum Range Or Double Fireballs", qr_BROKENSTATUES,
		"If enabled, statues will still fire at the Hero even if the Hero is close."
		" Statues also have a 1 in 16 chance to double shoot if enabled, with the second"
		" shot being 4 pixels to the left. If disabled, statues cannot double shoot, and"
		" they have a minimum range where they won't fire if the Hero is within 24 pixels"
		" of the statue."},
	{ "Killing Stunned Enemy With Melee Weapons Won't Hurt Hero", qr_DYING_ENEMIES_IGNORE_STUN,
		"If enabled, dying enemies won't damage the Hero if they are stunned, the Hero is"
		" on top of them, and the Hero kills them with a melee weapon. Presumably this was"
		" a feature to replicate NES behavior."},
	{ "Shop Items Disappear on Pickup", qr_SHOP_ITEMS_VANISH,
		"If enabled, all items will vanish after picking up a shop item. If both this and"
		" 'Items Disappear During Hold-Up' are disabled, shop items will not disappear when"
		" you buy one."},
	{ "Expanded Hero Tile Modifiers", qr_EXPANDEDLTM,
		"If enabled, Hero Tile Modifiers from items (such as shields) will always be applied to the Hero."
		" \nIf disabled, they will only be applied if the Hero is walking or standing (either on land or while"
		" sideswimming), and only if the Hero is not facing up."},
};

static GUI::ListData playerRulesList
{
	{ "Diagonal Movement", qr_LTTPWALK,
		"If enabled, disables the built in Hero gridlock, and allows the Hero to move diagonally."
		" \nDiagonal Movement also uses different logic for determining how many pixels the Hero should"
		" move from  non-Diagonal Movement (or '4-way Movement', from here out) when 'New Hero Movement'"
		" is disabled; 4-way Movement gives a different step speed depending on the Hero's current X/Y"
		" position, usually averaging out to 1.3333 pixels of movement per frame; while Diagonal Movement"
		" alternates between 1 and 2 pixels every frame, averaging out to 1.5 pixels of movement per frame."
		" \nIf 'New Hero Movement' is enabled, there is no speed difference between 4-Way Movement and"
		" Diagonal Movement, as the Hero's position and speed use decimal precision."},
	{ "Large Hitbox", qr_LTTPCOLLISION,
		"If enabled, the Hero's walking hitbox is changed from 16x8 (the bottom half of the Hero's sprite)"
		" to 16x16 (their entire sprite). The Hero cannot walk halfway into a solid combo from the bottom, and"
		" the collision detection of certain combos and flags is modified to account for this change of hitbox."},
	{ "New Hero Movement", qr_NEW_HERO_MOVEMENT,
		"Alters the Hero's movement; with this enabled, movement includes decimal precision,"
		" diagonal movement is 'smoother', and the Hero's speed can be adjusted using"
		" 'Hero->Step' via ZScript, as well as the 'Hero Step' option in 'Init Data'." },
	{ "Newer Hero Movement", qr_NEW_HERO_MOVEMENT2,
		"Alters the Hero's movement; with this enabled, most of the effects of 'New Hero Movement' and 'Disable 4-Way Movement's Gridlock'"
		" apply, and the Hero's collision code is newly cleaned up."+QRHINT({qr_NEW_HERO_MOVEMENT,qr_DISABLE_4WAY_GRIDLOCK}) },
	{ "Disable 4-Way Movement's Gridlock", qr_DISABLE_4WAY_GRIDLOCK,
		"If enabled, disables the built in Hero gridlock. This does not allow the Hero to move diagonally,"
		" but it does allow them to change direction when not aligned with the 8x8 pixel grid."},
	{ "Invincible Hero Flickers", qr_HEROFLICKER,
		"If enabled, the Hero will flicker when invincible or after taking damage instead of flashing colors."}
};

static GUI::ListData subscrRulesList
{
	//Compat stuff first
	{ "Old Engine Subscreen Quirks", qr_OLD_SUBSCR,
		"If enabled, the subscreen behaves in a way needed for compat."},
	{ "Old Subscreen Selector", qr_SUBSCR_OLD_SELECTOR,
		"If disabled, the subscreen selector will stretch to fit the item it is selecting." },
	{ "Old Subscreen Item ID order", qr_SUBSCR_BACKWARDS_ID_ORDER,
		"If enabled, the subscreen will prioritize lower-ID items when there is a 'Level' tie, instead"
		" of prioritizing higher-ID items like everything else in the engine." },
	//Then more option-y stuff
	{ "Item 0 is invisible on buttons", qr_ITM_0_INVIS_ON_BTNS,
		"If enabled, Item 0 will not be drawn if equipped to a button."},
	{ "Freeform Subscreen Cursor", qr_FREEFORM_SUBSCREEN_CURSOR,
		"If enabled, the subscreen cursor can move freely even to empty spots."},
	{ "Always Press To Equip", qr_SUBSCR_PRESS_TO_EQUIP,
		"If enabled, you must always press a button to equip an item, even if only B items are enabled."},
	{ "No Button Verification", qr_NO_BUTTON_VERIFY,
		"If enabled, the engine won't attempt to forcibly fill empty buttons with items."
		"\nAlso allows unequipping items by trying to equip it to the same button again."},
	{ "Old Gauge Tile Layout", qr_OLD_GAUGE_TILE_LAYOUT,
		"If enabled, 'Gauge' type widgets on the subscreen will use the old-style layout."},
	
	{ "Allow Setting A Button Items", qr_SELECTAWPN,
		"If enabled, you can select what item goes on the A button on the subscreen."
		" If disabled, the A Button is hardcoded to the highest leveled Sword item"
		" that you possess, and you are prevented from equipping Sword-class items"
		" on the B button."},
	{ "Allow Setting X Button Items", qr_SET_XBUTTON_ITEMS,
		"If enabled, allows setting items to the X (Ex1) button. If disabled, the"
		" X/Ex1 button has no usage outside of scripting."},
	{ "Allow Setting Y Button Items", qr_SET_YBUTTON_ITEMS,
		"If enabled, allows setting items to the Y (Ex2) button. If disabled, the"
		" Y/Ex2 button has no usage outside of scripting."},
	{ "Enable 1/4 Heart Life Bar", qr_QUARTERHEART,
		"If enabled, the Life Meter subscreen widget will display quarter hearts if the Hero"
		" has that much life. Otherwise, it will only display half and full hearts, rounding up."},
	{ "No L/R Inventory Shifting", qr_NO_L_R_BUTTON_INVENTORY_SWAP,
		"If enabled, L/R item switching is disabled. Normally, you can quickly change what item is selected to the B button"
		" by pressing L and R to shift the selection left and right. Enabling this disables this behavior, allowing the usage"
		" of L and R for more scripted purposes."},
	{ "Ex3 and Ex4 Shift A-Button Items", qr_USE_EX1_EX2_INVENTORYSWAP,
		"If enabled, Ex3 and Ex4 will shift the item equipped on the A button to whatever item is left or right of it on the"
		" subscreen, without having to open the subscreen. This is similar to the existing L/R B-Button shifting, but for the"
		" A button. Disabling this leaves Ex3 and Ex4 open for scripted usage."},
	{ "Higher Maximum Playtime", qr_GREATER_MAX_TIME, 
		"Bumps up the Max Playtime from 99 hours, 5 minutes, and 54 seconds, to 9000 hours."
		"Has no downsides, is only here for compatibility sake."},
};

static GUI::ListData weaponsRulesList
{
	{ "Prisms Reflect Angular Weapons At Angles", qr_ANGULAR_REFLECTED_WEAPONS,
		"If enabled, Prisms will reflect and duplicate angular weapons, creating new weapons at"
		" angles perpendicular to the angle of the original weapon. If disabled, they will reflect"
		" at regular directions instead of at angles."
		" \nNote that this behavior is broken and undefined if ''Prisms Reflect Angular Weapons' Uses Broken Logic' is checked."},
	{ "Mirrors Use Weapon Center for Collision", qr_MIRRORS_USE_WEAPON_CENTER,
		"If enabled, mirror/prism combos activate when the center of a weapon hits them,"
		" instead of when the edge hits them." },
	{ "Weapons Cannot Stunlock Enemies", qr_NO_STUNLOCK,
		"If enabled, enemies that are already stunned block any further stunning attacks."
		" This prevents re-stunning them to keep them stunlocked." },
	{ "Arrows Always Penetrate", qr_ARROWS_ALWAYS_PENETRATE,
		"If enabled, arrows will always penetrate enemies, regardless of if they belong to"
		" an arrow item with the 'Penetrate Enemies' flag checked or not." },
	{ "Swordbeams Always Penetrate", qr_SWORDBEAMS_ALWAYS_PENETRATE,
		"If enabled, swordbeams will always penetrate enemies, regardless of if they belong to"
		" a sword item with the 'Penetrate Enemies' flag checked or not." },
	{ "Bombs Pierce Enemy Shields", qr_BOMBSPIERCESHIELD,
		"If enabled, bombs will ignore enemy shields and deal damage to the enemy, if the enemy's"
		" bomb defense allows it to be hurt by bombs. Otherwise, if disabled, bombs are subject to"
		" being blocked by the enemy's shield; how they are blocked depends on the NES Fixes 'Shielded"
		" Enemies Check Bomb's Position Instead Of Direction' and 'Blocked Bombs Don't Trigger Shielded"
		" Enemies' I-Frames'."},
	{ "Scripted and Enemy Fire Lights Temporarily", qr_TEMPCANDLELIGHT,
		"If enabled, script created and enemy created fires will only light up"
		" the room temporarily, and will unlight the room when the last one"
		" disappears. This used to affect Hero-created fire, but that functionality"
		" can now be toggled on a per-candle or per-book basis in the item editor."},
	{ "Scripted Fire LWeapons Don't Hurt Hero", qr_FIREPROOFHERO,
		"If enabled, script-created Fire LWeapons will not hurt the Hero. In the original"
		" NES Zelda, the Hero took damage from their own fire weapons. Though Candle and Book"
		"-created fire weapons are determined by the item editor for those items now, this rule"
		" still affects script-created Hero fire weapons."
		" If disabled, the Hero will take damage from script created Hero fire weapons, even though"
		" they are Hero weapons and not enemy weapons."},
	{ "Scripted Bomb LWeapons Hurt Hero", qr_OUCHBOMBS,
		"If enabled, script-created Hero Bomb Explosions hurt the Hero. This mimics more modern"
		" Zelda games where the players own bombs can hurt them. This used to affect Hero-placed bombs,"
		" but that functionality has since been moved to the bomb item in the item editor. This rule now"
		" only determines if script-created Hero bombs can hurt the Hero."},
	{ "Lifted Bombs Explode In Hand", qr_HELD_BOMBS_EXPLODE,
		"If enabled, bomb lweapons that are 'lifted' by the Hero (currently script-only)"
		" will still tick down their fuse and explode." },
	{ "Enemy Fire Lights New Dark Rooms", qr_EW_FIRE_EMITS_LIGHT,
		"If enabled, enemy fire weapons will emit light by default, as Hero fire does." },
	
	//should maybe keep these last
	{ "Scripted and Enemy Boomerangs Have Corrected, Non-Hardcoded Animation", qr_CORRECTED_EW_BRANG_ANIM,
		"If enabled, Script-created and Enemy-created boomerangs use the same animation as other weapons."
		" \nIf disabled, they will use one of two hardcoded animations depending on whether or not 'BS-Zelda"
		" Animation Quirks' is checked or not. \nIf 'BS-Zelda Animation Quirks' is enabled, it will use one"
		" tile and just flip it, with it alternating between no flip (flip of 0), vertical flip (flip of 2),"
		" vertical and horizontal flip (flip of 3), and lastly horizontal flip (flip of 1); alternating every"
		" 4 frames through 4 different flips for an animation that lasts 16 frames. \nIf 'BS-Zelda Animation "
		" Quirks' is disabled, it will use a combination of 3 different tiles and different flip states to get"
		" an animation that has 8 different 'tiles'/frames that it alternates between every 2 frames, for an"
		" animation that, again, lasts 16 frames total. The exact offset from the original tile and the flip"
		" it uses is complicated, but will be listed below in the format of '({offset from o_tile}, {flip value})';"
		" in which {offset from o_tile} is the tile difference between the tile used and the sprite's o_tile,"
		" and {flip value} is the value of the flip, in which flip of 0 is no flip, flip of 1 is horizontal, 2"
		" is vertical, and 3 is both horizontal and vertical."
		" \nThe exact offsets and flip values are, as follows: (0, 0), (1, 0), (2, 0), (1, 1), (0, 1), (1, 3), (2, 2), (1, 2)."
		" \nAgain, none of this matters if you have this rule enabled, as enabling this rule will disable these"
		" hardcoded values, and will allow you to use the same animation system as every other weapon."
		" If looking for the Hero-created weapons version of this, look at the boomerang itemclass."},
	{ "Weapons Move Offscreen (Buggy, use at own risk)", qr_WEAPONSMOVEOFFSCREEN,
		"If enabled, weapons can go out of bounds without getting removed. This is"
		" mainly intended for script use and should not be turned on unless a script"
		" requires it. This can also cause certain issues with weapon types that only"
		" allow one onscreen at a time, such as sword beams and etc."}
};

static GUI::ListData combinedRulesList;
static bool inited_combined_rules = false;
GUI::ListData const& combinedQRList()
{
	if(!inited_combined_rules)
	{
		animRulesList.tag(rules_anim);
		comboRulesList.tag(rules_combo);
		compatRulesList.tag(rules_compat);
		enemiesRulesList.tag(rules_enemies);
		itemRulesList.tag(rules_item);
		miscRulesList.tag(rules_misc);
		nesfixesRulesList.tag(rules_nesfix);
		playerRulesList.tag(rules_hero);
		subscrRulesList.tag(rules_subscr);
		weaponsRulesList.tag(rules_weapon);
		
		combinedRulesList = animRulesList + comboRulesList + compatRulesList
			+ enemiesRulesList + itemRulesList + miscRulesList + nesfixesRulesList
			+ playerRulesList + subscrRulesList + weaponsRulesList;
		combinedRulesList.alphabetize();
		inited_combined_rules = true;
	}
	return combinedRulesList;
}
//}
//{ Script Rules

GUI::ListData scriptRulesList
{
	{ "Item Scripts Continue To Run", qr_ITEMSCRIPTSKEEPRUNNING },
	{ "Clear InitD[] on Script Change", qr_CLEARINITDONSCRIPTCHANGE },
	{ "Hero OnDeath script runs AFTER engine death animation", qr_ONDEATH_RUNS_AFTER_DEATH_ANIM },
	{ "Passive Subscreen Script runs during Active Subscreen Script", qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT },
	{ "DMap Active Script runs during Active Subscreen Script", qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT },
	{ "Combos Run Scripts on Layer 0", qr_COMBOSCRIPTS_LAYER_0 },
	{ "Combos Run Scripts on Layer 1", qr_COMBOSCRIPTS_LAYER_1 },
	{ "Combos Run Scripts on Layer 2", qr_COMBOSCRIPTS_LAYER_2 },
	{ "Combos Run Scripts on Layer 3", qr_COMBOSCRIPTS_LAYER_3 },
	{ "Combos Run Scripts on Layer 4", qr_COMBOSCRIPTS_LAYER_4 },
	{ "Combos Run Scripts on Layer 5", qr_COMBOSCRIPTS_LAYER_5 },
	{ "Combos Run Scripts on Layer 6", qr_COMBOSCRIPTS_LAYER_6 },
	{ "Use Old Global Init and SaveLoad Timing", qr_OLD_INIT_SCRIPT_TIMING },
	{ "Passive Subscreen Script runs during wipes/refills", qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN },
	{ "Scripts use 6-bit color (0-63) instead of 8-bit (0-255)", qr_SCRIPTS_6_BIT_COLOR},
};

GUI::ListData instructionRulesList
{
	{ "No Item Script Waitdraw()", qr_NOITEMWAITDRAW },
	{ "No FFC Waitdraw()", qr_NOFFCWAITDRAW },
	{ "Old eweapon->Parent", qr_OLDEWPNPARENT },
	{ "Old Args for CreateBitmap() and bitmap->Create()", qr_OLDCREATEBITMAP_ARGS },
	{ "Print Script Metadata on Traces", qr_TRACESCRIPTIDS },
	{ "Writing to INPUT Overrides Drunk State", qr_FIXDRUNKINPUTS },
	{ "Don't Allow Setting Action to Rafting", qr_DISALLOW_SETTING_RAFTING },
	{ "Writing npc->Defense[NPCD_SCRIPT] Sets All Script Defences", qr_250WRITEEDEFSCRIPT },
	{ "Writing npc->Weapon Sets its Weapon Sprite", qr_SETENEMYWEAPONSPRITESONWPNCHANGE },
	{ "Writing npc->Weapon Sets its Weapon Sound", 	qr_SETENEMYWEAPONSOUNDSONWPNCHANGE },
	{ "Broken DrawInteger and DrawCharacter Scaling", qr_BROKENCHARINTDRAWING },
	{ "npc->Weapon Uses Sprite 246-255 for EW_CUSTOM*", qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES },
	{ "All bitmap-> and FileSystem-> paths relative to quest 'Files' folder", qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE },
	{ "Don't allow overwriting hopping action", qr_NO_OVERWRITING_HOPPING },
	{ "Sprite->Step uses new, precise values", qr_STEP_IS_FLOAT },
	{ "Old printf() args", qr_OLD_PRINTF_ARGS,
		"Compat, turns off new printf features." },
	{ "No printf() 0-fill", qr_PRINTF_NO_0FILL,
		"Instead of filling all extra arguments with '0',"
		" just leave them unchanged." },
	{ "Writing Screen->EntryX, EntryY Resets Spawn Points", qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS },
	{ "Broken Combodata->InitD[]", qr_COMBODATA_INITD_MULT_TENK },
	{ "Script writes to Hero->Step don't carry over", qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER },
	{ "Disable accessing negative array indexes", qr_ZS_NO_NEG_ARRAY, "If enabled,"
		" the new feature allowing accessing negative indexes of an array to backwards-index them"
		" (i.e. 'arr[-1]' is the LAST element in the array) will be DISABLED if this is on."
		"\nUseful for debugging if you're using this feature *by mistake*." },
	{ "Game->Generic[GEN_CONTINUEHEARTS] is in 'Hearts'", qr_SCRIPT_CONTHP_IS_HEARTS,
		"If checked, read/write to 'Game->Generic[GEN_CONTINUEHEARTS]' is in 'Hearts'. Otherwise,"
		" it will be in 'HP'. (Has no effect if 'Game->Generic[GEN_CONTINUEISPERCENT]' is true)"}
};

GUI::ListData objectRulesList
{
	{ "Sprite Coordinates are Float", qr_SPRITEXY_IS_FLOAT },
	{ "Weapons Have Shadows", qr_WEAPONSHADOWS },
	{ "Items Have Shadows", qr_ITEMSHADOWS },
	{ "Weapons Live One Extra Frame With WDS_DEAD", qr_WEAPONS_EXTRA_FRAME }
};

GUI::ListData drawingRulesList
{
	{ "Scripts Draw When Stepping Forward In Dungeons", qr_SCRIPTSRUNINHEROSTEPFORWARD },
	{ "Scripts Draw During Warps", qr_SCRIPTDRAWSINWARPS },
	{ "Scripts Draw During Frozen Messages", qr_SCRIPTDRAWSFROZENMSG,
		"If checked, script draws will continue to clear during frozen messages. "
		"Otherwise, they will not clear until the message has closed and all draws during "
		"the message will be ignored." + QRHINT({qr_MSGFREEZE})}
};

GUI::ListData bugfixRulesList
{
	{ "Fix Scripts Running During Scrolling", qr_FIXSCRIPTSDURINGSCROLLING,
		"If disabled, the waiting phase of scrolling transitions (1-32 frames) will not"
		" run all scripts, and will not clear draw commands during each frame of scrolling."
		" It also will not run prewaitdraw scripts at the end of scrolling (for when dmap did not change)."},
	{ "GetPixel returns color / 10000", qr_BROKEN_GETPIXEL_VALUE },
	{ "Don't Deallocate Init/SaveLoad Local Arrays", qr_DO_NOT_DEALLOCATE_INIT_AND_SAVELOAD_ARRAYS },
	{ "Broken WarpEx Music", qr_OLD_BROKEN_WARPEX_MUSIC,
		"If enabled, script WarpEx music will use the old behavior for the 'WARP_FLAG_PLAYMUSIC'."
		" This means the features of 'WARP_FLAG_FORCE_RESET_MUSIC' and 'WARP_FLAG_FORCE_CONTINUE_MUSIC'"
		" will not work properly, and instead a mixture of the two settings will occur (Always resetting if it's off,"
		" and always continuing if it's on)."
		"\nIf disabled, the reset and continue flags will work separately, not using the old behavior." },
	{ "Old 'Hero->Warp' Return Square", qr_OLD_HERO_WARP_RETSQUARE,
		"If checked, 'Hero->Warp()' uses the return square of the current screen's Sidewarp A."
		" Otherwise, it uses return square A." },
};

extern GUI::ListData compileSettingList;

static GUI::ListData combinedZSRulesList;
static bool inited_combined_zsrules = false;
GUI::ListData const& combinedZSRList()
{
	if(!inited_combined_zsrules)
	{
		scriptRulesList.tag(rules_zs_script);
		instructionRulesList.tag(rules_zs_instruction);
		objectRulesList.tag(rules_zs_object);
		drawingRulesList.tag(rules_zs_drawing);
		bugfixRulesList.tag(rules_zs_bugfix);
		compileSettingList.tag(rules_compiler_setting);
		
		combinedZSRulesList = scriptRulesList + instructionRulesList
			+ objectRulesList + drawingRulesList + bugfixRulesList;
		combinedZSRulesList.alphabetize();
		combinedZSRulesList += compileSettingList;
		inited_combined_zsrules = true;
	}
	return combinedZSRulesList;
}

//}
int32_t onStrFix(); //zquest.cpp
void popup_bugfix_dlg(const char* cfg); //zq_class.cpp
bool hasCompatRulesEnabled()
{
	for(size_t q = 0; q < compatRulesList.size(); ++q)
	{
		auto rule = compatRulesList.getValue(q);
		if(get_qr(rule))
			return true;
	}
	return false;
}
void applyRuleTemplate(int32_t ruleTemplate, byte* qrptr)
{
	bool inv = false;
	switch(ruleTemplate)
	{
		case ruletemplateFixCompat:
		{
			for(size_t q = 0; q < compatRulesList.size(); ++q)
			{
				auto rule = compatRulesList.getValue(q);
				switch(rule)
				{
					case qr_OLD_STRING_EDITOR_MARGINS:
					case qr_STRING_FRAME_OLD_WIDTH_HEIGHT:
						continue; //Don't auto-unset, use 'onStrFix()' instead
				}
				set_qr(rule, 0, qrptr);
			}
			set_qr(qr_CORRECTED_EW_BRANG_ANIM, 1, qrptr); //this should be on
			onStrFix();
			break;
		}
		case ruletemplateFixZSCompat:
		{
			int zsOnRules[] = {
				qr_PARSER_SHORT_CIRCUIT, qr_PARSER_TRUE_INT_SIZE,
				qr_ITEMSCRIPTSKEEPRUNNING, qr_CLEARINITDONSCRIPTCHANGE,
				qr_COMBOSCRIPTS_LAYER_0, qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN,
				qr_FIXDRUNKINPUTS, qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE,
				qr_STEP_IS_FLOAT, qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS,
				qr_SPRITEXY_IS_FLOAT, qr_WEAPONS_EXTRA_FRAME, qr_FIXSCRIPTSDURINGSCROLLING,
				qr_ALWAYS_DEALLOCATE_ARRAYS,
			};
			int zsOffRules[] = {
				qr_PARSER_250DIVISION, qr_PARSER_NO_LOGGING, qr_PARSER_BOOL_TRUE_DECIMAL,
				qr_PARSER_BINARY_32BIT,
				qr_OLD_INIT_SCRIPT_TIMING, qr_NOITEMWAITDRAW, qr_NOFFCWAITDRAW,
				qr_OLDEWPNPARENT, qr_OLDCREATEBITMAP_ARGS, qr_DISALLOW_SETTING_RAFTING,
				qr_250WRITEEDEFSCRIPT, qr_SETENEMYWEAPONSPRITESONWPNCHANGE, qr_SETENEMYWEAPONSOUNDSONWPNCHANGE,
				qr_BROKENCHARINTDRAWING, qr_NO_OVERWRITING_HOPPING,
				qr_OLD_PRINTF_ARGS, qr_COMBODATA_INITD_MULT_TENK,
				qr_OLDQUESTMISC, qr_DO_NOT_DEALLOCATE_INIT_AND_SAVELOAD_ARRAYS,
				qr_BROKEN_GETPIXEL_VALUE, qr_ZS_NO_NEG_ARRAY, qr_SCRIPT_CONTHP_IS_HEARTS,
				qr_OLD_BROKEN_WARPEX_MUSIC, qr_OLD_HERO_WARP_RETSQUARE, qr_SCRIPTS_6_BIT_COLOR
			};
			for(int qr : zsOnRules)
				set_qr(qr, 1, qrptr);
			for(int qr : zsOffRules)
				set_qr(qr, 0, qrptr);
			break;
		}
		case ruletemplateOldSubscreen:
			inv = true;
			[[fallthrough]];
		case ruletemplateNewSubscreen:
		{
			int subscr_rules_off[] = {
				qr_OLD_SUBSCR, qr_ITM_0_INVIS_ON_BTNS,
				qr_SUBSCR_OLD_SELECTOR, qr_SUBSCR_BACKWARDS_ID_ORDER,
			};
			int subscr_rules_on[] = {
				qr_SUBSCR_PRESS_TO_EQUIP, qr_FREEFORM_SUBSCREEN_CURSOR,
				qr_NO_BUTTON_VERIFY,
			};
			//'inv' inverts the selection, to reverse to the old subscreen setup.
			for(int qr : subscr_rules_off)
				set_qr(qr,inv?1:0, qrptr);
			for(int qr : subscr_rules_on)
				set_qr(qr,inv?0:1, qrptr);
			break;
		}
		default: return;
	}
	if(qrptr == quest_rules || !qrptr)
		saved = false;
}

void QRDialog::reloadQRs()
{
	memcpy(local_qrs, realqrs, QR_SZ);
	unpack_qrs();
}
QRDialog::QRDialog(byte const* qrs, size_t qrs_per_tab, std::function<void(byte*)> setQRs):
	searchmode(false), setQRs(setQRs), realqrs(qrs), qrs_per_tab(qrs_per_tab)
{
	reloadQRs();
}

static string searchstring;
static size_t scroll_pos1;
static bool info_search = false, zs_search = true;
std::shared_ptr<GUI::Widget> QRDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	if(searchmode)
	{
		string lower_search = searchstring;
		lowerstr(lower_search);
		
		GUI::ListData tosearch = combinedQRList();
		if(zs_search)
			tosearch += combinedZSRList();
		
		window = Window(
			title = "Search Quest Rules",
			onClose = message::CANCEL,
			Column(
				QRPanel(
					ptr = &scroll_pos1,
					padding = 3_px,
					onToggle = message::TOGGLE_QR,
					initializer = local_qrs,
					count = 0, //scrollpane
					showtags = true,
					scrollWidth = 675_px,
					scrollHeight = 500_px,
					data = tosearch.filter(
						[&](GUI::ListItem& itm)
						{
							std::string tx = itm.text;
							lowerstr(tx);
							if(tx.find(lower_search) != std::string::npos)
								return true;
							if(info_search)
							{
								std::string inf = itm.info;
								lowerstr(inf);
								if(inf.find(lower_search) != std::string::npos)
									return true;
							}
							return false;
						})
				),
				Row(padding = 0_px,
					Button(
						text = "C", height = 24_px,
						rightPadding = 0_px,
						onClick = message::RERUN,
						onPressFunc = [&]()
						{
							searchstring.clear();
						}),
					TextField(
						focused = true, leftPadding = 0_px,
						maxwidth = 15_em, maxLength = 255,
						text = searchstring,
						onValueChanged = message::RERUN,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
						{
							searchstring = str;
						}
					),
					Column(padding = 0_px,
						Checkbox(
							bottomPadding = 0_px,
							hAlign = 0.0,
							text = "Search Help Text",
							checked = info_search,
							onToggle = message::RERUN,
							onToggleFunc = [&](bool state)
							{
								info_search = state;
							}
						),
						Checkbox(
							topPadding = 0_px,
							hAlign = 0.0,
							text = "Search Script Rules",
							checked = zs_search,
							onToggle = message::RERUN,
							onToggleFunc = [&](bool state)
							{
								zs_search = state;
							}
						)
					),
					Button(
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
	}
	else
	{
		window = Window(
			title = "Quest Options",
			onClose = message::CANCEL,
			Column(
				TabPanel(
					maxwidth = 800_px,
					TabRef(
						name = "Options",
						Column(
							Rows<2>(
								hPadding = 0.5_em,
								//
								Button(
									text = "&Header",
									fitParent = true,
									onClick = message::HEADER
								),
								Button(
									text = "&Pick Ruleset",
									fitParent = true,
									onClick = message::RULESET
								),
								//
								Button(
									text = "&Cheats",
									fitParent = true,
									onClick = message::CHEATS
								),
								Button(
									text = " Pick Rule &Templates ",
									fitParent = true,
									onClick = message::RULETMP
								),
								//
								Button(
									text = "Search QRs",
									fitParent = true,
									onClick = message::SEARCH
								),
								DummyWidget(),
								//
								Button(
									text = "Copy QR String",
									fitParent = true,
									onClick = message::QRSTR_CPY),
								Button(
									text = "Load QR String",
									fitParent = true,
									onClick = message::QRSTR_LOAD)
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
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = animRulesList
						)
					),
					TabRef(
						name = "Combo",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = comboRulesList
						)
					),
					TabRef(
						name = "Compat",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = compatRulesList
						)
					),
					TabRef(
						name = "Enemy",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = enemiesRulesList
						)
					),
					TabRef(
						name = "Item",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = itemRulesList
						)
					),
					TabRef(
						name = "Misc",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = miscRulesList
						)
					),
					TabRef(
						name = "NESFix",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = nesfixesRulesList
						)
					),
					TabRef(
						name = "Hero",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = playerRulesList
						)
					),
					TabRef(
						name = "Subscreen",
						QRPanel(
							padding = 3_px,
							onToggle = message::TOGGLE_QR,
							initializer = local_qrs,
							count = qrs_per_tab,
							data = subscrRulesList
						)
					),
					TabRef(
						name = "Weapon",
						QRPanel(
							padding = 3_px,
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
						minwidth = 90_px,
						onClick = message::OK),
					Button(
						text = "Cancel",
						minwidth = 90_px,
						onClick = message::CANCEL)
				)
			)
		);
	}
	return window;
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
			reloadQRs();
			rerun_dlg = true;
			return true;
		case message::RULETMP:
			call_ruletemplate_dlg();
			reloadQRs();
			rerun_dlg = true;
			return true;
		case message::CHEATS:
			call_cheats_dlg();
			return false;
		case message::QRSTR_CPY:
			al_set_clipboard_text(all_get_display(), get_qr_hexstr(local_qrs, true).c_str());
			InfoDialog("Copied", "QR String copied to clipboard!").show();
			return false;
		case message::QRSTR_LOAD:
			if(load_qr_hexstr_clipboard())
			{
				popup_bugfix_dlg("dsa_compatrule2");
				reloadQRs();
				rerun_dlg = true;
				return true;
			}
			InfoDialog("Error", "No QR String could be loaded from the clipboard").show();
			return false;
		case message::RERUN:
			while(gui_mouse_b()) rest(1); //wait for mouseup
			rerun_dlg = true;
			return true;
		case message::SEARCH:
		{
			bool do_rerun = false;
			auto dlg = QRDialog(local_qrs, qrs_per_tab, [&](byte* qrs)
				{
					memcpy(quest_rules, qrs, QR_SZ);
					reloadQRs();
					do_rerun = true;
				});
			dlg.searchmode = true;
			dlg.show();
			rerun_dlg = do_rerun;
			return do_rerun;
		}
		//Closing buttons
		case message::OK:
		{
			setQRs(local_qrs);
			if(!searchmode)
			{
				word new_map_count = mapCountTF->getVal();
				if(new_map_count < map_count)
				{
					AlertDialog(
						"WARNING! Map Deletion",
						"This action will delete " + std::to_string(map_count-new_map_count)
						+ " maps from the end of your map list!",
						[&new_map_count](bool ret,bool)
						{
							if(ret)
							{
								if(mapcount_will_affect_layers(new_map_count))
								{
									AlertDialog(
										"WARNING! Layer Deletion!",
										"Some of the maps being deleted are used as layermaps for screens that will remain!"
										" If you continue, these screens will have their layermap set to 0!",
										[&new_map_count](bool ret,bool)
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
		}
		[[fallthrough]];
		case message::CANCEL:
			return true;
	}
	return false;
}

