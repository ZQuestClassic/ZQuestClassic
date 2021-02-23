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
#include <allegro.h>

#include "zdefs.h"
#include "zelda.h"
#include "zsys.h"
#include "qst.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"




/********************************/
/*****   NES title screen   *****/
/********************************/


#define BKGDATA 224*256/32


static byte titlepal[24] =
 {0x0F,0x00,0x10, 0x17,0x27,0x0F, 0x08,0x1A,0x28, 0x30,0x3B,0x22,
  0x30,0x3B,0x16, 0x17,0x27,0x0F, 0x08,0x1A,0x28, 0x30,0x3B,0x22};

static byte darkpal1[24] =
 {0x0F,0x0F,0x00, 0x01,0x11,0x0F, 0x0C,0x01,0x02, 0x00,0x01,0x0C,
  0x00,0x01,0x0C, 0x01,0x11,0x0F, 0x0C,0x01,0x02, 0x00,0x01,0x0C};

static byte darkpal2[24] =
 {0x0F,0x0F,0x00, 0x01,0x11,0x0F, 0x0F,0x0C,0x01, 0x01,0x0C,0x0F,
  0x01,0x0C,0x0F, 0x01,0x11,0x0F, 0x0F,0x0C,0x01, 0x01,0x0C,0x0F};

static byte darkpal3[24] =
 {0x0F,0x0F,0x0F, 0x01,0x0F,0x0F, 0x0F,0x0C,0x0F, 0x0F,0x0C,0x0F,
  0x0F,0x0C,0x0F, 0x01,0x0F,0x0F, 0x0F,0x0C,0x0F, 0x0F,0x0C,0x0F};

static byte itemspal[24] =
 {0x30,0x30,0x30, 0x21,0x30,0x30, 0x16,0x30,0x30, 0x29,0x1A,0x09,
  0x29,0x37,0x17, 0x02,0x22,0x30, 0x16,0x27,0x30, 0x0B,0x1B,0x2B};


static void loadtitlepal(int clear,byte *dataofs,int shift)
{
  for(int i=0; i<4; i++) {
    RAMpal[CSET(i)+shift] = NESpal(clear);
    for(int c=1; c<4; c++)
      RAMpal[CSET(i)+c+shift] = NESpal(*dataofs++);
    }
  for(int i=6; i<10; i++) {
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
for(int i=0; i<3; i++) {
  wave[i]+=2;
  if(wave[i]==50)
    wave[i]=0;
  int y=wave[i]+170;

  if(wave[i]<8) {
    overtile16(framebuf,204,80,y,4,0);
    overtile16(framebuf,205,96,y,4,0); }
  else if(wave[i]<16) {
    overtile16(framebuf,206,80,y,4,0);
    overtile16(framebuf,207,96,y,4,0); }
  else {
    overtile16(framebuf,208,80,y,4,0);
    overtile16(framebuf,209,96,y,4,0); }
  }
}


static byte tri,fcnt;

static void mainscreen(int f)
{
if(f>=1010)
  return;
if(f==0) {
  blit((BITMAP*)data[BMP_TITLE_NES].dat,scrollbuf,0,0,0,0,256,224);
  blit(scrollbuf,framebuf,0,0,0,0,256,224);
  }
if(f<554+192+10) {
  blit(scrollbuf,framebuf,80,160,80,160,32,64);
  if(f&8) {
    puttile16(framebuf,200,80,160,4,0);
    puttile16(framebuf,201,96,160,4,0); }
  else {
    puttile16(framebuf,202,80,160,4,0);
    puttile16(framebuf,203,96,160,4,0); }
  cyclewaves();
  }

if(f<58*9)
  fcnt++;
if(fcnt==tridelay[tri]) {
  fcnt=0;
  tri = tri<5 ? tri+1 : 0;
  RAMpal[CSET(2)+2]=NESpal(tricolor[tri]);
  refreshpal=true;
  }
for(int i=0; i<8; i++) {
  if(f==dusktime[i])
    loadtitlepal(duskcolor[i],titlepal,4);
  }
if(f==554)
  loadtitlepal(0x0F,darkpal1,4);
if(f==554+192)
  loadtitlepal(0x0F,darkpal2,4);
if(f==554+192+6)
  loadtitlepal(0x0F,darkpal3,4);
if(f==554+192+10) {
  clear(framebuf);
  clear(scrollbuf);
  }
if(f==554+192+10+193)
  loadtitlepal(0x0F,itemspal,0);
}

void putstring(int x,int y,char* str,int cset)
{
  textout(scrollbuf,zfont,str,x,y,(cset<<CSET_SHFT)+1);
}

#define ii    11
static byte vine[5] = { 2,3,6,7,10 };

static void storyscreen(int f)
{
if(f<1010)  return;
if(f>=1804) return;
if(f==1010) {
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
if(f==16*4 +1010) {
  puttile8(scrollbuf,ii,104,232,2,0);
  puttile8(scrollbuf,ii,168,232,2,0);
  }
if(f==16*5 +1010) {
  putstring(32,232,"DARKNESS",0);
  putstring(120,232,"GANNON",2);
  putstring(184,232,"STOLE",0);
  }
if(f==16*7 +1010) {
  putstring(32,232,"ONE OF THE",0);
  putstring(120,232,"TRIFORCE",1);
  putstring(192,232,"WITH",0);
  }
if(f==16*9 +1010) {
  putstring(32,232,"POWER.    PRINCESS",0);
  putstring(184,232,"ZELDA",2);
  }
if(f==16*11 +1010) {
  putstring(32,232,"HAD  ONE OF THE",0);
  putstring(160,232,"TRIFORCE",1);
  }
if(f==16*13 +1010)
  putstring(32,232,"WITH WISDOM. SHE DIVIDED",0);
if(f==16*14 +1010) {
  puttile8(scrollbuf,ii,88,232,1,0);
  puttile8(scrollbuf,ii,112,232,1,0);
  } 
if(f==16*15 +1010) {
  putstring(32,232,"IT INTO    UNITS TO HIDE",0);
  putstring(104,232,"8",1); 
  }
if(f==16*16 +1010) {
  puttile8(scrollbuf,ii,96,232,2,0);
  puttile8(scrollbuf,ii,160,232,2,0);
  } 
if(f==16*17 +1010) {
  putstring(32,232,"IT FROM",0);
  putstring(112,232,"GANNON",2);
  putstring(176,232,"BEFORE",0); 
  }
if(f==16*19 +1010)
  putstring(32,232,"SHE WAS CAPTURED.",0);
if(f==16*20 +1010) {
  puttile8(scrollbuf,ii,136,232,2,0);
  puttile8(scrollbuf,ii,160,232,2,0);
  } 
if(f==16*21 +1010)
  putstring(48,232,"GO FIND THE  8 UNITS",2);
if(f==16*22 +1010) {
  puttile8(scrollbuf,ii,48,232,3,0);
  puttile8(scrollbuf,ii,96,232,3,0);
  } 
if(f==16*23 +1010) {
  putstring(64,232,"LINK",3);
  putstring(112,232,"TO SAVE HER.",2);
  }

if(f==16*25 +1010) {
  puttile8(scrollbuf,vine[4],16,232,3,1);
  for(int x=24; x<232; x+=16) {
    puttile8(scrollbuf,vine[2],x,232,3,0);
    puttile8(scrollbuf,vine[3],x+8,232,3,0);
    }
  puttile8(scrollbuf,vine[4],232,232,3,1);
  }

if((f>1010) && (f <= 16*24 + 1010)) {
  if(((f-1010)&15) == 0) {
    if((f-1010)&16) {
      puttile8(scrollbuf,vine[0],16,232,3,0);
      puttile8(scrollbuf,vine[1],232,232,3,0);
      }
    else {
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
static byte tr_items[] = { 2,28,34,4,0,1,29,30,12,16,5,6,7,8,23,24,3,15,13,14,10,11,
                  17,18,19,31,26,27,25,32,9,33,21,22,23};

static char* treasure_str[] = {"ALL OF TREASURES",
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
"        TRIFORCE           "};

static void treasures(int f)
{
if(f<1804) return;
if(f>4492) return;
if(f == 1804) {
  for(int x=0; x<48; x+=16) {
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
for(int i=0; i<34; i+=2) {
  if((y>=0)&&(y<240)) {
    if(i==2) // fairy
      puttile16(scrollbuf,((f&4)>>2)+63,68,y,8,0);
    else
      putitem(scrollbuf,68,y,tr_items[i]);
    putitem(scrollbuf,172,y,tr_items[i+1]);
    }
  y+=64;
  }

y+=8;
if((y>=0)&&(y<240))
  putitem(scrollbuf,120,y,20);

y+=80;
if(y>=0)
  for(int i=177; i<=217; i+=20) {
    if(y<240) {
      puttile16(scrollbuf,i,104,y,6,0);
      puttile16(scrollbuf,i+1,120,y,6,0);
      puttile16(scrollbuf,i+2,136,y,6,0);
      }
    y+=16;
    }
if(y<240)
  puttile16(scrollbuf,29,120,y,6,0);


if(f < 18*8*16 + 1820+128) {
  if(((f-1820)&15)==0) {
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
  jukebox(MUSIC_TITLE);
  clear(screen);
  clear(framebuf);
  init_NES_mode();
//  reset_items();
  CSET_SIZE=4;
  CSET_SHFT=2;
  loadtitlepal(0x36,titlepal,4);
  ALLOFF();
  do {
    mainscreen(f);
    storyscreen(f);
    treasures(f);
    if((f>1676)&&(f<=4492)&&(f&1))
      blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
    if(f>=4492)
      putitem(framebuf,120,24,20);

    f++;
    if( (f>4750&&midi_pos<0) || f>6000 ) {
      wave[0]=0;
      wave[1]=16;
      wave[2]=32;
      tri=fcnt=trstr=f=0;
      clear(framebuf);
      loadtitlepal(0x36,titlepal,4);
      stop_midi();
      jukebox(MUSIC_TITLE);
      }

    advanceframe();
    if(rSbtn())
      done=true;
  } while(!done && !Quit);

  stop_midi();
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
  if(f>=1010)
    return;

  DATAFILE *dat = (DATAFILE*)data[TITLE_DX].dat;
  BITMAP *bmp;

  if(f==0) {
    copy_pal((RGB*)dat[TITLE_DX_PAL_1].dat,RAMpal);
    refreshpal=true;
    }

  if(f<680+256 && (f&15)==0) {
    bmp = (BITMAP*)dat[pic<4 ? pic : 6-pic].dat;
    pic = (pic+1)%6;
    blit(bmp,framebuf, 0,0, 0,0, 256,224);
    text_mode(-1);
    textout(framebuf,font,"1986 Nintendo",46,138,255);
    textout(framebuf,font,"1999 Armageddon Games",46,146,255);
    text_mode(0);
    }

  if(f>=680 && f<680+256 && (f%3)==0) {
    fade_interpolate((RGB*)dat[TITLE_DX_PAL_1].dat,black_palette,RAMpal,
      (f-680)>>2,0,255);
    refreshpal=true;
    }

  if(f==680+256) {
    clear(framebuf);
    clear(scrollbuf);
    }

  if(f==680+256+2)
    loadtitlepal(0x0F,itemspal,0);
}


static void DX_titlescreen()
{
  int f=0;
  bool done=false;
  trstr=0;
  set_palette(black_palette);
  jukebox(MUSIC_TITLE);
  clear_to_color(screen,BLACK);
  clear(framebuf);
  init_NES_mode();
//  reset_items();
  CSET_SIZE=4;
  CSET_SHFT=2;
  ALLOFF();
  clear_keybuf();

  do {
    DX_mainscreen(f);
    storyscreen(f);
    treasures(f);
    if((f>1676)&&(f<=4492)&&(f&1))
      blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
    if(f>=4492)
      putitem(framebuf,120,24,20);

    f++;
    if( (f>4750&&midi_pos<0) || f>6000 ) {
      trstr=f=0;
      clear(framebuf);
      stop_midi();
      jukebox(MUSIC_TITLE);
      }

    advanceframe();
    if(rSbtn())
      done=true;
  } while(!done && !Quit);

  stop_midi();
  clear_to_color(screen,BLACK);
  CSET_SIZE=16;
  CSET_SHFT=4;
}



/***********************************/
/****  Game Selection Screens  *****/
/***********************************/


// first the game saving & loading system

static char *SAVE_HEADER = "Zelda Classic Save File";
static char *SAVE_FILE = "zelda.sav";


// call once at startup
int load_savedgames()
{
//  int old_max_saves=24;
  static char *conv_str = "Converting from old format... ";
  static char *too_old  = "Obsolete format.  Resetting game data...";
  char *fname = SAVE_FILE;
  int ret;
  word buf[8];
  PACKFILE *f;
  FILE *fin;
  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);

  if(saves==NULL)
  {
    saves = (gamedata*)malloc(sizeof(gamedata)*MAXSAVES);
    if(saves==NULL)
      return 1;
  }

  // see if it's there
  fin = fopen(SAVE_FILE, "rb");
  if(!fin)
    goto newdata;

  // check for new encrypted format with text header
  for(int i=0; SAVE_HEADER[i]; i++)
  {
    int c = fgetc(fin);
    if(c == EOF)                   // uh oh...
    {
      fclose(fin);
      goto newdata;
    }
    if((c&255) != SAVE_HEADER[i])  // old format
    {
      fclose(fin);
      goto unpack;
    }
  }
  fclose(fin);


  // decode to temp file
  ret = decode_file_007(fname, tmpfilename, SAVE_HEADER);
  if(ret)
  {
    delete_file(tmpfilename);
    printf("Format error.\nResetting game data... ");
    goto init;
  }
  fname = tmpfilename;



unpack:

  // load the games
  f = pack_fopen(fname, F_READ_PACKED);
  if(!f)
    goto newdata;

  if(!pfread(buf,16,f)) // read the mini-header
    goto reset;

  if(buf[0]!=0xFFFF)  // below v0.97
  {
    printf(too_old);
    goto reset;
/*
    pack_fseek(f,-16);
    printf(conv_str);
//    for(int i=0; i<MAXSAVES; i++)
    for(int i=0; i<old_max_saves; i++)
    {
      if(!pfread(saves+i,sizeof(gamedata)-128,f))
        goto reset;

      byte *p = (byte*)(saves+i);
      for(int j=sizeof(gamedata)-1; j>=256; j--)
        *(p+j) = *(p+j-128);
      for(int j=128; j<256; j++)
        *(p+j) = 0;
    }
//    for(int i=0; i<MAXSAVES; i++)
    for(int i=0; i<old_max_saves; i++)
    {
      if(saves[i].quest!=1)  // delete the demo quest saves
      {
//        for(int j=i; j<MAXSAVES-1; j++)
        for(int j=i; j<old_max_saves-1; j++)
          saves[j]=saves[j+1];
//        int *di = (int*)(saves+MAXSAVES-1);
        int *di = (int*)(saves+old_max_saves-1);
        for(unsigned k=0; k<sizeof(gamedata)/sizeof(int); k++)
          *(di++) = 0;
      }
    }
*/
  }
  else
  {
    if(buf[1] < 0x0174) // below v1.74, only has 10 maps instead of 16
    {
    printf(too_old);
    goto reset;
/*
      printf(conv_str);
//      for(int i=0; i<MAXSAVES; i++)
      for(int i=0; i<old_max_saves; i++)
      {
        byte *di = (byte*)(saves+i);

        // header and bmaps[]
        if(!pfread(di, 256+32*64, f))
          goto reset;

        // maps[0..9]
        di += 256+32*64;
        if(!pfread(di, 10*128, f))
          goto reset;

        // maps[10..15]
        di += 10*128;
        for(int i=0; i<6*128; i++)
          *(di++) = 0;

        // guys[0..9]
        if(!pfread(di, 10*128, f))
          goto reset;

        // guys[10..15]
        di += 10*128;
        for(int i=0; i<6*128; i++)
          *(di++) = 0;

        // last chunk of data
        if(!pfread(di, 260, f))
          goto reset;
      }
*/
    }
    else if (buf[1] < 0x0190) // below v1.90, only has 24 save states. ->255
    {
    printf(too_old);
    goto reset;
/*
      int *di = (int*)saves;
      for(unsigned i=0; i<sizeof(gamedata)*MAXSAVES/sizeof(int); i++)
        *(di++) = 0;
      if(!pfread(saves,sizeof(gamedata)*old_max_saves,f))
        goto reset;
*/
    }
    else if ((buf[1] < 0x0192) ||  // below v1.92, build 5 only has
            ((buf[1] == 0x0192) && // 16 maps and 32 dmaps -> 255, 255
             (buf[2]<5)))
    {
      printf(conv_str);
      int *di = (int*)saves;
      for(unsigned i=0; i<sizeof(gamedata)*MAXSAVES/sizeof(int); i++)
        *(di++) = 0;
      int saves_to_load=(buf[1] < 0x0190)?24:255;
      oldgamedata *oldsaves = (oldgamedata*)malloc(sizeof(oldgamedata)*MAXSAVES);
      if(!pfread(oldsaves,sizeof(oldgamedata)*saves_to_load,f))
        goto reset;
      for (int counter=0; counter<256; counter++) {
        memcpy(saves[counter].name, oldsaves[counter].name, 256);
        memcpy(saves[counter].bmaps, oldsaves[counter].bmaps, (32*64));
        memcpy(saves[counter].maps, oldsaves[counter].maps, (16*128));
        memcpy(saves[counter].guys, oldsaves[counter].guys, (16*128));
        memcpy(saves[counter].qstpath, oldsaves[counter].qstpath, 260);
      }

    } else
    {
      if(!pfread(saves,sizeof(gamedata)*MAXSAVES,f))
        goto reset;
    }
/*  never get to these.  too old.  save game reset.

    if(buf[1] < 0x0102) // below v1.02, trim file paths
    {
      char path[260];
      for(int i=0; i<MAXSAVES; i++)
      {
        strcpy(path,saves[i].qstpath);  // trim dir off the quest file name
        strcpy(saves[i].qstpath,get_filename(path));
      }
    }

    if(buf[1] < 0x0176) // below v1.76, has lvlitems[16] instead of 32
    {
      for(int i=0; i<MAXSAVES; i++)
      {
        memcpy(&(saves[i].lvlitems), ((byte*)(saves+i))+10, 16);
        memset(((byte*)(saves+i))+10, 0, 16);
      }
    }
*/

  }

  for(int i=0; i<MAXSAVES; i++)
  {
    if(strlen(saves[i].qstpath))
    {
      if(load_quest(saves+i, false) == 0)
        load_game_icon(saves+i);
    }
  }

  pack_fclose(f);
  delete_file(tmpfilename);
  if ((buf[1] < 0x0192) ||  // below v1.92, build 12 doesn't have saved
     ((buf[1] == 0x0192) && // continue points
     (buf[2]<12)))
  {
      for (int counter=0; counter<MAXSAVES; counter++) {
        saves[counter].continue_dmap=0;
      }
  }
  return 0;


newdata:
  printf("Save file not found.");
  goto init;

reset:
  pack_fclose(f);
  delete_file(tmpfilename);
  printf("Format error.\nResetting game data... ");

init:
  int *di = (int*)saves;
  for(unsigned i=0; i<sizeof(gamedata)*MAXSAVES/sizeof(int); i++)
    *(di++) = 0;

  return 0;
}




int save_savedgames()
{
  if(saves==NULL)
    return 1;

  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);

  word buf[8];
  buf[0]=0xFFFF;
  buf[1]=ZELDA_VERSION;
  buf[2]=VERSION_BUILD;

  PACKFILE *f = pack_fopen(tmpfilename, F_WRITE_PACKED);
  if(!f)
  {
    delete_file(tmpfilename);
    return 2;
  }

  if(!pfwrite(buf,16,f))
  {
    pack_fclose(f);
    delete_file(tmpfilename);
    return 3;
  }

  if(!pfwrite(saves,sizeof(gamedata)*MAXSAVES,f))
  {
    pack_fclose(f);
    delete_file(tmpfilename);
    return 4;
  }

  pack_fclose(f);
  int ret = encode_file_007(tmpfilename, SAVE_FILE, 0x413F0000 + (frame&0xffff), SAVE_HEADER);
  if(ret)
    ret += 100;
  delete_file(tmpfilename);
  return ret;
}



void load_game_icon(gamedata *g)
{
  int i = vbound(g->ring,0,3);
  int t = QMisc.icons[i];

  if(t<0 || t>=NEWMAXTILES)
    t=0;

  byte *si = tilebuf + ( (t ? t : 28) << 7 );

  for(int j=0; j<128; j++)
    g->icon[j] = *(si++);

  if(t)
    si = colordata + CSET(pSprite(i+spICON1))*3;
  else
  {
    if(i)
      si = colordata + CSET(pSprite(i-1+spBLUE))*3;
    else
      si = colordata + CSET(6)*3;
  }

  for(int j=0; j<48; j++)
    g->pal[j] = *(si++);
}



static void select_mode()
{
  textout(scrollbuf,zfont,"REGISTER YOUR NAME",48,152,1);
  textout(scrollbuf,zfont,"COPY FILE",48,168,1);
  textout(scrollbuf,zfont,"DELETE FILE",48,184,1);
}

static void register_mode()
{
  textout(scrollbuf,zfont,"REGISTER YOUR NAME",48,152,CSET(2)+3);
}

static void copy_mode()
{
  textout(scrollbuf,zfont,"COPY FILE",48,168,CSET(2)+3);
}

static void delete_mode()
{
  textout(scrollbuf,zfont,"DELETE FILE",48,184,CSET(2)+3);
}


static void selectscreen()
{
  init_NES_mode();
  loadfullpal();
  loadlvlpal(1);
  clear(scrollbuf);
  QMisc.colors.blueframe_tile = 238;
  QMisc.colors.blueframe_cset = 0;
  blueframe(scrollbuf,24,48,26,20);
  text_mode(0);
  textout(scrollbuf,zfont,"- S E L E C T -",64,24,1);
  textout(scrollbuf,zfont," NAME ",80,48,1);
  textout(scrollbuf,zfont," LIFE ",152,48,1);
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
      textout(framebuf,zfont,(char *)left_arrow_str,96,60,3);
    if(listpos+3<savecnt)
      textout(framebuf,zfont,(char *)right_arrow_str,176,60,3);
    textprintf(framebuf,zfont,112,60,3,"%2d - %-2d",listpos+1,listpos+3);
  }

  bool r = refreshpal;

  for(int i=0; i<3; i++)
  {
    if(listpos+i<savecnt)
    {
      game.maxlife = game.life = saves[listpos+i].maxlife;
      //boogie!
      lifemeter(framebuf,144,i*24+80+((game.maxlife>16*(HP_PER_HEART))?8:0));
      textout(framebuf,zfont,saves[listpos+i].name,72,i*24+72,1);

      if(saves[listpos+i].quest)
        textprintf(framebuf,zfont,72,i*24+80,1,"%3d",saves[listpos+i].deaths);

      if(saves[listpos+i].quest==2)
        overtile16(framebuf,41,56,i*24+70,9,0);  //put sword on second quests

      if(saves[listpos+i].quest==3) {
        overtile16(framebuf,41,56,i*24+70,9,0);  //put sword on second quests
        overtile16(framebuf,41,41,i*24+70,9,0);  //put sword on third quests
      }
      // maybe the triforce for the 4th quest?
      textprintf(framebuf,zfont,72,i*24+72,1,"%s",saves[listpos+i].name);
    }

    byte *hold = tilebuf;
    tilebuf = saves[listpos+i].icon;
    overtile16(framebuf,0,48,i*24+73,i+10,0);  //link?
    tilebuf = hold;

    hold = colordata;
    colordata = saves[listpos+i].pal;
    loadpalset(i+10,0);
    colordata = hold;

    textout(framebuf,zfont,"-",136,i*24+72,1);
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


  saves[savecnt].maxlife = 3*HP_PER_HEART;
  saves[savecnt].maxbombs = 8;
  saves[savecnt].continue_dmap=0;

  int s=savecnt;
  savecnt++;
  listpos=(s/3)*3;

  int pos=s%3;
  int y=pos*24+72;
  int x=0;

  register_mode();
  clear_keybuf();
  SystemKeys=false;
  refreshpal=true;
  bool done=false;

  do {
    if(keypressed())
    {
      int k=readkey();
      if(isprint(k&255)) {
        saves[s].name[min(x,7)]=k&0xFF;
        if(x<8)
          x++;
        sfx(WAV_PLACE);
        }
      else
        switch(k>>8) {
        case KEY_LEFT:  if(x>0)
                          { if(x==8) x=6; else x--; sfx(WAV_CHIME); }
                        break;
        case KEY_RIGHT: if(x<8 && saves[s].name[min(x,7)])
                          { x++; sfx(WAV_CHIME); }
                        break;
        case KEY_ENTER: {
          int ltrs=0;
          for(int i=0; i<8; i++)
            if(saves[s].name[i]!=' ' && saves[s].name[i]!=0)
              ltrs++;
          if(ltrs)
            done=true;
          } break;
        case KEY_BACKSPACE:
          if(x>0) {
            x--;
            for(int i=min(x,7); i<8; i++)
              saves[s].name[i]=saves[s].name[i+1];
            sfx(WAV_OUCH);
            }
          break;
        case KEY_DEL:
          for(int i=min(x,7); i<8; i++)
            saves[s].name[i]=saves[s].name[i+1];
          sfx(WAV_OUCH);
          break;
        }
    }

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
    if(rBbtn())
    {
      x=-1;
      done=true;
    }

  } while(!done && !Quit);

  if(x<0) {
    done=false;
    }

  if(done)
  {
    int quest=1;
    char buf[9];
    strcpy(buf,saves[s].name);
    strupr(buf);
    if(!stricmp(buf,"ZELDA"))
      quest=2;
    if(!stricmp(buf,"ALPHA"))
      quest=3;
    saves[s].quest=quest;
    if(load_quest(saves+s)) {
      saves[s].maxlife=zinit.hc*HP_PER_HEART;
      saves[s].ring=min(zinit.ring,3); ringcolor();
      saves[s].maxbombs=zinit.max_bombs;
      selectscreen();  // refresh palette
    }
    load_game_icon(saves+s);
    saves[s].timevalid=1;
  }

  if(x<0)
  {
    for(int i=s; i<MAXSAVES-1; i++)
      saves[i]=saves[i+1];
    int *di = (int*)(saves+MAXSAVES-1);
    for(unsigned i=0; i<sizeof(gamedata)/sizeof(int); i++)
      *(di++) = 0;
    savecnt--;
    if(listpos>savecnt-1)
      listpos=max(listpos-3,0);
  }

  SystemKeys=true;
  select_mode();
  return done;
}


static bool copy_file(int file)
{
  if(savecnt<MAXSAVES && file<savecnt) {
    saves[savecnt]=saves[file];
    savecnt++;
    listpos=((savecnt-1)/3)*3;
    sfx(WAV_SCALE1);
    select_mode();
    return true;
    }
  return false;
}

static bool delete_file(int file)
{
  if(file<savecnt) {
    for(int i=file; i<MAXSAVES-1; i++)
      saves[i]=saves[i+1];
    int *di = (int*)(saves+MAXSAVES-1);
    for(unsigned i=0; i<sizeof(gamedata)/sizeof(int); i++)
      *(di++) = 0;
    savecnt--;
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
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { jwin_win_proc,     40,   44,   240,  160,  0,       0,      0,       D_EXIT,     0,             0,       (void *) "Select Custom Quest" },
// 1
 { jwin_button_proc,  205,  76,   60,   21,   0,       0,      'b',     D_EXIT,     0,             0,       (void *) "&Browse" },
 { jwin_textbox_proc, 55,   78,   140,  16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_text_proc,       76,   106,  64,   8,    vc(0),   vc(11), 0,       0,          0,             0,       (void *) "Info:" },
 { jwin_textbox_proc, 76,   118,  168,  40,   0,       0,      0,       0,          0,             0,       NULL },
// 5
 { jwin_button_proc,  90,   171,  60,   21,   0,       0,      'k',     D_EXIT,     0,             0,       (void *) "O&K" },
 { jwin_button_proc,  170,  171,  60,   21,   0,       0,      27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



static int get_quest_info(zquestheader *header,char *str)
{
  if(strlen(get_filename(qstpath)) == 0)
  {
    str[0]=0;
    return 0;
  }

  // default error
  strcpy(str,"Error: Invalid quest file");

  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);

  int ret = decode_file_007(qstpath, tmpfilename, ENC_STR);
  if(ret)
  {
    delete_file(tmpfilename);
    switch(ret)
    {
    case 1: strcpy(str,"Error: Unable to open file"); break;
    case 2: strcpy(str,"Internal error occurred"); break;
    }
    return 0;
  }

  PACKFILE *f = pack_fopen(tmpfilename,F_READ_PACKED);
  if(!f)
  {
    delete_file(tmpfilename);
    strcpy(str,"Error: Unable to open file");
    return 0;
  }


  if(!pfread(header,sizeof(zquestheader),f))
  {
    pack_fclose(f);
    delete_file(tmpfilename);
    return 0;
  }

  pack_fclose(f);
  delete_file(tmpfilename);

  if(strcmp(header->id_str,QH_IDSTR))
    return 0;

  strcpy(str,"Error: Invalid version");
  if(header->zelda_version > ZELDA_VERSION)
    return 0;
  if(header->zelda_version < MIN_VERSION)
    return 0;

  if(header->quest_number > 0)
  {
    strcpy(str,"Error: Not a custom quest");
    return 0;
  }

  strcpy(str,"Title:\n");
  strcat(str,header->title);
  strcat(str,"\n\nAuthor:\n");
  strcat(str,header->author);
  return 1;
}



static int custom_game()
{
  zquestheader h;
  char infostr[200];
  char path[260];
  int ret=0;

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
    static EXT_LIST list[] = {
      { "ZC Quests (*.qst)", "qst" },
      { NULL }
    };

    strcpy(path, qstpath);
    if(jwin_file_browse("Load Quest", path, list, &sel))
    {
      strcpy(qstpath, path);
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
  if(saves[file].quest==0)
    return 0;
  BITMAP *info = create_bitmap(168,32);
  clear(info);
  blit(framebuf,info,40,pos*24+70,0,0,168,26);
  rectfill(info,40,0,168,1,0);
  rectfill(info,0,24,39,25,0);
  rectfill(info,0,0,7,15,0);
  rectfill(framebuf,40,70,216,192,0);
  rectfill(framebuf,96,60,183,67,0);

  int i=pos*24+70;
  do {
    blit(info,framebuf,0,0,40,i,168,32);
    advanceframe();
    i-=pos+pos;
  } while(pos && i>=70);

  textout(framebuf,zfont,"GAME TYPE",40,104,3);
  textout(framebuf,zfont,"QUEST",40,112,3);
  textout(framebuf,zfont,"STATUS",40,120,3);

  if(saves[file].quest<0xFf)
  {
    textout(framebuf,zfont,"Normal Game",120,104,1);
    textprintf(framebuf,zfont,120,112,1,"%s Quest",
      ordinal(saves[file].quest));
  }
  else
  {
    textout(framebuf,zfont,"Custom Quest",120,104,1);
    textprintf(framebuf,zfont,120,112,1,"%s",
      get_filename(saves[file].qstpath));
  }

  if(!saves[file].hasplayed)
    textout(framebuf,zfont,"Empty Game",120,120,1);
  else if(!saves[file].timevalid)
    textout(framebuf,zfont,"Time Unknown",120,120,1);
  else
    textout(framebuf,zfont,time_str_med(saves[file].time),120,120,1);

  if(saves[file].cheat)
    textout(framebuf,zfont,"Used Cheats",120,128,1);


  textout(framebuf,zfont,"START: PLAY GAME",56,152,1);
  textout(framebuf,zfont,"    B: CANCEL",56,168,1);
  if(!saves[file].hasplayed)
    textout(framebuf,zfont,"    A: CUSTOM QUEST",56,184,1);

  while(!Quit)
  {
    advanceframe();
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
    if(!saves[file].hasplayed && rAbtn())
    {
      chop_path(qstpath);
      sprintf(qstpath,"%s%s",qstpath,saves[file].qstpath);
      if(custom_game())
      {
        saves[file].quest=0xFF;
        strncpy(saves[file].qstpath,get_filename(qstpath),80);
        load_quest(saves+file);
        saves[file].maxlife=zinit.hc*HP_PER_HEART;
        saves[file].ring=min(zinit.ring,3); ringcolor();
        saves[file].maxbombs=zinit.max_bombs;
        load_game_icon(saves+file);
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

  text_mode(0);
  selectscreen();

  savecnt=0;
  while(savecnt<MAXSAVES && saves[savecnt].quest>0)
    savecnt++;

  if(savecnt==0)
    pos=3;

  bool done=false;
  refreshpal=true;

  do {
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
          if(saves[currgame].quest)
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
          if(delete_file(pos+listpos))
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
      pos--;
      if(pos<0)
        pos=(mode)?2:5;
      sfx(WAV_CHIME);
    }
    if(rDown())
    {
      pos++;
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
        if(saves[currgame].quest)
          done=true;
      }
    }
  } while(!Quit && !done);
}



/**************************************/
/****  Main title screen routine  *****/
/**************************************/


void titlescreen()
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
    if(DXtitle)
      DX_titlescreen();
    else
      NES_titlescreen();

  if(!Quit)
    select_game();

  if(!Quit)
    init_game();
}



void game_over()
{
  kill_sfx();
  stop_midi();
  clear_to_color(screen,BLACK);
  loadfullpal();
  if(Quit==qGAMEOVER)
    jukebox(MUSIC_GAMEOVER);
  Quit=0;

  clear(framebuf);
  text_mode(-1);
  textout(framebuf,zfont,"CONTINUE",88,72,QMisc.colors.text);
  textout(framebuf,zfont,"SAVE",88,96,QMisc.colors.text);
  textout(framebuf,zfont,"RETRY",88,120,QMisc.colors.text);

  int pos = 0;
  int f=-1;
  int htile = QHeader.data_flags[ZQ_TILES] ? 2 : 0;
  bool done=false;
  do {
    if(f==-1)
    {
      if(rUp())   { sfx(WAV_CHINK); pos=(pos==0)?2:pos-1; }
      if(rDown()) { sfx(WAV_CHINK); pos=(pos+1)%3; }
      if(rSbtn()) f++;
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
        case 0: textout(framebuf,zfont,"CONTINUE",88,72,c); break;
        case 1: textout(framebuf,zfont,"SAVE",88,96,c);     break;
        case 2: textout(framebuf,zfont,"RETRY",88,120,c);   break;
        }
      }
    }

    rectfill(framebuf,72,72,79,127,0);
    puttile8(framebuf,htile,72,pos*24+72,1,0);
    advanceframe();
  } while(!Quit && !done);
  reset_combo_animations();
  clear(framebuf);
  advanceframe();

  if(done)
  {
    Quit = pos ? qQUIT : qCONT;
    if(pos==1)
    {
      game.cheat |= cheat;
      saves[currgame] = game;
      load_game_icon(saves+currgame);
      save_savedgames();
    }
  }
}


/*** end of title.cc ***/

