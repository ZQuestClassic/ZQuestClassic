//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#include "sprite.h"
#include "zelda.h"
#include "maps.h"
#include "tiles.h"

void sprite::check_conveyor()
{
  if (conveyclk<=0)
  {
    int ctype=(combobuf[MAPDATA(x+8,y+8)].type);
    if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
    {
      switch (ctype-cCVUP)
      {
        case up:
          if(!_walkflag(x,y+8-2,2))
          {
            y=y-2;
          }
          break;
        case down:
          if(!_walkflag(x,y+15+2,2))
          {
            y=y+2;
          }
          break;
        case left:
          if(!_walkflag(x-2,y+8,1))
          {
            x=x-2;
          }
          break;
        case right:
          if(!_walkflag(x+15+2,y+8,1))
          {
            x=x+2;
          }
          break;
      }
    }
  }
}

void movingblock::push(fix bx,fix by,int d,int f)
{
  trigger=false;
  endx=x=bx; endy=y=by; dir=d; oldflag=f;
  word *di = &(tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]);
  byte *ci = &(tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]);
  //   bcombo = ((*di)&0xFF)+(tmpscr->cpage<<8);
  bcombo =  tmpscr->data[(int(y)&0xF0)+(int(x)>>4)];
  cs     =  tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)];
  tile = combobuf[bcombo].tile;
  flip = combobuf[bcombo].flip;
  //   cs = ((*di)&0x700)>>8;
  *di = tmpscr->undercombo;
  *ci = tmpscr->undercset;
  putcombo(scrollbuf,x,y,*di,*ci);
  clk=32;
  blockmoving=true;
}

bool movingblock::animate(int index)
{
  if(clk<=0)
    return false;

  move((fix)0.5);

  if(--clk==0)
  {
    blockmoving=false;
    tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]=bcombo;
    tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]=cs;
    if (tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]==mfBLOCKTRIGGER)
    {
      trigger=true;
    }
    tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfPUSHED;
    if (oldflag>=mfPUSHUDINS&&!trigger)
    {
      tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=oldflag;
    }
    for (int i=0; i<176; i++)
    {
      if (tmpscr->sflag[i]==mfBLOCKTRIGGER)
      {
        trigger=false;
      }
    }

    if (oldflag<mfPUSHUDNS||trigger)                        //triggers a secret
    {
      if(hiddenstair(0,true))
      {
        if(!nosecretsounds)
        {
          sfx(WAV_SECRET,128);
        }
      }
      else
      {
        hidden_entrance(0,true,true);
        if(((combobuf[bcombo].type == cPUSH_WAIT) ||
          (combobuf[bcombo].type == cPUSH_HW) ||
          (combobuf[bcombo].type == cPUSH_HW2) || trigger) &&
          (!nosecretsounds))
        {
          sfx(WAV_SECRET,128);
        }
      }

      if(isdungeon() && tmpscr->flags&fSHUTTERS)
      {
        opendoors=8;
      }

      if(!isdungeon())
      {
        if(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
          || combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2)
        {
          setmapflag(mSECRET);
        }
      }
    }
    putcombo(scrollbuf,x,y,bcombo,cs);
  }
  return false;
}

/*** end of sprite.cc ***/
