#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>

#include "zc_alleg.h"
#include "editbox.h"
#include "eb_fonts.h"

editbox_data temp_eb_data, temp_eb_data2, temp_eb_data3;
int screen_w=1024, screen_h=768, mode;
int close_dlg() { return D_CLOSE; }
char *helpbuf, *helpbuf2, *z3textbuf;
FONT *cfont,*z3font,*sfont,*lfont,*pfont;
DATAFILE *fontsdata=NULL;
PALETTE RAMpal;

#define vc(x)  ((x)+224)                                    // offset to 'VGA color' x

inline bool pfwrite(void *p,long n,PACKFILE *f)
{
  return pack_fwrite(p,n,f)==n;
}

inline bool pfread(void *p,long n,PACKFILE *f)
{
  return pack_fread(p,n,f)==n;
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
    sprintf(buf, "eb%03d.bmp", ++num);
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

bool readfile(char *path,void *buf,int count)
{
  PACKFILE *f=pack_fopen(path,F_READ);
  if(!f)
    return 0;
  bool good=pfread(buf,count,f);
  pack_fclose(f);
  return good;
}

bool writefile(char *path,void *buf,int count)
{
  PACKFILE *f=pack_fopen(path,F_WRITE);
  if(!f)
    return 0;

  bool good=pfwrite(buf,count,f);
  pack_fclose(f);
  return good;
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

char oldcb[65535];
char *testcb=oldcb;

static DIALOG help_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)          (h)          (fg)     (bg)      (key)    (flags)     (d1)      (d2)      (dp)    (dp2)   (dp3)
  { d_box_proc,        -1,   -1,   screen_w+2,  screen_h+2,  0,       vc(15),   0,       0,          0,        0,        0,        NULL },
  { d_editbox_proc,    17,   33,   289,         197,         0,       0,        0,       0,          0,        0,        &temp_eb_data },

  { d_keyboard_proc,   0,    0,    0,           0,           0,       0,        0,       0,          0,        KEY_ESC,  (void *)close_dlg },
  { d_keyboard_proc,   0,    0,    0,           0,           0,       0,        0,       0,          0,        KEY_F12,  (void *)onSnapshot },
  { NULL }
};

textline *loadtext(char *filename)
{
  if (0)
  {
  textline *start = (textline *)malloc(sizeof(textline));
  textline *currentline;
  currentline=start;
  currentline->prev = NULL;
  textline *templine;

  currentline->text = (char *)malloc(1024);
  sprintf(currentline->text, "This is line 1.\n");
  templine=currentline;
  currentline->next = (textline *)malloc(sizeof(textline));
  currentline=currentline->next;
  currentline->prev=templine;

  currentline->text = (char *)malloc(1024);
  sprintf(currentline->text, "And this here is line 2.\n");
  templine=currentline;
  currentline->next = (textline *)malloc(sizeof(textline));
  currentline=currentline->next;
  currentline->prev=templine;

  currentline->text = (char *)malloc(1024);
  sprintf(currentline->text, "Line 3, part a\nLine 3, part b.");
  templine=currentline;
  currentline->next = NULL;
  return start;
  }
  
  char src_text[65535];
  char *str_ptr=src_text-1;
  //read the file into src_text
  if (file_size(filename)==0)
  {
    al_trace("Error loading %s:  File not found.\n", filename);
    exit(1);
  }

  FILE *f=fopen(filename, "rb");
  while (!feof(f))
  {
    *(++str_ptr)=fgetc(f); 
  }
  *str_ptr=0;
  fclose(f);
  
  //find each newline character(s) and split the text into lines
  int pointer=0, old_pointer=0;
  textline *start = (textline *)malloc(sizeof(textline));
  textline *currentline;
  currentline=start;
  currentline->prev = NULL;
  textline *templine;
  while (src_text[pointer]!=0)
  {
    while (src_text[pointer]!=10&&src_text[pointer]!=13)
    {
      if (src_text[pointer]==0)
      {
        break;
      }
      ++pointer;
    }
    switch (src_text[pointer])
    {
      case 10:
        if (src_text[++pointer]==13)
        {
          ++pointer;
        }
        break;
      case 13:
        if (src_text[++pointer]==10)
        {
          ++pointer;
        }
        break;
    }
    templine=currentline;
    currentline->next=(textline *)malloc(sizeof(textline));
    currentline=currentline->next;
    currentline->prev=templine;

    currentline->text=(char *)malloc((pointer-old_pointer)*2);
    memcpy(currentline->text, src_text+old_pointer, pointer-old_pointer);
    currentline->text[pointer-old_pointer]=0;
    currentline->extra=(textline_extra *)malloc(sizeof(textline_extra));
    ((textline_extra *)(currentline->extra))->maxlen=((pointer-old_pointer)*2)-1;

    old_pointer=pointer;
  }

  templine=start;
  currentline=templine->next;
  templine->next=NULL;
  free(templine);
  currentline->prev=NULL;
  start=currentline;
  
  return start;
}

void doHelp(int bg,int fg)
{
  help_dlg[0].bg = bg;
  help_dlg[0].fg = fg;

  ((editbox_data*) (help_dlg[1].dp))->text_data=loadtext("z3text.txt");

  //textline *currentline=((editbox_data*) (help_dlg[1].dp))->text_data;

  ((editbox_data*) (help_dlg[1].dp))->font = lfont;
  ((editbox_data*) (help_dlg[1].dp))->maxlines=1;
  ((editbox_data*) (help_dlg[1].dp))->selfg=bg;
  ((editbox_data*) (help_dlg[1].dp))->selbg=fg;
  // ((editbox_data*) (help_dlg[1].dp))->clipboard=testcb;
  help_dlg[1].bg = bg;
  help_dlg[1].fg = fg;

  popup_dialog(help_dlg,1);

  return;
}

int onHelp()
{
  doHelp(vc(15),vc(0));
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

  Z_message("Loading data files:\n");

  Z_message("EB_Fonts.Dat...");
  if((fontsdata=load_datafile("eb_fonts.dat"))==NULL)
    Z_error("failed");
  Z_message("OK\n");

  font =(FONT*)fontsdata[FONT_GUI].dat;
  cfont =(FONT*)fontsdata[FONT_COMIC].dat;
  z3font=(FONT*)fontsdata[FONT_Z3].dat;
  sfont=(FONT*)fontsdata[FONT_6x6].dat;
  lfont=(FONT*)fontsdata[FONT_LARGEPROP].dat;
  pfont=(FONT*)fontsdata[FONT_8xPROP].dat;


  int helpsize = file_size("zquest.txt");
  if(helpsize==0)
    Z_error("zquest.txt not found");
  helpbuf = (char*)malloc(helpsize<65536?65536:helpsize*2+1);
  if(!helpbuf)
    Z_error("Error allocating help buffer");
  if(!readfile("zquest.txt",helpbuf,helpsize))
    Z_error("Error loading zquest.txt");
  helpbuf[helpsize]=0;
/*
  int helpsize2 = file_size("zquest.tx3");
  if(helpsize2==0)
    Z_error("zquest.tx3 not found");
  helpbuf2 = (char*)malloc(helpsize2<65536?65536:helpsize2*2+1);
  if(!helpbuf2)
    Z_error("Error allocating help buffer");
  if(!readfile("zquest.tx3",helpbuf2,helpsize2))
    Z_error("Error loading zquest.tx3");
  helpbuf2[helpsize2]=0;

  int z3textsize = file_size("z3text.txt");
  if(z3textsize==0)
    Z_error("z3text.txt not found");
  z3textbuf = (char*)malloc(z3textsize<65536?65536:z3textsize*2+1);
  if(!z3textbuf)
    Z_error("Error allocating Z3 Text buffer");
  if(!readfile("z3text.txt",z3textbuf,z3textsize))
    Z_error("Error loading z3text.txt");
  z3textbuf[z3textsize]=0;
*/


  Z_message("OK\n");                                        // loading data files...

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

  set_palette((RGB*)fontsdata[PAL_EB_TEST].dat);
  get_palette(RAMpal);
  clear_bitmap(screen);
  onHelp();

  return 0;

}

END_OF_MAIN();

/* end */
