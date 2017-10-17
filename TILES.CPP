//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  tiles.cc
//
//  Tile drawing routines for ZC.
//
//  !! Don't use them on "screen"; use memory BITMAPs only.
//
//--------------------------------------------------------

#include "zc_alleg.h"
#include <string.h>

#include "zdefs.h"
#include "tiles.h"

extern RGB_MAP rgb_table;
extern COLOR_MAP trans_table;
extern byte *tilebuf;
extern newcombo *combobuf;
extern word animated_combo_table[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern word animated_combo_table4[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern word animated_combos;
extern bool blank_tile_table[NEWMAXTILES];                  //keeps track of blank tiles
extern bool blank_tile_quarters_table[NEWMAXTILES*4];       //keeps track of blank tiles
extern fix  LinkModifiedX();
extern fix  LinkModifiedY();

byte unpackbuf[256];

bool isblanktile(int i)
{
  byte *tilestart=tilebuf+(i*128);
  qword *di=(qword*)tilestart;
  for (int j=0; j<16; ++j, ++di)
  {
    if (*di!=0)
    {
      return false;
    }
  }
  return true;
}

void register_blank_tile_quarters(int tile)
{
  byte *tilestart=tilebuf+(tile*128);
  dword *di=(dword*)tilestart;
  blank_tile_quarters_table[(tile<<2)]=true;
  blank_tile_quarters_table[(tile<<2)+1]=true;
  blank_tile_quarters_table[(tile<<2)+2]=true;
  blank_tile_quarters_table[(tile<<2)+3]=true;

  for (int i=0; i<4; ++i)
  {
    for (int j=0; j<32; ++j, ++di)
    {
      if (*di!=0)
      {
        blank_tile_quarters_table[(tile<<2)+((j>15)?2:0)+((j&1)?1:0)]=false;
      }
    }
  }

}

void register_blank_tiles()
{
  for (int i=0; i<NEWMAXTILES; ++i)
  {
    register_blank_tile_quarters(i);
    blank_tile_table[i]=isblanktile(i);
  }
}

//returns the number of tiles
word count_tiles(byte *buf)
{
  word tiles_used;
  bool used;
  int x;
  for(tiles_used=(NEWMAXTILES); tiles_used>0; --tiles_used)
  {
    used=false;
    for (x=0; x<128; ++x)
    {
      used=used || (buf[(tiles_used-1)*128+x]!=0);
    }
    if (used)
    {
      break;
    }
  }
  return tiles_used;
}

//returns the number of combos
word count_combos()
{
  word combos_used;
  for (combos_used=MAXCOMBOS; combos_used>0; --combos_used)
  {
    if (combobuf[combos_used-1].tile!=0)
    {
      break;
    }
  }
  return combos_used;
}

void setup_combo_animations()
{
  memset(animated_combo_table, 0, MAXCOMBOS*2*2);
  memset(animated_combo_table4, 0, MAXCOMBOS*2*2);
  int y=0;
  for (word x=0; x<MAXCOMBOS; ++x)
  {
    animated_combo_table[x][0]=y;
    animated_combo_table[x][1]=combobuf[x].tile;
    if (combobuf[x].frames>1)
    {
      animated_combo_table4[y][0]=x;
      animated_combo_table4[y][1]=0;
      ++y;
    }
  }
  animated_combos=y;
}

void reset_combo_animation(int c)
{
  for (word x=0; x<animated_combos; ++x)
  {
    int y=animated_combo_table4[x][0];                      //combo number
    if (y==c)
    {
      combobuf[y].tile=animated_combo_table[y][1];        //reset tile
      animated_combo_table4[x][1]=0;                        //reset clock
      return;
    }
  }
}

void reset_combo_animations()
{
  for (word x=0; x<animated_combos; ++x)
  {
    combobuf[animated_combo_table4[x][0]].tile=animated_combo_table[animated_combo_table4[x][0]][1];
  }
}

extern void update_combo_cycling();

void animate_combos()
{
  update_combo_cycling();
  for (word x=0; x<animated_combos; ++x)
  {
    int y=animated_combo_table4[x][0];                      //combo number
    if (animated_combo_table4[x][1]>=combobuf[y].speed)     //time to animate
    {
      if (combobuf[y].tile-combobuf[y].frames>=animated_combo_table[y][1]-1)
      {
        combobuf[y].tile=animated_combo_table[y][1];        //reset tile
      }
      else
      {
        ++combobuf[y].tile;                                 //increment tile
      }
      animated_combo_table4[x][1]=0;                        //reset clock
    }
    else
    {
      ++animated_combo_table4[x][1];                        //increment clock
    }
  }
}

//clears the tile buffer
void clear_tiles(byte *buf)
{
  dword i=0;
  for( ; i<NEWTILE_SIZE2/4; ++i)
    ((dword*)buf)[i] = 0;
}

// unpacks from tilebuf to unpackbuf
void unpack_tile(int tile, int flip, bool force)
{
  static byte *si, *di, *oldtilebuf=tilebuf;
  static int i, j, oldtile=-5, oldflip=-5;
  if (tile==oldtile&&(flip&5)==(oldflip&5)&&oldtilebuf==tilebuf&&!force)
  {
    return;
  }
  oldtile=tile;
  oldflip=flip;
  oldtilebuf=tilebuf;
  switch (flip&5)
  {
    case 1:  //horizontal
      si = tilebuf+((tile+1)<<7);
      for(i=15; i>=0; --i)
      {
        di=unpackbuf + (i<<4) - 1;
        for(j=7; j>=0; --j)
        {
          (*(++di)) = (*(--si)) >> 4;
          (*(++di)) = (*si) & 15;
        }
      }
      break;
    case 4:  //rotated
      si = tilebuf+((tile+1)<<7);
      for(i=15; i>=0; --i)
      {
        di=unpackbuf + 271 - i; //256 + 15 - i
        for(j=7; j>=0; --j)
        {
          di-=16;
          *di = (*(--si)) >> 4;
          di-=16;
          *di = (*si) & 15;
        }
      }
      break;
    case 5:  //rotated and horizontal
      si = tilebuf+((tile+1)<<7);
      for(i=15; i>=0; --i)
      {
        di=unpackbuf + 256 + i;
        for(j=7; j>=0; --j)
        {
          di-=16;
          *di = (*(--si)) >> 4;
          di-=16;
          *di = (*si) & 15;
        }
      }
      break;
    default: //none or invalid
      si = tilebuf+((tile+1)<<7);
      di = unpackbuf + 256;
      for(i=127; i>=0; --i)
      {
        (*(--di)) = (*(--si)) >> 4;
        (*(--di)) = (*si) & 15;
      }
      break;
  }
}

// packs from src[256] to tilebuf
void pack_tile(byte *src,int tile)
{
  byte *di = tilebuf+(tile<<7);

  for(int si=0; si<256; si+=2)
  {
    *di = (src[si]&15) + ((src[si+1]&15) << 4);
    ++di;
  }
}

// packs a whole set of tiles from old size to new size
void pack_tiles(byte *buf)
{
  int di = 0;
  for(int si=0; si<TILEBUF_SIZE; si+=2)
    buf[di++] = (buf[si]&15) + ((buf[si+1]&15) << 4);

  for( ; di<NEWTILE_SIZE2; ++di)
    buf[di]=0;
}

int rotate_table[8]=
{
  4, 6, 5, 7, 3, 1, 2, 0
};

int rotate_value(int flip)
{
  return rotate_table[flip&7];
}




















void puttiletranslucent8(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity)
{

  if(x<-7 || y<-7)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;

  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile>>2, 0, false);
  byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
  if (flip&1) //horizontal
  {
    si+=7;
  }

  if ((flip&2)==0)                                          //not flipped vertically
  {
    if (y<0)
    {
      si+=(0-y)<<4;
    }
    for (int dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); ++dy)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      for (int i=0; i<8; ++i)
      {
        if (x+i<dest->w)
        {
          //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
          *(di) = trans_table.data[(*di)][((*si) + cset)];
          ++di;
        }
        flip&1 ? --si : ++si;
      }
      if (flip&1)
      {
        si+=24;
      }
      else
      {
        si+=8;
      }
    }
  }                                                         //flipped vertically
  else
  {
    if (y+7>=dest->h)
    {
      si+=(8+y-dest->h)<<4;
    }
    for (int dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); --dy)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      for (int i=0; i<8; ++i)
      {
        if (x+i<dest->w)
        {
          //          *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
          *(di) = trans_table.data[(*di)][((*si) + cset)];
          ++di;
        }
        flip&1 ? --si : ++si;
      }
      if (flip&1)
      {
        si+=24;
      }
      else
      {
        si+=8;
      }
    }
  }
}

void overtiletranslucent8(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity)
{
  if(x<-7 || y<-7)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;

  if (blank_tile_quarters_table[tile])
  {
    return;
  }

  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile>>2, 0, false);
  byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
  if (flip&1)
  {
    si+=7;
  }

  if ((flip&2)==0)                                          //not flipped vertically
  {
    if (y<0)
    {
      si+=(0-y)<<4;
    }
    for (int dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); ++dy)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      for (int i=0; i<8; ++i)
      {
        if (x+i<dest->w)
        {
          if (*si)
          {
            //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *(di) = trans_table.data[(*di)][((*si) + cset)];
          }
          ++di;
        }
        flip&1 ? --si : ++si;
      }
      if (flip&1)
      {
        si+=24;
      }
      else
      {
        si+=8;
      }
    }
  }                                                         //flipped vertically
  else
  {
    if (y+7>=dest->h)
    {
      si+=(8+y-dest->h)<<4;
    }
    for (int dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); --dy)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      for (int i=0; i<8; ++i)
      {
        if (x+i<dest->w)
        {
          if(*si)
          {
            //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *(di) = trans_table.data[(*di)][((*si) + cset)];
          }
          ++di;
        }
        flip&1 ? --si : ++si;
      }
      if (flip&1)
      {
        si+=24;
      }
      else
      {
        si+=8;
      }
    }
  }
}

void puttiletranslucent16(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity)
{
  if(x<-15 || y<-15)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;
  if(tile<0 || tile>=NEWMAXTILES)
  {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
  }

  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile, 0, false);
  byte *si = unpackbuf;
  byte *di;
  if(flip&1)
    si+=15;

  if((flip&2)==0)
  {
    if(y<0)
      si+=(0-y)<<4;

    for(int dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          //          *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
          *di=trans_table.data[(*di)][((*si) + cset)];
          ++di;
          flip&1 ? --si : ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *di=trans_table.data[(*di)][((*si) + cset)];
            ++di;
          }
          flip&1 ? --si : ++si;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
  else
  {
    if(y+15>=dest->h)
      si+=(16+y-dest->h)<<4;

    for(int dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          //          *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
          *di=trans_table.data[(*di)][((*si) + cset)];
          ++di;
          flip&1 ? --si : ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *di=trans_table.data[(*di)][((*si) + cset)];
            ++di;
          }
          flip&1 ? --si : ++si;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
}

void overtiletranslucent16(BITMAP* dest,int tile,int x,int y,int cset,int flip,int opacity)
{
  if(x<-15 || y<-15)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;
  if(tile<0 || tile>=NEWMAXTILES)
  {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
  }

  if (blank_tile_table[tile])
  {
    return;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile, 0, false);
  byte *si = unpackbuf;
  byte *di;
  if(flip&1)
    si+=15;

  if((flip&2)==0)
  {
    if(y<0)
      si+=(0-y)<<4;

    for(int dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          if(*si)
          {
            //            *di=*si+cset;
            //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *di=trans_table.data[(*di)][((*si) + cset)];
          }
          ++di;
          flip&1 ? --si : ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            if(*si)
            {
              //              *di=*si+cset;
              //              *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
              *di=trans_table.data[(*di)][((*si) + cset)];
            }
            ++di;
          }
          flip&1 ? --si : ++si;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
  else
  {
    if(y+15>=dest->h)
      si+=(16+y-dest->h)<<4;

    for(int dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          if(*si)
          {
            //            *di=*si+cset;
            //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *di=trans_table.data[(*di)][((*si) + cset)];
          }
          ++di;
          flip&1 ? --si : ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            if(*si)
            {
              //              *di=*si+cset;
              //              *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
              *di=trans_table.data[(*di)][((*si) + cset)];
            }
            ++di;
          }
          flip&1 ? --si : ++si;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
}

void overtilecloaked16(BITMAP* dest,int tile,int x,int y,int flip)
{
  if(x<-15 || y<-15)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;
  if(tile<0 || tile>=NEWMAXTILES)
  {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
  }

  unpack_tile(tile, 0, false);
  byte *si = unpackbuf;
  byte *di;
  if(flip&1)
    si+=15;

  if((flip&2)==0)
  {
    if(y<0)
      si+=(0-y)<<4;

    for(int dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          if(*si)
          {
            *di=dest->line[((y+dy)^1)][((x+dx)^1)];
          }
          ++di;
          flip&1 ? --si : ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            if(*si)
            {
              *di=dest->line[((y+dy)^1)][(x^1)];
            }
            ++di;
          }
          flip&1 ? --si : ++si;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
  else
  {
    if(y+15>=dest->h)
      si+=(16+y-dest->h)<<4;

    for(int dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          if(*si)
          {
            *di=dest->line[((y+dy)^1)][((x+dx)^1)];
          }
          ++di;
          flip&1 ? --si : ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            if(*si)
            {
              *di=dest->line[((y+dy)^1)][(x^1)];
            }
            ++di;
          }
          flip&1 ? --si : ++si;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
}

void putblocktranslucent8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask,int opacity)
{
  int t[4];
  for(int i=0; i<4; ++i)
    t[i]=tile+i;

  switch(mask)
  {
    case 1:
      puttiletranslucent8(dest,tile,x,y,csets[0],flip,opacity);
      break;

    case 3:
      if(flip&2)
      {
        swap(t[0],t[1]);
        //      swap(csets[0],csets[1]);
      }
      puttiletranslucent8(dest,t[0],x,y,  csets[0],flip,opacity);
      puttiletranslucent8(dest,t[1],x,y+8,csets[1],flip,opacity);
      break;

    case 5:
      if(flip&1)
      {
        swap(t[0],t[1]);
        //      swap(csets[0],csets[1]);
      }
      puttiletranslucent8(dest,t[0],x,  y,csets[0],flip,opacity);
      puttiletranslucent8(dest,t[1],x+8,y,csets[1],flip,opacity);
      break;

    case 15:
      if(flip&1)
      {
        swap(t[0],t[1]);
        swap(t[2],t[3]);
        //      swap(csets[0],csets[1]);
        //      swap(csets[2],csets[3]);
      }
      if(flip&2)
      {
        swap(t[0],t[2]);
        swap(t[1],t[3]);
        //      swap(csets[0],csets[2]);
        //      swap(csets[1],csets[3]);
      }
      puttiletranslucent8(dest,t[0],x,  y,  csets[0],flip,opacity);
      puttiletranslucent8(dest,t[1],x+8,y,  csets[1],flip,opacity);
      puttiletranslucent8(dest,t[2],x,  y+8,csets[2],flip,opacity);
      puttiletranslucent8(dest,t[3],x+8,y+8,csets[3],flip,opacity);
      break;
  }
}

void overblocktranslucent8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask,int opacity)
{
  int t[4];
  for(int i=0; i<4; ++i)
    t[i]=tile+i;

  switch(mask)
  {
    case 1:
      overtiletranslucent8(dest,tile,x,y,csets[0],flip,opacity);
      break;

    case 3:
      if(flip&2)
      {
        swap(t[0],t[1]);
        swap(csets[0],csets[1]);
      }
      overtiletranslucent8(dest,t[0],x,y,  csets[0],flip,opacity);
      overtiletranslucent8(dest,t[1],x,y+8,csets[1],flip,opacity);
      break;

    case 5:
      if(flip&1)
      {
        swap(t[0],t[1]);
        //      swap(csets[0],csets[1]);
      }
      overtiletranslucent8(dest,t[0],x,  y,csets[0],flip,opacity);
      overtiletranslucent8(dest,t[1],x+8,y,csets[1],flip,opacity);
      break;

    case 15:
      if(flip&1)
      {
        swap(t[0],t[1]);
        swap(t[2],t[3]);
        //      swap(csets[0],csets[1]);
        //      swap(csets[2],csets[3]);
      }
      if(flip&2)
      {
        swap(t[0],t[2]);
        swap(t[1],t[3]);
        //      swap(csets[0],csets[2]);
        //      swap(csets[1],csets[3]);
      }
      overtiletranslucent8(dest,t[0],x,  y,  csets[0],flip,opacity);
      overtiletranslucent8(dest,t[1],x+8,y,  csets[1],flip,opacity);
      overtiletranslucent8(dest,t[2],x,  y+8,csets[2],flip,opacity);
      overtiletranslucent8(dest,t[3],x+8,y+8,csets[3],flip,opacity);
      break;
  }
}

//  cmbdat: fffffsss cccccccc
//          (f:flags, s:cset, c:combo)

void putcombotranslucent(BITMAP* dest,int x,int y,int cmbdat,int cset,int opacity)
{
  //  int cset=(cmbdat&0x0700)>>8;

  newcombo c = combobuf[cmbdat];                            //strip the flags
  int drawtile=c.tile;
  int tframes=max(1, combobuf[cmbdat].frames);
  double ddir=atan2(double(y-LinkModifiedY()),double(LinkModifiedX()-x));
  switch (combobuf[cmbdat].type)
  {
    case cEYEBALL_A:
      if ((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
    case cEYEBALL_B:
      if ((ddir<=(((-6)*PI)/8))&&(ddir>(((-8)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-4)*PI)/8))&&(ddir>(((-6)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-2)*PI)/8))&&(ddir>(((-4)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((0)*PI)/8))&&(ddir>(((-2)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((2)*PI)/8))&&(ddir>(((0)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((4)*PI)/8))&&(ddir>(((2)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((6)*PI)/8))&&(ddir>(((4)*PI)/8)))
      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
  }
  if(!(c.csets&0xF0) || !(c.csets&0x0F))
    puttiletranslucent16(dest,drawtile,x,y,cset,c.flip,opacity);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    if(cofs&8)
      cofs |= ~int(0xF);

    for(int i=0; i<4; ++i)
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

    putblocktranslucent8(dest,drawtile<<2,x,y,csets,c.flip,15,opacity);
  }
}

void overcombotranslucent(BITMAP* dest,int x,int y,int cmbdat,int cset,int opacity)
{
  newcombo c = combobuf[cmbdat];                            //strip the flags
  int drawtile=c.tile;
  int tframes=max(1, combobuf[cmbdat].frames);
  double ddir=atan2(double(y-LinkModifiedY()),double(LinkModifiedX()-x));
  switch (combobuf[cmbdat].type)
  {
    case cEYEBALL_A:
      if ((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
    case cEYEBALL_B:
      if ((ddir<=(((-6)*PI)/8))&&(ddir>(((-8)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-4)*PI)/8))&&(ddir>(((-6)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-2)*PI)/8))&&(ddir>(((-4)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((0)*PI)/8))&&(ddir>(((-2)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((2)*PI)/8))&&(ddir>(((0)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((4)*PI)/8))&&(ddir>(((2)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((6)*PI)/8))&&(ddir>(((4)*PI)/8)))


      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
  }
  if(!(c.csets&0xF0) || !(c.csets&0x0F))
    overtiletranslucent16(dest,drawtile,x,y,cset,c.flip,opacity);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    if(cofs&8)
      cofs |= ~int(0xF);

    for(int i=0; i<4; ++i)
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

    overblocktranslucent8(dest,drawtile<<2,x,y,csets,c.flip,15,opacity);
  }
}

//shnarf

void puttile8(BITMAP* dest,int tile,int x,int y,int cset,int flip)
{
  if(x<0 || y<0)
    return;
  if(y > dest->h-8)
    return;
  if(y == dest->h-8 && x > dest->w-8)
    return;

  cset &= 15;
  cset <<= CSET_SHFT;
  dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;
  unpack_tile(tile>>2, 0, false);

  //  to go to 24-bit color, do this kind of thing...
  //  ((long *)bmp->line[y])[x] = color;

  switch(flip&3)
  {
    case 1:                                                 // 1 byte at a time
    {
      byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
      for(int dy=0; dy<8; ++dy)
      {
        byte *di = &(dest->line[y+dy][x+7]);
        for(int i=0; i<8; ++i)
          *(di--) = *(si++) + cset;
        si+=8;
      }
    }
    break;

    case 2:                                                 // 4 bytes at a time
    {
      dword *si = ((dword*)unpackbuf) + ((tile&2)<<4) + ((tile&1)<<1);

      for(int dy=7; dy>=0; --dy)
      {
        dword *di=&((dword*)dest->line[y+dy])[x>>2];
        *(di++) = *(si++) + lcset;
        *(di++) = *(si++) + lcset;
        si+=2;
      }
    }
    break;

    case 3:                                                 // 1 byte at a time
    {
      byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
      for(int dy=7; dy>=0; --dy)
      {
        byte *di = &(dest->line[y+dy][x+7]);
        for(int i=0; i<8; ++i)
          *(di--) = *(si++) + cset;
        si+=8;
      }
    }
    break;

    default:                                                // 4 bytes at a time
    {
      dword *si = ((dword*)unpackbuf) + ((tile&2)<<4) + ((tile&1)<<1);
      for(int dy=0; dy<8; ++dy)
      {
        dword *di = &((dword*)dest->line[y+dy])[x>>2];
        *(di++) = *(si++) + lcset;
        *(di++) = *(si++) + lcset;
        si+=2;
      }
    }
    break;
  }
}

void overtile8(BITMAP* dest,int tile,int x,int y,int cset,int flip)
{
  if(x<-7 || y<-7)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;

  if (blank_tile_quarters_table[tile])
  {
    return;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile>>2, 0, false);
  byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
  if (flip&1)
  {
    si+=7;
  }

  if ((flip&2)==0)                                          //not flipped vertically
  {
    if (y<0)
    {
      si+=(0-y)<<4;
    }
    for (int dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); ++dy)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      for (int i=0; i<8; ++i)
      {
        if (x+i<dest->w)
        {
          if (*si)
          {
            //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *(di) = (*si) + cset;
          }
          ++di;
        }
        flip&1 ? --si : ++si;
      }
      if (flip&1)
      {
        si+=24;
      }
      else
      {
        si+=8;

      }
    }
  }                                                         //flipped vertically
  else
  {
    if (y+7>=dest->h)
    {
      si+=(8+y-dest->h)<<4;
    }
    for (int dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); --dy)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      for (int i=0; i<8; ++i)
      {
        if (x+i<dest->w)
        {
          if(*si)
          {
            //            *(di) = (opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *(di) = (*si) + cset;
          }
          ++di;
        }
        flip&1 ? --si : ++si;
      }
      if (flip&1)
      {
        si+=24;
      }
      else
      {
        si+=8;
      }
    }
  }
}

void puttile16(BITMAP* dest,int tile,int x,int y,int cset,int flip) //fixed
{
  if(x<0 || y<0)
    return;
  if(y > dest->h-16)
    return;
  if((y == dest->h-16) && (x > dest->w-16))
    return;
  if(tile<0 || tile>=NEWMAXTILES)
  {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
  }
  cset &= 15;
  cset <<= CSET_SHFT;

  unpack_tile(tile, flip&5, false);

  switch(flip&2)
  {
/*
    case 1:
    {
      byte *si = unpackbuf;
      for(int dy=0; dy<16; ++dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x+15]);
        for(int i=0; i<16; ++i)
          *(di--) = *(si++) + cset;
      }
    } break;
*/
    case 2: //vertical
    {
/*
      dword *si = (dword*)unpackbuf;
      for(int dy=15; dy>=0; --dy)
      {
        // 4 bytes at a time
        dword *di=&((dword*)dest->line[y+dy])[x>>2];
        for(int i=0; i<16; i+=4)
          *(di++) = *(si++) + lcset;
      }
*/
      qword llcset = (((qword)cset)<<56)+(((qword)cset)<<48)+(((qword)cset)<<40)+(((qword)cset)<<32)+(((qword)cset)<<24)+(cset<<16)+(cset<<8)+cset;
//      qword llcset = (((qword)cset)<<56)|(((qword)cset)<<48)|(((qword)cset)<<40)|(((qword)cset)<<32)|(((qword)cset)<<24)|(cset<<16)|(cset<<8)|cset;
      qword *si = (qword*)unpackbuf;
      for(int dy=15; dy>=0; --dy)
      {
        // 4 bytes at a time
//        qword *di=&((qword*)dest->line[y+dy])[x>>3];
        qword *di=(qword*)(dest->line[y+dy]+x);
        for(int i=0; i<16; i+=8)
          *(di++) = *(si++) + llcset;
      }
    } break;
/*
    case 3:
    {
      byte *si = unpackbuf;
      for(int dy=15; dy>=0; --dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x+15]);
        for(int i=0; i<16; ++i)
          *(di--) = *(si++) + cset;
      }
    } break;
*/
    default: //none or invalid
    {
/*
      dword *si = (dword*)unpackbuf;
      for(int dy=0; dy<16; ++dy)
      {
        // 4 bytes at a time
        dword *di=&((dword*)dest->line[y+dy])[x>>2];
        for(int i=0; i<16; i+=4)
          *(di++) = *(si++) + lcset;
      }
*/
      qword llcset = (((qword)cset)<<56)+(((qword)cset)<<48)+(((qword)cset)<<40)+(((qword)cset)<<32)+(((qword)cset)<<24)+(cset<<16)+(cset<<8)+cset;
//      qword llcset = (((qword)cset)<<56)|(((qword)cset)<<48)|(((qword)cset)<<40)|(((qword)cset)<<32)|(((qword)cset)<<24)|(cset<<16)|(cset<<8)|cset;
      qword *si = (qword*)unpackbuf;
      for(int dy=0; dy<16; ++dy)
      {
        // 4 bytes at a time
//        qword *di=&((qword*)dest->line[y+dy])[x>>3];
        qword *di=(qword*)(dest->line[y+dy]+x);
        for(int i=0; i<16; i+=8)
          *(di++) = *(si++) + llcset;
      }
    } break;
  }
}



void puttile16_old(BITMAP* dest,int tile,int x,int y,int cset,int flip)
{
  if(x<0 || y<0)
    return;
  if(y > dest->h-16)
    return;
  if((y == dest->h-16) && (x > dest->w-16))
    return;
  if(tile<0 || tile>=NEWMAXTILES)
  {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
  }
  cset &= 15;
  cset <<= CSET_SHFT;

  switch(flip&3)
  {
    case 1: //horizontal
    {

/*
      byte *si = tilebuf+(tile<<7);
      for(int dy=0; dy<16; ++dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x+15]);
        for(int i=0; i<8; ++i) {
          *(di--) = ((*si)&0x0F) + cset;
          *(di--) = ((*(si++))>>4) + cset;
        }
      }
*/
      byte *si = tilebuf+((tile+1)<<7);
      for(int dy=15; dy>=0; --dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x])-1;
        for(int i=0; i<8; ++i) {
          *(++di) = ((*(--si))>>4) | cset;
          *(++di) = ((*si)&0x0F) | cset;
        }
      }


    } break;

    case 2: //vertical
    {
/*
      byte *si = tilebuf+(tile<<7);
      for(int dy=15; dy>=0; --dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x]);
        for(int i=0; i<8; ++i) {
          *(di++) = ((*si)&0x0F) + cset;
          *(di++) = ((*(si++))>>4) + cset;
        }
      }
*/

      byte *si = tilebuf+((tile+1)<<7);
      for(int dy=0; dy<16; ++dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x+15])+1;
        for(int i=0; i<8; ++i) {
          *(--di) = ((*(--si))>>4) | cset;
          *(--di) = ((*si)&0x0F) | cset;
        }
      }
/*
      dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;

      unpack_tile(tile);
      dword *si = ((dword*)unpackbuf)-1;
      for(int dy=15; dy>=0; --dy)
      {
        // 4 bytes at a time
        dword *di=(&((dword*)dest->line[y+dy])[x>>2])-1;
        for(int i=0; i<16; i+=4)
          *(++di) = *(++si) + lcset;
      }
*/
    } break;

    case 3: //both
    {
/*
      byte *si = tilebuf+(tile<<7);
      for(int dy=15; dy>=0; --dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x+15]);
        for(int i=0; i<8; ++i) {
          *(di--) = ((*si)&0x0F) + cset;
          *(di--) = ((*(si++))>>4) + cset;
        }
      }
*/

      byte *si = tilebuf+((tile+1)<<7);
      for(int dy=0; dy<16; ++dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x])-1;
        for(int i=0; i<8; ++i) {
          *(++di) = ((*(--si))>>4) | cset;
          *(++di) = ((*si)&0x0F) | cset;
        }
      }
    } break;

    default: //none
    {
/*
      byte *si = tilebuf+(tile<<7);
      for(int dy=0; dy<16; ++dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x]);
        for(int i=0; i<8; ++i) {
          *(di++) = ((*si)&0x0F) + cset;
          *(di++) = ((*(si++))>>4) + cset;
        }
      }
*/

      byte *si = tilebuf+((tile+1)<<7);
      for(int dy=15; dy>=0; --dy)
      {
        // 1 byte at a time
        byte *di = &(dest->line[y+dy][x+15])+1;
        for(int i=0; i<8; ++i) {
          *(--di) = ((*(--si))>>4) | cset;
          *(--di) = ((*si)&0x0F) | cset;
        }
      }
/*
      dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;

      unpack_tile(tile);
      dword *si = ((dword*)unpackbuf)-1;
      for(int dy=0; dy<16; ++dy)
      {
        // 4 bytes at a time
        dword *di=(&((dword*)dest->line[y+dy])[x>>2])-1;
        for(int i=0; i<16; i+=4)
          *(++di) = *(++si) + lcset;
      }
*/
    } break;
  }
}

void overtile16(BITMAP* dest,int tile,int x,int y,int cset,int flip) //fixed
{
  if(x<-15 || y<-15)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;
  if(tile<0 || tile>=NEWMAXTILES)
  {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
  }

  if (blank_tile_table[tile])
  {
    return;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile, flip&5, false);
  byte *si = unpackbuf;
  byte *di;

  if((flip&2)==0)
  {
    if(y<0)
      si+=(0-y)<<4;

    for(int dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); ++dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          if(*si)
            *di=*si+cset;
          ++di;
          ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            if(*si)
              *di=*si+cset;
            ++di;
          }
          ++si;
        }
      }
    }
  }
  else
  {
    if(y+15>=dest->h)
      si+=(16+y-dest->h)<<4;

    for(int dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); --dy)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; ++dx)
        {
          if(*si)
            *di=*si+cset;
          ++di;
          ++si;
        }
      }
      else
      {
        for(int i=0; i<16; ++i)
        {
          if(x+i<dest->w)
          {
            if(*si)
              *di=*si+cset;
            ++di;
          }
          ++si;
        }
      }
    }
  }
}

void putblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask)
{
  int t[4];
  for(int i=0; i<4; ++i)
    t[i]=tile+i;

  switch(mask)
  {
    case 1: //top-left quarter
      puttile8(dest,tile,x,y,csets[0],flip);
      break;

    case 3: //vertical
      if(flip&2)
      {
        swap(t[0],t[1]);
        //      swap(csets[0],csets[1]);
      }
      puttile8(dest,t[0],x,y,  csets[0],flip);
      puttile8(dest,t[1],x,y+8,csets[1],flip);
      break;

    case 5: //horizontal
      if(flip&1)
      {
        swap(t[0],t[1]);
        //      swap(csets[0],csets[1]);
      }
      puttile8(dest,t[0],x,  y,csets[0],flip);
      puttile8(dest,t[1],x+8,y,csets[1],flip);
      break;

    case 15: //all 4 quarters
      if(flip&1)
      {
        swap(t[0],t[1]);
        swap(t[2],t[3]);
        //      swap(csets[0],csets[1]);
        //      swap(csets[2],csets[3]);
      }
      if(flip&2)
      {

        swap(t[0],t[2]);
        swap(t[1],t[3]);
        //      swap(csets[0],csets[2]);
        //      swap(csets[1],csets[3]);
      }
      puttile8(dest,t[0],x,  y,  csets[0],flip);
      puttile8(dest,t[1],x+8,y,  csets[1],flip);
      puttile8(dest,t[2],x,  y+8,csets[2],flip);
      puttile8(dest,t[3],x+8,y+8,csets[3],flip);
      break;
  }
}

void overblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask)
{
  int t[4];
  for(int i=0; i<4; ++i)
    t[i]=tile+i;

  switch(mask)
  {
    case 1:
      overtile8(dest,tile,x,y,csets[0],flip);
      break;

    case 3:
      if(flip&2)
      {
        swap(t[0],t[1]);
        //      swap(csets[0],csets[1]);
      }
      overtile8(dest,t[0],x,y,  csets[0],flip);
      overtile8(dest,t[1],x,y+8,csets[1],flip);
      break;

    case 5:
      if(flip&1)
      {
        swap(t[0],t[1]);
        //      swap(csets[0],csets[1]);
      }
      overtile8(dest,t[0],x,  y,csets[0],flip);
      overtile8(dest,t[1],x+8,y,csets[1],flip);
      break;

    case 15:
      if(flip&1)
      {
        swap(t[0],t[1]);
        swap(t[2],t[3]);
        //      swap(csets[0],csets[1]);
        //      swap(csets[2],csets[3]);
      }
      if(flip&2)
      {
        swap(t[0],t[2]);
        swap(t[1],t[3]);
        //      swap(csets[0],csets[2]);
        //      swap(csets[1],csets[3]);
      }
      overtile8(dest,t[0],x,  y,  csets[0],flip);
      overtile8(dest,t[1],x+8,y,  csets[1],flip);
      overtile8(dest,t[2],x,  y+8,csets[2],flip);
      overtile8(dest,t[3],x+8,y+8,csets[3],flip);
      break;
  }
}

//  cmbdat: fffffsss cccccccc
//          (f:flags, s:cset, c:combo)

void putcombo(BITMAP* dest,int x,int y,int cmbdat,int cset)
{
  //  int cset=(cmbdat&0x0700)>>8;

  newcombo c = combobuf[cmbdat];                            //strip the flags
  int drawtile=c.tile;
  int tframes=max(1, combobuf[cmbdat].frames);
  double ddir=atan2(double(y-LinkModifiedY()),double(LinkModifiedX()-x));
  switch (combobuf[cmbdat].type)
  {
    case cEYEBALL_A:
      if ((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
    case cEYEBALL_B:
      if ((ddir<=(((-6)*PI)/8))&&(ddir>(((-8)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-4)*PI)/8))&&(ddir>(((-6)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-2)*PI)/8))&&(ddir>(((-4)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((0)*PI)/8))&&(ddir>(((-2)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((2)*PI)/8))&&(ddir>(((0)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((4)*PI)/8))&&(ddir>(((2)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((6)*PI)/8))&&(ddir>(((4)*PI)/8)))
      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
  }
  if(!(c.csets&0xF0) || !(c.csets&0x0F))
    puttile16(dest,drawtile,x,y,cset,c.flip);
  //    puttile16(dest,c.drawtile,x,y,cset,c.flip);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    if(cofs&8)
      cofs |= ~int(0xF);

    for(int i=0; i<4; ++i)
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

    putblock8(dest,drawtile<<2,x,y,csets,c.flip,15);
    //    putblock8(dest,c.drawtile<<2,x,y,csets,c.flip,15);
  }
}

void overcombo(BITMAP* dest,int x,int y,int cmbdat,int cset)
{
  newcombo c = combobuf[cmbdat];                            //strip the flags
  int drawtile=c.tile;
  int tframes=max(1, combobuf[cmbdat].frames);
  double ddir=atan2(double(y-LinkModifiedY()),double(LinkModifiedX()-x));
  switch (combobuf[cmbdat].type)
  {
    case cEYEBALL_A:
      if ((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
    case cEYEBALL_B:
      if ((ddir<=(((-6)*PI)/8))&&(ddir>(((-8)*PI)/8)))
      {
        drawtile+=tframes*5;                                //dl
      }
      else if ((ddir<=(((-4)*PI)/8))&&(ddir>(((-6)*PI)/8)))
      {
        drawtile+=tframes*4;                                //d
      }
      else if ((ddir<=(((-2)*PI)/8))&&(ddir>(((-4)*PI)/8)))
      {
        drawtile+=tframes*3;                                //dr
      }
      else if ((ddir<=(((0)*PI)/8))&&(ddir>(((-2)*PI)/8)))
      {
        drawtile+=tframes*2;                                //r
      }
      else if ((ddir<=(((2)*PI)/8))&&(ddir>(((0)*PI)/8)))
      {
        drawtile+=tframes*1;                                //ur
      }
      else if ((ddir<=(((4)*PI)/8))&&(ddir>(((2)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else if ((ddir<=(((6)*PI)/8))&&(ddir>(((4)*PI)/8)))
      {
        drawtile+=tframes*7;                                //ul
      }
      else
      {
        drawtile+=tframes*6;                                //l
      }
      break;
  }
  if(!(c.csets&0xF0) || !(c.csets&0x0F))
    overtile16(dest,drawtile,x,y,cset,c.flip);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    if(cofs&8)
      cofs |= ~int(0xF);

    for(int i=0; i<4; ++i)
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

    overblock8(dest,drawtile<<2,x,y,csets,c.flip,15);
  }
}

void overcombo2(BITMAP* dest,int x,int y,int cmbdat,int cset)
{
  if (cmbdat!=0)
  {
    overcombo(dest,x,y,cmbdat,cset);
  }
}

/* end of tiles.cc */
