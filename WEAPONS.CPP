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

#include <string.h>
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
  else if(angle>(PI/2))   dir=l_down;
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

int weapon::seekEnemy2(int j)
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
      tempdistance=distance(dummy_fix[0],dummy_fix[1],GuyX(i),GuyY(i));
      if ((tempdistance<mindistance)&&(GuyID(i)>=10) && !GuySuperman(i))
      {
        mindistance=tempdistance;
        j=i;
      }
    }
  }
  if (j==-1)
  {
    return j;
  }
  al_trace("Guy: %d, gx: %f, gy: %f, x: %f, y: %f\n", j, float(GuyX(j)), float(GuyY(j)), float(dummy_fix[0]), float(dummy_fix[1]));
  angle = atan2(double(GuyY(j)-dummy_fix[1]),double(GuyX(j)-dummy_fix[0]));
  if(angle==-PI || angle==PI) dir=left;
  else if(angle==-PI/2) dir=up;
  else if(angle==PI/2)  dir=down;
  else if(angle==0)     dir=right;
  else if(angle<-PI/2)  dir=l_up;
  else if(angle<0)      dir=r_up;
  else if(angle>PI/2)   dir=l_down;
  else                  dir=r_down;
  return j;
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
  yofs=playing_field_offset - 2;
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
        case right: tile=o_tile+((frames>1)?frames:1); hxofs=2; hxsz=12; yofs = playing_field_offset+(BSZ ? 3 : 1); break;
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
        case right: tile=o_tile+((frames>1)?frames:1); yofs=playing_field_offset + 1; hyofs=2; hysz=14; hxofs=2; hxsz=12; break;
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
        case right: tile=o_tile+((frames>1)?frames:1); hxofs=2; hxsz=12; break;
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
      if(current_item(itype_hookshot,true)==2)
      {
        LOADGFX(wLSHEAD);
      }
      else
      {
        LOADGFX(wHSHEAD);
      }
      step = 4;
      //     step = 0;
      clk2=256;
      switch(dir)
      {
        case down:  flip=2; xofs+=4;  yofs+=1; hyofs=2; hysz=12; break;
        case up:    yofs+=3; xofs-=5; hyofs=2; hysz=12; break;
        case left:  flip=1; tile=o_tile+((frames>1)?frames:1); xofs+=2; yofs=playing_field_offset+4; hxofs=2; hxsz=12; break;
        case right: tile=o_tile+((frames>1)?frames:1); xofs-=2; yofs=playing_field_offset+4; hxofs=2; hxsz=12; break;
      }
      break;
    case wHSHandle:
      step = 0;
      if(current_item(itype_hookshot,true)==2)
      {
        LOADGFX(wLSHANDLE);
      }
      else
      {
        LOADGFX(wHSHANDLE);
      }
      switch(dir)
      {
        case down:  flip=2; xofs+=4;  yofs+=1; hyofs=2; hysz=12; break;
        case up:    yofs+=3; xofs-=5; hyofs=2; hysz=12; break;
        case left:  flip=1; tile=o_tile+((frames>1)?frames:1); xofs+=2; yofs=playing_field_offset+4; hxofs=2; hxsz=12; break;
        case right: tile=o_tile+((frames>1)?frames:1); xofs-=2; yofs=playing_field_offset+4; hxofs=2; hxsz=12; break;
      }
      break;
    case wHSChain:
      {
        int chain_gfx;
        if(current_item(itype_hookshot,true)==2)
        {
          if (dir<left)
          {
            chain_gfx=wLSCHAIN_V;
          }
          else
          {
            chain_gfx=wLSCHAIN_H;
          }
        }
        else
        {
          if (dir<left)
          {
            chain_gfx=wHSCHAIN_V;
          }
          else
          {
            chain_gfx=wHSCHAIN_H;
          }
        }
        step = 0;
        switch(dir)
        {
          case down:  LOADGFX(chain_gfx); xofs+=4;  yofs-=7;  break;
          case up:    LOADGFX(chain_gfx); xofs-=5;  yofs+=11; break;
          case left:  LOADGFX(chain_gfx); xofs+=10; yofs=playing_field_offset+4;  break;
          case right: LOADGFX(chain_gfx); xofs-=10; yofs=playing_field_offset+4;  break;
        }
      }
      break;
    case ewBomb:
      LOADGFX(ewBOMB);
      hxofs=0; hxsz=16;
      id = ewLitBomb;
      switch(dir)
      {
        case down:  flip=2;
        case up:    break;
        case left:  flip=1;
        case right: tile=o_tile+((frames>1)?frames:1); break;
      }
      break;
    case ewSBomb:
      LOADGFX(ewSBOMB);
      hxofs=0; hxsz=16;
      id = ewLitSBomb;
      switch(dir)
      {
        case down:  flip=2;
        case up:    break;
        case left:  flip=1;
        case right: tile=o_tile+((frames>1)?frames:1); break;
      }
      break;
    case ewBrang:
      hxofs=0; hxsz=15;
      wid = min(max(current_item(itype_brang,true),1),3)-1+wBRANG;
      LOADGFX(wid);
      break;
    case wRefFireball:
    case ewFireball:
      LOADGFX(ewFIREBALL);
      step=1.75;
      misc=dir-1;
      seekLink();
      break;
    case ewRock:
      LOADGFX(ewROCK);
      hxofs=0; hxsz=15;
      step=3;
      break;
    case ewArrow:
      LOADGFX(ewARROW);
      step=2;
      switch(dir)
      {
        case down:  flip=2;
        case up:    xofs=-4; hxsz=8; break;
        case left:  flip=1;
        case right: tile=o_tile+((frames>1)?frames:1); yofs=playing_field_offset+1; break;
      }
      break;
    case ewSword:
      LOADGFX(ewSWORD);
      hxofs=0; hxsz=15;
      step=3;
      switch(dir)
      {
        case down:  flip=2;
        case up:    break;
        case left:  flip=1;
        case right: tile=o_tile+((frames>1)?frames:1); yofs=playing_field_offset+1; break;
      }
      break;
    case wRefMagic:
    case ewMagic:
      LOADGFX(ewMAGIC);
      hxofs=0; hxsz=15;
      step=3;
      switch(dir)
      {
        case down:  flip=2;
        case up:    break;
        case left:  flip=1;
        case right: tile=o_tile+((frames>1)?frames:1); yofs=playing_field_offset+1; break;
      }
      if (id==wRefMagic)
      {
        ignorecombo=(((int)y&0xF0)+((int)x>>4));
      }
      break;
    case ewFlame:
	case ewFlame2:
		if(id==ewFlame)
			LOADGFX(ewFLAME);
		else
			LOADGFX(ewFLAME2);
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
    case ewFireTrail:
      LOADGFX(ewFIRETRAIL);
      step=0;
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
        default:
          tile=0;
          break;
      }
      break;
    default:
      tile=0;
      break;
  }
}

void weapon::LOADGFX(int wpn)
{
  wid = wpn;
  flash = wpnsbuf[wid].misc&3;
  tile  = wpnsbuf[wid].tile;
  cs = wpnsbuf[wid].csets&15;
  o_tile = wpnsbuf[wid].tile;
  o_cset = wpnsbuf[wid].csets;
  o_flip=(wpnsbuf[wid].misc>>2)&3;
  o_speed = wpnsbuf[wid].speed;
  o_type = wpnsbuf[wid].type;
  frames = wpnsbuf[wid].frames;
  temp1 = wpnsbuf[wFIRE].tile;
}

bool weapon::Dead() { return dead!=-1; }

bool weapon::clip()
{
  int c[4];
  int d2=isdungeon();
  int nb1 = get_bit(quest_rules,qr_NOBORDER) ? 16 : 0;
  int nb2 = get_bit(quest_rules,qr_NOBORDER) ? 8 : 0;

  if(id>wEnemyWeapons && id!=ewBrang)
  {
    c[0] = d2?32:(16-nb1);
    c[1] = d2?128:(144+nb1);
    c[2] = d2?32:(16-nb1);
    c[3] = d2?208:(224+nb1);
  }
  else if (id==wHookshot||id==wHSChain)
    {
      c[0] = d2?8:0;
      c[1] = d2?152:160;
      c[2] = d2?8:0;
      c[3] = d2?248:256;
    }
    else
    {
      c[0] = d2?18:2;
      c[1] = d2?144:160;
      c[2] = d2?20:4;
      c[3] = d2?220:236;
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
    c[0] = d2?32:(16-nb1);
    c[1] = d2?128:(144+nb1);
    c[2] = d2?32:(16-nb1);
    c[3] = d2?208:(224+nb1);
  }

  if(id==ewFireTrail)
  {
    c[0] = d2?32:(16-nb1);
    c[1] = d2?128:(144+nb1);
    c[2] = d2?32:(16-nb1);
    c[3] = d2?208:(224+nb1);
  }

  if(id==ewWind)
  {
    c[0] = d2?32:(16-nb1);
    c[1] = d2?128:(144+nb1);
    c[2] = d2?32:(16-nb1);
    c[3] = d2?208:(224+nb1);
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

  if(id>wEnemyWeapons)
  {
    if((x<(8-nb2) && dir==left)
       || (y<(8-nb2) && dir==up)
       || (x>(232+nb2) && dir==right)
       || (y>(168+nb2) && dir==down))
      return true;
  }

  if(x<0||y<0||x>240||y>176)
    return true;

  return false;
}

bool weapon::animate(int index)
{
  //these are here to bypass compiler warnings about unused arguments
  index=index;

  // do special timing stuff
  bool hooked=false;
  //static PALETTE temppal;
  //  fix tempx, tempy;
  switch(id)
  {
    // Link's weapons
    case wBeam:
      for (int i=0; i<current_item(itype_sword,true); i++)
      {
        if (findentrance(x,y,mfSWORDBEAM+i,true)) dead=23;
      }
      if (hitcombo(x,y,cBLOCKALL))
      {
        dead=23;
      }
      if (hitcombo(x,y,cBLOCKSBEAM))
      {
        dead=23;
      }
      break;

    case wWhistle:
      if(clk)
      {
        dead=1;
      }
      break;

    case wWind:
    {
      int wrx;
      if(get_bit(quest_rules,qr_NOARRIVALPOINT))
        wrx=tmpscr->warpreturnx[0];
      else wrx=tmpscr->warparrivalx;
      if(type==1 && dead==-1 && x>=(int)wrx) dead=2;
      break;
    }

    case wFire:
		{
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
					if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
					{
						lighting(2,dir);
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
					if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
					{
						lighting(2,dir);
					}
				}
			}
			break;
		}
    case wLitBomb:
    case wBomb:
    case ewLitBomb:
    case ewBomb:
		{
			if(clk==48&&step==0)
			{
				id = (id<wEnemyWeapons?wBomb:ewBomb);
				hxofs=1000;
			}
			if(clk==49&&(id==wBomb||id==ewBomb))
			{
				sfx(WAV_BOMB,pan(int(x)));
				hxofs=hyofs=-8;
				hxsz=hysz=32;
			}
			if(clk==81)
			{
				hxofs=1000;
			}
			if(id<wEnemyWeapons)
			{
				if(clk==49)
				{
					findentrance(x,y,mfBOMB,true);
					findentrance(x,y,mfSTRIKE,true);
				}
				if (!get_bit(quest_rules,qr_NOBOMBPALFLASH))
				{
					usebombpal=true;
					if(clk==50 || clk==55)
					{
						usebombpal=true;
						memcpy(tempbombpal, RAMpal, PAL_SIZE*sizeof(RGB));
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
						usebombpal=false;
						memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
						refreshpal = true;
					}
				}
				if(clk==80)
				{
					bombdoor(x,y);
				}
			}
			if(clk==84)
			{
				if (step==0)
				{
					dead=1;
				}
			}
			break;
		}
    case wArrow:
		{
			if (dead>0)
			{
				break;
			}
			if (findentrance(x,y,mfSTRIKE,true))
			{
				dead=4;
			}
			if (findentrance(x,y,mfARROW,true))
			{
				dead=4;
			}
			if (current_item(itype_arrow,true)>1)
			{
				if (findentrance(x,y,mfSARROW,true))
				{
					dead=4;
				}
			}
			if (current_item(itype_arrow,true)==3)
			{
				if (findentrance(x,y,mfGARROW,true))
				{
					dead=4;
				}
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
			if (hitcombo(x,y,cBLOCKALL))
			{
				dead=4;
			}
			if (hitcombo(x,y,cBLOCKARROW3))
			{
				dead=4;
			}
			if(current_item(itype_arrow,true)<3)
			{
				if (hitcombo(x,y,cBLOCKARROW2))
				{
					dead=4;
				}
			}
			if(current_item(itype_arrow,true)==1)
			{
				if (hitcombo(x,y,cBLOCKARROW1))
				{
					dead=4;
				}
			}
			break;
		}
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

    case ewLitSBomb:
    case ewSBomb:
    case wLitSBomb:
    case wSBomb:
		{
			if(clk==48&&step==0)
			{
				id = (id<wEnemyWeapons?wSBomb:ewSBomb);
				hxofs=1000;
			}
			if(clk==49&&(id==wSBomb||id==ewSBomb))
			{
				sfx(WAV_BOMB,pan(int(x)));
				hxofs=hyofs=-16;
				hxsz=hysz=48;
			}
			if (id<wEnemyWeapons)
			{
				if(clk==49)
				{
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
				}
				if (!get_bit(quest_rules,qr_NOBOMBPALFLASH))
				{
					if(clk==50 || clk==55)
					{
						memcpy(tempbombpal, RAMpal, PAL_SIZE*sizeof(RGB));
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
						memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
						refreshpal = true;
					}
				}
				if(clk==80)
				{
					bombdoor(x,y);
				}
			}
			if(clk==84)
			{
				if (step==0)
				{
					dead=1;
				}
			}
			break;
		}
    case wBait:
      if(clk==16*2*24)
      {
        dead=1;
      }
      break;

    case wBrang:
		{
			for (int i=0; i<current_item(itype_brang,true); i++)
      {
        if (findentrance(x,y,mfBRANG+i,true)) dead=1;
      }
			if (findentrance(x,y,mfSTRIKE,true)) dead=1;
			if (hitcombo(x,y,cBLOCKALL))
			{
				dead=1;
			}
			if (hitcombo(x,y,cBLOCKBRANG3))
			{
				dead=1;
			}
			if(current_item(itype_brang,true)<3)
			{
				if (hitcombo(x,y,cBLOCKBRANG2))
				{
					dead=1;
				}
			}
			if(current_item(itype_brang,true)==1)
			{
				if (hitcombo(x,y,cBLOCKBRANG1))
				{
					dead=1;
				}
			}
			++clk2;
			if(clk2==36)
			{
				misc=1;
			}
			if(clk2>18 && clk2<52)
			{
				step=1;
			}
			else if(misc)
			{
				step=2;
			}
			else
			{
				step=3;
			}
  
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
				{
					dir=left;
				}
				else if(Right())
				{
					dir=right;
				}
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
		}
    case wHookshot:
		{
			if (misc==0)
			{
				int maxlength=16*((current_item(itype_hookshot, true)==2)?zinit.longshot_length:zinit.hookshot_length);
				if((abs(LinkX()-x)>maxlength)||(abs(LinkY()-y)>maxlength))
				{
					dead=1;
				}
				if (findentrance(x,y,mfSTRIKE,true)) dead=1;
				if (findentrance(x,y,mfHOOKSHOT,true)) dead=1;

				if (dir==up)
				{
					if ((combobuf[MAPCOMBO(x+2,y+7)].type==cHSGRAB))
					{
						hooked=true;
					}
					if (!hooked && _walkflag(x+2,y+7,1) && !isstepable(MAPCOMBO(int(x+2),int(y+7))) && combobuf[MAPCOMBO(x+2,y+7)].type!=cHOOKSHOTONLY)
					{
						dead=1;
					}
				}

				if (dir==down)
				{
					if ((combobuf[MAPCOMBO(x+12,y+12)].type==cHSGRAB))
					{
						hooked=true;
					}
					if (!hooked && _walkflag(x+12,y+12,1) && !isstepable(MAPCOMBO(int(x+12),int(y+12))) && combobuf[MAPCOMBO(x+12,y+12)].type!=cHOOKSHOTONLY)
					{
						dead=1;
					}
				}

				if (dir==left)
				{
					if ((combobuf[MAPCOMBO(x+6,y+13)].type==cHSGRAB))
					{
						hooked=true;
					}
					if (!hooked && _walkflag(x+6,y+13,1) && !isstepable(MAPCOMBO(int(x+6),int(y+13))) && combobuf[MAPCOMBO(x+6,y+13)].type!=cHOOKSHOTONLY)
					{
						dead=1;
					}
				}

				if (dir==right)
				{
					if ((combobuf[MAPCOMBO(x+9,y+13)].type==cHSGRAB))
					{
						hooked=true;
					}
					if (!hooked && _walkflag(x+9,y+13,1) && !isstepable(MAPCOMBO(int(x+9),int(y+13))) && combobuf[MAPCOMBO(x+9,y+13)].type!=cHOOKSHOTONLY)
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
}
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
          tile=o_tile;
          if (dir&2)
          {
            if (frames>1)
            {
              tile+=frames;
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
          tile=o_tile;
          if (dir&2)
          {
            if (frames>1)
            {
              tile+=frames;
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
        if (hitcombo(x,y,cBLOCKALL))
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
      if (hitcombo(x,y,cBLOCKFIREBALL))
      {
        dead=0;
      }
      if (hitcombo(x,y,cBLOCKALL))
      {
        dead=0;
      }
      //     if ((id==wRefFireball)&&(dummy_bool[0])) { //homing
      //       seekEnemy(-1);
      //     } else {
      /*if ((id==ewFireball)&&(!(clk%8)&&0))                  //homing (remove &&0 to activate)
      {
        seekLink();
      }
      else*/
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
      if(clk==32)
      {
        step=0;
        lighting(1,dir);
      }
      if(clk==126)
      {
        dead=1;
        if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
        {
          lighting(2,dir);
        }
      }
      break;

    case ewFireTrail:
      if(clk==32)
      {
        step=0;  //should already be 0, but still...
        lighting(1,dir);
      }
      if(clk==640)
      {
        dead=1;
        if(get_bit(quest_rules,qr_TEMPCANDLELIGHT))
        {
          lighting(2,dir);
        }
      }
      break;

    case ewBrang:
      if(clk==0)
      {
        misc2=(dir<left)?y:x;                               // save home position
        if (dummy_bool[0]==true)
        {
          seekLink();
        }
      }
  
      ++clk2;
      if(clk2==45&&!dummy_bool[0])
      {
        misc=1;
        dir^=1;
      }
      if (dummy_bool[0])
      {
        step=5;
      }
      else
      {
        if(clk2>27 && clk2<61)
        {
          step=1;
        }
        else if(misc)
          {
            step=2;
          }
          else
          {
            step=3;
          }
      }
  
      if(dead==1)
      {
        dead=-1;
        misc=1;
        dir^=1;
      }
  
      if(misc==1)                                           // returning
      {
        if (dummy_bool[0]==true)
        {
          seekEnemy(type-100);
          if ((abs(x-GuyX(type-100))<7)&&(abs(y-GuyY(type-100))<7))
          {
            return true;
          }
        }
        else
        {
          switch(dir)
          {
            case up:    if(y<misc2) return true; break;
            case down:  if(y>misc2) return true; break;
            case left:  if(x<misc2) return true; break;
            case right: if(x>misc2) return true; break;
          }
        }
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
      {
        onhit(true);
      }
    }
  }

  if(bounce)
  {
    switch(dir)
    {
      case up:    x-=1; y+=2; break;
      case down:  x+=1; y-=2; break;
      case left:  x+=2; y-=1; break;
      case right: x-=2; y-=1; break;
    }
  }

  // update clocks
  ++clk;
  if(dead>0)
  {
    --dead;
  }
  return dead==0;
}

void weapon::onhit(bool clipped)
{
  onhit(clipped, 0, -1);
}

void weapon::onhit(bool clipped, int special, int linkdir)
{
  if(special==2)                                            // hit Link's mirror shield
  {
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
  }

  if(special==1)                                            // hit Link's shield
  {
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
  }

  switch(id)
  {
    case ewBomb:     step=0; break;
    case ewSBomb:    step=0; break;
    case ewLitBomb:  step=0; id=ewBomb;  clk=47; hxofs=hyofs=-7;  hxsz=hysz=30; break;
    case ewLitSBomb: step=0; id=ewSBomb; clk=47; hxofs=hyofs=-16; hxsz=hysz=48; break;
    case wLitBomb:  if(!clipped) dead=1;
    case wLitSBomb: if(!clipped) dead=1;
    case wWhistle:
    case wBomb:
    case wSBomb:
    case wBait:
    case wFire:
    case wHSHandle:
    case wPhantom: break;                                   // don't worry about clipping or hits with these
    case ewFireTrail:  if (!clipped) dead=1; break;
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
    case wMagic:
      wand_dead=true;
      wand_x=x; wand_y=y;        // set some global flags
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
      cs = (id==wBeam) ? 6 : o_cset&15;
      cs += frame&3;
    }
    else
    {
      if(id==wBeam)
        cs = ((frame>>2)&1)+7;
      else
      {
        cs = o_cset&15;
        if(++csclk >= 12)
          csclk = 0;
        cs += csclk>>2;
      }
    }
  }
  if(flash>1)
  {
    if(++csclk >= (o_speed<<1))
      csclk = 0;

    cs = o_cset&15;
    if(csclk >= o_speed)
      cs = o_cset>>4;
  }

  if(frames&&wid!=wBRANG)
  {
    if(++clk2 >= o_speed)
    {
      clk2 = 0;
      if(frames>1 && ++aframe >= frames)
      {
        aframe = 0;
      }
    }
    tile = o_tile+aframe;
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
      int type2 = o_type;
      tile = o_tile+2;
      if(type2)
        cs = o_cset>>4;
      if(type2==3 && (f&2))
        ++tile;
      if(!type2 || f==0 || (type2>1 && f==3)) overtile16(dest,tile,x-2-ofs,y+playing_field_offset-2-ofs,cs,0);
      if(!type2 || f==2 || (type2>1 && f==1)) overtile16(dest,tile,x+2+ofs,y+playing_field_offset-2-ofs,cs,1);
      if(!type2 || f==1 || (type2>1 && f==2)) overtile16(dest,tile,x-2-ofs,y+playing_field_offset+2+ofs,cs,2);
      if(!type2 || f==3 || (type2>1 && f==0)) overtile16(dest,tile,x+2+ofs,y+playing_field_offset+2+ofs,cs,3);
    } return;                                               // don't draw sword

    case wBomb:
    case wSBomb:
    case ewBomb:
    case ewSBomb:
      {
        if(clk<48) break;
        // draw the explosion
        int id2=0;
        switch (id)
        {
          case wBomb:
            id2=wBOOM;
            break;
          case wSBomb:
            id2=wSBOOM;
            break;
          case ewBomb:
            id2=ewBOOM;
            break;
          case ewSBomb:
            id2=ewSBOOM;
            break;
        }
        tile = wpnsbuf[id2].tile;
        cs = wpnsbuf[id2].csets&15;
      }
      if(clk>72)
      {
        ++tile;
      }
      overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-13,cs,0);
      overtile16(dest,tile,x,y+yofs,cs,0);
      overtile16(dest,tile,x+((clk&1)?-14:14),y+yofs,cs,0);
      overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+14,cs,0);
      if(id==wSBomb||id==ewSBomb)
      {
        overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-27,cs,0);
        overtile16(dest,tile,x+((clk&1)?-21:21),y+yofs-13,cs,0);
        overtile16(dest,tile,x+((clk&1)?-28:28),y+yofs,cs,0);
        overtile16(dest,tile,x+((clk&1)?21:-21),y+yofs+14,cs,0);
        overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+28,cs,0);
      }
      if(get_debug() && key[KEY_O])
        rectfill(dest,x+hxofs,y+hyofs+yofs,
                 x+hxofs+hxsz-1,y+hyofs+hysz-1+yofs,vc(id));
      return;                                               // don't draw bomb

    case wArrow:
    case ewArrow:
      if(dead>0 && !bounce)
      {
        cs=7;
        tile=54;
        flip=0;
      }
      break;

    case ewFireTrail:
    case ewFlame:
    case wFire:
    if(frames==0)
      flip = o_flip;
    break;

    case ewBrang:
    case wBrang:
    tile = o_tile;
    cs = o_cset&15;
    if(BSZ)
      flip = bszboomflip[(clk>>2)&3];
    else
    {
      tile = boomframe[clk&0xE] + o_tile;
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
    if(frames==0)
      flip ^= o_flip;
    if((dead!=-1) && !BSZ)
      tile = temp1;//wpnsbuf[wFIRE].tile;
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
