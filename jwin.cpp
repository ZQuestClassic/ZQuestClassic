 /*                 __                  __
  *                /_/\  __  __  __    /_/\  ______
  *               _\_\/ / /\/ /\/ /\  _\_\/ / ____ \
  *              / /\  / / / / / / / / /\  / /\_ / /\
  *         __  / / / / /_/ /_/ / / / / / / / / / / /
  *        / /_/ / / /_________/ / /_/ / /_/ / /_/ /
  *        \____/ /  \_________\/  \_\/  \_\/  \_\/
  *         \___\/
  *
  *
  *
  *     jwin.c
  *
  *     Windows(R) style GUI for Allegro.
  *     by Jeremy Craner
  *
  *     Most routines are adaptations of Allegro code.
  *     Allegro is by Shawn Hargreaves, et al.
  *
  *     Version: 3/22/00
  *     Allegro version: 3.1x  (don't know if it works with WIP)
  *
  */

/* This code is not fully tested */

#include <ctype.h>
#include <string.h>
#include "zc_alleg.h"
#include <allegro/internal/aintern.h>
#include "jwin.h"
#include "editbox.h"
#include <iostream>
#include "zsys.h"
#include <stdio.h>
#include "trapper_keeper.h"

//#ifndef _MSC_VER
#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))
//#endif
//#ifdef _ZQUEST_SCALE_
extern volatile int myvsync;
extern int zqwin_scale;
extern BITMAP *hw_screen;
//#endif
extern int zq_screen_w, zq_screen_h;

extern bool is_zquest();

/* ... Included in jwin.h ...

  enum { jcBOX, jcLIGHT, jcMEDLT, jcMEDDARK, jcDARK, jcBOXFG,
  jcTITLEL, jcTITLER, jcTITLEFG, jcTEXTBG, jcTEXTFG, jcSELBG, jcSELFG,
  jcMAX };
  */

int bound(int x,int low,int high)
{
  if(x<low) x=low;
  if(x>high) x=high;
  return x;
}
/*
  float bound(float x,float low,float high)
  {
  if(x<low) x=low;
  if(x>high) x=high;
  return x;
  }
  */

/* these are provided for external use */
int jwin_colors[jcMAX] =
{
  0xC0C0C0,0xF0F0F0,0xD0D0D0,0x808080,0x404040,0x000000,
  0x000080,0x00F0F0,0xFFFFFF,0xFFFFFF,0x000000,0x000080,0xFFFFFF
};

int scheme[jcMAX] =
{
	0xC0C0C0,0xF0F0F0,0xD0D0D0,0x808080,0x404040,0x000000,
	0x000080,0x00F0F0,0xFFFFFF,0xFFFFFF,0x000000,0x000080,0xFFFFFF
};

 /*  jwin_set_colors:
  *   Loads a set of colors in 0xRRGGBB or 256-color-indexed format
  *   into the current color scheme using the appropriate color depth
  *   conversions.
  */
void jwin_set_colors(int *colors)
{
  int i = 0;

  if(bitmap_color_depth(screen) == 8)
  {
    // use color indices
    for( ; i<jcMAX; i++)
      scheme[i] = colors[i];
  }
  else
  {
    // 0xRRGGBB format
    for( ; i<jcMAX; i++)
      scheme[i] = makecol(colors[i]>>16, (colors[i]>>8)&0xFF, colors[i]&0xFF);
  }
}

/* jwin_set_dialog_color:
  *  Sets the foreground and background colors of all the objects in a dialog.
  *
  *  Needs work!
  */
void jwin_set_dialog_color(DIALOG *dialog)
{
  int c;

  for (c=0; dialog[c].proc; c++)
  {
    dialog[c].fg = scheme[jcMEDDARK];
    dialog[c].bg = scheme[jcBOX];
  }
}

/*  jwin_draw_frame:
  *   Draws a frame using the specified style.
  */
void jwin_draw_frame(BITMAP *dest,int x,int y,int w,int h,int style)
{
  int c1,c2,c3,c4;

  switch(style)
  {
    case FR_BOX:
      c1 = jcLIGHT;
      c2 = jcMEDLT;
      c3 = jcMEDDARK;
      c4 = jcDARK;
      break;
    case FR_INV:
      c1 = jcDARK;
      c2 = jcMEDDARK;
      c3 = jcMEDLT;
      c4 = jcLIGHT;
      break;
    case FR_DEEP:
      c1 = jcMEDDARK;
      c2 = jcDARK;
      c3 = jcMEDLT;
      c4 = jcLIGHT;
      break;
    case FR_DARK:
      c1 = jcDARK;
      c2 = jcMEDDARK;
      c3 = jcMEDDARK;
      c4 = jcDARK;
      break;
    case FR_ETCHED:
      c1 = jcMEDDARK;
      c2 = jcLIGHT;
      c3 = jcMEDDARK;
      c4 = jcLIGHT;
      break;
    case FR_MEDDARK:
      c1 = jcMEDDARK;
      c2 = jcBOX;
      c3 = jcBOX;
      c4 = jcMEDDARK;
      break;
    case FR_WIN:
    default:
      c1 = jcMEDLT;
      c2 = jcLIGHT;
      c3 = jcMEDDARK;
      c4 = jcDARK;
      break;
  }
  _allegro_hline(dest, vbound(x,0,dest->w-1), vbound(y,0,dest->h-1)  , vbound(x+w-2, 0,dest->w-1), palette_color[scheme[c1]]);
  _allegro_vline(dest, vbound(x,0,dest->w-1), vbound(y+1,0,dest->h-1), vbound(y+h-2, 0, dest->h-1), palette_color[scheme[c1]]);

  _allegro_hline(dest, vbound(x+1,0,dest->w-1), vbound(y+1,0,dest->h-1), vbound(x+w-3,0,dest->w-1), palette_color[scheme[c2]]);
  _allegro_vline(dest, vbound(x+1,0,dest->w-1), vbound(y+2,0,dest->h-1), vbound(y+h-3,0,dest->h-1), palette_color[scheme[c2]]);

  _allegro_hline(dest, vbound(x+1,0,dest->w-1), vbound(y+h-2,0,dest->h-1), vbound(x+w-2,0,dest->w-1), palette_color[scheme[c3]]);
  _allegro_vline(dest, vbound(x+w-2,0,dest->w-1), vbound(y+1,0,dest->h-1), vbound(y+h-3,0,dest->h-1), palette_color[scheme[c3]]);

  _allegro_hline(dest, vbound(x,0,dest->w-1), vbound(y+h-1,0,dest->h-1), vbound(x+w-1,0, dest->w-1), palette_color[scheme[c4]]);
  _allegro_vline(dest, vbound(x+w-1,0,dest->w-1), vbound(y,0,dest->h-1), vbound(y+h-2,0,dest->h-1), palette_color[scheme[c4]]);
}

/*  jwin_draw_win:
  *   Draws a window -- a box with a frame.
  */
void jwin_draw_win(BITMAP *dest,int x,int y,int w,int h,int frame)
{
	rectfill(dest,zc_max(x+2,0),zc_max(y+2,0),zc_min(x+w-3, dest->w-1),zc_min(y+h-3, dest->h-1),palette_color[scheme[jcBOX]]);
  jwin_draw_frame(dest, x, y, w, h, frame);
}

/*  jwin_draw_button:
  *   Helper function for buttons.
  *   Draws a box with a frame that depends on "state":
  *     0: normal border (slightly different than window border)
  *     1: inverted border
  *     2: dark border
  *     3: medium dark border
  */
void jwin_draw_button(BITMAP *dest,int x,int y,int w,int h,int state,int type)
{
  int frame = FR_BOX;
  if (type==1)
  {
    frame=FR_WIN;
  }
  switch(state)
  {
    case 1: frame = FR_INV; break;
    case 2: frame = FR_DARK; break;
    case 3: frame = FR_MEDDARK; break;
  }
  jwin_draw_win(dest, x, y, w, h, frame);
}

/*  mix_value:
  *   Returns a mix of the values c1 and c2 with pos==0 being c1,
  *   pos==max being c2, pos==max/2 being half way between c1 and c2, etc.
  */
int mix_value(int c1,int c2,int pos,int max)
{
  if(max<=0)
    return c1;

  return (c2 - c1) * pos / max + c1;
}

/*  mix_color:
  *   Returns a mix of the colors c1 and c2 with pos==0 being c1,
  *   pos==max being c2, pos==max/2 being half way between c1 and c2, etc.
  *
  static int mix_color(int c1,int c2,int pos,int max)
  {
  int c;

  if(bitmap_color_depth(screen) == 8)
  c = mix_value(c1, c2, pos, max);
  else
  {
  int r = mix_value(getr(c1), getr(c2), pos, max);
  int g = mix_value(getg(c1), getg(c2), pos, max);
  int b = mix_value(getb(c1), getb(c2), pos, max);
  c = makecol(r,g,b);
  }

  return c;
  }
  */

char *shorten_string(char *dest, char *src, FONT *usefont, int maxchars, int maxwidth)
{
  strncpy(dest,src,maxchars);
  dest[maxchars-1]='\0';
  int len=(int)strlen(dest);
  int width=text_length(usefont, dest);
  dest[len]=0;

  while(width>maxwidth && len>4)
  {
    dest[len-4] = '.';
    dest[len-3] = '.';
    dest[len-2] = '.';
    dest[len-1] = 0;
    len--;
    width=text_length(usefont, dest);
  }

  return dest;
}

void jwin_draw_titlebar(BITMAP *dest, int x, int y, int w, int h, const char *str, bool draw_button)
{
  char buf[512];
  int len = (int)strlen(str);
  int length = text_length(font,str);
  int height = text_height(font);

  int tx = x + 2;
  int ty = y + (h-height)/2;
  PALETTE temp_pal;
  /*
    int i = 0;
    for( ; i<w; i++)
    {
    register int c = mix_color(scheme[jcTITLEL],scheme[jcTITLER],i,w);
    _allegro_vline(dest,x+i,y,y+h-1,c);
    }
    */
  get_palette(temp_pal);
  dither_rect(dest, &temp_pal, x, y, x+w-1, y+h-1,
              makecol15(temp_pal[scheme[jcTITLEL]].r*255/63,
                        temp_pal[scheme[jcTITLEL]].g*255/63,
                        temp_pal[scheme[jcTITLEL]].b*255/63),
              makecol15(temp_pal[scheme[jcTITLER]].r*255/63,
                        temp_pal[scheme[jcTITLER]].g*255/63,
                        temp_pal[scheme[jcTITLER]].b*255/63),
              scheme[jcTITLEL], scheme[jcTITLER]);


  if(len>509)
    len=509;

  strncpy(buf,str,len);
  buf[len]=0;

  // this part needs work

  if(length>w-20)
  {
    while(length>w-20 && len>1)
    {
      buf[len-4] = '.';
      buf[len-3] = '.';
      buf[len-2] = '.';
      buf[len-1] = 0;
      len--;
      length = text_length(font,buf);
    }
  }

  textout_ex(dest,font,buf,tx,ty,palette_color[scheme[jcTITLEFG]],-1);

  if (draw_button)
  {
    draw_x_button(dest, x + w - 18, y+2, 0);
  }

}

void draw_x_button(BITMAP *dest, int x, int y, int state)
{
  int c = scheme[jcDARK];

  jwin_draw_button(dest,x,y,16,14,state,0);
  x += 4 + (state?1:0);
  y += 3 + (state?1:0);

  line(dest,x,  y,  x+6,y+6,palette_color[c]);
  line(dest,x+1,y,  x+7,y+6,palette_color[c]);
  line(dest,x,  y+6,x+6,y,  palette_color[c]);
  line(dest,x+1,y+6,x+7,y,  palette_color[c]);
}

void draw_arrow_button(BITMAP *dest, int x, int y, int w, int h, int up, int state)
{
  int c = scheme[jcDARK];
  int ah = zc_min(h/3, 5);
  int i = 0;

  jwin_draw_button(dest,x,y,w,h,state,1);
  x += w/2 - (state?0:1);
  y += (h-ah)/2 + (state?1:0);

  for( ; i<ah; i++)
  {
    _allegro_hline(dest, x-(up?i:ah-i-1), y+i, x+(up?i:ah-i-1), c);
  }
}

void draw_arrow_button_horiz(BITMAP *dest, int x, int y, int w, int h, int up, int state)
{
  int c = scheme[jcDARK];
  int aw = zc_min(w/3, 5);
  int i = 0;

  jwin_draw_button(dest,x,y,w,h,state,1);
  y += h/2 - (state?0:1);
  x += (w-aw)/2 + (state?1:0);

  for( ; i<aw; i++)
  {
    _allegro_vline(dest, x+i,y-(up?i:aw-i-1), y+(up?i:aw-i-1), c);
  }
}

int mouse_in_rect(int x,int y,int w,int h)
{
  return ((gui_mouse_x() >= x) && (gui_mouse_y() >= y) &&
          (gui_mouse_x() < x + w) && (gui_mouse_y() < y + h));
}

static int do_x_button(BITMAP *dest, int x, int y)
{
  int down=0, last_draw = 0;

  while (gui_mouse_b())
  {
    down = mouse_in_rect(x,y,16,14);

    if(down!=last_draw)
    {
      scare_mouse();
      draw_x_button(dest,x,y,down);
      unscare_mouse();
      last_draw = down;
    }

    /* let other objects continue to animate */
    broadcast_dialog_message(MSG_IDLE, 0);
  }

  if(down)
  {
    scare_mouse();
    draw_x_button(dest,x,y,0);
    unscare_mouse();
  }

  return down;
}

/* dotted_rect:
  *  Draws a dotted rectangle, for showing an object has the input focus.
  */
void dotted_rect(BITMAP *dest, int x1, int y1, int x2, int y2, int fg, int bg)
{
  int x = ((x1+y1) & 1) ? 1 : 0;
  int c;

  //acquire the bitmap ourselves, because locking a surface within each call is *SLOW AS BALLS* -DD
  acquire_bitmap(dest);
  for (c=x1; c<=x2; c++)
  {
    putpixel(dest, c, y1, (((c+y1) & 1) == x) ? fg : bg);
    putpixel(dest, c, y2, (((c+y2) & 1) == x) ? fg : bg);
  }

  for (c=y1+1; c<y2; c++)
  {
    putpixel(dest, x1, c, (((c+x1) & 1) == x) ? fg : bg);
    putpixel(dest, x2, c, (((c+x2) & 1) == x) ? fg : bg);
  }

  release_bitmap(dest);

}

static void _dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg)
{
  dotted_rect(screen, x1, y1, x2, y2, palette_color[fg], palette_color[bg]);
}

/* gui_textout_ln:
  *  Wrapper function for drawing text to the screen, which interprets the
  *  & character as an underbar for displaying keyboard shortcuts. Returns
  *  the width of the output string in pixels.
  *
  *  Handles '\n' characters.
  */
int gui_textout_ln(BITMAP *bmp, FONT *f, unsigned char *s, int x, int y, int color, int bg, int pos)
{
  char tmp[1024];
  int c = 0;
  int len;
  int pix_len = 0;
  int hline_pos;
  int xx = x;

  while(s[c])
  {
    len = 0;
    hline_pos = -1;

    for ( ; (s[c]) && (len<(int)(sizeof(tmp)-1)); c++)
    {
      if (s[c] == '\n')
      {
        c++;
        break;
      }
      else if (s[c] == '&')
        {
          if (s[c+1] != '&')
            hline_pos = len;
          else
          {
            tmp[len++] = '&';
            c++;
          }
        }
        else
          tmp[len++] = s[c];
    }

    tmp[len] = 0;
    pix_len = zc_max( text_length(f, tmp), pix_len);

    x = xx;
    if (pos==1) //center
    {
      x -= pix_len / 2;
    }
    else if (pos==2) //right
      {
        x -= pix_len;
      }

      if (bmp)
    {
      textout_ex(bmp, f, tmp, x, y, color,bg);

      if (hline_pos >= 0)
      {
        int i;
        i = tmp[hline_pos];
        tmp[hline_pos] = 0;
        hline_pos = text_length(f, tmp);
        tmp[0] = i;
        tmp[1] = 0;
        i = text_length(f, tmp);
        _allegro_hline(bmp, x+hline_pos, y+text_height(f)-gui_font_baseline, x+hline_pos+i-1, color);
      }
    }

    y += text_height(f);
  }

  return pix_len;
}

int gui_textout_ln(BITMAP *bmp, unsigned char *s, int x, int y, int color, int bg, int pos)
{
  return gui_textout_ln(bmp, font, s, x, y, color, bg, pos);
}


/*******************************/
/*****  Misc Dialog Procs  *****/
/*******************************/

/* typedef for the listbox callback functions */
typedef char *(*getfuncptr)(int, int *);

/* event handler that closes a dialog */
int close_dlg() { return D_CLOSE; }

int jwin_frame_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  if(msg == MSG_DRAW)
  {
    jwin_draw_frame(screen, d->x, d->y, d->w, d->h, d->d1);
  }

  return D_O_K;
}

int jwin_guitest_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  if(msg == MSG_DRAW)
  {
    jwin_draw_frame(screen, d->x, d->y, d->w, d->h, d->d1);
  }

  return D_O_K;
}

/* jwin_win_proc:
  *  Draws a box with a frame. Use dp for a title string. If dp is NULL,
  *  it won't draw a title bar. If the D_EXIT flag is set, it will also
  *  draw an "X" button on the title bar that can be used to close the
  *  dialog.
  */
int jwin_win_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  rest(1);
  switch(msg)
  {
    case MSG_DRAW:
    jwin_draw_win(screen, d->x, d->y, d->w, d->h, FR_WIN);
    if(d->dp)
    {
      FONT *oldfont = font;
      if (d->dp2)
      {
        font = (FONT*)d->dp2;
      }
      jwin_draw_titlebar(screen, d->x+3, d->y+3, d->w-6, 18, (char*)d->dp, d->flags & D_EXIT);
      font = oldfont;
    }
    /*
      if(d->flags & D_EXIT)
      {
      draw_x_button(screen, d->x + d->w - 21, d->y+5, 0);
      }
      */
    break;

    case MSG_CLICK:
    if((d->flags & D_EXIT) && mouse_in_rect(d->x+d->w-21, d->y+5, 16, 14))
    {
      if(do_x_button(screen, d->x+d->w-21, d->y+5))
        return D_CLOSE;
    }
    break;
  }
  return D_O_K;
}

/* jwin_text_proc:
  *  Simple dialog procedure: draws the text string which is pointed to by dp.
  *
  *  Handles '\n' characters.
  */
int jwin_text_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  ASSERT(d);
  static BITMAP *dummy=create_bitmap_ex(8, 1, 1);
  switch (msg)
  {
    case MSG_START:
    d->w=gui_textout_ln(dummy, (unsigned char *)d->dp, 0, 0, scheme[jcMEDDARK], -1, 0);
    break;
    case MSG_DRAW:
    FONT *oldfont = font;

    if (d->dp2)
    {
      font = (FONT*)d->dp2;
    }

    if (d->flags & D_DISABLED)
    {
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x+1, d->y+1, palette_color[scheme[jcLIGHT]], palette_color[scheme[jcBOX]], 0);
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x, d->y, palette_color[scheme[jcMEDDARK]], -1, 0);
    }
    else
    {
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x, d->y, palette_color[scheme[jcBOXFG]], palette_color[scheme[jcBOX]], 0);
    }

    font = oldfont;
    break;
  }

  return D_O_K;
}

int jwin_ctext_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  ASSERT(d);
  static BITMAP *dummy=create_bitmap_ex(8, 320, 240);
  switch (msg)
  {
    case MSG_START:
    d->w=gui_textout_ln(dummy, (unsigned char *)d->dp, 0, 0, scheme[jcMEDDARK], -1, 0);
    break;
    case MSG_DRAW:
    FONT *oldfont = font;

    if (d->dp2)
    {
      font = (FONT*)d->dp2;
    }

    if (d->flags & D_DISABLED)
    {
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x+1, d->y+1, palette_color[scheme[jcLIGHT]], palette_color[scheme[jcBOX]], 1);
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x, d->y, palette_color[scheme[jcMEDDARK]], -1, 1);
    }
    else
    {
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x, d->y, palette_color[scheme[jcBOXFG]], palette_color[scheme[jcBOX]], 1);
    }

    font = oldfont;
    break;
  }

  return D_O_K;
}

int jwin_rtext_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  ASSERT(d);
  static BITMAP *dummy=create_bitmap_ex(8, 1, 1);
  switch (msg)
  {
    case MSG_START:
    d->w=gui_textout_ln(dummy, (unsigned char *)d->dp, 0, 0, scheme[jcMEDDARK], -1, 2);
    break;
    case MSG_DRAW:
    FONT *oldfont = font;

    if (d->dp2)
    {
      font = (FONT*)d->dp2;
    }

    if (d->flags & D_DISABLED)
    {
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x+1, d->y+1, palette_color[scheme[jcLIGHT]], palette_color[scheme[jcBOX]], 2);
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x, d->y, palette_color[scheme[jcMEDDARK]], -1, 2);
    }
    else
    {
      gui_textout_ln(screen, (unsigned char*)d->dp, d->x, d->y, palette_color[scheme[jcBOXFG]], palette_color[scheme[jcBOX]], 2);
    }

    font = oldfont;
    break;
  }

  return D_O_K;
}

/* draw_text_button:
  *  Helper for jwin_button_proc.
  */
void jwin_draw_text_button(BITMAP *dest, int x, int y, int w, int h, const char *str, int flags, bool show_dotted_rect)
{
  int g = (flags & D_SELECTED) ? 1 : 0;

  if(flags & D_SELECTED)
    jwin_draw_button(dest, x, y, w, h, 2, 0);
  else if( !(flags & D_GOTFOCUS) )
    jwin_draw_button(dest, x, y, w, h, 0, 0);
  else
  {
    rect(dest, x, y, x+w-1, y+h-1, palette_color[scheme[jcDARK]]);
    jwin_draw_button(dest, x+1, y+1, w-2, h-2, 0, 0);
  }

  if( !(flags & D_DISABLED) )
    gui_textout_ex(dest, str, x+w/2+g, y+h/2-text_height(font)/2+g, palette_color[scheme[jcBOXFG]], -1, TRUE);
  else
  {
    gui_textout_ex(dest, str, x+w/2+1,y+h/2-text_height(font)/2+1, palette_color[scheme[jcLIGHT]], -1, TRUE);
    gui_textout_ex(dest, str, x+w/2,  y+h/2-text_height(font)/2, palette_color[scheme[jcMEDDARK]], -1, TRUE);
  }

  if(show_dotted_rect&&(flags & D_GOTFOCUS))
    dotted_rect(dest, x+4, y+4, x+w-5, y+h-5, palette_color[scheme[jcDARK]], palette_color[scheme[jcBOX]]);
}

/* draw_graphics_button:
  *  Helper for jwin_button_proc.
  */
void jwin_draw_graphics_button(BITMAP *dest, int x, int y, int w, int h, BITMAP *bmp, BITMAP *bmp2, int flags, bool show_dotted_rect, bool overlay)
{
  int g = (flags & D_SELECTED) ? 1 : 0;

  if(flags & D_SELECTED)
    jwin_draw_button(dest, x, y, w, h, 2, 0);
  else if( !(flags & D_GOTFOCUS) )
    jwin_draw_button(dest, x, y, w, h, 0, 0);
  else
  {
    rect(dest, x, y, x+w-1, y+h-1, scheme[jcDARK]);
    jwin_draw_button(dest, x+1, y+1, w-2, h-2, 0, 0);
  }

  if( !(flags & D_DISABLED) )
  {
//    gui_textout_ex(dest, str, x+w/2+g, y+h/2-text_height(font)/2+g, scheme[jcBOXFG], -1, TRUE);
      if(bmp!=NULL)
      {
        if(overlay)
        {
          masked_blit(bmp, dest, 0, 0, x+w/2-bmp->w/2+g, y+h/2-bmp->h/2+g, bmp->h, bmp->w);
        }
        else
        {
          blit(bmp, dest, 0, 0, x+w/2-bmp->w/2+g, y+h/2-bmp->h/2+g, bmp->h, bmp->w);
        }
      }
  }
  else
  {
//    gui_textout_ex(dest, str, x+w/2+1,y+h/2-text_height(font)/2+1, scheme[jcLIGHT], -1, TRUE);
//    gui_textout_ex(dest, str, x+w/2,  y+h/2-text_height(font)/2, scheme[jcMEDDARK], -1, TRUE);
      if(bmp2!=NULL)
      {
        if(overlay)
        {
          masked_blit(bmp2, dest, 0, 0, x+w/2-bmp2->w/2+g, y+h/2-bmp2->h/2+g, bmp2->h, bmp2->w);
        }
        else
        {
          blit(bmp2, dest, 0, 0, x+w/2-bmp2->w/2+g, y+h/2-bmp2->h/2+g, bmp2->h, bmp2->w);
        }
      }
  }

  if(show_dotted_rect&&(flags & D_GOTFOCUS))
    dotted_rect(dest, x+4, y+4, x+w-5, y+h-5, scheme[jcDARK], scheme[jcBOX]);
}

/* jwin_button_proc:
  *  A button object (the dp field points to the text string). This object
  *  can be selected by clicking on it with the mouse or by pressing its
  *  keyboard shortcut. If the D_EXIT flag is set, selecting it will close
  *  the dialog, otherwise it will toggle on and off.
  */
int jwin_button_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  int down=0;
  int selected=(d->flags&D_SELECTED)?1:0;
  int last_draw;

  switch (msg)
  {

    case MSG_DRAW:
    {
      FONT *oldfont = font;
      if (d->dp2)
      {
        font = (FONT*)d->dp2;
      }
      jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, (char*)d->dp, d->flags, true);
      font = oldfont;
    }
    break;

    case MSG_WANTFOCUS:
    return D_WANTFOCUS;

    case MSG_KEY:
    /* close dialog? */
    if (d->flags & D_EXIT)
    {
      return D_CLOSE;
    }

    /* or just toggle */
    d->flags ^= D_SELECTED;
    scare_mouse();
    object_message(d, MSG_DRAW, 0);
    unscare_mouse();
    break;

    case MSG_CLICK:
    last_draw = 0;

    /* track the mouse until it is released */
    while (gui_mouse_b())
    {
      down = mouse_in_rect(d->x, d->y, d->w, d->h);

      /* redraw? */
      if (last_draw != down)
      {
        if(down != selected)
          d->flags |= D_SELECTED;
        else
          d->flags &= ~D_SELECTED;
        scare_mouse();
        object_message(d, MSG_DRAW, 0);
        unscare_mouse();
        last_draw = down;
      }

      /* let other objects continue to animate */
      broadcast_dialog_message(MSG_IDLE, 0);
      //	#ifdef _ZQUEST_SCALE_
      if (is_zquest())
      {
        if(myvsync)
        {
          if(zqwin_scale > 1)
          {
            stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
          }
          else
          {
            blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
          }
          myvsync=0;
        }
      }
      //	#endif
    }

    /* redraw in normal state */
    if(down)
    {
      if(d->flags&D_EXIT)
      {
        d->flags &= ~D_SELECTED;
        scare_mouse();
        object_message(d, MSG_DRAW, 0);
        unscare_mouse();
      }
    }

    /* should we close the dialog? */
    if(down && (d->flags & D_EXIT))
      return D_CLOSE;

    break;
  }

  return D_O_K;
}

/* jwin_edit_proc:
  *  An editable text object (the dp field points to the string). When it
  *  has the input focus (obtained by clicking on it with the mouse), text
  *  can be typed into this object. The d1 field specifies the maximum
  *  number of characters that it will accept, and d2 is the text cursor
  *  position within the string.
  */
int jwin_edit_proc(int msg, DIALOG *d, int c)
{
  int f, l, p, w, x, y, fg, bg;
  int b;
  int scroll;
  char *s;
  char buf[2];
  char nullbuf[2];
  sprintf(nullbuf, " ");
  if (d->dp==NULL)
  {
    d->dp=(void *)nullbuf;
  }
  s = (char*)d->dp;
  l = (int)strlen(s);
  if (d->d2 > l)
    d->d2 = l;

  /* calculate maximal number of displayable characters */
  b = x = 0;
  if (d->d2 == l)
  {
    buf[0] = ' ';
    buf[1] = 0;
    if (d->dp2)
      x = text_length((FONT*)d->dp2, buf);
    else
      x = text_length(font, buf);
  }

  buf[1] = 0;
  for (p=d->d2; p>=0; p--)
  {
    buf[0] = s[p];
    b++;
    if (d->dp2)
      x += text_length((FONT*)d->dp2, buf);
    else
      x += text_length(font, buf);
    if (x > d->w-6)
      break;
  }

  if (x <= d->w-6)
  {
    b = l;
    scroll = FALSE;
  }
  else
  {
    b--;
    scroll = TRUE;
  }

  FONT *oldfont = font;
  switch (msg)
  {

    case MSG_START:
      d->d2 = (int)strlen((char*)d->dp);;
      break;

    case MSG_DRAW:
      if (d->dp2)
      {
        font = (FONT*)d->dp2;
      }
      fg = (d->flags & D_DISABLED) || (d->flags & D_READONLY) ? scheme[jcMEDDARK] : scheme[jcTEXTFG];
      bg = (d->flags & D_DISABLED) || (d->flags & D_READONLY) ? scheme[jcBOX] : scheme[jcTEXTBG];
      x = 3;
      y = (d->h - text_height(font)) / 2 + d->y;

      /* first fill in the edges */

      if(y > d->y+2)
        rectfill(screen, d->x+2, d->y+2, d->x+d->w-3, y-1, bg);

      if(y+text_height(font)-1 < d->y+d->h-2)
        rectfill(screen, d->x+2, y+text_height(font)-1, d->x+d->w-3, d->y+d->h-3, bg);

      _allegro_vline(screen, d->x+2, d->y+2, d->y+d->h-3, bg);

      /* now the text */

      if (scroll)
      {
        p = d->d2-b+1;
        b = d->d2;
      }
      else
        p = 0;

      for (; p<=b; p++)
      {
        buf[0] = s[p] ? s[p] : ' ';
        w = text_length(font, buf);
        if (x+w > d->w)
          break;
        f = ((p == d->d2) && (d->flags & D_GOTFOCUS));
        textout_ex(screen, font, buf, d->x+x, y, f ? bg : fg,f ? fg : bg);
        x += w;
      }

      if (x < d->w-2)
        rectfill(screen, d->x+x, y, d->x+d->w-3, y+text_height(font)-1, bg);

      jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);
      font = oldfont;
      break;

    case MSG_CLICK:
      x = d->x+3;

      if (scroll)
      {
        p = d->d2-b+1;
        b = d->d2;
      }
      else
        p = 0;

      for (; p<b; p++)
      {
        buf[0] = s[p];
        x += text_length(font, buf);
        if (x > gui_mouse_x())
          break;
      }
      d->d2 = MID(0, p, l);
      scare_mouse();
      object_message(d, MSG_DRAW, 0);
      unscare_mouse();
      break;

    case MSG_WANTFOCUS:
    case MSG_LOSTFOCUS:
    case MSG_KEY:
      return D_WANTFOCUS;

    case MSG_CHAR:
    if ((c >> 8) == KEY_LEFT)
    {
      if (d->d2 > 0) d->d2--;
    }
    else if ((c >> 8) == KEY_RIGHT)
    {
      if (d->d2 < l) d->d2++;
    }
    else if ((c >> 8) == KEY_HOME)
    {
      d->d2 = 0;
    }
    else if ((c >> 8) == KEY_END)
    {
      d->d2 = l;
    }
    else if ((c >> 8) == KEY_DEL)
    {
      if (d->d2 < l)
        for (p=d->d2; s[p]; p++)
          s[p] = s[p+1];
    }
    else if ((c >> 8) == KEY_BACKSPACE)
    {
      if (d->d2 > 0)
      {
        d->d2--;
        for (p=d->d2; s[p]; p++)
          s[p] = s[p+1];
      }
    }
    else if ((c >> 8) == KEY_ENTER)
    {
      if (d->flags & D_EXIT)
      {
        scare_mouse();
        object_message(d, MSG_DRAW, 0);
        unscare_mouse();
        return D_CLOSE;
      }
      else
        return D_O_K;
    }
    else if ((c >> 8) == KEY_TAB)
    {
      return D_O_K;
    }
    else if (!(d->flags & D_READONLY))
    {
      c &= 0xFF;
      if ((c >= 32) && (c <= 255))
      {
        if (l < d->d1)
        {
          while (l >= d->d2)
          {
            s[l+1] = s[l];
            l--;
          }
          s[d->d2] = c;
          d->d2++;
        }
      }
      else
        return D_O_K;
    }

    /* if we changed something, better redraw... */
    scare_mouse();
    object_message(d, MSG_DRAW, 0);
    unscare_mouse();
    return D_USED_CHAR;
  }

  return D_O_K;
}

int jwin_hexedit_proc_old(int msg,DIALOG *d,int c)
{
  if(msg==MSG_CHAR)
    if(((isalpha(c&255) && !isxdigit(c&255))) || ispunct(c&255))
      return D_USED_CHAR;
    return jwin_edit_proc(msg,d,isalpha(c&255)?c&0xDF:c);
}

int jwin_hexedit_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_CHAR)
    if((isalpha(c&255) && !isxdigit(c&255)) || ispunct(c&255))
      return D_O_K;
    return jwin_edit_proc(msg,d,c);
}

int jwin_numedit_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_CHAR)
    if((isalnum(c&255) && !isdigit(c&255)) && ((c&255)!='-'))
      return D_O_K;
    return jwin_edit_proc(msg,d,c);
}

/*  _calc_scroll_bar:
  *   Helps find positions of buttons on the scroll bar.
  */
void _calc_scroll_bar(int h, int height, int listsize, int offset,
                             int *bh, int *len, int *pos)
{
  *bh = zc_max( zc_min( (h-4)/2, 14 ), 0 );
  *len = zc_max( ((h - 32) * height + listsize/2) / listsize , 6);
  *pos = ((h - 32 - *len) * offset) / (listsize-height);
}

/* _handle_scrollable_click:
  *  Helper to process a click on a scrollable object.
  */

void _handle_jwin_scrollable_scroll_click(DIALOG *d, int listsize, int *offset, FONT *fnt)
{
  enum { top_btn, bottom_btn, bar, top_bar, bottom_bar };

  int xx, yy;
  int height = (d->h-3) / text_height(fnt);
  int hh = d->h - 32;
  int obj = bar;
  int bh, len, pos;
  int down = 1, last_draw = 0;
  int redraw = 0, mouse_delay = 0;

  _calc_scroll_bar(d->h, height, listsize, *offset, &bh, &len, &pos);

  xx = d->x + d->w - 18;

  // find out which object is being clicked

  yy = gui_mouse_y();

  if (yy <= d->y+2+bh)
  {
    obj = top_btn;
    yy = d->y+2;
  }
  else if (yy >= d->y+d->h-2-bh)
    {
      obj = bottom_btn;
      yy = d->y+d->h-2-bh;
    }
    else if(d->h > 32+6)
      {
        if (yy < d->y+2+bh+pos)
          obj = top_bar;
        else if (yy >= d->y+2+bh+pos+len)
            obj = bottom_bar;
      }

      while (gui_mouse_b())
  {
    _calc_scroll_bar(d->h, height, listsize, *offset, &bh, &len, &pos);

    switch(obj)
    {
      case top_btn:
      case bottom_btn:
      down = mouse_in_rect(xx, yy, 16, bh);

      if(!down)
        mouse_delay = 0;
      else
      {
        if((mouse_delay&1)==0)
        {
          if(obj==top_btn && *offset>0)
          {
            (*offset)--;
            redraw = 1;
          }
          if(obj==bottom_btn && *offset<listsize-height)
          {
            (*offset)++;
            redraw = 1;
          }
        }
        mouse_delay++;
      }

      if(down!=last_draw || redraw)
      {
        vsync();
        scare_mouse();
        d->proc(MSG_DRAW, d, 0);
        draw_arrow_button(screen, xx, yy, 16, bh, obj==top_btn, down*3);
        unscare_mouse();
        last_draw = down;
      }
      break;

      case top_bar:
      case bottom_bar:
      if (mouse_in_rect(xx, d->y+2, 16, d->h-4))
      {
        if(obj==top_bar)
        {
          if(gui_mouse_y() < d->y+2+bh+pos)
            yy = *offset - height;
        }
        else
        {
          if(gui_mouse_y() >= d->y+2+bh+pos+len)
            yy = *offset + height;
        }

        if (yy < 0)
          yy = 0;

        if (yy > listsize-height)
          yy = listsize-height;

        if (yy != *offset)
        {
          *offset = yy;
          vsync();
          scare_mouse();
          d->proc(MSG_DRAW, d, 0);
          unscare_mouse();
        }
      }

      _calc_scroll_bar(d->h, height, listsize, *offset, &bh, &len, &pos);
      if (!mouse_in_rect(xx, d->y+2+bh+pos, 16, len))
        break;

      // fall through

      case bar:
      default:
      xx = gui_mouse_y() - pos;
      while (gui_mouse_b())
      {
        yy = (listsize * (gui_mouse_y() - xx) + hh/2) / hh;
        if (yy > listsize-height)
          yy = listsize-height;

        if (yy < 0)
          yy = 0;

        if (yy != *offset)
        {
          *offset = yy;
          scare_mouse();
          d->proc(MSG_DRAW, d, 0);
          unscare_mouse();
        }

        // let other objects continue to animate
        broadcast_dialog_message(MSG_IDLE, 0);
        //	#ifdef _ZQUEST_SCALE_
        if (is_zquest())
        {
          if(myvsync)
          {
            if(zqwin_scale > 1)
            {
              stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            else
            {
              blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
            myvsync=0;
          }
        }
        //	#endif
      }
      break;

    }                                                       // switch(obj)

    redraw = 0;
    //	#ifdef _ZQUEST_SCALE_
    if (is_zquest())
    {
      if(myvsync)
      {
        if(zqwin_scale > 1)
        {
          stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        else
        {
          blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
        myvsync=0;
      }
    }
    //	#endif
    // let other objects continue to animate
    broadcast_dialog_message(MSG_IDLE, 0);
  }

  if(last_draw==1)
  {
    scare_mouse();
    draw_arrow_button(screen, xx, yy, 16, bh, obj==top_btn, 0);
    unscare_mouse();
  }
}

/* _handle_scrollable_scroll:
  *  Helper function to scroll through a scrollable object.
  */

static void _handle_jwin_scrollable_scroll(DIALOG *d, int listsize, int *index, int *offset, FONT *fnt)
{
  int height = (d->h-3) / text_height(fnt);

  if (listsize <= 0)
  {
    *index = *offset = 0;
    return;
  }

  // check selected item
  if (*index < 0)
    *index = 0;
  else if (*index >= listsize)
      *index = listsize - 1;

    // check scroll position
    while ((*offset > 0) && (*offset + height > listsize))
    (*offset)--;

  if (*offset >= *index)
  {
    if (*index < 0)
      *offset = 0;
    else
      *offset = *index;
  }
  else
  {
    while ((*offset + height - 1) < *index)
      (*offset)++;
  }
}

/* idle_cb:
  *  rest_callback() routine to keep dialogs animating nice and smoothly.
  */

static void idle_cb()
{
  broadcast_dialog_message(MSG_IDLE, 0);
}

/* _handle_listbox_click:
  *  Helper to process a click on a listbox, doing hit-testing and moving
  *  the selection.
  */

static void _handle_jwin_listbox_click(DIALOG *d)
{
  ListData *data = (ListData *)d->dp;
  char *sel = (char *)d->dp2;
  int listsize, height;
  int i, j;

  (*data->listFunc)(-1, &listsize);
  if (!listsize)
    return;

  height = (d->h-3) / text_height(*data->font);

  i = MID(0, ((gui_mouse_y() - d->y - 4) / text_height(*data->font)),
          ((d->h-3) / text_height(*data->font) - 1));
  i += d->d2;
  if (i < d->d2)
    i = d->d2;
  else
  {
    if (i > d->d2 + height-1)
      i = d->d2 + height-1;
    if (i >= listsize)
      i = listsize-1;
  }

  if (gui_mouse_y() <= d->y)
    i = MAX(i-1, 0);
  else if (gui_mouse_y() >= d->y+d->h)
      i = MIN(i+1, listsize-1);

    if (i != d->d1)
  {
    if (sel)
    {
      if (key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG))
      {
        if ((key_shifts & KB_SHIFT_FLAG) || (d->flags & D_INTERNAL))
        {
          for (j=MIN(i, d->d1); j<=MAX(i, d->d1); j++)
            sel[j] = TRUE;
        }
        else
          sel[i] = TRUE;
      }
    }

    d->d1 = i;
    i = d->d2;
    _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);

    scare_mouse();
    object_message(d, MSG_DRAW, 0);
    unscare_mouse();

    if (i != d->d2)
      rest_callback(MID(10, text_height(font)*16-d->h, 100), idle_cb);
  }
}

/* _jwin_draw_scrollable_frame:
  *  Helper function to draw a frame for all objects with vertical scrollbars.
  */
void _jwin_draw_scrollable_frame(DIALOG *d, int listsize, int offset, int height, int type)
{
  int pos, len;
  int xx, yy, hh, bh;
  static BITMAP *pattern = NULL;                            // just create it once

  /* draw frame */
  if(type)
    // for droplists
    jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DARK);
  else
    jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);

  /* possibly draw scrollbar */
  if (listsize > height)
  {
    _calc_scroll_bar(d->h, height, listsize, offset, &bh, &len, &pos);

    xx = d->x + d->w - 18;

    draw_arrow_button(screen, xx, d->y+2, 16, bh, 1, 0);
    draw_arrow_button(screen, xx, d->y+d->h-2-bh, 16, bh, 0, 0);

    if(d->h > 32)
    {
      yy = d->y + 16;
      hh = (d->h-32);

      /* create and draw the scrollbar */
      if(!pattern)
        pattern = create_bitmap_ex(bitmap_color_depth(screen),2,2);

      putpixel(pattern, 0, 1, scheme[jcLIGHT]);
      putpixel(pattern, 1, 0, scheme[jcLIGHT]);
      putpixel(pattern, 0, 0, scheme[jcBOX]);
      putpixel(pattern, 1, 1, scheme[jcBOX]);

      drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
      rectfill(screen, xx, yy, xx+15, yy+hh-1, 0);
      solid_mode();

      if(d->h > 32+6)
      {
        jwin_draw_button(screen, xx, yy+pos, 16, len, 0, 1);
      }
    }

    if (d->flags & D_GOTFOCUS)
      _dotted_rect(d->x+2, d->y+2, d->x+d->w-19, d->y+d->h-3, scheme[jcTEXTFG], scheme[jcTEXTBG]);
  }
  else
    if (d->flags & D_GOTFOCUS)
      _dotted_rect(d->x+2, d->y+2, d->x+d->w-3, d->y+d->h-3, scheme[jcTEXTFG], scheme[jcTEXTBG]);
}

/* _jwin_draw_listbox:
  *  Helper function to draw a listbox object.
  */
void _jwin_draw_listbox(DIALOG *d)
{
  int height, listsize, i, len, bar, x, y, w;
  int fg_color, bg_color, fg, bg;
  char *sel = (char*)d->dp2;
  char s[1024];
  ListData *data = (ListData *)d->dp;

  (*data->listFunc)(-1, &listsize);
  height = (d->h-3) / text_height(*data->font);
  bar = (listsize > height);
  w = (bar ? d->w-21 : d->w-5);
  fg_color = (d->flags & D_DISABLED) ? scheme[jcMEDDARK] : (d->fg ? d->fg : scheme[jcTEXTFG]);
  bg_color = (d->flags & D_DISABLED) ? scheme[jcBOX] : (d->bg ? d->bg : scheme[jcTEXTBG]);

  rectfill(screen, d->x+2,  d->y+2, d->x+w+2, d->y+3, bg_color);
  _allegro_vline(screen, d->x+2, d->y+4, d->y+d->h-3, bg_color);
  _allegro_vline(screen, d->x+3, d->y+4, d->y+d->h-3, bg_color);
  _allegro_vline(screen, d->x+w+1, d->y+4, d->y+d->h-3, bg_color);
  _allegro_vline(screen, d->x+w+2, d->y+4, d->y+d->h-3, bg_color);

  /* draw box contents */
  for (i=0; i<height; i++)
  {
    if (d->d2+i < listsize)
    {
      if (d->d2+i == d->d1 && !(d->flags & D_DISABLED))
      {
        fg = scheme[jcSELFG];
        bg = scheme[jcSELBG];
      }
      else if ((sel) && (sel[d->d2+i]))
        {
          fg = scheme[jcMEDDARK];
          bg = scheme[jcSELBG];
        }
        else
        {
          fg = fg_color;
          bg = bg_color;
        }
      strncpy(s, (*data->listFunc)(i+d->d2, NULL), 1023);
      x = d->x + 4;
      y = d->y + 4 + i*text_height(*data->font);
      //         text_mode(bg);
      rectfill(screen, x, y, x+7, y+text_height(*data->font)-1, bg);
      x += 8;
      len = (int)strlen(s);
      while (text_length(*data->font, s) >= d->w - (bar ? 26 : 10))
      {
        len--;
        s[len] = 0;
      }
      textout_ex(screen, *data->font, s, x, y, fg,bg);
      x += text_length(*data->font, s);
      if (x <= d->x+w)
        rectfill(screen, x, y, d->x+w, y+text_height(*data->font)-1, bg);
    }
    else
      rectfill(screen, d->x+2,  d->y+4+i*text_height(*data->font),
               d->x+w+2, d->y+3+(i+1)*text_height(*data->font), bg_color);
  }

  if (d->y+4+i*text_height(font) <= d->y+d->h-3)
    rectfill(screen, d->x+2, d->y+4+i*text_height(*data->font),
             d->x+w+2, d->y+d->h-3, bg_color);

  /* draw frame, maybe with scrollbar */
  _jwin_draw_scrollable_frame(d, listsize, d->d2, height, (d->flags&D_USER)?1:0);
}

/* jwin_list_proc:
  *  A list box object. The dp field points to a ListData struct containing
  *  a function which it will call
  *  to obtain information about the list. This should follow the form:
  *     char *<list_func_name> (int index, int *list_size);
  *  If index is zero or positive, the function should return a pointer to
  *  the string which is to be displayed at position index in the list. If
  *  index is  negative, it should return null and list_size should be set
  *  to the number of items in the list. The list box object will allow the
  *  user to scroll through the list and to select items list by clicking
  *  on them, and if it has the input focus also by using the arrow keys. If
  *  the D_EXIT flag is set, double clicking on a list item will cause it to
  *  close the dialog. The index of the selected item is held in the d1
  *  field, and d2 is used to store how far it has scrolled through the list.
  */
int jwin_list_proc(int msg, DIALOG *d, int c)
{
  ListData *data = (ListData *)d->dp;
  int listsize, i, bottom, height, bar, orig;
  char *sel = (char *)d->dp2;
  int redraw = FALSE;

  switch (msg)
  {

    case MSG_START:
    (*data->listFunc)(-1, &listsize);
    _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);
    break;

    case MSG_DRAW:
    _jwin_draw_listbox(d);
    break;

    case MSG_CLICK:
    (*data->listFunc)(-1, &listsize);
    height = (d->h-3) / text_height(*data->font);
    bar = (listsize > height);
    if ((!bar) || (gui_mouse_x() < d->x+d->w-18))
    {
      if ((sel) && (!(key_shifts & KB_CTRL_FLAG)))
      {
        for (i=0; i<listsize; i++)
        {
          if (sel[i])
          {
            redraw = TRUE;
            sel[i] = FALSE;
          }
        }
        if (redraw)
        {
          scare_mouse();
          object_message(d, MSG_DRAW, 0);
          unscare_mouse();
        }
      }
      _handle_jwin_listbox_click(d);
      while (gui_mouse_b())
      {
        broadcast_dialog_message(MSG_IDLE, 0);
        d->flags |= D_INTERNAL;
        _handle_jwin_listbox_click(d);
        d->flags &= ~D_INTERNAL;
        //	#ifdef _ZQUEST_SCALE_
        if (is_zquest())
        {
          if(myvsync)
          {
            if(zqwin_scale > 1)
            {
              stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            else
            {
              blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
            myvsync=0;
          }
        }
        //	#endif
      }

      if (d->flags & D_USER)
      {
        if (listsize)
        {
          clear_keybuf();
          return D_CLOSE;
        }
      }
    }
    else
    {
      _handle_jwin_scrollable_scroll_click(d, listsize, &d->d2, *data->font);
    }
    break;

    case MSG_DCLICK:
    (*data->listFunc)(-1, &listsize);
    height = (d->h-3) / text_height(*data->font);
    bar = (listsize > height);
    if ((!bar) || (gui_mouse_x() < d->x+d->w-18))
    {
      if (d->flags & D_EXIT)
      {
        if (listsize)
        {
          i = d->d1;
          object_message(d, MSG_CLICK, 0);
          if (i == d->d1)
            return D_CLOSE;
        }
      }
    }
    break;

    case MSG_KEY:
    (*data->listFunc)(-1, &listsize);
    if ((listsize) && (d->flags & D_EXIT))
      return D_CLOSE;
    break;

    case MSG_WANTFOCUS:
    return D_WANTFOCUS;

    case MSG_WHEEL:
    (*data->listFunc)(-1, &listsize);
    height = (d->h-4) / text_height(*data->font);
    if (height < listsize) {
      int delta = (height > 3) ? 3 : 1;
      if (c > 0)
      {
        i = MAX(0, d->d2-delta);
      }
      else
      {
        i = MIN(listsize-height, d->d2+delta);
      }
      if (i != d->d2)
      {
        d->d2 = i;
        object_message(d, MSG_DRAW, 0);
      }
    }
    break;

    case MSG_CHAR:
    (*data->listFunc)(-1,&listsize);
    if (listsize)
    {
      c >>= 8;

      bottom = d->d2 + (d->h-3)/text_height(*data->font) - 1;
      if (bottom >= listsize-1)
        bottom = listsize-1;

      orig = d->d1;

      if (c == KEY_UP)
        d->d1--;
      else if (c == KEY_DOWN)
          d->d1++;
        else if (c == KEY_HOME)
            d->d1 = 0;
          else if (c == KEY_END)
              d->d1 = listsize-1;
            else if (c == KEY_PGUP)
              {
                if (d->d1 > d->d2)
                  d->d1 = d->d2;
                else
                  d->d1 -= (bottom - d->d2);
              }
              else if (c == KEY_PGDN)
                {
                  if (d->d1 < bottom)
                    d->d1 = bottom;
                  else
                    d->d1 += (bottom - d->d2);
                }
                else
                  return D_O_K;

      if (sel)
      {
        if (!(key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG)))
        {
          for (i=0; i<listsize; i++)
            sel[i] = FALSE;
        }
        else if (key_shifts & KB_SHIFT_FLAG)
          {
            for (i=MIN(orig, d->d1); i<=MAX(orig, d->d1); i++)
            {
              if (key_shifts & KB_CTRL_FLAG)
                sel[i] = (i != d->d1);
              else
                sel[i] = TRUE;
            }
          }
      }

      /* if we changed something, better redraw... */
      _handle_jwin_scrollable_scroll(d, listsize, &d->d1, &d->d2, *data->font);
      scare_mouse();
      object_message(d, MSG_DRAW, 0);
      unscare_mouse();
      return D_USED_CHAR;
    }
    break;
  }

  return D_O_K;
}

/* _jwin_draw_textbox:
  *  Helper function to draw a textbox object.
  */
void _jwin_draw_textbox(char *thetext, int *listsize, int draw, int offset,
                        int wword, int tabsize, int x, int y, int w, int h,
                        int disabled)
{
  int fg = scheme[jcTEXTFG];
  int bg = scheme[jcTEXTBG];
  int y1 = y+4;
  int x1;
  int len;
  int ww = w-10;
  char s[16];
  char text[16];
  char space[16];
  char *printed = text;
  char *scanned = text;
  char *oldscan = text;
  char *ignore = NULL;
  char *tmp, *ptmp;
  int width;
  int line = 0;
  int i = 0;
  int noignore;
  //   int rtm;

  usetc(s+usetc(s, '.'), 0);
  usetc(text+usetc(text, ' '), 0);
  usetc(space+usetc(space, ' '), 0);

  /* find the correct text */
  if (thetext != NULL)
  {
    printed = thetext;
    scanned = thetext;
  }

  /* choose the text color */
  if (disabled)
  {
    fg = scheme[jcMEDDARK];
    bg = scheme[jcBOX];
  }

  /* do some drawing setup */
  if (draw)
  {
    /* initial start blanking at the top */
    rectfill(screen, x+2, y+2, x+w-2, y1-1, bg);
  }

  //   rtm = text_mode(bg);

  /* loop over the entire string */
  for(;;)
  {
    width = 0;

    /* find the next break */
    while (ugetc(scanned))
    {
      /* check for a forced break */
      if (ugetc(scanned) == '\n')
      {
        scanned += uwidth(scanned);

        /* we are done parsing the line end */
        break;
      }

      /* the next character length */
      usetc(s+usetc(s, ugetc(scanned)), 0);
      len = text_length(font, s);

      /* modify length if its a tab */
      if (ugetc(s) == '\t')
        len = tabsize * text_length(font, space);

      /* check for the end of a line by excess width of next char */
      if (width+len >= ww)
      {
        /* we have reached end of line do we go back to find start */
        if (wword)
        {
          /* remember where we were */
          oldscan = scanned;
          noignore = FALSE;

          /* go backwards looking for start of word */
          while (!uisspace(ugetc(scanned)))
          {
            /* don't wrap too far */
            if (scanned == printed)
            {
              /* the whole line is filled, so stop here */
              tmp = ptmp = scanned;
              while (ptmp != oldscan)
              {
                ptmp = tmp;
                tmp += uwidth(tmp);
              }
              scanned = ptmp;
              noignore = TRUE;
              break;
            }
            /* look further backwards to wrap */
            tmp = ptmp = printed;
            while (tmp < scanned)
            {
              ptmp = tmp;
              tmp += uwidth(tmp);
            }
            scanned = ptmp;
          }
          /* put the space at the end of the line */
          if (!noignore)
          {
            ignore = scanned;
            scanned += uwidth(scanned);
          }
          else
            ignore = NULL;

          /* check for endline at the convenient place */
          if (ugetc(scanned) == '\n')
            scanned += uwidth(scanned);
        }
        /* we are done parsing the line end */
        break;
      }

      /* the character can be added */
      scanned += uwidth(scanned);
      width += len;
    }

    /* check if we are to print it */
    if ((draw) && (line >= offset) && (y1+text_height(font) < (y+h-3)))
    {
      x1 = x+4;

      /* the initial blank bit */
      rectfill(screen, x+2, y1, x1-1, y1+text_height(font), bg);

      /* print up to the marked character */
      while (printed != scanned)
      {
        /* do special stuff for each character */
        switch (ugetc(printed))
        {

          case '\r':
          case '\n':
          /* don't print endlines in the text */
          break;

          /* possibly expand the tabs */
          case '\t':
          for (i=0; i<tabsize; i++)
          {
            usetc(s+usetc(s, ' '), 0);
            textout_ex(screen, font, s, x1, y1, fg,bg);
            x1 += text_length(font, s);
          }
          break;

          /* print a normal character */
          default:
          if (printed != ignore)
          {
            usetc(s+usetc(s, ugetc(printed)), 0);
            textout_ex(screen, font, s, x1, y1, fg,bg);
            x1 += text_length(font, s);
          }
        }

        /* goto the next character */
        printed += uwidth(printed);
      }
      /* the last blank bit */
      if (x1 <= x+w-3)
        rectfill(screen, x1, y1, x+w-2, y1+text_height(font)-1, bg);

      /* print the line end */
      y1 += text_height(font);
    }
    printed = scanned;

    /* we have done a line */
    line++;

    /* check if we are at the end of the string */
    if (!ugetc(printed))
    {
      /* the under blank bit */
      if (draw)
        rectfill(screen, x+1, y1, x+w-2, y+h-1, bg);

      /* tell how many lines we found */
      *listsize = line;
      //	 text_mode(rtm);
      return;
    }
  }

  //   text_mode(rtm);
}

/* jwin_textbox_proc:
  *  A text box object. The dp field points to a char * which is the text
  *  to be displayed in the text box. If the text is long, there will be
  *  a vertical scrollbar on the right hand side of the object which can
  *  be used to scroll through the text. The default is to print the text
  *  with word wrapping, but if the D_SELECTED flag is set, the text will
  *  be printed with character wrapping. The d1 field is used internally
  *  to store the number of lines of text, and d2 is used to store how far
  *  it has scrolled through the text.
  */
int jwin_textbox_proc(int msg, DIALOG *d, int c)
{
  int height, bar, ret = D_O_K;
  int start, top, bottom,l;
  int used, delta;
  //   int fg_color = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;

  FONT *oldfont=NULL;
  if(d->dp2!=NULL)
  {
    oldfont=font;
    font=(FONT*)d->dp2;
  }

  /* calculate the actual height */
  height = (d->h-4) / text_height(font);

  switch (msg)
  {

    case MSG_START:
    /* measure how many lines of text we contain */
    _jwin_draw_textbox((char*)d->dp, &d->d1,
                       0,                                                  /* DONT DRAW anything */
                       d->d2, !(d->flags & D_SELECTED), 8,
                       d->x, d->y, d->w, d->h,
                       (d->flags & D_DISABLED));
    break;

    case MSG_DRAW:
    /* tell the object to sort of draw, but only calculate the listsize */
    _jwin_draw_textbox((char*)d->dp, &d->d1,
                       0,                                                  /* DONT DRAW anything */
                       d->d2, !(d->flags & D_SELECTED), 8,
                       d->x, d->y, d->w, d->h,
                       (d->flags & D_DISABLED));

    if (d->d1 > height)
    {
      bar = 16;
    }
    else
    {
      bar = 0;
      d->d2 = 0;
    }

    /* now do the actual drawing */
    _jwin_draw_textbox((char*)d->dp, &d->d1, 1, d->d2,
                       !(d->flags & D_SELECTED), 8,
                       d->x, d->y, d->w-bar-1, d->h,
                       (d->flags & D_DISABLED));

    /* draw the frame around */
    _jwin_draw_scrollable_frame(d, d->d1, d->d2, height, 0);
    break;

    case MSG_CLICK:
    /* figure out if it's on the text or the scrollbar */
    bar = (d->d1 > height);

    if ((!bar) || (gui_mouse_x() < d->x+d->w-18))
    {
      /* clicked on the text area */
      ret = D_O_K;
    }
    else
    {
      /* clicked on the scroll area */
      _handle_jwin_scrollable_scroll_click(d, d->d1, &d->d2, font);
    }
    break;

    case MSG_WHEEL:
    l = (d->h-8)/text_height(font);
    delta = (l > 3) ? 3 : 1;

    // scroll, making sure that the list stays in bounds
    start = d->d2;
    d->d2 = (c > 0) ? MAX(0, d->d2-delta) : MIN(d->d1-l, d->d2+delta);

    // if we changed something, better redraw...
    if (d->d2 != start)
    {
      d->flags |= D_DIRTY;
    }

    ret = D_O_K;
    break;

    case MSG_CHAR:
    start = d->d2;
    used = D_USED_CHAR;

    if (d->d1 > 0)
    {
      if (d->d2 > 0)
        top = d->d2+1;
      else
        top = 0;

      l = (d->h-3)/text_height(font);

      bottom = d->d2 + l - 1;
      if (bottom >= d->d1-1)
        bottom = d->d1-1;
      else
        bottom--;

      if ((c>>8) == KEY_UP)
        d->d2--;
      else if ((c>>8) == KEY_DOWN)
          d->d2++;
        else if ((c>>8) == KEY_HOME)
            d->d2 = 0;
          else if ((c>>8) == KEY_END)
              d->d2 = d->d1-l;
            else if ((c>>8) == KEY_PGUP)
                d->d2 = d->d2-(bottom-top);
              else if ((c>>8) == KEY_PGDN)
                  d->d2 = d->d2+(bottom-top);
                else
                  used = D_O_K;

      /* make sure that the list stays in bounds */
      if (d->d2 > d->d1-l)
        d->d2 = d->d1-l;
      if (d->d2 < 0)
        d->d2 = 0;
    }
    else
      used = D_O_K;

    /* if we changed something, better redraw... */
    if (d->d2 != start)
    {
      scare_mouse();
      d->proc(MSG_DRAW, d, 0);
      unscare_mouse();
    }
    ret = used;
    break;

    case MSG_WANTFOCUS:
    /* if we don't have a scrollbar we can't do anything with the focus */
    if (d->d1 > height)
      ret = D_WANTFOCUS;
    break;

    default:
    ret = D_O_K;
  }

  if(d->dp2!=NULL)
  {
    font=oldfont;
  }

  return ret;
}

/* jwin_slider_proc:
  *  A slider control object. This object returns a value in d2, in the
  *  range from 0 to d1. It will display as a vertical slider if h is
  *  greater than or equal to w; otherwise, it will display as a horizontal
  *  slider. dp can contain an optional bitmap to use for the slider handle;
  *  dp2 can contain an optional callback function, which is called each
  *  time d2 changes. The callback function should have the following
  *  prototype:
  *
  *  int function(void *dp3, int d2);
  *
  *  The d_slider_proc object will return the value of the callback function.
  */
int jwin_slider_proc(int msg, DIALOG *d, int c)
{
  BITMAP *slhan = NULL;
  int sfg;                /* slider foreground color */
  int vert = TRUE;        /* flag: is slider vertical? */
  int hh = 7;             /* handle height (width for horizontal sliders) */
  int hmar;               /* handle margin */
  int slp;                /* slider position */
  int irange;
  int slx, sly, slh, slw;
  fixed slratio, slmax, slpos;
  ASSERT(d);

  /* check for slider direction */
  if (d->h < d->w)
  {
    vert = FALSE;
  }

  /* set up the metrics for the control */
  if (d->dp != NULL)
  {
    slhan = (BITMAP *)d->dp;
    if (vert)
    {
      hh = slhan->h;
    }
    else
    {
      hh = slhan->w;
    }
  }

  hmar = hh/2;
  irange = (vert) ? d->h : d->w;
  slmax = itofix(irange-hh);
  slratio = slmax / (d->d1);
  slpos = slratio * d->d2;
  slp = fixtoi(slpos);

  switch (msg) {
    case MSG_DRAW:
    //      sfg = (d->flags & D_DISABLED) ? gui_mg_color : scheme[jcBOXFG];
    sfg = (d->flags & D_DISABLED) ? scheme[jcMEDDARK] : scheme[jcBOXFG];
    if (vert) {
      rectfill(screen, d->x, d->y, d->x+d->w/2-2, d->y+d->h, scheme[jcBOX]);
      rectfill(screen, d->x+d->w/2-1, d->y, d->x+d->w/2+1, d->y+d->h, sfg);
      rectfill(screen, d->x+d->w/2+2, d->y, d->x+d->w, d->y+d->h, scheme[jcBOX]);
    }
    else {
      rectfill(screen, d->x, d->y, d->x+d->w, d->y+d->h/2-2, scheme[jcBOX]);
      rectfill(screen, d->x, d->y+d->h/2-1, d->x+d->w, d->y+d->h/2+1, sfg);
      rectfill(screen, d->x, d->y+d->h/2+2, d->x+d->w, d->y+d->h, scheme[jcBOX]);
    }
    if (d->flags & D_GOTFOCUS)
    {
      _dotted_rect(d->x, d->y, d->x+d->w, d->y+d->h, sfg, scheme[jcBOX]);
    }
    /* okay, background and slot are drawn, now draw the handle */
    if (slhan)
    {
      if (vert)
      {
        slx = d->x+(d->w/2)-(slhan->w/2);
        sly = d->y+d->h-(hh+slp);
      }
      else
      {
        slx = d->x+slp;
        sly = d->y+(d->h/2)-(slhan->h/2);
      }
      draw_sprite(screen, slhan, slx, sly);
    }
    else
    {
      /* draw default handle */
      if (vert)
      {
        slx = d->x;
        sly = d->y+d->h-(hh+slp);
        slw = d->w;
        slh = hh;
      }
      else
      {
        slx = d->x+slp;
        sly = d->y;
        slw = hh;
        slh = d->h;
      }
      jwin_draw_button(screen, slx, sly, slw+1, slh+1, d->flags&D_DISABLED?3:0, 0);
    }
    break;
    default:
    return d_jslider_proc(msg, d, c);
  }
  return D_O_K;
}

/*********************/
/*****   Menus   *****/
/*********************/

typedef struct MENU_INFO                                    /* information about a popup menu */
{
  MENU *menu;                                               /* the menu itself */
  struct MENU_INFO *parent;                                 /* the parent menu, or NULL for root */
  int bar;                                                  /* set if it is a top level menu bar */
  int size;                                                 /* number of items in the menu */
  int sel;                                                  /* selected item */
  int hover;                                                /* set if is bar and child menu not down */
  int x, y, w, h;                                           /* screen position of the menu */
  int (*proc)();                                            /* callback function */
  BITMAP *saved;                                            /* saved what was underneath it */
} MENU_INFO;

/* get_menu_pos:
  *  Calculates the coordinates of an object within a top level menu bar.
  */
static void get_menu_pos(MENU_INFO *m, int c, int *x, int *y, int *w)
{
  int c2;

  *x = m->x + ((m->bar) ? 1 : 3);

  if (m->bar)
  {
    for (c2=0; c2<c; c2++)
      *x += gui_strlen(m->menu[c2].text) + 16;

    *y = m->y+1;
    *w = gui_strlen(m->menu[c].text) + 16;
  }
  else
  {
    *y = m->y + c*(text_height(font)+4) + 3;
    *w = m->w - 6;
  }
}

/* draw_menu_item:
  *  Draws an item from a popup menu onto the screen.
  */
static void draw_menu_item(MENU_INFO *m, int c)
{
  int fg, bg;
  int i, x, y, w, d=0;
  int yofs = (m->bar) ? 1 : 0;
  int h = text_height(font) + 4 + yofs;
  int g = 0;
  char buf[80], *tok;
  int my;

  fg = scheme[jcBOXFG];
  bg = scheme[jcBOX];

  if(m->menu[c].flags & D_DISABLED)
  {
    fg = scheme[jcMEDDARK];
    d = 1;
  }

  if(c == m->sel)
  {
    if(m->bar)
      g = !m->hover;
    else
    {
      if(d)
        d = 0;
      else
        fg = scheme[jcSELFG];
      bg = scheme[jcSELBG];
    }
  }

  get_menu_pos(m, c, &x, &y, &w);

  rectfill(screen, x, y, x+w-1, y+h-1, bg);
  //   text_mode(-1);

  if(m->menu[c].text[0] == 0)
  {
    _allegro_hline(screen, x+1, y+text_height(font)/2+2, x+w-2, scheme[jcMEDDARK]);
    _allegro_hline(screen, x+1, y+text_height(font)/2+3, x+w-2, scheme[jcLIGHT]);
  }
  else
  {
    for (i=0; (m->menu[c].text[i]) && (m->menu[c].text[i] != '\t'); i++)
      buf[i] = m->menu[c].text[i];
    buf[i] = 0;

    if(d)
      gui_textout_ex(screen, buf, x+9, y+yofs+2, scheme[jcLIGHT], -1, FALSE);

    gui_textout_ex(screen, buf, x+8+g, y+yofs+1+g, fg, -1, FALSE);

    if (m->menu[c].text[i] == '\t')
    {
      tok = m->menu[c].text+i+1;

      if(d)
        gui_textout_ex(screen, tok, x+w-gui_strlen(tok)-7, y+yofs+2, scheme[jcLIGHT], -1, FALSE);

      gui_textout_ex(screen, tok, x+w-gui_strlen(tok)-8+g, y+yofs+1+g, fg, -1, FALSE);
    }

    if (((m->menu[c].child) && (!m->bar))) {
      my = y + text_height(font)/2;
      putpixel(screen, x+w-7, my-2, fg);
      _allegro_hline(screen, x+w-7, my-1, x+w-6, fg);
      _allegro_hline(screen, x+w-7, my+0, x+w-5, fg);
      _allegro_hline(screen, x+w-7, my+1, x+w-4, fg);
      _allegro_hline(screen, x+w-7, my+2, x+w-5, fg);
      _allegro_hline(screen, x+w-7, my+3, x+w-6, fg);
      putpixel(screen, x+w-7, my+4, fg);
      if (d)
      {
		line(screen, x+w-6, my+5, x+w-3, my+2, scheme[jcLIGHT]);
        line(screen, x+w-6, my+4, x+w-4, my+2, scheme[jcLIGHT]);
      }
    }
  }

  if((c == m->sel) && m->bar)
  {
    int c1 = scheme[jcLIGHT];
    int c2 = scheme[jcMEDDARK];

    if(m->hover == 0)
    {
      c1 = scheme[jcMEDDARK];
      c2 = scheme[jcLIGHT];
    }

    _allegro_hline(screen, x,     y,     x+w-2, c1);
    _allegro_vline(screen, x,     y+1,   y+h-2, c1);
    _allegro_hline(screen, x,     y+h-1, x+w-1, c2);
    _allegro_vline(screen, x+w-1, y,     y+h-2, c2);
  }

  if (m->menu[c].flags & D_SELECTED)
  {
    line(screen, x+1, y+text_height(font)/2+1, x+3, y+text_height(font)+1, fg);
    line(screen, x+3, y+text_height(font)+1, x+6, y+2, fg);
  }
}

/* draw_menu:
  *  Draws a popup menu onto the screen.
  */
static void draw_menu(MENU_INFO *m)
{
  int c;

  if(m->bar)
	  rectfill(screen, zc_max(0,m->x), zc_max(0,m->y), zc_min(m->x+m->w-1, screen->w), zc_min(m->y+m->h-1, screen->h), scheme[jcBOX]);
  else
    jwin_draw_win(screen, m->x, m->y, m->w, m->h, FR_WIN);

  for (c=0; m->menu[c].text; c++)
    draw_menu_item(m, c);
}

/* menu_mouse_object:
  *  Returns the index of the object the mouse is currently on top of.
  */
static int menu_mouse_object(MENU_INFO *m)
{
  int c;
  int x, y, w;

  for (c=0; c<m->size; c++)
  {
    get_menu_pos(m, c, &x, &y, &w);

    if ((gui_mouse_x() >= x) && (gui_mouse_x() < x+w) &&
        (gui_mouse_y() >= y) && (gui_mouse_y() < y+(text_height(font)+4)))
      return (m->menu[c].text[0]) ? c : -1;
  }

  return -1;
}

/* mouse_in_parent_menu:
  *  Recursively checks if the mouse is inside a menu or any of its parents.
  */
static int mouse_in_parent_menu(MENU_INFO *m)
{
  int c;

  if (!m)
    return FALSE;

  c = menu_mouse_object(m);
  if ((c >= 0) && (c != m->sel))
    return TRUE;

  return mouse_in_parent_menu(m->parent);
}

/* fill_menu_info:
  *  Fills a menu info structure when initialising a menu.
  */
static void fill_menu_info(MENU_INFO *m, MENU *menu, MENU_INFO *parent, int bar, int x, int y, int minw, int minh)
{
  int c, i;
  int extra = 0;
  char buf[80], *tok;
  int border = (bar)?0:1;

  m->menu = menu;
  m->parent = parent;
  m->bar = bar;
  m->x = x - border*2;
  m->y = y - border*2;
  m->w = 2 + border*4;

  m->h = (m->bar) ? (text_height(font)+7) : 2 + border*4;

  m->proc = NULL;
  m->sel = -1;
  m->hover = 0;

  /* calculate size of the menu */
  for (m->size=0; m->menu[m->size].text; m->size++)
  {
    for (i=0; (m->menu[m->size].text[i]) && (m->menu[m->size].text[i] != '\t'); i++)
      buf[i] = m->menu[m->size].text[i];
    buf[i] = 0;

    c = gui_strlen(buf);

    if (m->bar)
    {
      m->w += c+16;
    }
    else
    {
      m->h += text_height(font)+4;
      m->w = MAX(m->w, c+16);
    }

    if (m->menu[m->size].text[i] == '\t')
    {
      tok = m->menu[m->size].text+i+1;
      c = gui_strlen(tok);
      extra = MAX(extra, c);
    }
  }

  if (extra)
    m->w += extra+16;

  m->w = MAX(m->w, minw);
  m->h = MAX(m->h, minh);
}

/* menu_key_shortcut:
  *  Returns true if c is indicated as a keyboard shortcut by a '&' character
  *  in the specified string.
  */
static int menu_key_shortcut(int c, AL_CONST char *s)
{
  int d;

  while ((d = ugetxc(&s)) != 0)
  {
    if (d == '&')
    {
      d = ugetc(s);
      if ((d != '&') && (utolower(d) == utolower(c & 0xFF)))
        return TRUE;
    }
  }

  return FALSE;
}

/* menu_alt_key:
  *  Searches a menu for keyboard shortcuts, for the alt+letter to bring
  *  up a menu.
  */

//#ifdef ALLEGRO_DOS
//int menu_alt_key(int k, MENU *m);
//#elif defined ALLEGRO_WINDOWS

int menu_alt_key(int k, MENU *m)
{
  static unsigned char alt_table[] =
  {
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z
  };

  AL_CONST char *s;
  int c, d;

  if (k & 0xFF)
    return 0;

  k >>= 8;

  c = scancode_to_ascii(k);
  if (c)
  {
    k = c;
  }
  else
  {
    for (c=0; c<(int)sizeof(alt_table); c++)
    {
      if (k == alt_table[c])
      {
        k = c + 'a';
        break;
      }
    }

    if (c >= (int)sizeof(alt_table))
      return 0;
  }

  for (c=0; m[c].text; c++)
  {
    s = m[c].text;
    while ((d = ugetxc(&s)) != 0)
    {
      if (d == '&')
      {
        d = ugetc(s);
        if ((d != '&') && (utolower(d) == utolower(k)))
          return k;
      }
    }
  }

  return 0;
}


//#elif defined ALLEGRO_LINUX
//int menu_alt_key(int k, MENU *m);
//#elif defined ALLEGRO_MACOSX
//int menu_alt_key(int k, MENU *m);
//#endif

/* _jwin_do_menu:
  *  The core menu control function, called by jwin_do_menu() and jwin_menu_proc().
  *
  *  I added joystick support.
  */
int _jwin_do_menu(MENU *menu, MENU_INFO *parent, int bar, int x, int y, int repos, int *dret, int minw, int minh)
{
  MENU_INFO m;
  MENU_INFO *i;
  int c, c2;
  int ret = -1;
  int mouse_on = gui_mouse_b();
  int joy_on = joy[0].stick[0].axis[0].d1 + joy[0].stick[0].axis[0].d2 +
    joy[0].stick[0].axis[1].d1 + joy[0].stick[0].axis[1].d2 +
    joy[0].button[0].b + joy[0].button[1].b;
  int old_sel;
  int mouse_sel;
  int cur_sel=0;
  int _x, _y;
  int redraw = TRUE;

  scare_mouse();

  fill_menu_info(&m, menu, parent, bar, x, y, minw, minh);

  if (repos)
  {
    m.x = MID(0, m.x, zq_screen_w-m.w-1);
    m.y = MID(0, m.y, zq_screen_h-m.h-1);
  }

  /* save screen under the menu */
  m.saved = create_bitmap_ex(bitmap_color_depth(screen),m.w+1, m.h+1);

  if (m.saved)
    blit(screen, m.saved, m.x, m.y, 0, 0, m.w+1, m.h+1);
  else
    errno = ENOMEM;

  m.sel = mouse_sel = menu_mouse_object(&m);
  if(bar && !mouse_on)
    m.hover = 1;
  if ((m.sel < 0) && (!mouse_on) && (!bar))
    m.sel = 0;

  unscare_mouse();

  do
  {
    rest(1);
    old_sel = m.sel;

    c = menu_mouse_object(&m);
    if ((gui_mouse_b()) || (c != mouse_sel))
      m.sel = mouse_sel = c;

    if (gui_mouse_b())                                      /* if button pressed */
    {
      if ((gui_mouse_x() < m.x) || (gui_mouse_x() > m.x+m.w) ||
          (gui_mouse_y() < m.y) || (gui_mouse_y() > m.y+m.h))
      {
        if (!mouse_on)                                      /* dismiss menu? */
          break;

        if (mouse_in_parent_menu(m.parent))                 /* back to parent? */
          break;
      }

      if ((m.sel >= 0) && (m.menu[m.sel].child))            /* bring up child? */
        ret = m.sel;

	  if(!mouse_on)	cur_sel=m.sel;
      mouse_on = TRUE;
    }
    else                                                    /* button not pressed */
    {
      if (mouse_on)                                         /* selected an item? */
	  {
	    //if(cur_sel == m.sel)
        ret = m.sel;
	  }

      mouse_on = FALSE;

      poll_joystick();
      if (joy[0].stick[0].axis[0].d1 + joy[0].stick[0].axis[0].d2 +
          joy[0].stick[0].axis[1].d1 + joy[0].stick[0].axis[1].d2 +
          joy[0].button[0].b + joy[0].button[1].b == 0)
        joy_on = FALSE;

      c = 0;

      if (keypressed())                                     /* keyboard input */
        c = readkey();
      else if (!joy_on)                                     /* joystick input */
        {
          if (joy[0].stick[0].axis[1].d1)               { c = KEY_UP    << 8; joy_on = TRUE; }
          if (joy[0].stick[0].axis[1].d2)               { c = KEY_DOWN  << 8; joy_on = TRUE; }
          if (joy[0].stick[0].axis[0].d1)               { c = KEY_LEFT  << 8; joy_on = TRUE; }
          if (joy[0].stick[0].axis[0].d2)               { c = KEY_RIGHT << 8; joy_on = TRUE; }
          if (joy[0].button[0].b | joy[0].button[1].b)  { c = KEY_ENTER << 8; joy_on = TRUE; }
        }

        if (c)
      {

        if ((c & 0xff) == 27)
        {
          ret = -1;
          goto getout;
        }

        switch (c >> 8)
        {

          case KEY_LEFT:
          if (m.parent)
          {
            if (m.parent->bar)
            {
              simulate_keypress(KEY_LEFT<<8);
              simulate_keypress(KEY_DOWN<<8);
            }
            ret = -1;
            goto getout;
          }
          /* fall through */

          case KEY_UP:
          if ((((c >> 8) == KEY_LEFT) && (m.bar)) ||
              (((c >> 8) == KEY_UP) && (!m.bar)))
          {
            c = m.sel;
            do
            {
              c--;
              if (c < 0)
                c = m.size - 1;
            } while ((!(m.menu[c].text[0])) && (c != m.sel));
            m.sel = c;
          }
          break;

          case KEY_RIGHT:
          if (((m.sel < 0) || (!m.menu[m.sel].child)) &&
              (m.parent) && (m.parent->bar))
          {
            simulate_keypress(KEY_RIGHT<<8);
            simulate_keypress(KEY_DOWN<<8);
            ret = -1;
            goto getout;
          }
          /* fall through */

          case KEY_DOWN:
          if ((m.sel >= 0) && (m.menu[m.sel].child) &&
              ((((c >> 8) == KEY_RIGHT) && (!m.bar)) ||
               (((c >> 8) == KEY_DOWN) && (m.bar))))
          {
            ret = m.sel;
          }
          else if ((((c >> 8) == KEY_RIGHT) && (m.bar)) ||
                   (((c >> 8) == KEY_DOWN) && (!m.bar)))
            {
              c = m.sel;
              do
              {
                c++;
                if (c >= m.size)
                  c = 0;
              } while ((!(m.menu[c].text[0])) && (c != m.sel));
              m.sel = c;
            }
            break;

          case KEY_SPACE:
          case KEY_ENTER:
          if (m.sel >= 0)
            ret = m.sel;
          break;

          default:
          if ((!m.parent) && ((c & 0xff) == 0))
            c = menu_alt_key(c, m.menu);
          for (c2=0; m.menu[c2].text; c2++)
          {
            if (menu_key_shortcut(c, m.menu[c2].text))
            {
              ret = m.sel = c2;
              break;
            }
          }
          if (m.parent)
          {
            i = m.parent;
            for (c2=0; i->parent; c2++)
              i = i->parent;
            c = menu_alt_key(c, i->menu);
            if (c)
            {
              while (c2-- > 0)
                simulate_keypress(27);
              simulate_keypress(c);
              ret = -1;
              goto getout;
            }
          }
          break;
        }
      }
    }

    if ((redraw) || (m.sel != old_sel))                     /* selection changed? */
    {
      scare_mouse();

      if (redraw)
      {
        draw_menu(&m);
        redraw = FALSE;
      }
      else
      {
        if (old_sel >= 0)
          draw_menu_item(&m, old_sel);

        if (m.sel >= 0)
          draw_menu_item(&m, m.sel);
      }

      unscare_mouse();
    }

    if ((ret >= 0) && (m.menu[ret].flags & D_DISABLED))
      ret = -1;

    if (ret >= 0)                                           /* child menu? */
    {
      if (m.menu[ret].child)
      {
        if (m.bar)
        {
          if(m.hover)
          {
            m.hover = 0;
            scare_mouse();
            draw_menu(&m);
            unscare_mouse();
          }
          get_menu_pos(&m, ret, &_x, &_y, &c);
          _x += 2;
          _y += text_height(font)+7;
        }
        else
        {
          _x = m.x+m.w-3;
          _y = m.y + (text_height(font)+4)*ret + text_height(font)/4;
        }
        c = _jwin_do_menu(m.menu[ret].child, &m, FALSE, _x, _y, TRUE, NULL, 0, 0);
        if (c < 0)
        {
          ret = -1;
          mouse_on = FALSE;
          mouse_sel = menu_mouse_object(&m);
        }
      }
    }

    if ((m.bar) && (!gui_mouse_b()) && (!keypressed()) &&
        ((gui_mouse_x() < m.x) || (gui_mouse_x() > m.x+m.w) ||
         (gui_mouse_y() < m.y) || (gui_mouse_y() > m.y+m.h)))
      break;

      //	#ifdef _ZQUEST_SCALE_
      if (is_zquest())
      {
        if(myvsync)
        {
          if(zqwin_scale > 1)
          {
            stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
          }
          else
          {
            blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
          }
          myvsync=0;
        }
      }
      //	#endif

  } while (ret < 0);

getout:

  if (dret)
    *dret = 0;

  /* callback function? */
  if ((!m.proc) && (ret >= 0))
  {
    active_menu = &m.menu[ret];    m.proc = active_menu->proc;
  }

  if (ret >= 0)
  {
    if (parent)
      parent->proc = m.proc;
    else
    {
      if (m.proc)
      {
        c = m.proc();
        if (dret)
          *dret = c;
        clear_keybuf();
      }
    }
  }

  /* restore screen */
  if (m.saved)
  {
    scare_mouse();
    blit(m.saved, screen, 0, 0, m.x, m.y, m.w+1, m.h+1);
    unscare_mouse();
    destroy_bitmap(m.saved);
  }

  return ret;
}

/* jwin_do_menu:
  *  Displays and animates a popup menu at the specified screen position,
  *  returning the index of the item that was selected, or -1 if it was
  *  dismissed. If the menu crosses the edge of the screen it will be moved.
  */
int jwin_do_menu(MENU *menu, int x, int y)
{
  int ret = _jwin_do_menu(menu, NULL, FALSE, x, y, TRUE, NULL, 0, 0);

  do
  {
  } while (gui_mouse_b());

  return ret;
}

/* jwin_menu_proc:
  *  Dialog procedure for adding drop down menus to a GUI dialog. This
  *  displays the top level menu items as a horizontal bar (eg. across the
  *  top of the screen), and pops up child menus when they are clicked.
  *  When it executes one of the menu callback routines, it passes the
  *  return value back to the dialog manager, so these can return D_O_K,
  *  D_CLOSE, D_REDRAW, etc.
  *
  *  Set the D_USER flag in d->flags to have it draw a frame around a bar menu.
  */
int jwin_menu_proc(int msg, DIALOG *d, int c)
{
  MENU_INFO m;
  int ret = D_O_K;
  int x;

  rest(1);
  switch (msg)
  {

    case MSG_START:
    fill_menu_info(&m, (MENU*)d->dp, NULL, TRUE, d->x-1, d->y-1, d->w+2, d->h+2);
    d->w = m.w-2;
    d->h = m.h-2;
    break;

    case MSG_DRAW:
    fill_menu_info(&m, (MENU*)d->dp, NULL, TRUE, d->x-1, d->y-1, d->w+2, d->h+2);
    draw_menu(&m);
    if(d->flags&D_USER)
      jwin_draw_frame(screen, d->x-3, d->y-3, d->w+6, d->h+6, FR_WIN);
    break;

    case MSG_XCHAR:
    x = menu_alt_key(c, (MENU*)d->dp);
    if (!x)
      break;

    ret |= D_USED_CHAR;
    simulate_keypress(x);
    /* fall through */

    case MSG_GOTMOUSE:
    case MSG_CLICK:
    _jwin_do_menu((MENU*)d->dp, NULL, TRUE, d->x-1, d->y-1, FALSE, &x, d->w+2, d->h+2);
    ret |= x;
    do
    {
    } while (gui_mouse_b());
    break;
  }

  return ret;
}

static DIALOG alert_dialog[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)   (dp2)  (dp3) */
  { jwin_win_proc,     0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
  { d_ctext_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
  { d_ctext_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
  { d_ctext_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL },
  { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
  { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
  { jwin_button_proc,  0,    0,    0,    0,    0,    0,    0,    D_EXIT,  0,    0,    NULL,  NULL,  NULL },
  { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,  NULL,  NULL }
};

#define A_S1  1
#define A_S2  2
#define A_S3  3
#define A_B1  4
#define A_B2  5
#define A_B3  6

/* jwin_alert3:
  *  Displays a simple alert box, containing three lines of text (s1-s3),
  *  and with either one, two, or three buttons. The text for these buttons
  *  is passed in b1, b2, and b3 (NULL for buttons which are not used), and
  *  the keyboard shortcuts in c1 and c2. Returns 1, 2, or 3 depending on
  *  which button was selected.
  */
int jwin_alert3(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, const char *b3, int c1, int c2, int c3, FONT *title_font)
{
  int maxlen = 0;
  int len1, len2, len3;
  int avg_w = text_length(font, " ");
  int avg_h = text_height(font);
  int buttons = 0;
  int yofs = (title ? 22 : 0);
  int b[3];
  int c;

#define SORT_OUT_BUTTON(x) { \
          if (b##x)          \
          {                  \
            alert_dialog[A_B##x].flags &= ~D_HIDDEN; \
            alert_dialog[A_B##x].key = c##x; \
            alert_dialog[A_B##x].dp =  (void *)b##x; \
            len##x = gui_strlen(b##x); \
            b[buttons++] = A_B##x; \
          }                  \
          else               \
          {                  \
            alert_dialog[A_B##x].flags |= D_HIDDEN; \
            len##x = 0;      \
          }                  \
        }

  if (title_font)
  {
    alert_dialog[0].dp2=title_font;
  }
  alert_dialog[A_S1].dp = alert_dialog[A_S2].dp = alert_dialog[A_S3].dp =
    alert_dialog[A_B1].dp = alert_dialog[A_B2].dp = (void*)"";

  if (s1)
  {
    alert_dialog[A_S1].dp =  (void *)s1;
    maxlen = text_length(font, s1);
  }

  if (s2)
  {
    alert_dialog[A_S2].dp =  (void *)s2;
    len1 = text_length(font, s2);
    if (len1 > maxlen)
      maxlen = len1;
  }

  if (s3)
  {
    alert_dialog[A_S3].dp =  (void *)s3;
    len1 = text_length(font, s3);
    if (len1 > maxlen)
      maxlen = len1;
  }

  SORT_OUT_BUTTON(1);
  SORT_OUT_BUTTON(2);
  SORT_OUT_BUTTON(3);

  len1 = MAX(len1, MAX(len2, len3)) + avg_w*3;
  if (len1*buttons > maxlen)
    maxlen = len1*buttons;

  maxlen += avg_w*4;
  maxlen=zc_max(text_length(title_font?title_font:font,title)+29,maxlen);
  alert_dialog[0].w = maxlen;
  alert_dialog[A_S1].x = alert_dialog[A_S2].x = alert_dialog[A_S3].x =
    alert_dialog[0].x + maxlen/2;

  alert_dialog[A_B1].w = alert_dialog[A_B2].w = alert_dialog[A_B3].w = len1;

  alert_dialog[A_B1].x = alert_dialog[A_B2].x = alert_dialog[A_B3].x =
    alert_dialog[0].x + maxlen/2 - len1/2;

  if (buttons == 3)
  {
    alert_dialog[b[0]].x = alert_dialog[0].x + maxlen/2 - len1*3/2 - avg_w;
    alert_dialog[b[2]].x = alert_dialog[0].x + maxlen/2 + len1/2 + avg_w;
  }
  else if (buttons == 2)
    {
      alert_dialog[b[0]].x = alert_dialog[0].x + maxlen/2 - len1 - avg_w;
      alert_dialog[b[1]].x = alert_dialog[0].x + maxlen/2 + avg_w;
    }

    alert_dialog[0].h = avg_h*7 + 13 + yofs;
  alert_dialog[A_S1].y = alert_dialog[0].y + avg_h + yofs;
  alert_dialog[A_S2].y = alert_dialog[0].y + avg_h*2 + yofs;
  alert_dialog[A_S3].y = alert_dialog[0].y + avg_h*3 + yofs;
  alert_dialog[A_S1].h = alert_dialog[A_S2].h = alert_dialog[A_S3].h = avg_h;

  alert_dialog[A_B1].y = alert_dialog[A_B2].y = alert_dialog[A_B3].y =
    alert_dialog[0].y + avg_h*5 + yofs;

  alert_dialog[A_B1].h = alert_dialog[A_B2].h = alert_dialog[A_B3].h = avg_h+13;

  alert_dialog[0].dp = (void *)title;
  alert_dialog[0].flags = (title) ? D_EXIT : 0;

  jwin_center_dialog(alert_dialog);
  set_dialog_color(alert_dialog, scheme[jcTEXTFG], scheme[jcBOX]);

  clear_keybuf();

  do
  {
  } while (gui_mouse_b());

  if (is_large)
  {
    large_dialog(alert_dialog);
	alert_dialog[0].d1 = 0;
  }
  c = popup_zqdialog(alert_dialog, A_B1);

  if (c == A_B1)
    return 1;
  else if (c == A_B2)
      return 2;
    else
      return 3;
}

/* jwin_alert:
  *  Displays a simple alert box, containing three lines of text (s1-s3),
  *  and with either one or two buttons. The text for these buttons is passed
  *  in b1 and b2 (b2 may be null), and the keyboard shortcuts in c1 and c2.
  *  Returns 1 or 2 depending on which button was selected.
  */
int jwin_alert(const char *title, const char *s1, const char *s2, const char *s3, const char *b1, const char *b2, int c1, int c2, FONT *title_font)
{
  int ret;

  ret = jwin_alert3(title, s1, s2, s3, b1, b2, NULL, c1, c2, 0, title_font);

  if (ret > 2)
    ret = 2;

  return ret;
}

/*****************************************/
/***********  drop list proc  ************/
/*****************************************/

static int d_dropcancel_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  d=d;
  c=c;

  if(msg==MSG_CLICK || msg==MSG_DCLICK)
    return D_CLOSE;
  return D_O_K;
}

static DIALOG droplist_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)     (d2)      (dp)              (dp2)   (dp3)*/
  { d_dropcancel_proc, 0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL,             NULL,   NULL },
  { d_list_proc,       0,    0,    0,    0,    0,       0,      0,       D_EXIT,     0,       0,        NULL,             NULL,   NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,       KEY_ESC,  (void*)close_dlg, NULL,   NULL },
  { NULL,              0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL,             NULL,   NULL }
};

static int droplist(DIALOG *d)
{
  ListData *data = (ListData *)d->dp;
  int d1 = d->d1;
  int listsize, x, y, w, h, max_w;

  (*data->listFunc)(-1, &listsize);
  y = d->y + d->h;
  h = zc_min(listsize,8) * text_height(*data->font) + 8;
  if(y+h >= zq_screen_h)
  {
    y = d->y - h;
  }

  x = d->x;
  w = d->w;
  max_w = zc_max(d->x+d->w, zq_screen_w-d->x);
  for(int i=0; i<listsize; ++i)
  {
    w=zc_min(max_w,zc_max(w,text_length(*data->font,(*data->listFunc)(i, NULL))+39));
  }
  if(x+w >= zq_screen_w)
  {
    x = d->x + d->w - w;
  }

  droplist_dlg[1] = *d;
  droplist_dlg[1].proc  = &jwin_abclist_proc;
  droplist_dlg[1].flags = D_EXIT + D_USER;
  droplist_dlg[1].x  = x;
  droplist_dlg[1].y  = y;
  droplist_dlg[1].w  = w;
  droplist_dlg[1].h  = h;
  droplist_dlg[1].d2 = listsize<=8 ? 0 : zc_max(d1-3,0);

  // cancel
  droplist_dlg[0].x = 0;
  droplist_dlg[0].y = 0;
  droplist_dlg[0].w = zq_screen_w;
  droplist_dlg[0].h = zq_screen_h;

  /*if (is_large)
  {
    large_dialog(droplist_dlg);
	alert_dialog[0].d1 = 0;
  }*/
  if(popup_zqdialog(droplist_dlg,1)==1)
  {
    position_mouse_z(0);
    return droplist_dlg[1].d1;
  }

  position_mouse_z(0);
  return d1;
}

/* jwin_droplist_proc:
  *   A drop list...
  */
int jwin_droplist_proc(int msg,DIALOG *d,int c)
{
  int ret;
  int down=0, last_draw=0;
  int d1;

  switch(msg)
  {
    case MSG_CLICK:
      if(mouse_in_rect(d->x+d->w-18,d->y+2,16,d->h))
        goto dropit;
    break;

    case MSG_KEY:
      goto dropit;
    break;
  }

  d1 = d->d1;
  ret = jwin_list_proc(msg,d,c);
  if (d->d1!=d->d2)
  {
    d->d1=d->d2;
    scare_mouse();
    jwin_droplist_proc(MSG_DRAW, d, 0);
    unscare_mouse();
  }
  if((d1 != d->d1) && (d->flags&D_EXIT))
    ret |= D_CLOSE;

  if(msg == MSG_DRAW)
  {
    draw_arrow_button(screen, d->x+d->w-18, d->y+2,16, d->h-4, 0, 0);
  }
  return ret;

dropit:
  last_draw = 0;

  while(gui_mouse_b())
  {
    down = mouse_in_rect(d->x+d->w-18,d->y+2,16,d->h);

    if(down!=last_draw)
    {
      scare_mouse();
      draw_arrow_button(screen, d->x+d->w-18, d->y+2,16, d->h-4, 0, down*3);
      unscare_mouse();
      last_draw = down;
    }

    clear_keybuf();
    //	#ifdef _ZQUEST_SCALE_
    if (is_zquest())
    {
      if(myvsync)
      {
        if(zqwin_scale > 1)
        {
          stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        else
        {
          blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
        myvsync=0;
      }
    }
    //	#endif
  }

  if(!down)
  {
    return D_O_K;
  }

  scare_mouse();
  draw_arrow_button(screen, d->x+d->w-18, d->y+2,16, d->h-4, 0, 0);
  unscare_mouse();

  d1 = d->d1;
  d->d2 = d->d1 = droplist(d);

  scare_mouse();
  object_message(d, MSG_DRAW, 0);
  unscare_mouse();

  while(gui_mouse_b())
    clear_keybuf();

  return ((d1 != d->d1) && (d->flags&D_EXIT)) ? D_CLOSE : D_O_K;
}

/*****************************************/
/************  ABC list proc  ************/
/*****************************************/

int jwin_abclist_proc(int msg,DIALOG *d,int c)
{
  ListData *data = (ListData *)d->dp;
  if(msg==MSG_CHAR && (isalpha(c&0xFF) || isdigit(c&0xFF)))
  {
    int max,dummy,h,i;

    h = (d->h-3) / text_height(*data->font);
    c = toupper(c&0xFF);

    (*data->listFunc)(-1, &max);

    for(i=0; i<max; i++)
    {
      if( toupper( ((*data->listFunc)(i,&dummy))[0] ) == c)
      {
        d->d1 = i;
        d->d2 = zc_max( zc_min( i-(h>>1), max-h), 0);
        goto gotit;
      }
    }

 gotit:
    scare_mouse();
    jwin_list_proc(MSG_DRAW,d,0);
    unscare_mouse();
    return D_USED_CHAR;
  }

  return jwin_list_proc(msg,d,c);
}

int jwin_checkfont_proc(int msg, DIALOG *d, int c)
{

	FONT *oldfont = font;
	if(d->dp2)
	{
		font = (FONT *)d->dp2;
	}
	int rval = jwin_check_proc(msg, d, c);
	font = oldfont;
	return rval;
}

/* jwin_check_proc:
  *  Who needs C++ after all? This is derived from d_button_proc,
  *  but overrides the drawing routine to provide a check box.
  */
int jwin_check_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;
  int x;
  int bx=0, tl=0;
  int tx=d->x;
  ASSERT(d);

  switch (msg)
  {
    case MSG_DRAW:
    x = d->x;
    if (!(d->d1))
    {
      if (d->dp)
      {
        if (d->flags & D_DISABLED)
        {
          gui_textout_ln(screen, (unsigned char *)d->dp, tx+1, d->y+1+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcLIGHT], scheme[jcBOX], 0);
          tl=gui_textout_ln(screen, (unsigned char *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcMEDDARK], -1, 0);
          bx=tl+text_height(font)/2;
        }
        else
        {
          tl=gui_textout_ln(screen, (unsigned char *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcBOXFG], scheme[jcBOX], 0);
          bx=tl+text_height(font)/2;
        }
      }
    }

    jwin_draw_frame(screen, x+bx, d->y, d->h, d->h, FR_DEEP);
    if (!(d->flags & D_DISABLED))
    {
      rectfill(screen, x+bx+2, d->y+2, x+bx+d->h-3, d->y+d->h-3, scheme[jcLIGHT]);
    }

    if (d->d1)
    {
      tx=x+bx+d->h-1+(text_height(font)/2);
      if (d->dp)
      {
        if (d->flags & D_DISABLED)
        {
          gui_textout_ln(screen, (unsigned char *)d->dp, tx+1, d->y+1+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcLIGHT], scheme[jcBOX], 0);
          tl=gui_textout_ln(screen, (unsigned char *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcMEDDARK], -1, 0);
        }
        else
        {
          tl=gui_textout_ln(screen, (unsigned char *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcBOXFG], scheme[jcBOX], 0);
        }
      }
    }

    if (d->flags & D_SELECTED)
    {
      line(screen, x+bx+2, d->y+2, x+bx+d->h-3, d->y+d->h-3, scheme[jcDARK]);
      line(screen, x+bx+2, d->y+d->h-3, x+bx+d->h-3, d->y+2, scheme[jcDARK]);
    }
    d->w=int(text_height(font)*1.5);
    if (d->dp)
    {
//      dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+d->h-1, (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
      dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+(text_height(font)), (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
      d->w+=tl+1;
    }
    return D_O_K;
    break;
  }
  return d_jwinbutton_proc(msg, d, 0);
}

int jwin_radiofont_proc(int msg, DIALOG *d, int c)
{
	FONT *oldfont = font;
	if(d->dp2)
	{
		font = (FONT *)d->dp2;
	}
	int rval = jwin_radio_proc(msg, d, c);
	font = oldfont;
	return rval;
}

/* jwin_radio_proc:
  *  GUI procedure for radio buttons.
  *  Parameters: d1-button group number; d2-button style (0=circle,1=square);
  *  dp-text to appear as label to the right of the button.
  */
int jwin_radio_proc(int msg, DIALOG *d, int c)
{
  int x, center, r, ret, tl=0, tx;
  ASSERT(d);

  switch(msg)
  {
    case MSG_DRAW:
    //      tx=d->x+d->h-1+text_height(font);
    tx=d->x+int(text_height(font)*1.5);

    if (d->dp)
    {
      if (d->flags & D_DISABLED)
      {
        gui_textout_ln(screen, (unsigned char *)d->dp, tx+1, d->y+1+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcLIGHT], scheme[jcBOX], 0);
        tl=gui_textout_ln(screen, (unsigned char *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcMEDDARK], -1, 0);
      }
      else
      {
        tl=gui_textout_ln(screen, (unsigned char *)d->dp, tx, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, scheme[jcBOXFG], scheme[jcBOX], 0);
      }
    }

    x = d->x;
    r = d->h/2;

    center = x+r;
    rectfill(screen, x, d->y, x+d->h-1, d->y+d->h-1, scheme[jcBOX]);

    switch (d->d2)
    {
      case 1:
      jwin_draw_frame(screen, x, d->y, d->h, d->h, FR_DEEP);
      if (!(d->flags & D_DISABLED))
      {
        rectfill(screen, x+2, d->y+2, x+d->h-3, d->y+d->h-3, scheme[jcLIGHT]);
      }
      if (d->flags & D_SELECTED)
      {
        rectfill(screen, x+r/2, d->y+r/2, x+d->h-1-r/2, d->y+d->h-1-r/2, scheme[jcDARK]);
        //line(screen, x+2, d->y+2, x+d->h-3, d->y+d->h-3, scheme[jcDARK]);
        //line(screen, x+2, d->y+d->h-3, x+d->h-3, d->y+2, scheme[jcDARK]);
      }
      break;
      default:
      circlefill(screen, center, d->y+r, r, scheme[jcLIGHT]);
      arc(screen, center, d->y+r, itofix(32), itofix(160), r, scheme[jcMEDDARK]);
      circlefill(screen, center, d->y+r, r-1, scheme[jcMEDLT]);
      arc(screen, center, d->y+r, itofix(32), itofix(160), r-1, scheme[jcDARK]);
      circlefill(screen, center, d->y+r, r-2, (d->flags & D_DISABLED)?scheme[jcBOX]:scheme[jcLIGHT]);
      if (d->flags & D_SELECTED)
      {
        circlefill(screen, center, d->y+r, r-3, scheme[jcDARK]);
      }
      break;
    }

    if (d->dp)
    {
//      dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+d->h-1, (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
      dotted_rect(screen, tx-1, d->y-1, tx+tl, d->y+(text_height(font)), (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
      d->w=tl+int(text_height(font)*1.5)+1;
    }
    return D_O_K;

    case MSG_KEY:
    case MSG_CLICK:
    if (d->flags & D_SELECTED)
    {
      return D_O_K;
    }
    break;

    case MSG_RADIO:
    if ((c == d->d1) && (d->flags & D_SELECTED))
    {
      d->flags &= ~D_SELECTED;
      object_message(d, MSG_DRAW, 0);
    }
    break;
  }

  ret = d_jwinbutton_proc(msg, d, 0);

  if (((msg==MSG_KEY) || (msg==MSG_CLICK)) && (d->flags & D_SELECTED) && (!(d->flags & D_EXIT)))
  {
    d->flags &= ~D_SELECTED;
    broadcast_dialog_message(MSG_RADIO, d->d1);
    d->flags |= D_SELECTED;
  }
  return ret;
}


/* 1.5k lookup table for color matching */
unsigned int col_diff[3*128];

/* bestfit_init:
  *  Color matching is done with weighted squares, which are much faster
  *  if we pregenerate a little lookup table...
  */
void bestfit_init(void)
{
  int i;

  col_diff[0] = col_diff[128] = col_diff[256] = 0;

  for (i=1; i<64; i++) {
    int k = i * i;
    col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
    col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
    col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
  }
}



/* bestfit_color:
  *  Searches a palette for the color closest to the requested R, G, B value.
  */
int bestfit_color_range(AL_CONST PALETTE pal, int r, int g, int b, unsigned char start, unsigned char end)
{
  int i, coldiff, lowest, bestfit;

  if (col_diff[1] == 0)
    bestfit_init();

  bestfit = start;
  lowest = INT_MAX;

  i = start;

  while (i<PAL_SIZE&&i<=end)
  {
    AL_CONST RGB *rgb = &pal[i];
    coldiff = (col_diff + 0) [ (rgb->g - g) & 0x7F ];
    if (coldiff < lowest)
    {
      coldiff += (col_diff + 128) [ (rgb->r - r) & 0x7F ];
      if (coldiff < lowest)
      {
        coldiff += (col_diff + 256) [ (rgb->b - b) & 0x7F ];
        if (coldiff < lowest)
        {
          bestfit = rgb - pal;    /* faster than `bestfit = i;' */
          if (coldiff == 0)
          {
            return bestfit;
          }
          lowest = coldiff;
        }
      }
    }
    i++;
  }
  return bestfit;
}


/* makecol8:
  *  Converts R, G, and B values (ranging 0-255) to an 8 bit paletted color.
  *  If the global rgb_map table is initialised, it uses that, otherwise
  *  it searches through the current palette to find the best match.
  */
int makecol8_map(int r, int g, int b, RGB_MAP *table)
{
  return table->data[r>>3][g>>3][b>>3];
}


/* create_rgb_table:
  *  Fills an RGB_MAP lookup table with conversion data for the specified
  *  palette. This is the faster version by Jan Hubicka.
  *
  *  Uses alg. similar to floodfill - it adds one seed per every color in
  *  palette to its best position. Then areas around seed are filled by
  *  same color because it is best approximation for them, and then areas
  *  about them etc...
  *
  *  It does just about 80000 tests for distances and this is about 100
  *  times better than normal 256*32000 tests so the calculation time
  *  is now less than one second at all computers I tested.
  */
void create_rgb_table_range(RGB_MAP *table, AL_CONST PALETTE pal, unsigned char start, unsigned char end, void (*callback)(int pos))
{
#define UNUSED 65535
#define LAST 65532

  /* macro add adds to single linked list */
#define add(i)    (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                                          (first != LAST ? (next[last] = (i)) : (first = (i))), \
                                          (last = (i))) : 0)

  /* same but w/o checking for first element */
#define add1(i)   (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                                          next[last] = (i), \
                                          (last = (i))) : 0)

  /* calculates distance between two colors */
#define dist(a1, a2, a3, b1, b2, b3) \
          (col_diff[ ((a2) - (b2)) & 0x7F] + \
           (col_diff + 128)[((a1) - (b1)) & 0x7F] + \
           (col_diff + 256)[((a3) - (b3)) & 0x7F])

  /* converts r,g,b to position in array and back */
#define pos(r, g, b) \
          (((r) / 2) * 32 * 32 + ((g) / 2) * 32 + ((b) / 2))

#define depos(pal, r, g, b) \
          ((b) = ((pal) & 31) * 2, \
           (g) = (((pal) >> 5) & 31) * 2, \
           (r) = (((pal) >> 10) & 31) * 2)

  /* is current color better than pal1? */
#define better(r1, g1, b1, pal1) \
          (((int)dist((r1), (g1), (b1), \
                      (pal1).r, (pal1).g, (pal1).b)) > (int)dist2)

  /* checking of position */
#define dopos(rp, gp, bp, ts) \
          if ((rp > -1 || r > 0) && (rp < 1 || r < 61) && \
              (gp > -1 || g > 0) && (gp < 1 || g < 61) && \
              (bp > -1 || b > 0) && (bp < 1 || b < 61)) { \
          i = first + rp * 32 * 32 + gp * 32 + bp; \
          if (!data[i]) {     \
            data[i] = val;    \
            add1(i);          \
          }                   \
          else if ((ts) && (data[i] != val)) { \
              dist2 = (rp ? (col_diff+128)[(r+2*rp-pal[val].r) & 0x7F] : r2) + \
                (gp ? (col_diff    )[(g+2*gp-pal[val].g) & 0x7F] : g2) + \
                (bp ? (col_diff+256)[(b+2*bp-pal[val].b) & 0x7F] : b2); \
              if (better((r+2*rp), (g+2*gp), (b+2*bp), pal[data[i]])) { \
                data[i] = val; \
                add1(i);      \
              }               \
            }                 \
        }

  int i, curr, r, g, b, val, dist2;
  unsigned int r2, g2, b2;
  unsigned short next[32*32*32];
  unsigned char *data;
  int first = LAST;
  int last = LAST;
  int count = 0;
  int cbcount = 0;

#define AVERAGE_COUNT   18000

  if (col_diff[1] == 0)
    bestfit_init();

  memset(next, 255, sizeof(next));
  memset(table->data, 0, sizeof(char)*32*32*32);

  data = (unsigned char *)table->data;

  /* add starting seeds for floodfill */
  for (i=start; i<PAL_SIZE&&i<=end; i++) {
    curr = pos(pal[i].r, pal[i].g, pal[i].b);
    if (next[curr] == UNUSED) {
      data[curr] = i;
      add(curr);
    }
  }

  /* main floodfill: two versions of loop for faster growing in blue axis */
  while (first < LAST) {
    depos(first, r, g, b);

    /* calculate distance of current color */
    val = data[first];
    r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
    g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
    b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

    /* try to grow to all directions */
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
    dopos( 0, 0, 1, 1);
    dopos( 0, 0,-1, 1);
    dopos( 1, 0, 0, 1);
    dopos(-1, 0, 0, 1);
    dopos( 0, 1, 0, 1);
    dopos( 0,-1, 0, 1);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

    /* faster growing of blue direction */
    if ((b > 0) && (data[first-1] == val)) {
      b -= 2;
      first--;
      b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
      dopos(-1, 0, 0, 0);
      dopos( 1, 0, 0, 0);
      dopos( 0,-1, 0, 0);
      dopos( 0, 1, 0, 0);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

      first++;
    }

    /* get next from list */
    i = first;
    first = next[first];
    next[i] = UNUSED;

    /* second version of loop */
    if (first != LAST) {
      depos(first, r, g, b);

      val = data[first];
      r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
      g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
      b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
      dopos( 0, 0, 1, 1);
      dopos( 0, 0,-1, 1);
      dopos( 1, 0, 0, 1);
      dopos(-1, 0, 0, 1);
      dopos( 0, 1, 0, 1);
      dopos( 0,-1, 0, 1);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

      if ((b < 61) && (data[first + 1] == val)) {
        b += 2;
        first++;
        b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7f];

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
        dopos(-1, 0, 0, 0);
        dopos( 1, 0, 0, 0);
        dopos( 0,-1, 0, 0);
        dopos( 0, 1, 0, 0);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

        first--;
      }

      i = first;
      first = next[first];
      next[i] = UNUSED;
    }

    count++;
    if (count == (cbcount+1)*AVERAGE_COUNT/256) {
      if (cbcount < 256) {
        if (callback)
          callback(cbcount);
        cbcount++;
      }
    }
  }

  if (callback)
    while (cbcount < 256)
      callback(cbcount++);
}

int short_bmp_avg(BITMAP *bmp, int i)
{
  int j=((short *)bmp->line[0])[i];
  int r=getr15(j);
  int g=getg15(j);
  int b=getb15(j);
  int k=1;
  if (i>0)
  {
    j=((short *)bmp->line[0])[i-1];
    r+=getr15(j);
    g+=getg15(j);
    b+=getb15(j);
    ++k;
  }
  if (i<(bmp->w-2))
  {
    j=((short *)bmp->line[0])[i+1];
    r+=getr15(j);
    g+=getg15(j);
    b+=getb15(j);
    ++k;
  }
  r/=k;
  g/=k;
  b/=k;
  return makecol15(r, g, b);
}
void dither_rect(BITMAP *bmp, PALETTE *pal, int x1, int y1, int x2, int y2,
                 int src_color1, int src_color2, unsigned char dest_color1,
                 unsigned char dest_color2)
{
  BITMAP *src_bmp=create_bitmap_ex(15, abs(x2-x1)+1, 1);
  BITMAP *dest_bmp=create_bitmap_ex(8, abs(x2-x1)+1, abs(y2-y1)+1);
  int r, g, b, direction=1;
  register int c;
  int r1, r2, g1, g2, b1, b2;
  //  int diff[2][x2-x1+3][3];
  int (*diff[2])[3];
  diff[0] = new int[x2-x1+3][3];
  diff[1] = new int[x2-x1+3][3];
  int cdiff[3];
  RGB_MAP table;
  int temp;
  int red_rand_strength=0, green_rand_strength=0, blue_rand_strength=0;

  clear_bitmap(dest_bmp);
  if (x1>x2)
  {
    temp=x1;
    x1=x2;
    x2=temp;
  }

  if (y1>y2)
  {
    temp=y1;
    y1=y2;
    y2=temp;
  }

  if (src_color1>src_color2)
  {
    temp=src_color1;
    src_color1=src_color2;
    src_color2=temp;
  }

  if (dest_color1>dest_color2)
  {
    temp=dest_color1;
    dest_color1=dest_color2;
    dest_color2=temp;
  }

  create_rgb_table_range(&table, *pal, dest_color1, dest_color2, NULL);
  r1=getr15(src_color1);
  r2=getr15(src_color2);
  g1=getg15(src_color1);
  g2=getg15(src_color2);
  b1=getb15(src_color1);
  b2=getb15(src_color2);
  red_rand_strength=getr8(dest_color1+1)-getr8(dest_color1);
  green_rand_strength=getg8(dest_color1+1)-getg8(dest_color1);
  blue_rand_strength=getb8(dest_color1+1)-getb8(dest_color1);
  memset(cdiff,0,3*sizeof(float));
  memset(diff[0],0,(x2-x1+3)*3*sizeof(int));
  memset(diff[1],0,(x2-x1+3)*3*sizeof(int));
  int mc, mr, mg, mb;
  for(int i=0; i<src_bmp->w; i++)
  {
    r = mix_value(r1, r2, i, src_bmp->w-1);
    g = mix_value(g1, g2, i, src_bmp->w-1);
    b = mix_value(b1, b2, i, src_bmp->w-1);
    c = makecol15(r,g,b);
    ((short *)src_bmp->line[0])[i] = c;
  }
  unsigned char tempcolor, origcolor;
  for(int j=0; j<=y2-y1; ++j)
  {
    if (direction==1)
    {
      for(int i=0; i<=x2-x1; ++i)
      {
        mc=((short *)src_bmp->line[0])[i];
        mr=bound(getr15(mc)+rand()%(red_rand_strength*2+1)-(red_rand_strength*1),0,255);
        mg=bound(getg15(mc)+rand()%(green_rand_strength*2+1)-(green_rand_strength*1),0,255);
        mb=bound(getb15(mc)+rand()%(blue_rand_strength*2+1)-(blue_rand_strength*1),0,255);
        cdiff[0]=bound(mr+
                       diff[0][i][0]+
                       diff[0][i+1][0]+
                       diff[0][i+2][0]+
                       cdiff[0],0,255);
        cdiff[1]=bound(mg+
                       diff[0][i][1]+
                       diff[0][i+1][1]+
                       diff[0][i+2][1]+
                       cdiff[1],0,255);
        cdiff[2]=bound(mb+
                       diff[0][i][2]+
                       diff[0][i+1][2]+
                       diff[0][i+2][2]+
                       cdiff[2],0,255);
        //        bmp->line[j][i+x1]=bound(makecol8_map(int(cdiff[0]),int(cdiff[1]),int(cdiff[2]),&table),dest_color1,dest_color2);
        origcolor=makecol8_map(mr,mg,mb,&table);
        //        tempcolor=bound(makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table),origcolor-1,origcolor+1);
        tempcolor=makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table);
        dest_bmp->line[j][i]=tempcolor;
        r=getr8(tempcolor);
        g=getg8(tempcolor);
        b=getb8(tempcolor);
        diff[1][i][0]=(cdiff[0]-r)*3/16;
        diff[1][i][1]=(cdiff[1]-g)*3/16;
        diff[1][i][2]=(cdiff[2]-b)*3/16;
        diff[1][i+1][0]=(cdiff[0]-r)*5/16;
        diff[1][i+1][1]=(cdiff[1]-g)*5/16;
        diff[1][i+1][2]=(cdiff[2]-b)*5/16;
        diff[1][i+2][0]=(cdiff[0]-r)*1/16;
        diff[1][i+2][1]=(cdiff[1]-g)*1/16;
        diff[1][i+2][2]=(cdiff[2]-b)*1/16;
        cdiff[0]=(cdiff[0]-r)*7/16;
        cdiff[1]=(cdiff[1]-g)*7/16;
        cdiff[2]=(cdiff[2]-b)*7/16;
      }
      memcpy(diff[0],diff[1],(x2-x1+3)*3*sizeof(int));
      memset(diff[1],0,(x2-x1+3)*3*sizeof(int));
      direction=-1;
    }
    else
    {
      for(int i=x2-x1; i>=0; --i)
      {
        mc=((short *)src_bmp->line[0])[i];
        mr=getr15(mc);
        mg=getg15(mc);
        mb=getb15(mc);
        cdiff[0]=bound(mr+
                       diff[0][i][0]+
                       diff[0][i+1][0]+
                       diff[0][i+2][0]+
                       cdiff[0],0,255);
        cdiff[1]=bound(mg+
                       diff[0][i][1]+
                       diff[0][i+1][1]+
                       diff[0][i+2][1]+
                       cdiff[1],0,255);
        cdiff[2]=bound(mb+
                       diff[0][i][2]+
                       diff[0][i+1][2]+
                       diff[0][i+2][2]+
                       cdiff[2],0,255);
        //        bmp->line[j][i+x1]=bound(makecol8_map(int(cdiff[0]),int(cdiff[1]),int(cdiff[2]),&table),dest_color1,dest_color2);
        origcolor=makecol8_map(mr,mg,mb,&table);
        //        tempcolor=bound(makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table),origcolor-1,origcolor+1);
        tempcolor=makecol8_map(cdiff[0],cdiff[1],cdiff[2],&table);
        dest_bmp->line[j][i]=tempcolor;
        r=getr8(tempcolor);
        g=getg8(tempcolor);
        b=getb8(tempcolor);
        diff[1][i][0]=(cdiff[0]-r)*3/16;
        diff[1][i][1]=(cdiff[1]-g)*3/16;
        diff[1][i][2]=(cdiff[2]-b)*3/16;
        diff[1][i+1][0]=(cdiff[0]-r)*5/16;
        diff[1][i+1][1]=(cdiff[1]-g)*5/16;
        diff[1][i+1][2]=(cdiff[2]-b)*5/16;
        diff[1][i+2][0]=(cdiff[0]-r)*1/16;
        diff[1][i+2][1]=(cdiff[1]-g)*1/16;
        diff[1][i+2][2]=(cdiff[2]-b)*1/16;
        cdiff[0]=(cdiff[0]-r)*7/16;
        cdiff[1]=(cdiff[1]-g)*7/16;
        cdiff[2]=(cdiff[2]-b)*7/16;
      }
      memcpy(diff[0],diff[1],(x2-x1+3)*3*sizeof(float));
      memset(diff[1],0,(x2-x1+3)*3*sizeof(float));
      direction=1;
    }
  }
  blit(dest_bmp, bmp, 0, 0, x1, y1, x2-x1+1, y2-y1+1);
  delete[] diff[1];
  delete[] diff[0];
  destroy_bitmap(src_bmp);
  destroy_bitmap(dest_bmp);
  return;
}

bool do_text_button(int x,int y,int w,int h,const char *text)
{
  bool over=false;
  while(gui_mouse_b())
  {
    //vsync();
    if(mouse_in_rect(x,y,w,h))
    {
      if(!over)
      {
        vsync();
        scare_mouse();
        jwin_draw_text_button(screen, x, y, w, h, text, D_SELECTED, true);
        unscare_mouse();
        over=true;
        //	#ifdef _ZQUEST_SCALE_
        if (is_zquest())
        {
          if(myvsync)
          {
            if(zqwin_scale > 1)
            {
              stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            else
            {
              blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
            myvsync=0;
          }
        }
        //	#endif
      }
    }
    else
    {
      if(over)
      {
        vsync();
        scare_mouse();
        jwin_draw_text_button(screen, x, y, w, h, text, 0, true);
        unscare_mouse();
        over=false;
        //	#ifdef _ZQUEST_SCALE_
        if (is_zquest())
        {
          if(myvsync)
          {
            if(zqwin_scale > 1)
            {
              stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            else
            {
              blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
            myvsync=0;
          }
        }
        //	#endif
      }
    }
  }
  return over;
}

bool do_text_button_reset(int x,int y,int w,int h,const char *text)
{
  bool over=false;
  while(gui_mouse_b())
  {
    //vsync();
    if(mouse_in_rect(x,y,w,h))
    {
      if(!over)
      {
        vsync();
        scare_mouse();
        jwin_draw_text_button(screen, x, y, w, h, text, D_SELECTED, true);
        unscare_mouse();
        over=true;
        //	#ifdef _ZQUEST_SCALE_
        if (is_zquest())
        {
          if(myvsync)
          {
            if(zqwin_scale > 1)
            {
              stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            else
            {
              blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
            myvsync=0;
          }
        }
        //	#endif
      }
    }
    else
    {
      if(over)
      {
        vsync();
        scare_mouse();
        jwin_draw_text_button(screen, x, y, w, h, text, 0, true);
        unscare_mouse();
        over=false;
        //	#ifdef _ZQUEST_SCALE_
        if (is_zquest())
        {
          if(myvsync)
          {
            if(zqwin_scale > 1)
            {
              stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            else
            {
              blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
            myvsync=0;
          }
        }
        //	#endif
      }
    }

  }

  if(over)
  {
    vsync();
    scare_mouse();
    jwin_draw_text_button(screen, x, y, w, h, text, 0, true);
    unscare_mouse();
    //	#ifdef _ZQUEST_SCALE_
    if (is_zquest())
    {
      if(myvsync)
      {
        if(zqwin_scale > 1)
        {
          stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
        }
        else
        {
          blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
        }
        myvsync=0;
      }
    }
    //	#endif
  }

  return over;
}

int jwin_tab_proc(int msg, DIALOG *d, int c)
{
  int i;
  int tx;
  int sd=2; //selected delta
  TABPANEL *panel=(TABPANEL *)d->dp;
  DIALOG   *panel_dialog=NULL, *current_object=NULL;
  int selected=0;
  int counter=0;
  ASSERT(d);
  int temp_d, temp_d2;
  if(d->dp==NULL) return D_O_K;
  panel_dialog=(DIALOG *)d->dp3;
  for (i=0; panel[i].text; ++i)
  {
    if ((panel[i].flags&D_SELECTED) && !(d->flags & D_HIDDEN))
    {
        for(counter=0; counter<panel[i].objects; counter++)
        {
          current_object=panel_dialog+(panel[i].dialog[counter]);
          current_object->flags&=~D_HIDDEN;
        }
    }
	else
    {
        for(counter=0; counter<panel[i].objects; counter++)
        {
          current_object=panel_dialog+(panel[i].dialog[counter]);
          current_object->flags|=D_HIDDEN;
		  if(current_object->proc == &jwin_tab_proc)
			object_message(current_object,0,c);
        }
    }
	/*if (d->flags & D_HIDDEN)
	{
		for(counter=0; counter<panel[i].objects; counter++)
        {
          current_object=panel_dialog+(panel[i].dialog[counter]);
          current_object->x=zq_screen_w*3;
          current_object->y=zq_screen_h*3;
        }
	}*/
  }
  switch (msg)
  {
    case MSG_DRAW:
    {
      FONT *oldfont = font;
      if (d->x<zq_screen_w&&d->y<zq_screen_h)
      {
        if (d->dp2)
        {
          font = (FONT *)d->dp2;
        }
        panel_dialog=(DIALOG *)d->dp3;
        rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+8+text_height(font), scheme[jcBOX]); //tab area
        rectfill(screen, d->x+1, d->y+sd+text_height(font)+7, d->x+d->w-2, d->y+sd+d->h-2, scheme[jcBOX]); //panel
        _allegro_vline(screen, d->x, d->y+sd+7+text_height(font), d->y+sd+d->h-2, scheme[jcLIGHT]);
        _allegro_vline(screen, d->x+1, d->y+sd+7+text_height(font), d->y+sd+d->h-3, scheme[jcMEDLT]);
        _allegro_vline(screen, d->x+d->w-2, d->y+sd+7+text_height(font), d->y+sd+d->h-2, scheme[jcMEDDARK]);
        _allegro_vline(screen, d->x+d->w-1, d->y+sd+7+text_height(font)-1, d->y+sd+d->h-1, scheme[jcDARK]);
        _allegro_hline(screen, d->x+1, d->y+sd+d->h-2, d->x+d->w-3, scheme[jcMEDDARK]);
        _allegro_hline(screen, d->x, d->y+sd+d->h-1, d->x+d->w-2, scheme[jcDARK]);
        tx=d->x;
        if (d->dp)
        {
          if (!(panel[((d->d1&0xFF00)>>8)].flags&D_SELECTED))
          {
            _allegro_hline(screen, tx+1, d->y+sd+6+text_height(font)+1, tx+2, scheme[jcMEDLT]); //initial bottom
            _allegro_hline(screen, tx, d->y+sd+6+text_height(font), tx+1, scheme[jcLIGHT]);     //initial bottom
          }
          tx+=2;
          for (i=0; panel[i].text; ++i)
          {
            if (panel[i].flags&D_SELECTED)
            {
              selected=i;
            }
          }
          for (i=((d->d1&0xFF00)>>8); panel[i].text&&i<=last_visible_tab(panel,((d->d1&0xFF00)>>8),d->w); ++i)
          {
            sd=(panel[i].flags&D_SELECTED)?0:2;
            if ((i==((d->d1&0xFF00)>>8)) || (!(panel[i-1].flags&D_SELECTED)))
            {
              _allegro_vline(screen, tx-(2-sd), d->y+sd+2, d->y+8+text_height(font), scheme[jcLIGHT]); //left side
              _allegro_vline(screen, tx-(2-sd)+1, d->y+sd+2, d->y+8+text_height(font), scheme[jcMEDLT]); //left side
              putpixel(screen, tx+1-(2-sd), d->y+sd+1, scheme[jcLIGHT]);                               //left angle
            }
            _allegro_hline(screen, tx+2-(2-sd), d->y+sd, tx+12+(2-sd)+text_length(font, (char *)panel[i].text), scheme[jcLIGHT]); //top
            _allegro_hline(screen, tx+2-(2-sd), d->y+sd+1, tx+12+(2-sd)+text_length(font, (char *)panel[i].text), scheme[jcMEDLT]); //top
            if (!(panel[i].flags&D_SELECTED))
            {
              _allegro_hline(screen, tx+1, d->y+sd+6+text_height(font), tx+13+text_length(font, (char *)panel[i].text)+1, scheme[jcLIGHT]); //bottom
              _allegro_hline(screen, tx, d->y+sd+6+text_height(font)+1, tx+13+text_length(font, (char *)panel[i].text)+1, scheme[jcMEDLT]); //bottom
            }
            tx+=4;
            gui_textout_ln(screen, (unsigned char*)panel[i].text, tx+4, d->y+sd+4, scheme[jcBOXFG], scheme[jcBOX], 0);
            tx+=text_length(font, (char *)panel[i].text)+10;
            if (!(panel[i+1].text) || (!(panel[i+1].flags&D_SELECTED)))
            {
              putpixel(screen, tx-1+(2-sd), d->y+sd+1, scheme[jcDARK]); //right angle
              _allegro_vline(screen, tx+(2-sd), d->y+sd+2, d->y+8+text_height(font)-1, scheme[jcDARK]); //right side
              _allegro_vline(screen, tx+(2-sd)-1, d->y+sd+2, d->y+8+text_height(font)-(sd?1:0), scheme[jcMEDDARK]); //right side
            }
            tx++;
          }
          if(((d->d1&0xFF00)>>8)!=0||last_visible_tab(panel,((d->d1&0xFF00)>>8),d->w)+1<tab_count(panel))
          {
            jwin_draw_text_button(screen,d->x+d->w-14,d->y+2, 14, 14, "\x8B", 0, true);
            jwin_draw_text_button(screen,d->x+d->w-28,d->y+2, 14, 14, "\x8A", 0, true);
          }
        }
        if ((tx+(2-sd))<(d->x+d->w))
        {
          _allegro_hline(screen, tx+(2-sd)-1, d->y+8+text_height(font), d->x+d->w-1, scheme[jcLIGHT]); //ending bottom
          _allegro_hline(screen, tx+(2-sd)-2, d->y+8+text_height(font)+1, d->x+d->w-2, scheme[jcMEDLT]); //ending bottom
        }
        font = oldfont;

        //what dialog is this tab control in (programmer must set manually)
        panel_dialog=(DIALOG *)d->dp3;
        //for each object handled by the currently selected tab...
        for(counter=0; counter<panel[selected].objects; counter++)
        {
          //assign current_object to one of the controls handled by the tab
          current_object=panel_dialog+(panel[selected].dialog[counter]);
          //remember the x and y positions of the control
          current_object->x=panel[selected].xy[counter*2];
          current_object->y=panel[selected].xy[counter*2+1];
          object_message(current_object, MSG_DRAW, 0);
        }
        //if there was a previously selected tab...
        if ((d->d1&0x00FF)!=0x00FF)
        {
          //for each object handled by the tab
          for(counter=0; counter<panel[d->d1&0xFF].objects; counter++)
          {
            //assign current_object to one of the controls handled by the tab
            current_object=panel_dialog+(panel[d->d1&0xFF].dialog[counter]);
            //          //remember the x and y positions of the control
            //          panel[d->d1].xy[counter*2]=current_object->x;
            //          panel[d->d1].xy[counter*2+1]=current_object->y;
            current_object->x=zq_screen_w*3;
            current_object->y=zq_screen_h*3;
          }
        }
      }
    }
    break;
    case MSG_CLICK:
    {
      d->d1&=0xFF00;
      d->d1|=0x00FF;
      // is the mouse on one of the tab arrows (if visible) or in the tab area?
      if(uses_tab_arrows(panel, d->w)&&(mouse_in_rect(d->x+d->w-28, d->y+2, 28, 14)))
      {
        if(mouse_in_rect(d->x+d->w-28, d->y+2, 14, 14))
        {
          if(do_text_button_reset(d->x+d->w-28, d->y+2, 14, 14, "\x8A"))
          {
            temp_d=((d->d1&0xFF00)>>8);
            temp_d2=(d->d1&0x00FF);
            if (temp_d>0)
            {
              --temp_d;
            }
            d->d1=(temp_d<<8)|temp_d2;
            d->flags|=D_DIRTY;
          }
        }
        else if(mouse_in_rect(d->x+d->w-14, d->y+2, 14, 14))
        {
          if(do_text_button_reset(d->x+d->w-14, d->y+2, 14, 14, "\x8B"))
          {
            temp_d=((d->d1&0xFF00)>>8);
            temp_d2=(d->d1&0x00FF);
            if (last_visible_tab(panel, temp_d, d->w)<(tab_count(panel)-1))
            {
              ++temp_d;
            }
            d->d1=(temp_d<<8)|temp_d2;
            d->flags|=D_DIRTY;
          }
        }
      }
      else
      {
        d_tab_proc(msg, d, c);
      }
    }
    break;
    default:
    return d_tab_proc(msg, d, c);
    break;
  }
  panel_dialog=(DIALOG *)d->dp3;
  if (d->flags & D_HIDDEN)
  {
    for (i=0; panel[i].text; ++i)
    {
		for(counter=0; counter<panel[i].objects; counter++)
        {
          current_object=panel_dialog+(panel[i].dialog[counter]);
          current_object->x=zq_screen_w*3;
          current_object->y=zq_screen_h*3;
        }
    }
	  //d->x=zq_screen_w*3;
	  //d->y=zq_screen_h*3;
  }
  else
  {
	  for (i=0; panel[i].text; ++i)
      {
		for(counter=0; counter<panel[i].objects; counter++)
        {
          current_object=panel_dialog+(panel[i].dialog[counter]);
          current_object->x=panel[i].xy[counter*2];
          current_object->y=panel[i].xy[counter*2+1];
        }
      }
	 // d->x=zq_screen_w*3;
	  //d->y=zq_screen_h*3;
  }

  return D_O_K;
}




int jwin_hline_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  ASSERT(d);
  if (msg==MSG_DRAW)
  {
    _allegro_hline(screen, d->x, d->y,   d->x+d->w-1, scheme[jcMEDDARK]);
    _allegro_hline(screen, d->x, d->y+1, d->x+d->w-1, scheme[jcLIGHT]);
  }
  return D_O_K;
}

int jwin_vline_proc(int msg, DIALOG *d, int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  ASSERT(d);
  if (msg==MSG_DRAW)
  {
    _allegro_vline(screen, d->x,   d->y, d->y+d->h-1, scheme[jcMEDDARK]);
    _allegro_vline(screen, d->x+1, d->y, d->y+d->h-1, scheme[jcLIGHT]);
  }
  return D_O_K;
}

int jwin_editbox_proc(int msg, DIALOG *d, int c)
{
  return d_editbox_proc(msg, d, c);
}

//centers dialog based on first object, which should be the containing window
void jwin_center_dialog(DIALOG *dialog)
{
  int xc, yc;
  int c;
  ASSERT(dialog);

  /* how much to move by? */
  xc = (zq_screen_w - dialog[0].w) / 2 - dialog[0].x;
  yc = (zq_screen_h - dialog[0].h) / 2 - dialog[0].y;

  /* move it */
  for (c=0; dialog[c].proc; c++) {
    dialog[c].x += xc;
    dialog[c].y += yc;
  }
}

//Custom slider proc
int d_jslider_proc(int msg, DIALOG *d, int c)
{
   BITMAP *gui_bmp = gui_get_screen();
   BITMAP *slhan = NULL;
   int oldpos, newpos;
   int sfg;                /* slider foreground color */
   int vert = TRUE;        /* flag: is slider vertical? */
   int hh = 7;             /* handle height (width for horizontal sliders) */
   int hmar;               /* handle margin */
   int slp;                /* slider position */
   int mp;                 /* mouse position */
   int irange;
   int slx, sly, slh, slw;
   int msx, msy;
   int retval = D_O_K;
   int upkey, downkey;
   int pgupkey, pgdnkey;
   int homekey, endkey;
   int delta;
   fixed slratio, slmax, slpos;
   typedef int (*SLIDER_TYPE)(void*, int);
   SLIDER_TYPE proc = NULL;
   //int (*proc)(void *cbpointer, int d2value);
   int oldval;
   ASSERT(d);

   /* check for slider direction */
   if (d->h < d->w)
      vert = FALSE;

   /* set up the metrics for the control */
   if (d->dp != NULL) {
      slhan = (BITMAP *)d->dp;
      if (vert)
	 hh = slhan->h;
      else
	 hh = slhan->w;
   }

   hmar = hh/2;
   irange = (vert) ? d->h : d->w;
   slmax = itofix(irange-hh);
   slratio = slmax / (d->d1);
   slpos = slratio * d->d2;
   slp = fixtoi(slpos);

   switch (msg) {

      case MSG_DRAW:
	 sfg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;

	 if (vert) {
	    rectfill(gui_bmp, d->x, d->y, d->x+d->w/2-2, d->y+d->h-1, d->bg);
	    rectfill(gui_bmp, d->x+d->w/2-1, d->y, d->x+d->w/2+1, d->y+d->h-1, sfg);
	    rectfill(gui_bmp, d->x+d->w/2+2, d->y, d->x+d->w-1, d->y+d->h-1, d->bg);
	 }
	 else {
	    rectfill(gui_bmp, d->x, d->y, d->x+d->w-1, d->y+d->h/2-2, d->bg);
	    rectfill(gui_bmp, d->x, d->y+d->h/2-1, d->x+d->w-1, d->y+d->h/2+1, sfg);
	    rectfill(gui_bmp, d->x, d->y+d->h/2+2, d->x+d->w-1, d->y+d->h-1, d->bg);
	 }

	 /* okay, background and slot are drawn, now draw the handle */
	 if (slhan) {
	    if (vert) {
	       slx = d->x+(d->w/2)-(slhan->w/2);
	       sly = d->y+(d->h-1)-(hh+slp);
	    }
	    else {
	       slx = d->x+slp;
	       sly = d->y+(d->h/2)-(slhan->h/2);
	    }
	    draw_sprite(gui_bmp, slhan, slx, sly);
	 }
	 else {
	    /* draw default handle */
	    if (vert) {
	       slx = d->x;
	       sly = d->y+(d->h)-(hh+slp);
	       slw = d->w-1;
	       slh = hh-1;
	    }
            else {
	       slx = d->x+slp;
	       sly = d->y;
	       slw = hh-1;
	       slh = d->h-1;
	    }

	    /* draw body */
	    rectfill(gui_bmp, slx+2, sly, slx+(slw-2), sly+slh, sfg);
	    vline(gui_bmp, slx+1, sly+1, sly+slh-1, sfg);
	    vline(gui_bmp, slx+slw-1, sly+1, sly+slh-1, sfg);
	    vline(gui_bmp, slx, sly+2, sly+slh-2, sfg);
	    vline(gui_bmp, slx+slw, sly+2, sly+slh-2, sfg);
	    vline(gui_bmp, slx+1, sly+2, sly+slh-2, d->bg);
	    hline(gui_bmp, slx+2, sly+1, slx+slw-2, d->bg);
	    putpixel(gui_bmp, slx+2, sly+2, d->bg);
	 }

	 if (d->flags & D_GOTFOCUS)
	    dotted_rect(gui_bmp, d->x, d->y, d->x+d->w-1, d->y+d->h-1, sfg, d->bg);
	 break;

      case MSG_WANTFOCUS:
      case MSG_LOSTFOCUS:
	 return D_WANTFOCUS;

      case MSG_KEY:
	 if (!(d->flags & D_GOTFOCUS))
	    return D_WANTFOCUS;
	 else
	    return D_O_K;

      case MSG_CHAR:
	 /* handle movement keys to move slider */
	 c >>= 8;

	 if (vert) {
	    upkey = KEY_UP;
	    downkey = KEY_DOWN;
	    pgupkey = KEY_PGUP;
	    pgdnkey = KEY_PGDN;
	    homekey = KEY_END;
	    endkey = KEY_HOME;
	 }
	 else {
	    upkey = KEY_RIGHT;
	    downkey = KEY_LEFT;
	    pgupkey = KEY_PGDN;
	    pgdnkey = KEY_PGUP;
	    homekey = KEY_HOME;
	    endkey = KEY_END;
	 }

	 if (c == upkey)
	    delta = 1;
	 else if (c == downkey)
	    delta = -1;
	 else if (c == pgdnkey)
	    delta = -d->d1 / 16;
	 else if (c == pgupkey)
	    delta = d->d1 / 16;
	 else if (c == homekey)
	    delta = -d->d2;
	 else if (c == endkey)
	    delta = d->d1 - d->d2;
	 else
	    delta = 0;

	 if (delta) {
	    oldpos = slp;
	    oldval = d->d2;
		//while (true) {
		for( ; ; ) { //thank you, MSVC ~pkmnfrk
	       d->d2 = d->d2+delta;
	       slpos = slratio*d->d2;
	       slp = fixtoi(slpos);
	       if ((slp != oldpos) || (d->d2 <= 0) || (d->d2 >= d->d1))
		  break;
	    }

	    if (d->d2 < 0)
	       d->d2 = 0;
	    if (d->d2 > d->d1)
	       d->d2 = d->d1;

	    retval = D_USED_CHAR;

	    if (d->d2 != oldval) {
	       /* call callback function here */
	       if (d->dp2) {
		  proc = (SLIDER_TYPE)(d->dp2);
		  retval |= (*proc)(d->dp3, d->d2);
	       }

	       object_message(d, MSG_DRAW, 0);
	    }
	 }
	 break;

      case MSG_WHEEL:
	 oldval = d->d2;
	 d->d2 = MID(0, d->d2+c, d->d1);
	 if (d->d2 != oldval) {
	    /* call callback function here */
	    if (d->dp2) {
	       proc = (SLIDER_TYPE)(d->dp2);
	       retval |= (*proc)(d->dp3, d->d2);
	    }

	    object_message(d, MSG_DRAW, 0);
	 }
	 break;

      case MSG_CLICK:
	 /* track the mouse until it is released */
	 mp = slp;

	 while (gui_mouse_b()) {
	    msx = gui_mouse_x();
	    msy = gui_mouse_y();
	    oldval = d->d2;
	    if (vert)
	       mp = (d->y+d->h-hmar)-msy;
	    else
	       mp = msx-(d->x+hmar);
	    if (mp < 0)
	       mp = 0;
	    if (mp > irange-hh)
	       mp = irange-hh;
	    slpos = itofix(mp);
	    slmax = fixdiv(slpos, slratio);
	    newpos = fixtoi(slmax);
	    if (newpos != oldval) {
	       d->d2 = newpos;

	       /* call callback function here */
	       if (d->dp2 != NULL) {
		  proc = (SLIDER_TYPE)(d->dp2);
		  retval |= (*proc)(d->dp3, d->d2);
	       }

	       object_message(d, MSG_DRAW, 0);
        //	#ifdef _ZQUEST_SCALE_
        if (is_zquest())
        {
          if(myvsync)
          {
            if(zqwin_scale > 1)
            {
              stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
            }
            else
            {
              blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
            }
            myvsync=0;
          }
        }
        //	#endif
	    }

	    /* let other objects continue to animate */
	    broadcast_dialog_message(MSG_IDLE, 0);
	 }
	 break;
   }

   return retval;
}

int d_jwinbutton_proc(int msg, DIALOG *d, int)
{
   BITMAP *gui_bmp;
   int state1, state2;
   int black;
   int swap;
   int g;
   ASSERT(d);

   gui_bmp = gui_get_screen();

   switch (msg) {

      case MSG_DRAW:
	 if (d->flags & D_SELECTED) {
	    g = 1;
	    state1 = d->bg;
	    state2 = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
	 }
	 else {
	    g = 0;
	    state1 = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
	    state2 = d->bg;
	 }

	 rectfill(gui_bmp, d->x+1+g, d->y+1+g, d->x+d->w-3+g, d->y+d->h-3+g, state2);
	 rect(gui_bmp, d->x+g, d->y+g, d->x+d->w-2+g, d->y+d->h-2+g, state1);
	 gui_textout_ex(gui_bmp, (char *)d->dp, d->x+d->w/2+g, d->y+d->h/2-text_height(font)/2+g, state1, -1, TRUE);

	 if (d->flags & D_SELECTED) {
	    vline(gui_bmp, d->x, d->y, d->y+d->h-2, d->bg);
	    hline(gui_bmp, d->x, d->y, d->x+d->w-2, d->bg);
	 }
	 else {
	    black = makecol(0,0,0);
	    vline(gui_bmp, d->x+d->w-1, d->y+1, d->y+d->h-2, black);
	    hline(gui_bmp, d->x+1, d->y+d->h-1, d->x+d->w-1, black);
	 }
	 if ((d->flags & D_GOTFOCUS) &&
	     (!(d->flags & D_SELECTED) || !(d->flags & D_EXIT)))
	    dotted_rect(gui_bmp, d->x+1+g, d->y+1+g, d->x+d->w-3+g, d->y+d->h-3+g, state1, state2);
	 break;

      case MSG_WANTFOCUS:
	 return D_WANTFOCUS;

      case MSG_KEY:
	 /* close dialog? */
	 if (d->flags & D_EXIT) {
	    return D_CLOSE;
	 }

	 /* or just toggle */
	 d->flags ^= D_SELECTED;
	 object_message(d, MSG_DRAW, 0);
	 break;

      case MSG_CLICK:
	 /* what state was the button originally in? */
	 state1 = d->flags & D_SELECTED;
	 if (d->flags & D_EXIT)
	    swap = FALSE;
	 else
	    swap = state1;

	 /* track the mouse until it is released */
	 while (gui_mouse_b()) {
	    state2 = ((gui_mouse_x() >= d->x) && (gui_mouse_y() >= d->y) &&
		      (gui_mouse_x() < d->x + d->w) && (gui_mouse_y() < d->y + d->h));
	    if (swap)
	       state2 = !state2;

	    /* redraw? */
	    if (((state1) && (!state2)) || ((state2) && (!state1))) {
	       d->flags ^= D_SELECTED;
	       state1 = d->flags & D_SELECTED;
	       object_message(d, MSG_DRAW, 0);
	    }

	    /* let other objects continue to animate */
	    broadcast_dialog_message(MSG_IDLE, 0);
      //	#ifdef _ZQUEST_SCALE_
      if (is_zquest())
      {
        if(myvsync)
        {
          if(zqwin_scale > 1)
          {
            stretch_blit(screen, hw_screen, 0, 0, screen->w, screen->h, 0, 0, hw_screen->w, hw_screen->h);
          }
          else
          {
            blit(screen, hw_screen, 0, 0, 0, 0, screen->w, screen->h);
          }
          myvsync=0;
        }
      }
      //	#endif
	 }

	 /* should we close the dialog? */
	 if ((d->flags & D_SELECTED) && (d->flags & D_EXIT)) {
	    d->flags ^= D_SELECTED;
	    return D_CLOSE;
	 }
	 break;
   }

   return D_O_K;
}



/***  The End  ***/

