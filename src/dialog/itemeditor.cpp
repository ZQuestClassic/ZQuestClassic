#include "itemeditor.h"
#include "info.h"
#include "alert.h"
#include "../zsys.h"
#include <gui/builder.h>

void reset_itembuf(itemdata *item, int id);
extern zquestheader header;
extern bool saved;
extern char *item_string[];
extern itemdata *itemsbuf;
extern zcmodule moduledata;
static bool _reset_default;
static itemdata reset_ref;
static std::string reset_name;
void call_item_editor(int index)
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
ItemNameInfo inameinf[]=
{
	//itemclass                       power                                     misc1                                       misc2                                           misc3                                   misc4                                   misc5                              misc6                              misc7                              misc8                              misc9                                        misc10                                  flag1                                   flag2                                   flag3                                                                   flag4                                   flag5                                   wpn1 (Sprites[0])                            wpn2 (Sprites[1])                               wpn3 (Sprites[2])                               wpn4 (Sprites[3])                               wpn5 (Sprites[4])                               wpn6 (Sprites[5])                               wpn7 (Sprites[6])                               wpn8 (Sprites[7])                               wpn9 (Sprites[8])                               wpn10 (Sprites[9])                              action sound                               flag6,                                          flag7,                                                                            flag8,                                          flag9,                                                      flag10,                                         flag11.                                         flag12,                                         flag13,                                         flag14,                                         flag15,                                         flag16               },
	{ itype_fairy,                    NULL,                                     "HP Regained:",                             "MP Regained:",                                 "Step Speed:",                          NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "HP. R. Is Percent",                    "MP. R. Is Percent",                    NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Item Drop Sound:",                        NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_triforcepiece,            NULL,                                     "Cutscene MIDI:",                           "Cutscene Type (0-1):",                         "Second Collect Sound:",                "Custom Cutscene Duration",             "Custom Refill Frame",             NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Side Warp Out",                        NULL,                                   "Removes Sword Jinxes",                                                 "Removes Item Jinxes",                  NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           "Don't Dismiss Messages",                                   "Cutscene Interrupts Action Script",            "Don't Affect Music",                           "No Cutscene",                                  "Run Action Script on Collection",              "Play Second Sound Effect",                     "Don't Play MIDI",                              NULL                 },
	{ itype_shield,                   NULL,                                     "Block Flags:",                             "Reflect Flags:",                               NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "De/Reflection Sound:",                    NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_agony,                    "Sensitivity:",                           "Vibration Frequency",                      NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_wealthmedal,              NULL,                                     "Discount Amount:",                         NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "D. A. Is Percent",                     NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_cbyrna,                   "Beam Damage:",                           "Beam Speed Divisor:",                      "Orbit Radius:",                                "Number Of Beams:",                     "Melee Damage:",                        NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        "Slash Sound:",                         "Penetrates Enemies",                   "Invincible Player",                    "No Melee Attack",                                                      "Can Slash",                            NULL,                                   "Stab Sprite:",                              "Slash Sprite:",                                "Beam Sprite:",                                 "Sparkle Sprite:",                              "Damaging Sparkle Sprite:",                     NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Orbit Sound:",                            "Melee Weapons Use Magic Cost",                 "Doesn't Pick Up Dropped Items",                                                  "Flip Right-Facing Slash",                      NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_whispring,                "Jinx Divisor:",                          "Jinx Type:",                               NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Perm. Jinx Are Temp.",                 NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_ladder,                   "Four-Way:",                              NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Can Ladder Over Pitfalls",             NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_magickey,                 "Dungeon Level:",                         NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Lesser D. Levels Also",                NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_wallet,                   "Infinite Supply:",                       "Increase Amount:",                         "Delay Duration:",                              NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_quiver,                   "Infinite Supply:",                       "Increase Amount:",                         "Delay Duration:",                              NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_bombbag,                  "Infinite Supply:",                       "Increase Amount:",                         "Delay Duration:",                              NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Super Bombs Also",                     NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_quakescroll2,             "Damage Multiplier:",                     "Stun Duration:",                           "Stun Radius:",                                 NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Quake Sound:",                            NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_quakescroll,              "Damage Multiplier:",                     "Stun Duration:",                           "Stun Radius:",                                 NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Quake Sound:",                            NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_perilscroll,              NULL,                                     "Maximum Hearts:",                          NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_spinscroll2,              "Damage Multiplier:",                     "Number of Spins:",                         NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Spinning Sound:",                         NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_spinscroll,               "Damage Multiplier:",                     "Number of Spins:",                         NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Spinning Sound:",                         NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_clock,                    NULL,                                     "Duration (0 = Infinite):",                 NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_magicring,                "Infinite Magic:",                        "Increase Amount:",                         "Delay Duration:",                              NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_heartring,                NULL,                                     "Increase Amount:",                         "Delay Duration:",                              NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	//itemclass                       power                                     misc1                                       misc2                                           misc3                                   misc4                                   misc5                              misc6                              misc7                              misc8                              misc9                                        misc10                                  flag1                                   flag2                                   flag3                                                                   flag4                                   flag5                                   wpn1                                         wpn2                                            wpn3                                            wpn4                                            wpn5                                            wpn6                                            wpn7                                            wpn8                                            wpn9                                            wpn10                                           action sound                               flag6,                                          flag7,                                                                            flag8,                                          flag9,                                                      flag10,                                         flag11.                                         flag12,                                         flag13,                                         flag14,                                         flag15,                                         flag16               },
	{ itype_chargering,               NULL,                                     "Charging Duration:",                       "Magic C. Duration:",                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_hoverboots,               NULL,                                     "Hover Duration:",                          NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Timer only resets on landing",         NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   "Halo Sprite:",                              NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Hovering Sound:",                         NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_rocs,                     "Jump Power:",                            "Extra Jumps:",                             NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Jump is Power/100",                    NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Jumping Sound:",                          NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_sbomb,                    "Damage:",                                "Fuse Duration (0 = Remote):",              "Max. On Screen:",                              "Damage to Player:",                    NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   "Explosion Hurts Player",               NULL,                                                                   NULL,                                   NULL,                                   "Bomb Sprite:",                              "Explosion Sprite:",                            NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Explosion Sound:",                        NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_bomb,                     "Damage:",                                "Fuse Duration (0 = Remote):",              "Max. On Screen:",                              "Damage to Player:",                    NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Use 1.92 Timing",                      "Explosion Hurts Player",               NULL,                                                                   NULL,                                   NULL,                                   "Bomb Sprite:",                              "Explosion Sprite:",                            NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Explosion Sound:",                        NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_nayruslove,               NULL,                                     "Duration:",                                NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Rocket Flickers",                      "Translucent Rocket",                   "Translucent Shield",                                                   "Shield Flickers",                      NULL,                                   "Left Rocket Sprite:",                       "L. Rocket Return Sprite:",                     "L. Rocket Sparkle Sprite:",                    "L. Return Sparkle Sprite:",                    "Shield Sprite (2x2, Over):",                   "Right Rocket Sprite:",                         "R. Rocket Return Sprite:",                     "R. Rocket Sparkle Sprite:",                    "R. Return Sparkle Sprite:",                    "Shield Sprite (2x2, Under):",                  "Shield Sound:",                           NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_faroreswind,              NULL,                                     "Warp Animation (0-2):",                    NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Wind Sound:",                             NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_dinsfire,                 "Damage:",                                "Number of Flames:",                        "Circle Width:",                                NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   "Don't Provide Light",                  "Falls in Sideview",                                                    NULL,                                   "Temporary Light",                      "Rocket Up Sprite:",                         "Rocket Down Sprite:",                          "R. Up Sparkle Sprite:",                        "R. Down Sparkle Sprite:",                      "Flame Sprite:",                                NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Ring Sound:",                             NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_hammer,                   "Damage:",                                NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   "No Melee Attack",                                                      NULL,                                   NULL,                                   "Hammer Sprite:",                            "Smack Sprite:",                                NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Pound Sound:",                            NULL,                                           "Doesn't Pick Up Dropped Items",                                                  NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_lens,                     NULL,                                     "Lens Width:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Show Hints",                           "Hide Secret Combos",                   "No X-Ray for Items",                                                   "Show Raft Paths",                      "Show Invis. Enemies",                  NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Activation Sound:",                       NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_hookshot,                 "Damage:",                                "Chain Length:",                            "Chain Links:",                                 "Block Flags:",                         "Reflect Flags:",                       NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Handle Damage",                     "Allow Diagonal",                       NULL,                                                                   "Pick Up Anything",                     "Drags Items",                          "Tip Sprite:",                               "Chain Sprite (H):",                            "Chain Sprite (V):",                            "Handle Sprite:",                               "Diagonal Tip Sprite:",                         "Diagonal Handle Sprite:",                      "Diagonal Chain Sprite:",                       NULL,                                           NULL,                                           NULL,                                           "Firing Sound:",                           "Reflects Enemy Projectiles",                   "Picks Up Keys",                                                                  NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_boots,                    "Damage Combo Level:",                    NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Not Solid Combos",                     "Iron",                                 NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_bracelet,                 "Push Combo Level:",                      NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Once Per Screen",                      NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_book,                     "M. Damage:",                             "W. Damage:",                               "Wand Sound",                                   "Special Step",                         "Fire Damage",                          NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Fire Magic",                           "Override Wand Damage",                 "Fire Doesn't Hurt Player",                                             "Override Wand SFX",                    "Temporary Light",                      "Magic Sprite:",                             "Projectile Sprite:",                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Firing Sound:",                           "Replace Wand Weapon",                          NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_ring,                     "Damage Divisor:",                        "Player Sprite Pal:",                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Affects Damage Combos",                "Divisor is Percentage Multiplier",     NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_wand,                     "Damage:",                                "M. Damage",                                "W. Type:",                                     "W. Speed:",                            "W. Range:",                            "Move Effect:",                    "Mvt Arg1:",                       "Mvt Arg2:",                       "No. of Clones:",                  "Clone Pattern:",                            "Slash Sound:",                         "Allow Magic w/o Book",                 "Wand Moves",                           "No Melee Attack",                                                      "Can Slash",                            "Quick",                                "Stab Sprite:",                              "Slash Sprite:",                                "Projectile Sprite:",                           "Projectile Misc:",                             NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Firing Sound:",                           "Melee Weapons Use Magic Cost",                 "Doesn't Pick Up Dropped Items",                                                  "Flip Right-Facing Slash",                      "Light Torches",                                            NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_bait,                     NULL,                                     "Duration:",                                NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   "Bait Sprite:",                              NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Baiting Sound:",                          NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_potion,                   NULL,                                     "HP Regained:",                             "MP Regained:",                                 NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "HP R. Is Percent",                     "MP R. Is Percent",                     "Removes Sword Jinxes",                                                 "Removes Item Jinxes",                  NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_whistle,                  NULL,                                     "Whirlwind Direction:",                     "Warp Ring:",                                   NULL,                                   NULL,                                   "Weapon Damage",                   NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "One W.Wind Per Scr.",                  "Has Damage",                           "Whirlwinds Reflect off Prism/Mirror Combos",                           NULL,                                   NULL,                                   "Whirlwind Sprite:",                         NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Music Sound:",                            NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_candle,                   "Damage:",                                "Damage to Player:",                        NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Once Per Screen",                      "Don't Provide Light",                  "Fire Doesn't Hurt Player",                                             "Can Slash",                            "Temporary Light",                      "Stab Sprite:",                              "Slash Sprite:",                                "Flame Sprite:",                                NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Firing Sound:",                           NULL,                                           NULL,                                                                             "Flip Right-Facing Slash",                      NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_arrow,                    "Damage:",                                "Duration (0 = Infinite):",                 NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Penetrate Enemies",                    "Allow Item Pickup",                    NULL,                                                                   "Pick Up Anything",                     NULL,                                   "Arrow Sprite:",                             "Sparkle Sprite:",                              "Damaging Sparkle Sprite:",                     NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Firing Sound:",                           NULL,                                           "Picks Up Keys",                                                                  NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_brang,                    "Damage:",                                "Range (0 = Infinite):",                    NULL,                                           "Block Flags:",                         "Reflect Flags:",                       NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "Corrected Animation",                  "Directional Sprites",                  "Do Not Return",                                                        "Pick Up Anything",                     "Drags Items",                          "Boomerang Sprite:",                         "Sparkle Sprite:",                              "Damaging Sparkle Sprite:",                     NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Spinning Sound:",                         "Reflects Enemy Projectiles",                   "Picks Up Keys",                                                                  "Triggers 'Fire(Any)'",                         NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_sword,                    "Damage:",                                "Beam Hearts:",                             "Beam Damage:",                                 NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "B.H. Is Percent",                      "B.D. Is Percent",                      "B. Penetrates Enemies",                                                "Can Slash",                            "Quick",                                "Stab Sprite:",                              "Slash Sprite:",                                "Beam Sprite:",                                 NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Slash/Stab Sound",                        "Melee Weapons Use Magic Cost",                 "Doesn't Pick Up Dropped Items",                                                  "Flip Right-Facing Slash",                      "Sword Beams Reflect off Prism/Mirror Combos",              "Walk slowly while charging",                   NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_whimsicalring,            "Damage Bonus:",                          "Chance (1 in n):",                         NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           "Whimsy Sound:",                           NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_perilring,                "Damage Divisor:",                        "Maximum Hearts:",                          NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   "Divisor is Percentage Multiplier",     NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_stompboots,               "Damage:",                                NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_bow,                      "Arrow Speed:",                           NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script1,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script2,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script3,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script4,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script5,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script6,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script7,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script8,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script9,                  "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_script10,                 "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom1,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom2,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom3,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom4,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom5,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom6,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom7,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom8,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom9,                  NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_custom10,                 NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_icerod,                   "W. Power:",                              "Step Speed:",                              NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Gfx Flip",                          NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_flippers,                 NULL,                                     "Dive Length:",                             "Dive Cooldown:",                               NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   "No Diving",                            "Cancellable Diving",                   "Can Swim in Lava",                                                     NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_raft,                     NULL,                                     "Speed Modifier:",                          NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	//itemclass                       power                                     misc1                                       misc2                                           misc3                                   misc4                                   misc5                              misc6                              misc7                              misc8                              misc9                                        misc10                                  flag1                                   flag2                                   flag3                                                                   flag4                                   flag5                                   wpn1                                         wpn2                                            wpn3                                            wpn4                                            wpn5                                            wpn6                                            wpn7                                            wpn8                                            wpn9                                            wpn10                                           action sound                               flag6,                                          flag7,                                                                            flag8,                                          flag9,                                                      flag10,                                         flag11.                                         flag12,                                         flag13,                                         flag14,                                         flag15,                                         flag16               },
	{ itype_atkring,                  NULL,                                     "Bonus Damage",                             "Damage Multiplier",                            NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ itype_lantern,                  NULL,                                     "Shape",                                    "Range",                                        NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 },
	{ -1,                             NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                                       NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 }
};

ItemNameInfo defInfo =
{
	-1, "Power:", "Attributes[0]:", "Attributes[1]:", "Attributes[2]:",
	"Attributes[3]:", "Attributes[4]:","Attributes[5]:","Attributes[6]:",
	"Attributes[7]:","Attributes[8]:","Attributes[9]:", "Flags[0]",
	"Flags[1]", "Flags[2]", "Flags[3]", "Flags[4]", "Sprites[0]:",
	"Sprites[1]:","Sprites[2]:","Sprites[3]:","Sprites[4]:","Sprites[5]:",
	"Sprites[6]:","Sprites[7]:","Sprites[8]:","Sprites[9]:","UseSound",
	"Flags[5]", "Flags[6]", "Flags[7]", "Flags[8]", "Flags[9]", "Flags[10]",
	"Flags[11]","Flags[12]","Flags[13]", "Flags[14]","Constant Script"
};

std::map<int, ItemNameInfo *> *inamemap = NULL;

std::map<int, ItemNameInfo *> *getItemNameMap()
{
	if(inamemap == NULL)
	{
		inamemap = new std::map<int, ItemNameInfo *>();
		
		for(int i=0;; i++)
		{
			ItemNameInfo *inf = &inameinf[i];
			
			if(inf->iclass == -1)
				break;
				
			(*inamemap)[inf->iclass] = inf;
		}
	}
	
	return inamemap;
}

ItemEditorDialog::ItemEditorDialog(itemdata const& ref, char const* str, int index):
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

ItemEditorDialog::ItemEditorDialog(int index):
	ItemEditorDialog(itemsbuf[index], item_string[index], index)
{}

//{ Macros

#define DISABLE_WEAP_DATA true
#define ATTR_WID 6_em
#define ATTR_LAB_WID 12_em
#define SPR_LAB_WID sized(14_em,10_em)
#define ACTION_LAB_WID 6_em
#define ACTION_FIELD_WID 6_em
#define FLAGS_WID 18_em

#define NUM_FIELD(member,_min,_max,wid) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, width = wid, \
	low = _min, high = _max, val = local_itemref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val) \
	{ \
		local_itemref.member = val; \
	})

#define ATTRIB_FIELD(member, index) \
Row(vPadding = 0_px, \
	l_attribs[index] = Label(textAlign = 2, width = ATTR_LAB_WID), \
	TextField(maxLength = 11, \
		type = GUI::TextField::type::INT_DECIMAL, width = ATTR_WID, \
		val = local_itemref.member, \
		onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val) \
		{ \
			local_itemref.member = val; \
		}) \
)

#define FLAG_CHECK(index, bit) \
l_flags[index] = Checkbox( \
	width = FLAGS_WID, \
	checked = (local_itemref.flags & bit), \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_itemref.flags,bit,state); \
	} \
)

#define SPRITE_DROP(ind, mem) \
Rows<2>(vPadding = 0_px, \
	l_spr[ind] = Label(textAlign = 2, width = SPR_LAB_WID, topMargin = 1_px), \
	DropDownList( \
		maxwidth = sized(18_em, 14_em), \
		data = list_sprites, \
		selectedValue = local_itemref.mem, \
		onSelectFunc = [&](int val) \
		{ \
			local_itemref.mem = val; \
		} \
	) \
)

//}

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
							onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int)
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
								InfoDialog(moduledata.item_editor_type_names[local_itemref.family],{
									moduledata.itemclass_help_strings[(local_itemref.family*3)+0],
									moduledata.itemclass_help_strings[(local_itemref.family*3)+1],
									moduledata.itemclass_help_strings[(local_itemref.family*3)+2]
								}).show();
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
					TabRef(name = "Data", TabPanel(
						TabRef(name = "Attrib", Column(
							Columns<6>(padding = 0_px,
								Row(
									Label(width=ATTR_LAB_WID,textAlign=2,text="Level:"),
									NUM_FIELD(fam_type, 1, 255, ATTR_WID)
								),
								ATTRIB_FIELD(misc1,0),
								ATTRIB_FIELD(misc2,1),
								ATTRIB_FIELD(misc3,2),
								ATTRIB_FIELD(misc4,3),
								ATTRIB_FIELD(misc5,4),
								Row(
									l_power = Label(width=ATTR_LAB_WID,textAlign=2),
									NUM_FIELD(power, 0, 255, ATTR_WID)
								),
								ATTRIB_FIELD(misc6,5),
								ATTRIB_FIELD(misc7,6),
								ATTRIB_FIELD(misc8,7),
								ATTRIB_FIELD(misc9,8),
								ATTRIB_FIELD(misc10,9)
							)
						)),
						TabRef(name = "Flags", Row(
							Columns<9>(padding = 0_px,
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & ITEM_EDIBLE),
									text = "Can Be Eaten By Enemies",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_EDIBLE,state);
									}
								),
								FLAG_CHECK(0,ITEM_FLAG1),
								FLAG_CHECK(1,ITEM_FLAG2),
								FLAG_CHECK(2,ITEM_FLAG3),
								FLAG_CHECK(3,ITEM_FLAG4),
								FLAG_CHECK(4,ITEM_FLAG5),
								FLAG_CHECK(5,ITEM_FLAG6),
								FLAG_CHECK(6,ITEM_FLAG7),
								FLAG_CHECK(7,ITEM_FLAG8),
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & ITEM_SIDESWIM_DISABLED),
									text = "Disabled In Sideview Water",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_SIDESWIM_DISABLED,state);
									}
								),
								FLAG_CHECK(8,ITEM_FLAG9),
								FLAG_CHECK(9,ITEM_FLAG10),
								FLAG_CHECK(10,ITEM_FLAG11),
								FLAG_CHECK(11,ITEM_FLAG12),
								FLAG_CHECK(12,ITEM_FLAG13),
								FLAG_CHECK(13,ITEM_FLAG14),
								FLAG_CHECK(14,ITEM_FLAG15),
								FLAG_CHECK(15,ITEM_FLAG16)
							)
						)),
						TabRef(name = "Action", Columns<4>(
							Row(
								Label(text = "Cost:", textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.magic,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.magic = val;
									}
								)
							),
							DropDownList(
								hAlign = 1.0,
								data = list_counters,
								selectedValue = local_itemref.cost_counter,
								onSelectFunc = [&](int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.magiccosttimer = val;
									}
								)
							),
							Row(
								l_sfx = Label(textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.usesound,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onSelectFunc = [&](int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.setmax = val;
									}
								),
								Label(text = "But Not Above:", hAlign = 1.0),
								TextField(
									val = local_itemref.max,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onSelectFunc = [&](int val)
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
										onSelectFunc = [&](int val)
										{
											local_itemref.useweapon = val;
										}
									), _d, _d,
									Label(hAlign = 1.0, text = "Default Defense:"),
									DropDownList(
										fitParent = true,
										data = list_deftypes,
										selectedValue = local_itemref.usedefence,
										onSelectFunc = [&](int val)
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
										onSelectFunc = [&](int val)
										{
											local_itemref.weap_pattern[0] = val;
										}
									), _d, _d,
									Label(hAlign = 1.0, text = "Movement Arg 1:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[1],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[1] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Range:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weaprange,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weaprange = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[2],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[2] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Duration:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weapduration,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weapduration = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 3:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[3],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[3] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 1:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[5],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[5] = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 4:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[4],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[4] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[6],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[6] = val;
										}
									)
								)
							)
						))
					)),
					TabRef(name = "Graphics", TabPanel(
						TabRef(name = "GFX", Row(
							Rows<2>(
								Label(text = "Flash CSet:", hAlign = 1.0),
								TextField(
									val = (local_itemref.csets>>4),
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 16,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.csets &= 0x0F;
										local_itemref.csets |= val<<4;
									}
								),
								Label(text = "Animation Frames:", hAlign = 1.0),
								TextField(
									val = local_itemref.frames,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.frames = val;
										animFrame->setFrames(val);
									}
								),
								Label(text = "Animation Speed:", hAlign = 1.0),
								TextField(
									val = local_itemref.speed,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.speed = val;
										animFrame->setSpeed(val);
									}
								),
								Label(text = "Initial Delay:", hAlign = 1.0),
								TextField(
									val = local_itemref.delay,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.delay = val;
										animFrame->setDelay(val);
									}
								),
								Label(text = "Player Tile Modifier:", hAlign = 1.0),
								TextField(
									val = local_itemref.ltm,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = (0-(NEWMAXTILES-1)), high = (NEWMAXTILES-1),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.ltm = val;
									}
								)
							),
							Column(
								SelTileSwatch(
									tile = local_itemref.tile,
									cset = (local_itemref.csets & 0x0F),
									onSelectFunc = [&](int t, int c)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.initiala[0] = val;
										}
									),
									Label(text = "A2:"),
									TextField(
										val = local_itemref.initiala[1],
										type = GUI::TextField::type::INT_DECIMAL,
										high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
						onClick = message::OK),
					Button(
						text = "Cancel",
						onClick = message::CANCEL),
					Button(
						text = "Default",
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
					TabRef(name = "Basic", Column(
						Rows<2>(padding = 0_px,
							Label(text = "Name:"),
							TextField(
								fitParent = true,
								maxLength = 63,
								text = itemname,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int)
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
									InfoDialog(moduledata.item_editor_type_names[local_itemref.family],{
										moduledata.itemclass_help_strings[(local_itemref.family*3)+0],
										moduledata.itemclass_help_strings[(local_itemref.family*3)+1],
										moduledata.itemclass_help_strings[(local_itemref.family*3)+2]
									}).show();
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
						TabRef(name = "Attrib", Column(
							Columns<6>(padding = 0_px,
								Row(
									Label(width=ATTR_LAB_WID,textAlign=2,text="Level:"),
									NUM_FIELD(fam_type, 1, 255, ATTR_WID)
								),
								ATTRIB_FIELD(misc1,0),
								ATTRIB_FIELD(misc2,1),
								ATTRIB_FIELD(misc3,2),
								ATTRIB_FIELD(misc4,3),
								ATTRIB_FIELD(misc5,4),
								Row(
									l_power = Label(width=ATTR_LAB_WID,textAlign=2),
									NUM_FIELD(power, 0, 255, ATTR_WID)
								),
								ATTRIB_FIELD(misc6,5),
								ATTRIB_FIELD(misc7,6),
								ATTRIB_FIELD(misc8,7),
								ATTRIB_FIELD(misc9,8),
								ATTRIB_FIELD(misc10,9)
							)
						)),
						TabRef(name = "Flags", Row(
							Columns<9>(padding = 0_px,
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & ITEM_EDIBLE),
									text = "Can Be Eaten By Enemies",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_EDIBLE,state);
									}
								),
								FLAG_CHECK(0,ITEM_FLAG1),
								FLAG_CHECK(1,ITEM_FLAG2),
								FLAG_CHECK(2,ITEM_FLAG3),
								FLAG_CHECK(3,ITEM_FLAG4),
								FLAG_CHECK(4,ITEM_FLAG5),
								FLAG_CHECK(5,ITEM_FLAG6),
								FLAG_CHECK(6,ITEM_FLAG7),
								FLAG_CHECK(7,ITEM_FLAG8),
								Checkbox(
									width = FLAGS_WID,
									checked = (local_itemref.flags & ITEM_SIDESWIM_DISABLED),
									text = "Disabled In Sideview Water",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_itemref.flags,ITEM_SIDESWIM_DISABLED,state);
									}
								),
								FLAG_CHECK(8,ITEM_FLAG9),
								FLAG_CHECK(9,ITEM_FLAG10),
								FLAG_CHECK(10,ITEM_FLAG11),
								FLAG_CHECK(11,ITEM_FLAG12),
								FLAG_CHECK(12,ITEM_FLAG13),
								FLAG_CHECK(13,ITEM_FLAG14),
								FLAG_CHECK(14,ITEM_FLAG15),
								FLAG_CHECK(15,ITEM_FLAG16)
							)
						)),
						TabRef(name = "Action", Columns<4>(
							Row(
								Label(text = "Cost:", textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.magic,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.magic = val;
									}
								)
							),
							DropDownList(
								hAlign = 1.0,
								data = list_counters,
								selectedValue = local_itemref.cost_counter,
								onSelectFunc = [&](int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.magiccosttimer = val;
									}
								)
							),
							Row(
								l_sfx = Label(textAlign = 2, width = ACTION_LAB_WID),
								TextField(
									val = local_itemref.usesound,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onSelectFunc = [&](int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.setmax = val;
									}
								),
								Label(text = "But Not Above:", hAlign = 1.0),
								TextField(
									val = local_itemref.max,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onSelectFunc = [&](int val)
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
										onSelectFunc = [&](int val)
										{
											local_itemref.useweapon = val;
										}
									),
									Label(hAlign = 1.0, text = "Default Defense:"),
									DropDownList(
										fitParent = true,
										data = list_deftypes,
										selectedValue = local_itemref.usedefence,
										onSelectFunc = [&](int val)
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
										onSelectFunc = [&](int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[1] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Range:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weaprange,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weaprange = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[2],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[2] = val;
										}
									),
									Label(hAlign = 1.0, text = "Weapon Duration:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weapduration,
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weapduration = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 3:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[3],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[3] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 1:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[5],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[5] = val;
										}
									),
									Label(hAlign = 1.0, text = "Movement Arg 4:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[4],
										type = GUI::TextField::type::INT_DECIMAL,
										hAlign = 0.0, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[4] = val;
										}
									),
									Label(hAlign = 1.0, text = "Other 2:"),
									TextField(disabled = DISABLE_WEAP_DATA,
										val = local_itemref.weap_pattern[6],
										type = GUI::TextField::type::INT_DECIMAL,
										fitParent = true, low = -214748, high = 214748,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.weap_pattern[6] = val;
										}
									)
								)
							)
						))
					)),
					TabRef(name = "Graphics", TabPanel(
						TabRef(name = "GFX", Row(
							Rows<2>(
								Label(text = "Flash CSet:", hAlign = 1.0),
								TextField(
									val = (local_itemref.csets>>4),
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 16,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.csets &= 0x0F;
										local_itemref.csets |= val<<4;
									}
								),
								Label(text = "Animation Frames:", hAlign = 1.0),
								TextField(
									val = local_itemref.frames,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.frames = val;
										animFrame->setFrames(val);
									}
								),
								Label(text = "Animation Speed:", hAlign = 1.0),
								TextField(
									val = local_itemref.speed,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.speed = val;
										animFrame->setSpeed(val);
									}
								),
								Label(text = "Initial Delay:", hAlign = 1.0),
								TextField(
									val = local_itemref.delay,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, high = 255,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.delay = val;
										animFrame->setDelay(val);
									}
								),
								Label(text = "Player Tile Modifier:", hAlign = 1.0),
								TextField(
									val = local_itemref.ltm,
									type = GUI::TextField::type::INT_DECIMAL,
									width = ACTION_FIELD_WID, low = (0-(NEWMAXTILES-1)), high = (NEWMAXTILES-1),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
									{
										local_itemref.ltm = val;
									}
								)
							),
							Column(
								SelTileSwatch(
									tile = local_itemref.tile,
									cset = (local_itemref.csets & 0x0F),
									onSelectFunc = [&](int t, int c)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
										{
											local_itemref.initiala[0] = val;
										}
									),
									Label(text = "A2:"),
									TextField(
										val = local_itemref.initiala[1],
										type = GUI::TextField::type::INT_DECIMAL,
										high = 32,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
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
						onClick = message::OK),
					Button(
						text = "Cancel",
						onClick = message::CANCEL),
					Button(
						text = "Default",
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
	std::map<int, ItemNameInfo *> *nmap = getItemNameMap();
	std::map<int, ItemNameInfo *>::iterator it = nmap->find(local_itemref.family);
	ItemNameInfo *inf = NULL;
	
	if(it != nmap->end())
		inf = it->second;
	
	#define __SET(obj, mem) \
	obj->setText(inf ? (inf->mem ? inf->mem : defInfo.mem) : defInfo.mem);
	
	__SET(l_power, power);
	
	__SET(l_attribs[0], misc1);
	__SET(l_attribs[1], misc2);
	__SET(l_attribs[2], misc3);
	__SET(l_attribs[3], misc4);
	__SET(l_attribs[4], misc5);
	__SET(l_attribs[5], misc6);
	__SET(l_attribs[6], misc7);
	__SET(l_attribs[7], misc8);
	__SET(l_attribs[8], misc9);
	__SET(l_attribs[9], misc10);
	
	__SET(l_flags[0], flag1);
	__SET(l_flags[1], flag2);
	__SET(l_flags[2], flag3);
	__SET(l_flags[3], flag4);
	__SET(l_flags[4], flag5);
	__SET(l_flags[5], flag6);
	__SET(l_flags[6], flag7);
	__SET(l_flags[7], flag8);
	__SET(l_flags[8], flag9);
	__SET(l_flags[9], flag10);
	__SET(l_flags[10], flag11);
	__SET(l_flags[11], flag12);
	__SET(l_flags[12], flag13);
	__SET(l_flags[13], flag14);
	__SET(l_flags[14], flag15);
	__SET(l_flags[15], flag16);
	
	__SET(l_sfx, actionsnd);
	
	__SET(l_spr[0], wpn1);
	__SET(l_spr[1], wpn2);
	__SET(l_spr[2], wpn3);
	__SET(l_spr[3], wpn4);
	__SET(l_spr[4], wpn5);
	__SET(l_spr[5], wpn6);
	__SET(l_spr[6], wpn7);
	__SET(l_spr[7], wpn8);
	__SET(l_spr[8], wpn9);
	__SET(l_spr[9], wpn10);
}

bool ItemEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::ITEMCLASS:
		{
			local_itemref.family = int(msg.argument);
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

