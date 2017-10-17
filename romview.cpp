/*
   ROMview.cc
   Version 3.5
   Last updated: 2/19/00

   By Phantom Menace, Jan. 2000
   E-mail: jer@armageddongames.com

   This code is pretty simple except for the selection array "sel".
   It stores the color set of each tile, one color set per byte,
   but the number of tiles varies depending on the number of bitplanes.
*/
#define ROMVIEW_SCALE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sdl/sdl.h>

//#ifndef _MSC_VER
//#include <dir.h>
//#endif

#include "zc_alleg.h"

#include <png.h>
#include <pngconf.h>

#include <loadpng.h>
#include <jpgalleg.h>

#include "load_gif.h"
#include "save_gif.h"

#include "jwin.h"
#include "jwinfsel.h"
#include "zsys.h"
#include "fontsdat.h"
#include "zdefs.h"
#include "zqscale.h"

//Yeah, not really zquest.
//Just here for scaling purposes.
bool is_zquest()
{
  return true;
}

int tempmode=GFX_AUTODETECT_FULLSCREEN;
int palbmpx=0;
int palbmpy=0;
bool is_large=false;

volatile int myvsync=0;
BITMAP *hw_screen;

char *VerStr(int version)
{
  static char ver_str[12];
  sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
  return ver_str;
}

int scale_arg;
int zq_scale;
char temppath[2048];
bool close_button_quit=false;
void hit_close_button()
{
  close_button_quit=true;
  return;
}

FONT *pfont;
FONT *lfont;
FONT *lfont_l;
FONT *sfont3;
BITMAP *tmp_scr, *scr_buf;
int scrx=0;
int scry=0;
DATAFILE *fontsdata=NULL;
char   fontsdat_sig[52];
int zq_screen_w=320;
int zq_screen_h=240;

void go()
{
  scare_mouse();
  blit(screen,tmp_scr,scrx,scry,0,0,screen->w,screen->h);
  unscare_mouse();
}

void comeback()
{
  scare_mouse();
  blit(tmp_scr,screen,0,0,scrx,scry,screen->w,screen->h);
  unscare_mouse();
}


// Just a way to save a little executable size...
// (Ordering: I prefer VESA over ModeX)
//BEGIN_COLOR_DEPTH_LIST
//  COLOR_DEPTH_8
//END_COLOR_DEPTH_LIST


#define zc_min(a,b)  ((a)<(b)?(a):(b))
#define PANE      28      // number of rows on the screen at a time
                          // rows in the window "pane"


typedef unsigned char  byte;
typedef unsigned short word;


const char *VERSION = "ROMView v4.1";
const int EXTLEN = 32;
const int STA_PAL = 0x5907;
const int ZST_PAL = 0x618;


PALETTE pal[4];
BITMAP  *palbmp;
byte *sel=NULL, *rombuf=NULL;
char *helpbuf=NULL;
int savew=128,saveh=0;
int bp=4,nes=0,cset=15;
int black,white,gray,dkgray,ltgray,blue;
int selsize=0,selcnt=0;
int currpal=0;
int ofs=0, fofs=0;      // offset, fine-tuning offset
long fsize=0;           // romfile size
long helpsize=0;        // helpbuf size
bool single=false;      // selection mode
bool locked=false;
bool prompt = true;     // auto setup


enum { PARTIAL=1, FULL, AUTOMENU };
int  redraw = FULL;

char romfile[260] = "", palfile[260] = "";

char rom_ext[EXTLEN] = " ";

const char *zst_desc = "ZSNES save states";
const char *sta_desc = "NESticle save states";
const char *zst_ext  = "zst;zs1;zs2;zs3;zs4;zs5;zs6;zs7;zs8;zs9";
const char *sta_ext  = "sta;st1;st2;st3;st4;st5;st6;st7;zt8;st9";


EXT_LIST rom_ext_list[] =
{
  { (char *)"SNES ROMs",      (char *)"smc;swc;fig" },
  { (char *)zst_desc,         (char *)zst_ext },
  { (char *)"NES ROMs",       (char *)"nes" },
  { (char *)sta_desc,         (char *)sta_ext },
  { (char *)"User defined",   (char *)rom_ext },
  { (char *)"All files",      NULL },
  { NULL,                     NULL }
};


EXT_LIST pal_ext_list[] =
{
  { (char *)"Image files", (char *)"bmp;pcx;gif;jpg;png" },
  { (char *)zst_desc,      (char *)zst_ext },
  { (char *)sta_desc,      (char *)sta_ext },
  { NULL,                  NULL }
};



void redraw_screen(int type);
void setup_colors();


void byebye()
{
  printf("%s, Phantom Menace, www.armageddongames.com\n", VERSION);
}



/*  puttile_SNES()
 *
 *  Puts an 8x8 tile using "bp" number of bitplanes (1,2,4,8)
 *  in the color set "cs".
 *
 *  See the SNES documentation at zophar.net for more info on
 *  the SNES tile format.
 */
void puttile_SNES(BITMAP *dest,int x,int y,byte *src,int cs)
{
  byte buf[64];

  for(int l=0; l<8; l++)
  {
    int  bx = l<<3;
    byte b = src[(bp&1)?l:l<<1];

    for(int i=7; i>=0; i--)
    {
      buf[bx+i] = (b&1)+(cs<<4);
      b>>=1;
    }
  }

  src++;

  for(int p=1; p<bp; p++)
  {
    for(int l=0; l<8; l++)
    {
      int  bx = l<<3;
      byte b = src[(bp&1)?l:l<<1];

      for(int i=7; i>=0; i--)
      {
        buf[bx+i] |= (b&1)<<p;
        b>>=1;
      }
    }
    if(p&1)
      src+=15;
    else
      src++;
  }

  byte *si = buf;
  for(int j=0; j<8; j++)
    for(int i=0; i<8; i++)
      putpixel(dest,x+i,y+j,*(si++));
}



/*  puttile_NES()
 *
 *  Puts an 8x8 tile in NES format using color set "cs".
 *
 *  See the NES documentation at zophar.net for more info on
 *  the NES tile format.
 */
void puttile_NES(BITMAP *dest,int x,int y,byte *src,int cs)
{
  byte buf[64];

  for(int l=0; l<8; l++)
  {
    int  bx = l<<3;
    byte b = *src;
    src++;

    for(int i=7; i>=0; i--)
    {
      buf[bx+i] = (b&1)+(cs<<4);
      b>>=1;
    }
  }

  for(int l=0; l<8; l++)
  {
    int  bx = l<<3;
    byte b = *src;
    src++;

    for(int i=7; i>=0; i--)
    {
      buf[bx+i] |= (b&1)<<1;
      b>>=1;
    }
  }

  byte *si = buf;
  for(int j=0; j<8; j++)
    for(int i=0; i<8; i++)
      putpixel(dest,x+i,y+j,*(si++));
}

bool sel_row_is_uniform(int offset)
{
  byte val=sel[offset];
  for(int i=1; i<16; i++)
    if(sel[offset+i]!=val)
      return false;
  return true;
}


void draw_pane(byte *buf,byte *s,int px, int py)
{
  // This draws a screen's worth of tiles stored in "buf"
  // using the selection info in "s".

  int bx = ((ofs/128)/bp)*16;
  if(fsize)
  {
    for(int y=0; y<208; y+=8)
    {
      if(*s)
      {
        for(int x=0; x<128; x+=8)
        {
          if(nes)
            puttile_NES(scr_buf,x+4,y+py+2,buf,s[x>>3]>>4);
          else
            puttile_SNES(scr_buf,x+4,y+py+2,buf,s[x>>3]>>4);
          buf += bp<<3;
        }
      }
      else
      {
        for(int x=0; x<128; x+=8)
        {
          if(nes)
            puttile_NES(scr_buf,x+px+2,y+py+2,buf,cset);
          else
            puttile_SNES(scr_buf,x+px+2,y+py+2,buf,cset);
          buf += bp<<3;
        }
      }
      s+=16;
    }
  }
  else
  {
    rectfill(scr_buf,px+2,py+2,px+129,py+209,black);
  }
  jwin_draw_frame(scr_buf,px,py,132, 212, FR_DEEP);

  rectfill(scr_buf,134+px,py+2,139+px,209+py,black);
  jwin_draw_frame(scr_buf,132+px,py,10, 212, FR_DEEP);

  //locked colors
  for(int i=0; i<PANE*16 && bx+i<selsize; i+=16)
  {
    int y = (i>>1) + py+2;

    if(!sel[bx+i])
    {
      textprintf_ex(scr_buf,font,135+px,y,white,-1," ");
    }
    else
    {
      if(sel_row_is_uniform(bx+i))
      {
        textprintf_ex(scr_buf,font,135+px,y,white,-1,"%X",sel[bx+i]>>4);
      }
      else
      {
        textprintf_ex(scr_buf,font,135+py,y,white,-1,"<");
      }
    }
  }

}


void set_sel_row(int offset, byte val)
{
  for(int i=0; i<16; i++)
    sel[offset+i]=val;
}


void calc_selcnt()
{
  selcnt=0;
    for(int i=0; i<selsize; i+=16)
      if(sel[i])
        selcnt++;
}


void calc_saveh()
{
  switch(savew)
  {
  case 128: saveh = selcnt*8; break;
  case 256: saveh = (selcnt/2)*8+((selcnt&1)?8:0); break;
  case 512: saveh = (selcnt/4)*8+((selcnt&3)?8:0); break;
  default:  saveh = -1;
  }
}


BITMAP *create_save_bitmap()
{
  byte *buf = rombuf + fofs;

  calc_saveh();
  BITMAP *bmp = create_bitmap(savew,saveh);
  if(!bmp)
    return NULL;

  clear_bitmap(bmp);

  int y=0;
  int xofs=0;
  for(int i=0; i<selsize; i+=16)
  {
    if(!sel[i])
      buf += bp<<7;
    else
    {
      for(int x=0; x<128; x+=8)
      {
        if(nes)
          puttile_NES(bmp,x+xofs,y,buf,sel[i+(x>>3)]>>4);
        else
          puttile_SNES(bmp,x+xofs,y,buf,sel[i+(x>>3)]>>4);
        buf += bp<<3;
      }
      y+=8;
      if(y>=saveh)
      {
        xofs+=128;
        y=0;
      }
    }
  }

  return bmp;
}



RGB _RGB(int r,int g,int b)
{
  RGB x;
  x.r = r;
  x.g = g;
  x.b = b;
  return x;
}


RGB invRGB(RGB c)
{
  RGB x;
  x.r = 63-c.r;
  x.g = 63-c.g;
  x.b = 63-c.b;
  return x;
}


inline int pal_sum(RGB p)
{
  return p.r + p.g + p.b;
}

int pal_abs(RGB p,RGB q)
{
  return abs(p.r-q.r) + abs(p.g-q.g) + abs(p.b-q.b);
}


void get_bw(RGB *p)
{
  // This locates black, white and gray colors in the palette "p".
  // The color indices are stored globally.

/*
  RGB DkGray = {20,20,20, 0};
  RGB Gray =   {32,32,32, 0};
  RGB LtGray = {48,48,48, 0};
  RGB Blue   = { 0, 0,32, 0};

  // don't use color 0
  black=white=gray=dkgray=ltgray=blue=1;

  for(int i=1; i<256; i++)
  {
    if(pal_sum(p[i])<pal_sum(p[black]))
      black=i;
    if(pal_sum(p[i])>pal_sum(p[white]))
      white=i;
    if(pal_abs(p[i],Gray) < pal_abs(p[gray],Gray))
      gray=i;
    if(pal_abs(p[i],DkGray) < pal_abs(p[dkgray],DkGray))
      dkgray=i;
    if(pal_abs(p[i],LtGray) < pal_abs(p[ltgray],LtGray))
      ltgray=i;
    if(pal_abs(p[i],Blue) < pal_abs(p[blue],Blue))
      blue=i;
  }
*/
  black= bestfit_color(p,  0,  0,  0);
  white= bestfit_color(p, 63, 63, 63);
  dkgray=bestfit_color(p, 20, 20, 20);
  gray=  bestfit_color(p, 32, 32, 32);
  ltgray=bestfit_color(p, 48, 48, 48);
  blue=  bestfit_color(p,  0,  0, 32);
}

/*
byte mouse_data[16*16] =
{
	1,1,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,1,1,1,1,
	1,2,2,1,2,2,1,0,0,0,
	1,2,1,0,1,2,2,1,0,0,
	1,1,0,0,1,2,2,1,0,0,
	0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,1,1,1,0
};
*/

byte mouse_data[16*16] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,
	0,2,3,2,1,0,0,0,0,0,0,0,0,0,0,0,
	0,2,4,3,2,1,0,0,0,0,0,0,0,0,0,0,
	0,2,4,3,3,2,1,0,0,0,0,0,0,0,0,0,
	0,2,4,4,3,3,2,1,0,0,0,0,0,0,0,0,
	0,2,4,4,3,3,3,2,1,0,0,0,0,0,0,0,
	0,2,4,4,4,3,3,3,2,1,0,0,0,0,0,0,
	0,2,4,4,4,3,3,3,3,2,1,0,0,0,0,0,
	0,2,4,4,4,4,3,3,3,1,1,1,0,0,0,0,
	0,2,4,4,3,3,1,1,1,0,0,0,0,0,0,0,
	0,2,3,3,1,1,0,0,0,0,0,0,0,0,0,0,
	0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
	0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


DIALOG help_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
 { d_textbox_proc,    0,    0,    319,  239,  0,       0,      0,       0,          0,        0,        NULL, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,        KEY_ESC,  (void *)close_dlg, NULL, NULL },
 { NULL,              0,    0,    0,    0,    0,    0,    0,     0,       0,    0,             NULL, NULL, NULL }
};


void do_help()
{
  help_dlg[0].dp = helpbuf;
  help_dlg[0].fg = ltgray;
  help_dlg[0].bg = black;
  help_dlg[0].dp2 = lfont;

  popup_dialog(help_dlg,0);
}



long readfile(const char *path,void *buf,int count)
{
  FILE *f=fopen(path,"rb");
  if(!f)
    return 0;
  long r=fread(buf,1,count,f);
  fclose(f);
  return r;
}



void setup_palettes()
{
  for(int i=0; i<256; i++)
    pal[1][i] = pal[2][i] = pal[0][i];

  for(int i=0; i<256; i+=16)
    pal[1][i] = black_palette[0];

  for(int i=0; i<256; i++)
    pal[3][i] = invRGB(pal[0][i]);

  for(int i=0; i<256; i+=16)
    pal[2][i] = pal[3][i];
}



RGB nes_palette[] =
{
  {31,31,31, 0}, //  0
  { 0, 0,63, 0}, //  1
  { 0, 0,47, 0}, //  2
  {17,10,47, 0}, //  3
  {37, 0,33, 0}, //  4
  {42, 0, 8, 0}, //  5
  {42, 4, 0, 0}, //  6
  {34, 5, 0, 0}, //  7
  {20,12, 0, 0}, //  8
  { 0,30, 0, 0}, //  9
  { 0,26, 0, 0}, // 10
  { 0,22, 0, 0}, // 11
  { 0,16,22, 0}, // 12
  { 0, 0, 0, 0}, // 13
  { 0, 0, 0, 0}, // 14
  { 0, 0, 0, 0}, // 15
  {47,47,47, 0}, // 16
  { 0,30,62, 0}, // 17
  { 0,22,62, 0}, // 18
  {26,17,63, 0}, // 19
  {54, 0,51, 0}, // 20
  {57, 0,22, 0}, // 21
  {62,14, 0, 0}, // 22
  {57,23, 4, 0}, // 23
  {43,31, 0, 0}, // 24
  { 0,46, 0, 0}, // 25
  { 0,42, 0, 0}, // 26
  { 0,42,17, 0}, // 27
  { 0,34,34, 0}, // 28
  { 0, 0, 0, 0}, // 29
  { 0, 0, 0, 0}, // 30
  { 0, 0,32, 0}, // 31
  {62,62,62, 0}, // 32
  {15,47,63, 0}, // 33
  {26,34,63, 0}, // 34
  {38,30,62, 0}, // 35
  {62,30,62, 0}, // 36
  {62,22,38, 0}, // 37
  {62,30,22, 0}, // 38
  {63,40,17, 0}, // 39
  {62,46, 0, 0}, // 40
  {46,62, 6, 0}, // 41
  {22,54,21, 0}, // 42
  {22,62,38, 0}, // 43
  { 0,58,54, 0}, // 44
  {30,30,30, 0}, // 45
  { 0, 0, 0, 0}, // 46
  {16,16,16, 0}, // 47
  {63,63,63, 0}, // 48
  {41,57,63, 0}, // 49
  {46,46,62, 0}, // 50
  {54,46,62, 0}, // 51
  {62,46,62, 0}, // 52
  {62,41,48, 0}, // 53
  {60,52,44, 0}, // 54
  {63,56,42, 0}, // 55
  {62,54,30, 0}, // 56
  {54,62,30, 0}, // 57
  {46,62,46, 0}, // 58
  {46,62,54, 0}, // 59
  { 0,63,63, 0}, // 60
  {62,54,62, 0}, // 61
  { 0,54,50, 0}, // 62
  {31,63,63, 0}  // 63
};



void load_sta_pal(byte *si)
{
//  pal[0] = black_palette;
  memcpy(pal[0], black_palette, sizeof(PALETTE));
  RGB *di = pal[0];

  for(int i=0; i<32; i++)
  {
    *di = *(di+128) = nes_palette[(*si)&63];
    si++;
    di++;
    if((i&3)==3)
      di+=12;
  }

  pal[0][15] = nes_palette[0x30];
  pal[0][14] = nes_palette[0x10];
  pal[0][13] = nes_palette[0x00];
  pal[0][12] = nes_palette[0x2F];
  pal[0][11] = nes_palette[0x1F];

  if(palbmp)
  {
    free(palbmp);
    palbmp = NULL;
  }

  setup_palettes();
  currpal = cset = 0;
  redraw=FULL;
  setup_colors();
}



void load_zst_pal(byte *src)
{
  word *si = (word*)src;
  RGB *di = pal[0];

  for(int i=0; i<256; i++)
  {
    *di = _RGB((((*si)<<1)&0x3f),(((*si)>>4)&0x3e),(((*si)>>9)&0x3e));
    si++;
    di++;
  }

  if(palbmp)
  {
    free(palbmp);
    palbmp = NULL;
  }

  setup_palettes();
  currpal = cset = 0;
  redraw=FULL;
  setup_colors();
}



void load_rom(char *path)
{
  if(!path)
    return;

  long size = file_size_ex_password(path,"");

  if(size==0)
  {
    jwin_alert("Load error",path,"not found",NULL,"OK",NULL,13,27,lfont);
    return;
  }

  fsize = size;

  if(rombuf)
    free(rombuf);
  if(sel)
    free(sel);

  // make sel big enough for one byte per tile in 1-bitplane mode
  // then a row of 16 tiles takes 128 bytes
  selsize = (fsize>>7)<<4;
  rombuf = (byte*)malloc(fsize);
  sel    = (byte*)malloc(selsize);

  if(!sel || !rombuf)
  {
    jwin_alert("Load error","Out of memory",NULL,NULL,"OK",NULL,13,27,lfont);
    fsize = selsize = 0;
    return;
  }

  for(int i=0; i<selsize; i++)
    sel[i]=0;

  if(readfile(path,rombuf,fsize)!=fsize)
  {
    jwin_alert("Load error","Error reading",path,NULL,"OK",NULL,13,27,lfont);
    fsize = selsize = 0;
    return;
  }

  ofs = fofs = selcnt = 0;
  locked=false;

  char *ext = get_extension(path);
  if((ext[0]&0xDF)=='S' && (ext[1]&0xDF)=='T')
  {
    if(!prompt || jwin_alert("Confirm Auto Setup","NESticle save state?","Want auto setup?",NULL,"&Yes","&No",'y','n',lfont)==1)
    {
      load_sta_pal(rombuf+STA_PAL);
      bp = 2;
      nes = 1;
      fofs = 7;
      ofs = 0x2900;
    }
  }

  if((ext[0]&0xDF)=='Z' && (ext[1]&0xDF)=='S')
  {
    if(!prompt || jwin_alert("Confirm Auto Setup","ZSNES save state?","Want auto setup?",NULL,"&Yes","&No",'y','n',lfont)==1)
    {
      load_zst_pal(rombuf+ZST_PAL);
      bp = 4;
      nes = 0;
      fofs = 0x13;
      ofs = 0x20C00;
    }
  }

  redraw=FULL;
  strncpy(romfile,path,260);
  char tbuf[2048];
  char *rbuf=get_filename(romfile);
  char *pbuf=get_filename(palfile);
  sprintf(tbuf, "ROMView%s%s%s%s", (rbuf[0]||pbuf[0])?" - ":"", rbuf[0]?rbuf:"", (rbuf[0]&&pbuf[0])?", ":"", pbuf[0]?pbuf:"");
  set_window_title(tbuf);
}


void load_pal(char *path)
{
  if(!path)
  {
    get_palette(pal[0]);

    for(int i=0; i<16; i++)
    {
      pal[0][i+32]  = _RGB(i<<1,i<<1,i<<2);
      pal[0][i+48]  = _RGB(i<<1,i<<2,i<<2);
      pal[0][i+64]  = _RGB(i<<1,i<<2,i<<1);
      pal[0][i+80]  = _RGB(i<<2,i<<2,i<<1);
      pal[0][i+96]  = _RGB(i<<2,i<<1,i<<1);
      pal[0][i+112] = _RGB(i<<2,i<<1,i<<2);
      pal[0][i+128] = _RGB(i<<2,i<<2,i<<2);
    }

    for(int i=144; i<256; i++)
      pal[0][i] = invRGB(pal[0][i-112]);
  }
  else
  {
    char *ext = get_extension(path);
    long size = file_size_ex_password(path, "");

    if(size==0)
    {
      jwin_alert("Load error",path,"not found",NULL,"OK",NULL,13,27,lfont);
      return;
    }
    else if((ext[0]&0xDF)=='S' && (ext[1]&0xDF)=='T')
    {
      byte *buf = (byte*)malloc(size);
      if(buf)
      {
        if(readfile(path,buf,size)!=size)
        {
          jwin_alert("Load error","Error reading",path,NULL,"OK",NULL,13,27,lfont);
          free(buf);
          return;
        }
        load_sta_pal(buf+STA_PAL);
        free(buf);
      }
    }
    else if((ext[0]&0xDF)=='Z' && (ext[1]&0xDF)=='S')
    {
      byte *buf = (byte*)malloc(size);
      if(buf)
      {
        if(readfile(path,buf,size)!=size)
        {
          jwin_alert("Load error","Error reading",path,NULL,"OK",NULL,13,27,lfont);
          free(buf);
          return;
        }
        load_zst_pal(buf+ZST_PAL);
        free(buf);
      }
    }
    else
    {
      PALETTE p;
      BITMAP *bmp = load_bitmap(path,p);

      if(!bmp)
      {
        jwin_alert("Load error","Error reading",path,"as an image file","OK",NULL,13,27,lfont);
        return;
      }

      int used[16];
      int used_cnt = 0;

      for(int r=0; r<16; r++)
      {
        used[r] = 0;
        for(int i=0; i<16; i++)
          if(pal_sum(p[(r<<4)+i]))
          {
            used[r] = 1;
            used_cnt++;
            break;
          }
      }


      if((used_cnt == 3) &&
         (jwin_alert("Load Palette","NESticle screen shot?",NULL,NULL,"&Yes","&No",'y','n',lfont)==1))
      {
         RGB *si = p+16;
         RGB *di = p+128;
         for(int i=0; i<32; i++)
         {
            *di = *si;
            si++;
            di++;
            if((i&3)==3)
              di+=12;
         }
         p[15] = nes_palette[0x30];
         p[14] = nes_palette[0x10];
         p[13] = nes_palette[0x00];
         p[12] = nes_palette[0x2F];
         p[11] = nes_palette[0x1F];
      }

//      pal[0] = p;
      memcpy(pal[0], p, sizeof(PALETTE));
      if(palbmp)
        free(palbmp);
      palbmp = bmp;

      setup_colors();
      cset = 0;
    }

    strncpy(palfile,path,260);
  }
  char tbuf[2048];
  char *rbuf=get_filename(romfile);
  char *pbuf=get_filename(palfile);
  sprintf(tbuf, "ROMView%s%s%s%s", rbuf[0]?" - ":"", rbuf[0]?rbuf:"", pbuf[0]?", ":"", pbuf[0]?pbuf:"");
  set_window_title(tbuf);
  palbmpx=0;
  palbmpy=0;
  setup_palettes();
  currpal=0;
  redraw=FULL;
  setup_colors();
}



DIALOG types_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
 { jwin_win_proc,     0,    0,    240,  96,   0,       0,      0,       D_EXIT,     0,        0,        (void *)"ROM File Types", NULL, NULL },
 { jwin_text_proc,    16,   28,   208,  8,    0,       0,      0,       0,          0,        0,        (void *)"User defined extensions", NULL, NULL },
 { jwin_edit_proc,    16,   38,   208,  16,   0,       0,      0,       0,          EXTLEN-1, 0,        NULL, NULL, NULL },
 { jwin_button_proc,  48,   65,   60,   21,   0,       0,      0,       D_EXIT,     0,        0,        (void *)"OK", NULL, NULL },
 { jwin_button_proc,  132,  65,   60,   21,   0,       0,      0,       D_EXIT,     0,        0,        (void *)"Cancel", NULL, NULL },
 { NULL,              0,    0,    0,    0,    0,    0,    0,     0,       0,    0,             NULL, NULL, NULL }
};



int onROMTypes()
{
   char buf[EXTLEN];
   strcpy(buf, rom_ext);

   centre_dialog(types_dlg);

   types_dlg[0].dp2 = lfont;
   types_dlg[2].dp = buf;

   int ret = popup_dialog(types_dlg, 0);

   if(ret==3)
     strcpy(rom_ext,buf);

   return D_O_K;
}


int onLoadROM()
{
   char path[260];
   strcpy(path,romfile);
   static int list_sel = 0;

   int ret = jwin_file_browse_ex("Load ROM File", path, rom_ext_list, &list_sel, 2048, -1, -1, lfont);
   redraw_screen(FULL);

   if(ret)
     load_rom(path);

   return D_O_K;
}


int onLoadPal()
{
   char path[260];
   strcpy(path,palfile);
   static int list_sel = 0;

   int ret = jwin_file_browse_ex("Load Palette", path, pal_ext_list, &list_sel, 2048, -1, -1, lfont);
   redraw_screen(FULL);

   if(ret)
     load_pal(path);

   return D_O_K;
}



int onBP1()   { if(!locked) { bp=1; nes=0; redraw=PARTIAL; }  return D_O_K; }
int onBP2()   { if(!locked) { bp=2; nes=0; redraw=PARTIAL; }  return D_O_K; }
int onBP4()   { if(!locked) { bp=4; nes=0; redraw=PARTIAL; }  return D_O_K; }
int onBP8()   { if(!locked) { bp=8; nes=0; redraw=PARTIAL; }  return D_O_K; }
int onBPNES() { if(!locked) { bp=2; nes=1; redraw=PARTIAL; }  return D_O_K; }

int onDecCSet()  { cset=(cset-1)&15; redraw=PARTIAL;  return D_O_K; }
int onIncCSet()  { cset=(cset+1)&15; redraw=PARTIAL;  return D_O_K; }

int onDecOfs()   { if(fofs>0) fofs--; redraw=PARTIAL;  return D_O_K; }
int onIncOfs()   { fofs++; redraw=PARTIAL;  return D_O_K; }

int onUp()       { ofs -= 8*16*bp; redraw=PARTIAL;  return D_O_K; }
int onDown()     { ofs += 8*16*bp; redraw=PARTIAL;  return D_O_K; }
int onLeft()     { fofs -= 8*bp; if(fofs<0) fofs=0; redraw=PARTIAL;  return D_O_K; }
int onRight()    { fofs += 8*bp; redraw=PARTIAL;  return D_O_K; }
int onPgUp()     { ofs -= 8*PANE*16*bp; redraw=PARTIAL;  return D_O_K; }
int onPgDn()     { ofs += 8*PANE*16*bp; redraw=PARTIAL;  return D_O_K; }
int onHome()     { ofs = 0;     redraw=PARTIAL;  return D_O_K; }
int onEnd()      { ofs = fsize; redraw=PARTIAL;  return D_O_K; }

int onMode()     { single = !single; redraw=PARTIAL;  return D_O_K; }


int onWidth()
{
   if(savew==512)
     savew=128;
   else
     savew<<=1;
   redraw=PARTIAL;
   return D_O_K;
}


int onPalette()
{
   currpal++;
   currpal&=3;
   setup_colors();
   redraw=FULL;
   return D_O_K;
}




void do_save(bool saveas)
{
   static int fnum=1;

   struct al_ffblk fb;
   BITMAP *bmp;
   char   fname[260];

   if(selcnt==0)
   {
     jwin_alert("Save Error","Nothing is selected",NULL,NULL,"OK",NULL,13,27,lfont);
     return;
   }

   // create the image
   bmp = create_save_bitmap();
   if(!bmp)
   {
     jwin_alert("Save Error","Out of memory",NULL,NULL,"OK",NULL,13,27,lfont);
     return;
   }

   // find next default file name
   sprintf(fname,"rombmp%02d.bmp",fnum);
   while(fnum<99 && !al_findfirst(fname, &fb, FA_ALL))
   {
     sprintf(fname,"rombmp%02d.bmp",++fnum);
   }


   // save as ?
   if(saveas)
   {
     int ret = jwin_file_select_ex("Save Image File (bmp,pcx)", fname, "bmp;pcx", 2048, -1, -1, lfont);
     redraw_screen(FULL);

     if(ret==0)
       goto clean_up;

     if(!al_findfirst(fname, &fb, FA_ALL))
     {
       if(jwin_alert("Confirm Overwrite",fname,"already exists.","Overwrite it?","&Yes","&No",'y','n',lfont)==2)
         goto clean_up;
     }
   }

   // write the file
   if(save_bitmap(fname,bmp,pal[currpal])==0)
     jwin_alert("ROMview","Saved as:",fname,NULL,"OK",NULL,13,27,lfont);
   else
     jwin_alert("Save Error","Error saving:",fname,NULL,"OK",NULL,13,27,lfont);

   clean_up:
   destroy_bitmap(bmp);
}



int onSave()
{
   do_save(false);
   return D_O_K;
}


int onSaveAs()
{
   do_save(true);
   return D_O_K;
}


int onPreview()
{
  BITMAP *bmp,*buf;
  int  px=0,py=0;
  double scale=1.0;
  bool done=false, refresh=true;

  if(selcnt==0)
  {
    jwin_alert("Preview Error","Nothing is selected",NULL,NULL,"OK",NULL,13,27,lfont);
    return D_O_K;
  }

  // create the image
  bmp = create_save_bitmap();
  if(!bmp)
  {
    jwin_alert("Preview Error","Out of memory",NULL,NULL,"OK",NULL,13,27,lfont);
    return D_O_K;
  }

  // use double buffering if possible
  buf = create_bitmap(320,240);

  scare_mouse();

  while(!done)
  {
    if(refresh)
    {
      // use double buffering if possible
      BITMAP *b = (buf!=NULL) ? buf : screen;

      if(buf==NULL)
        vsync();

      clear_to_color(b,black);
      stretch_blit(bmp,b,0,0,bmp->w,bmp->h,
          int(320+(px-bmp->w)*scale)/2,int(240+(py-bmp->h)*scale)/2,
          int(bmp->w*scale),int(bmp->h*scale));

      //text_mode(black);
      textprintf_ex(b,font,0,232,white,-1,"%dx%d %.0f%%",bmp->w,bmp->h,scale*100);

      if(buf)
      {
        vsync();
        blit(buf,screen,0,0,0,0,320,240);
      }
      refresh=false;
    }

    int speed = 5;
    if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
      speed = 25;

    if(key[KEY_UP])    { py+=speed; refresh=true; }
    if(key[KEY_DOWN])  { py-=speed; refresh=true; }
    if(key[KEY_LEFT])  { px+=speed; refresh=true; }
    if(key[KEY_RIGHT]) { px-=speed; refresh=true; }

    if(keypressed() && !refresh)
    {
      refresh=true;
      switch(readkey()>>8)
      {
        case KEY_ESC:   done=true; break;
        case KEY_PGUP:  if(scale>0.1) scale*=0.95; break;
        case KEY_PGDN:  if(scale<5.0) scale/=0.95; break;
        case KEY_TILDE: scale=0.5; break;
        case KEY_1:     scale=1.0; break;
        case KEY_2:     scale=2.0; break;
        case KEY_3:     scale=3.0; break;
        case KEY_4:     scale=4.0; break;
        case KEY_5:     scale=5.0; break;
        case KEY_C:     px=py=0; break;
        case KEY_Z:     px = bmp->w - int(320 / scale);
                        py = bmp->h - int(240 / scale);
                        break;
        default:        refresh=false;
      }
    }
  } // while(!done)

  destroy_bitmap(bmp);
  destroy_bitmap(buf);
  rectfill(screen,0,0,319,15,black);
  unscare_mouse();
  redraw_screen(FULL);
  return D_REDRAW;
}




int onSelectAll()
{
   if(fsize && jwin_alert("Confirm Select","Select entire ROM?",NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
   {
     locked=true;
     for(int i=0; i<selsize/bp; i+=16)
        set_sel_row(i,(cset<<4)+1);
   }
   redraw=PARTIAL;
   return D_O_K;
}


int onClearAll()
{
   if(fsize && jwin_alert("Confirm Clear","Clear all selections?",NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
   {
     locked=false;
     for(int i=0; i<selsize; i++)
        sel[i]=0;
   }
   redraw=PARTIAL;
   return D_O_K;
}


int onSelectPane()
{
   if(fsize)
   {
     locked=true;
     int bx = ((ofs/128)/bp)*16;
     for(int i=0; i<PANE*16; i+=16)
        set_sel_row(bx+i,16*cset+1);
     redraw=PARTIAL;
   }
   return D_O_K;
}


int onHelp()
{
  if(helpsize==0)
  {
    jwin_alert("ROMView","Help file not loaded",NULL,NULL,"OK",NULL,13,27,lfont);
  }
  else
  {
    do_help();
  }
  return D_O_K;
}


int onExit()
{
   if(jwin_alert("ROMview","Exit?",NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
      return D_CLOSE;

   return D_O_K;
}


int onAbout()
{
   jwin_alert("About",VERSION,"by Phantom Menace","www.armageddongames.com","OK",NULL,13,27,lfont);
   return D_O_K;
}


int onAutoAlways() { prompt = false; redraw = AUTOMENU; return D_O_K; }
int onAutoPrompt() { prompt = true;  redraw = AUTOMENU; return D_O_K; }


MENU auto_menu[] =
{
   { (char *)"Always",               onAutoAlways, NULL, 0, NULL },
   { (char *)"Prompt",               onAutoPrompt, NULL, 0, NULL },
   { NULL,                           NULL,         NULL, 0, NULL }
};


MENU file_menu[] =
{
   { (char *)"Load ROM\tF3",         onLoadROM,   NULL,      0, NULL },
   { (char *)"Load Palette\tF4",     onLoadPal,   NULL,      0, NULL },
   { (char *)"ROM File Types",       onROMTypes,  NULL,      0, NULL },
   { (char *)"Auto setup",           NULL,        auto_menu, 0, NULL },
   { (char *)"",                     NULL,        NULL,      0, NULL },
   { (char *)"E&xit\tESC",           onExit,      NULL,      0, NULL },
   { NULL,                           NULL,        NULL,      0, NULL }
};


MENU rip_menu[] =
{
   { (char *)"Bitmap &Width\tw",     onWidth,     NULL, 0, NULL },
   { (char *)"Pre&view Bitmap\tv",   onPreview,   NULL, 0, NULL },
   { (char *)"",                     NULL,        NULL, 0, NULL },
   { (char *)"Save\ts",              onSave,      NULL, 0, NULL },
   { (char *)"Save As...",           onSaveAs,    NULL, 0, NULL },
   { NULL,                           NULL,        NULL, 0, NULL }
};


MENU sel_menu[] =
{
   { (char *)"Single\tm",                 onMode,       NULL, 0, NULL },
   { (char *)"",                          NULL,         NULL, 0, NULL },
   { (char *)"Select &Screen\t<space>",   onSelectPane, NULL, 0, NULL },
   { (char *)"Select &All\ta",            onSelectAll,  NULL, 0, NULL },
   { (char *)"&Clear All\tc",             onClearAll,   NULL, 0, NULL },
   { NULL,                                NULL,         NULL, 0, NULL }
};


MENU bp_menu[] =
{
   { (char *)"&1\t1",                onBP1,       NULL, 0, NULL },
   { (char *)"&2\t2",                onBP2,       NULL, 0, NULL },
   { (char *)"&4\t4",                onBP4,       NULL, 0, NULL },
   { (char *)"&8\t8",                onBP8,       NULL, 0, NULL },
   { (char *)"&NES\tn",              onBPNES,     NULL, 0, NULL },
   { NULL,                           NULL,        NULL, 0, NULL }
};


MENU misc_menu[] =
{
   { (char *)"&About...",            onAbout,     NULL, 0, NULL },
   { (char *)"&Help\tF1",            onHelp,      NULL, 0, NULL },
   { (char *)"",                     NULL,        NULL, 0, NULL },
   { (char *)"&Palette\tp",          onPalette,   NULL, 0, NULL },
   { NULL,                           NULL,        NULL, 0, NULL }
};


MENU main_menu[] =
{
   { (char *)"&File",                NULL,        file_menu,  0, NULL },
   { (char *)"&Ripper",              NULL,        rip_menu,   0, NULL },
   { (char *)"&Selection",           NULL,        sel_menu,   0, NULL },
   { (char *)"&BP",                  NULL,        bp_menu,    0, NULL },
   { (char *)"&Misc",                NULL,        misc_menu,  0, NULL },
   { NULL,                           NULL,        NULL,       0, NULL }
};



DIALOG main_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)  (flags)  (d1)    (d2)         (dp) */
 { jwin_win_proc,     -2,   -2,   324,  244,  0,    0,    0,     0,       0,      0,             NULL, NULL, NULL },
 { jwin_menu_proc,    0,    0,    0,    0,    0,    0,    0,     0,       0,      0,             (void *)main_menu, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_ESC,       (void *)onExit, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_F1,        (void *)onHelp, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_F3,        (void *)onLoadROM, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_F4,        (void *)onLoadPal, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_1,         (void *)onBP1, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_2,         (void *)onBP2, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_4,         (void *)onBP4, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_8,         (void *)onBP8, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_N,         (void *)onBPNES, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_UP,        (void *)onUp, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_DOWN,      (void *)onDown, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_PGUP,      (void *)onPgUp, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_PGDN,      (void *)onPgDn, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_HOME,      (void *)onHome, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_END,       (void *)onEnd, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_LEFT,      (void *)onLeft, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_RIGHT,     (void *)onRight, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_MINUS_PAD, (void *)onDecCSet, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_PLUS_PAD,  (void *)onIncCSet, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_P,         (void *)onPalette, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_A,         (void *)onSelectAll, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_C,         (void *)onClearAll, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_SPACE,     (void *)onSelectPane, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_S,         (void *)onSave, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_W,         (void *)onWidth, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_V,         (void *)onPreview, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_Z,         (void *)onDecOfs, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_X,         (void *)onIncOfs, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_M,         (void *)onMode, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_EQUALS, (void *)onDecCSet, NULL, NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,     0,       0,      KEY_MINUS,  (void *)onIncCSet, NULL, NULL },
 { NULL,              0,    0,    0,    0,    0,    0,    0,     0,       0,      0,             NULL, NULL, NULL }
};



void setup_colors()
{
  static BITMAP *mouse = NULL;

  set_palette(pal[currpal]);
  get_bw(pal[currpal]);
  gui_bg_color = black;
  gui_fg_color = white;
  gui_mg_color = ltgray;
  set_dialog_color(main_dlg, gui_fg_color, gui_bg_color);

  jwin_colors[jcSELFG] =
  jwin_colors[jcLIGHT] =
  jwin_colors[jcTITLEFG] =
  jwin_colors[jcTEXTBG] = white;

  jwin_colors[jcMEDLT] = ltgray;

  jwin_colors[jcBOX] = ltgray;

  jwin_colors[jcMEDDARK] = gray;

  jwin_colors[jcBOXFG] =
  jwin_colors[jcTEXTFG] = black;

  jwin_colors[jcDARK] = dkgray;

  jwin_colors[jcTITLEL] =
  jwin_colors[jcTITLER] =
  jwin_colors[jcSELBG] = blue;

  jwin_set_colors(jwin_colors);

  if(!mouse)
    mouse = create_bitmap(16,16);

  if(mouse)
  {
    int si=0;
    for(int y=0; y<16; y++)
      for(int x=0; x<16; x++)
      {
        int c=mouse_data[si++];
        switch (c)
        {
          case 1:
            c=dkgray;
            break;
          case 2:
            c=gray;
            break;
          case 3:
            c=ltgray;
            break;
          case 4:
            c=white;
            break;
        }
        putpixel(mouse,x,y,c);
      }
    set_mouse_sprite(mouse);
  }
  show_mouse(screen);
}


void redraw_screen(int type)
{
  int bx = ((ofs/128)/bp)*16;
  calc_saveh();

  scare_mouse();
  clear_to_color(scr_buf, ltgray);

  //rom contents
  textout_ex(scr_buf,font,"Tiles",2+66,24-8,black,-1);
  draw_pane(rombuf+ofs+fofs,sel+bx,2,26);

  if(type==FULL)
  {
    //palette source image
    rectfill(scr_buf,148,76,315,235,black);
    if(palbmp)
    {
      blit(palbmp,scr_buf,palbmpx,palbmpy,148,76,168,160);
    }
    jwin_draw_frame(scr_buf,146,74,172, 164, FR_DEEP);

    //palette
    for(int i=0; i<256; i++)
    {
      int x = ((i&15)*3)+160+88;
      int y = ((i>>4)*3)+76-74;
      rectfill(scr_buf,x,y+16,x+2,y+2+16,i);
    }
    //palette rectangles
    jwin_draw_frame(scr_buf,159+87,0+16,52, 52, FR_DEEP);
    jwin_draw_frame(scr_buf,159+87+52,0+16,7, 52, FR_DEEP);
    //rect(scr_buf,159+88,75-74,230+88,140-74,white);
    //rect(scr_buf,224+88,76-74,225+88,139-74,white);
  }

  //line above info box
  //hline(scr_buf,144,16,319,black);
  //info box contents
  textprintf_ex(scr_buf,font,146,17,black,-1,"bit planes: %d %s  ",bp,locked?"locked":"      ");
  textprintf_ex(scr_buf,font,146,25,black,-1,"row offset: %06Xh   ",ofs);
  textprintf_ex(scr_buf,font,146,33,black,-1,"tuning offset: %03Xh   ",fofs);
  textprintf_ex(scr_buf,font,146,41,black,-1,"bitmap size: %dx%d     ",savew,saveh);
  textprintf_ex(scr_buf,font,146,49,black,-1,"bitmap size: %d KB     ",(saveh*savew)>>10);
  textprintf_ex(scr_buf,font,146,57,black,-1,"selection: %s     ",single?"single":"row   ");
  textprintf_ex(scr_buf,font,146,65,black,-1,"color set: %X",cset);
  textprintf_ex(scr_buf,font,215,65,black,-1,"pal: %d ",currpal);

  //rom contents/locked color divider
  //rectfill(scr_buf,128,16,131,239,black);
  //locked color/info box divider
  //rectfill(scr_buf,140,16,143,239,black);
  //cset indicator background
  rectfill(scr_buf,226+88-14,76-74+16,229+88-14-1,139-74,black);
  //cset indicator
  rectfill(scr_buf,226+88-14,76-74+(cset*3)+16,229+88-14-1,79-74+(cset*3)-1+16,white);

  blit(scr_buf, screen, 0, 16, 0, 16, zq_screen_w, zq_screen_h-16);
  unscare_mouse();

  redraw=0;
}



int log2(const int x)
{
  int log = 0;
  int a = 1;

  while(x > a)
  {
    a <<= 1;
    log++;
  }

  return log;
}

int main(int argc, char **argv)
{

/*
	// Initialize SDL
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		return(1);
	}
*/
  allegro_init();
  install_timer();
  install_keyboard();
  install_mouse();

  three_finger_flag=false;
  register_bitmap_file_type("GIF",  load_gif, save_gif);
  jpgalleg_init();
  loadpng_init();

  set_config_file("ag.cfg");
  tmp_scr=create_bitmap_ex(8, 320, 240);
  scr_buf=create_bitmap_ex(8, 320, 240);
  memset(temppath, 0, 2048);

  gui_mouse_focus = 0;

  set_color_depth(8);
  set_close_button_callback((void (*)()) hit_close_button);

#ifndef ALLEGRO_DOS
  zq_scale = get_config_int("romview","scale",1);
  scale_arg = used_switch(argc,argv,"-scale");

  scale_arg = false; // What!?

  if(scale_arg && (argc>(scale_arg+1)))
  {
    scale_arg = atoi(argv[scale_arg+1]);
    if (scale_arg == 0)
    {
      scale_arg = 1;
    }
    zq_scale=scale_arg;
  }
  else
  {
    scale_arg = zq_scale;
  }
  // Ho hum...Romview has some problems so just disable scaling altogether...for now. -Gleeok
  //zqwin_set_scale(scale_arg);
  zqwin_set_scale(0,false);
#endif

  if(used_switch(argc,argv,"-fullscreen"))
  {
    tempmode = GFX_AUTODETECT_FULLSCREEN;
  }
  else if(used_switch(argc,argv,"-windowed"))
  {
    tempmode=GFX_AUTODETECT_WINDOWED;
  }

  if (tempmode==GFX_AUTODETECT_FULLSCREEN)
  {
#ifdef ALLEGRO_MACOSX
    scale_arg=2;
#else
    if (scale_arg>2)
    {
      scale_arg=1;
    }
#endif
    zqwin_set_scale(scale_arg);
  }

  /* No -fullscreen while debugging */
#if defined _MSC_VER && defined _DEBUG
	tempmode=GFX_AUTODETECT_WINDOWED;
#endif

  if(set_gfx_mode(tempmode,320,240,0,0) != 0)
  {
    allegro_exit();
    byebye();
    printf("Error setting video mode: %s\n", allegro_error);
    return 2;
  }

  resolve_password(datapwd);
  //setPackfilePassword(datapwd);
  packfile_password(datapwd);

  sprintf(fontsdat_sig,"Fonts.Dat %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);

  Z_message("Fonts.Dat...");
  if((fontsdata=load_datafile("fonts.dat"))==NULL)
  {
    Z_error("failed");
  }
  if(strncmp((char*)fontsdata[0].dat,fontsdat_sig,23))
  {
    unload_datafile(fontsdata);
    Z_error("\nIncompatible version of fonts.dat.\nPlease upgrade to %s Build %d",VerStr(FONTSDAT_VERSION), FONTSDAT_BUILD);
  }
  Z_message("OK\n");

  //setPackfilePassword(NULL);
  packfile_password("");
  //const char *passwd = "";

  font = (FONT*)fontsdata[FONT_GUI_PROP].dat;
  pfont = (FONT*)fontsdata[FONT_8xPROP_THIN].dat;
  lfont = (FONT*)fontsdata[FONT_LARGEPROP].dat;

  load_rom((argc>1 && argv[1][0]!='-') ? argv[1] : NULL);
  load_pal((argc>2 && argv[2][0]!='-') ? argv[2] : NULL);


  helpsize = file_size_ex_password("romview.txt","");

  if(helpsize==0)
  {
    helpbuf=NULL;
  }
  else
  {
    helpbuf=(char*)malloc(helpsize+1);
    if(!helpbuf)
      helpsize=0;
    else
    {
      if(readfile("romview.txt",helpbuf,helpsize)!=helpsize)
        helpsize=0;
      else
        helpbuf[helpsize]=0;
    }
  }


  redraw_screen(FULL);
  redraw = FULL; // to update menu

  misc_menu[1].flags = (helpsize==0) ? D_DISABLED : 0;
  auto_menu[0].flags = prompt ? 0 : D_SELECTED;
  auto_menu[1].flags = prompt ? D_SELECTED : 0;

  DIALOG_PLAYER *player = init_dialog(main_dlg,-1);

  while(update_dialog(player))
  {
    if(redraw==AUTOMENU)
    {
      auto_menu[0].flags = prompt ? 0 : D_SELECTED;
      auto_menu[1].flags = prompt ? D_SELECTED : 0;
      redraw = 0;
    }
    else if(redraw)
    {
      int maxofs = (fsize - 128*PANE*bp) & (0xFFFFFF80 << (log2(bp)));
      if(ofs > maxofs)
         ofs = maxofs;
      if(ofs < 0)
         ofs = 0;

      calc_selcnt();

      // update menu flags
      if(fsize)
      {
        sel_menu[0].flags = single ? D_SELECTED : 0;
        bp_menu[0].flags = ((bp==1) ? D_SELECTED : 0) + (locked ? D_DISABLED : 0);
        bp_menu[1].flags = ((bp==2)&&(!nes) ? D_SELECTED : 0) + (locked ? D_DISABLED : 0);
        bp_menu[2].flags = ((bp==4) ? D_SELECTED : 0) + (locked ? D_DISABLED : 0);
        bp_menu[3].flags = ((bp==8) ? D_SELECTED : 0) + (locked ? D_DISABLED : 0);
        bp_menu[4].flags = ((bp==2)&&(nes) ? D_SELECTED : 0) + (locked ? D_DISABLED : 0);

        rip_menu[1].flags =
        rip_menu[3].flags =
        rip_menu[4].flags = (selcnt==0) ? D_DISABLED : 0;
      }

      // redraw screen

      if(redraw==FULL)
      {
        main_menu[1].flags =
        main_menu[2].flags =
        main_menu[3].flags = (fsize==0) ? D_DISABLED : 0;
//        rectfill(screen,0,0,319,15,black);
        broadcast_dialog_message(MSG_DRAW,0);
      }

      redraw_screen(FULL);

      // wait for mouse button release
      // and eat key presses in the mean time
      while(gui_mouse_b())
        clear_keybuf();
    }

    if(mouse_in_rect(2,26,142,208))
    {
      //rom contents
      if((gui_mouse_b()&1) && fsize)
      {
        locked=true;
        int bx = ((ofs/128)/bp)*16;
        int di = ((gui_mouse_y()-26)/8)*16;
        if(sel[bx+di]==0 || !single)
          set_sel_row(bx+di,16*cset+1);
        if(single)
        {
          int x = zc_min(gui_mouse_x()/8,15);
          sel[bx+di+x] = 16*cset+1;
        }
        redraw=PARTIAL;
        calc_selcnt();
      }
      else if(gui_mouse_b()&2)
      {
        int bx = ((ofs/128)/bp)*16;
        int di = ((gui_mouse_y()-26)/8)*16;
        set_sel_row(bx+di,0);
        redraw=PARTIAL;
        calc_selcnt();
      }
      else if (gui_mouse_z())
      {
        if (mouse_z>0)
        {
          switch (((key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])?2:0)+((key[KEY_LSHIFT]||key[KEY_RSHIFT])?1:0))
          {
            case 3: //control, shift
              onLeft();
              break;
            case 2: //control
              onDecOfs();
              break;
            case 1: //shift
              onPgUp();
              break;
            case 0: //nothing
            default:
              onUp();
              break;
          }
        }
        else
        {
          switch (((key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])?2:0)+((key[KEY_LSHIFT]||key[KEY_RSHIFT])?1:0))
          {
            case 3: //control, shift
              onRight();
              break;
            case 2: //control
              onIncOfs();
              break;
            case 1: //shift
              onPgDn();
              break;
            case 0: //nothing
            default:
              onDown();
              break;
          }
        }
        position_mouse_z(0);
      }
    }
    else if(mouse_in_rect(246,18,59,48))
    {
      //palette
      if(gui_mouse_b()&1)
      {
        while(gui_mouse_b()&1)
        {
          int new_cset = zc_min(zc_max(0, (gui_mouse_y()-18)/3),15);
          if (cset != new_cset)
          {
            cset = new_cset;
            scare_mouse();
            redraw_screen(FULL);
            unscare_mouse();
          }
        }
      }
      else if (gui_mouse_z())
      {
        if (mouse_z>0)
        {
          scare_mouse();
          cset=(cset+15)%16;
          redraw_screen(FULL);
          unscare_mouse();
        }
        else
        {
          scare_mouse();
          cset=(cset+1)%16;
          redraw_screen(FULL);
          unscare_mouse();
        }
        position_mouse_z(0);
      }
    }
    else if(mouse_in_rect(148,76,168,160))
    {
      // palette source image
      if (palbmp)
      {
        if(gui_mouse_b()&1)
        {
          int sx=palbmpx+gui_mouse_x();
          int sy=palbmpy+gui_mouse_y();
          while(gui_mouse_b()&1)
          {
            int newpalbmpx = zc_min(zc_max(0,sx-gui_mouse_x()),zc_max(palbmp->w-168,0));
            int newpalbmpy = zc_min(zc_max(0,sy-gui_mouse_y()),zc_max(palbmp->h-160,0));
            if (palbmpx != newpalbmpx || palbmpy != newpalbmpy)
            {
              palbmpx = newpalbmpx;
              palbmpy = newpalbmpy;
              scare_mouse();
              redraw_screen(FULL);
              unscare_mouse();
            }
          }
        }
      }
    }
    if (close_button_quit)
    {
      close_button_quit=false;
      if (onExit()==D_CLOSE)
      {
        break;
      }
    }
  } // while(update_dialog(player))

  shutdown_dialog(player);

  free(rombuf);
  free(sel);
  destroy_bitmap(tmp_scr);
  destroy_bitmap(scr_buf);
  allegro_exit();
  byebye();

  return 0;
}
END_OF_MAIN()

// these are here so that copy_dialog won't choke when compiling romview
int d_alltriggerbutton_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_comboa_radio_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_comboabutton_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssdn_btn_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssdn_btn2_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssdn_btn3_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssdn_btn4_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_sslt_btn_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_sslt_btn2_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_sslt_btn3_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_sslt_btn4_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssrt_btn_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssrt_btn2_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssrt_btn3_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssrt_btn4_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssup_btn_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssup_btn2_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssup_btn3_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_ssup_btn4_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_tri_edit_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_triggerbutton_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int dcs_auto_button_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_jbutton_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_kbutton_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_listen_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_savemidi_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_dummy_proc(int, DIALOG*, int)
{
  return D_O_K;
}

int d_timer_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;
  d=d;
  msg=msg;

  return D_O_K;
}

void large_dialog(DIALOG *d) { d=d; }

/* That's all folks */


