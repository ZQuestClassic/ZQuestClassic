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

#include <strings.h>
#include "weapons.h"
#include "zelda.h"
#include "zsys.h"
#include "maps.h"
#include "tiles.h"
#include "pal.h"
#include "link.h"

/**************************************/
/***********  Weapon Class  ***********/
/**************************************/

byte boomframe[16] = {0,0,1,0,2,0,1,1,0,1,1,3,2,2,1,2};
byte bszboomflip[4] = {0,2,3,1};

void weapon::seekLink()
{
  angular = true;
  angle = atan2(double(LinkY()-y),double(LinkX()-x));
  if(angle==-PI || angle==PI) dir=left;
  else if(angle==-PI/2) dir=up;
  else if(angle==PI/2)  dir=down;
  else if(angle==0)     dir=right;
  else if(angle<-PI/2)  dir=l_up;
  else if(angle<0)      dir=r_up;
  else if(angle>PI/2)   dir=l_down;
  else                  dir=r_down;
}

void weapon::seekEnemy(int j)
{
  angular = true;
  fix mindistance=(fix)1000000;
  fix tempdistance;
  if ((j==-1)||(j>=GuyCount()))
  {
    j=-1;
    for (int i=0; i<GuyCount(); i++)
    {
      //        tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
      tempdistance=distance(x,y,GuyX(i),GuyY(i));
      if ((tempdistance<mindistance)&&(GuyID(i)>=10) && !GuySuperman(i))
      {
        mindistance=tempdistance;
        j=i;
      }
    }
  }
  if (j==-1)
  {
    return;
  }
  angle = atan2(double(GuyY(j)-y),double(GuyX(j)-x));
  if(angle==-PI || angle==PI) dir=left;
  else if(angle==-PI/2) dir=up;
  else if(angle==PI/2)  dir=down;
  else if(angle==0)     dir=right;
  else if(angle<-PI/2)  dir=l_up;
  else if(angle<0)      dir=r_up;
  else if(angle>PI/2)   dir=l_down;
  else                  dir=r_down;
}

weapon::weapon(fix X,fix Y,int Id,int Type,int pow,int Dir) : sprite()
{
  x=X; y=Y; id=Id; type=Type; power=pow; dir=max(Dir,0);
  clk=clk2=flip=misc=misc2=0;
  flash=wid=aframe=csclk=0;
  ignorecombo=-1;
  step=0;
  dead=-1;
  bounce=ignoreLink=false;
  yofs=54;
  dragging=-1;

  if(id>wEnemyWeapons)
    canfreeze=true;

  switch(id)
  {
    case wWhistle:
      xofs=1000;                                            // don't show
      x=y=hxofs=hyofs=0;
      hxsz=hysz=255;                                        // hit the whole screen
      break;
    case wWind:
      LOADGFX(wWIND);
      clk=-14;
      step=2;
      break;
    case wBeam:
      step = 3;
      LOADGFX(current_item(itype_sword,true)-1+wSWORD);
      flash = 1;
      cs = 6;
      switch(dir)
      {
        case down:  flip=get_bit(quest_rules,qr_SWORDWANDFLIPFIX)?3:2;
        case up:    hyofs=2; hysz=12; break;
        case left:  flip=1;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); hxofs=2; hxsz=12; yofs = (BSZ ? 59 : 57); break;
      }
      break;
    case wArrow:
      if (current_item(itype_arrow,true)<3)
      {
        LOADGFX(wARROW+type-1);
      }
      else
      {
        LOADGFX(wGARROW);
      }
      step=3;
      switch(dir)
      {
        case down:  flip=2;
        case up:    hyofs=2; hysz=12; break;
        case left:  flip=1;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); yofs=57; hyofs=2; hysz=14; hxofs=2; hxsz=12; break;
      }
      break;
    case wSSparkle:
      LOADGFX(wSSPARKLE);
      step=0;
      break;
    case wGSparkle:
      LOADGFX(wGSPARKLE);
      step=0;
      break;
    case wMSparkle:
      LOADGFX(wMSPARKLE);
      step=0;
      break;
    case wFSparkle:
      LOADGFX(wFSPARKLE);
      step=0;
      break;
    case wFire:
      LOADGFX(wFIRE);
      step = (type<2)?.5:0;
      hxofs = hyofs=1;
      hxsz = hysz = 14;
      if(BSZ)
        yofs+=2;
      break;
    case wBomb:
      LOADGFX(wBOMB);
      hxofs=hyofs=4; hxsz=hysz=8; id = wLitBomb;
      break;
    case wSBomb:
      LOADGFX(wSBOMB);
      hxofs=hyofs=4; hxsz=hysz=8; id = wLitSBomb;
      break;
    case wBait:
      LOADGFX(wBAIT);
      break;
    case wMagic:
      LOADGFX(wMAGIC);
      step = (BSZ ? 3 : 2.5);
      switch(dir)
      {
        case down:  flip=2;
        case up:    hyofs=2; hysz=12; break;
        case left:  flip=1;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); yofs = (BSZ ? 59 : 57); hxofs=2; hxsz=12; break;
      }
      break;
    case wBrang:
      LOADGFX(wBRANG+type-1);
      if(type>=2)
        clk2=256;
      hxofs=4; hxsz=7;
      hyofs=2; hysz=11;
      dummy_bool[0]=false;                                  //grenade armed?
      break;

    case wHookshot:
      hookshot_used=true;
      LOADGFX(wHSHEAD);
      step = 4;
      //     step = 0;
      clk2=256;
      switch(dir)
      {
        case down:  flip=2; xofs+=4;  yofs+=1; hyofs=2; hysz=12; break;
        case up:    yofs+=3; xofs-=5; hyofs=2; hysz=12; break;
        case left:  flip=1; tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs+=2; yofs=60; hxofs=2; hxsz=12; break;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs-=2; yofs=60; hxofs=2; hxsz=12; break;
      }
      break;

    case wHSHandle:
      step = 0;
      LOADGFX(wHSHANDLE);
      switch(dir)
      {
        case down:  flip=2; xofs+=4;  yofs+=1; hyofs=2; hysz=12; break;
        case up:    yofs+=3; xofs-=5; hyofs=2; hysz=12; break;
        case left:  flip=1; tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs+=2; yofs=60; hxofs=2; hxsz=12; break;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs-=2; yofs=60; hxofs=2; hxsz=12; break;
      }
      break;

    case wHSChain:
      step = 0;
      switch(dir)
      {
        case down:  LOADGFX(wHSCHAIN_V); xofs+=4;  yofs-=7;  break;
        case up:    LOADGFX(wHSCHAIN_V); xofs-=5;  yofs+=11; break;
        case left:  LOADGFX(wHSCHAIN_H); xofs+=10; yofs=60;  break;
        case right: LOADGFX(wHSCHAIN_H); xofs-=10; yofs=60;  break;
      }
      break;

      /*
         case wHSChain:
           LOADGFX(wHSCHAIN);
           step = 0;
           switch(dir) {
           case down:  xofs+=4;  yofs-=7;  break;
           case up:    xofs-=5;  yofs+=11; break;
           case left:  tile=xofs+=10; yofs=60;  break;
           case right: tile=xofs-=10; yofs=60;  break;
           }
           break;
      */

    case ewBrang:
      hxofs=4; hxsz=8;
      wid = min(max(current_item(itype_brang,true),1),3)-1+wBRANG;
      break;

    case ewFireball:
      LOADGFX(ewFIREBALL);
      step=1.75;
      misc=dir-1;
      seekLink();
      break;

    case ewRock:
      LOADGFX(ewROCK);
      hxofs=4; hxsz=8; step=3;
      break;
    case ewArrow:
      LOADGFX(ewARROW);
      step=2;
      switch(dir)
      {
        case down:  flip=2;
        case up:    xofs=-4; hxsz=8; break;
        case left:  flip=1;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); yofs=57; break;
      }
      break;
    case ewSword:
      LOADGFX(ewSWORD);
      step=3;
      switch(dir)
      {
        case down:  flip=2;
        case up:    break;
        case left:  flip=1;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); yofs=57; break;
      }
      break;
    case wRefMagic:
    case ewMagic:
      LOADGFX(ewMAGIC);
      step=3;
      switch(dir)
      {
        case down:  flip=2;
        case up:    break;
        case left:  flip=1;
        case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); yofs=57; break;
      }
      if (id==wRefMagic)
      {
        ignorecombo=(((int)y&0xF0)+((int)x>>4));
      }
      break;
    case ewFlame:
      LOADGFX(ewFLAME);
      if (dir==255)
      {
        step=2;
        seekLink();
      }
      else
      {
        if (dir>right)
        {
          step = .707;
        }
        else
        {
          step = 1;
        }
      }
      hxofs = hyofs=1;
      hxsz = hysz = 14;
      if(BSZ)
        yofs+=2;
      break;
    case ewWind:
      LOADGFX(ewWIND);
      clk=0;
      step=3;
      break;
    case wPhantom:
      switch (type)
      {
        case pDINSFIREROCKET:
          LOADGFX(wDINSFIRE1A);
          step = 4;
          break;
        case pDINSFIREROCKETRETURN:
          LOADGFX(wDINSFIRE1B);
          step = 4;
          break;
        case pDINSFIREROCKETTRAIL:
          LOADGFX(wDINSFIRES1A);
          break;
        case pDINSFIREROCKETTRAILRETURN:
          LOADGFX(wDINSFIRES1B);
          break;
        case pMESSAGEMORE:
          LOADGFX(iwMore);
          break;
        case pNAYRUSLOVEROCKET1:
          LOADGFX(wNAYRUSLOVE1A);
          step = 4;
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
        case pNAYRUSLOVEROCKETRETURN1:
          LOADGFX(wNAYRUSLOVE1B);
          step = 4;
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
        case pNAYRUSLOVEROCKETTRAIL1:
          LOADGFX(wNAYRUSLOVES1A);
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
        case pNAYRUSLOVEROCKETTRAILRETURN1:
          LOADGFX(wNAYRUSLOVES1B);
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
        case pNAYRUSLOVEROCKET2:
          LOADGFX(wNAYRUSLOVE2A);
          step = 4;
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
        case pNAYRUSLOVEROCKETRETURN2:
          LOADGFX(wNAYRUSLOVE2B);
          step = 4;
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
        case pNAYRUSLOVEROCKETTRAIL2:
          LOADGFX(wNAYRUSLOVES2A);
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
        case pNAYRUSLOVEROCKETTRAILRETURN2:
          LOADGFX(wNAYRUSLOVES2B);
          drawstyle=get_bit(quest_rules,qr_TRANSLUCENTNAYRUSLOVEROCKET)?1:0;
          break;
      }
      break;
    default:
      tile=0; break;
  }
}

void weapon::LOADGFX(int wpn)
{
  wid = wpn;
  flash = wpnsbuf[wid].misc&3;
  tile  = wpnsbuf[wid].tile;
  cs = wpnsbuf[wid].csets&15;
}

bool weapon::Dead() { return dead!=-1; }

bool weapon::clip()
{
  int c[4];
  int d=isdungeon();

  if(id>wEnemyWeapons && id<wHammer && id!=ewBrang)
  {
    c[0] = d?32:16;
    c[1] = d?128:144;
    c[2] = d?32:16;
    c[3] = d?208:224;
  }
  else if (id==wHookshot||id==wHSChain)
  {
    c[0] = d?8:0;
    c[1] = d?152:160;
    c[2] = d?8:0;
    c[3] = d?248:256;
  }
  else
  {
    c[0] = d?18:2;
    c[1] = d?144:160;
    c[2] = d?20:4;
    c[3] = d?220:236;
  }

  if(id>=wSSparkle && id<=wFSparkle)
  {
    c[0] = 0;
    c[1] = 176;
    c[2] = 0;
    c[3] = 256;
  }

  if(id==ewFlame)
  {
    c[0] = d?32:16;
    c[1] = d?128:144;
    c[2] = d?32:16;
    c[3] = d?208:224;
  }

  if(id==ewWind)
  {
    c[0] = d?32:16;
    c[1] = d?128:144;
    c[2] = d?32:16;
    c[3] = d?208:224;
  }

  if(x < c[2])
    if(dir==left || dir==l_up || dir==l_down)
      return true;
  if(x > c[3])
    if(dir==right || dir==r_up || dir==r_down)
      return true;
  if(y < c[0])
    if(dir==up || dir==l_up || dir==r_up)
      return true;
  if(y > c[1])
    if(dir==down || dir==l_down || dir==r_down)
      return true;

  if(id>wEnemyWeapons && id<wHammer)
  {
    //   if(id>wEnemyWeapons) {
    if((x<8 && dir==left)
      || (y<8 && dir==up)
      || (x>232 && dir==right)
      || (y>168 && dir==down))
      return true;
  }

  if(x<0||y<0||x>240||y>176)
    return true;

  return false;
}

bool weapon::animate(int index)
{
  // do special timing stuff
  bool hooked=false;
  static PALETTE temppal;
  switch(id)
  {
    // Link's weapons
    case wBeam:
      for (int i=0; i<current_item(itype_sword,true); i++)
      {
        if (findentrance(x,y,mfSWORDBEAM+i,true)) dead=23;
      }
      break;
    case wWhistle:
      if(clk)
        dead=1;
      break;

    case wWind:  if(type==1 && dead==-1 && x>=tmpscr->warparrivalx) dead=2; break;
    case wFire:
      if((type<2)||(type>2))                                //candles and Din's fire
      {
        if(clk==32)
        {
          step=0;
          lighting(1,dir);
        }
        if(clk==94)
        {
          dead=1;
          findentrance(x,y,mfBCANDLE,true);
          if (type>0)
          {
            findentrance(x,y,mfRCANDLE,true);
          }
          if (type>2)
          {
            findentrance(x,y,mfWANDFIRE,true);
            findentrance(x,y,mfDINSFIRE,true);
          }
        }
      }                                                     //wand fire
      else
      {
        if(clk==1)
        {
          lighting(1,dir);
        }
        if(clk==80)
        {
          dead=1;
          findentrance(x,y,mfBCANDLE,true);
          findentrance(x,y,mfRCANDLE,true);
          findentrance(x,y,mfWANDFIRE,true);
        }
      }
      break;

    case wLitBomb:
    case wBomb:
		{
			if(clk==48)
			{
				id = wBomb;
				hxofs=1000;
			}
			if(clk==49)
			{
				sfx(WAV_BOMB,pan(int(x)));
				hxofs=hyofs=-8;
				hxsz=hysz=32;
			}
			if(clk==81)
			{
				hxofs=1000;
			}
      if(clk==49)
      {
        findentrance(x,y,mfBOMB,true);
        findentrance(x,y,mfSTRIKE,true);
      }
      if (!get_bit(quest_rules,qr_NOBOMBPALFLASH))
      {
        if(clk==50 || clk==55)
        {
          memcpy(temppal, RAMpal, PAL_SIZE*sizeof(RGB));
          //grayscale entire screen
          if(get_bit(quest_rules,qr_FADE))
          {
            for(int i=CSET(0); i < CSET(15); i++)
            {
              int g = min((RAMpal[i].r*42 + RAMpal[i].g*75 + RAMpal[i].b*14) >> 7, 63);
              g = (g >> 1) + 32;
              RAMpal[i] = _RGB(g,g,g);
            }
          }
          else
          {
            // this is awkward. NES Z1 converts colors based on the global
            // NES palette. Something like RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
            for(int i=CSET(0); i < CSET(15); i++)
            {
              RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
            }
          }
          refreshpal = true;
        }
        if(clk==54 || clk==59)
        {
          // undo grayscale
          memcpy(RAMpal, temppal, PAL_SIZE*sizeof(RGB));
          refreshpal = true;
        }
      }
      if(clk==80)
      {
        bombdoor(x,y);
      }
			if(clk==84)
			{
        dead=1;
			}
			break;
		}

    case wArrow:
      if (findentrance(x,y,mfSTRIKE,true)) dead=4;
      if (findentrance(x,y,mfARROW,true)) dead=4;
      if (current_item(itype_arrow,true)>1)
      {
        if (findentrance(x,y,mfSARROW,true)) dead=4;
      }
      if (current_item(itype_arrow,true)==3)
      {
        if (findentrance(x,y,mfGARROW,true)) dead=4;
      }
      //     if ((get_bit(quest_rules,qr_SASPARKLES+current_item(itype_arrow,true)-2)) && (current_item(itype_arrow,true)>=2)) {
      //       if (!(clk%(16>>(current_item(itype_arrow,true))))) {
      if ((get_bit(quest_rules,qr_SASPARKLES+type-2)) && (type>=2))
      {
        if (!(clk%(16>>type)))
        {
          arrow_x=x;
          arrow_y=y;
          add_asparkle=type-1;
        }
      }
      break;

    case wSSparkle:
      if(clk>=(((wpnsbuf[wSSPARKLE].frames) * (wpnsbuf[wSSPARKLE].speed))-1))
      {
        dead=1;
      }
      break;

    case wGSparkle:
      if(clk>=(((wpnsbuf[wGSPARKLE].frames) * (wpnsbuf[wGSPARKLE].speed))-1))
      {
        dead=1;
      }
      break;

    case wMSparkle:
      if(clk>=(((wpnsbuf[wMSPARKLE].frames) * (wpnsbuf[wMSPARKLE].speed))-1))
      {
        dead=1;
      }
      break;

    case wFSparkle:
      if(clk>=(((wpnsbuf[wFSPARKLE].frames) * (wpnsbuf[wFSPARKLE].speed))-1))
      {
        dead=1;
      }
      break;

    case wLitSBomb:
    case wSBomb:
      if(clk==48) {
        id = wSBomb;
        hxofs=1000;
      }
      if(clk==49)
      {
        sfx(WAV_BOMB,pan(int(x)));
        for (int tx=-8; tx<=8; tx+=16)
        {
          for (int ty=-16; ty<=16; ty+=32)
          {
            findentrance(x+tx,y+ty,mfBOMB,true);
            findentrance(x+tx,y+ty,mfSBOMB,true);
            findentrance(x+tx,y+ty,mfSTRIKE,true);
          }
        }
        for (int tx=-16; tx<=16; tx+=16)
        {
          findentrance(x+tx,y,mfBOMB,true);
          findentrance(x+tx,y,mfSBOMB,true);
          findentrance(x+tx,y,mfSTRIKE,true);
        }
        hxofs=hyofs=-16;
        hxsz=hysz=48;
      }
      if (!get_bit(quest_rules,qr_NOBOMBPALFLASH))
      {
        if(clk==50 || clk==55)
        {
          memcpy(temppal, RAMpal, PAL_SIZE*sizeof(RGB));
          //grayscale entire screen
          if(get_bit(quest_rules,qr_FADE))
          {
             for(int i=CSET(0); i < CSET(15); i++)
             {
               int g = min((RAMpal[i].r*42 + RAMpal[i].g*75 + RAMpal[i].b*14) >> 7, 63);
               g = (g >> 1) + 32;
               //g = ((g - 32) >> 1) + 32;
               RAMpal[i] = _RGB(g,g,g);
             }
          }
          else
          {
            // this is awkward. NES Z1 converts colors based on the global
            // NES palette. Something like RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
            for(int i=CSET(0); i < CSET(15); i++)
            {
              RAMpal[i] = NESpal( reverse_NESpal(RAMpal[i]) & 0x30 );
            }
          }
          refreshpal = true;
        }
        if(clk==54 || clk==59)
        {
          // undo grayscale
          memcpy(RAMpal, temppal, PAL_SIZE*sizeof(RGB));
          refreshpal = true;
        }
      }
      if(clk==80) bombdoor(x,y);
      if(clk==84) dead=1;
      break;
    case wBait:
      if(clk==16*2*24) dead=1;
      break;

    case wBrang:
      for (int i=0; i<current_item(itype_brang,true); i++)
      {
        if (findentrance(x,y,mfBRANG+i,true)) dead=1;
      }
      if (findentrance(x,y,mfSTRIKE,true)) dead=1;
      ++clk2;
      if(clk2==36)
      {
        misc=1;
      }
      if(clk2>18 && clk2<52)
        step=1;
      else if(misc)
        step=2;
      else
        step=3;

      if(clk==0)                                            // delay a frame
      {
        ++clk;
        sfx(WAV_BRANG,pan(int(x)),true);
        return false;
      }

      if(clk==1)                                            // then check directional input
      {
        if(Up())
        {
          dir=up;
          if(Left())  dir=l_up;
          if(Right()) dir=r_up;
        }
        else if(Down())
        {
          dir=down;
          if(Left())  dir=l_down;
          if(Right()) dir=r_down;
        }
        else if(Left())
          dir=left;
        else if(Right())
          dir=right;
      }

      if(dead==1)
      {
        dead=-1;
        misc=1;
      }

      if(misc==1)                                           // returning
      {
        if(abs(LinkY()-y)<7 && abs(LinkX()-x)<7)
        {
          CatchBrang();
          stop_sfx(WAV_BRANG);
          if (current_item(itype_brang,true)>1)
          {
            if (dummy_bool[0])
            {
              add_grenade(x,y,current_item(itype_brang,true)>2);
              dummy_bool[0]=false;
            }
          }
          return true;
        }
        seekLink();
      }

      adjust_sfx(WAV_BRANG,pan(int(x)),true);

      if ((get_bit(quest_rules,qr_MBSPARKLES+type-2)) && (type>=2))
      {
        if (!(clk%(16>>type)))
        {
          brang_x=x-3;
          brang_y=y-3;
          add_bsparkle=type-1;
        }
      }

      break;

    case wHookshot:
      if (misc==0)
      {
        if (findentrance(x,y,mfSTRIKE,true)) dead=1;
        if (findentrance(x,y,mfHOOKSHOT,true)) dead=1;

        if (dir==up)
        {
          if ((combobuf[MAPDATA(x+2,y+7)].type==cHSGRAB))
          {
            hooked=true;
          }
          if (!hooked && _walkflag(x+2,y+7,1) && !isstepable(MAPDATA(int(x+2),int(y+7))) && combobuf[MAPDATA(x+2,y+7)].type!=cHOOKSHOTONLY)
          {
            dead=1;
          }
        }

        if (dir==down)
        {
          if ((combobuf[MAPDATA(x+12,y+12)].type==cHSGRAB))
          {
            hooked=true;
          }
          if (!hooked && _walkflag(x+12,y+12,1) && !isstepable(MAPDATA(int(x+12),int(y+12))) && combobuf[MAPDATA(x+12,y+12)].type!=cHOOKSHOTONLY)
          {
            dead=1;
          }
        }

        if (dir==left)
        {
          if ((combobuf[MAPDATA(x+6,y+13)].type==cHSGRAB))
          {
            hooked=true;
          }
          if (!hooked && _walkflag(x+6,y+13,1) && !isstepable(MAPDATA(int(x+6),int(y+13))) && combobuf[MAPDATA(x+6,y+13)].type!=cHOOKSHOTONLY)
          {
            dead=1;
          }
        }

        if (dir==right)
        {
          if ((combobuf[MAPDATA(x+9,y+13)].type==cHSGRAB))
          {
            hooked=true;
          }
          if (!hooked && _walkflag(x+9,y+13,1) && !isstepable(MAPDATA(int(x+9),int(y+13))) && combobuf[MAPDATA(x+9,y+13)].type!=cHOOKSHOTONLY)
          {
            dead=1;
          }
        }
      }

      if (hooked==true)
      {
        misc=1;
        pull_link=true;
        step=0;
      }
      ++clk2;
      if(clk==0)                                            // delay a frame
      {
        ++clk;
        sfx(WAV_HOOKSHOT,pan(int(x)),true);
        return false;
      }

      if(dead==1)
      {
        dead=-1;
        misc=1;
      }

      if(misc==1)                                           // returning
      {
        if(abs(LinkY()-y)<9 && abs(LinkX()-x)<9)
        {
          hookshot_used=false;
          if (pull_link)
          {
            hs_fix=true;
          }
          pull_link=false;
          CatchBrang();
          stop_sfx(WAV_HOOKSHOT);
          if (dragging!=-1)
          {
            getdraggeditem(dragging);
          }
          return true;
        }
        seekLink();
      }

      adjust_sfx(WAV_HOOKSHOT,pan(int(x)),true);
      break;

    case wHSHandle:
      if(hookshot_used==false) { dead=0; }
      break;

    case wPhantom:
      switch (type)
      {
        case pDINSFIREROCKET:                                             //Din's Fire Rocket
          if (!(clk%(4)))
          {
            df_x=x-3;
            df_y=y-3;
            add_df1asparkle=true;
            add_df1bsparkle=false;
          }
          break;
        case pDINSFIREROCKETRETURN:                                             //Din's Fire Rocket return
          if (!(clk%(4)))
          {
            df_x=x-3;
            df_y=y-3;
            add_df1bsparkle=true;
            add_df1asparkle=false;
          }
          if (y>=casty)
          {
            dead=1;
            castnext=true;
          }
          break;
        case pDINSFIREROCKETTRAIL:                                             //Din's Fire Rocket trail
          if(clk>=(((wpnsbuf[wDINSFIRES1A].frames) * (wpnsbuf[wDINSFIRES1A].speed))-1)) { dead=0; }
          break;
        case pDINSFIREROCKETTRAILRETURN:                                             //Din's Fire Rocket return trail
          if(clk>=(((wpnsbuf[wDINSFIRES1B].frames) * (wpnsbuf[wDINSFIRES1B].speed))-1)) { dead=0; }
          break;
        case pNAYRUSLOVEROCKET1:                                             //Nayru's Love Rocket
          if (!(clk%(4)))
          {
            nl1_x=x-3;
            nl1_y=y-3;
            add_nl1asparkle=true;
            add_nl1bsparkle=false;
          }
          break;
        case pNAYRUSLOVEROCKETRETURN1:                                             //Nayru's Love Rocket return
          if (!(clk%(4)))
          {
            nl1_x=x-3;
            nl1_y=y-3;
            add_nl1bsparkle=true;
            add_nl1asparkle=false;
          }
          if (x>=castx)
          {
            dead=1;
            castnext=true;
          }
          break;
        case pNAYRUSLOVEROCKETTRAIL1:                                             //Nayru's Love Rocket trail
          if(clk>=(((wpnsbuf[wNAYRUSLOVES1A].frames) * (wpnsbuf[wNAYRUSLOVES1A].speed))-1)) { dead=0; }
          break;
        case pNAYRUSLOVEROCKETTRAILRETURN1:                                             //Nayru's Love Rocket return trail
          if(clk>=(((wpnsbuf[wNAYRUSLOVES1B].frames) * (wpnsbuf[wNAYRUSLOVES1B].speed))-1)) { dead=0; }
          break;


        case pNAYRUSLOVEROCKET2:                                             //Nayru's Love Rocket
          if (!(clk%(4)))
          {
            nl2_x=x-3;
            nl2_y=y-3;
            add_nl2asparkle=true;
            add_nl2bsparkle=false;
          }
          break;
        case pNAYRUSLOVEROCKETRETURN2:                                             //Nayru's Love Rocket return
          if (!(clk%(4)))
          {
            nl2_x=x-3;
            nl2_y=y-3;
            add_nl2bsparkle=true;
            add_nl2asparkle=false;
          }
          if (x<=castx)
          {
            dead=1;
            castnext=true;
          }
          break;
        case pNAYRUSLOVEROCKETTRAIL2:                                             //Nayru's Love Rocket trail
          if(clk>=(((wpnsbuf[wNAYRUSLOVES2A].frames) * (wpnsbuf[wNAYRUSLOVES2A].speed))-1)) { dead=0; }
          break;
        case pNAYRUSLOVEROCKETTRAILRETURN2:                                             //Nayru's Love Rocket return trail
          if(clk>=(((wpnsbuf[wNAYRUSLOVES2B].frames) * (wpnsbuf[wNAYRUSLOVES2B].speed))-1)) { dead=0; }
          break;

      }
      break;

    case wRefMagic:
    case wMagic:
    case ewMagic:
    {
      if ((id==wMagic)&&(findentrance(x,y,mfWANDMAGIC,true))) dead=0;
      if ((id==wRefMagic)&&(findentrance(x,y,mfREFMAGIC,true))) dead=0;
      if ((id!=ewMagic)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
      int checkx=0, checky=0;
      switch (dir)
      {
        case up: checkx=x; checky=y+8;
        break;
        case down: checkx=x; checky=y;
        break;
        case left: checkx=x+8; checky=y;
        break;
        case right: checkx=x; checky=y;
        break;
      }
      if (ignorecombo!=(((int)checky&0xF0)+((int)checkx>>4)))
      {
        if (hitcombo(checkx,checky,cMIRROR))
        {
          id = wRefMagic;
          dir ^= 1;
          if(dir&2)
            flip ^= 1;
          else
            flip ^= 2;
          ignoreLink=false;
          ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
          y=(int)y&0xF0;
          x=(int)x&0xF0;
        }
        if (hitcombo(checkx,checky,cMIRRORSLASH))
        {
          id = wRefMagic;
          dir = 3-dir;
          if((dir==1)||(dir==2))
            flip ^= 3;
          tile=wpnsbuf[wid].tile;
          if (dir&2)
          {
            if (wpnsbuf[wid].frames>1)
            {
              tile+=wpnsbuf[wid].frames;
            }
            else
            {
              ++tile;
            }
          }
          ignoreLink=false;
          ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
          y=(int)y&0xF0;
          x=(int)x&0xF0;
        }
        if (hitcombo(checkx,checky,cMIRRORBACKSLASH))
        {
          id = wRefMagic;
          dir ^= 2;
          if(dir&1)
            flip ^= 2;
          else
            flip ^= 1;
          tile=wpnsbuf[wid].tile;
          if (dir&2)
          {
            if (wpnsbuf[wid].frames>1)
            {
              tile+=wpnsbuf[wid].frames;
            }
            else
            {
              ++tile;
            }
          }
          ignoreLink=false;
          ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
          y=(int)y&0xF0;
          x=(int)x&0xF0;
        }
        if (hitcombo(checkx,checky,cMAGICPRISM))
        {
          int newx, newy;
          newy=(int)y&0xF0;
          newx=(int)x&0xF0;
          for (int tdir=0; tdir<4; tdir++)
          {
            if (dir!=(tdir^1))
            {
              addLwpn(newx,newy,wRefMagic,0,power,tdir);
            }
          }
          dead=0;
        }
        if (hitcombo(checkx,checky,cMAGICPRISM4))
        {
          int newx, newy;
          newy=(int)y&0xF0;
          newx=(int)x&0xF0;
          for (int tdir=0; tdir<4; tdir++)
          {
            addLwpn(newx,newy,wRefMagic,0,power,tdir);
          }
          dead=0;
        }
        if (hitcombo(checkx,checky,cMAGICSPONGE))
        {
          dead=0;
        }
      }
    }
    break;

    // enemy weapons
    case wRefFireball:
    case ewFireball:
      if ((id==wRefFireball)&&(findentrance(x,y,mfREFFIREBALL,true))) dead=0;
      if ((id==wRefFireball)&&(findentrance(x,y,mfSTRIKE,true))) dead=0;
      //     if ((id==wRefFireball)&&(dummy_bool[0])) { //homing
      //       seekEnemy(-1);
      //     } else {
      if ((id==ewFireball)&&(!(clk%8)&&0))                  //homing (remove &&0 to activate)
      {
        seekLink();
      }
      else
      {
        switch(misc)
        {
          case up:    y-=.5; break;
          case down:  y+=.5; break;
          case left:  x-=.5; break;
          case right: x+=.5; break;
        }
        //     }
      }
      if(clk<16)
      {
        ++clk;
        if(dead>0)
          --dead;
        return dead==0;
      }
      break;

    case ewFlame:
      if(clk==32)  { step=0; lighting(1,dir); }
      if(clk==126) { dead=1; }
      break;

    case ewBrang:
      if(clk==0)
        misc2=(dir<left)?y:x;                               // save home position

      ++clk2;
      if(clk2==45)
      {
        misc=1;
        dir^=1;
      }
      if(clk2>27 && clk2<61)
        step=1;
      else if(misc)
        step=2;
      else
        step=3;

      if(dead==1)
      {
        dead=-1;
        misc=1;
        dir^=1;
      }

      if(misc==1)                                           // returning
        switch(dir)
          {
            case up:    if(y<misc2) return true; break;
            case down:  if(y>misc2) return true; break;
            case left:  if(x<misc2) return true; break;
            case right: if(x>misc2) return true; break;
          }
      break;
  }

  // move sprite, check clipping
  if(dead==-1 && clk>=0)
  {
    move(step);
    if(clip())
    {
      onhit(true);
    }
    else if(id==ewRock)
    {
      if(_walkflag(x,y,2) || _walkflag(x,y+8,2))
        onhit(true);
    }
  }

  if(bounce)
    switch(dir)
    {
      case up:    x-=1; y+=2; break;
      case down:  x+=1; y-=2; break;
      case left:  x+=2; y-=1; break;
      case right: x-=2; y-=1; break;
    }

  // update clocks
  ++clk;
  if(dead>0)
    --dead;
  return dead==0;
}

void weapon::onhit(bool clipped)
{
  onhit(clipped, 0, -1);
}

void weapon::onhit(bool clipped, int special, int linkdir)
{
  if(special==2)                                            // hit Link's mirror shield
    switch(id)
    {
      case ewFireball:
        id = wRefFireball;
        switch(linkdir)
        {
          case up:    angle += (PI - angle) * 2.0;      break;
          case down:  angle = -angle;                   break;
          case left:  angle += ((-PI/2) - angle) * 2.0; break;
          case right: angle += (( PI/2) - angle) * 2.0; break;
        }
      return;

    case ewMagic:
    case wRefMagic:
      id = wRefMagic;
      dir ^= 1;
      if(dir&2)
        flip ^= 1;
      else
        flip ^= 2;
      return;

    default:
      special = 1;                                          // check normal shield stuff
  }

  if(special==1)                                            // hit Link's shield
    switch(id)
    {
      case ewMagic:
      case ewArrow:
      case ewSword:
      case ewRock:
        bounce=true;
        dead=16;
        return;
      case ewBrang:
        if(misc==0)
      {
        clk2=256;
        misc=1;
        dir^=1;
    }
    return;
  }

  switch(id)
  {
    case wLitBomb:  if(!clipped) dead=1;
    case wLitSBomb: if(!clipped) dead=1;
    case wWhistle:
    case wBomb:
    case wSBomb:
    case wBait:
    case wFire:
    case wHSHandle:
    case wPhantom: break;                                   // don't worry about clipping or hits with these
    case ewFlame:  if (!clipped) dead=1; break;
    case wBeam:    dead=23; break;
    case wArrow:   dead=4; break;                           //findentrance(x,y,mfARROW,true); break;
    case ewArrow:  dead=clipped?4:1; break;
    case wWind:
      if(x>=240)
        dead=2;
      break;
    case wBrang:
      if(misc==0)
      {
        clk2=256;
        if(clipped)
        {
          dead=4;
        }
        else
        {
          misc=1;
          /*
                    if (current_item(itype_brang,true)>1) {
                      if (dummy_bool[0]) {
                        add_grenade(x,y,current_item(itype_brang,true)>2);
                        dummy_bool[0]=false;
                      }
                    }
          */
        }
      }
      break;
    case wHookshot:
      if(misc==0)
      {
        clk2=256;
        if(clipped)
          dead=4;
        else
          misc=1;
      }
      break;
    case ewBrang:
      if(misc==0)
      {
        clk2=256;
        dead=4;
      }
      break;
    case wMagic: wand_dead=true; wand_x=x; wand_y=y;        // set some global flags
    dead=1; break;                                          //remove the dead part to make the wand only die when clipped

    case ewWind:
      if (clipped)
      {
        if (misc==999)                                      // in enemy wind
        {
          ewind_restart=true;
        }
        dead=1;
      }

      break;
    default: dead=1;
  }
}

// override hit detection to check for invicibility, etc
bool weapon::hit(sprite *s)
{
  if(id==ewBrang && misc)
    return false;
  return (dead!=-1) ? false : sprite::hit(s);
}

bool weapon::hit(int tx,int ty,int txsz,int tysz)
{
  if(id==ewBrang && misc)
    return false;
  return (dead!=-1) ? false : sprite::hit(tx,ty,txsz,tysz);
}

void weapon::draw(BITMAP *dest)
{
  if(flash==1)
  {
    if(!BSZ)
    {
      cs = (id==wBeam) ? 6 : wpnsbuf[wid].csets&15;
      cs += frame&3;
    }
    else
    {
      if(id==wBeam)
        cs = ((frame>>2)&1)+7;
      else
      {
        cs = wpnsbuf[wid].csets&15;
        if(++csclk >= 12)
          csclk = 0;
        cs += csclk>>2;
      }
    }
  }
  if(flash>1)
  {
    if(++csclk >= (wpnsbuf[wid].speed<<1))
      csclk = 0;

    cs = wpnsbuf[wid].csets&15;
    if(csclk >= wpnsbuf[wid].speed)
      cs = wpnsbuf[wid].csets>>4;
  }

  if(wpnsbuf[wid].frames)
  {
    if(++clk2 >= wpnsbuf[wid].speed)
    {
      clk2 = 0;
      if(++aframe >= wpnsbuf[wid].frames)
        aframe = 0;
    }
    //shnarf
    //     tile = wpnsbuf[wid].tile + aframe + (((!angular)&&(dir&2))?wpnsbuf[wid].frames:0);
    tile = wpnsbuf[wid].tile + aframe;
  }

  // do special case stuff
  switch(id)
  {
    case wBeam:
    {
      if(dead==-1) break;
      // draw the beam thingies
      int ofs=23-dead;
      int f = frame&3;
      int type = wpnsbuf[wid].type;
      tile = wpnsbuf[wid].tile+2;
      if(type)
        cs = wpnsbuf[wid].csets>>4;
      if(type==3 && (f&2))
        ++tile;
      if(!type || f==0 || (type>1 && f==3)) overtile16(dest,tile,x-2-ofs,y+54-ofs,cs,0);
      if(!type || f==2 || (type>1 && f==1)) overtile16(dest,tile,x+2+ofs,y+54-ofs,cs,1);
      if(!type || f==1 || (type>1 && f==2)) overtile16(dest,tile,x-2-ofs,y+58+ofs,cs,2);
      if(!type || f==3 || (type>1 && f==0)) overtile16(dest,tile,x+2+ofs,y+58+ofs,cs,3);
    } return;                                               // don't draw sword

    case wBomb:
    case wSBomb:
      if(clk<48) break;
      // draw the explosion
      tile = wpnsbuf[wBOOM].tile;
      cs = wpnsbuf[wBOOM].csets&15;
/*
      if ((clk>=42 && clk<=45)||(clk>=50 && clk<=53))
      {
        for (int c=0; c<256; ++c)
        {
          if (clk==42||clk==50)
          {
            temppal[c]=RAMpal[c];
          }
          int tempcol=(RAMpal[c].r*5 + RAMpal[c].g*8 + RAMpal[c].b*3)>>4;
          RAMpal[c].r=RAMpal[c].g=RAMpal[c].b=tempcol;
        }
        refreshpal=true;
      }
      else if (clk==46 || clk==54)
      {
        for (int c=0; c<256; ++c)
        {
          RAMpal[c]=temppal[c];
        }
        refreshpal=true;
      }
*/
      if(clk>72)
      {
        ++tile;
      }
      overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-13,cs,0);
      overtile16(dest,tile,x,y+yofs,cs,0);
      overtile16(dest,tile,x+((clk&1)?-14:14),y+yofs,cs,0);
      overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+14,cs,0);
      if(id==wSBomb)
      {
        overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-27,cs,0);
        overtile16(dest,tile,x+((clk&1)?-21:21),y+yofs-13,cs,0);
        overtile16(dest,tile,x+((clk&1)?-28:28),y+yofs,cs,0);
        overtile16(dest,tile,x+((clk&1)?21:-21),y+yofs+14,cs,0);
        overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+28,cs,0);
      }
      if(debug && key[KEY_O])
        rectfill(dest,x+hxofs,y+hyofs+yofs,
          x+hxofs+hxsz-1,y+hyofs+hysz-1+yofs,vc(id));
      return;                                               // don't draw bomb
    case wArrow:
    case ewArrow:
      if(dead>0 && !bounce)
        { cs=7; tile=54; flip=0; }
        break;

    case ewFlame:
    case wFire:
      if(wpnsbuf[wid].frames==0)
        flip = ((clk & wpnsbuf[wid].misc) >> 2) & 3;
      break;

    case ewBrang:
    case wBrang:
      tile = wpnsbuf[wid].tile;
      cs = wpnsbuf[wid].csets&15;
      if(BSZ)
        flip = bszboomflip[(clk>>2)&3];
      else
      {
        tile = boomframe[clk&0xE] + wpnsbuf[wid].tile;
        flip = boomframe[(clk&0xE)+1];
      }
      if(dead>0)
      {
        cs=7; tile=54; flip=0;
      }
      break;

    case wHookshot:
      break;

    case wWind:
      if(wpnsbuf[wid].frames==0)
        flip ^= (wpnsbuf[wid].misc>>2)&3;
      if((dead!=-1) && !BSZ)
        tile = wpnsbuf[wFIRE].tile;
      break;

    case ewWind:
      /*
            if(wpnsbuf[wid].frames==0)
              flip ^= (wpnsbuf[wid].misc>>2)&3;
      */
      break;

    case wPhantom:
      switch (type)
      {
        case pNAYRUSLOVEROCKET1:
        case pNAYRUSLOVEROCKETRETURN1:
        case pNAYRUSLOVEROCKETTRAIL1:
        case pNAYRUSLOVEROCKETTRAILRETURN1:
        case pNAYRUSLOVEROCKET2:
        case pNAYRUSLOVEROCKETRETURN2:
        case pNAYRUSLOVEROCKETTRAIL2:
        case pNAYRUSLOVEROCKETTRAILRETURN2:
          if ((get_bit(quest_rules,qr_FLICKERINGNAYRUSLOVEROCKET))&&!(frame&1))
          {
            return;
          }
          break;
      }
      break;
  }
  // draw it
  sprite::draw(dest);
}

void putweapon(BITMAP *dest,int x,int y,int weapon_id, int type, int dir, int &aclk, int &aframe)
{
  weapon temp((fix)x,(fix)y,weapon_id,type,0,dir);
  temp.yofs=0;
  temp.clk2=aclk;
  temp.aframe=aframe;
  temp.animate(0);
  temp.draw(dest);
  aclk=temp.clk2;
  aframe=temp.aframe;
}

/*** end of sprite.cc ***/
