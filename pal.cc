//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  pal.cc
//
//  Palette code for ZC.
//
//--------------------------------------------------------


#include <allegro.h>

#include "zdefs.h"
#include "zelda.h"
#include "colors.h"
#include "zsys.h"
#include "pal.h"


int CSET_SIZE = 16;     // this is only changed to 4 in the NES title screen
int CSET_SHFT = 4;      // log2 of CSET_SIZE


RGB _RGB(byte *si)
{
  RGB x;
  x.r = si[0];
  x.g = si[1];
  x.b = si[2];
  return x;
}

RGB _RGB(int r,int g,int b)
{
  RGB x;
  x.r = r;
  x.g = g;
  x.b = b;
  return x;
}

RGB invRGB(RGB s)
{
  RGB x;
  x.r = 63-s.r;
  x.g = 63-s.g;
  x.b = 63-s.b;
  return x;
}

RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio)
{
  RGB x;
  x.r = ( r1*(64-ratio) + r2*ratio ) >> 6;
  x.g = ( g1*(64-ratio) + g2*ratio ) >> 6;
  x.b = ( b1*(64-ratio) + b2*ratio ) >> 6;
  return x;
}


void copy_pal(RGB *src,RGB *dest)
{
  for(int i=0; i<256; i++)
    dest[i]=src[i];
}


void loadfullpal()
{
  for(int i=0; i<240; i++)
    RAMpal[i]=_RGB(colordata+i*3);
  for(int i=240; i<255; i++)
    RAMpal[i]=((RGB*)data[PAL_GUI].dat)[i];
  refreshpal=true;
}


void loadlvlpal(int level)
{
  byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;

  for(int i=0; i<16*3; i++)
  {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
  }

  for(int i=0; i<16; i++)
  {
    RAMpal[CSET(9)+i] = _RGB(si);
    si+=3;
  }

  if(!get_bit(QHeader.rules,qr1_NOLEVEL3FIX) && level==3)
    RAMpal[CSET(6)+2] = NESpal(0x37);

  refreshpal=true;
}


void loadpalset(int cset,int dataset)
{
  byte *si = colordata + CSET(dataset)*3;
  for(int i=0; i<16; i++)
  {
    RAMpal[CSET(cset)+i] = _RGB(si);
    si+=3;
  }
  refreshpal=true;
}


void ringcolor()
{
  switch(game.ring)
  {
  case 0: loadpalset(6,6); break;
  case 1: loadpalset(6,pSprite(6)); break;
  case 2: loadpalset(6,pSprite(7)); break;
  case 3: loadpalset(6,pSprite(8)); break;
  }
  refreshpal=true;
}



void loadfadepal(int dataset)
{
  byte *si = colordata + CSET(dataset)*3;

  for(int i=0; i<pdFADE*16; i++)
  {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
  }

  refreshpal=true;
}



void fade(int level,bool blackall,bool fromblack,bool total)
{
 int cx = fromblack ? 30 : 0;
 for(int i=0; i<=30; i++)
 {
  if(get_bit(QHeader.rules,qr1_FADE))
  {
    int dpos = (cx<<6)/30;
    int lpos = min(dpos,blackall?64:32);
    int last = CSET(5)-1;

    if(get_bit(QHeader.rules,qr1_FADECS5))
    {
      last += 16;
      loadpalset(5,5);
    }

    loadlvlpal(level);
    byte *si = colordata + CSET(level*pdLEVEL+poFADE1)*3;
    for(int i=0; i<16; i++)
    {
      int light = si[0]+si[1]+si[2];
      si+=3;
      fade_interpolate(RAMpal,black_palette,RAMpal,light?lpos:dpos,CSET(2)+i,CSET(2)+i);
    }
    fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(3),last);
    refreshpal=true;
  }
  else
  {
    switch(cx) {
    case 0:  loadlvlpal(level); break;
    case 10: loadfadepal(level*pdLEVEL+poFADE1); break;
    case 20: loadfadepal(level*pdLEVEL+poFADE2); break;
    case 30:
      if(blackall) {
        for(int i=0; i<pdFADE*16; i++)
          RAMpal[CSET(2)+i]=black_palette[0];
        refreshpal=true;
        }
      else
        loadfadepal(level*pdLEVEL+poFADE3);
      break;
    }
  }

  if(!get_bit(QHeader.rules,qr1_NOLEVEL3FIX) && level==3)
    RAMpal[CSET(6)+2] = NESpal(0x37);

  putsubscr(framebuf,0,0);
  advanceframe();
  if(Quit)
    break;
  fromblack ? cx-- : cx++;
 }
}


/*
void lighting(int funct)
// 1 = use candle
// 2 = exit room
// 3 = enter room
{
 switch(funct) {
  case 1:
   if(darkroom) {
     fade(DMaps[currdmap].color,false,true);
     darkroom=false;
     }
   break;
  case 2:
   if(tmpscr->flags&fDARK && !darkroom) {
     fade(DMaps[currdmap].color,false,false);
     darkroom=true;
     }
   break;
  case 3:
   if(!(tmpscr->flags&fDARK) && darkroom) {
     fade(DMaps[currdmap].color,false,true);
     darkroom=false;
     }
   break;
  }
}
*/

void lighting(int funct)
// 1 = use candle
// 2 = exit room
// 3 = enter room
// 4 = scrolling warp exit
{
 switch(funct) {
  case 1:
   if(darkroom) {
     fade(DMaps[currdmap].color,false,true,false);
     darkroom=false;
     }
   break;
  case 2:  //currently light.  room you are going to is supposed to be dark.
   if(tmpscr->flags&fDARK && !darkroom) {
     fade(DMaps[currdmap].color,false,false,false);
     darkroom=true;

   }
   break;
  case 3:  //currently dark.  room you are going to is supposed to be light.
   if((!(tmpscr->flags&fDARK) && darkroom)||(DMaps[currdmap].color!=currcset)) {
     fade(DMaps[currdmap].color,false,true,false);
     darkroom=false;
     }
   currcset=DMaps[currdmap].color;
   break;
  case 4:  //currently light.  room you are going to is supposed to be dark.
   if((tmpscr->flags&fDARK && !darkroom)||(DMaps[(tmpscr[((currscr<128)?0:1)].warpdmap2)].color!=currcset)) {
     fade(DMaps[currdmap].color,true,false,true);
     darkroom=true;
   }
   break;
  }
}


byte drycolors[11] = {0x12,0x11,0x22,0x21,0x31,0x32,0x33,0x35,0x34,0x36,0x37};

void dryuplake()
{
 if(whistleclk<0 || whistleclk>=88)
   return;
 if((++whistleclk)&7)
   return;
 if(whistleclk<88)
 {
   RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
   refreshpal=true;
 }
 else
 {
   if(hiddenstair(0,true))
     sfx(WAV_SECRET);
 }
}


void rehydratelake()
{
 if(whistleclk==-1)
   return;
 whistleclk&=0xF8;
 do {
   whistleclk-=8;
   RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
   refreshpal=true;
   advanceframe();
   if(((whistleclk>>3)&3) == 1)
     for(int i=0; i<4 && !Quit; i++)
       advanceframe();
   } while(whistleclk!=0 && !Quit);
 whistleclk=-1;
 loadpalset(3,3);
}





static int palclk[3];
static int palpos[3];

void reset_pal_cycling()
{
  for(int i=0; i<3; i++)
    palclk[i]=palpos[i]=0;
}

void cycle_palette()
{
  if(!get_bit(QHeader.rules,qr1_FADE) || darkroom)
    return;

  int level = (currscr<128) ? DMaps[currdmap].color : (isdungeon() ? 11 : 10);

  for(int i=0; i<3; i++)
  {
    palcycle c = QMisc.cycles[level][i];
    if(c.count&0xF0)
    {
      if(++palclk[i] >= c.speed)
      {
        palclk[i]=0;
        if(++palpos[i] >= (c.count>>4))
          palpos[i]=0;

        byte *si = colordata + CSET(level*pdLEVEL+poFADE1+1+palpos[i])*3;
        si += (c.first&15)*3;

        for(int col=c.first&15; col<=(c.count&15); col++)
        {
          RAMpal[CSET(c.first>>4)+col] = _RGB(si);
          si+=3;
        }
        refreshpal=true;
      }
    }
  }
}



/* end of pal.cc */
