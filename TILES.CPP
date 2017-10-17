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
#include "zsys.h"
#include "tiles.h"
#include "trapper_keeper.h"

extern RGB_MAP rgb_table;
extern COLOR_MAP trans_table;
extern itemdata   *itemsbuf;
extern wpndata    *wpnsbuf;
extern byte        quest_rules[QUESTRULES_SIZE];
//extern byte *tilebuf;
//BITMAP* tilebuf[NEWMAXTILES];
tiledata *newtilebuf, *grabtilebuf;
newcombo *combobuf;
word animated_combo_table[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
word animated_combo_table4[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
word animated_combos;
word animated_combo_table2[MAXCOMBOS][2];                    //[0]=position in act2, [1]=original tile
word animated_combo_table24[MAXCOMBOS][2];                   //[0]=combo, [1]=clock
word animated_combos2;
bool blank_tile_table[NEWMAXTILES];                         //keeps track of blank tiles
bool used_tile_table[NEWMAXTILES];                          //keeps track of used tiles
bool blank_tile_quarters_table[NEWMAXTILES*4];              //keeps track of blank tile quarters
extern fix  LinkModifiedX();
extern fix  LinkModifiedY();

bool unused_tile_table[NEWMAXTILES];                  //keeps track of unused tiles

byte unpackbuf[UNPACKSIZE];

bool isblanktile(tiledata *buf, int i)
{
  //  byte *tilestart=tilebuf+(i*128);
  byte *tilestart=buf[i].data;
  qword *di=(qword*)tilestart;
  int parts=tilesize(buf[i].format)>>3;
  for (int j=0; j<parts; ++j, ++di)
  {
    if (*di!=0)
    {
      return false;
    }
  }
  return true;
}


const char *tileformat_string[tfMax] =
{
  "(Invalid)", "4-bit", "8-bit", "16-bit", "24-bit", "32-bit"
};

void register_blank_tile_quarters(int tile)
{
  //  byte *tilestart=tilebuf+(tile*128);
  dword *di=(dword*)newtilebuf[tile].data;
  blank_tile_quarters_table[(tile<<2)]=true;
  blank_tile_quarters_table[(tile<<2)+1]=true;
  blank_tile_quarters_table[(tile<<2)+2]=true;
  blank_tile_quarters_table[(tile<<2)+3]=true;

  int parts=tilesize(newtilebuf[tile].format)>>6;

  for (int r=0; r<16; ++r)
  {
    for (int j=0; j<parts; ++j, ++di)
    {
      if (*di!=0)
      {
        int q=(tile<<2) + ((r/8)<<1) + ((j>(parts/2-1))?1:0);
        blank_tile_quarters_table[q]=false;
      }
    }
  }
}



void register_blank_tiles()
{
  //int tiles_used=count_tiles(newtilebuf);
  for (int i=0; i<NEWMAXTILES; ++i)
  {
    register_blank_tile_quarters(i);
    blank_tile_table[i]=isblanktile(newtilebuf, i);
  }
}

//returns the number of tiles
word count_tiles(tiledata *buf)
{
  word tiles_used;
  //  bool used;
  //  int x;
  for(tiles_used=(NEWMAXTILES); tiles_used>0; --tiles_used)
  {
    /*
      used=false;
      for (x=0; x<128; ++x)
      {
      used=used || (tilebuf[(tiles_used-1)*128+x]!=0);
      }
      if (used)
      {
      break;
      }
      */
    if(!isblanktile(buf, tiles_used-1))
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
	if ((combobuf[x].frames>1 || combobuf[x].nextcombo != 0)&&!(combobuf[x].animflags &	AF_FRESH))
    {
      animated_combo_table4[y][0]=x;
      animated_combo_table4[y][1]=0;
      ++y;
    }
  }
  animated_combos=y;
}

void setup_combo_animations2()
{
  memset(animated_combo_table2, 0, MAXCOMBOS*2*2);
  memset(animated_combo_table24, 0, MAXCOMBOS*2*2);
  int y=0;
  for (word x=0; x<MAXCOMBOS; ++x)
  {
    animated_combo_table2[x][0]=y;
    animated_combo_table2[x][1]=combobuf[x].tile;
	if ((combobuf[x].frames>1 || combobuf[x].nextcombo != 0)&&combobuf[x].animflags & AF_FRESH)
    {
      animated_combo_table24[y][0]=x;
      animated_combo_table24[y][1]=0;
      ++y;
    }
  }
  animated_combos2=y;
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

void reset_combo_animation2(int c)
{
  for (word x=0; x<animated_combos2; ++x)
  {
    int y=animated_combo_table24[x][0];                      //combo number
    if (y==c)
    {
      combobuf[y].tile=animated_combo_table2[y][1];        //reset tile
      animated_combo_table24[x][1]=0;                        //reset clock
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

void reset_combo_animations2()
{
  for (word x=0; x<animated_combos; ++x)
  {
    combobuf[animated_combo_table24[x][0]].tile=animated_combo_table2[animated_combo_table24[x][0]][1];
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
      if (combobuf[y].tile-
		  (combobuf[y].frames+((combobuf[y].frames-1)*combobuf[y].skipanim)+
		  ((combobuf[y].frames-1)*combobuf[y].skipanimy*TILES_PER_ROW))
		  >=animated_combo_table[y][1]-1)
      {
        combobuf[y].tile=animated_combo_table[y][1];        //reset tile
      }
      else
      {
	    int temp=combobuf[y].tile;
        combobuf[y].tile+=1+(combobuf[y].skipanim); //increment tile
		if(temp/TILES_PER_ROW!=combobuf[y].tile/TILES_PER_ROW)
		  combobuf[y].tile+=TILES_PER_ROW*combobuf[y].skipanimy;
      }
      animated_combo_table4[x][1]=0;                        //reset clock
    }
	else
    {
      ++animated_combo_table4[x][1];                        //increment clock
    }
  }

  for (word x=0; x<animated_combos2; ++x)
  {
    int y=animated_combo_table24[x][0];                      //combo number
    if (animated_combo_table24[x][1]>=combobuf[y].speed)     //time to animate
    {
      if (combobuf[y].tile-
		  (combobuf[y].frames+((combobuf[y].frames-1)*combobuf[y].skipanim)+
		  (combobuf[y].frames-1)*combobuf[y].skipanimy*TILES_PER_ROW)
		  >=animated_combo_table2[y][1]-1)
      {
        combobuf[y].tile=animated_combo_table2[y][1];        //reset tile
      }
      else
      {
	    int temp=combobuf[y].tile;
        combobuf[y].tile+=1+(combobuf[y].skipanim); //increment tile
		if(temp/TILES_PER_ROW!=combobuf[y].tile/TILES_PER_ROW)
		  combobuf[y].tile+=TILES_PER_ROW*combobuf[y].skipanimy;
      }
      animated_combo_table24[x][1]=0;                        //reset clock
    }
    else
    {
      ++animated_combo_table24[x][1];                        //increment clock
    }
  }
}

/*
bool isonline(float x1, float y1, float x2, float y2, float x3, float y3)
{
  float slope;
  float intercept;

  slope = (y2-y1)/(x2-x1);
  intercept = y1 - (slope*x1);
  return (y3 == (slope*x3)+intercept) && x3>zc_min(x1,x2) && x3<zc_max(x1,x2) && y3>zc_min(y1,y2) && y3<zc_max(y1,y2) ;
}
*/

//pixel-precise version of the above -DD
bool isonline(long x1, long y1, long x2, long y2, long x3, long y3)
{
	long long dx = x2-x1;
	long long dy = y2-y1;

	long long qx = x3-x1;
	long long qy = y3-y1;

	//if (x3,y3) is on the line, qx,qy must be multiples of dx,dy. Check this without division.
	if(qx*dy != qy*dx)
		return false;
	//check for degeneracy
	if(dx == 0 && dy == 0)
		return qx == 0 && qy == 0;
	long long nondegend;
	long long nondegenq;
	if(dx == 0)
	{
		nondegend = dy;
		nondegenq = qy;
	}
	else
	{
		nondegend = dx;
		nondegenq = qx;
	}
	//flip negatives
	if(nondegend < 0)
	{
		nondegend = -nondegend;
		nondegenq = -nondegenq;
	}
	//and compare
	return nondegenq >= 0 && nondegenq <= nondegend;
}

//clears a tile
/*
void clear_tile(tiledata *buf, word tile)
{
  buf[tile].format=tf4Bit;
  if (buf[tile].data!=NULL)
  {
    free(buf[tile].data);
    buf[tile].data = NULL;
  }
  buf[tile].data=(byte *)malloc(tilesize(buf[tile].format));
  if (buf[tile].data==NULL)
  {
    Z_error("Unable to initialize tile #%d.\n", tile);
    exit(1);
  }
  memset(buf[tile].data,0,tilesize(buf[tile].format));
}
*/

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
    quit_game();
    Z_error("Unable to initialize tile #%d.\n", t);
  }
  for(int i=0; i<tilesize(buf[t].format); i++)
  {
    buf[t].data[i]=0;
  }
  /*if(zctiles)
  {
	if(tilebuf[t] != NULL) destroy_bitmap(tilebuf[t]);
	tilebuf = create_bitmap_ex(8,16,16);
	if(buf[t].format == tf4Bit)
	{
	  for(int i=0; i<128; i++)
	  {
	    tilebuf[t]->line[i/8][(i%8)*2] = buf[t].data[i]>>4;
		tilebuf[t]->line[i/8][(i%8)*2+1] = buf[t].data[i]&0xF;
	  }
	}
	else
	{
	  for(int i=0; i<256; i++)
	  {
	    tilebuf[t]->line[i/16][i%16] = buf[t].data[i];
	  }
	}
  }*/
}

//clears the tile buffer
void clear_tiles(tiledata *buf)
{
  for(word i=0; i<NEWMAXTILES; ++i)
  {
    reset_tile(buf,i,tf4Bit);
  }
}

void overlay_tile(tiledata *buf,int dest,int src,int cs,bool backwards)
{
  byte upbuf[256];

  unpack_tile(buf, dest, 0, false);
  for(int i=0; i<256; i++)
    upbuf[i] = unpackbuf[i];

  unpack_tile(buf, src, 0, false);
  if (buf[src].format>tf4Bit)
  {
    cs=0;
  }
  cs &= 15;
  cs <<= CSET_SHFT;
  for(int i=0; i<256; i++)
  {
    if(backwards)
    {
      if(!upbuf[i])
      {
        upbuf[i] = unpackbuf[i]+cs;
      }
    }
    else
    {
      if(unpackbuf[i])
      {
        upbuf[i] = unpackbuf[i]+cs;
      }
    }
  }

  pack_tile(buf, upbuf,dest);
}

bool copy_tile(tiledata *buf, int src, int dest, bool swap)
{
  if (src==dest)
  {
    return true;
  }
  int tempformat=buf[dest].format;
  byte *temptiledata=(byte *)malloc(tilesize(tempformat));
  if (swap)
  {
    for(int j=0; j<tilesize(tempformat); j++)
    {
      temptiledata[j]=buf[dest].data[j];
    }
  }
  reset_tile(buf, dest, buf[src].format);
  for(int j=0; j<tilesize(buf[src].format); j++)
  {
    buf[dest].data[j]=buf[src].data[j];
  }
  if (swap)
  {
    reset_tile(buf, src, tempformat);
    for(int j=0; j<tilesize(tempformat); j++)
    {
      buf[src].data[j]=temptiledata[j];
    }
  }
  free(temptiledata);

  return true;
}


// unpacks from tilebuf to unpackbuf
void unpack_tile(tiledata *buf, int tile, int flip, bool force)
{
  static byte *si, *di;
  static byte *oldnewtilebuf=buf[tile].data;
  static int i, j, oldtile=-5, oldflip=-5;
  if (tile==oldtile&&(flip&5)==(oldflip&5)&&oldnewtilebuf==buf[tile].data&&!force)
  {
    return;
  }
  oldtile=tile;
  oldflip=flip;
  oldnewtilebuf=buf[tile].data;
  switch (flip&5)
  {
    case 1:  //horizontal
    si = buf[tile].data+tilesize(buf[tile].format);
    for(i=15; i>=0; --i)
    {
      switch(buf[tile].format)
      {
        case tf4Bit:
        di=unpackbuf + (i<<4) - 1;
        for(j=7; j>=0; --j)
        {
          (*(++di)) = (*(--si)) >> 4;
          (*(++di)) = (*si) & 15;
        }
        break;
        case tf8Bit:
        di=unpackbuf + (i<<4) - 1;
        for(j=1; j>=0; --j)
        {
          (*(++di)) = (*(--si));
          (*(++di)) = (*(--si));
          (*(++di)) = (*(--si));
          (*(++di)) = (*(--si));
          (*(++di)) = (*(--si));
          (*(++di)) = (*(--si));
          (*(++di)) = (*(--si));
          (*(++di)) = (*(--si));
        }
        break;
      }
    }
    break;
    case 4:  //rotated
    si = buf[tile].data+tilesize(buf[tile].format);
    for(i=15; i>=0; --i)
    {
      switch(buf[tile].format)
      {
        case tf4Bit:
        di=unpackbuf + 271 - i; //256 + 15 - i
        for(j=7; j>=0; --j)
        {
          di-=16;
          *di = (*(--si)) >> 4;
          di-=16;
          *di = (*si) & 15;
        }
        break;
        case tf8Bit:
        di=unpackbuf + 271 - i; //256 + 15 - i
        for(j=1; j>=0; --j)
        {
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
        }
        break;
      }
    }
    break;
    case 5:  //rotated and horizontal
    si = buf[tile].data+tilesize(buf[tile].format);
    for(i=15; i>=0; --i)
    {
      switch(buf[tile].format)
      {
        case tf4Bit:
        di=unpackbuf + 256 + i;
        for(j=7; j>=0; --j)
        {
          di-=16;
          *di = (*(--si)) >> 4;
          di-=16;
          *di = (*si) & 15;
        }
        break;
        case tf8Bit:
        di=unpackbuf + 256 + i;
        for(j=1; j>=0; --j)
        {
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
          di-=16;
          *di = (*(--si));
        }
        break;
      }
    }
    break;
    default: //none or invalid
    si = buf[tile].data+tilesize(buf[tile].format);
    switch(buf[tile].format)
    {
      case tf4Bit:
      di = unpackbuf + 256;
      for(i=127; i>=0; --i)
      {
        (*(--di)) = (*(--si)) >> 4;
        (*(--di)) = (*si) & 15;
      }
      break;
      case tf8Bit:
      di = unpackbuf + 256;
      for(i=31; i>=0; --i)
      {
        (*(--di)) = (*(--si));
        (*(--di)) = (*(--si));
        (*(--di)) = (*(--si));
        (*(--di)) = (*(--si));
        (*(--di)) = (*(--si));
        (*(--di)) = (*(--si));
        (*(--di)) = (*(--si));
        (*(--di)) = (*(--si));
      }
      break;
    }
    break;
  }
}

// packs from src[256] to tilebuf
void pack_tile(tiledata *buf, byte *src,int tile)
{
  pack_tiledata(buf[tile].data, src, buf[tile].format);
}

void pack_tiledata(byte *dest, byte *src, byte format)
{
  byte *di = dest;

  switch(format)
  {
    case tf4Bit:
    for(int si=0; si<256; si+=2)
    {
      *di = (src[si]&15) + ((src[si+1]&15) << 4);
      ++di;
    }
    break;
    case tf8Bit:
    for(int si=0; si<32; si+=1)
    {
      *di = src[si*8]; ++di;
      *di = src[si*8+1]; ++di;
      *di = src[si*8+2]; ++di;
      *di = src[si*8+3]; ++di;
      *di = src[si*8+4]; ++di;
      *di = src[si*8+5]; ++di;
      *di = src[si*8+6]; ++di;
      *di = src[si*8+7]; ++di;
    }
    break;
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
  //these are here to bypass compiler warnings about unused arguments
  opacity=opacity;

  if(x<-7 || y<-7)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;

  if (newtilebuf[tile>>2].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile>>2, 0, false);
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
  //these are here to bypass compiler warnings about unused arguments
  opacity=opacity;

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

  if (newtilebuf[tile>>2].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile>>2, 0, false);
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
  //these are here to bypass compiler warnings about unused arguments
  opacity=opacity;

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

  if(newtilebuf[tile].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile, 0, false);
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
  //these are here to bypass compiler warnings about unused arguments
  opacity=opacity;

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

  if(newtilebuf[tile].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile,flip&5, false);
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
          {
            //            *di=*si+cset;
            //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *di=trans_table.data[(*di)][((*si) + cset)];
          }
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
            {
              //              *di=*si+cset;
              //              *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
              *di=trans_table.data[(*di)][((*si) + cset)];
            }
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
          {
            //            *di=*si+cset;
            //            *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
            *di=trans_table.data[(*di)][((*si) + cset)];
          }
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
            {
              //              *di=*si+cset;
              //              *di=(opacity==255)?((*si) + cset):trans_table.data[(*di)][((*si) + cset)];
              *di=trans_table.data[(*di)][((*si) + cset)];
            }
            ++di;
          }
          ++si;
        }
      }
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

  unpack_tile(newtilebuf, tile, 0, false);
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
      zc_swap(t[0],t[1]);
    }
    puttiletranslucent8(dest,t[0],x,y,  csets[0],flip,opacity);
    puttiletranslucent8(dest,t[1],x,y+8,csets[1],flip,opacity);
    break;

    case 5:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
    }
    puttiletranslucent8(dest,t[0],x,  y,csets[0],flip,opacity);
    puttiletranslucent8(dest,t[1],x+8,y,csets[1],flip,opacity);
    break;

    case 15:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
      zc_swap(t[2],t[3]);
    }
    if(flip&2)
    {
      zc_swap(t[0],t[2]);
      zc_swap(t[1],t[3]);
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
      zc_swap(t[0],t[1]);
      zc_swap(csets[0],csets[1]);
    }
    overtiletranslucent8(dest,t[0],x,y,  csets[0],flip,opacity);
    overtiletranslucent8(dest,t[1],x,y+8,csets[1],flip,opacity);
    break;

    case 5:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
    }
    overtiletranslucent8(dest,t[0],x,  y,csets[0],flip,opacity);
    overtiletranslucent8(dest,t[1],x+8,y,csets[1],flip,opacity);
    break;

    case 15:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
      zc_swap(t[2],t[3]);
    }
    if(flip&2)
    {
      zc_swap(t[0],t[2]);
      zc_swap(t[1],t[3]);
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

int combo_tile(newcombo c, int x, int y)
{
  int drawtile=c.tile;
  int tframes=zc_max(1, c.frames);
  double ddir=0;
//  switch (c.type)
  double arg1 = y-LinkModifiedY()-playing_field_offset;
  double arg2 = LinkModifiedX()-x;
  ddir=atan2(arg1,arg2);
  switch (combo_class_buf[c.type].directional_change_type)
  {
    case 1: //cOLD_EYEBALL_A
    {
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
    }
	case 3: // 4-way Eyeball (up-down-left-right)
	{
      if ((ddir<=(((-2)*PI)/8))&&(ddir>(((-6)*PI)/8)))
      {
        drawtile+=tframes*1;                                //d
      }
      else if ((ddir<=(((2)*PI)/8))&&(ddir>(((-2)*PI)/8)))
      {
        drawtile+=tframes*3;                                //r
      }
      else if ((ddir<=(((6)*PI)/8))&&(ddir>(((2)*PI)/8)))
      {
        drawtile+=tframes*0;                                //u
      }
      else
      {
        drawtile+=tframes*2;                                //l
      }
      break;
	}
    case 2: //cOLD_EYEBALL_B
    {
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
  }
  return drawtile;
}

int combo_tile(int cmbdat, int x, int y)
{
  newcombo c = combobuf[cmbdat];
  return combo_tile(c, x, y);
}

void putcombotranslucent(BITMAP* dest,int x,int y,int cmbdat,int cset,int opacity)
{
  newcombo c = combobuf[cmbdat];
  int drawtile=combo_tile(c, x, y);
  if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[drawtile].format>tf4Bit))
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
	overcomboblocktranslucent(dest, x, y, cmbdat, cset, 1, 1, opacity);
}

void overcomboblocktranslucent(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h, int opacity)
{
  newcombo c = combobuf[cmbdat];
  int drawtile=combo_tile(c, x, y);
  for(int woff=0; woff<w; woff++)
  {
	  for(int hoff = 0; hoff<h; hoff++)
	  {
		  int tiletodraw = drawtile + 20*hoff+woff;
		  if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[tiletodraw].format>tf4Bit))
			overtiletranslucent16(dest,tiletodraw,x+16*woff,y+16*hoff,cset,c.flip,opacity);
		  else
		  {
			int csets[4];
			int cofs = c.csets&15;
			if(cofs&8)
			  cofs |= ~int(0xF);

			for(int i=0; i<4; ++i)
			  csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

			overblocktranslucent8(dest,tiletodraw<<2,x+16*woff,y+16*hoff,csets,c.flip,15,opacity);
		  }
	  }
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

  if (newtilebuf[tile>>2].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;
  unpack_tile(newtilebuf, tile>>2, 0, false);

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


void oldputtile8(BITMAP* dest,int tile,int x,int y,int cset,int flip)
{
  if(x<-7 || y<-7)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;

  if (newtilebuf[tile>>2].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile>>2, 0, false);
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
          *(di) = (*si) + cset;
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
          *(di) = (*si) + cset;
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
  if (newtilebuf[tile>>2].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile>>2, 0, false);
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

  if(newtilebuf[tile].format>tf4Bit)
  {
    cset=0;
  }
  cset &= 15;
  cset <<= CSET_SHFT;

  unpack_tile(newtilebuf, tile, flip&5, false);

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

void oldputtile16(BITMAP* dest,int tile,int x,int y,int cset,int flip) //fixed
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

  if(newtilebuf[tile].format>tf4Bit)
  {
    cset=0;
  }

  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile, flip&5, false);
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
            *di=*si+cset;
            ++di;
          }
          ++si;
        }
      }
    }
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

  if(newtilebuf[tile].format>tf4Bit)
  {
    cset=0;
  }

  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(newtilebuf, tile, flip&5, false);
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
      zc_swap(t[0],t[1]);
    }
    puttile8(dest,t[0],x,y,  csets[0],flip);
    puttile8(dest,t[1],x,y+8,csets[1],flip);
    break;

    case 5: //horizontal
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
    }
    puttile8(dest,t[0],x,  y,csets[0],flip);
    puttile8(dest,t[1],x+8,y,csets[1],flip);
    break;

    case 15: //all 4 quarters
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
      zc_swap(t[2],t[3]);
    }
    if(flip&2)
    {

      zc_swap(t[0],t[2]);
      zc_swap(t[1],t[3]);
    }
    puttile8(dest,t[0],x,  y,  csets[0],flip);
    puttile8(dest,t[1],x+8,y,  csets[1],flip);
    puttile8(dest,t[2],x,  y+8,csets[2],flip);
    puttile8(dest,t[3],x+8,y+8,csets[3],flip);
    break;
  }
}

void oldputblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask)
{
  int t[4];
  for(int i=0; i<4; ++i)
    t[i]=tile+i;

  switch(mask)
  {
    case 1:
    oldputtile8(dest,tile,x,y,csets[0],flip);
    break;

    case 3:
    if(flip&2)
    {
      zc_swap(t[0],t[1]);
    }
    oldputtile8(dest,t[0],x,y,  csets[0],flip);
    oldputtile8(dest,t[1],x,y+8,csets[1],flip);
    break;

    case 5:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
    }
    oldputtile8(dest,t[0],x,  y,csets[0],flip);
    oldputtile8(dest,t[1],x+8,y,csets[1],flip);
    break;

    case 15:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
      zc_swap(t[2],t[3]);
    }
    if(flip&2)
    {
      zc_swap(t[0],t[2]);
      zc_swap(t[1],t[3]);
    }
    oldputtile8(dest,t[0],x,  y,  csets[0],flip);
    oldputtile8(dest,t[1],x+8,y,  csets[1],flip);
    oldputtile8(dest,t[2],x,  y+8,csets[2],flip);
    oldputtile8(dest,t[3],x+8,y+8,csets[3],flip);
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
      zc_swap(t[0],t[1]);
    }
    overtile8(dest,t[0],x,y,  csets[0],flip);
    overtile8(dest,t[1],x,y+8,csets[1],flip);
    break;

    case 5:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
    }
    overtile8(dest,t[0],x,  y,csets[0],flip);
    overtile8(dest,t[1],x+8,y,csets[1],flip);
    break;

    case 15:
    if(flip&1)
    {
      zc_swap(t[0],t[1]);
      zc_swap(t[2],t[3]);
    }
    if(flip&2)
    {
      zc_swap(t[0],t[2]);
      zc_swap(t[1],t[3]);
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
  newcombo c = combobuf[cmbdat];
  int drawtile=combo_tile(c, x, y);
  if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[drawtile].format>tf4Bit))
    puttile16(dest,drawtile,x,y,cset,c.flip);
  //    puttile16(dest,c.drawtile,x,y,cset,c.flip);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    //    if(cofs&8)
    //      cofs |= ~int(0xF);

    for(int i=0; i<4; ++i)
    {
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;
    }

    putblock8(dest,drawtile<<2,x,y,csets,c.flip,15);
    //    putblock8(dest,c.drawtile<<2,x,y,csets,c.flip,15);
  }
}

void oldputcombo(BITMAP* dest,int x,int y,int cmbdat,int cset)
{
  newcombo c = combobuf[cmbdat];
  int drawtile=combo_tile(c, x, y);
  if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[drawtile].format>tf4Bit))
    oldputtile16(dest,drawtile,x,y,cset,c.flip);
  //    oldputtile16(dest,c.drawtile,x,y,cset,c.flip);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    //    if(cofs&8)
    //      cofs |= ~int(0xF);

    for(int i=0; i<4; ++i)
    {
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;
    }

    oldputblock8(dest,drawtile<<2,x,y,csets,c.flip,15);
    //    oldputblock8(dest,c.drawtile<<2,x,y,csets,c.flip,15);
  }
}

void overcombo(BITMAP* dest,int x,int y,int cmbdat,int cset)
{
	overcomboblock(dest, x, y, cmbdat, cset, 1, 1);
}

void overcomboblock(BITMAP *dest, int x, int y, int cmbdat, int cset, int w, int h)
{
  newcombo c = combobuf[cmbdat];
  int drawtile=combo_tile(c, x, y);
  for(int woff = 0; woff < w; woff++)
  {
	  for(int hoff =0; hoff < h; hoff++)
	  {
		  int tiletodraw = drawtile + 20*hoff+woff;
		  if(!(c.csets&0xF0) || !(c.csets&0x0F) || (newtilebuf[tiletodraw].format>tf4Bit))
			overtile16(dest,tiletodraw,x+16*woff,y+16*hoff,cset,c.flip);
		  else
		  {
			int csets[4];
			int cofs = c.csets&15;
			if(cofs&8)
			  cofs |= ~int(0xF);

			for(int i=0; i<4; ++i)
			  csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

			overblock8(dest,tiletodraw<<2,x+16*woff,y+16*hoff,csets,c.flip,15);
		  }
	  }
  }
}

void overcombo2(BITMAP* dest,int x,int y,int cmbdat,int cset)
{
  if (cmbdat!=0)
  {
    overcombo(dest,x,y,cmbdat,cset);
  }
}

int tilesize(byte format)
{
  switch (format)
  {
    case tf32Bit:
    return 1024;
    break;
    case tf24Bit:
    return 768;
    break;
    case tf16Bit:
    case tf8Bit:
    case tf4Bit:
    return (64<<format);
    break;
  }
  return 0;
}

int comboa_lmasktotal(byte layermask)
{
  int result=0;
  result+=layermask&1;
  result+=(layermask&2) >> 1;
  result+=(layermask&4) >> 2;
  result+=(layermask&8) >> 3;
  result+=(layermask&16) >> 4;
  result+=(layermask&32) >> 5;
  return result;
}

/* end of tiles.cc */

