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
    22,23,/*24,*/-1
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
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fade to/from Black Opening/Closing Wipes", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Expanded Link Tile Modifiers", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Screen Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fast Dungeon Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Very Fast Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Smooth Vertical Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "FFCs Are Visible While The Screen Is Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Interpolated Fading", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fade CSet 5", NULL, NULL },
    { jwin_check_proc,      10, 33+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Invincible Link Flickers", NULL, NULL },
    { jwin_check_proc,      10, 33+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Items Held Above Link's Head Continue To Animate", NULL, NULL },
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Bomb Explosions Don't Flash Palette", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Layers 1 and 2 Are Drawn Under Caves", NULL, NULL },
    //{ jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Animate Custom / Script Type Weapons", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int animationrules[] =
{
    qr_BSZELDA, qr_COOLSCROLL, qr_OVALWIPE, qr_TRIANGLEWIPE,
    qr_SMASWIPE, qr_FADEBLACKWIPE, qr_EXPANDEDLTM, qr_NOSCROLL, qr_FASTDNGN,
    qr_VERYFASTSCROLLING, qr_SMOOTHVERTICALSCROLLING, qr_FFCSCROLL, qr_FADE,
    qr_FADECS5, qr_LINKFLICKER, qr_HOLDITEMANIMATION, qr_NOBOMBPALFLASH,
    qr_LAYER12UNDERCAVE, /*qr_ANIMATECUSTOMWEAPONS,*/
	-1
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

static int comborules1_list[] =
{
    6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,-1
};

static int comborules2_list[] =
{
    22,23,24,25,26,27,28,29,-1 
};

static TABPANEL comborules_tabs[] =
{
    // (text)
    { (char *)" 1 ",     D_SELECTED, comborules1_list, 0, NULL },
    { (char *)" 2 ",     0,          comborules2_list, 0, NULL },
    { NULL,              0,          NULL,            0, NULL }
};

static DIALOG comborules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Combos", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) comborules_tabs, NULL, (void *)comborules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Link Drowns in Walkable Water", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Smart Screen Scrolling", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Can't Push Blocks Onto Unwalkable Combos", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Push Blocks Don't Move When Bumped", NULL, NULL },
    // 10
	{ jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Burn Flags Are Triggered Instantly", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Magic Mirror/Prism Combos Reflect Enemy and Scripted Sword Beams", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Magic Mirrors Reflect Scripted Whirlwinds", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Combo Cycling On Layers", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Full Priority Damage Combos", NULL, NULL },
    // 15
	{ jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Warps Ignore Arrival X/Y Position When Setting Continue Screen", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Use Warp Return Points Only", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Scrolling Warps Don't Set The Continue Point", NULL, NULL },
    { jwin_check_proc,      10, 33+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Use Old-Style Warp Detection (NES Movement Only)", NULL, NULL },
    { jwin_check_proc,      10, 33+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Damage Combos Work On Layers 1 And 2", NULL, NULL },
    // 20
	{ jwin_check_proc,      10, 33+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Hookshot Grab Combos Work On Layers 1 And 2", NULL, NULL },
    // { jwin_check_proc,      10, 33+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,       (void *) "Changing DMaps Doesn't Set The Continue Point", NULL, NULL },
    // { d_dummy_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_check_proc,      10, 33+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Broken Mirror and Weapon Interaction", NULL, NULL },
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Always Face Up on Sideview Ladders", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Press 'Down' to Fall Through Sideview Platforms", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Press 'Down+Jump' to Fall Through Sideview Platforms", NULL, NULL },
    // 25
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Falling Through Sideview Platforms Respects 'Drunk' Inputs", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Pressing Down Will Not Grab Sideview Ladders", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Custom Combos Work on Layers 1 and 2", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Slash Combos Work on Layers 1 and 2", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "New Combo Animation", NULL, NULL },
  
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int comborules[] =
{
    qr_DROWN, qr_SMARTSCREENSCROLL, qr_SOLIDBLK, qr_HESITANTPUSHBLOCKS, qr_INSTABURNFLAGS,
    qr_SWORDMIRROR, qr_WHIRLWINDMIRROR, qr_CMBCYCLELAYERS, qr_DMGCOMBOPRI,
    qr_WARPSIGNOREARRIVALPOINT, qr_NOARRIVALPOINT, qr_NOSCROLLCONTINUE, qr_OLDSTYLEWARP,
    qr_DMGCOMBOLAYERFIX, qr_HOOKSHOTLAYERFIX, qr_OLDMIRRORCOMBOS, qr_SIDEVIEWLADDER_FACEUP,
	qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS, qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS,
	qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK, qr_DOWN_DOESNT_GRAB_LADDERS, qr_CUSTOMCOMBOSLAYERS1AND2,
	qr_BUSHESONLAYERS1AND2, qr_NEW_COMBO_ANIMATION,
	
	-1
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

static int weapon_eweapon_rules_tab[] =
{
    6,
	-1
};

static int weapon_lweapon_rules_tab[] =
{
	//6
    -1
};

static int weapon_global_rules_tab[] =
{
	7,8,
    -1
};

static TABPANEL weaponrules_tabs[] =
{
  // (text)
  { (char *)"All",     D_SELECTED,          weapon_global_rules_tab, 0, NULL },
  { (char *)"EWeapons",     0,          weapon_eweapon_rules_tab, 0, NULL },
  { (char *)"LWeapons",     0, weapon_lweapon_rules_tab, 0, NULL },
  
  
  { NULL,              0,          NULL,             0, NULL }
};

static DIALOG weaponrules_dlg[] =
{
	/* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
	{ jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Weapons", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
    { jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) weaponrules_tabs, NULL, (void *)weaponrules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    
    // rules //6
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "npc->Weapon Uses Unique Sprites for Custom EWeapons", NULL, NULL },
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Angular Reflected Weapons", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Mirrors Use Weapon Centre for Collision", NULL, NULL },
    // { d_dummy_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) " ", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int weaponrules[] =
{
   qr_SCRIPT_WEAPONS_UNIQUE_SPRITES, qr_ANGULAR_REFLECTED_WEAPONS, qr_MIRRORS_USE_WEAPON_CENTRE,
    -1
};


int onWeaponRules()
{
    if(is_large)
        large_dialog(weaponrules_dlg);
        
    weaponrules_dlg[0].dp2=lfont;
    
    for(int i=0; weaponrules[i]!=-1; i++)
    {
        weaponrules_dlg[i+6].flags = get_bit(quest_rules,weaponrules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(weaponrules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; weaponrules[i]!=-1; i++)
        {
            set_bit(quest_rules, weaponrules[i], weaponrules_dlg[i+6].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

static int herorules_tab_1[] =
{
	6,7,
    -1
};

static TABPANEL herorules_tabs[] =
{
	// (text)
	{ (char *)" 1 ",     D_SELECTED,          herorules_tab_1, 0, NULL },
	
	{ NULL,              0,          NULL,             0, NULL }
};

static DIALOG herorules_dlg[] =
{
	/* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
	{ jwin_win_proc,         0,   0,    300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0, (void *) "Quest Rules - Hero", NULL, NULL },
	{ d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL },
	{ jwin_tab_proc,         5,   23,   290,  181,    vc(14),   vc(1),      0,      0,          1,             0, (void *) herorules_tabs, NULL, (void *)herorules_dlg },
	// 3
	{ jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
	
	// rules //6
	{ jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Diagonal Movement", NULL, NULL },
	{ jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Large Hitbox", NULL, NULL },
	{ jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "New Hero Movement", NULL, NULL },
	{ jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Disable 4-Way Movement's Gridlock", NULL, NULL },
	// { d_dummy_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "", NULL, NULL },
	{ NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int herorules[] =
{
   qr_LTTPWALK, qr_LTTPCOLLISION, qr_NEW_HERO_MOVEMENT, qr_DISABLE_4WAY_GRIDLOCK,
    -1
};

int onHeroRules()
{
    if(is_large)
        large_dialog(herorules_dlg);
        
    herorules_dlg[0].dp2=lfont;
    
    for(int i=0; herorules[i]!=-1; i++)
    {
        herorules_dlg[i+6].flags = get_bit(quest_rules,herorules[i]) ? D_SELECTED : 0;
    }
    
    int ret = zc_popup_dialog(herorules_dlg,4);
    
    if(ret==4)
    {
        saved=false;
        
        for(int i=0; herorules[i]!=-1; i++)
        {
            set_bit(quest_rules, herorules[i], herorules_dlg[i+6].flags & D_SELECTED);
        }
    }
    
    return D_O_K;
}

static int itemrules1_list[] =
{
	6, 8, 10, 11, 12, 17, 19, 20, 21, 27, 29, 31, 32, 33, -1
};

/*static int itemrules2_list[] =
{
	-1
};*/

static TABPANEL itemrules_tabs[] =
{
	// (text)
	{ (char *)" 1 ",     D_SELECTED, itemrules1_list, 0, NULL },
	// { (char *)" 2 ",     0,          itemrules2_list, 0, NULL },
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
	{ jwin_check_proc,      10, 21+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enable Magic", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "New Boomerang/Hookshot", NULL, NULL },
	{ jwin_check_proc,      10, 21+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "True Arrows", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Diving", NULL, NULL },
	//10
	{ jwin_check_proc,      10, 21+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Scripted and Enemy Fire Lights Temporarily", NULL, NULL },
	{ jwin_check_proc,      10, 21+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Scripted Fire LWeapons Don't Hurt Link", NULL, NULL },
	{ jwin_check_proc,      10, 21+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Scripted Bomb LWeapons Hurt Link", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Rings Affect Damage Combos", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Quick Sword", NULL, NULL },
	//15
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Flip Right-Facing Slash", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Wand Can't Be Used As Melee Weapon", NULL, NULL },
	{ jwin_check_proc,      10, 21+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Scripted Melee Weapons Can't Pick Up Items", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Boomerang/Hookshot Grabs All Items", NULL, NULL },
	{ jwin_check_proc,      10, 21+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "'Hearts Required' Affects Non-Special Items", NULL, NULL },
	//20
	{ jwin_check_proc,      10, 21+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Big Triforce Pieces", NULL, NULL },
	{ jwin_check_proc,      10, 21+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "3 or 6 Triforce Total", NULL, NULL },
	// rules 2 //22
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Slow Walk While Charging", NULL, NULL },
	//Deprecated:  23, 24, 25, 26, 27, 28, 29
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Show Secret Combo Hints When Using The Lens", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Lens Shows Raft Paths", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Lens Reveals Invisible Enemies", NULL, NULL },
	
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Medicine Doesn't Remove Sword Jinxes", NULL, NULL },
	{ jwin_check_proc,      10, 21+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fairy Combos Don't Remove Sword Jinxes", NULL, NULL },
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Triforce Doesn't Remove Sword Jinxes", NULL, NULL },
	{ jwin_check_proc,      10, 21+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Fairy Combos Remove Item Jinxes", NULL, NULL },
	//30
	{ d_dummy_proc,         10, 21+500, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Melee Weapons Require Magic Cost", NULL, NULL },
	{ jwin_check_proc,      10, 21+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Broken Magic Book Costs", NULL, NULL },
	{ jwin_check_proc,      10, 21+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Reroll Useless Drops", NULL, NULL },
	{ jwin_check_proc,      10, 21+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Items Ignore Sideview Platforms", NULL, NULL },
   
	
	{ NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};


static int itemrules[] =
{
    qr_ENABLEMAGIC, qr_Z3BRANG_HSHOT, qr_TRUEARROWS, qr_NODIVING, qr_TEMPCANDLELIGHT,
    qr_FIREPROOFLINK, qr_OUCHBOMBS, qr_RINGAFFECTDAMAGE, qr_QUICKSWORD, qr_SLASHFLIPFIX,
    qr_NOWANDMELEE, qr_NOITEMMELEE, qr_BRANGPICKUP, qr_HEARTSREQUIREDFIX, qr_4TRI, qr_3TRI,
    qr_SLOWCHARGINGWALK, qr_LENSHINTS, qr_RAFTLENS, qr_LENSSEESENEMIES,
    qr_NONBUBBLEMEDICINE, qr_NONBUBBLEFAIRIES, qr_NONBUBBLETRIFORCE, qr_ITEMBUBBLE,
	qr_MELEEMAGICCOST, qr_BROKENBOOKCOST, qr_SMARTDROPS, qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS, -1
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
    22,23,24,25,-1
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
    
    // rules //6
    { jwin_check_proc,      10, 33+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Use New Enemy Tiles", NULL, NULL },
    { jwin_check_proc,      10, 33+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Jump/Fly Through Z-Axis", NULL, NULL },
    { jwin_check_proc,      10, 33+30,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Hide Enemy-Carried Items", NULL, NULL },
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Always Return", NULL, NULL },
    //10
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Temporary No Return Disabled", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Must Kill All Bosses To Set 'No Return' Screen State", NULL, NULL },
    { jwin_check_proc,      10, 33+70, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Multi-Directional Traps", NULL, NULL },
    { jwin_check_proc,      10, 33+80, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Line-of-Sight Traps Can Move Across Entire Screen", NULL, NULL },
    { jwin_check_proc,      10, 33+90, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Constant Traps Can Pass Through Enemies", NULL, NULL },
    //15
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Flying Enemies Can Appear on Unwalkable Combos", NULL, NULL },
    { jwin_check_proc,      10, 33+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Broken Enemy Shield Tiles", NULL, NULL },
    { jwin_check_proc,      10, 33+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Don't Flash When Dying", NULL, NULL },
    { jwin_check_proc,      10, 33+130,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Show Shadows", NULL, NULL },
    { jwin_check_proc,      10, 33+140,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Translucent Shadows", NULL, NULL },
    //20
    { jwin_check_proc,      10, 33+150,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Shadows Flicker", NULL, NULL },
    { jwin_check_proc,      10, 33+160,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Flicker When Hit", NULL, NULL },
    
    // rules 2 //22
    { jwin_check_proc,      10, 33+10, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Statue Minimum Range Or Double Fireballs", NULL, NULL },
    { jwin_check_proc,      10, 33+20, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Use Super Bomb Explosions for Explode on Contact", NULL, NULL },
    { jwin_check_proc,      10, 33+30, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Enemies Can Go Out of Bounds (Offscreen)", NULL, NULL },
    { jwin_check_proc,      10, 33+40, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Disable Spawning Custom and Friendly Enemy Types", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int enemyrules[] =
{
    qr_NEWENEMYTILES, qr_ENEMIESZAXIS,
    qr_HIDECARRIEDITEMS, qr_ALWAYSRET, qr_NOTMPNORET, qr_KILLALL,
    qr_MEANTRAPS, qr_MEANPLACEDTRAPS, qr_PHANTOMPLACEDTRAPS, qr_WALLFLIERS,
    qr_BRKNSHLDTILES, qr_NOFLASHDEATH, qr_SHADOWS, qr_TRANSSHADOWS,
    qr_SHADOWSFLICKER, qr_ENEMIESFLICKER, qr_BROKENSTATUES, qr_BOMBCHUSUPERBOMB,qr_OUTOFBOUNDSENEMIES,
	qr_SCRIPT_FRIENDLY_ENEMY_TYPES,
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
    22,23,24,25,26,27,28,29,30,31,-1
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
    { jwin_check_proc,      10, 33+40,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No Ammo Required to Display Subscreen Items", NULL, NULL },
    { jwin_check_proc,      10, 33+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Triforce in Cellar Warps Link Out", NULL, NULL },
    { jwin_check_proc,      10, 33+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Reduced Flashing (Epilepsy Protection)", NULL, NULL },
    { jwin_check_proc,      10, 33+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "No L/R Inventory Shifting", NULL, NULL },
    { jwin_check_proc,      10, 33+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Ex3 and Ex4 Shift A-Button Items", NULL, NULL },
    { jwin_check_proc,      10, 33+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Disable Fast Mode (Uncap)", NULL, NULL },
    { jwin_check_proc,      10, 33+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0, (void *) "Allow permanent secrets on Dungeon-type dmaps", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,       NULL, NULL, NULL }
};

static int miscrules[] =
{
    qr_SELECTAWPN, qr_MORESOUNDS, qr_FASTFILL, qr_QUARTERHEART, qr_ALLOWFASTMSG, qr_ALLOWMSGBYPASS,
    qr_MSGFREEZE, qr_MSGDISAPPEAR, qr_TIME, qr_HEARTRINGFIX, qr_NOHEARTRING, qr_NOSAVE,
    qr_NOCONTINUE, qr_NOGUYFIRES, qr_NOGUYPOOF, qr_LOG, qr_SCRIPTERRLOG, qr_SHOPCHEAT, 
	qr_NOGANONINTRO,qr_NEVERDISABLEAMMOONSUBSCREEN, qr_SIDEVIEWTRIFORCECELLAR,
	qr_EPILEPSY, qr_NO_L_R_BUTTON_INVENTORY_SWAP, qr_USE_EX1_EX2_INVENTORYSWAP,
	qr_NOFASTMODE, qr_DUNGEON_DMAPS_PERM_SECRETS,

	-1
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

static int compatrules1_list[] =
{
	8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, -1
};

static int compatrules2_list[] =
{
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, -1
};

static TABPANEL compatrules_tabs[] =
{
    // (text)
    { (char *)" 1 ",     D_SELECTED,  compatrules1_list, 0, NULL },
    { (char *)" 2 ",     0,           compatrules2_list, 0, NULL },
    { NULL,              0,           NULL,             0, NULL }
};

static int compatrules[] = 
{
   qr_GOTOLESSNOTEQUAL, qr_OLDLENSORDER, qr_NOFAIRYGUYFIRES, qr_TRIGGERSREPEAT,
   qr_HOOKSHOTDOWNBUG, qr_REPLACEOPENDOORS, qr_NOSOLIDDAMAGECOMBOS, qr_OLDHOOKSHOTGRAB,
   qr_PEAHATCLOCKVULN, qr_OFFSCREENWEAPONS, qr_ITEMPICKUPSETSBELOW, qr_OLDSIDEVIEWSPIKES,
   qr_OLDINFMAGIC, qr_BITMAPOFFSETFIX, qr_OLDSPRITEDRAWS, qr_OLD_F6, qr_BROKEN_ASKIP_Y_FRAMES,
   qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY, qr_OLD_SIDEVIEW_CEILING_COLLISON, qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES,
   qr_OLD_ENEMY_KNOCKBACK_COLLISION, qr_WEAPONSMOVEOFFSCREEN, qr_CHECKSCRIPTWEAPONOFFSCREENCLIP,
   qr_SHORTDGNWALK,qr_OLD_STRING_EDITOR_MARGINS,qr_STRING_FRAME_OLD_WIDTH_HEIGHT,qr_IDIOTICSHASHNEXTSECRETBUGSUPPORT,
	-1 
};

static DIALOG compatrules_dlg[] =
{
    /* (dialog proc)       (x)    (y)   (w)   (h)     (fg)      (bg)     (key)      (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,         0,    0,   300,  235,    vc(14),   vc(1),      0,      D_EXIT,     0,             0,        (void *) "Quest Rules - Compatibility", NULL, NULL },
    { d_timer_proc,          0,    0,     0,    0,    0,        0,          0,      0,          0,             0,        NULL, NULL, NULL },
    { jwin_tab_proc,         5, 13+30,  290,  161,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) compatrules_tabs, NULL, (void *)compatrules_dlg },
    // 3
    { jwin_button_proc,    170,  210,    61,   21,    vc(14),   vc(1),     27,      D_EXIT,     0,             0,        (void *) "Cancel", NULL, NULL },
    { jwin_button_proc,     90,  210,    61,   21,    vc(14),   vc(1),     13,      D_EXIT,     0,             0,        (void *) "OK", NULL, NULL },
    { d_keyboard_proc,       0,    0,     0,    0,         0,       0,      0,      0,          KEY_F1,        0,        (void *) onHelp, NULL, NULL },
    
    // Notice
    { jwin_text_proc,       10, 13+10,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "These rules are enabled automatically for compatibility.", NULL, NULL },
    { jwin_text_proc,       10, 13+20,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Enabling them manually may cause unexpected behavior.", NULL, NULL },
    
    // rules 8
    { jwin_check_proc,      10, 13+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old GOTOLESS Behavior", NULL, NULL },
    { jwin_check_proc,      10, 13+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old Lens Drawing Order", NULL, NULL },
    // 10
	{ jwin_check_proc,      10, 13+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "No Fairy Guy Fires", NULL, NULL },
    { jwin_check_proc,      10, 13+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Continuous Step Triggers", NULL, NULL },
    { jwin_check_proc,      10, 13+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Downward Hookshot Bug", NULL, NULL },
    { jwin_check_proc,      10, 13+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Fix Open Door Solidity", NULL, NULL },
    { jwin_check_proc,      10, 13+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "No Solid Damage Combos", NULL, NULL },
    // 15
	{ jwin_check_proc,      10, 13+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old Hookshot Grab Checking", NULL, NULL },
    { jwin_check_proc,      10, 13+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Peahats Are Vulnerable When Frozen By Clocks", NULL, NULL },
    { jwin_check_proc,      10, 13+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Weapons With No Collision Detection Move Offscreen", NULL, NULL },
    { jwin_check_proc,      10, 13+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Screen Item Pickup Disables Hunger/Special Item", NULL, NULL },
    { jwin_check_proc,      10, 13+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Sideview Spike Detection Prior to 2.50.1RC3", NULL, NULL },
    // 20
	{ jwin_check_proc,      10, 13+170, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Infinite Magic Prevents Items From Draining Rupees", NULL, NULL },
    { jwin_check_proc,      10, 13+180, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Use DrawBitmap() Offsets Prior to 2.50.2", NULL, NULL },
    { jwin_check_proc,      10, 13+50,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old (Faster) Sprite Drawing", NULL, NULL },
	{ jwin_check_proc,      10, 13+60,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old (Instant) F6 Menu", NULL, NULL },
	{ jwin_check_proc,      10, 13+70,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Broken ASkipY Animation Frames", NULL, NULL },
	// 25
	{ jwin_check_proc,      10, 13+80,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Enemies Clip Through Top-Half Partial Solids", NULL, NULL },
	{ jwin_check_proc,      10, 13+90,  185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old Sideview Ceiling Collision", NULL, NULL },
	{ jwin_check_proc,      10, 13+100, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Items set to 0 AFrames ignore AFrame changes", NULL, NULL },
	{ jwin_check_proc,      10, 13+110, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old Enemy Knockback Collision", NULL, NULL },
	{ jwin_check_proc,      10, 13+120, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Weapons Can Go Out of Bounds (Offscreen)", NULL, NULL },
	// 30
	{ jwin_check_proc,      10, 13+130, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Check Clip for Custom / Script Type Weapons on Screen Edge", NULL, NULL },
	{ jwin_check_proc,      10, 13+140, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "1.92 Diagonal Walk Speed", NULL, NULL },
	{ jwin_check_proc,      10, 13+150, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old String Margins", NULL, NULL },
	{ jwin_check_proc,      10, 13+160, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Old String Frame Width/Height", NULL, NULL },
	{ jwin_check_proc,      10, 13+170, 185,    9,    vc(14),   vc(1),      0,      0,          1,             0,        (void *) "Bugged ->Next Combos", NULL, NULL },
    { NULL,                  0,    0,     0,    0,    0,        0,          0,      0,          0,             0,        NULL, NULL, NULL }
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

