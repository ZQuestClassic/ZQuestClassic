//--------------------------------------------------------
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

extern int loadlast;
extern int skipcont;
extern int skipicon;

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
static byte wave[3]={};

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
    sprintf(tbuf, " 1986 NINTENDO");
#ifdef _MSC_VER
#pragma warning(disable:4310)
#endif
    tbuf[0]=(char)0xBB;
    textout_ex(framebuf,zfont,tbuf,104,128,13,-1);
    sprintf(tbuf, " 2006 AG");
    tbuf[0]=(char)0xBC;
#ifdef _MSC_VER
#pragma warning(default:4310)
#endif
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

void putstring(int x,int y,char* str,int cset)
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
  if( (f>=1010) && (f<= 16*26 + 1010) && (f&1) )
    blit(scrollbuf,scrollbuf,0,8,0,7,256,248);

}


static int trstr;
static byte tr_items[] =
{
  2,28,34,4,0,1,29,30,12,16,5,6,7,8,23,24,3,15,13,14,10,11,
  17,18,19,31,26,27,25,32,9,33,21,22,23
};

static char* treasure_str[] =
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
  if(y<240)
    puttile16(scrollbuf,29,120,y,6,0);

  if(f < 18*8*16 + 1820+128)
  {
    if(((f-1820)&15)==0)
    {
      int ax=(f-1820)>>4;
      if((ax&0xF8) && ((ax&7)<2) )
        putstring(32,232,treasure_str[trstr++],0);
    }
  }
  blit(scrollbuf,framebuf,0,8,0,0,256,224);
}

static void NES_titlescreen()
{
  int f=0;
  bool done=false;
  wave[0]=0;
  wave[1]=16;
  wave[2]=32;
  tri=0;
  fcnt=0;
  trstr=0;
  set_palette(black_palette);
  jukebox(ZC_MIDI_TITLE);
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
    if( (f>4750&&midi_pos<0) || f>6000 )
    {
      wave[0]=0;
      wave[1]=16;
      wave[2]=32;
      tri=fcnt=trstr=f=0;
      clear_bitmap(framebuf);
      loadtitlepal(0x36,titlepal,4);
      music_stop();
      jukebox(ZC_MIDI_TITLE);
    }

    advanceframe();
    if(rSbtn())
      done=true;
  } while(!done && !Quit);



  music_stop();
  clear_to_color(screen,BLACK);
  CSET_SIZE=16;
  CSET_SHFT=4;
}

/************************************/
/********  DX title screen  *********/
/************************************/

static int pic=0;

static void DX_mainscreen(int f)
{
  set_uformat(U_ASCII);

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
    sprintf(tbuf, "%c2006 Armageddon Games",0xBC);
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
  int f=0;
  bool done=false;
  trstr=0;
  set_palette(black_palette);

  jukebox(ZC_MIDI_TITLE);
  /*
    if (install_mod(4) < 0) {
    Z_message("Error : Unable to set up voices\n");
    return;
    }

    // load the file from a datafile.
    yea = load_mod("yea.dat#yeam");
    if (yea == NULL) {
    Z_message("Error : Unable to load yea.dat#yeam\n");
    return;
    }

    play_mod(yea, TRUE);
    */
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
    if( (f>4750&&midi_pos<0) || f>6000 )
    {
      trstr=f=0;
      clear_bitmap(framebuf);
      music_stop();
      jukebox(ZC_MIDI_TITLE);
    }

    advanceframe();
    if(rSbtn())
      done=true;
  } while(!done && !Quit);

  music_stop();
  //  stop_mod();

  clear_to_color(screen,BLACK);
  CSET_SIZE=16;
  CSET_SHFT=4;
}

/***********************************/
/****  Game Selection Screens  *****/
/***********************************/

/*void computeMaxArrowsSave(gamedata *g)
{
	int temp=0;
	if(g->items[itype_quiver]&1) temp=1;
	if(g->items[itype_quiver]&2) temp=2;
	if(g->items[itype_quiver]&4) temp=3;
	switch(temp)
	{
	case 3:
		set_gamedata_maxcounter(game,max(get_gamedata_maxcounter(game,3),99),3);
	case 2:
		set_gamedata_maxcounter(game,max(get_gamedata_maxcounter(game,3),60),3);
	case 1:
		set_gamedata_maxcounter(game,max(get_gamedata_maxcounter(game,3),30),3);
	}
}*/

// first the game saving & loading system

static char *SAVE_HEADER = "Zelda Classic Save File";
extern char *SAVE_FILE;

int readsaves(gamedata *savedata, PACKFILE *f)
{
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
  if(section_version < 5)
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
  for (int i=0; i<save_count; i++)
  {
    if(!pfread(name,9,f,true))
    {
      return 6;
    }
    set_gamedata_name(&savedata[i], name);
    if(!p_getc(&tempbyte,f,true))
    {
      return 7;
    }
    set_gamedata_quest(&savedata[i], tempbyte);
	if(section_version<3)
	{
      if(!p_igetw(&tempword,f,true))
      {
        return 8;
      }
      set_gamedata_counter(&savedata[i], tempword, 0);
	  set_gamedata_dcounter(&savedata[i], tempword, 0);
      if(!p_igetw(&tempword,f,true))
      {
        return 9;
      }
      set_gamedata_maxcounter(&savedata[i], tempword, 0);
      if(!p_igetw(&tempshort,f,true))
      {
        return 10;
      }
      set_gamedata_dcounter(&savedata[i], tempshort, 1);
      if(!p_igetw(&tempword,f,true))
      {
        return 11;
      }
      set_gamedata_counter(&savedata[i], tempword, 1);
      if(!p_igetw(&tempword,f,true))
      {
        return 12;
      }
      set_gamedata_counter(&savedata[i], tempword, 3);
	  set_gamedata_dcounter(&savedata[i], tempword, 3);
	}
    if(!p_igetw(&tempword,f,true))
    {
      return 13;
    }
    set_gamedata_deaths(&savedata[i], tempword);
	if(section_version<3)
	{
      if(!p_getc(&tempbyte,f,true))
      {
       return 14;
      }
      set_gamedata_counter(&savedata[i], tempbyte, 5);
	  set_gamedata_dcounter(&savedata[i], tempbyte, 5);
      if(!p_getc(&tempbyte,f,true))
      {
        return 15;
      }
      set_gamedata_maxcounter(&savedata[i], tempbyte, 2);
	}
	if(section_version<4)
	{
      if(!p_getc(&tempbyte,f,true))
      {
        return 16;
      }
      set_gamedata_wlevel(&savedata[i], tempbyte);
	}
    if(!p_getc(&tempbyte,f,true))
    {
      return 17;
    }
    set_gamedata_cheat(&savedata[i], tempbyte);
    /*if(!p_igetw(&item_count,f,true))
    {
      return 18;
    }
    for (int j=0; j<item_count; ++j)
    {
      if(!p_getc(&(savedata[i].items[j]),f,true))
      {
        return 19;
      }
    }*/
	char temp;
	for(int j=0; j<256;j++)
	{
		if(!p_getc(&temp, f, true))
			return 18;
		savedata[i].item[j] = (temp != 0);
	}
	/*if(section_version<3)
	{
	  set_gamedata_counter(&savedata[i], savedata[i].items[itype_bomb], 2);
	  set_gamedata_counter(&savedata[i], savedata[i].items[itype_sbomb], 2);
	  set_gamedata_dcounter(&savedata[i], savedata[i].items[itype_bomb], 6);
	  set_gamedata_dcounter(&savedata[i], savedata[i].items[itype_sbomb], 6);
	  set_gamedata_maxcounter(&savedata[i], get_gamedata_maxcounter(&savedata[i], 2)/4, 6);
	  set_gamedata_maxcounter(&savedata[i], 255, 5);
	  int temp;
	  if(savedata[i].items[itype_wallet]&2) temp=999;
	  else if(savedata[i].items[itype_wallet]&1) temp=500;
	  else temp=255;
	  set_gamedata_maxcounter(&savedata[i], temp, 1);
	  set_gamedata_maxcounter(&savedata[i], 0, 3);
	  computeMaxArrowsSave(&savedata[i]);
	}*/
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
    set_gamedata_hasplayed(&savedata[i], tempbyte);
    if(!p_igetl(&tempdword,f,true))
    {
      return 23;
    }
    set_gamedata_time(&savedata[i], tempdword);
    if(!p_getc(&tempbyte,f,true))
    {
      return 24;
    }
    set_gamedata_timevalid(&savedata[i], tempbyte);
    for (int j=0; j<MAXLEVELS; ++j)
    {
      if(!p_getc(&(savedata[i].lvlitems[j]),f,true))
      {
        return 25;
      }
    }
	if(section_version<4)
	{
      if(!p_getc(&tempbyte,f,true))
      {
        return 26;
      }
      set_gamedata_HCpieces(&savedata[i], tempbyte);
	}
    if(!p_getc(&tempbyte,f,true))
    {
      return 27;
    }
    set_gamedata_continue_scrn(&savedata[i], tempbyte);
    if(!p_getc(&tempbyte,f,true))
    {
      return 28;
    }
    set_gamedata_continue_dmap(&savedata[i], tempbyte);
	if(section_version<3)
	{
      if(!p_igetw(&tempword,f,true))
      {
         return 29;
      }
      set_gamedata_counter(&savedata[i], tempword, 4);
      if(!p_igetw(&tempword,f,true))
      {
        return 30;
      }
      set_gamedata_maxcounter(&savedata[i], tempword, 4);
      if(!p_igetw(&tempshort,f,true))
      {
        return 31;
      }
      set_gamedata_dcounter(&savedata[i], tempshort, 4);
	}
	if(section_version<4)
	{
	  if(!p_getc(&tempbyte,f,true))
      {
        return 32;
      }
      set_gamedata_magicdrainrate(&savedata[i], tempbyte);
      if(!p_getc(&tempbyte,f,true))
      {
        return 33;
      }
      set_gamedata_canslash(&savedata[i], tempbyte);
	}
    for (int j=0; j<MAXDMAPS; ++j)
    {
      if(!p_getc(&(savedata[i].visited[j]),f,true))
      {
        return 34;
      }
    }
    for (int j=0; j<MAXDMAPS*64; ++j)
    {
      if(!p_getc(&(savedata[i].bmaps[j]),f,true))
      {
        return 35;
      }
    }
    for (int j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
    {
      if(!p_igetw(&savedata[i].maps[j],f,true))
      {
        return 36;
      }
    }
    for (int j=0; j<MAXMAPS2*MAPSCRSNORMAL; ++j)
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
    savedata[i].qstpath[qstpath_len]=0;
    if(!pfread(savedata[i].icon,sizeof(savedata[i].icon),f,true))
    {
      return 40;
    }
    if(!pfread(savedata[i].pal,sizeof(savedata[i].pal),f,true))
    {
      return 41;
    }
    for (int j=0; j<MAXLEVELS; ++j)
    {
      if(!p_getc(&(savedata[i].lvlkeys[j]),f,true))
      {
        return 42;
      }
    }
    if (section_version>1)
    {
      for (int j=0; j<MAXDMAPS*64; j++)
      {
        for (int k=0; k<8; k++)
        {
          if(!p_igetl(&savedata[i].screen_d[j][k],f,true))
          {
            return 43;
          }
        }
      }
      for (int j=0; j<256; j++)
      {
        if(!p_igetl(&savedata[i].global_d[j],f,true))
        {
          return 45;
        }
	  }
    }
	if(section_version>2)
	{
	  for(int j=0;j<32;j++)
	  {
	    if(!p_igetw(&tempword,f,true))
		{
		  return 46;
		}
		set_gamedata_counter(&savedata[i], tempword, j);
		if(!p_igetw(&tempword,f,true))
		{
		  return 47;
		}
		set_gamedata_maxcounter(&savedata[i], tempword, j);
		if(!p_igetw(&tempshort,f,true))
		{
		  return 48;
		}
		set_gamedata_dcounter(&savedata[i], tempshort, j);
	  }
	}
	if(section_version>3)
	{
	  for(int j=0;j<256;j++)
	  {
	    if(!p_getc(&tempbyte,f,true))
		{
		  return 49;
		}
		set_gamedata_generic(&savedata[i], tempbyte, j);
	  }
	}
  }

  return 0;
}

// call once at startup
int load_savedgames()
{
  //  static char *too_old  = "Obsolete format.  Resetting game data...";
  char *fname = SAVE_FILE;
  int ret;
  PACKFILE *f=NULL;
  char tmpbuf[L_tmpnam];
  temp_name(tmpbuf);
  char *tmpfilename = (char*)malloc(L_tmpnam);
  sprintf(tmpfilename, "%s", tmpbuf);

  if(saves==NULL)
  {
    saves = (gamedata*)malloc(sizeof(gamedata)*MAXSAVES);
    if(saves==NULL)
      return 1;
    memset(saves, 0, sizeof(gamedata)*MAXSAVES);
  }

  // see if it's there
  if (file_size(fname)==0)
  {
    goto newdata;
  }

  // decode to temp file
  ret = decode_file_007(fname, tmpfilename, SAVE_HEADER, ENC_METHOD_MAX-1, strstr(fname, ".dat#")!=NULL);
  if(ret)
  {
    if (f)
    {
      pack_fclose(f);
    }
    delete_file(tmpfilename);
    Z_message("Format error.  Resetting game data... ");
    goto init;
  }
  fname = tmpfilename;

  // load the games
  f = pack_fopen(fname, F_READ_PACKED);
  if(!f)
    goto newdata;

  if(readsaves(saves,f)!=0)
    goto reset;

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
        if(load_quest(saves+i, false) == 0)
        {
          load_game_icon(saves+i);
        }
      }
    }
  }

  pack_fclose(f);
  delete_file(tmpfilename);
  return 0;

newdata:
  Z_message("Save file not found.  Creating new save file.");
  goto init;

reset:
  pack_fclose(f);
  delete_file(tmpfilename);
  Z_message("Format error.  Resetting game data... ");

init:
  int *di = (int*)saves;
  for(unsigned i=0; i<sizeof(gamedata)*MAXSAVES/sizeof(int); i++)
    *(di++) = 0;

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
  for (int i=0; i<MAXSAVES; i++)
  {
    qstpath_len=strlen(savedata[i].qstpath);
    if(!pfwrite(get_gamedata_name(&savedata[i]),9,f))
    {
      return 6;
    }
    if(!p_putc(get_gamedata_quest(&savedata[i]),f))
    {
      return 7;
    }
    /*if(!p_iputw(get_gamedata_life(&savedata[i]),f))
    {
      return 8;
    }
    if(!p_iputw(get_gamedata_maxlife(&savedata[i]),f))
    {
      return 9;
    }
    if(!p_iputw(get_gamedata_drupy(&savedata[i]),f))
    {
      return 10;
    }
    if(!p_iputw(get_gamedata_rupies(&savedata[i]),f))
    {
      return 11;
    }
    if(!p_iputw(get_gamedata_arrows(&savedata[i]),f))
    {
      return 12;
    }*/
    if(!p_iputw(get_gamedata_deaths(&savedata[i]),f))
    {
      return 13;
    }
    /*if(!p_putc(get_gamedata_keys(&savedata[i]),f))
    {
      return 14;
    }
    if(!p_putc(get_gamedata_maxbombs(&savedata[i]),f))
    {
      return 15;
    }
    if(!p_putc(get_gamedata_wlevel(&savedata[i]),f))
    {
      return 16;
    }*/
    if(!p_putc(get_gamedata_cheat(&savedata[i]),f))
    {
      return 17;
    }
    /*if(!p_iputw(item_count,f))
    {
      return 18;
    }*/
	for(int j=0; j<MAXITEMS; j++)
	{
		if(!p_putc(savedata[i].item[j] ? 1 : 0,f))
			return 18;
	}
    /*if(!pfwrite(savedata[i].items,item_count,f))
    {
      return 19;
    }*/
    if(!pfwrite(savedata[i].version,sizeof(savedata[i].version),f))
    {
      return 20;
    }
    if(!pfwrite(savedata[i].title,sizeof(savedata[i].title),f))
    {
      return 21;
    }
    if(!p_putc(get_gamedata_hasplayed(&savedata[i]),f))
    {
      return 22;
    }
    if(!p_iputl(get_gamedata_time(&savedata[i]),f))
    {
      return 23;
    }
    if(!p_putc(get_gamedata_timevalid(&savedata[i]),f))
    {
      return 24;
    }
    if(!pfwrite(savedata[i].lvlitems,MAXLEVELS,f))
    {
      return 25;
    }
    /*if(!p_putc(get_gamedata_HCpieces(&savedata[i]),f))
    {
      return 26;
    }*/
    if(!p_putc(get_gamedata_continue_scrn(&savedata[i]),f))
    {
      return 27;
    }
    if(!p_putc(get_gamedata_continue_dmap(&savedata[i]),f))
    {
      return 28;
    }
    /*if(!p_iputw(get_gamedata_magic(&savedata[i]),f))
    {
      return 29;
    }
    if(!p_iputw(get_gamedata_maxmagic(&savedata[i]),f))
    {
      return 30;
    }
    if(!p_iputw(get_gamedata_dmagic(&savedata[i]),f))
    {
      return 31;
    }
    if(!p_putc(get_gamedata_magicdrainrate(&savedata[i]),f))
    {
      return 32;
    }
    if(!p_putc(get_gamedata_canslash(&savedata[i]),f))
    {
      return 33;
    }*/
    if(!pfwrite(savedata[i].visited,MAXDMAPS,f))
    {
      return 34;
    }
    if(!pfwrite(savedata[i].bmaps,MAXDMAPS*64,f))
    {
      return 35;
    }
    for (int j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
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
    for (int j=0; j<MAXDMAPS*64; j++)
    {
      for (int k=0; k<8; k++)
      {
        if(!p_iputl(savedata[i].screen_d[j][k],f))
        {
          return 43;
        }
      }
    }
    for (int j=0; j<256; j++)
    {
      if(!p_iputl(savedata[i].global_d[j],f))
      {
        return 44;
      }
    }
	for (int j=0; j<32; j++)
	{
	  if(!p_iputw(get_gamedata_counter(&savedata[i], j), f))
	  {
	    return 45;
	  }
	  if(!p_iputw(get_gamedata_maxcounter(&savedata[i], j), f))
	  {
	    return 46;
	  }
	  if(!p_iputw(get_gamedata_dcounter(&savedata[i], j), f))
	  {
	    return 47;
	  }
	}
	for(int j=0;j<256;j++)
	{
	  if(!p_putc(get_gamedata_generic(&savedata[i], j), f))
	  {
	    return 48;
	  }
	}
  }
  return 0;
}

int save_savedgames()
{
  if(saves==NULL)
    return 1;

  char tmpbuf[L_tmpnam];
  char *tmpfilename = temp_name(tmpbuf);

  PACKFILE *f = pack_fopen(tmpfilename, F_WRITE_PACKED);
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
  return ret;
}

void load_game_icon(gamedata *g)
{
  //  int i = vbound(g->items[itype_ring],0,3);
  //  int i = high_flag(g->items[itype_ring],itype_ring,true);
  //int i = high_item(imax_ring, itype_ring, true, g->items[itype_ring], true);
  int maxringid = getHighestLevelOfFamily(g, itemsbuf, itype_ring);
  int i=0;
  if(maxringid != -1)
  {
	  i = itemsbuf[maxringid].fam_type;
  }
  int t=0;
  if (get_gamedata_quest(g) >= 1 && get_gamedata_quest(g) <=3)
  {
    t = 28;
  }
  else
  {
    t = QMisc.icons[i];
  }

  if(t<0 || t>=NEWMAXTILES)
  {
    t=0;
  }

  byte *si = newtilebuf[t].data;
  if(newtilebuf[t].format==tf8Bit)
  {
    for(int j=0; j<128; j++)
    {
      g->icon[j] =0;
    }
  }
  else
  {
    for(int j=0; j<128; j++)
    {
      g->icon[j] = *(si++);
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

  if(newtilebuf[t].format==tf8Bit)
  {
    for(int j=0; j<48; j++)
    {
      g->pal[j] = 0;
    }
  }
  else
  {
    for(int j=0; j<48; j++)
    {
      g->pal[j] = *(si++);
    }
  }
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
  //  text_mode(0);
  init_NES_mode();
  //  loadfullpal();
  loadlvlpal(1);
  clear_bitmap(scrollbuf);
  QMisc.colors.blueframe_tile = 238;
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

static void list_saves()
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
      set_gamedata_maxlife(game, get_gamedata_maxlife(&saves[listpos+i]));
      set_gamedata_life(game, get_gamedata_maxlife(&saves[listpos+i]));
      //boogie!
      lifemeter(framebuf,144,i*24+56+((get_gamedata_maxlife(game)>16*(HP_PER_HEART))?8:0),0,0);
      textout_ex(framebuf,zfont,get_gamedata_name(&saves[listpos+i]),72,i*24+72,1,0);

      if(get_gamedata_quest(&saves[listpos+i]))
        textprintf_ex(framebuf,zfont,72,i*24+80,1,0,"%3d",get_gamedata_deaths(&saves[listpos+i]));

      if(get_gamedata_quest(&saves[listpos+i])==2)
        overtile16(framebuf,41,56,i*24+70,9,0);             //put sword on second quests

      if(get_gamedata_quest(&saves[listpos+i])==3)
      {
        overtile16(framebuf,41,56,i*24+70,9,0);             //put sword on second quests
        overtile16(framebuf,41,41,i*24+70,9,0);             //put sword on third quests
      }
      // maybe the triforce for the 4th quest?
      textprintf_ex(framebuf,zfont,72,i*24+72,1,0,"%s",get_gamedata_name(&saves[listpos+i]));
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

  set_gamedata_maxlife(&saves[savecnt], 3*HP_PER_HEART);
  set_gamedata_maxbombs(&saves[savecnt], 8);
  set_gamedata_continue_dmap(&saves[savecnt], 0);
  set_gamedata_continue_scrn(&saves[savecnt], 0xFF);

  int s=savecnt;
  ++savecnt;
  listpos=(s/3)*3;

  int pos=s%3;
  int y=pos*24+72;
  int x=0;
  char name[9];

  memset(name,0,9);
  register_mode();
  clear_keybuf();
  SystemKeys=false;
  refreshpal=true;
  bool done=false;

  do
  {
    if(keypressed())
    {
      int k=readkey();
      if(isprint(k&255))
      {
        name[min(x,7)]=k&0xFF;
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
          if(x<8 && name[min(x,7)])
          {
            ++x;
            sfx(WAV_CHIME);
          }
          break;
          case KEY_ENTER:
          {
            int ltrs=0;
            for(int i=0; i<8; i++)
            {
              if(name[i]!=' ' && name[i]!=0)
              {
                ++ltrs;
              }
            }
            if(ltrs)
            {
              done=true;
            }
          }
          break;
          case KEY_BACKSPACE:
          if(x>0)
          {
            --x;
            for(int i=min(x,7); i<8; i++)
            {
              name[i]=name[i+1];
            }
            sfx(WAV_OUCH);
          }
          break;
          case KEY_DEL:
          for(int i=min(x,7); i<8; i++)
          {
            name[i]=name[i+1];
          }
          sfx(WAV_OUCH);
          break;
          case KEY_ESC:
          x=-1;
          done=true;
          while(key[KEY_ESC]);
          break;
        }
      }
    }

    set_gamedata_name(&saves[s],name);
    blit(scrollbuf,framebuf,0,0,0,0,256,224);
    list_saves();

    if(frame&8)
    {
      int tx=(min(x,7)<<3)+72;
      for(int dy=0; dy<8; dy++)
        for(int dx=0; dx<8; dx++)
          if(framebuf->line[y+dy][tx+dx]==0)
            framebuf->line[y+dy][tx+dx]=CSET(9)+1;
    }

    draw_cursor(pos,0);
    advanceframe();
    /*
      if(rBbtn())
      {
      x=-1;
      done=true;
      }
      */
  } while(!done && !Quit);

  if(x<0)
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
    set_gamedata_quest(&saves[s], quest);

    packfile_password(datapwd);
    if(load_quest(saves+s))
    {
      set_gamedata_maxlife(&saves[s], zinit.hc*HP_PER_HEART);
      //saves[s].items[itype_ring]=0;
	  removeItemsOfFamily(&saves[s], itemsbuf, itype_ring);
      /*if (zinit.ring)
      {
        saves[s].items[itype_ring]=(1<<(min(zinit.ring,3)-1));
      }*/
	  int maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
	  if(maxringid != -1)
		  getitem(maxringid, true);
      ringcolor();
      //      set_gamedata_maxbombs(&saves[s], zinit.max_bombs);
      selectscreen();                                       // refresh palette
    }
    packfile_password(NULL);
    load_game_icon(saves+s);
    set_gamedata_timevalid(&saves[s], 1);
  }

  if(x<0)
  {
    for(int i=s; i<MAXSAVES-1; i++)
      saves[i]=saves[i+1];
    int *di = (int*)(saves+MAXSAVES-1);
    for(unsigned i=0; i<sizeof(gamedata)/sizeof(int); i++)
      *(di++) = 0;
    --savecnt;
    if(listpos>savecnt-1)
      listpos=max(listpos-3,0);
  }

  SystemKeys=true;
  select_mode();
  return done;
}

static bool copy_file(int file)
{
  if(savecnt<MAXSAVES && file<savecnt)
  {
    saves[savecnt]=saves[file];
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
      saves[i]=saves[i+1];
    int *di = (int*)(saves+MAXSAVES-1);
    for(unsigned i=0; i<sizeof(gamedata)/sizeof(int); i++)
      *(di++) = 0;
    --savecnt;
    if(listpos>savecnt-1)
      listpos=max(listpos-3,0);
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
  { jwin_win_proc,       40,   44,  240,  160,   0,       0,       0,       D_EXIT,     0,             0,       (void *) "Select Custom Quest", NULL,  NULL },
  // 1
  { jwin_button_proc,   205,   76,   61,   21,   0,       0,       'b',     D_EXIT,     0,             0,       (void *) "&Browse",             NULL,  NULL },
  { jwin_textbox_proc,   55,   78,  140,   16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
  { jwin_text_proc,      76,  106,   64,    8,   vc(0),   vc(11),  0,       0,          0,             0,       (void *) "Info:",               NULL,  NULL },
  { jwin_textbox_proc,   76,  118,  168,   40,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
  // 5
  { jwin_button_proc,    90,  171,   61,   21,   0,       0,       'k',     D_EXIT,     0,             0,       (void *) "O&K",                 NULL,  NULL },
  { jwin_button_proc,   170,  171,   61,   21,   0,       0,       27,      D_EXIT,     0,             0,       (void *) "Cancel",              NULL,  NULL },
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

  char tmpbuf[L_tmpnam];
  char *tmpfilename = temp_name(tmpbuf);
  int ret;
  PACKFILE *f;

  packfile_password(datapwd);
  ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_MAX-1, strstr(qstpath, ".dat#")!=NULL);
  if(ret)
  {
    switch(ret)
    {
      case 1: strcpy(str,"Error: Unable to open file"); break;
      case 2: strcpy(str,"Internal error occurred"); break;
      // be sure not to delete tmpfilename now...
    }
    if (ret==5)                                             //old encryption?
    {
      ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_192B185, strstr(qstpath, ".dat#")!=NULL);
    }
    if (ret==5)                                             //old encryption?
    {
      ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_192B105, strstr(qstpath, ".dat#")!=NULL);
    }
    if (ret==5)                                             //old encryption?
    {
      ret = decode_file_007(qstpath, tmpfilename, ENC_STR, ENC_METHOD_192B104, strstr(qstpath, ".dat#")!=NULL);
    }
    if (ret)
    {
      oldquest = true;
    }
  }

  f = pack_fopen(oldquest ? qstpath : tmpfilename, F_READ_PACKED);
  if(!f)
  {
    if(!oldquest&&(errno==EDOM))
    {
      f = pack_fopen(oldquest ? qstpath : tmpfilename, F_READ);
    }
    if (!f)
    {
      delete_file(tmpfilename);
    }
    strcpy(str,"Error: Unable to open file");
    packfile_password(NULL);
    return 0;
  }
  ret=readheader(f, header, true);

  if (f)
  {
    pack_fclose(f);
  }
  if(!oldquest)
  {
    delete_file(tmpfilename);
  }
  packfile_password(NULL);

  switch (ret)
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
    strcpy(str,"Error: Not a custom quest");
    return 0;
  }

  strcpy(str,"Title:\n");
  strcat(str,header->title);
  strcat(str,"\n\nAuthor:\n");
  strcat(str,header->author);
  packfile_password(NULL);

  return 1;
}

static int custom_game()
{
  zquestheader h;
  char infostr[200];
  char path[2048];
  int ret=0;

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

  while((ret=PopUp_dialog(gamemode_dlg,1))==1)
  {
    scare_mouse();
    blit(screen,tmp_scr,scrx,scry,0,0,320,240);
    unscare_mouse();

    int  sel=0;
    static EXT_LIST list[] =
    {
      { "ZC Quests (*.qst)", "qst" },
      { NULL,                NULL }
    };

    strcpy(path, qstpath);
    //    if(jwin_file_browse("Load Quest", path, list, &sel))
    if(jwin_file_browse_ex("Load Quest", path, list, &sel, 2048, -1, -1, lfont))
    {
      //      strcpy(qstpath, path);
      replace_extension(qstpath,path,"qst",2047);
      gamemode_dlg[2].dp = get_filename(qstpath);
      //*
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
      //*/
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
  return (ret==5) ? 1 : 0;
}

static int game_details(int file)
{
  int pos=file%3;
  if(get_gamedata_quest(&saves[file])==0)
    return 0;
  BITMAP *info = create_bitmap_ex(8,168,32);
  clear_bitmap(info);
  blit(framebuf,info,40,pos*24+70,0,0,168,26);
  rectfill(info,40,0,168,1,0);
  rectfill(info,0,24,39,25,0);
  rectfill(info,0,0,7,15,0);
  rectfill(framebuf,40,70,216,192,0);
  rectfill(framebuf,96,60,183,67,0);

  int i=pos*24+70;
  do
  {
    blit(info,framebuf,0,0,40,i,168,32);
    advanceframe();
    i-=pos+pos;
  } while(pos && i>=70);
  destroy_bitmap(info);

  textout_ex(framebuf,zfont,"GAME TYPE",40,104,3,0);
  textout_ex(framebuf,zfont,"QUEST",40,112,3,0);
  textout_ex(framebuf,zfont,"STATUS",40,120,3,0);

  if(get_gamedata_quest(&saves[file])<0xFF)
  {
    textout_ex(framebuf,zfont,"Normal Game",120,104,1,0);
    textprintf_ex(framebuf,zfont,120,112,1,0,"%s Quest",
                  ordinal(get_gamedata_quest(&saves[file])));
  }
  else
  {
    textout_ex(framebuf,zfont,"Custom Quest",120,104,1,0);
    textprintf_ex(framebuf,zfont,120,112,1,0,"%s",
                  get_filename(saves[file].qstpath));
  }

  if(!get_gamedata_hasplayed(&saves[file]))
    textout_ex(framebuf,zfont,"Empty Game",120,120,1,0);
  else if(!get_gamedata_timevalid(&saves[file]))
      textout_ex(framebuf,zfont,"Time Unknown",120,120,1,0);
    else
      textout_ex(framebuf,zfont,time_str_med(get_gamedata_time(&saves[file])),120,120,1,0);

  if(get_gamedata_cheat(&saves[file]))
    textout_ex(framebuf,zfont,"Used Cheats",120,128,1,0);

  textout_ex(framebuf,zfont,"START: PLAY GAME",56,152,1,0);
  textout_ex(framebuf,zfont,"    B: CANCEL",56,168,1,0);
  if(!get_gamedata_hasplayed(&saves[file]))
    textout_ex(framebuf,zfont,"    A: CUSTOM QUEST",56,184,1,0);

  while(!Quit)
  {
    advanceframe();
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
    if(!get_gamedata_hasplayed(&saves[file]) && rAbtn())
    {
      //chop_path(qstpath);
      if (is_relative_filename(saves[file].qstpath))
      {
        sprintf(qstpath,"%s%s",qstdir,saves[file].qstpath);
      }
      else
      {
        sprintf(qstpath,"%s", saves[file].qstpath);
      }
      //sprintf(qstpath,"%s%s",qstdir,saves[file].qstpath);
      //      packfile_password(datapwd);
      if(custom_game())
      {
        set_gamedata_quest(&saves[file],0xFF);
        //this next line strips the path off.  not good.
        //strncpy(saves[file].qstpath,get_filename(qstpath),80);
        char temppath[2048];
        memset(temppath, 0, 2048);
        zc_make_relative_filename(temppath, qstdir, qstpath, 2047);
        if (temppath[0]==0) //can't make relative, go absolute
        {
          sprintf(saves[file].qstpath, "%s", qstpath);
        }
        else
        {
          sprintf(saves[file].qstpath, "%s", temppath);
        }
        packfile_password(datapwd);
        load_quest(saves+file);

        packfile_password(NULL);
        //        set_gamedata_maxbombs(&saves[file], zinit.max_bombs);
        set_gamedata_maxlife(&saves[file], zinit.hc*HP_PER_HEART);
        //saves[file].items[itype_ring]=(1<<(zinit.ring-1)); 
		int maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
		if(maxringid != -1)
			getitem(maxringid, true);
		ringcolor();
        load_game_icon(saves+file);
        //      packfile_password(NULL);
        selectscreen();
        return 0;
      }
    }
  }
  return 0;
}

static void select_game()
{
  int pos = max(min(currgame-listpos,3),0);
  int mode = 0;

  //kill_sfx();

  //  text_mode(0);
  selectscreen();

  savecnt=0;
  while(savecnt<MAXSAVES && get_gamedata_quest(&saves[savecnt])>0)
    ++savecnt;

  if(savecnt==0)
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
    advanceframe();

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
          currgame=listpos+pos;
          loadlast=currgame+1;
          if(get_gamedata_quest(&saves[currgame]))
            done=true;
          break;

          case 2:
          if(copy_file(pos+listpos))
          {
            mode=0;
            pos=(savecnt-1)%3;
            refreshpal=true;
          }
          break;

          case 3:
          if(delete_save(pos+listpos))
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
      if(game_details(pos+listpos))
      {
        currgame=listpos+pos;
        loadlast=currgame+1;
        if(get_gamedata_quest(&saves[currgame]))
          done=true;
      }
      //      packfile_password(NULL);
    }
  } while(!Quit && !done);
}

/**************************************/
/****  Main title screen routine  *****/
/**************************************/

void titlescreen(int lsave)
{
  int q=Quit;

  Quit=0;
  Playing=Paused=false;

  if(q==qCONT)
  {
    cont_game();
    return;
  }

  if(q==qRESET)
  {
    show_subscreen_dmap_dots=true;
    show_subscreen_numbers=true;
    show_subscreen_items=true;
    show_subscreen_life=true;

    if(DXtitle)
    {
      DX_titlescreen();
    }
    else
    {
      NES_titlescreen();
    }
  }

  if(!Quit)
  {
    if(lsave<1)
    {
      if(slot_arg)
      {
        currgame=slot_arg2-1;
        savecnt=0;
        while(savecnt<MAXSAVES && get_gamedata_quest(&saves[savecnt])>0)
          ++savecnt;
        if (currgame>savecnt-1)
        {
          slot_arg=0;
          currgame=0;
          select_game();
        }
        //get_gamedata_quest(&saves[currgame]);
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
      if(!(get_gamedata_quest(&saves[currgame])))
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
  kill_sfx();
  music_stop();
  clear_to_color(screen,BLACK);
  loadfullpal();

  if(Quit==qGAMEOVER)
    jukebox(ZC_MIDI_GAMEOVER);
  Quit=0;

  clear_bitmap(framebuf);
  //  text_mode(-1);
  textout_ex(framebuf,zfont,"CONTINUE",88,72,QMisc.colors.text,-1);
  if(!type)
  {
    textout_ex(framebuf,zfont,"SAVE",88,96,QMisc.colors.text,-1);
    textout_ex(framebuf,zfont,"RETRY",88,120,QMisc.colors.text,-1);
  }
  else
    textout_ex(framebuf,zfont,"RETRY",88,96,QMisc.colors.text,-1);

  int pos = 0;
  int f=-1;
  //  int htile = QHeader.old_dat_flags[ZQ_TILES] ? 2 : 0;
  int htile = 2;
  bool done=false;
  do load_control_state(); while(rSbtn());
  do
  {
    load_control_state();
    if(f==-1)
    {
      if(rUp())   { sfx(WAV_CHINK); pos=(pos==0)?2:pos-1; if(type) {if(pos==1) pos--;} }
      if(rDown()) { sfx(WAV_CHINK); pos=(pos+1)%3; if(type) {if(pos==1) pos++;} }
      if(rSbtn()) ++f;
    }

    if(f>=0)
    {
      if(++f == 65)
        done=true;
      if(!(f&3))
      {
        int c = (f&4) ? QMisc.colors.text : QMisc.colors.caption;
        switch(pos)
        {
          case 0: textout_ex(framebuf,zfont,"CONTINUE",88,72,c,-1); break;
          case 1: textout_ex(framebuf,zfont,"SAVE",88,96,c,-1);     break;
          case 2:
          if(!type)
            textout_ex(framebuf,zfont,"RETRY",88,120,c,-1);
          else textout_ex(framebuf,zfont,"RETRY",88,96,c,-1);
          break;
        }
      }
    }

    rectfill(framebuf,72,72,79,127,0);
    puttile8(framebuf,htile,72,pos*(type?12:24)+72,1,0);
    advanceframe();
  } while(!Quit && !done);
  reset_combo_animations();
  reset_combo_animations2();
  clear_bitmap(framebuf);
  advanceframe();

  if(done)
  {
    Quit = pos ? qQUIT : qCONT;
    if(pos==1&&(!type))
    {
      set_gamedata_cheat(game, get_gamedata_cheat(game) | cheat);
      //      saves[currgame] = realgame;
      //      memcpy(saves+currgame,&realgame,sizeof(gamedata));
      memcpy(saves+currgame,game,sizeof(gamedata));
      load_game_icon(saves+currgame);
      save_savedgames();
    }
  }
}

void save_game(bool savepoint, int type)
{
  kill_sfx();
  //music_stop();
  clear_to_color(screen,BLACK);
  loadfullpal();

  //  int htile = QHeader.old_dat_flags[ZQ_TILES] ? 2 : 0;
  int htile = 2;
  bool done=false;
  do
  {
    int pos = 0;
    int f=-1;
    bool done2=false;
    clear_bitmap(framebuf);
    //  text_mode(-1);
    if(type)
    {
      textout_ex(framebuf,zfont,"SAVE AND QUIT",88,72,QMisc.colors.text,-1);
    }
    else 
    {
      textout_ex(framebuf,zfont,"SAVE",88,72,QMisc.colors.text,-1);
    }
    textout_ex(framebuf,zfont,"DON'T SAVE",88,96,QMisc.colors.text,-1);
    textout_ex(framebuf,zfont,"QUIT",88,120,QMisc.colors.text,-1);
    do
    {
		load_control_state();
      if(f==-1)
      {
        if(rUp())   { sfx(WAV_CHINK); pos=(pos==0)?2:pos-1; }
        if(rDown()) { sfx(WAV_CHINK); pos=(pos+1)%3; }
        if(rSbtn()) ++f;
      }

      if(f>=0)
      {
        if(++f == 65)
          done2=true;
        if(!(f&3))
        {
          int c = (f&4) ? QMisc.colors.text : QMisc.colors.caption;
          switch(pos)
          {
            case 0:
            if(type)
              textout_ex(framebuf,zfont,"SAVE AND QUIT",88,72,c,-1);
            else textout_ex(framebuf,zfont,"SAVE",88,72,c,-1);
            break;
            case 1: textout_ex(framebuf,zfont,"DON'T SAVE",88,96,c,-1);     break;
            case 2: textout_ex(framebuf,zfont,"QUIT",88,120,c,-1);   break;
          }
        }
      }

      rectfill(framebuf,72,72,79,127,0);
      puttile8(framebuf,htile,72,pos*24+72,1,0);
      advanceframe();
    } while(!Quit && !done2);
    //reset_combo_animations();
    clear_bitmap(framebuf);
    //advanceframe();

    if(done2)
    {
      //Quit = (pos==2) ? qQUIT : 0;
      if(pos==1||pos==0) done=true;
      if(pos==0)
      {
        if (savepoint)
        {
          set_gamedata_continue_scrn(game, homescr);
          set_gamedata_continue_dmap(game, currdmap);
          lastentrance_dmap = currdmap;
          lastentrance = get_gamedata_continue_scrn(game);
        }
        set_gamedata_cheat(game, get_gamedata_cheat(game) | cheat);
        //        saves[currgame] = realgame;
        //      memcpy(saves+currgame,&realgame,sizeof(gamedata));
        memcpy(saves+currgame,game,sizeof(gamedata));
        load_game_icon(saves+currgame);
        save_savedgames();
        if(type) {Quit = qQUIT; done=true; skipcont=1;}
      }

      if(pos==2)
      {
        clear_bitmap(framebuf);
        //  text_mode(-1);
        textout_ex(framebuf,zfont,"ARE YOU SURE?",88,72,QMisc.colors.text,-1);
        textout_ex(framebuf,zfont,"YES",88,96,QMisc.colors.text,-1);
        textout_ex(framebuf,zfont,"NO",88,120,QMisc.colors.text,-1);
        int pos2=0;
        int g=-1;
        bool done3=false;
        do
        {
          if(g==-1)
          {
            if(rUp())   { sfx(WAV_CHINK); pos2=(pos2==0)?1:pos2-1; }
            if(rDown()) { sfx(WAV_CHINK); pos2=(pos2+1)%2; }
            if(rSbtn()) ++g;
          }

          if(g>=0)
          {
            if(++g == 65)
              done3=true;
            if(!(g&3))
            {
              int c = (g&4) ? QMisc.colors.text : QMisc.colors.caption;
              switch(pos2)
              {
                case 0: textout_ex(framebuf,zfont,"YES",88,96,c,-1); break;
                case 1: textout_ex(framebuf,zfont,"NO",88,120,c,-1);     break;
                //case 2: textout_ex(framebuf,zfont,"QUIT",88,120,c,-1);   break;
              }
            }
          }


          rectfill(framebuf,72,72,79,127,0);
          puttile8(framebuf,htile,72,pos2*24+96,1,0);
          advanceframe();
        } while(!Quit && !done3);
        clear_bitmap(framebuf);

        if(pos2==0) {Quit = qQUIT; done=true; skipcont=1;}
      }
    }
  } while(!Quit && !done);
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);
}

/*** end of title.cc ***/
