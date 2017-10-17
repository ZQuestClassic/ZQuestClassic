//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zsys.cc
//
//  System functions, etc.
//
//--------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "zc_alleg.h"
#include <allegro/internal/aintern.h>

#ifdef ALLEGRO_DOS
#include <conio.h>
#endif

#include "zdefs.h"
#include "zsys.h"
#include "zc_sys.h"
#include "jwin.h"

/*
enum { jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
       jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
       jcMAX };

int jwin_pal[jcMAX] =
{
  vc(11),vc(15),vc(4),vc(7),vc(6),vc(0),
  192,223,vc(14),vc(15),vc(0),vc(1),vc(14)
};
*/


char *time_str_long(dword time)
{
  static char s[16];

  dword decs = (time%60)*100/60;
  dword secs = (time/60)%60;
  dword mins = (time/3600)%60;
  dword hours = time/216000;

  sprintf(s,"%ld:%02ld:%02ld.%02ld",hours,mins,secs,decs);
  return s;
}

char *time_str_med(dword time)
{
  static char s[16];

  dword secs = (time/60)%60;
  dword mins = (time/3600)%60;
  dword hours = time/216000;

  sprintf(s,"%ld:%02ld:%02ld",hours,mins,secs);
  return s;
}

char *time_str_short(dword time)
{
  static char s[16];

  dword mins = (time/3600)%60;
  dword hours = time/216000;

  sprintf(s,"%ld:%02ld",hours,mins);
  return s;
}

void extract_name(char *path,char *name,int type)
{
  int l=strlen(path);
  int i=l;
  while(i>0 && path[i-1]!='/' && path[i-1]!='\\')
    --i;
  int n=0;
  if(type==FILENAME8__)
  {
    while(i<l && n<8 && path[i]!='.')
      name[n++]=path[i++];
  }
  else if (type==FILENAME8_3)
  {
    while(i<l && n<12 )
      name[n++]=path[i++];
  }
  else
  {
    while(i<l)
      name[n++]=path[i++];
  }
  name[n]=0;
}

void chop_path(char *path)
{
  int p = strlen(path);
  int f = strlen(get_filename(path));
  if(f<p)
    path[p-f]=0;
}

char *temp_name(char *s)
{
  int tempnum;
  static char *temporaryname=(char*)malloc(L_tmpnam);
  //  sprintf(temporaryname, "tempfile.qsu");
  //  return temporaryname;


  for (int i=0; i<1000; ++i)
  {
    sprintf(temporaryname, "00000000.tmp");
    for (int j=0; j<8; ++j)
    {
      tempnum=rand()%62;
      if (tempnum<26)
      {
        temporaryname[j]='A'+tempnum;
      }
      else if (tempnum<52)
        {
          temporaryname[j]='A'+tempnum-26;
        }
        else
        {
          temporaryname[j]='0'+tempnum-52;
        }
    }
    if (!exists(temporaryname))
    {
      break;
    }
  }
  if (s!=NULL)
  {
    sprintf(s, "%s", temporaryname);
  }
  return temporaryname;
}

int vbound(int x,int low,int high)
{
  if(x<low) x=low;
  if(x>high) x=high;
  return x;
}

float vbound(float x,float low,float high)
{
  if(x<low) x=low;
  if(x>high) x=high;
  return x;
}

int used_switch(int argc,char *argv[],char *s)
{
  // assumes a switch won't be in argv[0]
  for(int i=1; i<argc; i++)
    if(stricmp(argv[i],s)==0)
      return i;
  return 0;
}

char zeldapwd[8]  = { 'T'+11,'e'+22,'m'+33,'e'+44,'n'+55,'k'+66,'u'+77,0+88 };
char zquestpwd[8] = { 'N'+11,'g'+22,'o'+33,'m'+44,'o'+55,'n'+66,'g'+77,0+88 };
char datapwd[8]   = { 'l'+11,'o'+22,'n'+33,'g'+44,'t'+55,'a'+66,'n'+77,0+88 };

void resolve_password(char *pwd)
{
  for(int i=0; i<8; i++)
    pwd[i]-=(i+1)*11;
}

void set_bit(byte *bitstr,int bit,byte val)
{
  bitstr += bit>>3;
  byte mask = 1 << (bit&7);

  if(val)
    *bitstr |= mask;
  else
    *bitstr &= ~mask;
}

int get_bit(byte *bitstr,int bit)
{
  bitstr += bit>>3;
  return ((*bitstr) >> (bit&7))&1;
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
  al_trace("%s\n",buf);
  #elif defined(ALLEGRO_MACOSX)
  al_trace("%s\n",buf);
  #endif
  exit(1);
}

void Z_message(char *format,...)
{
  char buf[2048];

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
  #elif defined(ALLEGRO_MACOSX)
  al_trace("%s",buf);
  #endif
}

void Z_title(char *format,...)
{
  char buf[256];
  va_list ap;
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  #ifdef ALLEGRO_DOS
  text_info ti;
  gettextinfo(&ti);
  int w = ti.screenwidth;

  int len = strlen(buf);

  if(len>w)
    printf("%s\n",buf);
  else
  {
    char title[81];
    for(int i=0; i<w; i++)
      title[i]=' ';
    title[w]=0;

    int center = (w - len) >> 1;
    memcpy(title+center,buf,len);

    printf("\n");
    textattr(0x4E);
    cprintf("%s",title);
    textattr(0x07);
    for(int i=0; i<w; i++)
      cprintf(" ");
  }
  #elif defined(ALLEGRO_WINDOWS)
  al_trace("%s\n",buf);
  #elif defined(ALLEGRO_LINUX)
  al_trace("%s\n",buf);
  #elif defined(ALLEGRO_MACOSX)
  al_trace("%s\n",buf);
  #endif
}

int anim_3_4(int clk, int speed)
{
  clk /= speed;
  switch(clk&3)
  {
    case 0:
    case 2: clk = 0; break;
    case 1: clk = 1; break;
    case 3: clk = 2; break;
  }
  return clk;
}

/**********  Encryption Stuff  *****************/

//#define MASK 0x4C358938
static int seed = 0;
//#define MASK 0x91B2A2D1
//static int seed = 7351962;
static int enc_mask[3]={0x4C358938,0x91B2A2D1,0x4A7C1B87};
static int pvalue[3]={0x62E9,0x7D14,0x1A82};
static int qvalue[3]={0x3619,0xA26B,0xF03C};

static int rand_007(int method)
{
  short BX = seed >> 8;
  short CX = (seed & 0xFF) << 8;
  char AL = seed >> 24;
  char C = AL >> 7;
  char D = BX >> 15;
  AL <<= 1;
  BX = (BX << 1) | C;
  CX = (CX << 1) | D;
  CX += seed & 0xFFFF;
  BX += (seed >> 16) + C;
  //  CX += 0x62E9;
  //  BX += 0x3619 + D;
  CX += pvalue[method];
  BX += qvalue[method] + D;
  seed = (BX << 16) + CX;
  return (CX << 16) + BX;
}

void encode_007(byte *buf, dword size, dword key, word *check1, word *check2, int method)
{
  dword i;
  byte *p;

  *check1 = 0;
  *check2 = 0;

  p = buf;
  for(i=0; i<size; i++)
  {
    *check1 += *p;
    *check2 = (*check2 << 4) + (*check2 >> 12) + *p;
    ++p;
  }

  p = buf;
  seed = key;
  for(i=0; i<size; i+=2)
  {
    byte q = rand_007(method);
    *p ^= q;
    ++p;
    if(i+1 < size)
    {
      *p += q;
      ++p;
    }
  }
}

bool decode_007(byte *buf, dword size, dword key, word check1, word check2, int method)
{
  dword i;
  word c1 = 0, c2 = 0;
  byte *p;

  p = buf;
  seed = key;
  for(i=0; i<size; i+=2)
  {
    char q = rand_007(method);
    *p ^= q;
    ++p;
    if(i+1 < size)
    {
      *p -= q;
      ++p;
    }
  }

  p = buf;
  for(i=0; i<size; i++)
  {
    c1 += *p;
    c2 = (c2 << 4) + (c2 >> 12) + *p;
    ++p;
  }

  return (c1 == check1) && (c2 == check2);
}

//
// RETURNS:
//   0 - OK
//   1 - srcfile not opened
//   2 - destfile not opened
//
int encode_file_007(char *srcfile, char *destfile, int key, char *header, int method)
{
  FILE *src, *dest;
  int tog = 0, c, r=0;
  short c1 = 0, c2 = 0;

  seed = key;
  src = fopen(srcfile, "rb");
  if(!src)
    return 1;

  dest = fopen(destfile, "wb");
  if(!dest)
  {
    fclose(src);
    return 2;
  }

  // write the header
  if(header)
  {
    for(c=0; header[c]; c++)
      fputc(header[c], dest);
  }

  // write the key, XORed with MASK
  key ^= enc_mask[method];
  fputc(key>>24, dest);
  fputc((key>>16)&255, dest);
  fputc((key>>8)&255, dest);
  fputc(key&255, dest);

  // encode the data
  while((c=fgetc(src)) != EOF)
  {
    c1 += c;
    c2 = (c2 << 4) + (c2 >> 12) + c;
    if(tog)
      c += r;
    else
    {
      r = rand_007(method);
      c ^= r;
    }
    tog ^= 1;

    fputc(c, dest);
  }

  // write the checksums
  r = rand_007(method);
  c1 ^= r;
  c2 += r;
  fputc(c1>>8, dest);
  fputc(c1&255, dest);
  fputc(c2>>8, dest);
  fputc(c2&255, dest);

  fclose(src);
  fclose(dest);
  return 0;
}

//
// RETURNS:
//   0 - OK
//   1 - srcfile not opened
//   2 - destfile not opened
//   3 - scrfile too small
//   4 - srcfile EOF
//   5 - checksum mismatch
//   6 - header mismatch
//
int decode_file_007(char *srcfile, char *destfile, char *header, int method, bool packed)
{
  FILE *normal_src=NULL, *dest=NULL;
  PACKFILE *packed_src=NULL;
  int tog = 0, c, r=0, err;
  long size, i;
  short c1 = 0, c2 = 0, check1, check2;

  // open files
  size = file_size_ex(srcfile);
  if(size < 1)
  {
    return 1;
  }
  size -= 8;                                                // get actual data size, minus key and checksums
  if(size < 1)
  {
    return 3;
  }

  if (!packed)
  {
    normal_src = fopen(srcfile, "rb");
    if(!normal_src)
    {
      return 1;
    }
  }
  else
  {
    packed_src = pack_fopen(srcfile, F_READ_PACKED);
    if (errno==EDOM)
    {
      packed_src = pack_fopen(srcfile, F_READ);
    }
    if(!packed_src)
    {
      return 1;
    }
  }

  dest = fopen(destfile, "wb");
  if(!dest)
  {
    if (packed)
    {
      pack_fclose(packed_src);
    }
    else
    {
      fclose(normal_src);
    }
    return 2;
  }

  // read the header
  err = 4;
  if(header)
  {
    for(i=0; header[i]; i++)
    {
      if(packed)
      {
        if((c=pack_getc(packed_src)) == EOF)
        {
          goto error;
        }
      }
      else
      {
        if((c=fgetc(normal_src)) == EOF)
        {
          goto error;
        }
      }
      if((c&255) != header[i])
      {
        err = 6;
        goto error;
      }
      --size;
    }
  }

  // read the key
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  seed = c << 24;
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  seed += (c & 255) << 16;
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  seed += (c & 255) << 8;
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  seed += c & 255;
  seed ^= enc_mask[method];

  // decode the data
  for(i=0; i<size; i++)
  {
    if(packed)
    {
      if((c=pack_getc(packed_src)) == EOF)
      {
        goto error;
      }
    }
    else
    {
      if((c=fgetc(normal_src)) == EOF)
      {
        goto error;
      }
    }

    if(tog)
    {
      c -= r;
    }
    else
    {
      r = rand_007(method);
      c ^= r;
    }
    tog ^= 1;

    c &= 255;
    c1 += c;
    c2 = (c2 << 4) + (c2 >> 12) + c;

    fputc(c, dest);
  }

  // read checksums
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  check1 = c << 8;
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  check1 += c & 255;
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  check2 = c << 8;
  if(packed)
  {
    if((c=pack_getc(packed_src)) == EOF)
    {
      goto error;
    }
  }
  else
  {
    if((c=fgetc(normal_src)) == EOF)
    {
      goto error;
    }
  }
  check2 += c & 255;

  // verify checksums
  r = rand_007(method);
  check1 ^= r;
  check2 -= r;
  check1 &= 0xFFFF;
  check2 &= 0xFFFF;
  if(check1 != c1 || check2 != c2)
  {
    err = 5;
    goto error;
  }

  if (packed)
  {
    pack_fclose(packed_src);
  }
  else
  {
    fclose(normal_src);
  }
  fclose(dest);
  return 0;

  error:
  if (packed)
  {
    pack_fclose(packed_src);
  }
  else
  {
    fclose(normal_src);
  }
  fclose(dest);
  delete_file(destfile);
  return err;
}

void copy_file(char *src, char *dest)
{
  int c;
  FILE *fin, *fout;
  fin = fopen(src, "rb");
  fout = fopen(dest, "wb");
  while((c=fgetc(fin)) != EOF)
    fputc(c, fout);
  fclose(fin);
  fclose(fout);
}

#define BOX_W     MIN(512, SCREEN_W-16)
#define BOX_H     MIN(256, (SCREEN_H-64)&0xFFF0)

#define BOX_L     ((SCREEN_W - BOX_W) / 2)
#define BOX_R     ((SCREEN_W + BOX_W) / 2)
#define BOX_T     ((SCREEN_H - BOX_H) / 2)
#define BOX_B     ((SCREEN_H + BOX_H) / 2)

static int box_x = 0;
static int box_y = 0;
static bool box_active=false;
static int box_store_x = 0;
static FONT *box_title_font=font;
static FONT *box_message_font=font;
static int box_style=0;
static int box_titlebar_height=0;
static int box_message_height=0;
static int box_w=BOX_W;
static int box_h=BOX_H;
static int box_l=BOX_L;
static int box_r=BOX_R;
static int box_t=BOX_T;
static int box_b=BOX_B;
static bool box_log=true;
static char box_log_msg[480];
static int box_msg_pos=0;
static int box_store_pos=0;
/*
static int jwin_pal[jcMAX] =
{
  vc(11),vc(15),vc(4),vc(7),vc(6),vc(0),
  192,223,vc(14),vc(15),vc(0),vc(1),vc(14)
};
*/
int onSnapshot2()
{
  char buf[20];
  int num=0;
  do
  {
    sprintf(buf, "zelda%03d.bmp", ++num);
  } while(num<999 && exists(buf));

  PALETTE temppal;
  get_palette(temppal);
  BITMAP *tempbmp=create_bitmap_ex(8,screen->w, screen->h);
  blit(screen,tempbmp,0,0,0,0,screen->w,screen->h);
  save_bitmap(buf,screen,temppal);
  destroy_bitmap(tempbmp);
  return D_O_K;
}


/* starts outputting a progress message */
void box_start(int style, char *title, FONT *title_font, FONT *message_font, bool log)
{
  box_style=style;
  box_title_font=(title_font!=NULL)?title_font:font;
  box_message_font=(message_font!=NULL)?message_font:font;
  box_message_height=text_height(box_message_font);
  box_titlebar_height=title?text_height(box_title_font)+2:0;
  box_w=BOX_W;
  box_h=BOX_H;
  box_l=BOX_L;
  box_r=BOX_R;
  box_t=BOX_T;
  box_b=BOX_B;
  box_log=log;
  memset(box_log_msg, 0, 480);
  box_msg_pos=0;
  box_store_pos=0;
  scare_mouse();

  jwin_draw_win(screen, box_l, box_t, box_r-box_l, box_b-box_t, FR_WIN);
  if(title!=NULL)
  {
    swap(font,box_title_font);
    jwin_draw_titlebar(screen, box_l+3, box_t+3, box_r-box_l-6, 18, title, false);
    swap(font,box_title_font);
    box_titlebar_height=18;
  }

  unscare_mouse();

  box_store_x = box_x = box_y = 0;
  box_active = true;
  box_t+=box_titlebar_height;
  box_h-=box_titlebar_height;
  box_log=log;
  memset(box_log_msg, 0, 480);
  box_msg_pos=0;
  box_store_pos=0;
}

/* outputs text to the progress message */
void box_out(char *msg)
{
  if (box_active)
  {
    scare_mouse();
    set_clip_rect(screen, box_l+8, box_t+1, box_r-8, box_b-1);
    textout_ex(screen, box_message_font, msg, box_l+8+box_x, box_t+(box_y+1)*box_message_height, gui_fg_color, gui_bg_color);
    set_clip_rect(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
    unscare_mouse();
  }
  if (box_log)
  {
    sprintf(box_log_msg+box_msg_pos, msg);
  }
  box_x += text_length(box_message_font, msg);
  box_msg_pos+=strlen(msg);
}

/* remembers the current x position */
void box_save_x()
{
  if (box_active)
  {
    box_store_x=box_x;
  }
  box_store_pos=box_msg_pos;
}

/* remembers the current x position */
void box_load_x()
{
  if (box_active)
  {
    box_x=box_store_x;
  }
  box_msg_pos=box_store_pos;
}

/* outputs text to the progress message */
void box_eol()
{
  if (box_active)
  {
    box_x = 0;
    box_y++;

    if ((box_y+2)*box_message_height >= box_h)
    {
      scare_mouse();
      blit(screen, screen, box_l+8, box_t+(box_message_height*2), box_l+8, box_t+(box_message_height), box_w-16, box_y*box_message_height);
      rectfill(screen, box_l+8, box_t+box_y*box_message_height, box_l+box_w-8, box_t+(box_y+1)*box_message_height, gui_bg_color);
      unscare_mouse();
      box_y--;
    }
  }
  box_msg_pos = 0;
  if (box_log)
  {
    al_trace(box_log_msg);
    al_trace("\n");
    memset(box_log_msg, 0, 480);
  }
}

/* ends output of a progress message */
void box_end(bool pause)
{
  if (box_active)
  {
    if (pause)
    {
      box_eol();
      box_out("-- press a key --");
      do
      {
        //        poll_mouse();
      } while (gui_mouse_b());

      do
      {
        //        poll_mouse();
      } while ((!keypressed()) && (!gui_mouse_b()));

      do
      {
        //        poll_mouse();
      } while (gui_mouse_b());

      clear_keybuf();
    }
    box_active = false;
  }
}

/* pauses box output */
void box_pause()
{
  if (box_active)
  {
    box_save_x();
    box_out("-- press a key --");
    do
    {
      //        poll_mouse();
    } while (gui_mouse_b());

    do
    {
      //        poll_mouse();
    } while ((!keypressed()) && (!gui_mouse_b()));

    do
    {
      //        poll_mouse();
    } while (gui_mouse_b());

    clear_keybuf();
    box_load_x();
  }
}

// Checking for double clicks is complicated. The user could release the
// mouse button at almost any point, and I might miss it if I am doing some
// other processing at the same time (eg. sending the single-click message).
// To get around this I install a timer routine to do the checking for me,
// so it will notice double clicks whenever they happen.

volatile int dclick_status, dclick_time;

// dclick_check:
//  Double click checking user timer routine.

void dclick_check(void)
{
  if (dclick_status==DCLICK_NOT)
  {
    if (gui_mouse_b())
    {
	    dclick_status = DCLICK_START;           // let's go!
      dclick_time = 0;
      return;
    }
  }
  else if (dclick_status==DCLICK_START) {              // first click...
    if (!gui_mouse_b()) {
	    dclick_status = DCLICK_RELEASE;           // aah! released first
      dclick_time = 0;
      return;
    }
  }
  else if (dclick_status==DCLICK_RELEASE) {       // wait for second click
    if (gui_mouse_b()) {
      dclick_status = DCLICK_AGAIN;             // yes! the second click
      dclick_time = 0;
      return;
    }
  }
  else
  {
    return;
  }

  // timeout?
  if (dclick_time++ > 10)
  {
    dclick_status = DCLICK_NOT;
  }
}

END_OF_FUNCTION(dclick_check)

void lock_dclick_function()
{
  LOCK_FUNCTION(dclick_check);
}

/*
void dclick_check(void)
{
  if (dclick_status==DCLICK_START) {              // first click...
    if (!gui_mouse_b()) {
	    dclick_status = DCLICK_RELEASE;           // aah! released first
      dclick_time = 0;
      return;
    }
  }
  else if (dclick_status==DCLICK_RELEASE) {       // wait for second click
    if (gui_mouse_b()) {
      dclick_status = DCLICK_AGAIN;             // yes! the second click
      dclick_time = 0;
      return;
    }
  }
  else
  {
    return;
  }

  // timeout?
  if (dclick_time++ > 10)
  {
    dclick_status = DCLICK_NOT;
  }
}
*/
