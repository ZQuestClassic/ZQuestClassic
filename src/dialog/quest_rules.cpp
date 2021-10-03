#include "quest_rules.h"
#include "pickruleset.h"
#include <gui/builder.h>
#include "gui/size.h"
#include "../jwin.h"
#include "../zquest.h"
#include "../zsys.h"

using GUI::Size;
using GUI::operator ""_em;
using GUI::operator ""_px;
using GUI::operator ""_lpx;
using GUI::operator ""_spx;

static const GUI::ListData animRulesList
{
	{ "BS-Zelda Animation", qr_BSZELDA },
	{ "Circle Opening/Closing Wipes", qr_COOLSCROLL },
	{ "Oval Opening/Closing Wipes", qr_OVALWIPE },
	{ "Triangle Opening/Closing Wipes", qr_TRIANGLEWIPE },
	{ "Super Mario All-Stars Opening/Closing Wipes", qr_SMASWIPE },
	{ "Fade to/from Black Opening/Closing Wipes", qr_FADEBLACKWIPE },
	{ "No Screen Scrolling", qr_NOSCROLL },
	{ "Fast Dungeon Scrolling", qr_FASTDNGN },
	{ "Very Fast Scrolling", qr_VERYFASTSCROLLING },
	{ "Smooth Vertical Scrolling", qr_SMOOTHVERTICALSCROLLING },
	{ "FFCs Are Visible While The Screen Is Scrolling", qr_FFCSCROLL },
	{ "Interpolated Fading", qr_FADE },
	{ "Fade CSet 5", qr_FADECS5 },
	{ "Bomb Explosions Don't Flash Palette", qr_NOBOMBPALFLASH },
	{ "Layers 1 and 2 Are Drawn Under Caves", qr_LAYER12UNDERCAVE }
};

static const GUI::ListData comboRulesList
{
	{ "Link Drowns in Walkable Water", qr_DROWN },
	{ "Smart Screen Scrolling", qr_SMARTSCREENSCROLL },
	{ "Can't Push Blocks Onto Unwalkable Combos", qr_SOLIDBLK },
	{ "Push Blocks Don't Move When Bumped", qr_HESITANTPUSHBLOCKS },
	{ "Burn Flags Are Triggered Instantly", qr_INSTABURNFLAGS },
	{ "Magic Mirror/Prism Combos Reflect Enemy and Scripted Sword Beams", qr_SWORDMIRROR },
	{ "Magic Mirrors Reflect Scripted Whirlwinds", qr_WHIRLWINDMIRROR },
	{ "Combo Cycling On Layers", qr_CMBCYCLELAYERS },
	{ "Full Priority Damage Combos", qr_DMGCOMBOPRI },
	{ "Warps Ignore Arrival X/Y Position When Setting Continue Screen", qr_WARPSIGNOREARRIVALPOINT },
	{ "Use Warp Return Points Only", qr_NOARRIVALPOINT },
	{ "Scrolling Warps Don't Set The Continue Point", qr_NOSCROLLCONTINUE },
	{ "Use Old-Style Warp Detection (NES Movement Only)", qr_OLDSTYLEWARP },
	{ "Damage Combos Work On Layers 1 And 2", qr_DMGCOMBOLAYERFIX },
	{ "Hookshot Grab Combos Work On Layers 1 And 2", qr_HOOKSHOTLAYERFIX },
	{ "Broken Mirror and Weapon Interaction", qr_OLDMIRRORCOMBOS },
	{ "Always Face Up on Sideview Ladders", qr_SIDEVIEWLADDER_FACEUP },
	{ "Press 'Down' to Fall Through Sideview Platforms", qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS },
	{ "Press 'Down+Jump' to Fall Through Sideview Platforms", qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS },
	{ "Falling Through Sideview Platforms Respects 'Drunk' Inputs", qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK },
	{ "Pressing Down Will Not Grab Sideview Ladders", qr_DOWN_DOESNT_GRAB_LADDERS },
	{ "Custom Combos Work on Layers 1 and 2", qr_CUSTOMCOMBOSLAYERS1AND2 },
	{ "Slash Combos Work on Layers 1 and 2", qr_BUSHESONLAYERS1AND2 },
	{ "New Combo Animation", qr_NEW_COMBO_ANIMATION },
	{ "New Water Collision", qr_SMARTER_WATER },
	{ "No Water Hopping", qr_NO_HOPPING },
	{ "Can't Swim in Solid Water", qr_NO_SOLID_SWIM },
	{ "Water works on Layer 1", qr_WATER_ON_LAYER_1 },
	{ "Water works on Layer 2", qr_WATER_ON_LAYER_2 },
	{ "New Shallow Water Detection", qr_SHALLOW_SENSITIVE },
	{ "Fixed Smart Scrolling", qr_SMARTER_SMART_SCROLL },
	{ "Block Triggers Are Perm For Non-Heavy Blocks", qr_NONHEAVY_BLOCKTRIGGER_PERM },
	{ "Overhead Combos work on Layers 1 and 2", qr_OVERHEAD_COMBOS_L1_L2 },
	{ "Auto Combos work on Layer 1", qr_AUTOCOMBO_LAYER_1 },
	{ "Auto Combos work on Layer 2", qr_AUTOCOMBO_LAYER_2 }
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
	{ "Weapons Can Go Out of Bounds (Offscreen)", qr_WEAPONSMOVEOFFSCREEN },
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
	{ "Hardcoded Shadow/Spawn/Death anim frames", qr_HARDCODED_ENEMY_ANIMS }
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
	{ "Scripted Fire LWeapons Don't Hurt Link", qr_FIREPROOFLINK },
	{ "Scripted Bomb LWeapons Hurt Link", qr_OUCHBOMBS },
	{ "Scripted Melee Weapons Can't Pick Up Items", qr_NOITEMMELEE },
	{ "'Hearts Required' Affects Non-Special Items", qr_HEARTSREQUIREDFIX },
	{ "Big Triforce Pieces", qr_4TRI },
	{ "3 or 6 Triforce Total", qr_3TRI },
	{ "Triforce Doesn't Remove Sword Jinxes", qr_NONBUBBLETRIFORCE },
	{ "Fairy Combos Remove Item Jinxes", qr_ITEMBUBBLE },
	{ "Broken Magic Book Costs", qr_BROKENBOOKCOST },
	{ "Reroll Useless Drops", qr_SMARTDROPS },
	{ "Items Ignore Sideview Platforms", qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS },
	{ "Items Held Above Link's Head Continue To Animate", qr_HOLDITEMANIMATION }
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
	{ "Healing Fairy Heart Circle Is Centered Around Link", qr_HEARTRINGFIX },
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
	{ "Triforce in Cellar Warps Link Out", qr_SIDEVIEWTRIFORCECELLAR },
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
	{ "Item Cellars/Passageways/Caves use own music", qr_SCREEN80_OWN_MUSIC }
};

static const GUI::ListData nesfixesRulesList
{
	{ "Freeform Dungeons", qr_FREEFORM },
	{ "Can Safely Trigger Armos/Grave From The South", qr_SAFEENEMYFADE },
	{ "Can Use Items/Weapons on Edge of Screen", qr_ITEMSONEDGES },
	{ "Fix Link's Position in Dungeons", qr_LINKDUNGEONPOSFIX },
	{ "Raft/Ladder Sprite Direction Fix", qr_RLFIX },
	{ "No Palette 3 CSet 6 Fix", qr_NOLEVEL3FIX },
	{ "Link Holds Special Bombs Over His Head", qr_BOMBHOLDFIX },
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
	{ "Invincible Link Isn't Hurt By Own Fire Weapons", qr_FIREPROOFLINK2 },
	{ "No Position Offset Of Screen Items", qr_NOITEMOFFSET },
	{ "Allow Ladder Anywhere", qr_LADDERANYWHERE },
	{ "Actually fixed Bomb/Darknut interaction", qr_TRUEFIXEDBOMBSHIELD }
};

static const GUI::ListData playerRulesList
{
	{ "Diagonal Movement", qr_LTTPWALK },
	{ "Large Hitbox", qr_LTTPCOLLISION },
	{ "New Hero Movement", qr_NEW_HERO_MOVEMENT },
	{ "Disable 4-Way Movement's Gridlock", qr_DISABLE_4WAY_GRIDLOCK },
	{ "Invincible Link Flickers", qr_LINKFLICKER },
	{ "Expanded Link Tile Modifiers", qr_EXPANDEDLTM }
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
	{ "Bombs pierce enemy shields", qr_BOMBSPIERCESHIELD }
};

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
		title = "Quest Rules",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			TabPanel(
				maxwidth = sized(308_px, 800_px),
				TabRef(
					name = "Options",
					Rows<2>(
						hPadding = 0.5_em,
						spacing = 2_em,
						Button(
							disabled = true,
							text = "&Header",
							onClick = message::HEADER
						),
						Button(
							text = "&Pick Ruleset",
							onClick = message::RULESET
						),
						Button(
							disabled = true,
							text = "&Map Count",
							onClick = message::MAPCOUNT
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

bool QRDialog::handleMessage(message msg, GUI::MessageArg messageArg)
{
	switch(msg)
	{
		case message::TOGGLE_QR:
			toggle_bit(local_qrs, messageArg);
			return false;
		case message::HEADER:
			
			return false;
		case message::RULESET:
			call_ruleset_dlg();
			return false;
		case message::MAPCOUNT:
			
			return false;
		
		//Closing buttons
		case message::OK:
			setQRs(local_qrs);
			[[fallthrough]];
		case message::CANCEL:
		default:
			return true;
	}
}
