//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------


#include "gui.h"
#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "zsys.h"
#include "zquest.h"

static int rules1_list[] =
{
  // dialog control number
  //rules1 (7-22)
  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, -1
};

static int rules2_list[] =
{
  // dialog control number
  //rules2 (23-38)
  23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, -1
};


static int rules3_list[] =
{
  // dialog control number
  //rules3 (39-54)
  39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, -1
};

static int rules4_list[] =
{
  // dialog control number
  //rules4 (55-70)
  55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, -1
};


static int rules5_list[] =
{
  // dialog control number
  //rules5 (71-86)
  71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, -1
};

static int rules6_list[] =
{
  // dialog control number
  //rules6 (88-103)
  89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, -1
};

static int rules7_list[] =
{
  // dialog control number
  //rules6 (88-103)
  105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, -1
};

static TABPANEL rules_tabs[] =
{
  // (text)
  { "  1  ", D_SELECTED, rules1_list, 0, NULL },
  { "  2  ", 0,          rules2_list, 0, NULL },
  { "  3  ", 0,          rules3_list, 0, NULL },
  { "  4  ", 0,          rules4_list, 0, NULL },
  { "  5  ", 0,          rules5_list, 0, NULL },
  { "  6  ", 0,          rules6_list, 0, NULL },
  { "  7  ", 0,          rules7_list, 0, NULL },
  { NULL,    0,          NULL,        0, NULL }
};

static DIALOG rules_dlg[] =
{
  /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,         0,   0,   300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0,       (void *) "Quest Rules", NULL, NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  // 3
  { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0,       (void *) onHelp, NULL, NULL },
  // 6
  { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) rules_tabs, NULL, (void *)rules_dlg },
  //rules1 (7-22)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Solid Blocks", NULL, NULL },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Temporary No Return Disabled", NULL, NULL },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Always Return", NULL, NULL },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Multi-Directional Traps", NULL, NULL },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "BS-Zelda Animation", NULL, NULL },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Interpolated Fading", NULL, NULL },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fade CSet 5", NULL, NULL },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fast Dungeon Scrolling", NULL, NULL },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Level 3 CSet Fix", NULL, NULL },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Cool Scrolling", NULL, NULL },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Start With 999 Rupies", NULL, NULL },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Big Triforce Pieces", NULL, NULL },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "24 Heart Containers", NULL, NULL },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fast Heart Refill", NULL, NULL },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "View Overworld Map", NULL, NULL },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "3 or 6 Triforce Total", NULL, NULL },

  //rules2 (23-38)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Time on Subscreen", NULL, NULL },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Freeform Dungeons", NULL, NULL },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Must Kill All Bosses", NULL, NULL },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Sprite Flicker", NULL, NULL },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Continue With Full Life", NULL, NULL },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Raft/Ladder Fix", NULL, NULL },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Secret Combo Hints When Using The Lens", NULL, NULL },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fix Link's Position in Dungeons", NULL, NULL },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Items Held Above Link's Head Continue To Animate", NULL, NULL },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Push Blocks Don't Move When Bumped", NULL, NULL },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Hide Enemy-Carried Items", NULL, NULL },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Silver Arrow Leaves Sparkle Trail Behind", NULL, NULL },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Golden Arrow Leaves Sparkle Trail Behind", NULL, NULL },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Magic Boomerang Leaves Sparkle Trail Behind", NULL, NULL },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fire Boomerang Leaves Fire Trail Behind", NULL, NULL },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Don't Flash When Dying", NULL, NULL },

  //rules3 (39-54)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Link Can Keep Old Items When Acquiring Upgraded Ones", NULL, NULL },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Link Isn't Hurt By Own Fire Weapons", NULL, NULL },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bombs Hurt Link", NULL, NULL },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Don't Randomly Drop Clocks", NULL, NULL },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Clocks Last A Limited Time", NULL, NULL },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Darknut Shields Can Be Broken By The Hammer", NULL, NULL },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Broken Darknut Shield Tiles", NULL, NULL },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Mean Placed Traps", NULL, NULL },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Placed Traps Can Pass Through Enemies", NULL, NULL },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Messages Can Be Sped Up With The A Button", NULL, NULL },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Walk-Affected Combos Are Linked Together", NULL, NULL },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Fires in Special Rooms", NULL, NULL },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Healing Fairy Heart Ring Is Centered Around Link", NULL, NULL },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Healing Fairy Heart Ring", NULL, NULL },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Dodongo CSet Fix", NULL, NULL },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Sword/Wand Flip Fix", NULL, NULL },

  //rules4 (55-70)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enable Magic", NULL, NULL },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Wand Consumes Magic", NULL, NULL },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Candle Consumes Magic", NULL, NULL },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Boots Consume Magic", NULL, NULL },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Medicine Doesn't Re-Enable Bubble-Removed Items", NULL, NULL },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fairies Don't Re-Enable Bubble-Removed Items", NULL, NULL },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Triforce Doesn't Re-Enable Bubble-Removed Items", NULL, NULL },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Use New Enemy Tiles", NULL, NULL },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Rope 2 Doesn't Flash", NULL, NULL },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bubbles Don't Flash", NULL, NULL },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Ghini 2 Flickers", NULL, NULL },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Weapon Animation Fix", NULL, NULL },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Ghini 2 Is Transparent", NULL, NULL },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "New Boomerang/Hookshot", NULL, NULL },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Melee Weapons Can't Pick Up Items", NULL, NULL },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Shadows", NULL, NULL },

  //rules5 (71-86)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Translucent Shadows", NULL, NULL },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Quick Sword", NULL, NULL },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Link Holds Special Bombs Over His Head", NULL, NULL },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Expanded Link Tile Modifiers", NULL, NULL },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "2 Blue Potions Can't Be Combined Into 1 Red Potion", NULL, NULL },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Invincible Link Flickers", NULL, NULL },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Shadows Flicker", NULL, NULL },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fliers Can Appear on Walls", NULL, NULL },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bomb Explosions Don't Flash Palette", NULL, NULL },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Transparent Nayru's Love Shield", NULL, NULL },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Nayru's Love Shield Flickers", NULL, NULL },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Transparent Nayru's Love Rocket", NULL, NULL },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Nayru's Love Rocket Flickers", NULL, NULL },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Combo Cycling On Layers", NULL, NULL },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Full Priority Damage Combos", NULL, NULL },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Warps Ignore Arrival X/Y Position", NULL, NULL },
  //  qr_LTTPCOLLISION (87)
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  //  qr_LTTPWALK (88)
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  //rules6 (89-104)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Slow Enemy Animation", NULL, NULL },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "True Arrows", NULL, NULL },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Saving When Continuing", NULL, NULL },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Skip Continue Screen", NULL, NULL },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enable 1/4 Heart Life Bar", NULL, NULL },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Use Warp Return Points Only", NULL, NULL },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Special Room Guys Don't Create A Poof When Appearing", NULL, NULL },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Messages Can Be Bypassed With The B Button", NULL, NULL },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Diving", NULL, NULL },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Layers 1 and 2 Are Drawn Under Caves", NULL, NULL },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Scrolling Warps Don't Set The Continue Point", NULL, NULL },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Smart Screen Scrolling", NULL, NULL },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Rings Affect Damage Combos", NULL, NULL },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Allow 10 Rupee Drops", NULL, NULL },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Trap Position Fix", NULL, NULL },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Dark Rooms Stay Lit Only While Fire Is On Screen", NULL, NULL },

  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Red Potion Disappears When Used", NULL, NULL },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Use Old-Style Warp Detection", NULL, NULL },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Invisible Border on Non-Dungeon Dmaps", NULL, NULL },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "1.90-Style Tribbles", NULL, NULL },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "All Shields Reflect Rocks", NULL, NULL },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Items Disappear During Hold-Up", NULL, NULL },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Jump/Fly Through Z-Axis", NULL, NULL },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Can Safely Trigger Armos/Grave From The South", NULL, NULL },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "More Sound Effects", NULL, NULL },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Boomerang/Hookshot Grabs All Items", NULL, NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },

  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },

  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL, NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int onRules()
{
  rules_dlg[0].dp2=lfont;
  for(int i=0; i<qr_MAX; i++)
  {
    rules_dlg[i+7].flags = get_bit(quest_rules,i) ? D_SELECTED : 0;
  }

  int ret = zc_popup_dialog(rules_dlg,4);

  if(ret==4)
  {
    saved=false;
    for(int i=0; i<qr_MAX; i++)
    {
      set_bit(quest_rules,i,rules_dlg[i+7].flags);
    }
    //this is only here until the subscreen style is selectable by itself
    zinit.subscreen_style=get_bit(quest_rules,qr_COOLSCROLL)?1:0;
  }
  return D_O_K;
}

void center_zq_rules_dialog()
{
  jwin_center_dialog(rules_dlg);
}
