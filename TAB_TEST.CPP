#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>

#include "zc_alleg.h"
#include "tab_ctl.h"

int screen_w=320, screen_h=240, mode;
int close_dlg() { return D_CLOSE; }
char buf[80];
PALETTE RAMpal;


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

static int palette_list[] =
{
  // dialog control number
  { 2 },
  { 3 },
  { -1 }
};

static int sprite_list[] =
{
  // dialog control number
  { 4 },
  { 5 },
  { 6 },
  { -1 }
};

static int combo_list[] =
{
  // dialog control number
  { 7 },
  { 8 },
  { 9 },
  { 10 },
  { -1 }
};

static TABPANEL tabs[] =
{
  // (text)
  { "Palettes", 0, palette_list },
  { "Sprites", D_SELECTED, sprite_list },
  { "Combos", 0, combo_list },
  { NULL }
};

static DIALOG help_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)          (h)          (fg)     (bg)      (key)    (flags)     (d1)      (d2)      (dp)                  (dp2)   (dp3)*/
  { d_box_proc,        -1,   -1,   screen_w+2,  screen_h+2,  0,       0,        0,       0,          0,        0,        0,                    NULL },
  { d_tab_proc,        17,   33,   300,         120,         0,       0,        0,       0,          0,        0,        (void *) tabs,        NULL,    (void *)help_dlg },
  // palettes
  { d_text_proc,       120,  90,   60,          8,          vc2(0),    vc2(15),   0,       0,          0,        0,        (void *) "FROGS!",  NULL },
  { d_button_proc,     40,   60,   80,          16,         16,      16,        0,       D_EXIT,     0,        0,        (void *) "Palettes",  NULL },
  // sprites
  { d_text_proc,       130, 100,   60,          8,           0,       1,        0,       0,          0,        0,        (void *) "woof.",  NULL },
  { d_check_proc,      140, 110,   60,          8,           2,       3,        0,       0,          0,        0,        (void *) "m30w?",  NULL },
  { d_button_proc,     50,   70,   80,          16,          4,       5,        0,       D_EXIT,     0,        0,        (void *) "Sprites",   NULL },
  //combos
  { d_text_proc,       150, 120,   60,          8,           6,       7,        0,       0,          0,        0,        (void *) "arf!",  NULL },
  { d_edit_proc,       160, 130,   60,          8,           8,       9,        0,       0,          4,        0,        NULL,             NULL },
  { d_text_proc,       170, 140,   60,          8,          10,      11,        0,       0,          0,        0,        (void *) "slick!!!!!",  NULL },
  { d_button_proc,     60,   80,   80,          16,         12,      13,        0,       D_EXIT,     0,        0,        (void *) "Combos",    NULL },
  //others
  { d_keyboard_proc,   0,    0,    0,           0,           0,       0,        0,       0,          0,        KEY_ESC,  (void *) close_dlg },
  { d_keyboard_proc,   0,    0,    0,           0,           0,       0,        0,       0,          0,        KEY_F12,  (void *) onSnapshot },
  { NULL }
};

#define DEFAULT_SPRITE_W   11          /* default arrow cursor */
#define DEFAULT_SPRITE_H   17

static char mouse_pointer_data[DEFAULT_SPRITE_H][DEFAULT_SPRITE_W] =
{
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
   { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
   { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
   { 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0 },
   { 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0 },
   { 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0 },
   { 0, 1, 2, 2, 2, 2, 1, 0, 0, 0, 0 },
   { 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0 },
   { 0, 1, 2, 2, 2, 2, 2, 2, 1, 0, 0 },
   { 0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0 },
   { 0, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1 },
   { 0, 1, 2, 2, 1, 2, 2, 1, 0, 0, 0 },
   { 0, 1, 2, 1, 0, 1, 2, 2, 1, 0, 0 },
   { 0, 0, 1, 0, 0, 1, 2, 2, 1, 0, 0 },
   { 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0 },
   { 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0 },
   { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 }
};

static BITMAP *create_mouse_pointer(void)
{
   BITMAP *bmp;
   int x, y;
   int col;
   
   bmp = create_bitmap(DEFAULT_SPRITE_W, DEFAULT_SPRITE_H);

   for (y=0; y<DEFAULT_SPRITE_H; y++) {
      for (x=0; x<DEFAULT_SPRITE_W; x++) {
	 switch (mouse_pointer_data[y][x]) {
	    case 1:  col = makecol(0, 0, 0);        break;
	    case 2:  col = makecol(255, 255, 255);  break;
	    default: col = bmp->vtable->mask_color; break;
	 }
	 putpixel(bmp, x, y, col);
      }
   }

   return bmp;
}



void doHelp(int bg,int fg)
{
  help_dlg[0].bg = bg;
  help_dlg[0].fg = fg;

  (int*)(help_dlg[1].dp2)=0;
  help_dlg[1].bg=bg;
  help_dlg[1].fg=fg;

  for (int i=2; i<11; i++)
  {
    help_dlg[i].bg=bg;
    help_dlg[i].fg=fg;
  }
  sprintf(buf,"Yo!");
  help_dlg[8].dp=buf;

  popup_dialog(help_dlg,1);
}

int onHelp()
{
  doHelp(vc2(15), vc2(0));
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

  clear_bitmap(screen);
  get_palette(RAMpal);
  set_mouse_sprite(create_mouse_pointer());
  gui_fg_color=vc2(0);
  gui_bg_color=vc2(15);
  gui_mg_color=vc2(8);
  onHelp();

  return 0;

}

END_OF_MAIN();

/* end */
