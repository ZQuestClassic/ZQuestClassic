//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------


#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "zsys.h"
#include "zquest.h"
#include "gui.h"

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

static TABPANEL rules_tabs[] =
{
  // (text)
  { "Page 1", D_SELECTED, rules1_list },
  { "Page 2", 0,          rules2_list },
  { "Page 3", 0,          rules3_list },
  { "Page 4", 0,          rules4_list },
  { "Page 5", 0,          rules5_list },
  { NULL }
};

static DIALOG rules_dlg[] =
{
  /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,        10,   20,   300,  175,    vc(14),   vc(1),      0,      D_EXIT,     0,             0,       (void *) "Quest Rules" },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  // 3
  { jwin_button_proc,    170,  165,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_button_proc,     90,  165,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0,       (void *) "OK" },
  { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0,       (void *) onHelp },
  // 6
  { jwin_tab_proc,        15,   48,   290,  110,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) rules_tabs, NULL, (void *)rules_dlg },
  //rules1 (7-22)
  { jwin_check_proc,      20,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Solid Blocks" },
  { jwin_check_proc,      20,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Temporary No Return Disabled" },
  { jwin_check_proc,      20,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enemies Always Return" },
  { jwin_check_proc,      20,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Multi-Directional Traps" },

  { jwin_check_proc,      20,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "BS-Zelda Animation" },
  { jwin_check_proc,      20,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Interpolated Fading" },
  { jwin_check_proc,      20,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fade CSet 5" },
  { jwin_check_proc,      20,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fast Dungeon Scrolling" },

  { jwin_check_proc,     170,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Level 3 CSet Fix" },
  { jwin_check_proc,     170,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Cool Scrolling" },
  { jwin_check_proc,     170,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Start With 999 Rupies" },
  { jwin_check_proc,     170,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Big Triforce Pieces" },

  { jwin_check_proc,     170,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "24 Heart Containers" },
  { jwin_check_proc,     170,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fast Heart Refill" },
  { jwin_check_proc,     170,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "View Overworld Map" },
  { jwin_check_proc,     170,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "3 or 6 Triforce Total" },

  //rules2 (23-38)
  { jwin_check_proc,      20,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Time on Subscreen" },
  { jwin_check_proc,      20,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Freeform Dungeons" },
  { jwin_check_proc,      20,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Must Kill All Bosses" },
  { jwin_check_proc,      20,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Sprite Flicker" },

  { jwin_check_proc,      20,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Continue With Full Life" },
  { jwin_check_proc,      20,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Raft/Ladder Fix" },
  { jwin_check_proc,      20,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Lens Hints" },
  { jwin_check_proc,      20,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fix Link's Position in Dungeons" },

  { jwin_check_proc,     170,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Animate Held Items" },
  { jwin_check_proc,     170,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Hesitant Push Blocks" },
  { jwin_check_proc,     170,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Hide Enemy-Carried Items" },
  { jwin_check_proc,     170,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Silver Arrow Sparkles" },

  { jwin_check_proc,     170,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Golden Arrow Sparkles" },
  { jwin_check_proc,     170,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Magic Boomerang Sparkles" },
  { jwin_check_proc,     170,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fire Boomerang Sparkles" },
  { jwin_check_proc,     170,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Flashing Enemy Death" },

  //rules3 (39-54)
  { jwin_check_proc,      20,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Can Keep Old Items" },
  { jwin_check_proc,      20,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Own Fire Doesn't Hurt" },
  { jwin_check_proc,      20,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bombs Hurt" },
  { jwin_check_proc,      20,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Random Clocks" },

  { jwin_check_proc,      20,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Temporary Clocks" },
  { jwin_check_proc,      20,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Breakable Darknut Shields" },
  { jwin_check_proc,      20,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Broken Darknut Shield Tiles" },
  { jwin_check_proc,      20,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Mean Placed Traps" },

  { jwin_check_proc,     170,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Phantom Placed Traps" },
  { jwin_check_proc,     170,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Allow Fast Messages" },
  { jwin_check_proc,     170,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Linked Combos" },
  { jwin_check_proc,     170,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Guy Room Fires" },

  { jwin_check_proc,     170,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Heart Ring Fix" },
  { jwin_check_proc,     170,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Heart Ring" },
  { jwin_check_proc,     170,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Dodongo CSet Fix" },
  { jwin_check_proc,     170,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Sword/Wand Flip Fix" },

  //rules4 (55-70)
  { jwin_check_proc,      20,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Enable Magic" },
  { jwin_check_proc,      20,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Wand Uses Magic" },
  { jwin_check_proc,      20,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Candle Uses Magic" },
  { jwin_check_proc,      20,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Boots Use Magic" },

  { jwin_check_proc,      20,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Non-Bubble Medicine" },
  { jwin_check_proc,      20,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Non-Bubble Fairies" },
  { jwin_check_proc,      20,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Non-Bubble Triforce" },
  { jwin_check_proc,      20,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "New Enemy Tiles" },

  { jwin_check_proc,     170,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Rope 2 Flasing" },
  { jwin_check_proc,     170,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Bubble Flashing" },
  { jwin_check_proc,     170,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Ghini 2 Flickering" },
  { jwin_check_proc,     170,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Weapon Animation Fix" },

  { jwin_check_proc,     170,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Ghini 2 Translucency" },
  { jwin_check_proc,     170,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "New Boomerang/Hookshot" },
  { jwin_check_proc,     170,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Melee Weapon Grabbing" },
  { jwin_check_proc,     170,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Show Shadows" },

  //rules5 (71-86)
  { jwin_check_proc,      20,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Translucent Shadows" },
  { jwin_check_proc,      20,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Quick Sword" },
  { jwin_check_proc,      20,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Bomb Hold Fix" },
  { jwin_check_proc,      20,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Full Link Tile Modifiers" },

  { jwin_check_proc,      20,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Potion Combining" },
  { jwin_check_proc,      20,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Invincible Link Flickers" },
  { jwin_check_proc,      20,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Shadows Flicker" },
  { jwin_check_proc,      20,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Fliers Can Appear on Walls" },

  { jwin_check_proc,     170,   75,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Bomb Palette Flash" },
  { jwin_check_proc,     170,   85,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Trans. N. Love Shield" },
  { jwin_check_proc,     170,   95,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "N. Love Shield Flickers" },
  { jwin_check_proc,     170,  105,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Trans. N. Love Rocket" },

  { jwin_check_proc,     170,  115,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "N. Love Rocket Flickers" },
  { jwin_check_proc,     170,  125,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "No Scroll Warp Continue" },
  { jwin_check_proc,     170,  135,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Old-Style Tribbles" },
//  { jwin_check_proc,     170,  145,   185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "" },

  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
  { d_dummy_proc,          0,   0,     0,    0,    vc(15),   vc(1),      0,      0,          0,             0,       NULL },
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
  }
  return D_O_K;
}

void center_zq_rules_dialog()
{
  jwin_center_dialog(rules_dlg);
}
