//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_tiles.cc
//
//  Tile editor stuff, etc., for ZQuest.
//
//--------------------------------------------------------

#include "gui.h"
#include "zquestdat.h"
#include "zq_tiles.h"
#include "zquest.h"
#include "tiles.h"
#include "zq_misc.h"
#include "zq_class.h"
#include "zsys.h"
#include "colors.h"
#include "qst.h"
#include "jwin.h"
#include "jwinfsel.h"
#include "zqscale.h"
#include "zc_custom.h"
#include <string.h>

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

int ex=0;
int nextcombo_fake_click=0;


BITMAP *selection_pattern;
byte selection_grid[18][18];
byte selection_anchor=0;

enum {selection_mode_normal, selection_mode_add, selection_mode_subtract, selection_mode_exclude};
BITMAP *selecting_pattern;
int selecting_x1, selecting_x2, selecting_y1, selecting_y2;

BITMAP *intersection_pattern;

byte relational_template[48][4]=
{
  {  0,  0,  0,  0 },
  {  0,  1,  0,  0 },
  {  1,  0,  0,  0 },
  {  1,  1,  0,  0 },
  {  0,  0,  1,  0 },
  {  0,  1,  1,  0 },
  {  1,  0,  1,  0 },
  {  1,  1,  1,  0 },
  {  0,  0,  0,  1 },
  {  0,  1,  0,  1 },
  {  1,  0,  0,  1 },
  {  1,  1,  0,  1 },
  {  0,  0,  1,  1 },
  {  0,  1,  1,  1 },
  {  1,  0,  1,  1 },
  {  1,  1,  1,  1 },
  {  0,  2,  0,  2 },
  {  1,  2,  0,  2 },
  {  0,  2,  1,  2 },
  {  1,  2,  1,  2 },
  {  3,  3,  0,  0 },
  {  3,  3,  1,  0 },
  {  3,  3,  0,  1 },
  {  3,  3,  1,  1 },
  {  3,  4,  0,  2 },
  {  3,  4,  1,  2 },
  {  2,  0,  2,  0 },
  {  2,  1,  2,  0 },
  {  2,  0,  2,  1 },
  {  2,  1,  2,  1 },
  {  2,  2,  2,  2 },
  {  4,  3,  2,  0 },
  {  4,  3,  2,  1 },
  {  4,  4,  2,  2 },
  {  0,  0,  3,  3 },
  {  0,  1,  3,  3 },
  {  1,  0,  3,  3 },
  {  1,  1,  3,  3 },
  {  0,  2,  3,  4 },
  {  1,  2,  3,  4 },
  {  3,  3,  3,  3 },
  {  3,  4,  3,  4 },
  {  2,  0,  4,  3 },
  {  2,  1,  4,  3 },
  {  2,  2,  4,  4 },
  {  4,  3,  4,  3 },
  {  4,  4,  4,  4 },
  {  5,  5,  5,  5 }
};

byte dungeon_carving_template[96][4]=
{
  {  0,  0,  0,  0 },
  {  0,  1,  0,  0 },
  {  1,  0,  0,  0 },
  {  1,  1,  0,  0 },
  {  0,  0,  1,  0 },
  {  0,  1,  1,  0 },
  {  1,  0,  1,  0 },
  {  1,  1,  1,  0 },
  {  0,  0,  0,  1 },
  {  0,  1,  0,  1 },
  {  1,  0,  0,  1 },
  {  1,  1,  0,  1 },
  {  0,  0,  1,  1 },
  {  0,  1,  1,  1 },
  {  1,  0,  1,  1 },
  {  1,  1,  1,  1 },
  {  0,  2,  0,  2 },
  {  1,  2,  0,  2 },
  {  0,  2,  1,  2 },
  {  1,  2,  1,  2 },
  {  3,  3,  0,  0 },
  {  3,  3,  1,  0 },
  {  3,  3,  0,  1 },
  {  3,  3,  1,  1 },
  {  3,  4,  0,  2 },
  {  3,  4,  1,  2 },
  {  2,  0,  2,  0 },
  {  2,  1,  2,  0 },
  {  2,  0,  2,  1 },
  {  2,  1,  2,  1 },
  {  2,  2,  2,  2 },
  {  4,  3,  2,  0 },
  {  4,  3,  2,  1 },
  {  4,  4,  2,  2 },
  {  0,  0,  3,  3 },
  {  0,  1,  3,  3 },
  {  1,  0,  3,  3 },
  {  1,  1,  3,  3 },
  {  0,  2,  3,  4 },
  {  1,  2,  3,  4 },
  {  3,  3,  3,  3 },
  {  3,  4,  3,  4 },
  {  2,  0,  4,  3 },
  {  2,  1,  4,  3 },
  {  2,  2,  4,  4 },
  {  4,  3,  4,  3 },
  {  4,  4,  4,  4 },
  {  5,  5,  5,  5 },

  {  5,  5,  5,  5 },
  {  6,  6,  6,  6 },
  {  7,  7,  7,  7 },
  {  7,  6,  7,  6 },
  {  8,  8,  8,  8 },
  { 16,  6,  8, 15 },
  {  7,  7,  8,  8 },
  {  7,  6,  8, 15 },
  {  9,  9,  9,  9 },
  {  6,  6,  9,  9 },
  {  7, 17, 14,  9 },
  {  7,  6, 14,  9 },
  {  8,  9,  8,  9 },
  { 16,  6,  8,  9 },
  {  7, 17,  8,  9 },
  {  7,  6,  8,  9 },
  { 10, 10, 10, 10 },
  {  7, 10, 14, 10 },
  { 16, 10,  8, 10 },
  {  7, 10,  8, 10 },
  { 11, 11, 11, 11 },
  { 11, 11,  8, 15 },
  { 11, 11, 14,  9 },
  { 11, 11,  8,  9 },
  { 14, 14, 14, 14 },
  { 14, 14,  8, 14 },
  { 12, 12, 12, 12 },
  { 12,  6, 12, 15 },
  { 12, 17, 12,  9 },
  { 12,  6, 12,  9 },
  { 12, 10, 12, 10 },
  { 15, 15, 15, 15 },
  { 15, 15, 15,  9 },
  { 15, 14, 12, 10 },
  { 13, 13, 13, 13 },
  { 16,  6, 13, 13 },
  {  7, 17, 13, 13 },
  {  7,  6, 13, 13 },
  { 16, 16, 16, 16 },
  {  7, 16, 16, 16 },
  { 11, 11, 13, 13 },
  { 11, 14, 13, 16 },
  { 17, 17, 17, 17 },
  { 17,  6, 17, 17 },
  { 12, 10, 17, 16 },
  { 15, 11, 17, 13 },
  { 15, 14, 17, 16 },
  { 18, 18, 18, 18 }
};

/*********************************/
/*****    Tiles & Combos    ******/
/*********************************/

void merge_tiles(int dest_tile, int src_quarter1, int src_quarter2, int src_quarter3, int src_quarter4)
{
  int size=tilesize(newtilebuf[dest_tile].format)>>4;
  int size2=size>>1;
  if (newtilebuf[dest_tile].data==NULL)
  {
    reset_tile(newtilebuf, dest_tile, newtilebuf[src_quarter1>>2].format);
  }
  int i=0;
  if ((dest_tile<<2)+i!=src_quarter1)
  {
    for (int j=0; j<8; ++j)
    {
      memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter1>>2].data[((j+((src_quarter1&2)<<2))*size)+((src_quarter1&1)*size2)]), size2);
    }
  }
  i=1;
  if ((dest_tile<<2)+i!=src_quarter2)
  {
    for (int j=0; j<8; ++j)
    {
      memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter2>>2].data[((j+((src_quarter2&2)<<2))*size)+((src_quarter2&1)*size2)]), size2);
    }
  }
  i=2;
  if ((dest_tile<<2)+i!=src_quarter3)
  {
    for (int j=0; j<8; ++j)
    {
      memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter3>>2].data[((j+((src_quarter3&2)<<2))*size)+((src_quarter3&1)*size2)]), size2);
    }
  }
  i=3;
  if ((dest_tile<<2)+i!=src_quarter4)
  {
    for (int j=0; j<8; ++j)
    {
      memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter4>>2].data[((j+((src_quarter4&2)<<2))*size)+((src_quarter4&1)*size2)]), size2);
    }
  }
}

int d_combo_proc(int msg,DIALOG *d,int c);

void go_tiles()
{
  for (dword i=0; i<NEWMAXTILES; ++i)
  {
    newundotilebuf[i].format=newtilebuf[i].format;
    if (newundotilebuf[i].data!=NULL)
    {
      free(newundotilebuf[i].data);
    }
    newundotilebuf[i].data=(byte *)malloc(tilesize(newundotilebuf[i].format));
    if (newundotilebuf[i].data==NULL)
    {
      Z_error("Unable to initialize undo tile #%ld.\n", i);
      exit(1);
    }
    //memcpy(newundotilebuf[i].data,newtilebuf[i].data,tilesize(newundotilebuf[i].format));
    int count = tilesize(newundotilebuf[i].format);
    byte *to = newundotilebuf[i].data;
    byte *from = newtilebuf[i].data;
    register int n = (count + 7) / 8;      /* count > 0 assumed */

    switch (count % 8)
    {
        case 0:        do {  *to++ = *from++;
        case 7:              *to++ = *from++;
        case 6:              *to++ = *from++;
        case 5:              *to++ = *from++;
        case 4:              *to++ = *from++;
        case 3:              *to++ = *from++;
        case 2:              *to++ = *from++;
        case 1:              *to++ = *from++;
      } while (--n > 0);
    }
  }
  /*
    int *si = (int*)tilebuf;
    int *di = (int*)undotilebuf;
    for(int i=0; i<NEWTILE_SIZE2/4; i++)
    *(di++) = *(si++);
    */
}

void go_slide_tiles(int columns, int rows, int top, int left)
{
  for (int c=0; c<columns;c++)
  {
    for (int r=0; r<rows; r++)
    {
      int t=((top+r)*TILES_PER_ROW)+left+c;
      newundotilebuf[t].format=newtilebuf[t].format;
      if (newundotilebuf[t].data!=NULL)
      {
        free(newundotilebuf[t].data);
      }
      newundotilebuf[t].data=(byte *)malloc(tilesize(newundotilebuf[t].format));
      if (newundotilebuf[t].data==NULL)
      {
        Z_error("Unable to initialize undo tile #%ld.\n", t);
        exit(1);
      }
      memcpy(newundotilebuf[t].data,newtilebuf[t].data,tilesize(newundotilebuf[t].format));
    }
  }
}

void comeback_tiles()
{
  for (dword i=0; i<NEWMAXTILES; ++i)
  {
    newtilebuf[i].format=newundotilebuf[i].format;
    if (newtilebuf[i].data!=NULL)
    {
      free(newtilebuf[i].data);
    }
    newtilebuf[i].data=(byte *)malloc(tilesize(newtilebuf[i].format));
    if (newtilebuf[i].data==NULL)
    {
      Z_error("Unable to initialize tile #%ld.\n", i);
      exit(1);
    }
    memcpy(newtilebuf[i].data,newundotilebuf[i].data,tilesize(newtilebuf[i].format));
  }


  /*
    int *si = (int*)undotilebuf;
    int *di = (int*)tilebuf;
    for(int i=0; i<NEWTILE_SIZE2/4; i++)
    *(di++) = *(si++);
    */
}

void go_combos()
{
  newcombo *si = combobuf;
  newcombo *di = undocombobuf;
  for(int i=0; i<MAXCOMBOS; i++)
    *(di++) = *(si++);
}

void comeback_combos()
{
  newcombo *si = undocombobuf;
  newcombo *di = combobuf;
  for(int i=0; i<MAXCOMBOS; i++)
    *(di++) = *(si++);
}

void little_x(BITMAP *dest, int x, int y, int c, int s)
{
  line(dest,x,y,x+s,y+s,c);
  line(dest,x+s,y,x,y+s,c);
}

enum {gm_light, gm_dark, gm_max};
int gridmode=gm_light;

bool has_selection()
{
  for(int i=1; i<17; ++i)
  {
    for(int j=1; j<17; ++j)
    {
      if (selection_grid[i][j])
      {
        return true;
      }
    }
  }
  return false;
}

void draw_selection_outline(BITMAP *dest, int x, int y, int scale2)
{
  drawing_mode(DRAW_MODE_COPY_PATTERN, selection_pattern, selection_anchor>>3, 0);
  for(int i=1; i<18; ++i)
  {
    for(int j=1; j<18; ++j)
    {
      //  zoomtile16(screen2,tile,79,31,cs,flip,8);
      if(selection_grid[i-1][j]!=selection_grid[i][j])
      {
        _allegro_vline(dest, x+((i-1)*scale2), y+((j-1)*scale2), y+(j*scale2), 255);
      }
      if(selection_grid[i][j-1]!=selection_grid[i][j])
      {
        _allegro_hline(dest, x+((i-1)*scale2), y+((j-1)*scale2), x+(i*scale2), 255);
      }
    }
  }
  drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
  //  selection_anchor=(selection_anchor+1)%64;
}

bool is_selecting()
{
  return (selecting_x1>-1&&selecting_x2>-1&&selecting_y1>-1&&selecting_y2>-1);
}

void draw_selecting_outline(BITMAP *dest, int x, int y, int scale2)
{
  int x1=min(selecting_x1,selecting_x2);
  int x2=max(selecting_x1,selecting_x2);
  int y1=min(selecting_y1,selecting_y2);
  int y2=max(selecting_y1,selecting_y2);

//  rect(dest, x+(x1*scale2), y+(y1*scale2), x+((x2+1)*scale2), y+((y2+1)*scale2), 255);
  for(int i=1; i<18; ++i)
  {
    for(int j=1; j<18; ++j)
    {
      drawing_mode(DRAW_MODE_COPY_PATTERN, selecting_pattern, selection_anchor>>3, 0);
      if (((j>=y1+1)&&(j<=y2+1))&&((i==x1+1)||(i==x2+2)))
      {
        if(selection_grid[i-1][j]!=selection_grid[i][j])
        {
          drawing_mode(DRAW_MODE_COPY_PATTERN, intersection_pattern, selection_anchor>>3, 0);
        }
        _allegro_vline(dest, x+((i-1)*scale2), y+((j-1)*scale2), y+(j*scale2), 255);
      }
      if (((i>=x1+1)&&(i<=x2+1))&&((j==y1+1)||(j==y2+2)))
      {
        if(selection_grid[i][j-1]!=selection_grid[i][j])
        {
          drawing_mode(DRAW_MODE_COPY_PATTERN, intersection_pattern, selection_anchor>>3, 0);
        }
        _allegro_hline(dest, x+((i-1)*scale2), y+((j-1)*scale2), x+(i*scale2), 255);
      }
    }
  }
  drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
  //  selection_anchor=(selection_anchor+1)%64;
}

void add_color_to_selection(int color)
{
  for(int i=1; i<17; ++i)
  {
    for(int j=1; j<17; ++j)
    {
      if(unpackbuf[((j-1)<<4)+(i-1)]==color)
      {
        selection_grid[i][j]=1;
      }
    }
  }
}

void remove_color_from_selection(int color)
{
  for(int i=1; i<17; ++i)
  {
    for(int j=1; j<17; ++j)
    {
      if(unpackbuf[((j-1)<<4)+(i-1)]==color)
      {
        selection_grid[i][j]=0;
      }
    }
  }
}

void intersect_color_with_selection(int color)
{
  for(int i=1; i<17; ++i)
  {
    for(int j=1; j<17; ++j)
    {
      if((unpackbuf[((j-1)<<4)+(i-1)]==color)&&(selection_grid[i][j]==1))
      {
        selection_grid[i][j]=1;
      }
      else
      {
        selection_grid[i][j]=0;
      }
    }
  }
}

bool is_in_selection(int x, int y)
{
  return (!has_selection()||(selection_grid[x+1][y+1]!=0));
}

void zoomtile16(BITMAP *dest,int tile,int x,int y,int cset,int flip,int m)
{
  //  rectfill(dest,x,y,x+(16*m),y+(16*m),gridmode==gm_light?jwin_pal[jcMEDLT]:jwin_pal[jcDARK]);
  rectfill(dest,x,y,x+(16*m),y+(16*m),gridmode==gm_light?vc(7):vc(8));
  cset <<= 4;

  if(newtilebuf[tile].format>tf4Bit)
  {
    cset=0;
  }

  unpack_tile(newtilebuf, tile, 0, false);
  byte *si = unpackbuf;

  for(int cy=0; cy<16; cy++)
  {
    for(int cx=0; cx<16; cx++)
    {
      int dx = ((flip&1)?15-cx:cx)*m;
      int dy = ((flip&2)?15-cy:cy)*m;
      rectfill(dest,x+dx+1,y+dy+1,x+dx+m-1,y+dy+m-1,*si+cset);
      if(*si==0)
      {
        little_x(dest,x+dx+3,y+dy+3,210,2);
      }
      ++si;
    }
  }
  if (has_selection()||is_selecting())
  {
    selection_anchor=(selection_anchor+1)%64;
    if (has_selection()||is_selecting())
    {
      draw_selection_outline(dest, x, y, m);
    }
    if (is_selecting())
    {
      draw_selecting_outline(dest, x, y, m);
    }
  }
}

void draw_button(BITMAP *dest,int x,int y,int w,int h,char *text,int bg,int fg,int flags,bool jwin)
{
  if (!jwin)
  {
    if (flags&D_SELECTED)
    {
      swap(fg,bg);
    }
    rect(dest,x+1,y+1,x+w-1,y+h-1,fg);
    rectfill(dest,x+1,y+1,x+w-3,y+h-3,bg);
    rect(dest,x,y,x+w-2,y+h-2,fg);
    textout_centre_ex(dest,font,text,(x+x+w)>>1,((y+y+h)>>1)-4,fg,-1);
  }
  else
  {
    jwin_draw_text_button(dest, x, y, w, h, text, flags, true);
  }
}

bool do_button(int x,int y,int w,int h,char *text,int bg,int fg,bool jwin)
{
  bool over=false;
  while(gui_mouse_b())
  {
    vsync();
    if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
    {
      if(!over)
      {
        scare_mouse();
        draw_button(screen,x,y,w,h,text,fg,bg,D_SELECTED,jwin);
        unscare_mouse();
        over=true;
      }
    }
    else
    {
      if(over)
      {
        scare_mouse();
        draw_button(screen,x,y,w,h,text,fg,bg,0,jwin);
        unscare_mouse();
        over=false;
      }
    }
  }
  return over;
}

bool do_button_reset(int x,int y,int w,int h,char *text,int bg,int fg,bool jwin)
{
  bool over=false;
  while(gui_mouse_b())
  {
    vsync();
    if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
    {
      if(!over)
      {
        scare_mouse();
        draw_button(screen,x,y,w,h,text,fg,bg,D_SELECTED,jwin);
        unscare_mouse();
        over=true;
      }
    }
    else
    {
      if(over)
      {
        scare_mouse();
        draw_button(screen,x,y,w,h,text,fg,bg,0,jwin);
        unscare_mouse();
        over=false;
      }
    }
  }

  if(over)
  {
    vsync();
    scare_mouse();
    draw_button(screen,x,y,w,h,text,fg,bg,0,jwin);
    unscare_mouse();
  }

  return over;
}

//    circle(BITMAP *bmp, int x, int y, int radius, int color);
//    circlefill(BITMAP *bmp, int x, int y, int radius, int color);

void draw_layerradio(BITMAP *dest,int x,int y,int bg,int fg, int value)
{
  //these are here to bypass compiler warnings about unused arguments
  bg=bg;
  fg=fg;

  int r, center;
  for (int k=0; k<7; k++)
  {
    if ((k==0)||(Map.CurrScr()->layermap[k-1]))
    {
      //      circle(dest, x+(k*25)+4, y+4, 4, fg);
      //      circlefill(dest, x+(k*25)+4, y+4, 2, (value==k)?fg:bg);
      //*
      r = 9/2;

      center = x+(k*25)+r;
      rectfill(dest, x+(k*25), y, x+(k*25)+9-1, y+9-1, jwin_pal[jcBOX]);

      circlefill(dest, center, y+r, r, jwin_pal[jcLIGHT]);
      arc(dest, center, y+r, itofix(32), itofix(160), r, jwin_pal[jcMEDDARK]);
      circlefill(dest, center, y+r, r-1, jwin_pal[jcMEDLT]);
      arc(dest, center, y+r, itofix(32), itofix(160), r-1, jwin_pal[jcDARK]);
      circlefill(dest, center, y+r, r-2, jwin_pal[jcLIGHT]);
      if (value==k)
      {
        circlefill(dest, center, y+r, r-3, jwin_pal[jcDARK]);
      }
      //*/
    }
  }
}

void do_layerradio(BITMAP *dest,int x,int y,int bg,int fg,int &value)
{
  while(gui_mouse_b())
  {
    vsync();
    for (int k=0; k<7; k++)
    {
      if ((k==0)||(Map.CurrScr()->layermap[k-1]))
      {
        //if on radio button
        if(isinRect(gui_mouse_x(),gui_mouse_y(),x+(k*25),y,x+(k*25)+8,y+8))
        {
          value=k;
          scare_mouse();
          draw_layerradio(dest,x,y,bg,fg,value);
          refresh(rMENU);
          unscare_mouse();
        }
      }
    }
  }
}

void draw_checkbox(BITMAP *dest,int x,int y,int bg,int fg, bool value)
{
  //these are here to bypass compiler warnings about unused arguments
  bg=bg;
  fg=fg;

  //  rect(dest,x, y, x+8, y+8, fg);
  //  line(dest,x+1,y+1,x+7,y+7,value?fg:bg);
  //  line(dest,x+1,y+7,x+7,y+1,value?fg:bg);

  jwin_draw_frame(dest, x, y, 9, 9, FR_DEEP);
  rectfill(dest, x+2, y+2, x+9-3, y+9-3, jwin_pal[jcLIGHT]);
  if (value)
  {
    line(dest, x+2, y+2, x+9-3, y+9-3, jwin_pal[jcDARK]);
    line(dest, x+2, y+9-3, x+9-3, y+2, jwin_pal[jcDARK]);
  }

}



bool do_checkbox(BITMAP *dest,int x,int y,int bg,int fg,int &value)
{
  bool over=false;
  while(gui_mouse_b())
  {
    vsync();
    if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+8,y+8))               //if on checkbox
    {
      if(!over)                                             //if wasn't here before
      {
        scare_mouse();
        value=!value;
        draw_checkbox(dest,x,y,bg,fg,value!=0);
        refresh(rMENU);
        unscare_mouse();
        over=true;
      }
    }
    else                                                    //if not on checkbox
    {
      if(over)                                              //if was here before
      {
        scare_mouse();
        value=!value;
        draw_checkbox(dest,x,y,bg,fg,value!=0);
        refresh(rMENU);
        unscare_mouse();
        over=false;
      }
    }
  }
  return over;
}

//*************** tile flood fill stuff **************

byte tf_c;
byte tf_u;

void tile_floodfill_rec(int x,int y)
{
  if (is_in_selection(x,y))
  {
    while(x>0 && (unpackbuf[(y<<4)+x-1] == tf_u))
      --x;

    while(x<=15 && (unpackbuf[(y<<4)+x] == tf_u))
    {
      if (is_in_selection(x,y))
      {
        unpackbuf[(y<<4)+x] = tf_c;
      }

      if(y>0 && (unpackbuf[((y-1)<<4)+x] == tf_u))
        tile_floodfill_rec(x,y-1);

      if(y<15 && (unpackbuf[((y+1)<<4)+x] == tf_u))
        tile_floodfill_rec(x,y+1);

      ++x;
    }
  }
}

void tile_floodfill(int tile,int x,int y,byte c)
{
  if (is_in_selection(x,y))
  {
    unpack_tile(newtilebuf, tile, 0, false);
    tf_c = c;
    tf_u = unpackbuf[(y<<4)+x];
    if(tf_u != tf_c)
      tile_floodfill_rec(x,y);
    pack_tile(newtilebuf,unpackbuf,tile);
  }
}

//***************** tile editor  stuff *****************

int c1=1;
int c2=0;
//int bgc=dvc(4+5);
//int bgc=vc(1);
//enum { t_pen, t_brush, t_recolor, t_select };
int tool = t_pen;
int old_tool = -1;
int tool_cur = -1;
int select_mode = 0;
int drawing=0;

int tool_buttons_left=22, tool_buttons_top=29, tool_buttons_columns=2;

void update_tool_cursor()
{
  int type=0;
  if (has_selection())
  {
    switch(tool)
    {
      case t_select:
      case t_wand:
        type+=select_mode;
        break;
    }
  }

  if(isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158)) //inside the zoomed tile window
  {
    if(tool_cur==-1)
    {
      set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][type]);
      switch(tool)
      {
        case t_brush:
          set_mouse_sprite_focus(1, 14);
          break;
        case t_move:
          set_mouse_sprite_focus(8, 8);
          break;
        default:
          set_mouse_sprite_focus(1, 1);
          break;
      }
    }
    tool_cur=tool;
  }
  else if(tool_cur != -1)
  {
    set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
    tool_cur = -1;
  }
}

void draw_edit_scr(int tile,int flip,int cs,byte *oldtile)
{
  PALETTE tpal;
  FONT *oldfont = font;
  static BITMAP *tbar = create_bitmap_ex(8,SCREEN_W-6, 18);
  static bool created_tbar=false;
  jwin_draw_win(screen2, 0, 0, SCREEN_W, SCREEN_H, FR_WIN);
  if (created_tbar)
  {
    blit(tbar, screen2, 0, 0, 3, 3, SCREEN_W-6, 18);
  }
  else
  {
    font = lfont;
    char buf[80];
    sprintf(buf,"Tile Editor (%d)", tile);
    jwin_draw_titlebar(tbar, 0, 0, SCREEN_W-6, 18, buf, true);
    font = oldfont;
    created_tbar=true;
    blit(tbar, screen2, 0, 0, 3, 3, SCREEN_W-6, 18);
  }
  //draw_x_button(screen2, SCREEN_W - 21, 5, 0);

  swap(oldtile,newtilebuf[tile].data);
  jwin_draw_win(screen2, 222, 46, 20, 20, FR_DEEP);
  puttile16(screen2,tile,224,48,cs,flip);
  jwin_draw_win(screen2, 246, 46, 20, 20, FR_DEEP);
  overtile16(screen2,tile,248,48,cs,flip);
  swap(oldtile,newtilebuf[tile].data);

  jwin_draw_win(screen2, 222, 78, 20, 20, FR_DEEP);
  puttile16(screen2,tile,224,80,cs,flip);
  jwin_draw_win(screen2, 246, 78, 20, 20, FR_DEEP);
  overtile16(screen2,tile,248,80,cs,flip);

  jwin_draw_win(screen2, 77, 29, 133, 133, FR_DEEP);
  zoomtile16(screen2,tile,79,31,cs,flip,8);

  textprintf_ex(screen2,font,224,112,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"tile: %d",tile);
  textprintf_ex(screen2,font,224,120,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"cset: %d",cs);

  //palette and mouse
  switch(newtilebuf[tile].format)
  {
    case tf4Bit:
    jwin_draw_win(screen2, 102, 174, 36, 36, FR_DEEP);
    for(int i=0; i<16; i++)
    {
      int x=((i&3)<<3)+104;
      int y=((i>>2)<<3)+176;
      rectfill(screen2,x,y,x+7,y+7,CSET(cs)+i);
    }
    little_x(screen2,106,178,210,2);

    masked_blit((BITMAP *)zcdata[BMP_MOUSESMALL].dat, screen2, 0, 0, 144, 172, 42, 39);
    little_x(screen2,149,179,c1==0?210:208,2);
    little_x(screen2,159,176,c2==0?210:209,2);
    break;
    case tf8Bit:
    jwin_draw_win(screen2, 92, 170, 68, 64, FR_DEEP);
    for(int i=0; i<240; i++)
    {
      int x=((i&15)<<2)+94;
      int y=((i>>4)<<2)+172;
      rectfill(screen2,x,y,x+3,y+3,i);
    }
    little_x(screen2,94,172,210,2);

    masked_blit((BITMAP *)zcdata[BMP_MOUSESMALL].dat, screen2, 0, 0, 168, 172, 42, 39);
    little_x(screen2,173,179,c1==0?210:208,2);
    little_x(screen2,183,176,c2==0?210:209,2);
    break;
  }

  //masked_blit((BITMAP *)zcdata[BMP_MOUSESMALL].dat, screen2, 0, 0, 144, 172, 42, 39);
  //little_x(screen2,149,179,c1==0?210:208,2);
  //little_x(screen2,159,176,c2==0?210:209,2);

  draw_button(screen2,224,168,61,21,"OK",vc(1),vc(14),0,true);
  draw_button(screen2,224,192,61,21,"Cancel",vc(1),vc(14),0,true);
  draw_button(screen2,24,184,61,21,"Edit",vc(1),vc(14),0,true);

  //tool buttons
  for (int i=MOUSE_BMP_SWORD; i<MOUSE_BMP_BLANK; i++)
  {
    int column=tool_buttons_columns*(i-MOUSE_BMP_SWORD)/(MOUSE_BMP_BLANK-MOUSE_BMP_SWORD+1);
    int rows=(MOUSE_BMP_BLANK-MOUSE_BMP_SWORD+2)/tool_buttons_columns;
    int row=(i-MOUSE_BMP_SWORD)-(column*rows);
    jwin_draw_button(screen2,tool_buttons_left+(column*23),tool_buttons_top+(row*23),22,22,tool==(i-MOUSE_BMP_SWORD)?2:0,0);
    masked_blit(mouse_bmp[i][0],screen2,0,0,tool_buttons_left+(column*23)+3+(tool==(i-MOUSE_BMP_SWORD)?1:0),tool_buttons_top+3+(row*23)+(tool==(i-MOUSE_BMP_SWORD)?1:0),16,16);
  }

  //coordinates
  if(isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158))
  {
    int temp_x=(gui_mouse_x()-80)/8;
    int temp_y=(gui_mouse_y()-32)/8;
    textprintf_ex(screen2,font,224,136,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"x: %d",temp_x);
    textprintf_ex(screen2,font,264,136,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"y: %d",temp_y);
    unpack_tile(newtilebuf, tile, 0, false);
    byte *si = unpackbuf;
    si+=(temp_y*16+temp_x);
    get_palette(tpal);
    if(newtilebuf[tile].format<=tf4Bit)
    {
      textprintf_ex(screen2,font,224,144,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%02d %02d %02d  (%d)",tpal[CSET(cs)+(*si)].r,tpal[CSET(cs)+(*si)].g,tpal[CSET(cs)+(*si)].b,*si);
    }
    else
    {
      textprintf_ex(screen2,font,224,144,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%02d %02d %02d",tpal[(*si)].r,tpal[(*si)].g,tpal[(*si)].b);
    }
  }

  vsync();
  scare_mouse();
  blit(screen2,screen,0,0,0,0,320,240);
  update_tool_cursor();
  unscare_mouse();
  SCRFIX();
}

void normalize(int tile,int tile2, bool rect_sel, int flip)
{
  if (tile>tile2)
  {
    swap(tile, tile2);
  }

  //  int top=tile_row(min(tile, tile2));
  int left=min(tile_col(tile), tile_col(tile2));
  //  int rows=tile_row(max(tile, tile2))-top+1;
  int columns=max(tile_col(tile), tile_col(tile2))-left+1;


  for (int temptile=tile; temptile<=tile2; temptile++)
  {
    if (!rect_sel || ((tile_col(temptile)>=left) && (tile_col(temptile)<=left+columns-1)))
    {
      unpack_tile(newtilebuf, temptile, 0, false);

      if(flip&1)
      {
        for(int y=0; y<16; y++)
        {
          for(int x=0; x<8; x++)
          {
            swap(unpackbuf[(y<<4)+x],unpackbuf[(y<<4)+15-x]);
          }
        }
      }
      if(flip&2)
      {
        for(int y=0; y<8; y++)
        {
          for(int x=0; x<16; x++)
          {
            swap(unpackbuf[(y<<4)+x],unpackbuf[((15-y)<<4)+x]);
          }
        }
      }

      pack_tile(newtilebuf,unpackbuf,temptile);
    }
  }
}

void rotate_tile(int tile, bool backward)
{
  unpack_tile(newtilebuf, tile, 0, false);
  byte tempunpackbuf[256];
  byte tempx, tempy;
  for (tempx=0; tempx<16; tempx++)
  {
    for (tempy=0; tempy<16; tempy++)
    {
      if (!backward)
      {
        tempunpackbuf[(tempy<<4)+tempx]=unpackbuf[((15-tempx)<<4)+tempy];
      }
      else
      {
        tempunpackbuf[((15-tempx)<<4)+tempy]=unpackbuf[(tempy<<4)+tempx];
      }
    }
  }
  pack_tile(newtilebuf,tempunpackbuf,tile);
}

static int undocount=128;
byte undotile[256];

void wrap_tile(int tile, int vertical, int horizontal, bool clear)
{
  byte buf[256];
  for(int i=0; i<undocount; i++)
  {
    newtilebuf[tile].data[i]=undotile[i];
  }
  if (!(horizontal||vertical))
  {
    return;
  }
  unpack_tile(newtilebuf, tile, 0, true);

  //vertical
  if (vertical)
  {
    for(int i=0; i<256; i++)
    {
      buf[(i+(vertical*16))&0xFF] = unpackbuf[i];
    }
    memcpy(unpackbuf,buf,256);
  }

  //horizontal
  if (horizontal)
  {
    for(int i=0; i<256; i++)
    {
      buf[((i+horizontal)&15)+(i&0xF0)] = unpackbuf[i];
    }
  }
  if (clear)
  {
    for (int r=0; r<abs(vertical); r++)
    {
      for(int c=0; c<16; c++)
      {
        buf[(vertical>0?r:15-r)*16+c]=0;
      }
    }
    for (int r=0; r<16; r++)
    {
      for(int c=0; c<abs(horizontal); c++)
      {
        buf[r*16+(horizontal>0?c:15-c)]=0;
      }
    }
  }
  pack_tile(newtilebuf,buf,tile);
}

void clear_selection_grid()
{
  for(int x=0; x<18; ++x)
  {
    for(int y=0; y<18; ++y)
    {
      selection_grid[x][y]=0;
    }
  }
}

void invert_selection_grid()
{
  for(int x=1; x<17; ++x)
  {
    for(int y=1; y<17; ++y)
    {
      selection_grid[x][y]=selection_grid[x][y]?0:1;
    }
  }
}

void edit_tile(int tile,int flip,int &cs)
{
  undocount = tilesize(newtilebuf[tile].format);
  clear_selection_grid();
  selecting_x1=selecting_x2=selecting_y1=selecting_y2=-1;

  PALETTE tpal;
  byte oldtile[256];
  for(int i=0; i<undocount; i++)
  {
    oldtile[i]=undotile[i]=newtilebuf[tile].data[i];
  }

  int tile_x=-1, tile_y=-1;
  int temp_x=-1, temp_y=-1;
  bool bdown=false;
  int done=0;
  drawing=0;
  tool_cur = -1;

  get_palette(tpal);
  if (newtilebuf[tile].format==tf4Bit)
  {
    tpal[208]=tpal[CSET(cs)+c1];
    tpal[209]=tpal[CSET(cs)+c2];
    tpal[210]=invRGB(tpal[CSET(cs)]);
  }
  else
  {
    tpal[208]=tpal[c1];
    tpal[209]=tpal[c2];
    tpal[210]=invRGB(tpal[0]);
  }
  vsync();
  set_palette(tpal);
  draw_edit_scr(tile,flip,cs,oldtile);
  while(gui_mouse_b());

  int move_origin_x=-1, move_origin_y=-1;
  int prev_x=-1, prev_y=-1;



  byte selection_pattern_source[8][8]=
  {
    {1, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 0, 0, 0, 0, 1},
    {1, 1, 0, 0, 0, 0, 1, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
  };

  byte selecting_pattern_source[8][8]=
  {
    {1, 1, 0, 0, 0, 0, 1, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 0, 0, 0, 0, 1},
  };

  byte intersection_pattern_source[8][8]=
  {
    {0, 0, 1, 1, 0, 0, 1, 1},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {1, 1, 0, 0, 1, 1, 0, 0},
    {1, 0, 0, 1, 1, 0, 0, 1},
    {0, 0, 1, 1, 0, 0, 1, 1},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {1, 1, 0, 0, 1, 1, 0, 0},
    {1, 0, 0, 1, 1, 0, 0, 1},
  };

  selection_pattern=create_bitmap_ex(8, 8, 8);
  for (int x=0; x<8; ++x)
  {
    for (int y=0; y<8; ++y)
    {
      //      rectfill(selection_pattern, x<<2, y<<2, (x<<2)+3, (y<<2)+3, (((x^y)&1)==0)?vc(15):vc(0));
      selection_pattern->line[y][x]=selection_pattern_source[x][y]?vc(0):vc(15);
    }
  }

  selecting_pattern=create_bitmap_ex(8, 8, 8);
  for (int x=0; x<8; ++x)
  {
    for (int y=0; y<8; ++y)
    {
      //      rectfill(selection_pattern, x<<2, y<<2, (x<<2)+3, (y<<2)+3, (((x^y)&1)==0)?vc(15):vc(0));
      selecting_pattern->line[y][x]=selecting_pattern_source[x][y]?vc(0):vc(15);
    }
  }

  intersection_pattern=create_bitmap_ex(8, 8, 8);
  for (int x=0; x<8; ++x)
  {
    for (int y=0; y<8; ++y)
    {
      //      rectfill(selection_pattern, x<<2, y<<2, (x<<2)+3, (y<<2)+3, (((x^y)&1)==0)?vc(15):vc(0));
      intersection_pattern->line[y][x]=intersection_pattern_source[x][y]?vc(0):vc(15);
    }
  }


  do
  {
    rest(4);
    bool redraw=false;
    bool did_wand_select=false;

    if(keypressed())
    {
      switch(readkey()>>8)
      {
        case KEY_ENTER_PAD:
        case KEY_ENTER: done=2; break;
        case KEY_ESC:   done=1; break;
        case KEY_A:     clear_selection_grid(); invert_selection_grid(); redraw=true; break;
        case KEY_D:     clear_selection_grid(); redraw=true; break;
        case KEY_I:     invert_selection_grid(); redraw=true; break;
        case KEY_H:     flip^=1; normalize(tile,tile,0,flip); flip=0; redraw=true; break;
        case KEY_V:     flip^=2; normalize(tile,tile,0,flip); flip=0; redraw=true; break;
        case KEY_F12:   onSnapshot(); break;
        case KEY_R:     go_tiles(); rotate_tile(tile,(key[KEY_LSHIFT] || key[KEY_RSHIFT])); redraw=true; saved=false; break;
        case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  redraw=true; break;
        case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; redraw=true; break;
        case KEY_SPACE: gridmode=(gridmode+1)%gm_max; redraw=true; break;
        case KEY_U:     for(int i=0; i<undocount; i++) swap(undotile[i],newtilebuf[tile].data[i]); redraw=true; break;
        case KEY_S:
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
          }
          unpack_tile(newtilebuf, tile, 0, false);
          for (int i=0; i<256; i++)
          {
            if (unpackbuf[i]==c1)
            {
              unpackbuf[i]=c2;
            }
            else if (unpackbuf[i]==c2)
              {
                unpackbuf[i]=c1;
              }
          }
          pack_tile(newtilebuf, unpackbuf,tile);
        }
        swap(c1,c2);
        redraw=true;
        break;
        case KEY_UP:
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          tile=max(0,tile-TILES_PER_ROW);
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
            oldtile[i]=undotile[i];
          }
          redraw=true;
        }
        else
        {
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
          }
          wrap_tile(tile, -1, 0, false);
          redraw=true;
        }
        break;
        case KEY_DOWN:
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          tile=min(tile+TILES_PER_ROW,NEWMAXTILES-1);
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
            oldtile[i]=undotile[i];
          }
          redraw=true;
        }
        else
        {
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
          }
          wrap_tile(tile, 1, 0, false);
          redraw=true;
        }
        break;
        case KEY_LEFT:
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          tile=max(0,tile-1);
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
            oldtile[i]=undotile[i];
          }
          redraw=true;
        }
        else
        {
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
          }
          wrap_tile(tile, 0, -1, false);
          redraw=true;
        }
        break;
        case KEY_RIGHT:
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          tile=min(tile+1, NEWMAXTILES-1);
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
            oldtile[i]=undotile[i];
          }
          redraw=true;
        }
        else
        {
          for(int i=0; i<undocount; i++)
          {
            undotile[i]=newtilebuf[tile].data[i];
          }
          wrap_tile(tile, 0, 1, false);
          redraw=true;
        }
        break;
      }
      clear_keybuf();
    }
    /*
      if(!key[KEY_LSHIFT] && !key[KEY_RSHIFT] &&
      !key[KEY_ZC_LCONTROL] && !key[KEY_ZC_RCONTROL] &&
      !key[KEY_ALT] && !key[KEY_ALTGR]) {
      */
    if (isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158))
    {
      if (!bdown&&(gui_mouse_b()&3)) //pressed the left or right button
      {
        zq_set_mouse_range(80,32,206,158);
      }
      else if (bdown&&!gui_mouse_b()) //released the mouse button
      {
        zq_set_mouse_range(0,0,zq_screen_w-1,zq_screen_h-1);
      }
    }

    if (!gui_mouse_b())
    {
      if(is_selecting())
      {
        int x1=min(selecting_x1,selecting_x2);
        int x2=max(selecting_x1,selecting_x2);
        int y1=min(selecting_y1,selecting_y2);
        int y2=max(selecting_y1,selecting_y2);
        if (select_mode==0)
        {
          clear_selection_grid();
        }
        for (int x=x1; x<=x2; ++x)
        {
          for (int y=y1; y<=y2; ++y)
          {
            selection_grid[x+1][y+1]=((select_mode<2)?(1):(((select_mode==2)?(0):(selection_grid[x+1][y+1]))));
          }
        }
        if (select_mode==3)
        {
          for (int y=0; y<16; ++y)
          {
            for (int x=0; x<x1; ++x)
            {
              selection_grid[x+1][y+1]=0;
            }
            for (int x=x2+1; x<16; ++x)
            {
              selection_grid[x+1][y+1]=0;
            }
          }
          for (int x=x1; x<=x2; ++x)
          {
            for (int y=0; y<y1; ++y)
            {
              selection_grid[x+1][y+1]=0;
            }
            for (int y=y2+1; y<16; ++y)
            {
              selection_grid[x+1][y+1]=0;
            }
          }
        }
      }
      selecting_x1=selecting_x2=selecting_y1=selecting_y2=-1;
      did_wand_select=false;
    }
    bool alt=(key[KEY_ALT]||key[KEY_ALTGR]);
    bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);

    if (tool==t_select||tool==t_wand)
    {
      if (!drawing)
      {
        int type=0;
        if (has_selection())
        {
          switch(tool)
          {
            case t_select:
            case t_wand:
              if (shift)
              {
                type+=1;
              }
              if (alt)
              {
                type+=2;
              }
              break;
          }
        }
        if (type!=select_mode)
        {
          select_mode=type;
          if(isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158)) //inside the zoomed tile window
          {
            set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][type]);
          }
        }
      }
    }
    else if(alt)
    {
      if (old_tool==-1)
      {
        old_tool = tool;
        tool = t_eyedropper;
        tool_cur = -1;
        redraw = true;
      }
    }
    else
    {
      if (old_tool!=-1)
      {
        tool = old_tool;
        old_tool = -1;
        tool_cur = -1;
        redraw = true;
      }
    }
    if(!bdown)
    {
      move_origin_x=prev_x=(gui_mouse_x()-80)>>3;
      move_origin_y=prev_y=(gui_mouse_y()-32)>>3;
    }

    if(gui_mouse_b()==1 && !bdown) //pressed the left mouse button
    {
      if(isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158))
      {
        if(tool==t_move)
        {
          scare_mouse();
          set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][1]);
          set_mouse_sprite_focus(8, 8);
          unscare_mouse();
          move_origin_x=prev_x=(gui_mouse_x()-80)>>3;
          move_origin_y=prev_y=(gui_mouse_y()-32)>>3;
        }
        for(int i=0; i<undocount; i++)
        {
          undotile[i]=newtilebuf[tile].data[i];
        }
        drawing=1;
      }

      if(isinRect(gui_mouse_x(),gui_mouse_y(),224,168,285,189))
      {
        if(do_button(224,168,61,21,"OK",vc(1),vc(14),true))
        {
          done=2;
        }
      }

      if(isinRect(gui_mouse_x(),gui_mouse_y(),224,192,285,213))
      {
        if(do_button(224,192,61,21,"Cancel",vc(1),vc(14),true))
        {
          done=1;
        }
      }

      if(isinRect(gui_mouse_x(),gui_mouse_y(),24,184,85,205))
      {
        if(do_button(24,184,61,21,"Edit",vc(1),vc(14),true))
        {
          popup_menu(colors_menu,26,144);
          redraw=true;
        }
      }

      switch(newtilebuf[tile].format)
      {
        case tf4Bit:
        if(isinRect(gui_mouse_x(),gui_mouse_y(),104,176,135,207))
        {
          int x=(gui_mouse_x()-104)>>3;
          int y=(gui_mouse_y()-176)>>3;
          c1 = (y<<2)+x;
          redraw=true;
        }
        break;
        case tf8Bit:
        if(isinRect(gui_mouse_x(),gui_mouse_y(),94,172,157,231))
        {
          int x=(gui_mouse_x()-94)>>2;
          int y=(gui_mouse_y()-172)>>2;
          c1 = (y<<4)+x;
          redraw=true;
        }
        break;
      }


      for(int i=0; i<t_max; i++)
      {
        int column=tool_buttons_columns*i/(t_max+1);
        int rows=(t_max+2)/tool_buttons_columns;
        int row=i-(column*rows);
//        if(isinRect(gui_mouse_x(),gui_mouse_y(),tool_buttons_left,(i*23)+tool_buttons_top,tool_buttons_left+21,(i*23)+tool_buttons_top+21))
        if(isinRect(gui_mouse_x(),gui_mouse_y(),tool_buttons_left+(column*23),tool_buttons_top+(row*23),tool_buttons_left+(column*23)+21,tool_buttons_top+(row*23)+21))
        {
          tool=i;
          redraw=true;
        }
      }
      if(isinRect(gui_mouse_x(),gui_mouse_y(),SCREEN_W - 21, 5, SCREEN_W - 21 + 15, 5 + 13))
      {
        if(do_x_button(screen, SCREEN_W - 21, 5))
        {
          done=1;
        }
      }
      bdown=true;
    }

    if(gui_mouse_b()&2 && !bdown) //pressed the left mouse button
    {
      if(isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158))
      {
        if(tool==t_move)
        {
          scare_mouse();
          set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][1]);
          set_mouse_sprite_focus(8, 8);
          unscare_mouse();
          move_origin_x=prev_x=(gui_mouse_x()-80)>>3;
          move_origin_y=prev_y=(gui_mouse_y()-32)>>3;
        }
        for(int i=0; i<undocount; i++)
        {
          undotile[i]=newtilebuf[tile].data[i];
        }
        drawing=2;
      }
      switch(newtilebuf[tile].format)
      {
        case tf4Bit:
        if(isinRect(gui_mouse_x(),gui_mouse_y(),104,176,135,207))
        {
          int x=(gui_mouse_x()-104)>>3;
          int y=(gui_mouse_y()-176)>>3;
          c2 = (y<<2)+x;
          redraw=true;
        }
        break;
        case tf8Bit:
        if(isinRect(gui_mouse_x(),gui_mouse_y(),94,172,157,231))
        {
          int x=(gui_mouse_x()-94)>>2;
          int y=(gui_mouse_y()-172)>>2;
          c2 = (y<<4)+x;
          redraw=true;
        }
        break;
      }
      bdown=true;
    }

    if (bdown&&!gui_mouse_b()) //released the buttons
    {
      if(isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158))
      {
        if(tool==t_move)
        {
          scare_mouse();
          set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][0]);
          set_mouse_sprite_focus(8, 8);
          unscare_mouse();
        }
      }
    }
    if(drawing && isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158))
    {
      int x=(gui_mouse_x()-80)>>3;
      int y=(gui_mouse_y()-32)>>3;

      switch(tool)
      {
        case t_pen:
          if(flip&1) x=15-x;
          if(flip&2) y=15-y;
          if (is_in_selection(x,y))
          {
            unpack_tile(newtilebuf, tile, 0, false);
            unpackbuf[((y<<4)+x)]=(drawing==1)?c1:c2;
            pack_tile(newtilebuf, unpackbuf,tile);
          }
          break;
        case t_brush:
          if (is_in_selection(x,y))
          {
            tile_floodfill(tile,x,y,(drawing==1)?c1:c2);
            drawing=0;
          }
          break;
        case t_recolor:
          if (is_in_selection(x,y))
          {
            unpack_tile(newtilebuf, tile, 0, false);
            tf_u = unpackbuf[(y<<4)+x];
            for(int i=0; i<256; i++)
            {
              if (is_in_selection(i&15,i>>4))
              {
                if(unpackbuf[i]==tf_u)
                {
                  unpackbuf[i]=(drawing==1)?c1:c2;
                }
              }
            }
            pack_tile(newtilebuf, unpackbuf,tile);
            drawing=0;
          }
          break;
        case t_eyedropper:
          unpack_tile(newtilebuf, tile, 0, false);
          if (gui_mouse_b()&1)
          {
            c1=unpackbuf[((y<<4)+x)];
          }
          if (gui_mouse_b()&2)
          {
            c2=unpackbuf[((y<<4)+x)];
          }
          break;
          case t_move:
          if ((prev_x!=x)||(prev_y!=y))
          {
            wrap_tile(tile, y-move_origin_y, x-move_origin_x, drawing==2);
            prev_x=x;
            prev_y=y;
          }
          break;
        case t_select:
          if(flip&1) x=15-x;
          if(flip&2) y=15-y;
          if(selecting_x1==-1||selecting_y1==-1)
          {
            selecting_x1=x;
            selecting_y1=y;
          }
          else
          {
            selecting_x2=x;
            selecting_y2=y;
          }
          break;
        case t_wand:
          if(flip&1) x=15-x;
          if(flip&2) y=15-y;
          switch(select_mode)
          {
            case 0:
              clear_selection_grid();
              add_color_to_selection(unpackbuf[((y<<4)+x)]);
              break;
            case 1:
              add_color_to_selection(unpackbuf[((y<<4)+x)]);
              break;
            case 2:
              remove_color_from_selection(unpackbuf[((y<<4)+x)]);
              break;
            case 3:
              intersect_color_with_selection(unpackbuf[((y<<4)+x)]);
              break;
          }
          drawing=0;
          break;
      }
      redraw=true;
    }

    if(gui_mouse_b()==0)
    {
      bdown=false;
      drawing=0;
    }

    temp_x=(gui_mouse_x()-80)/8;
    temp_y=(gui_mouse_y()-32)/8;
    if(!isinRect(gui_mouse_x(),gui_mouse_y(),80,32,206,158))
    {
      temp_x=-1;
      temp_y=-1;
    }
    if (temp_x!=tile_x||temp_y!=tile_y)
    {
      {
        tile_x=temp_x;
        tile_y=temp_y;
        redraw=true;
      }
    }

    if(redraw)
    {
      get_palette(tpal);
      if (newtilebuf[tile].format==tf4Bit)
      {
        tpal[208]=tpal[CSET(cs)+c1];
        tpal[209]=tpal[CSET(cs)+c2];
        tpal[210]=invRGB(tpal[CSET(cs)]);
      }
      else
      {
        tpal[208]=tpal[c1];
        tpal[209]=tpal[c2];
        tpal[210]=invRGB(tpal[0]);
      }
      vsync();
      set_palette(tpal);
      draw_edit_scr(tile,flip,cs,oldtile);
    }
    else
    {
      bool hs=has_selection();
      if (hs)
      {
        zoomtile16(screen2,tile,79,31,cs,flip,8);
      }
      vsync();
      scare_mouse();
      if (hs)
      {
        blit(screen2, screen, 79, 31, 79, 31, 129, 129);
      }
      update_tool_cursor();
      unscare_mouse();
      SCRFIX();
    }

  } while(!done);

  while(gui_mouse_b());

  if(done==1)
  {
    for(int i=0; i<undocount; i++)
    {
      newtilebuf[tile].data[i]=oldtile[i];
    }
  }
  else
  {
    byte *buf = new byte[undocount];
    // put back old tile
    for(int i=0; i<undocount; i++)
    {
      buf[i] = newtilebuf[tile].data[i];
      newtilebuf[tile].data[i] = oldtile[i];
    }
    // go
    go_tiles();
    // replace old tile with new one again
    for(int i=0; i<undocount; i++)
    {
      newtilebuf[tile].data[i] = buf[i];
    }

    //   usetiles=true;
    saved=false;
    if(buf!=NULL)
    {
      delete[] buf;
    }
  }
  set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
}

/*  Grab Tile Code  */

void *imagebuf=NULL;
long imagesize=0;
long tilecount=0;
int  imagetype=0;
int imagex,imagey,selx,sely;
int bp=4,grabmode=16,romofs=0,romtilemode=0, romtilecols=8;
bool nesmode=false;
int grabmask=0;
bool reduce=false;
PALETTE imagepal;

/* 1.5k lookup table for color matching */
//unsigned int col_diff[3*128];

/* bestfit_init:
  *  Color matching is done with weighted squares, which are much faster
  *  if we pregenerate a little lookup table...
  */
static void bestfit_cset_init(void)
{
  int i;

  for (i=1; i<64; i++)
  {
    int k = i * i;
    col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
    col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
    col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
  }
}

/* bestfit_color:
  *  Searches a palette for the color closest to the requested R, G, B value.
  */
int bestfit_cset_color(int cs, int r, int g, int b)
{
  int i, coldiff, lowest, bestfit;

  if (col_diff[1] == 0)
    bestfit_cset_init();

  bestfit = 0;
  lowest = INT_MAX;

  /* only the transparent (pink) color can be mapped to index 0 */
  i = 0;

  while (i<CSET_SIZE)
  {
    byte *rgb_byte= &colordata[((cs<<4)+i)*3];
    RGB rgb;
    rgb.r = *rgb_byte; rgb_byte++;
    rgb.g = *rgb_byte; rgb_byte++;
    rgb.b = *rgb_byte;
    coldiff = (col_diff + 0) [ (rgb.g - g) & 0x7F ];
    if (coldiff < lowest)
    {
      coldiff += (col_diff + 128) [ (rgb.r - r) & 0x7F ];
      if (coldiff < lowest)
      {
        coldiff += (col_diff + 256) [ (rgb.b - b) & 0x7F ];
        if (coldiff < lowest)
        {
          bestfit = i;                                      /* faster than `bestfit = i;' */
          if (coldiff == 0)
            return bestfit;
          lowest = coldiff;
        }
      }
    }
    i++;
  }

  return bestfit;
}


byte cset_reduce_table[PAL_SIZE];

void calc_cset_reduce_table(PALETTE pal, int cs)
{
  for (int i=0; i<PAL_SIZE; i++)
  {
    cset_reduce_table[i]=(bestfit_cset_color(cs, pal[i].r, pal[i].g, pal[i].b)&0x0F);
  }
}

void puttileROM(BITMAP *dest,int x,int y,byte *src,int cs)
{
  //storage space for the grabbed image
  byte buf[64];
  memset(buf,0,64);
  byte *oldsrc=src;

  //for 8 lines in the source image...
  for(int line=0; line<(nesmode?4:8); line++)
  {
    //bx is the pixel at the start of a line in the storage buffer
    int  bx=line<<(nesmode?4:3);
    //b is a byte in the source image (either an entire line in 1bp or the start of a line in others)
    byte b=src[(bp&1)?line:line<<1];

    //fill the storage buffer with data from the source image
    for(int i=7; i>=0; --i)
    {
      buf[bx+i] = (b&1)+(cs<<4);
      b>>=1;
    }
  }
  ++src;
  ///*
  for(int p=1; p<bp; p++)
  {
    for(int line=0; line<(nesmode?4:8); line++)
    {
      int  bx=line<<(nesmode?4:3);
      byte b=src[(bp&1)?line:line<<1];

      for(int i=7; i>=0; --i)
      {
        if(nesmode)
        {
          buf[bx+8+i] = (b&1)+(cs<<4);
        }
        else
        {
          buf[bx+i] |= (b&1)<<p;
        }
        b>>=1;
      }
    }
    if(p&1)
    {
      src+=15;
    }
    else
    {
      ++src;
    }
  }
  //*/

  if(nesmode)
  {
    src=oldsrc;

    for (int counter=0; counter<2; ++counter, ++src)
    {
      //for 8 lines in the source image...
      for(int line=0; line<4; line++)
      {
        //bx is the pixel at the start of a line in the storage buffer
        int  bx=line<<4;
        //b is a byte in the source image (either an entire line in 1bp or the start of a line in others)
        byte b=src[(line+4)<<1];

        //fill the storage buffer with data from the source image
        for(int i=7; i>=0; --i)
        {
          //        buf[bx+i] = (b&1)+(cs<<4);
          buf[bx+(counter<<3)+i] |= (b&1)<<1;
          b>>=1;
        }
      }
    }
  }

  int c=0;

  switch (romtilemode)
  {
    case 0:
    case 1:
    case 2:
    for(int j=0; j<8; j++)
    {
      for(int i=0; i<8; i++)
      {
        putpixel(dest,x+i,y+j,buf[c++]);
      }
    }
    break;
    case 3:
    for(int j=0; j<4; j++)
    {
      for(int i=0; i<16; i++)
      {
        putpixel(dest,x+i,y+j,buf[c++]);
      }
    }
    break;
  }
}

char *file_type[ftMAX]=
{
  "None", "BIN", "BMP", "TIL", "ZGP", "QSU", "ZQT", "QST"
};

void draw_grab_scr(int tile,int cs,byte *newtile,int black,int white, int width, int height, byte *newformat)
{
  //clear_to_color(screen2,bg);
  rectfill(screen2, 0, 0, 319, 159, black);
  jwin_draw_win(screen2, 0, 160, 320, 80, FR_WIN);

  // text_mode(-1);
  int tileromcolumns=20;
  switch(imagetype)
  {
    case ftBMP:
      if (!reduce)
      {
        blit((BITMAP*)imagebuf,screen2,imagex<<4,imagey<<4,0,0,320,160);
      }
      else
      {
        int maxy=min(160,((BITMAP*)imagebuf)->h);
        int maxx=min(320,((BITMAP*)imagebuf)->w);

        for (int y=0; y<maxy; y++)
        {
          if ((imagey<<4)+y>=((BITMAP*)imagebuf)->h)
          {
            break;
          }
          for (int x=0; x<maxx; x++)
          {
            if ((imagex<<4)+x>=((BITMAP*)imagebuf)->w)
            {
              break;
            }
            screen2->line[y][x]=(cset_reduce_table[((BITMAP*)imagebuf)->line[(imagey<<4)+y][(imagex<<4)+x]])+(cs<<4);
          }
        }
      }
      draw_button(screen2,141,190,61,20,"Pal",vc(1),vc(14),0,true);
      break;

    case ftZGP:
    case ftQST:
    case ftZQT:
    case ftQSU:
    case ftTIL:
    {
      tiledata *hold = newtilebuf;
      newtilebuf = grabtilebuf;
      //fixme
      int t=imagey*TILES_PER_ROW;
      for(int i=0; i<200; i++)                              // 10 rows, down to y=160
      {
        if(t <= tilecount)
        {
          puttile16(screen2,t,(i%TILES_PER_ROW)<<4,(i/TILES_PER_ROW)<<4,cs,0);
        }
        ++t;
      }
      newtilebuf = hold;
      //fixme
    }
    break;

    case ftBIN:
    {
      int ofs = (tileromcolumns*imagex + imagey) * 128*bp + romofs;
      byte *buf = (byte*)imagebuf;
      switch (romtilemode)
      {
        case 0:
        for(int y=0; y<160; y+=8)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
          {
            puttileROM(screen2,x,y,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        for(int y=0; y<160; y+=8)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
          {
            puttileROM(screen2,x+128,y,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        break;
        case 1:
        for(int y=0; y<160; y+=16)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
          {
            puttileROM(screen2,x,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x,y+8,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        for(int y=0; y<160; y+=16)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
          {
            puttileROM(screen2,x+128,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+128,y+8,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        break;
        case 2:
        for(int y=0; y<160; y+=16)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
          {
            puttileROM(screen2,x,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+8,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x,y+8,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+8,y+8,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        for(int y=0; y<160; y+=16)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
          {
            puttileROM(screen2,x+128,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+136,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+128,y+8,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+136,y+8,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        break;
        case 3:
        for(int y=0; y<160; y+=16)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
          {
            puttileROM(screen2,x,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x,y+4,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x,y+8,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x,y+12,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        for(int y=0; y<160; y+=16)
        {
          for(int x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
          {
            puttileROM(screen2,x+128,y,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+128,y+4,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+128,y+8,buf+ofs,cs);
            ofs+=8*bp;
            puttileROM(screen2,x+128,y+12,buf+ofs,cs);
            ofs+=8*bp;
          }
        }
        break;
      }
    }
    break;
  }

  //swap(newtile,newtilebuf[0].data);
  tiledata hold;
  hold.format = newtilebuf[0].format;
  hold.data = (byte *)malloc(tilesize(hold.format));
  memcpy(hold.data, newtilebuf[0].data, tilesize(hold.format));

  newtilebuf[0].format=newformat[0];
  if(newtilebuf[0].data!=NULL)
  {
    free(newtilebuf[0].data);
  }
  newtilebuf[0].data = (byte *)malloc(tilesize(newtilebuf[0].format));
  for(int i=0;i<tilesize(newtilebuf[0].format);i++)
  {
    newtilebuf[0].data[i]=newtile[i];
  }
  puttile16(screen2,0,208,168,cs,0);
  overtile16(screen2,0,232,168,cs,0);
  //swap(newtile,newtilebuf[0].data);
  newtilebuf[0].format=hold.format;
  if(newtilebuf[0].data!=NULL)
  {
    free(newtilebuf[0].data);
  }
  newtilebuf[0].data = (byte *)malloc(tilesize(newtilebuf[0].format));
  for(int i=0;i<newtilebuf[0].format*128;i++)
  {
    newtilebuf[0].data[i]=hold.data[i];
  }
  free(hold.data);

  puttile16(screen2,tile,208,192,cs,0);
  overtile16(screen2,tile,232,192,cs,0);

  rectfill(screen2,184,168,191,175,grabmask&1?vc(12):vc(7));
  rectfill(screen2,192,168,199,175,grabmask&2?vc(12):vc(7));
  rectfill(screen2,184,176,191,183,grabmask&4?vc(12):vc(7));
  rectfill(screen2,192,176,199,183,grabmask&8?vc(12):vc(7));

  // rect(screen2,183,167,200,184,dvc(7*2));
  // rect(screen2,207,167,224,184,dvc(7*2));
  // rect(screen2,231,167,248,184,dvc(7*2));
  // rect(screen2,207,191,224,208,dvc(7*2));
  // rect(screen2,231,191,248,208,dvc(7*2));

/*
  rect(screen2,183,167,200,184,vc(14));
  rect(screen2,207,167,224,184,vc(14));
  rect(screen2,231,167,248,184,vc(14));
  rect(screen2,207,191,224,208,vc(14));
  rect(screen2,231,191,248,208,vc(14));
*/
  jwin_draw_frame(screen2,182,166,20,20,FR_DEEP);
  jwin_draw_frame(screen2,206,166,20,20,FR_DEEP);
  jwin_draw_frame(screen2,230,166,20,20,FR_DEEP);
  jwin_draw_frame(screen2,206,190,20,20,FR_DEEP);
  jwin_draw_frame(screen2,230,190,20,20,FR_DEEP);

  switch(imagetype)
  {
    case 0:     textprintf_ex(screen2,font,8,216,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s",imgstr[imagetype]); break;
    case ftBMP: textprintf_ex(screen2,font,8,216,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s  %dx%d",imgstr[imagetype],((BITMAP*)imagebuf)->w,((BITMAP*)imagebuf)->h); break;
    case ftZGP:
    case ftQST:
    case ftZQT:
    case ftQSU:
    case ftTIL:
    case ftBIN: textprintf_ex(screen2,font,8,216,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s  %ld KB",imgstr[imagetype],imagesize>>10); break;
  }

  textprintf_ex(screen2,font,8,168,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"sel: %d %d",selx,sely);
  textprintf_ex(screen2,font,8,176,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"pos: %d %d",imagex,imagey);
  textprintf_ex(screen2,font,8,192,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"cset: %d",cs);
  textprintf_ex(screen2,font,8,200,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"step: %d",grabmode);
  if(imagetype==ftBIN)
  {
    textprintf_ex(screen2,font,104,192,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"bp:  %d%s",bp,nesmode?" (NES)":"");
    textprintf_ex(screen2,font,104,200,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"ofs: %Xh",romofs);
    textprintf_ex(screen2,font,104,208,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"m: %d",romtilemode);
  }

  textprintf_ex(screen2,font,8,224,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s",imagepath);
//  rectfill(screen2,256,224,319,231,black);
  draw_button(screen2,255,168,61,20,"OK",vc(1),vc(14),0,true);
  draw_button(screen2,255,192,61,20,"Cancel",vc(1),vc(14),0,true);
  draw_button(screen2,255,216,61,20,"File",vc(1),vc(14),0,true);
  draw_button(screen2,117,166,61,20,"Leech",vc(1),vc(14),0,true);

  vsync();
  scare_mouse();
  blit(screen2,screen,0,0,0,0,320,240);
  rect(screen,selx,sely,selx+((width-1)*16)+15,sely+((height-1)*16)+15,white);
  unscare_mouse();
  SCRFIX();
}

RGB_MAP rgb_table;
COLOR_MAP imagepal_table;


extern void return_RAMpal_color(AL_CONST PALETTE pal, int x, int y, RGB *rgb)
{
  //these are here to bypass compiler warnings about unused arguments
  x=x;

  rgb->r = pal[y].r;
  rgb->g = pal[y].g;
  rgb->b = pal[y].b;
}


void load_imagebuf()
{
  PACKFILE *f;
  bool compressed=false;
  bool encrypted=false;
  tiledata *hold=newtilebuf;
  zquestheader tempheader;
  if(imagebuf)
  {
    switch (imagetype)
    {
      case ftBMP:
        destroy_bitmap((BITMAP*)imagebuf);
        break;
      case ftZGP:
      case ftQST:
      case ftZQT:
      case ftQSU:
      case ftTIL:
        clear_tiles(grabtilebuf);
        break;
      case ftBIN:
        free(imagebuf);
        break;
    }
    imagebuf=NULL;
  }

  imagex=imagey=selx=sely=romofs=0;
  bp=4;
  imagetype=filetype(imagepath);

  dword section_id;
  dword section_version;
  dword section_cversion;

  switch(imagetype)
  {
    case ftBMP:
      imagebuf = load_bitmap(imagepath,imagepal);
      imagesize = file_size_ex(imagepath);
      tilecount=0;
      create_rgb_table(&rgb_table, imagepal, NULL);
      rgb_map = &rgb_table;
      create_color_table(&imagepal_table, RAMpal, return_RAMpal_color, NULL);
      if(!imagebuf)
      {
        imagetype=0;
      }
      break;
    case ftBIN:
      imagesize = file_size_ex(imagepath);
      tilecount=0;
      if(imagesize)
      {
        imagebuf = malloc(imagesize);
        if(!readfile(imagepath,imagebuf,imagesize))
        {
          free(imagebuf);
          imagesize=0;
          imagetype=0;
        }
      }
      break;
    case ftTIL:
      imagesize = file_size_ex(imagepath);
      f = pack_fopen(imagepath,F_READ);
      if (!f)
      {
        goto error;
      }
      if(!p_mgetl(&section_id,f,true))
      {
        goto error;
      }
      if(section_id==ID_TILES)
      {
        if (readtiles(f, grabtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false, true)==0)
        {
          goto error;
        }
      }
   error:
      pack_fclose(f);
      tilecount=count_tiles(grabtilebuf);
      break;

    case ftZGP:
      imagesize = file_size_ex(imagepath);
      f=pack_fopen(imagepath,F_READ);
      if(!f)
      {
        goto error2;
      }
      if(!p_mgetl(&section_id,f,true))
      {
        goto error2;
      }
      if(section_id!=ID_GRAPHICSPACK)
      {
        goto error2;
      }
      //section version info
      if(!p_igetw(&section_version,f,true))
      {
        goto error2;
      }
      if(!p_igetw(&section_cversion,f,true))
      {
        goto error2;
      }
      //tiles
      if(!p_mgetl(&section_id,f,true))
      {
        goto error2;
      }
      if(section_id==ID_TILES)
      {
        if (readtiles(f, grabtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false, true)!=0)
        {
          goto error2;
        }
      }
    error2:
      pack_fclose(f);
      tilecount=count_tiles(grabtilebuf);
      break;

    case ftQST:
      encrypted=true;
    case ftZQT:
      compressed=true;
    case ftQSU:
      imagesize = file_size_ex(imagepath);
      newtilebuf=grabtilebuf;
      byte skip_flags[4];
      for (int i=0; i<32; ++i)
      {
        set_bit(skip_flags,i,1);
      }
      set_bit(skip_flags,skip_tiles,0);
      set_bit(skip_flags,skip_header,0);
      if(encrypted) setPackfilePassword(datapwd);
      loadquest(imagepath,&tempheader,&misc,customMIDIs,false,compressed,encrypted,true,skip_flags);
      if (encrypted&&compressed)
      {
        if(!quest_access(imagepath, &tempheader, compressed))
        {
          imagetype=0;
          imagesize=0;
          clear_tiles(grabtilebuf);
          chop_path(imagepath);
        }
      }
      setPackfilePassword(NULL);
      newtilebuf=hold;
      tilecount=count_tiles(grabtilebuf);
      break;
  }
}

static char bitstrbuf[32];
bool leeching_from_tiles=false;

char *bitlist(int index, int *list_size)
{
  int imported=2;
  if(index>=0)
  {
    bound(index,0,leeching_from_tiles?2:1);
    if (index==imported)
    {
      sprintf(bitstrbuf,"Imported");
    }
    else
    {
      sprintf(bitstrbuf,"%d",4<<index);
    }
    return bitstrbuf;
  }
  *list_size=leeching_from_tiles?3:2;
  return NULL;
}

static DIALOG leech_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,       8,    20-4,   303+1,  216-42+1,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,      (void *) "Leech Options", NULL, NULL },
  { d_dummy_proc,      160,  27-4,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL, NULL, NULL },
  // 2
  { jwin_button_proc,     180,  210-42-4,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { jwin_button_proc,     80,   210-42-4,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp, NULL, NULL },
  // 5
  { jwin_text_proc,       14,   49-4,  176,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Update Status Every:  ", NULL, NULL },
  { jwin_edit_proc,      114,   45-4,   36,   16,    vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,     155,   49-4,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Tiles", NULL, NULL },
  { jwin_radio_proc,     200,   49-4,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Seconds", NULL, NULL },
  //9
  { jwin_frame_proc,      14,   63-2,   176+70,  50+30,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL, NULL, NULL },
  { jwin_text_proc,       14+8, 60-2,   80,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " Duplicates ", NULL, NULL },

  { jwin_check_proc,       20,   70,  168,   8+1,    vc(15),  vc(1),  0,       0,          1,             0,       (void *) "Only check new tiles", NULL, NULL },
  { jwin_text_proc,       20,   90,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Normal:", NULL, NULL },
  { jwin_text_proc,       20,  100,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Horizontal Flip:", NULL, NULL },
  { jwin_text_proc,       20,  110,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Vertical Flip:", NULL, NULL },
  { jwin_text_proc,       20,  120,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Horizontal/Vertical Flip:", NULL, NULL },
  //16
  { jwin_radio_proc,      144,  90,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      184,  90,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      224,  90,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },

  { jwin_radio_proc,      144, 100,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      184, 100,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      224, 100,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },

  { jwin_radio_proc,      144, 110,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      184, 110,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      224, 110,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },

  { jwin_radio_proc,      144, 120,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      184, 120,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,      224, 120,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },

  { jwin_ctext_proc,      144+4,  80,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Ignore", NULL, NULL },
  { jwin_ctext_proc,      184+4,  80,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Check", NULL, NULL },
  { jwin_ctext_proc,      224+4,  80,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Discard", NULL, NULL },
  { jwin_droplist_proc,   76,   145,   80,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) bitlist, NULL, NULL },
  { jwin_text_proc,       14,   149,  60,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Color Depth: ", NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

bool leech_tiles(tiledata *dest,int start,int cs)
{
  bool shift=true; // fix this!
  int cst=0;
  int currtile=start;
  int height=0, width=0;
  byte *testtile = new byte[tilesize(tf32Bit)];
  byte imported_format=0;
  char updatestring[6];
  bool canadd;
  bool temp_canadd;
  bool duplicate;
  int total_duplicates_found=0, total_duplicates_discarded=0;
  int duplicates_found[4]=                                  //, duplicates_discarded[4]={0,0,0,0};
  {
    0,0,0,0
  };
  BITMAP *status;
  status = create_bitmap_ex(8,240,140);
  clear_bitmap(status);
  sprintf(updatestring, "%d", LeechUpdate);
  leech_dlg[0].dp2=lfont;
  leech_dlg[6].dp=updatestring;

  leech_dlg[10].flags=(OnlyCheckNewTilesForDuplicates!=0) ? D_SELECTED : 0;

  for (int i=0; i<2; i++)
  {
    leech_dlg[i+7].flags=0;
  }
  leech_dlg[7+((LeechUpdateTiles==0) ? 1 : 0)].flags=D_SELECTED;

  for (int i=0; i<12; i++)
  {
    leech_dlg[i+16].flags=0;
  }
  for (int i=0; i<4; i++)
  {
    leech_dlg[(DuplicateAction[i])+16+(i*3)].flags=D_SELECTED;
  }

  leech_dlg[31].d1=0;

  int ret = zc_popup_dialog(leech_dlg,3);
  if (ret==2)
  {
    delete[] testtile;
    return false;
  }
  int cdepth=leech_dlg[31].d1+1;
  int newformat=0;
  LeechUpdate=atoi(updatestring);
  LeechUpdateTiles=(leech_dlg[7].flags&D_SELECTED)?1:0;

  for (int j=0; j<4; j++)
  {
    for (int i=0; i<3; i++)
    {
      if (leech_dlg[i+16+(j*3)].flags&D_SELECTED)
      {
        DuplicateAction[j]=i;
      }
    }
  }

  OnlyCheckNewTilesForDuplicates=leech_dlg[10].flags&D_SELECTED?1:0;

  leeching_from_tiles=false;
  switch(imagetype)
  {
    case ftBIN:
      width=imagesize/128;
      height=1;
      break;
    case ftZGP:
    case ftQST:
    case ftZQT:
    case ftQSU:
    case ftTIL:
      leeching_from_tiles=true;
      width=count_tiles(grabtilebuf);
      height=1;
    break;
    case ftBMP:
      width=((((BITMAP*)imagebuf)->w)+15)/16;
      height=((((BITMAP*)imagebuf)->h)+15)/16;
    break;
  }

  if (currtile+(width*height)>NEWMAXTILES)
  {
    if(jwin_alert("Confirm Truncation","Too many tiles.","Truncation may occur.",NULL,"&OK","&Cancel",'o','c',lfont)==2)
    {
      delete[] testtile;
      return false;
    }
  }

  go_tiles();
  saved=false;
  //  usetiles=true;
  for (int ty=0; ty<height; ty++)                           //for every row
  {
    for (int tx=0; tx<width; tx++)                          //for every column (tile)
    {
      if ((((ty*width)+tx)%LeechUpdate)==0)                 //update status
      {
        FONT *oldfont = font;
        static BITMAP *tbar = create_bitmap_ex(8,240-6, 18);
        static bool created_tbar=false;
        jwin_draw_win(status, 0, 0, 240, 140, FR_WIN);
        if (created_tbar)
        {
          blit(tbar, status, 0, 0, 3, 3, 240-6, 18);
        }
        else
        {
          font = lfont;
          jwin_draw_titlebar(tbar, 0, 0, 240-6, 18, "Leech Status", false);
          font = oldfont;
          created_tbar=true;
          blit(tbar, status, 0, 0, 3, 3, SCREEN_W-6, 18);
        }
        textprintf_centre_ex(status,font,120,24,jwin_pal[jcTEXTFG],-1,"Checking %d of %d",((ty*width)+tx), (width*height));
        textprintf_centre_ex(status,font,120,34,jwin_pal[jcTEXTFG],-1,"%d tiles imported",currtile-start);
        jwin_draw_frame(status, 40, 49, 160, 70, FR_ETCHED);
        textprintf_centre_ex(status,font,120,46,jwin_pal[jcTEXTFG],jwin_pal[jcBOX]," Duplicates ");
        textprintf_centre_ex(status,font,120,56,jwin_pal[jcTEXTFG],-1,"%d/%d found/discarded",total_duplicates_found, total_duplicates_discarded);
        textprintf_centre_ex(status,font,120,76,jwin_pal[jcTEXTFG],-1,"%d normal %s",duplicates_found[0],((DuplicateAction[0]<2)?"found":"discarded"));
        textprintf_centre_ex(status,font,120,86,jwin_pal[jcTEXTFG],-1,"%d flipped (h) %s",duplicates_found[1],((DuplicateAction[1]<2)?"found":"discarded"));
        textprintf_centre_ex(status,font,120,96,jwin_pal[jcTEXTFG],-1,"%d flipped (v) %s",duplicates_found[2],((DuplicateAction[2]<2)?"found":"discarded"));
        textprintf_centre_ex(status,font,120,106,jwin_pal[jcTEXTFG],-1,"%d flipped (hv) %s",duplicates_found[3],((DuplicateAction[3]<2)?"found":"discarded"));
        textprintf_centre_ex(status,font,120,128,jwin_pal[jcTEXTFG],-1,"Press any key to stop.");
        scare_mouse();
        blit(status,screen,0, 0, 40, 20, 240, 140);
        unscare_mouse();
        SCRFIX();
      }
      canadd=true;
      if (currtile>=NEWMAXTILES)                            //if we've maxed out on our tiles...
      {
        delete[] testtile;
        return true;
      }
      switch(imagetype)
      {
        case ftBIN: break;
        case ftZGP:
        case ftQST:
        case ftZQT:
        case ftQSU:
        case ftTIL:
            memset(testtile, 0, tilesize(tf32Bit));
            imported_format=grabtilebuf[tx].format;
            switch(cdepth)
            {
              case 1:  //4-bit
                newformat=tf4Bit;
                switch(imported_format)
                {
                  case tf4Bit:
                    memcpy(testtile,grabtilebuf[tx].data,tilesize(imported_format));
                    break;
                  case tf8Bit:
                    for(int y=0; y<16; y++)                           //snag a tile
                    {
                      for(int x=0; x<16; x+=2)
                      {
                        testtile[(y*8)+(x/2)]=
                          (grabtilebuf[tx].data[y*16+x]&15)+
                          ((grabtilebuf[tx].data[y*16+x+1]&15)<<4);
                      }
                    }
                    break;
                }
                break;
              case 2:  //8-bit
                newformat=tf8Bit;
                switch(imported_format)
                {
                  case tf4Bit:
                    unpack_tile(grabtilebuf, tx, 0, true);
                    cst = cs&15;
                    cst <<= CSET_SHFT;
                    for(int i=0;i<256;i++)
                    {
                      if (!shift||unpackbuf[i]!=0)
                      {
                        unpackbuf[i]+=cst;
                      }
                    }
                    pack_tiledata(testtile, unpackbuf, tf8Bit);
                    break;
                  case tf8Bit:
                    memcpy(testtile,grabtilebuf[tx].data,tilesize(imported_format));
                    break;
                }
                break;
              case 3:  //original tile's bit depth
                newformat=imported_format;
                memcpy(testtile,grabtilebuf[tx].data,tilesize(imported_format));
                break;
            }
            break;
        case ftBMP:
          newformat=cdepth;
          for(int y=0; y<16; y++)                           //snag a tile
          {
            for(int x=0; x<16; x+=2)
            {
              switch(cdepth)
              {
                case tf4Bit:
                  testtile[(y*8)+(x/2)]=
                    (getpixel(((BITMAP*)imagebuf),(tx*16)+x,(ty*16)+y)&15)+
                    ((getpixel(((BITMAP*)imagebuf),(tx*16)+x+1,(ty*16)+y)&15)<<4);
                  break;
                case tf8Bit:
                  testtile[(y*16)+x]=getpixel(((BITMAP*)imagebuf),(tx*16)+x,(ty*16)+y);
                  testtile[(y*16)+x+1]=getpixel(((BITMAP*)imagebuf),(tx*16)+x+1,(ty*16)+y);
                  break;
              }
            }
          }
          break;
      }

      if (DuplicateAction[0]+DuplicateAction[1]+DuplicateAction[2]+DuplicateAction[3]>0)
      {
        temp_canadd=true;
        //check all tiles before this one
        for (int checktile=((OnlyCheckNewTilesForDuplicates!=0)?start:0); ((temp_canadd==true)&&(checktile<currtile)); checktile++)
        {
          for (int flipping=0; ((temp_canadd==true)&&(flipping<4)); ++flipping)
          {
            if (DuplicateAction[flipping]>0)
            {
              if (keypressed())
              {
                delete[] testtile;
                return true;
              }
              duplicate=(newformat==imported_format);
              if (duplicate)
              {
                switch (flipping)
                {
                  case 0:                                     //normal
                    if (dest[checktile].data!=NULL)
                    {
                      for(int y=0; ((duplicate==true)&&(y<16)); y++)
                      {
                        for(int x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
                        {
                          //                        if ((dest[(checktile*128)+(y*8)+(x/2)])!=(testtile[(y*8)+(x/2)]))
                          if ((dest[checktile].data[(y*8*newformat)+(x/(3-newformat))])!=(newformat==tf4Bit?(testtile[(y*8)+(x/2)]):(testtile[(y*16)+x])))
                          {
                            duplicate=false;
                          }
                        }
                      }
                    }
                    break;
                  case 1:                                     //horizontal
                    if (dest[checktile].data!=NULL)
                    {
                      for(int y=0; ((duplicate==true)&&(y<16)); y++)
                      {
                        for(int x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
                        {
                          //                        if ((dest[(checktile*128)+(y*8)+((14-x)/2)])!=(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)))
                          if ((dest[checktile].data[(y*8*newformat)+(14+(newformat-1)-x)/(3-newformat)])!=(newformat==tf4Bit?(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)):(testtile[(y*16)+x])))
                          {
                            duplicate=false;
                          }
                        }
                      }
                    }
                    break;
                  case 2:                                     //vertical
                    if (dest[checktile].data!=NULL)
                    {
                      for(int y=0; ((duplicate==true)&&(y<16)); y++)
                      {
                        for(int x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
                        {
                          //                      if ((dest[(checktile*128)+((15-y)*8)+(x/2)])!=(testtile[(y*8)+(x/2)]))
                          if ((dest[checktile].data[((15-y)*8*newformat)+(x/(3-newformat))])!=(newformat==tf4Bit?(testtile[(y*8)+(x/2)]):(testtile[(y*16)+x])))
                          {
                            duplicate=false;
                          }
                        }
                      }
                    }
                    break;
                  case 3:                                     //both
                    if (dest[checktile].data!=NULL)
                    {
                      for(int y=0; ((duplicate==true)&&(y<16)); y++)
                      {
                        for(int x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
                        {
                          //                      if ((dest[(checktile*128)+((15-y)*8)+((14-x)/2)])!=(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)))
                          if ((dest[checktile].data[((15-y)*8*newformat)+((14+(newformat-1)-x)/(3-newformat))])!=(newformat==tf4Bit?(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)):testtile[(y*16)+x]))
                          {
                            duplicate=false;
                          }
                        }
                      }
                    }
                    break;
                }
              }
              if (duplicate==true)
              {
                ++duplicates_found[flipping];
                ++total_duplicates_found;
                if (DuplicateAction[flipping]>1)
                {
                  ++total_duplicates_discarded;
                  temp_canadd=false;
                }
              }
            }
            canadd=canadd&&temp_canadd;
          }
        }
      }

//      dest[currtile].format=(cdepth==3?imported_format:cdepth);
      dest[currtile].format=newformat;
      if (dest[currtile].data!=NULL)
      {
        free(dest[currtile].data);
      }
      dest[currtile].data=(byte *)malloc(tilesize(dest[currtile].format));
      if (dest[currtile].data==NULL)
      {
        Z_error("Unable to initialize tile #%d.\n", currtile);
      }
      if (canadd==true)
      {
        /*
          for(int y=0; y<16; y++)
          {
          for(int x=0; x<8; x++)
          {
          dest[currtile].data[(y*8)+x]=testtile[(y*8)+x];
          }
          }
          */
        memcpy(dest[currtile].data, testtile, tilesize(dest[currtile].format));
        ++currtile;
      }
    }
  }
  destroy_bitmap(status);
  delete[] testtile;
  return true;
}

void grab(byte (*dest)[256],byte *def, int width, int height, int oformat, byte *newformat)
{
  byte format=tf4Bit;
  int stile = ((imagey*TILES_PER_ROW)+imagex)+(((sely/16)*TILES_PER_ROW)+(selx/16));
  switch(imagetype)
  {
    case ftZGP:
    case ftQST:
    case ftZQT:
    case ftQSU:
    case ftTIL:
    case ftBIN:
    case ftBMP:
    for (int ty=0; ty<height; ty++)
    {
      for (int tx=0; tx<width; tx++)
      {
        format=tf4Bit;
        switch (imagetype)
        {
          case ftZGP:
          case ftQST:
          case ftZQT:
          case ftQSU:
          case ftTIL:
            format=grabtilebuf[stile+((ty*TILES_PER_ROW)+tx)].format;
            break;
        }
        for(int y=0; y<16; y++)
        {
          for(int x=0; x<16; x+=2)
          {
            if(y<8 && x<8 && grabmask&1)
            {
			  switch(oformat)
			  {
			  case tf4Bit:
			      {
				    switch(format)
					{
					case tf4Bit:
                      dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)]; break;
					case tf8Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4; break;
					}
				  }
				  break;
			  case tf8Bit:
				  {
				    switch(format)
					{
					case tf4Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4); break;
					case tf8Bit:
			          dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
				      dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)]; break;
					}
				  }
				  break;
			  }
            }
            else if(y<8 && x>7 && grabmask&2)
            {
              switch(oformat)
			  {
			  case tf4Bit:
			      {
				    switch(format)
					{
					case tf4Bit:
                      dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)]; break;
					case tf8Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4; break;
					}
				  }
				  break;
			  case tf8Bit:
				  {
				    switch(format)
					{
					case tf4Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4); break;
					case tf8Bit:
			          dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
				      dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)]; break;
					}
				  }
				  break;
			  }
            }
            else if(y>7 && x<8 && grabmask&4)
            {
              switch(oformat)
			  {
			  case tf4Bit:
			      {
				    switch(format)
					{
					case tf4Bit:
                      dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)]; break;
					case tf8Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4; break;
					}
				  }
				  break;
			  case tf8Bit:
				  {
				    switch(format)
					{
					case tf4Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4); break;
					case tf8Bit:
			          dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
				      dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)]; break;
					}
				  }
				  break;
			  }
            }
            else if(y>7 && x>7 && grabmask&8)
            {
              switch(oformat)
			  {
			  case tf4Bit:
			      {
				    switch(format)
					{
					case tf4Bit:
                      dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)]; break;
					case tf8Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
					  dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4; break;
					}
				  }
				  break;
			  case tf8Bit:
				  {
				    switch(format)
					{
					case tf4Bit:
					  dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4); break;
					case tf8Bit:
			          dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
				      dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)]; break;
					}
				  }
				  break;
			  }
            }
            else
            {
              switch (format)
              {
                case tf8Bit:
                  dest[(ty*TILES_PER_ROW)+tx][(y*16)+x]=getpixel(screen2,(tx*16)+x+selx,(ty*16)+y+sely);
                  dest[(ty*TILES_PER_ROW)+tx][(y*16)+x+1]=getpixel(screen2,(tx*16)+x+1+selx,(ty*16)+y+sely);
				  newformat[(ty*TILES_PER_ROW)+tx] = tf8Bit;
                  //al_trace("Tile %d is 8-bit\n", (ty*TILES_PER_ROW)+tx);
                  break;
                case tf4Bit:
                default:
                  dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(getpixel(screen2,(tx*16)+x+selx,(ty*16)+y+sely)&15)+((getpixel(screen2,(tx*16)+x+1+selx,(ty*16)+y+sely)&15)<<4);
				  newformat[(ty*TILES_PER_ROW)+tx] = tf4Bit;
                  break;
              }
            }
          }
        }
      }
    }
    break;

    default:
    for(int i=0; i<200; i++)
	{
	  for(int j=0; j<256; j++)
	  {
        dest[i][j]=0;
	  }
	  newformat[i] = tf4Bit;
	}
    break;
  }
}

void grab_tile(int tile,int &cs)
{
  byte newtile[200][256];
  byte newformat[200];

  static EXT_LIST list[] =
  {
    { "All Files (*.*)", NULL },
    { "Bitmap Image (*.bmp)", "bmp" },
    { "GIF Image (*.gif)", "gif" },
    { "JPEG Image (*.jpg, *.jpeg)", "jpg,jpeg" },
    { "ZC Tile Export (*.til)", "til" },
    { "ZC Quest Template (*.zqt)", "zqt" },
    { "ZC Quest (*.qst)", "qst" },
    { "ZC Graphics Pack (*.zgp)", "zgp" },
    { "ZC Unencoded Quest (*.qsu)", "qsu" },
    { "NES ROM Image (*.nes)", "nes" },
    { "SNES ROM Image (*.smc)", "smc" },
    { "Gameboy ROM Image (*.gb)", "gb" },
    { "Gameboy Advanced ROM Image (*.gba)", "gba" },
    { NULL, NULL }
  };


  memset(cset_reduce_table, 0, 256);
  memset(col_diff,0,3*128);
  bool bdown=false;
  int done=0;
  int pal=0;
  int f=0;
  int black=vc(0),white=vc(15);
  int selwidth=1, selheight=1;
  int selx2, sely2;
  bool xreversed=false, yreversed=false;
  bool doleech=false, dofile=false, dopal=false;

  int jwin_pal2[jcMAX];
  memcpy(jwin_pal2, jwin_pal, sizeof(int)*jcMAX);


  if(imagebuf==NULL)
    load_imagebuf();

  calc_cset_reduce_table(imagepal, cs);
  draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
  grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
  draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
  while(gui_mouse_b());

  do
  {
    rest(4);
    bool redraw=false;

    if(keypressed())
    {
      redraw=true;
      switch(readkey()>>8)
      {
        case KEY_F: dofile=true; break;
        case KEY_L: doleech=true; break;
        case KEY_P: if(imagetype==ftBMP) {dopal=true;} break;
        case KEY_ESC:   done=1; break;
        case KEY_ENTER_PAD:
        case KEY_ENTER: done=2; break;
        case KEY_DOWN:  if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) sely=min(sely+1,144); else ++imagey; break;
        case KEY_UP:    if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) sely=max(sely-1,0); else --imagey; break;
        case KEY_RIGHT: if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) selx=min(selx+1,304); else ++imagex; break;
        case KEY_LEFT:  if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) selx=max(selx-1,0); else --imagex; break;
        case KEY_PGDN:  imagey+=10; break;
        case KEY_PGUP:  imagey-=10; break;
        case KEY_HOME:  imagex=imagey=0; break;
        case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  calc_cset_reduce_table(imagepal, cs); break;
        case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; calc_cset_reduce_table(imagepal, cs); break;
        case KEY_S: if(grabmode==1) grabmode=8; else if(grabmode==8) grabmode=16; else grabmode=1; break;
        case KEY_1: imagex=(imagex*bp)>>3; bp=1; imagex<<=3; nesmode=false; break;
        case KEY_2: imagex=(imagex*bp)>>3; bp=2; imagex<<=2; nesmode=false; break;
        case KEY_N: imagex=(imagex*bp)>>3; bp=2; imagex<<=2; nesmode=true; break;
        case KEY_4: imagex=(imagex*bp)>>3; bp=4; imagex<<=1; nesmode=false; break;
        case KEY_8: imagex=(imagex*bp)>>3; bp=8; break;
        case KEY_B: if(bp==2&&!nesmode) {nesmode=true;} else {nesmode=false; bp<<=1; if (bp==16) {bp=1; imagex<<=3;} else {imagex>>=1;}}  break;
        case KEY_M: romtilemode=(romtilemode+1)%4; break;
        case KEY_Z: if(romofs>0) --romofs; break;
        case KEY_X: ++romofs; break;
        case KEY_R: reduce=!reduce; calc_cset_reduce_table(imagepal, cs); break;
        default: redraw=false;
      }
      clear_keybuf();

      if(imagex<0) imagex=0;
      if(imagey<0) imagey=0;
      draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
      grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
    }

    //boogie!
    if(gui_mouse_b()==1 && !bdown)
    {
      bool regrab=false;

      bdown=true;
      if(gui_mouse_y()<160)
      {
        while(gui_mouse_b())
        {
          if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
          {
            selx=min((gui_mouse_x()/grabmode)*grabmode,304);
            sely=min((gui_mouse_y()/grabmode)*grabmode,144);
            selx2=selx;
            sely2=sely;
            selwidth=1;
            selheight=1;
            xreversed=false;
            yreversed=false;
          }
          else
          {
            if (xreversed)
            {
              swap(selx, selx2);
              xreversed=false;
            }
            if (yreversed)
            {
              swap(sely, sely2);
              yreversed=false;
            }
            selx2=min((gui_mouse_x()/grabmode)*grabmode,304);
            sely2=min((gui_mouse_y()/grabmode)*grabmode,144);
            selwidth=1+(abs(selx2-selx))/16;
            selheight=1+(abs(sely2-sely))/16;
            if (selx2<selx)
            {
              swap(selx, selx2);
              xreversed=true;
            }
            if (sely2<sely)
            {
              swap(sely, sely2);
              yreversed=true;
            }
          }
          //         grab(newtile,tilebuf+(tile<<7), 1, 1);
          grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
          draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
        }
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),260,168,321,189))
      {
        if(do_button(255,168,61,20,"OK",vc(1),vc(14),true))
          done=2;
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),120,168,181,189))
      {
        if(do_button(117,166,61,20,"Leech",vc(1),vc(14),true))
        {
          doleech=true;
        }
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),260,192,321,213))
      {
        if(do_button(255,192,61,20,"Cancel",vc(1),vc(14),true))
          done=1;
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),260,216,321,237))
      {
        if(do_button(255,216,61,20,"File",vc(1),vc(14),true))
        {
          dofile=true;
        }
      }
      else if(imagetype==ftBMP && isinRect(gui_mouse_x(),gui_mouse_y(),140,192,201,213))
      {
        if(do_button(141,194,61,20,"Pal",vc(1),vc(14),true))
        {
          dopal=true;
        }
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),184,168,191,175))
      {
        regrab=true; grabmask^=1;
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),192,168,199,175))
      {
        regrab=true; grabmask^=2;
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),184,176,191,183))
      {
        regrab=true; grabmask^=4;
      }
      else if(isinRect(gui_mouse_x(),gui_mouse_y(),192,176,199,183))
      {
        regrab=true; grabmask^=8;
      }

      if(regrab)
      {
        //       grab(newtile,tilebuf+(tile<<7), 1, 1);
        grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
        redraw=true;
      }
    }

    if(gui_mouse_b()==0)
      bdown=false;

    if(dofile)
    {

      if(getname_nogo("Load File",NULL,list,imagepath,true))
      {
        set_palette(RAMpal);
        pal=0;
        white=vc(15);
        black=vc(0);
        strcpy(imagepath,temppath);
        load_imagebuf();
        draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
        //           grab(newtile,tilebuf+(tile<<7), 1, 1);
        grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
      }
      while (key[KEY_ESC]);
      clear_keybuf();
      dofile=false;
      redraw=true;
    }
    if(doleech)
    {
      if (leech_tiles(newtilebuf,tile,cs))
      {
        done=1;
      }
      else
      {
        while (key[KEY_ESC]);
        clear_keybuf();
        redraw=true;
      }
      doleech=false;
    }
    if(dopal)
    {
      pal^=1;
      if(pal)
      {
        get_bw(imagepal,black,white);

        jwin_pal[jcBOX]    =imagepal_table.data[0][jwin_pal[jcBOX]];
        jwin_pal[jcLIGHT]  =imagepal_table.data[0][jwin_pal[jcLIGHT]];
        jwin_pal[jcMEDLT]  =imagepal_table.data[0][jwin_pal[jcMEDLT]];
        jwin_pal[jcMEDDARK]=imagepal_table.data[0][jwin_pal[jcMEDDARK]];
        jwin_pal[jcDARK]   =imagepal_table.data[0][jwin_pal[jcDARK]];
        jwin_pal[jcBOXFG]  =imagepal_table.data[0][jwin_pal[jcBOXFG]];
        jwin_pal[jcTITLEL] =imagepal_table.data[0][jwin_pal[jcTITLEL]];
        jwin_pal[jcTITLER] =imagepal_table.data[0][jwin_pal[jcTITLER]];
        jwin_pal[jcTITLEFG]=imagepal_table.data[0][jwin_pal[jcTITLEFG]];
        jwin_pal[jcTEXTBG] =imagepal_table.data[0][jwin_pal[jcTEXTBG]];
        jwin_pal[jcTEXTFG] =imagepal_table.data[0][jwin_pal[jcTEXTFG]];
        jwin_pal[jcSELBG]  =imagepal_table.data[0][jwin_pal[jcSELBG]];
        jwin_pal[jcSELFG]  =imagepal_table.data[0][jwin_pal[jcSELFG]];
        gui_bg_color=jwin_pal[jcBOX];
        gui_fg_color=jwin_pal[jcBOXFG];
        gui_mg_color=jwin_pal[jcMEDDARK];
        jwin_set_colors(jwin_pal);
      }
      else
      {
        white=vc(15);
        black=vc(0);

        memcpy(jwin_pal, jwin_pal2, sizeof(int)*jcMAX);
        gui_bg_color=jwin_pal[jcBOX];
        gui_fg_color=jwin_pal[jcBOXFG];
        gui_mg_color=jwin_pal[jcMEDDARK];
        jwin_set_colors(jwin_pal);
      }
      set_palette_range(pal?imagepal:RAMpal,0,255,false);

      dopal=false;
      redraw=true;
    }

    if(redraw)
    {
      draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
    }
    else
    {
      vsync();
    }

    scare_mouse();
    blit(screen2,screen,selx,sely,selx,sely,selwidth*16,selheight*16);
    if(f&8)
    {
      /*
        for (int x=0; x<selwidth; x++) {
        for (int y=0; y<selheight; y++) {
        rect(screen,selx,sely,selx+15,sely+15,white);
        }
        }
        */
      //     rect(screen,selx,sely,selx+15,sely+15,white);
      rect(screen,selx,sely,selx+((selwidth-1)*16)+15,sely+((selheight-1)*16)+15,white);
    }
    unscare_mouse();
    //    SCRFIX();
    ++f;

  } while(!done);

  memcpy(jwin_pal, jwin_pal2, sizeof(int)*jcMAX);
  gui_bg_color=jwin_pal[jcBOX];
  gui_fg_color=jwin_pal[jcBOXFG];
  gui_mg_color=jwin_pal[jcMEDDARK];
  jwin_set_colors(jwin_pal);


  if(done==2)
  {
    go_tiles();
    saved=false;
    //   usetiles=true;
    for (int y=0; y<selheight; y++)
    {
      for (int x=0; x<selwidth; x++)
      {
        word temptile=tile+((TILES_PER_ROW*y)+x);
        newtilebuf[temptile].format=newformat[(TILES_PER_ROW*y)+x];
        if (newtilebuf[temptile].data!=NULL)
        {
          free(newtilebuf[temptile].data);
        }
        newtilebuf[temptile].data=(byte *)malloc(tilesize(newtilebuf[temptile].format));
        if (newtilebuf[temptile].data==NULL)
        {
          Z_error("Unable to initialize tile #%d.\n", temptile);
        }
        for(int i=0; i<128*newtilebuf[temptile].format; i++)
        {
          newtilebuf[temptile].data[i] = newtile[(TILES_PER_ROW*y)+x][i];
        }
      }
    }
  }

  if(pal)
    set_palette(RAMpal);
}

void draw_tiles(int first,int cs)
{
  clear_bitmap(screen2);
  for(int i=0; i<TILES_PER_PAGE; i++)                       // 13 rows, leaving 32 pixels from y=208 to y=239
    puttile16(screen2,first+i,(i%TILES_PER_ROW)<<4,(i/TILES_PER_ROW)<<4,cs,0);

}

void tile_info_0(int tile,int tile2,int cs,int copy,int copycnt,int page,bool rect_sel)
{
  jwin_draw_win(screen2,0, 208, 320, 32, FR_WIN);
  jwin_draw_frame(screen2,34,214,20,20,FR_DEEP);
  int coldiff=tile_col(copy)-tile_col(copy+copycnt-1);


  if(copy>=0)
  {
    puttile16(screen2,rect_sel&&coldiff>0?copy-coldiff:copy,36,216,cs,0);
    if(copycnt>1)
    {
      textprintf_right_ex(screen2,pfont,30,216,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d-",copy);
      textprintf_right_ex(screen2,pfont,26,224,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy+copycnt-1);
    }
    else
    {
      textprintf_right_ex(screen2,pfont,26,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy);
    }
  }
  else
  {
    for (int dy=0; dy<16; dy++)
    {
      for (int dx=0; dx<16; dx++)
      {
        screen2->line[216+dy][36+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
      }
    }
  }


  //current tile
  jwin_draw_frame(screen2,106,214,20,20,FR_DEEP);
  puttile16(screen2,tile,108,216,cs,0);

  jwin_draw_frame(screen2,130,214,20,20,FR_DEEP);
  blit(select_bmp[rect_sel?1:0],screen2,0,0,132,216,16,16);
  if(tile>tile2)
  {
    swap(tile,tile2);
  }
  char buf[8];
  buf[0]=0;
  if(tile2!=tile)
  {
    sprintf(buf,"-%d",tile2);
  }

  textprintf_ex(screen2,pfont,58,216,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"cs: %d",cs);
  textprintf_right_ex(screen2,pfont,102,216,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"tile:");
  textprintf_right_ex(screen2,pfont,102,224,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d%s",tile,buf);

  draw_button(screen2,157,213,44,21,"Grab",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
  draw_button(screen2,202,213,44,21,"Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
  draw_button(screen2,247,213,44,21,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);

  textprintf_ex(screen2,font,305,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"\x88");
  textprintf_ex(screen2,pfont,293,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"p:");
  textprintf_centre_ex(screen2,pfont,309,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",page);
  textprintf_ex(screen2,font,305,228,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"\x89");

  vsync();
  scare_mouse();
  blit(screen2,screen,0,0,0,0,320,240);
  unscare_mouse();
  SCRFIX();
}

void tile_info_1(int oldtile,int oldflip,int oldcs,int tile,int flip,int cs,int copy,int page)
{
  jwin_draw_win(screen2,0, 208, 320, 32, FR_WIN);
  jwin_draw_frame(screen2,122,214,20,20,FR_DEEP);
  if(copy>=0)
  {
    puttile16(screen2,copy,124,216,cs,flip);
  }
  else
  {
    for (int dy=0; dy<16; dy++)
    {
      for (int dx=0; dx<16; dx++)
      {
        screen2->line[216+dy][124+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
      }
    }
  }
  jwin_draw_frame(screen2,6,214,20,20,FR_DEEP);
  puttile16(screen2,oldtile,8,216,oldcs,oldflip);
  textprintf_right_ex(screen2,pfont,56,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Old Tile:");
  textprintf_ex(screen2,pfont,60,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",oldtile);
  textprintf_right_ex(screen2,pfont,56,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet:");
  textprintf_ex(screen2,pfont,60,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",oldcs);
  textprintf_right_ex(screen2,pfont,56,228,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Flip:");
  textprintf_ex(screen2,pfont,60,228,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",oldflip);

  jwin_draw_frame(screen2,146,214,20,20,FR_DEEP);
  puttile16(screen2,tile,148,216,cs,flip);
  textprintf_right_ex(screen2,pfont,201,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"New Tile:");
  textprintf_ex(screen2,pfont,205,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",tile);
  textprintf_right_ex(screen2,pfont,201,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet:");
  textprintf_ex(screen2,pfont,205,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",cs);
  textprintf_right_ex(screen2,pfont,201,228,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Flip:");
  textprintf_ex(screen2,pfont,205,228,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",flip);

  textprintf_ex(screen2,font,305,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"\x88");
  textprintf_ex(screen2,pfont,293,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"p:");
  textprintf_centre_ex(screen2,pfont,309,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",page);
  textprintf_ex(screen2,font,305,228,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"\x89");

  vsync();
  scare_mouse();
  blit(screen2,screen,0,0,0,0,320,240);
  unscare_mouse();
  SCRFIX();
}

void reset_tile(tiledata *buf, int t, int format=1)
{
  buf[t].format=format;
  if (buf[t].data!=NULL)
  {
    free(buf[t].data);
  }
  buf[t].data=(byte *)malloc(tilesize(buf[t].format));
  if (buf[t].data==NULL)
  {
    Z_error("Unable to initialize tile #%d.\n", t);
  }
  for(int i=0; i<tilesize(buf[t].format); i++)
  {
    buf[t].data[i]=0;
  }
}

static MENU select_tile_rc_menu[] =
{
  { "Copy",    NULL,  NULL, 0, NULL },
  { "Paste",   NULL,  NULL, 0, NULL },
  { "Delete",  NULL,  NULL, 0, NULL },
  { "",        NULL,  NULL, 0, NULL },
  { "Edit",    NULL,  NULL, 0, NULL },
  { "Grab",    NULL,  NULL, 0, NULL },
  { NULL,      NULL,  NULL, 0, NULL }
};

//returns the column the tile is in
int tile_col(int tile)
{
  return (tile%TILES_PER_ROW);
}

//returns the row the tile is in
int tile_row(int tile)
{
  return (tile/TILES_PER_ROW);
}

//returns the page the tile is on
int tile_page(int tile)
{
  return (tile/TILES_PER_PAGE);
}

//returns the row the tile is in on its page
int tile_page_row(int tile)
{
  return tile_row(tile)-(tile_page(tile)*TILE_ROWS_PER_PAGE);
}

enum {ti_none, ti_encompass, ti_broken};

//striped check and striped selection
int move_intersection_ss(int check_first, int check_last, int selection_first, int selection_last)
{
  // if selection is before or after check...
  if ((check_first>selection_last)||(selection_first>check_last))
  {
    return ti_none;
  }
  // if selection envelopes check
  if ((selection_first<=check_first)&&(selection_last>=check_last))
  {
    return ti_encompass;  //encompass
  }
  //everything else is a break
  return ti_broken;  //intersect
}



//rectangular check and striped selection
int move_intersection_rs(int check_left, int check_top, int check_width, int check_height, int selection_first, int selection_last)
{
  int ret1=-1, ret2=-1;
  for (int i=0; i<check_height; ++i)
  {
    int check_first=((check_top+i)*TILES_PER_ROW)+check_left;
    int check_last=check_first+check_width-1;
    ret2=move_intersection_ss(check_first, check_last, selection_first, selection_last);
    if (ret2==ti_broken)
    {
      return ti_broken;
    }
    ret1=(ret2==ti_encompass?ti_encompass:ret1);
  }
  if (ret1==ti_encompass)
  {
    if ((tile_row(selection_first)<=check_top) &&
        (tile_row(selection_last)>=(check_top+check_height-1)))
    {
      return ti_encompass;
    }
    else
    {
      return ti_broken;
    }
  }
  return ti_none;
}


//striped check and rectangular selection
int move_intersection_sr(int check_first, int check_last, int selection_left, int selection_top, int selection_width, int selection_height)
{
  if (selection_width < TILES_PER_ROW)
  {
    if ((check_last-check_first+1<=selection_width) &&
        (tile_row(check_first)>=selection_top) &&
        (tile_row(check_last)<=selection_top+selection_height-1) &&
        (tile_col(check_first)>=selection_left) &&
        (tile_col(check_last)<=tile_col(selection_left+selection_width-1)))
    {
      return ti_encompass;
    }
    else if ((check_last<selection_top*TILES_PER_ROW+selection_left) ||
             (check_first>(selection_top+selection_height-1)*TILES_PER_ROW+selection_left+selection_width-1))
      {
        return ti_none;
      }
      //else if (selection_top*TILES_PER_ROW+selection_left<check_first && (selection_top+1)*TILES_PER_ROW+selection_left>check_last)

	  //one last base case: the strip we're interested in only lies along one row
	  if(check_first/TILES_PER_ROW == check_last/TILES_PER_ROW)
	  {
		  int cfcol = check_first%TILES_PER_ROW;
		  int clcol = check_last%TILES_PER_ROW;
		  if(clcol < selection_left || cfcol >= selection_left+selection_width)
			  return ti_none;
		  else
			  return ti_broken;
	  }
	  else
	  {
		  //recursively cut the strip into substrips which lie entirely on one row
		  int currow = check_first/TILES_PER_ROW;
		  int endrow = check_last/TILES_PER_ROW;
		  int accum = 0;
		  accum |= move_intersection_sr(check_first,(currow+1)*TILES_PER_ROW-1,selection_left,selection_top,selection_width,selection_height);
		  for(++currow;currow<endrow; currow++)
		  {
			accum |= move_intersection_sr(currow*TILES_PER_ROW,(currow+1)*TILES_PER_ROW-1,selection_left,selection_top,selection_width,selection_height);
		  }
		  accum |= move_intersection_sr(currow*TILES_PER_ROW, check_last,selection_left,selection_top,selection_width,selection_height);
		  if(accum > 0)
			  return ti_broken;
		  return ti_none;
	  }
  }
  return move_intersection_ss(check_first, check_last, selection_top*TILES_PER_ROW+selection_left, (selection_top+selection_height-1)*TILES_PER_ROW+selection_left+selection_width-1);
}

//rectangular check and rectangular selection
int move_intersection_rr(int check_left, int check_top, int check_width, int check_height, int selection_left, int selection_top, int selection_width, int selection_height)
{
  if ((check_left>=selection_left) &&
      (check_left+check_width<=selection_left+selection_width) &&
      (check_top>=selection_top) &&
      (check_top+check_height<=selection_top+selection_height))
  {
    return ti_encompass;
  }
  else
  {
    for (int i=check_top; i<check_top+check_height; ++i)
    {
      if (move_intersection_rs(selection_left, selection_top, selection_width, selection_height, i*TILES_PER_ROW+check_left, i*TILES_PER_ROW+check_left+check_width-1)!=ti_none)
      {
        return ti_broken;
      }
    }
  }
  return ti_none;
}




static DIALOG tile_move_list_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,      0,   0,   254,  178,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,      NULL, NULL, NULL },
  { jwin_ctext_proc,   127,  24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "", NULL, NULL },
  { jwin_ctext_proc,   127,  34,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "", NULL, NULL },
  { jwin_ctext_proc,   127,  44,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "", NULL, NULL },
  { jwin_textbox_proc,  12,   54,   231,  96,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,      NULL, NULL, NULL },
  { jwin_button_proc,   57,   153,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { jwin_button_proc,  137,   153,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


typedef struct move_tiles_item
{
  char *name;
  int tile;
  int width;
  int height;
} move_tiles_item;

move_tiles_item map_styles_items[6]=
{
  { "Frame",                  0,  2,  2 },
  { "Heart Container Piece",  0,  1,  1 },
  { "Triforce Fragment",      0, -1, -1 },
  { "Triforce Frame",         0, -1, -1 },
  { "Overworld Map",          0,  5,  3 },
  { "Dungeon Map",            0,  5,  3 },
};

move_tiles_item dmap_map_items[4]=
{
  { "Minimap (Empty)",        0,  5,  3 },
  { "Minimap (Filled)",       0,  5,  3 },
  { "Large Map (Empty)",      0, -1,  5 },
  { "Large Map (Filled)",     0, -1,  5 },
};

move_tiles_item link_sprite_items[33]=
{
  { "Walk (Up)",                0,  0,  0 },
  { "Walk (Down)",              0,  0,  0 },
  { "Walk (Left)",              0,  0,  0 },
  { "Walk (Right)",             0,  0,  0 },
  { "Slash (Up)",               0,  0,  0 },
  { "Slash (Down)",             0,  0,  0 },
  { "Slash (Left)",             0,  0,  0 },
  { "Slash (Right)",            0,  0,  0 },
  { "Stab (Up)",                0,  0,  0 },
  { "Stab (Down)",              0,  0,  0 },
  { "Stab (Left)",              0,  0,  0 },
  { "Stab (Right)",             0,  0,  0 },
  { "Pound (Up)",               0,  0,  0 },
  { "Pound (Down)",             0,  0,  0 },
  { "Pound (Left)",             0,  0,  0 },
  { "Pound (Right)",            0,  0,  0 },
  { "Hold (Land, One Hand)",    0,  0,  0 },
  { "Hold (Land, Two Hands)",   0,  0,  0 },
  { "Cast",                     0,  0,  0 },
  { "Float (Up)",               0,  0,  0 },
  { "Float (Down)",             0,  0,  0 },
  { "Float (Left)",             0,  0,  0 },
  { "Float (Right)",            0,  0,  0 },
  { "Swim (Up)",                0,  0,  0 },
  { "Swim (Down)",              0,  0,  0 },
  { "Swim (Left)",              0,  0,  0 },
  { "Swim (Right)",             0,  0,  0 },
  { "Dive (Up)",                0,  0,  0 },
  { "Dive (Down)",              0,  0,  0 },
  { "Dive (Left)",              0,  0,  0 },
  { "Dive (Right)",             0,  0,  0 },
  { "Hold (Water, One Hand)",   0,  0,  0 },
  { "Hold (Water, Two Hands)",  0,  0,  0 },
};

int quick_select_3(int a, int b, int c, int d)
{
  return a==0?b:a==1?c:d;
}

void setup_link_sprite_items()
{
  int a_style=(zinit.linkanimationstyle);

  for (int i=0; i<4; ++i)
  {
    link_sprite_items[i].tile=walkspr[i][spr_tile]-(walkspr[i][spr_extend]<2?0:1)-(walkspr[i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[i].width=(walkspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, (i==0?1:2), 3, 9) + (walkspr[i][spr_extend]<2?0:1);
    link_sprite_items[i].height=walkspr[i][spr_extend]<2?1:2;
  }

  for (int i=0; i<4; ++i)
  {
    link_sprite_items[4+i].tile=slashspr[i][spr_tile]-(slashspr[i][spr_extend]<2?0:1)-(slashspr[i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[4+i].width=(slashspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 6) + (slashspr[i][spr_extend]<2?0:1);;
    link_sprite_items[4+i].height=slashspr[i][spr_extend]<2?1:2;
  }

  for (int i=0; i<4; ++i)
  {
    link_sprite_items[8+i].tile=stabspr[i][spr_tile]-(stabspr[i][spr_extend]<2?0:1)-(stabspr[i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[8+i].width=(stabspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 3) + (stabspr[i][spr_extend]<2?0:1);;
    link_sprite_items[8+i].height=stabspr[i][spr_extend]<2?1:2;
  }

  for (int i=0; i<4; ++i)
  {
    link_sprite_items[12+i].tile=poundspr[i][spr_tile]-(poundspr[i][spr_extend]<2?0:1)-(poundspr[i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[12+i].width=(poundspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 3) + (poundspr[i][spr_extend]<2?0:1);;
    link_sprite_items[12+i].height=poundspr[i][spr_extend]<2?1:2;
  }

  for (int i=0; i<2; ++i)
  {
    link_sprite_items[16+i].tile=holdspr[0][i][spr_tile]-(holdspr[0][i][spr_extend]<2?0:1)-(holdspr[0][i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[16+i].width=(holdspr[0][i][spr_extend]<2?1:2) + (holdspr[0][i][spr_extend]<2?0:1);;
    link_sprite_items[16+i].height=holdspr[0][i][spr_extend]<2?1:2;
  }

  link_sprite_items[18].tile=castingspr[spr_tile]-(castingspr[spr_extend]<2?0:1)-(castingspr[spr_extend]<1?0:TILES_PER_ROW);
  link_sprite_items[18].width=(castingspr[spr_extend]<2?1:2) + (castingspr[spr_extend]<2?0:1);;
  link_sprite_items[18].height=castingspr[spr_extend]<2?1:2;

  for (int i=0; i<4; ++i)
  {
    link_sprite_items[19+i].tile=floatspr[i][spr_tile]-(floatspr[i][spr_extend]<2?0:1)-(floatspr[i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[19+i].width=(floatspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 4) + (floatspr[i][spr_extend]<2?0:1);;
    link_sprite_items[19+i].height=floatspr[i][spr_extend]<2?1:2;
  }

  for (int i=0; i<4; ++i)
  {
    link_sprite_items[23+i].tile=swimspr[i][spr_tile]-(swimspr[i][spr_extend]<2?0:1)-(swimspr[i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[23+i].width=(swimspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 4) + (swimspr[i][spr_extend]<2?0:1);;
    link_sprite_items[23+i].height=swimspr[i][spr_extend]<2?1:2;
  }

  for (int i=0; i<4; ++i)
  {
    link_sprite_items[27+i].tile=divespr[i][spr_tile]-(divespr[i][spr_extend]<2?0:1)-(divespr[i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[27+i].width=(divespr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 4) + (divespr[i][spr_extend]<2?0:1);;
    link_sprite_items[27+i].height=divespr[i][spr_extend]<2?1:2;
  }

  for (int i=0; i<2; ++i)
  {
    link_sprite_items[31+i].tile=holdspr[1][i][spr_tile]-(holdspr[1][i][spr_extend]<2?0:1)-(holdspr[1][i][spr_extend]<1?0:TILES_PER_ROW);
    link_sprite_items[31+i].width=(holdspr[1][i][spr_extend]<2?1:2) + (holdspr[1][i][spr_extend]<2?0:1);;
    link_sprite_items[31+i].height=holdspr[1][i][spr_extend]<2?1:2;
  }
}


bool copy_tiles_united(int &tile,int &tile2,int &copy,int &copycnt, bool rect, bool move)
{
  bool alt=(key[KEY_ALT]||key[KEY_ALTGR]);
  bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);

  // if tile>tile2 then swap them
  if (tile>tile2)
  {
    swap(tile, tile2);
  }

  // alt=copy from right
  // shift=copy from bottom

  int copies=copycnt;
  int dest_first=tile;
  int dest_last=tile2;
  int src_first=copy;
  int src_last=copy+copies-1;

  int dest_top=0;
  int dest_bottom=0;
  int src_top=0;
  int src_bottom=0;
  int src_left=0, src_right=0;
  int src_width=0, src_height=0;
  int dest_left=0, dest_right=0;
  int dest_width=0, dest_height=0;
  int rows=0, cols=0;

  if (rect)
  {
    dest_top=tile_row(dest_first);
    dest_bottom=tile_row(dest_last);
    src_top=tile_row(src_first);
    src_bottom=tile_row(src_last);

    src_left= min(tile_col(src_first),tile_col(src_last));
    src_right=max(tile_col(src_first),tile_col(src_last));
    src_first=(src_top  * TILES_PER_ROW)+src_left;
    src_last= (src_bottom*TILES_PER_ROW)+src_right;

    dest_left= min(tile_col(dest_first),tile_col(dest_last));
    dest_right=max(tile_col(dest_first),tile_col(dest_last));
    dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
    dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;

    //if no dest range set, then set one
    if ((dest_first==dest_last)&&(src_first!=src_last))
    {
      if (alt)
      {
        dest_left=dest_right-(src_right-src_left);
      }
      else
      {
        dest_right=dest_left+(src_right-src_left);
      }

      if (shift)
      {
        dest_top=dest_bottom-(src_bottom-src_top);
      }
      else
      {
        dest_bottom=dest_top+(src_bottom-src_top);
      }

      dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
      dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
    }
    else
    {
      if(dest_right-dest_left<src_right-src_left) //destination is shorter than source
      {
        if(alt) //copy from right tile instead of left
        {
          src_left=src_right-(dest_right-dest_left);
        }
        else //copy from left tile
        {
          src_right=src_left+(dest_right-dest_left);
        }
      }
      else if (dest_right-dest_left>src_right-src_left) //destination is longer than source
        {
          if(alt) //copy from right tile instead of left
          {
            dest_left=dest_right-(src_right-src_left);
          }
          else //copy from left tile
          {
            dest_right=dest_left+(src_right-src_left);
          }
        }

        if(dest_bottom-dest_top<src_bottom-src_top) //destination is shorter than source
      {
        if(shift) //copy from bottom tile instead of top
        {
          src_top=src_bottom-(dest_bottom-dest_top);
        }
        else //copy from top tile
        {
          src_bottom=src_top+(dest_bottom-dest_top);
        }
      }
      else if (dest_bottom-dest_top>src_bottom-src_top) //destination is longer than source
        {
          if(shift) //copy from bottom tile instead of top
          {
            dest_top=dest_bottom-(src_bottom-src_top);
          }
          else //copy from top tile
          {
            dest_bottom=dest_top+(src_bottom-src_top);
          }
        }

        src_first=(src_top  * TILES_PER_ROW)+src_left;
      src_last= (src_bottom*TILES_PER_ROW)+src_right;
      dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
      dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
    }

    cols=src_right-src_left+1;
    rows=src_bottom-src_top+1;

    dest_width=dest_right-dest_left+1;
    dest_height=dest_bottom-dest_top+1;
    src_width=src_right-src_left+1;
    src_height=src_bottom-src_top+1;

  }
  else  //!rect
  {
    //if no dest range set, then set one
    if ((dest_first==dest_last)&&(src_first!=src_last))
    {
      if (alt)
      {
        dest_first=dest_last-(src_last-src_first);
      }
      else
      {
        dest_last=dest_first+(src_last-src_first);
      }
    }
    else
    {
      if(dest_last-dest_first<src_last-src_first) //destination is shorter than source
      {
        if(alt) //copy from last tile instead of first
        {
          src_first=src_last-(dest_last-dest_first);
        }
        else //copy from first tile
        {
          src_last=src_first+(dest_last-dest_first);
        }
      }
      else if (dest_last-dest_first>src_last-src_first) //destination is longer than source
        {
          if(alt) //copy from last tile instead of first
          {
            dest_first=dest_last-(src_last-src_first);
          }
          else //copy from first tile
          {
            dest_last=dest_first+(src_last-src_first);
          }
        }
    }

    copies=dest_last-dest_first+1;
  }



  char buf[80], buf2[80], buf3[80], buf4[80];
  sprintf(buf, " ");
  sprintf(buf2, " ");
  sprintf(buf3, " ");
  sprintf(buf4, " ");

  // warn if range extends beyond last tile
  sprintf(buf4, "Some tiles will not be %s", move?"moved.":"copied.");
  if (dest_last>=(TILES_PER_PAGE*TILE_PAGES))
  {
    if (jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", "&Cancel", 'o', 'c', lfont)==2)
    {
      return false;
    }
  }

  char tile_move_list_text[65535];
  char temptext[80];

  sprintf(buf, "Destination Error");
  tile_move_list_dlg[0].dp=buf;
  tile_move_list_dlg[0].dp2=lfont;
  bool found;
  bool flood;

  int i;
  bool move_combo_list[MAXCOMBOS];
  bool move_items_list[iMax];
  bool move_weapons_list[wMAX];
  bool move_link_sprites_list[33];
  bool move_mapstyles_list[6];
  bool move_game_icons_list[4];
  bool move_dmap_maps_list[MAXDMAPS][4];
  //    bool move_enemies_list[eMAXGUYS];  //to be implemented once custom enemies are in

  // warn if paste overwrites other defined tiles or
  // if delete erases other defined tiles
  int selection_first=0, selection_last=0, selection_left=0, selection_top=0, selection_width=0, selection_height=0;

  for (int q=0; q<2; ++q)
  {

    switch (q)
    {
      case 0:
        selection_first=dest_first;
        selection_last=dest_last;
        selection_left=dest_left;
        selection_top=dest_top;
        selection_width=dest_width;
        selection_height=dest_height;
        break;
      case 1:
        selection_first=src_first;
        selection_last=src_last;
        selection_left=src_left;
        selection_top=src_top;
        selection_width=src_width;
        selection_height=src_height;
        break;
    }

    if (move||q==0)
    {
      //check combos
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        for(int u=0; u<MAXCOMBOS; u++)
        {
          move_combo_list[u]=false;
          if (rect)
          {
            i=move_intersection_sr(combobuf[u].tile, combobuf[u].tile+max(combobuf[u].frames,1)-1, selection_left, selection_top, selection_width, selection_height);
          }
          else
          {
            i=move_intersection_ss(combobuf[u].tile, combobuf[u].tile+max(combobuf[u].frames,1)-1, selection_first, selection_last);
          }
          if ((i!=ti_none)&&(combobuf[u].tile!=0))
          {
            if (i==ti_broken || q==0)
            {
              sprintf(temptext, "%d\n", u);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            else if (i==ti_encompass)
            {
              move_combo_list[u]=true;
            }
          }
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following combos");
          if (move)
          {
            sprintf(buf3, "will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "will be partially or completely");
            sprintf(buf4, "overwritten by this process.  Proceed?");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }

      //check items
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        build_bii_list(false);
        for(int u=0; u<iMax; u++)
        {
          move_items_list[u]=false;
          if (rect)
          {
            i=move_intersection_sr(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+max(itemsbuf[bii[u].i].frames,1)-1, selection_left, selection_top, selection_width, selection_height);
          }
          else
          {
            i=move_intersection_ss(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+max(itemsbuf[bii[u].i].frames,1)-1, selection_first, selection_last);
          }
          if ((i!=ti_none)&&(itemsbuf[bii[u].i].tile!=0))
          {
            if (i==ti_broken || q==0)
            {
              sprintf(temptext, "%s\n", bii[u].s);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            else if (i==ti_encompass)
            {
              move_items_list[u]=true;
            }
          }
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following items");
          if (move)
          {
            sprintf(buf3, "will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "will be partially or completely");
            sprintf(buf4, "overwritten by this process.  Proceed?");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }

      //check weapons/misc
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        build_biw_list();
        bool BSZ2=get_bit(quest_rules,qr_BSZELDA)!=0;
        for(int u=0; u<wMAX; u++)
        {
          move_weapons_list[u]=false;
          int m=0;
          switch (biw[u].i)
          {
            case wSWORD:
            case wWSWORD:
            case wMSWORD:
            case wXSWORD:
            m=3+((wpnsbuf[biw[u].i].type==3)?1:0);
            break;
            case wSWORDSLASH:
            case wWSWORDSLASH:
            case wMSWORDSLASH:
            case wXSWORDSLASH:
            m=4;
            break;
            case iwMMeter:
            m=9;
            break;
            case wBRANG:
            case wMBRANG:
            case wFBRANG:
            m=BSZ2?1:3;
            break;
            case wBOOM:
            case wSBOOM:
            m=2;
            case iwDeath:
            m=BSZ2?4:2;
            case iwSpawn:
            m=3;
            default:
            m=0;
            break;
          }
          if (rect)
          {
            i=move_intersection_sr(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+max(wpnsbuf[biw[u].i].frames,1)-1+m, selection_left, selection_top, selection_width, selection_height);
          }
          else
          {
            i=move_intersection_ss(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+max(wpnsbuf[biw[u].i].frames,1)-1+m, selection_first, selection_last);
          }
          if ((i!=ti_none)&&(wpnsbuf[biw[u].i].tile!=0))
          {
            if (i==ti_broken || q==0)
            {
              sprintf(temptext, "%s\n", biw[u].s);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            else if (i==ti_encompass)
            {
              move_weapons_list[u]=true;
            }
          }
          if ((u==3)||(u==9))
          {
            if (rect)
            {
              i=move_intersection_sr(54, 55, selection_left, selection_top, selection_width, selection_height);
            }
            else
            {
              i=move_intersection_ss(54, 55, selection_first, selection_last);
            }
            if (i!=ti_none)
            {
              sprintf(temptext, "%s Impact (not shown in sprite list)\n", (u==3)?"Arrow":"Boomerang");
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
          }
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following weapons");
          if (move)
          {
            sprintf(buf3, "will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "will be partially or completely");
            sprintf(buf4, "overwritten by this process.  Proceed?");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }

      //check Link sprites
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        setup_link_sprite_items();
        for(int u=0; u<33; u++)
        {
          move_link_sprites_list[u]=false;
          if (rect)
          {
            i=move_intersection_rr(tile_col(link_sprite_items[u].tile), tile_row(link_sprite_items[u].tile), link_sprite_items[u].width, link_sprite_items[u].height, selection_left, selection_top, selection_width, selection_height);
          }
          else
          {
            i=move_intersection_rs(tile_col(link_sprite_items[u].tile), tile_row(link_sprite_items[u].tile), link_sprite_items[u].width, link_sprite_items[u].height, selection_first, selection_last);
          }
          if ((i!=ti_none)&&(link_sprite_items[u].tile!=0))
          {
            if (i==ti_broken || q==0)
            {
              sprintf(temptext, "%s\n", link_sprite_items[u].name);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            else if (i==ti_encompass)
            {
              move_link_sprites_list[u]=true;
            }
          }
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following Link sprites");
          if (move)
          {
            sprintf(buf3, "will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "sprites will be partially or completely");
            sprintf(buf4, "overwritten by this process.  Proceed?");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }

      //check map styles
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        bool BSZ2=(zinit.subscreen>2);
        map_styles_items[0].tile=misc.colors.blueframe_tile;
        map_styles_items[1].tile=misc.colors.HCpieces_tile;
        map_styles_items[2].tile=misc.colors.triforce_tile;
        map_styles_items[2].width=BSZ2?2:1;
        map_styles_items[2].height=BSZ2?3:1;
        map_styles_items[3].tile=misc.colors.triframe_tile;
        map_styles_items[3].width=BSZ2?7:6;
        map_styles_items[3].height=BSZ2?7:3;
        map_styles_items[4].tile=misc.colors.overworld_map_tile;
        map_styles_items[5].tile=misc.colors.dungeon_map_tile;
        for(int u=0; u<6; u++)
        {
          move_mapstyles_list[u]=false;
          if (rect)
          {
            i=move_intersection_rr(tile_col(map_styles_items[u].tile), tile_row(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_left, selection_top, selection_width, selection_height);
          }
          else
          {
            i=move_intersection_rs(tile_col(map_styles_items[u].tile), tile_row(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_first, selection_last);
          }
          if ((i!=ti_none)&&(map_styles_items[u].tile!=0))
          {
            if (i==ti_broken || q==0)
            {
              sprintf(temptext, "%s\n", map_styles_items[u].name);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            else if (i==ti_encompass)
            {
              move_mapstyles_list[u]=true;
            }
          }
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following map style");
          if (move)
          {
            sprintf(buf3, "items will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "items will be partially or completely");
            sprintf(buf4, "overwritten by this process.  Proceed?");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }

      //check game icons
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        char *icon_title[4]=
        {
          "No (Green) Ring", "Blue Ring", "Red Ring", "Golden Ring"
        };
        for(int u=0; u<4; u++)
        {
          move_game_icons_list[u]=false;
          if (rect)
          {
            i=move_intersection_sr(misc.icons[u], misc.icons[u], selection_left, selection_top, selection_width, selection_height);
          }
          else
          {
            i=move_intersection_ss(misc.icons[u], misc.icons[u], selection_first, selection_last);
          }
          if ((i!=ti_none)&&(misc.icons[u]!=0))
          {
            if (i==ti_broken || q==0)
            {
              sprintf(temptext, "%s\n", icon_title[u]);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            else if (i==ti_encompass)
            {
              move_game_icons_list[u]=true;
            }
          }
        }
        if (rect)
        {
          i=move_intersection_sr(41, 41, selection_left, selection_top, selection_width, selection_height);
        }
        else
        {
          i=move_intersection_ss(41, 41, selection_first, selection_last);
        }
        if ((i!=ti_none))// &&(41!=0))  //this is for when the quest sword can change
        {
          sprintf(temptext, "Quest Sword");
          if (strlen(tile_move_list_text)<65000)
          {
            strcat(tile_move_list_text, temptext);
          }
          else
          {
            if (!flood)
            {
              strcat(tile_move_list_text, "...\n...\n...\nmany others");
              flood=true;
            }
          }
          found=true;
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following quest icons");
          if (move)
          {
            sprintf(buf3, "will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "will be overwritten by this process.  Proceed?");
            sprintf(buf4, " ");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }

      //check dmap maps
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        bool BSZ2=(zinit.subscreen>2);
        for(int t=0; t<MAXDMAPS; t++)
        {
          dmap_map_items[0].tile=DMaps[t].minimap_1_tile;
          dmap_map_items[1].tile=DMaps[t].minimap_2_tile;
          dmap_map_items[2].tile=DMaps[t].largemap_1_tile;
          dmap_map_items[2].width=BSZ2?7:9;
          dmap_map_items[3].tile=DMaps[t].largemap_2_tile;
          dmap_map_items[3].width=BSZ2?7:9;

          for(int u=0; u<4; u++)
          {
            move_dmap_maps_list[t][u]=false;
            if (rect)
            {
              i=move_intersection_rr(tile_col(dmap_map_items[u].tile), tile_row(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_left, selection_top, selection_width, selection_height);
            }
            else
            {
              i=move_intersection_rs(tile_col(dmap_map_items[u].tile), tile_row(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_first, selection_last);
            }
            if ((i!=ti_none)&&(dmap_map_items[u].tile!=0))
            {
              if (i==ti_broken || q==0)
              {
                sprintf(temptext, "DMap %d %s\n", t, dmap_map_items[u].name);
                if (strlen(tile_move_list_text)<65000)
                {
                  strcat(tile_move_list_text, temptext);
                }
                else
                {
                  if (!flood)
                  {
                    strcat(tile_move_list_text, "...\n...\n...\nmany others");
                    flood=true;
                  }
                }
                found=true;
              }
              else if (i==ti_encompass)
              {
                move_dmap_maps_list[t][u]=true;
              }
            }
          }
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following dmap-specific");
          if (move)
          {
            sprintf(buf3, "subscreen maps will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "subscreen maps will be partially or completely");
            sprintf(buf4, "overwritten by this process.  Proceed?");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }

      //check enemies
      {  //this is here to allow this section to fold
        tile_move_list_text[0]=0;
        found=false;
        flood=false;
        build_bie_list(false);
        bool newtiles=get_bit(quest_rules,qr_NEWENEMYTILES)!=0;
        int u;
        for(u=0; u<eMAXGUYS; u++)
        {
          bool darknut=false;
          bool npc=false;
          int gleeok=0;
          switch (bie[u].i)
          {
            case eDKNUT1:
            case eDKNUT2:
            case eDKNUT3:
            case eDKNUT5:
              darknut=true;
              break;
            case eNPCSTAND1:
            case eNPCSTAND2:
            case eNPCSTAND3:
            case eNPCSTAND4:
            case eNPCSTAND5:
            case eNPCSTAND6:
            case eNPCWALK1:
            case eNPCWALK2:
            case eNPCWALK3:
            case eNPCWALK4:
            case eNPCWALK5:
            case eNPCWALK6:
              npc=true;
              break;
          }
          if (bie[u].i>=eGLEEOK1 && bie[u].i<=eGLEEOK4)
          {
            gleeok=1;
          }
          else if (bie[u].i>=eGLEEOK1F && bie[u].i<=eGLEEOK4F)
          {
            gleeok=2;
          }

          if (newtiles)
          {
            if (guysbuf[bie[u].i].e_tile==0)
            {
              continue;
            }
            if (guysbuf[bie[u].i].e_height==0)
            {
              if (rect)
              {
                i=move_intersection_sr(guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0), guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0)+max(guysbuf[bie[u].i].e_width-1, 0), selection_left, selection_top, selection_width, selection_height);
              }
              else
              {
                i=move_intersection_ss(guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0), guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0)+max(guysbuf[bie[u].i].e_width-1, 0), selection_first, selection_last);
              }
            }
            else
            {
              if (rect)
              {
                i=move_intersection_rr(tile_col(guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0)), tile_row(guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0)), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
              }
              else
              {
                i=move_intersection_rs(tile_col(guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0)), tile_row(guysbuf[bie[u].i].e_tile+(npc?wpnsbuf[iwNPCs].tile:0)), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
              }
            }
            if (((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
            {
              sprintf(temptext, "%s\n", bie[u].s);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            if (darknut)
            {
              if (rect)
              {
                i=move_intersection_rr(tile_col(guysbuf[bie[u].i].e_tile+120), tile_row(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
              }
              else
              {
                i=move_intersection_rs(tile_col(guysbuf[bie[u].i].e_tile+120), tile_row(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
              }
              if (((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
              {
                sprintf(temptext, "%s (broken shield)\n", bie[u].s);
                if (strlen(tile_move_list_text)<65000)
                {
                  strcat(tile_move_list_text, temptext);
                }
                else
                {
                  if (!flood)
                  {
                    strcat(tile_move_list_text, "...\n...\n...\nmany others");
                    flood=true;
                  }
                }
                found=true;
              }
            }
            else if (bie[u].i==eGANON && i==ti_none)
            {
              if (rect)
              {
                i=move_intersection_rr(tile_col(guysbuf[bie[u].i].e_tile), tile_row(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_left, selection_top, selection_width, selection_height);
              }
              else
              {
                i=move_intersection_rs(tile_col(guysbuf[bie[u].i].e_tile), tile_row(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_first, selection_last);
              }
              if (((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
              {
                sprintf(temptext, "%s\n", bie[u].s);
                if (strlen(tile_move_list_text)<65000)
                {
                  strcat(tile_move_list_text, temptext);
                }
                else
                {
                  if (!flood)
                  {
                    strcat(tile_move_list_text, "...\n...\n...\nmany others");
                    flood=true;
                  }
                }
                found=true;
              }
            }
            else if (gleeok && i==ti_none)
            {
              for (int j=0; j<4 && i==ti_none; ++j)
              {
                if (rect)
                {
                  i=move_intersection_rr(tile_col(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), tile_row(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_left, selection_top, selection_width, selection_height);
                }
                else
                {
                  i=move_intersection_rs(tile_col(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), tile_row(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_first, selection_last);
                }
              }
              if (i==ti_none)
              {
                int c=tile_col(guysbuf[bie[u].i].e_tile)+(gleeok>1?-12:0);
                int r=tile_row(guysbuf[bie[u].i].e_tile)+(gleeok>1?17:8);
                if (rect)
                {
                  i=move_intersection_rr(c, r, 20, 3, selection_left, selection_top, selection_width, selection_height);
                }
                else
                {
                  i=move_intersection_rs(c, r, 20, 3, selection_first, selection_last);
                }
                if (i==ti_none)
                {
                  if (rect)
                  {
                    i=move_intersection_rr(c, r+3, 16, 6, selection_left, selection_top, selection_width, selection_height);
                  }
                  else
                  {
                    i=move_intersection_rs(c, r+3, 16, 6, selection_first, selection_last);
                  }
                }
              }
              if (((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
              {
                sprintf(temptext, "%s\n", bie[u].s);
                if (strlen(tile_move_list_text)<65000)
                {
                  strcat(tile_move_list_text, temptext);
                }
                else
                {
                  if (!flood)
                  {
                    strcat(tile_move_list_text, "...\n...\n...\nmany others");
                    flood=true;
                  }
                }
                found=true;
              }
            }
          }
          else
          {
            if (guysbuf[bie[u].i].tile==0)
            {
              continue;
            }
            if (guysbuf[bie[u].i].height==0)
            {
              if (rect)
              {
                i=move_intersection_sr(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+max(guysbuf[bie[u].i].width-1, 0), selection_left, selection_top, selection_width, selection_height);
              }
              else
              {
                i=move_intersection_ss(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+max(guysbuf[bie[u].i].width-1, 0), selection_first, selection_last);
              }
            }
            else
            {
              if (rect)
              {
                i=move_intersection_rr(tile_col(guysbuf[bie[u].i].tile), tile_row(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_left, selection_top, selection_width, selection_height);
              }
              else
              {
                i=move_intersection_rs(tile_col(guysbuf[bie[u].i].tile), tile_row(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_first, selection_last);
              }
            }
            if (((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
            {
              sprintf(temptext, "%s\n", bie[u].s);
              if (strlen(tile_move_list_text)<65000)
              {
                strcat(tile_move_list_text, temptext);
              }
              else
              {
                if (!flood)
                {
                  strcat(tile_move_list_text, "...\n...\n...\nmany others");
                  flood=true;
                }
              }
              found=true;
            }
            if (guysbuf[bie[u].i].s_tile!=0)
            {
              if (guysbuf[bie[u].i].s_height==0)
              {
                if (rect)
                {
                  i=move_intersection_sr(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+max(guysbuf[bie[u].i].s_width-1, 0), selection_left, selection_top, selection_width, selection_height);
                }
                else
                {
                  i=move_intersection_ss(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+max(guysbuf[bie[u].i].s_width-1, 0), selection_first, selection_last);
                }
              }
              else
              {
                if (rect)
                {
                  i=move_intersection_rr(tile_col(guysbuf[bie[u].i].s_tile), tile_row(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_left, selection_top, selection_width, selection_height);
                }
                else
                {
                  i=move_intersection_rs(tile_col(guysbuf[bie[u].i].s_tile), tile_row(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_first, selection_last);
                }
              }
              if (((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
              {
                sprintf(temptext, "%s (%s)\n", bie[u].s, darknut?"broken shield":"secondary tiles");
                if (strlen(tile_move_list_text)<65000)
                {
                  strcat(tile_move_list_text, temptext);
                }
                else
                {
                  if (!flood)
                  {
                    strcat(tile_move_list_text, "...\n...\n...\nmany others");
                    flood=true;
                  }
                }
                found=true;
              }
            }
          }
        }
        if (found)
        {
          sprintf(buf2, "The tiles used by the following enemies");
          if (move)
          {
            sprintf(buf3, "will be partially cleared by the move.");
            sprintf(buf4, "Proceed?");
          }
          else
          {
            sprintf(buf3, "will be partially or completely");
            sprintf(buf4, "overwritten by this process.  Proceed?");
          }
          tile_move_list_dlg[1].dp=buf2;
          tile_move_list_dlg[2].dp=buf3;
          tile_move_list_dlg[3].dp=buf4;
          tile_move_list_dlg[4].dp=tile_move_list_text;
          tile_move_list_dlg[4].d2=0;
          if (TileProtection)
          {
            int ret=zc_popup_dialog(tile_move_list_dlg,2);
            position_mouse_z(0);
            if (ret!=5)
            {
              return false;
            }
          }
        }
      }
    }
  }
  //
  // copy tiles and delete if needed (move)
  go_tiles();

  int diff=dest_first-src_first;
  if (rect)
  {
    for (int r=0; r<rows; ++r)
    {
      for (int c=0; c<cols; ++c)
      {
        int dt=(dest_first+((r*TILES_PER_ROW)+c));
        int st=(src_first+((r*TILES_PER_ROW)+c));
        reset_tile(newtilebuf, dt, newundotilebuf[st].format);
        for(int j=0; j<tilesize(newundotilebuf[st].format); j++)
        {
          newtilebuf[dt].data[j]=newundotilebuf[st].data[j];
        }
        if (move)
        {
          if(((st%TILES_PER_ROW)<(dest_first%TILES_PER_ROW)||(st%TILES_PER_ROW)>((dest_first+cols-1)%TILES_PER_ROW))||(st<dest_first||st>(dest_first+((rows-1)*TILES_PER_ROW)+(cols-1))))
            reset_tile(newtilebuf, st);
        }
      }
    }
  }
  else
  {
    for (int c=0; c<copies; ++c)
    {
      int dt=(dest_first+c);
      int st=(src_first+c);
      reset_tile(newtilebuf, dt, newundotilebuf[st].format);
      for(int j=0; j<tilesize(newundotilebuf[st].format); j++)
      {
        newtilebuf[dt].data[j]=newundotilebuf[st].data[j];
      }
      if (move)
      {
        if(st<dest_first||st>(dest_first+c-1))
          reset_tile(newtilebuf, st);
      }
    }
  }

  if (move)
  {
    for(int u=0; u<MAXCOMBOS; u++)
    {
      if (move_combo_list[u])
      {
        combobuf[u].tile+=diff;
      }
    }
    for(int u=0; u<iMax; u++)
    {
      if (move_items_list[u])
      {
        itemsbuf[bii[u].i].tile+=diff;
      }
    }
    for(int u=0; u<wMAX; u++)
    {
      if (move_weapons_list[u])
      {
        wpnsbuf[bii[u].i].tile+=diff;
      }
    }
    for(int u=0; u<33; u++)
    {
      if (move_link_sprites_list[u])
      {
        switch(u)
        {
          case 0:
          case 1:
          case 2:
          case 3:
          walkspr[u][spr_tile]+=diff;
          break;
          case 4:
          case 5:
          case 6:
          case 7:
          slashspr[u-4][spr_tile]+=diff;
          break;
          case 8:
          case 9:
          case 10:
          case 11:
          stabspr[u-8][spr_tile]+=diff;
          break;
          case 12:
          case 13:
          case 14:
          case 15:
          poundspr[u-12][spr_tile]+=diff;
          break;
          case 16:
          case 17:
          holdspr[0][u-16][spr_tile]+=diff;
          break;
          case 18:
          castingspr[spr_tile]+=diff;
          break;
          case 19:
          case 20:
          case 21:
          case 22:
          floatspr[u-19][spr_tile]+=diff;
          break;
          case 23:
          case 24:
          case 25:
          case 26:
          swimspr[u-23][spr_tile]+=diff;
          break;
          case 27:
          case 28:
          case 29:
          case 30:
          divespr[u-27][spr_tile]+=diff;
          break;
          case 31:
          case 32:
          holdspr[1][u-31][spr_tile]+=diff;
          break;
        }
      }
    }
    for(int u=0; u<6; u++)
    {
      if (move_mapstyles_list[u])
      {
        switch(u)
        {
          case 0:
          misc.colors.blueframe_tile+=diff;
          break;
          case 1:
          misc.colors.HCpieces_tile+=diff;
          break;
          case 2:
          misc.colors.triforce_tile+=diff;
          break;
          case 3:
          misc.colors.triframe_tile+=diff;
          break;
          case 4:
          misc.colors.overworld_map_tile+=diff;
          break;
          case 5:
          misc.colors.dungeon_map_tile+=diff;
          break;
        }
      }
    }
    for(int u=0; u<4; u++)
    {
      if (move_game_icons_list[u])
      {
        misc.icons[u]+=diff;
      }
    }
    for(int t=0; t<MAXDMAPS; t++)
    {
      for(int u=0; u<4; u++)
      {
        move_dmap_maps_list[t][u]=false;
        if (move_dmap_maps_list[t][u])
        {
          switch(u)
          {
            case 0:
            DMaps[t].minimap_1_tile+=diff;
            break;
            case 1:
            DMaps[t].minimap_2_tile+=diff;
            break;
            case 2:
            DMaps[t].largemap_1_tile+=diff;
            break;
            case 3:
            DMaps[t].largemap_2_tile+=diff;
            break;
          }
        }
      }
    }
  }
  return true;
}
//

bool copy_tiles(int &tile,int &tile2,int &copy,int &copycnt, bool rect_sel, bool move)
{
  bool ctrl=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
  bool copied=false;
  copied=copy_tiles_united(tile,tile2,copy,copycnt,rect_sel,move);
  if (copied)
  {
    if (!ctrl)
    {
      copy=-1;
      tile2=tile;
    }
    saved=false;
  }
  return copied;
}


void copy_combos(int &tile,int &tile2,int &copy,int &copycnt, bool masscopy)
{
	if(tile2<tile)
	{
		swap(tile,tile2);
	}

  if (!masscopy) {
		if (tile==copy)
		{
			copy=-1;
			tile2=tile;
			return;
		}
		// go_combos(); // commented because caller does it for us
		//if copying to an earlier combo, copy from left to right
		//otherwise, copy from right to left
		for(int t=(tile<copy)?0:(copycnt-1); (tile<copy)?(t<copycnt):(t>=0); (tile<copy)?(t++):(t--))
		{
		combobuf[tile+t]=combobuf[copy+t];
		}
		copy=-1;
		tile2=tile;
		saved=false;
	}
	else {
		// go_combos();
		int src=copy, dest=tile;
		do {
			combobuf[dest]=combobuf[src];
			++src; ++dest;
			if((src-copy)==copycnt) src=copy;
		} while(dest<=tile2);
		copy=-1;
		tile2=tile;
		saved=false;
	}
	return;
}

void move_combos(int &tile,int &tile2,int &copy,int &copycnt)
{
  if(tile2<tile)
  {
    swap(tile,tile2);
  }
  if (tile==copy)
  {
    copy=-1;
    tile2=tile;
    return;
  }
  go_combos();
  for(int t=(tile<copy)?0:(copycnt-1); (tile<copy)?(t<copycnt):(t>=0); (tile<copy)?(t++):(t--))
  {
    combobuf[tile+t]=combobuf[copy+t];
    clear_combo(copy+t);
  }

  for(int i=0; i<map_count && i<MAXMAPS2; i++)
  {
    for (int j=0; j<MAPSCRS; j++)
    {
      for (int k=0; k<176; k++)
      {
        if ((TheMaps[i*MAPSCRS+j].data[k]>=copy)&&(TheMaps[i*MAPSCRS+j].data[k]<copy+copycnt))
        {
          TheMaps[i*MAPSCRS+j].data[k]=TheMaps[i*MAPSCRS+j].data[k]-copy+tile;
        }
      }
      for (int k=0; k<128; k++)
      {
        if ((TheMaps[i*MAPSCRS+j].secretcombo[k]>=copy)&& (TheMaps[i*MAPSCRS+j].secretcombo[k]<copy+copycnt))
        {
          TheMaps[i*MAPSCRS+j].secretcombo[k]=TheMaps[i*MAPSCRS+j].secretcombo[k]-copy+tile;
        }
      }
      if ((TheMaps[i*MAPSCRS+j].undercombo>=copy)&&(TheMaps[i*MAPSCRS+j].undercombo<copy+copycnt))
      {
        TheMaps[i*MAPSCRS+j].undercombo=TheMaps[i*MAPSCRS+j].undercombo-copy+tile;
      }
    }
  }
  for (int i=0; i<MAXDOORCOMBOSETS; i++)
  {
    for (int j=0; j<9; j++)
    {
      if (j<4)
      {
        if ((DoorComboSets[i].walkthroughcombo[j]>=copy)&&(DoorComboSets[i].walkthroughcombo[j]<copy+copycnt))
        {
          DoorComboSets[i].walkthroughcombo[j]=DoorComboSets[i].walkthroughcombo[j]-copy+tile;
        }
        if (j<3)
        {
          if (j<2)
          {
            if ((DoorComboSets[i].bombdoorcombo_u[j]>=copy)&&(DoorComboSets[i].bombdoorcombo_u[j]<copy+copycnt))
            {
              DoorComboSets[i].bombdoorcombo_u[j]=DoorComboSets[i].bombdoorcombo_u[j]-copy+tile;
            }
            if ((DoorComboSets[i].bombdoorcombo_d[j]>=copy)&&(DoorComboSets[i].bombdoorcombo_d[j]<copy+copycnt))
            {
              DoorComboSets[i].bombdoorcombo_d[j]=DoorComboSets[i].bombdoorcombo_d[j]-copy+tile;
            }
          }
          if ((DoorComboSets[i].bombdoorcombo_l[j]>=copy)&&(DoorComboSets[i].bombdoorcombo_l[j]<copy+copycnt))
          {
            DoorComboSets[i].bombdoorcombo_l[j]=DoorComboSets[i].bombdoorcombo_l[j]-copy+tile;
          }
          if ((DoorComboSets[i].bombdoorcombo_r[j]>=copy)&&(DoorComboSets[i].bombdoorcombo_r[j]<copy+copycnt))
          {
            DoorComboSets[i].bombdoorcombo_r[j]=DoorComboSets[i].bombdoorcombo_r[j]-copy+tile;
          }
        }
      }
      for (int k=0; k<6; k++)
      {
        if (k<4)
        {
          if ((DoorComboSets[i].doorcombo_u[j][k]>=copy)&&(DoorComboSets[i].doorcombo_u[j][k]<copy+copycnt))
          {
            DoorComboSets[i].doorcombo_u[j][k]=DoorComboSets[i].doorcombo_u[j][k]-copy+tile;
          }
          if ((DoorComboSets[i].doorcombo_d[j][k]>=copy)&&(DoorComboSets[i].doorcombo_d[j][k]<copy+copycnt))
          {
            DoorComboSets[i].doorcombo_d[j][k]=DoorComboSets[i].doorcombo_d[j][k]-copy+tile;
          }
        }
        if ((DoorComboSets[i].doorcombo_l[j][k]>=copy)&&(DoorComboSets[i].doorcombo_l[j][k]<copy+copycnt))
        {
          DoorComboSets[i].doorcombo_l[j][k]=DoorComboSets[i].doorcombo_l[j][k]-copy+tile;
        }
        if ((DoorComboSets[i].doorcombo_r[j][k]>=copy)&&(DoorComboSets[i].doorcombo_r[j][k]<copy+copycnt))
        {
          DoorComboSets[i].doorcombo_r[j][k]=DoorComboSets[i].doorcombo_r[j][k]-copy+tile;
        }
      }
    }
  }

  for(int i=0; i<MAXCOMBOS; i++)
  {
    if((combobuf[i].nextcombo>=copy)&&(combobuf[i].nextcombo<copy+copycnt))
    {
      combobuf[i].nextcombo=combobuf[i].nextcombo-copy+tile;
    }
  }

  for(int i=0;i<MAXCOMBOALIASES;i++)
  {
    int count=(comboa_lmasktotal(combo_aliases[i].layermask)+1)*(combo_aliases[i].width)*(combo_aliases[i].height);
    for(int j=0;j<count;j++)
    {
      if((combo_aliases[i].combos[j]>=copy)&&(combo_aliases[i].combos[j]<copy+copycnt)&&(combo_aliases[i].combos[j]!=0))
      {
        combo_aliases[i].combos[j]=combo_aliases[i].combos[j]-copy+tile;
      }
    }
  }

  copy=-1;
  tile2=tile;
  saved=false;
}

void delete_tiles(int &tile,int &tile2,bool rect_sel)
{
  char buf[40];
  if(tile==tile2)
  {
    sprintf(buf,"Delete tile %d?",tile);
  }
  else
  {
    sprintf(buf,"Delete tiles %d-%d?",min(tile,tile2),max(tile,tile2));
  }

  if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
  {
    go_tiles();

    int firsttile=min(tile,tile2), lasttile=max(tile,tile2), coldiff=0;

    if (rect_sel && tile_col(firsttile)>tile_col(lasttile))
    {
      coldiff=tile_col(firsttile)-tile_col(lasttile);
      firsttile-=coldiff;
      lasttile+=coldiff;
    }

    go_tiles();

    //if copying to an earlier tile, copy from left to right
    //otherwise, copy from right to left
    for(int t=firsttile; t<=lasttile; t++)
      if (!rect_sel ||
          ((tile_col(t)>=tile_col(firsttile)) &&
           (tile_col(t)<=tile_col(lasttile))))
        reset_tile(newtilebuf, t);
      tile=tile2=min(tile,tile2);
    saved=false;
  }
}

void overlay_tile(int dest,int src,bool backwards)
{
  byte buf[256];
  go_tiles();

  unpack_tile(newtilebuf, dest, 0, false);
  for(int i=0; i<256; i++)
    buf[i] = unpackbuf[i];

  unpack_tile(newtilebuf, src, 0, false);
  for(int i=0; i<256; i++)
  {
    if(backwards)
    {
      if(!buf[i])
      {
        buf[i] = unpackbuf[i];
      }
    }
    else
    {
      if(unpackbuf[i])
      {
        buf[i] = unpackbuf[i];
      }
    }
  }

  pack_tile(newtilebuf, buf,dest);
  saved=false;
}

void mass_overlay_tile(int dest1, int dest2, int src, bool backwards, bool rect_sel)
{
	byte buf[256];
	go_tiles();

	unpack_tile(newtilebuf, src, 0, false);
	for(int i=0; i<256; i++)
    buf[i] = unpackbuf[i];

	if (!rect_sel) {
		for ( int d=dest1; d <= dest2; ++d)
		{
			unpack_tile(newtilebuf, d, 0, false);
			for(int i=0; i<256; i++)
			{
				if(!backwards)
			    {
			      if(!buf[i])
			      {
			        buf[i] = unpackbuf[i];
			      }
			    }
			    else
			    {
			      if(unpackbuf[i])
			      {
			        buf[i] = unpackbuf[i];
			      }
			    }
			}
			pack_tile(newtilebuf, buf,d);
		}
	}
	else {
		int rmin=min(tile_row(dest1),tile_row(dest2)), rmax=max(tile_row(dest1),tile_row(dest2)), cmin=min(tile_col(dest1),tile_col(dest2)), cmax=max(tile_col(dest1),tile_col(dest2)), d=0;
		for (int j=cmin; j<=cmax; ++j) {
			for (int k=rmin; k<=rmax; ++k) {
				d=j+TILES_PER_ROW*k;
				unpack_tile(newtilebuf, d, 0, false);
			for(int i=0; i<256; i++)
			{
				if(!backwards)
			    {
			      if(!buf[i])
			      {
			        buf[i] = unpackbuf[i];
			      }
			    }
			    else
			    {
			      if(unpackbuf[i])
			      {
			        buf[i] = unpackbuf[i];
			      }
			    }
			}
			pack_tile(newtilebuf, buf,d);
		}
		}
	}
	return;
}

void sel_tile(int &tile, int &tile2, int &first, int type, int s)
{
  tile+=s;
  bound(tile,0,NEWMAXTILES-1);
  if(type!=0 || !(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
    tile2 = tile;
  first = tile - (tile%TILES_PER_PAGE);
}

void convert_tile(int t, int bp2, int cs, bool shift, bool alt)
{
  int cst;
  switch(bp2)
  {
    case tf4Bit:
      switch(newtilebuf[t].format)
      {
        case tf4Bit:
          //already in the right format
          break;
        case tf8Bit:
          unpack_tile(newtilebuf, t, 0, true);
          if (alt) //reduce
          {
            for(int i=0;i<256;i++)
            {
              if (!shift||unpackbuf[i]!=0)
              {
                unpackbuf[i]=(cset_reduce_table[unpackbuf[i]]);
              }
            }
          }
          else //truncate
          {
            for(int i=0;i<256;i++)
            {
              unpackbuf[i]&=15;
            }
          }
          reset_tile(newtilebuf, t, tf4Bit);
          pack_tile(newtilebuf, unpackbuf, t);
          break;
      }
      break;
    case tf8Bit:
      switch(newtilebuf[t].format)
      {
        case tf4Bit:
          unpack_tile(newtilebuf, t, 0, true);
          cst = cs&15;
          cst <<= CSET_SHFT;
          for(int i=0;i<256;i++)
          {
            if (!shift||unpackbuf[i]!=0)
            {
              unpackbuf[i]+=cst;
            }
          }
          reset_tile(newtilebuf, t, tf8Bit);
          pack_tile(newtilebuf, unpackbuf, t);
          break;
        case tf8Bit:
          //already in the right format
          break;
      }
      break;
  }
}

static DIALOG create_relational_tiles_dlg[] =
{
  // (dialog proc)       (x)   (y)    (w)     (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,         0,    0,   160,     92,   vc(0),              vc(11),           0,       D_EXIT,     0,             0,       (void *) "Tile Setup", NULL, NULL },
  { jwin_rtext_proc,      74,   28,    48,      8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Frames:", NULL, NULL },
  { jwin_edit_proc,       78,   24,    48,     16,   0,                  0,                0,       0,          6,             0,       NULL, NULL, NULL },
  { jwin_radio_proc,       8,   44,    64,      9,   vc(14),             vc(1),            0,       D_SELECTED, 0,             0,       (void *) "Relational", NULL, NULL },
  { jwin_radio_proc,      68,   44,    64,      9,   vc(14),             vc(1),            0,       0,          0,             0,       (void *) "Dungeon Carving", NULL, NULL },
  { jwin_button_proc,     10,   66,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { jwin_button_proc,     90,   66,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { NULL,                  0,    0,     0,      0,   0,                  0,                0,       0,          0,             0,       NULL, NULL,  NULL }
};

int select_tile(int &tile,int &flip,int type,int &cs,bool edit_cs,int exnow)
{
  reset_combo_animations();
  reset_combo_animations2();
  bound(tile,0,NEWMAXTILES-1);
  ex=exnow;
  int done=0;
  int oflip=flip;
  int otile=tile;
  int ocs=cs;
  int first=(tile/TILES_PER_PAGE)*TILES_PER_PAGE; //first tile on the current page
  int copy=-1;
  int tile2=tile,copycnt=0;
  int tile_clicked=-1;
  bool rect_sel=false;
  bound(first,0,(TILES_PER_PAGE*TILE_PAGES)-1);
  position_mouse_z(0);


  if(type==0)
    select_tile_rc_menu[5].text = "Grab";

  go();

  draw_tiles(first,cs);
  if(type==0)
  {
    tile_info_0(tile,tile2,cs,copy,copycnt,first/TILES_PER_PAGE,rect_sel);
  }
  else
  {
    tile_info_1(otile,oflip,ocs,tile,flip,cs,copy,first/TILES_PER_PAGE);
  }

  go_tiles();
  while(gui_mouse_b());
  bool bdown=false;
  int f=0;

  do
  {
    //int tile_col(int tile)
    //int tile_row(int tile)
    //int tile_page(int tile)
    //int tile_page_row(int tile)
    rest(4);
    int top=tile_row(min(tile, tile2));
    int left=min(tile_col(tile), tile_col(tile2));
    int rows=tile_row(max(tile, tile2))-top+1;
    int columns=max(tile_col(tile), tile_col(tile2))-left+1;
    bool is_rect=(rows==1)||(columns==TILES_PER_ROW)||rect_sel;
    bool redraw=false;
    if (mouse_z!=0)
    {
      sel_tile(tile,tile2,first,type,((mouse_z/abs(mouse_z))*(-1)*TILES_PER_PAGE));
      position_mouse_z(0);
      redraw=true;
    }

    if(keypressed())
    {
      //tile_page=(tile/TILES_PER_PAGE);
      //tile_row=(tile/TILES_PER_ROW);
      //tile_col=(tile%TILES_PER_ROW);
      //tile_page_row=(tile_row/TILE_ROWS_PER_PAGE);
      switch(readkey()>>8)
      {
        case KEY_ENTER_PAD:
        case KEY_ENTER: done=2; break;
        case KEY_ESC:   done=1; break;
        case KEY_F1:    onHelp(); break;
        case KEY_PLUS_PAD:  if (edit_cs) {cs = (cs<11) ? cs+1:0;  redraw=true;} break;
        case KEY_MINUS_PAD: if (edit_cs) {cs = (cs>0)  ? cs-1:11; redraw=true;} break;
        case KEY_UP:
        {
          switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
          {
            case 3:  //ALT and CTRL
            case 2:  //ALT
            if (is_rect)
            {
              go_slide_tiles(columns, rows, top, left);
              int bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
              bool same = true;
              for (int d=0; d<columns;d++)
              {
                for (int s=0; s<rows; s++)
                {
                  int t=((top+s)*TILES_PER_ROW)+left+d;
                  if (newtilebuf[t].format!=bitcheck) same = false;
                }
              }

              if (!same) break;

              for (int c=0; c<columns; c++)
              {
                for (int r=0; r<rows; r++)
                {
                  int temptile=((top+r)*TILES_PER_ROW)+left+c;
                  qword *src_pixelrow=(qword*)(newundotilebuf[temptile].data+(8*bitcheck));
                  qword *dest_pixelrow=(qword*)(newtilebuf[temptile].data);
                  for (int pixelrow=0; pixelrow<16*bitcheck; pixelrow++)
                  {
                    if (pixelrow==15*bitcheck)
                    {
                      src_pixelrow=(qword*)(newtilebuf[temptile+TILES_PER_ROW].data);
                    }
                    *dest_pixelrow=*src_pixelrow;
                    dest_pixelrow++;
                    src_pixelrow++;
                  }
                }
                qword *dest_pixelrow=(qword*)(newtilebuf[((top+rows-1)*TILES_PER_ROW)+left+c].data+(120*bitcheck));
                for(int b=0;b<bitcheck;b++,dest_pixelrow++)
                {
                  if ((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
                  {
                    *dest_pixelrow=0;
                  }
                  else
                  {
                    qword *src_pixelrow=(qword*)(newundotilebuf[(top*TILES_PER_ROW)+left+c].data+(8*b));
                    *dest_pixelrow=*src_pixelrow;
                  }
                }
              }
            }
            break;
            case 1:  //CTRL
            case 0:  //None
            sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-1*(tile_page_row(tile)*TILES_PER_ROW):-TILES_PER_ROW);
            redraw=true;
            default: //Others
            break;
          }
        }
        break;
        case KEY_DOWN:
        {
          switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
          {
            case 3:  //ALT and CTRL
            case 2:  //ALT
            if (is_rect)
            {
              go_slide_tiles(columns, rows, top, left);
              int bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
              bool same = true;
              for (int c=0; c<columns;c++)
              {
                for (int r=0; r<rows; r++)
                {
                  int t=((top+r)*TILES_PER_ROW)+left+c;
                  if (newtilebuf[t].format!=bitcheck) same = false;
                }
              }

              if (!same) break;
              for (int c=0; c<columns; c++)
              {
                for (int r=rows-1; r>=0; r--)
                {
                  int temptile=((top+r)*TILES_PER_ROW)+left+c;
                  qword *src_pixelrow=(qword*)(newundotilebuf[temptile].data+(112*bitcheck)+(8*(bitcheck-1)));
                  qword *dest_pixelrow=(qword*)(newtilebuf[temptile].data+(120*bitcheck)+(8*(bitcheck-1)));
                  for (int pixelrow=(8<<bitcheck)-1; pixelrow>=0; pixelrow--)
                  {
                    if (pixelrow<bitcheck)
                    {
                      qword *tempsrc=(qword*)(newtilebuf[temptile-TILES_PER_ROW].data+(120*bitcheck)+(8*pixelrow));
                      *dest_pixelrow=*tempsrc;
                      //*dest_pixelrow=0;
                    }
                    else
                    {
                      *dest_pixelrow=*src_pixelrow;
                    }
                    dest_pixelrow--;
                    src_pixelrow--;
                  }
                }
                qword *dest_pixelrow=(qword*)(newtilebuf[(top*TILES_PER_ROW)+left+c].data);
                qword *src_pixelrow=(qword*)(newundotilebuf[((top+rows-1)*TILES_PER_ROW)+left+c].data+(120*bitcheck));
                for(int b=0;b<bitcheck;b++)
                {
                  if ((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
                  {
                    *dest_pixelrow=0;
                  }
                  else
                  {
                    *dest_pixelrow=*src_pixelrow;
                  }
                  dest_pixelrow++;
                  src_pixelrow++;
                }
              }
            }
            redraw=true;
            break;
            case 1:  //CTRL
            case 0:  //None
            sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?((TILE_ROWS_PER_PAGE-1)-tile_page_row(tile))*TILES_PER_ROW:TILES_PER_ROW);
            redraw=true;
            default: //Others
            break;
          }
        }
        break;
        case KEY_LEFT:
        {
          switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
          {
            case 3:  //ALT and CTRL
            case 2:  //ALT
            if (is_rect)
            {
              go_slide_tiles(columns, rows, top, left);
              int bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
              bool same = true;
              for (int c=0; c<columns;c++)
              {
                for (int r=0; r<rows; r++)
                {
                  int t=((top+r)*TILES_PER_ROW)+left+c;
                  if (newtilebuf[t].format!=bitcheck) same = false;
                }
              }

              if (!same) break;
              for (int r=0; r<rows; r++)
              {
                for (int c=0; c<columns; c++)
                {
                  int temptile=((top+r)*TILES_PER_ROW)+left+c;
                  byte *dest_pixelrow=(newtilebuf[temptile].data);
                  for (int pixelrow=0; pixelrow<16; pixelrow++)
                  {
#ifdef ALLEGRO_LITTLE_ENDIAN
                    //if(bitcheck==tf4Bit)
                    //{
                    for(int p=0;p<(8*bitcheck)-1;p++)
                    {
                      if(bitcheck==tf4Bit)
                      {
                        *dest_pixelrow=*dest_pixelrow>>4;
                        *dest_pixelrow|=(*(dest_pixelrow+1)<<4);
                        if(p==6) *(dest_pixelrow+1)=*(dest_pixelrow+1)>>4;
                      }
                      else
                      {
                        *dest_pixelrow=*(dest_pixelrow+1);
                      }
                      dest_pixelrow++;
                    }
#else
                    for(int p=0;p<(8*bitcheck)-1;p++)
                    {
                      if(bitcheck==tf4Bit)
                      {
                        *dest_pixelrow=*dest_pixelrow<<4;
                        *dest_pixelrow|=(*(dest_pixelrow+1)>>4);
                        if(p==6) *(dest_pixelrow+1)=*(dest_pixelrow+1)<<4;
                      }
                      else
                      {
                        *dest_pixelrow=*(dest_pixelrow+1);
                      }
                      dest_pixelrow++;
                    }
#endif

                    if(c==columns-1)
                    {
                      if (!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
                      {
                        byte *tempsrc=(newundotilebuf[((top+r)*TILES_PER_ROW)+left].data+(pixelrow*8*bitcheck));
#ifdef ALLEGRO_LITTLE_ENDIAN
                        if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
                        else *dest_pixelrow=*tempsrc;
#else
                        if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
                        else *dest_pixelrow=*tempsrc;
#endif
                      }
                    }
                    else

                    {
                      byte *tempsrc=(newtilebuf[temptile+1].data+(pixelrow*8*bitcheck));
#ifdef ALLEGRO_LITTLE_ENDIAN
                      if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
                      else *dest_pixelrow=*tempsrc;
#else
                      if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
                      else *dest_pixelrow=*tempsrc;
#endif
                    }
                    dest_pixelrow++;
                  }
                }
              }
            }
            break;
            case 1:  //CTRL
            case 0:  //None
            sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-(tile%TILES_PER_ROW):-1);
            redraw=true;
            default: //Others
            break;
          }
        }
        break;
        case KEY_RIGHT:
        {
          switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
          {
            case 3:  //ALT and CTRL
            case 2:  //ALT
            if (is_rect)
            {
              go_slide_tiles(columns, rows, top, left);
              int bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
              bool same = true;
              for (int c=0; c<columns;c++)
              {
                for (int r=0; r<rows; r++)
                {
                  int t=((top+r)*TILES_PER_ROW)+left+c;
                  if (newtilebuf[t].format!=bitcheck) same = false;
                }
              }

              if (!same) break;
              for (int r=0; r<rows; r++)
              {
                for (int c=columns-1; c>=0; c--)
                {
                  int temptile=((top+r)*TILES_PER_ROW)+left+c;
                  byte *dest_pixelrow=(newtilebuf[temptile].data)+(128*bitcheck)-1;
                  for (int pixelrow=15; pixelrow>=0; pixelrow--)
                  {
#ifdef ALLEGRO_LITTLE_ENDIAN
                    //*dest_pixelrow=(*dest_pixelrow)<<4;
                    for(int p=0;p<(8*bitcheck)-1;p++)
                    {
                      if(bitcheck==tf4Bit)
                      {
                        *dest_pixelrow=*dest_pixelrow<<4;
                        *dest_pixelrow|=(*(dest_pixelrow-1)>>4);
                        if(p==6) *(dest_pixelrow-1)=*(dest_pixelrow-1)<<4;
                      }
                      else
                      {
                        *dest_pixelrow=*(dest_pixelrow-1);
                      }
                      dest_pixelrow--;
                    }
#else
                    for(int p=0;p<(8*bitcheck)-1;p++)
                    {
                      if(bitcheck==tf4Bit)
                      {
                        *dest_pixelrow=*dest_pixelrow>>4;
                        *dest_pixelrow|=(*(dest_pixelrow-1)<<4);
                        if(p==6) *(dest_pixelrow-1)=*(dest_pixelrow-1)>>4;
                      }
                      else
                      {
                        *dest_pixelrow=*(dest_pixelrow-1);
                      }
                      dest_pixelrow--;
                    }
#endif
                    if(c==0)
                    {
                      if (!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
                      {
                        byte *tempsrc=(newundotilebuf[(((top+r)*TILES_PER_ROW)+left+columns-1)].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
#ifdef ALLEGRO_LITTLE_ENDIAN
                        if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
                        else *dest_pixelrow=*tempsrc;
#else
                        if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
                        else *dest_pixelrow=*tempsrc;
#endif
                      }
                    }
                    else
                    {
                      byte *tempsrc=(newtilebuf[temptile-1].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
#ifdef ALLEGRO_LITTLE_ENDIAN
                      //(*dest_pixelrow)|=((*(dest_pixelrow-16))&0xF000000000000000ULL)>>60;
                      if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
                      else *dest_pixelrow=*tempsrc;
#else
                      if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
                      else *dest_pixelrow=*tempsrc;
#endif
                    }
                    dest_pixelrow--;
                  }
                }
              }
            }
            break;
            case 1:  //CTRL
            case 0:  //None
            sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?(TILES_PER_ROW)-(tile%TILES_PER_ROW)-1:1);
            redraw=true;
            default: //Others
            break;
          }
        }
        break;
        case KEY_PGUP:  sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-1*(tile_row(tile)*TILES_PER_ROW):-TILES_PER_PAGE); redraw=true; break;
        case KEY_PGDN:  sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?((TILE_PAGES*TILE_ROWS_PER_PAGE)-tile_row(tile)-1)*TILES_PER_ROW:TILES_PER_PAGE);  redraw=true; break;
        case KEY_HOME:  sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-(tile):-(tile%TILES_PER_PAGE));   redraw=true; break;
        case KEY_END:   sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?(TILE_PAGES)*(TILES_PER_PAGE)-tile-1:(TILES_PER_PAGE)-(tile%TILES_PER_PAGE)-1); redraw=true; break;
        case KEY_P:     sel_tile(tile,tile2,first,type,((getnumber("Goto Page", tile_page(tile))-tile_page(tile))*TILE_ROWS_PER_PAGE)*TILES_PER_ROW);  break;
        case KEY_O:
			if(type==0 && copy>=0) {
				if (key[KEY_LSHIFT] ||key[KEY_RSHIFT]) {
					mass_overlay_tile(min(tile,tile2),max(tile,tile2),copy,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]), rect_sel);
				}
				else {
					overlay_tile(tile,copy,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]));
					redraw=true;
				}
			}
			break;
        case KEY_E:     if(type==0) { edit_tile(tile,flip,cs); redraw=true; } break;
        case KEY_G:     if(type==0) { grab_tile(tile,cs); redraw=true; } break;
        case KEY_C:     copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; redraw=true; break;
        case KEY_X:     if(type==2) { ex=(ex+1)%3;}  break;
        //usetiles=true;
        case KEY_R:     if(type==2) break; go_tiles(); rotate_tile(tile,(key[KEY_LSHIFT] || key[KEY_RSHIFT])); redraw=true; saved=false; break;
        case KEY_SPACE: rect_sel=!rect_sel; copy=-1; redraw=true; break;
        //     case KEY_N:     go_tiles(); normalize(tile,tile2,flip); flip=0; redraw=true; saved=false; usetiles=true; break;
        case KEY_H:     flip^=1;
        go_tiles();
        if (type==0)
        {
          normalize(tile,tile2,rect_sel,flip);
          flip=0;
        }
        redraw=true;
        break;
        case KEY_F12:   onSnapshot(); break;
        case KEY_V:     if(copy==-1)
        {
          if(type!=2) {
            flip^=2;
            go_tiles();
            if (type==0)
            {
              normalize(tile,tile2,rect_sel,flip);
              flip=0;
            }
          }
        }
        else
        {
          go_tiles();
          saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,false);
        }
        redraw=true;
        break;
        case KEY_DEL:   delete_tiles(tile,tile2,rect_sel); redraw=true; break;
        case KEY_U:     comeback_tiles(); redraw=true; break;
        case KEY_M:     if((copy!=-1)&&(copy!=min(tile,tile2)))
        {
          go_tiles();
          saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,true);
        }
        else if (copy==-1)
        {
          //snuffles
          int combo2=0;
          int cs2=0;
          if (select_combo_2(combo2,cs2))
          {
            go_combos();
            int temptile=combobuf[combo2].tile;
            combobuf[combo2].tile=min(tile,tile2);
            if (edit_combo(combo2,false,cs2))
            {
              int frames=max(combobuf[combo2].frames,1);
              for (int i=1; i<(max(tile2,tile)-min(tile,tile2)+1)/frames; ++i)
              {
                memcpy(&(combobuf[combo2+i]), &(combobuf[combo2]), sizeof(newcombo));
                combobuf[combo2+i].tile+=(frames*i);
              }
              setup_combo_animations();
              setup_combo_animations2();
            }
            else
            {
              combobuf[combo2].tile=temptile;
            }
          }
        }
        redraw=true;
        break;
        case KEY_D:
        {
          int frames=1;
          char buf[80];
          sprintf(buf, "%d", frames);
          create_relational_tiles_dlg[0].dp2=lfont;
          create_relational_tiles_dlg[2].dp=buf;
          int ret=zc_popup_dialog(create_relational_tiles_dlg,2);
          if(ret==5)
          {
            frames=max(atoi(buf),1);
            bool same = true;
            int bitcheck=newtilebuf[tile].format;
            for (int t=1; t<frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?6:19); ++t)
            {
              if (newtilebuf[tile+t].format!=bitcheck) same = false;
            }
            if (!same)
            {
              jwin_alert("Error","The source tiles are not","in the same format.",NULL,"&OK",NULL,13,27,lfont);
              break;
            }
            if (tile+(frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?48:96))>NEWMAXTILES)
            {
              jwin_alert("Error","Too many tiles will be created",NULL,NULL,"&OK",NULL,13,27,lfont);
              break;
            }
            for (int i=frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?6:19); i<(frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?48:96)); ++i)
            {
              reset_tile(newtilebuf, tile+i, bitcheck);
            }
            if (create_relational_tiles_dlg[3].flags&D_SELECTED)
            {
              for (int i=create_relational_tiles_dlg[3].flags&D_SELECTED?47:95; i>0; --i)
              {
                for (int j=0; j<frames; ++j)
                {
                  merge_tiles(tile+(i*frames)+j, (tile+(relational_template[i][0]*frames)+j)<<2, ((tile+(relational_template[i][1]*frames)+j)<<2)+1, ((tile+(relational_template[i][2]*frames)+j)<<2)+2, ((tile+(relational_template[i][3]*frames)+j)<<2)+3);
                }
              }
            }
            else
            {
              for (int i=create_relational_tiles_dlg[3].flags&D_SELECTED?47:95; i>0; --i)
              {
                for (int j=0; j<frames; ++j)
                {
                  merge_tiles(tile+(i*frames)+j, (tile+(dungeon_carving_template[i][0]*frames)+j)<<2, ((tile+(dungeon_carving_template[i][1]*frames)+j)<<2)+1, ((tile+(dungeon_carving_template[i][2]*frames)+j)<<2)+2, ((tile+(dungeon_carving_template[i][3]*frames)+j)<<2)+3);
                }
              }
            }
          }
        }
        redraw=true;
        break;
        case KEY_B:
        {
          char buf[80];
          bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);
          bool control=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
          bool alt=(key[KEY_ALT] || key[KEY_ALTGR]);
          sprintf(buf, "Do you want to convert the selected %s to %d-bit color?", tile==tile2?"tile":"tiles",control?4:8);
          if(jwin_alert("Convert Tile?",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
          {
            go_tiles();
            saved=false;
            if (control)
            {
              memset(cset_reduce_table, 0, 256);
              memset(col_diff,0,3*128);
              calc_cset_reduce_table(RAMpal, cs);
            }
            if (rect_sel)
            {
              int top2=tile_row(min(tile, tile2));
              int left2=min(tile_col(tile), tile_col(tile2));
              int rows2=tile_row(max(tile, tile2))-top+1;
              int columns2=max(tile_col(tile), tile_col(tile2))-left+1;
              for (int r=top2; r<top2+rows2; ++r)
              {
                for (int c=left2; c<left2+columns2; ++c)
                {
                  int t=r*TILES_PER_ROW+c;
                  convert_tile(t, control?tf4Bit:tf8Bit, cs, shift, alt);
                }
              }
            }
            else
            {
              for (int t=min(tile,tile2); t<=max(tile,tile2); ++t)
              {
                convert_tile(t, control?tf4Bit:tf8Bit, cs, shift, alt);
              }
            }
          }
        }
        break;
      }
      clear_keybuf();
    }

    if(gui_mouse_b()&1)
    {
      int x=gui_mouse_x();
      int y=gui_mouse_y();
      if(y<208)
      {
        int t = (y>>4)*TILES_PER_ROW + (x>>4) + first;

        if(type==0 && (key[KEY_LSHIFT] || key[KEY_RSHIFT]))
        {
          tile2=t;
        }
        else
        {
          tile=tile2=t;
        }
        if (tile_clicked!=t)
        {
          dclick_status=DCLICK_NOT;
        }
        else if (dclick_status == DCLICK_AGAIN)
          {
            while(gui_mouse_b());
            if (((gui_mouse_y()>>4)*TILES_PER_ROW + (gui_mouse_x()>>4) + first)!=t)
            {
              dclick_status=DCLICK_NOT;
            }
            else
            {
              if(type==0)
              {
                edit_tile(tile,flip,cs);
                redraw=true;
              }
              else
              {
                done=2;
              }
            }
          }
          tile_clicked=t;
      }
      else if(x>300 && !bdown)
        {
          if(y<224 && first>0) { first-=TILES_PER_PAGE; redraw=true; }
          if(y>=224 && first<TILES_PER_PAGE*(TILE_PAGES-1)) { first+=TILES_PER_PAGE; redraw=true; }
          bdown=true;
        }
        if(type==1||type==2)
      {
        if(!bdown && isinRect(x,y,8,216,23,231))
          done=1;
        if(!bdown && isinRect(x,y,148,216,163,231))
          done=2;
      }
      else if(!bdown && isinRect(x,y,132,216,132+15,216+15))
        {
          rect_sel=!rect_sel;
          copy=-1;
          redraw=true;
        }
        else if(!bdown && isinRect(x,y,157,213,157+44,213+21))
          {
            if(do_button(157,213,44,21,"Grab",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
            {
              grab_tile(tile,cs);
              redraw=true;
            }
          }
          else if(!bdown && isinRect(x,y,202,213,202+44,213+21))
            {
              if(do_button(202,213,44,21,"Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
              {
                edit_tile(tile,flip,cs);
                redraw=true;
              }
            }
            else if(!bdown && isinRect(x,y,247,213,247+44,213+21))
              {
                if(do_button(247,213,44,21,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
                {
                  done=1;
                }
              }
              bdown=true;
    }

    bool r_click = false;
    if(gui_mouse_b()&2 && !bdown && type==0)
    {
      int x=gui_mouse_x()&0xFF0;
      int y=gui_mouse_y()&0xF0;
      if(y<208)
      {
        int t = (y>>4)*TILES_PER_ROW + (x>>4) + first;
        if(t<min(tile,tile2) || t>max(tile,tile2))
          tile=tile2=t;
      }
      bdown = r_click = true;
      f=8;
    }

    if(gui_mouse_b()==0)
      bdown=false;

 REDRAW:

    if ((f%16)==0)
      redraw=true;
    if(redraw)
      draw_tiles(first,cs);
    if(f&8)
    {
      if (rect_sel)
      {
        for(int i=min(tile_row(tile),tile_row(tile2))*TILES_PER_ROW+
            min(tile_col(tile),tile_col(tile2));
            i<=max(tile_row(tile),tile_row(tile2))*TILES_PER_ROW+
            max(tile_col(tile),tile_col(tile2)); i++)
        {
          if(i>=first && i<first+TILES_PER_PAGE &&
             tile_col(i)>=min(tile_col(tile),tile_col(tile2)) &&
             tile_col(i)<=max(tile_col(tile),tile_col(tile2)))
          {
            int x=(i%TILES_PER_ROW)<<4;
            int y=((i-first)/TILES_PER_ROW)<<4;
            rect(screen2,x,y,x+15,y+15,vc(15));
          }
        }
      }
      else
      {
        for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
        {
          if(i>=first && i<first+TILES_PER_PAGE)
          {
            int x=tile_col(i)<<4;
            int y=tile_row(i-first)<<4;
            rect(screen2,x,y,x+15,y+15,vc(15));
          }
        }
      }
    }
    if(type==0)
      tile_info_0(tile,tile2,cs,copy,copycnt,first/TILES_PER_PAGE,rect_sel);
    else
      tile_info_1(otile,oflip,ocs,tile,flip,cs,copy,first/TILES_PER_PAGE);

    if(type==2)
      textprintf_ex(screen,pfont,235,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Extend: %d",ex);

    ++f;

    if(r_click)
    {
      select_tile_rc_menu[1].flags = (copy==-1) ? D_DISABLED : 0;
      int m = popup_menu(select_tile_rc_menu,gui_mouse_x(),gui_mouse_y());
      redraw=true;
      switch(m)
      {
        case 0: copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; break;
        case 1: saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,false); break;
        case 2: delete_tiles(tile,tile2,rect_sel); break;
        case 4: edit_tile(tile,flip,cs); break;
        case 5: grab_tile(tile,cs); break;
        default: redraw=false; break;
      }
      r_click = false;
      goto REDRAW;
    }

  } while(!done);

  while(gui_mouse_b());
  comeback();
  register_blank_tiles();
  setup_combo_animations();
  setup_combo_animations2();
  return done-1;
}

int onTiles()
{
  static int t=0;
  int f=0;
  int c=CSet;
  select_tile(t,f,0,c,true);
  refresh(rALL);
  return D_O_K;
}

void draw_combo(int x,int y,int c,int cs)
{
  if(c<MAXCOMBOS)
    put_combo(screen2,x,y,c,cs,0,0);
  else
    rectfill(screen2,x,y,x+15,y+15,0);

}

void draw_combos(int page,int cs,bool cols)
{
  clear_bitmap(screen2);
  if(cols==false)
  {
    for(int i=0; i<256; i++)                                // 13 rows, leaving 32 pixels from y=208 to y=239
      draw_combo((i%COMBOS_PER_ROW)<<4,(i/COMBOS_PER_ROW)<<4,i+(page<<8),cs);
  }
  else
  {
    int c = 0;
    for(int i=0; i<256; i++)
    {
      draw_combo((i%COMBOS_PER_ROW)<<4,(i/COMBOS_PER_ROW)<<4,c+(page<<8),cs);
      ++c;
      if((i&3)==3)
        c+=48;
      if((i%COMBOS_PER_ROW)==(COMBOS_PER_ROW-1))
        c-=256;
    }
  }
  for(int x=64; x<320; x+=64)
  {
    _allegro_vline(screen2,x,0,207,vc(15));
  }
}

void combo_info(int tile,int tile2,int cs,int copy,int copycnt,int page,int buttons)
{
  jwin_draw_win(screen2,0, 208, 320, 32, FR_WIN);
  jwin_draw_frame(screen2,29,214,20,20,FR_DEEP);
  if(copy>=0)
  {
    put_combo(screen2,31,216,copy,cs,0,0);
    if(copycnt>1)
    {
      textprintf_right_ex(screen2,pfont,28,216,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d-",copy);
      textprintf_right_ex(screen2,pfont,24,224,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy+copycnt-1);
    }
    else
    {
      textprintf_right_ex(screen2,pfont,24,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy);
    }
  }
  else
  {
    for (int dy=0; dy<16; dy++)
    {
      for (int dx=0; dx<16; dx++)
      {
        screen2->line[216+dy][31+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
      }
    }
  }
  jwin_draw_frame(screen2,51,214,20,20,FR_DEEP);
  put_combo(screen2,53,216,tile,cs,0,0);

  if(tile>tile2)
  {
    swap(tile,tile2);
  }
  char buf[8];
  buf[0]=0;
  if(tile2!=tile)
  {
    sprintf(buf,"-%d",tile2);
  }

  textprintf_ex(screen2,pfont,73,216,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"combo:");
  textprintf_ex(screen2,pfont,73,224,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d%s",tile,buf);

  if (tile2==tile)
  {
    int nextcombo=combobuf[tile].nextcombo;
    int nextcset=combobuf[tile].nextcset;
    jwin_draw_frame(screen2,134,214,20,20,FR_DEEP);
    if (nextcombo>0)
    {
      put_combo(screen2,136,216,nextcombo,nextcset,0,0);
    }
    else
    {
      for (int dy=0; dy<16; dy++)
      {
        for (int dx=0; dx<16; dx++)
        {
          screen2->line[216+dy][136+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
        }
      }
    }
    textprintf_right_ex(screen2,pfont,132,216,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Cycle:");
    textprintf_right_ex(screen2,pfont,132,224,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",tile);//nextcombo);
  }


  if (buttons&1)
  {
    draw_button(screen2,157,213,44,21,"Grab",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
  }
  if (buttons&2)
  {
    draw_button(screen2,202,213,44,21,"Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
  }
  if (buttons&4)
  {
    draw_button(screen2,247,213,44,21,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
  }

  textprintf_ex(screen2,font,305,212,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"\x88");
  textprintf_ex(screen2,pfont,293,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"p:");
  textprintf_centre_ex(screen2,pfont,309,220,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",page);
  textprintf_ex(screen2,font,305,228,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"\x89");

  vsync();
  scare_mouse();
  blit(screen2,screen,0,0,0,0,320,240);
  unscare_mouse();
  SCRFIX();
}

void sel_combo(int &tile, int &tile2, int s, bool cols)
{
  int page = tile&0xFF00;
  tile &= 0xFF;

  if(!cols)
    tile += s;
  else
  {
    if(s==-COMBOS_PER_ROW)
      tile-=4;
    if(s==COMBOS_PER_ROW)
      tile+=4;
    if(s==-1)
      tile-=1;
    if(s==1)
      tile+=1;
  }
  /*
    if(s==1)
    {
    if((tile&3)==3)
    tile+=48;
    else
    ++tile;
    }
    if(s==-1)
    {
    if((tile&3)==0)
    tile-=48;
    else
    --tile;
    }
    }
    */
  bound(tile,0,255);
  tile += page;
  if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
    tile2 = tile;
}

word ctable[MAXCOMBOS];

int select_combo_2(int &tile,int &cs)
{
  reset_combo_animations();
  reset_combo_animations2();
  // static int tile=0;
  int page=tile>>8;
  int tile2=tile;
  int done=0;
  int tile_clicked=-1;
  int t2;
  // int cs = CSet;
  int copy=-1;
  int copycnt=0;
  position_mouse_z(0);

  for(int i=0; i<MAXCOMBOS; i++)
    //   for (int x=0; x<9; x++)
    combobuf[i].foo=i;

  go();
  draw_combos(page,cs,combo_cols);
  combo_info(tile,tile2,cs,copy,copycnt,page,4);
  while(gui_mouse_b());
  bool bdown=false;
  int f=0;

  do
  {
    rest(4);
    bool redraw=false;

    if (mouse_z<0)
    {
      if(page<COMBO_PAGES-1) { ++page; tile=tile2=(page<<8)+(tile&0xFF); }
      position_mouse_z(0);
      redraw=true;
    }
    else if (mouse_z>0)
      {
        if(page>0) { --page; tile=tile2=(page<<8)+(tile&0xFF); }
        position_mouse_z(0);
        redraw=true;
      }

      if(keypressed())
    {
      switch(readkey()>>8)
      {
        case KEY_DEL:   tile=0; done=2; break;
        case KEY_ENTER_PAD:
        case KEY_ENTER: done=2; break;
        case KEY_ESC:   done=1; break;
        case KEY_F1:    onHelp(); break;
        case KEY_SPACE: combo_cols=!combo_cols; redraw=true; break;
        case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  redraw=true; break;
        case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; redraw=true; break;
        case KEY_UP:    sel_combo(tile,tile2,-COMBOS_PER_ROW,combo_cols); redraw=true; break;
        case KEY_DOWN:  sel_combo(tile,tile2,COMBOS_PER_ROW,combo_cols);  redraw=true; break;
        case KEY_LEFT:  sel_combo(tile,tile2,-1,combo_cols);  redraw=true; break;
        case KEY_RIGHT: sel_combo(tile,tile2,1,combo_cols);   redraw=true; break;
        case KEY_PGUP:  if(page>0)             { --page;      tile=tile2=(page<<8)+(tile&0xFF); } redraw=true; break;
        case KEY_PGDN:  if(page<COMBO_PAGES-1) { ++page;      tile=tile2=(page<<8)+(tile&0xFF); } redraw=true; break;
        case KEY_P:
        {
          int choosepage=getnumber("Goto Page", page);
          page=(min(choosepage,COMBO_PAGES-1));  tile=tile2=(page<<8)+(tile&0xFF); redraw=true; break;
        }
      }
      clear_keybuf();
    }

    if(gui_mouse_b()&1)
    {
      int x=gui_mouse_x();
      int y=gui_mouse_y();
      if(y<208)
      {
        int t;
        if(!combo_cols)
        {
          t = (y>>4)*COMBOS_PER_ROW + (x>>4);
        }
        else
        {
          t = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
        }
        bound(t,0,255);
        t+=page<<8;
        tile=tile2=t;

        if (tile_clicked!=t)
        {
          dclick_status=DCLICK_NOT;
        }
        else if (dclick_status == DCLICK_AGAIN)
          {
            while(gui_mouse_b());
            if(!combo_cols)
            {
              t2 = (y>>4)*COMBOS_PER_ROW + (x>>4);
            }
            else
            {
              t2 = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
            }
            if (t2!=t)
            {
              dclick_status=DCLICK_NOT;
            }
            else
            {
              done=2;
            }
          }
          tile_clicked=t;












      }
      else if(x>300 && !bdown)
        {
          if(y<224 && page>0) { --page; redraw=true; }
          if(y>=224 && page<COMBO_PAGES-1) { ++page; redraw=true; }
          bdown=true;
        }

        if(!bdown && isinRect(x,y,247,213,247+44,213+21))
      {
        if(do_button(247,213,44,21,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
        {
          done=2;
        }
      }
      bdown=true;
    }

    bool r_click = false;
    if(gui_mouse_b()&2 && !bdown)
    {
      int x=gui_mouse_x();
      int y=gui_mouse_y();
      if(y<208)
      {
        int t;
        if(!combo_cols)
          t = (y>>4)*COMBOS_PER_ROW + (x>>4);
        else
          t = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
        bound(t,0,255);
        t+=page<<8;

        if(t<min(tile,tile2) || t>max(tile,tile2))
          tile=tile2=t;
      }
      bdown = r_click = true;
      f=8;
    }

    if(gui_mouse_b()==0)
      bdown=false;

    if(redraw)
      draw_combos(page,cs,combo_cols);
    combo_info(tile,tile2,cs,copy,copycnt,page,4);

    if(f&8)
    {
      int x,y;
      scare_mouse();
      for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
      {
        if((i>>8)==page)
        {
          int t=i&255;
          if(!combo_cols)
          {
            x=(t%COMBOS_PER_ROW)<<4;
            y=(t/COMBOS_PER_ROW)<<4;
          }
          else
          {
            x=((t&3) + ((t/52)<<2)) << 4;
            y=((t%52)>>2) << 4;
          }
          rect(screen,x,y,x+15,y+15,vc(15));
        }
      }
      unscare_mouse();
      SCRFIX();
    }
    ++f;

  } while(!done);

  for(int p=0; p<MAXCOMBOS; p+=256)
  {
    for(int i=0; i<256; i++)
    {
      int pos=0;
      for(int j=0; j<256; j++)
      {
        if(combobuf[j+p].foo==i+p)
        {
          pos=j+p;
          goto down;
        }
      }
   down:
      ctable[i+p]=pos;
    }
  }

  while(gui_mouse_b());
  comeback();
  setup_combo_animations();
  setup_combo_animations2();
  return done-1;
}

static DIALOG advpaste_dialog[] =
{
	/* (dialog proc)     (x)   (y)    (w)   (h)    (fg)      (bg)     (key)    (flags)       (d1)           (d2)      (dp) */
	{ jwin_win_proc,         0,    0,   200,  161,   vc(14),   vc(1),       0,     D_EXIT,       0,             0,        (void *) "Advanced Paste", NULL, NULL },
	{ jwin_button_proc,     27,   130,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       (void *) "O&K", NULL, NULL },
	{ jwin_button_proc,     112,  130,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },

	{ jwin_check_proc,		10,	   30,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,			(void*) "Tile", NULL, NULL },
	{ jwin_check_proc,		10,	   40,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,			(void*) "Cset2", NULL, NULL },
	{ jwin_check_proc,		10,	   50,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,			(void*) "Walkability", NULL, NULL },
	{ jwin_check_proc,		10,	   60,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,			(void*) "Animation", NULL, NULL },
	{ jwin_check_proc,		10,	   70,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,			(void*) "Type", NULL, NULL },
	{ jwin_check_proc,		10,	   80,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,			(void*) "Inherent Flag", NULL, NULL },

  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int advpaste(int tile, int tile2, int copy)
{
	int ret = zc_popup_dialog(advpaste_dialog,-1);
	if (ret!=1) return ret;

	// save original in case it's in paste range
	newcombo combo=combobuf[copy];

	for(int i=min(tile,tile2);i<=max(tile,tile2);++i)
	{
		if (advpaste_dialog[3].flags & D_SELECTED)  // tile
			{ combobuf[i].tile=combo.tile; combobuf[i].flip=combo.flip; }
		if (advpaste_dialog[4].flags & D_SELECTED)  // cset2
			{ combobuf[i].csets=combo.csets;}
		if (advpaste_dialog[5].flags & D_SELECTED)  // walk
			{ combobuf[i].csets=combo.csets;}
		if (advpaste_dialog[6].flags & D_SELECTED)  // anim
			{ combobuf[i].frames=combo.frames; combobuf[i].speed=combo.speed; combobuf[i].nextcombo=combo.nextcombo;combobuf[i].nextcset=combo.nextcset;combobuf[i].skipanim=combo.skipanim;combobuf[i].nexttimer=combo.nexttimer;combobuf[i].skipanimy=combo.skipanimy;}
		if (advpaste_dialog[7].flags & D_SELECTED)  // type
			{ combobuf[i].type=combo.type;}
		if (advpaste_dialog[8].flags & D_SELECTED)  // flag
			{ combobuf[i].flag=combo.flag;}
	}

	return ret;
}

int combo_screen()
{
  reset_combo_animations();
  reset_combo_animations2();
  static int tile=0;
  static int page=0;
  int tile2=tile;
  int done=0;
  int cs = CSet;
  int copy=-1;
  int copycnt=0;

  int tile_clicked=-1;
  int t2;

  bool masscopy;

  select_tile_rc_menu[5].text = NULL;

  for(int i=0; i<MAXCOMBOS; i++)
    //   for (int x=0; x<9; x++)
    combobuf[i].foo=i;

  go();
  draw_combos(page,cs,combo_cols);
  combo_info(tile,tile2,cs,copy,copycnt,page,6);
  go_combos();
  position_mouse_z(0);
  while(gui_mouse_b());
  bool bdown=false;
  int f=0;

  do
  {
    rest(4);
    bool redraw=false;

    if (mouse_z<0)
    {
      if(page<COMBO_PAGES-1) { ++page; tile=tile2=(page<<8)+(tile&0xFF); }
      position_mouse_z(0);
      redraw=true;
    }
    else if (mouse_z>0)
      {
        if(page>0) { --page; tile=tile2=(page<<8)+(tile&0xFF); }
        position_mouse_z(0);
        redraw=true;
      }

      if(keypressed())
    {
      switch(readkey()>>8)
      {
        case KEY_ENTER_PAD:
        case KEY_ENTER: done=2; break;
        case KEY_ESC:   done=1; break;
        case KEY_F1:    onHelp(); break;
        case KEY_SPACE: combo_cols=!combo_cols; redraw=true; break;
        case KEY_PLUS_PAD:
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          for (int i=min(tile,tile2); i<=max(tile,tile2); ++i)
          {
            if(combobuf[i].tile==NEWMAXTILES-1)
            {
              combobuf[i].tile=0;
            }
            else
            {
              ++combobuf[i].tile;
            }
          }
          redraw=true;
        }
        else
        {
          cs = (cs<11)  ? cs+1:0;
          redraw=true;
        }
        break;
        case KEY_MINUS_PAD:
        if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
        {
          for (int i=min(tile,tile2); i<=max(tile,tile2); ++i)
          {
            if(combobuf[i].tile==0)
            {
              combobuf[i].tile=NEWMAXTILES-1;
            }
            else
            {
              --combobuf[i].tile;
            }
          }
          redraw=true;
        }
        else
        {
          cs = (cs>0)  ? cs-1:11;
          redraw=true;
        }
        break;
        case KEY_UP:    sel_combo(tile,tile2,-COMBOS_PER_ROW,combo_cols); redraw=true; break;
        case KEY_DOWN:  sel_combo(tile,tile2,COMBOS_PER_ROW,combo_cols);  redraw=true; break;
        case KEY_LEFT:  sel_combo(tile,tile2,-1,combo_cols);  redraw=true; break;
        case KEY_RIGHT: sel_combo(tile,tile2,1,combo_cols);   redraw=true; break;
        case KEY_PGUP:  if(page>0)             { --page;      tile=tile2=(page<<8)+(tile&0xFF); } redraw=true; break;
        case KEY_PGDN:  if(page<COMBO_PAGES-1) { ++page;      tile=tile2=(page<<8)+(tile&0xFF); } redraw=true; break;
        case KEY_A:     { tile=(page<<8); tile2=(page<<8)+(0xFF); } redraw=true; break;
        case KEY_P:
        {
          int choosepage = getnumber("Goto Page", page);
          page=(min(choosepage,COMBO_PAGES-1));  tile=tile2=(page<<8)+(tile&0xFF); redraw=true; break;
        }
        case KEY_U:     comeback_combos(); redraw=true; break;
        case KEY_E:     go_combos(); edit_combo(tile,false,cs); redraw=true; setup_combo_animations(); setup_combo_animations2(); break;
        case KEY_C:     go_combos(); copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; redraw=true; break;

        case KEY_H:     for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
		{
			combobuf[i].flip^=1;
			byte w=combobuf[i].walk;
			combobuf[i].walk=(w& ~3)>>2 | (w&3)<<2;
			w=combobuf[i].csets;
			combobuf[i].csets=(w& ~0x50)>>1 | (w&0x50)<<1;
		}
        redraw=true;
        saved=false;
        break;

        case KEY_M:     if((copy!=-1)&&(copy!=min(tile,tile2)))
        {
          move_combos(tile,tile2,copy,copycnt);
          saved=false;
        }
        redraw=true;
        break;
        case KEY_S:     tile=tile2=min(tile,tile2);
        if(copy>=0 && tile!=copy)
        {
          go_combos();
          for(int i=0; i<copycnt; i++)
            swap(combobuf[copy+i],combobuf[tile+i]);
          saved=false;
          setup_combo_animations();
		  setup_combo_animations2();
        }
        redraw=true;
        copy=-1;
        break;

        case KEY_V:
		if (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] && copy != -1)
		{
			if (advpaste(tile, tile2, copy)==1)
				{saved=false; redraw=true; copy=-1;}
			break;
		}

		masscopy=(key[KEY_LSHIFT] || key[KEY_RSHIFT])?1:0;
		if(copy==-1)
        {
          go_combos();
          for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
		  {
			combobuf[i].flip^=2;
			byte w=combobuf[i].walk;
			combobuf[i].walk=(w&5)<<1 | (w& ~5)>>1;
			w=combobuf[i].csets;
			combobuf[i].csets=(w&0x30)<<2 | (w& ~0x30)>>2;
		  }
          saved=false;
        }
        else
        {
          go_combos();
          copy_combos(tile,tile2,copy,copycnt,masscopy);
          setup_combo_animations();
		  setup_combo_animations2();
          saved=false;
        }
        redraw=true;
        break;
		case KEY_I:
		{
			// insert 1 combo
			go_combos();

			int z=tile;
			tile=min(tile,tile2);
			tile2=MAXCOMBOS;
			copy=tile+1;
			copycnt=MAXCOMBOS-tile;

			if (key[KEY_LSHIFT]||key[KEY_RSHIFT])
				move_combos(tile,tile2,copy, copycnt);
			else
				move_combos(copy,tile2,tile, copycnt);

			copy=-1;
			tile2=tile=z;
			setup_combo_animations();
			setup_combo_animations2();
			redraw=true;
			saved=false;
		}
		break;
        case KEY_DEL:
        {
          char buf[40];
          if(tile==tile2)
            sprintf(buf,"Delete combo %d?",tile);
          else
            sprintf(buf,"Delete combos %d-%d?",min(tile,tile2),max(tile,tile2));

          if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
          {
            go_combos();
            for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
              clear_combo(i);
            tile=tile2=min(tile,tile2);
            redraw=true;
            saved=false;
            setup_combo_animations();
			setup_combo_animations2();
          }
        } break;
      }
      clear_keybuf();
    }

    if(gui_mouse_b()&1)
    {
      int x=gui_mouse_x();
      int y=gui_mouse_y();
      if(y<208)
      {
        int t;
        if(!combo_cols)
        {
          t = (y>>4)*COMBOS_PER_ROW + (x>>4);
        }
        else
        {
          t = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
        }
        bound(t,0,255);
        t+=page<<8;

        if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
        {
          tile2=t;
        }
        else
        {
          tile=tile2=t;
        }




        if (tile_clicked!=t)
        {
          dclick_status=DCLICK_NOT;
        }
        else if (dclick_status == DCLICK_AGAIN)
          {
            while(gui_mouse_b());
            if(!combo_cols)
            {
              t2 = (y>>4)*COMBOS_PER_ROW + (x>>4);
            }
            else
            {
              t2 = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
            }
            bound(t2,0,255);
            t2+=page<<8;
            if (t2!=t)
            {
              dclick_status=DCLICK_NOT;
            }
            else
            {
              go_combos();
              edit_combo(tile,false,cs);
              redraw=true;
              setup_combo_animations();
			  setup_combo_animations2();
            }
          }
          tile_clicked=t;




      }
      else if(x>300 && !bdown)
        {
          if(y<224 && page>0) { --page; redraw=true; }
          if(y>=224 && page<COMBO_PAGES-1) { ++page; redraw=true; }
          bdown=true;
        }

        if(!bdown && isinRect(x,y,202,213,202+44,213+21))
      {
        if(do_button(202,213,44,21,"Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
        {
          edit_combo(tile,false,cs);
          redraw=true;
        }
      }
      else if(!bdown && isinRect(x,y,247,213,247+44,213+21))
        {
          if(do_button(247,213,44,21,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
          {
            done=1;
          }
        }
        bdown=true;
    }

    bool r_click = false;
    if(gui_mouse_b()&2 && !bdown)
    {
      int x=gui_mouse_x();
      int y=gui_mouse_y();
      if(y<208)
      {
        int t;
        if(!combo_cols)
          t = (y>>4)*COMBOS_PER_ROW + (x>>4);
        else
          t = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
        bound(t,0,255);
        t+=page<<8;

        if(t<min(tile,tile2) || t>max(tile,tile2))
          tile=tile2=t;
      }
      bdown = r_click = true;
      f=8;
    }

 REDRAW:

    if(gui_mouse_b()==0)
      bdown=false;

    if(redraw)
      draw_combos(page,cs,combo_cols);
    combo_info(tile,tile2,cs,copy,copycnt,page,6);

    if(f&8)
    {
      int x,y;
      scare_mouse();
      for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
      {
        if((i>>8)==page)
        {
          int t=i&255;
          if(!combo_cols)
          {
            x=(t%COMBOS_PER_ROW)<<4;
            y=(t/COMBOS_PER_ROW)<<4;
          }
          else
          {
            x=((t&3) + ((t/52)<<2)) << 4;
            y=((t%52)>>2) << 4;
          }
          rect(screen,x,y,x+15,y+15,vc(15));
        }
      }
      unscare_mouse();
      SCRFIX();
    }
    ++f;

    if(r_click)
    {
      select_tile_rc_menu[1].flags = (copy==-1) ? D_DISABLED : 0;
      int m = popup_menu(select_tile_rc_menu,gui_mouse_x(),gui_mouse_y());
      redraw=true;
      switch(m)
      {
        case 0: go_combos(); copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; break;
        case 1:
        if(min(tile,tile2)!=copy)
        {
          go_combos();
          tile=tile2=min(tile,tile2);
          for(int i=0; i<copycnt; i++)
            combobuf[tile+i]=combobuf[copy+i];
          saved=false;
        }
        copy=-1;
        break;

        case 2:
        {
          char buf[40];
          if(tile==tile2)
            sprintf(buf,"Delete combo %d?",tile);
          else
            sprintf(buf,"Delete combos %d-%d?",min(tile,tile2),max(tile,tile2));

          if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
          {
            go_combos();
            for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
              clear_combo(i);
            tile=tile2=min(tile,tile2);
            redraw=true;
            saved=false;
          }
        }
        break;

        case 4: go_combos(); edit_combo(tile,false,cs); break;
        default: redraw=false; break;
      }
      r_click = false;
      goto REDRAW;
    }

  } while(!done);

  while(gui_mouse_b());
  comeback();
  setup_combo_animations();
  setup_combo_animations2();
  return done-1;
}

int onCombos()
{
  // reset_combo_animations();
  combo_screen();
  // setup_combo_animations();
  refresh(rALL);
  return D_O_K;
}

int edit_combo_cset;

int d_ctile_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  d=d;
  c=c;

  if (msg==MSG_CLICK)
  {
    int t=curr_combo.tile;
    int f=curr_combo.flip;
    if(select_tile(t,f,1,edit_combo_cset,true))
    {
      curr_combo.tile=t;
      curr_combo.flip=f;
      return D_REDRAW;
    }
  }
  return D_O_K;
}

int d_combo_loader(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  if(msg==MSG_DRAW)
  {
    newcombo *hold = combobuf;
    combobuf = &curr_combo;
    putcombo(combo_bmp,0,0,0,edit_combo_cset);
    combobuf = hold;

    textprintf_ex(screen,font,d->x,d->y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Tile:");
    textprintf_ex(screen,font,d->x+50,d->y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",curr_combo.tile);
    textprintf_ex(screen,font,d->x,d->y+8,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Flip:");
    textprintf_ex(screen,font,d->x+50,d->y+8,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",curr_combo.flip);
    textprintf_ex(screen,font,d->x,d->y+24,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet2:");

  }
  return D_O_K;
}

combotype_struct bict[cMAX];
int bict_cnt=-1;

void build_bict_list()
{
  bict[0].s = "(None)";
  bict[0].i = 0;
  bict_cnt=1;

  for(int i=1; i<cMAX; i++)
  {
    if(combotype_string[i][0]!='-')
    {
      bict[bict_cnt].s = combotype_string[i];
      bict[bict_cnt].i = i;
      ++bict_cnt;
    }
  }

  for(int i=1; i<bict_cnt-1; i++)
    for(int j=i+1; j<bict_cnt; j++)
      if(stricmp(bict[i].s,bict[j].s)>0)
        swap(bict[i],bict[j]);

}

char *combotypelist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size = bict_cnt;
    return NULL;
  }
  return bict[index].s;
}

int onCmb_dlg_h();
int onCmb_dlg_v();
int onCmb_dlg_r();

int click_d_ctile_proc()
{
  d_ctile_proc(MSG_CLICK,NULL,0);
  return D_REDRAW;
}

int click_d_combo_proc();

static DIALOG combo_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     54,   21,   241,  184,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Edit Combo", NULL, NULL },
  { jwin_button_proc,     105,  180,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { jwin_button_proc,     185,  180,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp, NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'h',     0,          0,             0,       (void *) onCmb_dlg_h, NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'v',     0,          0,             0,       (void *) onCmb_dlg_v, NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'r',     0,          0,             0,       (void *) onCmb_dlg_r, NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      't',     0,          0,             0,       (void *) click_d_ctile_proc, NULL, NULL },
  // 8
  { d_combo_loader,    60,   48,   0,    0,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       NULL, NULL, NULL },
  { jwin_frame_proc,   158,  46,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL, NULL, NULL },
  { d_bitmap_proc,     160,  48,   16,   16,   0,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
  { d_ctile_proc,      160,  48,   16,   16,   0,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
  { jwin_numedit_proc, 102+5,  68,   21,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  // 13
  { jwin_frame_proc,   190,  46,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL, NULL, NULL },
  { d_wflag_proc,      192,  48,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  { d_wflag_proc,      192,  56,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  { d_wflag_proc,      200,  48,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  { d_wflag_proc,      200,  56,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  // 188
  { jwin_frame_proc,   222,  46,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL, NULL, NULL },
  { d_wflag_proc,      224,  48,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  { d_wflag_proc,      232,  48,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  { d_wflag_proc,      224,  56,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  { d_wflag_proc,      232,  56,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL, NULL, NULL },
  // 23
  { jwin_text_proc,       60,   126,  48,   8,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) "Type:", NULL, NULL },
  { jwin_droplist_proc,   89,   122,  200,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       NULL, NULL, NULL },
  { jwin_text_proc,       60,   90,   72,   8,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          2,             0,       (void *) "A.Frames:", NULL, NULL },
  { jwin_text_proc,       60,   108,   64,   8,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          2,             0,       (void *) "A.Speed:", NULL, NULL },
  { jwin_numedit_proc,   102+5,  86,   26,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_numedit_proc,   102+5,  104,   26,   16,    vc(12),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
  { jwin_text_proc,       192,  71,   40,   8,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) "Cycle:", NULL, NULL },
  // 30
  { jwin_frame_proc,   190,  79,   20,   20,   0,       0,      0,       0,             FR_DEEP,       0,       NULL, NULL, NULL },
  { d_combo_proc,      192,  81,   16,   16,   0,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
  { jwin_text_proc,       60,   144,  48,   8,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) "Flag:", NULL, NULL },
  { jwin_droplist_proc,   89,   140,  200,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          0,             0,       (void *) flaglist, NULL, NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'n',     0,          0,             0,       (void *) click_d_combo_proc, NULL, NULL },
  { jwin_text_proc,       140,   108,   40,   8,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          2,             0,       (void *) "A.SkipX:", NULL, NULL },
  { jwin_numedit_proc,   180,  104,   26,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  { jwin_text_proc,       210,   108,   40,   8,    jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          2,             0,       (void *) "A.SkipY:", NULL, NULL },
  { jwin_numedit_proc,   250,  104,   26,   16,    vc(12),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
  { jwin_check_proc,       60,   160,  168,   8+1,    vc(15),  vc(1),  0,       0,          1,             0,       (void *) "Refresh Animation on Room Entry", NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int click_d_combo_proc()
{
  if (key[KEY_LCONTROL]||key[KEY_RCONTROL])
  {
    nextcombo_fake_click=2;
  }
  else
  {
    nextcombo_fake_click=1;
  }
  d_combo_proc(MSG_CLICK,combo_dlg+31,0);
  nextcombo_fake_click=0;
  return D_REDRAW;
}

int onCmb_dlg_h()
{
  curr_combo.flip^=1;

  swap(combo_dlg[14].flags, combo_dlg[16].flags);
  swap(combo_dlg[15].flags, combo_dlg[17].flags);
  swap(combo_dlg[19].flags, combo_dlg[20].flags);
  swap(combo_dlg[21].flags, combo_dlg[22].flags);

  for(int i=0; i<4; i++)
    if(combo_dlg[i+14].flags & D_SELECTED)
      curr_combo.walk |= 1<<i;
    else
      curr_combo.walk &= ~(1<<i);
  curr_combo.csets &= 15;
  for(int i=0; i<4; i++)
    if(combo_dlg[i+19].flags & D_SELECTED)
      curr_combo.csets |= 16<<i;

    return D_REDRAW;
}

int onCmb_dlg_v()
{
  curr_combo.flip^=2;

  swap(combo_dlg[14].flags, combo_dlg[15].flags);
  swap(combo_dlg[16].flags, combo_dlg[17].flags);
  swap(combo_dlg[19].flags, combo_dlg[21].flags);
  swap(combo_dlg[20].flags, combo_dlg[22].flags);

  for(int i=0; i<4; i++)
    if(combo_dlg[i+6].flags & D_SELECTED)
      curr_combo.walk |= 1<<i;
    else
      curr_combo.walk &= ~(1<<i);
  curr_combo.csets &= 15;
  for(int i=0; i<4; i++)
    if(combo_dlg[i+15].flags & D_SELECTED)
      curr_combo.csets |= 16<<i;

    return D_REDRAW;
}

int onCmb_dlg_r()
{
  curr_combo.flip=rotate_value(curr_combo.flip);

  swap(combo_dlg[14].flags, combo_dlg[16].flags);
  swap(combo_dlg[14].flags, combo_dlg[17].flags);
  swap(combo_dlg[14].flags, combo_dlg[15].flags);
  swap(combo_dlg[19].flags, combo_dlg[20].flags);
  swap(combo_dlg[19].flags, combo_dlg[22].flags);
  swap(combo_dlg[19].flags, combo_dlg[21].flags);

  for(int i=0; i<4; i++)
    if(combo_dlg[i+14].flags & D_SELECTED)
      curr_combo.walk |= 1<<i;
    else
      curr_combo.walk &= ~(1<<i);
  curr_combo.csets &= 15;
  for(int i=0; i<4; i++)
    if(combo_dlg[i+19].flags & D_SELECTED)
      curr_combo.csets |= 16<<i;

    return D_REDRAW;
}

bool edit_combo(int c,bool freshen,int cs)
{
  combo_dlg[0].dp2=lfont;
  if (bict_cnt==-1)
  {
    build_bict_list();
  }
  reset_combo_animations();
  reset_combo_animations2();

  curr_combo = combobuf[c];

  char cset_str[8];
  char frm[8];
  char spd[8];
  char skip[8];
  char skipy[8];

  char csets = curr_combo.csets & 15;

  combo_dlg[10].dp=combo_bmp;
  if(csets&8) //if csets>8, then it's a negative.
  {
    csets |= 0xF0;
  }

  sprintf(cset_str,"%d",csets);
  sprintf(frm,"%d",curr_combo.frames);
  sprintf(spd,"%d",curr_combo.speed);
  sprintf(skip,"%d",curr_combo.skipanim);
  sprintf(skipy,"%d",curr_combo.skipanimy);
  combo_dlg[12].dp = cset_str;

  for(int i=0; i<4; i++)
  {
    combo_dlg[i+14].flags = curr_combo.walk&(1<<i) ? D_SELECTED : 0;
  }
  for(int i=0; i<4; i++)
  {
    combo_dlg[i+19].flags = curr_combo.csets&(16<<i) ? D_SELECTED : 0;
  }

  combo_dlg[27].dp = frm;
  combo_dlg[28].dp = spd;
  combo_dlg[31].d1 = curr_combo.nextcombo;
  combo_dlg[31].fg = curr_combo.nextcset;
  combo_dlg[33].d1 = curr_combo.flag;
  combo_dlg[36].dp = skip;
  combo_dlg[38].dp = skipy;

  combo_dlg[39].flags = curr_combo.freshanim ? D_SELECTED : 0;

  int index=0;

  for(int j=0; j<bict_cnt; j++)
  {
    if(bict[j].i == curr_combo.type)
    {
      index=j;
    }
  }

  combo_dlg[24].d1 = index; //*
  combo_dlg[24].dp = (void *) combotypelist; //*
                            //  combo_dlg[1].fg = cs;
  edit_combo_cset = cs;
  int ret=zc_popup_dialog(combo_dlg,4);
  if(freshen)
  {
    refresh(rALL);
  }

  if(ret==1)
  {
    saved=false;
    curr_combo.csets = csets;
    for(int i=0; i<4; i++)
    {
      if(combo_dlg[i+14].flags & D_SELECTED)
      {
        curr_combo.walk |= 1<<i;
      }
      else
      {
        curr_combo.walk &= ~(1<<i);
      }
    }
    curr_combo.csets = atoi(cset_str) & 15;
    for(int i=0; i<4; i++)
    {
      if(combo_dlg[i+19].flags & D_SELECTED)
      {
        curr_combo.csets |= 16<<i;
      }
      else
      {
        curr_combo.csets &= ~(16<<i);
      }
    }

    curr_combo.frames = atoi(frm);
    curr_combo.speed = atoi(spd);
	curr_combo.skipanim = max(0,atoi(skip));
	curr_combo.skipanimy = max(0,atoi(skipy));
    curr_combo.type = bict[combo_dlg[24].d1].i;
    curr_combo.nextcombo = combo_dlg[31].d1;
    curr_combo.nextcset = combo_dlg[31].fg;
    curr_combo.flag = combo_dlg[33].d1;

	curr_combo.freshanim = combo_dlg[39].flags & D_SELECTED ? 1 : 0;
    combobuf[c] = curr_combo;
  }

  if(freshen)
  {
    refresh(rALL);
  }

  setup_combo_animations();
  setup_combo_animations2();
  return (ret==1);
}

int d_itile_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  switch(msg)
  {
    case MSG_CLICK:
    {
      int cs = d->d2;
      int f  = 0;
      if(select_tile(d->d1,f,1,cs,true))
        return D_REDRAW;
    }
    break;

    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,20,20);
    if(buf)
    {
      clear_to_color(buf, d->bg);
      jwin_draw_frame(buf,0, 0, 20, 20, FR_DEEP);
      overtile16(buf,d->d1,2,2,d->fg,0);
      blit(buf,screen,0,0,d->x,d->y,20,20);
      destroy_bitmap(buf);
    }
    break;
  }
  return D_O_K;
}

static DIALOG icon_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc, 80,   80,   160,  84,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Game Icons", NULL, NULL },
  { d_dummy_proc,      160,  88,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL, NULL, NULL },
  { d_itile_proc,      108+3,  108,  20,   20,   0,       0,      0,       0,          0,             6,       NULL, NULL, NULL },
  { d_itile_proc,      138-1,  108,  20,   20,   0,       0,      0,       0,          0,             7,       NULL, NULL, NULL },
  { d_itile_proc,      168-5,  108,  20,   20,   0,       0,      0,       0,          0,             8,       NULL, NULL, NULL },
  { d_itile_proc,      198-9,  108,  20,   20,   0,       0,      0,       0,          0,             9,       NULL, NULL, NULL },
  { jwin_button_proc,     90,   136,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK", NULL, NULL },
  { jwin_button_proc,     170,  136,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel", NULL, NULL },
  { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int onIcons()
{
  PALETTE pal;
  //  pal = RAMpal;
  memcpy(pal,RAMpal,sizeof(RAMpal));
  icon_dlg[0].dp2=lfont;
  for(int i=0; i<4; i++)
  {
    icon_dlg[i+2].d1 = misc.icons[i];
    icon_dlg[i+2].fg = i+6;
    load_cset(pal, i+6, pSprite(i+spICON1));
  }

  set_palette(pal);
  int ret = zc_popup_dialog(icon_dlg,7);

  if(ret==6)
  {
    for(int i=0; i<4; i++)
    {
      if(misc.icons[i] != icon_dlg[i+2].d1)
      {
        misc.icons[i] = icon_dlg[i+2].d1;
        saved=false;
      }
    }
  }

  set_palette(RAMpal);
  return D_O_K;
}

void center_zq_tiles_dialogs()
{
  jwin_center_dialog(combo_dlg);
  jwin_center_dialog(create_relational_tiles_dlg);
  jwin_center_dialog(icon_dlg);
  jwin_center_dialog(leech_dlg);
  jwin_center_dialog(tile_move_list_dlg);
}

int d_combo_proc(int msg,DIALOG *d,int c)
{
  //these are here to bypass compiler warnings about unused arguments
  c=c;

  switch(msg)
  {
    case MSG_CLICK:
    {
      int combo2;
      int cs;
	  if(key[KEY_LSHIFT])
	  {
	    if(gui_mouse_b()&1)
		{
	      d->d1++;
		  if(d->d1>=MAXCOMBOS) d->d1=0;
		}
		else if(gui_mouse_b()&2)
		{
		  d->d1--;
		  if(d->d1<0) d->d1=MAXCOMBOS-1;
		}
		return D_REDRAW;
	  }
	  else if(key[KEY_RSHIFT])
	  {
	    if(gui_mouse_b()&1)
		{
	      d->fg++;
		  if(d->fg>11) d->fg=0;
		}
		else if(gui_mouse_b()&2)
		{
		  d->fg--;
		  if(d->fg<0) d->fg=11;
		}
		return D_REDRAW;
	  }
	  else if(key[KEY_ALT])
	  {
	    if(gui_mouse_b()&1)
		{
	      d->d1 = Combo;
		  d->fg = CSet;
		}
		return D_REDRAW;
	  }
      else if (gui_mouse_b()&2||nextcombo_fake_click==2) //right mouse button
      {
        if (d->d1==0&&d->fg==0&&!(gui_mouse_b()&1))
        {
          return D_O_K;
        }
        d->d1=0;
        d->fg=0;
		return D_REDRAW;
      }
      else if (gui_mouse_b()&1||nextcombo_fake_click==1) //left mouse button
      {
        combo2=d->d1;
        cs=d->fg;
        if(select_combo_2(combo2, cs))
        {
          d->d1=combo2;
          d->fg=cs;
        }
        return D_REDRAW;
      }
      else
      {
        return D_REDRAWME;
      }
    }

    break;

    case MSG_DRAW:
    BITMAP *buf = create_bitmap_ex(8,16,16);
    if(buf)
    {
      clear_bitmap(buf);
      if(d->d1)
        putcombo(buf,0,0,d->d1,d->fg);
      blit(buf,screen,0,0,d->x,d->y,d->w,d->h);
      destroy_bitmap(buf);
    }
    break;
  }
  return D_O_K;
}

