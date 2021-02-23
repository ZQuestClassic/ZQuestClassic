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


#include <allegro.h>

#include "zdefs.h"
#include "tiles.h"

extern byte *tilebuf;
extern newcombo *combobuf;
extern word animated_combo_table[MAXCOMBOS][2]; //[0]=original tile, [1]=clock


byte unpackbuf[256];

//returns the number of tiles
word count_tiles() {
  word tiles_used;
  bool used;
  int x;
  for(tiles_used=(NEWMAXTILES); tiles_used>0; tiles_used--) {
    used=false;
    for (x=0; x<128; x++) {
      used=used || (tilebuf[(tiles_used-1)*128+x]!=0);
    }
    if (used) {
      break;
    }
  }
  return tiles_used;
}

//returns the number of combos
word count_combos() {
  word combos_used;
  for (combos_used=MAXCOMBOS; combos_used>0; combos_used--) {
    if (combobuf[combos_used-1].tile!=0) {
      break;
    }
  }
  return combos_used;
}

void setup_combo_animations() {
  for (word x=0; x<MAXCOMBOS; x++) {
    animated_combo_table[x][0]=combobuf[x].tile;
    animated_combo_table[x][1]=0;
  }
}

void reset_combo_animations() {
  for (word x=0; x<MAXCOMBOS; x++) {
    combobuf[x].tile=animated_combo_table[x][0];
  }
}

void animate_combos() {
  for (word x=0; x<MAXCOMBOS; x++) {
    if (combobuf[x].frames>1) {
      if (animated_combo_table[x][1]>=combobuf[x].speed) {
        if (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][0]-1) {
          combobuf[x].tile=animated_combo_table[x][0];
        } else {
          combobuf[x].tile++;
        }
        animated_combo_table[x][1]=0;
      } else {
        animated_combo_table[x][1]++;
      }
    }
  }
}


//clears the tile buffer
void clear_tiles() {
  dword i=0;
  for( ; i<NEWTILE_SIZE2/4; i++)
    ((dword*)tilebuf)[i] = 0;
}

// unpacks from tilebuf to unpackbuf
void unpack_tile(int tile)
{
  byte *si = tilebuf+((tile+1)<<7);

  for(int di=254; di>=0; di-=2)
  {
    si--;
    unpackbuf[di] = (*si) & 15;
    unpackbuf[di+1] = (*si) >> 4;
  }
}


// packs from src[256] to tilebuf
void pack_tile(byte *src,int tile)
{
  byte *di = tilebuf+(tile<<7);

  for(int si=0; si<256; si+=2)
  {
    *di = (src[si]&15) + ((src[si+1]&15) << 4);
    di++;
  }
}


// packs a whole set of tiles from old size to new size
void pack_tiles(byte *buf)
{
  int di = 0;
  for(int si=0; si<TILEBUF_SIZE; si+=2)
    buf[di++] = (buf[si]&15) + ((buf[si+1]&15) << 4);

  for( ; di<NEWTILE_SIZE2; di++)
    buf[di]=0;
}



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
  unpack_tile(tile>>2);

//  to go to 24-bit color, do this kind of thing...
//  ((long *)bmp->line[y])[x] = color;

  switch(flip)
  {
  case 1: {  // 1 byte at a time
    byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
    for(int dy=0; dy<8; dy++) {
      byte *di = &(dest->line[y+dy][x+7]);
      for(int i=0; i<8; i++)
        *(di--) = *(si++) + cset;
      si+=8;
      }
    }
    break;

  case 2: {  // 4 bytes at a time
    dword *si = ((dword*)unpackbuf) + ((tile&2)<<4) + ((tile&1)<<1);
     for(int dy=7; dy>=0; dy--) {
      dword *di=&((dword*)dest->line[y+dy])[x>>2];
      *(di++) = *(si++) + lcset;
      *(di++) = *(si++) + lcset;
      si+=2;
      }
    }
    break;

  case 3: {  // 1 byte at a time
    byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
    for(int dy=7; dy>=0; dy--) {
      byte *di = &(dest->line[y+dy][x+7]);
      for(int i=0; i<8; i++)
        *(di--) = *(si++) + cset;
      si+=8;
      }
    }
    break;

  default: { // 4 bytes at a time
    dword *si = ((dword*)unpackbuf) + ((tile&2)<<4) + ((tile&1)<<1);
    for(int dy=0; dy<8; dy++) {
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

  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile>>2);
  byte *si = unpackbuf + ((tile&2)<<6) + ((tile&1)<<3);
  if(flip&1)
    si+=7;

  if((flip&2)==0)
  {
    if(y<0)
      si+=(0-y)<<4;
    for(int dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); dy++)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+7<dest->w) {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<8; dx++)
        {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
        }
      }
      else
      {
        for(int i=0; i<8; i++)
        {
          if(x+i<dest->w)
          {
            if(*si)
              *di=*si+cset;
            di++;
          }
          flip&1 ? si-- : si++;
        }
      }
      if(flip&1)
        si+=32;
      else
        si+=8;
    }
  }
  else
  {
    if(y+7>=dest->h)
      si+=(8+y-dest->h)<<4;
    for(int dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); dy--)
    {
      byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+7<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<8; dx++)
        {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
        }
      }
      else
      {
        for(int i=0; i<8; i++)
        {
          if(x+i<dest->w)
          {
            if(*si)
              *di=*si+cset;
            di++;
          }
          flip&1 ? si-- : si++;
        }
      }
      if(flip&1)
        si+=32;
      else
        si+=8;
    }
  }
}



void puttile16(BITMAP* dest,int tile,int x,int y,int cset,int flip)
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
  dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;

  unpack_tile(tile);

  switch(flip) {
  case 1: {
    byte *si = unpackbuf;
    for(int dy=0; dy<16; dy++) {
      // 1 byte at a time
      byte *di = &(dest->line[y+dy][x+15]);
      for(int i=0; i<16; i++)
        *(di--) = *(si++) + cset;
      }
    } break;

  case 2: {
    dword *si = (dword*)unpackbuf;
     for(int dy=15; dy>=0; dy--) {
      // 4 bytes at a time
      dword *di=&((dword*)dest->line[y+dy])[x>>2];
      for(int i=0; i<16; i+=4)
        *(di++) = *(si++) + lcset;
      }
    } break;

  case 3: {
    byte *si = unpackbuf;
    for(int dy=15; dy>=0; dy--) {
      // 1 byte at a time
      byte *di = &(dest->line[y+dy][x+15]);
      for(int i=0; i<16; i++)
        *(di--) = *(si++) + cset;
      }
    } break;

  default: {
    dword *si = (dword*)unpackbuf;
    for(int dy=0; dy<16; dy++) {
      // 4 bytes at a time
      dword *di=&((dword*)dest->line[y+dy])[x>>2];
      for(int i=0; i<16; i+=4)
        *(di++) = *(si++) + lcset;
      }
    } break;
  }
}




void overtile16(BITMAP* dest,int tile,int x,int y,int cset,int flip)
{
  if(x<-15 || y<-15)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;
  if(tile<0 || tile>=NEWMAXTILES) {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
    }

  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile);
  byte *si = unpackbuf;
  byte *di;
  if(flip&1)
    si+=15;

  if((flip&2)==0)
  {
    if(y<0)
      si+=(0-y)<<4;

    for(int dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); dy++)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w) {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; dx++)
        {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
        }
      }
      else
      {
        for(int i=0; i<16; i++)
        {
          if(x+i<dest->w)
          {
            if(*si)
              *di=*si+cset;
            di++;
          }
          flip&1 ? si-- : si++;
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

    for(int dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); dy--)
    {
      di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; dx++)
        {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
        }
      }
      else
      {
        for(int i=0; i<16; i++)
        {
          if(x+i<dest->w)
          {
            if(*si)
              *di=*si+cset;
            di++;
            }
          flip&1 ? si-- : si++;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
}


void overtilerotate16(BITMAP* dest,int tile,int x,int y,int cset,int flip,int rot)
{
  if(x<-15 || y<-15)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;
  if(tile<0 || tile>=NEWMAXTILES) {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
    }

  BITMAP *tmpbmp = create_bitmap(dest->w, dest->h);
  BITMAP *tmpbmp2= create_bitmap(16, 16);
  BITMAP *tmpbmp3= create_bitmap(16, 16);
  clear(tmpbmp);
  clear(tmpbmp2);
  clear(tmpbmp3);
//  blit(dest, tmpbmp, 0, 0, 0, 0, dest->w, dest->h);
  
  cset &= 15;
  cset <<= CSET_SHFT;
  unpack_tile(tile);
  byte *si = unpackbuf;
  byte *di;
  if(flip&1)
    si+=15;

  if((flip&2)==0)
  {
    if(y<0)
      si+=(0-y)<<4;

    for(int dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<tmpbmp->h); dy++)
    {
      di = &(tmpbmp->line[y+dy][x<0 ? 0 : x]);
      if(x+15<tmpbmp->w) {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; dx++)
        {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
        }
      }
      else
      {
        for(int i=0; i<16; i++)
        {
          if(x+i<tmpbmp->w)
          {
            if(*si)
              *di=*si+cset;
            di++;
          }
          flip&1 ? si-- : si++;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
  else
  {
    if(y+15>=tmpbmp->h)
      si+=(16+y-tmpbmp->h)<<4;

    for(int dy=(y+15>=tmpbmp->h ? tmpbmp->h-y-1 : 15); (dy>=0)&&(dy+y>=0); dy--)
    {
      di = &(tmpbmp->line[y+dy][x<0 ? 0 : x]);
      if(x+15<tmpbmp->w)
      {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; dx++)
        {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
        }
      }
      else
      {
        for(int i=0; i<16; i++)
        {
          if(x+i<tmpbmp->w)
          {
            if(*si)
              *di=*si+cset;
            di++;
            }
          flip&1 ? si-- : si++;
        }
      }
      if(flip&1)
        si+=32;
    }
  }
  int tempx, tempy;

  blit(tmpbmp, tmpbmp2, x, y, 0, 0, 16, 16);

  for (tempx=0; tempx<16; tempx++) {
    for (tempy=0; tempy<16; tempy++) {
      switch (rot%4) {
        case 0: putpixel(tmpbmp3, tempx, tempy, (getpixel(tmpbmp2, tempx, tempy))); break;
        case 1: putpixel(tmpbmp3, tempx, tempy, (getpixel(tmpbmp2, tempx, 15-tempy))); break;
        case 2: putpixel(tmpbmp3, tempx, tempy, (getpixel(tmpbmp2, 15-tempx, 15-tempy))); break;
        case 3: putpixel(tmpbmp3, tempx, tempy, (getpixel(tmpbmp2, 15-tempy, tempx))); break;
      }
    }
  }
  draw_sprite(dest, tmpbmp3, x, y);
  destroy_bitmap(tmpbmp);
  destroy_bitmap(tmpbmp2);
  destroy_bitmap(tmpbmp3);
}


void putblock8(BITMAP *dest,int tile,int x,int y,int csets[],int flip,int mask)
{
  int t[4];
  for(int i=0; i<4; i++)
    t[i]=tile+i;

  switch(mask)
  {
  case 1:
    puttile8(dest,tile,x,y,csets[0],flip);
    break;

  case 3:
    if(flip&2)
    {
      swap(t[0],t[1]);
      swap(csets[0],csets[1]);
    }
    puttile8(dest,t[0],x,y,  csets[0],flip);
    puttile8(dest,t[1],x,y+8,csets[1],flip);
    break;

  case 5:
    if(flip&1)
    {
      swap(t[0],t[1]);
      swap(csets[0],csets[1]);
    }
    puttile8(dest,t[0],x,  y,csets[0],flip);
    puttile8(dest,t[1],x+8,y,csets[1],flip);
    break;

  case 15:
    if(flip&1)
    {
      swap(t[0],t[2]);
      swap(t[1],t[3]);
      swap(csets[0],csets[2]);
      swap(csets[1],csets[3]);
    }
    if(flip&2)
    {
      swap(t[0],t[1]);
      swap(t[2],t[3]);
      swap(csets[0],csets[1]);
      swap(csets[2],csets[3]);
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
  for(int i=0; i<4; i++)
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
      swap(csets[0],csets[1]);
    }
    overtile8(dest,t[0],x,y,  csets[0],flip);
    overtile8(dest,t[1],x,y+8,csets[1],flip);
    break;

  case 5:
    if(flip&1)
    {
      swap(t[0],t[1]);
      swap(csets[0],csets[1]);
    }
    overtile8(dest,t[0],x,  y,csets[0],flip);
    overtile8(dest,t[1],x+8,y,csets[1],flip);
    break;

  case 15:
    if(flip&1)
    {
      swap(t[0],t[2]);
      swap(t[1],t[3]);
      swap(csets[0],csets[2]);
      swap(csets[1],csets[3]);
    }
    if(flip&2)
    {
      swap(t[0],t[1]);
      swap(t[2],t[3]);
      swap(csets[0],csets[1]);
      swap(csets[2],csets[3]);
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

void putcombo(BITMAP* dest,int x,int y,int cmbdat,int page)
{
  int cset=(cmbdat&0x0700)>>8;
  newcombo c = combobuf[(cmbdat&0xFF)+(page<<8)];  //strip the flags
  if(!(c.csets&0xF0) || !(c.csets&0x0F))
    puttile16(dest,c.tile,x,y,cset,c.flip);
//    puttile16(dest,c.drawtile,x,y,cset,c.flip);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    if(cofs&8)
      cofs |= ~int(0xF);

    for(int i=0; i<4; i++)
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

    putblock8(dest,c.tile<<2,x,y,csets,c.flip,15);
//    putblock8(dest,c.drawtile<<2,x,y,csets,c.flip,15);
  }
}


void overcombo(BITMAP* dest,int x,int y,int cmbdat,int page)
{
  int cset=(cmbdat&0x0700)>>8;
  newcombo c = combobuf[(cmbdat&0xFF)+(page<<8)];
  if(!(c.csets&0xF0) || !(c.csets&0x0F))
    overtile16(dest,c.tile,x,y,cset,c.flip);
//    overtile16(dest,c.drawtile,x,y,cset,c.flip);
  else
  {
    int csets[4];
    int cofs = c.csets&15;
    if(cofs&8)
      cofs |= ~int(0xF);

    for(int i=0; i<4; i++)
      csets[i] = c.csets&(16<<i) ? cset + cofs : cset;

    overblock8(dest,c.tile<<2,x,y,csets,c.flip,15);
//    overblock8(dest,c.drawtile<<2,x,y,csets,c.flip,15);
  }
}



/*

void mirrorblock(BITMAP* dest,int tile,int x,int y,int cset,bool odd)
{
  overtile8(dest,tile,x,y,cset,0);
  overtile8(dest,tile+1,x,y+8,cset,0);
  x+=8;
  if(odd)
    x--;
  overtile8(dest,tile,x,y,cset,1);
  overtile8(dest,tile+1,x,y+8,cset,1);
}

void mirrorblock_v(BITMAP* dest,int tile,int x,int y,int cset,bool odd)
{
  overtile8(dest,tile+1,x,y,cset,2);
  overtile8(dest,tile,x,y+8,cset,2);
  x+=8;
  if(odd)
    x--;
  overtile8(dest,tile+1,x,y,cset,3);
  overtile8(dest,tile,x,y+8,cset,3);
}


*/

/* end of tiles.cc */
