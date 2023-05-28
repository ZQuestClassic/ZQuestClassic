//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include "base/gui.h"
#include "subscr.h"
#include "zq/zq_subscr.h"
#include "jwin.h"
#include "zq/zquest.h"
#include "base/zsys.h"
#include "zq/zq_misc.h"
#include "tiles.h"
#include "qst.h"
#include "init.h"
#include <assert.h>
#include "dialog/info.h"
#include "dialog/subscr_props.h"

#ifndef _MSC_VER
#include <strings.h>
#endif
#include <string.h>

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

extern void large_dialog(DIALOG *d);
extern void large_dialog(DIALOG *d, float RESIZE_AMT);

int32_t curr_subscreen_object;
char *str_oname;
subscreen_group *css;
bool sso_selection[MAXSUBSCREENITEMS];
static int32_t ss_propCopySrc=-1;

gamedata *game;

extern int32_t bii_cnt;

void delete_subscreen(int32_t subscreenidx);


const char *colortype_str[14] =
{
    "Misc. Color", "Sys. Color", "CSet 0", "CSet 1", "CSet 2", "CSet 3", "CSet 4", "CSet 5", "CSet 6", "CSet 7", "CSet 8", "CSet 9", "CSet 10", "CSet 11"
};

const char *colortypelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 14;
        return NULL;
    }
    
    return colortype_str[index];
}

int32_t d_cs_color_proc(int32_t msg,DIALOG *d,int32_t)
{
    int32_t w=(d->w-4)/16;
    
    switch(msg)
    {
    case MSG_DRAW:
        //rectfill(screen, d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, jwin_pal[jcBOX]);
        //top blank part
        rectfill(screen, d->x, d->y, d->x+(w*16)+3, d->y+1, jwin_pal[jcBOX]);
        jwin_draw_frame(screen,d->x,d->y+2,w*16+4, d->h-4, FR_DEEP);
        
        for(int32_t i=0; i<16; ++i)
        {
            rectfill(screen, d->x+2+(w*i), d->y+4, d->x+2+(w*(i+1))-1, d->y+d->h-5, ((d-2)->d1-2)*16+i);
        }
        
        // right end
        rectfill(screen, d->x+(w*16)+4, d->y, d->x+d->w-1, d->y+d->h-1, jwin_pal[jcBOX]);
        // bottom part
        rectfill(screen, d->x, d->y+d->h-2, d->x+(w*16)+3, d->y+d->h-1, jwin_pal[jcBOX]);
        
        //indicator lines
        hline(screen, d->x+2+(w*d->d1), d->y, d->x+2+(w*(d->d1+1))-1, jwin_pal[jcBOXFG]);
        hline(screen, d->x+2+(w*d->d1), d->y+d->h-1, d->x+2+(w*(d->d1+1))-1, jwin_pal[jcBOXFG]);
        
        break;
        
    case MSG_CLICK:
        d->d1=vbound((gui_mouse_x()-d->x-2)/w,0,15);
        d->flags|=D_DIRTY;
        break;
    }
    
    return D_O_K;
}

int32_t d_sys_color_proc(int32_t msg,DIALOG *d,int32_t)
{
    int32_t w=(d->w-4)/17;
    
    switch(msg)
    {
    case MSG_DRAW:
        //rectfill(screen, d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, jwin_pal[jcBOX]);
        //top blank part
        rectfill(screen, d->x, d->y, d->x+(w*17)+3, d->y+1, jwin_pal[jcBOX]);
        jwin_draw_frame(screen,d->x,d->y+2,w*17+4, d->h-4, FR_DEEP);
        
        for(int32_t i=0; i<17; ++i)
        {
            rectfill(screen, d->x+2+(w*i), d->y+4, d->x+2+(w*(i+1))-1, d->y+d->h-5, vc(zc_max(0,i-1)));
        }
        
        line(screen, d->x+2, d->y+4, d->x+2+w-1, d->y+d->h-5, vc(15));
        line(screen, d->x+2, d->y+d->h-5, d->x+2+w-1, d->y+4, vc(15));
        // right end
        rectfill(screen, d->x+(w*17)+4, d->y, d->x+d->w-1, d->y+d->h-1, jwin_pal[jcBOX]);
        // bottom part
        rectfill(screen, d->x, d->y+d->h-2, d->x+(w*17)+3, d->y+d->h-1, jwin_pal[jcBOX]);
        
        //indicator lines
        hline(screen, d->x+2+(w*(d->d1+1)), d->y, d->x+2+(w*(d->d1+2))-1, jwin_pal[jcBOXFG]);
        hline(screen, d->x+2+(w*(d->d1+1)), d->y+d->h-1, d->x+2+(w*(d->d1+2))-1, jwin_pal[jcBOXFG]);
        
        break;
        
    case MSG_CLICK:
        d->d1=vbound((gui_mouse_x()-d->x-2)/w,0,16)-1;
        d->flags|=D_DIRTY;
        break;
    }
    
    return D_O_K;
}

void update_ctl_proc(DIALOG *d, int32_t ct)
{
    switch(ct)
    {
    case 0:
        d->proc=jwin_droplist_proc;
        break;
        
    case 1:
        d->proc=d_sys_color_proc;
        break;
        
    default:
        d->proc=d_cs_color_proc;
        break;
    }
    
    d->flags|=D_DIRTY;
}

int32_t d_ctl_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t old_d1=d->d1;
    int32_t ret=jwin_droplist_proc(msg, d, c);
    
    if(d->d1!=old_d1)
    {
        update_ctl_proc(d+2, d->d1);
    }
    
    return ret;
}

int32_t d_csl2_proc(int32_t msg,DIALOG *d,int32_t c);

void update_csl_proc(DIALOG *d, int32_t cs)
{
    switch(cs)
    {
    case 0:
        d->proc=jwin_text_proc;
        d->fg=jwin_pal[jcBOXFG];
        d->bg=jwin_pal[jcBOX];
        (d+1)->proc=d_csl2_proc;
        (d+1)->fg=0;
        (d+1)->bg=0;
        //(d+59)->fg=subscreen_cset(&misc,(d-1)->d1?(d-1)->d1-1:ssctMISC, (d+1)->d1);
        break;
        
    default:
        d->proc=d_box_proc;
        d->fg=jwin_pal[jcBOX];
        d->bg=jwin_pal[jcBOX];
        (d+1)->proc=d_box_proc;
        (d+1)->fg=jwin_pal[jcBOX];
        (d+1)->bg=jwin_pal[jcBOX];
        break;
    }
    
    d->flags|=D_DIRTY;
    (d+1)->flags|=D_DIRTY;
}

int32_t d_csl_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t old_d1=d->d1;
    int32_t ret=jwin_droplist_proc(msg, d, c);
    
    if(d->d1!=old_d1)
    {
        update_csl_proc(d+1, d->d1);
        (d+60)->fg=subscreen_cset(&misc,d->d1?d->d1-1:ssctMISC, (d+2)->d1);
    }
    
    return ret;
}

int32_t d_csl2_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t old_d1=d->d1;
    int32_t ret=jwin_droplist_proc(msg, d, c);
    
    if(d->d1!=old_d1)
    {
        (d+58)->fg=subscreen_cset(&misc,(d-2)->d1?(d-2)->d1-1:ssctMISC, d->d1);
    }
    
    return ret;
}

int32_t jwin_fontdrop_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t old_d1=d->d1;
    int32_t ret=jwin_droplist_proc(msg, d, c);
    
    if(d->d1!=old_d1)
    {
        (d+3)->dp2=ss_font(d->d1);
        (d+3)->flags|=D_DIRTY;
        
        if((d+3)->proc!=d_dummy_proc)
        {
            rectfill(screen, (d+3)->x, (d+3)->y, (d+3)->x+(d+3)->w-1, (d+3)->y+(d+3)->h-1, jwin_pal[jcBOX]);
        }
        
        (d+3)->h=text_height(ss_font(d->d1))+8;
        (d+49)->dp2=ss_font(d->d1);
        (d+49)->flags|=D_DIRTY;
        
        if((d+49)->proc!=d_dummy_proc)
        {
            rectfill(screen, (d+49)->x, (d+49)->y, (d+49)->x+(d+49)->w-1, (d+49)->y+(d+49)->h-1, jwin_pal[jcBOX]);
        }
        
        (d+49)->h=text_height(ss_font(d->d1))+8;
    }
    
    return ret;
}

const char *misccolor_str[ssctMAX] =
{
    "Text", "Caption", "Overworld BG", "Dungeon BG", "Dungeon FG", "Cave FG", "BS Dark", "BS Goal", "Compass (Lt)", "Compass (Dk)",
    "SS BG", "SS Shadow", "Tri. Frame", "Big Map BG", "Big Map FG", "Player's Pos", "Message Text"
};

const char *misccolorlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = ssctMAX;
        return NULL;
    }
    
    return misccolor_str[index];
}

const char *csettype_str[13] =
{
    "Misc. CSet", "CSet 0", "CSet 1", "CSet 2", "CSet 3", "CSet 4", "CSet 5", "CSet 6", "CSet 7", "CSet 8", "CSet 9", "CSet 10", "CSet 11"
};

const char *csettypelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 13;
        return NULL;
    }
    
    return csettype_str[index];
}

const char *misccset_str[sscsMAX] =
{
    "Triforce", "Tri. Frame", "Overworld Map", "Dungeon Map", "Blue Frame", "HC Piece", "SS Vine"
};

const char *misccsetlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = sscsMAX;
        return NULL;
    }
    
    return misccset_str[index];
}

const char *spectile_str[ssmstMAX+1] =
{
    "None", "SS Vine", "Magic Meter"
};

const char *spectilelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = ssmstMAX+1;
        return NULL;
    }
    
    return spectile_str[index];
}

const char *ssfont_str[ssfMAX] =
{
	"Zelda NES", "SS 1", "SS 2", "SS 3", "SS 4", "BS Time", "Small", "Small Prop.", "LttP Small", "Link's Awakening", "Link to the Past",
	"Goron", "Zoran", "Hylian 1", "Hylian 2", "Hylian 3", "Hylian 4", "Proportional", "Oracle", "Oracle Proportional", "Phantom", "Phantom Proportional",
	"Atari 800", "Acorn", "ADOS", "Allegro", "Apple II", "Apple II 80 Column", "Apple IIgs", "Aquarius",
	"Atari 400", "C64", "C64 HiRes", "IBM CGA", "COCO Mode I", "COCO Mode II", "Coupe", "Amstrad CPC",
	"Fantasy Letters", "FDS Katakana", "FDSesque", "FDS Roman", "FF", "Elder Futhark", "Gaia", "Hira",
	"JP Unsorted", "Kong", "Mana", "Mario", "Mot CPU", "MSX Mode 0", "MSX Mode 1", "PET", "Homebrew",
	"Mr. Saturn", "Sci-Fi", "Sherwood", "Sinclair QL", "Spectrum", "Spectrum Large", "TI99", "TRS",
	"Zelda 2", "ZX", "Lisa"
};

const char *ssfontlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = ssfMAX;
        return NULL;
    }
    
    return ssfont_str[index];
}

extern const char *shadowstyle_str[sstsMAX];

const char *shadowstylelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = sstsMAX;
        return NULL;
    }
    
    return shadowstyle_str[index];
}

const char *alignment_str[3] =
{
    "Left", "Center", "Right"
};

const char *alignmentlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 3;
        return NULL;
    }
    
    return alignment_str[index];
}

const char *wrapping_str[2] =
{
    "Character", "Word"
};

const char *wrappinglist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 2;
        return NULL;
    }
    
    return wrapping_str[index];
}

const char *rows_str[2] =
{
    "Two", "Three"
};

const char *rowslist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 2;
        return NULL;
    }
    
    return rows_str[index];
}

const char *button_str[4] =
{
    "A", "B", "X", "Y"
};

const char *buttonlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 4;
        return NULL;
    }
    
    return button_str[index];
}

const char *icounter_str[sscMAX] =
{
    "Rupees", "Bombs", "Super Bombs", "Arrows", "Gen. Keys w/Magic", "Gen. Keys w/o Magic", "Level Keys w/Magic",
    "Level Keys w/o Magic", "Any Keys w/Magic", "Any Keys w/o Magic", "Custom 1", "Custom 2", "Custom 3", "Custom 4",
    "Custom 5", "Custom 6", "Custom 7", "Custom 8", "Custom 9", "Custom 10", "Custom 11", "Custom 12", "Custom 13",
    "Custom 14", "Custom 15", "Custom 16", "Custom 17", "Custom 18", "Custom 19", "Custom 20", "Custom 21",
    "Custom 22", "Custom 23", "Custom 24", "Custom 25", "Life", "Magic", "Max Life", "Max Magic"
};

const char *icounterlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = sscMAX;
        return NULL;
    }
    
    return icounter_str[index];
}


int32_t d_stilelist_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t old_d1=d->d1;
    int32_t ret=jwin_droplist_proc(msg, d, c);
    
    if(d->d1!=old_d1)
    {
        (d-14)->h=32;
        (d-15)->h=(d-14)->h+4;
        
        switch(d->d1-1)
        {
        case ssmstSSVINETILE:
            (d-15)->w=52;
            (d-14)->w=48;
            (d-14)->d1=wpnsbuf[iwSubscreenVine].tile;
            break;
            
        case ssmstMAGICMETER:
            (d-15)->w=148;
            (d-14)->w=144;
            (d-14)->d1=wpnsbuf[iwMMeter].tile;
            break;
            
        case -1:
        default:
            (d-15)->w=20;
            (d-14)->w=16;
            break;
        }
        
        (d-14)->w*=2;
        (d-15)->w=(d-14)->w+4;
        (d-14)->bg=vbound((d-14)->bg,0,((d-14)->w-1)>>2);
        (d-17)->flags|=D_DIRTY;
        (d-15)->flags|=D_DIRTY;
        (d-14)->flags|=D_DIRTY;
        (d-1)->flags|=D_DIRTY;
        d->flags|=D_DIRTY;
    }
    
    return ret;
}



static int32_t ssop_location_list[] =
{
    // dialog control number
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,212,213, -1
};

static int32_t ssop_color_list[] =
{
    // dialog control number
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, -1
};

static int32_t ssop_attributes_list[] =
{
    // dialog control number
    95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, -1
};

static TABPANEL sso_properties_tabs[] =
{
    // (text)
    { (char *)"Location",    D_SELECTED,  ssop_location_list,   0, NULL },
    { (char *)"Color",       0,           ssop_color_list,      0, NULL },
    { (char *)"Attributes",  0,           ssop_attributes_list, 0, NULL },
    { NULL,                  0,           NULL,                 0, NULL }
};

int32_t jwin_tflpcheck_proc(int32_t msg,DIALOG *d,int32_t c)
{
    bool selected=(d->flags&D_SELECTED)!=0;
    int32_t ret = jwin_checkfont_proc(msg,d,c);
    bool new_selected=(d->flags&D_SELECTED)!=0;
    
    if(new_selected!=selected)
    {
        (d-3)->w=(new_selected?32:16)*2;
        (d-3)->h=(new_selected?48:16)*2;
        (d-4)->w=(d-3)->w+4;
        (d-4)->h=(d-3)->h+4;
//    (d-5)->x=((d-4)->x)+(((d-4)->w)/2);
        (d-6)->w=(new_selected?112:96)*2;
        (d-6)->h=(new_selected?112:48)*2;
        (d-7)->w=(d-6)->w+4;
        (d-7)->h=(d-6)->h+4;
//    (d-8)->x=((d-7)->x)+(((d-7)->w)/2);
        (d-3)->flags|=D_DIRTY;
        (d-4)->flags|=D_DIRTY;
        (d-5)->flags|=D_DIRTY;
        (d-6)->flags|=D_DIRTY;
        (d-7)->flags|=D_DIRTY;
        (d-8)->flags|=D_DIRTY;
        (d-9)->flags|=D_DIRTY;
    }
    
    return ret;
}

int32_t jwin_lscheck_proc(int32_t msg,DIALOG *d,int32_t c)
{
    bool selected=(d->flags&D_SELECTED)!=0;
    int32_t ret = jwin_checkfont_proc(msg,d,c);
    bool new_selected=(d->flags&D_SELECTED)!=0;
    
    if(new_selected!=selected || msg==MSG_START)
    {
        (d-6)->w=(new_selected?32:16)*2;
        (d-6)->h=(new_selected?48:16)*2;
        (d-7)->w=(d-6)->w+4;
        (d-7)->h=(d-6)->h+4;
        (d-6)->flags|=D_DIRTY;
        (d-7)->flags|=D_DIRTY;
        (d-8)->flags|=D_DIRTY;
        (d-9)->flags|=D_DIRTY;
    }
    
    return ret;
}

int32_t d_qtile_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_DRAW:
    {
        int32_t dw = d->w / 2;
        int32_t dh = d->h / 2;
        
        BITMAP *buf = create_bitmap_ex(8,dw,dh);
        
        if(buf)
        {
            clear_bitmap(buf);
            
            for(int32_t y=0; y<dh; y+=16)
            {
                for(int32_t x=0; x<dw; x+=16)
                {
                    puttile16(buf,d->d1+(y>>4)*20+(x>>4),x,y,d->fg,d->d2);
                }
            }
            
            int32_t t=d->bg>>2;
            int32_t t2=d->bg-(t<<2);
            rect(buf,
                 (t<<4)+((t2&1)<<3) ,
                 ((t2&2)<<2),
                 (t<<4)+((t2&1)<<3)+7,
                 ((t2&2)<<2)+7, jwin_pal[jcTITLER]);
            stretch_blit(buf,screen,0,0,dw,dh,d->x-1,d->y-1,dw*2,dh*2);
            destroy_bitmap(buf);
        }
        
        //textprintf_ex(screen, get_zc_font(font_pfont), d->x,d->y, vc(15), -1, "%d", d->bg);
        return D_O_K;
    }
    break;
    
    case MSG_CLICK:
    {
        int32_t old_fg=d->fg;
        
        if(gui_mouse_b()&2)  //right mouse button
        {
            int32_t old_bg = d->bg;
            int32_t mx = vbound(gui_mouse_x()-d->x,0,d->w-1) / 2;
            int32_t my = vbound(gui_mouse_y()-d->y,0,d->h-1) / 2;
            
            int32_t t=mx>>4;
            d->bg=(t<<2)+((my>>3)<<1)+((mx-(t<<4))>>3);
            
            if(d->bg!=old_bg)
            {
                d->flags|=D_DIRTY;
            }
            
            return D_O_K;
        }
        
        int32_t ret=d_maptile_proc(msg, d, c);
        
        if(d->fg!=old_fg)
        {
            (d-60)->d1=d->fg+1;
            (d-60)->d2=(d-60)->d1;
            update_csl_proc(d-59, d->fg+1);
        }
        
        d->flags|=D_DIRTY;
        return ret;
    }
    break;
    }
    
    return d_maptile_proc(msg, d, c);
}

int32_t d_spectile_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t d1=d->d1;
    int32_t ret=d_qtile_proc(msg,d,c);
    
    if(d1!=d->d1)
    {
        (d+14)->d1=0;
        (d+14)->d2=0;
        (d+14)->flags|=D_DIRTY;
        d->w=32;
        (d-1)->w=d->w+4;
        d->flags|=D_DIRTY;
        (d-1)->flags|=D_DIRTY;
        (d-3)->flags|=D_DIRTY;
    }
    
    return ret;
}

int32_t d_tileblock_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t old_fg=d->fg;
    int32_t ret=d_maptile_proc(msg, d, c);
    
    switch(msg)
    {
    case MSG_CLICK:
        if(d->fg!=old_fg)
        {
            (d-60)->d1=d->fg+1;
            (d-60)->d2=(d-60)->d1;
            update_csl_proc(d-59, d->fg+1);
        }
        
        d->flags|=D_DIRTY;
        break;
    }
    
    return ret;
}

void extract_colortype(DIALOG *d, subscreen_object *tempsso, int32_t ct)
{
    switch(ct)
    {
    case 3:
        switch(tempsso->colortype3)
        {
        case ssctMISC:
            d->d1=0;
            break;
            
        case ssctSYSTEM:
            d->d1=1;
            break;
            
        default:
            d->d1=tempsso->colortype3+2;
            break;
        }
        
        break;
        
    case 2:
        switch(tempsso->colortype2)
        {
        case ssctMISC:
            d->d1=0;
            break;
            
        case ssctSYSTEM:
            d->d1=1;
            break;
            
        default:
            d->d1=tempsso->colortype2+2;
            break;
        }
        
        break;
        
    case 1:
    default:
        switch(tempsso->colortype1)
        {
        case ssctMISC:
            d->d1=0;
            break;
            
        case ssctSYSTEM:
            d->d1=1;
            break;
            
        default:
            d->d1=tempsso->colortype1+2;
            break;
        }
        
        break;
    }
}

void insert_colortype(DIALOG *d, subscreen_object *tempsso, int32_t ct)
{
    switch(ct)
    {
    case 3:
        switch(d->d1)
        {
        case 0:
            tempsso->colortype3=ssctMISC;
            break;
            
        case 1:
            tempsso->colortype3=ssctSYSTEM;
            break;
            
        default:
            tempsso->colortype3=d->d1-2;
            break;
        }
        
        break;
        
    case 2:
        switch(d->d1)
        {
        case 0:
            tempsso->colortype2=ssctMISC;
            break;
            
        case 1:
            tempsso->colortype2=ssctSYSTEM;
            break;
            
        default:
            tempsso->colortype2=d->d1-2;
            break;
        }
        
        break;
        
    case 1:
    default:
        switch(d->d1)
        {
        case 0:
            tempsso->colortype1=ssctMISC;
            break;
            
        case 1:
            tempsso->colortype1=ssctSYSTEM;
            break;
            
        default:
            tempsso->colortype1=d->d1-2;
            break;
        }
        
        break;
    }
}

void extract_cset(DIALOG *d, subscreen_object *tempsso, int32_t ct)
{
    switch(ct)
    {
    case 3:
        switch(tempsso->colortype3)
        {
        case ssctMISC:
            d->d1=0;
            break;
            
        default:
            d->d1=tempsso->colortype3+1;
            break;
        }
        
        break;
        
    case 2:
        switch(tempsso->colortype2)
        {
        case ssctMISC:
            d->d1=0;
            break;
            
        default:
            d->d1=tempsso->colortype2+1;
            break;
        }
        
        break;
        
    case 1:
    default:
        switch(tempsso->colortype1)
        {
        case ssctMISC:
            d->d1=0;
            break;
            
        default:
            d->d1=tempsso->colortype1+1;
            break;
        }
        
        break;
    }
}

void insert_cset(DIALOG *d, subscreen_object *tempsso, int32_t ct)
{
    switch(ct)
    {
    case 3:
        switch(d->d1)
        {
        case 0:
            tempsso->colortype3=ssctMISC;
            break;
            
        default:
            tempsso->colortype3=d->d1-1;
            break;
        }
        
        break;
        
    case 2:
        switch(d->d1)
        {
        case 0:
            tempsso->colortype2=ssctMISC;
            break;
            
        default:
            tempsso->colortype2=d->d1-1;
            break;
        }
        
        break;
        
    case 1:
    default:
        switch(d->d1)
        {
        case 0:
            tempsso->colortype1=ssctMISC;
            break;
            
        default:
            tempsso->colortype1=d->d1-1;
            break;
        }
        
        break;
    }
}

static DIALOG sso_raw_data_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     60-12,   40,   200+24,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Raw Data", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_textbox_proc,       72-12,   60+4,   176+24+1,  92+4,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,      NULL, NULL, NULL },
    { jwin_button_proc,     130,   163,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static ListData rows_list(rowslist, &font);
static ListData itemclass_list(item_class_list, &font);

int32_t sso_raw_data(subscreen_object *tempsso)
{
    char raw_text[65535];
    char title[80];
    sprintf(title, "Raw Data for Object #%d", curr_subscreen_object);
    sprintf(raw_text, "Type:  %d\nPosition:  %d\nX:  %d\nY:  %d\nW:  %d\nH:  %d\nColor Type 1:  %d\nColor 1:  %d\nColor Type 2:  %d\nColor 2:  %d\nColor Type 3:  %d\nColor 3:  %d\nD1:  %d\nD2:  %d\nD3:  %d\nD4:  %d\nD5:  %d\nD6:  %d\nD7:  %d\nD8:  %d\nD9:  %d\nD10:  %d\nFrames:  %d\nSpeed:  %d\nDelay:  %d\nFrame:  %d\nDp1:  %s",
            tempsso->type, tempsso->pos, tempsso->x, tempsso->y, tempsso->w, tempsso->h, tempsso->colortype1, tempsso->color1, tempsso->colortype2, tempsso->color2, tempsso->colortype3, tempsso->color3, tempsso->d1, tempsso->d2, tempsso->d3, tempsso->d4, tempsso->d5, tempsso->d6, tempsso->d7, tempsso->d8, tempsso->d9, tempsso->d10, tempsso->frames, tempsso->speed, tempsso->delay, tempsso->frame, tempsso->dp1!=NULL?(char *)tempsso->dp1:"NULL");
    sso_raw_data_dlg[0].dp2=get_zc_font(font_lfont);
    sso_raw_data_dlg[2].dp=raw_text;
    sso_raw_data_dlg[2].d2=0;
    
    large_dialog(sso_raw_data_dlg);
        
    zc_popup_dialog(sso_raw_data_dlg,2);
    return D_O_K;
}

static ListData wrapping_list(wrappinglist, &font);
static ListData alignment_list(alignmentlist, &font);
ListData shadowstyle_list(shadowstylelist, &font);
static ListData misccolor_list(misccolorlist, &font);
static ListData spectile_list(spectilelist, &font);
static ListData ssfont_list(ssfontlist, &font);
static ListData colortype_list(colortypelist, &font);
static ListData item_list(itemlist_num, &font);

int32_t sso_properties(subscreen_object *tempsso)
{
    return call_subscrprop_dialog(tempsso, curr_subscreen_object) ? 0 : -1;
}

int32_t onBringToFront();
int32_t onBringForward();
int32_t onSendBackward();
int32_t onSendToBack();
int32_t onReverseArrangement();
int32_t onAlignLeft();
int32_t onAlignCenter();
int32_t onAlignRight();
int32_t onAlignTop();
int32_t onAlignMiddle();
int32_t onAlignBottom();
int32_t onDistributeLeft();
int32_t onDistributeCenter();
int32_t onDistributeRight();
int32_t onDistributeTop();
int32_t onDistributeMiddle();
int32_t onDistributeBottom();
int32_t onGridSnapLeft();
int32_t onGridSnapCenter();
int32_t onGridSnapRight();
int32_t onGridSnapTop();
int32_t onGridSnapMiddle();
int32_t onGridSnapBottom();
void copySSOProperties(subscreen_object& src, subscreen_object& dest);

int32_t onSubscreenObjectProperties()
{
    if(curr_subscreen_object >= 0)
    {
        if(sso_properties(&(css->objects[curr_subscreen_object]))!=-1)
        {
            for(int32_t i=0; i<MAXSUBSCREENITEMS; i++)
            {
                if(!sso_selection[i])
                    continue;
                
                copySSOProperties(css->objects[curr_subscreen_object], css->objects[i]);
            }
        }
    }
    
    return D_O_K;
}

int32_t onSubscreenObjectRawProperties()
{
    if(curr_subscreen_object >= 0)
    {
        sso_raw_data(&(css->objects[curr_subscreen_object]));
    }
    
    return D_O_K;
}

int32_t onNewSubscreenObject();

int32_t onDeleteSubscreenObject()
{
    int32_t objs=ss_objects(css);
    
    if(objs==0)
    {
        return D_O_K;
    }
    
    for(int32_t i=curr_subscreen_object; i<objs-1; ++i)
    {
        css->objects[i]=css->objects[i+1];
        sso_selection[i]=sso_selection[i+1];
    }
    
    if(css->objects[objs-1].dp1!=NULL)
    {
        //No, don't do this.  css->objects[objs-2] is pointing at this.  Leave it be.
        //delete [] (char *)css->objects[objs-1].dp1;
        css->objects[objs-1].dp1=NULL;
    }
    
    css->objects[objs-1].type=ssoNULL;
    sso_selection[objs-1]=false;
    
    if(ss_propCopySrc==curr_subscreen_object)
        ss_propCopySrc=-1;
    else if(ss_propCopySrc>curr_subscreen_object)
        ss_propCopySrc--;
    
    if(curr_subscreen_object==objs-1)
    {
        --curr_subscreen_object;
    }
    
    update_sso_name();
    update_up_dn_btns();
    
    return D_O_K;
}

int32_t onAddToSelection()
{
    if(curr_subscreen_object >= 0)
    {
        sso_selection[curr_subscreen_object]=true;
    }
    
    return D_O_K;
}

int32_t onRemoveFromSelection()
{
    if(curr_subscreen_object >= 0)
    {
        sso_selection[curr_subscreen_object]=false;
    }
    
    return D_O_K;
}

int32_t onInvertSelection()
{
    for(int32_t i=0; i<ss_objects(css); ++i)
    {
        sso_selection[i]=!sso_selection[i];
    }
    
    return D_O_K;
}

int32_t onClearSelection()
{
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        sso_selection[i]=false;
    }
    
    return D_O_K;
}

int32_t onDuplicateSubscreenObject()
{
    int32_t objs=ss_objects(css);
    
    if(objs==0)
    {
        return D_O_K;
    }
    
    int32_t counter=0;
    
    for(int32_t i=0; i<objs; ++i)
    {
        int32_t c=objs+counter;
        
        if(sso_selection[i]||i==curr_subscreen_object)
        {
            if(css->objects[c].dp1!=NULL)
            {
                delete [](char *)css->objects[c].dp1;
            }
            
            css->objects[c]=css->objects[i];
            
            if(css->objects[c].dp1!=NULL)
            {
                //No, don't do this.  css->objects[i] is pointing at this.  Leave it be.
                //delete [] (char *)css->objects[c].dp1;
                css->objects[c].dp1=NULL;
            }
            
            if(css->objects[i].dp1!=NULL)
            {
                //css->objects[c].dp1=malloc(strlen((char *)css->objects[i].dp1)+1);
                css->objects[c].dp1= new char[strlen((char *)css->objects[i].dp1)+1];
                strcpy((char *)css->objects[c].dp1,(char *)css->objects[i].dp1);
            }
            else
            {
                //css->objects[c].dp1=malloc(2);
                css->objects[c].dp1 = new char[2];
                ((char *)css->objects[c].dp1)[0]=0;
            }
            
            css->objects[c].x+=zc_max(zinit.ss_grid_x>>1,4);
            css->objects[c].y+=zc_max(zinit.ss_grid_y>>1,4);
            ++counter;
        }
    }
    
    update_sso_name();
    update_up_dn_btns();
    
    return D_O_K;
}

static int32_t onToggleInvis();
static int32_t onEditGrid();
static int32_t onSelectionOptions();
static int32_t onShowHideGrid();


static MENU subscreen_rc_menu[] =
{
    { (char *)"Properties ",       NULL,  NULL, 0, NULL },
    { (char *)"Inspect ",          NULL,  NULL, 0, NULL },
    { (char *)"Copy Properties ",  NULL,  NULL, 0, NULL },
    { (char *)"Paste Properties ", NULL,  NULL, 0, NULL },
    { NULL,                        NULL,  NULL, 0, NULL }
};


int32_t d_subscreen_proc(int32_t msg,DIALOG *d,int32_t)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        for(int32_t i=ss_objects(css)-1; i>=0; --i)
        {
            int32_t x=sso_x(&css->objects[i])*2;
            int32_t y=sso_y(&css->objects[i])*2;
            int32_t w=sso_w(&css->objects[i])*2;
            int32_t h=sso_h(&css->objects[i])*2;
            
            switch(get_alignment(&css->objects[i]))
            {
            case sstaCENTER:
                x-=(w/2);
                break;
                
            case sstaRIGHT:
                x-=w;
                break;
                
            case sstaLEFT:
            default:
                break;
            }
            
            if(isinRect(gui_mouse_x(),gui_mouse_y(),d->x+x, d->y+y, d->x+x+w-1, d->y+y+h-1))
            {
                if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
                {
                    if(sso_selection[i]==true)
                    {
                        sso_selection[i]=false;
                    }
                    else
                    {
                        sso_selection[curr_subscreen_object]=true;
                        curr_subscreen_object=i;
                        update_sso_name();
                        update_up_dn_btns();
                    }
                }
                else
                {
                    onClearSelection();
                    curr_subscreen_object=i;
                    update_sso_name();
                    update_up_dn_btns();
                }
                
                break;
            }
        }
        
        if(gui_mouse_b()&2) //right mouse button
        {
            object_message(d,MSG_DRAW,0);
            
            // Disable "Paste Properties" if the copy source is invalid
            if(ss_propCopySrc<0 || css->objects[ss_propCopySrc].type==ssoNULL)
                subscreen_rc_menu[3].flags|=D_DISABLED;
            else
                subscreen_rc_menu[3].flags&=~D_DISABLED;
            
            int32_t m = popup_menu(subscreen_rc_menu,gui_mouse_x(),gui_mouse_y());
            
            switch(m)
            {
            case 0: // Properties
                onSubscreenObjectProperties();
                break;
                
            case 1: // Inspect
                onSubscreenObjectRawProperties();
                break;
            
            case 2: // Copy Properties
                ss_propCopySrc=curr_subscreen_object;
                break;
                
            case 3: // Paste Properties
                if(ss_propCopySrc>=0) // Hopefully unnecessary)
                {
                    copySSOProperties(css->objects[ss_propCopySrc], css->objects[curr_subscreen_object]);
                    for(int32_t i=0; i<MAXSUBSCREENITEMS; i++)
                    {
                        if(!sso_selection[i])
                            continue;
                        
                        copySSOProperties(css->objects[ss_propCopySrc], css->objects[i]);
                    }
                }
                break;
            }
        }
        
        return D_O_K;
    }
    break;
    
    case MSG_VSYNC:
        d->flags|=D_DIRTY;
        break;
        
    case MSG_DRAW:
    {
        Sitems.animate();
        BITMAP *buf = create_bitmap_ex(8,d->w,d->h);//In Large Mode, this is actually 2x as large as needed, but whatever.
        
        if(buf)
        {
            clear_bitmap(buf);
            show_custom_subscreen(buf, &misc, (subscreen_group *)(d->dp), 0, 0, true, sspUP | sspDOWN | sspSCROLLING);
            
            for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
            {
                if(sso_selection[i])
                {
                    sso_bounding_box(buf, css, i, vc(zinit.ss_bbox_2_color));
                }
            }
            
            sso_bounding_box(buf, css, curr_subscreen_object, vc(zinit.ss_bbox_1_color));
            
            if(zinit.ss_flags&ssflagSHOWGRID)
            {
                for(int32_t x=zinit.ss_grid_xofs; x<d->w; x+=zinit.ss_grid_x)
                {
                    for(int32_t y=zinit.ss_grid_yofs; y<d->h; y+=zinit.ss_grid_y)
                    {
                        buf->line[y][x]=vc(zinit.ss_grid_color);
                    }
                }
            }
            
            stretch_blit(buf,screen,0,0,d->w/2,d->h/2,d->x,d->y,d->w,d->h);
            
            destroy_bitmap(buf);
        }
    }
    break;
    
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
        break;
    }
    
    return D_O_K;
}

int32_t onSSUp();
int32_t onSSDown();
int32_t onSSLeft();
int32_t onSSRight();
int32_t onSSPgDn();
int32_t onSSPgUp();

int32_t d_ssup_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_ssdn_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_sslt_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_ssrt_btn_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t onSSUp()
{
    int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?-zinit.ss_grid_y:-1;
    
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        if(sso_selection[i]&&i!=curr_subscreen_object)
        {
            if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
            {
                css->objects[i].h+=delta;
            }
            else
            {
                css->objects[i].y+=delta;
            }
        }
    }
    
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        css->objects[curr_subscreen_object].h+=delta;
    }
    else
    {
        css->objects[curr_subscreen_object].y+=delta;
    }
    
    return D_O_K;
}

int32_t onSSDown()
{
    int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?zinit.ss_grid_y:1;
    
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        if(sso_selection[i]&&i!=curr_subscreen_object)
        {
            if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
            {
                css->objects[i].h+=delta;
            }
            else
            {
                css->objects[i].y+=delta;
            }
        }
    }
    
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        css->objects[curr_subscreen_object].h+=delta;
    }
    else
    {
        css->objects[curr_subscreen_object].y+=delta;
    }
    
    return D_O_K;
}

int32_t onSSLeft()
{
    int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?-zinit.ss_grid_x:-1;
    
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        if(sso_selection[i]&&i!=curr_subscreen_object)
        {
            if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
            {
                css->objects[i].w+=delta;
            }
            else
            {
                css->objects[i].x+=delta;
            }
        }
    }
    
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        css->objects[curr_subscreen_object].w+=delta;
    }
    else
    {
        css->objects[curr_subscreen_object].x+=delta;
    }
    
    return D_O_K;
}

int32_t onSSRight()
{
    int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?zinit.ss_grid_x:1;
    
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        if(sso_selection[i]&&i!=curr_subscreen_object)
        {
            if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
            {
                css->objects[i].w+=delta;
            }
            else
            {
                css->objects[i].x+=delta;
            }
        }
    }
    
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        css->objects[curr_subscreen_object].w+=delta;
    }
    else
    {
        css->objects[curr_subscreen_object].x+=delta;
    }
    
    return D_O_K;
}

int32_t d_ssup_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSUp();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSDown();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSLeft();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSRight();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssup_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        
        for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
        {
            if(sso_selection[i]&&i!=curr_subscreen_object)
            {
                --css->objects[i].h;
            }
        }
        
        --css->objects[curr_subscreen_object].h;
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        
        for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
        {
            if(sso_selection[i]&&i!=curr_subscreen_object)
            {
                ++css->objects[i].h;
            }
        }
        
        ++css->objects[curr_subscreen_object].h;
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        
        for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
        {
            if(sso_selection[i]&&i!=curr_subscreen_object)
            {
                --css->objects[i].w;
            }
        }
        
        --css->objects[curr_subscreen_object].w;
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        
        for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
        {
            if(sso_selection[i]&&i!=curr_subscreen_object)
            {
                ++css->objects[i].w;
            }
        }
        
        ++css->objects[curr_subscreen_object].w;
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t Bweapon(int32_t pos)
{
    int32_t p=-1;
    
    for(int32_t i=0; css->objects[i].type!=ssoNULL; ++i)
    {
        if(css->objects[i].type==ssoCURRENTITEM && css->objects[i].d3==pos)
        {
            p=i;
            break;
        }
    }
    
    if(p==-1)
    {
        return 0;
    }
    
    int32_t family = 0;
    bool bow = false;
    
    switch(css->objects[p].d1)
    {
    case itype_arrow:
    case itype_bowandarrow:
        if(current_item(itype_bow) && current_item(itype_arrow))
        {
            bow=(css->objects[p].d1==itype_bowandarrow);
            family=itype_arrow;
        }
        
        break;
        
    case itype_letterpotion:
        if(current_item(itype_potion))
            family=itype_potion;
        else if(current_item(itype_letter))
            family=itype_letter;
        
        break;
        
    case itype_sword:
    {
        if(!get_bit(quest_rules,qr_SELECTAWPN))
            break;
            
        family=itype_sword;
    }
    break;
    
    default:
        family=css->objects[p].d1;
    }
    
    for(int32_t i=0; i<MAXITEMS; i++)
    {
        if(itemsbuf[i].family==family) return i+(bow ? 0xF000 : 0);
    }
    
    return 0;
}

void selectBwpn(int32_t xstep, int32_t ystep)
{
    if((xstep==0)&&(ystep==0))
    {
        Bwpn=Bweapon(Bpos);
        update_subscr_items();
        
        if(Bwpn)
        {
            return;
        }
        
        xstep=1;
    }
    
    if((xstep==8)&&(ystep==8))
    {
        Bwpn=Bweapon(Bpos);
        update_subscr_items();
        
        if(Bwpn)
        {
            return;
        }
        
        xstep=-1;
    }
    
    int32_t pos = Bpos;
    int32_t tries=0;
    
    do
    {
        int32_t p=-1;
        
        for(int32_t i=0; css->objects[i].type!=ssoNULL; ++i)
        {
            if(css->objects[i].type==ssoCURRENTITEM)
            {
                if(css->objects[i].d3==Bpos)
                {
                    p=i;
                    break;
                }
            }
        }
        
        if(p!=-1)
        {
            if(xstep!=0)
            {
                Bpos=xstep<0?css->objects[p].d6:css->objects[p].d7;
            }
            else
            {
                Bpos=ystep<0?css->objects[p].d4:css->objects[p].d5;
            }
        }
        
        Bwpn = Bweapon(Bpos);
        update_subscr_items();
        
        if(Bwpn)
        {
            return;
        }
    }
    while(Bpos!=pos && ++tries<0x100);
    
    if(!Bwpn)
        Bpos=0;
}


int32_t d_ssup_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        selectBwpn(0, -1);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        selectBwpn(0, 1);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        selectBwpn(-1, 0);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        selectBwpn(1, 0);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

const char *sso_type[ssoMAX]=
{
    "ssoNULL", "ssoNONE", "sso2X2FRAME", "ssoTEXT", "ssoLINE", "ssoRECT", "ssoBSTIME", "ssoTIME", "ssoSSTIME", "ssoMAGICMETER", "ssoLIFEMETER", "ssoBUTTONITEM", "ssoICON", "ssoCOUNTER",
    "ssoCOUNTERS", "ssoMINIMAPTITLE", "ssoMINIMAP", "ssoLARGEMAP", "ssoCLEAR", "ssoCURRENTITEM", "ssoITEM", "ssoTRIFRAME", "ssoTRIFORCE", "ssoTILEBLOCK", "ssoMINITILE", "ssoSELECTOR1", "ssoSELECTOR2",
    "ssoMAGICGAUGE", "ssoLIFEGAUGE", "ssoTEXTBOX", "ssoCURRENTITEMTILE", "ssoSELECTEDITEMTILE", "ssoCURRENTITEMTEXT", "ssoCURRENTITEMNAME", "ssoSELECTEDITEMNAME", "ssoCURRENTITEMCLASSTEXT",
    "ssoCURRENTITEMCLASSNAME", "ssoSELECTEDITEMCLASSNAME"
};

const char *sso_textstyle[sstsMAX]=
{
    "sstsNORMAL", "sstsSHADOW", "sstsSHADOWU", "sstsOUTLINE8", "sstsOUTLINEPLUS", "sstsOUTLINEX", "sstsSHADOWED", "sstsSHADOWEDU", "sstsOUTLINED8", "sstsOUTLINEDPLUS", "sstsOUTLINEDX"
};

const char *sso_fontname[ssfMAX]=
{
    "ssfZELDA", "ssfSS1", "ssfSS2", "ssfSS3", "ssfSS4", "ssfZTIME", "ssfSMALL", "ssfSMALLPROP", "ssfZ3SMALL", "ssfGBLA", "ssfZ3",
    "ssfGORON", "ssfZORAN", "ssfHYLIAN1", "ssfHYLIAN2", "ssfHYLIAN3", "ssfHYLIAN4", "ssfPROP", "ssfGBORACLE", "ssfGBORACLEP",
    "ssfDSPHANTOM", "ssfDSPHANTOMP",
	
	"ssfAT800", "ssfACORN", "ssfALLEG", "ssfAPL2", "ssfAPL280", "ssfAPL2GS", "ssfAQUA", "ssfAT400", "ssfC64", "ssfC64HR",
	"ssfCGA", "ssfCOCO", "ssfCOCO2", "ssfCOUPE", "ssfCPC", "ssfFANTASY", "ssfFDSKANA", "ssfFDSLIKE", "ssfFDSROM", "ssfFF",
	"ssfFUTHARK", "ssfGAIA", "ssfHIRA", "ssfJP", "ssfKONG", "ssfMANA", "ssfML", "ssfMOT", "ssfMSX0", "ssfMSX1", "ssfPET",
	"ssfPSTART", "ssfSATURN", "ssfSCIFI", "ssfSHERW", "ssfSINQL", "ssfSPEC", "ssfSPECLG", "ssfTI99", "ssfTRS", "ssfZ2", "ssfZX", "ssfLISA"
	
};

const char *sso_colortype[2]=
{
    "ssctSYSTEM", "ssctMISC"
};

const char *sso_specialcolor[ssctMAX]=
{
    "ssctTEXT", "ssctCAPTION", "ssctOVERWBG", "ssctDNGNBG", "ssctDNGNFG", "ssctCAVEFG", "ssctBSDK", "ssctBSGOAL", "ssctCOMPASSLT", "ssctCOMPASSDK", "ssctSUBSCRBG", "ssctSUBSCRSHADOW",
    "ssctTRIFRAMECOLOR", "ssctBMAPBG", "ssctBMAPFG", "ssctHERODOT", "ssctMSGTEXT"
};

const char *sso_specialcset[sscsMAX]=
{
    "sscsTRIFORCECSET", "sscsTRIFRAMECSET", "sscsOVERWORLDMAPCSET", "sscsDUNGEONMAPCSET", "sscsBLUEFRAMECSET", "sscsHCPIECESCSET", "sscsSSVINECSET"
};

const char *sso_specialtile[ssmstMAX]=
{
    "ssmstSSVINETILE", "ssmstMAGICMETER"
};

const char *sso_counterobject[sscMAX]=
{
    "sscRUPEES", "sscBOMBS", "sscSBOMBS", "sscARROWS", "sscGENKEYMAGIC", "sscGENKEYNOMAGIC", "sscLEVKEYMAGIC",
    "sscLEVKEYNOMAGIC", "sscANYKEYMAGIC", "sscANYKEYNOMAGIC", "sscSCRIPT1", "sscSCRIPT2", "sscSCRIPT3", "sscSCRIPT4",
    "sscSCRIPT5", "sscSCRIPT6", "sscSCRIPT7", "sscSCRIPT8", "sscSCRIPT9", "sscSCRIPT10"
};

const char *sso_alignment[3]=
{
    "sstaLEFT", "sstaCENTER", "sstaRIGHT"
};


bool save_subscreen_code(char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE,"");
    
    if(!f)
    {
        return false;
    }
    
    int32_t ssobjs=ss_objects(css);
    char buf[512];
    memset(buf,0,512);
    sprintf(buf, "subscreen_object exported_subscreen[%d]=\n", ssobjs);
    pack_fputs(buf, f);
    
    if(pack_ferror(f))
    {
        pack_fclose(f);
        return false;
    }
    
    pack_fputs("{\n", f);
    
    if(pack_ferror(f))
    {
        pack_fclose(f);
        return false;
    }
    
    for(int32_t i=0; i<ssobjs; ++i)
    {
//    pack_fputs("{\n", f);
        sprintf(buf, "  { %s, %d, %d, %d, %d, %d, ",
                sso_type[css->objects[i].type], css->objects[i].pos, css->objects[i].x, css->objects[i].y, css->objects[i].w, css->objects[i].h);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        if(css->objects[i].colortype1>=ssctSYSTEM)
        {
            sprintf(buf, "%s, ", sso_colortype[css->objects[i].colortype1==ssctSYSTEM?0:1]);
            pack_fputs(buf, f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
            
            if(css->objects[i].colortype1==ssctMISC)
            {
                int32_t t=css->objects[i].type;
                
                if(t==sso2X2FRAME||t==ssoCURRENTITEMTILE||t==ssoICON||t==ssoMINITILE||t==ssoSELECTEDITEMTILE||t==ssoSELECTOR1||t==ssoSELECTOR2||t==ssoTRIFORCE||t==ssoTILEBLOCK)
                {
                    sprintf(buf, "%s, ", sso_specialcset[css->objects[i].color1]);
                    pack_fputs(buf, f);
                    
                    if(pack_ferror(f))
                    {
                        pack_fclose(f);
                        return false;
                    }
                }
                else
                {
                    sprintf(buf, "%s, ", sso_specialcolor[css->objects[i].color1]);
                    pack_fputs(buf, f);
                    
                    if(pack_ferror(f))
                    {
                        pack_fclose(f);
                        return false;
                    }
                }
            }
            else
            {
                sprintf(buf, "%d, ", css->objects[i].color1);
                pack_fputs(buf, f);
                
                if(pack_ferror(f))
                {
                    pack_fclose(f);
                    return false;
                }
            }
        }
        else
        {
            sprintf(buf, "%d, %d, ", css->objects[i].colortype1, css->objects[i].color1);
            pack_fputs(buf, f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
        }
        
        if(css->objects[i].colortype2>=ssctSYSTEM)
        {
            sprintf(buf, "%s, ", sso_colortype[css->objects[i].colortype2==ssctSYSTEM?0:1]);
            pack_fputs(buf, f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
            
            if(css->objects[i].colortype2==ssctMISC)
            {
                int32_t t=css->objects[i].type;
                
                if(t==sso2X2FRAME||t==ssoCURRENTITEMTILE||t==ssoICON||t==ssoMINITILE||t==ssoSELECTEDITEMTILE||t==ssoSELECTOR1||t==ssoSELECTOR2||t==ssoTRIFORCE||t==ssoTILEBLOCK)
                {
                    sprintf(buf, "%s, ", sso_specialcset[css->objects[i].color2]);
                    pack_fputs(buf, f);
                    
                    if(pack_ferror(f))
                    {
                        pack_fclose(f);
                        return false;
                    }
                }
                else
                {
                    sprintf(buf, "%s, ", sso_specialcolor[css->objects[i].color2]);
                    pack_fputs(buf, f);
                    
                    if(pack_ferror(f))
                    {
                        pack_fclose(f);
                        return false;
                    }
                }
            }
            else
            {
                sprintf(buf, "%d, ", css->objects[i].color2);
                pack_fputs(buf, f);
                
                if(pack_ferror(f))
                {
                    pack_fclose(f);
                    return false;
                }
            }
        }
        else
        {
            sprintf(buf, "%d, %d, ", css->objects[i].colortype2, css->objects[i].color2);
            pack_fputs(buf, f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
        }
        
        if(css->objects[i].colortype3>=ssctSYSTEM)
        {
            sprintf(buf, "%s, ", sso_colortype[css->objects[i].colortype3==ssctSYSTEM?0:1]);
            pack_fputs(buf, f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
            
            if(css->objects[i].colortype3==ssctMISC)
            {
                int32_t t=css->objects[i].type;
                
                if(t==sso2X2FRAME||t==ssoCURRENTITEMTILE||t==ssoICON||t==ssoMINITILE||t==ssoSELECTEDITEMTILE||t==ssoSELECTOR1||t==ssoSELECTOR2||t==ssoTRIFORCE||t==ssoTILEBLOCK)
                {
                    sprintf(buf, "%s, ", sso_specialcset[css->objects[i].color3]);
                    pack_fputs(buf, f);
                    
                    if(pack_ferror(f))
                    {
                        pack_fclose(f);
                        return false;
                    }
                }
                else
                {
                    sprintf(buf, "%s, ", sso_specialcolor[css->objects[i].color3]);
                    pack_fputs(buf, f);
                    
                    if(pack_ferror(f))
                    {
                        pack_fclose(f);
                        return false;
                    }
                }
            }
            else
            {
                sprintf(buf, "%d, ", css->objects[i].color3);
                pack_fputs(buf, f);
                
                if(pack_ferror(f))
                {
                    pack_fclose(f);
                    return false;
                }
            }
        }
        else
        {
            sprintf(buf, "%d, %d, ", css->objects[i].colortype3, css->objects[i].color3);
            pack_fputs(buf, f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
        }
        
        switch(css->objects[i].type)
        {
        case ssoCURRENTITEM:
            sprintf(buf, "%s, ", itype_names[css->objects[i].d1]);
            break;
            
        case ssoCOUNTER:
        case ssoCOUNTERS:
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoMINIMAPTITLE:
        case ssoSELECTEDITEMNAME:
        case ssoTIME:
        case ssoSSTIME:
        case ssoBSTIME:
            sprintf(buf, "%s, ", sso_fontname[css->objects[i].d1]);
            break;
            
        default:
            sprintf(buf, "%d, ", css->objects[i].d1);
            break;
        }
        
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        switch(css->objects[i].type)
        {
        case ssoCOUNTER:
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoMINIMAPTITLE:
        case ssoSELECTEDITEMNAME:
        case ssoTIME:
        case ssoSSTIME:
        case ssoBSTIME:
            sprintf(buf, "%s, ", sso_alignment[css->objects[i].d2]);
            break;
            
        case ssoMINITILE:
            if(css->objects[i].d1==-1)
            {
                sprintf(buf, "%s, ", sso_specialtile[css->objects[i].d2]);
            }
            else
            {
                sprintf(buf, "%d, ", css->objects[i].d2);
            }
            
            break;
            
        default:
            sprintf(buf, "%d, ", css->objects[i].d2);
            break;
        }
        
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        switch(css->objects[i].type)
        {
        case ssoCOUNTER:
        case ssoCOUNTERS:
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoMINIMAPTITLE:
        case ssoSELECTEDITEMNAME:
        case ssoTIME:
        case ssoSSTIME:
        case ssoBSTIME:
            sprintf(buf, "%s, ", (char *)sso_textstyle[css->objects[i].d3]);
            break;
            
        default:
            sprintf(buf, "%d, ", css->objects[i].d3);
            break;
        }
        
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].d4);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        switch(css->objects[i].type)
        {
        case ssoCOUNTER:
        case ssoCOUNTERS:
            sprintf(buf, "\'%c\', ", css->objects[i].d5);
            break;
            
        default:
            sprintf(buf, "%d, ", css->objects[i].d5);
            break;
        }
        
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].d6);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        switch(css->objects[i].type)
        {
        case ssoCOUNTER:
            sprintf(buf, "%s, ", sso_counterobject[css->objects[i].d7]);
            break;
            
        default:
            sprintf(buf, "%d, ", css->objects[i].d7);
            break;
        }
        
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].d8);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].d9);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].d10);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].frames);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].speed);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].delay);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        sprintf(buf, "%d, ", css->objects[i].frame);
        pack_fputs(buf, f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
        
        
        if(!css->objects[i].dp1)
        {
            pack_fputs("NULL", f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
        }
        else
        {
            pack_fputs("(void *)\"", f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
            
            pack_fputs((char *)css->objects[i].dp1, f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
            
            pack_fputs("\"", f);
            
            if(pack_ferror(f))
            {
                pack_fclose(f);
                return false;
            }
        }
        
        pack_fputs(" },\n", f);
        
        if(pack_ferror(f))
        {
            pack_fclose(f);
            return false;
        }
    }
    
    pack_fputs("  { ssoNULL }\n", f);
    
    if(pack_ferror(f))
    {
        pack_fclose(f);
        return false;
    }
    
    pack_fputs("};\n", f);
    
    if(pack_ferror(f))
    {
        pack_fclose(f);
        return false;
    }
    
    pack_fclose(f);
    return true;
}



int32_t onExport_Subscreen_Code()
{
    if(!getname("Export Subscreen Code (.zss)","zss",NULL,datapath,false))
        return D_O_K;
        
    char buf[256+20],buf2[256+20],name[256];
    extract_name(temppath,name,FILENAME8_3);
    
    if(save_subscreen_code(temppath))
    {
        sprintf(buf,"ZQuest");
        sprintf(buf2,"Saved %s",name);
    }
    else
    {
        sprintf(buf,"Error");
        sprintf(buf2,"Error saving %s",name);
    }
    
    jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
    return D_O_K;
}

int32_t onActivePassive();

static MENU ss_arrange_menu[] =
{
    { (char *)"Bring to Front",       onBringToFront,          NULL, 0, NULL },
    { (char *)"Bring Forward",        onBringForward,          NULL, 0, NULL },
    { (char *)"Send Backward",        onSendBackward,          NULL, 0, NULL },
    { (char *)"Send to Back",         onSendToBack,            NULL, 0, NULL },
    { (char *)"Reverse",              onReverseArrangement,    NULL, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_grid_snap_menu[] =
{
    { (char *)"Left Edges",           onGridSnapLeft,          NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onGridSnapCenter,        NULL, 0, NULL },
    { (char *)"Right Edges",          onGridSnapRight,         NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"Top Edges",            onGridSnapTop,           NULL, 0, NULL },
    { (char *)"Vertical Centers",     onGridSnapMiddle,        NULL, 0, NULL },
    { (char *)"Bottom Edges",         onGridSnapBottom,        NULL, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_align_menu[] =
{
    { (char *)"Left Edges",           onAlignLeft,             NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onAlignCenter,           NULL, 0, NULL },
    { (char *)"Right Edges",          onAlignRight,            NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"Top Edges",            onAlignTop,              NULL, 0, NULL },
    { (char *)"Vertical Centers",     onAlignMiddle,           NULL, 0, NULL },
    { (char *)"Bottom Edges",         onAlignBottom,           NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"To Grid",              NULL,                    ss_grid_snap_menu, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_distribute_menu[] =
{
    { (char *)"Left Edges",           onDistributeLeft,             NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onDistributeCenter,           NULL, 0, NULL },
    { (char *)"Right Edges",          onDistributeRight,            NULL, 0, NULL },
    { (char *)"",                     NULL,                         NULL, 0, NULL },
    { (char *)"Top Edges",            onDistributeTop,              NULL, 0, NULL },
    { (char *)"Vertical Centers",     onDistributeMiddle,           NULL, 0, NULL },
    { (char *)"Bottom Edges",         onDistributeBottom,           NULL, 0, NULL },
    { NULL,                           NULL,                         NULL, 0, NULL }
};

static MENU ss_edit_menu[] =
{
    { (char *)"&New\tIns",           onNewSubscreenObject,                 NULL, 0, NULL },
    { (char *)"&Delete\tDel",        onDeleteSubscreenObject,              NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Duplicate",          onDuplicateSubscreenObject,           NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Properties",         onSubscreenObjectProperties,          NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Arrange",            NULL,                                 ss_arrange_menu, 0, NULL },
    { (char *)"Al&ign",              NULL,                                 ss_align_menu, 0, NULL },
    { (char *)"Dis&tribute",         NULL,                                 ss_distribute_menu, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"Switch Active/Passive",              onActivePassive,                      NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Take Snapshot\tZ",   onSnapshot,                           NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"E&xport as Code\tX",  onExport_Subscreen_Code,              NULL, 0, NULL },
    { NULL,                          NULL,                                 NULL, 0, NULL }
};

static MENU ss_view_menu[] =
{
    { (char *)"Show in&visible items",           onToggleInvis,                 NULL, 0, NULL },
    { (char *)"&Edit grid",                      onEditGrid,                    NULL, 0, NULL },
    { (char *)"&Show grid",                      onShowHideGrid,                NULL, 0, NULL },
    { NULL,                                 NULL,                          NULL, 0, NULL }
};

static MENU ss_selection_menu[] =
{
    { (char *)"&Add to Selection\tA",       onAddToSelection,                 NULL, 0, NULL },
    { (char *)"&Remove from Selection\tR",  onRemoveFromSelection,            NULL, 0, NULL },
    { (char *)"&Invert Selection\tI",       onInvertSelection,                NULL, 0, NULL },
    { (char *)"&Clear Selection\tC",        onClearSelection,                 NULL, 0, NULL },
    { (char *)"",                           NULL,                             NULL, 0, NULL },
    { (char *)"&Options",                   onSelectionOptions,               NULL, 0, NULL },
    { NULL,                                 NULL,                             NULL, 0, NULL }
};

static MENU subscreen_menu[] =
{
    { (char *)"&Edit",               NULL,                                 ss_edit_menu, 0, NULL },
    { (char *)"&View",               NULL,                                 ss_view_menu, 0, NULL },
    { (char *)"&Selection",          NULL,                                 ss_selection_menu, 0, NULL },
    { NULL,                         NULL,                                 NULL, 0, NULL }
};


static DIALOG subscreen_dlg[] =
{
    // (dialog proc)       (x)   (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,    0,      320,    240,  vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Subscreen Editor", NULL, NULL },
    { jwin_button_proc,     192,  215,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     255,  215,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_frame_proc,      4,    37,     260,    172,  0,                  0,                0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { d_subscreen_proc,     6,    39,     256,    168,  0,                  0,                0,       0,          0,             0,       NULL, NULL, NULL },
    // 5
    { d_box_proc,           11,   211,    181,    8,    0,                  0,                0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       11,   211,    181,    16,   0,                  0,                0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       4,    225,    30,     16,   0,                  0,                0,       0,          0,             0, (void *) "Name:", NULL, NULL },
    { jwin_edit_proc,       34,   221,    155,    16,   0,                  0,                0,       0,          64,            0,       NULL, NULL, NULL },
    
    { d_ssup_btn_proc,      284,  23,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x88", NULL, NULL },
    { d_ssdn_btn_proc,      284,  53,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x89", NULL, NULL },
    { d_sslt_btn_proc,      269,  38,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8A", NULL, NULL },
    { d_ssrt_btn_proc,      299,  38,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8B", NULL, NULL },
    
    { d_ssup_btn2_proc,     284,  70,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x88", NULL, NULL },
    { d_ssdn_btn2_proc,     284,  100,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x89", NULL, NULL },
    { d_sslt_btn2_proc,     269,  85,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8A", NULL, NULL },
    { d_ssrt_btn2_proc,     299,  85,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8B", NULL, NULL },
    
    { d_ssup_btn3_proc,     284,  117,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x98", NULL, NULL },
    { d_ssdn_btn3_proc,     284,  147,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x99", NULL, NULL },
    { d_sslt_btn3_proc,     269,  132,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x9A", NULL, NULL },
    { d_ssrt_btn3_proc,     299,  132,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x9B", NULL, NULL },
    
    { d_ssup_btn4_proc,     284,  164,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x88", NULL, NULL },
    { d_ssdn_btn4_proc,     284,  194,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x89", NULL, NULL },
    { d_sslt_btn4_proc,     269,  179,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8A", NULL, NULL },
    { d_ssrt_btn4_proc,     299,  179,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8B", NULL, NULL },
    
    { jwin_menu_proc,       4,    23,     0,      13,    0,                 0,                0,       0,          0,             0, (void *) subscreen_menu, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_UP,        0, (void *) onSSUp, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_DOWN,      0, (void *) onSSDown, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_LEFT,      0, (void *) onSSLeft, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_RIGHT,     0, (void *) onSSRight, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_PGUP,      0, (void *) onSSPgUp, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_PGDN,      0, (void *) onSSPgDn, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'a',     0,          0,             0, (void *) onAddToSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'r',     0,          0,             0, (void *) onRemoveFromSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'i',     0,          0,             0, (void *) onInvertSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'c',     0,          0,             0, (void *) onClearSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_INSERT,    0, (void *) onNewSubscreenObject, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_DEL,       0, (void *) onDeleteSubscreenObject, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'd',     0,          0,             0, (void *) onDuplicateSubscreenObject, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'e',     0,          0,             0, (void *) onSubscreenObjectProperties, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'z',     0,          0,             0, (void *) onSnapshot, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'x',     0,          0,             0, (void *) onExport_Subscreen_Code, NULL, NULL },
    { d_vsync_proc,         0,     0,     0,       0,    0,                 0,                0,       0,          0,             0,       NULL, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t onActivePassive()
{
    if(css->ss_type == sstACTIVE)
    {
        css->ss_type = sstPASSIVE;
        subscreen_dlg[3].h=116;
        subscreen_dlg[4].h=subscreen_dlg[3].h-4;
    }
    else if(css->ss_type == sstPASSIVE)
    {
        css->ss_type = sstACTIVE;
        subscreen_dlg[3].h=172*2-4;
        subscreen_dlg[4].h=subscreen_dlg[3].h-4;
    }
    
    return D_REDRAW;
}


const char *color_str[16] =
{
    "Black", "Blue", "Green", "Cyan", "Red", "Magenta", "Brown", "Light Gray",
    "Dark Gray", "Light Blue", "Light Green", "Light Cyan", "Light Red", "Light Magenta", "Yellow", "White"
};

const char *colorlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return color_str[index];
}

static ListData color_list(colorlist, &font);

static DIALOG grid_dlg[] =
{
    // (dialog proc)       (x)   (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,    0,      158,    120,  vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Edit Grid Properties", NULL, NULL },
    { jwin_button_proc,     18,    95,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     81,    95,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_text_proc,       6,     29,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "X Size:", NULL, NULL },
    { jwin_edit_proc,       42,    25,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    // 5
    { jwin_text_proc,       6,     49,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Y Size:", NULL, NULL },
    { jwin_edit_proc,       42,    45,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       78,    29,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "X Offset:", NULL, NULL },
    { jwin_edit_proc,       126,   25,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       78,    49,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Y Offset:", NULL, NULL },
    // 10
    { jwin_edit_proc,       126,   45,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       6,     69,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Color:", NULL, NULL },
    { jwin_droplist_proc,   36,    65,    116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static DIALOG sel_options_dlg[] =
{
    // (dialog proc)       (x)    (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,     0,    225,   120,   vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Selection Options", NULL, NULL },
    { jwin_button_proc,     51,    95,    61,    21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,    114,    95,    61,    21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_frame_proc,       6,    28,   213,    51,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,     0,       NULL, NULL, NULL },
    { jwin_text_proc,       10,    25,    48,     8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) " Selection Outlines ", NULL, NULL },
    // 5
    { jwin_text_proc,       14,    41,   186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Primary Color:", NULL, NULL },
    { jwin_droplist_proc,   94,    37,   116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { jwin_text_proc,       14,    61,   186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Secondary Color:", NULL, NULL },
    { jwin_droplist_proc,   94,    57,   116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

const char *sso_str[ssoMAX]=
{
    "NULL", "(None)", "2x2 Frame", "Text", "Line", "Rectangle", "BS-Zelda Time", "Game Time", "Game Time (Quest Rule)", "Magic Meter", "Life Meter",
    "Button Item", "-Icon (Not Implemented)", "Counter", "Counter Block", "Minimap Title", "Minimap", "Large Map", "Background Color", "Current Item", "-Item (Not Implemented)",
    "Triforce Frame", "Triforce Piece", "Tile Block", "Minitile", "Selector 1", "Selector 2", "Magic Gauge Piece", "Life Gauge Piece", "Text Box", "-Current Item -> Tile (Not Implemented)",
    "-Selected Item -> Tile (Not Implemented)", "-Current Item -> Text (Not Implemented)", "-Current Item Name (Not Implemented)", "Selected Item Name",
    "-Current Item Class -> Text (Not Implemented)", "-Current Item Class Name (Not Implemented)", "-Selected Item Class Name (Not Implemented)"
};

char *sso_name(int32_t type)
{
    char *tempname;
    tempname=(char*)malloc(255);
    
    if(type>=0 && type <ssoMAX)
    {
        sprintf(tempname, "%s", sso_str[type]);
    }
    else
    {
        sprintf(tempname, "INVALID OBJECT!  type=%d", type);
    }
    
    return tempname;
}

char *sso_name(subscreen_object *tempss, int32_t id)
{
    return sso_name(tempss[id].type);
}

sso_struct bisso[ssoMAX];
int32_t bisso_cnt=-1;

void build_bisso_list()
{
    int32_t start=1;
    bisso_cnt=0;
    
    for(int32_t i=start; i<ssoMAX; i++)
    {
        if(sso_str[i][0]!='-')
        {
            bisso[bisso_cnt].s = (char *)sso_str[i];
            bisso[bisso_cnt].i = i;
            ++bisso_cnt;
        }
    }
    
    for(int32_t i=start; i<bisso_cnt-1; i++)
    {
        for(int32_t j=i+1; j<bisso_cnt; j++)
        {
            if(stricmp(bisso[i].s,bisso[j].s)>0)
            {
                std::swap(bisso[i],bisso[j]);
            }
        }
    }
}

const char *ssolist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = bisso_cnt;
        return NULL;
    }
    
    return bisso[index].s;
}

static ListData sso_list(ssolist, &font);

static DIALOG ssolist_dlg[] =
{
    // (dialog proc)      (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,    0,   255,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Select Object Type", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_abclist_proc,    4,   24,   247,  95,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0, (void *) &sso_list, NULL, NULL },
    { jwin_button_proc,    65,  123,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,   128,  123,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void doNewSubscreenObject(int32_t type)
{
    subscreen_object tempsso;
	memset(&tempsso,0,sizeof(subscreen_object));
	//tempsso.dp1=(char *)malloc(2);
	tempsso.dp1 = new char[2];
	((char *)tempsso.dp1)[0]=0;
	tempsso.type=type;
	tempsso.pos = sspUP | sspDOWN | sspSCROLLING;
	tempsso.w=1;
	tempsso.h=1;
	
	switch(tempsso.type)
	{
		case ssoCURRENTITEM:
			tempsso.d2 = 1; // Should not be invisible!
			break;
		case ssoMAGICGAUGE:
			tempsso.d9 = -1; // 'Always show' by default
			break;
		case ssoCOUNTER:
			tempsso.d10 = -1; //(None) inf item
            tempsso.colortype1 = ssctMISC; //Default text color
			break;
		
		case ssoBSTIME: case ssoCOUNTERS: case ssoSSTIME:
        case ssoTIME: case ssoMINIMAPTITLE: case ssoSELECTEDITEMNAME:
        case ssoTEXT: case ssoTEXTBOX:
			tempsso.colortype1 = ssctMISC; //Default text color
			break;
	}
		
	int32_t temp_cso=curr_subscreen_object;
	curr_subscreen_object=ss_objects(css);
	
	if(sso_properties(&tempsso)!=-1)
	{
		if(css->objects[curr_subscreen_object].dp1!=NULL)
		{
			delete [](char *)css->objects[curr_subscreen_object].dp1;
			css->objects[curr_subscreen_object].dp1=NULL;
		}
		
		css->objects[curr_subscreen_object]=tempsso;
		update_sso_name();
		update_up_dn_btns();
	}
	else
	{
		curr_subscreen_object=temp_cso;
	}
}

std::string getssname(int32_t type)
{
	switch(type)
	{
		case ssoNULL: return "ssoNULL";
		case ssoNONE: return "ssoNONE";
		case sso2X2FRAME: return "sso2X2FRAME";
		case ssoTEXT: return "ssoTEXT";
		case ssoLINE: return "ssoLINE";
		case ssoRECT: return "ssoRECT";
		case ssoBSTIME: return "ssoBSTIME";
		case ssoTIME: return "ssoTIME";
		case ssoSSTIME: return "ssoSSTIME";
		case ssoMAGICMETER: return "ssoMAGICMETER";
		case ssoLIFEMETER: return "ssoLIFEMETER";
		case ssoBUTTONITEM: return "ssoBUTTONITEM";
		case ssoICON: return "ssoICON";
		case ssoCOUNTER: return "ssoCOUNTER";
		case ssoCOUNTERS: return "ssoCOUNTERS";
		case ssoMINIMAPTITLE: return "ssoMINIMAPTITLE";
		case ssoMINIMAP: return "ssoMINIMAP";
		case ssoLARGEMAP: return "ssoLARGEMAP";
		case ssoCLEAR: return "ssoCLEAR";
		case ssoCURRENTITEM: return "ssoCURRENTITEM";
		case ssoITEM: return "ssoITEM";
		case ssoTRIFRAME: return "ssoTRIFRAME";
		case ssoTRIFORCE: return "ssoTRIFORCE";
		case ssoTILEBLOCK: return "ssoTILEBLOCK";
		case ssoMINITILE: return "ssoMINITILE";
		case ssoSELECTOR1: return "ssoSELECTOR1";
		case ssoSELECTOR2: return "ssoSELECTOR2";
		case ssoMAGICGAUGE: return "ssoMAGICGAUGE";
		case ssoLIFEGAUGE: return "ssoLIFEGAUGE";
		case ssoTEXTBOX: return "ssoTEXTBOX";
		case ssoCURRENTITEMTILE: return "ssoCURRENTITEMTILE";
		case ssoSELECTEDITEMTILE: return "ssoSELECTEDITEMTILE";
		case ssoCURRENTITEMTEXT: return "ssoCURRENTITEMTEXT";
		case ssoCURRENTITEMNAME: return "ssoCURRENTITEMNAME";
		case ssoSELECTEDITEMNAME: return "ssoSELECTEDITEMNAME";
		case ssoCURRENTITEMCLASSTEXT: return "ssoCURRENTITEMCLASSTEXT";
		case ssoCURRENTITEMCLASSNAME: return "ssoCURRENTITEMCLASSNAME";
		case ssoSELECTEDITEMCLASSNAME: return "ssoSELECTEDITEMCLASSNAME";
		case ssoMAX: return "ssoMAX";
	}
	return "NIL_UNDEFINED_VAL";
}

int32_t onNewSubscreenObject()
{
    int32_t ret=-1;
    ssolist_dlg[0].dp2=get_zc_font(font_lfont);
    build_bisso_list();
    
    large_dialog(ssolist_dlg);
        
    ret=zc_popup_dialog(ssolist_dlg,2);
    
    if(ret!=0&&ret!=4)
    {
		doNewSubscreenObject(bisso[ssolist_dlg[2].d1].i);
    }
    
    return D_O_K;
}


void align_objects(subscreen_group *tempss, bool *selection, int32_t align_type)
{
    int32_t l=sso_x(&tempss->objects[curr_subscreen_object]);
    int32_t t=sso_y(&tempss->objects[curr_subscreen_object]);
    int32_t w=sso_w(&tempss->objects[curr_subscreen_object]);
    int32_t h=sso_h(&tempss->objects[curr_subscreen_object]);
    
    switch(get_alignment(&tempss->objects[curr_subscreen_object]))
    {
    case sstaCENTER:
        l-=(w/2);
        break;
        
    case sstaRIGHT:
        l-=w;
        break;
        
    case sstaLEFT:
    default:
        break;
    }
    
    int32_t r=l+w-1;
    int32_t b=t+h-1;
    int32_t c=l+w/2;
    int32_t m=t+h/2;
    
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        if(selection[i]&&i!=curr_subscreen_object)
        {
            int32_t tl=sso_x(&tempss->objects[i]);
            int32_t tt=sso_y(&tempss->objects[i]);
            int32_t tw=sso_w(&tempss->objects[i]);
            int32_t th=sso_h(&tempss->objects[i]);
            
            switch(get_alignment(&tempss->objects[i]))
            {
            case sstaCENTER:
                tl-=(tw/2);
                break;
                
            case sstaRIGHT:
                tl-=tw;
                break;
                
            case sstaLEFT:
            default:
                break;
            }
            
            int32_t tr=tl+tw-1;
            int32_t tb=tt+th-1;
            int32_t tc=tl+tw/2;
            int32_t tm=tt+th/2;
            
            switch(align_type)
            {
            case ssoaBOTTOM:
                tempss->objects[i].y+=b-tb;
                break;
                
            case ssoaMIDDLE:
                tempss->objects[i].y+=m-tm;
                break;
                
            case ssoaTOP:
                tempss->objects[i].y+=t-tt;
                break;
                
            case ssoaRIGHT:
                tempss->objects[i].x+=r-tr;
                break;
                
            case ssoaCENTER:
                tempss->objects[i].x+=c-tc;
                break;
                
            case ssoaLEFT:
            default:
                tempss->objects[i].x+=l-tl;
                break;
            }
        }
    }
}

void grid_snap_objects(subscreen_group *tempss, bool *selection, int32_t snap_type)
{
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        if(selection[i]||i==curr_subscreen_object)
        {
            int32_t tl=sso_x(&tempss->objects[i]);
            int32_t tt=sso_y(&tempss->objects[i]);
            int32_t tw=sso_w(&tempss->objects[i]);
            int32_t th=sso_h(&tempss->objects[i]);
            
            switch(get_alignment(&tempss->objects[i]))
            {
            case sstaCENTER:
                tl-=(tw/2);
                break;
                
            case sstaRIGHT:
                tl-=tw;
                break;
                
            case sstaLEFT:
            default:
                break;
            }
            
            int32_t tr=tl+tw-1;
            int32_t tb=tt+th-1;
            int32_t tc=tl+tw/2;
            int32_t tm=tt+th/2;
            int32_t l1=(tl-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
            int32_t l2=l1+zinit.ss_grid_x;
            int32_t c1=(tc-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
            int32_t c2=c1+zinit.ss_grid_x;
            int32_t r1=(tr-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
            int32_t r2=r1+zinit.ss_grid_x;
            
            int32_t t1=(tt-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
            int32_t t2=t1+zinit.ss_grid_y;
            int32_t m1=(tm-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
            int32_t m2=m1+zinit.ss_grid_y;
            int32_t b1=(tb-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
            int32_t b2=b1+zinit.ss_grid_y;
            
            switch(snap_type)
            {
            case ssosBOTTOM:
                tempss->objects[i].y+=(abs(b1-tb)>abs(b2-tb))?(b2-tb):(b1-tb);
                break;
                
            case ssosMIDDLE:
                tempss->objects[i].y+=(abs(m1-tm)>abs(m2-tm))?(m2-tm):(m1-tm);
                break;
                
            case ssosTOP:
                tempss->objects[i].y+=(abs(t1-tt)>abs(t2-tt))?(t2-tt):(t1-tt);
                break;
                
            case ssosRIGHT:
                tempss->objects[i].x+=(abs(r1-tr)>abs(r2-tr))?(r2-tr):(r1-tr);
                break;
                
            case ssosCENTER:
                tempss->objects[i].x+=(abs(c1-tc)>abs(c2-tc))?(c2-tc):(c1-tc);
                break;
                
            case ssosLEFT:
            default:
                tempss->objects[i].x+=(abs(l1-tl)>abs(l2-tl))?(l2-tl):(l1-tl);
                break;
            }
        }
    }
}

typedef struct dist_obj
{
    int32_t index;
    int32_t l;
    int32_t t;
    int32_t w;
    int32_t h;
    int32_t r;
    int32_t b;
    int32_t c;
    int32_t m;
} dist_obj;

void distribute_objects(subscreen_group *tempss, bool *selection, int32_t distribute_type)
{
    //these are here to bypass compiler warnings about unused arguments
    selection=selection;
    
    int32_t count=0;
    dist_obj temp_do[MAXSUBSCREENITEMS];
    
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        if(sso_selection[i]==true||i==curr_subscreen_object)
        {
            temp_do[count].index=i;
            temp_do[count].l=sso_x(&tempss->objects[i]);
            temp_do[count].t=sso_y(&tempss->objects[i]);
            temp_do[count].w=sso_w(&tempss->objects[i]);
            temp_do[count].h=sso_h(&tempss->objects[i]);
            
            switch(get_alignment(&tempss->objects[i]))
            {
            case sstaCENTER:
                temp_do[count].l-=(temp_do[count].w/2);
                break;
                
            case sstaRIGHT:
                temp_do[count].l-=temp_do[count].w;
                break;
                
            case sstaLEFT:
            default:
                break;
            }
            
            temp_do[count].r=temp_do[count].l+temp_do[count].w-1;
            temp_do[count].b=temp_do[count].t+temp_do[count].h-1;
            temp_do[count].c=temp_do[count].l+temp_do[count].w/2;
            temp_do[count].m=temp_do[count].t+temp_do[count].h/2;
            ++count;
        }
    }
    
    if(count<3)
    {
        return;
    }
    
    //sort all objects in order of position, then index (yeah, bubble sort; sue me)
    dist_obj tempdo2;
    
    for(int32_t j=0; j<count-1; j++)
    {
        for(int32_t k=0; k<count-1-j; k++)
        {
            switch(distribute_type)
            {
            case ssodBOTTOM:
                if(temp_do[k+1].b<temp_do[k].b||((temp_do[k+1].b==temp_do[k].b)&&(temp_do[k+1].index<temp_do[k].index)))
                {
                    tempdo2=temp_do[k];
                    temp_do[k]=temp_do[k+1];
                    temp_do[k+1]=tempdo2;
                }
                
                break;
                
            case ssodMIDDLE:
                if(temp_do[k+1].m<temp_do[k].m||((temp_do[k+1].m==temp_do[k].m)&&(temp_do[k+1].index<temp_do[k].index)))
                {
                    tempdo2=temp_do[k];
                    temp_do[k]=temp_do[k+1];
                    temp_do[k+1]=tempdo2;
                }
                
                break;
                
            case ssodTOP:
                if(temp_do[k+1].t<temp_do[k].t||((temp_do[k+1].t==temp_do[k].t)&&(temp_do[k+1].index<temp_do[k].index)))
                {
                    tempdo2=temp_do[k];
                    temp_do[k]=temp_do[k+1];
                    temp_do[k+1]=tempdo2;
                }
                
                break;
                
            case ssodRIGHT:
                if(temp_do[k+1].r<temp_do[k].r||((temp_do[k+1].r==temp_do[k].r)&&(temp_do[k+1].index<temp_do[k].index)))
                {
                    tempdo2=temp_do[k];
                    temp_do[k]=temp_do[k+1];
                    temp_do[k+1]=tempdo2;
                }
                
                break;
                
            case ssodCENTER:
                if(temp_do[k+1].c<temp_do[k].c||((temp_do[k+1].c==temp_do[k].c)&&(temp_do[k+1].index<temp_do[k].index)))
                {
                    tempdo2=temp_do[k];
                    temp_do[k]=temp_do[k+1];
                    temp_do[k+1]=tempdo2;
                }
                
                break;
                
            case ssodLEFT:
            default:
                if(temp_do[k+1].l<temp_do[k].l||((temp_do[k+1].l==temp_do[k].l)&&(temp_do[k+1].index<temp_do[k].index)))
                {
                    tempdo2=temp_do[k];
                    temp_do[k]=temp_do[k+1];
                    temp_do[k+1]=tempdo2;
                }
                
                break;
            }
        }
    }
    
    int32_t ld=temp_do[count-1].l-temp_do[0].l;
    int32_t td=temp_do[count-1].t-temp_do[0].t;
    int32_t rd=temp_do[count-1].r-temp_do[0].r;
    int32_t bd=temp_do[count-1].b-temp_do[0].b;
    int32_t cd=temp_do[count-1].c-temp_do[0].c;
    int32_t md=temp_do[count-1].m-temp_do[0].m;
    
    for(int32_t i=1; i<count-1; ++i)
    {
        switch(distribute_type)
        {
        case ssodBOTTOM:
            tempss->objects[temp_do[i].index].y+=bd*i/(count-1)-temp_do[i].b+temp_do[0].b;
            break;
            
        case ssodMIDDLE:
            tempss->objects[temp_do[i].index].y+=md*i/(count-1)-temp_do[i].m+temp_do[0].m;
            break;
            
        case ssodTOP:
            tempss->objects[temp_do[i].index].y+=td*i/(count-1)-temp_do[i].t+temp_do[0].t;
            break;
            
        case ssodRIGHT:
            tempss->objects[temp_do[i].index].x+=rd*i/(count-1)-temp_do[i].r+temp_do[0].r;
            break;
            
        case ssodCENTER:
            tempss->objects[temp_do[i].index].x+=cd*i/(count-1)-temp_do[i].c+temp_do[0].c;
            break;
            
        case ssodLEFT:
        default:
            tempss->objects[temp_do[i].index].x+=ld*i/(count-1)-temp_do[i].l+temp_do[0].l;
            break;
        }
    }
    
}

int32_t onBringToFront()
{
    while(curr_subscreen_object<ss_objects(css)-1)
    {
        onBringForward();
    }
    
    return D_O_K;
}

int32_t onSendToBack()
{
    while(curr_subscreen_object>0)
    {
        onSendBackward();
    }
    
    return D_O_K;
}

int32_t onReverseArrangement()
{
    int32_t i=0;
    int32_t j=MAXSUBSCREENITEMS-1;
    subscreen_object tempsso;
    
    sso_selection[curr_subscreen_object]=true;
    
    while(true)
    {
        while(i<MAXSUBSCREENITEMS && !sso_selection[i])
            i++;
            
        while(j>=0 && !sso_selection[j])
            j--;
            
        if(i>=j)
        {
            sso_selection[curr_subscreen_object]=false;
            return D_O_K;
        }
        
        if(curr_subscreen_object==i)
            curr_subscreen_object=j;
        else if(curr_subscreen_object==j)
            curr_subscreen_object=i;
            
        tempsso=css->objects[i];
        css->objects[i]=css->objects[j];
        css->objects[j]=tempsso;
        
        i++;
        j--;
    }
}

int32_t onAlignLeft()
{
    align_objects(css, sso_selection, ssoaLEFT);
    return D_O_K;
}

int32_t onAlignCenter()
{
    align_objects(css, sso_selection, ssoaCENTER);
    return D_O_K;
}

int32_t onAlignRight()
{
    align_objects(css, sso_selection, ssoaRIGHT);
    return D_O_K;
}

int32_t onAlignTop()
{
    align_objects(css, sso_selection, ssoaTOP);
    return D_O_K;
}

int32_t onAlignMiddle()
{
    align_objects(css, sso_selection, ssoaMIDDLE);
    return D_O_K;
}

int32_t onAlignBottom()
{
    align_objects(css, sso_selection, ssoaBOTTOM);
    return D_O_K;
}

int32_t onDistributeLeft()
{
    distribute_objects(css, sso_selection, ssodLEFT);
    return D_O_K;
}

int32_t onDistributeCenter()
{
    distribute_objects(css, sso_selection, ssodCENTER);
    return D_O_K;
}

int32_t onDistributeRight()
{
    distribute_objects(css, sso_selection, ssodRIGHT);
    return D_O_K;
}

int32_t onDistributeTop()
{
    distribute_objects(css, sso_selection, ssodTOP);
    return D_O_K;
}

int32_t onDistributeMiddle()
{
    distribute_objects(css, sso_selection, ssodMIDDLE);
    return D_O_K;
}

int32_t onDistributeBottom()
{
    distribute_objects(css, sso_selection, ssodBOTTOM);
    return D_O_K;
}

int32_t onGridSnapLeft()
{
    grid_snap_objects(css, sso_selection, ssosLEFT);
    return D_O_K;
}

int32_t onGridSnapCenter()
{
    grid_snap_objects(css, sso_selection, ssosCENTER);
    return D_O_K;
}

int32_t onGridSnapRight()
{
    grid_snap_objects(css, sso_selection, ssosRIGHT);
    return D_O_K;
}

int32_t onGridSnapTop()
{
    grid_snap_objects(css, sso_selection, ssosTOP);
    return D_O_K;
}

int32_t onGridSnapMiddle()
{
    grid_snap_objects(css, sso_selection, ssosMIDDLE);
    return D_O_K;
}

int32_t onGridSnapBottom()
{
    grid_snap_objects(css, sso_selection, ssosBOTTOM);
    return D_O_K;
}

static int32_t onToggleInvis()
{
    bool show=!(zinit.ss_flags&ssflagSHOWINVIS);
    zinit.ss_flags&=~ssflagSHOWINVIS;
    zinit.ss_flags|=(show?ssflagSHOWINVIS:0);
    ss_view_menu[0].flags=zinit.ss_flags&ssflagSHOWINVIS?D_SELECTED:0;
    return D_O_K;
}

static int32_t onEditGrid()
{
    grid_dlg[0].dp2=get_zc_font(font_lfont);
    char xsize[11];
    char ysize[11];
    char xoffset[4];
    char yoffset[4];
    sprintf(xsize, "%d", zc_max(zinit.ss_grid_x,1));
    sprintf(ysize, "%d", zc_max(zinit.ss_grid_y,1));
    sprintf(xoffset, "%d", zinit.ss_grid_xofs);
    sprintf(yoffset, "%d", zinit.ss_grid_yofs);
    grid_dlg[4].dp=xsize;
    grid_dlg[6].dp=ysize;
    grid_dlg[8].dp=xoffset;
    grid_dlg[10].dp=yoffset;
    grid_dlg[12].d1=zinit.ss_grid_color;
    
    large_dialog(grid_dlg);
        
    int32_t ret = zc_popup_dialog(grid_dlg,2);
    
    if(ret==1)
    {
        zinit.ss_grid_x=zc_max(atoi(xsize),1);
        zinit.ss_grid_xofs=atoi(xoffset);
        zinit.ss_grid_y=zc_max(atoi(ysize),1);
        zinit.ss_grid_yofs=atoi(yoffset);
        zinit.ss_grid_color=grid_dlg[12].d1;
    }
    
    return D_O_K;
}

static int32_t onShowHideGrid()
{
    bool show=!(zinit.ss_flags&ssflagSHOWGRID);
    zinit.ss_flags&=~ssflagSHOWGRID;
    zinit.ss_flags|=(show?ssflagSHOWGRID:0);
    ss_view_menu[2].flags=zinit.ss_flags&ssflagSHOWGRID?D_SELECTED:0;
    return D_O_K;
}

int32_t onSelectionOptions()
{
    sel_options_dlg[0].dp2=get_zc_font(font_lfont);
    sel_options_dlg[6].d1=zinit.ss_bbox_1_color;
    sel_options_dlg[8].d1=zinit.ss_bbox_2_color;
    
    large_dialog(sel_options_dlg);
        
    int32_t ret = zc_popup_dialog(sel_options_dlg,2);
    
    if(ret==1)
    {
        zinit.ss_bbox_1_color=sel_options_dlg[6].d1;
        zinit.ss_bbox_2_color=sel_options_dlg[8].d1;
    }
    
    return D_O_K;
}


void update_up_dn_btns()
{
    if(curr_subscreen_object<1)
    {
        subscreen_dlg[10].flags|=D_DISABLED;
    }
    else
    {
        subscreen_dlg[10].flags&=~D_DISABLED;
    }
    
    if(curr_subscreen_object>=ss_objects(css)-1)
    {
        subscreen_dlg[9].flags|=D_DISABLED;
    }
    else
    {
        subscreen_dlg[9].flags&=~D_DISABLED;
    }
    
    subscreen_dlg[9].flags|=D_DIRTY;
    subscreen_dlg[10].flags|=D_DIRTY;
}

int32_t onSSCtrlPgUp()
{
    return onBringForward();
}

int32_t onSSCtrlPgDn()
{
    return onSendBackward();
}

int32_t onSendBackward()
{
    subscreen_object tempsso;
    bool tempsel;
    
    if(curr_subscreen_object>0)
    {
        tempsso=css->objects[curr_subscreen_object];
        tempsel=sso_selection[curr_subscreen_object];
        
        css->objects[curr_subscreen_object]=css->objects[curr_subscreen_object-1];
        sso_selection[curr_subscreen_object]=sso_selection[curr_subscreen_object-1];
        
        css->objects[curr_subscreen_object-1]=tempsso;
        sso_selection[curr_subscreen_object-1]=tempsel;
        
        --curr_subscreen_object;
        update_sso_name();
    }
    
    update_up_dn_btns();
    return D_O_K;
}

int32_t onSSPgDn()
{
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        return onSSCtrlPgDn();
    }
    else
    {
        --curr_subscreen_object;
        
        if(curr_subscreen_object<0)
        {
            curr_subscreen_object=ss_objects(css)-1;
        }
        
        update_sso_name();
        update_up_dn_btns();
    }
    
    return D_O_K;
}

// Send forward
int32_t onBringForward()
{
    subscreen_object tempsso;
    bool tempsel;
    
    if(curr_subscreen_object<ss_objects(css)-1)
    {
        tempsso=css->objects[curr_subscreen_object];
        tempsel=sso_selection[curr_subscreen_object];
        
        css->objects[curr_subscreen_object]=css->objects[curr_subscreen_object+1];
        sso_selection[curr_subscreen_object]=sso_selection[curr_subscreen_object+1];
        
        css->objects[curr_subscreen_object+1]=tempsso;
        sso_selection[curr_subscreen_object+1]=tempsel;
        
        ++curr_subscreen_object;
        update_sso_name();
    }
    
    update_up_dn_btns();
    return D_O_K;
}


int32_t onSSPgUp()
{
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        return onSSCtrlPgUp();
    }
    else
    {
        if(ss_objects(css)>0)
        {
            ++curr_subscreen_object;
            
            if(curr_subscreen_object>=ss_objects(css))
            {
                curr_subscreen_object=0;
            }
        }
        
        update_sso_name();
        update_up_dn_btns();
    }
    
    return D_O_K;
}

int32_t d_ssup_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        return onSSCtrlPgUp();
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        return onSSCtrlPgDn();
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSPgDn();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSPgUp();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}



void edit_subscreen()
{
    game = new gamedata;
    game->Clear();
    game->set_time(0);
    resetItems(game,&zinit,true);
    
    //so that these will show up on the subscreen -DD
    if(game->get_bombs() == 0)
        game->set_bombs(1);
        
    if(game->get_sbombs() == 0)
        game->set_sbombs(1);
        
    if(game->get_arrows() == 0)
        game->set_arrows(1);
        
    subscreen_dlg[0].dp2=get_zc_font(font_lfont);
    load_Sitems(&misc);
    curr_subscreen_object=0;
    ss_propCopySrc=-1;
    subscreen_group tempss;
    memset(&tempss, 0, sizeof(subscreen_group));
    int32_t i;
    
    for(i=0; i<MAXSUBSCREENITEMS; i++)
    {
        memcpy(&tempss.objects[i],&css->objects[i],sizeof(subscreen_object));
        
        switch(css->objects[i].type)
        {
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoCURRENTITEMTEXT:
        case ssoCURRENTITEMCLASSTEXT:
            tempss.objects[i].dp1 = NULL;
            tempss.objects[i].dp1 = new char[strlen((char*)css->objects[i].dp1)+1];
            strcpy((char*)tempss.objects[i].dp1,(char*)css->objects[i].dp1);
            break;
            
        default:
            break;
        }
    }
    
    tempss.ss_type=css->ss_type;
    strcpy(tempss.name, css->name);
    
    if(ss_objects(css)==0)
    {
        curr_subscreen_object=-1;
    }
    
    onClearSelection();
    ss_view_menu[0].flags=zinit.ss_flags&ssflagSHOWINVIS?D_SELECTED:0;
    ss_view_menu[2].flags=zinit.ss_flags&ssflagSHOWGRID?D_SELECTED:0;
    
    if(css->objects[0].type==ssoNULL)
    {
        css->objects[0].type=ssoNONE;
    }
    
    subscreen_dlg[4].dp=(void *)css;
    subscreen_dlg[5].fg=jwin_pal[jcBOX];
    subscreen_dlg[5].bg=jwin_pal[jcBOX];
    str_oname=(char *)malloc(255);
    subscreen_dlg[6].dp=(void *)str_oname;
    subscreen_dlg[8].dp=(void *)(css->name);
    update_sso_name();
    subscreen_dlg[10].flags|=D_DISABLED;
    
    if(css->ss_type==sstPASSIVE)
    {
        subscreen_dlg[21].flags|=D_DISABLED;
        subscreen_dlg[22].flags|=D_DISABLED;
        subscreen_dlg[23].flags|=D_DISABLED;
        subscreen_dlg[24].flags|=D_DISABLED;
    }
    else
    {
        subscreen_dlg[21].flags&=~D_DISABLED;
        subscreen_dlg[22].flags&=~D_DISABLED;
        subscreen_dlg[23].flags&=~D_DISABLED;
        subscreen_dlg[24].flags&=~D_DISABLED;
    }
    
    selectBwpn(0, 0);
    
	bool enlarge = subscreen_dlg[0].d1==0;
	
	if(enlarge)
	{
		large_dialog(subscreen_dlg,2);
		subscreen_dlg[4].y-=32;
		subscreen_dlg[3].y-=31;
		subscreen_dlg[3].x+=1;
		
		if(css->ss_type == sstPASSIVE)
			subscreen_dlg[3].h=60*2-4;
		else if(css->ss_type == sstACTIVE)
			subscreen_dlg[3].h=172*2-4;
			
		subscreen_dlg[4].h=subscreen_dlg[3].h-4;
	}
	
    int32_t ret = zc_popup_dialog(subscreen_dlg,2);
    
    if(ret==1)
    {
        saved=false;
        zinit.subscreen=ssdtMAX;
    }
    else
    {
        reset_subscreen(css);
        int32_t j;
        
        for(j=0; j<MAXSUBSCREENITEMS; j++)
        {
            memcpy(&css->objects[j],&tempss.objects[j],sizeof(subscreen_object));
            
            switch(tempss.objects[j].type)
            {
            case ssoTEXT:
            case ssoTEXTBOX:
            case ssoCURRENTITEMTEXT:
            case ssoCURRENTITEMCLASSTEXT:
                css->objects[j].dp1 = NULL;
                css->objects[j].dp1 = new char[strlen((char*)tempss.objects[j].dp1)+1];
                strcpy((char*)css->objects[j].dp1,(char*)tempss.objects[j].dp1);
                break;
                
            default:
                break;
            }
        }
        
        css->ss_type=tempss.ss_type;
        strcpy(css->name, tempss.name);
        reset_subscreen(&tempss);
    }
    
    delete game;
    game=NULL;
}

const char *allsubscrtype_str[30] =
{
    "Original (Top, Triforce)", "Original (Top, Map)",
    "New Subscreen (Top, Triforce)", "New Subscreen (Top, Map)",
    "Revision 2 (Top, Triforce)", "Revision 2 (Top, Map)",
    "BS Zelda Original (Top, Triforce)", "BS Zelda Original (Top, Map)",
    "BS Zelda Modified (Top, Triforce)", "BS Zelda Modified (Top, Map)",
    "BS Zelda Enhanced (Top, Triforce)", "BS Zelda Enhanced (Top, Map)",
    "BS Zelda Complete (Top, Triforce)", "BS Zelda Complete (Top, Map)",
    "Zelda 3 (Top)",
    "Original (Bottom, Magic)", "Original (Bottom, No Magic)",
    "New Subscreen (Bottom, Magic)", "New Subscreen (Bottom, No Magic)",
    "Revision 2 (Bottom, Magic)", "Revision 2 (Bottom, No Magic)",
    "BS Zelda Original (Bottom, Magic)", "BS Zelda Original (Bottom, No Magic)",
    "BS Zelda Modified (Bottom, Magic)", "BS Zelda Modified (Bottom, No Magic)",
    "BS Zelda Enhanced (Bottom, Magic)", "BS Zelda Enhanced (Bottom, No Magic)",
    "BS Zelda Complete (Bottom, Magic)", "BS Zelda Complete (Bottom, No Magic)",
    "Zelda 3 (Bottom)"
};

const char *activesubscrtype_str[16] =
{
    "Blank",
    "Original (Top, Triforce)", "Original (Top, Map)",
    "New Subscreen (Top, Triforce)", "New Subscreen (Top, Map)",
    "Revision 2 (Top, Triforce)", "Revision 2 (Top, Map)",
    "BS Zelda Original (Top, Triforce)", "BS Zelda Original (Top, Map)",
    "BS Zelda Modified (Top, Triforce)", "BS Zelda Modified (Top, Map)",
    "BS Zelda Enhanced (Top, Triforce)", "BS Zelda Enhanced (Top, Map)",
    "BS Zelda Complete (Top, Triforce)", "BS Zelda Complete (Top, Map)",
    "Zelda 3 (Top)"
};

const char *activelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return activesubscrtype_str[index];
}

const char *passivesubscrtype_str[16] =
{
    "Blank",
    "Original (Bottom, Magic)", "Original (Bottom, No Magic)",
    "New Subscreen (Bottom, Magic)", "New Subscreen (Bottom, No Magic)",
    "Revision 2 (Bottom, Magic)", "Revision 2 (Bottom, No Magic)",
    "BS Zelda Original (Bottom, Magic)", "BS Zelda Original (Bottom, No Magic)",
    "BS Zelda Modified (Bottom, Magic)", "BS Zelda Modified (Bottom, No Magic)",
    "BS Zelda Enhanced (Bottom, Magic)", "BS Zelda Enhanced (Bottom, No Magic)",
    "BS Zelda Complete (Bottom, Magic)", "BS Zelda Complete (Bottom, No Magic)",
    "Zelda 3 (Bottom)"
};

const char *passivelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return passivesubscrtype_str[index];
}

const char *activepassive_str[sstMAX] =
{
    "Active", "Passive"
};

const char *activepassivelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = sstMAX;
        return NULL;
    }
    
    return activepassive_str[index];
}

static ListData passive_list(passivelist, &font);
static ListData active_list(activelist, &font);

int32_t sstype_drop_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t tempd1=d->d1;
    int32_t ret=jwin_droplist_proc(msg,d,c);
    
    if(tempd1!=d->d1)
    {
        (d+1)->dp=(d->d1)?(void*)&passive_list:(void*)&active_list;
        object_message(d+1,MSG_START,0);
        (d+1)->flags|=D_DIRTY;
    }
    
    return ret;
}

static ListData activepassive_list(activepassivelist, &font);

static DIALOG sstemplatelist_dlg[] =
{
    // (dialog proc)         (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,          0,    0,   265,  87,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "New Subscreen", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_text_proc,         4,   28,     8,   8,   0,                  0,                0,       0,          0,             0, (void *) "Type:", NULL, NULL },
    { jwin_text_proc,         4,   46,     8,   8,   0,                  0,                0,       0,          0,             0, (void *) "Template:", NULL, NULL },
    { sstype_drop_proc,      33,   24,    72,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,         0,     0,             0, (void *) &activepassive_list, NULL, NULL },
    { jwin_droplist_proc,    50,   42,   211,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,         0,     0,             0, (void *) &active_list, NULL, NULL },
    { jwin_button_proc,      61,   62,    61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     142,   62,    61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

bool show_new_ss=true;

const char *subscreenlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        int32_t j=0;
        
        while(custom_subscreen[j].objects[0].type!=ssoNULL)
        {
            ++j;
        }
        
        *list_size = j+(show_new_ss?1:0);
        sprintf(custom_subscreen[j].name, "<New>");
        return NULL;
    }
    
    return custom_subscreen[index].name;
}

const char *subscreenlist_a(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        int32_t i=0, j=0;
        
        while(custom_subscreen[j].objects[0].type!=ssoNULL)
        {
            if(custom_subscreen[j].ss_type==sstACTIVE)
            {
                ++i;
            }
            
            ++j;
        }
        
        *list_size = i;
        return NULL;
    }
    
//  return custsubscrtype_str[index];
    int32_t i=-1, j=0;
    
    while(custom_subscreen[j].objects[0].type!=ssoNULL&&i!=index)
    {
        if(custom_subscreen[j].ss_type==sstACTIVE)
        {
            ++i;
        }
        
        ++j;
    }
    
    return custom_subscreen[j-1].name;
}

const char *subscreenlist_b(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        int32_t i=0, j=0;
        
        while(custom_subscreen[j].objects[0].type!=ssoNULL)
        {
            if(custom_subscreen[j].ss_type==sstPASSIVE)
            {
                ++i;
            }
            
            ++j;
        }
        
        *list_size = i;
        return NULL;
    }
    
//  return custsubscrtype_str[index];
    int32_t i=-1, j=0;
    
    while(custom_subscreen[j].name[0]&&i!=index)
    {
        if(custom_subscreen[j].ss_type==sstPASSIVE)
        {
            ++i;
        }
        
        ++j;
    }
    
    return custom_subscreen[j-1].name;
}

static ListData subscreen_list(subscreenlist, &font);

DIALOG sslist_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     0,   0,   234,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Select Subscreen", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_abclist_proc,    12,   24,   211,  95,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0, (void *) &subscreen_list, NULL, NULL },
    { jwin_button_proc,     12,   123,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Edit", NULL, NULL },
    { jwin_button_proc,     85,  123,  61,   21,   vc(14),  vc(1),  KEY_DEL,     D_EXIT,     0,             0, (void *) "Delete", NULL, NULL },
    { jwin_button_proc,     158,  123,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Done", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t onEditSubscreens()
{
    int32_t ret=-1;
    sslist_dlg[0].dp2=get_zc_font(font_lfont);
    sstemplatelist_dlg[0].dp2=get_zc_font(font_lfont);
    
    large_dialog(sslist_dlg);
        
    while(ret!=0&&ret!=5)
    {
        ret=zc_popup_dialog(sslist_dlg,2);
        
        if(ret==4)
        {
            int32_t confirm = jwin_alert("Confirm Delete", "You are about to delete the selected subscreen!", "Are you sure?", NULL, "OK", "Cancel", KEY_ENTER, KEY_ESC, get_zc_font(font_lfont));
            
            if(confirm==1)
            {
                delete_subscreen(sslist_dlg[2].d1);
                saved=false;
            }
        }
        else if(ret==2 || ret ==3)
        {
            if(custom_subscreen[sslist_dlg[2].d1].ss_type==sstACTIVE)
            {
                subscreen_dlg[3].h=340;
                subscreen_dlg[4].h=subscreen_dlg[3].h-4;
            }
            else if(custom_subscreen[sslist_dlg[2].d1].ss_type==sstPASSIVE)
            {
                subscreen_dlg[3].h=116;
                subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                //iu;hukl;kh;
            }
            else
            {
                subscreen_dlg[3].h=40;
                subscreen_dlg[4].h=subscreen_dlg[3].h-4;
            }
            
            css = &custom_subscreen[sslist_dlg[2].d1];
            bool edit_it=true;
            
            if(css->objects[0].type==ssoNULL)
            {
                large_dialog(sstemplatelist_dlg);
                    
                ret=zc_popup_dialog(sstemplatelist_dlg,4);
                
                if(ret==6)
                {
                    if(sstemplatelist_dlg[5].d1<15)
                    {
                        if(sstemplatelist_dlg[5].d1 != 0)
                        {
                            subscreen_object *tempsub;
                            
                            if(sstemplatelist_dlg[4].d1==0)
                            {
                                tempsub = default_subscreen_active[(sstemplatelist_dlg[5].d1-1)/2][(sstemplatelist_dlg[5].d1-1)&1];
                            }
                            else
                            {
                                tempsub = default_subscreen_passive[(sstemplatelist_dlg[5].d1-1)/2][(sstemplatelist_dlg[5].d1-1)&1];
                            }
                            
                            int32_t i;
                            
                            for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
                            {
                                switch(tempsub[i].type)
                                {
                                case ssoTEXT:
                                case ssoTEXTBOX:
                                case ssoCURRENTITEMTEXT:
                                case ssoCURRENTITEMCLASSTEXT:
                                    if(css->objects[i].dp1 != NULL) delete [](char *)css->objects[i].dp1;
                                    
                                    memcpy(&css->objects[i],&tempsub[i],sizeof(subscreen_object));
                                    css->objects[i].dp1 = NULL;
                                    css->objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                                    strcpy((char*)css->objects[i].dp1,(char*)tempsub[i].dp1);
                                    break;
                                    
                                default:
                                    memcpy(&css->objects[i],&tempsub[i],sizeof(subscreen_object));
                                    break;
                                }
                            }
                        }
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            css->ss_type=sstACTIVE;
                            strcpy(css->name, activesubscrtype_str[sstemplatelist_dlg[5].d1]);
                            subscreen_dlg[3].h=172*2;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                        else
                        {
                            css->ss_type=sstPASSIVE;
                            strcpy(css->name, passivesubscrtype_str[sstemplatelist_dlg[5].d1]);
                            subscreen_dlg[3].h=120;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                    }
                    else //Z3
                    {
                        subscreen_object *tempsub;
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            tempsub = z3_active_a;
                        }
                        else
                        {
                            tempsub = z3_passive_a;
                        }
                        
                        int32_t i;
                        
                        for(i=0; (i<MAXSUBSCREENITEMS&&tempsub[i].type!=ssoNULL); i++)
                        {
                            switch(tempsub[i].type)
                            {
                            case ssoTEXT:
                            case ssoTEXTBOX:
                            case ssoCURRENTITEMTEXT:
                            case ssoCURRENTITEMCLASSTEXT:
                                if(css->objects[i].dp1 != NULL) delete [](char *)css->objects[i].dp1;
                                
                                memcpy(&css->objects[i],&tempsub[i],sizeof(subscreen_object));
                                css->objects[i].dp1 = NULL;
                                css->objects[i].dp1 = new char[strlen((char*)tempsub[i].dp1)+1];
                                strcpy((char*)css->objects[i].dp1,(char*)tempsub[i].dp1);
                                break;
                                
                            default:
                                memcpy(&css->objects[i],&tempsub[i],sizeof(subscreen_object));
                                break;
                            }
                        }
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            css->ss_type=sstACTIVE;
                            strcpy(css->name, activesubscrtype_str[sstemplatelist_dlg[5].d1]);
                            subscreen_dlg[3].h=344;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                            
                        }
                        else
                        {
                            css->ss_type=sstPASSIVE;
                            strcpy(css->name, passivesubscrtype_str[sstemplatelist_dlg[5].d1]);
                            subscreen_dlg[3].h=120;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                    }
                }
                else
                {
                    edit_it=false;
                }
            }
            
            if(edit_it)
            {
                edit_subscreen();
            }
        }
    }
    
    position_mouse_z(0);
    return D_O_K;
}

void update_sso_name()
{
    if(curr_subscreen_object<0)
    {
        sprintf(str_oname, "No object selected");
    }
    else
    {
        sprintf(str_oname, "%3d:  %s", curr_subscreen_object, sso_name(css->objects, curr_subscreen_object));
    }
    
    subscreen_dlg[5].flags|=D_DIRTY;
    subscreen_dlg[6].flags|=D_DIRTY;
}

void center_zq_subscreen_dialogs()
{
    jwin_center_dialog(grid_dlg);
    jwin_center_dialog(sel_options_dlg);
    jwin_center_dialog(sslist_dlg);
    jwin_center_dialog(ssolist_dlg);
    jwin_center_dialog(sstemplatelist_dlg);
    jwin_center_dialog(subscreen_dlg);
}

void delete_subscreen(int32_t subscreenidx)
{
    if(custom_subscreen[subscreenidx].objects[0].type == ssoNULL)
        return;
        
    //delete
    reset_subscreen(&custom_subscreen[subscreenidx]);
    
    //and move all other subscreens up
    for(int32_t i=subscreenidx+1; i<MAXCUSTOMSUBSCREENS; i++)
    {
        memcpy(&custom_subscreen[i-1], &custom_subscreen[i], sizeof(subscreen_group));
    }
    
    //fix dmaps
    int32_t dmap_count=count_dmaps();
    
    for(int32_t i=0; i<dmap_count; i++)
    {
        //decrement
        if(DMaps[i].active_subscreen > subscreenidx)
            DMaps[i].active_subscreen--;
            
        if(DMaps[i].passive_subscreen > subscreenidx)
            DMaps[i].passive_subscreen--;
    }
}

// These were defined in ffscript.h; no need for them here
#undef DELAY
#undef WIDTH
#undef HEIGHT

#define D1        0x00000001
#define D2        0x00000002
#define D3        0x00000004
#define D4        0x00000008
#define D5        0x00000010
#define D6        0x00000020
#define D7        0x00000040
#define D8        0x00000080
#define D9        0x00000100
#define D10       0x00000200
#define D1_TO_D10 0x000003FF
#define COLOR1    0x00000400
#define COLOR2    0x00000800
#define COLOR3    0x00001000
#define FRAMES    0x00002000
#define FRAME     0x00004000
#define SPEED     0x00008000
#define DELAY     0x00010000
#define WIDTH     0x00020000
#define HEIGHT    0x00040000

// This function does the actual copying. Name sucks, but whatever.
// what controls which properties are copied. Type, x, y, and dp1
// are never copied. The active up/down/scrolling flags from pos
// are always copied, but the rest of it is not.
void doCopySSOProperties(subscreen_object& src, subscreen_object& dest, int32_t what)
{
    dest.pos&=~(sspUP|sspDOWN|sspSCROLLING);
    dest.pos|=src.pos&(sspUP|sspDOWN|sspSCROLLING);
    
    // Actually, I think pos is nothing but those three flags...
    
    if(what&WIDTH)
        dest.w=src.w;
    if(what&HEIGHT)
        dest.h=src.h;
    
    if(what&D1)
        dest.d1=src.d1;
    if(what&D2)
        dest.d2=src.d2;
    if(what&D3)
        dest.d3=src.d3;
    if(what&D4)
        dest.d4=src.d4;
    if(what&D5)
        dest.d5=src.d5;
    if(what&D6)
        dest.d6=src.d6;
    if(what&D7)
        dest.d7=src.d7;
    if(what&D8)
        dest.d8=src.d8;
    if(what&D9)
        dest.d9=src.d9;
    if(what&D10)
        dest.d10=src.d10;
    
    if(what&COLOR1)
    {
        dest.colortype1=src.colortype1;
        dest.color1=src.color1;
    }
    if(what&COLOR2)
    {
        dest.colortype2=src.colortype2;
        dest.color2=src.color2;
    }
    if(what&COLOR3)
    {
        dest.colortype3=src.colortype3;
        dest.color3=src.color3;
    }
    
    if(what&FRAMES)
        dest.frames=src.frames;
    if(what&FRAME)
        dest.frame=src.frame;
    if(what&SPEED)
        dest.speed=src.speed;
    if(what&DELAY)
        dest.delay=src.delay;
}

// Copies one object's properties to another. Selects properties depending on
// the object type; some things are deliberately skipped, like which heart
// container a life gauge piece corresponds to.
void copySSOProperties(subscreen_object& src, subscreen_object& dest)
{
    if(src.type!=dest.type || &src==&dest)
        return;
    
    switch(src.type)
    {
        case sso2X2FRAME:
            doCopySSOProperties(src, dest, D1|D2|D3|D4|COLOR1);
            break;
            
        case ssoTEXT:
            doCopySSOProperties(src, dest, D1|D2|D3|COLOR1|COLOR2|COLOR3);
            break;
            
        case ssoLINE:
            doCopySSOProperties(src, dest, D1|D2|COLOR1|WIDTH|HEIGHT);
            break;
            
        case ssoRECT:
            doCopySSOProperties(src, dest, D1|D2|COLOR1|COLOR2|WIDTH|HEIGHT);
            break;
            
        case ssoBSTIME:
        case ssoTIME:
        case ssoSSTIME:
            doCopySSOProperties(src, dest, D1|D2|D3|COLOR1|COLOR2|COLOR3);
            break;
            
        case ssoMAGICMETER: // Full meter
            // No properties but pos
            doCopySSOProperties(src, dest, 0);
            break;
            
        case ssoLIFEMETER:
            doCopySSOProperties(src, dest, D2|D3);
            break;
            
        case ssoBUTTONITEM:
            doCopySSOProperties(src, dest, D2);
            break;
            
        case ssoCOUNTER: // Single counter
            doCopySSOProperties(src, dest, D1|D2|D3|D4|D5|D6|COLOR1|COLOR2|COLOR3);
            break;
            
        case ssoCOUNTERS: // Counter block
            doCopySSOProperties(src, dest, D1|D2|D3|D4|D5|COLOR1|COLOR2|COLOR3);
            break;
            
        case ssoMINIMAPTITLE:
            doCopySSOProperties(src, dest, D1|D2|D3|D4|COLOR1|COLOR2|COLOR3);
            break;
            
        case ssoMINIMAP:
            doCopySSOProperties(src, dest, D1|D2|D3|COLOR1|COLOR2|COLOR3);
            break;
            
        case ssoLARGEMAP:
            doCopySSOProperties(src, dest, D1|D2|D3|D10|COLOR1|COLOR2);
            break;
            
        case ssoCLEAR:
            doCopySSOProperties(src, dest, COLOR1);
            break;
            
        case ssoCURRENTITEM:
            // Only the invisible flag
            doCopySSOProperties(src, dest, D2);
            break;
            
        case ssoTRIFRAME:
            doCopySSOProperties(src, dest, D1|D2|D3|D4|D5|D6|D7|COLOR1|COLOR2);
            break;
            
        case ssoTRIFORCE: // Single piece
            doCopySSOProperties(src, dest, D1|D2|D3|D4|COLOR1);
            break;
            
        case ssoTILEBLOCK:
            doCopySSOProperties(src, dest, D1|D2|D3|D4|COLOR1|WIDTH|HEIGHT);
            break;
            
        case ssoMINITILE:
            // Does this one work at all?
            doCopySSOProperties(src, dest, D1|D2|D3|D4|D5|D6|COLOR1|WIDTH|HEIGHT);
            break;
            
        case ssoSELECTOR1:
        case ssoSELECTOR2:
            doCopySSOProperties(src, dest, D1|D2|D3|D4|D5|COLOR1);
            break;
            
        case ssoMAGICGAUGE: // Single piece
            // Skip magic container (d1)
            doCopySSOProperties(src, dest, (D1_TO_D10&~D1)|COLOR1|COLOR2|WIDTH|HEIGHT);
            break;
            
        case ssoLIFEGAUGE: // Single piece
            // Skip heart container (d1)
            doCopySSOProperties(src, dest, (D1_TO_D10&~D1)|COLOR1|COLOR2|WIDTH|HEIGHT);
            break;
            
        case ssoTEXTBOX:
        case ssoSELECTEDITEMNAME:
            doCopySSOProperties(src, dest, D1|D2|D3|D4|D5|COLOR1|COLOR2|COLOR3|WIDTH|HEIGHT);
            break;
    }
}

/*** end of subscr.cc ***/

