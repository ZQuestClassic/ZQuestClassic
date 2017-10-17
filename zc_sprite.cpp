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

/*
void sprite::check_conveyor()
{
  if (conveyclk<=0)
  {
    int ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
    if((ctype>=cOLD_CVUP) && (ctype<=cOLD_CVRIGHT))
    {
      switch (ctype-cOLD_CVUP)
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
*/

void sprite::check_conveyor()
{
  int deltax=0;
  int deltay=0;
  if (conveyclk<=0 && (z==0 || (tmpscr->flags2&fAIRCOMBOS)))
  {
    int ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
    deltax=combo_class_buf[ctype].conveyor_x_speed;
    deltay=combo_class_buf[ctype].conveyor_y_speed;
    if(deltax!=0||deltay!=0)
    {
      if (deltay<0&&!_walkflag(x,y+8-2,2))
      {
        y=y-abs(deltay);
      }
      else if (deltay>0&&!_walkflag(x,y+15+2,2))
      {
        y=y+abs(deltay);
      }
      if (deltax<0&&!_walkflag(x-2,y+8,1))
      {
        x=x-abs(deltax);
      }
      else if (deltax>0&&!_walkflag(x+15+2,y+8,1))
      {
        x=x+abs(deltax);
      }
    }
  }
}

void movingblock::push(fix bx,fix by,int d2,int f)
{
  trigger=false;
  endx=x=bx; endy=y=by; dir=d2; oldflag=f;
  word *di = &(tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]);
  byte *ci = &(tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]);
  //   bcombo = ((*di)&0xFF)+(tmpscr->cpage<<8);
  bcombo =  tmpscr->data[(int(y)&0xF0)+(int(x)>>4)];
  oldcset = tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)];
  cs     =  (isdungeon() && !get_bit(quest_rules, qr_PUSHBLOCKCSETFIX)) ? 9 : oldcset;
  tile = combobuf[bcombo].tile;
  flip = combobuf[bcombo].flip;
  //   cs = ((*di)&0x700)>>8;
  *di = tmpscr->undercombo;
  *ci = tmpscr->undercset;
  putcombo(scrollbuf,x,y,*di,*ci);
  clk=32;
  blockmoving=true;
  
  // I'm moving this here. -L 7/11/9
  if(!isdungeon())
  {
      if(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
         || combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2)
      {
        if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
      }
  }
}

bool movingblock::animate(int index)
{
  //these are here to bypass compiler warnings about unused arguments
  index=index;

  if(clk<=0)
    return false;

  move((fix)0.5);

  if(--clk==0)
  {
    bool bhole=false;
    blockmoving=false;
    int f1 = tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)];
    int f2 = MAPCOMBOFLAG(x,y);

    tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]=bcombo;
    tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]=oldcset;

    if ((f1==mfBLOCKTRIGGER)||f2==mfBLOCKTRIGGER)
    {
      trigger=true;
      tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfPUSHED;
      //the above line used to be in the following if statement.
      //However, it caused inherent-flag pushblocks to not lock into
      //block trigger combos unless the block trigger is also an
      //inherent flag
      /*
      if(f2==mfBLOCKTRIGGER)
      {
        tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfPUSHED;
      }
      */
    }
    if ((f1==mfBLOCKHOLE)||f2==mfBLOCKHOLE)
    {
      tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]+=1;
      bhole=true;
      //tmpscr->cset[(int(y)&0xF0)+(int(x)>>4)]=;
    }
    if (bhole)
    {
      tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfNONE;
    }
    else
    {
      f2 = MAPCOMBOFLAG(x,y);
      if (!((f2==mfPUSHUDINS && dir<=down) ||
           (f2==mfPUSHLRINS && dir>=left) ||
           (f2==mfPUSHUINS && dir==up) ||
           (f2==mfPUSHDINS && dir==down) ||
           (f2==mfPUSHLINS && dir==left) ||
           (f2==mfPUSHRINS && dir==right) ||
           (f2==mfPUSH4INS)))
      {
        tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=mfPUSHED;
      }
    }
    if (oldflag>=mfPUSHUDINS&&oldflag&&!trigger&&!bhole)
    {
      tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=oldflag;
    }
    for (int i=0; i<176; i++)
    {
      if (tmpscr->sflag[i]==mfBLOCKTRIGGER||combobuf[tmpscr->data[i]].flag==mfBLOCKTRIGGER)
      {
        trigger=false;
      }
    }

    //triggers a secret
    f2 = MAPCOMBOFLAG(x,y);
    if ((oldflag==mfPUSHUD || oldflag==mfPUSH4 || (oldflag>=mfPUSHLR && oldflag<=mfPUSHR)) ||
        ((oldflag<mfPUSHUDNS || oldflag>mfPUSHRINS) && (f2==mfPUSHUD || f2==mfPUSH4 || (f2>=mfPUSHLR && f2<=mfPUSHR))) ||
        trigger)
        //if(oldflag<mfPUSHUDNS||trigger)
    {
      if(hiddenstair(0,true))
      {
        sfx(tmpscr->secretsfx);
      }
      else
      {
        hidden_entrance(0,true,true);
        if((combobuf[bcombo].type == cPUSH_WAIT) ||
            (combobuf[bcombo].type == cPUSH_HW) ||
            (combobuf[bcombo].type == cPUSH_HW2) || trigger)
        {
          sfx(tmpscr->secretsfx);
        }
      }

      if(isdungeon() && tmpscr->flags&fSHUTTERS)
      {
        opendoors=8;
      }
    }
    putcombo(scrollbuf,x,y,bcombo,cs);
  }
  return false;
}

/*** end of sprite.cc ***/
 
