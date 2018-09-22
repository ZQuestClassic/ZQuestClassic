//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  title.cc
//
//  Title screen and intro for zelda.cc
//  Also has game loading and select screen code.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "zc_alleg.h"

#include "zdefs.h"
#include "zelda.h"
#include "zsys.h"
#include "qst.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "gui.h"
#include "sprite.h"
#include "subscr.h"
//#include "jwin.h"
#include "jwinfsel.h"
#include "title.h"
#include "gamedata.h"
#include "link.h"
#include "mem_debug.h"

#ifdef _MSC_VER
#define strupr _strupr
#define stricmp _stricmp
#define snprintf _snprintf
#endif

extern int loadlast;
extern int skipcont;
extern int skipicon;
extern FFScript FFCore;

bool load_custom_game(int file);

struct savedicon
{
    byte loaded;
    byte ring;
    byte pal[4][48];
    byte icon[4][128];
};

savedicon iconbuffer[15];

static bool chosecustomquest = false;

/********************************/
/*****   NES title screen   *****/
/********************************/

#define BKGDATA 224*256/32

static byte titlepal[24] =
{
    0x0F,0x00,0x10, 0x17,0x27,0x0F, 0x08,0x1A,0x28, 0x30,0x3B,0x22,
    0x30,0x3B,0x16, 0x17,0x27,0x0F, 0x08,0x1A,0x28, 0x30,0x3B,0x22
};

static byte darkpal1[24] =
{
    0x0F,0x0F,0x00, 0x01,0x11,0x0F, 0x0C,0x01,0x02, 0x00,0x01,0x0C,
    0x00,0x01,0x0C, 0x01,0x11,0x0F, 0x0C,0x01,0x02, 0x00,0x01,0x0C
};

static byte darkpal2[24] =
{
    0x0F,0x0F,0x00, 0x01,0x11,0x0F, 0x0F,0x0C,0x01, 0x01,0x0C,0x0F,
    0x01,0x0C,0x0F, 0x01,0x11,0x0F, 0x0F,0x0C,0x01, 0x01,0x0C,0x0F
};

static byte darkpal3[24] =
{
    0x0F,0x0F,0x0F, 0x01,0x0F,0x0F, 0x0F,0x0C,0x0F, 0x0F,0x0C,0x0F,
    0x0F,0x0C,0x0F, 0x01,0x0F,0x0F, 0x0F,0x0C,0x0F, 0x0F,0x0C,0x0F
};

static byte itemspal[24] =
{
    0x30,0x30,0x30, 0x21,0x30,0x30, 0x16,0x30,0x30, 0x29,0x1A,0x09,
    0x29,0x37,0x17, 0x02,0x22,0x30, 0x16,0x27,0x30, 0x0B,0x1B,0x2B
};

static void loadtitlepal(int clear,byte *dataofs,int shift)
{
    for(int i=0; i<4; i++)
    {
        RAMpal[CSET(i)+shift] = NESpal(clear);
        
        for(int c=1; c<4; c++)
            RAMpal[CSET(i)+c+shift] = NESpal(*dataofs++);
    }
    
    for(int i=6; i<10; i++)
    {
        RAMpal[CSET(i)+shift] = NESpal(clear);
        
        for(int c=1; c<4; c++)
            RAMpal[CSET(i)+c+shift] = NESpal(*dataofs++);
    }
    
    refreshpal=true;
}

static byte tricolor[] = {0x27,0x37,0x27,0x17,0x07,0x17};
static byte tridelay[] = {6,12,6,6,12,16};
static word dusktime[] = {522,522+8,522+14,522+19,522+23,522+26,522+28,522+30};
static byte duskcolor[]= {0x39,0x31,0x3C,0x3B,0x2C,0x1C,0x02,0x0C};
static byte wave[3]= {};

static void cyclewaves()
{
    for(int i=0; i<3; i++)
    {
        wave[i]+=2;
        
        if(wave[i]==50)
            wave[i]=0;
            
        int y=wave[i]+170;
        
        if(wave[i]<8)
        {
            overtile16(framebuf,204,80,y,4,0);
            overtile16(framebuf,205,96,y,4,0);
        }
        
        
        else if(wave[i]<16)
        {
            overtile16(framebuf,206,80,y,4,0);
            overtile16(framebuf,207,96,y,4,0);
        }
        else
        {
            overtile16(framebuf,208,80,y,4,0);
            overtile16(framebuf,209,96,y,4,0);
        }
    }
}

static byte tri,fcnt;

static void mainscreen(int f)
{
    if(f>=1010)
        return;
        
    if(f==0)
    {
        blit((BITMAP*)data[BMP_TITLE_NES].dat,scrollbuf,0,0,0,0,256,224);
        blit(scrollbuf,framebuf,0,0,0,0,256,224);
        char tbuf[80];
        sprintf(tbuf, "%c1986 NINTENDO", 0xBB);
        textout_ex(framebuf,zfont,tbuf,104,128,13,-1);
        sprintf(tbuf, "%c" COPYRIGHT_YEAR " AG", 0xBC);
        //tbuf[0]=(char)0xBC;
        textout_ex(framebuf,zfont,tbuf,104,136,13,-1);
    }
    
    if(f<554+192+10)
    {
        blit(scrollbuf,framebuf,80,160,80,160,32,64);
        
        if(f&8)
        {
            puttile16(framebuf,200,80,160,4,0);
            puttile16(framebuf,201,96,160,4,0);
        }
        else
        {
            puttile16(framebuf,202,80,160,4,0);
            puttile16(framebuf,203,96,160,4,0);
        }
        
        cyclewaves();
    }
    
    if(f<58*9)
        ++fcnt;
        
    if(fcnt==tridelay[tri])
    {
        fcnt=0;
        tri = tri<5 ? tri+1 : 0;
        RAMpal[CSET(2)+2]=NESpal(tricolor[tri]);
        refreshpal=true;
    }
    
    for(int i=0; i<8; i++)
    {
        if(f==dusktime[i])
            loadtitlepal(duskcolor[i],titlepal,4);
    }
    
    if(f==554)
        loadtitlepal(0x0F,darkpal1,4);
        
    if(f==554+192)
        loadtitlepal(0x0F,darkpal2,4);
        
    if(f==554+192+6)
        loadtitlepal(0x0F,darkpal3,4);
        
    if(f==554+192+10)
    {
        clear_bitmap(framebuf);
        clear_bitmap(scrollbuf);
    }
    
    if(f==554+192+10+193)
        loadtitlepal(0x0F,itemspal,0);
}

void putstring(int x,int y,const char* str,int cset)
{
    textout_ex(scrollbuf,zfont,str,x,y,(cset<<CSET_SHFT)+1,0);
}

//#define ii    11
#define ii    14
//static byte vine[5] = { 2,3,6,7,10 };
static byte vine[5] = { 3,6,7,10,11 };

static void storyscreen(int f)
{
    if(f<1010)  return;
    
    if(f>=1804) return;
    
    if(f==1010)
    {
        puttile8(scrollbuf,vine[4],16,232,3,1);
        puttile8(scrollbuf,vine[2],24,232,3,1);
        puttile8(scrollbuf,vine[3],32,232,3,1);
        putstring(48,232,"THE LEGEND OF ZELDA",2);
        puttile8(scrollbuf,vine[3],208,232,3,0);
        puttile8(scrollbuf,vine[2],216,232,3,1);
        puttile8(scrollbuf,vine[3],224,232,3,1);
        puttile8(scrollbuf,vine[4],232,232,3,1);
    }
    
    if(f==16*3 +1010)
        putstring(32,232,"MANY  YEARS  AGO  PRINCE",0);
        
    if(f==16*4 +1010)
    {
        puttile8(scrollbuf,ii,104,232,2,0);
        puttile8(scrollbuf,ii,168,232,2,0);
    }
    
    if(f==16*5 +1010)
    {
        putstring(32,232,"DARKNESS",0);
        putstring(120,232,"GANNON",2);
        putstring(184,232,"STOLE",0);
    }
    
    if(f==16*7 +1010)
    {
        putstring(32,232,"ONE OF THE",0);
        putstring(120,232,"TRIFORCE",1);
        putstring(192,232,"WITH",0);
    }
    
    if(f==16*9 +1010)
    {
        putstring(32,232,"POWER.    PRINCESS",0);
        putstring(184,232,"ZELDA",2);
    }
    
    if(f==16*11 +1010)
    {
        putstring(32,232,"HAD  ONE OF THE",0);
        putstring(160,232,"TRIFORCE",1);
    }
    
    if(f==16*13 +1010)
        putstring(32,232,"WITH WISDOM. SHE DIVIDED",0);
        
    if(f==16*14 +1010)
    {
        puttile8(scrollbuf,ii,88,232,1,0);
        puttile8(scrollbuf,ii,112,232,1,0);
    }
    
    if(f==16*15 +1010)
    {
        putstring(32,232,"IT INTO    UNITS TO HIDE",0);
        putstring(104,232,"8",1);
    }
    
    if(f==16*16 +1010)
    {
        puttile8(scrollbuf,ii,96,232,2,0);
        puttile8(scrollbuf,ii,160,232,2,0);
    }
    
    if(f==16*17 +1010)
    {
        putstring(32,232,"IT FROM",0);
        putstring(112,232,"GANNON",2);
        putstring(176,232,"BEFORE",0);
    }
    
    if(f==16*19 +1010)
        putstring(32,232,"SHE WAS CAPTURED.",0);
        
    if(f==16*20 +1010)
    {
        puttile8(scrollbuf,ii,136,232,2,0);
        puttile8(scrollbuf,ii,160,232,2,0);
    }
    
    if(f==16*21 +1010)
        putstring(48,232,"GO FIND THE  8 UNITS",2);
        
    if(f==16*22 +1010)
    {
        puttile8(scrollbuf,ii,48,232,3,0);
        puttile8(scrollbuf,ii,96,232,3,0);
    }
    
    if(f==16*23 +1010)
    {
        putstring(64,232,"LINK",3);
        putstring(112,232,"TO SAVE HER.",2);
    }
    
    if(f==16*25 +1010)
    {
        puttile8(scrollbuf,vine[4],16,232,3,1);
        
        for(int x=24; x<232; x+=16)
        {
            puttile8(scrollbuf,vine[2],x,232,3,0);
            puttile8(scrollbuf,vine[3],x+8,232,3,0);
        }
        
        puttile8(scrollbuf,vine[4],232,232,3,1);
    }
    
    
    if((f>1010) && (f <= 16*24 + 1010))
    {
        if(((f-1010)&15) == 0)
        {
            if((f-1010)&16)
            {
                puttile8(scrollbuf,vine[0],16,232,3,0);
                puttile8(scrollbuf,vine[1],232,232,3,0);
            }
            else
            {
                puttile8(scrollbuf,vine[1],16,232,3,0);
                puttile8(scrollbuf,vine[0],232,232,3,0);
            }
        }
    }
    
    blit(scrollbuf,framebuf,0,8,0,0,256,224);
    
    if((f>=1010) && (f<= 16*26 + 1010) && (f&1))
        blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
        
}


static int trstr;
static byte tr_items[] =
{
    iHeart,iHeartC,iFairyMoving,iClock,iRupy,i5Rupies,iBPotion,iRPotion,iLetter,iBait,iSword,iWSword,iMSword,iShield,iBrang,iMBrang,iBombs,iBow,iArrow,iSArrow,iBCandle,iRCandle,
    iBRing,iRRing,iBracelet,iWhistle,iRaft,iLadder,iWand,iBook,iKey,iMKey,iMap,iCompass,iTriforce
};

static const char* treasure_str[] =
{
    "ALL OF TREASURES",
    "   HEART     CONTAINER     ",
    "                HEART      ",
    "   FAIRY        CLOCK      ",
    "                           ",
    "   RUPY       5 RUPIES     ",
    "                           ",
    "LIFE POTION   2ND POTION   ",
    "                           ",
    "  LETTER        FOOD       ",
    "                           ",
    "   SWORD        WHITE      ",
    "                SWORD      ",
    "  MAGICAL      MAGICAL     ",
    "   SWORD        SHIELD     ",
    " BOOMERANG     MAGICAL     ",
    "              BOOMERANG    ",
    "   BOMB          BOW       ",
    "                           ",
    "   ARROW        SILVER     ",
    "                ARROW      ",
    "   BLUE          RED       ",
    "   CANDLE       CANDLE     ",
    "   BLUE          RED       ",
    "   RING          RING      ",
    "   POWER       RECORDER    ",
    " BRACELET                  ",
    "   RAFT       STEPLADDER   ",
    "                           ",
    "  MAGICAL      BOOK OF     ",
    "    ROD         MAGIC      ",
    "    KEY        MAGICAL     ",
    "                 KEY       ",
    "    MAP        COMPASS     ",
    "                           ",
    "                           ",
    "        TRIFORCE           "
};

static void treasures(int f)
{
    if(f<1804) return;
    
    if(f>4492) return;
    
    if(f == 1804)
    
    {
        for(int x=0; x<48; x+=16)
        {
            puttile8(scrollbuf,vine[2],x,232,3,0);
            puttile8(scrollbuf,vine[3],x+8,232,3,0);
            puttile8(scrollbuf,vine[2],x+208,232,3,0);
            puttile8(scrollbuf,vine[3],x+216,232,3,0);
        }
        
        puttile8(scrollbuf,vine[4],48,232,3,1);
        puttile8(scrollbuf,vine[4],200,232,3,1);
        putstring(64,232,treasure_str[trstr++],0);
    }
    
    int y = (1820 + 96 + 448 - f) >>1;
    y += f&1;
    
    for(int i=0; i<34; i+=2)
    {
        if((y>=0)&&(y<240))
        {
            if(i==2)                                              // fairy
                puttile16(scrollbuf,((f&4)>>2)+63,68,y,8,0);
            else
                //      putitem(scrollbuf,68,y,tr_items[i],lens_hint_item[tr_items[i]][0], lens_hint_item[tr_items[i]][1], 0);
                //    putitem(scrollbuf,172,y,tr_items[i+1],lens_hint_item[tr_items[i+1]][0],lens_hint_item[tr_items[i+1]][1], 0);
                putitem(scrollbuf,68,y,tr_items[i]);
                
            putitem(scrollbuf,172,y,tr_items[i+1]);
        }
        
        y+=64;
    }
    
    y+=8;
    
    if((y>=0)&&(y<240))
        putitem(scrollbuf,120,y,iTriforce);
        
    y+=80;
    
    if(y>=0)
        for(int i=177; i<=217; i+=20)
        {
            if(y<240)
            {
                puttile16(scrollbuf,i,104,y,6,0);
                puttile16(scrollbuf,i+1,120,y,6,0);
                puttile16(scrollbuf,i+2,136,y,6,0);
            }
            
            y+=16;
        }
        
    if(y<240) //Link holding item
        puttile16(scrollbuf,29,120,y,6,0);
        
    if(f < 18*8*16 + 1820+128)
    {
        if(((f-1820)&15)==0)
        {
            int ax=(f-1820)>>4;
            
            if((ax&0xF8) && ((ax&7)<2))
                putstring(32,232,treasure_str[trstr++],0);
        }
    }
    
    blit(scrollbuf,framebuf,0,8,0,0,256,224);
}

static void NES_titlescreen()
{
    FFCore.kb_typing_mode = false;
    int f=0;
    bool done=false;
    wave[0]=0;
    wave[1]=16;
    wave[2]=32;
    tri=0;
    fcnt=0;
    trstr=0;
    set_palette(black_palette);
    try_zcmusic((char*)"zelda.nsf",0, ZC_MIDI_TITLE);
    clear_bitmap(screen);
    clear_bitmap(framebuf);
    init_NES_mode();
    reset_items(true, &QHeader);
    CSET_SIZE=4;
    CSET_SHFT=2;
    loadtitlepal(0x36,titlepal,4);
    ALLOFF();
    
    do
    {
        load_control_state();
        mainscreen(f);
        storyscreen(f);
        treasures(f);
        
        if((f>1676)&&(f<=4492)&&(f&1))
            blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
            
        if(f>=4492)
            putitem(framebuf,120,24,iTriforce);
            
        ++f;
        
        if((f>4750&&midi_pos<0) || f>6000)
        {
            wave[0]=0;
            wave[1]=16;
            wave[2]=32;
            tri=fcnt=trstr=f=0;
            clear_bitmap(framebuf);
            loadtitlepal(0x36,titlepal,4);
            music_stop();
            try_zcmusic((char*)"zelda.nsf",0, ZC_MIDI_TITLE);
        }
        
        advanceframe(true);
        
        if(rSbtn())
            done=true;
    }
    while(!done && !Quit);
    
    
    
    music_stop();
    clear_to_color(screen,BLACK);
    CSET_SIZE=16;
    CSET_SHFT=4;
}

/************************************/
/********  DX title screen  *********/
/************************************/

static void DX_mainscreen(int f)
{
    FFCore.kb_typing_mode = false;
    set_uformat(U_ASCII);
    
    static int pic=0;
    char tbuf[80];
    
    if(f>=1010)
        return;
        
    DATAFILE *dat = (DATAFILE*)data[TITLE_DX].dat;
    BITMAP *bmp;
    
    if(f==0)
    {
        copy_pal((RGB*)dat[TITLE_DX_PAL_1].dat,RAMpal);
        refreshpal=true;
    }
    
    if(f<680+256 && (f&15)==0)
    {
        bmp = (BITMAP*)dat[pic<4 ? pic : 6-pic].dat;
        pic = (pic+1)%6;
        blit(bmp,framebuf, 0,0, 0,0, 256,224);
        //    text_mode(-1);
        sprintf(tbuf, "%c1986 Nintendo",0xBB);
        //tbuf[0]=0xBB;
        textout_ex(framebuf,font,tbuf,46,138,255,-1);
        sprintf(tbuf, "%c" COPYRIGHT_YEAR " Armageddon Games",0xBC);
        //tbuf[0]=0xBC;
        textout_ex(framebuf,font,tbuf,46,146,255,-1);
        //    text_mode(0);
    }
    
    if(f>=680 && f<680+256 && (f%3)==0)
    {
        fade_interpolate((RGB*)dat[TITLE_DX_PAL_1].dat,black_palette,RAMpal,
                         (f-680)>>2,0,255);
        refreshpal=true;
    }
    
    if(f==680+256)
    {
        clear_bitmap(framebuf);
        clear_bitmap(scrollbuf);
    }
    
    if(f==680+256+2)
        loadtitlepal(0x0F,itemspal,0);
}

static void DX_titlescreen()
{
    //  JGMOD *yea;
    FFCore.kb_typing_mode = false;
    int f=0;
    bool done=false;
    trstr=0;
    set_palette(black_palette);
    
    try_zcmusic((char*)"zelda.nsf",0, ZC_MIDI_TITLE);
    clear_to_color(screen,BLACK);
    clear_bitmap(framebuf);
    init_NES_mode();
    reset_items(true, &QHeader);
    CSET_SIZE=4;
    CSET_SHFT=2;
    ALLOFF();
    clear_keybuf();
    
    do
    {
        load_control_state();
        DX_mainscreen(f);
        storyscreen(f);
        treasures(f);
        
        if((f>1676)&&(f<=4492)&&(f&1))
            blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
            
        if(f>=4492)
            putitem(framebuf,120,24,iTriforce);
            
        ++f;
        
        if((f>4750&&midi_pos<0) || f>6000)
        {
            trstr=f=0;
            clear_bitmap(framebuf);
            music_stop();
            try_zcmusic((char*)"zelda.nsf",0, ZC_MIDI_TITLE);
        }
        
        advanceframe(true);
        
        if(rSbtn())
            done=true;
    }
    while(!done && !Quit);
    
    music_stop();
    //  stop_mod();
    
    clear_to_color(screen,BLACK);
    CSET_SIZE=16;
    CSET_SHFT=4;
}

/*************************************/
/********  2.5 title screen  *********/
/*************************************/

static void v25_mainscreen(int f)
{
    FFCore.kb_typing_mode = false;
    set_uformat(U_ASCII);
    
    static int pic=0;
    char tbuf[80];
    
    if(f>=1010)
        return;
        
    DATAFILE *dat = (DATAFILE*)data[TITLE_25].dat;
    BITMAP *bmp;
    
    if(f==0)
    {
        copy_pal((RGB*)dat[TITLE_25_PAL_1].dat,RAMpal);
        refreshpal=true;
    }
    
    if(f<680+256 && (f&15)==0)
    {
        bmp = (BITMAP*)dat[pic<5 ? pic : 8-pic].dat;
        pic = (pic+1)%8;
        blit(bmp,framebuf, 0,0, 0,0, 256,224);
        //    text_mode(-1);
        sprintf(tbuf, "%c1986 Nintendo",0xBB);
        //tbuf[0]=0xBB;
        textout_ex(framebuf,font,tbuf,80,134,255,-1);
        sprintf(tbuf, "%c" COPYRIGHT_YEAR " Armageddon Games",0xBC);
        //tbuf[0]=0xBC;
        textout_ex(framebuf,font,tbuf,80,142,255,-1);
        //    text_mode(0);
    }
    
    if(f>=680 && f<680+256 && (f%3)==0)
    {
        fade_interpolate((RGB*)dat[TITLE_25_PAL_1].dat,black_palette,RAMpal,
                         (f-680)>>2,0,255);
        refreshpal=true;
    }
    
    if(f==680+256)
    {
        clear_bitmap(framebuf);
        clear_bitmap(scrollbuf);
    }
    
    if(f==680+256+2)
        loadtitlepal(0x0F,itemspal,0);
}

static void v25_titlescreen()
{
    FFCore.kb_typing_mode = false;
    //  JGMOD *yea;
    int f=0;
    bool done=false;
    trstr=0;
    set_palette(black_palette);
    
    try_zcmusic((char*)"zelda.nsf",0, ZC_MIDI_TITLE);
    clear_to_color(screen,BLACK);
    clear_bitmap(framebuf);
    init_NES_mode();
    reset_items(true, &QHeader);
    CSET_SIZE=4;
    CSET_SHFT=2;
    ALLOFF();
    clear_keybuf();
    
    do
    {
        load_control_state();
        v25_mainscreen(f);
        storyscreen(f);
        treasures(f);
        
        if((f>1676)&&(f<=4492)&&(f&1))
            blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
            
        if(f>=4492)
            putitem(framebuf,120,24,iTriforce);
            
        ++f;
        
        if((f>4750&&midi_pos<0) || f>6000)
        {
            trstr=f=0;
            clear_bitmap(framebuf);
            music_stop();
            try_zcmusic((char*)"zelda.nsf",0, ZC_MIDI_TITLE);
        }
        
        advanceframe(true);
        
        if(rSbtn())
            done=true;
    }
    while(!done && !Quit);
    
    music_stop();
    //  stop_mod();
    
    clear_to_color(screen,BLACK);
    CSET_SIZE=16;
    CSET_SHFT=4;
}

/***********************************/
/****  Game Selection Screens  *****/
/***********************************/

// first the game saving & loading system

static const char *SAVE_HEADER = "Zelda Classic Save File";
extern char *SAVE_FILE;

int readsaves(gamedata *savedata, PACKFILE *f)
{
    FFCore.kb_typing_mode = false;
    //word item_count;
    word qstpath_len;
    word save_count;
    char name[9];
    byte tempbyte;
    short tempshort;
    //  long templong;
    word tempword;
    dword tempdword;
    long section_id=0;
    word section_version=0;
    word section_cversion=0;
    dword section_size;
    
    //section id
    if(!p_mgetl(&section_id,f,true))
    {
        return 1;
    }
    
    //section version info
    if(!p_igetw(&section_version,f,true))
    {
        return 2;
    }
    
    if(!p_igetw(&section_cversion,f,true))
    {
        return 3;
    }
    
    if(section_version < 11) //Sorry!
    {
        //Currently unsupported
        return 1;
    }
    
    //section size
    if(!p_igetl(&section_size,f,true))
    {
        return 4;
    }
    
    if(!p_igetw(&save_count,f,true))
    {
        return 5;
    }
    
    // Excess saves would get deleted, so...
    if(standalone_mode && save_count>1)
    {
        system_pal();
        jwin_alert("Invalid save file",
                   "This save file cannot be",
                   "used in standalone mode.",
                   "",
                   "OK",NULL,'o',0,lfont);
        exit(0);
    }
    else if(!standalone_mode && save_count==1)
    {
        system_pal();
        
        if(jwin_alert3("Standalone save file",
                       "This save file was created in standalone mode.",
                       "If you continue, you will no longer be able",
                       "to use it in standalone mode. Continue anyway?",
                       "No","Yes",NULL, 'n','y', 0, lfont)!=2)
        {
            exit(0);
        }
    }
    
    for(int i=0; i<save_count; i++)
    {
        if(!pfread(name,9,f,true))
        {
            return 6;
        }
        
        savedata[i].set_name(name);
        
        if(!p_getc(&tempbyte,f,true))
        {
            return 7;
        }
        
        savedata[i].set_quest(tempbyte);
        
        if(section_version<3)
        {
            if(!p_igetw(&tempword,f,true))
            {
                return 8;
            }
            
            savedata[i].set_counter(tempword, 0);
            savedata[i].set_dcounter(tempword, 0);
            
            if(!p_igetw(&tempword,f,true))
            {
                return 9;
            }
            
            savedata[i].set_maxcounter(tempword, 0);
            
            if(!p_igetw(&tempshort,f,true))
            {
                return 10;
            }
            
            savedata[i].set_dcounter(tempshort, 1);
            
            if(!p_igetw(&tempword,f,true))
            {
                return 11;
            }
            
            savedata[i].set_counter(tempword, 1);
            
            if(!p_igetw(&tempword,f,true))
            {
                return 12;
            }
            
            savedata[i].set_counter(tempword, 3);
            savedata[i].set_dcounter(tempword, 3);
        }
        
        if(!p_igetw(&tempword,f,true))
        {
            return 13;
        }
        
        savedata[i].set_deaths(tempword);
        
        if(section_version<3)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return 14;
            }
            
            savedata[i].set_counter(tempbyte, 5);
            savedata[i].set_dcounter(tempbyte, 5);
            
            if(!p_getc(&tempbyte,f,true))
            {
                return 15;
            }
            
            savedata[i].set_maxcounter(tempbyte, 2);
        }
        
        if(section_version<4)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return 16;
            }
            
            savedata[i].set_wlevel(tempbyte);
        }
        
        if(!p_getc(&tempbyte,f,true))
        {
            return 17;
        }
        
        savedata[i].set_cheat(tempbyte);
        char temp;
        
        for(int j=0; j<256; j++) // why not MAXITEMS ?
        {
            if(!p_getc(&temp, f, true))
                return 18;
                
            savedata[i].set_item(j, (temp != 0));
        }
        
        if(!pfread(savedata[i].version,sizeof(savedata[i].version),f,true))
        {
            return 20;
        }
        
        if(!pfread(savedata[i].title,sizeof(savedata[i].title),f,true))
        {
            return 21;
        }
        
        if(!p_getc(&tempbyte,f,true))
        {
            return 22;
        }
        
        savedata[i].set_hasplayed(tempbyte);
        
        if(!p_igetl(&tempdword,f,true))
        {
            return 23;
        }
        
        savedata[i].set_time(tempdword);
        
        if(!p_getc(&tempbyte,f,true))
        {
            return 24;
        }
        
        savedata[i].set_timevalid(tempbyte);
        
        if(section_version <= 5)
        {
            for(int j=0; j<OLDMAXLEVELS; ++j)
            {
                if(!p_getc(&(savedata[i].lvlitems[j]),f,true))
                {
                    return 25;
                }
            }
        }
        else
        {
            for(int j=0; j<MAXLEVELS; ++j)
            {
                if(!p_getc(&(savedata[i].lvlitems[j]),f,true))
                {
                    return 25;
                }
            }
        }
        
        if(section_version<4)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return 26;
            }
            
            savedata[i].set_HCpieces(tempbyte);
        }
        
        if(!p_getc(&tempbyte,f,true))
        {
            return 27;
        }
        
        savedata[i].set_continue_scrn(tempbyte);
        
        if(section_version <= 5)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return 28;
            }
            
            savedata[i].set_continue_dmap(tempbyte);
        }
        else
        {
            if(!p_igetw(&tempword,f,true))
            {
                return 28;
            }
            
            savedata[i].set_continue_dmap(tempword);
        }
        
        if(section_version<3)
        {
            if(!p_igetw(&tempword,f,true))
            {
                return 29;
            }
            
            savedata[i].set_counter(tempword, 4);
            
            if(!p_igetw(&tempword,f,true))
            {
                return 30;
            }
            
            savedata[i].set_maxcounter(tempword, 4);
            
            if(!p_igetw(&tempshort,f,true))
            {
                return 31;
            }
            
            savedata[i].set_dcounter(tempshort, 4);
        }
        
        if(section_version<4)
        {
            if(!p_getc(&tempbyte,f,true))
            {
                return 32;
            }
            
            savedata[i].set_magicdrainrate(tempbyte);
            
            if(!p_getc(&tempbyte,f,true))
            {
                return 33;
            }
            
            savedata[i].set_canslash(tempbyte);
        }
        
        if(section_version <= 5)
        {
            for(int j=0; j<OLDMAXDMAPS; ++j)
            {
                if(!p_getc(&(savedata[i].visited[j]),f,true))
                {
                    return 34;
                }
            }
            
            for(int j=0; j<OLDMAXDMAPS*64; ++j)
            {
                if(!p_getc(&(savedata[i].bmaps[j]),f,true))
                {
                    return 35;
                }
            }
        }
        else
        {
            for(int j=0; j<MAXDMAPS; ++j)
            {
                if(!p_getc(&(savedata[i].visited[j]),f,true))
                {
                    return 34;
                }
            }
            
            for(int j=0; j<MAXDMAPS*64; ++j)
            {
                if(!p_getc(&(savedata[i].bmaps[j]),f,true))
                {
                    return 35;
                }
            }
        }
        
        for(int j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
        {
            if(!p_igetw(&savedata[i].maps[j],f,true))
            {
                return 36;
            }
        }
        
        for(int j=0; j<MAXMAPS2*MAPSCRSNORMAL; ++j)
        {
            if(!p_getc(&(savedata[i].guys[j]),f,true))
            {
                return 37;
            }
        }
        
        if(!p_igetw(&qstpath_len,f,true))
        {
            return 38;
        }
        
        if(!pfread(savedata[i].qstpath,qstpath_len,f,true))
        {
            return 39;
        }
        
        if(standalone_mode && strcmp(savedata[i].qstpath, standalone_quest)!=0)
        {
            system_pal();
            jwin_alert("Invalid save file",
                       "This save file is for",
                       "a different quest.",
                       "",
                       "OK",NULL,'o',0,lfont);
            exit(0);
        }
        
        // Convert path separators so save files work across platforms (hopefully)
        for(int j=0; j<qstpath_len; j++)
        {
#ifdef _ALLEGRO_WINDOWS
        
            if(savedata[i].qstpath[j]=='/')
            {
                savedata[i].qstpath[j]='\\';
            }
            
#else
            
            if(savedata[i].qstpath[j]=='\\')
            {
                savedata[i].qstpath[j]='/';
            }
            
#endif
        }
        
        savedata[i].qstpath[qstpath_len]=0;
        
        if(!pfread(savedata[i].icon,sizeof(savedata[i].icon),f,true))
        {
            return 40;
        }
        
        if(!pfread(savedata[i].pal,sizeof(savedata[i].pal),f,true))
        {
            return 41;
        }
        
        if(section_version <= 5)
        {
            for(int j=0; j<OLDMAXLEVELS; ++j)
            {
                if(!p_getc(&(savedata[i].lvlkeys[j]),f,true))
                {
                    return 42;
                }
            }
        }
        else
        {
            for(int j=0; j<MAXLEVELS; ++j)
            {
                if(!p_getc(&(savedata[i].lvlkeys[j]),f,true))
                {
                    return 42;
                }
            }
        }
        
        if(section_version>1)
        {
            if(section_version <= 5)
            {
                for(int j=0; j<OLDMAXDMAPS*64; j++)
                {
                    for(int k=0; k<8; k++)
                    {
                        if(!p_igetl(&savedata[i].screen_d[j][k],f,true))
                        {
                            return 43;
                        }
                    }
                }
            }
            else if(section_version < 10)
            {
                for(int j=0; j<MAXDMAPS*64; j++)
                {
                    for(int k=0; k<8; k++)
                    {
                        if(!p_igetl(&savedata[i].screen_d[j][k],f,true))
                        {
                            return 43;
                        }
                    }
                }
            }
            else
            {
                for(int j=0; j<MAXDMAPS*MAPSCRSNORMAL; j++)
                {
                    for(int k=0; k<8; k++)
                    {
                        if(!p_igetl(&savedata[i].screen_d[j][k],f,true))
                        {
                            return 43;
                        }
                    }
                }
            }
            
            for(int j=0; j<256; j++)
            {
                if(!p_igetl(&savedata[i].global_d[j],f,true))
                {
                    return 45;
                }
            }
        }
        
        if(section_version>2)
        {
            for(int j=0; j<32; j++)
            {
                if(!p_igetw(&tempword,f,true))
                {
                    return 46;
                }
                
                savedata[i].set_counter(tempword, j);
                
                if(!p_igetw(&tempword,f,true))
                {
                    return 47;
                }
                
                savedata[i].set_maxcounter(tempword, j);
                
                if(!p_igetw(&tempshort,f,true))
                {
                    return 48;
                }
                
                savedata[i].set_dcounter(tempshort, j);
            }
        }
        
        if(section_version>3)
        {
            for(int j=0; j<256; j++)
            {
                if(!p_getc(&tempbyte,f,true))
                {
                    return 49;
                }
                
                savedata[i].set_generic(tempbyte, j);
            }
        }
        
        if(section_version>6)
        {
            if(!p_getc(&tempbyte, f, true))
            {
                return 50;
            }
            
            savedata[i].awpn = tempbyte;
            
            if(!p_getc(&tempbyte, f, true))
            {
                return 51;
            }
            
            savedata[i].bwpn = tempbyte;
        }
        else
        {
            savedata[i].awpn = 0;
            savedata[i].bwpn = 0;
        }
        
        //First we get the size of the vector
        if(!p_igetl(&tempdword, f, true))
            return 53;
            
        if(tempdword != 0) //Might not be any at all
        {
            //Then we allocate the vector
            savedata[i].globalRAM.resize(tempdword);
            
            for(dword j = 0; j < savedata[i].globalRAM.size(); j++)
            {
                ZScriptArray& a = savedata[i].globalRAM[j];
                
                //We get the size of each container
                if(!p_igetl(&tempdword, f, true))
                    return 54;
                    
                //We allocate the container
                a.Resize(tempdword);
                
                //And then fill in the contents
                for(dword k = 0; k < a.Size(); k++)
                    if(!p_igetl(&(a[k]), f, true))
                        return 55;
            }
        }
    }
    
    return 0;
}

void set_up_standalone_save()
{
    char *fn=get_filename(standalone_quest);
    saves[0].set_name(fn);
    
    qstpath=(char*)zc_malloc(2048);
    strncpy(qstpath, standalone_quest, 2047);
    qstpath[2047]='\0';
    chosecustomquest=true;
    load_custom_game(0);
    
    // Why does the continue screen need set when
    // everything else gets set automatically?
    saves[0].set_continue_dmap(0);
    saves[0].set_continue_scrn(0xFF);
    saves[0].set_hasplayed(false);
    
    load_game_icon_to_buffer(false, 0);
    load_game_icon(saves, true, 0);
}

// call once at startup
int load_savedgames()
{
    FFCore.kb_typing_mode = false;
    char *fname = SAVE_FILE;
    char *iname = (char *)zc_malloc(2048);
    int ret;
    PACKFILE *f=NULL;
    FILE *f2=NULL;
    char tmpfilename[32];
    temp_name(tmpfilename);
//  const char *passwd = datapwd;

    if(saves == NULL)
    {
        saves = new gamedata[MAXSAVES];
        
        if(saves==NULL)
            return 1;
    }
    
    // see if it's there
    if(!exists(fname))
    {
        goto newdata;
    }
    
    if(file_size_ex_password(fname, "") == 0)
    {
        if(errno==0) // No error, file's empty
        {
            goto init;
        }
        else // Error...
        {
            goto cantopen;
        }
    }
    
    // decode to temp file
    ret = decode_file_007(fname, tmpfilename, SAVE_HEADER, ENC_METHOD_MAX-1, strstr(fname, ".dat#")!=NULL, "");
    
    if(ret)
    {
        goto cantopen;
    }
    
    fname = tmpfilename;
    
    // load the games
    f = pack_fopen_password(fname, F_READ_PACKED, "");
    
    if(!f)
        goto cantopen;
        
    if(readsaves(saves,f)!=0)
        goto reset;
        
    strcpy(iname, SAVE_FILE);
    
    for(int i=0; iname[i]!='\0'; iname[i]=='.'?iname[i]='\0':i++)
    {
        /* do nothing */
    }
    
    strcat(iname,".icn");
    
    if(!exists(iname))
    {
        byte *di2 = (byte *)iconbuffer;
        
        for(dword i=0; i<sizeof(savedicon)*MAXSAVES; i++)
            *(di2++) = 0;
    }
    else
    {
        f2=fopen(iname,"rb");
        byte *di2 = (byte *)iconbuffer;
        
        for(dword i=0; (i<sizeof(savedicon)*MAXSAVES)&&!feof(f2); i++)
            *(di2++) = fgetc(f2);
            
        fclose(f2);
    }
    
    //Load game icons
    for(int i=0; i<MAXSAVES; i++)
    {
        if(strlen(saves[i].qstpath))
        {
            if(skipicon)
            {
                for(int j=0; j<128; j++)
                {
                    saves[i].icon[j]=0;
                }
                
                for(int j=0; j<48; j++)
                {
                    saves[i].pal[j]=0;
                }
            }
            else
            {
                if(!iconbuffer[i].loaded)
                {
                    int ret2 = load_quest(saves+i, false);
                    
                    if(ret2 == qe_OK)
                    {
                        load_game_icon_to_buffer(false,i);
                    }
                }
            }
        }
    }
    
    pack_fclose(f);
    delete_file(tmpfilename);
    zc_free(iname);
    return 0;
    
newdata:
    system_pal();
    
    if(standalone_mode)
        goto init;
        
    if(jwin_alert("Can't Find Saved Game File",
                  "The save file could not be found.",
                  "Create a new file from scratch?",
                  "Warning: Doing so will erase any previous saved games!",
                  "OK","Cancel",13,27,lfont)!=1)
    {
        exit(1);
    }
    
    game_pal();
    Z_message("Save file not found.  Creating new save file.");
    goto init;
    
cantopen:
    {
        system_pal();
        char buf[256];
        snprintf(buf, 256, "still can't be opened, you'll need to delete %s.", SAVE_FILE);
        jwin_alert("Can't Open Saved Game File",
                   "The save file was found, but could not be opened. Wait a moment",
                   "and try again. If this problem persists, reboot. If the file",
                   buf,
                   "OK",NULL,'o',0,lfont);
    }
    exit(1);
    
reset:
    system_pal();
    
    if(jwin_alert3("Can't Open Saved Game File",
                   "Unable to read the save file.",
                   "Create a new file from scratch?",
                   "Warning: Doing so will erase any previous saved games!",
                   "No","Yes",NULL,'n','y',0,lfont)!=2)
    {
        exit(1);
    }
    
    game_pal();
    
    if(f)
        pack_fclose(f);
        
    delete_file(tmpfilename);
    Z_message("Format error.  Resetting game data... ");
    
init:

    for(int i=0; i<MAXSAVES; i++)
        saves[i].Clear();
        
    memset(iconbuffer, 0, sizeof(savedicon)*MAXSAVES);
    
    if(standalone_mode)
        set_up_standalone_save();
        
    zc_free(iname);
    return 0;
    
}


int writesaves(gamedata *savedata, PACKFILE *f)
{
    int section_id=ID_SAVEGAME;
    int section_version=V_SAVEGAME;
    int section_cversion=CV_SAVEGAME;
    int section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        return 1;
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        return 2;
    }
    
    if(!p_iputw(section_cversion,f))
    {
        return 3;
    }
    
    //section size
    if(!p_iputl(section_size,f))
    {
        return 4;
    }
    
    //word item_count=iMax;
    word qstpath_len=0;
    
    if(!p_iputw(MAXSAVES,f))
    {
        return 5;
    }
    
    for(int i=0; i<MAXSAVES; i++)
    {
        qstpath_len=(word)strlen(savedata[i].qstpath);
        
        if(!pfwrite(savedata[i].get_name(),9,f))
        {
            return 6;
        }
        
        if(!p_putc(savedata[i].get_quest(),f))
        {
            return 7;
        }
        
        if(!p_iputw(savedata[i].get_deaths(),f))
        {
            return 13;
        }
        
        if(!p_putc(savedata[i].get_cheat(),f))
        {
            return 17;
        }
        
        for(int j=0; j<MAXITEMS; j++)
        {
            if(!p_putc(savedata[i].get_item(j) ? 1 : 0,f))
                return 18;
        }
        
        if(!pfwrite(savedata[i].version,sizeof(savedata[i].version),f))
        {
            return 20;
        }
        
        if(!pfwrite(savedata[i].title,sizeof(savedata[i].title),f))
        {
            return 21;
        }
        
        if(!p_putc(savedata[i].get_hasplayed(),f))
        {
            return 22;
        }
        
        if(!p_iputl(savedata[i].get_time(),f))
        {
            return 23;
        }
        
        if(!p_putc(savedata[i].get_timevalid(),f))
        {
            return 24;
        }
        
        if(!pfwrite(savedata[i].lvlitems,MAXLEVELS,f))
        {
            return 25;
        }
        
        if(!p_putc(savedata[i].get_continue_scrn(),f))
        {
            return 27;
        }
        
        if(!p_iputw(savedata[i].get_continue_dmap(),f))
        {
            return 28;
        }
        
        if(!pfwrite(savedata[i].visited,MAXDMAPS,f))
        {
            return 34;
        }
        
        if(!pfwrite(savedata[i].bmaps,MAXDMAPS*64,f))
        {
            return 35;
        }
        
        for(int j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
        {
            if(!p_iputw(savedata[i].maps[j],f))
            {
                return 36;
            }
        }
        
        if(!pfwrite(savedata[i].guys,MAXMAPS2*MAPSCRSNORMAL,f))
        {
            return 37;
        }
        
        if(!p_iputw(qstpath_len,f))
        {
            return 38;
        }
        
        if(!pfwrite(savedata[i].qstpath,qstpath_len,f))
        {
            return 39;
        }
        
        if(!pfwrite(savedata[i].icon,sizeof(savedata[i].icon),f))
        {
            return 40;
        }
        
        if(!pfwrite(savedata[i].pal,sizeof(savedata[i].pal),f))
        {
            return 41;
        }
        
        if(!pfwrite(savedata[i].lvlkeys,MAXLEVELS,f))
        {
            return 42;
        }
        
        for(int j=0; j<MAXDMAPS*MAPSCRSNORMAL; j++)
        {
            for(int k=0; k<8; k++)
            {
                if(!p_iputl(savedata[i].screen_d[j][k],f))
                {
                    return 43;
                }
            }
        }
        
        for(int j=0; j<256; j++)
        {
            if(!p_iputl(savedata[i].global_d[j],f))
            {
                return 44;
            }
        }
        
        for(int j=0; j<32; j++)
        {
            if(!p_iputw(savedata[i].get_counter(j), f))
            {
                return 45;
            }
            
            if(!p_iputw(savedata[i].get_maxcounter(j), f))
            {
                return 46;
            }
            
            if(!p_iputw(savedata[i].get_dcounter(j), f))
            {
                return 47;
            }
        }
        
        for(int j=0; j<256; j++)
        {
            if(!p_putc(savedata[i].get_generic(j), f))
            {
                return 48;
            }
        }
        
        if(!p_putc(savedata[i].awpn, f))
        {
            return 49;
        }
        
        if(!p_putc(savedata[i].bwpn, f))
        {
            return 50;
        }
        
        //First we put the size of the vector
        if(!p_iputl(savedata[i].globalRAM.size(), f))
            return 51;
            
        for(dword j = 0; j < savedata[i].globalRAM.size(); j++)
        {
            ZScriptArray& a = savedata[i].globalRAM[j];
            
            //Then we put the size of each container
            if(!p_iputl(a.Size(), f))
                return 52;
                
            //Followed by its contents
            for(dword k = 0; k < a.Size(); k++)
                if(!p_iputl(a[k], f))
                    return 53;
        }
    }
    
    return 0;
}

int save_savedgames()
{
    if(saves==NULL)
        return 1;
    
    // Not sure why this happens, but apparently it does...
    for(int i=0; i<MAXSAVES; i++)
    {
        for(int j=0; j<48; j++)
        {
            saves[i].pal[j]&=63;
        }
    }
    
    char tmpfilename[32];
    temp_name(tmpfilename);
    
    PACKFILE *f = pack_fopen_password(tmpfilename, F_WRITE_PACKED, "");
    
    if(!f)
    {
        delete_file(tmpfilename);
        return 2;
    }
    
    if(writesaves(saves, f)!=0)
    {
        pack_fclose(f);
        delete_file(tmpfilename);
        return 4;
    }
    
    pack_fclose(f);
    int ret = encode_file_007(tmpfilename, SAVE_FILE, 0x413F0000 + (frame&0xffff), SAVE_HEADER, ENC_METHOD_MAX-1);
    
    if(ret)
        ret += 100;
        
    delete_file(tmpfilename);
    
    FILE *f2=NULL;
    char *iname = (char *)zc_malloc(2048);
    strcpy(iname, SAVE_FILE);
    
    for(int i=0; iname[i]!='\0'; iname[i]=='.'?iname[i]='\0':i++)
    {
        /* do nothing */
    }
    
    strcat(iname,".icn");
    
    f2=fopen(iname,"wb");
    byte *di2 = (byte *)iconbuffer;
    
    for(dword i=0; (i<sizeof(savedicon)*MAXSAVES); i++)
        fputc(*(di2++),f2);
        
    fclose(f2);
    zc_free(iname);
    return ret;
}

void load_game_icon(gamedata *g, bool, int index)
{
	//We need an override that fixes the palette here to prevent monochrome overwriting it. -Z
    int i=iconbuffer[index].ring; //
    
    byte *si = iconbuffer[index].icon[i];
    
    for(int j=0; j<128; j++)
    {
        g->icon[j] = *(si++);
    }
    
    si = iconbuffer[index].pal[i];
    
    for(int j=0; j<48; j++)
    {
        g->pal[j] = *(si++);
    }
}

void load_game_icon_to_buffer(bool forceDefault, int index)
{
    int ring=0;
    
    if(!forceDefault)
    {
        flushItemCache();
        int maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
        
        if(maxringid != -1)
        {
            ring = itemsbuf[maxringid].fam_type;
        }
    }
    
    //blue rings now start at level 2 for some reason, account for that -DD
    ring = ring ? ring-1 : 0;
    iconbuffer[index].ring = zc_min(ring, 3);
    
    int t=0;
    //if(!forceDefault)
    //t = QMisc.icons[i];
    
    for(int i=0; i<4; i++)
    {
        t = QMisc.icons[i];
        
        if(t<0 || t>=NEWMAXTILES)
        {
            t=0;
        }
        
        int tileind = t ? t : 28;
        
        byte *si = newtilebuf[tileind].data;
        
        if(newtilebuf[tileind].format==tf8Bit)
        {
            for(int j=0; j<128; j++)
            {
                iconbuffer[index].icon[i][j] =0;
            }
        }
        else
        {
            for(int j=0; j<128; j++)
            {
                iconbuffer[index].icon[i][j] = *(si++);
            }
        }
        
        if(t)
        {
            si = colordata + CSET(pSprite(i+spICON1))*3;
        }
        else
        {
            if(i)
            {
                si = colordata + CSET(pSprite(i-1+spBLUE))*3;
            }
            else
            {
                si = colordata + CSET(6)*3;
            }
        }
        
        if(newtilebuf[tileind].format==tf8Bit)
        {
            for(int j=0; j<48; j++)
            {
                iconbuffer[index].pal[i][j] = 0;
            }
        }
        else
        {
            for(int j=0; j<48; j++)
            {
                iconbuffer[index].pal[i][j] = *(si++);
            }
        }
    }
    
    iconbuffer[index].loaded=1;
}

static void select_mode()
{
    textout_ex(scrollbuf,zfont,"REGISTER YOUR NAME",48,152,1,0);
    textout_ex(scrollbuf,zfont,"COPY FILE",48,168,1,0);
    textout_ex(scrollbuf,zfont,"DELETE FILE",48,184,1,0);
}

static void register_mode()
{
    textout_ex(scrollbuf,zfont,"REGISTER YOUR NAME",48,152,CSET(2)+3,0);
}

static void copy_mode()
{
    textout_ex(scrollbuf,zfont,"COPY FILE",48,168,CSET(2)+3,0);
}

static void delete_mode()
{
    textout_ex(scrollbuf,zfont,"DELETE FILE",48,184,CSET(2)+3,0);
}

static void selectscreen()
{
    FFCore.kb_typing_mode = false;
    //  text_mode(0);
    init_NES_mode();
    //  loadfullpal();
    loadlvlpal(1);
    clear_bitmap(scrollbuf);
    QMisc.colors.blueframe_tile = 237;
    QMisc.colors.blueframe_cset = 0;
//  blueframe(scrollbuf,&QMisc,24,48,26,20);
    frame2x2(scrollbuf,&QMisc,24,48,QMisc.colors.blueframe_tile,QMisc.colors.blueframe_cset,26,20,0,1,0);
    textout_ex(scrollbuf,zfont,"- S E L E C T -",64,24,1,0);
    textout_ex(scrollbuf,zfont," NAME ",80,48,1,0);
    textout_ex(scrollbuf,zfont," LIFE ",152,48,1,0);
    select_mode();
    RAMpal[CSET(9)+1]=NESpal(0x15);
    RAMpal[CSET(9)+2]=NESpal(0x27);
    RAMpal[CSET(9)+3]=NESpal(0x30);
    RAMpal[CSET(13)+1]=NESpal(0x30);
}

static byte left_arrow_str[] = {132,0};
static byte right_arrow_str[] = {133,0};

static int savecnt;

static void list_save(int save_num, int ypos)
{
    bool r = refreshpal;
    
    if(save_num<savecnt)
    {
        game->set_maxlife(saves[save_num].get_maxlife());
        game->set_life(saves[save_num].get_maxlife());
        wpnsbuf[iwQuarterHearts].newtile = 4;
        //boogie!
        lifemeter(framebuf,144,ypos+((game->get_maxlife()>16*(HP_PER_HEART))?8:0),0,0);
        textout_ex(framebuf,zfont,saves[save_num].get_name(),72,ypos+16,1,0);
        
        if(saves[save_num].get_quest())
            textprintf_ex(framebuf,zfont,72,ypos+24,1,0,"%3d",saves[save_num].get_deaths());
            
        if(saves[save_num].get_quest()==2)
            overtile16(framebuf,41,56,ypos+14,9,0);             //put sword on second quests
            
        if(saves[save_num].get_quest()==3)
        {
            overtile16(framebuf,41,56,ypos+14,9,0);             //put sword on second quests
            overtile16(framebuf,41,41,ypos+14,9,0);             //put sword on third quests
        }
        
        if(saves[save_num].get_quest()==4)
        {
            overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
            overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
        }

		if(saves[save_num].get_quest()==5)
		{
			overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
		}

        textprintf_ex(framebuf,zfont,72,ypos+16,1,0,"%s",saves[save_num].get_name());
    }
    
    byte *hold = newtilebuf[0].data;
    byte holdformat=newtilebuf[0].format;
    newtilebuf[0].format=tf4Bit;
    newtilebuf[0].data = saves[save_num].icon;
    overtile16(framebuf,0,48,ypos+17,(save_num%3)+10,0);               //link
    newtilebuf[0].format=holdformat;
    newtilebuf[0].data = hold;
    
    hold = colordata;
    colordata = saves[save_num].pal;
    loadpalset((save_num%3)+10,0);
    colordata = hold;
    
    textout_ex(framebuf,zfont,"-",136,ypos+16,1,0);
    
    refreshpal = r;
}

static void list_saves()
{
    // Fourth Quest turns the menu red.
    bool red = false;
    
    for(int i=0; i<savecnt; i++)
        if(saves[i].get_quest()==4)
            red = true;
            
    loadpalset(0,red ? pSprite(spPILE) : 0);
    
    for(int i=0; i<3; i++)
    {
        list_save(listpos+i,i*24+56);
    }
    
    // Draw the arrows above the lifemeter!
    if(savecnt>3)
    {
        if(listpos>=3)
            textout_ex(framebuf,zfont,(char *)left_arrow_str,96,60,3,0);
            
        if(listpos+3<savecnt)
            textout_ex(framebuf,zfont,(char *)right_arrow_str,176,60,3,0);
            
        textprintf_ex(framebuf,zfont,112,60,3,0,"%2d - %-2d",listpos+1,listpos+3);
    }
    
    
}

static void draw_cursor(int pos,int mode)
{
    int cs = (mode==3)?13:9;
    
    if(pos<3)
        overtile8(framebuf,0,40,pos*24+77,cs,0);
    else
        overtile8(framebuf,0,40,(pos-3)*16+153,cs,0);
}

static bool register_name()
{
    if(savecnt>=MAXSAVES)
        return false;
        
    int NameEntryMode2=NameEntryMode;
    
    saves[savecnt].set_maxlife(3*HP_PER_HEART);
    saves[savecnt].set_maxbombs(8);
    saves[savecnt].set_continue_dmap(0);
    saves[savecnt].set_continue_scrn(0xFF);
    
    int s=savecnt;
    ++savecnt;
    listpos=(s/3)*3;
//  clear_bitmap(framebuf);
    rectfill(framebuf,32,56,223,151,0);
    list_saves();
    blit(framebuf,scrollbuf,0,0,0,0,256,224);
    
    int pos=s%3;
    int y=((NameEntryMode2>0)?0:(pos*24))+72;
    int x=0;
    int spos=0;
    char name[9];
    
    memset(name,0,9);
    register_mode();
    clear_keybuf();
    SystemKeys=(NameEntryMode2>0);
    refreshpal=true;
    bool done=false;
    bool cancel=false;
    
    int letter_grid_x=(NameEntryMode2==2)?34:44;
    int letter_grid_y=120;
    int letter_grid_offset=(NameEntryMode2==2)?10:8;
    int letter_grid_width=(NameEntryMode2==2)?16:11;
    int letter_grid_height=(NameEntryMode2==2)?6:4;
    int letter_grid_spacing=(NameEntryMode2==2)?12:16;
    
    const char *simple_grid="ABCDEFGHIJKLMNOPQRSTUVWXYZ-.,!'&.0123456789 ";
    const char *complete_grid=" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    
    if(NameEntryMode2>0)
    {
        //int pos=file%3;
        BITMAP *info = create_bitmap_ex(8,168,32);
        clear_bitmap(info);
        blit(framebuf,info,40,pos*24+70,0,0,168,26);
        rectfill(info,40,0,168,1,0);
        rectfill(info,0,24,39,25,0);
        rectfill(info,0,0,7,15,0);
        rectfill(framebuf,40,64,216,192,0);
        rectfill(framebuf,96,60,183,67,0);
        
        int i=pos*24+70;
        
        do
        {
            blit(info,framebuf,0,0,40,i,168,32);
            advanceframe(true);
            i-=pos+pos;
        }
        while(pos && i>=70);
        
        clear_bitmap(framebuf);
        frame2x2(framebuf,&QMisc,24,48,QMisc.colors.blueframe_tile,QMisc.colors.blueframe_cset,26,8,0,1,0);
        textout_ex(framebuf,zfont," NAME ",80,48,1,0);
        textout_ex(framebuf,zfont," LIFE ",152,48,1,0);
        
        blit(info,framebuf,0,0,40,70,168,32);
        destroy_bitmap(info);
        
        frame2x2(framebuf,&QMisc,letter_grid_x-letter_grid_offset,letter_grid_y-letter_grid_offset,QMisc.colors.blueframe_tile,QMisc.colors.blueframe_cset,(NameEntryMode2==2)?26:23,(NameEntryMode2==2)?11:9,0,1,0);
        
        if(NameEntryMode2==1)
        {
            textout_ex(framebuf,zfont,"A B C D E F G H I J K",letter_grid_x,letter_grid_y,1,-1);
            textout_ex(framebuf,zfont,"L M N O P Q R S T U V",letter_grid_x,letter_grid_y+16,1,-1);
            textout_ex(framebuf,zfont,"W X Y Z - . , ! ' & .",letter_grid_x,letter_grid_y+32,1,-1);
            textout_ex(framebuf,zfont,"0 1 2 3 4 5 6 7 8 9  ",letter_grid_x,letter_grid_y+48,1,-1);
        }
        else
        {
            textout_ex(framebuf,zfont,"   \"  $  &  (  *  ,  .",letter_grid_x,   letter_grid_y,   1,-1);
            textout_ex(framebuf,zfont, "!  #  %  '  )  +  -  /",letter_grid_x+12,letter_grid_y,   1,-1);
            textout_ex(framebuf,zfont,"0  2  4  6  8  :  <  >", letter_grid_x,   letter_grid_y+12,1,-1);
            textout_ex(framebuf,zfont, "1  3  5  7  9  ;  =  ?",letter_grid_x+12,letter_grid_y+12,1,-1);
            textout_ex(framebuf,zfont,"@  B  D  F  H  J  L  N", letter_grid_x,   letter_grid_y+24,1,-1);
            textout_ex(framebuf,zfont, "A  C  E  G  I  K  M  O",letter_grid_x+12,letter_grid_y+24,1,-1);
            textout_ex(framebuf,zfont,"P  R  T  V  X  Z  \\  ^",letter_grid_x,   letter_grid_y+36,1,-1);
            textout_ex(framebuf,zfont, "Q  S  U  W  Y  [  ]  _",letter_grid_x+12,letter_grid_y+36,1,-1);
            textout_ex(framebuf,zfont,"`  b  d  f  h  j  l  n", letter_grid_x,   letter_grid_y+48,1,-1);
            textout_ex(framebuf,zfont, "a  c  e  g  i  k  m  o",letter_grid_x+12,letter_grid_y+48,1,-1);
            textout_ex(framebuf,zfont,"p  r  t  v  x  z  |  ~", letter_grid_x,   letter_grid_y+60,1,-1);
            textout_ex(framebuf,zfont, "q  s  u  w  y  {  }",   letter_grid_x+12,letter_grid_y+60,1,-1);
        }
        
        advanceframe(true);
        blit(framebuf,scrollbuf,0,0,0,0,256,224);
        
    }
    
    int grid_x=0;
    int grid_y=0;
    
    
    do
    {
        if(NameEntryMode2>0)
        {
            spos = grid_y*letter_grid_width+grid_x;
            load_control_state();
            
            if(rLeft())
            {
                --grid_x;
                
                if(grid_x<0)
                {
                    grid_x=letter_grid_width-1;
                    --grid_y;
                    
                    if(grid_y<0)
                    {
                        grid_y=letter_grid_height-1;
                    }
                }
                
                sfx(WAV_CHIME);
            }
            else if(rRight())
            {
                ++grid_x;
                
                if(grid_x>=letter_grid_width)
                {
                    grid_x=0;
                    ++grid_y;
                    
                    if(grid_y>=letter_grid_height)
                    {
                        grid_y=0;
                    }
                }
                
                sfx(WAV_CHIME);
            }
            else if(rUp())
            {
                --grid_y;
                
                if(grid_y<0)
                {
                    grid_y=letter_grid_height-1;
                }
                
                sfx(WAV_CHIME);
            }
            else if(rDown())
            {
                ++grid_y;
                
                if(grid_y>=letter_grid_height)
                {
                    grid_y=0;
                }
                
                sfx(WAV_CHIME);
            }
            else if(rBbtn())
            {
                ++x;
                
                if(x>=8)
                {
                    x=0;
                }
            }
            else if(rAbtn())
            {
                name[zc_min(x,7)]=(NameEntryMode2==2)?complete_grid[spos]:simple_grid[spos];
                ++x;
                
                if(x>=8)
                {
                    x=0;
                }
                
                sfx(WAV_PLACE);
            }
            else if(rSbtn())
            {
                done=true;
                int ltrs=0;
                
                for(int i=0; i<8; i++)
                {
                    if(name[i]!=' ' && name[i]!=0)
                    {
                        ++ltrs;
                    }
                }
                
                if(!ltrs)
                {
                    cancel=true;
                }
            }
            
        }
        else
        {
            if(keypressed())
            {
                int k=readkey();
                
                if(isprint(k&255))
                {
                    name[zc_min(x,7)]=k&0xFF;
                    
                    if(x<8)
                    {
                        ++x;
                    }
                    
                    sfx(WAV_PLACE);
                }
                else
                {
                    switch(k>>8)
                    {
                    case KEY_LEFT:
                        if(x>0)
                        {
                            if(x==8)
                            {
                                x=6;
                            }
                            else
                            {
                                --x;
                            }
                            
                            sfx(WAV_CHIME);
                        }
                        
                        break;
                        
                    case KEY_RIGHT:
                        if(x<8 && name[zc_min(x,7)])
                        {
                            ++x;
                            sfx(WAV_CHIME);
                        }
                        
                        break;
                        
                    case KEY_ENTER:
                    case KEY_ENTER_PAD:
                    {
                        done=true;
                        int ltrs=0;
                        
                        for(int i=0; i<8; i++)
                        {
                            if(name[i]!=' ' && name[i]!=0)
                            {
                                ++ltrs;
                            }
                        }
                        
                        if(!ltrs)
                        {
                            cancel=true;
                        }
                    }
                    break;
                    
                    case KEY_BACKSPACE:
                        if(x>0)
                        {
                            --x;
                            
                            for(int i=zc_min(x,7); i<8; i++)
                            {
                                name[i]=name[i+1];
                            }
                            
                            sfx(WAV_OUCH);
                        }
                        
                        break;
                        
                    case KEY_DEL:
                        for(int i=zc_min(x,7); i<8; i++)
                        {
                            name[i]=name[i+1];
                        }
                        
                        sfx(WAV_OUCH);
                        break;
                        
                    case KEY_ESC:
                        x=-1;
                        done=true;
                        
                        while(key[KEY_ESC])
                        {
                            /* do nothing */
                        }
                        
                        break;
                    }
                }
            }
        }
        
        saves[s].set_name(name);
        blit(scrollbuf,framebuf,0,0,0,0,256,224);
//    list_saves();
        list_save(s,56+((NameEntryMode2>0)?0:(pos*24)));
        
        int x2=letter_grid_x + grid_x*letter_grid_spacing;
        int y2=letter_grid_y + grid_y*letter_grid_spacing;
        
        if(frame&8)
        {
            int tx=(zc_min(x,7)<<3)+72;
            
            for(int dy=0; dy<8; dy++)
            {
                for(int dx=0; dx<8; dx++)
                {
                    if(framebuf->line[y+dy][tx+dx]==0)
                    {
                        framebuf->line[y+dy][tx+dx]=CSET(9)+1;
                    }
                    
                    if(NameEntryMode2>0)
                    {
                        if(framebuf->line[y2+dy][x2+dx]==0)
                        {
                            framebuf->line[y2+dy][x2+dx]=CSET(9)+1;
                        }
                    }
                }
            }
        }
        
        draw_cursor((NameEntryMode2>0)?0:pos,0);
        advanceframe(true);
        /*
          if(rBbtn())
          {
          x=-1;
          done=true;
          }
          */
    }
    while(!done && !Quit);
    
    if(x<0 || cancel)
    {
        done=false;
    }
    
    if(done)
    {
        int quest=1;
        char buf[9];
        strcpy(buf,name);
        strupr(buf);
        
        if(!stricmp(buf,"ZELDA"))
            quest=2;
            
        if(!stricmp(buf,"ALPHA"))
            quest=3;
            
        if(!stricmp(buf,"GANON"))
            quest=4;
           
        if(!stricmp(buf,"JEAN")) // This is what BigJoe wanted. I have no problem with it.
			quest=5;
        
        saves[s].set_quest(quest);
        
//	setPackfilePassword(datapwd);
        //0 is success
        int ret = load_quest(saves+s);
        
        if(ret==qe_OK)
        {
            flushItemCache();
            //messy hack to get this to work, since game is not yet initialized -DD
            gamedata *oldgame = game;
            game = saves+s;
            saves[s].set_maxlife(zinit.hc*HP_PER_HEART);
            //saves[s].items[itype_ring]=0;
            removeItemsOfFamily(&saves[s], itemsbuf, itype_ring);
            int maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
            
            if(maxringid != -1)
                getitem(maxringid, true);
                
            //      game->set_maxbombs(&saves[s], zinit.max_bombs);
            selectscreen();                                       // refresh palette
            game = oldgame;
            ringcolor(false);
            load_game_icon_to_buffer(false,s);
            load_game_icon(saves+s, true, s);
        }
        else
        {
            ringcolor(true);
            load_game_icon(saves+s, true, s);
        }
        
//    setPackfilePassword(NULL);
        saves[s].set_timevalid(1);
    }
    
    if(x<0 || cancel)
    {
        for(int i=s; i<MAXSAVES-1; i++)
            saves[i]=saves[i+1];
            
        saves[MAXSAVES-1].Clear();
        --savecnt;
        
        if(listpos>savecnt-1)
            listpos=zc_max(listpos-3,0);
    }
    
    SystemKeys=true;
    selectscreen();
    list_saves();
    select_mode();
    return done;
}

static bool copy_file(int file)
{
    if(savecnt<MAXSAVES && file<savecnt)
    {
        saves[savecnt]=saves[file];
        iconbuffer[savecnt]=iconbuffer[file];
        ++savecnt;
        listpos=((savecnt-1)/3)*3;
        sfx(WAV_SCALE);
        select_mode();
        return true;
    }
    
    return false;
}

static bool delete_save(int file)
{
    if(file<savecnt)
    {
        for(int i=file; i<MAXSAVES-1; i++)
        {
            saves[i]=saves[i+1];
            iconbuffer[i]=iconbuffer[i+1];
        }
        
        saves[MAXSAVES-1].Clear();
        --savecnt;
        
        if(listpos>savecnt-1)
            listpos=zc_max(listpos-3,0);
            
        sfx(WAV_OUCH);
        select_mode();
        return true;
    }
    
    return false;
}

/** game mode stuff **/

DIALOG gamemode_dlg[] =
{
    // (dialog proc)      (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)     (d1)           (d2)     (dp)                            (dp2)  (dp3)
    { jwin_win_proc,       40,   44,  240,  180,   0,       0,       0,       D_EXIT,     0,             0, (void *) "Select Custom Quest", NULL,  NULL },
    // 1
    { jwin_button_proc,   205,   76,   61,   21,   0,       0,       'b',     D_EXIT,     0,             0, (void *) "&Browse",             NULL,  NULL },
    { jwin_textbox_proc,   55,   78,  140,   16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
    { jwin_text_proc,      76,  106,   64,    8,   vc(0),   vc(11),  0,       0,          0,             0, (void *) "Info:",               NULL,  NULL },
    { jwin_textbox_proc,   76,  118,  168,   60,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
    // 5
    { jwin_button_proc,    90,  191,   61,   21,   0,       0,       'k',     D_EXIT,     0,             0, (void *) "O&K",                 NULL,  NULL },
    { jwin_button_proc,   170,  191,   61,   21,   0,       0,       27,      D_EXIT,     0,             0, (void *) "Cancel",              NULL,  NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


static int get_quest_info(zquestheader *header,char *str)
{
    if(strlen(get_filename(qstpath)) == 0)
    {
        str[0]=0;
        return 0;
    }
    
    bool oldquest=false;
    
    // default error
    strcpy(str,"Error: Invalid quest file");
    
    char tmpfilename[32];
    temp_name(tmpfilename);
    int ret;
    PACKFILE *f;
    
    const char *passwd = datapwd;
    ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_MAX-1, strstr(qstpath, ".dat#")!=NULL, passwd);
    
    if(ret)
    {
        switch(ret)
        {
        case 1:
            strcpy(str,"Error: Unable to open file");
            break;
            
        case 2:
            strcpy(str,"Internal error occurred");
            break;
            // be sure not to delete tmpfilename now...
        }
        
        if(ret==5)                                              //old encryption?
        {
            ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_211B9, strstr(qstpath, ".dat#")!=NULL,passwd);
        }
        
        if(ret==5)                                              //old encryption?
        {
            ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_192B185, strstr(qstpath, ".dat#")!=NULL,passwd);
        }
        
        if(ret==5)                                              //old encryption?
        {
            ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_192B105, strstr(qstpath, ".dat#")!=NULL,passwd);
        }
        
        if(ret==5)                                              //old encryption?
        {
            ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_192B104, strstr(qstpath, ".dat#")!=NULL,passwd);
        }
        
        if(ret)
        {
            oldquest = true;
            passwd = "";
        }
    }
    
    f = pack_fopen_password(oldquest ? qstpath : tmpfilename, F_READ_PACKED, passwd);
    
    if(!f)
    {
        if(!oldquest&&(errno==EDOM))
        {
            f = pack_fopen_password(oldquest ? qstpath : tmpfilename, F_READ, passwd);
        }
        
        if(!f)
        {
            delete_file(tmpfilename);
        }
        
        strcpy(str,"Error: Unable to open file");
//	setPackfilePassword(NULL);
        return 0;
    }
    
    ret=readheader(f, header, true);
    
    if(f)
    {
        pack_fclose(f);
    }
    
    if(!oldquest)
    {
        delete_file(tmpfilename);
    }
    
//  setPackfilePassword(NULL);

    switch(ret)
    {
    case 0:
        break;
        
    case qe_invalid:
        strcpy(str,"Error: Invalid quest file");
        return 0;
        break;
        
    case qe_version:
        strcpy(str,"Error: Invalid version");
        return 0;
        break;
        
    case qe_obsolete:
        strcpy(str,"Error: Obsolete version");
        return 0;
        break;
    }
    
    if(header->quest_number > 0)
    {
        strcpy(str,
               (header->quest_number == 4) ? "Error: Not a custom quest! Clear the Second Quest with all 16 Heart Containers to play this." :
               (header->quest_number == 3) ? "Error: Not a custom quest! Clear the Second Quest to play this." :
               (header->quest_number > 1) ? "Error: Not a custom quest! Clear the First Quest to play this." :
               "Error: Not a custom quest! Create a new save file and press Start to play the First Quest.");
        return 0;
    }
    
    strcpy(str,"Title:\n");
    strcat(str,header->title);
    strcat(str,"\n\nAuthor:\n");
    strcat(str,header->author);
//  setPackfilePassword(NULL);

    return 1;
}

bool load_custom_game(int file)
{
    if(!saves[file].get_hasplayed())
    {
        if(chosecustomquest)
        {
            clear_to_color(screen,BLACK);
            saves[file].set_quest(0xFF);
            char temppath[2048];
            memset(temppath, 0, 2048);
            zc_make_relative_filename(temppath, qstdir, qstpath, 2047);
            
            if(temppath[0]==0)  //can't make relative, go absolute
            {
                sprintf(saves[file].qstpath, "%s", qstpath);
            }
            else
            {
                sprintf(saves[file].qstpath, "%s", temppath);
            }
            
            load_quest(saves+file);
            
            saves[file].set_maxlife(zinit.hc*HP_PER_HEART);
            flushItemCache();
            
            //messy hack to get this to work properly since game is not initialized -DD
            gamedata *oldgame = game;
            game = saves+file;
            int maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
            
            if(maxringid != -1)
                getitem(maxringid, true);
                
            rest(200); // Formerly 1000 -L
            ringcolor(false);
            load_game_icon_to_buffer(false,file);
            load_game_icon(game,false,file);
            game = oldgame;
            chosecustomquest = false;
            return true;
        }
    }
    
    return false;
}

int custom_game(int file)
{
    zquestheader h;
    char infostr[200];
    char path[2048];
    int ret=0; 
	 int focus_obj = 1; //Fixes the issue where the button tied to the enter key is stuck on 'browse'.
    
    if(is_relative_filename(saves[file].qstpath))
    {
        sprintf(qstpath,"%s%s",qstdir,saves[file].qstpath);
    }
    else
    {
        sprintf(qstpath,"%s", saves[file].qstpath);
    }
    
    gamemode_dlg[0].dp2 = lfont;
    gamemode_dlg[2].dp = get_filename(qstpath);
    
    if(get_quest_info(&h,infostr)==0)
    {
        gamemode_dlg[4].dp = infostr;
        gamemode_dlg[5].flags = D_DISABLED;
    }
    else
    {
        gamemode_dlg[4].dp = infostr;
        gamemode_dlg[5].flags = D_EXIT;
    }
    
    gamemode_dlg[2].d1 = gamemode_dlg[4].d1 = 0;
    gamemode_dlg[2].d2 = gamemode_dlg[4].d2 = 0;
    system_pal();
    show_mouse(screen);
    
    clear_keybuf();
    
    if(is_large)
        large_dialog(gamemode_dlg);
   
        
    while((ret=zc_popup_dialog(gamemode_dlg,focus_obj))==1)
    {
        scare_mouse();
        blit(screen,tmp_scr,scrx,scry,0,0,320,240);
        unscare_mouse();
        
        int  sel=0;
        static EXT_LIST list[] =
        {
            { (char *)"ZC Quests (*.qst)", (char *)"qst" },
            { NULL,                        NULL }
        };
        
        strcpy(path, qstpath);
        
        if(jwin_file_browse_ex("Load Quest", path, list, &sel, 2048, -1, -1, lfont))
        {
            //      strcpy(qstpath, path);
            replace_extension(qstpath,path,"qst",2047);
            gamemode_dlg[2].dp = get_filename(qstpath);
            
            if(get_quest_info(&h,infostr)==0)
            {
                gamemode_dlg[4].dp = infostr;
                gamemode_dlg[5].flags = D_DISABLED;
		    focus_obj = 1;
            }
            else
            {
                gamemode_dlg[4].dp = infostr;
                gamemode_dlg[5].flags = D_EXIT;
		    focus_obj = 5;
            }
            
            gamemode_dlg[2].d1 = gamemode_dlg[4].d1 = 0;
            gamemode_dlg[2].d2 = gamemode_dlg[4].d2 = 0;
        }
        
        scare_mouse();
        blit(tmp_scr,screen,0,0,scrx,scry,320,240);
        unscare_mouse();
    }
    
    show_mouse(NULL);
    game_pal();
    key[KEY_ESC]=0;
    chosecustomquest = (ret==5);
    return (int)chosecustomquest;
}

static int game_details(int file)
{

    int pos=file%3;
    
    if(saves[file].get_quest()==0)
        return 0;
        
    BITMAP *info = create_bitmap_ex(8,168,32);
    clear_bitmap(info);
    blit(framebuf,info,40,pos*24+70,0,0,168,26);
    rectfill(info,40,0,168,1,0);
    rectfill(info,0,24,39,25,0);
    rectfill(info,0,0,7,15,0);
    rectfill(framebuf,40,64,216,192,0);
    rectfill(framebuf,96,60,183,67,0);
    
    int i=pos*24+70;
    
    do
    {
        blit(info,framebuf,0,0,40,i,168,32);
        advanceframe(true);
        i-=pos+pos;
    }
    while(pos && i>=70);
    
    destroy_bitmap(info);
    
    textout_ex(framebuf,zfont,"GAME TYPE",40,104,3,0);
    textout_ex(framebuf,zfont,"QUEST",40,112,3,0);
    textout_ex(framebuf,zfont,"STATUS",40,120,3,0);
    
    if(saves[file].get_quest()<0xFF)
    {
        textout_ex(framebuf,zfont,"Normal Game",120,104,1,0);
        textprintf_ex(framebuf,zfont,120,112,1,0,"%s Quest",
                      ordinal(saves[file].get_quest()));
    }
    else
    {
        textout_ex(framebuf,zfont,"Custom Quest",120,104,1,0);
        textprintf_ex(framebuf,zfont,120,112,1,0,"%s",
                      get_filename(saves[file].qstpath));
    }
    
    if(!saves[file].get_hasplayed())
        textout_ex(framebuf,zfont,"Empty Game",120,120,1,0);
    else if(!saves[file].get_timevalid())
        textout_ex(framebuf,zfont,"Time Unknown",120,120,1,0);
    else
        textout_ex(framebuf,zfont,time_str_med(saves[file].get_time()),120,120,1,0);
        
    if(saves[file].get_cheat())
        textout_ex(framebuf,zfont,"Used Cheats",120,128,1,0);
        
    textout_ex(framebuf,zfont,"START: PLAY GAME",56,152,1,0);
    textout_ex(framebuf,zfont,"    B: CANCEL",56,168,1,0);
    
    if(!saves[file].get_hasplayed())
        textout_ex(framebuf,zfont,"    A: CUSTOM QUEST",56,184,1,0);
        
    while(!Quit)
    {
        advanceframe(true);
        load_control_state();
        
        if(rBbtn())
        {
            blit(scrollbuf,framebuf,0,0,0,0,256,224);
            return 0;
        }
        
        if(rSbtn())
        {
            blit(framebuf,scrollbuf,0,0,0,0,256,224);
            return 1;
        }
        
        if(rAbtn() && !saves[file].get_hasplayed())
        {
            (void)custom_game(file);
        }
        
        if(chosecustomquest && load_custom_game(file))
        {
            selectscreen();
            return 0;
        }
    }
    
    return 0;
}

static int saveslot = -1;

int getsaveslot()
{
    if(saveslot >= 0 && (!saves[saveslot].get_quest() || saves[saveslot].get_hasplayed()))
    {
        return -1;
    }
    
    return saveslot;
}

static void select_game()
{
    if(standalone_mode)
        return;
        
    int pos = zc_max(zc_min(currgame-listpos,3),0);
    int mode = 0;
    
    //kill_sfx();
    
    //  text_mode(0);
    selectscreen();
    
    savecnt=0;
    
    while(savecnt < MAXSAVES && saves[savecnt].get_quest()>0)
        ++savecnt;
        
    if(savecnt == 0)
        pos=3;
        
    bool done=false;
    refreshpal=true;
    
    do
    {
        load_control_state();
        sfxdat=1;
        blit(scrollbuf,framebuf,0,0,0,0,256,224);
        list_saves();
        draw_cursor(pos,mode);
        advanceframe(true);
        saveslot = pos + listpos;
        
        if(rSbtn())
            switch(pos)
            {
            case 3:
                if(!register_name())
                    pos = 3;
                else
                    pos = (savecnt-1)%3;
                    
                refreshpal=true;
                break;
                
            case 4:
                if(savecnt && savecnt<MAXSAVES)
                {
                    mode=2;
                    pos=0;
                    copy_mode();
                }
                
                refreshpal=true;
                break;
                
            case 5:
                if(savecnt)
                {
                    mode=3;
                    pos=0;
                    delete_mode();
                }
                
                refreshpal=true;
                break;
                
            default:
                switch(mode)
                {
                case 0:
                    currgame=saveslot;
                    loadlast=currgame+1;
                    
                    if(saves[currgame].get_quest())
                        done=true;
                        
                    break;
                    
                case 2:
                    if(copy_file(saveslot))
                    {
                        mode=0;
                        pos=(savecnt-1)%3;
                        refreshpal=true;
                    }
                    
                    break;
                    
                case 3:
                    if(delete_save(saveslot))
                    {
                        mode=0;
                        pos=3;
                        refreshpal=true;
                    }
                    
                    break;
                }
            }
            
        if(rUp())
        {
            --pos;
            
            if(pos<0)
                pos=(mode)?2:5;
                
            sfx(WAV_CHIME);
        }
        
        if(rDown())
        {
            ++pos;
            
            if(pos>((mode)?2:5))
                pos=0;
                
            sfx(WAV_CHIME);
        }
        
        if(rLeft() && listpos>2)
        {
            listpos-=3;
            sfx(WAV_CHIME);
            refreshpal=true;
        }
        
        if(rRight() && listpos+3<savecnt)
        {
            listpos+=3;
            sfx(WAV_CHIME);
            refreshpal=true;
        }
        
        if(rBbtn() && mode)
        {
            if(mode==2) pos=4;
            
            if(mode==3) pos=5;
            
            mode=0;
            select_mode();
        }
        
        if(rAbtn() && !mode && pos<3)
        {
            if(game_details(saveslot))
            {
                currgame=saveslot;
                loadlast=currgame+1;
                
                if(saves[currgame].get_quest())
                    done=true;
            }
        }
        
        if(chosecustomquest)
        {
            load_custom_game(saveslot);
            selectscreen();
        }
    }
    while(!Quit && !done);
    
    saveslot = -1;
}

/**************************************/
/****  Main title screen routine  *****/
/**************************************/

void titlescreen(int lsave)
{
    int q=Quit;
    
    Quit=0;
    Playing=Paused=false;
    FFCore.kb_typing_mode = false;
    
    if(q==qCONT)
    {
        cont_game();
        return;
    }
    
    if(q==qRESET && !skip_title)
    {
        show_subscreen_dmap_dots=true;
        show_subscreen_numbers=true;
        show_subscreen_items=true;
        show_subscreen_life=true;
        
        switch(title_version)
        {
        case 2:
            v25_titlescreen();
            break;
            
        case 1:
            DX_titlescreen();
            break;
            
        case 0:
        default:
            NES_titlescreen();
            break;
        }
    }
    
    if(!Quit)
    {
        if(lsave<1)
        {
            if(slot_arg)
            {
                currgame = slot_arg2-1;
                savecnt=0;
                
                while(savecnt < MAXSAVES && saves[savecnt].get_quest() > 0)
                    ++savecnt;
                    
                if(currgame > savecnt-1)
                {
                    slot_arg = 0;
                    currgame = 0;
                    select_game();
                }
                
                slot_arg = 0;
                //game->get_quest(&saves[currgame]);
                //select_game();
            }
            else
            {
                select_game();
            }
        }
        else
        {
            currgame = lsave-1;
            
            if(!saves[currgame].get_quest())
            {
                select_game();
            }
        }
    }
    
    if(!Quit)
    {
        init_game();
    }
}

void game_over(int type)
{
    FFCore.kb_typing_mode = false; 
    kill_sfx();
    music_stop();
    clear_to_color(screen,BLACK);
	setMonochrome(false); //Clear monochrome before drawing the file select. 
    loadfullpal();
    
    if(Quit==qGAMEOVER)
        jukebox(ZC_MIDI_GAMEOVER);
        
    Quit=0;
    
    clear_bitmap(framebuf);
    //  text_mode(-1);
    
    //Setting the colour via the array isn't working. Perhaps misc colours need to be assigned to the array in init.
    textout_ex(framebuf,zfont,SaveScreenText[SAVESC_CONTINUE],88,72,( SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] : QMisc.colors.msgtext) ,-1);
    //WTF! Setting this in zq Init() didn't work?! -Z
    if(!type)
    {
        textout_ex(framebuf,zfont,"SAVE",88,96,QMisc.colors.msgtext,-1);
        textout_ex(framebuf,zfont,"RETRY",88,120,QMisc.colors.msgtext,-1);
    }
    else
        textout_ex(framebuf,zfont,"RETRY",88,96,QMisc.colors.msgtext,-1);
        
    int pos = 0;
    int f=-1;
    //  int htile = QHeader.old_dat_flags[ZQ_TILES] ? 2 : 0;
    int htile = SaveScreenSettings[SAVESC_USETILE];
    int curcset = SaveScreenSettings[SAVESC_CURSOR_CSET];
    bool done=false;
    
    do load_control_state();
    
    while(rSbtn());
    
    do
    {
        load_control_state();
        
        if(f==-1)
        {
            if(rUp())
            {
                sfx(WAV_CHINK);
                pos=(pos==0)?2:pos-1;
                
                if(type)
                {
                    if(pos==1) pos--;
                }
            }
            
            if(rDown())
            {
                sfx(WAV_CHINK);
                pos=(pos+1)%3;
                
                if(type)
                {
                    if(pos==1) pos++;
                }
            }
            
            if(rSbtn()) ++f;
        }
        
        if(f>=0)
        {
            if(++f == 65)
                done=true;
                
            if(!(f&3))
            {
                int c = (f&4) ? QMisc.colors.msgtext : QMisc.colors.caption;
                
                switch(pos)
                {
                case 0:
                    textout_ex(framebuf,zfont,"CONTINUE",88,72,c,-1);
                    break;
                    
                case 1:
                    textout_ex(framebuf,zfont,"SAVE",88,96,c,-1);
                    break;
                    
                case 2:
                    if(!type)
                        textout_ex(framebuf,zfont,"RETRY",88,120,c,-1);
                    else textout_ex(framebuf,zfont,"RETRY",88,96,c,-1);
                    
                    break;
                }
            }
        }
        
        rectfill(framebuf,72,72,79,127,0);
        puttile8(framebuf,htile,72,pos*(type?12:24)+72,curcset,0);
        advanceframe(true);
    }
    while(!Quit && !done);
    
    reset_combo_animations();
    reset_combo_animations2();
    clear_bitmap(framebuf);
    advanceframe(true);
    
    if(done)
    {
        if(pos)
        {
            if(standalone_mode && !skip_title)
            {
                Quit=qRESET;
            }
            else
            {
                Quit=qQUIT;
            }
        }
        else
        {
            Quit=qCONT;
        }
        
        //Quit = pos ? ((standalone_mode && skip_title) ? qRESET : qQUIT) : qCONT;
        if(pos==1&&(!type))
        {
		setMonochrome(false); //Clear monochrome before drawing the file select.
            game->set_cheat(game->get_cheat() | cheat);
            
            saves[currgame]=*game;
            
            int ring=0;
            flushItemCache();
            int maxringid = getHighestLevelOfFamily(game, itemsbuf, itype_ring);
            
            if(maxringid != -1)
            {
                ring = itemsbuf[maxringid].fam_type;
            }
            
            ring = ring ? ring-1 : 0;
            iconbuffer[currgame].ring = zc_min(ring, 3);
            
            load_game_icon(saves+currgame,false,currgame);
            show_saving(screen);
            save_savedgames();
        }
    }
}

void save_game(bool savepoint)
{
    if(savepoint)
    {
        game->set_continue_scrn(homescr);
        game->set_continue_dmap(currdmap);
        lastentrance_dmap = currdmap;
        lastentrance = game->get_continue_scrn();
    }
    
    game->set_cheat(game->get_cheat() | cheat);
    
    saves[currgame]=*game;
    
    int ring=0;
    flushItemCache();
    int maxringid = getHighestLevelOfFamily(game, itemsbuf, itype_ring);
    
    if(maxringid != -1)
    {
        ring = itemsbuf[maxringid].fam_type;
    }
    
    ring = ring ? ring-1 : 0;
    iconbuffer[currgame].ring = zc_min(ring, 3);
    load_game_icon(saves+currgame,false,currgame);
    show_saving(screen);
    save_savedgames();
}

bool save_game(bool savepoint, int type)
{
    kill_sfx();
    //music_stop();
    clear_to_color(screen,SaveScreenSettings[SAVESC_BACKGROUND]);
    loadfullpal();
    
    //  int htile = QHeader.old_dat_flags[ZQ_TILES] ? 2 : 0;
    int htile = 2;
    bool done=false;
    bool saved=false;
    FFCore.kb_typing_mode = false;
    do
    {
        int pos = 0;
        int f=-1;
        bool done2=false;
        clear_bitmap(framebuf);
        
        //  text_mode(-1);
        if(type)
        {
		//Migrate this to use SaveScreenColours[SAVESC_TEXT] and set that to a default
		//of QMisc.colors.msgtext when loading the quest in the loadquest function
		//for quests with a version < 0x254! -Z
            textout_ex(framebuf,zfont,"SAVE AND QUIT",88,72,QMisc.colors.msgtext,-1);
        }
        else
        {
            textout_ex(framebuf,zfont,"SAVE",88,72,QMisc.colors.msgtext,-1);
        }
        
        textout_ex(framebuf,zfont,"DON'T SAVE",88,96,QMisc.colors.msgtext,-1);
        textout_ex(framebuf,zfont,"QUIT",88,120,QMisc.colors.msgtext,-1);
        
        do
        {
            load_control_state();
            
            if(f==-1)
            {
                if(rUp())
                {
                    sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
                    pos=(pos==0)?2:pos-1;
                }
                
                if(rDown())
                {
                    sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
                    pos=(pos+1)%3;
                }
                
                if(rSbtn()) ++f;
            }
            
            if(f>=0)
            {
                if(++f == 65)
                    done2=true;
                    
                if(!(f&3))
                {
                    int c = (f&4) ? QMisc.colors.msgtext : QMisc.colors.caption;
                    
                    switch(pos)
                    {
                    case 0:
                        if(type)
                            textout_ex(framebuf,zfont,"SAVE AND QUIT",88,72,c,-1);
                        else textout_ex(framebuf,zfont,"SAVE",88,72,c,-1);
                        
                        break;
                        
                    case 1:
                        textout_ex(framebuf,zfont,"DON'T SAVE",88,96,c,-1);
                        break;
                        
                    case 2:
                        textout_ex(framebuf,zfont,"QUIT",88,120,c,-1);
                        break;
                    }
                }
            }
            
            rectfill(framebuf,72,72,79,127,0);
            puttile8(framebuf,htile,72,pos*24+72,1,0);
            advanceframe(true);
        }
        while(!Quit && !done2);
        
        //reset_combo_animations();
        clear_bitmap(framebuf);
        //advanceframe();
        
        if(done2)
        {
            //Quit = (pos==2) ? qQUIT : 0;
            if(pos==1||pos==0) done=true;
            
            if(pos==0)
            {
                if(savepoint)
                {
                    game->set_continue_scrn(homescr);
                    game->set_continue_dmap(currdmap);
                    lastentrance_dmap = currdmap;
                    lastentrance = game->get_continue_scrn();
                }
                
                game->set_cheat(game->get_cheat() | cheat);
                
                saves[currgame]=*game;
                
                int ring=0;
                flushItemCache();
                int maxringid = getHighestLevelOfFamily(game, itemsbuf, itype_ring);
                
                if(maxringid != -1)
                {
                    ring = itemsbuf[maxringid].fam_type;
                }
                
                ring = ring ? ring-1 : 0;
                iconbuffer[currgame].ring = zc_min(ring, 3);
                load_game_icon(saves+currgame,false,currgame);
                show_saving(screen);
                save_savedgames();
                saved=true;
                
                if(type)
                {
                    Quit = qQUIT;
                    done=true;
                    skipcont=1;
                }
            }
            
            if(pos==2)
            {
                clear_bitmap(framebuf);
                //  text_mode(-1);
                textout_ex(framebuf,zfont,"ARE YOU SURE?",88,72,QMisc.colors.msgtext,-1);
                textout_ex(framebuf,zfont,"YES",88,96,QMisc.colors.msgtext,-1);
                textout_ex(framebuf,zfont,"NO",88,120,QMisc.colors.msgtext,-1);
                int pos2=0;
                int g=-1;
                bool done3=false;
                
                do
                {
                    load_control_state();
                    
                    if(g==-1)
                    {
                        if(rUp())
                        {
                            sfx(WAV_CHINK);
                            pos2=(pos2==0)?1:pos2-1;
                        }
                        
                        if(rDown())
                        {
                            sfx(WAV_CHINK);
                            pos2=(pos2+1)%2;
                        }
                        
                        if(rSbtn()) ++g;
                    }
                    
                    if(g>=0)
                    {
                        if(++g == 65)
                            done3=true;
                            
                        if(!(g&3))
                        {
                            int c = (g&4) ? QMisc.colors.msgtext : QMisc.colors.caption;
                            
                            switch(pos2)
                            {
                            case 0:
                                textout_ex(framebuf,zfont,"YES",88,96,c,-1);
                                break;
                                
                            case 1:
                                textout_ex(framebuf,zfont,"NO",88,120,c,-1);
                                break;
                                //case 2: textout_ex(framebuf,zfont,"QUIT",88,120,c,-1);   break;
                            }
                        }
                    }
                    
                    
                    rectfill(framebuf,72,72,79,127,0);
                    puttile8(framebuf,htile,72,pos2*24+96,1,0);
                    advanceframe(true);
                }
                while(!Quit && !done3);
                
                clear_bitmap(framebuf);
                
                if(pos2==0)
                {
                    Quit = qQUIT;
                    done=true;
                    skipcont=1;
                }
            }
        }
    }
    while(!Quit && !done);
    
    ringcolor(false);
    loadlvlpal(DMaps[currdmap].color);
    
    if(darkroom)
    {
        if(get_bit(quest_rules,qr_FADE))
        {
            interpolatedfade();
        }
        else
        {
            loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
        }
    }
    
    return saved;
}


/*
static void list_saves2()
{
  if(savecnt>3)
  {
    if(listpos>=3)
      textout_ex(framebuf,zfont,(char *)left_arrow_str,96,60,3,0);
    if(listpos+3<savecnt)
      textout_ex(framebuf,zfont,(char *)right_arrow_str,176,60,3,0);
    textprintf_ex(framebuf,zfont,112,60,3,0,"%2d - %-2d",listpos+1,listpos+3);
  }

  bool r = refreshpal;

  for(int i=0; i<3; i++)
  {
    if(listpos+i<savecnt)
    {
      game->set_maxlife( saves[listpos+i].get_maxlife());
      game->set_life( saves[listpos+i].get_maxlife());
      //boogie!
      lifemeter(framebuf,144,i*24+56+((game->get_maxlife()>16*(HP_PER_HEART))?8:0),0,0);
      textout_ex(framebuf,zfont,saves[listpos+i].get_name(),72,i*24+72,1,0);

      if(saves[listpos+i].get_quest())
        textprintf_ex(framebuf,zfont,72,i*24+80,1,0,"%3d",saves[listpos+i].get_deaths());

      if(saves[listpos+i].get_quest()==2)
        overtile16(framebuf,41,56,i*24+70,9,0);             //put sword on second quests

      if(saves[listpos+i].get_quest()==3)
      {
        overtile16(framebuf,41,56,i*24+70,9,0);             //put sword on second quests
        overtile16(framebuf,41,41,i*24+70,9,0);             //put sword on third quests
      }
      // maybe the triforce for the 4th quest?
      textprintf_ex(framebuf,zfont,72,i*24+72,1,0,"%s",saves[listpos+i].get_name());
    }

    byte *hold = newtilebuf[0].data;
    byte holdformat=newtilebuf[0].format;
    newtilebuf[0].format=tf4Bit;
    newtilebuf[0].data = saves[listpos+i].icon;
    overtile16(framebuf,0,48,i*24+73,i+10,0);               //link
    newtilebuf[0].format=holdformat;
    newtilebuf[0].data = hold;

    hold = colordata;
    colordata = saves[listpos+i].pal;
    loadpalset(i+10,0);
    colordata = hold;

    textout_ex(framebuf,zfont,"-",136,i*24+72,1,0);
  }

  refreshpal = r;
}

*/

/*** end of title.cc ***/
