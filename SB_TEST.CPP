#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>

#include "zc_alleg.h"
#include "scroll_bmp.h"

int screen_w=1024, screen_h=768, mode;
int close_dlg() { return D_CLOSE; }
PALETTE RAMpal;
BITMAP *bmp;

//#define vc(x)  ((x)+224)    // offset to 'VGA color' x

int vc(int x)
{
  switch (x)
  {
    case 0:                                                 //black
      return makecol(0, 0, 0);
      break;
    case 1:                                                 //blue
      return makecol(0, 0, 170);
      break;
    case 2:                                                 //green
      return makecol(0, 170, 0);
      break;
    case 3:                                                 //cyan
      return makecol(0, 170, 170);
      break;
    case 4:                                                 //red
      return makecol(170, 0, 0);
      break;
    case 5:                                                 //magenta
      return makecol(170, 0, 170);
      break;
    case 6:                                                 //brown
      return makecol(170, 85, 170);
      break;
    case 7:                                                 //light grey
      return makecol(170, 170, 170);
      break;
    case 8:                                                 //dark grey
      return makecol(85, 85, 85);
      break;
    case 9:                                                 //light blue
      return makecol(85, 85, 255);
      break;
    case 10:                                                //light green
      return makecol(85, 255, 85);
      break;
    case 11:                                                //light cyan
      return makecol(85, 255, 255);
      break;
    case 12:                                                //light red (pink)
      return makecol(255, 85, 85);
      break;
    case 13:                                                //light magenta (lavender)
      return makecol(255, 85, 255);
      break;
    case 14:                                                //light brown (yellow)
      return makecol(255, 255, 85);
      break;
    case 15:                                                //white
      return makecol(255, 255, 255);
      break;
  }
  return -1;
}

template <class T>
static inline void swap(T &a,T &b)
{
  T c = a;
  a = b;
  b = c;
}

int onSnapshot()
{
  char buf[20];
  int num=0;
  do
  {
    sprintf(buf, "sb%03d.bmp", ++num);
  } while(num<999 && exists(buf));

  save_bitmap(buf,screen,RAMpal);

  return D_O_K;
}

void Z_error(char *format,...)
{
  char buf[256];

  va_list ap;
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  #ifdef ALLEGRO_DOS
  printf("%s\n",buf);
  #elif defined(ALLEGRO_WINDOWS)
  al_trace("%s\n",buf);
  #elif defined(ALLEGRO_LINUX)
  al_trace("%s",buf);
  #endif
  exit(1);
}

void Z_message(char *format,...)
{
  char buf[256];

  va_list ap;
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  #ifdef ALLEGRO_DOS
  printf("%s",buf);
  #elif defined(ALLEGRO_WINDOWS)
  al_trace("%s",buf);
  #elif defined(ALLEGRO_LINUX)
  al_trace("%s",buf);
  #endif
}

int used_switch(int argc,char *argv[],char *s)
{
  // assumes a switch won't be in argv[0]
  for(int i=1; i<argc; i++)
    if(stricmp(argv[i],s)==0)
      return i;
  return 0;
}

/********************/
/******  Help  ******/
/********************/

static DIALOG help_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)          (h)          (fg)     (bg)      (key)    (flags)     (d1)      (d2)      (dp)    (dp2)   (dp3)*/
  { d_box_proc,        -1,   -1,   screen_w+2,  screen_h+2,  0,       0,        0,       0,          0,        0,        0,        NULL },
  { d_scroll_bmp_proc, 17,   33,   128,         100,         0,       0,        0,       0,          0,        0,        NULL },
  { d_keyboard_proc,   0,    0,    0,           0,           0,       0,        0,       0,          0,        KEY_ESC,  close_dlg },
  { d_keyboard_proc,   0,    0,    0,           0,           0,       0,        0,       0,          0,        KEY_F12,  onSnapshot },
  { NULL }
};

void doHelp(int bg,int fg)
{
  help_dlg[0].bg = bg;
  help_dlg[0].fg = fg;

  help_dlg[1].dp=bmp;
  (int*)(help_dlg[1].dp2)=0;
  help_dlg[1].bg=bg;
  help_dlg[1].fg=fg;

  popup_dialog(help_dlg,1);
}

int onHelp()
{
  doHelp(vc(15), vc(0));
  return D_O_K;
}

/********************/
/******  MAIN  ******/
/********************/

int main(int argc,char **argv)
{
  set_uformat(U_ASCII);
  Z_message("Initializing Allegro... ");

  allegro_init();

  if(install_timer() < 0)
    Z_error(allegro_error);

  if(install_keyboard() < 0)
    Z_error(allegro_error);

  if(install_mouse() < 0)
    Z_error(allegro_error);

  Z_message("OK\n");                                        // Initializing Allegro...

  set_color_depth(8);
  request_refresh_rate(60);

  #ifdef ALLEGRO_DOS
  mode = GFX_AUTODETECT;
  if(used_switch(argc,argv,"-modex"))  mode=GFX_MODEX;
  if(used_switch(argc,argv,"-vesa1"))  mode=GFX_VESA1;
  if(used_switch(argc,argv,"-vesa2b")) mode=GFX_VESA2B;
  if(used_switch(argc,argv,"-vesa2l")) mode=GFX_VESA2L;
  if(used_switch(argc,argv,"-vesa3"))  mode=GFX_VESA3;
  switch(mode)
  {
    case GFX_AUTODETECT:
    case GFX_VESA3:
      if(set_gfx_mode(GFX_VESA3,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    case GFX_VESA2L:
      if(set_gfx_mode(GFX_VESA2L,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    case GFX_VESA2B:
      if(set_gfx_mode(GFX_VESA2B,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    case GFX_VESA1:
      if(set_gfx_mode(GFX_VESA1,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    case GFX_MODEX:
      if(set_gfx_mode(GFX_MODEX,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    default:
      Z_message("Can't set video mode.\n");
      exit(1);
      break;
  }
  #elif defined(ALLEGRO_WINDOWS)
  int mode = GFX_AUTODETECT_FULLSCREEN;

  if(used_switch(argc,argv,"-fullscreen"))
  {
    if(used_switch(argc,argv,"-directx"))
    {
      mode=GFX_DIRECTX;
      if(used_switch(argc,argv,"-soft"))
      {
        mode=GFX_DIRECTX_SOFT;
      }
      else if(used_switch(argc,argv,"-safe"))
      {
        mode=GFX_DIRECTX_SAFE;
      }
    }
  }
  else if(used_switch(argc,argv,"-windowed"))
  {
    mode=GFX_AUTODETECT_WINDOWED;
    if(used_switch(argc,argv,"-directx"))
    {
      mode=GFX_DIRECTX_WIN;
    }
    else if(used_switch(argc,argv,"-gdi"))
    {
      mode=GFX_GDI;
    }
  }
  switch(mode)
  {
    case GFX_AUTODETECT_FULLSCREEN:
    case GFX_DIRECTX:
      if(set_gfx_mode(GFX_DIRECTX,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    case GFX_DIRECTX_SOFT:
      if(set_gfx_mode(GFX_DIRECTX_SOFT,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    case GFX_DIRECTX_SAFE:
      if(set_gfx_mode(GFX_DIRECTX_SAFE,screen_w,screen_h,0,0)==0)
      {
        Z_message("Can't set video mode.\n");
        exit(1);
      }
      break;
    case GFX_AUTODETECT_WINDOWED:
    case GFX_DIRECTX_WIN:
      if(set_gfx_mode(GFX_DIRECTX_WIN,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    case GFX_GDI:
      if(set_gfx_mode(GFX_GDI,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    default:
      Z_message("Can't set video mode.\n");
      exit(1);
      break;
  }
  #elif defined(ALLEGRO_LINUX)
  int mode = GFX_AUTODETECT_WINDOWED;

  switch(mode)
  {
    case GFX_AUTODETECT_WINDOWED:
      if(set_gfx_mode(GFX_AUTODETECT_WINDOWED,screen_w,screen_h,0,0)==0)
      {
        break;
      }
    default:
      Z_message("Can't set video mode.\n");
      exit(1);
      break;
  }
  #endif

  // set_palette((RGB*)fontsdata[PAL_EB_TEST].dat);
  bmp=load_bitmap("zelda001.bmp", RAMpal);
  set_palette(RAMpal);
  clear_bitmap(screen);
  onHelp();

  return 0;

}

END_OF_MAIN();

/* end */
