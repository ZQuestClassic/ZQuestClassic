/*
  tiles.cc
  Jeremy Craner, 1999
  Tile drawing routines for zelda.cc
*/


extern byte *tilebuf;
extern combo *combobuf;


void puttile8(BITMAP* dest,int tile,int x,int y,byte cset,int flip)
{
if(x<0 || y<0)
  return;
if(y > dest->h-8)
  return;
if(y == dest->h-8 && x > dest->w-8)
  return;

cset <<= CSET_SHFT;
dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;
if(flip==0) {
  dword *si = ((dword*)tilebuf)+(tile<<4);
  for(int dy=0; dy<8; dy++) {
    // 4 bytes at a time
    dword *di=&((dword*)dest->line[y+dy])[x>>2];
    *(di++) = *(si++) + lcset;
    *(di++) = *(si++) + lcset;
    }
  }
if(flip==1) {
  byte *si = tilebuf+(tile<<6);
  for(int dy=0; dy<8; dy++) {
    // 1 byte at a time
    byte *di = &(dest->line[y+dy][x+7]);
    for(int i=0; i<8; i++)
      *(di--) = *(si++) + cset;
    }
   }
if(flip==2) {
  dword *si = ((dword*)tilebuf)+(tile<<4);
   for(int dy=7; dy>=0; dy--) {
    // 4 bytes at a time
    dword *di=&((dword*)dest->line[y+dy])[x>>2];
    *(di++) = *(si++) + lcset;
    *(di++) = *(si++) + lcset;
    }
   }
if(flip==3) {
  byte *si = tilebuf+(tile<<6);
  for(int dy=7; dy>=0; dy--) {
    // 1 byte at a time
    byte *di = &(dest->line[y+dy][x+7]);
    for(int i=0; i<8; i++)
      *(di--) = *(si++) + cset;
    }
  }
}




void overtile8(BITMAP* dest,int tile,int x,int y,byte cset,int flip)
{
if(x<-7 || y<-7)
  return;
if(y > dest->h)
  return;
if(y == dest->h && x > dest->w)
  return;

cset <<= CSET_SHFT;
byte *si = tilebuf+(tile<<6);
if(flip&1)
  si+=7;
if((flip&2)==0) {
  if(y<0)
    si+=(0-y)<<3;
  for(int dy=(y<0 ? 0-y : 0); (dy<8)&&(dy+y<dest->h); dy++) {
    byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
    if(x+7<dest->w) {
      if(x<0)
        flip&1 ? si-=0-x : si+=0-x;
      for(int dx=(x<0 ? 0-x : 0); dx<8; dx++) {
        if(*si)
          *di=*si+cset;
        di++;
        flip&1 ? si-- : si++;
        }
      }
    else {
      for(int i=0; i<8; i++) {
        if(x+i<dest->w) {
          if(*si)
            *di=*si+cset;
          di++;
          }
        flip&1 ? si-- : si++;
        }
      }
    if(flip&1)
      si+=16;
    }
  }
if((flip&2)==2) {
  if(y+7>=dest->h)
    si+=(8+y-dest->h)<<3;
  for(int dy=(y+7>=dest->h ? dest->h-y-1 : 7); (dy>=0)&&(dy+y>=0); dy--) {
    byte* di = &(dest->line[y+dy][x<0 ? 0 : x]);
    if(x+7<dest->w) {
      if(x<0)
        flip&1 ? si-=0-x : si+=0-x;
      for(int dx=(x<0 ? 0-x : 0); dx<8; dx++) {
        if(*si)
          *di=*si+cset;
        di++;
        flip&1 ? si-- : si++;
        }
      }
    else {
      for(int i=0; i<8; i++) {
        if(x+i<dest->w) {
          if(*si)
            *di=*si+cset;
          di++;
          }
        flip&1 ? si-- : si++;
        }
      }
    if(flip&1)
      si+=16;
    }
  }

}




void putblock(BITMAP* dest,int tile,int x,int y,int cset,int flip,int mask)
{
int t1=tile;
int t2=tile+1;
int t3=tile+2;
int t4=tile+3;
if(mask==1)
  puttile8(dest,t1,x,y,cset,flip);
if(mask==3) {
  if(flip&2)
    swap(t1,t2);
  puttile8(dest,t1,x,y,cset,flip);
  puttile8(dest,t2,x,y+8,cset,flip);
  }
if(mask==5) {
  if(flip&1)
    swap(t1,t2);
  puttile8(dest,t1,x,y,cset,flip);
  puttile8(dest,t2,x+8,y,cset,flip);
  }
if(mask==15) {
  if(flip&1) {
    swap(t1,t3);
    swap(t2,t4);
    }
  if(flip&2) {
    swap(t1,t2);
    swap(t3,t4);
    }
  puttile8(dest,t1,x,y,cset,flip);
  puttile8(dest,t2,x,y+8,cset,flip);
  puttile8(dest,t3,x+8,y,cset,flip);
  puttile8(dest,t4,x+8,y+8,cset,flip);
  }
}



void overblock(BITMAP* dest,int tile,int x,int y,int cset,int flip,int mask)
{
int t1=tile;
int t2=tile+1;
int t3=tile+2;
int t4=tile+3;
if(mask==1)
  overtile8(dest,t1,x,y,cset,flip);
if(mask==3) {
  if(flip&2)
    swap(t1,t2);
  overtile8(dest,t1,x,y,cset,flip);
  overtile8(dest,t2,x,y+8,cset,flip);
  }
if(mask==5) {
  if(flip&1)
    swap(t1,t2);
  overtile8(dest,t1,x,y,cset,flip);
  overtile8(dest,t2,x+8,y,cset,flip);
  }
if(mask==15) {
  if(flip&1) {
    swap(t1,t3);
    swap(t2,t4);
    }
  if(flip&2) {
    swap(t1,t2);
    swap(t3,t4);
    }
  overtile8(dest,t1,x,y,cset,flip);
  overtile8(dest,t2,x,y+8,cset,flip);
  overtile8(dest,t3,x+8,y,cset,flip);
  overtile8(dest,t4,x+8,y+8,cset,flip);
  }
}


void puttile16(BITMAP* dest,int tile,int x,int y,byte cset,int flip)
{
  if(x<0 || y<0)
    return;
  if(y > dest->h-16)
    return;
  if((y == dest->h-16) && (x > dest->w-16))
    return;
  if(tile<0 || tile>=600) {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
    }


  cset <<= CSET_SHFT;
  dword lcset = (cset<<24)+(cset<<16)+(cset<<8)+cset;

  switch(flip) {
  case 1: {
    byte *si = tilebuf+(tile<<8);
    for(int dy=0; dy<16; dy++) {
      // 1 byte at a time
      byte *di = &(dest->line[y+dy][x+15]);
      for(int i=0; i<16; i++)
        *(di--) = *(si++) + cset;
      }
    } break;

  case 2: {
    dword *si = ((dword*)tilebuf)+(tile<<6);
     for(int dy=15; dy>=0; dy--) {
      // 4 bytes at a time
      dword *di=&((dword*)dest->line[y+dy])[x>>2];
      for(int i=0; i<16; i+=4)
        *(di++) = *(si++) + lcset;
      }
    } break;

  case 3: {
    byte *si = tilebuf+(tile<<8);
    for(int dy=15; dy>=0; dy--) {
      // 1 byte at a time
      byte *di = &(dest->line[y+dy][x+15]);
      for(int i=0; i<16; i++)
        *(di--) = *(si++) + cset;
      }
    } break;

  default: {
    dword *si = ((dword*)tilebuf)+(tile<<6);
    for(int dy=0; dy<16; dy++) {
      // 4 bytes at a time
      dword *di=&((dword*)dest->line[y+dy])[x>>2];
      for(int i=0; i<16; i+=4)
        *(di++) = *(si++) + lcset;
      }
    } break;
  }
}




void overtile16(BITMAP* dest,int tile,int x,int y,byte cset,int flip)
{
  if(x<-15 || y<-15)
    return;
  if(y > dest->h)
    return;
  if(y == dest->h && x > dest->w)
    return;
  if(tile<0 || tile>=600) {
    rectfill(dest,x,y,x+15,y+15,0);
    return;
    }

  cset <<= CSET_SHFT;
  byte *si = tilebuf+(tile<<8);
  if(flip&1)
    si+=15;

  if((flip&2)==0) {
    if(y<0)
      si+=(0-y)<<4;
    for(int dy=(y<0 ? 0-y : 0); (dy<16)&&(dy+y<dest->h); dy++) {
      byte *di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w) {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; dx++) {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
          }
        }
      else {
        for(int i=0; i<16; i++) {
          if(x+i<dest->w) {
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
  else {
    if(y+15>=dest->h)
      si+=(16+y-dest->h)<<4;
    for(int dy=(y+15>=dest->h ? dest->h-y-1 : 15); (dy>=0)&&(dy+y>=0); dy--) {
      byte *di = &(dest->line[y+dy][x<0 ? 0 : x]);
      if(x+15<dest->w) {
        if(x<0)
          flip&1 ? si-=0-x : si+=0-x;
        for(int dx=(x<0 ? 0-x : 0); dx<16; dx++) {
          if(*si)
            *di=*si+cset;
          di++;
          flip&1 ? si-- : si++;
          }
        }
      else {
        for(int i=0; i<16; i++) {
          if(x+i<dest->w) {
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


//  cmbdat: ffffsssc cccccccc
//          (f:flags, s:cset, c:combo)

void putcombo(BITMAP* dest,int x,int y,word cmbdat)
{
  int cset=(cmbdat&0x0E00)>>9;
  combo c = combobuf[cmbdat&0x1FF];
  puttile16(dest,c.tile,x,y,cset,c.flip);
}


void overcombo(BITMAP* dest,int x,int y,word cmbdat)
{
  int cset=(cmbdat&0x0E00)>>9;
  combo c = combobuf[cmbdat&0x1FF];
  overtile16(dest,c.tile,x,y,cset,c.flip);
}


/* end of tiles.cc */
