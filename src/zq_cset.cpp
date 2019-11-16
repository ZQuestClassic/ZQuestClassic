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

//#include <mem.h>
#include <string.h>
#include <stdio.h>

#include "gui.h"
#include "zc_alleg.h"
#include "zdefs.h"
#include "colors.h"
#include "pal.h"
#include "zquest.h"
#include "jwin.h"
#include "zsys.h"
#include "zq_tiles.h"
#include "zq_misc.h"
#include "zq_cset.h"
#include "zq_class.h"

extern int d_dummy_proc(int msg,DIALOG *d,int c);
extern int d_dropdmaplist_proc(int msg,DIALOG *d,int c);
extern int onHelp();
extern FONT *lfont;
extern int jwin_pal[jcMAX];
extern const char *dmaplist(int index, int *list_size);
extern bool saved;
extern int dmap_list_size;
extern bool dmap_list_zero;
extern zinitdata zinit;
extern int startdmapxy[6];
extern PALETTE RAMpal;


#include "gfxpal.h"

int hue_x=96, hue_y=30, hue_h=16;
int light_x=74, light_y=52, light_w=16;
int sat_x=230, sat_y=52, sat_w=16;
int color_x=96, color_y=52, color_w=128, color_h=64;
int cset_x=96, cset_y=170, cset_h=8, cset_spacing=4;

void get_cset(int dataset,int row,RGB *pal)
{
    saved=false;
    
    for(int i=0; i<16; i++)
    {
        colordata[(CSET(dataset)+i)*3]   = pal[(row<<4)+i].r;
        colordata[(CSET(dataset)+i)*3+1] = pal[(row<<4)+i].g;
        colordata[(CSET(dataset)+i)*3+2] = pal[(row<<4)+i].b;
    }
}

void draw_edit_dataset_specs(int index,int copy)
{
    int window_xofs=0;
    int window_yofs=0;
    
    if(is_large)
    {
        window_xofs=(zq_screen_w-480)>>1;
        window_yofs=(zq_screen_h-360)>>1;
    }
    
    jwin_draw_frame(screen, cset_x-2, cset_y+cset_h+cset_spacing-2, int(128*(is_large?1.5:1)+4), cset_h+4, FR_DEEP);
    
    for(int i=0; i<16; i++)
    {
        rectfill(screen,int((i<<3)*(is_large?1.5:1))+cset_x,cset_y+cset_h+cset_spacing,int((i<<3)*(is_large?1.5:1)+cset_x+cset_h-1),cset_y+cset_h+cset_spacing+cset_h-1,14*16+i);
    }
    
    //  text_mode(ed1);
    rectfill(screen,int(96*(is_large?1.5:1)+window_xofs),int(193*(is_large?1.5:1)+window_yofs),int(223*(is_large?1.5:1)+window_xofs),int(220*(is_large?1.5:1)+window_yofs),jwin_pal[jcBOX]);
    textout_ex(screen,(is_large?lfont_l:font),"\x88",int((index<<3)*(is_large?1.5:1)+cset_x),int(193*(is_large?1.5:1)+window_yofs),jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    
    if(copy>=0)
    {
        textout_ex(screen,(is_large?lfont_l:font),"\x81",int((copy<<3)*(is_large?1.5:1)+cset_x),int(193*(is_large?1.5:1)+window_yofs),jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
    }
    
    textprintf_ex(screen,(is_large?lfont_l:font),88*(is_large?2:1)+window_xofs,int(204*(is_large?1.5:1)+window_yofs),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Old: %2d - %2d %2d %2d",index, RAMpal[12*16+index].r,RAMpal[12*16+index].g,RAMpal[12*16+index].b);
    textprintf_ex(screen,(is_large?lfont_l:font),88*(is_large?2:1)+window_xofs,int(214*(is_large?1.5:1)+window_yofs),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"New: %2d - %2d %2d %2d",index, RAMpal[14*16+index].r,RAMpal[14*16+index].g,RAMpal[14*16+index].b);
}

void init_colormixer()
{
    jwin_draw_frame(screen, hue_x-2, hue_y-2, int(128*(is_large?1.5:1)+4), hue_h+4, FR_DEEP);
    
    for(int i=0; i<128; i++)
    {
        RAMpal[i] = _RGB(gfx_pal+i*3); //hue
        rectfill(screen,int(floor(i*(is_large?1.5:1))+hue_x),hue_y,int(ceil(i*(is_large?1.5:1))+hue_x),hue_y+hue_h-1,i);
    }
    
    jwin_draw_frame(screen, light_x-2, light_y-2, light_w+4, int(64*(is_large?1.5:1)+4), FR_DEEP);
    
    for(int i=0; i<32; i++)
    {
        RAMpal[i+128] = _RGB(i<<1,i<<1,i<<1); //lightness
        rectfill(screen,light_x,((int)floor(i*(is_large?1.5:1))<<1)+light_y,
                 light_x+light_w-1,((int)ceil(i*(is_large?1.5:1))<<1)+light_y+1,i+128);
    }
    
    //  rect(screen,95,31,224,96,ed15);
    //  rect(screen,224,31,240,96,ed15);
    set_palette_range(RAMpal,0,255,false);
}

void colormixer(int color,int gray,int ratio)
{
    int window_xofs=0;
    int window_yofs=0;
    
    if(is_large)
    {
        window_xofs=int(zq_screen_w-320*(is_large?1.5:1))>>1;
        window_yofs=int(zq_screen_h-240*(is_large?1.5:1))>>1;
        color /= 1.5;
        ratio /= 1.5;
        gray /= 1.5;
    }
    
    custom_vsync();
    scare_mouse();
    
    jwin_draw_frame(screen, sat_x-2, sat_y-2, sat_w+4, int(64*(is_large?1.5:1)+4), FR_DEEP);
    
    for(int i=0; i<32; i++)
    {
        RAMpal[i+160] = mixRGB(gfx_pal[color*3],gfx_pal[color*3+1],
                               gfx_pal[color*3+2],gray,gray,gray,i<<1); //saturation
        rectfill(screen,sat_x,((int)floor(i*(is_large?1.5:1))<<1)+sat_y,
                 sat_x+sat_w-1,((int)ceil(i*(is_large?1.5:1))<<1)+sat_y+1,i+160);
    }
    
    RAMpal[edc] = mixRGB(gfx_pal[color*3],gfx_pal[color*3+1],gfx_pal[color*3+2],gray,gray,gray,ratio);
    RAMpal[edi] = invRGB(RAMpal[edc]);
    set_palette_range(RAMpal,160,255,false);
    
    jwin_draw_frame(screen, color_x-2, color_y-2, color_w+4, color_h+4, FR_DEEP);
    rectfill(screen,color_x,color_y,color_x+color_w-1,color_y+color_h-1,edc);
    
    if(is_large)
    {
        color *=1.5;
        ratio *=1.5;
        gray *=1.5;
    }
    
    _allegro_hline(screen,color_x,gray+color_y,color_x+color_w-1,edi);
    _allegro_vline(screen,color+color_x,color_y,color_y+color_h-1,edi);
    _allegro_hline(screen,sat_x,ratio+sat_y,sat_x+sat_w-1,edi);
    //  text_mode(ed1);
    textprintf_centre_ex(screen,font,zq_screen_w/2,int(color_y+color_h+10*(is_large?1.5:1)),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"  RGB - %2d %2d %2d  ",RAMpal[edc].r,RAMpal[edc].g,RAMpal[edc].b);
    unscare_mouse();
    SCRFIX();
}

int color = 0;
int gray  = 0;
int ratio = 32;



void edit_dataset(int dataset)
{
    PALETTE holdpal;
    memcpy(holdpal,RAMpal,sizeof(RAMpal));
    int index = 0;
    
    hue_x=96;
    hue_y=30;
    hue_h=16;
    light_x=74;
    light_y=52;
    light_w=16;
    sat_x=230;
    sat_y=52;
    sat_w=16;
    color_x=96;
    color_y=52;
    color_w=128;
    color_h=64;
    cset_x=96;
    cset_y=170;
    cset_h=8;
    cset_spacing=4;
    int ok_button_x=240, ok_button_y=152, ok_button_w=61, ok_button_h=21;
    int cancel_button_x=240, cancel_button_y=184, cancel_button_w=61, cancel_button_h=21;
    int insert_button_x=130, insert_button_y=140, insert_button_w=61, insert_button_h=21;
    int window_xofs=0;
    int window_yofs=0;
    bool just_clicked=false;
    
    if(is_large)
    {
        window_xofs=(zq_screen_w-480)>>1;
        window_yofs=(zq_screen_h-360)>>1;
        hue_x = int(hue_x*1.5);
        hue_y = int(hue_y*1.5);
        hue_h = int(hue_h*1.5);
        hue_x+=window_xofs;
        hue_y+=window_yofs;
        light_x = int(light_x*1.5);
        light_y = int(light_y*1.5);
        light_w = int(light_w*1.5);
        light_x+=window_xofs;
        light_y+=window_yofs;
        sat_x = int(sat_x*1.5);
        sat_y = int(sat_y*1.5);
        sat_x+=window_xofs;
        sat_y+=window_yofs;
        sat_w = int(sat_w*1.5);
        color_x = int(color_x*1.5);
        color_y = int(color_y*1.5);
        color_w =int(color_w*1.5);
        color_h = int(color_h*1.5);
        color_x+=window_xofs;
        color_y+=window_yofs;
        cset_x = int(cset_x*1.5);
        cset_y = int(cset_y*1.5);
        cset_h = int(cset_h*1.5);
        cset_x+=window_xofs;
        cset_y+=window_yofs;
        cset_spacing = int(cset_spacing*1.5);
        ok_button_x = int(ok_button_x*1.5);
        ok_button_y = int(ok_button_y*1.5);
        ok_button_x+=window_xofs;
        ok_button_y+=window_yofs;
        ok_button_w = int(ok_button_w*1.5);
        ok_button_h = int(ok_button_h*1.5);
        cancel_button_x = int(cancel_button_x*1.5);
        cancel_button_y = int(cancel_button_y*1.5);
        cancel_button_x+=window_xofs;
        cancel_button_y+=window_yofs;
        cancel_button_w = int(cancel_button_w*1.5);
        cancel_button_h = int(cancel_button_h*1.5);
        insert_button_x = int(insert_button_x*1.5);
        insert_button_y = int(insert_button_y*1.5);
        insert_button_x+=window_xofs;
        insert_button_y+=window_yofs;
        insert_button_w = int(insert_button_w*1.5);
        insert_button_h = int(insert_button_h*1.5);
    }
    
    custom_vsync();
    scare_mouse();
    
    if(is_large)
        rectfill(screen, 0, 0, screen->w, screen->h, 128);
        
    jwin_draw_win(screen, window_xofs, window_yofs, int(320*(is_large?1.5:1)), int(240*(is_large?1.5:1)), FR_WIN);
    FONT *oldfont=is_large?lfont_l:nfont;
    font=lfont;
    jwin_draw_titlebar(screen, 3+window_xofs, 3+window_yofs, int((320*(is_large?1.5:1))-6), 18, "Edit CSet", true);
    font = oldfont;
    //draw_x_button(screen, 320 - 21, 5, 0);
    load_cset(RAMpal,12,dataset);
    load_cset(RAMpal,14,dataset);
    set_palette_range(RAMpal,0,255,false);
    
    init_colormixer();
    colormixer(color,gray,ratio);
    
    jwin_draw_frame(screen, cset_x-2, cset_y-2, int(128*(is_large?1.5:1)+4), cset_h+4, FR_DEEP);
    
    for(int i=0; i<16; i++)
    {
        rectfill(screen,int((i<<3)*(is_large?1.5:1)+cset_x),cset_y,int((i<<3)*(is_large?1.5:1)+cset_x+cset_h-1),cset_y+cset_h-1,12*16+i);
    }
    
    draw_edit_dataset_specs(index,-1);
    
    draw_text_button(screen,ok_button_x,ok_button_y,ok_button_w,ok_button_h,"OK",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
    draw_text_button(screen,cancel_button_x,cancel_button_y,cancel_button_w,cancel_button_h,"Cancel",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
    draw_text_button(screen,insert_button_x,insert_button_y,insert_button_w,insert_button_h,"Insert",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
    
    unscare_mouse();
    //if(zqwin_scale > 1)
    {
        //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
    }
    //else
    {
        //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
    }
    
    while(gui_mouse_b())
    {
        /* do nothing */
    }
    
    bool bdown=false;
    int doing=0;
    //doing:
    //1=hue
    //2=color index
    //3=saturation
    
    int copy=-1;
    
    int done=0;
    
    do
    {
        rest(4);
        bool setpal=false;
        int x=gui_mouse_x();
        int y=gui_mouse_y();
        
        if(gui_mouse_b()==1 && !bdown)
        {
            just_clicked=true;
            
            if(isinRect(x,y,color_x,color_y,color_x+color_w-1,color_y+color_h-1))
            {
                doing=1; //hue/lightness (color)
            }
            
            if(isinRect(x,y,sat_x,sat_y,sat_x+sat_w-1,sat_y+(is_large?96:64)-1))
            {
                doing=3; //saturation
            }
            
            if(isinRect(x,y,cset_x,cset_y,cset_x+(is_large?192:128)-1,cset_y+(cset_h*2)+cset_spacing+10))
            {
                doing=2; //color index
            }
            
            if(isinRect(x,y,ok_button_x,ok_button_y,ok_button_x+ok_button_w-1,ok_button_y+ok_button_h-1))
            {
                if(do_text_button(ok_button_x,ok_button_y,ok_button_w,ok_button_h,"OK",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
                {
                    done=2;
                }
            }
            
            if(!doing && isinRect(x,y,cancel_button_x,cancel_button_y,cancel_button_x+cancel_button_w-1,cancel_button_y+cancel_button_h-1))
            {
                if(do_text_button(cancel_button_x,cancel_button_y,cancel_button_w,cancel_button_h,"Cancel",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
                {
                    done=1;
                }
            }
            
            if(isinRect(x,y,int(320*(is_large?1.5:1) - 21+window_xofs), 5+window_yofs, int(320*(is_large?1.5:1) - 21 + 15+window_xofs), 5 + 13+window_yofs))
            {
                if(do_x_button(screen, int(320*(is_large?1.5:1) - 21+window_xofs), 5+window_yofs))
                {
                    done=1;
                }
            }
            
            if(isinRect(x,y,insert_button_x,insert_button_y,insert_button_x+insert_button_w-1,insert_button_y+insert_button_h-1))
            {
                if(do_text_button_reset(insert_button_x,insert_button_y,insert_button_w,insert_button_h,"Insert",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
                {
                    RAMpal[14*16+index] = RAMpal[edc];
                    setpal=true;
                }
            }
            
            bdown=true;
        }
        
        if(gui_mouse_b()==1)
        {
            switch(doing)
            {
            case 1: // hue/lightness
                color = vbound(x-color_x,0,is_large?191:127);
                gray = vbound(y-light_y,0,is_large?95:63);
                break;
                
            case 2: // color index
            {
                int new_index=vbound((int)((x-cset_x)/(is_large?1.5:1))>>3,0,15);
                
                if(index!=new_index && !just_clicked)
                {
                    if(index<new_index)
                    {
                        for(int i=index; i<new_index; i++)
                            zc_swap(RAMpal[14*16+i], RAMpal[14*16+i+1]);
                    }
                    else
                    {
                        for(int i=index; i>new_index; i--)
                            zc_swap(RAMpal[14*16+i], RAMpal[14*16+i-1]);
                    }
                    
                    setpal=true;
                }
                
                index = new_index;
                break;
            }
            
            case 3: // saturation
                ratio = vbound(y-sat_y,0,is_large?95:63);
                break;
            }
        }
        
        just_clicked=false;
        
        if(gui_mouse_b()==0)
        {
            bdown=false;
            doing=0;
        }
        
        if(keypressed())
        {
            int k = readkey();
            
            switch(k>>8)
            {
            case KEY_ESC:
                done=1;
                break;
                
            case KEY_ENTER:
                done=2;
                break;
                
            case KEY_LEFT:
            {
                if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
                {
                    zc_swap(RAMpal[14*16+index], RAMpal[14*16+((index-1)&15)]);
                    setpal=true;
                }
                
                index=(index-1)&15;
                break;
            }
            
            case KEY_RIGHT:
            {
                if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
                {
                    zc_swap(RAMpal[14*16+index], RAMpal[14*16+((index+1)&15)]);
                    setpal=true;
                }
                
                index=(index+1)&15;
                break;
            }
            
            case KEY_HOME:
                ++RAMpal[14*16+index].r;
                RAMpal[14*16+index].r&=0x3F;
                setpal=true;
                break;
                
            case KEY_END:
                --RAMpal[14*16+index].r;
                RAMpal[14*16+index].r&=0x3F;
                setpal=true;
                break;
                
            case KEY_UP:
                ++RAMpal[14*16+index].g;
                RAMpal[14*16+index].g&=0x3F;
                setpal=true;
                break;
                
            case KEY_DOWN:
                --RAMpal[14*16+index].g;
                RAMpal[14*16+index].g&=0x3F;
                setpal=true;
                break;
                
            case KEY_PGUP:
                ++RAMpal[14*16+index].b;
                RAMpal[14*16+index].b&=0x3F;
                setpal=true;
                break;
                
            case KEY_PGDN:
                --RAMpal[14*16+index].b;
                RAMpal[14*16+index].b&=0x3F;
                setpal=true;
                break;
                
            case KEY_C:
                copy=index;
                break;
                
            case KEY_V:
                if(copy>=0)
                {
                    RAMpal[14*16+index]=RAMpal[14*16+copy];
                    setpal=true;
                    copy=-1;
                }
                
                break;
                
            case KEY_M:
                if(copy>=0)
                {
                    RGB temp=RAMpal[14*16+index];
                    RAMpal[14*16+index]=RAMpal[14*16+copy];
                    RAMpal[14*16+copy]=temp;
                    setpal=true;
                    copy=-1;
                }
                
                break;
                
            case KEY_I:
            case KEY_INSERT:
                RAMpal[14*16+index]=RAMpal[edc];
                setpal=true;
                break;
                
            case KEY_F1:
                doHelp(jwin_pal[jcTEXTFG],jwin_pal[jcTEXTBG]);
                break;
                
            case KEY_S:
                onSnapshot();
                break;
                
            default:
                switch(k&255)
                {
                case '4':
                {
                    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
                    {
                        zc_swap(RAMpal[14*16+index], RAMpal[14*16+((index-1)&15)]);
                        setpal=true;
                    }
                    
                    index=(index-1)&15;
                    break;
                }
                
                case '6':
                {
                    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
                    {
                        zc_swap(RAMpal[14*16+index], RAMpal[14*16+((index+1)&15)]);
                        setpal=true;
                    }
                    
                    index=(index+1)&15;
                    break;
                }
                
                case '7':
                    ++RAMpal[14*16+index].r;
                    RAMpal[14*16+index].r&=0x3F;
                    setpal=true;
                    break;
                    
                case '1':
                    --RAMpal[14*16+index].r;
                    RAMpal[14*16+index].r&=0x3F;
                    setpal=true;
                    break;
                    
                case '8':
                    ++RAMpal[14*16+index].g;
                    RAMpal[14*16+index].g&=0x3F;
                    setpal=true;
                    break;
                    
                case '2':
                    --RAMpal[14*16+index].g;
                    RAMpal[14*16+index].g&=0x3F;
                    setpal=true;
                    break;
                    
                case '9':
                    ++RAMpal[14*16+index].b;
                    RAMpal[14*16+index].b&=0x3F;
                    setpal=true;
                    break;
                    
                case '3':
                    --RAMpal[14*16+index].b;
                    RAMpal[14*16+index].b&=0x3F;
                    setpal=true;
                    break;
                }
            }
        }
        
        if(gui_mouse_b() && (doing==1 || doing==3))
        {
            colormixer(color,gray,ratio);
        }
        else
        {
            custom_vsync();
            scare_mouse();
            
            if(setpal)
                set_palette_range(RAMpal,14*16,15*16,false);
                
            draw_edit_dataset_specs(index,copy);
            unscare_mouse();
            SCRFIX();
        }
        
        //if(zqwin_scale > 1)
        {
            //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        // else
        {
            //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
        
    }
    while(!done);
    
    if(done==2)
    {
        get_cset(dataset,14,RAMpal);
        saved=false;
    }
    
    font = nfont;
    
    //  RAMpal = holdpal;
    
    memcpy(RAMpal, holdpal, sizeof(holdpal));
    
    set_palette(RAMpal);
    
    while(gui_mouse_b())
    {
        /* do nothing */
    }
}

int pal_index(RGB *pal,RGB c)
{
    for(int i=0; i<256; i++)
        if(pal[i].r==c.r && pal[i].g==c.g && pal[i].b==c.b)
            return i;
            
    return -1;
}

void grab_dataset(int dataset)
{
    int row=0;
    static int palx=176;
    static int paly=64;
    int imagex=0;
    int imagey=0;
    int buttonx=184;
    int buttony=195;
    int filenamex=0;
    int filenamey=232;
    
    if(is_large)
    {
        palx=665;
        paly=354;
        imagex=4;
        imagey=4;
        buttonx=570;
        buttony=521;
        filenamex=4;
        filenamey=583;
    }
    
    PALETTE tmp;
    
    if(!pic && load_the_pic(&pic,picpal))
        return;
        
    get_palette(imagepal);
    
    create_rgb_table(&rgb_table, imagepal, NULL);
    rgb_map = &rgb_table;
    create_color_table(&imagepal_table, RAMpal, return_RAMpal_color, NULL);
    
    int jwin_pal2[jcMAX];
    memcpy(jwin_pal2, jwin_pal, sizeof(int)*jcMAX);
    
    jwin_pal[jcBOX]    =imagepal_table.data[0][jwin_pal[jcBOX]];
    jwin_pal[jcLIGHT]  =imagepal_table.data[0][jwin_pal[jcLIGHT]];
    jwin_pal[jcMEDLT]  =imagepal_table.data[0][jwin_pal[jcMEDLT]];
    jwin_pal[jcMEDDARK]=imagepal_table.data[0][jwin_pal[jcMEDDARK]];
    jwin_pal[jcDARK]   =imagepal_table.data[0][jwin_pal[jcDARK]];
    jwin_pal[jcBOXFG]  =imagepal_table.data[0][jwin_pal[jcBOXFG]];
    jwin_pal[jcTITLEL] =imagepal_table.data[0][jwin_pal[jcTITLEL]];
    jwin_pal[jcTITLER] =imagepal_table.data[0][jwin_pal[jcTITLER]];
    jwin_pal[jcTITLEFG]=imagepal_table.data[0][jwin_pal[jcTITLEFG]];
    jwin_pal[jcTEXTBG] =imagepal_table.data[0][jwin_pal[jcTEXTBG]];
    jwin_pal[jcTEXTFG] =imagepal_table.data[0][jwin_pal[jcTEXTFG]];
    jwin_pal[jcSELBG]  =imagepal_table.data[0][jwin_pal[jcSELBG]];
    jwin_pal[jcSELFG]  =imagepal_table.data[0][jwin_pal[jcSELFG]];
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    gui_mg_color=jwin_pal[jcMEDDARK];
    jwin_set_colors(jwin_pal);
    
    get_bw(picpal,pblack,pwhite);
    int bg = gui_bg_color;
    int fg = gui_fg_color;
    gui_bg_color = pblack;
    gui_fg_color = pwhite;
    
    char fname[13];
    extract_name(imagepath,fname,FILENAME8_3);
    
    draw_bw_mouse(pwhite, MOUSE_BMP_NORMAL, MOUSE_BMP_BLANK);
    scare_mouse();
    clear_bitmap(screen2);
    set_mouse_sprite(mouse_bmp[MOUSE_BMP_BLANK][0]);
    unscare_mouse();
    set_palette(picpal);
    
    bool redraw=true;
    bool reload=false;
    int done=0;
    int f=0;
    FONT *fnt = font;
    
    if(is_large)
        font = lfont_l;
        
    do
    {
        rest(1);
        int x=gui_mouse_x();
        int y=gui_mouse_y();
        
        custom_vsync();
        
        if(reload)
        {
            reload=false;
            
            if(load_the_pic(&pic,picpal)==2)
                done=1;
            else
            {
                clear_bitmap(screen2);
                set_palette(picpal);
                redraw=true;
            }
        }
        
        if(redraw)
        {
            redraw=false;
            scare_mouse();
            clear_to_color(screen2,is_large?jwin_pal[jcBOX]:pblack);
            
            if(is_large)
            {
                jwin_draw_frame(screen2,imagex-2,imagey-2,658,551,FR_DEEP);
                rectfill(screen2, imagex, imagey, imagex+654-1, imagey+547-1, jwin_pal[jcBOXFG]);
                jwin_draw_frame(screen2,palx-3,paly-3,134,134,FR_DEEP);
            }
            
            blit(pic,screen2,0,0,imagex,imagey,pic->w,pic->h);
            textout_ex(screen2,is_large?lfont_l:font,fname,filenamex,filenamey,is_large?jwin_pal[jcBOXFG]:pwhite,is_large?jwin_pal[jcBOX]:pblack);
            draw_text_button(screen2,buttonx,buttony+(is_large?36:24),(is_large?90:60),(is_large?31:21),"File",pblack,pwhite,0,true);
            draw_text_button(screen2,buttonx+(is_large?114:76),buttony,
                             (is_large?90:60),(is_large?31:21),"OK",pblack,pwhite,0,true);
            draw_text_button(screen2,buttonx+(is_large?114:76),buttony+(is_large?36:24),
                             (is_large?90:60),(is_large?31:21),"Cancel",pblack,pwhite,0,true);
            unscare_mouse();
        }
        
        if((gui_mouse_b()&1) && isinRect(x,y,palx,paly,palx+127,paly+127))
        {
            row=((y-paly)>>3);
        }
        
        if((gui_mouse_b()&1) && isinRect(x,y,buttonx,buttony+(is_large?36:24),buttonx+(is_large?90:60),buttony+(is_large?36+31:24+21)))
        {
            if(do_text_button(buttonx,buttony+(is_large?36:24),(is_large?90:60),(is_large?31:21),"File",pblack,pwhite,true))
            {
                reload=true;
            }
        }
        
        if((gui_mouse_b()&1) && isinRect(x,y,buttonx+(is_large?114:76),buttony,buttonx+(is_large?114+90:76+60),buttony+(is_large?31:21)))
        {
            if(do_text_button(buttonx+(is_large?114:76),buttony,(is_large?90:60),(is_large?31:21),"OK",pblack,pwhite,true))
            {
                done=2;
            }
        }
        
        if((gui_mouse_b()&1) && isinRect(x,y,buttonx+(is_large?114:76),buttony+(is_large?36:24),buttonx+(is_large?114+90:76+60),buttony+(is_large?36+31:24+21)))
        {
            if(do_text_button(buttonx+(is_large?114:76),buttony+(is_large?36:24),(is_large?90:60),(is_large?31:21),"Cancel",pblack,pwhite,true))
            {
                done=1;
            }
        }
        
        if(keypressed())
        {
            switch(readkey()>>8)
            {
            case KEY_ESC:
                done=1;
                break;
                
            case KEY_ENTER:
                done=2;
                break;
                
            case KEY_SPACE:
                reload=true;
                break;
                
            case KEY_UP:
                row=(row-1)&15;
                break;
                
            case KEY_DOWN:
                row=(row+1)&15;
                break;
                
            case KEY_LEFT:
            case KEY_RIGHT:
                if(!is_large)
                {
                    palx = 192-palx;
                    redraw=true;
                }
                
                break;
                
            case KEY_TAB:
                //        tmp = picpal;
                memcpy(tmp,picpal,sizeof(picpal));
                
                for(int i=0; i<16; i++)
                    tmp[(row<<4)+i] = invRGB(tmp[(row<<4)+i]);
                    
                for(int i=0; i<12; i++)
                {
                    custom_vsync();
                    
                    if(i&2)
                        set_palette(picpal);
                    else
                        set_palette(tmp);
                        
                    //if(zqwin_scale > 1)
                    {
                        //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
                    }
                    //else
                    {
                        //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
                    }
                }
                
                break;
            }
        }
        
        scare_mouse();
        
        for(int i=0; i<256; i++)
        {
            int x2=((i&15)<<3)+palx;
            int y2=((i>>4)<<3)+paly;
            rectfill(screen2,x2,y2,x2+7,y2+7,i);
        }
        
        ++f;
        rect(screen2,palx-1,paly-1,palx+128,paly+128,is_large?pblack:pwhite);
        rect(screen2,palx-1,(row<<3)+paly-1,palx+128,(row<<3)+paly+8,(f&2)?pwhite:pblack);
        blit(screen2, screen, 0, 0, 0, 0, screen->w, screen->h);
        unscare_mouse();
        SCRFIX();
        
        //if(zqwin_scale > 1)
        {
            //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        //else
        {
            //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
    }
    while(!done);
    
    if(done==2)
        get_cset(dataset,row,picpal);
        
    font = fnt;
    
    gui_bg_color = bg;
    gui_fg_color = fg;
    
    while(gui_mouse_b())
    {
        /* do nothing */
    }
    
    scare_mouse();
    
    set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
    clear_to_color(mouse_bmp[MOUSE_BMP_BLANK][0],0);
    memcpy(jwin_pal, jwin_pal2, sizeof(int)*jcMAX);
    
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    gui_mg_color=jwin_pal[jcMEDDARK];
    jwin_set_colors(jwin_pal);
    
    rgb_map = &zq_rgb_table;
    unscare_mouse();
}

byte cset_hold[15][16*3];
byte cset_hold_cnt;
bool cset_ready = false;
int cset_count,cset_first;
PALETTE pal,undopal;

//#define FLASH 243
//byte rc[16] = {253,248,0,0,0,0,0,246,247,249,250,251,252,240,255,254};

void undo_pal()
{
    for(int i=0; i<cset_count; i++)
        get_cset(cset_first+i,i,undopal);
        
    //  undopal=pal;
    memcpy(undopal,pal,sizeof(pal));
}

void calc_dark(int first)
{
    //  undopal=pal;
    memcpy(undopal,pal,sizeof(pal));
    
    PALETTE tmp;
    //  tmp = black_palette;
    memcpy(tmp,black_palette,sizeof(black_palette));
    
    fade_interpolate(pal,black_palette,tmp,16,0,47);
    
    for(int i=0; i<48; i++)
        tmp[i+64] = tmp[i];
        
    fade_interpolate(pal,black_palette,tmp,32,0,47);
    
    for(int i=0; i<48; i++)
        tmp[i+112] = tmp[i];
        
    tmp[160+3] = tmp[3];
    
    for(int i=4; i<11; i++)
        get_cset(first+i,i,tmp);
}

static DIALOG cycle_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc, 64,   48,   192+1,  152+1,  0,  0,  0,       D_EXIT,          0,             0, (void *) "Palette Cycles", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_button_proc,     90,   176,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     170,  176,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_text_proc,       152,  72,   96,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "1   2   3", NULL, NULL },
    { jwin_text_proc,       88,   88,   56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "CSet:", NULL, NULL },
    { jwin_text_proc,       88,   104,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "First:", NULL, NULL },
    { jwin_text_proc,       88,   120,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Last:", NULL, NULL },
    { jwin_text_proc,       88,   136,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Count:", NULL, NULL },
    { jwin_text_proc,       88,   152,  56,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Speed:", NULL, NULL },
    // 10
    { jwin_edit_proc,       152,  88,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    152,  104,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    152,  120,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       152,  136,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       152,  152,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    
    { jwin_edit_proc,       184,  88,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    184,  104,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    184,  120,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       184,  136,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       184,  152,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    
    { jwin_edit_proc,       216,  88,   24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    216,  104,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_hexedit_proc,    216,  120,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          1,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       216,  136,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_edit_proc,       216,  152,  24,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          3,             0,       NULL, NULL, NULL },
    
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void edit_cycles(int level)
{
    char buf[15][8];
    palcycle cycle_none[1][3];  //create a null palette cycle here. -Z
	memset(cycle_none, 0, sizeof(cycle_none)); 
    for(int i=0; i<3; i++)
    {
        palcycle c = ( level < 256 ) ? misc.cycles[level][i] : cycle_none[0][i]; //Only level palettes 0 through 255 have valid data in 2.50.x. -Z
        sprintf(buf[i*5  ],"%d",c.first>>4);
        sprintf(buf[i*5+1],"%X",c.first&15);
        sprintf(buf[i*5+2],"%X",c.count&15);
        sprintf(buf[i*5+3],"%d",c.count>>4);
        sprintf(buf[i*5+4],"%d",c.speed);
    }
    
    for(int i=0; i<15; i++)
        cycle_dlg[i+10].dp = buf[i];
        
    if(is_large)
        large_dialog(cycle_dlg);
        
    if(zc_popup_dialog(cycle_dlg,3)==2)
    {
        saved=false;
        reset_pal_cycling();
        
        for(int i=0; i<3; i++)
        {
            palcycle c;
            c.first = (atoi(buf[i*5])&7)<<4;
            c.first += xtoi(buf[i*5+1])&15;
            c.count =  xtoi(buf[i*5+2])&15;
            c.count += (atoi(buf[i*5+3])&15)<<4;
            c.speed =  atoi(buf[i*5+4]);
            misc.cycles[level][i] = c;
        }
    }
}

void draw_cset_proc(DIALOG *d)
{
    int d1 = zc_min(d->d1, d->d2);
    int d2 = zc_max(d->d1, d->d2);
    
    
    d_bitmap_proc(MSG_DRAW,d,0);
    //  text_mode(d->bg);
    
    //  rect(screen,d->x,(d1<<3)+d->y,d->x+d->w-1,(d2<<3)+d->y+7,FLASH);
    rect(screen,d->x,int((d1<<3)*(is_large?1.5:1)+d->y),d->x+d->w-1,int((d2<<3)*(is_large?1.5:1)+d->y+(is_large?11:7)),dvc(0)); //highlights currently selected cset
    int drc;
    
    if((d->flags & D_GOTFOCUS))
        drc = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
    else
        drc = d->bg;
        
    dotted_rect(d->x-1, d->y-1, d->x+d->w, d->y+d->h, drc, d->bg);
}

int d_cset_proc(int msg,DIALOG *d,int c)
{
    switch(msg)
    {
    case MSG_START:
        d->d2 = d->d1;
        break;
        
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
        
    case MSG_DRAW:
        draw_cset_proc(d);
        break;
        
    case MSG_CLICK:
    {
        bool dragging=false;
        int src=0;
        int x;
        int y;
        
        // Start dragging?
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
            x=gui_mouse_x();
            y=gui_mouse_y();
            
            if(isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
            {
                dragging=true;
                src=vbound((int)((y-d->y) / (is_large?1.5:1))>>3,0,15) * 16 +
                    vbound((int)((x-d->x) / (is_large?1.5:1))>>3,0,15);
            }
        }
        
        do
        {
            x=gui_mouse_x();
            y=gui_mouse_y();
            
            if(!dragging && isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
            {
                d->d2 = vbound((int)((y-d->y)/(is_large?1.5:1))>>3,0,15);
                
                if(!(key_shifts&KB_SHIFT_FLAG))
                    d->d1 = d->d2;
            }
            
            custom_vsync();
            scare_mouse();
            draw_cset_proc(d);
            unscare_mouse();
            //sniggles
            //        ((RGB*)d->dp3)[243]=((RGB*)d->dp3)[rc[(fc++)&15]];
            //        set_palette_range(((RGB*)d->dp3),FLASH,FLASH,false);
            ((RGB*)d->dp3)[dvc(0)]=((RGB*)d->dp3)[rand()%14+dvc(1)];
            set_palette_range(((RGB*)d->dp3),dvc(0),dvc(0),false);
            //if(zqwin_scale > 1)
            {
                //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            //else
            {
                //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
        }
        while(gui_mouse_b());
        
        if(dragging && isinRect(x,y,d->x,d->y,d->x+d->w-1,d->y+d->h-1))
        {
            int dest=vbound((int)((y-d->y) / (is_large?1.5:1))>>3,0,15) * 16 +
                     vbound((int)((x-d->x) / (is_large?1.5:1))>>3,0,15);
                     
            if(src!=dest)
            {
                memcpy(undopal,pal,sizeof(pal));
                
                if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
                {
                    for(int i=0; i<3; i++)
                        zc_swap(*(((byte*)d->dp2)+dest*3+i), *(((byte*)d->dp2)+src*3+i));
                }
                else
                {
                    for(int i=0; i<3; i++)
                        *(((byte*)d->dp2)+dest*3+i) = *(((byte*)d->dp2)+src*3+i);
                }
                
                for(int i=0; i<cset_count; i++)
                    load_cset(pal,i,cset_first+i);
                    
                set_palette(pal);
                saved=false;
            }
        }
    }
    break;
    
    case MSG_CHAR:
    {
        int shift = (key_shifts&KB_SHIFT_FLAG);
        int k=c>>8;
        
        switch(k)
        {
        case KEY_UP:
            if(d->d2>0) --d->d2;
            
            if(!shift) d->d1 = d->d2;
            
            break;
            
        case KEY_DOWN:
            if(d->d2<((int)ceil((d->h)/(is_large?1.5:1))>>3)-1)
                ++d->d2;
                
            if(!shift) d->d1 = d->d2;
            
            break;
            
        case KEY_PGUP:
            d->d2=0;
            
            if(!shift) d->d1 = d->d2;
            
            break;
            
        case KEY_PGDN:
            d->d2=((int)ceil((d->h)/(is_large?1.5:1))>>3)-1;
            
            if(!shift) d->d1 = d->d2;
            
            break;
            
        case KEY_C:
            cset_hold_cnt=0;
            
            for(int row=0; row <= abs(d->d1 - d->d2); row++)
            {
                int d1 = zc_min(d->d1,d->d2);
                ++cset_hold_cnt;
                
                for(int i=0; i<16*3; i++)
                    cset_hold[row][i] = *(((byte*)d->dp2)+CSET(d1+row)*3+i);
            }
            
            cset_ready=true;
            break;
            
            
        case KEY_V:
            if(cset_ready)
            {
                //         undopal=pal;
                memcpy(undopal,pal,sizeof(pal));
                int d1 = zc_min(d->d1,d->d2);
                
                for(int row=0; row<cset_hold_cnt && d1+row<cset_count; row++)
                {
                    for(int i=0; i<16*3; i++)
                        *(((byte*)d->dp2)+CSET(d1+row)*3+i) = cset_hold[row][i];
                }
                
                for(int i=0; i<cset_count; i++)
                    load_cset(pal,i,cset_first+i);
                    
                set_palette(pal);
                saved=false;
            }
            
            break;
            
        case KEY_U:
            undo_pal();
            
            for(int i=0; i<cset_count; i++)
                load_cset(pal,i,cset_first+i);
                
            set_palette(pal);
            break;
            
        default:
            return D_O_K;
        }
        
        custom_vsync();
        scare_mouse();
        draw_cset_proc(d);
        unscare_mouse();
        //if(zqwin_scale > 1)
        {
            //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        //else
        {
            //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
        return D_USED_CHAR;
    }
    }
    
    return D_O_K;
}

byte mainpal_csets[30]    = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 11,11,12,12,12,11, 10,10,10,12,10,10,10,10,9 };
byte levelpal_csets[26]   = { 2,3,4,9,2,3,4,2,3,4, 2, 3, 4,       15,15,15,15, 7,7,7, 8,8,8, 0,0,0 };
byte levelpal2_csets[26]  = { 2,3,4,9,2,0,1,2,3,4, 5, 6, 7,       15,15,15,15, 8,  9,9,9,9,9,9,9,9 };
byte spritepal_csets[30]  = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };
byte spritepal2_csets[30] = { 15,16,17,18,19,20,21,22,23,24,25,26,27,28,29, 14,14,14,14,14,14, 14,14,14,14,14,14,14,14,14 };

static DIALOG colors_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     40,   16,   240+1,  216+1,  jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       D_EXIT,          0,             0,       NULL, NULL, NULL },
    
    { jwin_frame_proc,   103,  48,   96,  256,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_DEEP,             0,       NULL, NULL, NULL },
    { d_cset_proc,       106,  51,   256,  96,   jwin_pal[jcBOXFG],  jwin_pal[jcLIGHT],  0,       0,          0,             0,       NULL, NULL, NULL },
    
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_rtext_proc,      102,   0,    16,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
    
    { d_dummy_proc,       104,  39,   128,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "0123456789ABCDEF", NULL, NULL },
    // 19
    
    { jwin_button_proc,     60,   178,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'e',     D_EXIT,     0,             0, (void *) "&Edit", NULL, NULL },
    { jwin_button_proc,     130,  178,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'g',     D_EXIT,     0,             0, (void *) "&Grab", NULL, NULL },
    { jwin_button_proc,     200,  178,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       D_EXIT,     0,             0, (void *) "Cycle", NULL, NULL },
    { jwin_button_proc,     244,  152,  25,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'u',     D_EXIT,     0,             0, (void *) "&U", NULL, NULL },
    // 23
    
    { jwin_button_proc,     60,   204,  131,  21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  'l',     D_EXIT,     0,             0, (void *) "&Load to CS 9", NULL, NULL },
    
    { jwin_button_proc,     200,  204,  61,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  27,      D_EXIT,     0,             0, (void *) "Done", NULL, NULL },
    { jwin_rtext_proc,      60,   164,  48,   8,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Name: ", NULL, NULL },
    { jwin_edit_proc,       88,  160, 136,   16,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          16,            0,       NULL, NULL, NULL },
    { jwin_ctext_proc,       110,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "0", NULL, NULL },
    { jwin_ctext_proc,       118,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "1", NULL, NULL },
    { jwin_ctext_proc,       126,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "2", NULL, NULL },
    { jwin_ctext_proc,       134,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "3", NULL, NULL },
    { jwin_ctext_proc,       142,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "4", NULL, NULL },
    { jwin_ctext_proc,       150,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "5", NULL, NULL },
    { jwin_ctext_proc,       158,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "6", NULL, NULL },
    { jwin_ctext_proc,       166,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "7", NULL, NULL },
    { jwin_ctext_proc,       174,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "8", NULL, NULL },
    { jwin_ctext_proc,       182,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "9", NULL, NULL },
    { jwin_ctext_proc,       190,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "A", NULL, NULL },
    { jwin_ctext_proc,       198,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "B", NULL, NULL },
    { jwin_ctext_proc,       206,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "C", NULL, NULL },
    { jwin_ctext_proc,       214,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "D", NULL, NULL },
    { jwin_ctext_proc,       222,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "E", NULL, NULL },
    { jwin_ctext_proc,       230,  39,   0,  8,    jwin_pal[jcBOXFG],   jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "F", NULL, NULL },
    //43
    { jwin_button_proc,     244,  125,  25,   21,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  's',     D_EXIT,     0,             0, (void *) "Save", NULL, NULL },
    
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int EditColors(const char *caption,int first,int count,byte *label)
{

    char tempstuff[17];
    cset_first=first;
    cset_count=count;
    
    for(int i=240; i<256; i++)
    {
        pal[i] = RAMpal[i];
    }
    
    go();
    
    int bw = 128;
    int bh = count*8;
    
    if(colors_dlg[0].d1)
    {
        bw = (is_large?192:128);
        bh = count*(is_large?12:8);
    }
    
    BITMAP *bmp = create_bitmap_ex(8,(is_large?192:128),count*(is_large?12:8));
    
    if(!bmp)
        return 0;
        
    for(int i=0; i<16*count; i++)
    {
        int x=int(((i&15)<<3)*(is_large?1.5:1));
        int y=int(((i>>4)<<3)*(is_large?1.5:1));
        rectfill(bmp,x,y,x+(is_large ? 15:7),y+(is_large ? 15:7),i);
    }
    
    colors_dlg[2].dp = bmp;
    colors_dlg[2].w  = bw;
    colors_dlg[2].h  = bh;
    colors_dlg[2].d1 = colors_dlg[2].d1 = 0;
    colors_dlg[2].dp2 = colordata+CSET(first)*3;
    colors_dlg[2].dp3 = pal;
    colors_dlg[1].w  = bw + 6;
    colors_dlg[1].h  = bh + 6;
    colors_dlg[21].proc = (count==pdLEVEL) ? jwin_button_proc : d_dummy_proc;
    //if the palette is > 255, disable button [21]
    colors_dlg[21].dp   = get_bit(quest_rules,qr_FADE) ? (void *) "Cycle" : (void *) "Dark";
    colors_dlg[26].dp   =  tempstuff;
    colors_dlg[25].x    =(count==pdLEVEL)?colors_dlg[0].x+60:colors_dlg[0].x+12;
    colors_dlg[25].w    =(count==pdLEVEL)?48:0;
    colors_dlg[25].dp   =(count==pdLEVEL)?(void *) "Name: ":(void *) " ";
    colors_dlg[26].proc =(count==pdLEVEL) ? jwin_edit_proc : d_dummy_proc;
    colors_dlg[26].x    =(count==pdLEVEL)?colors_dlg[0].x+63:colors_dlg[0].x+12;
    colors_dlg[26].w    =(count==pdLEVEL)?134:0;
    colors_dlg[26].d1   =(count==pdLEVEL)?16:0;
    colors_dlg[26].dp   =(count==pdLEVEL)?palnames[(first-poLEVEL)/pdLEVEL]:NULL;
    
    //char (*buf)[4]= new char[count][4];
    char buf[50][4];
    
    for(int i=0; i<15; i++)
    {
        if(i<count)
        {
            sprintf(buf[i],"%2d",label[i]);
        }
        else
        {
            buf[i][0]=0;
        }
        
        colors_dlg[3+i].dp=buf[i];
        colors_dlg[3+i].y=int(((i<<3)*((is_large && colors_dlg[0].d1)?1.5:1))+colors_dlg[0].y+36*((is_large && colors_dlg[0].d1)?1.5:1));
        //sniggles
        //    colors_dlg[3+i].fg=rc[label[i+count]];
    }
    
    colors_dlg[0].dp  = (void *)caption;
    colors_dlg[0].dp2 = lfont;
    
    for(int i=0; i<count; i++)
    {
        load_cset(pal,i,i+first);
    }
    
    memcpy(undopal,pal,sizeof(pal));
    int ret=0;
    
    do
    {
        rest(4);
        
        for(int i=0; i<count; i++)
        {
            load_cset(pal,i,i+first);
        }
        
        scare_mouse();
        clear_to_color(screen,0);
        set_palette(pal);
        unscare_mouse();
        colors_dlg[19].flags =
            colors_dlg[20].flags =
                colors_dlg[23].flags = D_EXIT;
                
        if(is_large)
        {
            if(!colors_dlg[0].d1)
            {
                colors_dlg[2].x  += 0;
                colors_dlg[2].y  -= 1;
            }
            
            large_dialog(colors_dlg);
        }
        
        DIALOG_PLAYER *p = init_dialog(colors_dlg,2);
        bool enable = true;
        
        while(update_dialog(p))
        {
            custom_vsync();
            //sniggles
            //      pal[FLASH]=pal[rc[(fc++)&15]];
            pal[dvc(0)]=pal[rand()%14+dvc(1)];
            set_palette_range(pal,dvc(0),dvc(0),false);
            
            bool en = (colors_dlg[2].d1 == colors_dlg[2].d2);
            
            if(en!=enable)
            {
                colors_dlg[19].flags =
                    colors_dlg[20].flags =
                        colors_dlg[23].flags = en  ? D_EXIT : D_DISABLED;
                broadcast_dialog_message(MSG_DRAW,0);
                enable = en;
            }
            
            //if(zqwin_scale > 1)
            {
                //stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            //else
            {
                //blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
        }
        
        ret = shutdown_dialog(p);
        
        if(ret==19)
        {
            memcpy(undopal,pal,sizeof(pal));
            edit_dataset(first+colors_dlg[2].d2);
        }
        
        if(ret==20)
        {
            memcpy(undopal,pal,sizeof(pal));
            grab_dataset(first+colors_dlg[2].d2);
        }
        
        if(ret==21)
        {
		int curpal = (first/pdLEVEL+poLEVEL)-10; 
		
		
		    if(!get_bit(quest_rules,qr_FADE))
		    {
			calc_dark(first);
		    }
		    else
		    {
			if ( curpal < 256 ) //don't display cycle data for palettes 256 through 511. They don't have valid cycle data. 
				edit_cycles((first-poLEVEL)/pdLEVEL);
			else jwin_alert("Notice","Palettes above 0xFF do not have Palette Cycles",NULL,NULL,"O&K",NULL,'k',0,lfont);
		    }
		
        }
        
        if(ret==22)
        {
            undo_pal();
        }
	if ( ret == 43 )
	{
		if(getname("Export Subscreen (.sub)",".png",NULL,datapath,false))
		{
			//bool cancel;
			//char buf[80],buf2[80],
			char name[13];
			extract_name(temppath,name,FILENAME8_3);
			save_bitmap(temppath, bmp, pal);
			//save_bitmap("current_lvl_pal.png", bmp, pal);
		}
	}
    }
    while(ret<23&&ret!=0);
    
    while(gui_mouse_b())
    {
        /* do nothing */
    }
    
    clear_to_color(screen,vc(0));
    set_palette(RAMpal);
    
    loadlvlpal(Color);
    
    if(ret==23)
    {
        load_cset(RAMpal,9,first+colors_dlg[2].d2);
        set_pal();
    }
    
    saved=false; //It's just easier this way :)
    //  gui_fg_color = vc(14);
    //  gui_bg_color = vc(1);
    //  gui_mg_color = vc(9);
    
    comeback();
    destroy_bitmap(bmp);
    //delete[] buf;
    return int(ret==23);
}

int onColors_Main()
{
    int l9 = EditColors("Main Palette",0,pdFULL,mainpal_csets);
    
    // copy main to level 0
    int di = CSET(poLEVEL)*3;
    int si = CSET(2)*3;
    
    for(int i=0; i<CSET(3)*3; i++)
        colordata[di++] = colordata[si++];
        
    si = CSET(9)*3;
    
    for(int i=0; i<16*3; i++)
        colordata[di++] = colordata[si++];
        
    loadlvlpal(Color);
    
    if(l9)
    {
        load_cset(RAMpal,9,colors_dlg[2].d2);
        set_pal();
    }
    
    return D_O_K;
}

void copyPal(int src, int dest)
{
    int srcStart=CSET(src*pdLEVEL+poLEVEL)*3;
    int destStart=CSET(dest*pdLEVEL+poLEVEL)*3;
    
    for(int i=0; i<pdLEVEL*16*3; i++)
        colordata[destStart+i]=colordata[srcStart+i];
}

int onColors_Levels()
{
    int cycle = get_bit(quest_rules,qr_FADE);
    int index=Map.getcolor();
    
    while((index=select_data("Select Level",index,levelnumlist,"Edit","Done",lfont, copyPal))!=-1)
    {
        char buf[40];
        sprintf(buf,"Level %X Palettes",index);
        int l9 = EditColors(buf,index*pdLEVEL+poLEVEL,pdLEVEL,cycle?levelpal2_csets:levelpal_csets);
        setup_lcolors();
        
        if(index==0)
        {
            // copy level 0 to main
            int si = CSET(poLEVEL)*3;
            int di = CSET(2)*3;
            
            for(int i=0; i<CSET(3)*3; i++)
                colordata[di++] = colordata[si++];
                
            di = CSET(9)*3;
            
            for(int i=0; i<16*3; i++)
                colordata[di++] = colordata[si++];
        }
        
        loadlvlpal(Color);
        
        if(l9)
        {
            load_cset(RAMpal,9,index*pdLEVEL+poLEVEL+colors_dlg[2].d2);
            set_pal();
        }
    }
    
    return D_O_K;
}

int onColors_Sprites()
{
    int index;
    
    do
    {
        index = jwin_alert3("Edit Palette", "Select an extra sprite","palette set to edit",NULL,"&1","&2","&Done",'1','2','d', lfont);
        
        if(index==1)
            EditColors("Extra Sprite Palettes 1",newerpoSPRITE,15,spritepal_csets);
            
        if(index==2)
            EditColors("Extra Sprite Palettes 2",newerpoSPRITE+15,15,spritepal2_csets);
    }
    while(index==1 || index==2);
    
    return D_O_K;
}


void center_zq_cset_dialogs()
{
    jwin_center_dialog(cycle_dlg);
    jwin_center_dialog(colors_dlg);
}

