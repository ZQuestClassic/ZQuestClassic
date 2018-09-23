//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <vector>

#include "gui.h"
#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "zsys.h"
#include "zquest.h"
#include "zq_custom.h"

static int animrules1_list[] =
{
    6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,-1
};

// Yep, a whole tab for one rule.
static int animrules2_list[] =
{
    22,-1
};

static TABPANEL animrules_tabs[] =
{
    // (text)
    { (char *)" 1 ",     D_SELECTED, animrules1_list, 0, NULL },
    { (char *)" 2 ",     0,          animrules2_list, 0, NULL },
    { NULL,              0,          NULL,            0, NULL }
};

static DIALOG animationrules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Animation", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) animrules_tabs, NULL, (void *)animationrules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "BS-Zelda Animation", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Circle Opening/Closing Wipes", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Oval Opening/Closing Wipes", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Triangle Opening/Closing Wipes", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Super Mario All-Stars Opening/Closing Wipes", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Expanded Link Tile Modifiers", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Screen Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fast Dungeon Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Very Fast Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Smooth Vertical Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "FFCs Are Visible While The Screen Is Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Interpolated Fading", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fade CSet 5", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Invincible Link Flickers", NULL, NULL },
    { jwin_check_proc,      10, 33+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Items Held Above Link's Head Continue To Animate", NULL, NULL },
    { jwin_check_proc,      10, 33+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Bomb Explosions Don't Flash Palette", NULL, NULL },
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Layers 1 and 2 Are Drawn Under Caves", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int animationrules[] =
{
    qr_BSZELDA, qr_COOLSCROLL, qr_OVALWIPE, qr_TRIANGLEWIPE,
    qr_SMASWIPE, qr_EXPANDEDLTM, qr_NOSCROLL, qr_FASTDNGN,
    qr_VERYFASTSCROLLING, qr_SMOOTHVERTICALSCROLLING, qr_FFCSCROLL, qr_FADE,
    qr_FADECS5, qr_LINKFLICKER, qr_HOLDITEMANIMATION, qr_NOBOMBPALFLASH,
    qr_LAYER12UNDERCAVE, -1
};

int onAnimationRules()
{
    if(is_large)
        large_dialog(animationrules_dlg);
        
    animationrules_dlg[0].dp2=lfont;
    
    for(int i=0; animationrules[i]!=-1; i++)
    {
        animationrules_dlg[i+6].flags = get_bit(quest_rules,animationrules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(animationrules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; animationrules[i]!=-1; i++)
        {
            set_bit(quest_rules,animationrules[i],animationrules_dlg[i+6].flags & D_SELECTED);
        }
        
        // For 2.50.0 and 2.50.1
        if(get_bit(quest_rules, qr_VERYFASTSCROLLING))
            set_bit(quest_rules, qr_FASTDNGN, 1);
        
        //this is only here until the subscreen style is selectable by itself
        zinit.subscreen_style=get_bit(quest_rules,qr_COOLSCROLL)?1:0;
    }
    
    return D_O_K;
}

static DIALOG comborules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Combos", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Link Drowns in Walkable Water", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Smart Screen Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Can't Push Blocks Onto Unwalkable Combos", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Push Blocks Don't Move When Bumped", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Burn Flags Are Triggered Instantly", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Magic Mirror/Prism Combos Reflect Sword Beams", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Magic Mirrors Reflect Whistle Whirlwinds", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Combo Cycling On Layers", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Full Priority Damage Combos", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Warps Ignore Arrival X/Y Position When Setting Continue Screen", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Use Warp Return Points Only", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Scrolling Warps Don't Set The Continue Point", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Use Old-Style Warp Detection (NES Movement Only)", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Damage Combos Work On Layers 1 And 2", NULL, NULL },
    { jwin_check_proc,      10, 33+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Hookshot Grab Combos Work On Layers 1 And 2", NULL, NULL },
    //{ jwin_check_proc,      10, 33+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Changing DMaps Doesn't Set The Continue Point", NULL, NULL },
    { d_dummy_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int comborules[] =
{
    qr_DROWN, qr_SMARTSCREENSCROLL, qr_SOLIDBLK, qr_HESITANTPUSHBLOCKS, qr_INSTABURNFLAGS,
    qr_SWORDMIRROR, qr_WHIRLWINDMIRROR, qr_CMBCYCLELAYERS, qr_DMGCOMBOPRI,
    qr_WARPSIGNOREARRIVALPOINT, qr_NOARRIVALPOINT, qr_NOSCROLLCONTINUE, qr_OLDSTYLEWARP,
    qr_DMGCOMBOLAYERFIX, qr_HOOKSHOTLAYERFIX, -1
};

int onComboRules()
{
    if(is_large)
        large_dialog(comborules_dlg);
        
    comborules_dlg[0].dp2=lfont;
    
    for(int i=0; comborules[i]!=-1; i++)
    {
        comborules_dlg[i+6].flags = get_bit(quest_rules,comborules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(comborules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; comborules[i]!=-1; i++)
        {
            set_bit(quest_rules, comborules[i], comborules_dlg[i+6].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

static int itemrules1_list[] =
{
    6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,-1
};

static int itemrules2_list[] =
{
    22,26,27,28,29,-1
	//22,23,24,25,26,27,28,29,-1
};

static TABPANEL itemrules_tabs[] =
{
    // (text)
    { (char *)" 1 ",     D_SELECTED, itemrules1_list, 0, NULL },
    { (char *)" 2 ",     0,          itemrules2_list, 0, NULL },
    { NULL,              0,          NULL,            0, NULL }
};

static DIALOG itemrules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Items", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) itemrules_tabs, NULL, (void *)itemrules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules //6
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enable Magic", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "New Boomerang/Hookshot", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "True Arrows", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Diving", NULL, NULL },
   //10
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Dark Rooms Stay Lit Only While Fire Is On Screen", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Link Isn't Hurt By Own Fire Weapons", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Link's Bombs Hurt Link", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Rings Affect Damage Combos", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Quick Sword", NULL, NULL },
    //15
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Flip Right-Facing Slash", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Wand Can't Be Used As Melee Weapon", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Melee Weapons Can't Pick Up Items", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Boomerang/Hookshot Grabs All Items", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "'Hearts Required' Affects Non-Special Items", NULL, NULL },
  //20
    { jwin_check_proc,      10, 33+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Big Triforce Pieces", NULL, NULL },
    { jwin_check_proc,      10, 33+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "3 or 6 Triforce Total", NULL, NULL },
    // rules 2 //22
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Slow Walk While Charging", NULL, NULL },
 //Deprecated:  23, 24, 25, 26, 27, 28, 29
    { d_dummy_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Show Secret Combo Hints When Using The Lens", NULL, NULL },
    { d_dummy_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Lens Shows Raft Paths", NULL, NULL },
    { d_dummy_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Lens Reveals Invisible Enemies", NULL, NULL },
	
    { d_dummy_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Medicine Doesn't Remove Sword Jinxes", NULL, NULL },
    { d_dummy_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fairies Don't Remove Sword Jinxes", NULL, NULL },
    { d_dummy_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Triforce Doesn't Remove Sword Jinxes", NULL, NULL },
    { d_dummy_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Things That Remove Sword Jinxes Also Remove Item Jinxes", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};


static int itemrules[] =
{
    qr_ENABLEMAGIC, qr_Z3BRANG_HSHOT, qr_TRUEARROWS, qr_NODIVING, qr_TEMPCANDLELIGHT,
    qr_FIREPROOFLINK, qr_OUCHBOMBS, qr_RINGAFFECTDAMAGE, qr_QUICKSWORD, qr_SLASHFLIPFIX,
    qr_NOWANDMELEE, qr_NOITEMMELEE, qr_BRANGPICKUP, qr_HEARTSREQUIREDFIX, qr_4TRI, qr_3TRI,
    qr_SLOWCHARGINGWALK, qr_LENSHINTS, qr_RAFTLENS, qr_LENSSEESENEMIES,
    qr_NONBUBBLEMEDICINE, qr_NONBUBBLEFAIRIES, qr_NONBUBBLETRIFORCE, qr_ITEMBUBBLE, -1
};

int onItemRules()
{
    if(is_large)
        large_dialog(itemrules_dlg);
        
    itemrules_dlg[0].dp2=lfont;
    
    for(int i=0; itemrules[i]!=-1; i++)
    {
        itemrules_dlg[i+6].flags = get_bit(quest_rules,itemrules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(itemrules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; itemrules[i]!=-1; i++)
        {
            set_bit(quest_rules, itemrules[i], itemrules_dlg[i+6].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

static int enemyrules1_list[] =
{
    6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,-1
};

static int enemyrules2_list[] =
{
    22,-1
};

static TABPANEL enemyrules_tabs[] =
{
  // (text)
  { (char *)" 1 ",     D_SELECTED, enemyrules1_list, 0, NULL },
  { (char *)" 2 ",     0,          enemyrules2_list, 0, NULL },
  { NULL,              0,          NULL,             0, NULL }
};

static DIALOG enemyrules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Enemies", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) enemyrules_tabs, NULL, (void *)enemyrules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Use New Enemy Tiles", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Jump/Fly Through Z-Axis", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Hide Enemy-Carried Items", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Always Return", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Temporary No Return Disabled", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Must Kill All Bosses To Set 'No Return' Screen State", NULL, NULL },
    { jwin_check_proc,      10, 33+70, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Multi-Directional Traps", NULL, NULL },
    { jwin_check_proc,      10, 33+80, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Line-of-Sight Traps Can Move Across Entire Screen", NULL, NULL },
    { jwin_check_proc,      10, 33+90, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Constant Traps Can Pass Through Enemies", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Flying Enemies Can Appear on Unwalkable Combos", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Broken Enemy Shield Tiles", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Don't Flash When Dying", NULL, NULL },
    { jwin_check_proc,      10, 33+130,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Show Shadows", NULL, NULL },
    { jwin_check_proc,      10, 33+140,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Translucent Shadows", NULL, NULL },
    { jwin_check_proc,      10, 33+150,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Shadows Flicker", NULL, NULL },
    { jwin_check_proc,      10, 33+160,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Flicker When Hit", NULL, NULL },
    
    // rules 2
    { jwin_check_proc,      10, 33+10, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Statue Minimum Range Or Double Fireballs", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int enemyrules[] =
{
    qr_NEWENEMYTILES, qr_ENEMIESZAXIS,
    qr_HIDECARRIEDITEMS, qr_ALWAYSRET, qr_NOTMPNORET, qr_KILLALL,
    qr_MEANTRAPS, qr_MEANPLACEDTRAPS, qr_PHANTOMPLACEDTRAPS, qr_WALLFLIERS,
    qr_BRKNSHLDTILES, qr_NOFLASHDEATH, qr_SHADOWS, qr_TRANSSHADOWS,
    qr_SHADOWSFLICKER, qr_ENEMIESFLICKER, qr_BROKENSTATUES,
    -1
};

int onEnemyRules()
{
    if(is_large)
        large_dialog(enemyrules_dlg);
        
    enemyrules_dlg[0].dp2=lfont;
    
    for(int i=0; enemyrules[i]!=-1; i++)
    {
        enemyrules_dlg[i+6].flags = get_bit(quest_rules,enemyrules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(enemyrules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; enemyrules[i]!=-1; i++)
        {
            set_bit(quest_rules, enemyrules[i], enemyrules_dlg[i+6].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

static int fixesrules1_list[] =
{
    6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,-1
};

static int fixesrules2_list[] =
{
    22,23,24,25,26,27,28,-1
};

static TABPANEL fixesrules_tabs[] =
{
    // (text)
    { (char *)" 1 ",     D_SELECTED,  fixesrules1_list, 0, NULL },
    { (char *)" 2 ",     0,           fixesrules2_list, 0, NULL },
    { NULL,              0,           NULL,             0, NULL }
};

static DIALOG fixesrules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - NES Fixes", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) fixesrules_tabs, NULL, (void *)fixesrules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Freeform Dungeons", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Can Safely Trigger Armos/Grave From The South", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Can Use Items/Weapons on Edge of Screen", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fix Link's Position in Dungeons", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Raft/Ladder Sprite Direction Fix", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Palette 3 CSet 6 Fix", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Link Holds Special Bombs Over His Head", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Holding Up Items Doesn't Restart Music", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Leaving Item Cellar/Passageway Doesn't Restart Music", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Tunic Color Can Change On Overworld", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Sword/Wand Flip Fix", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Push Block CSet Fix", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Trap Position Fix", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Invisible Border on Non-Dungeon Dmaps", NULL, NULL },
    { jwin_check_proc,      10, 33+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Items Disappear During Hold-Up", NULL, NULL },
    { jwin_check_proc,      10, 33+160,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Subscreen Appears Above Sprites", NULL, NULL },
    // rules 2
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Correct Bomb/Darknut Interaction", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Correct Offset Enemy Weapon Collision Detection", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Special Items Don't Appear In Passageways", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No NES Sprite Flicker", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Invincible Link Isn't Hurt By Own Fire Weapons", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Position Offset Of Screen Items", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Allow Ladder Anywhere", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int fixesrules[] =
{
    qr_FREEFORM, qr_SAFEENEMYFADE, qr_ITEMSONEDGES, qr_LINKDUNGEONPOSFIX, qr_RLFIX,
    qr_NOLEVEL3FIX, qr_BOMBHOLDFIX, qr_HOLDNOSTOPMUSIC, qr_CAVEEXITNOSTOPMUSIC,
    qr_OVERWORLDTUNIC, qr_SWORDWANDFLIPFIX, qr_PUSHBLOCKCSETFIX,
    qr_TRAPPOSFIX, qr_NOBORDER, qr_OLDPICKUP, qr_SUBSCREENOVERSPRITES,
    qr_BOMBDARKNUTFIX, qr_OFFSETEWPNCOLLISIONFIX, qr_ITEMSINPASSAGEWAYS,
    qr_NOFLICKER, qr_FIREPROOFLINK2, qr_NOITEMOFFSET, qr_LADDERANYWHERE,-1
};

int onFixesRules()
{
    if(is_large)
        large_dialog(fixesrules_dlg);
        
    fixesrules_dlg[0].dp2=lfont;
    
    for(int i=0; fixesrules[i]!=-1; i++)
    {
        fixesrules_dlg[i+6].flags = get_bit(quest_rules,fixesrules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(fixesrules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; fixesrules[i]!=-1; i++)
        {
            set_bit(quest_rules, fixesrules[i], fixesrules_dlg[i+6].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

static int miscrules1_list[] =
{
    6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,-1
};

static int miscrules2_list[] =
{
    22,23,24,-1
};

static TABPANEL miscrules_tabs[] =
{
    // (text)
    { (char *)" 1 ",     D_SELECTED,  miscrules1_list, 0, NULL },
    { (char *)" 2 ",     0,           miscrules2_list, 0, NULL },
    { NULL,              0,           NULL,             0, NULL }
};

static DIALOG miscrules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Other", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) miscrules_tabs, NULL, (void *)miscrules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Can Select A-Button Weapon On Subscreen", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "More Sound Effects", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fast Heart Refill", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enable 1/4 Heart Life Bar", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Messages Can Be Sped Up With The A Button", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Messages Can Be Bypassed With The B Button", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Messages Freeze All Action", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Messages Disappear", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Show Time On Subscreen", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Healing Fairy Heart Circle Is Centered Around Link", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Healing Fairy Heart Circle", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Saving When Continuing", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Skip Continue Screen", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Fires In Special Rooms", NULL, NULL },
    { jwin_check_proc,      10, 33+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Special Room Guys Don't Create A Puff When Appearing", NULL, NULL },
    { jwin_check_proc,      10, 33+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Log Game Events To Allegro.log", NULL, NULL },
    
    // rules 2
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Log Script Errors To Allegro.log", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Draining Rupees Can Still Be Spent", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Intro String in Ganon Room", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int miscrules[] =
{
    qr_SELECTAWPN, qr_MORESOUNDS, qr_FASTFILL, qr_QUARTERHEART, qr_ALLOWFASTMSG, qr_ALLOWMSGBYPASS,
    qr_MSGFREEZE, qr_MSGDISAPPEAR, qr_TIME, qr_HEARTRINGFIX, qr_NOHEARTRING, qr_NOSAVE,
    qr_NOCONTINUE, qr_NOGUYFIRES, qr_NOGUYPOOF, qr_LOG, qr_SCRIPTERRLOG, qr_SHOPCHEAT, 
	qr_NOGANONINTRO, -1
};

int onMiscRules()
{
    if(is_large)
        large_dialog(miscrules_dlg);
        
    miscrules_dlg[0].dp2=lfont;
    
    for(int i=0; miscrules[i]!=-1; i++)
    {
        miscrules_dlg[i+6].flags = get_bit(quest_rules,miscrules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(miscrules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; miscrules[i]!=-1; i++)
        {
            set_bit(quest_rules, miscrules[i], miscrules_dlg[i+6].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

static int compatrules[] = 
{
   qr_GOTOLESSNOTEQUAL, qr_OLDLENSORDER, qr_NOFAIRYGUYFIRES, qr_TRIGGERSREPEAT,
   qr_HOOKSHOTDOWNBUG, qr_REPLACEOPENDOORS, qr_NOSOLIDDAMAGECOMBOS, qr_OLDHOOKSHOTGRAB,
   qr_PEAHATCLOCKVULN, qr_OFFSCREENWEAPONS, qr_ITEMPICKUPSETSBELOW,
   -1 
};

static DIALOG compatrules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Compatibility", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0,        NULL, NULL, NULL },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // Notice
    { jwin_text_proc,       10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "These rules are enabled automatically for compatibility.", NULL, NULL },
    { jwin_text_proc,       10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enabling them manually may cause unexpected behavior.", NULL, NULL },
    
    // rules
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Old GOTOLESS Behavior", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Old Lens Drawing Order", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Fairy Guy Fires", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Continuous Step Triggers", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Downward Hookshot Bug", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fix Open Door Solidity", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Solid Damage Combos", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Old Hookshot Grab Checking", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Peahats Are Vulnerable When Frozen By Clocks", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Weapons With No Collision Detection Move Offscreen", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Screen Item Pickup Disables Hunger/Special Item", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

int onCompatRules()
{
    if(is_large)
        large_dialog(compatrules_dlg);
        
    compatrules_dlg[0].dp2=lfont;
    
    for(int i=0; compatrules[i]!=-1; i++)
    {
        compatrules_dlg[i+8].flags = get_bit(quest_rules,compatrules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(compatrules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; compatrules[i]!=-1; i++)
        {
            set_bit(quest_rules, compatrules[i], compatrules_dlg[i+8].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

void center_zq_rules_dialog()
{
    jwin_center_dialog(animationrules_dlg);
    jwin_center_dialog(comborules_dlg);
    jwin_center_dialog(itemrules_dlg);
    jwin_center_dialog(enemyrules_dlg);
    jwin_center_dialog(fixesrules_dlg);
    jwin_center_dialog(miscrules_dlg);
    jwin_center_dialog(compatrules_dlg);
}

