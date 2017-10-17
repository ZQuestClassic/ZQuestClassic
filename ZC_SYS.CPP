//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_sys.cc
//
//  System functions, input handlers, GUI stuff, etc.
//  for Zelda Classic.
//
//--------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <dir.h>
#include <ctype.h>
#include "zc_alleg.h"

#ifdef ALLEGRO_DOS
#include <unistd.h>
#endif

#include "zdefs.h"
#include "zelda.h"
#include "tiles.h"
#include "colors.h"
#include "pal.h"
#include "zsys.h"
#include "qst.h"
#include "zc_sys.h"
#include "debug.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "gui.h"
#include "midi.h"
#include "subscr.h"
#include "maps.h"
#include "sprite.h"
#include "guys.h"
#include "link.h"
#include "title.h"
#include "particles.h"
#include "midi_strict.inc"

extern FONT *lfont;
extern LinkClass Link;
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;
//extern movingblock mblock2; //mblock[4]?

//extern int db;

static char *ZC_str = "Zelda Classic";
#ifdef ALLEGRO_DOS
static char *qst_dir_name = "dos_qst_dir";
#elif defined(ALLEGRO_WINDOWS)
static char *qst_dir_name = "win_qst_dir";
#elif defined(ALLEGRO_LINUX)
static char *qst_dir_name = "linux_qst_dir";
#elif defined(ALLEGRO_MACOSX)
static char *qst_dir_name = "macosx_qst_dir";
#endif

/**********************************/
/******** System functions ********/
/**********************************/

static char cfg_sect[] = "zeldadx";

int d_dummy_proc(int msg,DIALOG *d,int c) { return D_O_K; }

#ifdef NEEDS_MIDI_STRICT
bool midi_strict=true;
#endif
void load_game_configs()
{
  Akey = get_config_int(cfg_sect,"key_a",KEY_ALT);
  Bkey = get_config_int(cfg_sect,"key_b",KEY_ZC_LCONTROL);
  Skey = get_config_int(cfg_sect,"key_s",KEY_ENTER);
  Lkey = get_config_int(cfg_sect,"key_l",KEY_Z);
  Rkey = get_config_int(cfg_sect,"key_r",KEY_X);
  Pkey = get_config_int(cfg_sect,"key_p",KEY_SPACE);

  DUkey = get_config_int(cfg_sect,"key_up",   KEY_UP);
  DDkey = get_config_int(cfg_sect,"key_down", KEY_DOWN);
  DLkey = get_config_int(cfg_sect,"key_left", KEY_LEFT);
  DRkey = get_config_int(cfg_sect,"key_right",KEY_RIGHT);

  Abtn = get_config_int(cfg_sect,"btn_a",2);
  Bbtn = get_config_int(cfg_sect,"btn_b",1);
  Sbtn = get_config_int(cfg_sect,"btn_s",3);
  Mbtn = get_config_int(cfg_sect,"btn_m",4);
  Lbtn = get_config_int(cfg_sect,"btn_l",5);
  Rbtn = get_config_int(cfg_sect,"btn_r",6);
  Pbtn = get_config_int(cfg_sect,"btn_p",7);

  digi_volume = get_config_int(cfg_sect,"sfx",248);
  midi_volume = get_config_int(cfg_sect,"music",255);
  pan_style = get_config_int(cfg_sect,"pan",1);
  volkeys = (bool)get_config_int(cfg_sect,"volkeys",0);
//  Vsync = (bool)get_config_int(cfg_sect,"vsync",0);
  Throttlefps = get_config_int(cfg_sect,"throttlefps",1)!=0;
  TransLayers = (bool)get_config_int(cfg_sect,"translayers",1);
  ShowFPS = (bool)get_config_int(cfg_sect,"showfps",0);
  NESquit = (bool)get_config_int(cfg_sect,"fastquit",0);
  DXtitle = (bool)get_config_int(cfg_sect,"title",1);
#ifdef ALLEGRO_DOS
  VidMode = get_config_int(cfg_sect,"dos_mode",GFX_AUTODETECT);
#elif defined(ALLEGRO_WINDOWS)
  VidMode = get_config_int(cfg_sect,"win_mode",GFX_AUTODETECT_FULLSCREEN);
#elif defined(ALLEGRO_LINUX)
  VidMode = get_config_int(cfg_sect,"linux_mode",GFX_AUTODETECT_FULLSCREEN);
#elif defined(ALLEGRO_MACOSX)
  VidMode = get_config_int(cfg_sect,"macosx_mode",GFX_AUTODETECT_FULLSCREEN);
#endif
  resx = get_config_int(cfg_sect,"resx",320);
  resy = get_config_int(cfg_sect,"resy",240);
  sbig = get_config_int(cfg_sect,"sbig",0);
  scanlines = get_config_int(cfg_sect,"scanlines",0);
  strcpy(qstpath,get_config_string(cfg_sect,qst_dir_name,""));

  if(strlen(qstpath)==0)
  {
    getcwd(qstpath,280);
    fix_filename_case(qstpath);
    fix_filename_slashes(qstpath);
    put_backslash(qstpath);
  }
  else
  {
    chop_path(qstpath);
  }
  ss_after = vbound(get_config_int(cfg_sect,"ss_after",5), 0, 14);
  ss_speed = vbound(get_config_int(cfg_sect,"ss_speed",2), 0, 6);
  ss_density = vbound(get_config_int(cfg_sect,"ss_density",3), 0, 6);
  heart_beep = get_config_int(cfg_sect,"heart_beep",1);
  gui_colorset = get_config_int(cfg_sect,"gui_colorset",0);
  midi_strict = get_config_int(cfg_sect,"strict_midi",1);
}

void save_game_configs()
{
  set_config_int(cfg_sect,"key_a",Akey);
  set_config_int(cfg_sect,"key_b",Bkey);
  set_config_int(cfg_sect,"key_s",Skey);
  set_config_int(cfg_sect,"key_l",Lkey);
  set_config_int(cfg_sect,"key_r",Rkey);
  set_config_int(cfg_sect,"key_p",Pkey);

  set_config_int(cfg_sect,"key_up",   DUkey);
  set_config_int(cfg_sect,"key_down", DDkey);
  set_config_int(cfg_sect,"key_left", DLkey);
  set_config_int(cfg_sect,"key_right",DRkey);

  set_config_int(cfg_sect,"btn_a",Abtn);
  set_config_int(cfg_sect,"btn_b",Bbtn);
  set_config_int(cfg_sect,"btn_s",Sbtn);
  set_config_int(cfg_sect,"btn_m",Mbtn);
  set_config_int(cfg_sect,"btn_l",Lbtn);
  set_config_int(cfg_sect,"btn_r",Rbtn);
  set_config_int(cfg_sect,"btn_p",Pbtn);

  set_config_int(cfg_sect,"sfx",digi_volume);
  set_config_int(cfg_sect,"music",midi_volume);
  set_config_int(cfg_sect,"pan",pan_style);
  set_config_int(cfg_sect,"volkeys",(int)volkeys);
//  set_config_int(cfg_sect,"vsync",(int)Vsync);
  set_config_int(cfg_sect,"throttlefps", (int)Throttlefps);
  set_config_int(cfg_sect,"translayers",(int)TransLayers);
  set_config_int(cfg_sect,"showfps",(int)ShowFPS);
  set_config_int(cfg_sect,"fastquit",(int)NESquit);
  set_config_int(cfg_sect,"title",(int)DXtitle);

#ifdef ALLEGRO_DOS
  set_config_int(cfg_sect,"dos_mode",VidMode);
#elif defined(ALLEGRO_WINDOWS)
  set_config_int(cfg_sect,"win_mode",VidMode);
#elif defined(ALLEGRO_LINUX)
  set_config_int(cfg_sect,"linux_mode",VidMode);
#elif defined(ALLEGRO_MACOSX)
  set_config_int(cfg_sect,"macosx_mode",VidMode);
#endif

  set_config_int(cfg_sect,"resx",resx);
  set_config_int(cfg_sect,"resy",resy);
  set_config_int(cfg_sect,"sbig",sbig);
  set_config_int(cfg_sect,"scanlines",scanlines);
  chop_path(qstpath);
  set_config_string(cfg_sect,"qst_dir",qstpath);
  set_config_int(cfg_sect,"ss_after",ss_after);
  set_config_int(cfg_sect,"ss_speed",ss_speed);
  set_config_int(cfg_sect,"ss_density",ss_density);
  set_config_int(cfg_sect,"heart_beep",heart_beep);
  set_config_int(cfg_sect,"gui_colorset",gui_colorset);
  set_config_int(cfg_sect,"strict_midi",midi_strict);
}

//----------------------------------------------------------------

// Timers

void fps_callback()
{
  lastfps=framecnt;
  avgfps=((long double)avgfps*fps_secs+lastfps)/(++fps_secs);
  framecnt=0;
}

END_OF_FUNCTION(fps_callback)

void myvsync_callback()
{
  ++myvsync;
}

END_OF_FUNCTION(myvsync_callback)

void Z_init_timers()
{
  static bool didit = false;
  static char *err_str = "Couldn't allocate timer";

  if(didit)
    return;

  didit = true;

  LOCK_VARIABLE(lastfps);
  LOCK_VARIABLE(framecnt);
  LOCK_FUNCTION(fps_callback);
  if(install_int_ex(fps_callback,SECS_TO_TIMER(1)))
    Z_error(err_str);

  LOCK_VARIABLE(myvsync);
  LOCK_FUNCTION(myvsync_callback);
  if(install_int_ex(myvsync_callback,BPS_TO_TIMER(60)))
    Z_error(err_str);
}

//----------------------------------------------------------------

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

void dump_pal(BITMAP *dest)
{
  for(int i=0; i<256; i++)
    rectfill(dest,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}

void show_paused()
{
  //  return;
  char buf[7] = "PAUSED";
  for(int i=0; buf[i]!=0; i++)
    buf[i]+=0x60;

  //  text_mode(-1);
  if(sbig)
    textout_ex(screen,zfont,buf,scrx+40-120,scry+224+104,-1,-1);
  else
    textout_ex(screen,zfont,buf,scrx+40,scry+224,-1,-1);
}

void show_fps()
{
  char buf[50];

  //  text_mode(-1);
  sprintf(buf,"%2d/60",lastfps);
  //  sprintf(buf,"%d/%u/%f/%u",lastfps,int(avgfps),avgfps,fps_secs);
  for(int i=0; buf[i]!=0; i++)
    if(buf[i]!=' ')
      buf[i]+=0x60;

  if(sbig)
  {
    textout_ex(screen,zfont,buf,scrx+40-120,scry+216+104,-1,-1);
  }
  else
  {
    textout_ex(screen,zfont,buf,scrx+40,scry+216,-1,-1);
  }
}

//----------------------------------------------------------------

// sets the video mode and initializes the palette and mouse sprite
bool game_vid_mode(int mode,int wait)
{
  //  set_color_depth(colordepth);

  #ifdef ALLEGRO_DOS
  switch(mode)
  {
    case GFX_AUTODETECT:
    case GFX_VESA3:
      if(set_gfx_mode(GFX_VESA3,resx,resy,0,0)==0)
      {
        VidMode=GFX_VESA3;
        break;
      }
    case GFX_VESA2L:
      if(set_gfx_mode(GFX_VESA2L,resx,resy,0,0)==0)
      {
        VidMode=GFX_VESA2L;
        break;
      }
    case GFX_VESA2B:
      if(set_gfx_mode(GFX_VESA2B,resx,resy,0,0)==0)
      {
        VidMode=GFX_VESA2B;
        break;
      }
    case GFX_VESA1:
      if(set_gfx_mode(GFX_VESA1,resx,resy,0,0)==0)
      {
        VidMode=GFX_VESA1;
        break;
      }
    case GFX_MODEX:
      if(set_gfx_mode(GFX_MODEX,320,240,0,0)==0)
      {
        VidMode=GFX_MODEX;
        resx=320;
        resy=240;
        sbig=false;
        break;
      }
    default:
      return false;
      break;
  }
  #elif defined(ALLEGRO_WINDOWS)
  switch(mode)
  {
    case GFX_AUTODETECT_FULLSCREEN:
    case GFX_DIRECTX:
      if(set_gfx_mode(GFX_DIRECTX,resx,resy,0,0)==0)
      {
        VidMode=GFX_DIRECTX;
        break;
      }
    case GFX_DIRECTX_SOFT:
      if(set_gfx_mode(GFX_DIRECTX_SOFT,resx,resy,0,0)==0)
      {
        VidMode=GFX_DIRECTX_SOFT;
        break;
      }
    case GFX_DIRECTX_SAFE:
      if(set_gfx_mode(GFX_DIRECTX_SAFE,resx,resy,0,0)==0)
      {
        VidMode=GFX_DIRECTX_SAFE;
      }
      break;
    case GFX_AUTODETECT_WINDOWED:
    case GFX_DIRECTX_WIN:
      if(set_gfx_mode(GFX_DIRECTX_WIN,resx,resy,0,0)==0)
      {
        VidMode=GFX_DIRECTX_WIN;
        break;
      }
    case GFX_GDI:
      if(set_gfx_mode(GFX_GDI,resx,resy,0,0)==0)
      {
        VidMode=GFX_GDI;
        break;
      }
    default:
      return false;
      break;
  }
  #elif defined(ALLEGRO_LINUX)
  switch(mode)
  {
case GFX_AUTODETECT_FULLSCREEN:    
case GFX_AUTODETECT_WINDOWED:
      if(set_gfx_mode(GFX_AUTODETECT_WINDOWED,resx,resy,0,0)==0)
      {
        VidMode=GFX_AUTODETECT_WINDOWED;
        break;
      }
    default:
      return false;
      break;
  }
  #elif defined(ALLEGRO_MACOSX)
  switch(mode)
  {
    case GFX_AUTODETECT_WINDOWED:
      if(set_gfx_mode(GFX_AUTODETECT_WINDOWED,resx,resy,0,0)==0)
      {
        VidMode=GFX_AUTODETECT_WINDOWED;
        break;
      }
    default:
      return false;
      break;
  }
  #endif

  scrx = (resx-320)>>1;
  scry = (resy-240)>>1;

  set_mouse_sprite((BITMAP*)data[BMP_MOUSE].dat);
  for(int i=240; i<256; i++)
    RAMpal[i]=((RGB*)data[PAL_GUI].dat)[i];
  set_palette(RAMpal);
  clear_to_color(screen,BLACK);

  rest(wait);
  return true;
}

void init_NES_mode()
{
  if (!init_colordata(true, &QHeader, &QMisc))
  {
    return;
  }
  loadfullpal();
  init_tiles(false, &QHeader);
}

//----------------------------------------------------------------

qword trianglelines[16]=
{
  0x0000000000000000ULL,
  0xFD00000000000000ULL,
  0xFDFD000000000000ULL,
  0xFDFDFD0000000000ULL,
  0xFDFDFDFD00000000ULL,
  0xFDFDFDFDFD000000ULL,
  0xFDFDFDFDFDFD0000ULL,
  0xFDFDFDFDFDFDFD00ULL,
  0xFDFDFDFDFDFDFDFDULL,
  0x00FDFDFDFDFDFDFDULL,
  0x0000FDFDFDFDFDFDULL,
  0x000000FDFDFDFDFDULL,
  0x00000000FDFDFDFDULL,
  0x0000000000FDFDFDULL,
  0x000000000000FDFDULL,
  0x00000000000000FDULL,
};

word screen_triangles[28][32];
/*
qword triangles[4][16]= //[direction][value]
{
  {
    0x00000000, 0x10000000, 0x21000000, 0x32100000, 0x43210000, 0x54321000, 0x65432100, 0x76543210, 0x87654321, 0x88765432, 0x88876543, 0x88887654, 0x88888765, 0x88888876, 0x88888887, 0x88888888
  },
  {
    0x00000000, 0xF0000000, 0xEF000000, 0xFDF00000, 0xCFDF0000, 0xBCFDF000, 0xABCFDF00, 0x9ABCFDF0, 0x89ABCFDF, 0x889ABCFD, 0x8889ABCD, 0x88889ABC, 0x888889AB, 0x8888889A, 0x88888889, 0x88888888
  },
  {
    0x00000000, 0x00000001, 0x00000012, 0x00000123, 0x00001234, 0x00012345, 0x00123456, 0x01234567, 0x12345678, 0x23456788, 0x34567888, 0x45678888, 0x56788888, 0x67888888, 0x78888888, 0x88888888
  },
  {
    0x00000000, 0x0000000F, 0x000000FE, 0x00000FED, 0x0000FEDC, 0x000FEDCB, 0x00FEDCBA, 0x0FEDCBA9, 0xFEDCBA98, 0xEDCBA988, 0xDCBA9888, 0xCBA98888, 0xBA988888, 0xA9888888, 0x98888888, 0x88888888
  }
};
*/


/*
byte triangles[4][16][8]= //[direction][value][line]
{
  {
    {
       0,  0,  0,  0,  0,  0,  0,  0
    },
    {
       1,  0,  0,  0,  0,  0,  0,  0
    },
    {
       2,  1,  0,  0,  0,  0,  0,  0
    },
    {
       3,  2,  1,  0,  0,  0,  0,  0
    },
    {
       4,  3,  2,  1,  0,  0,  0,  0
    },
    {
       5,  4,  3,  2,  1,  0,  0,  0
    },
    {
       6,  5,  4,  3,  2,  1,  0,  0
    },
    {
       7,  6,  5,  4,  3,  2,  1,  0
    },
    {
       8,  7,  6,  5,  4,  3,  2,  1
    },
    {
       8,  8,  7,  6,  5,  4,  3,  2
    },
    {
       8,  8,  8,  7,  6,  5,  4,  3
    },
    {
       8,  8,  8,  8,  7,  6,  5,  4
    },
    {
       8,  8,  8,  8,  8,  7,  6,  5
    },
    {
       8,  8,  8,  8,  8,  8,  7,  6
    },
    {
       8,  8,  8,  8,  8,  8,  8,  7
    },
    {
       8,  8,  8,  8,  8,  8,  8,  8
    }
  },
  {
    {
       0,  0,  0,  0,  0,  0,  0,  0
    },
    {
      15,  0,  0,  0,  0,  0,  0,  0
    },
    {
      14, 15,  0,  0,  0,  0,  0,  0
    },
    {
      13, 14, 15,  0,  0,  0,  0,  0
    },
    {
      12, 13, 14, 15,  0,  0,  0,  0
    },
    {
      11, 12, 13, 14, 15,  0,  0,  0
    },
    {
      10, 11, 12, 13, 14, 15,  0,  0
    },
    {
       9, 10, 11, 12, 13, 14, 15,  0
    },
    {
       8,  9, 10, 11, 12, 13, 14, 15
    },
    {
       8,  8,  9, 10, 11, 12, 13, 14
    },
    {
       8,  8,  8,  9, 10, 11, 12, 13
    },
    {
       8,  8,  8,  8,  9, 10, 11, 12
    },
    {
       8,  8,  8,  8,  8,  9, 10, 11
    },
    {
       8,  8,  8,  8,  8,  8,  9, 10
    },
    {
       8,  8,  8,  8,  8,  8,  8,  9
    },
    {
       8,  8,  8,  8,  8,  8,  8,  8
    }
  },
  {
    {
       0,  0,  0,  0,  0,  0,  0,  0
    },
    {
       0,  0,  0,  0,  0,  0,  0,  1
    },
    {
       0,  0,  0,  0,  0,  0,  1,  2
    },
    {
       0,  0,  0,  0,  0,  1,  2,  3
    },
    {
       0,  0,  0,  0,  1,  2,  3,  4
    },
    {
       0,  0,  0,  1,  2,  3,  4,  5
    },
    {
       0,  0,  1,  2,  3,  4,  5,  6
    },
    {
       0,  1,  2,  3,  4,  5,  6,  7
    },
    {
       1,  2,  3,  4,  5,  6,  7,  8
    },
    {
       2,  3,  4,  5,  6,  7,  8,  8
    },
    {
       3,  4,  5,  6,  7,  8,  8,  8
    },
    {
       4,  5,  6,  7,  8,  8,  8,  8
    },
    {
       5,  6,  7,  8,  8,  8,  8,  8
    },
    {
       6,  7,  8,  8,  8,  8,  8,  8
    },
    {
       7,  8,  8,  8,  8,  8,  8,  8
    },
    {
       8,  8,  8,  8,  8,  8,  8,  8
    }
  },
  {
    {
       0,  0,  0,  0,  0,  0,  0,  0
    },
    {
       0,  0,  0,  0,  0,  0,  0, 15
    },
    {
       0,  0,  0,  0,  0,  0, 15, 14
    },
    {
       0,  0,  0,  0,  0, 15, 14, 13
    },
    {
       0,  0,  0,  0, 15, 14, 13, 12
    },
    {
       0,  0,  0, 15, 14, 13, 12, 11
    },
    {
       0,  0, 15, 14, 13, 12, 11, 10
    },
    {
       0, 15, 14, 13, 12, 11, 10,  9
    },
    {
      15, 14, 13, 12, 11, 10,  9,  8
    },
    {
      14, 13, 12, 11, 10,  9,  8,  8
    },
    {
      13, 12, 11, 10,  9,  8,  8,  8
    },
    {
      12, 11, 10,  9,  8,  8,  8,  8
    },
    {
      11, 10,  9,  8,  8,  8,  8,  8
    },
    {
      10,  9,  8,  8,  8,  8,  8,  8
    },
    {
       9,  8,  8,  8,  8,  8,  8,  8
    },
    {
       8,  8,  8,  8,  8,  8,  8,  8
    }
  }
};
*/



/*
for (int blockrow=0; blockrow<30; ++i)
{
  for (int linerow=0; linerow<8; ++i)
  {
    qword *triangleline=(qword*)(tmp_scr->line[(blockrow*8+linerow)]);
    for (int blockcolumn=0; blockcolumn<40; ++i)
    {
      triangleline=triangles[0][screen_triangles[blockrow][blockcolumn]][linerow];
      ++triangleline;
    }
  }
}
*/

// the ULL suffixes are to prevent this warning:
// warning: integer constant is too large for "long" type

qword triangles[4][16][8]= //[direction][value][line]
{
  {
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFD00000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFDFD000000000000ULL,
      0xFD00000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFDFDFD0000000000ULL,
      0xFDFD000000000000ULL,
      0xFD00000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFDFDFDFD00000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFD000000000000ULL,
      0xFD00000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFD000000000000ULL,
      0xFD00000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFD000000000000ULL,
      0xFD00000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFD000000000000ULL,
      0xFD00000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFD000000000000ULL,
      0xFD00000000000000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFD000000000000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFD0000000000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFD00000000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFD000000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFD0000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFD00ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    }
  },
  {
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x00000000000000FDULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x000000000000FDFDULL,
      0x00000000000000FDULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x0000000000FDFDFDULL,
      0x000000000000FDFDULL,
      0x00000000000000FDULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x00000000FDFDFDFDULL,
      0x0000000000FDFDFDULL,
      0x000000000000FDFDULL,
      0x00000000000000FDULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x000000FDFDFDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x0000000000FDFDFDULL,
      0x000000000000FDFDULL,
      0x00000000000000FDULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x0000FDFDFDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x0000000000FDFDFDULL,
      0x000000000000FDFDULL,
      0x00000000000000FDULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x00FDFDFDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x0000000000FDFDFDULL,
      0x000000000000FDFDULL,
      0x00000000000000FDULL,
      0x0000000000000000ULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x0000000000FDFDFDULL,
      0x000000000000FDFDULL,
      0x00000000000000FDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x0000000000FDFDFDULL,
      0x000000000000FDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x0000000000FDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x00000000FDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x000000FDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    }
  },
  {
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0xFD00000000000000ULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0xFD00000000000000ULL,
      0xFDFD000000000000ULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0xFD00000000000000ULL,
      0xFDFD000000000000ULL,
      0xFDFDFD0000000000ULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0xFD00000000000000ULL,
      0xFDFD000000000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFDFDFD00000000ULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0xFD00000000000000ULL,
      0xFDFD000000000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFDFDFD000000ULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0xFD00000000000000ULL,
      0xFDFD000000000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFDFDFD0000ULL
    },
    {
      0x0000000000000000ULL,
      0xFD00000000000000ULL,
      0xFDFD000000000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFDFDFD00ULL
    },
    {
      0xFD00000000000000ULL,
      0xFDFD000000000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFD000000000000ULL,
      0xFDFDFD0000000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFD0000000000ULL,
      0xFDFDFDFD00000000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFD00000000ULL,
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFD000000ULL,
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFD0000ULL,
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFD00ULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    }
  },
  {
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x00000000000000FDULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x00000000000000FDULL,
      0x000000000000FDFDULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x00000000000000FDULL,
      0x000000000000FDFDULL,
      0x0000000000FDFDFDULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x00000000000000FDULL,
      0x000000000000FDFDULL,
      0x0000000000FDFDFDULL,
      0x00000000FDFDFDFDULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x00000000000000FDULL,
      0x000000000000FDFDULL,
      0x0000000000FDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x000000FDFDFDFDFDULL
    },
    {
      0x0000000000000000ULL,
      0x0000000000000000ULL,
      0x00000000000000FDULL,
      0x000000000000FDFDULL,
      0x0000000000FDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL
    },
    {
      0x0000000000000000ULL,
      0x00000000000000FDULL,
      0x000000000000FDFDULL,
      0x0000000000FDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL
    },
    {
      0x00000000000000FDULL,
      0x000000000000FDFDULL,
      0x0000000000FDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0x000000000000FDFDULL,
      0x0000000000FDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0x0000000000FDFDFDULL,
      0x00000000FDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0x00000000FDFDFDFDULL,
      0x000000FDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0x000000FDFDFDFDFDULL,
      0x0000FDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0x0000FDFDFDFDFDFDULL,
      0x00FDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0x00FDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    },
    {
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL,
      0xFDFDFDFDFDFDFDFDULL
    }
  }
};

int black_opening_count=0;
int black_opening_x,black_opening_y;
int black_opening_shape=bosCIRCLE;
//int black_opening_shape=bosSMAS;

void close_black_opening(int x, int y, bool wait)
{
  int w=256, h=224;
  int blockrows=28, blockcolumns=32;
  int xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
  for (int blockrow=0; blockrow<blockrows; ++blockrow) //30
  {
    for (int blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn) //40
    {
      screen_triangles[blockrow][blockcolumn]=max(abs(int(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
    }
  }
  black_opening_count = 66;
  black_opening_x = x;
  black_opening_y = y;
  lensclk = 0;
  //black_opening_shape=(black_opening_shape+1)%bosMAX;

  if(wait)
  {
    for(int i=0; i<66; i++)
    {
      if (FrameSkip)
      {
        logic_counter=0;
        while(logic_counter==0)
        {
        }
      }
      drawit=true;
      draw_screen(tmpscr, 0, 0);
      putsubscr(framebuf,0,0);
//      blit(scrollbuf,framebuf,0,0,0,56,256,168);
      syskeys();
      advanceframe();
      if(Quit)
      {
        break;
      }
    }
  }
  logic_counter=0;
  drawit=true;
}

void open_black_opening(int x, int y, bool wait)
{
  int w=256, h=224;
  int blockrows=28, blockcolumns=32;
  int xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
  for (int blockrow=0; blockrow<blockrows; ++blockrow) //30
  {
    for (int blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn) //40
    {
      screen_triangles[blockrow][blockcolumn]=max(abs(int(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
    }
  }
  black_opening_count = -66;
  black_opening_x = x;
  black_opening_y = y;
  lensclk = 0;

  if(wait)
  {
    for(int i=0; i<66; i++)
    {
      if (FrameSkip)
      {
        logic_counter=0;
        while(logic_counter==0)
        {
        }
      }
      drawit=true;
      draw_screen(tmpscr, 0, 0);
      putsubscr(framebuf,0,0);
      syskeys();
      advanceframe();
      if(Quit)
      {
        break;
      }
    }
  }
  logic_counter=0;
  drawit=true;
}

void black_opening(BITMAP *dest,int x,int y,int a,int max_a)
{
  clear_to_color(tmp_scr,BLACK);
  int w=256, h=224;
  switch (black_opening_shape)
  {
    case bosOVAL:
    {
      double new_w=(w/2)+abs(w/2-x);
      double new_h=(h/2)+abs(h/2-y);
      double b=sqrt(((new_w*new_w)/4)+(new_h*new_h));
      ellipsefill(tmp_scr,x,y,int(2*a*b/max_a),int(a*b/max_a),0);
      break;
    }
    case bosTRIANGLE:
    {
      double new_w=(w/2)+abs(w/2-x);
      double new_h=(h/2)+abs(h/2-y);
      double r=a*(new_w*sqrt(3)+new_h)/max_a;
      double P2= (PI/2);
      double P23=(2*PI/3);
      double P43=(4*PI/3);
      double Pa= (-4*PI*a/(3*max_a));
      double angle=P2+Pa;
      double a0=angle;
      double a2=angle+P23;
      double a4=angle+P43;
      triangle(tmp_scr, x+int(cos(a0)*r), y-int(sin(a0)*r),
                        x+int(cos(a2)*r), y-int(sin(a2)*r),
                        x+int(cos(a4)*r), y-int(sin(a4)*r),
                        0);
      break;
    }
    case bosSMAS:
    {
      int distance=max(abs(w/2-x),abs(h/2-y))/8;
      for (int blockrow=0; blockrow<28; ++blockrow) //30
      {
        for (int linerow=0; linerow<8; ++linerow)
        {
          qword *triangleline=(qword*)(tmp_scr->line[(blockrow*8+linerow)]);
          for (int blockcolumn=0; blockcolumn<32; ++blockcolumn) //40
          {
            //*triangleline=triangles[(screen_triangles[blockrow][blockcolumn]&0xC000)>>14][min(max((((max_a-a)/2)+(screen_triangles[blockrow][blockcolumn]&0x0FFF)-15),0),15)][linerow];
            *triangleline=triangles[(screen_triangles[blockrow][blockcolumn]&0xC000)>>14]
                                   [min(max((((31+distance)*(max_a-a)/max_a)+((screen_triangles[blockrow][blockcolumn]&0x0FFF)-0x0100)-(15+distance)),0),15)]
                                   [linerow];
            ++triangleline;
            if (linerow==0)
            {
            }
          }
        }
      }
      break;
    }
    case bosCIRCLE:
    default:
    {
      double new_w=(w/2)+abs(w/2-x);
      double new_h=(h/2)+abs(h/2-y);
      int r=int(sqrt((new_w*new_w)+(new_h*new_h))*a/max_a);
      //circlefill(tmp_scr,x,y,a<<3,0);
      circlefill(tmp_scr,x,y,r,0);
      break;
    }
  }
  masked_blit(tmp_scr,dest,0,0,0,0,320,240);
}

//----------------------------------------------------------------

bool item_disabled(int item_type, int item)                 //is this item disabled?
{
  return false;
}

bool can_use_item(int item_type, int item)                  //can Link use this item?
{
  if (has_item(item_type, item) && !item_disabled(item_type, item))
  {
    return true;
  }
  return false;
}

bool has_item(int item_type, int it)                        //does Link possess this item?
{
  switch (item_type)
  {
    case itype_bomb:
    case itype_sbomb:
      return (game.items[item_type]>0);
      break;
    case itype_clock:
      return Link.getClock()?1:0;
    case itype_key:
      return (game.keys>0);
    case itype_magiccontainer:
      return (game.maxmagic>=MAGICPERBLOCK);
    case itype_triforcepiece:                               //it: -2=any, -1=current level, other=that level
    {
      switch (it)
      {
        case -2:
        {
          for (int i=0; i<MAXLEVELS; i++)
          {
            if (game.lvlitems[i]|liTRIFORCE)

            {
              return true;
            }
          }
          return false;
          break;
        }
        case -1:
          return (game.lvlitems[dlevel]|liTRIFORCE);
          break;
        default:
          if (it>=0&&it<MAXLEVELS)
          {
            return (game.lvlitems[it]|liTRIFORCE);
          }
          break;
      }
      return 0;
    }
    case itype_map:                                         //it: -2=any, -1=current level, other=that level
    {
      switch (it)
      {
        case -2:
        {
          for (int i=0; i<MAXLEVELS; i++)
          {
            if (game.lvlitems[i]|liMAP)
            {
              return true;
            }
          }
          return false;
        }
        break;
        case -1:
          return (game.lvlitems[dlevel]|liMAP);
          break;
        default:
          if (it>=0&&it<MAXLEVELS)
          {
            return (game.lvlitems[it]|liMAP);
          }
          break;
      }
      return 0;
    }
    case itype_compass:                                     //it: -2=any, -1=current level, other=that level
    {
      switch (it)
      {
        case -2:
        {
          for (int i=0; i<MAXLEVELS; i++)
          {
            if (game.lvlitems[i]|liCOMPASS)
            {
              return true;
            }
          }
          return false;
          break;
        }
        case -1:
          return (game.lvlitems[dlevel]|liCOMPASS);
          break;
        default:
          if (it>=0&&it<MAXLEVELS)
          {
            return (game.lvlitems[it]|liCOMPASS);
          }
          break;
      }
      return 0;
    }
    case itype_bosskey:                                     //it: -2=any, -1=current level, other=that level
    {
      switch (it)
      {
        case -2:
        {
          for (int i=0; i<MAXLEVELS; i++)
          {
            if (game.lvlitems[i]|liBOSSKEY)
            {
              return true;
            }
          }
          return false;
          break;
        }
        case -1:
          return (game.lvlitems[dlevel]|liBOSSKEY)?1:0;
          break;
        default:
          if (it>=0&&it<MAXLEVELS)
          {
            return (game.lvlitems[it]|liBOSSKEY)?1:0;
          }
          break;
      }
      return 0;
    }
    default:
      it=(1<<(it-1));
      if (item_type>=itype_max)
      {
        system_pal();
        jwin_alert("Error","has_item exception",NULL,NULL,"O&K",NULL,'k',0,lfont);
        game_pal();

        return false;
      }
      if (game.items[item_type]&it)
      {
        return true;
      }
      break;
  }
  return false;
}

int high_item(int jmax, int item_type, bool consecutive, int itemcluster, bool usecluster)
{

  if (usecluster)
  {
    for (int j=jmax-1; j>0; j--)
    {
      if (itemcluster&(1<<(j-1)))
      {
        return consecutive?j:(1<<(j-1));
      }
    }
  }
  else
  {
    for (int j=jmax-1; j>0; j--)
    {
      if (can_use_item(item_type, j))

      {
        return consecutive?j:(1<<(j-1));
      }
    }
  }

  return 0;
}

int current_item(int item_type, bool consecutive)           //item currently being used
{
  int jmax=0;
  switch(item_type)
  {
    case itype_sword:
      jmax=imax_sword;
      break;
    case itype_brang:
      jmax=imax_brang;
      break;
    case itype_arrow:
      jmax=imax_arrow;
      break;
    case itype_candle:
      jmax=imax_candle;
      break;
    case itype_whistle:
      jmax=imax_whistle;
      break;
    case itype_bait:
      jmax=imax_bait;
      break;
    case itype_letter:
      jmax=imax_letter;
      break;
    case itype_potion:
      jmax=imax_potion;
      break;
    case itype_wand:
      jmax=imax_wand;
      break;
    case itype_ring:
      jmax=imax_ring;
      break;
    case itype_wallet:
      jmax=imax_wallet;
      break;
    case itype_amulet:
      jmax=imax_amulet;
      break;
    case itype_shield:
      jmax=imax_shield;
      break;
    case itype_bow:
      jmax=imax_bow;
      break;
    case itype_raft:
      jmax=imax_raft;
      break;
    case itype_ladder:
      jmax=imax_ladder;
      break;
    case itype_book:
      jmax=imax_book;
      break;
    case itype_magickey:
      jmax=imax_magickey;
      break;
    case itype_bracelet:
      jmax=imax_bracelet;
      break;
    case itype_flippers:
      jmax=imax_flippers;
      break;
    case itype_boots:
      jmax=imax_boots;
      break;
    case itype_hookshot:
      jmax=imax_hookshot;
      break;
    case itype_lens:
      jmax=imax_lens;
      break;
    case itype_hammer:
      jmax=imax_hammer;
      break;
    case itype_dinsfire:
      jmax=imax_dinsfire;
      break;
    case itype_faroreswind:
      jmax=imax_faroreswind;
      break;
    case itype_nayruslove:
      jmax=imax_nayruslove;
      break;
    case itype_bomb:
    case itype_sbomb:
      return can_use_item(item_type,1) ? game.items[item_type] : 0;
      break;
    case itype_clock:
      return has_item(itype_clock,1) ? 1 : 0;
      break;
    case itype_key:
      return game.keys;
    case itype_magiccontainer:
      return game.maxmagic/MAGICPERBLOCK;
    case itype_triforcepiece:
    {
      int count=0;
      for (int i=0; i<MAXLEVELS; i++)
      {
        count+=(game.lvlitems[i]|liTRIFORCE)?1:0;
      }
      return 0;
      break;
    }
    case itype_map:
    {
      int count=0;
      for (int i=0; i<MAXLEVELS; i++)
      {
        count+=(game.lvlitems[i]|liMAP)?1:0;
      }
      return count;
      break;
    }
    case itype_compass:
    {
      int count=0;
      for (int i=0; i<MAXLEVELS; i++)
      {
        count+=(game.lvlitems[i]|liCOMPASS)?1:0;
      }
      return count;
      break;
    }
    case itype_bosskey:
    {
      int count=0;
      for (int i=0; i<MAXLEVELS; i++)
      {
        count+=(game.lvlitems[i]|liBOSSKEY)?1:0;
      }
      return count;
      break;
    }
    default:
      return 0;
      break;
  }
  return high_item(jmax, item_type, consecutive, 0, false);
}

int item_tile_mod()
{
  long tile=0;
  int ret=0;
  ret=current_item(itype_sword, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iSword].ltm; break;
    case 2: ret=itemsbuf[iWSword].ltm; break;
    case 3: ret=itemsbuf[iMSword].ltm; break;
    case 4: ret=itemsbuf[iXSword].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_brang, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBrang].ltm; break;
    case 2: ret=itemsbuf[iMBrang].ltm; break;
    case 3: ret=itemsbuf[iFBrang].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_arrow, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iArrow].ltm; break;
    case 2: ret=itemsbuf[iSArrow].ltm; break;
    case 3: ret=itemsbuf[iGArrow].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_candle, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBCandle].ltm; break;
    case 2: ret=itemsbuf[iRCandle].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_whistle, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iWhistle].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;
  ret=current_item(itype_bait, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBait].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_letter, true);
  switch(ret)
  {
    case 1:
    case 2: ret=itemsbuf[iLetter].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_potion, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBPotion].ltm; break;
    case 2: ret=itemsbuf[iRPotion].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_wand, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iWand].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_ring, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBRing].ltm; break;
    case 2: ret=itemsbuf[iRRing].ltm; break;
    case 3: ret=itemsbuf[iGRing].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_wallet, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iWallet500].ltm; break;
    case 2: ret=itemsbuf[iWallet999].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;
  ret=current_item(itype_amulet, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iAmulet].ltm; break;
    case 2: ret=itemsbuf[iL2Amulet].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_shield, true);
  switch(ret)
  {
    case 1: ret=0; break;
    case 2: ret=itemsbuf[iShield].ltm; break;
    case 3: ret=itemsbuf[iMShield].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_bow, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBow].ltm; break;
    case 2: ret=itemsbuf[iBow2].ltm; break;
    default: ret=0; break;

  }
  tile+=ret;

  ret=current_item(itype_raft, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iRaft].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_ladder, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iLadder].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_book, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBook].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;
  ret=current_item(itype_magickey, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iMKey].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_bracelet, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBracelet].ltm; break;
    case 2: ret=itemsbuf[iL2Bracelet].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_flippers, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iFlippers].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_boots, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iBoots].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_hookshot, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iHookshot].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_lens, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iLens].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;
  ret=current_item(itype_hammer, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iHammer].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_dinsfire, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iDinsFire].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_faroreswind, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iFaroresWind].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_nayruslove, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iNayrusLove].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_bomb, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iBombs].ltm; break;
  }
  tile+=ret;

  ret=current_item(itype_sbomb, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iSBomb].ltm; break;
  }
  tile+=ret;

  ret=current_item(itype_clock, true);
  switch(ret)
  {
    case 1: ret=itemsbuf[iClock].ltm; break;
    default: ret=0; break;
  }
  tile+=ret;

  ret=current_item(itype_key, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iKey].ltm; break;
  }
  tile+=ret;

  ret=current_item(itype_map, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iMap].ltm; break;
  }
  tile+=ret;

  ret=current_item(itype_compass, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iCompass].ltm; break;
  }
  tile+=ret;

  ret=current_item(itype_bosskey, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iBossKey].ltm; break;
  }
  tile+=ret;

  ret=current_item(itype_magiccontainer, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iMagicC].ltm; break;
  }
  tile+=ret;

  ret=current_item(itype_triforcepiece, true);
  switch(ret)
  {
    case 0: ret=0; break;
    default: ret=itemsbuf[iTriforce].ltm; break;
  }
  tile+=ret;
  return tile;
}

int dmap_tile_mod()
{
  return 0;
}

void draw_lens_under()
{
  int strike_hint_table[11]=
  {
    mfARROW, mfBOMB, mfBRANG, mfWANDMAGIC,
    mfSWORD, mfREFMAGIC, mfHOOKSHOT,
    mfREFFIREBALL, mfHAMMER, mfSWORDBEAM, mfWAND
  };

  //  int page = tmpscr->cpage;
  {
    int blink_rate=1;
    //    int temptimer=0;
    int tempitem, tempweapon;
    strike_hint=strike_hint_table[strike_hint_counter];
    if (strike_hint_timer>32)
    {
      strike_hint_timer=0;
      strike_hint_counter=((strike_hint_counter+1)%11);
    }
    ++strike_hint_timer;

    for(int i=0; i<176; i++)
    {
      int x = (i & 15) << 4;
      int y = (i & 0xF0) + 56;
      int tempitemx=-16, tempitemy=-16;
      int tempweaponx=-16, tempweapony=-16;

      switch(tmpscr->sflag[i])
      {
        case 0:
        case mfZELDA:
        case mfPUSHED:
        case mfENEMY0:
        case mfENEMY1:
        case mfENEMY2:
        case mfENEMY3:
        case mfENEMY4:
        case mfENEMY5:
        case mfENEMY6:

        case mfENEMY7:
        case mfENEMY8:
        case mfENEMY9:
          break;

        case mfPUSHUD:
        case mfPUSHLR:
        case mfPUSH4:
        case mfPUSHU:
        case mfPUSHD:
        case mfPUSHL:
        case mfPUSHR:
        case mfPUSHUDNS:
        case mfPUSHLRNS:
        case mfPUSH4NS:
        case mfPUSHUNS:
        case mfPUSHDNS:
        case mfPUSHLNS:
        case mfPUSHRNS:
        case mfPUSHUDINS:
        case mfPUSHLRINS:
        case mfPUSH4INS:

        case mfPUSHUINS:
        case mfPUSHDINS:
        case mfPUSHLINS:
        case mfPUSHRINS:
          if( (!(debug && key[KEY_L]) && (lensclk&16))
            || ((debug && key[KEY_L]) && (frame&16)) )
          {
            putcombo(framebuf,x,y,tmpscr->undercombo,tmpscr->undercset);
          }
          if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
            || ((debug && key[KEY_L]) && (frame&blink_rate)) )
          {
            if (get_bit(quest_rules,qr_LENSHINTS))
            {
              switch (combobuf[tmpscr->data[i]].type)
              {
                case cPUSH_HEAVY:
                case cPUSH_HW:
                  tempitem=iBracelet;
                  tempitemx=x, tempitemy=y;
                  putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                  break;
                case cPUSH_HEAVY2:
                case cPUSH_HW2:
                  tempitem=iL2Bracelet;
                  tempitemx=x, tempitemy=y;
                  putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                  break;
              }
            }
          }
          break;

        case mfWHISTLE:
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iWhistle;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfFAIRY:
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iFairyMoving;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfBCANDLE:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sBCANDLE],tmpscr->secretcset[sBCANDLE]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iBCandle;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfRCANDLE:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sRCANDLE],tmpscr->secretcset[sRCANDLE]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iRCandle;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfWANDFIRE:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sWANDFIRE],tmpscr->secretcset[sWANDFIRE]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iWand;
            tempweapon=wFire;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            else
            {
              tempweaponx=x; tempweapony=y;
            }
            putweapon(framebuf,tempweaponx,tempweapony,tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfDINSFIRE:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sDINSFIRE],tmpscr->secretcset[sDINSFIRE]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iDinsFire;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfARROW:
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            putcombo(framebuf,x,y,tmpscr->secretcombo[sARROW],tmpscr->secretcset[sARROW]);
            tempitem=iArrow;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfSARROW:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sSARROW],tmpscr->secretcset[sSARROW]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iSArrow;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfGARROW:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sGARROW],tmpscr->secretcset[sGARROW]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iGArrow;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfBOMB:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sBOMB],tmpscr->secretcset[sBOMB]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempweapon=wBomb;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempweaponx=x; tempweapony=y;
            }
            putweapon(framebuf,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
          }
          break;

        case mfSBOMB:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sSBOMB],tmpscr->secretcset[sSBOMB]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempweapon=wSBomb;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempweaponx=x; tempweapony=y;
            }
            putweapon(framebuf,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
          }
          break;

        case mfARMOS_SECRET:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sSTAIRS],tmpscr->secretcset[sSTAIRS]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
          }
          break;

        case mfBRANG:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sBRANG],tmpscr->secretcset[sBRANG]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iBrang;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfMBRANG:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sMBRANG],tmpscr->secretcset[sMBRANG]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iMBrang;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfFBRANG:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sFBRANG],tmpscr->secretcset[sFBRANG]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iFBrang;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfWANDMAGIC:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sWANDMAGIC],tmpscr->secretcset[sWANDMAGIC]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iWand;
            tempweapon=wMagic;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            else
            {
              tempweaponx=x; tempweapony=y;
              --lens_hint_weapon[wMagic][4];
              if (lens_hint_weapon[wMagic][4]<-8)
              {
                lens_hint_weapon[wMagic][4]=8;
              }
            }
            putweapon(framebuf,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfREFMAGIC:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sREFMAGIC],tmpscr->secretcset[sREFMAGIC]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iMShield;
            tempweapon=ewMagic;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            else
            {
              tempweaponx=x; tempweapony=y;
              if (lens_hint_weapon[ewMagic][2]==up)
              {
                --lens_hint_weapon[ewMagic][4];
              }
              else
              {
                ++lens_hint_weapon[ewMagic][4];
              }
              if (lens_hint_weapon[ewMagic][4]>8)
              {
                lens_hint_weapon[ewMagic][2]=up;
              }
              if (lens_hint_weapon[ewMagic][4]<=0)
              {
                lens_hint_weapon[ewMagic][2]=down;
              }
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
            putweapon(framebuf,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, lens_hint_weapon[ewMagic][2], lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
          }
          break;

        case mfREFFIREBALL:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sREFFIREBALL],tmpscr->secretcset[sREFFIREBALL]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iMShield;
            tempweapon=ewFireball;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
              tempweaponx=x; tempweapony=y;
              ++lens_hint_weapon[ewFireball][3];
              if (lens_hint_weapon[ewFireball][3]>8)
              {
                lens_hint_weapon[ewFireball][3]=-8;
                lens_hint_weapon[ewFireball][4]=8;
              }
              if (lens_hint_weapon[ewFireball][3]>0)
              {
                ++lens_hint_weapon[ewFireball][4];
              }
              else
              {
                --lens_hint_weapon[ewFireball][4];
              }
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
            putweapon(framebuf,tempweaponx+lens_hint_weapon[tempweapon][3],tempweapony+lens_hint_weapon[ewFireball][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
          }
          break;

        case mfSWORD:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sSWORD],tmpscr->secretcset[sSWORD]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfWSWORD:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sWSWORD],tmpscr->secretcset[sWSWORD]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iWSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfMSWORD:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sMSWORD],tmpscr->secretcset[sMSWORD]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iMSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfXSWORD:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sXSWORD],tmpscr->secretcset[sXSWORD]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iXSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfSWORDBEAM:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sSWORDBEAM],tmpscr->secretcset[sSWORDBEAM]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 1);
          }
          break;

        case mfWSWORDBEAM:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sWSWORDBEAM],tmpscr->secretcset[sWSWORDBEAM]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iWSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 2);
          }
          break;

        case mfMSWORDBEAM:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sMSWORDBEAM],tmpscr->secretcset[sMSWORDBEAM]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iMSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 3);
          }
          break;

        case mfXSWORDBEAM:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sXSWORDBEAM],tmpscr->secretcset[sXSWORDBEAM]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iXSword;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 4);
          }
          break;

        case mfHOOKSHOT:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sHOOKSHOT],tmpscr->secretcset[sHOOKSHOT]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iHookshot;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfWAND:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sWAND],tmpscr->secretcset[sWAND]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iWand;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfHAMMER:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sHAMMER],tmpscr->secretcset[sHAMMER]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            tempitem=iHammer;
            if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
              || ((debug && key[KEY_L]) && (frame&blink_rate)) )
            {
              tempitemx=x; tempitemy=y;
            }
            putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
          }
          break;

        case mfSTRIKE:
          putcombo(framebuf,x,y,tmpscr->secretcombo[sSTRIKE],tmpscr->secretcset[sSTRIKE]);
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
            switch (strike_hint)
            {
              case mfARROW:
                tempitem=iArrow;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                break;
              case mfBOMB:
                tempweapon=wBomb;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempweaponx=x; tempweapony=y;
                }
                putweapon(framebuf,tempweaponx,tempweapony+lens_hint_weapon[tempweapon][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
                break;
              case mfBRANG:
                tempitem=iBrang;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                break;
              case mfWANDMAGIC:
                tempitem=iWand;
                tempweapon=wMagic;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                else
                {
                  tempweaponx=x; tempweapony=y;
                  --lens_hint_weapon[wMagic][4];
                  if (lens_hint_weapon[wMagic][4]<-8)
                  {
                    lens_hint_weapon[wMagic][4]=8;
                  }
                }
                putweapon(framebuf,tempweaponx,tempweapony+lens_hint_weapon[wMagic][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                break;
              case mfREFMAGIC:
                tempitem=iMShield;
                tempweapon=ewMagic;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                else
                {
                  tempweaponx=x; tempweapony=y;
                  if (lens_hint_weapon[ewMagic][2]==up)
                  {
                    --lens_hint_weapon[ewMagic][4];
                  }
                  else
                  {
                    ++lens_hint_weapon[ewMagic][4];
                  }
                  if (lens_hint_weapon[ewMagic][4]>8)
                  {
                    lens_hint_weapon[ewMagic][2]=up;
                  }
                  if (lens_hint_weapon[ewMagic][4]<=0)
                  {
                    lens_hint_weapon[ewMagic][2]=down;
                  }
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                putweapon(framebuf,tempweaponx,tempweapony+lens_hint_weapon[ewMagic][4],tempweapon, 0, lens_hint_weapon[ewMagic][2], lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
                break;
              case mfREFFIREBALL:
                tempitem=iMShield;
                tempweapon=ewFireball;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                  tempweaponx=x; tempweapony=y;
                  ++lens_hint_weapon[ewFireball][3];
                  if (lens_hint_weapon[ewFireball][3]>8)
                  {
                    lens_hint_weapon[ewFireball][3]=-8;
                    lens_hint_weapon[ewFireball][4]=8;
                  }
                  if (lens_hint_weapon[ewFireball][3]>0)
                  {
                    ++lens_hint_weapon[ewFireball][4];
                  }
                  else
                  {
                    --lens_hint_weapon[ewFireball][4];
                  }
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                putweapon(framebuf,tempweaponx+lens_hint_weapon[ewFireball][3],tempweapony+lens_hint_weapon[ewFireball][4],tempweapon, 0, up, lens_hint_weapon[tempweapon][0], lens_hint_weapon[tempweapon][1]);
                break;
              case mfSWORD:
                tempitem=iSword;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                break;
              case mfSWORDBEAM:
                tempitem=iSword;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 1);
                break;
              case mfHOOKSHOT:
                tempitem=iHookshot;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                break;
              case mfWAND:
                tempitem=iWand;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                break;
              case mfHAMMER:
                tempitem=iHammer;
                if( (!(debug && key[KEY_L]) && (lensclk&blink_rate))
                  || ((debug && key[KEY_L]) && (frame&blink_rate)) )
                {
                  tempitemx=x; tempitemy=y;
                }
                putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
                break;
            }
          }
          break;
        case mfARMOS_ITEM:
        case mfDIVE_ITEM:
          if (get_bit(quest_rules,qr_LENSHINTS))
          {
          }
          if(!getmapflag())
                                                            //          putitem2(framebuf,x,y,tmpscr->catchall);
            putitem2(framebuf,x,y,tmpscr->catchall, lens_hint_item[tmpscr->catchall][0], lens_hint_item[tmpscr->catchall][1], 0);
          break;

        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
          putcombo(framebuf,x,y,tmpscr->secretcombo[(tmpscr->sflag[i])-16+4],
            tmpscr->secretcset[(tmpscr->sflag[i])-16+4]);
          break;

        default:
          if( (!(debug && key[KEY_L]) && (lensclk&1))
            || ((debug && key[KEY_L]) && (frame&1)) )
          {
            rectfill(framebuf,x,y,x+15,y+15,WHITE);
          }
          break;
      }
    }

    if(tmpscr->door[0]==dWALK)
      rectfill(framebuf, 120, 16+56, 135, 31+56, WHITE);

    if(tmpscr->door[1]==dWALK)
      rectfill(framebuf, 120, 144+56, 135, 159+56, WHITE);

    if(tmpscr->door[2]==dWALK)
      rectfill(framebuf, 16, 80+56, 31, 95+56, WHITE);

    if(tmpscr->door[3]==dWALK)
      rectfill(framebuf, 224, 80+56, 239, 95+56, WHITE);

    if(tmpscr->door[0]==dBOMB)
    {
      showbombeddoor(0);
    }

    if(tmpscr->door[1]==dBOMB)
    {
      showbombeddoor(1);
    }

    if(tmpscr->door[2]==dBOMB)
    {
      showbombeddoor(2);
    }

    if(tmpscr->door[3]==dBOMB)
    {
      showbombeddoor(3);
    }

    if(tmpscr->stairx + tmpscr->stairy)
    {
      putcombo(framebuf,tmpscr->stairx,tmpscr->stairy+56,tmpscr->secretcombo[sSTAIRS],tmpscr->secretcset[sSTAIRS]);
      if (get_bit(quest_rules,qr_LENSHINTS))
      {
        if(tmpscr->flags&fWHISTLE)
        {
          tempitem=iWhistle;
          int tempitemx=-16; int tempitemy=-16;
          if( (!(debug && key[KEY_L]) && (lensclk&(blink_rate/4)))
            || ((debug && key[KEY_L]) && (frame&(blink_rate/4))) )
          {
            tempitemx=tmpscr->stairx; tempitemy=tmpscr->stairy+56;
          }
          putitem2(framebuf,tempitemx,tempitemy,tempitem, lens_hint_item[tempitem][0], lens_hint_item[tempitem][1], 0);
        }
      }
    }
  }
}

void draw_lens_over()
{
  clear_to_color(tmp_scr, BLACK);
  circlefill(tmp_scr, LinkX()+8, LinkY()+8+56, 60, 0);
  circle(tmp_scr, LinkX()+8, LinkY()+8+56, 62, 0);
  circle(tmp_scr, LinkX()+8, LinkY()+8+56, 65, 0);
  masked_blit(tmp_scr, framebuf, 0, 56, 0, 56, 256, 168);
}

                                                            //----------------------------------------------------------------

void draw_wavy(int amplitude)
{
  BITMAP *wavebuf = create_bitmap_ex(8,288,224);
  clear_to_color(wavebuf,0);
  blit(framebuf,wavebuf,0,0,16,0,256,224);

  int ofs;
                                                            //  int amplitude=8;
                                                            //  int wavelength=4;
  int amp2=168;
  int i=frame%amp2;
  for(int j=0; j<168; j++)
  {
    ofs=0;
    if (j&1)
    {
      ofs=int(sin((double(i+j)*2*PI/amp2))*amplitude);
    }
    else
    {
      ofs-=int(sin((double(i+j)*2*PI/amp2))*amplitude);
    }
    for (int k=0; k<256; k++)
    {
      framebuf->line[j+56][k]=wavebuf->line[j+56][k+ofs+16];
    }
  }
  destroy_bitmap(wavebuf);
}

void draw_fuzzy(int fuzz)
                                                            // draws from right half of scrollbuf to framebuf
{
  int firstx, firsty, xstep, ystep, i, y, dx, dy;
  byte *start, *si, *di;

  if(fuzz<1)
    fuzz = 1;

  xstep = 128%fuzz;
  if(xstep > 0)
    xstep = fuzz-xstep;

  ystep = 112%fuzz;
  if(ystep > 0)
    ystep = fuzz-ystep;

  firsty = 1;

  for(y=0; y<224; )
  {
    start = &(scrollbuf->line[y][256]);

    for(dy=0; dy<ystep && dy+y<224; dy++)
    {
      si = start;
      di = &(framebuf->line[y+dy][0]);
      i = xstep;
      firstx = 1;

      for(dx=0; dx<256; dx++)
      {
        *(di++) = *si;
        if(++i >= fuzz)
        {
          if(!firstx)
            si += fuzz;
          else
          {
            si += fuzz-xstep;
            firstx = 0;
          }
          i = 0;
        }
      }
    }

    if(!firsty)
      y += fuzz;
    else
    {
      y += ystep;
      ystep = fuzz;
      firsty = 0;
    }
  }
}

void waitvsync()
{
  if((Throttlefps ^ (true && key[KEY_TILDE])))
  {
    while(!myvsync) rest(1);
    //    vsync();
  }
  myvsync=0;
}

void updatescr()
{
	//rest(1);
	if (halt)
	{
		char buf[20];
		int num=0;
		do
		{
			sprintf(buf, "zelda%03d.bmp", ++num);
		} while(num<999 && exists(buf));

		PALETTE tpal;
		get_palette(tpal);
		if (tmpscr->flags3&fNOSUBSCR)
		{
			BITMAP *ssbuf = create_bitmap_ex(8,256,168);
			clear_to_color(ssbuf,0);
			blit(framebuf,ssbuf,0,56,0,0,256,168);
			save_bitmap(buf,ssbuf,tpal);
			destroy_bitmap(ssbuf);
		}
		else
		{
			save_bitmap(buf,framebuf,tpal);
		}
	}

	if(toogam)
	{
		textout_ex(framebuf,font,"no walls",8,216,1,-1);
	}

	if(Showpal)
		dump_pal(framebuf);

	if(!Playing)
		black_opening_count=0;

	if(black_opening_count<0) //shape is opening up
	{
		black_opening(framebuf,black_opening_x,black_opening_y,(66+black_opening_count),66);
		if (Advance||(!Paused))
		{
			++black_opening_count;
		}
	}
	else if(black_opening_count>0) //shape is closing
	{
		black_opening(framebuf,black_opening_x,black_opening_y,black_opening_count,66);
		if (Advance||(!Paused))
		{
			--black_opening_count;
		}
	}

	//redo this and waitvsync(), too

	if (triplebuffer_not_available)
	{
		waitvsync();
	}
	else
	{
		//    myvsync=0;
	}


	if(refreshpal)
	{
		refreshpal=false;
		RAMpal[253] = _RGB(0,0,0);
		RAMpal[254] = _RGB(63,63,63);
		set_palette_range(RAMpal,0,255,false);
	}

	show_details();

	if (Link.DrunkClock())
	{
		draw_wavy(Link.DrunkClock()/(MAXDRUNKCLOCK/32));
	}
	BITMAP *panorama = NULL;

	bool nosubscr = (tmpscr->flags3&fNOSUBSCR) != 0;

	if (nosubscr)
	{
		panorama = create_bitmap_ex(8,256,224);
		rectfill(panorama,0,0,255,56/2,0);
		rectfill(panorama,0,168+56/2,255,168+56-1,0);
		blit(framebuf,panorama,0,56,0,56/2,256,224-56);
	}
	BITMAP *target;
	bool dontusetb = triplebuffer_not_available ||
		!(Throttlefps ^ (true && key[KEY_TILDE]));
	if(dontusetb)
		target=screen;
	else
		target=tb_page[curr_tb_page];

	if (scanlines && sbig)
	{
		BITMAP *scanlinesbmp = create_bitmap_ex(8,512,448);
		stretch_blit(nosubscr?panorama:framebuf,scanlinesbmp,0,0,256,224,0,0    ,512,448);
		for (int i=0; i<224; ++i)
		{
			_allegro_hline(scanlinesbmp, 0, i*2+1, 512, BLACK);
		}
		blit(scanlinesbmp,target,0,0,scrx+32-128,scry+8-112,512,448);

		destroy_bitmap(scanlinesbmp);
	}
	else if (sbig)
	{
		//jman2050, what are you doing with this line below?
		//exists((char*)panorama); exists((char*)screen);
		//did you mean
		//assert((char*)panorama); assert((char*)screen);
		BITMAP *tempscreen = create_bitmap_ex(8,512,448);
		clear_bitmap(tempscreen);
		stretch_blit(nosubscr?panorama:framebuf,tempscreen,0,0,256,224,0,0,512,448);
		blit(tempscreen,target,0,0,scrx+32-128,scry+8-112,512,448);
		destroy_bitmap(tempscreen);
	}
	else
	{
		blit(nosubscr?panorama:framebuf,target,0,0,scrx+32,scry+8,256,224);
	}

	if (!dontusetb)
	{
		if(!poll_scroll())
		{
			request_video_bitmap(tb_page[curr_tb_page]);
			curr_tb_page=(curr_tb_page+1)%3;
			clear_to_color(tb_page[curr_tb_page],BLACK);
		}
		waitvsync();
	}


	if(ShowFPS)
		show_fps();


	if(Paused)
		show_paused();

	if(details)
	{
		textprintf_ex(screen,font,0,SCREEN_H-8,254,BLACK,"%-6d (%s)", idle_count, time_str_long(idle_count));
	}

	if(panorama!=NULL) destroy_bitmap(panorama);

	++framecnt;
}

//----------------------------------------------------------------

PALETTE sys_pal;

int onGUISnapshot()
{
  char buf[20];
  int num=0;
  do
  {
    sprintf(buf, "zelda%03d.bmp", ++num);
  } while(num<999 && exists(buf));

  BITMAP *b = create_bitmap_ex(8,resx,resy);
  if(b)
  {
    blit(screen,b,0,0,0,0,resx,resy);
    save_bmp(buf,b,sys_pal);
    destroy_bitmap(b);
  }
  return D_O_K;
}

int onNonGUISnapshot()
{
  char buf[20];
  int num=0;
  do
  {
    sprintf(buf, "zelda%03d.bmp", ++num);
  } while(num<999 && exists(buf));

  BITMAP *panorama = create_bitmap_ex(8,256,168);
  if (tmpscr->flags3&fNOSUBSCR)
  {
    clear_to_color(panorama,0);
    blit(framebuf,panorama,0,56,0,0,256,168);
    save_bitmap(buf,panorama,RAMpal);
  }
  else
  {
    save_bitmap(buf,framebuf,RAMpal);
  }
  destroy_bitmap(panorama);
  return D_O_K;
}

int onSnapshot()
{
  if (key[KEY_LSHIFT]||key[KEY_RSHIFT])
  {
    onGUISnapshot();
  }
  else
  {
    onNonGUISnapshot();
  }
  return D_O_K;
}

int onSaveMapPic()
{
  BITMAP* mappic = NULL;
  int mapres = 0;
  char buf[20];
  int num=0;
  do
  {
    sprintf(buf, "zelda%03d.bmp", ++num);
  } while(num<999 && exists(buf));

                                                            //  if(!mappic) {
  mappic = create_bitmap_ex(8,(256*16)>>mapres,(176*8)>>mapres);

  if(!mappic)
  {
    system_pal();
    jwin_alert("Save Map Picture","Not enough memory.",NULL,NULL,"OK",NULL,13,27,lfont);
    game_pal();
    return D_O_K;
  }
                                                            //  }

  int layermap, layerscreen;
  int x2=256, y2=-56;

                                                            // draw the map
  for(int y=0; y<8; y++)
  {
    for(int x=0; x<16; x++)
    {
      int s = (y<<4) + x;

      if(!(game.maps[(currmap<<7)+s]&mVISITED))
      {
        rectfill(scrollbuf, 256, 0, 511, 223, WHITE);
      }
      else
      {
        loadscr(1,s,-1);
        putscr(scrollbuf, 256, 0, tmpscr+1);

        for (int k=0; k<4; k++)
        {
          layermap=TheMaps[currmap*MAPSCRS+s].layermap[k]-1;
          if (layermap>-1)
          {
            layerscreen=layermap*MAPSCRS+TheMaps[currmap*MAPSCRS+s].layerscreen[k];
            if (TheMaps[currmap*MAPSCRS+s].layeropacity[k]==255)
            {
              for (int i=0; i<176; i++)
              {
                overcombo(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
              }
            }
            else
            {
              for (int i=0; i<176; i++)
              {
                overcombotranslucent(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],TheMaps[currmap*MAPSCRS+s].layeropacity[k]);
              }
            }
          }
        }

        for (int i=0; i<176; i++)
        {
          if (COMBOTYPE((i&15)<<4,i&0xF0)==cOVERHEAD)
          {
            overcombo(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,MAPDATA((i&15)<<4,i&0xF0),MAPCSET((i&15)<<4,i&0xF0));
          }
        }

        for (int k=4; k<6; k++)
        {
          layermap=TheMaps[currmap*MAPSCRS+s].layermap[k]-1;
          if (layermap>-1)
          {
            layerscreen=layermap*MAPSCRS+TheMaps[currmap*MAPSCRS+s].layerscreen[k];
            if (TheMaps[currmap*MAPSCRS+s].layeropacity[k]==255)
            {
              for (int i=0; i<176; i++)
              {
                overcombo(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
              }
            }
            else
            {
              for (int i=0; i<176; i++)
              {
                overcombotranslucent(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],TheMaps[currmap*MAPSCRS+s].layeropacity[k]);
              }
            }
          }
        }
      }
      stretch_blit(scrollbuf, mappic, 256, 0, 256, 176,
        x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
    }

  }

  save_bitmap(buf,mappic,RAMpal);
  destroy_bitmap(mappic);
  return D_O_K;
}

void f_Quit(int type)
{
  if(type==qQUIT && !Playing)
    return;

  music_pause();
  pause_all_sfx();
  system_pal();

  switch(type)
  {
    case qQUIT:  onQuit(); break;
    case qRESET: onReset(); break;
    case qEXIT:  onExit(); break;
  }
  if(Quit)
  {
    kill_sfx();
    music_stop();
    clear_to_color(screen,BLACK);
  }
  else
  {
    game_pal();
    music_resume();
    resume_all_sfx();
  }

  eat_buttons();
  if(key[KEY_ESC])
    key[KEY_ESC]=0;
}

//----------------------------------------------------------------

int onNoWalls()
{
  toogam = !toogam;
  if(toogam)
  {
    cheat_superman=true;
    setClock(true);
  }
  return D_O_K;
}

int input_idle(bool checkmouse)
{
  static int mx, my, mb;

  if( keypressed() || Up() || Down() || Left() || Right() || cAbtn()
    || cBbtn() || cSbtn() || cMbtn() || cLbtn() || cRbtn() || cPbtn()
    || (checkmouse && (mx != gui_mouse_x() || my != gui_mouse_y() || mb != gui_mouse_b())) )
  {
    idle_count = 0;
  }
  else if(idle_count < MAX_IDLE)
  {
    ++idle_count;
  }

  mx = gui_mouse_x();
  my = gui_mouse_y();
  mb = gui_mouse_b();

  return idle_count;
}

int onGoFast()
{
  gofast=gofast?false:true;
  return D_O_K;
}

int onGoTo();

void syskeys()
{
  if (close_button_quit)
  {
    close_button_quit=false;
    f_Quit(qEXIT);
  }
  poll_joystick();

  if(rMbtn() || (gui_mouse_b() && !mouse_down))
    System();

  mouse_down=gui_mouse_b();

  if(ReadKey(KEY_F1))
  {
    if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
      halt=!halt;
      //zinit.subscreen=(zinit.subscreen+1)%ssdtMAX;
    }
    else
    {
      Throttlefps=!Throttlefps;
      logic_counter=0;
    }
  }
                                                            //  if(ReadKey(KEY_F1))    Vsync=!Vsync;
                                                            /*
    if(ReadKey(KEY_F1))    set_bit(QHeader.rules4,qr4_NEWENEMYTILES,
                           1-((get_bit(QHeader.rules4,qr4_NEWENEMYTILES))));
  */

  if(ReadKey(KEY_F2))    ShowFPS=!ShowFPS;
  if(ReadKey(KEY_F3) && Playing)    Paused=!Paused;
  if(ReadKey(KEY_F4) && Playing)  { Paused=true; Advance=true; }
  if(ReadKey(KEY_F6))    f_Quit(qQUIT);
  if(ReadKey(KEY_F7))    f_Quit(qRESET);
  if(ReadKey(KEY_F8))    f_Quit(qEXIT);
  if(ReadKey(KEY_F5)&&(get_bit(quest_rules,qr_VIEWMAP)))    onSaveMapPic();
  if(ReadKey(KEY_F12))   if (key[KEY_LSHIFT]||key[KEY_RSHIFT])
  {
    onGUISnapshot();
  }
  else
  {
    onSnapshot();
  }

  if(debug_enabled && ReadKey(KEY_TAB))
    debug = !debug;

  if(debug || cheat>=1)
  {
    if(ReadKey(KEY_ASTERISK))   game.life=game.maxlife;
    if(ReadKey(KEY_SLASH_PAD))   game.magic=game.maxmagic;
    if(ReadKey(KEY_R))          game.drupy = 999;
    if(ReadKey(KEY_B))
    {
      onCheatBombs();
      /*
      game.items[itype_bomb] = game.maxbombs;
      if(debug)
      {
        game.items[itype_sbomb] = game.items[itype_bomb]>>2;
      }
      */
    }
  }

  if(debug || cheat>=2)
    if(ReadKey(KEY_I))          { setClock(!getClock()); cheat_superman=getClock(); }

    if(debug || cheat>=4)
  {
    if(ReadKey(KEY_F11))        onNoWalls();
    if(ReadKey(KEY_Q))          onGoFast();
    if(ReadKey(KEY_F))          if(Link.getAction()==freeze) {Link.unfreeze();} else {Link.Freeze();}
                                                            //    if(ReadKey(KEY_G))          onGoTo();
  }

  if(volkeys)
  {
    if(ReadKey(KEY_PGUP)) master_volume(-1,midi_volume+8);
    if(ReadKey(KEY_PGDN)) master_volume(-1,midi_volume==255?248:midi_volume-8);
    if(ReadKey(KEY_HOME)) master_volume(digi_volume+8,-1);
    if(ReadKey(KEY_END))  master_volume(digi_volume==255?248:digi_volume-8,-1);
  }

  if(!debug || !SystemKeys)
    goto bottom;

  if(ReadKey(KEY_D))
  {
    details = !details;
    rectfill(screen,0,0,319,7,BLACK);
    rectfill(screen,0,8,31,239,BLACK);
    rectfill(screen,288,8,319,239,BLACK);
    rectfill(screen,32,232,287,239,BLACK);
  }

  if(ReadKey(KEY_W))   setClock((watch=true));
  if(ReadKey(KEY_V))
  {
    Throttlefps=!Throttlefps;
    /*
      remove_int(fps_callback);
      remove_int(myvsync_callback);

      install_int_ex(fps_callback,SECS_TO_TIMER(1));
      install_int_ex(myvsync_callback,BPS_TO_TIMER(60));
      */
    avgfps=fps_secs=framecnt=0;
    logic_counter = 0;
  }
  if(ReadKey(KEY_P))   Paused=!Paused;
  if(ReadKey(KEY_A)) { Paused=true; Advance=true; }
  if(ReadKey(KEY_G))   db=(db==999)?0:999;

  if(ReadKey(KEY_F10))  Showpal=!Showpal;
  if(ReadKey(KEY_F9))   { Matrix(ss_speed, ss_density, 0); game_pal(); }

  if(ReadKey(KEY_PLUS_PAD))
  {
                                                            //change containers
    if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
                                                            //magic containers
      if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
      {
        game.maxmagic=min(game.maxmagic+MAGICPERBLOCK,MAGICPERBLOCK*8);
      }
      else
      {
        game.maxlife=min(game.maxlife+HP_PER_HEART,HP_PER_HEART*24);
      }
    }
    else
    {
      if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
      {
        game.magic=min(game.magic+1,game.maxmagic);
      }
      else
      {
        game.life=min(game.life+1,game.maxlife);
      }
    }
  }
  if(ReadKey(KEY_MINUS_PAD))
  {
                                                            //change containers
    if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
                                                            //magic containers
      if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
      {
        game.maxmagic=max(game.maxmagic-MAGICPERBLOCK,0);
        game.magic=min(game.maxmagic, game.magic);
                                                            //heart containers
      }
      else
      {
        game.maxlife=max(game.maxlife-HP_PER_HEART,HP_PER_HEART);
        game.life=min(game.maxlife, game.life);
      }
    }
    else
    {
      if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
      {
        game.magic=max(game.magic-1,0);
      }
      else
      {
        game.life=max(game.life-1,0);
      }
    }
  }

  if(ReadKey(KEY_S))   game.items[itype_shield]^=i_largeshield;
  if(ReadKey(KEY_0))   game.items[itype_sword]=0;
  if(ReadKey(KEY_1))   game.items[itype_sword]^=(1<<(i_sword-1));
  if(ReadKey(KEY_2))   game.items[itype_sword]^=(1<<(i_wsword-1));
  if(ReadKey(KEY_3))   game.items[itype_sword]^=(1<<(i_msword-1));
  if(ReadKey(KEY_4))   game.items[itype_sword]^=(1<<(i_xsword-1));
  if(ReadKey(KEY_5)) { game.items[itype_ring]=0; ringcolor(); }
  if(ReadKey(KEY_6)) { game.items[itype_ring]^=(1<<(i_bring-1)); ringcolor(); }
  if(ReadKey(KEY_7)) { game.items[itype_ring]^=(1<<(i_rring-1)); ringcolor(); }
  if(ReadKey(KEY_8)) { game.items[itype_ring]^=(1<<(i_gring-1)); ringcolor(); }

  if(ReadKey(KEY_COMMA))  jukebox(currmidi-1);
  if(ReadKey(KEY_STOP))   jukebox(currmidi+1);

                                                            /*
    if(ReadKey(KEY_TILDE)) {
      wavyout();
      zinit.subscreen=(zinit.subscreen+1)%3;
      wavyin();
    }
  */

  selectAwpn(0);

  bottom:
  if(input_idle(true) > after_time())
  {
    Matrix(ss_speed, ss_density, 0);
    game_pal();
  }

  while(Playing && keypressed())
    readkey();
}

                                                            // 99*360 + 59*60
#define MAXTIME  21405240

void advanceframe()
{
  if((Throttlefps ^ (true && key[KEY_TILDE])))
  {

    while(!logic_counter) rest(1);
    logic_counter=0;
  }
  if (zcmusic!=NULL)
  {
    zcmusic_poll();
  }
  while(Paused && !Advance && !Quit)
  {
    // have to call this, otherwise we'll get an infinite loop
    syskeys();
    // to keep fps constant
    updatescr();
    // to keep music playing
    if (zcmusic!=NULL)
    {
      zcmusic_poll();
    }
  }
  if(Quit)
    return;

  if(Playing && game.time<MAXTIME)
    ++game.time;

  Advance=false;
  ++frame;

  syskeys();
  updatescr();
  //textprintf_ex(screen,font,0,72,254,BLACK,"%d %d", lastentrance, lastentrance_dmap);
  sfx_cleanup();
}

void zapout()
{
                                                            // draw screen on right half of scrollbuf
                                                            /*
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer(framebuf, 0, tmpscr, 0, 0, 2);
    do_layer(framebuf, 1, tmpscr, 0, 0, 2);
    do_layer(framebuf, -2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 3, tmpscr, 0, 0, 2);
    do_layer(framebuf, -1, tmpscr, 0, 0, 2);
    do_layer(framebuf, 4, tmpscr, 0, 0, 2);
    do_layer(framebuf, 5, tmpscr, 0, 0, 2);
  */
  blit(framebuf,scrollbuf,0,0,256,0,256,224);

                                                            // zap out
  for(int i=1; i<=24; i++)
  {
    draw_fuzzy(i);
    syskeys();
    advanceframe();
    if(Quit)
    {
      break;
    }
  }
}

void zapin()
{
                                                            // draw screen on right half of scrollbuf
                                                            /*
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer(framebuf, 0, tmpscr, 0, 0, 2);
    do_layer(framebuf, 1, tmpscr, 0, 0, 2);
    do_layer(framebuf, -2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 3, tmpscr, 0, 0, 2);
    do_layer(framebuf, -1, tmpscr, 0, 0, 2);
    do_layer(framebuf, 4, tmpscr, 0, 0, 2);
    do_layer(framebuf, 5, tmpscr, 0, 0, 2);
  */
  draw_screen(tmpscr, 0, 0);
  putsubscr(framebuf,0,0);
  blit(framebuf,scrollbuf,0,0,256,0,256,224);

                                                            // zap out
  for(int i=24; i>=1; i--)
  {
    draw_fuzzy(i);
    syskeys();
    advanceframe();
    if(Quit)
    {
      break;
    }
  }
}


void wavyout()
{
                                                            /*
    // draw screen on right half of scrollbuf
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer(framebuf, 0, tmpscr, 0, 0, 2);
    do_layer(framebuf, 1, tmpscr, 0, 0, 2);
    do_layer(framebuf, -2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 3, tmpscr, 0, 0, 2);
    do_layer(framebuf, -1, tmpscr, 0, 0, 2);
    do_layer(framebuf, 4, tmpscr, 0, 0, 2);
  do_layer(framebuf, 5, tmpscr, 0, 0, 2);
  */

                                                            /*
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer(framebuf, 0, tmpscr, 0, 0, 2);
    do_layer(framebuf, 1, tmpscr, 0, 0, 2);
    do_layer(framebuf, -2, tmpscr, 0, 0, 2);
    if(lensclk || (debug && key[KEY_L]))
  draw_lens_under();
  if (tmpscr->layermap[2]==0 && tmpscr->layermap[3]==0 &&
  tmpscr->layermap[4]==0 && tmpscr->layermap[5]==0 &&
  !overheadcombos(tmpscr)) {
  masked_blit(msgdisplaybuf,framebuf,0,0,0,56,256,176);
  masked_blit(pricesdisplaybuf,framebuf,0,0,0,56,256,176);
  }
  Link.draw_under(framebuf);
  mblock2.draw(framebuf);
  if (Link.isSwimming()) {
  Link.draw(framebuf);
  }
  if(drawguys) {
  if(get_bit(quest_rules,qr_NOFLICKER) || (frame&1)) {
  guys.draw(framebuf,true);
  chainlinks.draw(framebuf,true);
  Lwpns.draw(framebuf,true);
  Ewpns.draw(framebuf,true);
  items.draw(framebuf,true);
  } else {
  items.draw(framebuf,false);
  chainlinks.draw(framebuf,false);
  Lwpns.draw(framebuf,false);
  guys.draw(framebuf,false);
  Ewpns.draw(framebuf,false);
  }
  guys.draw2(framebuf,true);
  }
  if (!Link.isSwimming()) {
  Link.draw(framebuf);
  }

  // draw likelike over Link
  for(int i=0; i<guys.Count(); i++) {
  if(guys.spr(i)->id == eLIKE) {
  if(((eLikeLike*)guys.spr(i))->haslink) {
  guys.spr(i)->draw(framebuf);
  }
  }
  if(guys.spr(i)->id == eWALLM) {
  if(((eWallM*)guys.spr(i))->haslink) {
  guys.spr(i)->draw(framebuf);
  }
  }
  }

  do_layer(framebuf, 2, tmpscr, 0, 0, 2);
  do_layer(framebuf, 3, tmpscr, 0, 0, 2);
  do_layer(framebuf, -1, tmpscr, 0, 0, 2);
  for(int i=0; i<guys.Count(); i++) {
  if(isflier(guys.spr(i)->id)) {
  guys.spr(i)->draw(framebuf);
  }
  }
  do_layer(framebuf, 4, tmpscr, 0, 0, 2);
  do_layer(framebuf, 5, tmpscr, 0, 0, 2);
  putsubscr(framebuf,0,0);
  */
  draw_screen(tmpscr, 0, 0);
  putsubscr(framebuf,0,0);

  BITMAP *wavebuf = create_bitmap_ex(8,288,224);
  clear_to_color(wavebuf,0);
  blit(framebuf,wavebuf,0,0,16,0,256,224);

  PALETTE wavepal;

  int ofs;
  int amplitude=8;

  int wavelength=4;
  double palpos=0, palstep=4, palstop=126;
  for(int i=0; i<168; i+=wavelength)
  {
    for (int l=0; l<256; l++)
    {
      wavepal[l].r=vbound(int(RAMpal[l].r+((palpos/palstop)*(63-RAMpal[l].r))),0,63);
      wavepal[l].g=vbound(int(RAMpal[l].g+((palpos/palstop)*(63-RAMpal[l].g))),0,63);
      wavepal[l].b=vbound(int(RAMpal[l].b+((palpos/palstop)*(63-RAMpal[l].b))),0,63);
    }
    palpos+=palstep;
    if (palpos>=0)
    {
      set_palette(wavepal);
    }
    else
    {
      set_palette(RAMpal);
    }
    for(int j=0; j<168; j++)
    {
      for (int k=0; k<256; k++)
      {
        ofs=0;
        if ((j<i)&&(j&1))
        {
          ofs=int(sin((double(i+j)*2*PI/168.0))*amplitude);
        }
        framebuf->line[j+56][k]=wavebuf->line[j+56][k+ofs+16];
      }
    }
    syskeys();
    advanceframe();
                                                            //    animate_combos();
    if(Quit)
      break;
  }
  destroy_bitmap(wavebuf);
}

void wavyin()
{
                                                            /*
    // draw screen on right half of scrollbuf
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer(framebuf, 0, tmpscr, 0, 0, 2);
    do_layer(framebuf, 1, tmpscr, 0, 0, 2);
    do_layer(framebuf, -2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 3, tmpscr, 0, 0, 2);
    do_layer(framebuf, -1, tmpscr, 0, 0, 2);
    do_layer(framebuf, 4, tmpscr, 0, 0, 2);
  do_layer(framebuf, 5, tmpscr, 0, 0, 2);
  */

                                                            /*
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer(framebuf, 0, tmpscr, 0, 0, 2);
    do_layer(framebuf, 1, tmpscr, 0, 0, 2);
    do_layer(framebuf, -2, tmpscr, 0, 0, 2);
    if(lensclk || (debug && key[KEY_L]))
  draw_lens_under();
  if (tmpscr->layermap[2]==0 && tmpscr->layermap[3]==0 &&
  tmpscr->layermap[4]==0 && tmpscr->layermap[5]==0 &&
  !overheadcombos(tmpscr)) {
  masked_blit(msgdisplaybuf,framebuf,0,0,0,56,256,176);
  masked_blit(pricesdisplaybuf,framebuf,0,0,0,56,256,176);
  }
  Link.draw_under(framebuf);
  mblock2.draw(framebuf);
  if (Link.isSwimming()) {
  Link.draw(framebuf);
  }
  if(drawguys) {
  if(get_bit(quest_rules,qr_NOFLICKER) || (frame&1)) {
  guys.draw(framebuf,true);
  chainlinks.draw(framebuf,true);
  Lwpns.draw(framebuf,true);
  Ewpns.draw(framebuf,true);
  items.draw(framebuf,true);
  } else {
  items.draw(framebuf,false);
  chainlinks.draw(framebuf,false);
  Lwpns.draw(framebuf,false);
  guys.draw(framebuf,false);
  Ewpns.draw(framebuf,false);
  }
  guys.draw2(framebuf,true);
  }
  if (!Link.isSwimming()) {
  Link.draw(framebuf);
  }

  // draw likelike over Link
  for(int i=0; i<guys.Count(); i++) {
  if(guys.spr(i)->id == eLIKE) {
  if(((eLikeLike*)guys.spr(i))->haslink) {
  guys.spr(i)->draw(framebuf);
  }
  }
  if(guys.spr(i)->id == eWALLM) {
  if(((eWallM*)guys.spr(i))->haslink) {
  guys.spr(i)->draw(framebuf);
  }
  }
  }

  do_layer(framebuf, 2, tmpscr, 0, 0, 2);
  do_layer(framebuf, 3, tmpscr, 0, 0, 2);
  do_layer(framebuf, -1, tmpscr, 0, 0, 2);
  for(int i=0; i<guys.Count(); i++) {
  if(isflier(guys.spr(i)->id)) {
  guys.spr(i)->draw(framebuf);
  }
  }
  do_layer(framebuf, 4, tmpscr, 0, 0, 2);
  do_layer(framebuf, 5, tmpscr, 0, 0, 2);
  putsubscr(framebuf,0,0);
  */
  draw_screen(tmpscr, 0, 0);
  putsubscr(framebuf,0,0);

  BITMAP *wavebuf = create_bitmap_ex(8,288,224);
  clear_to_color(wavebuf,0);
  blit(framebuf,wavebuf,0,0,16,0,256,224);

  PALETTE wavepal;
  loadfullpal();
  loadlvlpal(DMaps[currdmap].color);
  ringcolor();
  refreshpal=false;
  int ofs;
  int amplitude=8;
  int wavelength=4;
  double palpos=168, palstep=4, palstop=126;
  for(int i=0; i<168; i+=wavelength)
  {
    for (int l=0; l<256; l++)
    {
      wavepal[l].r=vbound(int(RAMpal[l].r+((palpos/palstop)*(63-RAMpal[l].r))),0,63);
      wavepal[l].g=vbound(int(RAMpal[l].g+((palpos/palstop)*(63-RAMpal[l].g))),0,63);
      wavepal[l].b=vbound(int(RAMpal[l].b+((palpos/palstop)*(63-RAMpal[l].b))),0,63);
    }
    palpos-=palstep;

    if (palpos>=0)
    {
      set_palette(wavepal);
    }
    else
    {
      set_palette(RAMpal);
    }
    for(int j=0; j<168; j++)
    {
      for (int k=0; k<256; k++)
      {
        ofs=0;
        if ((j<(167-i))&&(j&1))
        {
          ofs=int(sin((double(i+j)*2*PI/168.0))*amplitude);
        }
        framebuf->line[j+56][k]=wavebuf->line[j+56][k+ofs+16];
      }
    }
    syskeys();
    advanceframe();
                                                            //    animate_combos();

    if(Quit)
      break;
  }
  destroy_bitmap(wavebuf);
}

void blackscr(int fcnt,bool showsubscr)
{
  reset_pal_cycling();
  while(fcnt>0)
  {
    clear_bitmap(framebuf);
    if(showsubscr)
    {
      putsubscr(framebuf,0,0);
    }
    syskeys();
    advanceframe();
    if(Quit)
      break;;
    --fcnt;
  }
}

void openscreen()
{
  reset_pal_cycling();
  black_opening_count=0;

  if(COOLSCROLL)
  {
    open_black_opening(LinkX()+8, LinkY()+8+56, true);
    return;
  }
  else
  {
    Link.setDontDraw(true);
    show_subscreen_dmap_dots=false;
    show_subscreen_numbers=false;
    show_subscreen_items=false;
    show_subscreen_life=false;
  }

  int x=128;

  for(int i=0; i<80; i++)
  {
    draw_screen(tmpscr, 0, 0);
    putsubscr(framebuf,0,0);
    x=128-(((i*128/80)/8)*8);
    if (x>0)
    {
      rectfill(framebuf,0,56,x,223,0);
      rectfill(framebuf,256-x,56,255,223,0);
    }
//    x=((80-i)/2)*4;
/*
    --x;
    switch(++c)
    {
      case 5: c=0;
      case 0:
      case 2:
      case 3: --x; break;
    }
*/
    syskeys();
    advanceframe();
    if(Quit)
    {
      break;
    }
  }
  Link.setDontDraw(false);
  show_subscreen_items=true;
  show_subscreen_dmap_dots=true;
}

int TriforceCount()
{
  int c=0;
  for(int i=1; i<=8; i++)
    if(game.lvlitems[i]&liTRIFORCE)
      ++c;
  return c;
}

int onContinue()
{
  return D_CLOSE;
}

int onEsc()
{
  return key[KEY_ESC]?D_CLOSE:D_O_K;
}

int onVsync()
{
  Throttlefps = !Throttlefps;
  return D_O_K;
}

int onFrameSkip()
{
  FrameSkip = !FrameSkip;
  return D_O_K;
}

int onTransLayers()
{
  TransLayers = !TransLayers;
  return D_O_K;
}

int onNESquit()
{
  NESquit = !NESquit;
  return D_O_K;
}

int onVolKeys()
{
  volkeys = !volkeys;
  return D_O_K;
}

int onShowFPS()
{
  ShowFPS = !ShowFPS;
  scare_mouse();

  if(ShowFPS)
    show_fps();
  else if(sbig)
    stretch_blit(fps_undo,screen,0,0,64,16,scrx+40-120,scry+216+96,128,32);
  else
    blit(fps_undo,screen,0,0,scrx+40,scry+216,64,16);
  if(Paused)
    show_paused();

  unscare_mouse();
  return D_O_K;
}

bool is_Fkey(int k)
{
  switch(k)
  {
    case KEY_F1:
    case KEY_F2:
    case KEY_F3:
    case KEY_F4:
    case KEY_F5:
    case KEY_F6:
    case KEY_F7:
    case KEY_F8:
    case KEY_F9:
    case KEY_F10:
    case KEY_F11:
    case KEY_F12:
      return true;
  }
  return false;
}

void kb_getkey(DIALOG *d)
{
  d->flags|=D_SELECTED;

  scare_mouse();
  jwin_button_proc(MSG_DRAW,d,0);
  jwin_draw_win(screen, (resx-160)/2, (resy-48)/2, 160, 48, FR_WIN);
                                                            //  text_mode(vc(11));
  textout_centre_ex(screen, font, "Press a key", resx/2, resy/2 - 8, vc(0),vc(11));
  textout_centre_ex(screen, font, "ESC to cancel", resx/2, resy/2, vc(0),vc(11));
  unscare_mouse();

  clear_keybuf();
  int k = next_press_key();
  clear_keybuf();

                                                            //shnarf
                                                            //47=f1
                                                            //59=esc
  if(k>0 && k<123 && !((k>46)&&(k<60)))
    *((int*)d->dp3) = k;


  d->flags&=~D_SELECTED;
}

int d_kbutton_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
    case MSG_KEY:
    case MSG_CLICK:

      kb_getkey(d);
      while(gui_mouse_b())
        clear_keybuf();
      return D_REDRAW;
  }

  return jwin_button_proc(msg,d,c);
}

void j_getbtn(DIALOG *d)
{
  d->flags|=D_SELECTED;
  scare_mouse();
  jwin_button_proc(MSG_DRAW,d,0);
  jwin_draw_win(screen, (resx-160)/2, (resy-48)/2, 160, 48, FR_WIN);
                                                            //  text_mode(vc(11));
  int y = resy/2 - 12;
  textout_centre_ex(screen, font, "Press a button", resx/2, y, vc(0),vc(11));
  textout_centre_ex(screen, font, "ESC to cancel", resx/2, y+8, vc(0),vc(11));
  textout_centre_ex(screen, font, "SPACE to disable", resx/2, y+16, vc(0),vc(11));
  unscare_mouse();

  int b = next_press_btn();
  if(b>=0)
    *((int*)d->dp3) = b;

  d->flags&=~D_SELECTED;
  player->joy_on = TRUE;
}

int d_jbutton_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
    case MSG_KEY:
    case MSG_CLICK:

      j_getbtn(d);
      while(gui_mouse_b())
        clear_keybuf();
      return D_REDRAW;
  }

  return jwin_button_proc(msg,d,c);
}

                                                            //shnarf
char *key_str[] =
{
  "(none)",         "a",              "b",              "c",
  "d",              "e",              "f",              "g",
  "h",              "i",              "j",              "k",
  "l",              "m",              "n",              "o",
  "p",              "q",              "r",              "s",
  "t",              "u",              "v",              "w",
  "x",              "y",              "z",              "0",
  "1",              "2",              "3",              "4",
  "5",              "6",              "7",              "8",
  "9",              "num 0",          "num 1",          "num 2",
  "num 3",          "num 4",          "num 5",          "num 6",
  "num 7",          "num 8",          "num 9",          "f1",
  "f2",             "f3",             "f4",             "f5",
  "f6",             "f7",             "f8",             "f9",
  "f10",            "f11",            "f12",            "esc",
  "~",              "-",              "=",              "backspace",
  "tab",            "{",              "}",              "enter",
  ":",              "quote",          "\\",             "\\ (2)",
  ",",              ".",              "/",              "space",
  "insert",         "delete",         "home",           "end",
  "page up",        "page down",      "left",           "right",
  "up",             "down",           "num /",          "num *",
  "num -",          "num +",          "num delete",     "num enter",
  "print screen",   "pause",          "abnt c1",        "yen",
  "kana",           "convert",        "no convert",     "at",
  "circumflex",     ": (2)",          "kanji",          "num =",
  "back quote",     ";",              "command",        "unknown (0)",
  "unknown (1)",    "unknown (2)",    "unknown (3)",    "unknown (4)",
  "unknown (5)",    "unknown (6)",    "unknown (7)",    "left shift",
  "right shift",    "left control",   "right control",  "alt",
  "alt gr",         "left win",       "right win",      "menu",
  "scroll lock",    "number lock",    "caps lock",      "MAX"
};

char *pan_str[4] = { "MONO", " 1/2", " 3/4", "FULL" };
//char zcmusic_bufsz = 10;

static char str_a[44],str_b[44],str_s[44],str_m[16],str_l[16],str_r[16],str_p[16];

int d_stringloader(int msg,DIALOG *d,int c)
{
  if(msg==MSG_DRAW)
  {
    switch(d->w)
    {
      case 0:
        sprintf(str_a,"%d\n%s",Akey,key_str[Akey]);
        sprintf(str_b,"%d\n%s",Bkey,key_str[Bkey]);
        sprintf(str_s,"%d\n%s",Skey,key_str[Skey]);
        sprintf(str_l,"%d\n%s",Lkey,key_str[Lkey]);
        sprintf(str_r,"%d\n%s",Rkey,key_str[Rkey]);
        sprintf(str_p,"%d\n%s",Pkey,key_str[Pkey]);
        break;

      case 1:
        sprintf(str_a,"%d\n%s",DUkey,key_str[DUkey]);
        sprintf(str_b,"%d\n%s",DDkey,key_str[DDkey]);
        sprintf(str_l,"%d\n%s",DLkey,key_str[DLkey]);
        sprintf(str_r,"%d\n%s",DRkey,key_str[DRkey]);
        break;

      case 2:
        sprintf(str_a,"%d",Abtn);
        sprintf(str_b,"%d",Bbtn);
        sprintf(str_s,"%d",Sbtn);

        sprintf(str_l,"%d",Lbtn);
        sprintf(str_r,"%d",Rbtn);
        sprintf(str_m,"%d",Mbtn);
        sprintf(str_p,"%d",Pbtn);
        break;

      case 3:
        sprintf(str_a,"%3d",midi_volume);
        sprintf(str_b,"%3d",digi_volume);
        strcpy(str_s,pan_str[pan_style]);
        sprintf(str_m,"%2dKB",zcmusic_bufsz);
        break;
    }
  }
  return D_O_K;
}

int set_vol(void *dp3, int d2)
{
  switch(((int*)dp3)[0])
  {
    case 0:  midi_volume = min(d2<<3,255); break;
    case 1:  digi_volume = min(d2<<3,255); break;
  }
  scare_mouse();
  // text_mode(vc(11));
  textprintf_right_ex(screen,font,((int*)dp3)[1],((int*)dp3)[2],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%3d",min(d2<<3,255));
  unscare_mouse();
  return D_O_K;
}

int set_pan(void *dp3, int d2)
{
  pan_style = vbound(d2,0,3);
  scare_mouse();
                                                            // text_mode(vc(11));
  textout_right_ex(screen,font,pan_str[pan_style],((int*)dp3)[1],((int*)dp3)[2],jwin_pal[jcBOXFG],jwin_pal[jcBOX]);
  unscare_mouse();
  return D_O_K;
}

int set_buf(void *dp3, int d2)
{
  scare_mouse();
                                                            // text_mode(vc(11));
  zcmusic_bufsz = d2 + 1;
  textprintf_right_ex(screen,font,((int*)dp3)[1],((int*)dp3)[2],jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%2dKB",zcmusic_bufsz);
  unscare_mouse();
  return D_O_K;
}

static DIALOG key_dlg[] =
{
  // (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3)
  { jwin_win_proc,       8,    44,   304,  172,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "Keyboard Buttons" },
  { d_stringloader,      0,    0,    0 },
  { jwin_frame_proc,     14,   70,   147,  104,  0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_frame_proc,     159,  70,   147,  104,  0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_text_proc,         30,   76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Button  Key" },
  { jwin_text_proc,         175,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Button  Key" },

  { jwin_text_proc,      92,   92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a },
  { jwin_text_proc,      92,   120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_b },
  { jwin_text_proc,      92,   148,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_s },
  { jwin_text_proc,      237,  92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_l },
  { jwin_text_proc,      237,  120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_r },
  { jwin_text_proc,      237,  148,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_p },
                                                            // 12
  { d_kbutton_proc,      22,   90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "A",     NULL, &Akey},
  { d_kbutton_proc,      22,   118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "B",     NULL, &Bkey},
  { d_kbutton_proc,      22,   146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Start", NULL, &Skey},
  { d_kbutton_proc,      167,  90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "L",     NULL, &Lkey},
  { d_kbutton_proc,      167,  118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "R",     NULL, &Rkey},
  { d_kbutton_proc,      167,  146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Map",   NULL, &Pkey},
                                                            // 18
  { jwin_button_proc,    90,   184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "OK" },
  { jwin_button_proc,    170,  184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "Cancel" },
  { NULL }
};
static DIALOG keydir_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
  { jwin_win_proc,       8,    44,   304,  172,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "Keyboard Directions" },
  { d_stringloader,      0,    0,    1 },
  { jwin_frame_proc,     14,   70,   147,  104,  0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_frame_proc,     159,  70,   147,  104,  0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_text_proc,         30,   76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Button  Key" },
  { jwin_text_proc,         175,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Button  Key" },

  { jwin_text_proc,      92,   92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a },
  { jwin_text_proc,      92,   120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_b },
  { jwin_text_proc,      237,  92,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_l },
  { jwin_text_proc,      237,  120,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_r },

  { d_kbutton_proc,      22,   90,   61,   21,   vc(14),  vc(11),  0,       0,         0,        0,       (void *) "Up",     NULL, &DUkey},
  { d_kbutton_proc,      22,   118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Down",   NULL, &DDkey},
  { d_kbutton_proc,      167,  90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Left",   NULL, &DLkey},
  { d_kbutton_proc,      167,  118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Right",  NULL, &DRkey},
                                                            // 14
  { jwin_button_proc,    90,   184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "OK" },
  { jwin_button_proc,    170,  184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "Cancel" },
  { NULL }
};

static DIALOG btn_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
  { jwin_win_proc,       8,    44,   304,  172,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "Joystick Buttons" },
  { d_stringloader,      0,    0,    2 },
  { jwin_frame_proc,     159,  70,   147,  104,  0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_frame_proc,     14,   70,   147,  132,  0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_ctext_proc,     22+31,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Button" },
  { jwin_ctext_proc,     167+31,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Button" },

  { jwin_ctext_proc,         100+20,  96,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a },
  { jwin_ctext_proc,         100+20,  124,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_b },
  { jwin_ctext_proc,         100+20,  152,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_s },
  { jwin_ctext_proc,         100+20,  180,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_m },
  { jwin_ctext_proc,         245+20,  96,   60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_l },
  { jwin_ctext_proc,         245+20,  124,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_r },
  { jwin_ctext_proc,         245+20,  152,  60,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_p },
                                                            // 13
  { d_jbutton_proc,      22,   90,   61,   21,   vc(14),  vc(11),  0,       0,         0,        0,       (void *) "A",     NULL, &Abtn},
  { d_jbutton_proc,      22,   118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "B",     NULL, &Bbtn},
  { d_jbutton_proc,      22,   146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Start", NULL, &Sbtn},
  { d_jbutton_proc,      22,   174,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Menu",  NULL, &Mbtn},
  { d_jbutton_proc,      167,  90,   61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "L",     NULL, &Lbtn},
  { d_jbutton_proc,      167,  118,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "R",     NULL, &Rbtn},
  { d_jbutton_proc,      167,  146,  61,   21,   vc(14),  vc(1),   0,       0,         0,        0,       (void *) "Map",   NULL, &Pbtn},
                                                            // 20
  { jwin_button_proc,    170,  184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "OK" },
  { jwin_button_proc,    240,  184,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "Cancel" },
  { jwin_ctext_proc,     100+20,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Joystick" },
  { jwin_ctext_proc,     245+20,  76,   160,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Joystick" },
  { NULL }
};

int midi_dp[3] = {0,147,104};
int digi_dp[3] = {1,147,120};
int pan_dp[3]  = {0,147,136};
int buf_dp[3]  = {0,147,152};


static DIALOG sound_dlg[] =
{
  /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)  (dp2)    (dp3) */
  { jwin_win_proc,       32,   64,   256,  162,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "Sound Settings" },
  { d_stringloader,      0,    0,    3 },
  { jwin_frame_proc,     42,   92,   236,  96,   0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_rtext_proc,     147,  104,  40,   8,    vc(7),   vc(11),  0,       0,         0,        0,       (void *) str_a },
  { jwin_rtext_proc,     147,  120,  40,   8,    vc(7),   vc(11),  0,       0,         0,        0,       (void *) str_b },
  { jwin_rtext_proc,     147,  136,  40,   8,    vc(7),   vc(11),  0,       0,         0,        0,       (void *) str_s },
  { jwin_slider_proc,    164,  104,  96,   8,    vc(0),   jwin_pal[jcBOX],  0,       0,         32,       0,       NULL, (void *) set_vol, midi_dp },
  { jwin_slider_proc,    164,  120,  96,   8,    vc(0),   jwin_pal[jcBOX],  0,       0,         32,       0,       NULL, (void *) set_vol, digi_dp },
  { jwin_slider_proc,    164,  136,  96,   8,    vc(0),   jwin_pal[jcBOX],  0,       0,         3,        0,       NULL, (void *) set_pan, pan_dp },
  { jwin_text_proc,      60,   104,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "BG Vol" },
  { jwin_text_proc,      60,   120,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "SFX Vol" },
  { jwin_text_proc,      60,   136,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "SFX Pan" },
  { jwin_button_proc,    90,   196,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "OK" },
  { jwin_button_proc,    170,  196,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "Cancel" },
  { jwin_rtext_proc,     147,  152,  40,   8,    vc(7),   vc(11),  0,       0,         0,        0,       (void *) str_m },
  { jwin_slider_proc,    164,  152,  96,   8,    vc(0),   jwin_pal[jcBOX],  0,       0,         31,       0,       NULL, (void *) set_buf, buf_dp },
  { jwin_text_proc,      60,   152,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "BG Buf" },
  { jwin_check_proc,     60,   168,  48,   9,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Strict MIDI" },
  { NULL }
};

static DIALOG about_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
  { jwin_win_proc,       68,   52,   184,  154,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "About" },
  { jwin_button_proc,    140,  176,  41,   21,   vc(14),  0,       0,       D_EXIT,    0,        0,       (void *) "OK" },
  { jwin_ctext_proc,        160,  84,   0,    8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Zelda Classic" },
  { jwin_ctext_proc,        160,  92,   0,    8,    vc(0) ,  vc(11),  0,       0,         0,        0,       str_s },
  { jwin_ctext_proc,        160,  100,  0,    8,    vc(0) ,  vc(11),  0,       0,         0,        0,       (void *) DATE_STR },
  { jwin_text_proc,         88,   124,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Coded by:" },
  { jwin_text_proc,         88,   132,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "  Phantom Menace" },
  { jwin_text_proc,         88,   144,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Produced by:" },
  { jwin_text_proc,         88,   152,  140,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "  Armageddon Games" },
  { jwin_frame_proc,     80,   117,  160,  50,   0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { NULL }
};

static DIALOG quest_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
  { jwin_win_proc,       68,   25,   184,  190,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "Quest Info" },
  { jwin_edit_proc,      84,   54,   152,  16,   0,       0,       0,       D_READONLY, 100,     0,       NULL },
  { jwin_text_proc,         89,   84,   141,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Number:" },
  { jwin_text_proc,         152,  84,   24,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_a },
  { jwin_text_proc,         89,   94,   141,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Version:" },
  { jwin_text_proc,         160,  94,   64,   8,    vc(7),   vc(11),  0,       0,         0,        0,       QHeader.version },
  { jwin_text_proc,         89,   104,  141,  8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "ZQ Version:" },
  { jwin_text_proc,         184,  104,  64,   8,    vc(7),   vc(11),  0,       0,         0,        0,       str_s },
  { jwin_text_proc,         84,   126,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Title:" },
  { jwin_textbox_proc,   84,   136,  152,  24,   0,       0,       0,       0,         0,        0,       QHeader.title },
  { jwin_text_proc,         84,   168,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Author:" },
  { jwin_textbox_proc,   84,   178,  152,  24,   0,       0,       0,       0,         0,        0,       QHeader.author },
  { jwin_frame_proc,     84,   79,   152,  38,   0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { NULL }
};

static DIALOG triforce_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,      72,    64,  177,  105,  vc(14),   vc(1),    0,  D_EXIT,           0,        0,     (void *) "Triforce Pieces" },
                                                            // 1
  { jwin_check_proc,   129,    94,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "1" },
  { jwin_check_proc,   129,   104,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "2" },
  { jwin_check_proc,   129,   114,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "3" },
  { jwin_check_proc,   129,   124,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "4" },
  { jwin_check_proc,   172,    94,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "5" },
  { jwin_check_proc,   172,   104,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "6" },
  { jwin_check_proc,   172,   114,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "7" },
  { jwin_check_proc,   172,   124,   24,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "8" },
                                                            // 9
  { jwin_button_proc,  90,    144,   61,   21,   vc(0),  vc(11),  'k',  D_EXIT,           0,        0,     (void *) "O&K" },
  { jwin_button_proc,  170,   144,   61,   21,   vc(0),  vc(11),   27,  D_EXIT,           0,        0,     (void *) "Cancel" },
  { NULL }
};

static DIALOG equip_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      16,    18,  289,  215,  vc(14),   vc(1),    0,  D_EXIT,           0,        0,     (void *) "Equipment" },
                                                            // 1
  { jwin_button_proc,   90,   206,   61,   21,   vc(0),  vc(11),  'k',  D_EXIT,           0,        0,     (void *) "O&K" },
  { jwin_button_proc,  170,   206,   61,   21,   vc(0),  vc(11),   27,  D_EXIT,           0,        0,     (void *) "Cancel" },
                                                            // 3
  { jwin_frame_proc,    25,    45,   77,   50,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,        29,    42,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Sword" },
  { jwin_check_proc,    33,    52,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
  { jwin_check_proc,    33,    62,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "White " },
  { jwin_check_proc,    33,    72,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic " },
  { jwin_check_proc,    33,    82,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Master" },
                                                            // 9
  { jwin_frame_proc,    25,    99,   77,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,        29,    96,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Shield" },
  { jwin_check_proc,    33,   106,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small " },
  { jwin_check_proc,    33,   116,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic " },
  { jwin_check_proc,    33,   126,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Mirror" },
                                                            // 14
  { jwin_frame_proc,    25,   143,   61,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,        29,   140,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Ring" },
  { jwin_check_proc,    33,   150,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
  { jwin_check_proc,    33,   160,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red " },
  { jwin_check_proc,    33,   170,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Gold" },
                                                            // 19
  { jwin_frame_proc,   110,    45,   85,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       114,    42,   64,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Bracelet" },
  { jwin_check_proc,   118,    52,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" },
  { jwin_check_proc,   118,    62,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" },
                                                            // 23
  { jwin_frame_proc,   110,    79,   85,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       114,    76,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Amulet" },
  { jwin_check_proc,   118,    86,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" },
  { jwin_check_proc,   118,    96,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 2" },
                                                            // 27
  { jwin_frame_proc,   110,   113,   69,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       114,   110,   48,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Wallet" },
  { jwin_check_proc,   118,   120,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
  { jwin_check_proc,   118,   130,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
                                                            // 31
  { jwin_frame_proc,   110,   147,   69,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       114,   144,   24,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Bow" },
  { jwin_check_proc,   118,   154,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Small" },
  { jwin_check_proc,   118,   164,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Large" },
                                                            // 35
  { jwin_frame_proc,   203,    45,   93,   70,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       207,    42,   40,    8,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Other" },
  { jwin_check_proc,   211,    52,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Raft    " },
  { jwin_check_proc,   211,    62,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Ladder  " },
  { jwin_check_proc,   211,    72,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Book    " },
  { jwin_check_proc,   211,    82,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "M. Key  " },
  { jwin_check_proc,   211,    92,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Flippers" },
  { jwin_check_proc,   211,   102,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Boots   " },
  { NULL }
};

static DIALOG items_dlg[] =
{
                                                            // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      16,    18,  289,  215,  vc(14),   vc(1),    0,  D_EXIT,           0,        0,     (void *) "Items" },
                                                            //1
  { jwin_button_proc,   90,   206,   61,   21,   vc(0),  vc(11),  'k',  D_EXIT,           0,        0,     (void *) "O&K" },
  { jwin_button_proc,  170,   206,   61,   21,   vc(0),  vc(11),   27,  D_EXIT,           0,        0,     (void *) "Cancel" },
                                                            // 3
  { jwin_frame_proc,    27,    45,   77,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,        31,    42,   64,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "B-rang" },
  { jwin_check_proc,    35,    52,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
  { jwin_check_proc,    35,    62,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Magic " },
  { jwin_check_proc,    35,    72,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Fire  " },
                                                            // 8
  { jwin_frame_proc,    27,    89,   77,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,        31,    86,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Arrow" },
  { jwin_check_proc,    35,    96,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wooden" },
  { jwin_check_proc,    35,   106,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Silver" },
  { jwin_check_proc,    35,   116,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Golden" },
                                                            // 13
  { jwin_frame_proc,    27,   133,   63,   40,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,        31,   130,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Potion" },
  { jwin_radio_proc,    35,   140,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "None" },
  { jwin_radio_proc,    35,   150,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
  { jwin_radio_proc,    35,   160,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red" },
                                                            // 18
  { jwin_frame_proc,   114,    45,   93,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       118,    42,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Whistle" },
  { jwin_check_proc,   122,    52,   80,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Recorder" },
                                                            // 21
  { jwin_frame_proc,   114,    69,   86,   20,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       118,    66,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Hammer" },
  { jwin_check_proc,   122,    76,   72,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Level 1" },
                                                            // 24
  { jwin_frame_proc,   114,    93,   69,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       118,    90,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "H-shot" },
  { jwin_check_proc,   122,   100,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Short" },
  { jwin_check_proc,   122,   110,   56,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Long " },
                                                            // 28
  { jwin_frame_proc,   114,   127,   60,   30,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       118,   124,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Candle" },
  { jwin_check_proc,   122,   134,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Blue" },
  { jwin_check_proc,   122,   144,   48,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Red " },
                                                            // 32
  { jwin_frame_proc,   217,    45,   77,  138,       0,       0,    0,       0,   FR_ETCHED,        0,     NULL },
  { jwin_text_proc,       221,    42,   80,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Other" },
  { jwin_check_proc,   225,    52,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Bait  " },
  { jwin_check_proc,   225,    62,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Letter" },
  { jwin_check_proc,   225,    72,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Wand  " },
  { jwin_check_proc,   225,    82,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "Lens  " },
  { jwin_check_proc,   225,    92,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "D-Fire" },
  { jwin_check_proc,   225,   102,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "F-Wind" },
  { jwin_check_proc,   225,   112,   64,    9,   vc(0),  vc(11),    0,       0,           1,        0,     (void *) "N-Love" },
  { jwin_text_proc,       225,   122,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "Bombs:" },
  { jwin_edit_proc,    229,   132,   40,   16,       0,       0,    0,       0,           6,        0,     NULL },
  { jwin_text_proc,       225,   152,   48,    9,   vc(0),  vc(11),    0,       0,           0,        0,     (void *) "S-Bombs:" },
  { jwin_edit_proc,    229,   162,   40,   16,       0,       0,    0,       0,           6,        0,     NULL },
  { NULL }
};

static DIALOG credits_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
  { jwin_win_proc,       40,   38,   241,  173,  vc(14),  vc(1),   0,       D_EXIT,    0,        0,       (void *) "Zelda Classic Credits" },
  { jwin_frame_proc,     47,   65,   227,  115,  vc(15),  vc(1),   0,       0,         FR_DEEP,  0,       NULL },
  { d_bitmap_proc,       49,   67,   222,  110,  vc(15),  vc(1),   0,       0,         0,        0,       NULL },
  { jwin_button_proc,    140,  184,  41,   21,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       (void *) "OK" },
  { NULL }
};

static DIALOG goto_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)                     (dp2) (dp3) */
  { jwin_win_proc,       68,   25,   185,  191,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "Goto Location" },
  { jwin_button_proc,    90,   176,  61,   21,   vc(14),  0,       0,       D_EXIT,    0,        0,       (void *) "OK" },
  { jwin_button_proc,    170,  176,  61,   21,   vc(14),  0,       0,       D_EXIT,    0,        0,       (void *) "Cancel" },
  { jwin_text_proc,         80,   129,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Map:" },
  { jwin_edit_proc,      128,  126,  115,  16,   0,       0,       0,       0,         3,        0,       NULL },
  { jwin_text_proc,         80,   149,  80,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Screen:" },
  { jwin_edit_proc,      152,  146,  91,   16,   0,       0,       0,       0,         2,        0,       NULL },
  { NULL }
};

int xtoi(char *hexstr)
{
  int val=0;
  while(isxdigit(*hexstr))
  {
    val<<=4;
    if(*hexstr<='9')
      val += *hexstr-'0';
    else val+= ((*hexstr)|0x20)-'a'+10;
    ++hexstr;
  }
  return val;
}

int onGoTo()
{
  sprintf(cheat_goto_map_str,"%d",cheat_goto_map);
  sprintf(cheat_goto_screen_str,"%X",cheat_goto_screen);

  goto_dlg[0].dp2=lfont;
  goto_dlg[4].dp=cheat_goto_map_str;
  goto_dlg[6].dp=cheat_goto_screen_str;

  if (PopUp_dialog(goto_dlg,1)==1)
  {
    cheat_goto_map=atoi(cheat_goto_map_str);
    cheat_goto_screen=xtoi(cheat_goto_screen_str);
  };
  return D_O_K;
}

int onCredits()
{
  go();

  BITMAP *win = create_bitmap_ex(8,222,110);
  if(!win)
    return D_O_K;

  int c=0;
  int l=0;
  int ol=-1;
  RLE_SPRITE *rle = (RLE_SPRITE*)(data[RLE_CREDITS].dat);
  RGB *pal = (RGB*)(data[PAL_CREDITS].dat);
  PALETTE tmppal;

  clear_bitmap(win);
  draw_rle_sprite(win,rle,0,0);
  credits_dlg[0].dp2=lfont;
  credits_dlg[1].fg = gui_mg_color;
  credits_dlg[2].dp = win;
  set_palette_range(black_palette,0,127,false);

  DIALOG_PLAYER *p = init_dialog(credits_dlg,3);

  while(update_dialog(p))
  {
    ++c;
    l = max((c>>1)-30,0);

    if(l > rle->h)
      l = c = 0;
    if(l > rle->h - 112)
      l = rle->h - 112;

    clear_bitmap(win);
    draw_rle_sprite(win,rle,0,0-l);

    if(c<=64)
      fade_interpolate(black_palette,pal,tmppal,c,0,127);

    waitvsync();

    if(c<=64)
      set_palette_range(tmppal,0,127,false);
    if(l!=ol)
    {
      scare_mouse();
      d_bitmap_proc(MSG_DRAW,credits_dlg+2,0);
      unscare_mouse();
      SCRFIX();
      ol=l;
    }
  }

  shutdown_dialog(p);
  destroy_bitmap(win);
  comeback();
  return D_O_K;
}

char *midilist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size=0;
    for(int i=0; i<MAXMUSIC; i++)
      if(tunes[i].midi)
        ++(*list_size);
    return NULL;
  }

  int i=0,m=0;
  while(m<=index && i<=MAXMUSIC)
  {
    if(tunes[i].midi)
      ++m;
    ++i;
  }
  --i;
  if(i==MAXMUSIC && m<index)
    return "(null)";
  return tunes[i].title;
}

                                                            /*  ------- MIDI info stuff -------- */

char *text;
midi_info *zmi;
bool dialog_running;
bool listening;

void get_info(int index);

int d_midilist_proc(int msg,DIALOG *d,int c)
{
  int d2 = d->d2;
  int ret = jwin_droplist_proc(msg,d,c);
  if(d2!=d->d2)
    get_info(d->d2);
  return ret;
}

int d_listen_proc(int msg,DIALOG *d,int c)
{
                                                            /* 'd->d1' is offset from 'd' in DIALOG array to midilist proc */

  int ret = jwin_button_proc(msg,d,c);

  if(ret == D_CLOSE)
  {
                                                            // get current midi index
    int index = (d+(d->d1))->d2;
    int i=0, m=0;
    while(m<=index && i<=MAXMUSIC)
    {
      if(tunes[i].midi)
        ++m;
      ++i;
    }
    --i;
    jukebox(i);
    listening = true;
    ret = D_O_K;
  }

  return ret;
}

int d_savemidi_proc(int msg,DIALOG *d,int c)
{
                                                            /* 'd->d1' is offset from 'd' in DIALOG array to midilist proc */

  int ret = jwin_button_proc(msg,d,c);

  if(ret == D_CLOSE)
  {
                                                            // get current midi index
    int index = (d+(d->d1))->d2;
    int i=0, m=0;

    while(m<=index && i<=MAXMUSIC)
    {
      if(tunes[i].midi)
        ++m;
      ++i;
    }
    --i;

                                                            // get file name

    int  sel=0;
                                                            //struct ffblk f;
    char title[40] = "Save MIDI: ";
    static char fname[260] = "";
    static EXT_LIST list[] =
    {
      { "MIDI files (*.mid)", "mid" },
      { "HTML files (*.html, *.html)", "htm html" },
      { NULL }
    };

    strcpy(title+11, tunes[i].title);

    if(jwin_file_browse_ex(title, fname, list, &sel, 1024, -1, -1, lfont)==0)
      goto done;

    if(exists(fname))
    {
      if(jwin_alert(title, fname, "already exists.", "Overwrite it?", "&Yes","&No",'y','n',lfont)==2)
        goto done;
    }

                                                            // save midi i

    if(save_midi(fname, tunes[i].midi) != 0)
      jwin_alert(title, "Error saving MIDI to", fname, NULL, "Darn", NULL,13,27,lfont);

    done:
    chop_path(fname);
    ret = D_REDRAW;
  }

  return ret;
}

static DIALOG midi_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
  { jwin_win_proc,       8,    28,   304,  184,  0,       0,        0,       D_EXIT,    0,        0,       (void *) "MIDI Info" },
  { jwin_text_proc,         32,   60,   40,   8,    vc(0),   vc(11),   0,       0,         0,        0,       (void *) "Tune:" },
  { d_midilist_proc,     80,   56,   192,  16,   0,       0,        0,       0,         0,        0,       (void *) midilist },
  { jwin_textbox_proc,   15,   80,   290,  96,   0,       0,        0,       0,         0,        0,       NULL },
  { d_listen_proc,       24,   183,  72,   21,   0,       0,        'l',     D_EXIT,    -2,       0,       (void *) "&Listen" },
  { d_savemidi_proc,     108,  183,  72,   21,   0,       0,        's',     D_EXIT,    -3,       0,       (void *) "&Save" },
  { jwin_button_proc,    236,  183,  61,   21,   0,       0,        'k',     D_EXIT,    0,        0,       (void *) "O&K" },
  { NULL }
};

void get_info(int index)
{
  int i=0, m=0;
  while(m<=index && i<=MAXMUSIC)
  {
    if(tunes[i].midi)
      ++m;
    ++i;
  }
  --i;

  if(i==MAXMUSIC && m<index)
    strcpy(text,"(null)");
  else
  {
    get_midi_info(tunes[i].midi,zmi);
    get_midi_text(tunes[i].midi,zmi,text);
  }

  midi_dlg[0].dp2=lfont;
  midi_dlg[3].dp = text;
  midi_dlg[3].d1 = midi_dlg[3].d2 = 0;

  if(dialog_running)
  {
    scare_mouse();
    jwin_textbox_proc(MSG_DRAW,midi_dlg+3,0);
    unscare_mouse();
  }
}

int onMIDICredits()
{
  text = (char*)malloc(4096);
  zmi = (midi_info*)malloc(sizeof(midi_info));

  if(!text || !zmi)
  {
    jwin_alert(NULL,"Not enough memory",NULL,NULL,"OK",NULL,13,27,lfont);
    return D_O_K;
  }

  midi_dlg[0].dp2=lfont;
  midi_dlg[2].d1 = 0;
  midi_dlg[2].d2 = 0;
  midi_dlg[4].flags = (midi_pos >= 0) ? D_DISABLED : D_EXIT;

  listening = false;
  dialog_running=false;
  get_info(0);

  dialog_running=true;
  PopUp_dialog(midi_dlg,0);
  dialog_running=false;

  if(listening)
    music_stop();

  free(text);
  free(zmi);
  return D_O_K;
}

int onAbout()
{
  switch (IS_BETA)
  {
    case 1:
      sprintf(str_s,"%s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
    case -1:
      sprintf(str_s,"%s Alpha (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);break;
    case 0:
      sprintf(str_s,"%s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
  }
                                                            //  sprintf(str_s,"%s",VerStr(ZELDA_VERSION));
  about_dlg[0].dp2=lfont;
  PopUp_dialog(about_dlg,1);
  return D_O_K;
}

int onQuest()
{
  char fname[100];
  strcpy(fname, get_filename(qstpath));
  quest_dlg[0].dp2=lfont;
  quest_dlg[1].dp = fname;

  if(QHeader.quest_number==0)
    sprintf(str_a,"Custom");
  else
    sprintf(str_a,"%d",QHeader.quest_number);
  sprintf(str_s,"%s",VerStr(QHeader.zelda_version));

  quest_dlg[11].d1 = quest_dlg[9].d1 = 0;
  quest_dlg[11].d2 = quest_dlg[9].d2 = 0;

  PopUp_dialog(quest_dlg, 0);
  return D_O_K;
}

int onVidMode()
{

  #ifdef ALLEGRO_DOS
  switch(VidMode)
  {
    case GFX_MODEX:  sprintf(str_a,"VGA Mode X"); break;
    case GFX_VESA1:  sprintf(str_a,"VESA 1.x"); break;
    case GFX_VESA2B: sprintf(str_a,"VESA2 Banked"); break;
    case GFX_VESA2L: sprintf(str_a,"VESA2 Linear"); break;
    case GFX_VESA3:  sprintf(str_a,"VESA3"); break;
    default:         sprintf(str_a,"Unknown... ?"); break;
  }
  #elif defined(ALLEGRO_WINDOWS)
  switch(VidMode)
  {
    case GFX_DIRECTX:      sprintf(str_a,"DirectX Hardware Accelerated"); break;
    case GFX_DIRECTX_SOFT: sprintf(str_a,"DirectX Software Accelerated"); break;
    case GFX_DIRECTX_SAFE: sprintf(str_a,"DirectX Safe"); break;
    case GFX_DIRECTX_WIN:  sprintf(str_a,"DirectX Windowed"); break;
    case GFX_GDI:          sprintf(str_a,"GDI"); break;
    default:               sprintf(str_a,"Unknown... ?"); break;
  }
  #elif defined(ALLEGRO_MACOSX)
  switch(VidMode)
  {
    case GFX_SAFE:               sprintf(str_a,"MacOS X Safe"); break;
    case GFX_QUARTZ_FULLSCREEN:  sprintf(str_a,"MacOS X Fullscreen Quartz"); break;
    case GFX_QUARTZ_WINDOW:      sprintf(str_a,"MacOS X Windowed Quartz"); break;
    default:                     sprintf(str_a,"Unknown... ?"); break;
  }
  #elif defined(ALLEGRO_LINUX)
  switch(VidMode)
  {
    case GFX_AUTODETECT_WINDOWED:  sprintf(str_a,"Autodetect Windowed"); break;
    default:               sprintf(str_a,"Unknown... ?"); break;
  }
  #endif

  sprintf(str_b,"%dx%d 8-bit",resx,resy);
  jwin_alert("Video Mode",str_a,str_b,NULL,"OK",NULL,13,27,lfont);
  return D_O_K;
}

int onKeyboard()
{
  int a = Akey;
  int b = Bkey;
  int s = Skey;
  int l = Lkey;
  int r = Rkey;
  int p = Pkey;

  key_dlg[0].dp2=lfont;
  int ret = PopUp_dialog(key_dlg,18);
                                                            // not OK'd
  if(ret != 18)
  {
    Akey = a;
    Bkey = b;
    Skey = s;
    Lkey = l;
    Rkey = r;
    Pkey = p;
  }
  return D_O_K;
}

int onKeyboardDir()
{
  int u = DUkey;
  int d = DDkey;
  int l = DLkey;
  int r = DRkey;

  keydir_dlg[0].dp2=lfont;
  int ret = PopUp_dialog(keydir_dlg,14);
  if(ret != 14)
  {
    DUkey = u;
    DDkey = d;
    DLkey = l;
    DRkey = r;
  }
  return D_O_K;
}

int onJoystick()
{
  int a = Abtn;
  int b = Bbtn;
  int s = Sbtn;
  int l = Lbtn;
  int r = Rbtn;
  int m = Mbtn;
  int p = Pbtn;

  btn_dlg[0].dp2=lfont;
  int ret = PopUp_dialog(btn_dlg,20);
                                                            // not OK'd
  if(ret != 20)
  {
    Abtn = a;
    Bbtn = b;
    Sbtn = s;
    Lbtn = l;
    Rbtn = r;
    Mbtn = m;
    Pbtn = p;
  }
  return D_O_K;
}

int onSound()
{
  int d = digi_volume;
  int m = midi_volume;
  int p = pan_style;

  pan_style = vbound(pan_style,0,3);

  sound_dlg[0].dp2=lfont;
  midi_dp[1] = sound_dlg[3].x;
  midi_dp[2] = sound_dlg[3].y;
  digi_dp[1] = sound_dlg[4].x;
  digi_dp[2] = sound_dlg[4].y;
  pan_dp[1]  = sound_dlg[5].x;
  pan_dp[2]  = sound_dlg[5].y;
  buf_dp[1]  = sound_dlg[14].x;
  buf_dp[2]  = sound_dlg[14].y;
  sound_dlg[6].d2 = (midi_volume==255) ? 32 : midi_volume>>3;
  sound_dlg[7].d2 = (digi_volume==255) ? 32 : digi_volume>>3;
  sound_dlg[8].d2 = pan_style;
  sound_dlg[17].flags = midi_strict?D_SELECTED:0;

  int ret = PopUp_dialog(sound_dlg,12);

  if(ret==12)
  {
    master_volume(digi_volume,midi_volume);
    midi_strict=sound_dlg[17].flags&D_SELECTED?1:0;
  }
  else
  {
    digi_volume = d;
    midi_volume = m;
    pan_style = p;
  }

  return D_O_K;
}

int queding(char *s1,char *s2,char *s3)
{
  return jwin_alert(ZC_str,s1,s2,s3,"&Yes","&No",'y','n',lfont);
}

int onQuit()
{
  if(Playing && queding("Quit current game?",NULL,NULL)==1)
  {
    Quit=qQUIT;
    return D_CLOSE;
  }
  return D_O_K;
}

int onReset()
{
  if(queding("  Reset system?  ",NULL,NULL)==1)
  {
    Quit=qRESET;
    return D_CLOSE;
  }
  return D_O_K;
}

int onExit()
{
  if(queding("  Exit Program?  ",NULL,NULL)==1)
  {
    Quit=qEXIT;
    return D_CLOSE;
  }
  return D_O_K;
}

int onTitle_NES() { DXtitle=false; return D_O_K; }
int onTitle_DX() { DXtitle=true; return D_O_K; }

int onDebug()
{
  if(debug_enabled)
    debug=!debug;
  return D_O_K;
}

int onHeartBeep()
{
  heart_beep=!heart_beep;
  return D_O_K;
}

int onTriforce()
{
  triforce_dlg[0].dp2=lfont;
  for(int i=1; i<=8; i++)
    triforce_dlg[i].flags = (game.lvlitems[i] & liTRIFORCE) ? D_SELECTED : 0;

  if(PopUp_dialog(triforce_dlg,-1)==9)
  {
    for(int i=1; i<=8; i++)
    {
      game.lvlitems[i] &= ~liTRIFORCE;
      game.lvlitems[i] |= (triforce_dlg[i].flags & D_SELECTED) ? liTRIFORCE : 0;
    }
  }
  return D_O_K;
}

bool rc = false;

int onEquipment()
{
  equip_dlg[0].dp2=lfont;
  for(int i=0; i<4; i++)
    equip_dlg[i+5].flags = (game.items[itype_sword]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<3; i++)
    equip_dlg[i+11].flags = (game.items[itype_shield]&(1<<i)) ? D_SELECTED : 0;
  equip_dlg[11].flags = D_SELECTED;
  equip_dlg[11].flags |= D_DISABLED;
  for(int i=0; i<3; i++)
    equip_dlg[i+16].flags = (game.items[itype_ring]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<2; i++)
    equip_dlg[i+21].flags = (game.items[itype_bracelet]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<2; i++)
    equip_dlg[i+25].flags = (game.items[itype_amulet]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<2; i++)
    equip_dlg[i+29].flags = (game.items[itype_wallet]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<2; i++)
    equip_dlg[i+33].flags = (game.items[itype_bow]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    equip_dlg[i+37].flags = (game.items[itype_raft]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    equip_dlg[i+38].flags = (game.items[itype_ladder]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    equip_dlg[i+39].flags = (game.items[itype_book]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    equip_dlg[i+40].flags = (game.items[itype_magickey]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    equip_dlg[i+41].flags = (game.items[itype_flippers]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    equip_dlg[i+42].flags = (game.items[itype_boots]&(1<<i)) ? D_SELECTED : 0;

  if(PopUp_dialog(equip_dlg,-1)==1)
  {
    game.items[itype_sword]=0;
    for(int i=0; i<4; i++)
      if(equip_dlg[i+5].flags & D_SELECTED)
        game.items[itype_sword]|=(1<<i);

    game.items[itype_shield]=0;
    for(int i=0; i<3; i++)
      if(equip_dlg[i+11].flags & D_SELECTED)
        game.items[itype_shield]|=(1<<i);

    game.items[itype_ring]=0;
    for(int i=0; i<3; i++)
      if(equip_dlg[i+16].flags & D_SELECTED)
        game.items[itype_ring]|=(1<<i);

    game.items[itype_bracelet]=0;
    for(int i=0; i<2; i++)
      if(equip_dlg[i+21].flags & D_SELECTED)
        game.items[itype_bracelet]|=(1<<i);

    game.items[itype_amulet]=0;
    for(int i=0; i<2; i++)
      if(equip_dlg[i+25].flags & D_SELECTED)
        game.items[itype_amulet]|=(1<<i);

    game.items[itype_wallet]=0;
    for(int i=0; i<2; i++)
      if(equip_dlg[i+29].flags & D_SELECTED)
        game.items[itype_wallet]|=(1<<i);

    game.items[itype_bow]=0;
    for(int i=0; i<2; i++)
      if(equip_dlg[i+33].flags & D_SELECTED)
        game.items[itype_bow]|=(1<<i);

    game.items[itype_raft]=0;
    for(int i=0; i<1; i++)
      if(equip_dlg[i+37].flags & D_SELECTED)
        game.items[itype_raft]|=(1<<i);

    game.items[itype_ladder]=0;
    for(int i=0; i<1; i++)
      if(equip_dlg[i+38].flags & D_SELECTED)
        game.items[itype_ladder]|=(1<<i);

    game.items[itype_book]=0;
    for(int i=0; i<1; i++)
      if(equip_dlg[i+39].flags & D_SELECTED)
        game.items[itype_book]|=(1<<i);

    game.items[itype_magickey]=0;
    for(int i=0; i<1; i++)
      if(equip_dlg[i+40].flags & D_SELECTED)
        game.items[itype_magickey]|=(1<<i);

    game.items[itype_flippers]=0;
    for(int i=0; i<1; i++)
      if(equip_dlg[i+41].flags & D_SELECTED)
        game.items[itype_flippers]|=(1<<i);

    game.items[itype_boots]=0;
    for(int i=0; i<1; i++)
      if(equip_dlg[i+42].flags & D_SELECTED)
        game.items[itype_boots]|=(1<<i);

    rc=true;
  }
  return D_O_K;
}

int onItems()
{
  char bombstring[5], sbombstring[5];
  sprintf(bombstring, "%d", game.items[itype_bomb]);
  sprintf(sbombstring, "%d", game.items[itype_sbomb]);
  items_dlg[0].dp2=lfont;
  for(int i=0; i<3; i++)
    items_dlg[i+5].flags = (game.items[itype_brang]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<3; i++)
    items_dlg[i+10].flags = (game.items[itype_arrow]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<3; i++)
    items_dlg[i+15].flags = (game.items[itype_potion]==i)? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+20].flags = (game.items[itype_whistle]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+23].flags = (game.items[itype_hammer]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<2; i++)
    items_dlg[i+26].flags = (game.items[itype_hookshot]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<2; i++)
    items_dlg[i+30].flags = (game.items[itype_candle]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+34].flags = (game.items[itype_bait]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+35].flags = (game.items[itype_letter]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+36].flags = (game.items[itype_wand]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+37].flags = (game.items[itype_lens]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+38].flags = (game.items[itype_dinsfire]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+39].flags = (game.items[itype_faroreswind]&(1<<i)) ? D_SELECTED : 0;
  for(int i=0; i<1; i++)
    items_dlg[i+40].flags = (game.items[itype_nayruslove]&(1<<i)) ? D_SELECTED : 0;
  items_dlg[42].dp = bombstring;
  items_dlg[44].dp = sbombstring;

  if(PopUp_dialog(items_dlg,-1)==1)
  {
    game.items[itype_brang]=0;
    for(int i=0; i<3; i++)
      if(items_dlg[i+5].flags & D_SELECTED)
        game.items[itype_brang]|=(1<<i);
    game.items[itype_arrow]=0;
    for(int i=0; i<3; i++)
      if(items_dlg[i+10].flags & D_SELECTED)
        game.items[itype_arrow]|=(1<<i);
    for(int i=0; i<3; i++)
      if(items_dlg[i+15].flags & D_SELECTED)
        game.items[itype_potion]=i;
    game.items[itype_whistle]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+20].flags & D_SELECTED)
        game.items[itype_whistle]|=(1<<i);
    game.items[itype_hammer]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+23].flags & D_SELECTED)
        game.items[itype_hammer]|=(1<<i);
    game.items[itype_hookshot]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+26].flags & D_SELECTED)
        game.items[itype_hookshot]|=(1<<i);
    game.items[itype_candle]=0;
    for(int i=0; i<2; i++)
      if(items_dlg[i+30].flags & D_SELECTED)
        game.items[itype_candle]|=(1<<i);
    game.items[itype_bait]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+34].flags & D_SELECTED)
        game.items[itype_bait]|=(1<<i);
    game.items[itype_letter]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+35].flags & D_SELECTED)
        game.items[itype_letter]|=(1<<i);
    game.items[itype_wand]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+36].flags & D_SELECTED)
        game.items[itype_wand]|=(1<<i);
    game.items[itype_lens]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+37].flags & D_SELECTED)
        game.items[itype_lens]|=(1<<i);
    game.items[itype_dinsfire]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+38].flags & D_SELECTED)
        game.items[itype_dinsfire]|=(1<<i);
    game.items[itype_faroreswind]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+39].flags & D_SELECTED)
        game.items[itype_faroreswind]|=(1<<i);
    game.items[itype_nayruslove]=0;
    for(int i=0; i<1; i++)
      if(items_dlg[i+40].flags & D_SELECTED)
        game.items[itype_nayruslove]|=(1<<i);
    game.items[itype_bomb]=atoi(bombstring);
    game.items[itype_sbomb]=atoi(sbombstring);
  }

  return D_O_K;
}

static DIALOG getnum_dlg[] =
{
   // (dialog proc)       (x)   (y)    (w)     (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,        80,   80,     160,    72,   vc(0),  vc(11),  0,       D_EXIT,     0,             0,       NULL, lfont },
  { jwin_text_proc,          104,  104+4,  48,     8,    vc(0),  vc(11),  0,       0,          0,             0,       (void *) "Number:" },
  { jwin_edit_proc,       168,  104,    48,     16,    0,     0,       0,       0,          6,             0,       NULL },
  { jwin_button_proc,     90,   126,    61,     21,   vc(0),  vc(11),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  126,    61,     21,   vc(0),  vc(11),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { NULL }
};

int getnumber(char *prompt,int initialval)
{
  char buf[20];
  sprintf(buf,"%d",initialval);
  getnum_dlg[0].dp=prompt;
  getnum_dlg[0].dp2=lfont;
  getnum_dlg[2].dp=buf;
  if(zc_popup_dialog(getnum_dlg,2)==3)
    return atoi(buf);
  return initialval;
}

int onLife()
{
  game.life = vbound(getnumber("Life",game.life),0,game.maxlife);
  return D_O_K;
}

int onHeartC()
{
  game.maxlife = vbound(getnumber("Heart Containers",game.maxlife/HP_PER_HEART),1,24) * HP_PER_HEART;
  game.life = vbound(game.life,1,game.maxlife);
  return D_O_K;
}

int onRupies()
{
  game.rupies = vbound(getnumber("Rupies",game.rupies),0,255);
  return D_O_K;
}

int onMaxBombs()
{
  game.maxbombs = vbound(getnumber("Max Bombs",game.maxbombs),8,99);
  game.items[itype_bomb] = game.maxbombs;
  return D_O_K;
}

int onRefillLife() { game.life = game.maxlife; return D_O_K; }
int onRefillMagic() { game.magic = game.maxmagic; return D_O_K; }
int onClock()  { setClock(!getClock()); return D_O_K; }

int onQstPath()
{
  char path[280];

  chop_path(qstpath);
  strcpy(path,qstpath);

  go();

  if(jwin_dfile_select_ex("Quest File Directory", path, "qst", 1024, -1, -1, lfont))
  {
    chop_path(path);
    strcpy(qstpath,path);
  }

  comeback();
  return D_O_K;
}

static DIALOG cheat_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp) */
  { jwin_win_proc,       72,   72,   176,  96,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "Enter cheat code" },
  { jwin_edit_proc,      88,   104,  144,  16,   0,       0,       0,       0,         40,       0,       str_a },
  { jwin_button_proc,    130,  136,  61,   21,   0,       0,       13,      D_EXIT,    0,        0,       (void *) "OK" },
  { NULL }
};

int onCheat()
{
  if(!zcheats.flags && !debug)
    return D_O_K;

  str_a[0]=0;
  cheat_dlg[0].dp2=lfont;
  cheat_dlg[1].dp=str_a;
  int ret=zc_popup_dialog(cheat_dlg,1);
  if((ret==2) && strlen(str_a))
  {
    char str[80];
    for(int i=0; i<4; i++)
    {
      if(!strcmp(str_a, zcheats.codes[i]))
      {
        cheat   = i + 1;
        sprintf(str, "Cheat level %d enabled", i+1);
        jwin_alert("ZQuest",NULL,str,NULL,"OK",NULL,13,27,lfont);
        goto done;
      }
    }
    jwin_alert("ZQuest",NULL,"Invalid cheat code",NULL,"OK",NULL,13,27,lfont);
  }

done:
  return D_O_K;
}

int onCheatRupies()
{
  game.drupy = 999;
  return D_O_K;
}

int onCheatBombs()
{
  game.items[itype_bomb] = game.maxbombs;
  game.items[itype_sbomb] = game.maxbombs >> 2;
  return D_O_K;
}

                                                            // *** screen saver

int after_time()
{
  if(ss_after <= 0)
    return 5 * 60;

  if(ss_after <= 3)
    return ss_after * 15 * 60;

  if(ss_after <= 13)
    return (ss_after - 3) * 60 * 60;

  return MAX_IDLE + 1;
}

static char *after_str[15] =
{
  " 5 sec", "15 sec", "30 sec", "45 sec", " 1 min", " 2 min", " 3 min",
  " 4 min", " 5 min", " 6 min", " 7 min", " 8 min", " 9 min", "10 min",
  "Never"
};

char *after_list(int index, int *list_size)
{
  if(index < 0)
  {
    *list_size = 15;
    return NULL;
  }
  return after_str[index];
}

static DIALOG scrsaver_dlg[] =
{
                                                            /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)  (dp2)    (dp3) */
  { jwin_win_proc,       32,   64,   256,  136,  0,       0,       0,       D_EXIT,    0,        0,       (void *) "Screen Saver Settings" },
  { jwin_frame_proc,     42,   92,   236,  70,   0,       0,       0,       0,         FR_ETCHED,0,       NULL },
  { jwin_text_proc,         60,   104,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Run After" },
  { jwin_text_proc,         60,   128,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Speed" },
  { jwin_text_proc,         60,   144,  48,   8,    vc(0),   vc(11),  0,       0,         0,        0,       (void *) "Density" },
  { jwin_droplist_proc,  144,  100,  96,   16,   0,       0,       0,       0,         0,        0,       (void *) after_list },
  { jwin_slider_proc,    144,  128,  116,  8,    vc(0),   jwin_pal[jcBOX],  0,       0,         6,        0,       NULL },
  { jwin_slider_proc,    144,  144,  116,  8,    vc(0),   jwin_pal[jcBOX],  0,       0,         6,        0,       NULL },
  { jwin_button_proc,    42,   170,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "OK" },
  { jwin_button_proc,    124,  170,  72,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "Preview" },
  { jwin_button_proc,    218,  170,  61,   21,   0,       0,       0,       D_EXIT,    0,        0,       (void *) "Cancel" },
  { NULL }
};

int onScreenSaver()
{
  scrsaver_dlg[0].dp2=lfont;
  scrsaver_dlg[5].d1 = scrsaver_dlg[5].d2 = ss_after;
  scrsaver_dlg[6].d2 = ss_speed;
  scrsaver_dlg[7].d2 = ss_density;

  int ret = PopUp_dialog(scrsaver_dlg,-1);

  if(ret == 8 || ret == 9)
  {
    ss_after   = scrsaver_dlg[5].d1;
    ss_speed   = scrsaver_dlg[6].d2;
    ss_density = scrsaver_dlg[7].d2;
  }

  if(ret == 9)
                                                            // preview Screen Saver
  {
    while(keypressed())
      readkey();
    scare_mouse();
    Matrix(ss_speed, ss_density, 30);
    system_pal();
    unscare_mouse();
  }

  return D_O_K;
}

                                                            /*****  Menus  *****/

                                                            /*
int onTestBox() {
  char box_text[30];
  go();
  box_start(1, "hi", pfont, z3font);
  for (int i=0; i<30; i++) {
    sprintf(box_text, "%d", i);
    box_out(box_text); box_out("     "); box_out(box_text); box_out("     "); box_out(box_text); box_out("     "); box_eol();
    rest(100);
  }
  box_end(true);
comeback();
return D_O_K;
}
*/

static MENU game_menu[] =
{
  { "&Continue\tESC",       onContinue,  NULL },
  { "" },
  { "&Quit\tF6",            onQuit,      NULL },
  { "" },
  { "&Reset\tF7",           onReset,     NULL },
  { "E&xit\tF8",            onExit,      NULL },
                                                            //   { "Test Box",             onTestBox,   NULL },
  { NULL }
};

static MENU title_menu[] =
{
  { "&Original",            onTitle_NES, NULL },
  { "&Zelda Classic",       onTitle_DX,  NULL },
  { NULL }
};

static MENU settings_menu[] =
{
  { "Key &Buttons...",      onKeyboard,     NULL },
  { "Key &Directions...",   onKeyboardDir,  NULL },
  { "&Joystick...",         onJoystick,     NULL },
  { "&Sound...",            onSound,        NULL },
  { "&Title Screen",        NULL,           title_menu },
  { "" },
  { "T&hrottle FPS\tF1",    onVsync,        NULL },
  { "Show &FPS\tF2",        onShowFPS,      NULL },
  { "&Show Trans. Lyrs.",   onTransLayers,  NULL },
  { "Fast &Quit",           onNESquit,      NULL },
  { "Volume &Keys",         onVolKeys,      NULL },
  { "Cont. Heart Beep",     onHeartBeep,    NULL },
  //{ "Frame Skip",          onFrameSkip,        NULL },
  { "" },
  { "Debug",                onDebug,        NULL },
  { NULL }
};

static MENU misc_menu[] =
{
  { "&About...",            onAbout,     NULL },
  { "&Credits...",          onCredits,   NULL },
  { "&MIDI Info...",        onMIDICredits, NULL },
  { "&Video Mode...",       onVidMode,   NULL },
  { "" },
  { "&Quest Info...",       onQuest,     NULL },
  { "Quest &Dir...",        onQstPath,   NULL },
  { "" },
  { "Take &Snapshot\tF12",  onSnapshot, NULL },
  { "Sc&reen Saver...",     onScreenSaver, NULL },
  { NULL }
};

static MENU refill_menu[] =
{
  { "Life\t*",            onRefillLife,    NULL },
  { "Magic\t\\",          onRefillMagic,    NULL },
  { NULL }
};



static MENU cheat_menu[] =
{
  { "Enter code...",        onCheat,     NULL },
  { "" },
  { "Refill",               NULL,    refill_menu },
  { "Bombs\tb",             onCheatBombs,  NULL },
  { "Rupies\tr",            onCheatRupies, NULL },
  { "" },
  { "Clock\ti",             onClock,     NULL },
  { "Max Bombs...",         onMaxBombs,  NULL },
  { "H.Containers...",      onHeartC,    NULL },
  { "" },
  { "Items...",             onItems,     NULL },
  { "Equipment...",         onEquipment, NULL },
  { "Triforce...",          onTriforce,  NULL },
  { "" },
  { "No walls\tF11",        onNoWalls,   NULL },
  { "Quick Movement\tq",    onGoFast,    NULL },
                                                            //   { "Goto Location\t[g]",   onGoTo,      NULL },
  { NULL }
};

MENU the_menu[] =
{
  { "&Game",                NULL,        game_menu },
  { "&Settings",            NULL,        settings_menu },
  { "&Cheat",               NULL,        cheat_menu },
  { "&Misc",                NULL,        misc_menu },
  { NULL }
};

MENU the_menu2[] =
{
  { "&Game",                NULL,        game_menu },
  { "&Settings",            NULL,        settings_menu },
  { "&Misc",                NULL,        misc_menu },
  { NULL }
};

void fix_menu()
{
  if(!debug_enabled)
    settings_menu[12].text = NULL;
}

static DIALOG system_dlg[] =
{
                                                            /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)  (d1)      (d2)     (dp) */
  { jwin_menu_proc,    0,    0,    0,    0,    0,    0,    0,       D_USER,  0,        0,       (void *) the_menu },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onVsync },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F2,   0,       (void *) onShowFPS },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F6,   0,       (void *) onQuit },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F7,   0,       (void *) onReset },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F8,   0,       (void *) onExit },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,  0,       (void *) onSnapshot },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_TAB,  0,       (void *) onDebug },
  { NULL }
};

static DIALOG system_dlg2[] =
{
                                                            /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)  (d1)      (d2)     (dp) */
  { jwin_menu_proc,    0,    0,    0,    0,    0,    0,    0,       D_USER,  0,        0,       (void *) the_menu2 },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onVsync },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F2,   0,       (void *) onShowFPS },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F6,   0,       (void *) onQuit },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F7,   0,       (void *) onReset },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F8,   0,       (void *) onExit },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,  0,       (void *) onSnapshot },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_TAB,  0,       (void *) onDebug },
  { NULL }
};

void color_layer(RGB *src,RGB *dest,char r,char g,char b,char pos,int from,int to)
{
  PALETTE tmp;
  for(int i=0; i<256; i++)
  {
    tmp[i].r=r;
    tmp[i].g=g;
    tmp[i].b=b;
  }
  fade_interpolate(src,tmp,dest,pos,from,to);
}


void system_pal()
{
  PALETTE pal;
  copy_pal((RGB*)data[PAL_GUI].dat, pal);

  // set up the grayscale palette
  for(int i=128; i<192; i++)
  {
    pal[i].r = i-128;
    pal[i].g = i-128;
    pal[i].b = i-128;
  }


  switch (gui_colorset)
  {
    /*
    enum
    {
      jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
      jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
      jcMAX
    };
    */
    case 1:  //Windows 98
      {
        pal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB(128*63/255, 128*63/255, 128*63/255);
        pal[dvc(3)] = _RGB(192*63/255, 192*63/255, 192*63/255);
        pal[dvc(4)] = _RGB(223*63/255, 223*63/255, 223*63/255);
        pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

        byte palrstart=  0*63/255, palrend=166*63/255,
             palgstart=  0*63/255, palgend=202*63/255,
             palbstart=128*63/255, palbend=240*63/255,
             paldivs=7;
        for(int i=0; i<paldivs; i++)
        {
          pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }

        jwin_pal[jcBOX]    =dvc(3);
        jwin_pal[jcLIGHT]  =dvc(5);
        jwin_pal[jcMEDLT]  =dvc(4);
        jwin_pal[jcMEDDARK]=dvc(2);
        jwin_pal[jcDARK]   =dvc(1);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(9);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(7);
        jwin_pal[jcTEXTBG] =dvc(5);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(8);
        jwin_pal[jcSELFG]  =dvc(6);
      }
      break;
    case 2:  //Windows 99
      {
        pal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  64*63/255);
        pal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
        pal[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
        pal[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(9)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

        byte palrstart=  0*63/255, palrend=166*63/255,
             palgstart=  0*63/255, palgend=202*63/255,

             palbstart=128*63/255, palbend=240*63/255,
             paldivs=6;
        for(int i=0; i<paldivs; i++)
        {
          pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }

        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(6);
        jwin_pal[jcMEDLT]  =dvc(5);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(10);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(8);
        jwin_pal[jcTEXTBG] =dvc(6);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(9);
        jwin_pal[jcSELFG]  =dvc(7);
      }
      break;
    case 3:  //Windows 2000 Blue
      {
        pal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB( 16*63/255,  15*63/255, 116*63/255);
        pal[dvc(3)] = _RGB( 82*63/255,  80*63/255, 182*63/255);
        pal[dvc(4)] = _RGB(162*63/255, 158*63/255, 250*63/255);
        pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 127*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 225*63/255,  63*63/255);
        pal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

        byte palrstart=  0*63/255, palrend=162*63/255,
             palgstart=  0*63/255, palgend=158*63/255,
             palbstart= 80*63/255, palbend=250*63/255,
             paldivs=7;
        for(int i=0; i<paldivs; i++)
        {
          pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }

        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(5);
        jwin_pal[jcMEDLT]  =dvc(4);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(9);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(7);
        jwin_pal[jcTEXTBG] =dvc(5);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(8);
        jwin_pal[jcSELFG]  =dvc(6);
      }
      break;
    case 687:  //Windows 2000 Gold (6-87 was the North American release date of LoZ)
      {
        pal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  43*63/255);
        pal[dvc(3)] = _RGB(128*63/255, 128*63/255,  85*63/255);
        pal[dvc(4)] = _RGB(192*63/255, 192*63/255, 128*63/255);
        pal[dvc(5)] = _RGB(223*63/255, 223*63/255, 149*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(9)] = _RGB( 80*63/255,  80*63/255,   0*63/255);

        byte palrstart=128*63/255, palrend=240*63/255,
             palgstart=128*63/255, palgend=202*63/255,
             palbstart=  0*63/255, palbend=166*63/255,
             paldivs=6;
        for(int i=0; i<paldivs; i++)
        {
          pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }

        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(6);
        jwin_pal[jcMEDLT]  =dvc(5);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(10);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(8);
        jwin_pal[jcTEXTBG] =dvc(6);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(9);
        jwin_pal[jcSELFG]  =dvc(7);
      }
      break;
    case 4104:  //Windows 2000 Easter (4-1-04 is April Fools Day, the date of this release)
      {
        pal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  64*63/255);
        pal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
        pal[dvc(4)] = _RGB(252*63/255, 186*63/255, 188*63/255);
        pal[dvc(5)] = _RGB(254*63/255, 238*63/255, 238*63/255);
        pal[dvc(6)] = _RGB(244*63/255, 243*63/255, 161*63/255);
        pal[dvc(7)] = _RGB(120*63/255, 173*63/255, 189*63/255);
        pal[dvc(8)] = _RGB(220*63/255, 183*63/255, 227*63/255);

        byte palrstart=244*63/255, palrend=220*63/255,
             palgstart=243*63/255, palgend=183*63/255,
             palbstart=161*63/255, palbend=227*63/255,
             paldivs=7;
        for(int i=0; i < paldivs; i++)
        {
          pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }

        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(5);
        jwin_pal[jcMEDLT]  =dvc(4);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(7);
        jwin_pal[jcTITLEL] =dvc(9);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(7);
        jwin_pal[jcTEXTBG] =dvc(5);
        jwin_pal[jcTEXTFG] =dvc(7);
        jwin_pal[jcSELBG]  =dvc(8);
        jwin_pal[jcSELFG]  =dvc(6);
      }
      break;
    default:  //Windows 2000
      {
        pal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
        pal[dvc(2)] = _RGB( 66*63/255,  65*63/255,  66*63/255);
        pal[dvc(3)] = _RGB(132*63/255, 130*63/255, 132*63/255);
        pal[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
        pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
        pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
        pal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
        pal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

        byte palrstart= 10*63/255, palrend=166*63/255,
             palgstart= 36*63/255, palgend=202*63/255,
             palbstart=106*63/255, palbend=240*63/255,
             paldivs=7;
        for(int i=0; i<paldivs; i++)
        {
          pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
          pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
        }

        jwin_pal[jcBOX]    =dvc(4);
        jwin_pal[jcLIGHT]  =dvc(5);
        jwin_pal[jcMEDLT]  =dvc(4);
        jwin_pal[jcMEDDARK]=dvc(3);
        jwin_pal[jcDARK]   =dvc(2);
        jwin_pal[jcBOXFG]  =dvc(1);
        jwin_pal[jcTITLEL] =dvc(9);
        jwin_pal[jcTITLER] =dvc(15);
        jwin_pal[jcTITLEFG]=dvc(7);
        jwin_pal[jcTEXTBG] =dvc(5);
        jwin_pal[jcTEXTFG] =dvc(1);
        jwin_pal[jcSELBG]  =dvc(8);
        jwin_pal[jcSELFG]  =dvc(6);
      }
      break;
  }

  gui_bg_color=jwin_pal[jcBOX];
  gui_fg_color=jwin_pal[jcBOXFG];
  gui_mg_color=jwin_pal[jcMEDDARK];

  jwin_set_colors(jwin_pal);

  color_layer(pal, pal, 24,16,16, 28, 128,191);

  for(int i=0; i<256; i+=2)
  {
    int v = (i>>3)+2;
    int c = (i>>3)+192;
    pal[c] = _RGB(v,v,v+(v>>1));
/*
    if(i<240)
    {
      hline(tmp_scr,0,i,319,c);
      hline(tmp_scr,0,i+1,319,c);
    }
*/
  }

  // draw the vertical screen gradient
  for(int i=0; i<240; ++i)
  {
    hline(tmp_scr,0,i,319,192+(i*31/239));
  }
/*
   palrstart= 10*63/255; palrend=166*63/255;
   palgstart= 36*63/255; palgend=202*63/255;
   palbstart=106*63/255; palbend=240*63/255;
   paldivs=32;
   for(int i=0; i<paldivs; i++)
   {
     pal[223-paldivs+1+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
     pal[223-paldivs+1+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
     pal[223-paldivs+1+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
   }
*/
  BITMAP *panorama = create_bitmap_ex(8,256,224);
  int ts_height, ts_start;
  if (tmpscr->flags3&fNOSUBSCR)
  {
    clear_to_color(panorama,0);
    blit(framebuf,panorama,0,56,0,28,256,224-56);
    ts_height=224-56;
    ts_start=28;
  }
  else
  {
    blit(framebuf,panorama,0,0,0,0,256,224);
    ts_height=224;
    ts_start=0;
  }

  // gray scale the current frame
  for(int y=0; y<ts_height; y++)
  {
    for(int x=0; x<256; x++)
    {
      int c = panorama->line[y+ts_start][x];
      int gray = min((RAMpal[c].r*42 + RAMpal[c].g*75 + RAMpal[c].b*14) >> 7, 63);
      tmp_scr->line[y+8+ts_start][x+32] = gray+128;
    }
  }
  destroy_bitmap(panorama);

                                                            // save the fps_undo section
  blit(tmp_scr,fps_undo,40,216,0,0,64,16);

                                                            // display everything
  vsync();
  set_palette_range(pal,0,255,false);

  if(sbig)
    stretch_blit(tmp_scr,screen,0,0,320,240,scrx-160,scry-120,640,480);
  else
    blit(tmp_scr,screen,0,0,scrx,scry,320,240);

  if(ShowFPS)
    show_fps();
  if(Paused)
    show_paused();

                                                            //  sys_pal = pal;
  memcpy(sys_pal,pal,sizeof(pal));
}


void system_pal2()
{
  PALETTE RAMpal;
  copy_pal((RGB*)data[PAL_GUI].dat, RAMpal);

/* Windows 2000 colors
  RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
  RAMpal[dvc(2)] = _RGB( 66*63/255,  65*63/255,  66*63/255);
  RAMpal[dvc(3)] = _RGB(132*63/255, 130*63/255, 132*63/255);
  RAMpal[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
  RAMpal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
  RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
  RAMpal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
  RAMpal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

  byte palrstart= 10*63/255, palrend=166*63/255,
       palgstart= 36*63/255, palgend=202*63/255,
       palbstart=106*63/255, palbend=240*63/255,
       paldivs=7;
  for(int i=0; i<paldivs; i++)
  {
    RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
  }
*/

/* Windows 98 colors
  RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
  RAMpal[dvc(2)] = _RGB(128*63/255, 128*63/255, 128*63/255);
  RAMpal[dvc(3)] = _RGB(192*63/255, 192*63/255, 192*63/255);
  RAMpal[dvc(4)] = _RGB(223*63/255, 223*63/255, 223*63/255);
  RAMpal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
  RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
  RAMpal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
  RAMpal[dvc(8)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

  byte palrstart=  0*63/255, palrend=166*63/255,
       palgstart=  0*63/255, palgend=202*63/255,
       palbstart=128*63/255, palbend=240*63/255,
       paldivs=7;
  for(int i=0; i<paldivs; i++)
  {
    RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
  }
*/

/* Windows 99 colors
  RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
  RAMpal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  64*63/255);
  RAMpal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
  RAMpal[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
  RAMpal[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
  RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
  RAMpal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
  RAMpal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
  RAMpal[dvc(9)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

  byte palrstart=  0*63/255, palrend=166*63/255,
       palgstart=  0*63/255, palgend=202*63/255,

       palbstart=128*63/255, palbend=240*63/255,
       paldivs=6;
  for(int i=0; i<paldivs; i++)
  {
    RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
  }
*/



  RAMpal[dvc(1)] = _RGB(  0*63/255,   0*63/255,   0*63/255);
  RAMpal[dvc(2)] = _RGB( 64*63/255,  64*63/255,  64*63/255);
  RAMpal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
  RAMpal[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
  RAMpal[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
  RAMpal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
  RAMpal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
  RAMpal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
  RAMpal[dvc(9)] = _RGB(  0*63/255,   0*63/255,  80*63/255);

  byte palrstart=  0*63/255, palrend=166*63/255,
       palgstart=  0*63/255, palgend=202*63/255,
       palbstart=128*63/255, palbend=240*63/255,
       paldivs=6;
  for(int i=0; i<paldivs; i++)
  {
    RAMpal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
    RAMpal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
  }

  gui_bg_color=jwin_pal[jcBOX];
  gui_fg_color=jwin_pal[jcBOXFG];
  gui_mg_color=jwin_pal[jcMEDDARK];

  jwin_set_colors(jwin_pal);


  // set up the new palette
  for(int i=128; i<192; i++)
  {
    RAMpal[i].r = i-128;
    RAMpal[i].g = i-128;
    RAMpal[i].b = i-128;
  }

/*
  for(int i=0; i<64; i++)
  {
    RAMpal[128+i] = _RGB(i,i,i)1));
  }
*/

/*

  pal[vc(1)]  = _RGB(0x00,0x00,0x14);
  pal[vc(4)]  = _RGB(0x36,0x36,0x36);
  pal[vc(6)]  = _RGB(0x10,0x10,0x10);
  pal[vc(7)]  = _RGB(0x20,0x20,0x20);
  pal[vc(9)]  = _RGB(0x20,0x20,0x24);
  pal[vc(11)] = _RGB(0x30,0x30,0x30);
  pal[vc(14)] = _RGB(0x3F,0x38,0x28);

  gui_fg_color=vc(14);
  gui_bg_color=vc(1);
  gui_mg_color=vc(9);

  jwin_set_colors(jwin_pal);
*/

//  color_layer(RAMpal, RAMpal, 24,16,16, 28, 128,191);

  // set up the colors for the vertical screen gradient
  for(int i=0; i<256; i+=2)
  {
    int v = (i>>3)+2;
    int c = (i>>3)+192;
    RAMpal[c] = _RGB(v,v,v+(v>>1));

/*
    if(i<240)
    {
      hline(tmp_scr,0,i,319,c);
      hline(tmp_scr,0,i+1,319,c);
    }
*/
  }

  set_palette(RAMpal);

  for(int i=0; i<240; ++i)
  {
    hline(tmp_scr,0,i,319,192+(i*31/239));
  }
/*
  byte palrstart= 10*63/255, palrend=166*63/255,
        palgstart= 36*63/255, palgend=202*63/255,
        palbstart=106*63/255, palbend=240*63/255,
        paldivs=32;
   for(int i=0; i<paldivs; i++)
   {
     pal[223-paldivs+1+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
     pal[223-paldivs+1+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
     pal[223-paldivs+1+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
   }
*/
  BITMAP *panorama = create_bitmap_ex(8,256,224);
  int ts_height, ts_start;
  if (tmpscr->flags3&fNOSUBSCR)
  {
    clear_to_color(panorama,0);
    blit(framebuf,panorama,0,56,0,28,256,224-56);
    ts_height=224-56;
    ts_start=28;
  }
  else
  {
    blit(framebuf,panorama,0,0,0,0,256,224);
    ts_height=224;
    ts_start=0;
  }

  // gray scale the current frame
  for(int y=0; y<ts_height; y++)
  {
    for(int x=0; x<256; x++)
    {
      int c = panorama->line[y+ts_start][x];
      int gray = min((RAMpal[c].r*42 + RAMpal[c].g*75 + RAMpal[c].b*14) >> 7, 63);
      tmp_scr->line[y+8+ts_start][x+32] = gray+128;
    }
  }
  destroy_bitmap(panorama);

                                                            // save the fps_undo section
  blit(tmp_scr,fps_undo,40,216,0,0,64,16);

                                                            // display everything
  vsync();
  set_palette_range(RAMpal,0,255,false);

  if(sbig)
    stretch_blit(tmp_scr,screen,0,0,320,240,scrx-160,scry-120,640,480);
  else
    blit(tmp_scr,screen,0,0,scrx,scry,320,240);

  if(ShowFPS)
    show_fps();
  if(Paused)
    show_paused();

                                                            //  sys_pal = pal;
  memcpy(sys_pal,RAMpal,sizeof(RAMpal));
}

void game_pal()
{
                                                            /*
    clear_to_color(tmp_scr,BLACK);

    BITMAP *panorama = create_bitmap_ex(8,256,224);
    if (tmpscr->flags3&fNOSUBSCR) {
      clear_to_color(panorama,0);
      blit(framebuf,panorama,0,56,0,28,256,224-56);
    } else {
      blit(framebuf,panorama,0,0,0,0,256,224);
    }
    blit(panorama,tmp_scr,0,0,scrx+32,scry+8,256,224);
  destroy_bitmap(panorama);

  //  blit(framebuf,tmp_scr,0,0,32,8,256,224);
  vsync();
  if(sbig)
  stretch_blit(tmp_scr,screen,0,0,320,240,scrx-160,scry-120,640,480);
  else
  blit(tmp_scr,screen,0,0,scrx,scry,320,240);
  */
  clear_to_color(screen,BLACK);
  set_palette_range(RAMpal,0,255,false);
}

static char bar_str[] = "";

void music_pause()
{
                                                            //al_pause_duh(tmplayer);
  zcmusic_pause(zcmusic, ZCM_PAUSE);
  midi_pause();
}

void music_resume()
{
                                                            //al_resume_duh(tmplayer);
  zcmusic_pause(zcmusic, ZCM_RESUME);
  midi_resume();
}

void music_stop()
{
                                                            //al_stop_duh(tmplayer);
                                                            //unload_duh(tmusic);
                                                            //tmusic=NULL;
                                                            //tmplayer=NULL;
  zcmusic_stop(zcmusic);
  zcmusic_unload_file(zcmusic);
  stop_midi();
}

void System()
{
  mouse_down=gui_mouse_b();
  music_pause();
  pause_all_sfx();

  system_pal();
                                                            //  FONT *oldfont=font;
                                                            //  font=tfont;

  game_menu[2].flags =
    misc_menu[5].flags = Playing ? 0 : D_DISABLED;
  misc_menu[6].flags = !Playing ? 0 : D_DISABLED;

  clear_keybuf();
  show_mouse(screen);

  DIALOG_PLAYER *p;

  if(!Playing || (!zcheats.flags && !debug))
  {
    p = init_dialog(system_dlg2,-1);
  }
  else
  {
    p = init_dialog(system_dlg,-1);
  }
                                                            // drop the menu on startup if menu button pressed
  if(joybtn(Mbtn))
    simulate_keypress(KEY_G << 8);

  do
  {
    if(mouse_down && !gui_mouse_b())
      mouse_down=0;

    title_menu[0].flags = DXtitle ? 0 : D_SELECTED;
    title_menu[1].flags = title_menu[0].flags ^ D_SELECTED;

    settings_menu[6].flags = Throttlefps?D_SELECTED:0;
    settings_menu[7].flags = ShowFPS?D_SELECTED:0;
    settings_menu[8].flags = TransLayers?D_SELECTED:0;
    settings_menu[9].flags = NESquit?D_SELECTED:0;
    settings_menu[10].flags = volkeys?D_SELECTED:0;

                                                            /*
        if(!Playing || (!zcheats.flags && !debug))
        {
          cheat_menu[0].flags = D_DISABLED;
          cheat_menu[1].text  = NULL;
        }
        else */
    {
      cheat_menu[0].flags = 0;
      cheat_menu[1].text  = (cheat >= 1) || debug ? bar_str : NULL;
      cheat_menu[5].text  = (cheat >= 2) || debug ? bar_str : NULL;
      cheat_menu[9].text  = (cheat >= 3) || debug ? bar_str : NULL;
      cheat_menu[13].text = (cheat >= 4) || debug ? bar_str : NULL;
      cheat_menu[6].flags = getClock() ? D_SELECTED : 0;
      cheat_menu[14].flags = toogam ? D_SELECTED : 0;
      cheat_menu[15].flags = gofast ? D_SELECTED : 0;
    }

    settings_menu[11].flags = heart_beep ? D_SELECTED : 0;
    if(debug_enabled)
    {
      settings_menu[13].flags = debug ? D_SELECTED : 0;
    }
    //settings_menu[12].flags = FrameSkip ? D_SELECTED : 0;

    if(gui_mouse_b() && !mouse_down)
      break;

                                                            // press menu to drop the menu
    if(rMbtn())
      simulate_keypress(KEY_G << 8);

    if(myvsync)
    {
      myvsync = 0;
      if(input_idle(true) > after_time())
                                                            // run Screeen Saver
      {
        while(keypressed())
          readkey();
        scare_mouse();
        Matrix(ss_speed, ss_density, 0);
        system_pal();
        unscare_mouse();
        broadcast_dialog_message(MSG_DRAW, 0);
      }
    }

  } while(update_dialog(p));

                                                            //  font=oldfont;
  mouse_down=gui_mouse_b();
  shutdown_dialog(p);
  show_mouse(NULL);

  if(Quit)
  {
    kill_sfx();
    music_stop();
    clear_to_color(screen,BLACK);
  }
  else
  {
    game_pal();
    music_resume();
    resume_all_sfx();
    if(rc)
      ringcolor();
  }

  eat_buttons();

  rc=false;
  clear_keybuf();
                                                            //  text_mode(0);
}

void fix_dialog(DIALOG *d)
{
  for( ; d->proc != NULL; d++)
  {
    d->x += scrx;
    d->y += scry;
  }
}

void fix_dialogs()
{
  int x = scrx-(sbig?160:0);
  int y = scry-(sbig?120:0);
  if(x>0) x+=3;
  if(y>0) y+=3;
  if(x<0) x=0;
  if(y<0) y=0;

  system_dlg[0].x = x;
  system_dlg[0].y = y;
  system_dlg2[0].x = x;
  system_dlg2[0].y = y;

//  fix_dialog(key_dlg);
//  fix_dialog(keydir_dlg);
//  fix_dialog(btn_dlg);
//  fix_dialog(sound_dlg);
//  fix_dialog(about_dlg);
//  fix_dialog(quest_dlg);
//  fix_dialog(triforce_dlg);
//  fix_dialog(equip_dlg);
//  fix_dialog(items_dlg);
//  fix_dialog(gamemode_dlg);
//  fix_dialog(getnum_dlg);
//  fix_dialog(credits_dlg);
//  fix_dialog(midi_dlg);
//  fix_dialog(cheat_dlg);
//  fix_dialog(scrsaver_dlg);
  jwin_center_dialog(about_dlg);
  jwin_center_dialog(btn_dlg);
  jwin_center_dialog(cheat_dlg);
  jwin_center_dialog(credits_dlg);
  jwin_center_dialog(equip_dlg);
  jwin_center_dialog(gamemode_dlg);
  jwin_center_dialog(getnum_dlg);
  jwin_center_dialog(items_dlg);
  jwin_center_dialog(key_dlg);
  jwin_center_dialog(keydir_dlg);
  jwin_center_dialog(midi_dlg);
  jwin_center_dialog(quest_dlg);
  jwin_center_dialog(scrsaver_dlg);
  jwin_center_dialog(sound_dlg);
  jwin_center_dialog(triforce_dlg);

  digi_dp[1] += scrx;
  digi_dp[2] += scry;
  midi_dp[1] += scrx;
  midi_dp[2] += scry;
  pan_dp[1]  += scrx;
  pan_dp[2]  += scry;
}

/*****************************/
/**** Custom Sound System ****/
/*****************************/

inline int mixvol(int v1,int v2)
{
  return (min(v1,255)*min(v2,255)) >> 8;
}

void jukebox(int index,int loop)
{
  music_stop();
  if(index<0)         index=MAXMUSIC-1;
  if(index>=MAXMUSIC) index=0;

  music_stop();
  set_volume(-1, mixvol(tunes[index].volume,midi_volume>>1));
  play_midi(tunes[index].midi,loop);
  if(tunes[index].start>0)
    midi_seek(tunes[index].start);

  midi_loop_end = tunes[index].loop_end;
  midi_loop_start = tunes[index].loop_start;

  currmidi=index;
  master_volume(digi_volume,midi_volume);
}

void jukebox(int index)
{
  if(index<0)         index=MAXMUSIC-1;
  if(index>=MAXMUSIC) index=0;

  // do nothing if it's already playing
  if(index==currmidi && midi_pos>=0)
    return;

  jukebox(index,tunes[index].loop);
}

void play_DmapMusic()
{
  static char tfile[2048];
  bool domidi=false;
  if (DMaps[currdmap].tmusic[0]!=0)
  {
    if((zcmusic==NULL) || (strcmp(tfile,DMaps[currdmap].tmusic)!=0))
    {
      char tmfname[2048];

      if(zcmusic != NULL)
      {
        zcmusic_stop(zcmusic);
        zcmusic_unload_file(zcmusic);
        zcmusic = NULL;
      }

      sprintf(tmfname, "%s",qstdir);
      chop_path(tmfname);
      strcat(tmfname, DMaps[currdmap].tmusic);

      zcmusic = (ZCMUSIC*)zcmusic_load_file(tmfname);

      if (zcmusic!=NULL)
      {
        stop_midi();
        strcpy(tfile,DMaps[currdmap].tmusic);
        zcmusic_play(zcmusic, midi_volume);
      }
      else
      {
        tfile[0] = 0;
        domidi=true;
      }
    }
  }
  else
  {
    domidi=true;
  }

  if (domidi)
  {
    int m=DMaps[currdmap].midi;
    switch(m)
    {
      case 1: jukebox(MUSIC_OVERWORLD); break;
      case 2: jukebox(MUSIC_DUNGEON); break;
      case 3: jukebox(MUSIC_LEVEL9); break;
      default:
        if(m>=4 && m<4+MAXMIDIS)
          jukebox(m-4+MUSIC_COUNT);
        else
          music_stop();
    }
  }
}

void master_volume(int dv,int mv)
{
  if(dv>=0) digi_volume=max(min(dv,255),0);
  if(mv>=0) midi_volume=max(min(mv,255),0);
  int i = min(max(currmidi,0),MAXMUSIC-1);
  set_volume(digi_volume,mixvol(tunes[i].volume,midi_volume));
}

                                                            /*****************/
                                                            /*****  SFX  *****/
                                                            /*****************/

                                                            // array of voices, one for each sfx sample in the data file
                                                            // 0+ = voice #
                                                            // -1 = voice not allocated
static int sfx_voice[WAV_COUNT];

void Z_init_sound()
{
  for(int i=0; i<WAV_COUNT; i++)
    sfx_voice[i]=-1;
  for(int i=0; i<MUSIC_COUNT; i++)
    tunes[i].midi = (MIDI*)mididata[i].dat;
  master_volume(digi_volume,midi_volume);
}

                                                            // returns number of voices currently allocated
int sfx_count()
{
  int c=0;
  for(int i=0; i<WAV_COUNT; i++)
    if(sfx_voice[i]!=-1)
      ++c;
  return c;
}

                                                            // clean up finished samples
void sfx_cleanup()
{
  for(int i=0; i<WAV_COUNT; i++)
    if(sfx_voice[i]!=-1 && voice_get_position(sfx_voice[i])<0)
  {
    deallocate_voice(sfx_voice[i]);
    sfx_voice[i]=-1;
  }
}

                                                            // allocates a voice for the sample "wav_index" (index into zelda.dat)
                                                            // if a voice is already allocated (and/or playing), then it just returns true
                                                            // Returns true:  voice is allocated
                                                            //         false: unsuccessful
bool sfx_init(int index)
{
                                                            // check index
  if(index<0 || index>=WAV_COUNT)
    return false;

  if(sfx_voice[index]==-1)
  {
                                                            // allocate voice
                                                            /*
        if(index!=WAV_REFILL)
          sfx_voice[index]=allocate_voice((SAMPLE*)sfxdata[index].dat);
        else
          sfx_voice[index]=allocate_voice(&wav_refill);
    */
    sfx_voice[index]=allocate_voice((SAMPLE*)sfxdata[index].dat);
  }

  return sfx_voice[index] != -1;
}

                                                            // plays an sfx sample
void sfx(int index,int pan,bool loop)
{
  if(!sfx_init(index))
    return;

  voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
  voice_set_pan(sfx_voice[index],pan);

  int pos = voice_get_position(sfx_voice[index]);
  voice_set_position(sfx_voice[index],0);
  if(pos<=0)
    voice_start(sfx_voice[index]);
}

                                                            // start it (in loop mode) if it's not already playing,
                                                            // otherwise just leave it in its current position
void cont_sfx(int index)
{
  if(!sfx_init(index))
    return;

  if(voice_get_position(sfx_voice[index])<=0)
  {
    voice_set_position(sfx_voice[index],0);
    voice_set_playmode(sfx_voice[index],PLAYMODE_LOOP);
    voice_start(sfx_voice[index]);
  }
}

                                                            // adjust parameters while playing
void adjust_sfx(int index,int pan,bool loop)
{
  if(index<0 || index>=WAV_COUNT || sfx_voice[index]==-1)
    return;

  voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
  voice_set_pan(sfx_voice[index],pan);
}

                                                            // pauses a voice
void pause_sfx(int index)
{
  if(index>=0 && index<WAV_COUNT && sfx_voice[index]!=-1)
    voice_stop(sfx_voice[index]);
}

                                                            // resumes a voice
void resume_sfx(int index)
{
  if(index>=0 && index<WAV_COUNT && sfx_voice[index]!=-1)
    voice_start(sfx_voice[index]);
}

                                                            // pauses all active voices
void pause_all_sfx()
{
  for(int i=0; i<WAV_COUNT; i++)
    if(sfx_voice[i]!=-1)
      voice_stop(sfx_voice[i]);
}

                                                            // resumes all paused voices
void resume_all_sfx()
{
  for(int i=0; i<WAV_COUNT; i++)
    if(sfx_voice[i]!=-1)
      voice_start(sfx_voice[i]);
}

                                                            // stops an sfx and deallocates the voice
void stop_sfx(int index)
{
  if(index<0 || index>=WAV_COUNT)
    return;

  if(sfx_voice[index]!=-1)
  {
    deallocate_voice(sfx_voice[index]);
    sfx_voice[index]=-1;
  }
}

void kill_sfx()
{
  for(int i=0; i<WAV_COUNT; i++)
    if(sfx_voice[i]!=-1)
  {
    deallocate_voice(sfx_voice[i]);
    sfx_voice[i]=-1;
  }
}

int pan(int x)
{
  switch(pan_style)
  {
    case 0: return 128;
    case 1: return vbound((x>>1)+68,0,255);
    case 2: return vbound(((x*3)>>2)+36,0,255);
  }
  return vbound(x,0,255);
}

                                                            /*******************************/
                                                            /******* Input Handlers ********/
                                                            /*******************************/

bool joybtn(int b)
{
  return joy[0].button[b-1].b;
                                                            /*
    switch(b)
    {
    case 1: return joy_b1;
    case 2: return joy_b2;
    case 3: return joy_b3;
    case 4: return joy_b4;
    case 5: return joy_b5;
    case 6: return joy_b6;
    case 7: return joy_b7;
    case 8: return joy_b8;
  }
  */
  return false;
}

int next_press_key()
{
  char k[128];
  for(int i=0; i<128; i++)
    k[i]=key[i];
  do
  {
    for(int i=0; i<128; i++)
      if(key[i]!=k[i])
        return i;
  } while(1);
}

int next_press_btn()
{
  clear_keybuf();
  bool b[9];
  for(int i=1; i<=8; i++)
    b[i]=joybtn(i);

  while(1)
  {
    if(keypressed())
    {
      switch(readkey()>>8)
      {
        case KEY_ESC:   return -1;
        case KEY_SPACE: return 0;
      }
    }
    poll_joystick();
    for(int i=1; i<=8; i++)
    {
      if(!joybtn(i))
        b[i]=0;
      else if(b[i]==0)
        return i;
    }
  }
}

static bool rButton(bool(proc)(),bool &flag)
{
  if(!proc())
    flag=false;
  else if(!flag)
  {
    flag=true;
    return true;
  }
  return false;
}

bool Up()     { return key[DUkey]||joy[0].stick[0].axis[1].d1; }
bool Down()   { return key[DDkey]||joy[0].stick[0].axis[1].d2; }
bool Left()   { return key[DLkey]||joy[0].stick[0].axis[0].d1; }
bool Right()  { return key[DRkey]||joy[0].stick[0].axis[0].d2; }
bool cAbtn()  { return key[Akey]||joybtn(Abtn); }
bool cBbtn()  { return key[Bkey]||joybtn(Bbtn); }
bool cSbtn()  { return key[Skey]||joybtn(Sbtn); }
bool cMbtn()  { return key[KEY_ESC]||joybtn(Mbtn); }
bool cLbtn()  { return key[Lkey]||joybtn(Lbtn); }
bool cRbtn()  { return key[Rkey]||joybtn(Rbtn); }
bool cPbtn()  { return key[Pkey]||joybtn(Pbtn); }

bool rUp()    { return rButton(Up,Udown); }
bool rDown()  { return rButton(Down,Ddown); }
bool rLeft()  { return rButton(Left,Ldown); }
bool rRight() { return rButton(Right,Rdown); }
bool rAbtn()  { return rButton(cAbtn,Adown); }
bool rBbtn()  { return rButton(cBbtn,Bdown); }
bool rSbtn()  { return rButton(cSbtn,Sdown); }
bool rMbtn()  { return rButton(cMbtn,Mdown); }
bool rLbtn()  { return rButton(cLbtn,LBdown); }
bool rRbtn()  { return rButton(cRbtn,RBdown); }
bool rPbtn()  { return rButton(cPbtn,Pdown); }

bool drunk()
{
                                                            //  return ((!(frame%((rand()%100)+1)))&&(rand()%MAXDRUNKCLOCK<Link.DrunkClock()));
  return false;
}

bool DrunkUp()     { return drunk()?(rand()%2)?0:!Up():Up(); }
bool DrunkDown()   { return drunk()?(rand()%2)?0:!Down():Down(); }
bool DrunkLeft()   { return drunk()?(rand()%2)?0:!Left():Left(); }
bool DrunkRight()  { return drunk()?(rand()%2)?0:!Right():Right(); }
bool DrunkcAbtn()  { return drunk()?(rand()%2)?0:!cAbtn():cAbtn(); }
bool DrunkcBbtn()  { return drunk()?(rand()%2)?0:!cBbtn():cBbtn(); }
bool DrunkcSbtn()  { return drunk()?(rand()%2)?0:!cSbtn():cSbtn(); }
bool DrunkcMbtn()  { return drunk()?(rand()%2)?0:!cMbtn():cMbtn(); }
bool DrunkcLbtn()  { return drunk()?(rand()%2)?0:!cLbtn():cLbtn(); }
bool DrunkcRbtn()  { return drunk()?(rand()%2)?0:!cRbtn():cRbtn(); }
bool DrunkcPbtn()  { return drunk()?(rand()%2)?0:!cPbtn():cPbtn(); }

bool DrunkrUp()    { return drunk()?(rand()%2)?0:!rUp():rUp(); }
bool DrunkrDown()  { return drunk()?(rand()%2)?0:!rDown():rDown(); }
bool DrunkrLeft()  { return drunk()?(rand()%2)?0:!rLeft():rLeft(); }
bool DrunkrRight() { return drunk()?(rand()%2)?0:!rRight():rRight(); }
bool DrunkrAbtn()  { return drunk()?(rand()%2)?0:!rAbtn():rAbtn(); }
bool DrunkrBbtn()  { return drunk()?(rand()%2)?0:!rBbtn():rBbtn(); }
bool DrunkrSbtn()  { return drunk()?(rand()%2)?0:!rSbtn():rSbtn(); }
bool DrunkrMbtn()  { return drunk()?(rand()%2)?0:!rMbtn():rMbtn(); }
bool DrunkrLbtn()  { return drunk()?(rand()%2)?0:!rLbtn():rLbtn(); }
bool DrunkrRbtn()  { return drunk()?(rand()%2)?0:!rRbtn():rRbtn(); }
bool DrunkrPbtn()  { return drunk()?(rand()%2)?0:!rPbtn():rPbtn(); }

void eat_buttons()
{
  rAbtn();
  rBbtn();
  rSbtn();
  rMbtn();
  rLbtn();
  rRbtn();
  rPbtn();
}

bool ReadKey(int k)
{
  if(key[k])
  {
    key[k]=0;
    return true;
  }
  return false;
}

void zc_putpixel(int layer, int x, int y, int cset, int color, int timer)
{
  particles.add(new particle(fix(x), fix(y), layer, cset, color, timer));
}

                                                            /*** end of zc_sys.cc ***/
