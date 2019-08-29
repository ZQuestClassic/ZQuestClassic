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
  { "  1  ", D_SELECTED, rules1_list },
  { "  2  ", 0,          rules2_list },
  { "  3  ", 0,          rules3_list },
  { "  4  ", 0,          rules4_list },
  { "  5  ", 0,          rules5_list },
  { "  6  ", 0,          rules6_list },
  { "  7  ", 0,          rules7_list },
  { NULL }
};

static DIALOG rules_dlg[] =
{
  /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,         0,   0,   300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0,       (void *) "Quest Rules" },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  // 3
  { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0,       (void *) "OK" },
  { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0,       (void *) onHelp },
  // 6
  { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) rules_tabs, NULL, (void *)rules_dlg },
  //rules1 (7-22)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Solid Blocks" },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Temporary No Return Disabled" },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Always Return" },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Multi-Directional Traps" },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "BS-Zelda Animation" },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Interpolated Fading" },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fade CSet 5" },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fast Dungeon Scrolling" },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Level 3 CSet Fix" },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Cool Scrolling" },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Start With 999 Rupies" },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Big Triforce Pieces" },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "24 Heart Containers" },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fast Heart Refill" },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "View Overworld Map" },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "3 or 6 Triforce Total" },

  //rules2 (23-38)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Time on Subscreen" },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Freeform Dungeons" },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Must Kill All Bosses" },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Sprite Flicker" },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Continue With Full Life" },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Raft/Ladder Fix" },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Secret Combo Hints When Using The Lens" },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fix Link's Position in Dungeons" },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Items Held Above Link's Head Continue To Animate" },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Push Blocks Don't Move When Bumped" },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Hide Enemy-Carried Items" },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Silver Arrow Leaves Sparkle Trail Behind" },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Golden Arrow Leaves Sparkle Trail Behind" },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Magic Boomerang Leaves Sparkle Trail Behind" },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fire Boomerang Leaves Fire Trail Behind" },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Don't Flash When Dying" },

  //rules3 (39-54)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Link Can Keep Old Items When Acquiring Upgraded Ones" },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Link Isn't Hurt By Own Fire Weapons" },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bombs Hurt Link" },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Don't Randomly Drop Clocks" },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Clocks Last A Limited Time" },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Darknut Shields Can Be Broken By The Hammer" },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Broken Darknut Shield Tiles" },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Mean Placed Traps" },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Placed Traps Can Pass Through Enemies" },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Messages Can Be Sped Up With The A Button" },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Walk-Affected Combos Are Linked Together" },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Fires in Special Rooms" },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Healing Fairy Heart Ring Is Centered Around Link" },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Healing Fairy Heart Ring" },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Dodongo CSet Fix" },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Sword/Wand Flip Fix" },

  //rules4 (55-70)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enable Magic" },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Wand Consumes Magic" },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Candle Consumes Magic" },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Boots Consume Magic" },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Medicine Doesn't Re-Enable Bubble-Removed Items" },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fairies Don't Re-Enable Bubble-Removed Items" },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Triforce Doesn't Re-Enable Bubble-Removed Items" },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Use New Enemy Tiles" },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Rope 2 Doesn't Flash" },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bubbles Don't Flash" },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Ghini 2 Flickers" },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Weapon Animation Fix" },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Ghini 2 Is Transparent" },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "New Boomerang/Hookshot" },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Melee Weapons Can't Pick Up Items" },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Shadows" },

  //rules5 (71-86)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Translucent Shadows" },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Quick Sword" },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Link Holds Special Bombs Over His Head" },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Expanded Link Tile Modifiers" },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "2 Blue Potions Can't Be Combined Into 1 Red Potion" },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Invincible Link Flickers" },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Shadows Flicker" },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fliers Can Appear on Walls" },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bomb Explosions Don't Flash Palette" },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Transparent Nayru's Love Shield" },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Nayru's Love Shield Flickers" },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Transparent Nayru's Love Rocket" },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Nayru's Love Rocket Flickers" },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Combo Cycling On Layers" },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Full Priority Damage Combos" },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Warps Ignore Arrival X/Y Position" },
  //  qr_LTTPCOLLISION (87)
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  //  qr_LTTPWALK (88)
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  //rules6 (89-104)
  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Slow Enemy Animation" },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "True Arrows" },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Saving When Continuing" },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Skip Continue Screen" },

  { jwin_check_proc,      10,   83,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enable 1/4 Heart Life Bar" },
  { jwin_check_proc,      10,   93,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Use Warp Return Points Only" },
  { jwin_check_proc,      10,  103,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Special Room Guys Don't Create A Poof When Appearing" },
  { jwin_check_proc,      10,  113,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Messages Can Be Bypassed With The B Button" },

  { jwin_check_proc,      10,  123,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Diving" },
  { jwin_check_proc,      10,  133,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Layers 1 and 2 Are Drawn Under Caves" },
  { jwin_check_proc,      10,  143,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Scrolling Warps Don't Set The Continue Point" },
  { jwin_check_proc,      10,  153,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Smart Screen Scrolling" },

  { jwin_check_proc,      10,  163,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Rings Affect Damage Combos" },
  { jwin_check_proc,      10,  173,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Allow 10 Rupee Drops" },
  { jwin_check_proc,      10,  183,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Trap Position Fix" },
  { jwin_check_proc,      10,  193,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Dark Rooms Stay Lit Only While Fire Is On Screen" },

  { jwin_check_proc,      10,   43,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Red Potion Disappears When Used" },
  { jwin_check_proc,      10,   53,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Use Old-Style Warp Detection" },
  { jwin_check_proc,      10,   63,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Invisible Border on Non-Dungeon Dmaps" },
  { jwin_check_proc,      10,   73,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "1.90-Style Tribbles"},

  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },

  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },

  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },

  { d_dummy_proc,          0,    0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { NULL }
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
