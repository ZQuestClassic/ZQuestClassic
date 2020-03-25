//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_custom.cc
//
//  Custom item, enemy, etc. for ZQuest.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <map>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "zc_custom.h"
#include "zq_custom.h"
#include "tiles.h"
#include "zq_tiles.h"
#include "zq_misc.h"
#include "zquest.h"
#include "items.h"
#include "zsys.h"
#include "sfx.h"
#include "init.h"
#include "ffasm.h"
#include "defdata.h"
#include "zc_malloc.h"

extern int ex;
extern void reset_itembuf(itemdata *item, int id);
extern const char *item_class_list(int index, int *list_size);
extern const char *sfxlist(int index, int *list_size);
// zq_subscr.cpp
extern int d_qtile_proc(int msg,DIALOG *d,int c);
extern int d_ctl_proc(int msg,DIALOG *d,int c);
extern int d_csl_proc(int msg,DIALOG *d,int c);
extern int d_csl2_proc(int msg,DIALOG *d,int c);
extern int d_tileblock_proc(int msg,DIALOG *d,int c);
extern int d_stilelist_proc(int msg,DIALOG *d,int c);
extern int sstype_drop_proc(int msg,DIALOG *d,int c);
extern int jwin_fontdrop_proc(int msg,DIALOG *d,int c);
extern int jwin_tflpcheck_proc(int msg,DIALOG *d,int c);
extern int jwin_lscheck_proc(int msg,DIALOG *d,int c);

extern int biw_cnt;
extern int biic_cnt;



#ifdef _MSC_VER
#define stricmp _stricmp
#endif

extern word quest_header_zelda_version;
extern word quest_header_zelda_build;

int link_animation_speed = 1; //lower is faster animation

int d_ecstile_proc(int msg,DIALOG *d,int c);

int d_cstile_proc(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    switch(msg)
    {
    case MSG_CLICK:
    {
        int f = 0;
        int t = d->d1;
        int cs = d->d2;
        
        if(select_tile(t,f,1,cs,true))
        {
            d->d1 = t;
            d->d2 = cs;
            return D_REDRAW;
        }
    }
    break;
    
    case MSG_DRAW:
        if(is_large)
        {
            d->w = 36;
            d->h = 36;
        }
        
        BITMAP *buf = create_bitmap_ex(8,20,20);
        BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
        
        if(buf && bigbmp)
        {
            clear_bitmap(buf);
            
            if(d->d1)
                overtile16(buf,d->d1,2,2,d->d2,0);
                
            stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->w-2, d->h-2);
            destroy_bitmap(buf);
            jwin_draw_frame(bigbmp,0,0,d->w,d->h,FR_DEEP);
            blit(bigbmp,screen,0,0,d->x,d->y,d->w,d->h);
            destroy_bitmap(bigbmp);
        }
        
        //    text_mode(d->bg);
        FONT *fonty = is_large ? font : pfont;
        textprintf_ex(screen,fonty,d->x+d->w,d->y+2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Tile: %d",d->d1);
        textprintf_ex(screen,fonty,d->x+d->w,d->y+text_height(fonty)+3,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet: %d",d->d2);
        break;
    }
    
    return D_O_K;
}

void large_dialog(DIALOG *d)
{
    large_dialog(d, 1.5f);
}

void large_dialog(DIALOG *d, float RESIZE_AMT)
{
    if(d[0].d1 == 0)
    {
        d[0].d1 = 1;
        int oldwidth = d[0].w;
        int oldheight = d[0].h;
        int oldx = d[0].x;
        int oldy = d[0].y;
        d[0].x -= int(float(d[0].w)/RESIZE_AMT);
        d[0].y -= int(float(d[0].h)/RESIZE_AMT);
        d[0].w = int(float(d[0].w)*RESIZE_AMT);
        d[0].h = int(float(d[0].h)*RESIZE_AMT);
        
        for(int i=1; d[i].proc!=NULL; i++)
        {
            // Place elements horizontally
            double xpc = ((double)(d[i].x - oldx) / (double)oldwidth);
            d[i].x = int(d[0].x + (xpc*double(d[0].w)));
            
            // Horizontally resize elements
            if((d[i].proc == d_maptile_proc && d[i].dp2!=(void*)1) || d[i].proc==d_intro_edit_proc || d[i].proc==d_title_edit_proc)
            {
                d[i].x += (int)(float(d[i].w)/4.f);
            }
            else if(d[i].proc == d_comboframe_proc)
            {
                d[i].w *= 2;
                d[i].w -= 4;
            }
            else if(d[i].proc == d_wflag_proc || d[i].proc==d_bitmap_proc || d[i].proc == d_maptile_proc || d[i].proc==d_qtile_proc ||  d[i].proc==d_tileblock_proc)
            {
                d[i].w *= 2;
            }
            else if(d[i].proc == jwin_button_proc)
                d[i].w = int(d[i].w*1.5);
            else d[i].w = int(float(d[i].w)*RESIZE_AMT);
            
            // Place elements vertically
            double ypc = ((double)(d[i].y - oldy) / (double)oldheight);
            d[i].y = int(d[0].y + (ypc*double(d[0].h)));
            
            // Vertically resize elements
            if((d[i].proc == d_maptile_proc && d[i].dp2!=(void*)1) || d[i].proc==d_intro_edit_proc || d[i].proc==d_title_edit_proc)
            {
            }
            else if(d[i].proc == jwin_edit_proc || d[i].proc == jwin_check_proc || d[i].proc == jwin_checkfont_proc || d[i].proc == jwin_tflpcheck_proc || d[i].proc == jwin_lscheck_proc)
            {
                d[i].h = int((double)d[i].h*1.5);
            }
            else if(d[i].proc == jwin_droplist_proc || d[i].proc == d_ndroplist_proc || d[i].proc == d_idroplist_proc || d[i].proc == d_nidroplist_proc || d[i].proc == d_dropdmaplist_proc
                    || d[i].proc == d_dropdmaptypelist_proc || d[i].proc == jwin_as_droplist_proc  || d[i].proc == d_ffcombolist_proc || d[i].proc == sstype_drop_proc || d[i].proc == d_ctl_proc
                    || d[i].proc == jwin_fontdrop_proc || d[i].proc == d_csl_proc || d[i].proc == d_csl2_proc || d[i].proc == d_stilelist_proc || d[i].proc == d_comboalist_proc)
            {
                d[i].y += int((double)d[i].h*0.25);
                d[i].h = int((double)d[i].h*1.25);
            }
            else if(d[i].proc == d_comboframe_proc)
            {
                d[i].h *= 2;
                d[i].h -= 4;
            }
            else if(d[i].proc == d_wflag_proc || d[i].proc==d_bitmap_proc || d[i].proc == d_maptile_proc || d[i].proc==d_qtile_proc || d[i].proc==d_tileblock_proc)
            {
                d[i].h *= 2;
            }
            else if(d[i].proc == jwin_button_proc)
                d[i].h = int(d[i].h*1.5);
            else d[i].h = int(float(d[i].h)*RESIZE_AMT);
            
            // Fix frames
            if(d[i].proc == jwin_frame_proc)
            {
                d[i].x++;
                d[i].y++;
                d[i].w-=4;
                d[i].h-=4;
            }
            
            // Fix menus
            if(d[i].proc == jwin_menu_proc)
            {
                d[i].y=d[0].y+23;
                d[i].h=13;
            }
        }
    }
    
    for(int i=1; d[i].proc != NULL; i++)
    {
        if(d[i].proc==jwin_slider_proc)
            continue;
            
        // Bigger font
        bool bigfontproc = (d[i].proc != jwin_initlist_proc && d[i].proc != jwin_droplist_proc && d[i].proc != jwin_abclist_proc && d[i].proc != d_ilist_proc && d[i].proc != d_wlist_proc && d[i].proc != jwin_list_proc && d[i].proc != d_dmaplist_proc
                            && d[i].proc != d_dropdmaplist_proc && d[i].proc != d_xmaplist_proc && d[i].proc != d_dropdmaptypelist_proc && d[i].proc != d_warplist_proc && d[i].proc != d_warplist_proc && d[i].proc != d_wclist_proc && d[i].proc != d_ndroplist_proc
                            && d[i].proc != d_idroplist_proc && d[i].proc != d_nidroplist_proc && d[i].proc != jwin_as_droplist_proc && d[i].proc != d_ffcombolist_proc && d[i].proc != d_enelist_proc && d[i].proc != sstype_drop_proc && d[i].proc !=  d_ctl_proc
                            && d[i].proc != jwin_fontdrop_proc && d[i].proc != d_csl_proc && d[i].proc != d_csl2_proc && d[i].proc != d_stilelist_proc && d[i].proc != d_comboalist_proc);
                            
        if(bigfontproc && !d[i].dp2)
        {
            //d[i].dp2 = (d[i].proc == jwin_edit_proc) ? sfont3 : lfont_l;
            d[i].dp2 = lfont_l;
        }
        else if(!bigfontproc)
        {
//      ((ListData *)d[i].dp)->font = &sfont3;
            ((ListData *) d[i].dp)->font = &lfont_l;
        }
        
        // Make checkboxes work
        if(d[i].proc == jwin_check_proc)
            d[i].proc = jwin_checkfont_proc;
        else if(d[i].proc == jwin_radio_proc)
            d[i].proc = jwin_radiofont_proc;
    }
    
    jwin_center_dialog(d);
}

/*****************************/
/******  onCustomItems  ******/
/*****************************/

struct ItemNameInfo
{
    int iclass;
    char *power;
    char *misc1;
    char *misc2;
    char *misc3;
    char *misc4;
    char *misc5;
    char *misc6;
    char *misc7;
    char *misc8;
    char *misc9;
    char *misc10;
    char *flag1;
    char *flag2;
    char *flag3;
    char *flag4;
    char *flag5;
    char *wpn1;
    char *wpn2;
    char *wpn3;
    char *wpn4;
    char *wpn5;
    char *wpn6;
    char *wpn7;
    char *wpn8;
    char *wpn9;
    char *wpn10;
    char *actionsnd;
};


static ItemNameInfo inameinf[]=
{
    //itemclass                          power                                      misc1                                        misc2                                      misc3                              misc4                              misc5                              misc6                              misc7                              misc8                              misc9                              misc10                             flag1                                      flag2                              flag3                              flag4                              flag5                              wpn1                               wpn2                                       wpn3                                       wpn4                                       wpn5                                       wpn6                                       wpn7                                       wpn8                                       wpn9                                       wpn10                                      action sound
    { itype_fairy,                       NULL, (char *)"HP Regained:", (char *)"MP Regained:", (char *)"Step Speed:",             NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"HP. R. Is Percent", (char *)"MP. R. Is Percent",       NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Item Drop Sound:"                 },
    { itype_triforcepiece,               NULL, (char *)"Cutscene MIDI:", (char *)"Cutscene Type (0-1):",            NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Side Warp Out",                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_shield,                      NULL, (char *)"Block Flags:", (char *)"Reflect Flags:",                  NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"De/Reflection Sound:"             },
    { itype_agony, (char *)"Sensitivity:", (char *)"Vibration Frequency",               NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_wealthmedal,                 NULL, (char *)"Discount Amount:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"D. A. Is Percent",                NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_cbyrna, (char *)"Beam Damage:", (char *)"Beam Speed Divisor:", (char *)"Orbit Radius:", (char *)"Number Of Beams:",        NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Penetrates Enemies",              NULL,                              NULL, (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Beam Sprite:", (char *)"Sparkle Sprite:", (char *)"Damaging Sparkle Sprite:",        NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Orbit Sound:"                     },
    { itype_whispring, (char *)"Jinx Divisor:", (char *)"Jinx Type:",                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Perm. Jinx Are Temp.",            NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_ladder, (char *)"Four-Way:",                       NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_magickey, (char *)"Dungeon Level:",			      NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Lesser D. Levels Also",           NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_wallet, (char *)"Infinite Supply:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_quiver, (char *)"Infinite Supply:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_bombbag, (char *)"Infinite Supply:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Super Bombs Also",                NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_quakescroll2, (char *)"Damage Multiplier:", (char *)"Stun Duration:", (char *)"Stun Radius:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Quake Sound:"                     },
    { itype_quakescroll, (char *)"Damage Multiplier:", (char *)"Stun Duration:", (char *)"Stun Radius:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Quake Sound:"                     },
    { itype_perilscroll,                 NULL, (char *)"Maximum Hearts:",                   NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_spinscroll2, (char *)"Damage Multiplier:", (char *)"Number of Spins:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Spinning Sound:"                  },
    { itype_spinscroll, (char *)"Damage Multiplier:", (char *)"Number of Spins:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Spinning Sound:"                  },
    { itype_clock,                       NULL, (char *)"Duration (0 = Infinite):",          NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_magicring, (char *)"Infinite Magic:", (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_heartring,                   NULL, (char *)"Increase Amount:", (char *)"Delay Duration:",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    //itemclass                          power                                      misc1                                        misc2                                      misc3                              misc4                              misc5                              misc6                              misc7                              misc8                              misc9                              misc10                             flag1                                      flag2                              flag3                              flag4                              flag5                              wpn1                               wpn2                                       wpn3                                       wpn4                                       wpn5                                       wpn6                                       wpn7                                       wpn8                                       wpn9                                       wpn10                                      action sound
    { itype_chargering,                  NULL, (char *)"Charging Duration:", (char *)"Magic C. Duration:",              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_hoverboots,                  NULL, (char *)"Hover Duration:",                   NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Halo Sprite:",            NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Hovering Sound:"                  },
    { itype_rocs, (char *)"Height Multiplier:",              NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Jumping Sound:"                   },
    { itype_sbomb, (char *)"Damage:", (char *)"Fuse Duration (0 = Remote):", (char *)"Max. On Screen:", (char *)"Damage to Link:",	     NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Bomb Sprite:", (char *)"Explosion Sprite:",               NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Explosion Sound:"                 },
    { itype_bomb, (char *)"Damage:", (char *)"Fuse Duration (0 = Remote):", (char *)"Max. On Screen:", (char *)"Damage to Link:",	     NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Use 1.92 Timing",                 NULL,                              NULL,                              NULL,                              NULL, (char *)"Bomb Sprite:", (char *)"Explosion Sprite:",               NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Explosion Sound:"                 },
    { itype_nayruslove,                  NULL, (char *)"Duration:",                         NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Rocket Flickers", (char *)"Translucent Rocket", (char *)"Translucent Shield", (char *)"Shield Flickers",         NULL, (char *)"Left Rocket Sprite:", (char *)"L. Rocket Return Sprite:", (char *)"L. Rocket Sparkle Sprite:", (char *)"L. Return Sparkle Sprite:", (char *)"Shield Sprite (2x2, Over):", (char *)"Right Rocket Sprite:", (char *)"R. Rocket Return Sprite:", (char *)"R. Rocket Sparkle Sprite:", (char *)"R. Return Sparkle Sprite:", (char *)"Shield Sprite (2x2, Under):", (char *)"Shield Sound:"                    },
    { itype_faroreswind,                 NULL, (char *)"Warp Animation (0-2):",             NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Wind Sound:"                      },
    { itype_dinsfire, (char *)"Damage:", (char *)"Number of Flames:", (char *)"Circle Width:",                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Don't Provide Light", (char *)"Falls in Sideview",       NULL,                              NULL, (char *)"Rocket Up Sprite:", (char *)"Rocket Down Sprite:", (char *)"R. Up Sparkle Sprite:", (char *)"R. Down Sparkle Sprite:", (char *)"Flame Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Ring Sound:"                      },
    { itype_hammer, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Hammer Sprite:", (char *)"Smack Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Pound Sound:"                     },
    { itype_lens,                        NULL, (char *)"Lens Width:",                       NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Activation Sound:"                },
    { itype_hookshot, (char *)"Damage:", (char *)"Chain Length:", (char *)"Chain Links:", (char *)"Block Flags:", (char *)"Reflect Flags:",          NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"No Handle Damage",                 NULL,                              NULL,                              NULL,                              NULL, (char *)"Tip Sprite:", (char *)"Chain Sprite (H):", (char *)"Chain Sprite (V):", (char *)"Handle Sprite:",                  NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_boots, (char *)"Damage Combo Level:",             NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Not Solid Combos",                NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_bracelet, (char *)"Push Combo Level:",               NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Once Per Screen",                 NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_book, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Fire Magic",                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Magic Sprite:", (char *)"Flame Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_ring, (char *)"Damage Divisor:", (char *)"Link Sprite Pal:",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_wand, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Allow Magic w/o Book",            NULL,                              NULL, (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Magic Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_bait,                        NULL, (char *)"Duration:",                         NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL, (char *)"Bait Sprite:",            NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Baiting Sound:"                   },
    { itype_potion,                      NULL, (char *)"HP Regained:", (char *)"MP Regained:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"HP R. Is Percent", (char *)"MP R. Is Percent",        NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_whistle,                     NULL, (char *)"Whirlwind Direction:", (char *)"Warp Ring:",                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"One W.Wind Per Scr.",             NULL,                              NULL,                              NULL,                              NULL, (char *)"Whirlwind Sprite:",       NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Music Sound:"                     },
    { itype_candle, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Once Per Screen", (char *)"Don't Provide Light",     NULL, (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Flame Sprite:",                   NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_arrow, (char *)"Damage:", (char *)"Duration (0 = Infinite):",          NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Penetrate Enemies",               NULL,                              NULL,                              NULL,                              NULL, (char *)"Arrow Sprite:", (char *)"Sparkle Sprite:", (char *)"Damaging Sparkle Sprite:",        NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Firing Sound:"                    },
    { itype_brang, (char *)"Damage:", (char *)"Range (0 = Infinite):",             NULL, (char *)"Block Flags:", (char *)"Reflect Flags:",          NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"Corrected Animation", (char *)"Directional Sprites", (char *)"Do Not Return",           NULL,                              NULL, (char *)"Boomerang Sprite:", (char *)"Sparkle Sprite:", (char *)"Damaging Sparkle Sprite:",        NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Spinning Sound:"                  },
    { itype_sword, (char *)"Damage:", (char *)"Beam Hearts:", (char *)"Beam Damage:",                    NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL, (char *)"B.H. Is Percent", (char *)"B.D. Is Percent", (char *)"B. Penetrates Enemies", (char *)"Can Slash",               NULL, (char *)"Stab Sprite:", (char *)"Slash Sprite:", (char *)"Beam Sprite:",                    NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Stabbing/Slashing Sound:"         },
    { itype_whimsicalring, (char *)"Damage Bonus:", (char *)"Chance (1 in n):",                  NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL, (char *)"Whimsy Sound:"                    },
    { itype_perilring, (char *)"Damage Divisor:", (char *)"Maximum Hearts:",                   NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_stompboots, (char *)"Damage:",                         NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { itype_bow, (char *)"Arrow Speed:",                    NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       },
    { -1,                                NULL,                                      NULL,                                        NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                              NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL,                                      NULL                                       }
};

static std::map<int, ItemNameInfo *> *inamemap = NULL;

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

static int itemdata_flags_list[] =
{
    // dialog control number
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, -1
};

static int itemdata_gfx_list[] =
{
    // dialog control number
    57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, -1
};

static int itemdata_pickup_list[] =
{
    // dialog control number
    92, 93, 94, 95, 96, 97, 98, 99, 100, /*101, 102,*/ 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, -1
};

static int itemdata_script_list[] =
{
    // dialog control number
    /*131, 132,*/ 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, -1
};

static int itemdata_scriptargs_list[] =
{
    // dialog control number
    101, 102, 131, 132, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, -1
};

static TABPANEL itemdata_tabs[] =
{
    // (text)
    { (char *)"Data",         D_SELECTED,    itemdata_flags_list,         0, NULL },
    { (char *)"GFX",          0,             itemdata_gfx_list,           0, NULL },
    { (char *)"Pickup",       0,             itemdata_pickup_list,        0, NULL },
    { (char *)"Action",       0,             itemdata_script_list,        0, NULL },
    { (char *)"Scripts",      0,             itemdata_scriptargs_list,    0, NULL },
    { NULL,                   0,             NULL,                        0, NULL }
};

static ListData item_class__list(item_class_list, &pfont);
static ListData weapon_list(weaponlist, &pfont);

static char counterlist_str_buf[12];

const char *counterlist(int index, int *list_size)
{
    if(index >= 0)
    {
        bound(index,0,32);
        
        switch(index)
        {
        case 0:
            return "None";
            
        case 1:
            return "Life";
            
        case 2:
            return "Rupees";
            
        case 3:
            return "Bombs";
            
        case 4:
            return "Arrows";
            
        case 5:
            return "Magic";
            
        case 6:
            return "Keys";
            
        case 7:
            return "Super Bombs";
            
        default:
            sprintf(counterlist_str_buf,"Script %d",index-7);
            return counterlist_str_buf;
        }
    }
    
    *list_size = 33;
    return NULL;
}

static ListData counter_list(counterlist, &pfont);

int jwin_nbutton_proc(int msg, DIALOG *d, int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    d=d;
    
    FONT *tfont=font;
    
    switch(msg)
    {
    case MSG_DRAW:
        tfont=font;
        font=is_large?lfont_l:nfont;
        jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, (char*)d->dp, d->flags, true);
        font=tfont;
        return D_O_K;
        break;
    }
    
    return jwin_button_proc(msg, d, c);
}

const char *itemscriptdroplist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = biitems_cnt;
        return NULL;
    }
    
    return biitems[index].first.c_str();
}

//droplist like the dialog proc, naming scheme for this stuff is awful...
static ListData itemscript_list(itemscriptdroplist, &pfont);


static DIALOG itemdata_dlg[] =
{
    // (dialog proc)           (x)     (y)     (w)     (h)        (fg)                   (bg)                   (key)       (flags)     (d1)     (d2)    (dp)                                            (dp2)               (dp3)
    { jwin_win_proc,            0,      0,    320,    237,    vc(14),                 vc(1),                   0,       D_EXIT,      0,    0,  NULL,                                           NULL,   NULL                  },
    { d_timer_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_tab_proc,            4,     25,    312,    182,    0,                      0,                       0,       0,           0,    0, (void *) itemdata_tabs,                         NULL, (void *)itemdata_dlg  },
    { jwin_nbutton_proc,       58,    212,     61,     21,    vc(14),                 vc(1),                  13,       D_EXIT,      0,    0, (void *) "OK",                                  NULL,   NULL                  },
    { jwin_nbutton_proc,      129,    212,     61,     21,    vc(14),                 vc(1),                  27,       D_EXIT,      0,    0, (void *) "Cancel",                              NULL,   NULL                  },
    { jwin_nbutton_proc,      200,    212,     61,     21,    vc(14),                 vc(1),                   0,       D_EXIT,      0,    0, (void *) "Defaults",                            NULL,   NULL                  },
    
    // 6
    { jwin_text_proc,           8,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Name:",                               NULL,   NULL                  },
    { jwin_edit_proc,          55,     44,    150,     16,    vc(12),                 vc(1),                   0,       0,          63,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_text_proc,           8,     66,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Class:",                              NULL,   NULL                  },
    { jwin_droplist_proc,      55,     62,    150,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       D_EXIT,      3,    0, (void *) &item_class__list,                       NULL,   NULL                  },
    
    //10
    { jwin_text_proc,         236,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Level:",                              NULL,   NULL                  },
    { jwin_edit_proc,         283,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_text_proc,           8,     84,     60,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Power:",                              NULL,   NULL                  },
    { jwin_edit_proc,          87,     80,     28,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //14
    { jwin_check_proc,        120,     80,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Equipment Item",                      NULL,   NULL                  },
    { jwin_check_proc,        120,     90,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 1",                        NULL,   NULL                  },
    { jwin_check_proc,        120,    100,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 2",                        NULL,   NULL                  },
    { jwin_check_proc,        215,     80,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 3",                        NULL,   NULL                  },
    { jwin_check_proc,        215,     90,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 4",                        NULL,   NULL                  },
    { jwin_check_proc,        215,    100,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Misc. Flag 5",                        NULL,   NULL                  },
    
    //20
    { jwin_text_proc,           8,    115,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 1:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    111,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    133,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 2:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    129,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    151,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 3:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    147,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    169,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 4:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    165,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    187,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 5:",                  NULL,   NULL                  },
    { jwin_edit_proc,         107,    183,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //30
    { jwin_text_proc,         177,    115,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 6:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    111,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    133,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 7:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    129,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    151,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 8:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    147,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    169,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 9:",                  NULL,   NULL                  },
    { jwin_edit_proc,         276,    165,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         177,    187,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Misc. Attribute 10:",                 NULL,   NULL                  },
    { jwin_edit_proc,         276,    183,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //40
    { jwin_button_proc,        205,    65,     12,     12,   vc(14),					vc(1),					 0,      D_EXIT,       0,    0, (void *) "?",								     NULL,	 NULL				  },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //47
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //57
    { jwin_text_proc,           8,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Flash CSet:",                         NULL,   NULL                  },
    { jwin_edit_proc,         107,     44,     35,     16,    vc(12),                 vc(1),                   0,       0,           2,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     66,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Animation Frames:",                   NULL,   NULL                  },
    { jwin_edit_proc,         107,     62,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     84,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Animation Speed:",                    NULL,   NULL                  },
    { jwin_edit_proc,         107,     80,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    102,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Initial Delay:",                      NULL,   NULL                  },
    { jwin_edit_proc,         107,     98,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,    120,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Link Tile Modification:",             NULL,   NULL                  },
    { jwin_edit_proc,         107,    116,     35,     16,    vc(12),                 vc(1),                   0,       0,           6,    0,  NULL,                                           NULL,   NULL                  },
    
    //67
    { d_dummy_proc,          8,    136,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) NULL,       NULL,   NULL                  },
    
    { d_cstile_proc,          145,     44,     20,     20,    vc(11),                 vc(1),                   0,       0,           0,    6,  NULL,                                           NULL,   NULL                  },
    { jwin_check_proc,        145,     69,     65,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Flash ",                              NULL,   NULL                  },
    { jwin_check_proc,        145,     80,     65,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "2-Hand",                              NULL,   NULL                  },
    
    { jwin_button_proc,       149,    100,     53,     21,    vc(14),                 vc(1),                   't',     D_EXIT,      0,    0, (void *) "&Test",                               NULL,   NULL                  },
    
    //72
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //82
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //92
    { jwin_text_proc,           8,     66,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Increase Amount:",                    NULL,   NULL                  },
    { jwin_edit_proc,         107,     62,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_check_proc,        147,     66,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Gradual",                       NULL,   NULL                  },
    
    { jwin_text_proc,           8,     48,     96,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Counter Reference:",                  NULL,   NULL                  },
    { jwin_droplist_proc,     107,     44,     72,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &counter_list,						 NULL,   NULL 				   },
    { jwin_text_proc,         147,     84,     60,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "But Not Above:",                           NULL,   NULL                  },
    { jwin_edit_proc,         204,     80,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     84,     60,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Increase Counter Max:",                               NULL,   NULL                  },
    { jwin_edit_proc,         107,     80,     35,     16,    vc(12),                 vc(1),                   0,       0,           5,    0,  NULL,                                           NULL,   NULL                  },
    
    //101
    { jwin_text_proc,          112+10,    47+38,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Pickup Script:",                            NULL,   NULL                  },
    { jwin_droplist_proc,      112+10,    47+38+10,     150,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &itemscript_list,                   NULL,   NULL 				   },
    //{ jwin_edit_proc,          55,     98,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,          8,    102,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sound:",                              NULL,   NULL                  },
    { jwin_edit_proc,         107,     98,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_text_proc,           8,    120,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Hearts Required:",                    NULL,   NULL                  },
    { jwin_edit_proc,         107,    116,     35,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_check_proc,          8,    142,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Keep Lower Level Items",              NULL,   NULL                  },
    { jwin_check_proc,          8,    152,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Gain All Lower Level Items",          NULL,   NULL                  },
    { jwin_check_proc,          8,    162,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Can Be Eaten By Enemies",             NULL,   NULL                  },
    { jwin_check_proc,          8,    172,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Upgrade Item When Collected Twice",   NULL,   NULL                  },
    
    //111
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //121
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //131
    { jwin_text_proc,           112+10,  47+38+10 + 18,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Action Script:",                      NULL,   NULL                  },
    { jwin_droplist_proc,       112+10,  47+38+10*2 + 18,     150,      16, jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],           0,       0,           1,    0, (void *) &itemscript_list,                   NULL,   NULL 				   },
    //{ jwin_edit_proc,          55,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,           8,     48,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Magic Cost:",                              NULL,   NULL                  },
    { jwin_edit_proc,         125,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    { jwin_text_proc,         161,     48,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Sound:",                              NULL,   NULL                  },
    { jwin_edit_proc,         284,     44,     28,     16,    vc(12),                 vc(1),                   0,       0,           3,    0,  NULL,                                           NULL,   NULL                  },
    
    { jwin_check_proc,          8,     62,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Remove Item When Used",               NULL,   NULL                  },
    { jwin_check_proc,        161,     62,     60,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Uses Rupees Instead Of Magic",        NULL,   NULL                  },
    
    //139
    { jwin_text_proc,           8,     74,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 1:",                           NULL,   NULL                  },
    { jwin_droplist_proc,       8,     83,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,           8,    100,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 2:",                           NULL,   NULL                  },
    { jwin_droplist_proc,       8,    109,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,           8,    126,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 3:",                           NULL,   NULL                  },
    { jwin_droplist_proc,       8,    135,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,           8,    152,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 4:",                           NULL,   NULL                  },
    { jwin_droplist_proc,       8,    161,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,           8,    178,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 5:",                           NULL,   NULL                  },
    { jwin_droplist_proc,       8,    187,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    
    //149
    { jwin_text_proc,         161,     74,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 6:",                           NULL,   NULL                  },
    { jwin_droplist_proc,     161,     83,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,         161,    100,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 7:",                           NULL,   NULL                  },
    { jwin_droplist_proc,     161,    109,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,         161,    126,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 8:",                           NULL,   NULL                  },
    { jwin_droplist_proc,     161,    135,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,         161,    152,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 9:",                           NULL,   NULL                  },
    { jwin_droplist_proc,     161,    161,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    { jwin_text_proc,         161,    178,     35,      8,    vc(14),                 vc(1),                   0,       0,           0,    0, (void *) "Weapon 10:",                          NULL,   NULL                  },
    { jwin_droplist_proc,     161,    187,    151,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,       0,           0,    0, (void *) &weapon_list,                            NULL,   NULL                  },
    
    //159
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //169
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    { d_dummy_proc,             0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                 },
    
    //179
    { jwin_text_proc,       6+10,   29+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D0:", NULL, NULL },
    { jwin_text_proc,       6+10,   47+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D1:", NULL, NULL },
    { jwin_text_proc,       6+10,   65+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D2:", NULL, NULL },
    { jwin_text_proc,       6+10,   83+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D3:", NULL, NULL },
    { jwin_text_proc,       6+10,  101+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D4:", NULL, NULL },
    { jwin_text_proc,       6+10,  119+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D5:", NULL, NULL },
    { jwin_text_proc,       6+10,  137+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "D6:", NULL, NULL },
    { jwin_text_proc,       6+10,  155+20,   24,    12,   0,        0,       0,       0,          0,             0, (void *) "D7:", NULL, NULL },
    { jwin_edit_proc,      34+10,   25+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   43+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   61+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   79+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,   97+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  115+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  133+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      34+10,  151+20,   72,    16,   vc(12),   vc(1),   0,       0,          12,             0,       NULL, NULL, NULL },
    
    { jwin_text_proc,       112+10,  29+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "A1:", NULL, NULL },
    { jwin_text_proc,       112+10,  47+20,   24,    36,   0,        0,       0,       0,          0,             0, (void *) "A2:", NULL, NULL },
    //74
    { jwin_edit_proc,      140+10,  25+20,   32,    16,   vc(12),   vc(1),   0,       0,          2,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,      140+10,  43+20,   32,    16,   vc(12),   vc(1),   0,       0,          2,             0,       NULL, NULL, NULL },
    
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,          0, (void *) onSnapshot, NULL, NULL },
    { NULL,                     0,      0,      0,      0,    0,                      0,                       0,       0,           0,    0,  NULL,                                           NULL,   NULL                  },
};

void setLabels(int iclass)
{
    std::map<int, ItemNameInfo *> *nmap = getItemNameMap();
    std::map<int, ItemNameInfo *>::iterator it = nmap->find(iclass);
    ItemNameInfo *inf = NULL;
    
    if(it != nmap->end())
        inf = it->second;
        
        
    if(inf == NULL)
        itemdata_dlg[12].dp = (void *)"Power:";
    else if(inf->power == NULL)
    {
        itemdata_dlg[12].dp = (void *)"<Unused>";
        itemdata_dlg[13].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[12].dp = inf->power;
        itemdata_dlg[13].flags &= ~D_DISABLED;
    }
    
    // Disable the Equipment item checkbox
    if((iclass >= itype_rupee && iclass <= itype_bombammo) ||
            (iclass >= itype_key && iclass <= itype_magiccontainer) ||
            (iclass >= itype_map && iclass <= itype_bosskey) || iclass == itype_clock ||
            iclass==itype_lkey || iclass==itype_misc ||
            iclass==itype_bowandarrow || iclass==itype_letterpotion)
    {
        itemdata_dlg[14].dp = (void *)"<Unused>";
        itemdata_dlg[14].flags |= D_DISABLED;
        itemdata_dlg[14].flags &= ~D_SELECTED;
    }
    else
    {
        itemdata_dlg[14].dp = (void *)"Equipment Item";
        itemdata_dlg[14].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag1 == NULL))
    {
        itemdata_dlg[15].dp = (void *)"<Unused>";
        itemdata_dlg[15].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[15].dp = inf->flag1;
        itemdata_dlg[15].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag2 == NULL))
    {
        itemdata_dlg[16].dp = (void *)"<Unused>";
        itemdata_dlg[16].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[16].dp = inf->flag2;
        itemdata_dlg[16].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag3 == NULL))
    {
        itemdata_dlg[17].dp = (void *)"<Unused>";
        itemdata_dlg[17].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[17].dp = inf->flag3;
        itemdata_dlg[17].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag4 == NULL))
    {
        itemdata_dlg[18].dp = (void *)"<Unused>";
        itemdata_dlg[18].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[18].dp = inf->flag4;
        itemdata_dlg[18].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag5 == NULL))
    {
        itemdata_dlg[19].dp = (void *)"<Unused>";
        itemdata_dlg[19].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[19].dp = inf->flag5;
        itemdata_dlg[19].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc1 == NULL))
    {
        itemdata_dlg[20].dp = (void *)"<Unused>";
        itemdata_dlg[21].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[20].dp = inf->misc1;
        itemdata_dlg[21].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc2 == NULL))
    {
        itemdata_dlg[22].dp = (void *)"<Unused>";
        itemdata_dlg[23].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[22].dp = inf->misc2;
        itemdata_dlg[23].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc3 == NULL))
    {
        itemdata_dlg[24].dp = (void *)"<Unused>";
        itemdata_dlg[25].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[24].dp = inf->misc3;
        itemdata_dlg[25].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc4 == NULL))
    {
        itemdata_dlg[26].dp = (void *)"<Unused>";
        itemdata_dlg[27].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[26].dp = inf->misc4;
        itemdata_dlg[27].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc5 == NULL))
    {
        itemdata_dlg[28].dp = (void *)"<Unused>";
        itemdata_dlg[29].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[28].dp = inf->misc5;
        itemdata_dlg[29].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc6 == NULL))
    {
        itemdata_dlg[30].dp = (void *)"<Unused>";
        itemdata_dlg[31].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[30].dp = inf->misc6;
        itemdata_dlg[31].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc7 == NULL))
    {
        itemdata_dlg[32].dp = (void *)"<Unused>";
        itemdata_dlg[33].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[32].dp = inf->misc7;
        itemdata_dlg[33].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc8 == NULL))
    {
        itemdata_dlg[34].dp = (void *)"<Unused>";
        itemdata_dlg[35].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[34].dp = inf->misc8;
        itemdata_dlg[35].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc9 == NULL))
    {
        itemdata_dlg[36].dp = (void *)"<Unused>";
        itemdata_dlg[37].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[36].dp = inf->misc9;
        itemdata_dlg[37].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->misc10 == NULL))
    {
        itemdata_dlg[38].dp = (void *)"<Unused>";
        itemdata_dlg[39].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[38].dp = inf->misc10;
        itemdata_dlg[39].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->actionsnd == NULL))
        itemdata_dlg[135].dp = (void *)"<Unused>";
    else
        itemdata_dlg[135].dp = inf->actionsnd;
        
    if((inf == NULL) || (inf->wpn1 == NULL))
    {
        itemdata_dlg[139].dp = (void *)"<Unused>";
        itemdata_dlg[140].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[139].dp = inf->wpn1;
        itemdata_dlg[140].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn2 == NULL))
    {
        itemdata_dlg[141].dp = (void *)"<Unused>";
        itemdata_dlg[142].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[141].dp = inf->wpn2;
        itemdata_dlg[142].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn3 == NULL))
    {
        itemdata_dlg[143].dp = (void *)"<Unused>";
        itemdata_dlg[144].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[143].dp = inf->wpn3;
        itemdata_dlg[144].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn4 == NULL))
    {
        itemdata_dlg[145].dp = (void *)"<Unused>";
        itemdata_dlg[146].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[145].dp = inf->wpn4;
        itemdata_dlg[146].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn5 == NULL))
    {
        itemdata_dlg[147].dp = (void *)"<Unused>";
        itemdata_dlg[148].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[147].dp = inf->wpn5;
        itemdata_dlg[148].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn6 == NULL))
    {
        itemdata_dlg[149].dp = (void *)"<Unused>";
        itemdata_dlg[150].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[149].dp = inf->wpn6;
        itemdata_dlg[150].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn7 == NULL))
    {
        itemdata_dlg[151].dp = (void *)"<Unused>";
        itemdata_dlg[152].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[151].dp = inf->wpn7;
        itemdata_dlg[152].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn8 == NULL))
    {
        itemdata_dlg[153].dp = (void *)"<Unused>";
        itemdata_dlg[154].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[153].dp = inf->wpn8;
        itemdata_dlg[154].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn9 == NULL))
    {
        itemdata_dlg[155].dp = (void *)"<Unused>";
        itemdata_dlg[156].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[155].dp = inf->wpn9;
        itemdata_dlg[156].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->wpn10 == NULL))
    {
        itemdata_dlg[157].dp = (void *)"<Unused>";
        itemdata_dlg[158].flags |= D_DISABLED;
    }
    else
    {
        itemdata_dlg[157].dp = inf->wpn10;
        itemdata_dlg[158].flags &= ~D_DISABLED;
    }
}

void itemdata_help(int id)
{
    if(id < 0 || id > itype_max) return;
    
    if(id >= itype_custom1 && id <= itype_custom20)
    {
        jwin_alert(itype_names[id],"This has no built-in effect, but can be given","special significance using ZScripts or ZASM.","","O&K",NULL,'k',0,lfont);
    }
    else if(id >= itype_last)
    {
        jwin_alert("Undefined Item Class","This type currently has no use. Do not","use it in your quests, as its purpose may change","in future versions of Zelda Classic.","O&K",NULL,'k',0,lfont);
    }
    else if(id<itype_custom1)
        jwin_alert(itype_names[id],itemclass_help_string[id*3],itemclass_help_string[1+(id*3)],itemclass_help_string[2+(id*3)],"O&K",NULL,'k',0,lfont);
    else // Beyond itype_custom20...
        jwin_alert(itype_names[id],itemclass_help_string[(id-20)*3],itemclass_help_string[1+((id-20)*3)],itemclass_help_string[2+((id-20)*3)],"O&K",NULL,'k',0,lfont);
}

void test_item(itemdata test, int x, int y)
{
    itemdata *hold = itemsbuf;
    itemsbuf = &test;
    BITMAP *buf = create_bitmap_ex(8,16,16);
    BITMAP *buf2 = create_bitmap_ex(8,64,64);
    
    item temp((fix)0,(fix)0,(fix)0,0,0,0);
    temp.yofs = 0;
    go();
    scare_mouse();
    itemdata_dlg[0].flags=0;
    jwin_win_proc(MSG_DRAW, itemdata_dlg, 0);
    itemdata_dlg[0].flags=D_EXIT;
    //  d_ctext_proc(MSG_DRAW, itemdata_dlg+1, 0);
    
    frame=0;
//  jwin_draw_frame(screen,itemdata_dlg[0].x+(itemdata_dlg[0].w/2)-32,itemdata_dlg[0].y+(itemdata_dlg[0].h/2)-32, is_large?68:20,is_large?68:20,FR_DEEP);
    jwin_draw_frame(screen, x, y, is_large?68:20,is_large?68:20,FR_DEEP);
    unscare_mouse();
    
    for(;;)
    {
    
        ++frame;
        clear_bitmap(buf);
        temp.animate(0);
        temp.draw(buf);
        custom_vsync();
        scare_mouse();
        
        if(is_large)
        {
            stretch_blit(buf,buf2,0,0,16,16,0,0,64,64);
//      blit(buf2,screen,0,0,64,64,itemdata_dlg[0].x+(itemdata_dlg[0].w/2)-30,itemdata_dlg[0].y+(itemdata_dlg[0].h/2)-30);
            blit(buf2,screen,0,0,x+2,y+2,64,64);
        }
        else
        {
            blit(buf,screen,0,0,x+2,y+2,16,16);
        }
        
        unscare_mouse();
        
        //if(zqwin_scale > 1)
        {
            //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        //else
        {
            //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
        
        if(keypressed())
        {
            readkey();
            break;
        }
        
        if(gui_mouse_b())
            break;
    }
    
    comeback();
    
    while(gui_mouse_b())
    {
        /* do nothing */
    }
    
    destroy_bitmap(buf);
    destroy_bitmap(buf2);
    itemsbuf = hold;
}


void edit_itemdata(int index)
{
    char frm[8], spd[8], fcs[8], dly[8], ltm[8];
    char /*cls[8],*/ cll[8], amt[8], fmx[8], max[8], asn[8];
    char snd[8], mgc[8], hrt[8], pow[8];
    char name[64], zname[64];
    char ms1[8], ms2[8], ms3[8], ms4[8], ms5[8], ms6[8], ms7[8], ms8[8], ms9[8], ms10[8];
    char itemnumstr[75];
    char da[10][13];
    
    sprintf(itemnumstr,"Item %d: %s", index, item_string[index]);
    sprintf(fcs,"%d",itemsbuf[index].csets>>4);
    sprintf(frm,"%d",itemsbuf[index].frames);
    sprintf(spd,"%d",itemsbuf[index].speed);
    sprintf(dly,"%d",itemsbuf[index].delay);
    sprintf(ltm,"%ld",itemsbuf[index].ltm);
    sprintf(cll,"%d",itemsbuf[index].fam_type);
    sprintf(amt,"%d",(itemsbuf[index].amount&0x4000)?(-(itemsbuf[index].amount&0x3FFF)):(itemsbuf[index].amount&0x3FFF));
    sprintf(fmx,"%d",itemsbuf[index].max);
    sprintf(max,"%d",itemsbuf[index].setmax);
    sprintf(snd,"%d",itemsbuf[index].playsound);
    sprintf(ms1,"%ld",itemsbuf[index].misc1);
    sprintf(ms2,"%ld",itemsbuf[index].misc2);
    sprintf(ms3,"%ld",itemsbuf[index].misc3);
    sprintf(ms4,"%ld",itemsbuf[index].misc4);
    sprintf(ms5,"%ld",itemsbuf[index].misc5);
    sprintf(ms6,"%ld",itemsbuf[index].misc6);
    sprintf(ms7,"%ld",itemsbuf[index].misc7);
    sprintf(ms8,"%ld",itemsbuf[index].misc8);
    sprintf(ms9,"%ld",itemsbuf[index].misc9);
    sprintf(ms10,"%ld",itemsbuf[index].misc10);
    sprintf(mgc,"%d",itemsbuf[index].magic);
    sprintf(hrt,"%d",itemsbuf[index].pickup_hearts);
    sprintf(pow,"%d",itemsbuf[index].power);
    sprintf(asn,"%d",itemsbuf[index].usesound);
    sprintf(name,"%s",item_string[index]);
    
    for(int j=0; j<8; j++)
        sprintf(da[j],"%.4f",itemsbuf[index].initiald[j]/10000.0);
        
    sprintf(da[8],"%d",itemsbuf[index].initiala[0]/10000);
    sprintf(da[9],"%d",itemsbuf[index].initiala[1]/10000);
    
    itemdata_dlg[0].dp = itemnumstr;
    itemdata_dlg[0].dp2 = lfont;
    
    if(biic_cnt==-1)
    {
        build_biic_list();
    }
    
    if(biw_cnt==-1)
    {
        build_biw_list();
    }
    
    itemdata_dlg[7].dp = name;
    
    for(int j=0; j<biic_cnt; j++)
    {
        if(biic[j].i == itemsbuf[index].family)
            itemdata_dlg[9].d1 = j;
    }
    
    itemdata_dlg[11].dp = cll;
    itemdata_dlg[13].dp = pow;
    itemdata_dlg[14].flags = (itemsbuf[index].flags&ITEM_GAMEDATA) ? D_SELECTED : 0;
    itemdata_dlg[15].flags = (itemsbuf[index].flags&ITEM_FLAG1) ? D_SELECTED : 0;
    itemdata_dlg[16].flags = (itemsbuf[index].flags&ITEM_FLAG2) ? D_SELECTED : 0;
    itemdata_dlg[17].flags = (itemsbuf[index].flags&ITEM_FLAG3) ? D_SELECTED : 0;
    itemdata_dlg[18].flags = (itemsbuf[index].flags&ITEM_FLAG4) ? D_SELECTED : 0;
    itemdata_dlg[19].flags = (itemsbuf[index].flags&ITEM_FLAG5) ? D_SELECTED : 0;
    itemdata_dlg[21].dp = ms1;
    itemdata_dlg[23].dp = ms2;
    itemdata_dlg[25].dp = ms3;
    itemdata_dlg[27].dp = ms4;
    itemdata_dlg[29].dp = ms5;
    itemdata_dlg[31].dp = ms6;
    itemdata_dlg[33].dp = ms7;
    itemdata_dlg[35].dp = ms8;
    itemdata_dlg[37].dp = ms9;
    itemdata_dlg[39].dp = ms10;
    
    itemdata_dlg[58].dp = fcs;
    itemdata_dlg[60].dp = frm;
    itemdata_dlg[62].dp = spd;
    itemdata_dlg[64].dp = dly;
    itemdata_dlg[66].dp = ltm;
    itemdata_dlg[68].d1 = itemsbuf[index].tile;
    itemdata_dlg[68].d2 = itemsbuf[index].csets&15;
    itemdata_dlg[69].flags = (itemsbuf[index].misc&1) ? D_SELECTED : 0;
    itemdata_dlg[70].flags = (itemsbuf[index].misc&2) ? D_SELECTED : 0;
    
    itemdata_dlg[93].dp = amt;
    itemdata_dlg[94].flags = (itemsbuf[index].amount & 0x8000)  ? D_SELECTED : 0;
    itemdata_dlg[96].d1 = itemsbuf[index].count+1;
    itemdata_dlg[98].dp = fmx;
    itemdata_dlg[100].dp = max;
    itemdata_dlg[104].dp = snd;
    itemdata_dlg[106].dp = hrt;
    
    itemdata_dlg[107].flags = (itemsbuf[index].flags&ITEM_KEEPOLD) ? D_SELECTED : 0;
    itemdata_dlg[108].flags = (itemsbuf[index].flags&ITEM_GAINOLD) ? D_SELECTED : 0;
    itemdata_dlg[109].flags = (itemsbuf[index].flags&ITEM_EDIBLE) ? D_SELECTED : 0;
    itemdata_dlg[110].flags = (itemsbuf[index].flags&ITEM_COMBINE) ? D_SELECTED : 0;
    
    itemdata_dlg[134].dp = mgc;
    itemdata_dlg[136].dp = asn;
    itemdata_dlg[137].flags = (itemsbuf[index].flags&ITEM_DOWNGRADE) ? D_SELECTED : 0;
    itemdata_dlg[138].flags = (itemsbuf[index].flags&ITEM_RUPEE_MAGIC) ? D_SELECTED : 0;
    
    for(int i=0; i<10; ++i)
    {
//    itemdata_dlg[140+(i*2)].dp3 = is_large ? sfont3 : pfont;
        itemdata_dlg[140+(i*2)].dp3 = is_large ? lfont_l : pfont;
    }
    
    for(int j=0; j<biw_cnt; j++)
    {
        if(biw[j].i == itemsbuf[index].wpn)
            itemdata_dlg[140].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn2)
            itemdata_dlg[142].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn3)
            itemdata_dlg[144].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn4)
            itemdata_dlg[146].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn5)
            itemdata_dlg[148].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn6)
            itemdata_dlg[150].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn7)
            itemdata_dlg[152].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn8)
            itemdata_dlg[154].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn9)
            itemdata_dlg[156].d1 = j;
            
        if(biw[j].i == itemsbuf[index].wpn10)
            itemdata_dlg[158].d1 = j;
    }
    
    for(int j=0; j<8; j++)
        itemdata_dlg[187+j].dp = da[j];
        
    itemdata_dlg[197].dp = da[8];
    itemdata_dlg[198].dp = da[9];
    
    build_biitems_list();
    int script = 0, pickupscript = 0;
    
    for(int j = 0; j < biitems_cnt; j++)
    {
        if(biitems[j].second == itemsbuf[index].script - 1)
            script = j;
            
        if(biitems[j].second == itemsbuf[index].collect_script - 1)
            pickupscript = j;
    }
    
    itemdata_dlg[102].d1 = pickupscript;
    itemdata_dlg[132].d1 = script;
    
    int ret;
    itemdata test;
    memset(&test, 0, sizeof(itemdata));
    test.playsound = 25;
    
    setLabels(itemsbuf[index].family);
    FONT *tfont=font;
    font=pfont;
    
    if(is_large)
        large_dialog(itemdata_dlg);
        
    do
    {
        ret = zc_popup_dialog(itemdata_dlg,3);
        
        test.misc  = 0;
        test.flags = 0;
        
        test.fam_type = vbound(atoi(cll), 1, 255);
        test.count = itemdata_dlg[96].d1-1;
        test.amount = atoi(amt)<0?-(vbound(atoi(amt), -0x3FFF, 0))|0x4000:vbound(atoi(amt), 0, 0x3FFF);
        test.setmax = atoi(max);
        test.max = atoi(fmx);
        test.script = biitems[itemdata_dlg[132].d1].second + 1;
        test.playsound = vbound(atoi(snd), 0, 127);
        test.collect_script = biitems[itemdata_dlg[102].d1].second + 1;
        test.misc1  = atoi(ms1);
        test.misc2  = atoi(ms2);
        test.misc3  = atoi(ms3);
        test.misc4  = atoi(ms4);
        test.misc5  = atoi(ms5);
        test.misc6  = atoi(ms6);
        test.misc7  = atoi(ms7);
        test.misc8  = atoi(ms8);
        test.misc9  = atoi(ms9);
        test.misc10 = atoi(ms10);
        test.magic = vbound(atoi(mgc), 0, 255);
        test.pickup_hearts = vbound(atoi(hrt), 0, 255);
        test.power = vbound(atoi(pow), 0, 255);
        test.usesound = vbound(atoi(asn), 0, 127);
        
        test.family = vbound(biic[itemdata_dlg[9].d1].i, 0, 255);
        
        if(itemdata_dlg[14].flags & D_SELECTED)
            test.flags |= ITEM_GAMEDATA;
            
        if(itemdata_dlg[15].flags & D_SELECTED)
            test.flags |= ITEM_FLAG1;
            
        if(itemdata_dlg[16].flags & D_SELECTED)
            test.flags |= ITEM_FLAG2;
            
        if(itemdata_dlg[17].flags & D_SELECTED)
            test.flags |= ITEM_FLAG3;
            
        if(itemdata_dlg[18].flags & D_SELECTED)
            test.flags |= ITEM_FLAG4;
            
        if(itemdata_dlg[19].flags & D_SELECTED)
            test.flags |= ITEM_FLAG5;
            
        test.tile  = itemdata_dlg[68].d1;
        test.csets = itemdata_dlg[68].d2;
        
        if(itemdata_dlg[69].flags & D_SELECTED)
            test.misc |= 1;
            
        if(itemdata_dlg[70].flags & D_SELECTED)
            test.misc |= 2;
            
        test.amount |= (itemdata_dlg[94].flags & D_SELECTED) ? 0x8000 : 0;
        
        if(itemdata_dlg[107].flags & D_SELECTED)
            test.flags |= ITEM_KEEPOLD;
            
        if(itemdata_dlg[108].flags & D_SELECTED)
            test.flags |= ITEM_GAINOLD;
            
        if(itemdata_dlg[109].flags & D_SELECTED)
            test.flags |= ITEM_EDIBLE;
            
        if(itemdata_dlg[110].flags & D_SELECTED)
            test.flags |= ITEM_COMBINE;
            
        if(itemdata_dlg[137].flags & D_SELECTED)
            test.flags |= ITEM_DOWNGRADE;
            
        if(itemdata_dlg[138].flags & D_SELECTED)
            test.flags |= ITEM_RUPEE_MAGIC;
            
        test.csets  |= (atoi(fcs)&15)<<4;
        test.frames = zc_min(atoi(frm),255);
        test.speed  = zc_min(atoi(spd),255);
        test.delay  = zc_min(atoi(dly),255);
        test.ltm    = zc_max(zc_min(atol(ltm),NEWMAXTILES-1),0-(NEWMAXTILES-1));
        test.wpn   = biw[itemdata_dlg[140].d1].i;
        test.wpn2  = biw[itemdata_dlg[142].d1].i;
        test.wpn3  = biw[itemdata_dlg[144].d1].i;
        test.wpn4  = biw[itemdata_dlg[146].d1].i;
        test.wpn5  = biw[itemdata_dlg[148].d1].i;
        test.wpn6  = biw[itemdata_dlg[150].d1].i;
        test.wpn7  = biw[itemdata_dlg[152].d1].i;
        test.wpn8  = biw[itemdata_dlg[154].d1].i;
        test.wpn9  = biw[itemdata_dlg[156].d1].i;
        test.wpn10 = biw[itemdata_dlg[158].d1].i;
        
        for(int j=0; j<8; j++)
            test.initiald[j] = vbound(ffparse(da[j]),-2147483647, 2147483647);
            
        test.initiala[0] = vbound(atoi(da[8])*10000,0,320000);
        test.initiala[1] = vbound(atoi(da[9])*10000,0,320000);
        
        if(ret == 40)
        {
            itemdata_help(test.family);
        }
        
        if(ret==71)
        {
            test_item(test,itemdata_dlg[0].x+itemdata_dlg[0].w/2-(is_large?34:10),itemdata_dlg[0].y+itemdata_dlg[0].h/2-(is_large?34:10));
            sprintf(fcs,"%d",test.csets>>4);
            sprintf(frm,"%d",test.frames);
            sprintf(spd,"%d",test.speed);
            sprintf(dly,"%d",test.delay);
            sprintf(ltm,"%ld",test.ltm);
            
            sprintf(cll,"%d",test.fam_type);
            sprintf(amt,"%d",test.amount&0x4000?-(test.amount&0x3FFF):test.amount&0x3FFF);
            sprintf(fmx,"%d",test.max);
            sprintf(max,"%d",test.setmax);
        }
        
        if(ret==5)
        {
            //do not reset graphics fields! -DD
            //memset(&test, 0, sizeof(itemdata));
            //test.family=0xFF;
            
            reset_itembuf(&test, index);
            
            sprintf(fcs,"%d",test.csets>>4);
            sprintf(frm,"%d",test.frames);
            sprintf(spd,"%d",test.speed);
            sprintf(dly,"%d",test.delay);
            sprintf(ltm,"%ld",test.ltm);
            sprintf(cll,"%d",test.fam_type);
            sprintf(amt,"%d",test.amount&0x4000?-(test.amount&0x3FFF):test.amount&0x3FFF);
            sprintf(fmx,"%d",test.max);
            sprintf(max,"%d",test.setmax);
            pickupscript = test.script;
            sprintf(snd,"%d",test.playsound);
            pickupscript = test.collect_script;
            sprintf(ms1,"%ld",test.misc1);
            sprintf(ms2,"%ld",test.misc2);
            sprintf(ms3,"%ld",test.misc3);
            sprintf(ms4,"%ld",test.misc4);
            sprintf(ms5,"%ld",test.misc5);
            sprintf(ms6,"%ld",test.misc6);
            sprintf(ms7,"%ld",test.misc7);
            sprintf(ms8,"%ld",test.misc8);
            sprintf(ms9,"%ld",test.misc9);
            sprintf(ms10,"%ld",test.misc10);
            sprintf(mgc,"%d",test.magic);
            sprintf(hrt,"%d",test.pickup_hearts);
            sprintf(pow,"%d",test.power);
            sprintf(asn,"%d",test.usesound);
            sprintf(zname, "zz%03d", index);
            sprintf(name,"%s",index<iLast?old_item_string[index]:zname);
            
            for(int j=0; j<8; j++)
                sprintf(da[j],"%.4f",test.initiald[j]/10000.0);
                
            sprintf(da[8],"%d",test.initiala[0]/10000);
            sprintf(da[9],"%d",test.initiala[1]/10000);
            sprintf(itemnumstr,"Item %d: %s", index, name);
            
            itemdata_dlg[0].dp = itemnumstr;
            itemdata_dlg[0].dp2 = lfont;
            
            if(biic_cnt==-1)
            {
                build_biic_list();
            }
            
            if(biw_cnt==-1)
            {
                build_biw_list();
            }
            
            itemdata_dlg[7].dp = name;
            
            for(int j=0; j<biic_cnt; j++)
            {
                if(biic[j].i == test.family)
                    itemdata_dlg[9].d1 = j;
            }
            
            itemdata_dlg[11].dp = cll;
            itemdata_dlg[13].dp = pow;
            itemdata_dlg[14].flags = (itemsbuf[index].flags&ITEM_GAMEDATA) ? D_SELECTED : 0;
            itemdata_dlg[15].flags = (itemsbuf[index].flags&ITEM_FLAG1) ? D_SELECTED : 0;
            itemdata_dlg[16].flags = (itemsbuf[index].flags&ITEM_FLAG2) ? D_SELECTED : 0;
            itemdata_dlg[17].flags = (itemsbuf[index].flags&ITEM_FLAG3) ? D_SELECTED : 0;
            itemdata_dlg[18].flags = (itemsbuf[index].flags&ITEM_FLAG4) ? D_SELECTED : 0;
            itemdata_dlg[19].flags = (itemsbuf[index].flags&ITEM_FLAG5) ? D_SELECTED : 0;
            itemdata_dlg[21].dp = ms1;
            itemdata_dlg[23].dp = ms2;
            itemdata_dlg[25].dp = ms3;
            itemdata_dlg[27].dp = ms4;
            itemdata_dlg[29].dp = ms5;
            itemdata_dlg[31].dp = ms6;
            itemdata_dlg[33].dp = ms7;
            itemdata_dlg[35].dp = ms8;
            itemdata_dlg[37].dp = ms9;
            itemdata_dlg[39].dp = ms10;
            
            itemdata_dlg[58].dp = fcs;
            itemdata_dlg[60].dp = frm;
            itemdata_dlg[62].dp = spd;
            itemdata_dlg[64].dp = dly;
            itemdata_dlg[66].dp = ltm;
            itemdata_dlg[68].d1 = test.tile;
            itemdata_dlg[68].d2 = test.csets&15;
            itemdata_dlg[69].flags = (test.misc&1) ? D_SELECTED : 0;
            itemdata_dlg[70].flags = (test.misc&2) ? D_SELECTED : 0;
            
            itemdata_dlg[93].dp = amt;
            itemdata_dlg[94].flags = (test.amount & 0x8000)  ? D_SELECTED : 0;
            itemdata_dlg[96].d1 = itemsbuf[index].count+1;
            itemdata_dlg[98].dp = fmx;
            itemdata_dlg[100].dp = max;
            itemdata_dlg[102].d1 = pickupscript;
            itemdata_dlg[104].dp = snd;
            itemdata_dlg[106].dp = hrt;
            
            itemdata_dlg[107].flags = (test.flags&ITEM_KEEPOLD) ? D_SELECTED : 0;
            itemdata_dlg[108].flags = (test.flags&ITEM_GAINOLD) ? D_SELECTED : 0;
            itemdata_dlg[109].flags = (test.flags&ITEM_EDIBLE) ? D_SELECTED : 0;
            itemdata_dlg[110].flags = (test.flags&ITEM_COMBINE) ? D_SELECTED : 0;
            
            itemdata_dlg[132].d1 = script;
            itemdata_dlg[134].dp = mgc;
            itemdata_dlg[136].dp = asn;
            itemdata_dlg[137].flags = (test.flags&ITEM_DOWNGRADE) ? D_SELECTED : 0;
            itemdata_dlg[138].flags = (test.flags&ITEM_RUPEE_MAGIC) ? D_SELECTED : 0;
            
            for(int j=0; j<biw_cnt; j++)
            {
                if(biw[j].i == test.wpn)
                    itemdata_dlg[140].d1 = j;
                    
                if(biw[j].i == test.wpn2)
                    itemdata_dlg[142].d1 = j;
                    
                if(biw[j].i == test.wpn3)
                    itemdata_dlg[144].d1 = j;
                    
                if(biw[j].i == test.wpn4)
                    itemdata_dlg[146].d1 = j;
                    
                if(biw[j].i == test.wpn5)
                    itemdata_dlg[148].d1 = j;
                    
                if(biw[j].i == test.wpn6)
                    itemdata_dlg[150].d1 = j;
                    
                if(biw[j].i == test.wpn7)
                    itemdata_dlg[152].d1 = j;
                    
                if(biw[j].i == test.wpn8)
                    itemdata_dlg[154].d1 = j;
                    
                if(biw[j].i == test.wpn9)
                    itemdata_dlg[156].d1 = j;
                    
                if(biw[j].i == test.wpn10)
                    itemdata_dlg[158].d1 = j;
            }
            
            for(int j=0; j<8; j++)
                itemdata_dlg[187+j].dp = da[j];
                
            itemdata_dlg[197].dp = da[8];
            itemdata_dlg[198].dp = da[9];
            
            setLabels(test.family);
        }
        
        if(ret==9)
            setLabels(test.family);
    }
    while(ret==5 || ret==9 || ret==71 || ret==40);
    
    font=tfont;
    
    if(ret==3)
    {
        strcpy(item_string[index],name);
        itemsbuf[index] = test;
        saved = false;
    }
    
}

extern DIALOG ilist_dlg[];
static int copiedItem;
static MENU ilist_rclick_menu[] =
{
    { (char *)"Copy",  NULL, NULL, 0, NULL },
    { (char *)"Paste", NULL, NULL, 0, NULL },
    { (char *)"Save", NULL, NULL, 0, NULL },
    { (char *)"Load", NULL, NULL, 0, NULL },
    { NULL,            NULL, NULL, 0, NULL }
};

int readoneitem(PACKFILE *f, int index)
{
	dword section_version = 0;
	dword section_cversion = 0;
	int zversion = 0;
	int zbuild = 0;
	itemdata tempitem;
	memset(&tempitem, 0, sizeof(itemdata));
        //reset_itembuf(&tempitem,i);
    
   
	char istring[64]={0};
	//section version info
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
    
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	al_trace("readoneitem section_version: %d\n", section_version);
	al_trace("readoneitem section_cversion: %d\n", section_cversion);
    
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zitem packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	else if ( ( section_version > V_ITEMS ) || ( section_version == V_ITEMS && section_cversion < CV_ITEMS ) )
	{
		al_trace("Cannot read .zitem packfile made using V_ITEMS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zitem packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	if(!pfread(&istring, 64, f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.tile,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.misc,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.csets,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.frames,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.speed,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.delay,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.ltm,f,true))
	{	
		return 0;
	}
    
	if(!p_igetl(&tempitem.family,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.fam_type,f,true))
	{
		return 0;
	}
	
	if(!p_igetl(&tempitem.power,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.flags,f,true))
	{
		return 0;
	}
    
	if(!p_igetw(&tempitem.script,f,true))
	{
		return 0;
	}
	
	if(!p_getc(&tempitem.count,f,true))
	{
		return 0;
	}
    
	if(!p_igetw(&tempitem.amount,f,true))
	{
		return 0;
	}
    
	if(!p_igetw(&tempitem.collect_script,f,true))
	{
		return 0;
	}
    
	if(!p_igetw(&tempitem.setmax,f,true))
	{
		return 0;
	}
    
	if(!p_igetw(&tempitem.max,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.playsound,f,true))
	{
		return 0;
	}
    
	for(int j=0; j<8; j++)
	{
		if(!p_igetl(&tempitem.initiald[j],f,true))
		{
		return 0;
		}
	}
    
	for(int j=0; j<2; j++)
	{
		if(!p_getc(&tempitem.initiala[j],f,true))
		{
			return 0;
		}
	}
    
	if(!p_getc(&tempitem.wpn,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn2,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn3,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn4,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn5,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn6,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn7,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn8,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn9,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.wpn10,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.pickup_hearts,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc1,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc2,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.magic,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc3,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc4,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc5,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc6,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc7,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc8,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc9,f,true))
	{
		return 0;
	}
    
	if(!p_igetl(&tempitem.misc10,f,true))
	{
		return 0;
	}
    
	if(!p_getc(&tempitem.usesound,f,true))
	{
		return 0;
	}
    
	memcpy(&itemsbuf[index], &tempitem, sizeof(itemdata));
	//strcpy(item_string[index], istring);
	itemsbuf[bii[index].i] = tempitem;
	strcpy(item_string[bii[index].i], istring);
       
	return 1;
}



int writeoneitem(PACKFILE *f, int i)
{
    
    dword section_version=V_ITEMS;
    dword section_cversion=CV_ITEMS;
	int zversion = ZELDA_VERSION;
	int zbuild = VERSION_BUILD;
    
  
    //section version info
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		new_return(2);
	}
    
	if(!p_iputw(section_cversion,f))
	{
		new_return(3);
	}
    
   
        
       //section data        
        
            if(!pfwrite(item_string[i], 64, f))
            {
                new_return(5);
            }
	    
	    if(!p_iputl(itemsbuf[i].tile,f))
            {
                new_return(6);
            }
            
            if(!p_putc(itemsbuf[i].misc,f))
            {
                new_return(7);
            }
            
            if(!p_putc(itemsbuf[i].csets,f))
            {
                new_return(8);
            }
            
            if(!p_putc(itemsbuf[i].frames,f))
            {
                new_return(9);
            }
            
            if(!p_putc(itemsbuf[i].speed,f))
            {
                new_return(10);
            }
            
            if(!p_putc(itemsbuf[i].delay,f))
            {
                new_return(11);
            }
            
            if(!p_iputl(itemsbuf[i].ltm,f))
            {
                new_return(12);
            }
            
            if(!p_iputl(itemsbuf[i].family,f))
            {
                new_return(13);
            }
            
            if(!p_putc(itemsbuf[i].fam_type,f))
            {
                new_return(14);
            }
            
            if(!p_iputl(itemsbuf[i].power,f))
            {
                new_return(14);
            }
            
            if(!p_iputl(itemsbuf[i].flags,f))
            {
                new_return(15);
            }
            
            if(!p_iputw(itemsbuf[i].script,f))
            {
                new_return(16);
            }
            
            if(!p_putc(itemsbuf[i].count,f))
            {
                new_return(17);
            }
            
            if(!p_iputw(itemsbuf[i].amount,f))
            {
                new_return(18);
            }
            
            if(!p_iputw(itemsbuf[i].collect_script,f))
            {
                new_return(19);
            }
            
            if(!p_iputw(itemsbuf[i].setmax,f))
            {
                new_return(21);
            }
            
            if(!p_iputw(itemsbuf[i].max,f))
            {
                new_return(22);
            }
            
            if(!p_putc(itemsbuf[i].playsound,f))
            {
                new_return(23);
            }
            
            for(int j=0; j<8; j++)
            {
                if(!p_iputl(itemsbuf[i].initiald[j],f))
                {
                    new_return(24);
                }
            }
            
            for(int j=0; j<2; j++)
            {
                if(!p_putc(itemsbuf[i].initiala[j],f))
                {
                    new_return(25);
                }
            }
            
            if(!p_putc(itemsbuf[i].wpn,f))
            {
                new_return(26);
            }
            
            if(!p_putc(itemsbuf[i].wpn2,f))
            {
                new_return(27);
            }
            
            if(!p_putc(itemsbuf[i].wpn3,f))
            {
                new_return(28);
            }
            
            if(!p_putc(itemsbuf[i].wpn4,f))
            {
                new_return(29);
            }
            
            if(!p_putc(itemsbuf[i].wpn5,f))
            {
                new_return(30);
            }
            
            if(!p_putc(itemsbuf[i].wpn6,f))
            {
                new_return(31);
            }
            
            if(!p_putc(itemsbuf[i].wpn7,f))
            {
                new_return(32);
            }
            
            if(!p_putc(itemsbuf[i].wpn8,f))
            {
                new_return(33);
            }
            
            if(!p_putc(itemsbuf[i].wpn9,f))
            {
                new_return(34);
            }
            
            if(!p_putc(itemsbuf[i].wpn10,f))
            {
                new_return(35);
            }
            
            if(!p_putc(itemsbuf[i].pickup_hearts,f))
            {
                new_return(36);
            }
            
            if(!p_iputl(itemsbuf[i].misc1,f))
            {
                new_return(37);
            }
            
            if(!p_iputl(itemsbuf[i].misc2,f))
            {
                new_return(38);
            }
            
            if(!p_putc(itemsbuf[i].magic,f))
            {
                new_return(39);
            }
            
            if(!p_iputl(itemsbuf[i].misc3,f))
            {
                new_return(40);
            }
            
            if(!p_iputl(itemsbuf[i].misc4,f))
            {
                new_return(41);
            }
            
            if(!p_iputl(itemsbuf[i].misc5,f))
            {
                new_return(42);
            }
            
            if(!p_iputl(itemsbuf[i].misc6,f))
            {
                new_return(43);
            }
            
            if(!p_iputl(itemsbuf[i].misc7,f))
            {
                new_return(44);
            }
            
            if(!p_iputl(itemsbuf[i].misc8,f))
            {
                new_return(45);
            }
            
            if(!p_iputl(itemsbuf[i].misc9,f))
            {
                new_return(46);
            }
            
            if(!p_iputl(itemsbuf[i].misc10,f))
            {
                new_return(47);
            }
            
            if(!p_putc(itemsbuf[i].usesound,f))
            {
                new_return(48);
            }
	    
	    
		return 1;
}

void ilist_rclick_func(int index, int x, int y)
{
    if(bii[index].i<0) // Clicked (none)?
        return;
    
    if(copiedItem<0)
        ilist_rclick_menu[1].flags|=D_DISABLED;
    else
        ilist_rclick_menu[1].flags&=~D_DISABLED;
    
    int ret=popup_menu(ilist_rclick_menu, x, y);
    
    if(ret==0) // copy
        copiedItem=bii[index].i;
    else if(ret==1) // paste
    {
        itemsbuf[bii[index].i]=itemsbuf[copiedItem];
        ilist_dlg[2].flags|=D_DIRTY;
        saved=false;
    }
    else if(ret==2) // save
    {
	if(!getname("Save Item(.zitem)", "zitem", NULL,datapath,false))
                return;
	int iid = bii[index].i; //the item id is not the sajme as the editor index
	//the editor index is the position in the current LIST. -Z
	
	//al_trace("Saving item index: %d\n",index);
	//al_trace("Saving item id: %d\n",iid);
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
	if(!f) return;
	/*if (!writeoneitem(f,iid))
	{
		al_trace("Could not write to .zitem packfile %s\n", temppath);
	}
	*/
	writeoneitem(f,iid);
	pack_fclose(f);
     
        
    }
    else if(ret==3) // load
    {
	if(!getname("Load Item(.zitem)", "zitem", NULL,datapath,false))
                return;
	PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
	if(!f) return;
	
	if (!readoneitem(f,index))
	{
		al_trace("Could not read from .zitem packfile %s\n", temppath);
		jwin_alert("ZITEM File: Error","Could not load the specified item.",NULL,NULL,"O&K",NULL,'k',0,lfont);
	}
	
	pack_fclose(f);
        //itemsbuf[bii[index].i]=itemsbuf[copiedItem];
        ilist_dlg[2].flags|=D_DIRTY; //Causes the dialogie list to refresh, updating the item name.
        saved=false;
    }
}



int onCustomItems()
{
    /*
      char *hold = item_string[0];
      item_string[0] = "rupee (1)";
      */
    
    build_bii_list(false);
    int foo;
    int index = select_item("Select Item",bii[0].i,true,foo);
    copiedItem=-1;
    
    while(index >= 0)
    {
        build_biw_list();
        edit_itemdata(index);
        index = select_item("Select Item",index,true,foo);
    }
    
    refresh(rMAP+rCOMBOS);
    return D_O_K;
}

/****************************/
/******  onCustomWpns  ******/
/****************************/

static DIALOG wpndata_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     55,   40,   210,  163,  vc(14),  vc(1),  0,       D_EXIT,     0,             0,       NULL, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { d_cstile_proc,     198,  84,   20,   20,   vc(11),  vc(1),  0,       0,          0,             6,       NULL, NULL, NULL },
    { jwin_button_proc,  90,   176,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,  170,  176,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_check_proc,   198,  109,   65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "Auto-flash", NULL, NULL },
    { jwin_check_proc,   198,  120,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "2P Flash", NULL, NULL },
    { jwin_check_proc,   198,  131,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "H-Flip", NULL, NULL },
    { jwin_check_proc,   198,  142,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "V-Flip", NULL, NULL },
    { jwin_text_proc,    61,   88,   96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Flash CSet:", NULL, NULL },
    //10
    { jwin_text_proc,    61,   106,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Animation Frames:", NULL, NULL },
    { jwin_text_proc,    61,   124,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Animation Speed:", NULL, NULL },
    { jwin_text_proc,    61,   142,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Miscellaneous Type:", NULL, NULL },
    { jwin_edit_proc,    160,   88,   35,   16,   vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,    160,  106,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,    160,  124,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,    160,  142,  35,   16,   vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_check_proc,   198,  153,  65,   9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "Behind", NULL, NULL },
    { jwin_edit_proc,     92,  65,   155,  16,   vc(12),  vc(1),  0,       0,          64,             0,       NULL, NULL, NULL },
    { jwin_text_proc,     61,  69,   35,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Name:", NULL, NULL },
    
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,          0, (void *) onSnapshot, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};




void edit_weapondata(int index)
{
    char frm[8], spd[8], fcs[8], typ[8];
    char name[64];
    char wpnnumstr[75];
    
    sprintf(wpnnumstr, "Sprite %d: %s", index, weapon_string[index]);
    wpndata_dlg[0].dp  = wpnnumstr;
    wpndata_dlg[0].dp2 = lfont;
    wpndata_dlg[2].d1  = wpnsbuf[index].tile;
    wpndata_dlg[2].d2  = wpnsbuf[index].csets&15;
    
    for(int i=0; i<4; i++)
        wpndata_dlg[i+5].flags = (wpnsbuf[index].misc&(1<<i)) ? D_SELECTED : 0;
        
    wpndata_dlg[17].flags = (wpnsbuf[index].misc & WF_BEHIND) ? D_SELECTED : 0;
    
    sprintf(fcs,"%d",wpnsbuf[index].csets>>4);
    sprintf(frm,"%d",wpnsbuf[index].frames);
    sprintf(spd,"%d",wpnsbuf[index].speed);
    sprintf(typ,"%d",wpnsbuf[index].type);
    wpndata_dlg[13].dp = fcs;
    wpndata_dlg[14].dp = frm;
    wpndata_dlg[15].dp = spd;
    wpndata_dlg[16].dp = typ;
    sprintf(name,"%s",weapon_string[index]);
    wpndata_dlg[18].dp = name;
    
    if(is_large)
    {
        large_dialog(wpndata_dlg);
    }
    
    int ret;
    wpndata test;
    
    do
    {
        ret = zc_popup_dialog(wpndata_dlg,3);
        
        test.tile  = wpndata_dlg[2].d1;
        test.csets = wpndata_dlg[2].d2;
        
        test.misc  = 0;
        
        for(int i=0; i<4; i++)
            if(wpndata_dlg[i+5].flags & D_SELECTED)
                test.misc |= 1<<i;
                
        test.misc |= (wpndata_dlg[17].flags & D_SELECTED) ? WF_BEHIND : 0;
        
        test.csets  |= (atoi(fcs)&15)<<4;
        test.frames = atoi(frm);
        test.speed  = atoi(spd);
        test.type   = atoi(typ);
        test.script = 0; // Not used yet
    }
    while(ret==17);
    
    if(ret==3)
    {
        strcpy(weapon_string[index],name);
        wpnsbuf[index] = test;
        saved = false;
    }
}

int onCustomWpns()
{
    /*
      char *hold = item_string[0];
      item_string[0] = "rupee (1)";
      */
    
    build_biw_list();
    
    int index = select_weapon("Select Weapon",biw[0].i);
    
    while(index >= 0)
    {
        edit_weapondata(index);
        index = select_weapon("Select Weapon",index);
    }
    
    refresh(rMAP+rCOMBOS);
    return D_O_K;
}

/****************************/
/***** onCustomEnemies  *****/
/****************************/


static int enedata_data_list[] =
{
    2,3,4,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
    31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,138,139,140,141,143,-1
};

static int enedata_data2_list[] =
{
    54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,179,180,181,182,183,184,187,188,189,190,-1
};

static int enedata_flags_list[] =
{
    74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,98,99,100,101,102,103,104,105,106,120,121,-1
};

static int enedata_flags2_list[] =
{
    90,91,92,93,94,95,96,97,-1
};

static int enedata_flags3_list[] =
{
    107,108,109,110,111,112,113,114,115,116,117,118,119,185,186,-1
};

static int enedata_defense_list[] =
{
    144,145,146,147,148,149,150,151,152,161,162,163,164,165,166,167,168,169,178,-1
};

static int enedata_defense2_list[] =
{
    153,154,155,156,157,158,159,160,170,171,172,173,174,175,176,177,191,192,-1
};

static TABPANEL enedata_tabs[] =
{
    { (char *)"Data 1",       D_SELECTED,      enedata_data_list,     0, NULL },
    { (char *)"Data 2",       0,               enedata_data2_list,    0, NULL },
    { (char *)"Misc. Flags",	 0,               enedata_flags_list,    0, NULL },
//{ (char *)"Flags 2",	    0,               enedata_flags2_list,   0, NULL },
    { (char *)"Defenses 1",	 0,               enedata_defense_list,   0, NULL },
    { (char *)"Defenses 2",	 0,               enedata_defense2_list,   0, NULL },
    { (char *)"Spawn Flags",	 0,               enedata_flags3_list,   0, NULL },
    { NULL,                   0,               NULL,                  0, NULL }
};

list_data_struct bief[eeMAX];
int bief_cnt=-1;

void build_bief_list()
{
    int start=bief_cnt=0;
    
    for(int i=start; i<eeMAX; i++)
    {
        if(enetype_string[i][0]!='-')
        {
            bief[bief_cnt].s = (char *)enetype_string[i];
            bief[bief_cnt].i = i;
            ++bief_cnt;
        }
    }
    
    // No alphabetic sorting for this list
    for(int j=start+1; j<bief_cnt-1; j++)
    {
        if(!strcmp(bief[j].s,"(None)"))
        {
            for(int i=j; i>0; i--)
                zc_swap(bief[i],bief[i-1]);
                
            break;
        }
    }
}

const char *enetypelist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = bief_cnt;
        return NULL;
    }
    
    return bief[index].s;
}

list_data_struct biea[wMAX];
int biea_cnt=-1;

void build_biea_list()
{
    int start=biea_cnt=0;
    
    for(int i=start; i<aMAX; i++)
    {
        if(eneanim_string[i][0]!='-')
        {
            biea[biea_cnt].s = (char *)eneanim_string[i];
            biea[biea_cnt].i = i;
            ++biea_cnt;
        }
    }
    
    for(int i=start; i<biea_cnt-1; i++)
        for(int j=i+1; j<biea_cnt; j++)
            if(stricmp(biea[i].s,biea[j].s)>0 && strcmp(biea[j].s,""))
                zc_swap(biea[i],biea[j]);
}


const char *eneanimlist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = biea_cnt;
        return NULL;
    }
    
    return biea[index].s;
}

const char *itemsetlist(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,count_item_drop_sets()-1);
        return item_drop_sets[index].name;
        //  return itemset_string[index];
    }
    
    *list_size=count_item_drop_sets();
    return NULL;
}

list_data_struct biew[wMax-wEnemyWeapons];
int biew_cnt=-1;

void build_biew_list()
{
    biew_cnt=0;
    
    for(int i=0; i<wMax-wEnemyWeapons; i++)
    {
        if(eweapon_string[i][0]!='-')
        {
            biew[biew_cnt].s = (char *)eweapon_string[i];
            biew[biew_cnt].i = i;
            ++biew_cnt;
        }
    }
}

const char *eweaponlist(int index, int *list_size)
{
    if(biew_cnt==-1)
        build_biew_list();
        
    if(index>=0)
    {
        bound(index,0,biew_cnt-1);
        return biew[index].s;
    }
    
    *list_size=biew_cnt;
    return NULL;
}

const char *defenselist(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,edLAST-1);
        
        switch(index)
        {
        default:
            return "(None)";
            
        case edHALFDAMAGE:
            return "1/2 Damage";
            
        case edQUARTDAMAGE:
            return "1/4 Damage";
            
        case edSTUNONLY:
            return "Stun";
            
        case edSTUNORCHINK:
            return "Stun Or Block";
            
        case edSTUNORIGNORE:
            return "Stun Or Ignore";
            
        case edCHINKL1:
            return "Block If < 1";
            
        case edCHINKL2:
            return "Block If < 2";
            
        case edCHINKL4:
            return "Block If < 4";
            
        case edCHINKL6:
            return "Block If < 6";
            
        case edCHINKL8:
            return "Block If < 8";
            
        case edCHINK:
            return "Block";
            
        case edIGNOREL1:
            return "Ignore If < 1";
            
        case edIGNORE:
            return "Ignore";
            
        case ed1HKO:
            return "One-Hit-Kill";
        }
    }
    
    *list_size = edLAST;
    return NULL;
}


//
// Enemy Misc. Attribute label swapping device
//
struct EnemyNameInfo
{
    int family;
    char *misc[10];
    void* list[10];
};

const char *walkmisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e1tLAST-1);
        return walkmisc1_string[index];
    }
    
    *list_size = e1tLAST;
    return NULL;
}

const char *walkmisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e2tTRIBBLE);
        return walkmisc2_string[index];
    }
    
    *list_size = e2tTRIBBLE+1;
    return NULL;
}

const char *walkmisc7list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e7tEATHURT);
        return walkmisc7_string[index];
    }
    
    *list_size = e7tEATHURT+1;
    return NULL;
}

const char *walkmisc9list(int index, int *list_size)
{
    if(index>=0)
    {
        bound(index,0,e9tARMOS);
        return walkmisc9_string[index];
    }
    
    *list_size = e9tARMOS+1;
    return NULL;
}

const char *gleeokmisc3list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Breath" : "1 Shot");
    }
    
    *list_size = 2;
    return NULL;
}

const char *gohmamisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Breath" : index==1 ? "3 Shots" : "1 Shot");
    }
    
    *list_size = 3;
    return NULL;
}

const char *manhandlamisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "2x2 (8 Heads)" : "1x1 (4 Heads)");
    }
    
    *list_size = 2;
    return NULL;
}

const char *aquamisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Left (Facing Right)" : "Right (Facing Left)");
    }
    
    *list_size = 2;
    return NULL;
}

const char *patramisc4list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Oval" : "Big Circle");
    }
    
    *list_size = 2;
    return NULL;
}

const char *patramisc5list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Inner Eyes" : index==1 ? "Center Eye" : "None");
    }
    
    *list_size = 3;
    return NULL;
}

const char *patramisc10list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "2x2 Patra" : "1x1 Patra");
    }
    
    *list_size = 2;
    return NULL;
}

const char *dodongomisc10list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "BS-Zelda" : "NES");
    }
    
    *list_size = 2;
    return NULL;
}

const char *digdoggermisc10list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Kid" : "Digdogger");
    }
    
    *list_size = 2;
    return NULL;
}

const char *walkerspawnlist(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Instant" : index==1 ? "Flicker" : "Puff");
    }
    
    *list_size = 3;
    return NULL;
}

const char *wizzrobemisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Phase" : "Teleport");
    }
    
    *list_size = 2;
    return NULL;
}

const char *wizzrobemisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==3 ? "Summon (Layer)" : index==2 ? "Summon" : index==1 ? "8 Shots" : "1 Shot");
    }
    
    *list_size = 4;
    return NULL;
}

const char *keesemisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Bat" : "Keese");
    }
    
    *list_size = 2;
    return NULL;
}

const char *keesemisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Tribble" : "Normal");
    }
    
    *list_size = 2;
    return NULL;
}

const char *trapmisc2list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Constant" : "Line Of Sight");
    }
    
    *list_size = 2;
    return NULL;
}

const char *trapmisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index==2 ? "Vertical" : index==1 ? "Horizontal" : "4-Way");
    }
    
    *list_size = 3;
    return NULL;
}

const char *leevermisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        if(index==0)
            return "Link's path";
        else if(index==1)
            return "In place";
        else if(index==2)
            return "Link's path + second";
    }
    
    *list_size = 3;
    return NULL;
}

const char *rockmisc1list(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "2x2" : "1x1");
    }
    
    *list_size = 2;
    return NULL;
}

// 0: no, 1: yes
const char *yesnomisclist(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "Yes" : "No");
    }
    
    *list_size = 2;
    return NULL;
}

// 0: yes, 1: no
const char *noyesmisclist(int index, int *list_size)
{
    if(index>=0)
    {
        return (index ? "No" : "Yes");
    }
    
    *list_size = 2;
    return NULL;
}

static ListData walkmisc1_list(walkmisc1list, is_large? &lfont_l : &font);
static ListData walkmisc2_list(walkmisc2list, is_large? &lfont_l : &font);
static ListData walkmisc7_list(walkmisc7list, is_large? &lfont_l : &font);
static ListData walkmisc9_list(walkmisc9list, is_large? &lfont_l : &font);

static ListData gleeokmisc3_list(gleeokmisc3list, is_large? &lfont_l : &font);
static ListData gohmamisc1_list(gohmamisc1list, is_large? &lfont_l : &font);
static ListData manhandlamisc2_list(manhandlamisc2list, is_large? &lfont_l : &font);
static ListData aquamisc1_list(aquamisc1list, is_large? &lfont_l : &font);

static ListData patramisc4_list(patramisc4list, is_large? &lfont_l : &font);
static ListData patramisc5_list(patramisc5list, is_large? &lfont_l : &font);
static ListData patramisc10_list(patramisc10list, is_large? &lfont_l : &font);

static ListData dodongomisc10_list(dodongomisc10list, is_large? &lfont_l : &font);

static ListData keesemisc1_list(keesemisc1list, is_large? &lfont_l : &font);
static ListData keesemisc2_list(keesemisc2list, is_large? &lfont_l : &font);

static ListData digdoggermisc10_list(digdoggermisc10list, is_large? &lfont_l : &font);

static ListData wizzrobemisc1_list(wizzrobemisc1list, is_large? &lfont_l : &font);
static ListData wizzrobemisc2_list(wizzrobemisc2list, is_large? &lfont_l : &font);

static ListData trapmisc1_list(trapmisc1list, is_large? &lfont_l : &font);
static ListData trapmisc2_list(trapmisc2list, is_large? &lfont_l : &font);

static ListData leevermisc1_list(leevermisc1list, is_large? &lfont_l : &font);
static ListData rockmisc1_list(rockmisc1list, is_large? &lfont_l : &font);

static ListData yesnomisc_list(yesnomisclist, is_large? &lfont_l : &font);
static ListData noyesmisc_list(noyesmisclist, is_large? &lfont_l : &font);

static EnemyNameInfo enameinf[]=
{
    {
        eeWALK, { "Shot Type:", "Death Type:", "Death Attr. 1:", "Death Attr. 2:", "Death Attr. 3:", "Extra Shots:", "Touch Effects:", "Effect Strength:", "Walk Style:", "Walk Attr.:" },
        { (void*)&walkmisc1_list, (void*)&walkmisc2_list, NULL, NULL, NULL, NULL, (void*)&walkmisc7_list, NULL, (void*)&walkmisc9_list, NULL }
    },
    {
        eeGLEEOK, { "Heads:", "Head HP:", "Shot Type:", NULL, "Neck Segments:", "Neck Offset 1:", "Neck Offset 2:", "Head Offset:", "Fly Head Offset:", NULL },
        { NULL, NULL, (void*)&gleeokmisc3_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeDIG, { "Enemy 1 ID:", "Enemy 2 ID:", "Enemy 3 ID:", "Enemy 4 ID:", "Enemy 1 Qty:", "Enemy 2 Qty:", "Enemy 3 Qty:", "Enemy 4 Qty:", "Unused:", "Type:" },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&digdoggermisc10_list }
    },
    {
        eePATRA, { "Outer Eyes:", "Inner Eyes:", "Eyes' HP:", "Eye Movement:", "Shooters:", "Pattern Odds:", "Pattern Cycles:", "Eye Offset:", "Eye CSet:", "Type:" },
        { NULL, NULL, NULL, (void*)&patramisc4_list, (void*)&patramisc5_list, NULL, NULL, NULL, NULL, (void*)&patramisc10_list }
    },
    {
        eePROJECTILE, { "Shot Type:",  NULL, "Shot Attr. 1:", "Shot Attr. 2:", NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&walkmisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeGHOMA, { "Shot Type:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&gohmamisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeAQUA, { "Side:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&aquamisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeMANHAN, { "Frame rate:",  "Size:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { NULL, (void*)&manhandlamisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeLANM, { "Segments:",  "Segment Lag:", "Item per segment:", NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { NULL, NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeMOLD, { "Segments:",  "Item per segment:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeWIZZ, { "Walk Style:",  "Shot Type:", "Shot Attr. 1:", "Solid Combos OK:", "Teleport Delay:", NULL, NULL, NULL, NULL, NULL  },
        { (void*)&wizzrobemisc1_list, (void*)&wizzrobemisc2_list, NULL, &yesnomisc_list, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeDONGO,{ NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Type :"  },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&dodongomisc10_list }
    },
    {
        eeKEESE, { "Walk Style:",  "Death Type:", "Enemy ID:", NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&keesemisc1_list, (void*)&keesemisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeTEK,  { "1/n jump start:",  "1/n jump cont.:", "Jump Z velocity:", NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeLEV,  { "Emerge style:",  "Submerged CSet:", "Emerging step:", NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { (void*)&leevermisc1_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeWALLM,{ "Fixed distance:",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { (void*)&noyesmisc_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeTRAP, { "Direction:",  "Move Style:", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  },
        { (void*)&trapmisc1_list, (void*)&trapmisc2_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        eeROCK, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Rock size:" },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&rockmisc1_list }
    },
    {
        eeNONE, { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Boss Death Trigger:"  },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&yesnomisc_list }
    },
    {
        eeGUY,  { NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Boss Death Trigger:"  },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)&yesnomisc_list }
    },
    {
        -1,		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    }
};

static std::map<int, EnemyNameInfo *> *enamemap = NULL;

std::map<int, EnemyNameInfo *> *getEnemyNameMap()
{
    if(enamemap == NULL)
    {
        enamemap = new std::map<int, EnemyNameInfo *>();
        
        for(int i=0;; i++)
        {
            EnemyNameInfo *inf = &enameinf[i];
            
            if(inf->family == -1)
                break;
                
            (*enamemap)[inf->family] = inf;
        }
    }
    
    return enamemap;
}

static ListData itemset_list(itemsetlist, &font);
static ListData eneanim_list(eneanimlist, &font);
static ListData enetype_list(enetypelist, &font);
static ListData eweapon_list(eweaponlist, &font);

static ListData defense_list(defenselist, &font);
static ListData walkerspawn_list(walkerspawnlist, &font);

static ListData sfx__list(sfxlist, &font);

static DIALOG enedata_dlg[] =
{
    {  jwin_win_proc,            0,      0,    320,    240,    vc(14),                 vc(1),                   0,    D_EXIT,      0,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_tab_proc,            4,     24,    312,    192,    0,                      0,                       0,    0,           0,    0, (void *) enedata_tabs,                                          NULL, (void *)enedata_dlg  },
    //2
    {  d_ecstile_proc,          16,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   NULL                 },
    {  d_ecstile_proc,          52,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   NULL                 },
    {  d_ecstile_proc,          88,     62,     20,     20,    vc(11),                 vc(1),                   0,    0,           0,    6,  NULL,                                                           NULL,   NULL                 },
    //5
    {  jwin_button_proc,        50,    220,     61,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "OK",                                                  NULL,   NULL                 },
    {  jwin_button_proc,       130,    220,     61,     16,    vc(14),                 vc(1),                  27,    D_EXIT,      0,    0, (void *) "Cancel",                                              NULL,   NULL                 },
    //7
    {  jwin_text_proc,          21,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Old",                                                 NULL,   NULL                 },
    {  jwin_text_proc,          44,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Special",                                          NULL,   NULL                 },
    {  jwin_text_proc,          90,     46,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "New",                                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,           6,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
    //12
    {  jwin_text_proc,          42,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,          42,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,          78,     90,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W:",                                                  NULL,   NULL                 },
    {  jwin_text_proc,          78,    108,     10,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "H:",                                                  NULL,   NULL                 },
    //16
    {  jwin_text_proc,         114,     54,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Name:",                                               NULL,   NULL                 },
    {  jwin_text_proc,         114,     72,     30,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "HP:",                                                 NULL,   NULL                 },
    {  jwin_text_proc,         114,     90,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Damage:",                                             NULL,   NULL                 },
    {  jwin_text_proc,         114,    108,     50,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "W. Damage:",                                          NULL,   NULL                 },
    {  jwin_text_proc,         140,    126,     35,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hunger:",                                             NULL,   NULL                 },
    //21
    {  jwin_text_proc,         210,     72,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Random Rate:",                                        NULL,   NULL                 },
    {  jwin_text_proc,         210,     90,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Halt Rate:",                                          NULL,   NULL                 },
    {  jwin_text_proc,         210,    108,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Homing Factor:",                                      NULL,   NULL                 },
    {  jwin_text_proc,         210,    126,     60,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Step Speed:",                                         NULL,   NULL                 },
    //25
    {  jwin_text_proc,           8,    126,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Weapon:",                                             NULL,   NULL                 },
    {  jwin_text_proc,           8,    144,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Type:",                                               NULL,   NULL                 },
    {  jwin_text_proc,           8,    162,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "O. Anim:",                                            NULL,   NULL                 },
    {  jwin_text_proc,           8,    180,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "E. Anim:",                                            NULL,   NULL                 },
    {  jwin_text_proc,           8,    198,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Item Set:",                                           NULL,   NULL                 },
    //30
    {  jwin_edit_proc,          16,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          16,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          52,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          52,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          88,     86,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          88,    104,     22,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    //36
    {  jwin_edit_proc,         144,     50,    166,     16,    vc(12),                 vc(1),                   0,    0,          63,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         166,     68,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         166,     86,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         166,    104,     40,     16,    vc(12),                 vc(1),                   0,    0,           5,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         186,    122,     20,     16,    vc(12),                 vc(1),                   0,    0,           1,    0,  NULL,                                                           NULL,   NULL                 },
    //41
    {  jwin_edit_proc,         280,     68,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,     86,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    104,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    122,     30,     16,    vc(12),                 vc(1),                   0,    0,           4,    0,  NULL,                                                           NULL,   NULL                 },
    //45
    {  jwin_droplist_proc,      51,    122,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eweapon_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    140,    110,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    D_EXIT,      0,    0, (void *) &enetype_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    158,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    176,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &eneanim_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,      51,    194,    184,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &itemset_list,                                         NULL,   NULL                 },
    //50
    {  d_dummy_proc,           240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0,  NULL,													       NULL,   NULL                 },
    {  jwin_text_proc,         240,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Pal CSet:",                                           NULL,   NULL                 },
    {  d_dummy_proc,           280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    140,     30,     16,    vc(12),                 vc(1),                   0,    0,           2,    0,  NULL,                                                           NULL,   NULL                 },
    //54
    {  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 1:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 2:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 3:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 4:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 5:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 6:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         160,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 7:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         160,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 8:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         160,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 9:",                                   NULL,   NULL                 },
    {  jwin_text_proc,         160,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 10:",                                  NULL,   NULL                 },
    //64
    {  jwin_edit_proc,          86,     50,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,     68,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,     86,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,    104,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,    122,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,          86,    140,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,     50,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,     68,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,     86,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,    104,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    //74
    {  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Damaged By Power 0 Weapons",                          NULL,   NULL                 },
    {  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Invisible",                                        NULL,   NULL                 },
    {  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Never Returns After Death",						   NULL,   NULL                 },
    {  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Doesn't Count As Beatable Enemy",					   NULL,   NULL                 },
    {  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",										       NULL,   NULL                 },
    {  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    //82
    {  d_dummy_proc,             6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    180,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    //90
    {  d_dummy_proc,             6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  d_dummy_proc,             6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    //98
    {  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Front",                                   NULL,   NULL                 },
    {  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Left",                                    NULL,   NULL                 },
    {  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded On Right",                                   NULL,   NULL                 },
    {  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shielded In Back",                                    NULL,   NULL                 },
    {  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Hammer Can Break Shield",                             NULL,   NULL                 },
    {  d_dummy_proc,             6,    190,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Shield Is Mirrored",                                  NULL,   NULL                 },
    {  d_dummy_proc,             6,    200,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Unused",											   NULL,   NULL                 },
    {  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Can Only Be Seen By Lens Of Truth",                   NULL,   NULL                 },
    //106
    {  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flashing",                                         NULL,   NULL                 },
    {  jwin_check_proc,          6,     50,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Zora\" Screen Flag",                      NULL,   NULL                 },
    {  jwin_check_proc,          6,     60,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Falling Rocks\" Screen Flag",             NULL,   NULL                 },
    {  jwin_check_proc,          6,     70,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Corner Traps\" Screen Flag",              NULL,   NULL                 },
    {  jwin_check_proc,          6,     80,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Horz Trap\" Combo Type/Flag",            NULL,   NULL                 },
    {  jwin_check_proc,          6,     90,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Vert Trap\" Combo Type/Flag",            NULL,   NULL                 },
    {  jwin_check_proc,          6,    100,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"4-Way Trap\" Combo Type/Flag",           NULL,   NULL                 },
    {  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"LR Trap\" Combo Type/Flag",              NULL,   NULL                 },
    //114
    {  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"UD Trap\" Combo Type/Flag",              NULL,   NULL                 },
    {  jwin_check_proc,          6,    130,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Middle Traps\" Screen Flag",              NULL,   NULL                 },
    {  jwin_check_proc,          6,    140,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Statue Fire\" Screen Flag",               NULL,   NULL                 },
    {  jwin_check_proc,          6,    150,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Armos\" Combo Type",                     NULL,   NULL                 },
    {  jwin_check_proc,          6,    160,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Grave\" Combo Type",                     NULL,   NULL                 },
    {  jwin_check_proc,          6,    170,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Spawned By \"Ganon\" Room Type",                      NULL,   NULL                 },
    {  jwin_check_proc,          6,    110,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Flickering",                                       NULL,   NULL                 },
    {  jwin_check_proc,          6,    120,    280,      9,    vc(14),                 vc(1),                   0,    0,           1,    0, (void *) "Is Translucent",                                      NULL,   NULL                 },
    //122
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    //130
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_dummy_proc,           160,     40,      0,      8,    vc(15),                 vc(1),                   0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    //138
    {  jwin_text_proc,         240,    162,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
    {  jwin_text_proc,         240,    180,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "F. Rate:",                                      NULL,   NULL                 },
    {  jwin_edit_proc,         280,    158,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         280,    176,     30,     16,    vc(12),                 vc(1),                   0,    0,           3,    0,  NULL,                                                           NULL,   NULL                 },
    {  d_timer_proc,             0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_check_proc,        165,    144,     40,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Use Pal CSet",                                        NULL,   NULL                 },
    //143 - note: these are defenses 0-16, 17 is at 191
    {  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Boomerang Defense:",                                  NULL,   NULL                 },
    {  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Bomb Defense:",                                       NULL,   NULL                 },
    {  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Super Bomb Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Arrow Defense:",                                      NULL,   NULL                 },
    {  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Fire Defense:",                                       NULL,   NULL                 },
    {  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Wand Melee Defense:",                                       NULL,   NULL                 },
    {  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Magic Defense:",                                      NULL,   NULL                 },
    {  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hookshot Defense:",                                   NULL,   NULL                 },
    {  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hammer Defense:",                                     NULL,   NULL                 },
    {  jwin_text_proc,           6,     54,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Defense:",                                      NULL,   NULL                 },
    {  jwin_text_proc,           6,     72,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Sword Beam Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,     90,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Beam Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,    108,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Magic Defense:",                                NULL,   NULL                 },
    {  jwin_text_proc,           6,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Fireball Defense:",                             NULL,   NULL                 },
    {  jwin_text_proc,           6,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Refl. Rock Defense:",                                 NULL,   NULL                 },
    {  jwin_text_proc,           6,    162,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Stomp Boots Defense:",                                NULL,   NULL                 },
    {  jwin_text_proc,           6,    180,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Byrna Beam Defense:",                              NULL,   NULL                 },
    //161
    {  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   54-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   72-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,   90-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  108-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  126-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  144-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  162-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_droplist_proc,         126,  180-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    {  jwin_button_proc,           255,    54-4,     48,     16,    vc(14),                 vc(1),                  13,    D_EXIT,      0,    0, (void *) "Set All",                                            NULL,   NULL                 },
    //179
    {  jwin_text_proc,           8,    162,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "BG Sound:",                                            NULL,   NULL                 },
    {  jwin_text_proc,           8,    180,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Hit Sound:",                                           NULL,   NULL                 },
    {  jwin_text_proc,           8,    198,     45,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Death Sound:",                                         NULL,   NULL                 },
    //182
    {  jwin_droplist_proc,      86,    158,     140,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
    {  jwin_droplist_proc,      86,    176,     140,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
    {  jwin_droplist_proc,      86,    194,     140,     16,   jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &sfx__list,                                           NULL,   NULL                 },
    {  jwin_text_proc,          6,     184,     95,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Spawn Animation:",                               NULL,   NULL                 },
    {  jwin_droplist_proc,      86,    180,     85,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &walkerspawn_list,                                     NULL,   NULL                 },
    {  jwin_text_proc,         160,    126,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 11:",                                  NULL,   NULL                 },
    {  jwin_text_proc,         160,    144,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Misc Attr. 12:",                                  NULL,   NULL                 },
    {  jwin_edit_proc,         242,    122,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    {  jwin_edit_proc,         242,    140,     65,     16,    vc(12),                 vc(1),                   0,    0,           6,    0,  NULL,                                                           NULL,   NULL                 },
    
    // 191
    {  jwin_text_proc,           6,    198,     80,      8,    vc(14),                 vc(1),                   0,    0,           0,    0, (void *) "Script Weapon Defense:",                              NULL,   NULL                 },
    {  jwin_droplist_proc,      126, 198-4,    115,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,           0,    0, (void *) &defense_list,                                         NULL,   NULL                 },
    
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,          0, (void *) onSnapshot, NULL, NULL },
    {  NULL,                     0,      0,      0,      0,    0,                      0,                       0,    0,           0,    0,  NULL,                                                           NULL,   NULL                 }
};


void setEnemyLabels(int family)
{
    std::map<int, EnemyNameInfo *> *nmap = getEnemyNameMap();
    std::map<int, EnemyNameInfo *>::iterator it = nmap->find(family);
    EnemyNameInfo *inf = NULL;
    
    if(it != nmap->end())
        inf = it->second;
        
    // Default labels
    enedata_dlg[54].dp = (void*)"Misc. Attr. 1:";
    enedata_dlg[55].dp = (void*)"Misc. Attr. 2:";
    enedata_dlg[56].dp = (void*)"Misc. Attr. 3:";
    enedata_dlg[57].dp = (void*)"Misc. Attr. 4:";
    enedata_dlg[58].dp = (void*)"Misc. Attr. 5:";
    enedata_dlg[59].dp = (void*)"Misc. Attr. 6:";
    enedata_dlg[60].dp = (void*)"Misc. Attr. 7:";
    enedata_dlg[61].dp = (void*)"Misc. Attr. 8:";
    enedata_dlg[62].dp = (void*)"Misc. Attr. 9:";
    enedata_dlg[63].dp = (void*)"Misc. Attr. 10:";
    
    if(family==eeKEESE || family==eeGHINI || family==eePEAHAT || family==eeMANHAN
            || family==eeGLEEOK || family==eePATRA || family==eeDIG)
        enedata_dlg[22].dp = (void*)"Turn Freq.:";
    else
        enedata_dlg[22].dp = (void*)"Halt Rate:";
        
    for(int i = 0; i < 10; i++)
    {
        if(inf != NULL)
        {
            if(inf->misc[i]!=NULL)
            {
                enedata_dlg[54+i].dp = inf->misc[i];
                //enedata_dlg[54+i].flags = enedata_dlg[64+i].flags = 0;
            }
            else
            {
                //enedata_dlg[64+i].flags = enedata_dlg[54+i].flags = D_DISABLED;
            }
        }
        
        if(inf != NULL && inf->list[i] != NULL)
        {
            enedata_dlg[64+i].proc = jwin_droplist_proc;
            enedata_dlg[64+i].fg = jwin_pal[jcTEXTFG];
            enedata_dlg[64+i].bg = jwin_pal[jcTEXTBG];
            ((ListData*)inf->list[i])->font = (is_large ? &lfont_l : &font);
            enedata_dlg[64+i].dp = inf->list[i];
            enedata_dlg[64+i].dp2 = NULL;
            enedata_dlg[64+i].h = (is_large ? 22 : 16);
        }
        else
        {
            enedata_dlg[64+i].proc = jwin_edit_proc;
            enedata_dlg[64+i].fg = vc(14);
            enedata_dlg[64+i].bg = vc(1);
            enedata_dlg[64+i].dp = NULL;
            enedata_dlg[64+i].d1 = 6;
            enedata_dlg[64+i].h = int(16 * (is_large ? 1.5 : 1));
            enedata_dlg[64+i].dp2 = (is_large ? lfont_l : font);
        }
    }
    
    bool r = 0 != get_bit(quest_rules,qr_NEWENEMYTILES);
    enedata_dlg[r ? 47 : 48].flags |= D_DISABLED;
    enedata_dlg[r ? 48 : 47].flags &= ~D_DISABLED;
    enedata_dlg[r ? 140 : 141].flags |= D_DISABLED;
    enedata_dlg[r ? 141 : 140].flags &= ~D_DISABLED;
    
    if(family==eeTRAP || family==eeROCK || family==eeDONGO || family==eeGANON)
    {
        for(int j=0; j <= edefBYRNA+1 /* + the Set All button*/; j++) enedata_dlg[j+161].flags |= D_DISABLED;
        
        enedata_dlg[192].flags |= D_DISABLED;
    }
    else
    {
        for(int j=0; j <= edefBYRNA+1 /* + the Set All button*/; j++) enedata_dlg[j+161].flags &= ~D_DISABLED;
        
        enedata_dlg[192].flags &= ~D_DISABLED;
    }
    
    if(!(family==eeWALK || family==eeFIRE || family==eeOTHER))
    {
        enedata_dlg[98].flags |= D_DISABLED;
        enedata_dlg[99].flags |= D_DISABLED;
        enedata_dlg[100].flags |= D_DISABLED;
        enedata_dlg[101].flags |= D_DISABLED;
        enedata_dlg[102].flags |= D_DISABLED;
        enedata_dlg[186].flags |= D_DISABLED;
    }
    else
    {
        enedata_dlg[98].flags &= ~D_DISABLED;
        enedata_dlg[99].flags &= ~D_DISABLED;
        enedata_dlg[100].flags &= ~D_DISABLED;
        enedata_dlg[101].flags &= ~D_DISABLED;
        enedata_dlg[102].flags &= ~D_DISABLED;
        enedata_dlg[186].flags &= ~D_DISABLED;
    }
}

int d_ecstile_proc(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    int *tempint=enedata_flags2_list;
    tempint=tempint;
    
    switch(msg)
    {
    case MSG_CLICK:
    {
        int f = -8; // Suppress Flip, but in a way that the lowest 3 bits are 0. (Trust me here.) -L
        int t = d->d1;
        int cs = d->d2;
        
        if(select_tile(t,f,1,cs,true))
        {
            d->d1 = t;
            enedata_dlg[2].d2 = cs;
            enedata_dlg[3].d2 = cs;
            enedata_dlg[4].d2 = cs;
            return D_REDRAW;
        }
    }
    break;
    
    case MSG_DRAW:
        if(is_large)
        {
            d->w = 36;
            d->h = 36;
        }
        
        BITMAP *buf = create_bitmap_ex(8,20,20);
        BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
        
        if(buf && bigbmp)
        {
            clear_bitmap(buf);
            
            if(d->d1)
                overtile16(buf,d->d1,2,2,d->d2,0);
                
            stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2, d->w-3, d->h-3);
            destroy_bitmap(buf);
            jwin_draw_frame(bigbmp,0,0,d->w,d->h,FR_DEEP);
            blit(bigbmp,screen,0,0,d->x,d->y,d->w,d->h);
            destroy_bitmap(bigbmp);
        }
        
        //    text_mode(d->bg);
        break;
    }
    
    return D_O_K;
}

void edit_enemydata(int index)
{
    char hp[8], dp[8], wdp[8], rat[8], hrt[8], hom[8], grm[8], spd[8],
         frt[8], efr[8], bsp[8];
    char w[8],h[8],sw[8],sh[8],ew[8],eh[8];
    char name[64];
    char ms[12][8];
    char enemynumstr[75];
    
    //disable the missing dialog items!
    //else they will lurk in the background
    //stealing mouse focus -DD
    for(int i=0; enedata_flags2_list[i] != -1; i++)
    {
        enedata_dlg[enedata_flags2_list[i]].proc = d_dummy_proc;
        enedata_dlg[enedata_flags2_list[i]].x = 0;
        enedata_dlg[enedata_flags2_list[i]].y = 0;
        enedata_dlg[enedata_flags2_list[i]].w = 0;
        enedata_dlg[enedata_flags2_list[i]].h = 0;
    }
    
    sprintf(enemynumstr,"Enemy %d: %s", index, guy_string[index]);
    enedata_dlg[0].dp = enemynumstr;
    enedata_dlg[0].dp2 = lfont;
    enedata_dlg[2].d1 = guysbuf[index].tile;
    enedata_dlg[2].d2 = guysbuf[index].cset;
    enedata_dlg[3].d1 = guysbuf[index].s_tile;
    enedata_dlg[3].d2 = guysbuf[index].cset;
    enedata_dlg[4].d1 = guysbuf[index].e_tile;
    enedata_dlg[4].d2 = guysbuf[index].cset;
    
    // Enemy weapon list
    if(guysbuf[index].weapon==wNone)
    {
        enedata_dlg[45].d1 = 0;
    }
    else
    {
        if(biew_cnt==-1)
        {
            build_biew_list();
        }
        
        for(int j=0; j<biew_cnt; j++)
        {
            if(biew[j].i == guysbuf[index].weapon - wEnemyWeapons)
                enedata_dlg[45].d1 = j;
        }
    }
    
    // Enemy family list
    if(bief_cnt==-1)
    {
        build_bief_list();
    }
    
    for(int j=0; j<bief_cnt; j++)
    {
        if(bief[j].i == guysbuf[index].family)
            enedata_dlg[46].d1 = j;
    }
    
    // Enemy animation list
    if(biea_cnt==-1)
    {
        build_biea_list();
    }
    
    for(int j=0; j<biea_cnt; j++)
    {
        if(biea[j].i == guysbuf[index].anim)
            enedata_dlg[47].d1 = j;
            
        if(biea[j].i == guysbuf[index].e_anim)
            enedata_dlg[48].d1 = j;
    }
    
    
    enedata_dlg[49].d1 = guysbuf[index].item_set;
    
    sprintf(w,"%d",guysbuf[index].width);
    sprintf(h,"%d",guysbuf[index].height);
    sprintf(sw,"%d",guysbuf[index].s_width);
    sprintf(sh,"%d",guysbuf[index].s_height);
    sprintf(ew,"%d",guysbuf[index].e_width);
    sprintf(eh,"%d",guysbuf[index].e_height);
    enedata_dlg[30].dp = w;
    enedata_dlg[31].dp = h;
    enedata_dlg[32].dp = sw;
    enedata_dlg[33].dp = sh;
    enedata_dlg[34].dp = ew;
    enedata_dlg[35].dp = eh;
    
    sprintf(hp,"%d",guysbuf[index].hp);
    sprintf(dp,"%d",guysbuf[index].dp);
    sprintf(wdp,"%d",guysbuf[index].wdp);
    sprintf(grm,"%d",guysbuf[index].grumble);
    enedata_dlg[37].dp = hp;
    enedata_dlg[38].dp = dp;
    enedata_dlg[39].dp = wdp;
    enedata_dlg[40].dp = grm;
    
    sprintf(rat,"%d",guysbuf[index].rate);
    sprintf(hrt,"%d",guysbuf[index].hrate);
    sprintf(hom,"%d",guysbuf[index].homing);
    sprintf(spd,"%d",guysbuf[index].step);
    enedata_dlg[41].dp = rat;
    enedata_dlg[42].dp = hrt;
    enedata_dlg[43].dp = hom;
    enedata_dlg[44].dp = spd;
    
    sprintf(name,"%s",guy_string[index]);
    enedata_dlg[36].dp = name;
    
    sprintf(ms[0],"%ld",guysbuf[index].misc1);
    sprintf(ms[1],"%ld",guysbuf[index].misc2);
    sprintf(ms[2],"%ld",guysbuf[index].misc3);
    sprintf(ms[3],"%ld",guysbuf[index].misc4);
    sprintf(ms[4],"%ld",guysbuf[index].misc5);
    sprintf(ms[5],"%ld",guysbuf[index].misc6);
    sprintf(ms[6],"%ld",guysbuf[index].misc7);
    sprintf(ms[7],"%ld",guysbuf[index].misc8);
    sprintf(ms[8],"%ld",guysbuf[index].misc9);
    sprintf(ms[9],"%ld",guysbuf[index].misc10);
    sprintf(ms[10],"%ld",guysbuf[index].misc11);
    sprintf(ms[11],"%ld",guysbuf[index].misc12);
    
    for(int j=0; j <= edefBYRNA; j++)
    {
        enedata_dlg[j+161].d1 = guysbuf[index].defense[j];
    }
    
    enedata_dlg[192].d1 = guysbuf[index].defense[edefSCRIPT];
    
    sprintf(frt,"%d",guysbuf[index].frate);
    sprintf(efr,"%d",guysbuf[index].e_frate);
    enedata_dlg[140].dp = frt;
    enedata_dlg[141].dp = efr;
    
    //sprintf(sfx,"%d",guysbuf[index].bgsfx);
    enedata_dlg[182].d1= (int)guysbuf[index].bgsfx;
    enedata_dlg[183].d1= (int)guysbuf[index].hitsfx;
    if ( ( enedata_dlg[183].d1 == 0 ) && quest_header_zelda_version < 0x250 || (( quest_header_zelda_version == 0x250 ) && quest_header_zelda_build < 32 ) )
    {
	    //If no user-set hit sound was in place, and the quest was made in a version before 2.53.0 Gamma 2:
		enedata_dlg[183].d1 = WAV_EHIT; //Fix quests using the wrong hit sound when loading this. 
		//Force SFX_HIT here. 
	    
    }
    enedata_dlg[184].d1= (int)guysbuf[index].deadsfx;
    
    sprintf(bsp,"%d",guysbuf[index].bosspal);
    
    if(guysbuf[index].cset == 14)
    {
        enedata_dlg[143].flags = D_SELECTED;
    }
    else
    {
        enedata_dlg[143].flags = 0;
    }
    
    enedata_dlg[53].dp = bsp;
    
    for(int i=0; i<32; i++)
        enedata_dlg[74+i].flags = (guysbuf[index].flags & (1<<i)) ? D_SELECTED : 0;
        
    enedata_dlg[186].d1 = (guysbuf[index].flags & guy_fadeinstant ? 2
                           : guysbuf[index].flags & guy_fadeflicker ? 1 : 0);
                           
    for(int i=0; i<16; i++)
        enedata_dlg[106+i].flags = (guysbuf[index].flags2 & (1<<i)) ? D_SELECTED : 0;
        
    int ret;
    guydata test;
    memset(&test, 0, sizeof(guydata));
    
    if(is_large)
    {
        large_dialog(enedata_dlg);
    }
    
    setEnemyLabels(guysbuf[index].family);
    
    do
    {
        for(int i=0; i<10; i++)
        {
            if(enedata_dlg[64+i].proc==jwin_droplist_proc)
            {
                int size = 0;
                ((ListData*)enedata_dlg[64+i].dp)->listFunc(-1,&size);
                // Bound ms[i] as well as enedata_dlg[64+i].d1
                sprintf(ms[i],"%d",vbound(atoi(ms[i]), 0, size));
                enedata_dlg[64+i].d1 = atoi(ms[i]);
            }
            else
                enedata_dlg[64+i].dp = ms[i];
        }
        
        enedata_dlg[189].dp = ms[10];
        enedata_dlg[190].dp = ms[11];
    
        ret = zc_popup_dialog(enedata_dlg,3);
        
        test.tile  = enedata_dlg[2].d1;
        test.cset = enedata_dlg[2].d2;
        test.s_tile  = enedata_dlg[3].d1;
        test.e_tile  = enedata_dlg[4].d1;
        
        test.width = vbound(atoi(w),0,20);
        test.height = vbound(atoi(h),0,20);
        test.s_width = vbound(atoi(sw),0,20);
        test.s_height = vbound(atoi(sh),0,20);
        test.e_width = vbound(atoi(ew),0,20);
        test.e_height = vbound(atoi(eh),0,20);
        
        test.weapon = enedata_dlg[45].d1 != 0 ? biew[enedata_dlg[45].d1].i + wEnemyWeapons : wNone;
        test.family = bief[enedata_dlg[46].d1].i;
        test.anim = biea[enedata_dlg[47].d1].i;
        test.e_anim = biea[enedata_dlg[48].d1].i;
        test.item_set = enedata_dlg[49].d1;
        
        test.hp = vbound(atoi(hp), 0, 32767); //0x7FFF, not 0xFFFF?
        test.dp = vbound(atoi(dp), 0, 32767);
        test.wdp = vbound(atoi(wdp), 0, 32767);
        test.grumble = vbound(atoi(grm), 0, 4);
        
        test.rate = vbound(atoi(rat), 0, (test.family == eeFIRE || test.family == eeOTHER)?32767:16);
        test.hrate = vbound(atoi(hrt), 0, (test.family == eeFIRE || test.family == eeOTHER)?32767:16);
        test.homing = vbound(atoi(hom), 0, (test.family == eeFIRE || test.family == eeOTHER)?32767:256);
        test.step = vbound(atoi(spd),0, (test.family == eeFIRE || test.family == eeOTHER)?32767:1000);
        
        test.frate = vbound(atoi(frt),0,256);
        test.e_frate = vbound(atoi(efr),0,256);
        test.bosspal = vbound(atoi(bsp),-1,29);
        test.bgsfx = enedata_dlg[182].d1;
        test.hitsfx = enedata_dlg[183].d1;
        test.deadsfx = enedata_dlg[184].d1;
        
        test.misc1 = (enedata_dlg[64].proc==jwin_droplist_proc) ? enedata_dlg[64].d1 : atol(ms[0]);
        test.misc2 = (enedata_dlg[65].proc==jwin_droplist_proc) ? enedata_dlg[65].d1 : atol(ms[1]);
        test.misc3 = (enedata_dlg[66].proc==jwin_droplist_proc) ? enedata_dlg[66].d1 : atol(ms[2]);
        test.misc4 = (enedata_dlg[67].proc==jwin_droplist_proc) ? enedata_dlg[67].d1 : atol(ms[3]);
        test.misc5 = (enedata_dlg[68].proc==jwin_droplist_proc) ? enedata_dlg[68].d1 : atol(ms[4]);
        test.misc6 = (enedata_dlg[69].proc==jwin_droplist_proc) ? enedata_dlg[69].d1 : atol(ms[5]);
        test.misc7 = (enedata_dlg[70].proc==jwin_droplist_proc) ? enedata_dlg[70].d1 : atol(ms[6]);
        test.misc8 = (enedata_dlg[71].proc==jwin_droplist_proc) ? enedata_dlg[71].d1 : atol(ms[7]);
        test.misc9 = (enedata_dlg[72].proc==jwin_droplist_proc) ? enedata_dlg[72].d1 : atol(ms[8]);
        test.misc10 = (enedata_dlg[73].proc==jwin_droplist_proc) ? enedata_dlg[73].d1 : atol(ms[9]);
        test.misc11 = atol(ms[10]);
        test.misc12 = atol(ms[11]);
        
        for(int j=0; j <= edefBYRNA; j++)
        {
            test.defense[j] = enedata_dlg[j+161].d1;
        }
        
        test.defense[edefSCRIPT] = enedata_dlg[192].d1;
        
        
        for(int i=0; i<32; i++)
            test.flags |= (enedata_dlg[74+i].flags & D_SELECTED) ? (1<<i) : 0;
            
        test.flags &= ~(guy_fadeinstant|guy_fadeflicker);
        test.flags |= (enedata_dlg[186].d1==2 ? guy_fadeinstant : enedata_dlg[186].d1==1 ? guy_fadeflicker : 0);
        
        for(int i=0; i<16; i++)
            test.flags2 |= (enedata_dlg[106+i].flags & D_SELECTED) ? (1<<i) : 0;
            
        if(enedata_dlg[143].flags & D_SELECTED)
	{
            test.cset = 14;
	}
	//if we disable the box, revert to cset 8 -Z.
	else if(guysbuf[index].cset == 14 || test.cset == 14)
	{
		test.cset = 8;
	}
            
        if(ret==5)
        {
            strcpy(guy_string[index],name);
            guysbuf[index] = test;
            saved = false;
        }
        else if(ret==46)
        {
            setEnemyLabels(bief[enedata_dlg[46].d1].i);
        }
        else if(ret==178)
        {
            for(int j=1; j <= edefBYRNA; j++)
            {
                enedata_dlg[j+161].d1 = enedata_dlg[161].d1;
            }
            
            enedata_dlg[192].d1 = enedata_dlg[161].d1;
        }
    }
    while(ret != 5 && ret != 6 && ret != 0);
    
}

extern DIALOG elist_dlg[];
static int copiedGuy;
static MENU elist_rclick_menu[] =
{
    { (char *)"Copy",  NULL, NULL, 0, NULL },
    { (char *)"Paste", NULL, NULL, 0, NULL },
    { (char *)"Save", NULL, NULL, 0, NULL },
    { (char *)"Load", NULL, NULL, 0, NULL },
    { NULL,            NULL, NULL, 0, NULL }
};

int readonenpc(PACKFILE *f, int index)
{
	dword section_version = 0;
	dword section_cversion = 0;
	int zversion = 0;
	int zbuild = 0;
	guydata tempguy;
	memset(&tempguy, 0, sizeof(guydata));
        //reset_itembuf(&tempitem,i);
    
   
	char npcstring[64]={0}; //guy_string[]
	//section version info
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
    
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	al_trace("readonenpc section_version: %d\n", section_version);
	al_trace("readonenpc section_cversion: %d\n", section_cversion);
    
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .znpc packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	else if ( ( section_version > V_GUYS ) || ( section_version == V_GUYS && section_cversion < CV_GUYS ) )
	{
		al_trace("Cannot read .znpc packfile made using V_GUYS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .znpc packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
   
	if(!pfread(&npcstring, 64, f,true))
	{
		return 0;
	}
    
	//section data
	if(!p_igetl(&tempguy.flags,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.flags2,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.tile,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_getc(&tempguy.width,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_getc(&tempguy.height,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.s_tile,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_getc(&tempguy.s_width,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_getc(&tempguy.s_height,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.e_tile,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_getc(&tempguy.e_width,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_getc(&tempguy.e_height,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.hp,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.family,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.cset,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.anim,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.e_anim,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.frate,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.e_frate,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.dp,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.wdp,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.weapon,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.rate,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.hrate,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.step,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.homing,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.grumble,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.item_set,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc1,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc2,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc3,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc4,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc5,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc6,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc7,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc8,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc9,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc10,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.bgsfx,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.bosspal,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetw(&tempguy.extend,f,true))
		    {
			return 0;
		    }
		    
		    for(int j=0; j < edefLAST; j++)
		    {
			if(!p_getc(&tempguy.defense[j],f,true))
			{
			   return 0;
			}
		    }
		    
		    if(!p_getc(&tempguy.hitsfx,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_getc(&tempguy.deadsfx,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc11,f,true))
		    {
			return 0;
		    }
		    
		    if(!p_igetl(&tempguy.misc12,f,true))
		    {
			return 0;
		    }
		    
    
	memcpy(&guysbuf[index], &tempguy, sizeof(guydata));
	//strcpy(item_string[index], istring);
	guysbuf[bie[index].i] = tempguy;
	strcpy(guy_string[bie[index].i], npcstring);
       
	return 1;
}

int writeonenpc(PACKFILE *f, int i)
{
    
    dword section_version=V_GUYS;
    dword section_cversion=CV_GUYS;
	int zversion = ZELDA_VERSION;
	int zbuild = VERSION_BUILD;
    
  
    //section version info
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		new_return(2);
	}
    
	if(!p_iputw(section_cversion,f))
	{
		new_return(3);
	}
    
	if(!pfwrite(guy_string[i], 64, f))
            {
                new_return(5);
            }
        
       if(!p_iputl(guysbuf[i].flags,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].flags2,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].tile,f))
	    {
		return 0;
	    }
	    
	    if(!p_putc(guysbuf[i].width,f))
	    {
		return 0;
	    }
	    
	    if(!p_putc(guysbuf[i].height,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].s_tile,f))
	    {
		return 0;
	    }
	    
	    if(!p_putc(guysbuf[i].s_width,f))
	    {
		return 0;
	    }
	    
	    if(!p_putc(guysbuf[i].s_height,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].e_tile,f))
	    {
		return 0;
	    }
	    
	    if(!p_putc(guysbuf[i].e_width,f))
	    {
		return 0;
	    }
	    
	    if(!p_putc(guysbuf[i].e_height,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].hp,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].family,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].cset,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].anim,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].e_anim,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].frate,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].e_frate,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].dp,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].wdp,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].weapon,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].rate,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].hrate,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].step,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].homing,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].grumble,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].item_set,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc1,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc2,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc3,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc4,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc5,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc6,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc7,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc8,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc9,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc10,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].bgsfx,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].bosspal,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputw(guysbuf[i].extend,f))
	    {
		return 0;
	    }
	    
	    for(int j=0; j < edefLAST; j++)
	    {
		if(!p_putc(guysbuf[i].defense[j],f))
		{
		   return 0;
		}
	    }
	    
	    if(!p_putc(guysbuf[i].hitsfx,f))
	    {
		return 0;
	    }
	    
	    if(!p_putc(guysbuf[i].deadsfx,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc11,f))
	    {
		return 0;
	    }
	    
	    if(!p_iputl(guysbuf[i].misc12,f))
	    {
		return 0;
	    }
	    
	return 1;
}







void elist_rclick_func(int index, int x, int y)
{
    if(index==0)
        return;
    
    if(copiedGuy<=0)
        elist_rclick_menu[1].flags|=D_DISABLED;
    else
        elist_rclick_menu[1].flags&=~D_DISABLED;
    
    int ret=popup_menu(elist_rclick_menu, x, y);
    
    if(ret==0) // copy
        copiedGuy=bie[index].i;
    else if(ret==1) // paste
    {
        guysbuf[bie[index].i]=guysbuf[copiedGuy];
        elist_dlg[2].flags|=D_DIRTY;
        saved=false;
    }
    else if(ret==2) // save
    {
	if(!getname("Save NPC(.znpc)", "znpc", NULL,datapath,false))
                return;
	int iid = bie[index].i; //the item id is not the sajme as the editor index
	//the editor index is the position in the current LIST. -Z
	
	//al_trace("Saving item index: %d\n",index);
	//al_trace("Saving item id: %d\n",iid);
	PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
	if(!f) return;
	/*if (!writeoneitem(f,iid))
	{
		al_trace("Could not write to .znpc packfile %s\n", temppath);
	}
	*/
	writeonenpc(f,iid);
	pack_fclose(f);
     
        
    }
    else if(ret==3) // load
    {
	if(!getname("Load NPC(.znpc)", "znpc", NULL,datapath,false))
                return;
	PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
	if(!f) return;
	
	if (!readonenpc(f,index))
	{
		al_trace("Could not read from .znpc packfile %s\n", temppath);
		jwin_alert("ZNPC File: Error","Could not load the specified npc.",NULL,NULL,"O&K",NULL,'k',0,lfont);
	}
	
	pack_fclose(f);
        //itemsbuf[bie[index].i]=itemsbuf[copiedItem];
        elist_dlg[2].flags|=D_DIRTY; //Causes the dialogie list to refresh, updating the item name.
        saved=false;
    }
}

int onCustomEnemies()
{
    /*
      char *hold = item_string[0];
      item_string[0] = "rupee (1)";
      */
    
    int foo;
    int index = select_enemy("Select Enemy",bie[0].i,true,true,foo);
    
    while(index >= 0)
    {
        //I can't get the fucking dialog to handle a simple copy paste so I stuck it here else I'm going to rage kill something.
        //,,.Someone feel free to fix the thing properly later on.
        //Right now creating custom enemies remains a long painful process, but it shouldn't be this hard for users to use.
        //-Two cents worth. -Gleeok
        if(key[KEY_OPENBRACE])   //copy
        {
            if(index != 0)
                copiedGuy=index;
        }
        else if(key[KEY_CLOSEBRACE])   //paste
        {
            if(copiedGuy>0 && index!=0)
            {
                guysbuf[index]=guysbuf[copiedGuy];
                saved=false;
            }
        }
        else
        {
            if(index != 0)
                edit_enemydata(index);
        }
        
        index = select_enemy("Select Enemy",index,true,true,foo);
    }
    
    refresh(rMAP+rCOMBOS);
    return D_O_K;
}


int onCustomGuys()
{
    return D_O_K;
}

int d_ltile_proc(int msg,DIALOG *d,int c)
{
    //these are here to bypass compiler warnings about unused arguments
    c=c;
    
    //d1=dir
    //d2=type (determines how to animate)
    //fg=cset (6)
    enum {lt_clock, lt_tile, lt_flip, lt_extend};
    static int bg=makecol(0, 0, 0);
    int *p=(int*)d->dp3;
    int oldtile=0;
    int oldflip=0;
    
    switch(msg)
    {
    case MSG_START:
    {
        d->dp3=(int*)zc_malloc(sizeof(int)*4);
        p=(int*)d->dp3;
        p[lt_clock]=0;
        p[lt_tile]=0;
        p[lt_flip]=0;
        p[lt_extend]=0;
        linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
        break;
    }
    
    case MSG_CLICK:
    {
        int t;
        int f;
        int extend;
        int cs = 6;
        linktile(&t, &f, &extend, d->d2, d->d1, zinit.linkanimationstyle);
        
        switch(extend)
        {
        case 0:
            if(!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+2+4, d->x+(16*(is_large+1))+8+2, d->y+(16+16*(is_large+1))+2))
            {
                return D_O_K;
            }
            
            break;
            
        case 1:
            if(!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+2+4, d->x+(16*(is_large+1))+8+2, d->y+(4+32*(is_large+1))+2))
            {
                return D_O_K;
            }
            
            break;
            
        case 2:
            if(!isinRect(gui_mouse_x(),gui_mouse_y(),d->x+2+8, d->y+4, d->x+(32*(is_large+1))+8+2, d->y+(4+32*(is_large+1))+2))
            {
                return D_O_K;
            }
            
            break;
        }
        
        if(select_tile(t,f,2,cs,false,extend, true))
        {
            extend=ex;
            setlinktile(t,f,extend,d->d2,d->d1);
            return D_REDRAW;
        }
    }
    break;
    
    case MSG_VSYNC:
        oldtile=p[lt_tile];
        oldflip=p[lt_flip];
        p[lt_clock]++;
        
        switch(zinit.linkanimationstyle)
        {
        case las_original:                                             //2-frame
            switch(d->d2)
            {
            case ls_charge:
            case ls_walk:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=6)
                {
                    if(d->d1==up&&d->d2==ls_walk)
                    {
                        p[lt_flip]=1;                //h flip
                    }
                    else
                    {
                        p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;               //tile++
                    }
                };
                
                if(p[lt_clock]>=11)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_jump:
                if(p[lt_clock]>=24)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=36)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
                }
                
                break;
                
            case ls_slash:
                if(p[lt_clock]<6)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_stab:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_pound:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<30)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=31)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_float:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=12)
                {
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                };
                
                if(p[lt_clock]>=23)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_swim:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=12)
                {
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                };
                
                if(p[lt_clock]>=23)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_dive:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=50)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                };
                
                if((p[lt_clock]/12)&1)
                {
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                };
                
                if(p[lt_clock]>=81)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_landhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_landhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_cast:
                linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]<96)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                };
                
                if(p[lt_clock]>=194)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            default:
                break;
            }
            
            break;
            
        case las_bszelda:                                             //3-frame BS
            switch(d->d2)
            {
            case ls_charge:
            case ls_walk:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=27)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_jump:
                if(p[lt_clock]>=24)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=36)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
                }
                
                break;
                
            case ls_slash:
                if(p[lt_clock]<6)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_slash, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_stab:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<13)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    p[lt_extend]==2?p[lt_tile]+=2:p[lt_tile]++;                  //tile++
                    
                    if(p[lt_clock]>=16)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_pound:
                if(p[lt_clock]<12)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_pound, d->d1, zinit.linkanimationstyle);
                }
                else if(p[lt_clock]<30)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_stab, d->d1, zinit.linkanimationstyle);
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=31)
                    {
                        p[lt_clock]=-1;
                    }
                };
                
                break;
                
            case ls_float:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=55)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_swim:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=55)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_dive:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=50)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                };
                
                p[lt_tile]+=anim_3_4(p[lt_clock],7)*(p[lt_extend]==2?2:1);
                
                if(p[lt_clock]>=81)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_landhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_landhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_cast:
                linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]<96)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                };
                
                if(p[lt_clock]>=194)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            default:
                break;
            }
            
            break;
            
        case las_zelda3slow:                                         //multi-frame Zelda 3 (slow)
        case las_zelda3:                                             //multi-frame Zelda 3
            switch(d->d2)
            {
            case ls_charge:
            case ls_walk:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], d->d2, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=(64*(link_animation_speed)))
                {
                    p[lt_tile]+=(p[lt_extend]==2?2:1);
                    
                    int l=((p[lt_clock]/link_animation_speed)&15);
                    l-=((l>3)?1:0)+((l>12)?1:0);
                    p[lt_tile]+=(l/2)*(p[lt_extend]==2?2:1);
                    
                    //p[lt_tile]+=(((p[lt_clock]>>2)%8)*(p[lt_extend]==2?2:1));
                    if(p[lt_clock]>=255)
                    {
                        p[lt_clock]=-1;
                    }
                }
                
                break;
                
            case ls_jump:
                if(p[lt_clock]>=24)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                    
                    if(p[lt_clock]>=36)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_jump, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=p[lt_extend]==2?((int)p[lt_clock]/8)*2:((int)p[lt_clock]/8);
                }
                
                break;
                
            case ls_slash:
                if(p[lt_clock]>23)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_slash, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=(((p[lt_clock]>>2)%6)*(p[lt_extend]==2?2:1));
                    
                    if(p[lt_clock]>=47)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                
                break;
                
            case ls_stab:
                if(p[lt_clock]>35)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_stab, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
                    
                    if(p[lt_clock]>=47)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                
                break;
                
            case ls_pound:
                if(p[lt_clock]>35)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_pound, d->d1, zinit.linkanimationstyle);
                    p[lt_tile]+=(((p[lt_clock]>>2)%3)*(p[lt_extend]==2?2:1));
                    
                    if(p[lt_clock]>=47)
                    {
                        p[lt_clock]=-1;
                    }
                }
                else
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_walk, d->d1, zinit.linkanimationstyle);
                }
                
                break;
                
            case ls_float:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
                
                if(p[lt_clock]>=23)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_swim:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_swim, d->d1, zinit.linkanimationstyle);
                p[lt_tile]+=((p[lt_clock]/12)%4)<<(p[lt_extend]==2?1:0);
                
                if(p[lt_clock]>=47)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_dive:
                linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_dive, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]>=50)
                {
                    linktile(&p[lt_tile], &p[lt_flip], &p[lt_extend], ls_float, d->d1, zinit.linkanimationstyle);
                };
                
                p[lt_tile]+=((p[lt_clock]/6)%4)<<(p[lt_extend]==2?1:0);
                
                if(p[lt_clock]>=81)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            case ls_landhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_landhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold1:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold1, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_waterhold2:
                linktile(&p[lt_tile], &p[lt_flip], ls_waterhold2, d->d1, zinit.linkanimationstyle);
                break;
                
            case ls_cast:
                linktile(&p[lt_tile], &p[lt_flip], ls_cast, d->d1, zinit.linkanimationstyle);
                
                if(p[lt_clock]<96)
                {
                    linktile(&p[lt_tile], &p[lt_flip], ls_landhold2, d->d1, zinit.linkanimationstyle);
                };
                
                if(p[lt_clock]>=194)
                {
                    p[lt_clock]=-1;
                }
                
                break;
                
            default:
                break;
            }
            
            break;
            
        default:
            break;
        }
        
        if((p[lt_tile]!=oldtile)||(p[lt_flip]!=oldflip))
        {
            d->flags|=D_DIRTY;
        }
        
        break;
        
    case MSG_DRAW:
    {
        BITMAP *buf=create_bitmap_ex(8,1,1);
        BITMAP *buf2=buf;
        int dummy1, dummy2;
        int extend;
        linktile(&dummy1, &dummy2, &extend, d->d2, d->d1, zinit.linkanimationstyle);
        int w = 16;
        int h = 16;
        
        switch(extend)
        {
        case 0: //16x16
            break;
            
        case 1: //16x32
            w = 16;
            h = 32;
            break;
            
        case 2: //32x32
            w = 32;
            h = 32;
            break;
            
        default:
            break;
        }
        
        buf = create_bitmap_ex(8,w,h);
        
        if(is_large)
        {
            w *= 2;
            h *= 2;
        }
        
        BITMAP *bigbmp = create_bitmap_ex(8,w+4,h+4);
        
        if(buf && bigbmp)
        {
            clear_to_color(buf, bg);
            
            switch(extend)
            {
            case 0:
                overtile16(buf,p[lt_tile],0,0,6,p[lt_flip]);
                stretch_blit(buf, bigbmp, 0,0, 16, 16, 2, 2, w, h);
                jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
                blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
                break;
                
            case 1:
                overtile16(buf,p[lt_tile]-TILES_PER_ROW,0,0,6,p[lt_flip]);
                overtile16(buf,p[lt_tile],0,16,6,p[lt_flip]);
                stretch_blit(buf, bigbmp, 0,0, 16, 32, 2, 2, w, h);
                jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
                blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
                break;
                
            case 2:
                overtile16(buf,p[lt_tile]-TILES_PER_ROW,8,0,6,p[lt_flip]); //top middle
                overtile16(buf,p[lt_tile]-TILES_PER_ROW-(p[lt_flip]?-1:1),-8,0,6,p[lt_flip]); //top left
                overtile16(buf,p[lt_tile]-TILES_PER_ROW+(p[lt_flip]?-1:1),8+16,0,6,p[lt_flip]); //top right
                overtile16(buf,p[lt_tile]-(p[lt_flip]?-1:1),-8,18,6,p[lt_flip]); //bottom left
                overtile16(buf,p[lt_tile]+(p[lt_flip]?-1:1),8+16,18,6,p[lt_flip]); //bottom right
                overtile16(buf,p[lt_tile],8,16,6,p[lt_flip]);              //bottom middle
                stretch_blit(buf, bigbmp, 0,0, 32, 32, 2, 2, w, h);
                jwin_draw_frame(bigbmp,0, 0, w+4, h+4, FR_DEEP);
                blit(bigbmp,screen,0,0,d->x+8,d->y+4,w+4,h+4);
                break;
                
            default:
                break;
            }
            
            destroy_bitmap(bigbmp);
            destroy_bitmap(buf);
            destroy_bitmap(buf2);
        }
    }
    break;
    
    case MSG_END:
    {
        zc_free(d->dp3);
        break;
    }
    }
    
    return D_O_K;
}

static int linktile_land_walk_list[] =
{
    // dialog control number
    11, 12, 13, 14, 15, 16, 17, 18, -1
};

static int linktile_land_slash_list[] =
{
    // dialog control number
    19, 20, 21, 22, 23, 24, 25, 26, -1
};

static int linktile_land_stab_list[] =
{
    // dialog control number
    27, 28, 29, 30, 31, 32, 33, 34, -1
};

static int linktile_land_pound_list[] =
{
    // dialog control number
    35, 36, 37, 38, 39, 40, 41, 42, -1
};

static int linktile_land_hold_list[] =
{
    // dialog control number
    43, 44, 45, 46, -1
};

static int linktile_land_cast_list[] =
{
    // dialog control number
    47, -1
};

static int linktile_land_jump_list[] =
{
    // dialog control number
    77, 78, 79, 80, 81, 82, 83, 84, -1
};

static int linktile_land_charge_list[] =
{
    // dialog control number
    85, 86, 87, 88, 89, 90, 91, 92, -1
};

static TABPANEL linktile_land_tabs[] =
{
    // (text)
    { (char *)"Walk",       D_SELECTED,  linktile_land_walk_list, 0, NULL },
    { (char *)"Slash",      0,           linktile_land_slash_list, 0, NULL },
    { (char *)"Stab",       0,           linktile_land_stab_list, 0, NULL },
    { (char *)"Pound",      0,           linktile_land_pound_list, 0, NULL },
    { (char *)"Jump",       0,           linktile_land_jump_list, 0, NULL },
    { (char *)"Charge",     0,           linktile_land_charge_list, 0, NULL },
    { (char *)"Hold",       0,           linktile_land_hold_list, 0, NULL },
    { (char *)"Cast",       0,           linktile_land_cast_list, 0, NULL },
    { NULL,                 0,           NULL,                   0, NULL }
};

static int linktile_water_float_list[] =
{
    // dialog control number
    48, 49, 50, 51, 52, 53, 54, 55, -1
};

static int linktile_water_swim_list[] =
{
    // dialog control number
    56, 57, 58, 59, 60, 61, 62, 63, -1
};

static int linktile_water_dive_list[] =
{
    // dialog control number
    64, 65, 66, 67, 68, 69, 70, 71, -1
};

static int linktile_water_hold_list[] =
{
    // dialog control number
    72, 73, 74, 75, -1
};

static TABPANEL linktile_water_tabs[] =
{
    // (text)
    { (char *)"Float",      D_SELECTED,  linktile_water_float_list, 0, NULL },
    { (char *)"Swim",       0,           linktile_water_swim_list, 0, NULL },
    { (char *)"Dive",       0,           linktile_water_dive_list, 0, NULL },
    { (char *)"Hold",       0,           linktile_water_hold_list, 0, NULL },
    { NULL,                 0,           NULL,                     0, NULL }
};

static int linktile_land_list[] =
{
    // dialog control number
    9, -1
};

static int linktile_water_list[] =
{
    // dialog control number
    10, -1
};

static TABPANEL linktile_tabs[] =
{
    // (text)
    { (char *)"Land",       D_SELECTED,   linktile_land_list, 0, NULL },
    { (char *)"Water",      0,            linktile_water_list, 0, NULL },
    { NULL,                 0,            NULL,                0, NULL }
};

const char *animationstyles[las_max]= { "Original", "BS-Zelda", "Zelda 3", "Zelda 3 (Slow Walk)" };

const char *animationstylelist(int index, int *list_size)
{
    if(index>=0)
    {
        return animationstyles[index];
    }
    
    *list_size=las_max;
    return NULL;
}

const char *swimspeeds[2]= { "Slow", "Fast" };

const char *swimspeedlist(int index, int *list_size)
{
    if(index>=0)
    {
        return swimspeeds[index];
    }
    
    *list_size=2;
    return NULL;
}

int jwin_as_droplist_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_droplist_proc(msg,d,c);
    
    switch(msg)
    {
    case MSG_CHAR:
    case MSG_CLICK:
        zinit.linkanimationstyle=d->d1;
        
        if(zinit.linkanimationstyle==las_zelda3slow)
        {
            link_animation_speed=2;
        }
        else
        {
            link_animation_speed=1;
        }
    }
    
    return ret;
}

static ListData animationstyle_list(animationstylelist, &font);
static ListData swimspeed_list(swimspeedlist, &font);

static DIALOG linktile_dlg[] =
{
    // (dialog proc)                       (x)     (y)     (w)     (h)    (fg)                     (bg)                 (key)     (flags)    (d1)        (d2)              (dp)                             (dp2)   (dp3)
    {  jwin_win_proc,                        0,      0,    320,    240,    vc(14),                 vc(1),                   0,    D_EXIT,     0,          0, (void *) "Link Sprites",         NULL,   NULL                   },
    {  d_vsync_proc,                         0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   },
    {  d_keyboard_proc,                      0,      0,      0,      0,    0,                      0,                       0,    0,          KEY_F1,     0, (void *) onHelp,                 NULL,   NULL                   },
    {  jwin_button_proc,                    90,    215,     61,     21,    vc(14),                 vc(1),                  13,    D_EXIT,     0,          0, (void *) "OK",                   NULL,   NULL                   },
    {  jwin_button_proc,                   170,    215,     61,     21,    vc(14),                 vc(1),                  27,    D_EXIT,     0,          0, (void *) "Cancel",               NULL,   NULL                   },
    // 5
    {  jwin_check_proc,                    217,    200,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Large Link Hit Box",   NULL,   NULL                   },
    {  jwin_text_proc,                       4,    201,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Animation Style:",     NULL,   NULL                   },
    {  jwin_as_droplist_proc,               77,    197,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void *) &animationstyle_list,   NULL,   NULL                   },
    {  jwin_tab_proc,                        4,     25,    312,    150,    0,                      0,                       0,    0,          0,          0, (void *) linktile_tabs,          NULL, (void *)linktile_dlg   },
    // 9
    {  jwin_tab_proc,                        7,     46,    305,    125,    0,                      0,                       0,    0,          0,          0, (void *) linktile_land_tabs,     NULL, (void *)linktile_dlg   },
    {  jwin_tab_proc,                        7,     46,    305,    125,    0,                      0,                       0,    0,          0,          0, (void *) linktile_water_tabs,    NULL, (void *)linktile_dlg   },
    // 11 (walk sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 15 (walk sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_walk,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_walk,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_walk,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_walk,         NULL,                            NULL,   NULL                   },
    // 19 (slash spritetitles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 23 (slash sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_slash,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_slash,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_slash,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_slash,        NULL,                            NULL,   NULL                   },
    // 27 (stab sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 31 (stab sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_stab,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_stab,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_stab,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_stab,         NULL,                            NULL,   NULL                   },
    // 35 (pound sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 39 (pound sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_pound,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_pound,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_pound,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_pound,        NULL,                            NULL,   NULL                   },
    // 43 (hold sprite titles)
    {  jwin_rtext_proc,                     67,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "One Hand",             NULL,   NULL                   },
    {  jwin_rtext_proc,                     67,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Two Hands",            NULL,   NULL                   },
    // 45 (hold sprites)
    {  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_landhold1,    NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        70,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_landhold2,    NULL,                            NULL,   NULL                   },
    // 47 (casting sprites)
    {  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_cast,         NULL,                            NULL,   NULL                   },
    
    // 48 (float sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 52 (float sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_float,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_float,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_float,        NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_float,        NULL,                            NULL,   NULL                   },
    // 56 (swim sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 60 (swim sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_swim,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_swim,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_swim,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_swim,         NULL,                            NULL,   NULL                   },
    // 64 (dive sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 68 (dive sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_dive,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_dive,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_dive,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_dive,         NULL,                            NULL,   NULL                   },
    // 72 (hold sprite titles)
    {  jwin_rtext_proc,                     67,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "One Hand",             NULL,   NULL                   },
    {  jwin_rtext_proc,                     67,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Two Hands",            NULL,   NULL                   },
    // 74 (hold sprites)
    {  d_ltile_proc,                        70,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_waterhold1,   NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        70,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_waterhold2,   NULL,                            NULL,   NULL                   },
    {  jwin_check_proc,                    217,    186,      0,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Diagonal Movement",    NULL,   NULL                   },
    // 77 (jump sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 81 (jump sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_jump,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_jump,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_jump,         NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_jump,         NULL,                            NULL,   NULL                   },
    // 85 (charge sprite titles)
    {  jwin_rtext_proc,                     33,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Up",                   NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,     88,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Down",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                     33,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Left",                 NULL,   NULL                   },
    {  jwin_rtext_proc,                    101,    126,     32,      8,    jwin_pal[jcBOXFG],      jwin_pal[jcBOX],         0,    0,          0,          0, (void *) "Right",                NULL,   NULL                   },
    // 89 (charge sprites)
    {  d_ltile_proc,                        36,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          up,         ls_charge,       NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,     74,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          down,       ls_charge,       NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                        36,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          left,       ls_charge,       NULL,                            NULL,   NULL                   },
    {  d_ltile_proc,                       104,    112,     40,     40,    6,                      jwin_pal[jcBOX],         0,    0,          right,      ls_charge,       NULL,                            NULL,   NULL                   },
    // 93
    {  d_timer_proc,                         0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   },
    {  jwin_text_proc,                       4,    183,     17,      9,    vc(14),                 vc(1),                   0,    0,          1,          0, (void *) "Swim Speed:",          NULL,   NULL                   },
    {  jwin_droplist_proc,                  77,    179,     78,     16,    jwin_pal[jcTEXTFG],     jwin_pal[jcTEXTBG],      0,    0,          0,          0, (void *) &swimspeed_list,        NULL,   NULL                   },
    
    { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,          0, (void *) onSnapshot, NULL, NULL },
    {  NULL,                                 0,      0,      0,      0,    0,                      0,                       0,    0,          0,          0,               NULL,                            NULL,   NULL                   }
};



int onCustomLink()
{
    //setuplinktiles(zinit.linkanimationstyle);
    if(zinit.linkanimationstyle==las_zelda3slow)
    {
        link_animation_speed=2;
    }
    else
    {
        link_animation_speed=1;
    }
    
    linktile_dlg[0].dp2=lfont;
    linktile_dlg[5].flags = get_bit(quest_rules, qr_LTTPCOLLISION)? D_SELECTED : 0;
    linktile_dlg[76].flags = get_bit(quest_rules, qr_LTTPWALK)? D_SELECTED : 0;
    linktile_dlg[95].d1=(zinit.link_swim_speed<60)?0:1;
    linktile_dlg[7].d1=zinit.linkanimationstyle;
    
    if(is_large)
        large_dialog(linktile_dlg, 2.0);
        
    int oldWalkSpr[4][3];
    int oldStabSpr[4][3];
    int oldSlashSpr[4][3];
    int oldFloatSpr[4][3];
    int oldSwimSpr[4][3];
    int oldDiveSpr[4][3];
    int oldPoundSpr[4][3];
    int oldJumpSpr[4][3];
    int oldChargeSpr[4][3];
    int oldCastSpr[3];
    int oldHoldSpr[2][2][3];
    memcpy(oldWalkSpr, walkspr, 4*3*sizeof(int));
    memcpy(oldStabSpr, stabspr, 4*3*sizeof(int));
    memcpy(oldSlashSpr, slashspr, 4*3*sizeof(int));
    memcpy(oldFloatSpr, floatspr, 4*3*sizeof(int));
    memcpy(oldSwimSpr, swimspr, 4*3*sizeof(int));
    memcpy(oldDiveSpr, divespr, 4*3*sizeof(int));
    memcpy(oldPoundSpr, poundspr, 4*3*sizeof(int));
    memcpy(oldJumpSpr, jumpspr, 4*3*sizeof(int));
    memcpy(oldChargeSpr, chargespr, 4*3*sizeof(int));
    memcpy(oldCastSpr, castingspr, 3*sizeof(int));
    memcpy(oldHoldSpr, holdspr, 2*2*3*sizeof(int));
    
    
    int ret = popup_dialog_through_bitmap(screen2,linktile_dlg,3);
    
    if(ret==3)
    {
        saved=false;
        set_bit(quest_rules, qr_LTTPCOLLISION, (linktile_dlg[5].flags&D_SELECTED)?1:0);
        set_bit(quest_rules, qr_LTTPWALK, (linktile_dlg[76].flags&D_SELECTED)?1:0);
        zinit.link_swim_speed=(linktile_dlg[95].d1==0)?50:67;
    }
    else
    {
        memcpy(walkspr, oldWalkSpr, 4*3*sizeof(int));
        memcpy(stabspr, oldStabSpr, 4*3*sizeof(int));
        memcpy(slashspr, oldSlashSpr, 4*3*sizeof(int));
        memcpy(floatspr, oldFloatSpr, 4*3*sizeof(int));
        memcpy(swimspr, oldSwimSpr, 4*3*sizeof(int));
        memcpy(divespr, oldDiveSpr, 4*3*sizeof(int));
        memcpy(poundspr, oldPoundSpr, 4*3*sizeof(int));
        memcpy(jumpspr, oldJumpSpr, 4*3*sizeof(int));
        memcpy(chargespr, oldChargeSpr, 4*3*sizeof(int));
        memcpy(castingspr, oldCastSpr, 3*sizeof(int));
        memcpy(holdspr, oldHoldSpr, 2*2*3*sizeof(int));
    }
    
    ret=ret;
    return D_O_K;
}

void center_zq_custom_dialogs()
{
    jwin_center_dialog(itemdata_dlg);
    jwin_center_dialog(linktile_dlg);
    jwin_center_dialog(wpndata_dlg);
    jwin_center_dialog(enedata_dlg);
}

