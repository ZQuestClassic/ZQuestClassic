//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  link.cc
//
//  Link's class: LinkClass
//  Handles a lot of game play stuff as well as Link's
//  movement, attacking, etc.
//
//--------------------------------------------------------

#include <string.h>
#include "link.h"
#include "guys.h"
#include "subscr.h"
#include "zc_subscr.h"
#include "decorations.h"
#include "gamedata.h"
#include "zc_custom.h"
#include "title.h"
#include <stdio.h>

extern int draw_screen_clip_rect_x1;
extern int draw_screen_clip_rect_x2;
extern int draw_screen_clip_rect_y1;
extern int draw_screen_clip_rect_y2;
extern bool draw_screen_clip_rect_show_link;

extern short ffposx[32];
extern short ffposy[32];
extern long ffprvx[32];
extern long ffprvy[32];

int link_count = -1;
int z3step = 2;
byte lshift = 0;

void playLevelMusic();


const byte lsteps[8] = {1,1,2,1,1,2,1,1};


int LinkClass::DrunkClock() {return drunkclk;}
LinkClass::LinkClass() : sprite() { init(); }
void LinkClass::linkstep() { lstep = lstep<(BSZ?27:11) ? lstep+1 : 0; }

// called by ALLOFF()
void LinkClass::resetflags(bool all)
{
  refilling=inlikelike=inwallm=false;
  blowcnt=whirlwind=hclk=fairyclk=didstuff=0;
  if(swordclk>0 || all)
    swordclk=0;
  if(itemclk>0 || all)
    itemclk=0;
  if(all)
  {
    NayrusLoveShieldClk=0;
  }
  hopclk=0;
  attackclk=0;
  diveclk=0;
  action=none;
  conveyor_flags=0;
}

void LinkClass::Freeze() { action=freeze; }
void LinkClass::unfreeze() { if(action==freeze) action=none; }
void LinkClass::beatlikelike() { inlikelike=false; }
fix  LinkClass::getX()   { return x; }
fix  LinkClass::getY()   { return y; }
fix  LinkClass::getXOfs() { return xofs; }
fix  LinkClass::getYOfs() { return yofs; }
void LinkClass::setXOfs(int newxofs) { xofs=newxofs; }
void LinkClass::setYOfs(int newyofs) { yofs=newyofs; }
int  LinkClass::getHXOfs()   { return hxofs; }
int  LinkClass::getHYOfs()   { return hyofs; }
int  LinkClass::getHXSz()   { return hxsz; }
int  LinkClass::getHYSz()   { return hysz; }
fix  LinkClass::getClimbCoverX()   { return climb_cover_x; }
fix  LinkClass::getClimbCoverY()   { return climb_cover_y; }
void LinkClass::setX(int new_x)   { x=new_x; }
void LinkClass::setY(int new_y)   { y=new_y; }
void LinkClass::setClimbCoverX(int new_x)   { climb_cover_x=new_x; }
void LinkClass::setClimbCoverY(int new_y)   { climb_cover_y=new_y; }
int  LinkClass::getLStep() { return lstep; }
fix  LinkClass::getModifiedX()
{
  fix tempx=x;
  if (screenscrolling&&(dir==left))
  {
    tempx=tempx+256;
  }
  return tempx;
}

fix  LinkClass::getModifiedY()
{
  fix tempy=y;
  if (screenscrolling&&(dir==up))
  {
    tempy=tempy+176;
  }
  return tempy;
}

int  LinkClass::getDir() { return dir; }
int  LinkClass::getClk() { return clk; }
int  LinkClass::getPushing() { return pushing; }
void LinkClass::Catch()
{
  if(!inwallm && (action==none || action==walking))
  {
    action=attacking;
    attackclk=0;
    attack=wCatching;
  }
}

bool LinkClass::getClock() { return superman; }
void LinkClass::setClock(bool state) { superman=state; }
int  LinkClass::getAction() { return action; }
bool LinkClass::isDiving() { return (diveclk>30); }
bool LinkClass::isSwimming()
{
  return ((action==swimming)||(action==swimhit)||
          (action==waterhold1)||(action==waterhold2)||
          (hopclk==0xFF));
}

void LinkClass::setDontDraw(bool new_dontdraw)
{
  dontdraw=new_dontdraw;
}

void LinkClass::setHClk(int newhclk)
{
  hclk=newhclk;
}

int LinkClass::getHClk()
{
  return hclk;
}

void LinkClass::init()
{
  hookshot_used=false;
  hookshot_frozen=false;
  dir = up;
  shiftdir = -1;
  holddir = -1;
  landswim = 0;
  sdir = up;
  ilswim=true;
  walkable=false;
  if(get_bit(quest_rules,qr_NOARRIVALPOINT))
  {
    x=tmpscr->warpreturnx[0];
    y=tmpscr->warpreturny[0];
  }
  else
  {
    x=tmpscr->warparrivalx;
    y=tmpscr->warparrivaly;
  }
  if(x==0)   dir=right;
  if(x==240) dir=left;
  if(y==0)   dir=down;
  if(y==160) dir=up;
  lstep=0;
  skipstep=0;
  autostep=false;
  attackclk=holdclk=0;
  attack=wNone;
  action=none;
  xofs=0;
  yofs=playing_field_offset;
  cs=6;
  pushing=fairyclk=0;
  id=0;
  inlikelike=superman=inwallm=false;
  blowcnt=whirlwind=0;
  hopclk=diveclk=0;
  conveyor_flags=0;
  drunkclk=0;
  drawstyle=3;
  ffwarp = false;
}

void LinkClass::draw_under(BITMAP* dest)
{
  if(action==rafting)
  {
    if (((dir==left) || (dir==right)) && (get_bit(quest_rules,qr_RLFIX)))
    {

      overtile16(dest, itemsbuf[iRaft].tile, x, y+60,
                 itemsbuf[iRaft].csets&15, rotate_value((itemsbuf[iRaft].misc>>2)&3)^3);
    }
    else
    {
      overtile16(dest, itemsbuf[iRaft].tile, x, y+60,
                 itemsbuf[iRaft].csets&15, (itemsbuf[iRaft].misc>>2)&3);
    }
  }

  if(ladderx+laddery)
  {

    if ((ladderdir>=left) && (get_bit(quest_rules,qr_RLFIX)))
    {
      overtile16(dest, itemsbuf[iLadder].tile, ladderx, laddery+playing_field_offset,
                 itemsbuf[iLadder].csets&15, rotate_value((itemsbuf[iRaft].misc>>2)&3)^3);
    }
    else
    {
      overtile16(dest, itemsbuf[iLadder].tile, ladderx, laddery+playing_field_offset,
                 itemsbuf[iLadder].csets&15, (itemsbuf[iLadder].misc>>2)&3);
    }
  }
}

void LinkClass::draw(BITMAP* dest)
{
  int oxofs, oyofs;
  bool shieldModify = false;
  if (tmpscr->flags3&fINVISLINK)
  {
    return;
  }

  if(action==dying)
  {
    if(!dontdraw)
      sprite::draw(dest);
    return;
  }
  else
  {
    if (dontdraw)
    {
      return;
    }
  }

  bool useltm=(get_bit(quest_rules,qr_EXPANDEDLTM) != 0);

  yofs = playing_field_offset-((!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 2 : 0);
  detail_int[0]=yofs;
  oxofs=xofs;
  oyofs=yofs;
  if (action!=dying)
  {
    cs = 6;
    if (!get_bit(quest_rules,qr_LINKFLICKER))
    {
      if(superman)
      {
        cs += (((~frame)>>1)&3);
      }
      else if (hclk&&(NayrusLoveShieldClk<=0))
        {
          cs += ((hclk>>1)&3);
        }
    }
  }
  if(attackclk || action==attacking)
  {
    if(attackclk>4||(attack==wSword&&get_gamedata_canslash(game)))
    {
      if(attack==wSword || attack==wWand)
      {
        int wy=1;
        int wx=1;
        int f=0,t,cs;

        if(attack==wWand)
        {
          t = wpnsbuf[wWAND].tile;
          cs = wpnsbuf[wWAND].csets&15;
        }
        else
        {
          t = wpnsbuf[current_item(itype_sword,true)-1].tile;
          cs = wpnsbuf[current_item(itype_sword,true)-1].csets&15;
        }
        //end of slash/stab
        switch(dir)
        {
          case up:
          wx=-1; wy=-12;
          if (get_gamedata_canslash(game) && attack==wSword)
          {
            if(attackclk>10) //extended stab
            {
              yofs-=3;
              wy-=3;
            }
            if(attackclk==14) //retracting stab
            {
              yofs+=3;
              wy+=4;
            }
          }
          if (!get_gamedata_canslash(game) || attack!=wSword)
          {
            if(attackclk==13) wy+=4;
            if(attackclk==14) wy+=8;
          }
          break;
          case down:
          f=get_bit(quest_rules,qr_SWORDWANDFLIPFIX)?3:2; wy=11;
          if (get_gamedata_canslash(game) && attack==wSword)
          {
            if(attackclk>10) //extended stab
            {
              yofs+=3;
              wy+=4;
            }
            if(attackclk==14) //retracting stab
            {
              yofs-=3;
              wy-=2;
            }
          }
          if (!get_gamedata_canslash(game) || attack!=wSword)
          {
            if(attackclk==13) wy-=4;
            if(attackclk==14) wy-=8;
          }
          break;
          case left:
          f=1; wx=-11; ++t;
          if (get_gamedata_canslash(game) && attack==wSword)
          {
            if(attackclk>10)  //extended stab
            {
              xofs-=4;
              wx-=7;
            }
            if(attackclk==14) //retracting stab
            {
              xofs+=3;
              wx+=7;
            }
          }
          if (!get_gamedata_canslash(game) || attack!=wSword)
          {
            if(attackclk==13) wx+=4;
            if(attackclk==14) wx+=8;
          }
          break;
          case right:
          wx=11; ++t;
          if (get_gamedata_canslash(game) && attack==wSword)
          {
            if(attackclk>10) //extended stab
            {
              xofs+=4;
              wx+=7;
            }
            if(attackclk==14) //retracting stab
            {
              xofs-=3;
              wx-=7;
            }
          }
          if (!get_gamedata_canslash(game) || attack!=wSword)
          {
            if(attackclk==13) wx-=4;
            if(attackclk==14) wx-=8;
          }
          break;
        }

        if (get_gamedata_canslash(game) && attack==wSword && attackclk<11)
        {
          //slashing tiles
          switch(dir)
          {
            case up:
            wx=15; wy=-3;
            ++t; f=0;                                     //starts pointing right
            if(attackclk>=7)
            {
              wy-=9;
              wx-=3;
              t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
              cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
              f=0;
            }
            check_slash_block(x+wx,y+wy+8);
            break;
            case down:
            wx=-13; wy=-1;
            ++t; f=1;                                     //starts pointing left
            if(attackclk>=7)
            {
              wy+=15; wx+=2;
              t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
              cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
              ++t;
              f=0;
            }
            check_slash_block(x+wx,y+wy+8);
            break;
            case left:
            wx=3; wy=-15;
            --t; f=0;                                     //starts pointing up
            if(attackclk>=7)
            {
              wx-=15;
              wy+=3;
              xofs-=1;
              t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
              cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
              t+=2;
              f=0;
            }
            check_slash_block(x+wx,y+wy);
            break;
            case right:
            wx=-3; wy=-15;
            --t; f=1;                                     //starts pointing up
            if(attackclk>=7)
            {
              wx+=15;
              wy+=3;
              xofs+=1;
              t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
              cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
              t+=3;
              f=0;
            }
            check_slash_block(x+wx+10,y+wy);
            break;
          }
        }

        if(BSZ || ((isdungeon() && currscr<128) && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)))
        {
          wy+=2;
        }
        if (!(get_bit(quest_rules,qr_LINKFLICKER)&&((superman||hclk)&&(frame&1))))
        {
          if(isdungeon() && currscr<128 && !get_bit(quest_rules,qr_FREEFORM))
          {
            BITMAP *sub = create_sub_bitmap(dest,16,playing_field_offset+16,224,144);
            overtile16(sub,t,x+wx-16,y+wy-(72-yofs),cs,f);
            destroy_bitmap(sub);
          }
          else
          {
            overtile16(dest,t,x+wx,y+yofs+wy,cs,f);
          }
        }
      }
    }

    if(attackclk<7 || (attack==wSword && attackclk<(get_gamedata_canslash(game)?15:13) || attack==wWand && attackclk<13) || ((attack==wHammer) && attackclk<=30))
    {
      linktile(&tile, &flip, &extend, ls_stab, dir, zinit.linkanimationstyle);

      if(get_gamedata_canslash(game) && (attack==wSword) && (attackclk<7))
      {
        linktile(&tile, &flip, &extend, ls_slash, dir, zinit.linkanimationstyle);
      }
      if((attack==wHammer) && (attackclk<13))
      {
        linktile(&tile, &flip, &extend, ls_pound, dir, zinit.linkanimationstyle);
      }

      if (useltm)
      {
        tile+=item_tile_mod(shieldModify);
      }

      tile+=dmap_tile_mod();
      if (!(get_bit(quest_rules,qr_LINKFLICKER)&&((superman||hclk)&&(frame&1))))
      {
        masked_draw(dest);
      }
      if (attack!=wHammer)
      {
        xofs=oxofs;
        yofs=oyofs;
        return;
      }
    }

    if(attack==wHammer)
    {
      int wy=1;
      int wx=1;
      int f=0,t,cs;
      t = wpnsbuf[wHAMMER].tile;
      cs = wpnsbuf[wHAMMER].csets&15;

      switch(dir)
      {
        case up:
        wx=-1; wy=-15;
        if(attackclk>=13)
        {
          wx-=1; wy+=1; ++t;
        }
        if(attackclk>=15)
        {
          ++t;
        }
        break;
        case down:
        wx=3;   wy=-14;  t+=3;
        if(attackclk>=13)
        {
          wy+=16;
          ++t;
        }
        if(attackclk>=15)
        {
          wx-=1; wy+=12;
          ++t;
        }
        break;
        case left:
        wx=0;   wy=-14;  t+=6; f=1;
        if(attackclk>=13)
        {
          wx-=7; wy+=8;
          ++t;
        }
        if(attackclk>=15)
        {
          wx-=8; wy+=8;
          ++t;
        }
        break;
        case right:
        wx=0;  wy=-14;  t+=6;
        if(attackclk>=13)
        {
          wx+=7; wy+=8;
          ++t;
        }
        if(attackclk>=15)
        {
          wx+=8; wy+=8;
          ++t;
        }
        break;
      }

      if(BSZ || ((isdungeon() && currscr<128) &&
                 !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)))
        wy+=2;
      if (!(get_bit(quest_rules,qr_LINKFLICKER)&&((superman||hclk)&&(frame&1))))
      {
        if(isdungeon() && currscr<128)
        {
          BITMAP *sub = create_sub_bitmap(dest,16,playing_field_offset+16,224,144);
          overtile16(sub,t,x+wx-16,y+wy-(72-yofs),cs,f);
          destroy_bitmap(sub);
        }
        else
        {
          overtile16(dest,t,x+wx,y+yofs+wy,cs,f);
        }
      }

      if (attackclk==15)
      {
        sfx(WAV_HAMMER,pan(int(x)));
      }
      xofs=oxofs;
      yofs=oyofs;
      return;
    }
  }

  if (action!=casting)
  {
    switch (zinit.linkanimationstyle)
    {
      case las_original:                                               //normal
      if(action==swimming || action==swimhit || hopclk==0xFF)
      {
        linktile(&tile, &flip, &extend, ls_float, dir, zinit.linkanimationstyle);
        if (lstep>=6)
        {
          if (dir==up)
          {
            ++flip;
          }
          else
          {
            extend==2?tile+=2:++tile;
          }
        }
        if (diveclk>30)
        {
          linktile(&tile, &flip, &extend, ls_dive, dir, zinit.linkanimationstyle);
          tile+=((frame>>3) & 1)*(extend==2?2:1);
        }
      }
      else
      {
        linktile(&tile, &flip, &extend, ls_walk, dir, zinit.linkanimationstyle);
        if (dir>up)
        {
          useltm=true;
          shieldModify=true;
        }
        /*
          else if (dir==up)
          {
          useltm=true;
          }
          */
        if (lstep>=6)
        {
          if (dir==up)
          {
            ++flip;
          }
          else
          {
            extend==2?tile+=2:++tile;
          }
        }
      }
      break;
      case las_bszelda:                                               //BS
      if(action==swimming || action==swimhit || hopclk==0xFF)
      {
        linktile(&tile, &flip, &extend, ls_float, dir, zinit.linkanimationstyle);
        tile += anim_3_4(lstep,7)*(extend==2?2:1);
        if(diveclk>30)
        {
          linktile(&tile, &flip, &extend, ls_dive, dir, zinit.linkanimationstyle);
          tile += anim_3_4(lstep,7)*(extend==2?2:1);
        }
      }
      else
      {
        linktile(&tile, &flip, &extend, ls_walk, dir, zinit.linkanimationstyle);
        if (dir>up)
        {
          useltm=true;
          shieldModify=true;
        }
        /*
          else if (dir==up)
          {
          useltm=true;
          }
          */
        tile += anim_3_4(lstep,7)*(extend==2?2:1);
      }
      break;
      case las_zelda3:                                               //8-frame Zelda 3
      if(action==swimming || action==swimhit || hopclk==0xFF)
      {
        linktile(&tile, &flip, &extend, ls_float, dir, zinit.linkanimationstyle);
        tile += anim_3_4(lstep,7)*(extend==2?2:1);
        if(diveclk>30)
        {
          linktile(&tile, &flip, &extend, ls_dive, dir, zinit.linkanimationstyle);
          tile += anim_3_4(lstep,7)*(extend==2?2:1);
        }
      }
      else
      {
        linktile(&tile, &flip, &extend, ls_walk, dir, zinit.linkanimationstyle);
        if (action==walking)
        {
          tile+=(extend==2?2:1);
        }
        if (dir>up)
        {
          useltm=true;
          shieldModify=true;
        }
        if (action==walking||action==hopping)
        {
          //tile+=(extend==2?2:1);
          //tile+=(((active_count>>2)%8)*(extend==2?2:1));
          int l=link_count;
          l-=((l>3)?1:0)+((l>12)?1:0);
          tile+=(l/2)*(extend==2?2:1);
        }
      }
      break;
      default:
      break;
    }
  }

  yofs = playing_field_offset-((!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 2 : 0);
  if(action==won)
  {
    yofs=playing_field_offset - 2;
  }
  if(action==landhold1 || action==landhold2)
  {
    useltm=(get_bit(quest_rules,qr_EXPANDEDLTM) != 0);
    yofs = playing_field_offset-((!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 2 : 0);
    linktile(&tile, &flip, &extend, (action==landhold1)?ls_landhold1:ls_landhold2, dir, zinit.linkanimationstyle);
  }
  else if(action==waterhold1 || action==waterhold2)
  {
    useltm=(get_bit(quest_rules,qr_EXPANDEDLTM) != 0);
    linktile(&tile, &flip, &extend, (action==waterhold1)?ls_waterhold1:ls_waterhold2, dir, zinit.linkanimationstyle);
  }
  if(action!=casting)
  {
    if (useltm)
    {
      tile+=item_tile_mod(shieldModify);
    }
  }
  tile+=dmap_tile_mod();
  if (!(get_bit(quest_rules,qr_LINKFLICKER)&&((superman||hclk)&&(frame&1))))
  {
    masked_draw(dest);
  }

  //draw held items after Link so they don't go behind his head
  if(action==landhold1 || action==landhold2)
  {
    if (get_bit(quest_rules,qr_HOLDITEMANIMATION))
    {
      putitem2(dest,x-((action==landhold1)?4:0),y+yofs-16,holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
    }
    else
    {
      putitem(dest,x-((action==landhold1)?4:0),y+yofs-16,holditem);
    }
  }
  else if(action==waterhold1 || action==waterhold2)
  {
    if (get_bit(quest_rules,qr_HOLDITEMANIMATION))
    {
      putitem2(dest,x-((action==waterhold1)?4:0),y+yofs-12,holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
    }
    else
    {
      putitem(dest,x-((action==waterhold1)?4:0),y+yofs-12,holditem);
    }
  }








  if(fairyclk==0||(get_bit(quest_rules,qr_NOHEARTRING)))
  {
    xofs=oxofs;
    yofs=oyofs;
    return;
  }

  double a = fairyclk*2*PI/80 + (PI/2);
  int hearts=0;
  //  int htile = QHeader.dat_flags[ZQ_TILES] ? 2 : 0;
  int htile = 2;
  do
  {
    int nx=125;
    if (get_bit(quest_rules,qr_HEARTRINGFIX))
    {
      nx=x;
    }
    int ny=88;
    if (get_bit(quest_rules,qr_HEARTRINGFIX))
    {
      ny=y;
    }
    double tx = cos(a)*53  +nx;
    double ty = -sin(a)*53 +ny+playing_field_offset;
    overtile8(dest,htile,int(tx),int(ty),1,0);
    a-=PI/4;
    ++hearts;
  } while(a>PI/2 && hearts<8);
  xofs=oxofs;
  yofs=oyofs;
}

void LinkClass::masked_draw(BITMAP* dest)
{
  if(isdungeon() && currscr<128 && (x<16 || x>224 || y<18 || y>146) && !get_bit(quest_rules,qr_FREEFORM))
  {                                                         // clip under doorways
    BITMAP *sub=create_sub_bitmap(dest,16,playing_field_offset+16,224,144);
    if(sub!=NULL)
    {
      yofs -= (playing_field_offset+16);
      xofs -= 16;
      sprite::draw(sub);
      xofs=0;
      destroy_bitmap(sub);
    }
  }
  else
  {
    sprite::draw(dest);
  }
  return;
}

// separate case for sword/wand/hammer only
// the main weapon checking is in the global function check_collisions()
void LinkClass::checkstab()
{
  if(action!=attacking || (attack!=wSword && attack!=wWand && attack!=wHammer)
     || (attackclk<=4))
    return;

  int wx=0,wy=0,wxsz=0,wysz=0;
  switch(dir)
  {
    case up:    wx=x+1;  wy=y-13; wxsz=14; wysz=20; break;
    case down:  wx=x+1;  wy=y+5;  wxsz=14; wysz=20; break;
    case left:  wx=x-12; wy=y+2;  wxsz=20; wysz=14; break;
    case right: wx=x+8;  wy=y+2;  wxsz=20; wysz=14; break;
  }

  if((attack==wHammer) && (attackclk<15))
  {

    switch(dir)
    {
      case up:    wx=x-1;  wy=y-4;   break;
      case down:  wx=x+8;  wy=y+28;  break;
      case left:  wx=x-13; wy=y+14;  break;
      case right: wx=x+21; wy=y+14;  break;
    }
    if (attackclk==12)
    {
      decorations.add(new dHammerSmack((fix)wx, (fix)wy, dHAMMERSMACK, 0));
    }
    return;
  }

  if (get_gamedata_canslash(game) && attack==wSword && attackclk<11)
  {
    switch(dir)
    {
      case up:
      wx=x+8; wy=y;
      if(attackclk>=8)
      {
        wy-=8;
      }
      break;
      case down:
      wx=x-8; wy=y;
      if(attackclk>=8)
      {
        wy+=8;
      }
      break;
      case left:
      wx=x; wy=y-7;
      if(attackclk>=8)
      {
        wx-=8;
      }
      break;
      case right:
      wx=x; wy=y-8;
      if(attackclk>=8)
      {
        wx+=8;
      }
      break;
    }
    wxsz=16; wysz=16;
  }

  for(int i=0; i<guys.Count(); i++)
  {
    if(guys.spr(i)->hit(wx,wy,wxsz,wysz) || (attack==wWand && guys.spr(i)->hit(x,y-8,16,24))
       || (attack==wHammer && guys.spr(i)->hit(x,y-8,16,24)))
    {
      int h = hit_enemy(i,attack,(attack==wSword)
                        ? ((1*DAMAGE_MULTIPLIER)<<(current_item(itype_sword,true)-1)) :
                        (attack==wWand) ? 2*DAMAGE_MULTIPLIER
                        : 4*(DAMAGE_MULTIPLIER), wx,wy,dir);
      if(h>0 && hclk==0 && !inlikelike)
      {
        if(GuyHit(i,x+7,y+7,2,2)!=-1)
        {
          hitlink(i);
        }
      }
      if(h==2)
        break;
    }
  }
  if(!get_bit(quest_rules,qr_NOITEMMELEE))
  {
    for(int j=0; j<items.Count(); j++)
    {
      if(((item*)items.spr(j))->pickup & ipTIMER)
      {
        if(((item*)items.spr(j))->clk2 >= 32)
        {
          if(items.spr(j)->hit(wx,wy,wxsz,wysz) || (attack==wWand && items.spr(j)->hit(x,y-8,16,24))
             || (attack==wHammer && items.spr(j)->hit(x,y-8,16,24)))
          {
            getitem(items.spr(j)->id);
            items.del(j);
            for(int i=0; i<Lwpns.Count(); i++)
            {
              weapon *w = (weapon*)Lwpns.spr(i);
              if(w->dragging==j)
              {
                w->dragging=-1;
              }
              else if (w->dragging>j)
                {
                  w->dragging-=1;
                }
            }
            --j;
          }
        }
      }
    }
  }

  if(attack==wSword)
  {
    for (int q=0; q<176; q++)
    {
      set_bit(screengrid,q,0);
    }
    if(dir==up && ((int(x)&15)==0))
    {
      check_slash_block(wx,wy);
      check_slash_block(wx,wy+8);
    }
    else if(dir==up && ((int(x)&15)==8||get_bit(quest_rules,qr_LTTPWALK)))
      {
        check_slash_block(wx,wy);
        check_slash_block(wx,wy+8);
        check_slash_block(wx+8,wy);
        check_slash_block(wx+8,wy+8);
      }
      if(dir==down && ((int(x)&15)==0))
    {
      check_slash_block(wx,wy+wysz-8);
      check_slash_block(wx,wy+wysz);
    }
    else if(dir==down && ((int(x)&15)==8||get_bit(quest_rules,qr_LTTPWALK)))
      {
        check_slash_block(wx,wy+wysz-8);
        check_slash_block(wx,wy+wysz);
        check_slash_block(wx+8,wy+wysz-8);
        check_slash_block(wx+8,wy+wysz);
      }
      if(dir==left)
    {
      check_slash_block(wx,wy+8);
      check_slash_block(wx+8,wy+8);
    }
    if(dir==right)
    {
      check_slash_block(wx+wxsz,wy+8);
      check_slash_block(wx+wxsz-8,wy+8);
    }
  }
  else if(attack==wWand)
    {
      for (int q=0; q<176; q++)
      {
        set_bit(screengrid,q,0);
      }
      // cutable blocks
      if(dir==up && (int(x)&15)==0)
      {
        check_wand_block(x,wy);
        check_wand_block(x,wy+8);
      }
      else if(dir==up && ((int(x)&15)==8||get_bit(quest_rules,qr_LTTPWALK)))
        {
          check_wand_block(x,wy);
          check_wand_block(x,wy+8);
          check_wand_block(x+8,wy);
          check_wand_block(x+8,wy+8);
        }
        if(dir==down && (int(x)&15)==0)
      {
        check_wand_block(x,wy+wysz-8);
        check_wand_block(x,wy+wysz);
      }
      else if(dir==down && ((int(x)&15)==8||get_bit(quest_rules,qr_LTTPWALK)))
        {
          check_wand_block(x,wy+wysz-8);
          check_wand_block(x,wy+wysz);
          check_wand_block(x+8,wy+wysz-8);
          check_wand_block(x+8,wy+wysz);
        }
        if(dir==left)
      {
        check_wand_block(wx,y+8);
        check_wand_block(wx+8,y+8);
      }
      if(dir==right)
      {
        check_wand_block(wx+wxsz,y+8);
        check_wand_block(wx+wxsz-8,y+8);
      }
    }
    else if ((attack==wHammer) && (attackclk==15))
      {
        /*
          // general area stun
          for (int i=0; i<GuyCount(); i++) {
          //      tempdistance=sqrt(pow(abs(x-GuyX(i)),2)+pow(abs(y-GuyY(i)),2));
          if ((distance(x,y,GuyX(i),GuyY(i))<64)&&(!isflier(GuyID(i)))) {
          StunGuy(i);
          }
          }
          */

        // poundable blocks
        for (int q=0; q<176; q++)
        {
          set_bit(screengrid,q,0);
        }
        if(dir==up && (int(x)&15)==0)
        {
          check_pound_block(x,wy);
          check_pound_block(x,wy+8);
        }
        else if(dir==up && ((int(x)&15)==8||get_bit(quest_rules,qr_LTTPWALK)))
          {
            check_pound_block(x,wy);
            check_pound_block(x,wy+8);
            check_pound_block(x+8,wy);
            check_pound_block(x+8,wy+8);
          }
          if(dir==down && (int(x)&15)==0)
        {
          check_pound_block(x,wy+wysz-8);
          check_pound_block(x,wy+wysz);
        }
        else if(dir==down && ((int(x)&15)==8||get_bit(quest_rules,qr_LTTPWALK)))
          {
            check_pound_block(x,wy+wysz-8);
            check_pound_block(x,wy+wysz);
            check_pound_block(x+8,wy+wysz-8);
            check_pound_block(x+8,wy+wysz);
          }
          if(dir==left)
        {
          check_pound_block(wx,y+8);
          check_pound_block(wx+8,y+8);
        }
        if(dir==right)
        {
          check_pound_block(wx+wxsz,y+8);
          check_pound_block(wx+wxsz-8,y+8);
        }
      }
      return;
}

void LinkClass::check_slash_block(int bx, int by)
{
  //keep things inside the screen boundaries
  bx=vbound(bx, 0, 255);
  by=vbound(by, 0, 176);
  int fx=vbound(bx, 0, 255);
  int fy=vbound(by, 0, 176);

  //find out which combo row/column the coordinates are in
  bx &= 0xF0;
  by &= 0xF0;

  int type = COMBOTYPE(bx,by);
  int flag = MAPFLAG(bx,by);
  int flag2 = MAPCOMBOFLAG(bx,by);
  int flag3 = MAPFFCOMBOFLAG(fx,fy);
  int i = (bx>>4) + by;

  if(type!=cSLASH && type!=cSLASHITEM && type!=cBUSH && type!=cFLOWERS &&
     type!=cTALLGRASS && (flag<mfSWORD || flag>mfXSWORD) &&
     flag!=mfSTRIKE && (flag2<mfSWORD || flag2>mfXSWORD) && flag2!=mfSTRIKE
	 && (flag3<mfSWORD || flag3>mfXSWORD) && flag3!=mfSTRIKE)
    return;

  if(i > 175)
    return;

  if (get_bit(screengrid,i))
  {
    return;
  }

  mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);

  if((flag >= 16)&&(flag <= 31))
  {
    s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
    s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
    s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
  }
  else if(flag == mfARMOS_SECRET)
  {
    s->data[i] = s->secretcombo[sSTAIRS];
    s->cset[i] = s->secretcset[sSTAIRS];
    s->sflag[i] = s->secretflag[sSTAIRS];
    if (!nosecretsounds)
    {
      sfx(WAV_SECRET);
    }
  }
  else if ((flag>=mfSWORD&&flag<=mfXSWORD)||(flag==mfSTRIKE))
  {
    for (int i=0; i<current_item(itype_sword,true); i++)
    {
      findentrance(bx,by,mfSWORD+i,true);
    }
    findentrance(bx,by,mfSTRIKE,true);
  }
  else if((flag2 >= 16)&&(flag2 <= 31))
  {
    s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
    s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
    s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
  }
  else if(flag2 == mfARMOS_SECRET)
  {
    s->data[i] = s->secretcombo[sSTAIRS];
    s->cset[i] = s->secretcset[sSTAIRS];
    s->sflag[i] = s->secretflag[sSTAIRS];
    if (!nosecretsounds)
    {
      sfx(WAV_SECRET);
    }
  }
  else if ((flag2>=mfSWORD&&flag2<=mfXSWORD)||(flag2==mfSTRIKE))
  {
    for (int i=0; i<current_item(itype_sword,true); i++)
    {
      findentrance(bx,by,mfSWORD+i,true);
    }
    findentrance(bx,by,mfSTRIKE,true);
  }
  else if ((flag3>=mfSWORD&&flag<=mfXSWORD)||(flag3==mfSTRIKE))
  {
    for (int i=0; i<current_item(itype_sword,true); i++)
    {
      findentrance(fx,fy,mfSWORD+i,true);
    }
    findentrance(fx,fy,mfSTRIKE,true);
  }
  else
  {
    s->data[i] = s->undercombo;
    s->cset[i] = s->undercset;
    s->sflag[i] = 0;
    set_bit(screengrid,i,1);
    //    pausenow=true;
  }

  if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && !getmapflag())
  {
    items.add(new item((fix)bx, (fix)by, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
    if (!nosecretsounds)
    {
      sfx(WAV_SECRET);
    }
  }
  else if (type==cSLASHITEM||type==cBUSH||type==cFLOWERS||type==cTALLGRASS)
  {
    int it=-1;
    int r=rand()%100;

    if(r<15)      it=iHeart;                                // 15%
    else if(r<35) it=iRupy;                                 // 20%

      if(it!=-1)
      items.add(new item((fix)bx, (fix)by, it, ipBIGRANGE + ipTIMER, 0));
  }

  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
  switch (type)
  {
    case cBUSH:
    decorations.add(new dBushLeaves((fix)bx, (fix)by, dBUSHLEAVES, 0));
    break;
    case cFLOWERS:
    decorations.add(new dFlowerClippings((fix)bx, (fix)by, dFLOWERCLIPPINGS, 0));
    break;
    case cTALLGRASS:
    decorations.add(new dGrassClippings((fix)bx, (fix)by, dGRASSCLIPPINGS, 0));
    break;
  }
}

void LinkClass::check_wand_block(int bx, int by)
{
  //keep things inside the screen boundaries
  bx=vbound(bx, 0, 255);
  by=vbound(by, 0, 176);
  int fx=vbound(bx, 0, 255);
  int fy=vbound(by, 0, 176);

  //find out which combo row/column the coordinates are in
  bx &= 0xF0;
  by &= 0xF0;

  int flag = MAPFLAG(bx,by);
  int flag2 = MAPCOMBOFLAG(bx,by);
  int flag3=0;
  int flag31 = MAPFFCOMBOFLAG(fx,fy);
  int flag32 = MAPFFCOMBOFLAG(fx,fy);
  int flag33 = MAPFFCOMBOFLAG(fx,fy);
  int flag34 = MAPFFCOMBOFLAG(fx,fy);
  if(flag31==mfWAND||flag32==mfWAND||flag33==mfWAND||flag34==mfWAND)
    flag3=mfWAND;
  if(flag31==mfSTRIKE||flag32==mfSTRIKE||flag33==mfSTRIKE||flag34==mfSTRIKE)
	flag3=mfSTRIKE;
  int i = (bx>>4) + by;

  if(flag!=mfWAND&&flag2!=mfWAND&&flag3!=mfWAND&&flag!=mfSTRIKE&&flag2!=mfSTRIKE&&flag3!=mfSTRIKE)
    return;

  if(i > 175)
    return;

  //mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);

  //findentrance(bx,by,mfWAND,true);
  //findentrance(bx,by,mfSTRIKE,true);
  if((findentrance(bx,by,mfWAND,true)==false)&&(findentrance(bx,by,mfSTRIKE,true)==false))
  {
    if(flag3==mfWAND||flag3==mfSTRIKE)
	{
      findentrance(fx,fy,mfWAND,true);
	  findentrance(fx,fy,mfSTRIKE,true);
	}
  }
  //putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
}

void LinkClass::check_pound_block(int bx, int by)
{
  //keep things inside the screen boundaries
  bx=vbound(bx, 0, 255);
  by=vbound(by, 0, 176);
  int fx=vbound(bx, 0, 255);
  int fy=vbound(by, 0, 176);

  //find out which combo row/column the coordinates are in
  bx &= 0xF0;
  by &= 0xF0;

  int type = COMBOTYPE(bx,by);
  int flag = MAPFLAG(bx,by);
  int flag2 = MAPCOMBOFLAG(bx,by);
  int flag3 = MAPCOMBOFLAG(fx,fy);
  int i = (bx>>4) + by;

  if(type!=cPOUND && flag!=mfHAMMER && flag!=mfSTRIKE && flag2!=mfHAMMER && flag2!=mfSTRIKE)
    return;

  if(i > 175)
    return;

  if (get_bit(screengrid,i))
  {
    return;
  }

  mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);

  if((flag >= 16)&&(flag <= 31))
  {
    s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
    s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
    s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
  }
  else if(flag == mfARMOS_SECRET)
    {
      s->data[i] = s->secretcombo[sSTAIRS];
      s->cset[i] = s->secretcset[sSTAIRS];
      s->sflag[i] = s->secretflag[sSTAIRS];
      if (!nosecretsounds)
      {
        sfx(WAV_SECRET);
      }
    }
    else if (flag==mfHAMMER||flag==mfSTRIKE)
      {
        findentrance(bx,by,mfHAMMER,true);
        findentrance(bx,by,mfSTRIKE,true);
      }
      else if((flag2 >= 16)&&(flag2 <= 31))
        {
          s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
          s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
          s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
        }
        else if(flag2 == mfARMOS_SECRET)
          {
            s->data[i] = s->secretcombo[sSTAIRS];
            s->cset[i] = s->secretcset[sSTAIRS];
            s->sflag[i] = s->secretflag[sSTAIRS];
            if (!nosecretsounds)
            {
              sfx(WAV_SECRET);
            }
          }
          else if (flag2==mfHAMMER||flag2==mfSTRIKE)
            {
              findentrance(bx,by,mfHAMMER,true);
              findentrance(bx,by,mfSTRIKE,true);
            }
		  else if (flag3==mfHAMMER||flag3==mfSTRIKE)
            {
              findentrance(fx,fy,mfHAMMER,true);
              findentrance(fx,fy,mfSTRIKE,true);
            }
            else
            {
              s->data[i]+=1;
              set_bit(screengrid,i,1);
            }

  if(flag==mfARMOS_ITEM && !getmapflag())
  {
    items.add(new item((fix)bx, (fix)by, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
    if (!nosecretsounds)
    {
      sfx(WAV_SECRET);
    }
  }
  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
  return;
}

int LinkClass::EwpnHit()
{
  for(int i=0; i<Ewpns.Count(); i++)
    if(Ewpns.spr(i)->hit(x+7,y+7,2,2))
    {
      weapon *ew = (weapon*)(Ewpns.spr(i));
      bool hitshield=false;
      if ((ew->ignoreLink)==true)
        break;
      if (ew->id==ewWind)
      {
        xofs=1000;
        action=freeze;
        ew->misc=999;                                         // in enemy wind
        attackclk=0;
        return -1;
      }

      switch(dir)
      {
        case up:
        if(ew->dir==down || ew->dir==l_down || ew->dir==r_down)
          hitshield=true;
        break;
        case down:
        if(ew->dir==up || ew->dir==l_up || ew->dir==r_up)
          hitshield=true;
        break;
        case left:
        if(ew->dir==right || ew->dir==r_up || ew->dir==r_down)
          hitshield=true;
        break;
        case right:
        if(ew->dir==left || ew->dir==l_up || ew->dir==l_down)
          hitshield=true;
        break;
      }
      switch (ew->id)
      {
        case ewLitBomb:
        case ewBomb:
        case ewLitSBomb:
        case ewSBomb:
          return i;
      }
      if(!hitshield || action==attacking || action==swimming || hopclk==0xFF)
      {
        return i;
      }

      switch(ew->id)
      {
        case ewFireball:
        case ewMagic:
        case ewSword:
        if((current_item(itype_shield,true)<i_largeshield) || ew->type)
        {
          return i;
        }
        break;
        case ewFlame:
        if(current_item(itype_shield,true)<i_mirrorshield)
          return i;
        break;
      }

      int oldid = ew->id;
      ew->onhit(false, (current_item(itype_shield,true)>=i_mirrorshield) ? 2 : 1, dir);
      if(ew->id != oldid)                                     // changed type from ewX to wX
      {
        //        ew->power*=DAMAGE_MULTIPLIER;
        Lwpns.add(ew);
        Ewpns.remove(ew);
      }
      if (ew->id==wRefMagic)
      {
        ew->ignoreLink=true;
        ew->ignorecombo=-1;
      }

      sfx(WAV_CHINK,pan(int(x)));
    }
    return -1;
}

int LinkClass::LwpnHit()                                    //only here to check magic hits
{
  for(int i=0; i<Lwpns.Count(); i++)
    if(Lwpns.spr(i)->hit(x+7,y+7,2,2))
    {
      weapon *lw = (weapon*)(Lwpns.spr(i));
      bool hitshield=false;
      if ((lw->ignoreLink)==true)
        break;

      switch(dir)
      {
        case up:
        if(lw->dir==down || lw->dir==l_down || lw->dir==r_down)
          hitshield=true;
        break;
        case down:
        if(lw->dir==up || lw->dir==l_up || lw->dir==r_up)
          hitshield=true;
        break;
        case left:
        if(lw->dir==right || lw->dir==r_up || lw->dir==r_down)
          hitshield=true;
        break;
        case right:
        if(lw->dir==left || lw->dir==l_up || lw->dir==l_down)
          hitshield=true;
        break;
      }

      switch(lw->id)
      {
        case wMagic:
        case wRefMagic:
        if((current_item(itype_shield,true)<i_largeshield) || lw->type)
        {
          return i;
        }
        break;
        default:
        return -1;
        break;
      }

      if(!hitshield || action==attacking || action==swimming || hopclk==0xFF)
        return i;

      lw->onhit(false, (current_item(itype_shield,true)>=i_mirrorshield) ? 2 : 1, dir);
      if (lw->id==wRefMagic)
      {
        lw->ignoreLink=true;
        lw->ignorecombo=-1;
      }

      sfx(WAV_CHINK,pan(int(x)));
    }
    return -1;
}

void LinkClass::checkhit()
{
  if(checklink==true)
  {
    if(hclk>0)
    {
      --hclk;
    }
    if(NayrusLoveShieldClk>0)
    {
      --NayrusLoveShieldClk;
    }
  }

  if(hclk<39 && action==gothit)
    action=none;
  if(hclk<39 && action==swimhit)
    action=swimming;

  if(hclk>=40 && action==gothit)
  {
    if (((ladderx+laddery) && ((hitdir&2)==ladderdir))||(!(ladderx+laddery)))
    {
      for(int i=0; i<4; i++)
      {
        byte lttpcol = get_bit(quest_rules, qr_LTTPCOLLISION);
        switch(hitdir)
        {
          case up:    if(hit_walkflag(x,y+(lttpcol?-1:7),2)||(int(x)&7?hit_walkflag(x+16,y+(lttpcol?-1:7),1):0))    action=none; else --y; break;
          case down:  if(hit_walkflag(x,y+16,2)||(int(x)&7?hit_walkflag(x+16,y+16,1):0))   action=none; else ++y; break;
          case left:  if(hit_walkflag(x-1,y+(lttpcol?0:8),1)||hit_walkflag(x-1,y+8,1)||(int(y)&7?hit_walkflag(x-1,y+16,1):0))  action=none; else --x; break;
          case right: if(hit_walkflag(x+16,y+(lttpcol?0:8),1)||hit_walkflag(x+16,y+8,1)||(int(y)&7?hit_walkflag(x+16,y+16,1):0)) action=none; else ++x; break;
        }
      }
    }
  }

  if(hclk>0 || inlikelike || action==inwind || inwallm || diveclk>30 || (action==hopping && hopclk<255) )
  {
    return;
  }

  for(int i=0; i<Lwpns.Count(); i++)
  {
    sprite *s = Lwpns.spr(i);

    if (!get_bit(quest_rules,qr_FIREPROOFLINK))
    {
      if(s->id==wFire && (superman ? (get_bit(quest_rules,qr_LTTPWALK)?s->hit(x+4,y+4,7,7):s->hit(x+7,y+7,2,2)) : s->hit(this))&&
         ((weapon*)(Lwpns.spr(i)))->type<3)
      {
        if(NayrusLoveShieldClk<=0)
        {
          set_gamedata_life(game, max(get_gamedata_life(game)-((HP_PER_HEART/2)>>current_item(itype_ring,true)),0));
        }
        hitdir = s->hitdir(x,y,16,16,dir);
        if(action!=rafting && action!=freeze)
          action=gothit;
        if(action==swimming || hopclk==0xFF)
          action=swimhit;
        hclk=48;
        sfx(WAV_OUCH,pan(int(x)));
        return;
      }
    }
    //   check enemy weapons true, 1, -1
    //
    if (get_bit(quest_rules,qr_Z3BRANG_HSHOT))
    {
      if (s->id==wBrang || s->id==wHookshot)
      {
        int w=(s->id==wHookshot)?0:current_item(itype_brang,true);
        for(int j=0; j<Ewpns.Count(); j++)
        {
          sprite *t = Ewpns.spr(j);
          if (s->hit(t->x+7,t->y+7,2,2))
          {
            switch (w)
            {
              case 0:                                       //hookshot
              switch (t->id)
              {
                case ewLitBomb:
                case ewLitSBomb:
                case ewFireball:
                case ewSword:
                case ewBrang:
                case ewArrow:
                case ewRock:
                ((weapon*)s)->dead=1;
                ((weapon*)t)->onhit(true, 1, -1);
                break;
                case ewMagic:
                break;
              }
              break;
              case 1:                                       //wooden boomerang
              switch (t->id)
              {
                case ewLitBomb:
                case ewLitSBomb:
                case ewBrang:
                case ewArrow:
                case ewRock:
                ((weapon*)s)->dead=1;
                ((weapon*)t)->onhit(true, 1, -1);
                break;
                case ewFireball:
                case ewSword:
                case ewMagic:
                break;
              }
              break;
              case 2:                                       //magic boomerang
              switch (t->id)
              {
                case ewLitBomb:
                case ewLitSBomb:
                case ewBrang:
                case ewArrow:
                case ewRock:
                case ewFireball:
                case ewSword:
                case ewMagic:
                ((weapon*)s)->dead=1;
                ((weapon*)t)->onhit(true, 1, -1);
                if (s->dummy_bool[0])
                {
                  add_grenade(s->x,s->y,0);
                  s->dummy_bool[0]=false;
                }
                break;
              }
              break;
              case 3:                                       //fire boomerang
              switch (t->id)
              {
                case ewLitBomb:
                case ewLitSBomb:
                case ewBrang:
                case ewArrow:
                case ewRock:
                case ewFireball:
                case ewSword:
                case ewMagic:
                weapon *ew = ((weapon*)t);
                int oldid = ew->id;
                ((weapon*)s)->dead=1;
                ew->onhit(true, 2, ew->dir);
                if (s->dummy_bool[0])
                {
                  add_grenade(s->x,s->y,1);
                  s->dummy_bool[0]=false;
                }
                if(ew->id != oldid)                     // changed type from ewX to wX
                {
                  Lwpns.add(ew);
                  Ewpns.remove(ew);
                }
                if (ew->id==wRefMagic)
                {
                  ew->ignoreLink=true;
                  ew->ignorecombo=-1;
                }

                break;
              }
              break;
            }
          }
        }
      }
    }

    if (get_bit(quest_rules,qr_OUCHBOMBS))
    {
      //     if(((s->id==wBomb)||(s->id==wSBomb)) && (superman ? s->hit(x+7,y+7,2,2) : s->hit(this)))
      if(((s->id==wBomb)||(s->id==wSBomb)) && s->hit(this) && !superman)
      {
        if(NayrusLoveShieldClk<=0)
        {
          set_gamedata_life(game, max(get_gamedata_life(game)-((4*HP_PER_HEART/2)>>current_item(itype_ring,true)),0));
          if (s->id==wSBomb)
          {
            set_gamedata_life(game, max(get_gamedata_life(game)-((12*HP_PER_HEART/2)>>current_item(itype_ring,true)),0));
          }
        }
        hitdir = s->hitdir(x,y,16,16,dir);
        if(action!=rafting && action!=freeze)
          action=gothit;
        if(action==swimming || hopclk==0xFF)
          action=swimhit;
        hclk=48;
        sfx(WAV_OUCH,pan(int(x)));
        return;
      }
    }
    if(hclk==0 && s->id==wWind && s->hit(x+7,y+7,2,2))
    {
      xofs=1000;
      action=inwind;
      attackclk=0;
      return;
    }
  }

  if(action==rafting || action==freeze ||
     action==casting || superman)
    return;

  int hit = get_bit(quest_rules,qr_LTTPWALK)?GuyHit(x+4,y+4,8,8):GuyHit(x+7,y+7,2,2);
  if(hit!=-1)
  {
    hitlink(hit);
    return;
  }

  hit = LwpnHit();
  if(hit!=-1)
  {
    if(NayrusLoveShieldClk<=0)
    {
      set_gamedata_life(game, max(get_gamedata_life(game)-(lwpn_dp(hit)>>current_item(itype_ring,true)),0));
    }
    hitdir = Lwpns.spr(hit)->hitdir(x,y,16,16,dir);
    ((weapon*)Lwpns.spr(hit))->onhit(false);
    if(action==swimming || hopclk==0xFF)
      action=swimhit;
    else
      action=gothit;
    hclk=48;
    sfx(WAV_OUCH,pan(int(x)));
    return;
  }

  hit = EwpnHit();
  if(hit!=-1)
  {
    if(NayrusLoveShieldClk<=0)
    {
      set_gamedata_life(game, max(get_gamedata_life(game)-(ewpn_dp(hit)>>current_item(itype_ring,true)),0));
    }
    hitdir = Ewpns.spr(hit)->hitdir(x,y,16,16,dir);
    ((weapon*)Ewpns.spr(hit))->onhit(false);
    if(action==swimming || hopclk==0xFF)
      action=swimhit;
    else
      action=gothit;
    hclk=48;
    sfx(WAV_OUCH,pan(int(x)));
    return;
  }


  int dx1 = (int)x+8-(tmpscr->csensitive);
  int dx2 = (int)x+8+(tmpscr->csensitive-1);
  int dy1 = (int)y+(get_bit(quest_rules,qr_LTTPCOLLISION)?8:12)-(get_bit(quest_rules,qr_LTTPCOLLISION)?tmpscr->csensitive:(tmpscr->csensitive+1)/2);
  int dy2 = (int)y+(get_bit(quest_rules,qr_LTTPCOLLISION)?8:12)+(get_bit(quest_rules,qr_LTTPCOLLISION)?tmpscr->csensitive-1:((tmpscr->csensitive+1)/2)-1);
  int ctype1(combobuf[MAPCOMBO(dx1,dy1)].type);
  int ctype2(combobuf[MAPCOMBO(dx1,dy2)].type);
  int ctype3(combobuf[MAPCOMBO(dx2,dy1)].type);
  int ctype4(combobuf[MAPCOMBO(dx2,dy2)].type);
  if(get_bit(quest_rules,qr_DMGCOMBOPRI))
  {
    if (((ctype1>=cDAMAGE1)&&(ctype1<=cDAMAGE4))||((ctype2>=cDAMAGE1)&&(ctype2<=cDAMAGE4))||((ctype3>=cDAMAGE1)&&(ctype3<=cDAMAGE4))||((ctype4>=cDAMAGE1)&&(ctype4<=cDAMAGE4))||
		((ctype1>=cDAMAGE5)&&(ctype1<=cDAMAGE7))||((ctype2>=cDAMAGE5)&&(ctype2<=cDAMAGE7))||((ctype3>=cDAMAGE5)&&(ctype3<=cDAMAGE7))||((ctype4>=cDAMAGE5)&&(ctype4<=cDAMAGE7)))
    {
      if((ctype1<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype1>cDAMAGE7))
        ctype1=cDAMAGE1;
      if((ctype2<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype2>cDAMAGE7))
        ctype2=cDAMAGE1;
	  if((ctype3<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype3>cDAMAGE7))
        ctype3=cDAMAGE1;
	  if((ctype4<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype4>cDAMAGE7))
        ctype4=cDAMAGE1;
	}
  }
  int ctypea=max(ctype1, ctype2);
  int ctypeb=max(ctype3, ctype4);
  int ctypec = max(ctypea,ctypeb);

  ctype1=combobuf[MAPFFCOMBO(dx1,dy1)].type;
  ctype2=combobuf[MAPFFCOMBO(dx1,dy2)].type;
  ctype3=combobuf[MAPFFCOMBO(dx2,dy1)].type;
  ctype4=combobuf[MAPFFCOMBO(dx2,dy2)].type;
  if(get_bit(quest_rules,qr_DMGCOMBOPRI))
  {
    if (((ctype1>=cDAMAGE1)&&(ctype1<=cDAMAGE4))||((ctype2>=cDAMAGE1)&&(ctype2<=cDAMAGE4))||((ctype3>=cDAMAGE1)&&(ctype3<=cDAMAGE4))||((ctype4>=cDAMAGE1)&&(ctype4<=cDAMAGE4))||
		((ctype1>=cDAMAGE5)&&(ctype1<=cDAMAGE7))||((ctype2>=cDAMAGE5)&&(ctype2<=cDAMAGE7))||((ctype3>=cDAMAGE5)&&(ctype3<=cDAMAGE7))||((ctype4>=cDAMAGE5)&&(ctype4<=cDAMAGE7)))
    {
      if((ctype1<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype1>cDAMAGE7))
        ctype1=cDAMAGE1;
      if((ctype2<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype2>cDAMAGE7))
        ctype2=cDAMAGE1;
	  if((ctype3<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype3>cDAMAGE7))
        ctype3=cDAMAGE1;
	  if((ctype4<cDAMAGE1||ctype1>cDAMAGE4)&&(ctype1<cDAMAGE5||ctype4>cDAMAGE7))
        ctype4=cDAMAGE1;
	}
  }
  ctypea=max(ctype1, ctype2);
  ctypeb=max(ctype3, ctype4);
  int ctyped = max(ctypea,ctypeb);
  int ctype = max(ctypec,ctyped);

  int damage=0xFF;
  bool global_ring = (get_bit(quest_rules,qr_RINGAFFECTDAMAGE) != 0);
  bool current_ring = ((tmpscr->flags6&fTOGGLERINGDAMAGE) != 0);
  int ringoffset=(global_ring!=current_ring)?current_item(itype_ring,true):0;

  if(ctype==cDAMAGE1) damage=6;
  else if(ctype==cDAMAGE2) damage=5;
  else if(ctype==cDAMAGE3) damage=4;
  else if(ctype==cDAMAGE4) damage=3;
  else if(ctype==cDAMAGE5) damage=2;
  else if(ctype==cDAMAGE6) damage=1;
  else if(ctype==cDAMAGE7) damage=0;

  if(damage!=0xFF)
  {
    if((tmpscr->flags5&fDAMAGEWITHBOOTS)|| (!(current_item(itype_boots, true))) || ((get_bit(quest_rules,qr_MAGICBOOTS))&&(get_gamedata_magic(game)+get_gamedata_dmagic(game)<BOOTSDRAINAMOUNT*get_gamedata_magicdrainrate(game))))
    {
      if(NayrusLoveShieldClk<=0)
      {
        set_gamedata_life(game, max((get_gamedata_life(game)-(((HP_PER_HEART<<5)>>damage)>>ringoffset)),0));
      }
      hitdir = (dir^1);
      if(action!=rafting && action!=freeze)
        action=gothit;
      if(action==swimming || hopclk==0xFF)
        action=swimhit;
      hclk=48;
      sfx(WAV_OUCH,pan(int(x)));
      return;
    }
    else if (get_bit(quest_rules,qr_MAGICBOOTS))
      {
        if (!(magicdrainclk%BOOTSDRAINSPEED))
        {
          change_gamedata_magic(game, -BOOTSDRAINAMOUNT*get_gamedata_magicdrainrate(game));
        }
      }

  }


}

void LinkClass::hitlink(int hit)
{
  if(superman)
    return;

  if(NayrusLoveShieldClk<=0)
  {
    set_gamedata_life(game, max(get_gamedata_life(game)-(enemy_dp(hit)>>current_item(itype_ring,true)),0));
  }
  hitdir = guys.spr(hit)->hitdir(x,y,16,16,dir);

  if(action==swimming || hopclk==0xFF)
    action=swimhit;
  else
    action=gothit;
  hclk=48;
  sfx(WAV_OUCH,pan(int(x)));
  enemy_scored(hit);
  switch(guys.spr(hit)->id)
  {
    case eLIKE:
    EatLink(hit);
    inlikelike=true;
    action=none;
    break;

    case eWALLM:
    if(((enemy*)guys.spr(hit))->hp>0)
    {
      GrabLink(hit);
      inwallm=true;
      action=none;
    }
    break;

    case eBUBBLEST:
    if(swordclk>=0)
      swordclk=150;
    break;

    case eBUBBLESP:
    swordclk=-1;
    break;

    case eBUBBLESR:
    swordclk=0;
    break;

    case eBUBBLEIT:
    if(itemclk>=0)
      itemclk=150;
    break;

    case eBUBBLEIP:
    itemclk=-1;
    break;

    case eBUBBLEIR:
    itemclk=0;
    break;
  }
}

// returns true when game over
bool LinkClass::animate(int index)
{
  int lsave=0;
  if(do_cheat_goto)
  {
    didpit=true;
    pitx=x;
    pity=y;
    dowarp(3,0);
    do_cheat_goto=false;
    return false;
  }

  if (action!=climbcovertop&&action!=climbcoverbottom)
  {
    climb_cover_x=-1000;
    climb_cover_y=-1000;
  }
  if((COMBOTYPE(x,y+15)==cTALLGRASS)&&(COMBOTYPE(x+15,y+15)==cTALLGRASS))
  {
    if (decorations.idCount(dTALLGRASS)==0)
    {
      decorations.add(new dTallGrass(x, y, dTALLGRASS, 0));
    }
  }

  if((COMBOTYPE(x,y+15)==cSHALLOWWATER)&&(COMBOTYPE(x+15,y+15)==cSHALLOWWATER))
  {
    if (decorations.idCount(dRIPPLES)==0)
    {
      decorations.add(new dRipples(x, y, dRIPPLES, 0));
    }
  }

  if (drunkclk)
  {
    --drunkclk;
  }
  if (!is_on_conveyor&&!(get_bit(quest_rules,qr_LTTPWALK)))
  {
    switch (dir)
    {
      case up:
      case down:
      x=(int(x)+4)&0xFFF8;
      break;
      case left:
      case right:
      y=(int(y)+4)&0xFFF8;
      break;
    }
  }
  if ((watch==true) && (get_bit(quest_rules,qr_TEMPCLOCKS)))
  {
    ++clockclk;
    if (clockclk==256)
    {
      if (cheat_superman==false)
      {
        setClock(false);
      }
      watch=false;
      for (int zoras=0; zoras<clock_zoras; zoras++)
      {
        addenemy(0,0,eZORA,0);
      }
    }
  }
  if (hookshot_frozen==true)
  {
    if (hookshot_used==true)
    {
      action=freeze;
      if (pull_link==true)
      {
        sprite *t;
        int i;
        for(i=0; i<Lwpns.Count() && (Lwpns.spr(i)->id!=wHSHandle); i++);
        t = Lwpns.spr(i);
        for(i=0; i<Lwpns.Count(); i++)
        {
          sprite *s = Lwpns.spr(i);
          if(s->id==wHookshot)
          {
            if ((s->y)>y)
            {
              y+=4;
              if (Lwpns.idFirst(wHSHandle)!=-1)
              {
                t->y+=4;
              }
              hs_starty+=4;
            }
            if ((s->y)<y)
            {
              y-=4;
              if (Lwpns.idFirst(wHSHandle)!=-1)
              {
                t->y-=4;
              }
              hs_starty-=4;
            }
            if ((s->x)>x)
            {
              x+=4;
              if (Lwpns.idFirst(wHSHandle)!=-1)
              {
                t->x+=4;
              }
              hs_startx+=4;
            }
            if ((s->x)<x)
            {
              x-=4;
              if (Lwpns.idFirst(wHSHandle)!=-1)
              {
                t->x-=4;
              }
              hs_startx-=4;
            }
          }
        }
      }
    }
    else
    {
      Lwpns.del(Lwpns.idFirst(wHSHandle));
      reset_hookshot();
    }
    if (hs_fix)
    {
      if (dir==up)
      {
        y=int(y+7)&0xF0;
      }
      if (dir==down)
      {
        y=int(y+7)&0xF0;
      }
      if (dir==left)
      {
        x=int(x+7)&0xF0;
      }
      if (dir==right)
      {
        x=int(x+7)&0xF0;
      }
      hs_fix=false;
    }

  }
  if(DrunkrLbtn())         selectBwpn(-1, 0);
  else if(DrunkrRbtn())    selectBwpn(1, 0);
    if(rPbtn())         onViewMap();

  // make the flames from the wand
  if(wand_dead)
  {
    wand_dead=false;
    if(can_use_item(itype_book,i_book) && Lwpns.idCount(wFire)<2)
    {
      Lwpns.add(new weapon((fix)wand_x,(fix)wand_y,wFire,2,1*DAMAGE_MULTIPLIER,0));
      sfx(WAV_FIRE,pan(wand_x));
    }
  }

  if(add_asparkle)
  {
    if (current_item(itype_arrow,true)>=2)
    {
      Lwpns.add(new weapon((fix)(arrow_x+(rand()%7)),
                           (fix)(arrow_y+(rand()%7)),
                           wSSparkle+add_asparkle-1,0,0,0));
    }
    add_asparkle=false;
  }

  if(add_bsparkle)
  {
    if (current_item(itype_brang,true)>=2)
    {
      Lwpns.add(new weapon((fix)(brang_x+(rand()%7)),
                           (fix)(brang_y+(rand()%7)),
                           wMSparkle+add_bsparkle-1,0,0,0));
    }
    add_bsparkle=false;
  }

  if(add_df1asparkle)
  {
    Lwpns.add(new weapon((fix)(df_x+(rand()%7)),(fix)(df_y+(rand()%7)),wPhantom,pDINSFIREROCKETTRAIL,0,0));
    add_df1asparkle=false;
  }

  if(add_df1bsparkle)
  {
    Lwpns.add(new weapon((fix)(df_x+(rand()%7)),(fix)(df_y+(rand()%7)),wPhantom,pDINSFIREROCKETTRAILRETURN,0,0));
    add_df1bsparkle=false;
  }

  if(add_nl1asparkle)
  {
    Lwpns.add(new weapon((fix)(nl1_x+(rand()%7)),(fix)(nl1_y+(rand()%7)),wPhantom,pNAYRUSLOVEROCKETTRAIL1,0,0));
    add_nl1asparkle=false;
  }

  if(add_nl1bsparkle)
  {
    Lwpns.add(new weapon((fix)(nl1_x+(rand()%7)),(fix)(nl1_y+(rand()%7)),wPhantom,pNAYRUSLOVEROCKETTRAILRETURN1,0,0));
    add_nl1bsparkle=false;
  }

  if(add_nl2asparkle)
  {
    Lwpns.add(new weapon((fix)(nl2_x+(rand()%7)),(fix)(nl2_y+(rand()%7)),wPhantom,pNAYRUSLOVEROCKETTRAIL2,0,0));
    add_nl2asparkle=false;
  }

  if(add_nl2bsparkle)
  {
    Lwpns.add(new weapon((fix)(nl2_x+(rand()%7)),(fix)(nl2_y+(rand()%7)),wPhantom,pNAYRUSLOVEROCKETTRAILRETURN2,0,0));
    add_nl2bsparkle=false;
  }

  checkhit();
  if(get_gamedata_life(game)<=0)
  {
    drunkclk=0;
    gameover();

    return true;
  }

  if(swordclk>0)
    --swordclk;

  if(itemclk>0)
    --itemclk;

  if(inwallm)
  {
    attackclk=0;
    linkstep();
    if(CarryLink()==false)
      restart_level();
    return false;
  }

  if (ewind_restart)
  {
    attackclk=0;
    restart_level();
    xofs=0;
    action=none;
    ewind_restart=false;
    return false;
  }

  /*if(!get_bit(quest_rules,qr_LTTPWALK)) {
    if(hopclk)
    action = hopping; }
    else {
    if(hopclk==0xFF)
    action = hopping;
    }*/
  if(hopclk)
    action = hopping;

  // get user input or do other animation
  freeze_guys=false;                                        // reset this flag, set it again if holding
  switch(action)
  {
    case gothit:
    if(attackclk)
      if(!doattack())
        attackclk=0;

    case swimhit:
    case freeze:
    case scrolling:
    break;

    case casting:
    if (magictype==mgc_none)
    {
      action=none;
    }
    break;

    case landhold1:
    case landhold2:
    if(--holdclk == 0)
      action=none;
    else
      freeze_guys=true;
    break;

    case waterhold1:
    case waterhold2:
    diveclk=0;
    if(--holdclk == 0)
      action=swimming;
    else
      freeze_guys=true;
    break;

    case hopping:
    do_hopping();
    break;

    case inwind:
    {
      int i=Lwpns.idFirst(wWind);
      if(i<0)
      {
        if(whirlwind==255)
        {
          action=none;
          xofs=0;
          whirlwind=0;
          dir=right;
          lstep=0;
        }
        else
          x=241;
      }
      else
      {
        x=Lwpns.spr(i)->x;
        y=Lwpns.spr(i)->y;
      }
    }
    break;

    case swimming:
    if(frame&1)
      linkstep();
    // fall through

    default:
    movelink();                                           // call the main movement routine
  }
  // check for ladder removal
  if(get_bit(quest_rules,qr_LTTPWALK))
  {
    if(ladderx+laddery)
    {
      if(ladderdir==up)
      {
        if( (laddery-int(y)>=(16+(ladderstart==dir?ladderstart==down?1:0:0))) || (laddery-int(y)<=(-16-(ladderstart==dir?ladderstart==up?1:0:0))) || (abs(ladderx-int(x))>8))
        {
          reset_ladder();
        }
      }
      else
      {
        if((abs(laddery-int(y))>8) || (ladderx-int(x)>=(16+(ladderstart==dir?ladderstart==right?1:0:0))) || (ladderx-int(x)<=(-16-(ladderstart==dir?ladderstart==left?1:0:0))))
        {
          reset_ladder();
        }
      }
    }
  }
  else
  {
    if((abs(laddery-int(y))>=16) || (abs(ladderx-int(x))>=16))
    {
      reset_ladder();
    }
  }

  if(ilswim)
    landswim++;
  else landswim=0;
  if(hopclk!=0xFF) ilswim=false;

  if((!loaded_guys) && (frame - newscr_clk >= 1))
  {
    if(tmpscr->room==rGANON)
    {
      ganon_intro();
    }
    else
    {
      loadguys();
    }
  }

  if((!loaded_enemies) && (frame - newscr_clk >= 2))
  {
    loadenemies();
  }

  // check lots of other things
  checkscroll();
  if(action!=inwind)
  {
    checkspecial();
    if (action!=attacking)
    {
      checkitems();
    }
    checklocked();
    checklockblock();
    checkbosslockblock();
    checkchest();
    checklockedchest();
    checkbosschest();
    checkpushblock();
    if (hookshot_frozen==false)
    {
      checkspecial2(&lsave);
    }
    if(action==won)

    {
      return true;
    }
  }


  if((!activated_timed_warp) && (tmpscr->timedwarptics>0) &&
     (frame - newscr_clk >= tmpscr->timedwarptics))
  {
    activated_timed_warp=true;
    if(tmpscr->flags4 & fTIMEDDIRECT) {
      didpit=true;
      pitx=x;
      pity=y;
    }
    int index = 0;
    if(tmpscr->flags5 & fRANDOMTIMEDWARP) index=rand()%4;
    sdir = dir; dowarp(1,index);
  }

  bool awarp = false;

  for(int i=0;i<176;i++)
  {

    int ind=0;
    if(!awarp)
    {
      if(combobuf[tmpscr->data[i]].type==cAWARPA)
      {awarp=true; ind=0;}
      else if(combobuf[tmpscr->data[i]].type==cAWARPB)
        {awarp=true; ind=1;}
        else if(combobuf[tmpscr->data[i]].type==cAWARPC)
          {awarp=true; ind=2;}
          else if(combobuf[tmpscr->data[i]].type==cAWARPD)
            {awarp=true; ind=3;}
            else if(combobuf[tmpscr->data[i]].type==cAWARPR)
              {awarp=true; ind=rand()%4;}
              if(awarp)
      {
        if(tmpscr->flags5&fDIRECTAWARP)
        {
          didpit=true;
          pitx=x;
          pity=y;
        }
        sdir = dir; dowarp(1,ind);
      }
    }

  }

  awarp=false;
  for(int i=0;i<32;i++)
  {

    int ind=0;
    if(!awarp)
    {
      if(combobuf[tmpscr->ffdata[i]].type==cAWARPA)
      {awarp=true; ind=0;}
      else if(combobuf[tmpscr->ffdata[i]].type==cAWARPB)
        {awarp=true; ind=1;}
        else if(combobuf[tmpscr->ffdata[i]].type==cAWARPC)
          {awarp=true; ind=2;}
          else if(combobuf[tmpscr->ffdata[i]].type==cAWARPD)
            {awarp=true; ind=3;}
            else if(combobuf[tmpscr->ffdata[i]].type==cAWARPR)
              {awarp=true; ind=rand()%4;}
              if(awarp)
      {
        if(tmpscr->flags5&fDIRECTAWARP)
        {
          didpit=true;
          pitx=x;
          pity=y;
        }
        sdir = dir; dowarp(1,ind);
      }
    }

  }

  if(ffwarp)
  {
	  ffwarp=false;
	  dowarp(1,0);
  }

  // walk through bombed doors and fake walls
  bool walk=false;
  int dtype=dBOMBED;
  if(pushing>=24) dtype=dWALK;

  if(isdungeon() && action!=freeze && loaded_guys && !inlikelike)
  {
    if(((dtype==dBOMBED)?DrunkUp():dir==up) && (get_bit(quest_rules,qr_LTTPWALK)?x>112&&x<128:x==120) && y<=32 && tmpscr->door[0]==dtype)
    {
      walk=true;
      dir=up;
    }

    if(((dtype==dBOMBED)?DrunkDown():dir==down) && (get_bit(quest_rules,qr_LTTPWALK)?x>112&&x<128:x==120) && y>=128 && tmpscr->door[1]==dtype)
    {
      walk=true;
      dir=down;
    }

    if(((dtype==dBOMBED)?DrunkLeft():dir==left) && x<=32 && (get_bit(quest_rules,qr_LTTPWALK)?y>72&&y<88:y==80) && tmpscr->door[2]==dtype)
    {
      walk=true;
      dir=left;
    }

    if(((dtype==dBOMBED)?DrunkRight():dir==right) && x>=208 && (get_bit(quest_rules,qr_LTTPWALK)?y>72&&y<88:y==80) && tmpscr->door[3]==dtype)
    {
      walk=true;
      dir=right;
    }
  }
  if(walk)
  {
    hclk=0;
    drawguys=false;
    if((dtype==dWALK)&&(!nosecretsounds))
    {
      sfx(WAV_SECRET);
    }
    stepforward(29, true);
    action=scrolling;
    pushing=false;
  }

  if(get_gamedata_life(game)<=(HP_PER_HEART))
  {
    if (heart_beep)
    {
      cont_sfx(WAV_ER);
    }
    else
    {
      if (heart_beep_timer==-1)
      {
        heart_beep_timer=70;
      }
      if (heart_beep_timer>0)
      {
        --heart_beep_timer;
        cont_sfx(WAV_ER);
      }
      else
      {
        stop_sfx(WAV_ER);
      }
    }
  }
  else
  {
    heart_beep_timer=-1;
    stop_sfx(WAV_ER);
  }
  if(rSbtn())
  {
    int tmp_subscr_clk = frame;
    switch(lsave)
    {
      case 0:
      conveyclk=3;
      dosubscr(&QMisc);
      newscr_clk += frame - tmp_subscr_clk;
      break;
      case 1:
      save_game((tmpscr->flags4&fSAVEROOM) != 0, 0);
      break;
      case 2:
      save_game((tmpscr->flags4&fSAVEROOM) != 0, 1);
      break;
    }
  }

  checkstab();

  check_conveyor();

  check_cheat_warp();
  return false;
}

bool LinkClass::startwpn(int wpn)                           // an item index
{
  if((dir==up && y<24) || (dir==down && y>128) ||
     (dir==left && x<32) || (dir==right && x>208))
    return false;

  int wx=x;
  int wy=y;
  switch(dir)
  {
    case up:    wy-=16; break;
    case down:  wy+=16; break;
    case left:  wx-=16; break;
    case right: wx+=16; break;
  }
  bool use_hookshot=true;

  switch(wpn&0xFFF)
  {

    case iRPotion:
    case iBPotion:
    //  --game->potion;
    game->items[itype_potion]=game->items[itype_potion]>>1;
    if(get_bit(quest_rules,qr_REDPOTIONONCE)&&(((wpn&0xFFF)==iRPotion)))
    {
      game->items[itype_potion]=game->items[itype_potion]>>1;
    }
    
    Bwpn=0;

    refill_what=REFILL_ALL;
    refill_why=((wpn&0xFFF)==iBPotion?REFILL_BPOTION:REFILL_RPOTION);
    StartRefill();
    while(refill())
    {
      put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,false);
      advanceframe();
    }

    selectBwpn(0, 0);
    return false;

    case iLetter:
    //  if(game->letter==1 && currscr==128 && tmpscr[1].room==rP_SHOP) {
    if(current_item(itype_letter,true)==i_letter &&
       tmpscr[currscr<128?0:1].room==rP_SHOP &&
       tmpscr[currscr<128?0:1].guy &&
       //        ((currscr<128&&isdungeon())||(currscr>=128&&!isdungeon()))
       ((currscr<128&&dlevel)||(currscr>=128&&!isdungeon()))
      )
    {
      game->items[itype_letter]|=i_letter_used;
      setupscreen();
      action=none;
    }
    return false;

    case iWhistle:
    sfx(WAV_WHISTLE);
    if(dir==up || dir==right)
      ++blowcnt;
    else
      --blowcnt;

    for(int i=0; i<150; i++)
    {
      advanceframe();
      if(Quit)
        return false;
    }
    Lwpns.add(new weapon(x,y,wWhistle,0,0,dir));

    if(findentrance(x,y,mfWHISTLE,false))
      didstuff |= did_whistle;

    if((didstuff&did_whistle) || currscr>=128)
      return false;

    didstuff |= did_whistle;
    if(tmpscr->flags&fWHISTLE)
      whistleclk=0;                                       // signal to start drying lake or doing other stuff
    else if(dlevel==0 && TriforceCount())
        Lwpns.add(new weapon((fix)0,(fix)y,wWind,0,0,right));
      return false;

    case iBombs:
    {
      /*
        //remote detonation
        if(Lwpns.idCount(wLitBomb)) {
        weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wLitBomb)));
        ew->clk=41;
        ew->id=wBomb;
        return false;
        }
        */
      if(Lwpns.idCount(wLitBomb))
      {
        return false;
      }
      if(!get_debug())
        --game->items[itype_bomb];
      selectBwpn(8,8);
      if (isdungeon())
      {
        wy=max(wy,16);
      }
      Lwpns.add(new weapon((fix)wx,(fix)wy,wBomb,0,4*DAMAGE_MULTIPLIER,dir));
      sfx(WAV_PLACE,pan(wx));
    } break;

    case iSBomb:
    {
      if(Lwpns.idCount(wLitSBomb))
        return false;
      if(!get_debug())
        --game->items[itype_sbomb];
      selectBwpn(8,8);
      Lwpns.add(new weapon((fix)wx,(fix)wy,wSBomb,0,16*DAMAGE_MULTIPLIER,dir));
      sfx(WAV_PLACE,pan(wx));
    } break;

    case iWand:
    if(Lwpns.idCount(wMagic))
      return false;
    if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
       ((get_gamedata_magic(game)+get_gamedata_dmagic(game))<WANDDRAINAMOUNT*get_gamedata_magicdrainrate(game))&&
       ((get_bit(quest_rules,qr_MAGICWAND))))
      return false;
    if(Lwpns.idCount(wBeam))
      Lwpns.del(Lwpns.idFirst(wBeam));
    Lwpns.add(new weapon((fix)wx,(fix)wy,wMagic,0,2*DAMAGE_MULTIPLIER,dir));
    if (get_bit(quest_rules,qr_MAGICWAND))
      change_gamedata_magic(game, -(WANDDRAINAMOUNT*get_gamedata_magicdrainrate(game)));
    sfx(WAV_WAND,pan(wx));

    /*
      //    Fireball Wand
      Lwpns.add(new weapon((fix)wx,(fix)wy,wRefFireball,0,2*DAMAGE_MULTIPLIER,dir));
      switch (dir) {
      case up:
      Lwpns.spr(Lwpns.Count()-1)->angle=-PI/2;
      Lwpns.spr(Lwpns.Count()-1)->dir=up;
      break;
      case down:
      Lwpns.spr(Lwpns.Count()-1)->angle=PI/2;
      Lwpns.spr(Lwpns.Count()-1)->dir=down;
      break;
      case left:
      Lwpns.spr(Lwpns.Count()-1)->angle=PI;
      Lwpns.spr(Lwpns.Count()-1)->dir=left;
      break;
      case right:
      Lwpns.spr(Lwpns.Count()-1)->angle=0;
      Lwpns.spr(Lwpns.Count()-1)->dir=right;
      break;
      }
      Lwpns.spr(Lwpns.Count()-1)->clk=16;
      ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step=3.5;
      Lwpns.spr(Lwpns.Count()-1)->dummy_bool[0]=true; //homing
      */
    break;

    case iSword:
    if(Lwpns.idCount(wBeam)||Lwpns.idCount(wMagic))
      return false;
    float temppower;
    temppower=DAMAGE_MULTIPLIER<<(current_item(itype_sword,true)-1);
    temppower=temppower*zinit.beam_power[current_item(itype_sword,true)-1];
    temppower=temppower/100;
    Lwpns.add(new weapon((fix)wx,(fix)wy,wBeam,0,int(temppower),dir));
    sfx(WAV_BEAM,pan(wx));
    break;

    case iBCandle: if(didstuff&did_candle) return false;
    case iRCandle:
    if(Lwpns.idCount(wFire)>=2)
      return false;
    if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
       ((get_gamedata_magic(game)+get_gamedata_dmagic(game))<CANDLEDRAINAMOUNT*get_gamedata_magicdrainrate(game))&&
       ((get_bit(quest_rules,qr_MAGICCANDLE))))
      return false;
    didstuff|=did_candle;
    Lwpns.add(new weapon((fix)wx,(fix)wy,wFire,(wpn==iBCandle)?0:1,1*DAMAGE_MULTIPLIER,dir));
    if (get_bit(quest_rules,qr_MAGICCANDLE))
      change_gamedata_magic(game, -(CANDLEDRAINAMOUNT*get_gamedata_magicdrainrate(game)));
    sfx(WAV_FIRE,pan(wx));
    attack=wFire;
    break;

    case iGArrow:
    case iSArrow:
    case iArrow:
    if(Lwpns.idCount(wArrow))
      return false;
    if(get_bit(quest_rules,qr_TRUEARROWS))
    {
      if(get_gamedata_arrows(game)<=0)
        return false;
      change_gamedata_arrows(game, -1);
    }
    else
    {
      if(get_gamedata_drupy(game)+get_gamedata_rupies(game)<=0)
        return false;
      change_gamedata_drupy(game, -1);
    }
    Lwpns.add(new weapon((fix)wx,(fix)wy,wArrow,current_item(itype_arrow,true),(1*DAMAGE_MULTIPLIER)<<current_item(itype_arrow,true),dir));
    ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step*=current_item(itype_bow,true);
    sfx(WAV_ARROW,pan(wx));
    break;

    case iBait:
    if(Lwpns.idCount(wBait))
      return false;
    if(tmpscr->room==rGRUMBLE && !getmapflag())
    {
      items.add(new item((fix)wx,(fix)wy,iBait,ipDUMMY+ipFADE,0));
      fadeclk=66;
      msgstr=0;
      clear_bitmap(msgdisplaybuf);
      set_clip_state(msgdisplaybuf, 1);
      clear_bitmap(pricesdisplaybuf);
      set_clip_state(pricesdisplaybuf, 1);
      //    putscr(scrollbuf,0,0,tmpscr);
      setmapflag();
      game->items[itype_bait]=0;
      selectBwpn(0,0);
      if(!nosecretsounds)
      {
        sfx(WAV_SECRET);
      }
      return false;
    }
    Lwpns.add(new weapon((fix)wx,(fix)wy,wBait,0,0,dir));
    break;

    case iBrang:
    case iMBrang:
    case iFBrang:
    if(Lwpns.idCount(wBrang))
      return false;
    Lwpns.add(new weapon((fix)wx,(fix)wy,wBrang,current_item(itype_brang,true),(current_item(itype_brang,true)*DAMAGE_MULTIPLIER),dir));
    break;

    case iHookshot:
    case iLongshot:
    if(inlikelike || Lwpns.idCount(wHookshot))
      return false;
    if (dir==up)
    {
      if ((combobuf[MAPCOMBO(x,y-7)].type==cHSGRAB)||
          (_walkflag(x+2,y+4,1) && !isstepable(MAPCOMBO(int(x),int(y+4)))&&(combobuf[MAPCOMBO(x,y-7)].type!=cHOOKSHOTONLY)))
      {
        use_hookshot=false;
      }
    }

    if (dir==down)
    {
      if (int(x)&8)
      {
        if ((combobuf[MAPCOMBO(x+16,y+23)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
      }
      else if ((combobuf[MAPCOMBO(x,y+23)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
    }

    if (dir==left)
    {
      if (int(y)&8)
      {
        if ((combobuf[MAPCOMBO(x-7,y+16)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
      }
      else if ((combobuf[MAPCOMBO(x-7,y)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
    }

    if (dir==right)
    {
      if (int(y)&8)
      {
        if ((combobuf[MAPCOMBO(x+23,y+16)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
      }
      else if ((combobuf[MAPCOMBO(x+23,y)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
    }

    if (use_hookshot)
    {
      if (dir==up)
      {
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy,wHSHandle,0,DAMAGE_MULTIPLIER,dir));
        Lwpns.add(new weapon((fix)wx,(fix)wy-4,wHookshot,0,DAMAGE_MULTIPLIER,dir));
        hs_startx=wx; hs_starty=wy-4;
      }
      if (dir==down)
      {
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy,wHSHandle,0,DAMAGE_MULTIPLIER,dir));
        Lwpns.add(new weapon((fix)wx,(fix)wy+4,wHookshot,0,DAMAGE_MULTIPLIER,dir));
        hs_startx=wx; hs_starty=wy+4;
      }
      if (dir==left)
      {
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy,wHSHandle,0,DAMAGE_MULTIPLIER,dir));
        Lwpns.add(new weapon((fix)(wx-4),(fix)wy,wHookshot,0,DAMAGE_MULTIPLIER,dir));
        hs_startx=wx-4; hs_starty=wy;
      }
      if (dir==right)
      {
        hookshot_used=true;
        Lwpns.add(new weapon((fix)wx,(fix)wy,wHSHandle,0,DAMAGE_MULTIPLIER,dir));
        Lwpns.add(new weapon((fix)(wx+4),(fix)wy,wHookshot,0,DAMAGE_MULTIPLIER,dir));
        hs_startx=wx+4; hs_starty=wy;
      }

      hookshot_frozen=true;
    }
    break;
    case iDinsFire:
    if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
       ((get_gamedata_magic(game)+get_gamedata_dmagic(game))<DINSFIREDRAINAMOUNT*get_gamedata_magicdrainrate(game)))
      return false;
    change_gamedata_magic(game, -(DINSFIREDRAINAMOUNT*get_gamedata_magicdrainrate(game)));
    action=casting;
    magictype=mgc_dinsfire;
    break;
    case iFaroresWind:
    if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
       ((get_gamedata_magic(game)+get_gamedata_dmagic(game))<FARORESWINDDRAINAMOUNT*get_gamedata_magicdrainrate(game)))
      return false;
    change_gamedata_magic(game, -(FARORESWINDDRAINAMOUNT*get_gamedata_magicdrainrate(game)));
    action=casting;
    magictype=mgc_faroreswind;
    break;
    case iNayrusLove:
    if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
       ((get_gamedata_magic(game)+get_gamedata_dmagic(game))<NAYRUSLOVEDRAINAMOUNT*get_gamedata_magicdrainrate(game)))
      return false;
    change_gamedata_magic(game, -(NAYRUSLOVEDRAINAMOUNT*get_gamedata_magicdrainrate(game)));
    action=casting;
    magictype=mgc_nayruslove;
    break;
	case iCByrna:
	if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
	  ((get_gamedata_magic(game)+get_gamedata_dmagic(game))<BYRNADRAINAMOUNT*get_gamedata_magicdrainrate(game)))
	  return false;
	change_gamedata_magic(game, -(BYRNADRAINAMOUNT*get_gamedata_magicdrainrate(game)));
	Lwpns.add(new weapon((fix)wx,(fix)wy,wCByrna,current_item(itype_cbyrna,true),(current_item(itype_cbyrna,true)*DAMAGE_MULTIPLIER),dir));
	break;

    default:
    return false;
  }

  return true;
}

bool LinkClass::doattack()
{
  int s = BSZ ? 0 : 11;

  if ((attack!=wHammer) && (attackclk>=14))
    return false;

  if (attackclk>29)
    return false;

  if(attack==wCatching && attackclk>4)
  {
    if(DrunkUp()||DrunkDown()||DrunkLeft()||DrunkRight())
    {
      lstep = s;
      return false;
    }
  }
  else if(attack!=wWand && attack!=wSword
          && attack!=wHammer && attackclk>7)
    {
      if(DrunkUp()||DrunkDown()||DrunkLeft()||DrunkRight())
      {
        lstep = s;
        return false;
      }
    }
    lstep=0;
  ++attackclk;
  if(attackclk==2 && attack==wBrang)
  {
    if(DrunkUp() && !DrunkLeft() && !DrunkRight() && !DrunkDown())  dir=up;
    if(!DrunkUp() && !DrunkLeft() && !DrunkRight() && DrunkDown())  dir=down;
    if(!DrunkUp() && DrunkLeft() && !DrunkRight() && !DrunkDown())  dir=left;
    if(!DrunkUp() && !DrunkLeft() && DrunkRight() && !DrunkDown())  dir=right;
  }
  if(attackclk==13)
  {
    int templife;
    templife=zinit.beam_hearts[current_item(itype_sword,true)-1];
    if (get_bit(&(zinit.beam_percent),(current_item(itype_sword,true)-1)))
    {
      templife=templife*get_gamedata_maxlife(game);
      templife=templife/100;
    }
    else
    {
      templife*=HP_PER_HEART;
    }

	if(attack==wSword && get_gamedata_life(game)+(get_bit(quest_rules,qr_QUARTERHEART)?((HP_PER_HEART/4)-1):((HP_PER_HEART/2)-1))>=templife)
      startwpn(iSword);
    if(attack==wWand)
      startwpn(iWand);
  }
  if(attackclk==14)
    lstep = s;
  return true;
}

bool LinkClass::can_attack()
{
  if(action==hopping || action==swimming || action==freeze ||
     (action==attacking && !get_bit(quest_rules,qr_QUICKSWORD)))
  {
    return false;
  }
  int r = (isdungeon()) ? 16 : 0;
  switch(dir)
  {
    case up:
    case down:  return !( y<(8+r) || y>(152-r) );
    case left:
    case right: return !( x<(8+r) || x>(232-r) );
  }
  return true;
}

bool isRaftFlag(int flag)
{
  return (flag==mfRAFT || flag==mfRAFT_BRANCH);
}

void do_lens()
{
  //  if(cBbtn() && lensclk==0 && (lenscnt > 0 || game->drupy+game->rupies > 0))
  //  ((get_bit(quest_rules,qr_ENABLEMAGIC))? (game->drupy+game->rupies > 0) : (game->dmagic+game->magic > 0))
  if(DrunkcBbtn() && lensclk==0 && (lenscnt > 0 ||
                                    ((get_bit(quest_rules,qr_ENABLEMAGIC)) ?
                                     (get_gamedata_dmagic(game)+get_gamedata_magic(game) > 0) :
                                     (get_gamedata_drupy(game)+get_gamedata_rupies(game) > 0))))
  {
    if(lenscnt)
      --lenscnt;
    else
    {
      if (get_bit(quest_rules,qr_ENABLEMAGIC))
      {
        if (!(magicdrainclk%LENSDRAINSPEED))
        {
          change_gamedata_magic(game, -LENSDRAINAMOUNT*get_gamedata_magicdrainrate(game));
        }
      }
      else
      {
        change_gamedata_drupy(game, -LENSDRAINAMOUNT/2);
      }
      lenscnt = 4;
    }
    lensclk = 3;
  }
}

void LinkClass::do_hopping()
{
  if(Bwpn==iLens&&itemclk==0)
    do_lens();

  if(hopclk == 0xFF)                                        // swimming
  {
    if(diveclk>0)
      --diveclk;
    else if(DrunkrAbtn())
        diveclk=80;

      if((!(int(x)&7) && !(int(y)&7)) || get_bit(quest_rules,qr_LTTPWALK))
    {
      action = swimming;
      hopclk = 0;
    }
    else
    {
      linkstep();
      if(diveclk<=30 || (frame&1))
      {
        switch(dir)
        {
          case up:    y -= 1; break;
          case down:  y += 1; break;
          case left:  x -= 1; break;
          case right: x += 1; break;
        }
      }
    }
  }
  else                                                      // hopping in or out (need to separate the cases...)
  {
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
      if(hopclk==1) //hopping out
      {
        int lc = get_bit(quest_rules, qr_LTTPCOLLISION);
        landswim=0;
        if(dir==up)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(iswater(MAPCOMBO(x,y+(lc?0:8)-1)) && !iswater(MAPCOMBO(x+8,y+(lc?0:8)-1)) && !iswater(MAPCOMBO(x+15,y+(lc?0:8)-1)))
            sidestep=1;
          else if(!iswater(MAPCOMBO(x,y+(lc?0:8)-1)) && !iswater(MAPCOMBO(x+7,y+(lc?0:8)-1)) && iswater(MAPCOMBO(x+15,y+(lc?0:8)-1)))
              sidestep=2;
            if(sidestep==1) x++;
          else if(sidestep==2) x--;
            else y--;
          if(!iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&!iswater(MAPCOMBO(int(x),int(y)+15)))
          {
            hopclk=0; diveclk=0; action=none;
          }
        }
        if(dir==down)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(iswater(MAPCOMBO(x,y+16)) && !iswater(MAPCOMBO(x+8,y+16)) && !iswater(MAPCOMBO(x+15,y+16)))
            sidestep=1;
          else if(!iswater(MAPCOMBO(x,y+16)) && !iswater(MAPCOMBO(x+7,y+16)) && iswater(MAPCOMBO(x+15,y+16)))
              sidestep=2;
            if(sidestep==1) x++;
          else if(sidestep==2) x--;
            else y++;
          if(!iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&!iswater(MAPCOMBO(int(x),int(y)+15)))
          {
            hopclk=0; diveclk=0; action=none;
          }
        }
        if(dir==left)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(iswater(MAPCOMBO(x-1,y+(lc?0:8))) && !iswater(MAPCOMBO(x-1,y+(lc?8:12))) && !iswater(MAPCOMBO(x-1,y+15)))
            sidestep=1;
          else if(!iswater(MAPCOMBO(x-1,y+(lc?0:8))) && !iswater(MAPCOMBO(x-1,y+(lc?7:11))) && iswater(MAPCOMBO(x-1,y+15)))
              sidestep=2;
            if(sidestep==1) y++;
          else if(sidestep==2) y--;
            else x--;
          if(!iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&!iswater(MAPCOMBO(int(x)+15,int(y)+8)))
          {
            hopclk=0; diveclk=0; action=none;
          }
        }
        if(dir==right)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(iswater(MAPCOMBO(x+16,y+(lc?0:8))) && !iswater(MAPCOMBO(x+16,y+(lc?8:12))) && !iswater(MAPCOMBO(x+16,y+15)))
            sidestep=1;
          else if(!iswater(MAPCOMBO(x+16,y+(lc?0:8))) && !iswater(MAPCOMBO(x+16,y+(lc?7:11))) && iswater(MAPCOMBO(x+16,y+15)))
              sidestep=2;
            if(sidestep==1) y++;
          else if(sidestep==2) y--;
            else x++;
          if(!iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&!iswater(MAPCOMBO(int(x)+15,int(y)+8)))
          {
            hopclk=0; diveclk=0; action=none;
          }
        }
      }

      if(hopclk==2) //hopping in
      {
        int lc=get_bit(quest_rules,qr_LTTPCOLLISION);
        landswim=0;
        if(dir==up)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(!iswater(MAPCOMBO(x,y+(lc?0:8)-1)) && iswater(MAPCOMBO(x+8,y+(lc?0:8)-1)) && iswater(MAPCOMBO(x+15,y+(lc?0:8)-1)))
            sidestep=1;
          else if(iswater(MAPCOMBO(x,y+(lc?0:8)-1)) && iswater(MAPCOMBO(x+7,y+(lc?0:8)-1)) && !iswater(MAPCOMBO(x+15,y+(lc?0:8)-1)))
              sidestep=2;
            if(sidestep==1) x++;
          else if(sidestep==2) x--;
            else y--;
          if(iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&iswater(MAPCOMBO(int(x),int(y)+15)))
          {
            hopclk=0xFF; diveclk=0; action=swimming;
          }
        }
        if(dir==down)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(!iswater(MAPCOMBO(x,y+16)) && iswater(MAPCOMBO(x+8,y+16)) && iswater(MAPCOMBO(x+15,y+16)))
            sidestep=1;
          else if(iswater(MAPCOMBO(x,y+16)) && iswater(MAPCOMBO(x+7,y+16)) && !iswater(MAPCOMBO(x+15,y+16)))
              sidestep=2;
            if(sidestep==1) x++;
          else if(sidestep==2) x--;
            else y++;
          if(iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&iswater(MAPCOMBO(int(x),int(y)+15)))
          {
            hopclk=0xFF; diveclk=0; action=swimming;
          }
        }
        if(dir==left)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(!iswater(MAPCOMBO(x-1,y+(lc?0:8))) && iswater(MAPCOMBO(x-1,y+(lc?8:12))) && iswater(MAPCOMBO(x-1,y+15)))
            sidestep=1;
          else if(iswater(MAPCOMBO(x-1,y+(lc?0:8))) && iswater(MAPCOMBO(x-1,y+(lc?7:11))) && !iswater(MAPCOMBO(x-1,y+15)))
              sidestep=2;
            if(sidestep==1) y++;
          else if(sidestep==2) y--;
            else x--;
          if(iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&iswater(MAPCOMBO(int(x)+15,int(y)+8)))
          {
            hopclk=0xFF; diveclk=0; action=swimming;
          }
        }
        if(dir==right)
        {
          linkstep();
          linkstep();
          int sidestep=0;
          if(!iswater(MAPCOMBO(x+16,y+(lc?0:8))) && iswater(MAPCOMBO(x+16,y+(lc?8:12))) && iswater(MAPCOMBO(x+16,y+15)))
            sidestep=1;
          else if(iswater(MAPCOMBO(x+16,y+(lc?0:8))) && iswater(MAPCOMBO(x+16,y+(lc?7:11))) && !iswater(MAPCOMBO(x+16,y+15)))
              sidestep=2;
            if(sidestep==1) y++;
          else if(sidestep==2) y--;
            else x++;
          if(iswater(MAPCOMBO(int(x),int(y)+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)))&&iswater(MAPCOMBO(int(x)+15,int(y)+8)))
          {
            hopclk=0xFF; diveclk=0; action=swimming;
          }
        }
      }

    }
    else
    {
      //if(dir<left ? get_bit(quest_rules,qr_LTTPWALK)?1:!(int(x)&7) && !(int(y)&15) : !(int(x)&15) && get_bit(quest_rules,qr_LTTPWALK)?1:!(int(y)&7))
      if(dir<left ? !(int(x)&7) && !(int(y)&15) : !(int(x)&15) && !(int(y)&7))
      {
        action = none;
        hopclk = 0;
        diveclk = 0;
        if(iswater(MAPCOMBO(int(x),int(y)+8)))
        {
          // hopped in
          action = swimming;
        }
      }
      else
      {
        linkstep();
        linkstep();
        if(++link_count>16)
          link_count=0;
        int xofs = int(x)&15;
        int yofs = int(y)&15;
        int s = 1 + (frame&1);
        switch(dir)
        {
          case up:    if(yofs<3 || yofs>13) --y; else y-=s; break;
          case down:  if(yofs<3 || yofs>13) ++y; else y+=s; break;
          case left:  if(xofs<3 || xofs>13) --x; else x-=s; break;
          case right: if(xofs<3 || xofs>13) ++x; else x+=s; break;
        }
      }
    }
  }
}

void LinkClass::do_rafting()
{

  if(Bwpn==iLens&&itemclk==0)
    do_lens();

  linkstep();
  if(!(int(x)&15) && !(int(y)&15))
  {
    if((MAPFLAG(x,y)==mfRAFT_BRANCH||MAPCOMBOFLAG(x,y)==mfRAFT_BRANCH))
    {
      if(dir!=down && DrunkUp() && (isRaftFlag(nextflag(x,y,up,false))||isRaftFlag(nextflag(x,y,up,true))))
      {
        dir = up;
        goto skip;
      }
      if(dir!=up && DrunkDown() && (isRaftFlag(nextflag(x,y,down,false))||isRaftFlag(nextflag(x,y,down,true))))
      {
        dir = down;
        goto skip;
      }
      if(dir!=right && DrunkLeft() && (isRaftFlag(nextflag(x,y,left,false))||isRaftFlag(nextflag(x,y,left,true))))
      {
        dir = left;
        goto skip;
      }
      if(dir!=left && DrunkRight() && (isRaftFlag(nextflag(x,y,right,false))||isRaftFlag(nextflag(x,y,right,true))))
      {
        dir = right;
        goto skip;
      }
    }

    if(!isRaftFlag(nextflag(x,y,dir,false))&&!isRaftFlag(nextflag(x,y,dir,true)))
    {
      if(dir<left)
      {
        if((isRaftFlag(nextflag(x,y,right,false))||isRaftFlag(nextflag(x,y,right,true))))
          dir=right;
        else if((isRaftFlag(nextflag(x,y,left,false))||isRaftFlag(nextflag(x,y,left,true))))
            dir=left;
          else if(y>0 && y<160)
              action=none;
      }
      else
      {
        if((isRaftFlag(nextflag(x,y,down,false))||isRaftFlag(nextflag(x,y,down,true))))
          dir=down;
        else if((isRaftFlag(nextflag(x,y,up,false))||isRaftFlag(nextflag(x,y,up,true))))
            dir=up;
          else if(x>0 && x<240)
              action=none;
      }
    }
  }

skip:

  switch(dir)
  {
    case up:
    if(int(x)&15)
    {
      if(int(x)&8)
        x++;
      else x--;
    }
    else	--y;
    break;
    case down:
    if(int(x)&15)
    {
      if(int(x)&8)
        x++;
      else x--;
    }
    else ++y;
    break;
    case left:
    if(int(y)&15)
    {
      if(int(y)&8)
        y++;
      else y--;
    }
    else --x;
    break;
    case right:
    if(int(y)&15)
    {
      if(int(y)&8)
        y++;
      else y--;
    }
    else ++x;
    break;
  }
}

void LinkClass::movelink()
{
  int xoff=int(x)&7;
  int yoff=int(y)&7;
  int push=pushing;
  pushing=0;

  if(diveclk>0)
  {
    --diveclk;
  }
  else if(action==swimming)
    {
      bool global_diving=(get_bit(quest_rules,qr_NODIVING) != 0);
      bool screen_diving=(tmpscr->flags5&fTOGGLEDIVING) != 0;
      if(DrunkrAbtn()&&(global_diving==screen_diving))
      {
        diveclk=80;
      }
    }

    if(action==rafting)
  {
    do_rafting();
    if(action==rafting)
    {
      return;
    }
  }

  if(can_attack() && current_item(itype_sword,true)>0 && swordclk==0 && DrunkrAbtn())
  {
    action=attacking;
    attack=wSword;
    attackclk=0;
    sfx(WAV_SWORD,pan(int(x)));
  }
  int wx=x;
  int wy=y;

  switch(dir)
  {
    case up:    wy-=16; break;
    case down:  wy+=16; break;
    case left:  wx-=16; break;
    case right: wx+=16; break;
  }

  if(Bwpn==iLens&&itemclk==0)
  {
    do_lens();
    /* flamethrower
      } else if((Bwpn==iBCandle)||(Bwpn==iRCandle)) {
      if (!(frame%4)) {

      if(can_attack() && itemclk==0 && DrunkcBbtn()) {
      if(!((get_bit(quest_rules,qr_ENABLEMAGIC))&&
      ((game->magic+game->dmagic)<CANDLEDRAINAMOUNT*game->magicdrainrate)&&
      ((get_bit(quest_rules,qr_MAGICCANDLE))))) {
      didstuff|=did_candle;
      Lwpns.add(new weapon((fix)wx,(fix)wy,wFire,4,4*DAMAGE_MULTIPLIER,0));
      if (!(frame%24)&&(get_bit(quest_rules,qr_MAGICCANDLE))) {
      game->magic-=(CANDLEDRAINAMOUNT*game->magicdrainrate);
      }
      sfx(WAV_FIRE,pan(wx));
      //         action=attacking;
      attackclk=0;
      attack=wFire;

      int i=Lwpns.Count()-1;
      weapon *lw = (weapon*)(Lwpns.spr(i));
      if (wpnsbuf[wFIRE].frames>1) {
      lw->aframe=rand()%wpnsbuf[wFIRE].frames;
      } else {
      lw->flip=rand()%2;
      }
      lw->angular=true;
      if     (dir==up)           lw->angle=-PI/2;
      else if(dir==down)         lw->angle=PI/2;
      else if(dir==left)         lw->angle=PI;
      else if(dir==right)        lw->angle=0;

      lw->angle+=((double)(rand()%64)/64)-.325;

      if(lw->angle==-PI || lw->angle==PI) lw->dir=left;
      else if(lw->angle==-PI/2) lw->dir=up;
      else if(lw->angle==PI/2)  lw->dir=down;
      else if(lw->angle==0)     lw->dir=right;
      else if(lw->angle<-PI/2)  lw->dir=l_up;
      else if(lw->angle<0)      lw->dir=r_up;
      else if(lw->angle>PI/2)   lw->dir=l_down;
      else                      lw->dir=r_down;

      lw->step=2;
      if (Bwpn==iRCandle) {
      lw->step=3;
      }
      lw->clk=rand()%32;

      for(int j=Lwpns.Count()-1; j>0; j--) {
      Lwpns.swap(j,j-1);
      }
      }
      }
      }
      */
  }
  else if(can_attack() && itemclk==0 && DrunkrBbtn())
    {
      if(Bwpn==iWand)
      {
        action=attacking;
        attack=wWand;
        attackclk=0;
      }
      //    else if(Bwpn==iHammer)
      else if((Bwpn==iHammer)&&!(action==attacking && attack==wHammer))
        {
          action=attacking;
          attack=wHammer;
          attackclk=0;
        }
        else if(startwpn(Bwpn))
          {
            if (action==casting)
            {
              return;
            }
            else
            {
              action=attacking;
              attackclk=0;
              attack=none;
              if(Bwpn==iBrang || Bwpn==iMBrang || Bwpn==iFBrang)
              {
                attack=wBrang;
              }
            }
          }
    }

    if(attackclk || action==attacking)
  {
    if(!inlikelike && attackclk>4 && (attackclk&3)==0)
    {
      if(xoff==0)
      {
        if(DrunkUp()) dir=up;
        if(DrunkDown()) dir=down;
      }
      if(yoff==0)
      {
        if(DrunkLeft()) dir=left;
        if(DrunkRight()) dir=right;
      }
    }
    if(doattack())
      return;
    action=none;
    attackclk=0;
  }

  if(action==walking)
  {
    if(!DrunkUp() && !DrunkDown() && !DrunkLeft() && !DrunkRight() && !autostep)
    {
      action=none;
      link_count=-1;
      return;
    }
    autostep=false;

    if(!(get_bit(quest_rules, qr_LTTPWALK)))
    {
      //if (xoff||yoff)
      //{

      /*if(get_bit(quest_rules, qr_LTTPWALK))
        {
        if(dir==up)
        {
        if(int(x)&7)
        {
        if(!walkflag(x, y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1, 2, up))
        {
        if(!walkflag(x+8, y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1, 2, up))
        {
        if(walkflag(x+15, y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1, 2, up))
        {
        if(shift==1)*/


      if(dir==up&&yoff)
      {
        if(int(x)&8)
        {
          if(!walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-int(lsteps[int(y)&7]),2,up))
          {
            move(up);
          }
          else
          {
            action=none;
          }
        }
        else if(!walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-int(lsteps[int(y)&7]),2,up))
          {
            move(up);
          }
          else
          {
            action=none;
          }
        return;
      }
      if(dir==down&&yoff)
      {
        if(int(x)&8)
        {
          if(!walkflag(x,y+15+int(lsteps[int(y)&7]),2,down))
          {
            move(down);
          }
          else
          {
            action=none;
          }
        }
        else if(!walkflag(x,y+15+int(lsteps[int(y)&7]),2,down))
          {
            move(down);
          }
          else
          {
            action=none;
          }
        return;
      }
      if(dir==left&&xoff)
      {
        if(!walkflag(x-int(lsteps[int(x)&7]),y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,left)&&!walkflag(x-int(lsteps[int(x)&7]),y+8,1,left))
        {
          move(left);
        }
        else
        {
          action=none;
        }
        return;
      }
      if(dir==right&&xoff)
      {
        if(!walkflag(x+15+int(lsteps[int(x)&7]),y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,right)&&!walkflag(x+15+int(lsteps[int(x)&7]),y+8,1,right))
        {
          move(right);
        }
        else
        {
          action=none;
        }
        return;
      }
    }
    //return;
    //}
  }

  if((action!=swimming)&&(action!=casting))
  {
    action=none;
  }

  if(get_bit(quest_rules, qr_LTTPWALK))
  {
    switch(holddir)
    {
      case up:
      if(!Up()) holddir=-1; break;
      case down:
      if(!Down()) holddir=-1; break;
      case left:
      if(!Left()) holddir=-1; break;
      case right:
      if(!Right()) holddir=-1; break;
      default:
      break;
    }

    if(DrunkUp()&&(holddir==-1||holddir==up))
    {

      if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM))
      {

      }
      else
      {

        dir=up;
        holddir=up;


        if(DrunkRight()&&shiftdir!=left)
        {
          shiftdir=right;
        }
        else if(DrunkLeft()&&shiftdir!=right)
          {
            shiftdir=left;
          }
          else shiftdir=-1;

        //bool walkable;
        if(walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-z3step,2,up)||(int(x)&7?walkflag(x+16,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-z3step,1,up):0)) {
          if(z3step==2) {
            z3step=1;
            if(walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-z3step,2,up)||(int(x)&7?walkflag(x+16,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-z3step,1,up):0))
              walkable = false;
            else walkable=true;
          }
          else walkable=false;
        }
        else walkable = true;

        int s=shiftdir;
        if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM))
        {
          shiftdir=-1;
        }
        else
        {
          if(s==left) {
            if(walkflag(x-1,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,left)||walkflag(x-1,y+15,1,left))
              shiftdir=-1;
            else if(walkable)
                if(walkflag(x-1,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,left))
                  shiftdir=-1;
          }
          else if(s==right) {
              if(walkflag(x+16,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,right)||walkflag(x+16,y+15,1,right))
                shiftdir=-1;
              else if(walkable)
                  if(walkflag(x+16,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,right))
                    shiftdir=-1;
            }
        }
        move(up);
        shiftdir=s;

        if(!walkable)
        {
          if(shiftdir==-1)
          {
            if(!walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up)&&
               !walkflag(x+8,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up)&&
               walkflag(x+15,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up))
            {
              sprite::move((fix)-1,(fix)0);
            }
            else if(walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up)&&
                    !walkflag(x+7,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up)&&
                    !walkflag(x+15,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up))
              {
                sprite::move((fix)1,(fix)0);
              }
              else if(shiftdir==-1)
                {
                  pushing=push+1;
                  if(action!=swimming)
                  {
                    //linkstep();
                    //if(++link_count > 16)
                    //link_count=0;
                  }
                }
                z3step=2;
          }
          else
          {
            if(action!=swimming)
            {
              //linkstep();
              //if(++link_count > 16)
              //link_count=0;
            }
            z3step=2;
          }
        }
        return;
      }
    }

    if(DrunkDown()&&(holddir==-1||holddir==down))
    {

      if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM))
      {
      }
      else
      {

        dir=down;
        holddir=down;


        if(DrunkRight()&&shiftdir!=left)
        {
          shiftdir=right;
        }
        else if(DrunkLeft()&&shiftdir!=right)
          {
            shiftdir=left;
          }
          else shiftdir=-1;

        //bool walkable;
        if(walkflag(x,y+15+z3step,2,down)||(int(x)&7?walkflag(x+16,y+15+z3step,1,down):0)) {
          if(z3step==2) {
            z3step=1;
            if(walkflag(x,y+15+z3step,2,down)||(int(x)&7?walkflag(x+16,y+15+z3step,1,down):0))
              walkable = false;
            else walkable=true;
          }
          else walkable=false;
        }
        else walkable = true;

        int s=shiftdir;
        if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM))
        {
          shiftdir=-1;
        }
        else
        {
          if(s==left) {
            if(walkflag(x-1,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,left)||walkflag(x-1,y+15,1,left))
              shiftdir=-1;
            else if(walkable)
                if(walkflag(x-1,y+16,1,left))
                  shiftdir=-1;
          }
          else if(s==right) {
              if(walkflag(x+16,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,right)||walkflag(x+16,y+15,1,right))
                shiftdir=-1;
              else if(walkable)
                  if(walkflag(x+16,y+16,1,right))
                    shiftdir=-1;
            }
        }
        move(down);
        shiftdir=s;

        if(!walkable)
        {
          if(shiftdir==-1)
          {
            if(!walkflag(x,y+15+1,1,down)&&
               !walkflag(x+8,y+15+1,1,down)&&
               walkflag(x+15,y+15+1,1,down))
            {
              sprite::move((fix)-1,(fix)0);
            }
            else if(walkflag(x,y+15+1,1,down)&&
                    !walkflag(x+7,y+15+1,1,down)&&
                    !walkflag(x+15,y+15+1,1,down))
              {
                sprite::move((fix)1,(fix)0);
              }
              else if(shiftdir==-1)
                {
                  pushing=push+1;
                  if(action!=swimming)
                  {
                    //linkstep();
                    //if(++link_count > 16)
                    //link_count=0;
                  }
                }
                z3step=2;
          }
          else
          {
            if(action!=swimming)
            {
              //linkstep();
              //if(++link_count > 16)
              //link_count=0;
            }
            z3step=2;
          }
        }
        return;
      }
    }

    if(DrunkLeft()&&(holddir==-1||holddir==left))
    {

      if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM))
      {
      }
      else
      {
        dir=left;
        holddir=left;


        if(DrunkUp()&&shiftdir!=down)
        {
          shiftdir=up;
        }
        else if(DrunkDown()&&shiftdir!=up)
          {
            shiftdir=down;
          }
          else shiftdir=-1;

        //bool walkable;
        if(walkflag(x-z3step,y+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8),1,left)||walkflag(x-z3step,y+8,1,left)||(int(y)&7?walkflag(x-z3step,y+16,1,left):0)) {
          if(z3step==2) {
            z3step=1;
            if(walkflag(x-z3step,y+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8),1,left)||walkflag(x-z3step,y+8,1,left)||(int(y)&7?walkflag(x-z3step,y+16,1,left):0))
              walkable = false;
            else walkable=true;
          }
          else walkable=false;
        }
        else walkable = true;

        int s=shiftdir;
        if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM))
        {
          shiftdir=-1;
        }
        else
        {
          if(s==up) {
            if(walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,2,up)||walkflag(x+15,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up))
              shiftdir=-1;
            else if(walkable)
                if(walkflag(x-1,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up))
                  shiftdir=-1;
          }
          else if(s==down) {
              if(walkflag(x,y+16,2,down)||walkflag(x+15,y+16,1,down))
                shiftdir=-1;
              else if(walkable)
                  if(walkflag(x-1,y+16,1,down))
                    shiftdir=-1;
            }
        }
        move(left);
        shiftdir=s;

        if(!walkable)
        {
          if(shiftdir==-1) {
            int v1=get_bit(quest_rules,qr_LTTPCOLLISION)?0:8;
            int v2=get_bit(quest_rules,qr_LTTPCOLLISION)?8:12;
            if(!walkflag(x-1,y+v1,1,left)&&
               !walkflag(x-1,y+v2,1,left)&&
               walkflag(x-1,y+15,1,left))
            {
              sprite::move((fix)0,(fix)-1);
            }
            else if(walkflag(x-1,y+v1,1,left)&&
                    !walkflag(x-1,y+v2-1,1,left)&&
                    !walkflag(x-1,y+15,1,left))
              {
                sprite::move((fix)0,(fix)1);
              }
              else if(shiftdir==-1)
                {
                  pushing=push+1;
                  if(action!=swimming)
                  {
                    //linkstep();
                    //if(++link_count > 16)
                    //link_count=0;
                  }
                }
                z3step=2;
          }
          else
          {
            if(action!=swimming)
            {
              //linkstep();
              //if(++link_count > 16)
              //link_count=0;
            }
            z3step=2;
          }
        }
        return;
      }
    }

    if(DrunkRight()&&(holddir==-1||holddir==right))
    {

      if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM))
      {
      }
      else
      {

        dir=right;
        holddir=right;


        if(DrunkUp()&&shiftdir!=down)
        {
          shiftdir=up;
        }
        else if(DrunkDown()&&shiftdir!=up)
          {
            shiftdir=down;
          }
          else shiftdir=-1;

        //bool walkable;
        if(walkflag(x+15+z3step,y+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8),1,right)||walkflag(x+15+z3step,y+8,1,right)||(int(y)&7?walkflag(x+15+z3step,y+16,1,right):0)) {
          if(z3step==2) {
            z3step=1;
            if(walkflag(x+15+z3step,y+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8),1,right)||walkflag(x+15+z3step,y+8,1,right)||(int(y)&7?walkflag(x+15+z3step,y+16,1,right):0))
              walkable = false;
            else walkable=true;
          }
          else walkable=false;
        }
        else walkable = true;

        int s=shiftdir;
        if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM))
        {
          shiftdir=-1;
        }
        else
        {
          if(s==up) {
            if(walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,2,up)||walkflag(x+15,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up))
              shiftdir=-1;
            else if(walkable)
                if(walkflag(x+16,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-1,1,up))
                  shiftdir=-1;
          }
          else if(s==down) {
              if(walkflag(x,y+16,2,down)||walkflag(x+15,y+16,1,down))
                shiftdir=-1;
              else if(walkable)
                  if(walkflag(x+16,y+16,1,down))
                    shiftdir=-1;
            }
        }
        move(right);
        shiftdir=s;

        if(!walkable)
        {
          if(shiftdir==-1)
          {
            int v1=get_bit(quest_rules,qr_LTTPCOLLISION)?0:8;
            int v2=get_bit(quest_rules,qr_LTTPCOLLISION)?8:12;
            if(!walkflag(x+16,y+v1,1,right)&&
               !walkflag(x+16,y+v2,1,right)&&
               walkflag(x+16,y+15,1,right))
            {
              sprite::move((fix)0,(fix)-1);
            }
            else if(walkflag(x+16,y+v1,1,right)&&
                    !walkflag(x+16,y+v2-1,1,right)&&
                    !walkflag(x+16,y+15,1,right))
              {
                sprite::move((fix)0,(fix)1);
              }
              else //if(shiftdir==-1)
              {
                pushing=push+1; z3step=2;
                if(action!=swimming)
                {
                  //linkstep();
                  //if(++link_count > 16)
                  //link_count=0;
                }
              }
            z3step=2;
          }
          else
          {
            if(action!=swimming)
            {
              //linkstep();
              //if(++link_count > 16)
              //link_count=0;
            }
            z3step=2;
          }
        }
        return;
      }
    }
    bool wtry  = iswater(MAPCOMBO(x,y+15));
    bool wtry8 = iswater(MAPCOMBO(x+15,y+15));
    bool wtrx = iswater(MAPCOMBO(x,y+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)));
    bool wtrx8 = iswater(MAPCOMBO(x+15,y+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8)));
    if(current_item(itype_flippers,true)&&!(ladderx+laddery))
    {
      if(wtrx&&wtrx8&&wtry&&wtry8) {
        //action=swimming;
        hopclk = 0xFF;
      }
    }

    return;
  }


  if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM))
  {
    goto LEFTRIGHT;
  }

  // make it easier to get in left & right doors
  if(isdungeon() && DrunkLeft() && x==32 && y==80 && !walkflag(x-int(lsteps[int(x)&7]),y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,left) && !walkflag(x-int(lsteps[int(x)&7]),y+8,1,left))
  {
    move(left);
    return;
  }
  if(isdungeon() && DrunkRight() && x==208 && y==80 && !walkflag(x+15+int(lsteps[int(x)&7]),y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,right) && !walkflag(x+15+int(lsteps[int(x)&7]),y+8,1,right))
  {
    move(right);
    return;
  }

  if(DrunkUp())
  {
    if(xoff)
    {
      if(dir!=up&&dir!=down)
      {
        if(xoff>2&&xoff<6)
        {
          move(dir);
        }
        else if(xoff>=6)
          {
            move(right);
          }
          else if(xoff>=1)
            {
              move(left);
            }
      }
      else
      {
        if(xoff>=4)
        {
          move(right);
        }
        else if(xoff<4)
          {
            move(left);
          }
      }
    }
    else
    {
      if((isdungeon())||(int(x)&8))
      {
        if(!walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-int(lsteps[int(y)&7]),2,up))
        {
          move(up);
          return;
        }
      }
      else
      {
        if(!walkflag(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8)-int(lsteps[int(y)&7]),2,up))
        {
          move(up);
          return;
        }
      }

      if( !DrunkLeft() && !DrunkRight() )
      {
        pushing=push+1;
        dir=up;
        if(action!=swimming)
        {
          linkstep();
        }
        return;
      }
      else
      {
        goto LEFTRIGHT;
      }
    }
    return;
  }

  if(DrunkDown())
  {
    if(xoff)
    {
      if(dir!=up&&dir!=down)
      {
        if(xoff>2&&xoff<6)
          move(dir);
        else if(xoff>=6)
            move(right);
          else if(xoff>=1)
              move(left);
      }
      else
      {
        if(xoff>=4)
        {
          move(right);
        }
        else if(xoff<4)
          {
            move(left);
          }
      }
    }
    else
    {
      if((isdungeon())||(int(x)&8))
      {
        if(!walkflag(x,y+15+int(lsteps[int(y)&7]),2,down))
        {
          move(down);
          return;
        }
      }
      else
      {
        if(!walkflag(x,y+15+int(lsteps[int(y)&7]),2,down))
        {
          move(down);
          return;
        }
      }
      if( !DrunkLeft() && !DrunkRight() )
      {
        pushing=push+1;
        dir=down;
        if(action!=swimming)
        {
          linkstep();
        }
        return;
      }
      else goto LEFTRIGHT;
    }
    return;
  }

LEFTRIGHT:

  if(isdungeon() && (y<=26 || y>=134) && !get_bit(quest_rules,qr_FREEFORM))
    return;

  if(DrunkLeft())
  {
    if(yoff)
    {
      if(dir!=left&&dir!=right)
      {
        if(yoff>2&&yoff<6)
        {
          move(dir);
        }
        else if(yoff>=6)
          {
            move(down);
          }
          else if(yoff>=1)
            {
              move(up);
            }
      }
      else
      {
        if(yoff>=4)
        {
          move(down);
        }
        else if(yoff<4)
          {
            move(up);
          }
      }
    }
    else
    {
      if(!walkflag(x-int(lsteps[int(x)&7]),y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,left) && !walkflag(x-int(lsteps[int(x)&7]),y+8,1,left))
      {
        move(left);
      }
      else if( !DrunkUp() && !DrunkDown() )
        {
          pushing=push+1;
          dir=left;
          if(action!=swimming)
          {
            linkstep();
          }
          return;
        }
    }
    return;
  }

  if(DrunkRight())
  {
    if(yoff)
    {
      if(dir!=left&&dir!=right)
      {
        if(yoff>2&&yoff<6)
        {
          move(dir);
        }
        else if(yoff>=6)
          {
            move(down);
          }
          else if(yoff>=1)
            {
              move(up);
            }
      }
      else
      {
        if(yoff>=4)
        {
          move(down);
        }
        else if(yoff<4)
          {
            move(up);
          }
      }
    }
    else
    {
      if(!walkflag((int)x+15+(lsteps[int(x)&7]),y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,right) && !walkflag((int)x+15+(lsteps[int(x)&7]),y+8,1,right))
      {
        move(right);
      }
      else if( !DrunkUp() && !DrunkDown() )
        {
          pushing=push+1;
          dir=right;
          if(action!=swimming)
          {
            linkstep();
          }
          return;
        }
    }
  }

}

void LinkClass::move(int d)
{
  if(inlikelike)
    return;
  int dx=0,dy=0;
  int xstep=lsteps[int(x)&7];
  int ystep=lsteps[int(y)&7];
  int z3skip=0;
  // xstep=ystep=0;
  if(combobuf[MAPCOMBO(x+7,y+8)].type==cWALKSLOW)
  {
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
      skipstep=(skipstep+1)%4;
      if(skipstep) z3skip=1; else z3skip=0;
    }
    else
    {
      if(d<left)
      {
        //if(get_bit(quest_rules, qr_LTTPWALK))
        //{

        if(ystep>1)
        {
          skipstep^=1; ystep=skipstep;
        }
      }
      else
      {
        if(xstep>1)
        {
          skipstep^=1; xstep=skipstep;
        }
      }
    }
  }
  else
  {
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
      skipstep=0;
    }
  }
  if(get_bit(quest_rules,qr_LTTPWALK))
  {
    switch(d)
    {
      case up:
      if(shiftdir==left) {
        if(walkable) {
          dy-=1-z3skip; dx-=1-z3skip; z3step=2; }
        else { dx-=1-z3skip; z3step=2; }
      }
      else if(shiftdir==right) {
          if(walkable) {
            dy-=1-z3skip; dx+=1-z3skip; z3step=2; }
          else {dx+=1-z3skip; z3step=2;}
        }
        else {
          if(walkable) {dy-=z3step-z3skip; z3step=(z3step%2)+1; }
        }
      break;
      case down:
      if(shiftdir==left) {
        if(walkable) {
          dy+=1-z3skip; dx-=1-z3skip; z3step=2; }
        else { dx-=1-z3skip; z3step=2; }
      }
      else if(shiftdir==right) {
          if(walkable) {
            dy+=1-z3skip; dx+=1-z3skip; z3step=2; }
          else {dx+=1-z3skip; z3step=2;}
        }
        else {
          if(walkable) {dy+=z3step-z3skip; z3step=(z3step%2)+1; }
        }
      break;
      case right:
      if(shiftdir==up) {
        if(walkable) {
          dy-=1-z3skip; dx+=1-z3skip; z3step=2; }
        else { dy-=1-z3skip; z3step=2; }
      }
      else if(shiftdir==down) {
          if(walkable) {
            dy+=1-z3skip; dx+=1-z3skip; z3step=2; }
          else {dy+=1-z3skip; z3step=2;}
        }
        else {
          if(walkable) {dx+=z3step-z3skip; z3step=(z3step%2)+1; }
        }
      break;
      case left:
      if(shiftdir==up) {
        if(walkable) {
          dy-=1-z3skip; dx-=1-z3skip; z3step=2; }
        else { dy-=1-z3skip; z3step=2; }
      }
      else if(shiftdir==down) {
          if(walkable) {
            dy+=1-z3skip; dx-=1-z3skip; z3step=2; }
          else {dy+=1-z3skip; z3step=2;}
        }
        else {
          if(walkable) {dx-=z3step-z3skip; z3step=(z3step%2)+1; }
        }
      break;
    }
  }
  else
  {
    switch(d)
    {
      case up:    dy-=ystep; break;
      case down:  dy+=ystep; break;
      case left:  dx-=xstep; break;
      case right: dx+=xstep; break;
    }
  }
  dir=d;
  linkstep();
  if(action!=swimming) {
    action=walking;
    if(++link_count > 16)
      link_count=0;
  }
  sprite::move((fix)dx,(fix)dy);
}

bool LinkClass::walkflag(int wx,int wy,int cnt,byte d)
{
  if(toogam) return false;
  if(blockpath && wy<((get_bit(quest_rules,qr_LTTPCOLLISION))?80:88))
    return true;

  // for(int i=0; i<4; i++)
  if(mblock2.clk && mblock2.hit(wx,wy,d<=down?16:1,1))
    return true;

  if(isdungeon() && currscr<128 && wy<(get_bit(quest_rules,qr_LTTPCOLLISION)?32:40) && ((get_bit(quest_rules,qr_LTTPWALK)?(x<=112||x>=128):x!=120) || _walkflag(120,24,2))
     && !get_bit(quest_rules,qr_FREEFORM) )
    return true;

  bool wf = _walkflag(wx,wy,cnt);
  if(isdungeon() && currscr<128 && !get_bit(quest_rules,qr_FREEFORM))
  {
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
      if(wx>=112&&wx<120&&wy<40&&wy>=32) wf=true;
      if(wx>=136&&wx<144&&wy<40&&wy>=32) wf=true;
    }
  }


  if(action==swimming)
  {
    if(!wf)
    {

      if(landswim>=22||(!get_bit(quest_rules,qr_LTTPWALK)))
      {
        //Check for out of bounds for swimming
        int hop = hopclk;
        // hop out of the water
        hopclk = 1;
        //landswim=0;

        if(get_bit(quest_rules,qr_LTTPWALK)) {
          if(wx<0||wy<0) hopclk=hop;
          else if(wx>248) hopclk=hop;
            else if(wx>240&&cnt==2) hopclk=hop;
              else if(wy>168) hopclk=hop;
                //else {hopclk=0; diveclk=0; action=none;}
                //}
        }
      }
      else
      {
        if(dir==d)
        {
          //int vx=((int)x+4)&0xFFF8;
          //int vy=((int)y+4)&0xFFF8;
          int lc = get_bit(quest_rules,qr_LTTPCOLLISION);
          if(dir==left)
          {
            if(!iswater(MAPCOMBO(x-1,y+(lc?7:11)))&&!iswater(MAPCOMBO(x-1,y+(lc?8:12))))
              ilswim=true;
            //else ilswim=false;
          }
          else if(dir==right)
            {
              if(!iswater(MAPCOMBO(x+16,y+(lc?7:11)))&&!iswater(MAPCOMBO(x+16,y+(lc?8:12))))
                ilswim=true;
              //else ilswim=false;
            }
            else if(dir==up)
              {
                if(!iswater(MAPCOMBO(x+7,y-1))&&!iswater(MAPCOMBO(x+8,y-1)))
                  ilswim=true;
                //else ilswim=false;
              }
              else if(dir==down)
                {
                  if(!iswater(MAPCOMBO(x+7,y+16))&&!iswater(MAPCOMBO(x+8,y+16)))
                    ilswim=true;
                  //else ilswim=false;
                }
        }
        if(wx<0||wy<0);
        else if(wx>248);
          else if(wx>240&&cnt==2);
            else if(wy>168);
              else return true;
      }


    }
    else
    {
      bool wtrx  = iswater(MAPCOMBO(wx,wy));
      bool wtrx8 = iswater(MAPCOMBO(x+8,wy));

      /*if(get_bit(quest_rules,qr_LTTPWALK))
        {
        bool wtry  = iswater(MAPCOMBO(x,y+15));
        bool wtry8 = iswater(MAPCOMBO(x+15,y+15));
        wtrx = iswater(MAPCOMBO(x,y+8));
        wtrx8 = iswater(MAPCOMBO(x+15,y+8));
        if(wtrx&&wtrx8&&wtry&&wtry8) {
        hopclk = 0xFF;
        return false;
        }
        }
        else
        {*/
      if((d>=left && wtrx) || (d<=down && wtrx && wtrx8))
      {
        // swim
        hopclk = 0xFF;
        return false;
      }
      //}
    }
  }
  else if(ladderx+laddery)                                  // ladder is being used
    {
      if(get_bit(quest_rules, qr_LTTPWALK))
      {
        if(ladderdir==up)
        {
          if(wy>=laddery&&wy<(laddery+16))
          {
            bool temp = false;
            if(!(wx>=ladderx&&wx<(ladderx+16)))
              temp = true;
            if(cnt==2) wx+=8;
            if(!(wx>=ladderx&&wx<(ladderx+16)))
              temp=true;
            if(!temp) return false; else return true;
          }
        }
        else
        {
          if(wx>=ladderx&&wx<(ladderx+16))
          {
            if(wy>=(laddery+(get_bit(quest_rules,qr_LTTPCOLLISION)?0:8))&&wy<(laddery+16))
              return false;
            else return true;
          }
        }
      }
      else
      {
        if((d&2)==ladderdir)                                    // same direction
        {
          switch(d)
          {
            case up:
            if(int(y)<=laddery)
            {
              return _walkflag(ladderx,laddery-8,1) ||
                _walkflag(ladderx+8,laddery-8,1);

            }
            // no break
            case down:
            if((wy&0xF0)==laddery)
              return false;
            break;

            default:
            if((wx&0xF0)==ladderx)
              return false;
          }

          if(d<=down)
            return _walkflag(ladderx,wy,1) || _walkflag(ladderx+8,wy,1);
          return _walkflag((wx&0xF8),wy,1) && _walkflag((wx&0xF8)+8,wy,1);
        }
        // different dir
        return true;
      }
    }
    else if(wf)
      {
        // see if it's a good spot for the ladder or for swimming
        bool wtrx  = iswater(MAPCOMBO(wx,wy));
        bool wtrx8 = iswater(MAPCOMBO(x+8,wy));
        bool flgx  = _walkflag(wx,wy,1) && !wtrx;
        bool flgx8 = _walkflag(x+8,wy,1) && !wtrx8;

        // check if he can swim
        if(current_item(itype_flippers,true))
        {
          /*if(get_bit(quest_rules,qr_LTTPWALK))
            {
            bool wtry  = iswater(MAPCOMBO(x,y+15));
            bool wtry8 = iswater(MAPCOMBO(x+15,y+15));
            wtrx = iswater(MAPCOMBO(x,y+8));
            wtrx8 = iswater(MAPCOMBO(x+15,y+8));
            if(wtrx&&wtrx8&&wtry&&wtry8) {
            //action=swimming;
            hopclk = 0xFF;
            return false;
            }

            wtrx  = iswater(MAPCOMBO(wx,wy));
            wtrx8 = iswater(MAPCOMBO(x+8,wy));
            wtry = iswater(MAPCOMBO(wx,wy+(get_bit(quest_rules,qr_LTTPCOLLISION)?8:0)));
            wtry8 = iswater(MAPCOMBO(x+8,y+8));

            if((d>=left && (wtrx?1:!_walkflag(wx,wy,1)) && (wtry?1:!_walkflag(wx,wy+(get_bit(quest_rules, qr_LTTPCOLLISION)?8:0),1))) || (d<=down && (wtrx?1:!_walkflag(wx,wy,1)) && (wtrx8?1:!_walkflag(x+8,wy,1))))
            {
            return false;
            }
            }
            else {*/
          if(landswim>=22)
          {
            hopclk = 2;
            //landswim=0;
            return false;
          }
          else if((d>=left && wtrx) || (d<=down && wtrx && wtrx8))
            {
              if(!get_bit(quest_rules,qr_LTTPWALK))
              { hopclk=2; return false; }
              else if(dir==d) { ilswim=true; ladderx=0; laddery=0;}
            }
            //}
        }

        // check if he can use the ladder
        if(can_use_item(itype_ladder, i_ladder) && (tmpscr->flags&fLADDER || isdungeon()) && !ilswim)
        {
          // add ladder combos
          wtrx  = isstepable(MAPCOMBO(wx,wy));
          wtrx8 = isstepable(MAPCOMBO(x+8,wy));
          flgx  = _walkflag(wx,wy,1) && !wtrx;
          flgx8 = _walkflag(x+8,wy,1) && !wtrx8;

          if(get_bit(quest_rules,qr_LTTPWALK))
          {
            if(d==dir)
            {
              if(d>=left)
              {
                // int temp = ((wy&0xF0)==(int(y+8)&0xF0));
                if(((wy&0xF0)==(int(y+8)&0xF0))&&(isstepable(MAPCOMBO(wx,wy))))
                {
                  ladderdir=left;
                  ladderx = wx&0xF0;
                  laddery = int(y+8)&0xF0;
                  ladderstart=d;
                  if(laddery==int(y))return false; else return true;
                }
              }
              else if(d<=down)
                {
                  if(((wx&0xF0)==(int(x+8)&0xF0))&&(isstepable(MAPCOMBO(wx,wy))))
                  {
                    ladderdir=up;
                    ladderx = int(x+8)&0xF0;
                    laddery = wy&0xF0;
                    ladderstart=d;
                    if(ladderx==int(x))return false; else return true;
                  }
                  if(cnt==2)
                  {
                    int tx = wx+8;
                    if(((tx&0xF0)==(int(x+8)&0xF0))&&(isstepable(MAPCOMBO(tx,wy))))
                    {
                      ladderdir=up;
                      ladderx = int(x+8)&0xF0;
                      laddery = wy&0xF0;
                      ladderstart=d;
                      if(ladderx==int(x))return false; else return true;
                    }
                  }

                }
            }
          }
          else
          {
            if((d>=left && wtrx) || (d<=down && ((wtrx && !flgx8) || (wtrx8 && !flgx))) )
            {
              if( ((int(y)+15) < wy) || ((int(y)+8) > wy) )
                ladderdir = up;
              else
                ladderdir = left;

              if(ladderdir==up)
              {
                ladderx = int(x)&0xF8;
                laddery = wy&0xF0;
              }
              else
              {
                ladderx = wx&0xF0;
                laddery = int(y)&0xF8;
              }
              return false;
            }
          }
        }
      }
      return wf;
}

void LinkClass::checkpushblock()
{
  if(toogam) return;
  if(!get_bit(quest_rules,qr_LTTPWALK))
    if(int(x)&15) return;
    // if(y<16) return;

    int bx = int(x)&0xF0;
  int by = (int(y)&0xF0);
  switch(dir)
  {
    case up:
    if (y<16)
    {
      return;
    }
    if(!((int)y&15)&&y!=0) by-=get_bit(quest_rules, qr_LTTPCOLLISION)*16;
    if((int)x&8) bx+=16;
    break;
    case down:
    if (y>128)
    {
      return;
    }
    else
    {
      by+=16;
      if((int)x&8) bx+=16;
    }
    break;
    case left:
    if (x<32)
    {
      return;
    }
    else
    {
      bx-=16;
      if(int(y)&8)
      {
        by+=16;
      }
    }
    break;
    case right:
    if (x>208)
    {
      return;
    }
    else
    {
      bx+=16;
      if(int(y)&8)
      {
        by+=16;
      }
    }
    break;
  }

  int f = MAPFLAG(bx,by);
  int f2 = MAPCOMBOFLAG(bx,by);
  int t = combobuf[MAPCOMBO(bx,by)].type;

  if( (t==cPUSH_WAIT || t==cPUSH_HW || t==cPUSH_HW2) && (pushing<16 || hasMainGuy()) ) return;
  if( (t==cPUSH_HEAVY || t==cPUSH_HW) && (current_item(itype_bracelet,true)<1) ) return;
  if( (t==cPUSH_HEAVY2 || t==cPUSH_HW2) && (current_item(itype_bracelet,true)<2) ) return;

  if(get_bit(quest_rules,qr_HESITANTPUSHBLOCKS)&&(pushing<4)) return;

  bool doit=false;
  bool changeflag=false;
  bool changecombo=false;

  if (((f==mfPUSHUD || f==mfPUSHUDNS|| f==mfPUSHUDINS) && dir<=down) ||
      ((f==mfPUSHLR || f==mfPUSHLRNS|| f==mfPUSHLRINS) && dir>=left) ||
      ((f==mfPUSHU || f==mfPUSHUNS || f==mfPUSHUINS) && dir==up) ||
      ((f==mfPUSHD || f==mfPUSHDNS || f==mfPUSHDINS) && dir==down) ||
      ((f==mfPUSHL || f==mfPUSHLNS || f==mfPUSHLINS) && dir==left) ||
      ((f==mfPUSHR || f==mfPUSHRNS || f==mfPUSHRINS) && dir==right) ||
      f==mfPUSH4 || f==mfPUSH4NS || f==mfPUSH4INS)
  {
    changeflag=true;
    doit=true;
  }

  if ((((f2==mfPUSHUD || f2==mfPUSHUDNS|| f2==mfPUSHUDINS) && dir<=down) ||
      ((f2==mfPUSHLR || f2==mfPUSHLRNS|| f2==mfPUSHLRINS) && dir>=left) ||
      ((f2==mfPUSHU || f2==mfPUSHUNS || f2==mfPUSHUINS) && dir==up) ||
      ((f2==mfPUSHD || f2==mfPUSHDNS || f2==mfPUSHDINS) && dir==down) ||
      ((f2==mfPUSHL || f2==mfPUSHLNS || f2==mfPUSHLINS) && dir==left) ||
      ((f2==mfPUSHR || f2==mfPUSHRNS || f2==mfPUSHRINS) && dir==right) ||
      f2==mfPUSH4 || f2==mfPUSH4NS || f2==mfPUSH4INS)&&(f!=mfPUSHED))
  {
    changecombo=true;
    doit=true;
  }

  if(get_bit(quest_rules,qr_SOLIDBLK))
  {
    switch(dir)
    {
      case up:    if(_walkflag(bx,by-8,2))    doit=false; break;
      case down:  if(_walkflag(bx,by+24,2))   doit=false; break;
      case left:  if(_walkflag(bx-16,by+8,2)) doit=false; break;
      case right: if(_walkflag(bx+16,by+8,2)) doit=false; break;
    }
  }

  switch(dir)
  {
    case up:    if((MAPFLAG(bx,by-8)==mfNOBLOCKS||MAPCOMBOFLAG(bx,by-8)==mfNOBLOCKS))       doit=false; break;
    case down:  if((MAPFLAG(bx,by+24)==mfNOBLOCKS||MAPCOMBOFLAG(bx,by+24)==mfNOBLOCKS))     doit=false; break;
    case left:  if((MAPFLAG(bx-16,by+8)==mfNOBLOCKS||MAPCOMBOFLAG(bx-16,by+8)==mfNOBLOCKS)) doit=false; break;
    case right: if((MAPFLAG(bx+16,by+8)==mfNOBLOCKS||MAPCOMBOFLAG(bx+16,by+8)==mfNOBLOCKS)) doit=false; break;
  }

  if(doit)
  {
    //   for(int i=0; i<1; i++)
    if (!blockmoving)
    {
      if (changeflag)
      {
        tmpscr->sflag[(by&0xF0)+(bx>>4)]=0;
      }
      //if (changecombo)
      //{
        //++tmpscr->data[(by&0xF0)+(bx>>4)];
      //}
      if(mblock2.clk<=0)
      {
        mblock2.push((fix)bx,(fix)by,dir,f);
        //       break;
      }
    }
  }
}

bool usekey()
{
  if(!can_use_item(itype_magickey, i_magickey))
  {
    if(game->lvlkeys[dlevel]!=0)
	{
	  game->lvlkeys[dlevel]--;
	  return true;
	}
    if(get_gamedata_keys(game)==0)
      return false;
    change_gamedata_keys(game, -1);
  }
  return true;
}

void LinkClass::checklockblock()
{
  if(toogam) return;

  int bx = int(x)&0xF0;
  int bx2 = int(x+8)&0xF0;
  int by = int(y)&0xF0;

  switch(dir)
  {
    case up:
    if(!((int)y&15)&&y!=0) by-=get_bit(quest_rules, qr_LTTPCOLLISION)*16;
    break;
    case down:
    by+=16;
    break;
    case left:
    bx-=16;
    if(int(y)&8)
    {
      by+=16;
    }
    bx2=bx;
    break;
    case right:
    bx+=16;
    if(int(y)&8)
    {
      by+=16;
    }
    bx2=bx;
    break;
  }

  bool found=false;
  if ((combobuf[MAPCOMBO(bx,by)].type==cLOCKBLOCK)||
      (combobuf[MAPCOMBO(bx2,by)].type==cLOCKBLOCK))
  {
    found=true;
  }
  if (!found)
  {
    for (int i=0; i<2; i++)
    {
      if ((combobuf[MAPCOMBO2(i,bx,by)].type==cLOCKBLOCK)||
          (combobuf[MAPCOMBO2(i,bx2,by)].type==cLOCKBLOCK))
      {
        found=true;
        break;
      }
    }
  }
  if(!found || pushing<8)
  {
    return;
  }

  if (!usekey()) return;
  setmapflag(mLOCKBLOCK);
  remove_lockblocks((currscr>=128)?1:0,true);
  sfx(WAV_DOOR);
}

void LinkClass::checkbosslockblock()
{
  if(toogam) return;

  int bx = int(x)&0xF0;
  int bx2 = int(x+8)&0xF0;
  int by = int(y)&0xF0;

  switch(dir)
  {
    case up:
    if(!((int)y&15)&&y!=0) by-=get_bit(quest_rules, qr_LTTPCOLLISION)*16;
    break;
    case down:
    by+=16;
    break;
    case left:
    bx-=16;
    if(int(y)&8)
    {
      by+=16;
    }
    bx2=bx;
    break;
    case right:
    bx+=16;
    if(int(y)&8)
    {
      by+=16;
    }
    bx2=bx;
    break;
  }

  bool found=false;
  if ((combobuf[MAPCOMBO(bx,by)].type==cBOSSLOCKBLOCK)||
      (combobuf[MAPCOMBO(bx2,by)].type==cBOSSLOCKBLOCK))
  {
    found=true;
  }
  if (!found)
  {
    for (int i=0; i<2; i++)
    {
      if ((combobuf[MAPCOMBO2(i,bx,by)].type==cBOSSLOCKBLOCK)||
          (combobuf[MAPCOMBO2(i,bx2,by)].type==cBOSSLOCKBLOCK))
      {
        found=true;
        break;
      }
    }
  }
  if(!found || pushing<8)
  {
    return;
  }

  if(!(game->lvlitems[dlevel]&liBOSSKEY)) return;
  setmapflag(mBOSSLOCKBLOCK);
  remove_bosslockblocks((currscr>=128)?1:0,true);
  sfx(WAV_DOOR);
}

void LinkClass::checkchest()
{
  if(toogam) return;

  int bx = int(x)&0xF0;
  int bx2 = int(x+8)&0xF0;
  int by = int(y)&0xF0;

  switch(dir)
  {
    case up:
    if(!((int)y&15)&&y!=0) by-=get_bit(quest_rules, qr_LTTPCOLLISION)*16;
    break;
    case down:
    case left:
    case right:
    return;
    break;
  }

  bool found=false;
  bool itemflag=false;
  if ((combobuf[MAPCOMBO(bx,by)].type==cCHEST)||
      (combobuf[MAPCOMBO(bx2,by)].type==cCHEST))
  {
    found=true;
  }
  if (!found)
  {
    for (int i=0; i<2; i++)
    {
      if ((combobuf[MAPCOMBO2(i,bx,by)].type==cCHEST)||
          (combobuf[MAPCOMBO2(i,bx2,by)].type==cCHEST))
      {
        found=true;
        break;
      }
    }
  }
  if(!found || pushing<8)
  {
    return;
  }

  setmapflag(mCHEST);
  remove_chests((currscr>=128)?1:0,true);
  itemflag = MAPFLAG(bx,by)==mfARMOS_ITEM;
  itemflag |= MAPFLAG(bx2,by)==mfARMOS_ITEM;
  itemflag |= MAPCOMBOFLAG(bx,by)==mfARMOS_ITEM;
  itemflag |= MAPCOMBOFLAG(bx2,by)==mfARMOS_ITEM;
  if (!itemflag)
  {
    for (int i=0; i<2; i++)
    {
      itemflag |= MAPFLAG2(i,bx,by)==mfARMOS_ITEM;
      itemflag |= MAPFLAG2(i,bx2,by)==mfARMOS_ITEM;
      itemflag |= MAPCOMBOFLAG2(i,bx,by)==mfARMOS_ITEM;
      itemflag |= MAPCOMBOFLAG2(i,bx2,by)==mfARMOS_ITEM;
    }
  }

  if(itemflag && !getmapflag())
  {
    items.add(new item(x, y, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
  }
}

void LinkClass::checklockedchest()
{
  if(toogam) return;

  int bx = int(x)&0xF0;
  int bx2 = int(x+8)&0xF0;
  int by = int(y)&0xF0;

  switch(dir)
  {
    case up:
    if(!((int)y&15)&&y!=0) by-=get_bit(quest_rules, qr_LTTPCOLLISION)*16;
    break;
    case down:
    case left:
    case right:
    return;
    break;
  }

  bool found=false;
  bool itemflag=false;
  if ((combobuf[MAPCOMBO(bx,by)].type==cLOCKEDCHEST)||
      (combobuf[MAPCOMBO(bx2,by)].type==cLOCKEDCHEST))
  {
    found=true;
  }
  if (!found)
  {
    for (int i=0; i<2; i++)
    {
      if ((combobuf[MAPCOMBO2(i,bx,by)].type==cLOCKEDCHEST)||
          (combobuf[MAPCOMBO2(i,bx2,by)].type==cLOCKEDCHEST))
      {
        found=true;
        break;
      }
    }
  }
  if(!found || pushing<8)
  {
    return;
  }

  if (!usekey()) return;
  setmapflag(mLOCKEDCHEST);
  remove_lockedchests((currscr>=128)?1:0,true);
  itemflag = MAPFLAG(bx,by)==mfARMOS_ITEM;
  itemflag |= MAPFLAG(bx2,by)==mfARMOS_ITEM;
  itemflag |= MAPCOMBOFLAG(bx,by)==mfARMOS_ITEM;
  itemflag |= MAPCOMBOFLAG(bx2,by)==mfARMOS_ITEM;
  if (!itemflag)
  {
    for (int i=0; i<2; i++)
    {
      itemflag |= MAPFLAG2(i,bx,by)==mfARMOS_ITEM;
      itemflag |= MAPFLAG2(i,bx2,by)==mfARMOS_ITEM;
      itemflag |= MAPCOMBOFLAG2(i,bx,by)==mfARMOS_ITEM;
      itemflag |= MAPCOMBOFLAG2(i,bx2,by)==mfARMOS_ITEM;
    }
  }

  if(itemflag && !getmapflag())
  {
    items.add(new item(x, y, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
  }
}


void LinkClass::checkbosschest()
{
  if(toogam) return;

  int bx = int(x)&0xF0;
  int bx2 = int(x+8)&0xF0;
  int by = int(y)&0xF0;

  switch(dir)
  {
    case up:
    if(!((int)y&15)&&y!=0) by-=get_bit(quest_rules, qr_LTTPCOLLISION)*16;
    break;
    case down:
    case left:
    case right:
    return;
    break;
  }

  bool found=false;
  bool itemflag=false;
  if ((combobuf[MAPCOMBO(bx,by)].type==cBOSSCHEST)||
      (combobuf[MAPCOMBO(bx2,by)].type==cBOSSCHEST))
  {
    found=true;
  }
  if (!found)
  {
    for (int i=0; i<2; i++)
    {
      if ((combobuf[MAPCOMBO2(i,bx,by)].type==cBOSSCHEST)||
          (combobuf[MAPCOMBO2(i,bx2,by)].type==cBOSSCHEST))
      {
        found=true;
        break;
      }
    }
  }
  if(!found || pushing<8)
  {
    return;
  }

  if(!(game->lvlitems[dlevel]&liBOSSKEY)) return;
  setmapflag(mBOSSCHEST);
  remove_bosschests((currscr>=128)?1:0,true);
  itemflag = MAPFLAG(bx,by)==mfARMOS_ITEM;
  itemflag |= MAPFLAG(bx2,by)==mfARMOS_ITEM;
  itemflag |= MAPCOMBOFLAG(bx,by)==mfARMOS_ITEM;
  itemflag |= MAPCOMBOFLAG(bx2,by)==mfARMOS_ITEM;
  if (!itemflag)
  {
    for (int i=0; i<2; i++)
    {
      itemflag |= MAPFLAG2(i,bx,by)==mfARMOS_ITEM;
      itemflag |= MAPFLAG2(i,bx2,by)==mfARMOS_ITEM;
      itemflag |= MAPCOMBOFLAG2(i,bx,by)==mfARMOS_ITEM;
      itemflag |= MAPCOMBOFLAG2(i,bx2,by)==mfARMOS_ITEM;
    }
  }

  if(itemflag && !getmapflag())
  {
    items.add(new item(x, y, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
  }
}






void LinkClass::checklocked()
{
  if(toogam) return;
  if(!isdungeon()) return;
  if(pushing!=8) return;
  if((tmpscr->door[dir]!=dLOCKED) && (tmpscr->door[dir]!=dBOSS)) return;

  int si = (currmap<<7) + currscr;
  int di = nextscr(dir);
  if(di==0xFFFF) return; 

  switch(dir)
  {
    case up:
    if(y>32 || (get_bit(quest_rules,qr_LTTPWALK)?(x<=112||x>=128):x!=120)) return;
    if (tmpscr->door[dir]==dLOCKED)
    {
      if(usekey())
      {
        putdoor(0,up,dUNLOCKED);
        tmpscr->door[0]=dUNLOCKED;
        game->maps[si] |= 1;
            game->maps[di] |= 2;
      } else return;
    }
    else if (tmpscr->door[dir]==dBOSS)
      {
        if(game->lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,up,dOPENBOSS);
          tmpscr->door[0]=dOPENBOSS;
          game->maps[si] |= 1;
              game->maps[di] |= 2;
        } else return;
      }
      break;
    case down:
    if(y!=128 || (get_bit(quest_rules,qr_LTTPWALK)?(x<=112||x>=128):x!=120)) return;
    if (tmpscr->door[dir]==dLOCKED)
    {
      if(usekey())
      {
        putdoor(0,down,dUNLOCKED);
        tmpscr->door[1]=dUNLOCKED;
        game->maps[si] |= 2;
            game->maps[di] |= 1;
      } else return;
    }
    else if (tmpscr->door[dir]==dBOSS)
      {
        if(game->lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,down,dOPENBOSS);
          tmpscr->door[1]=dOPENBOSS;
          game->maps[si] |= 2;
              game->maps[di] |= 1;
        } else return;
      }
      break;
    case left:
    if((get_bit(quest_rules,qr_LTTPWALK)?(y<=72||y>=88):y!=80) || x!=32) return;

    if (tmpscr->door[dir]==dLOCKED)
    {
      if(usekey())
      {
        putdoor(0,left,dUNLOCKED);
        tmpscr->door[2]=dUNLOCKED;
        game->maps[si] |= 4;
            game->maps[di] |= 8;
      } else return;
    }
    else if (tmpscr->door[dir]==dBOSS)
      {
        if(game->lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,left,dOPENBOSS);
          tmpscr->door[2]=dOPENBOSS;
          game->maps[si] |= 4;
              game->maps[di] |= 8;
        } else return;
      }
      break;
    case right:
    if((get_bit(quest_rules,qr_LTTPWALK)?(y<=72||y>=88):y!=80) || x!=208) return;
    if (tmpscr->door[dir]==dLOCKED)
    {
      if(usekey())
      {
        putdoor(0,right,dUNLOCKED);
        tmpscr->door[3]=dUNLOCKED;
        game->maps[si] |= 8;
            game->maps[di] |= 4;
      } else return;
    }
    else if (tmpscr->door[dir]==dBOSS)
      {
        if(game->lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,right,dOPENBOSS);
          tmpscr->door[3]=dOPENBOSS;
          game->maps[si] |= 8;
              game->maps[di] |= 4;
        } else return;
      }
  }
  sfx(WAV_DOOR);
  markBmap(-1);
}

void LinkClass::fairycircle(int type)
{
  switch(type)
  {
    case REFILL_LIFE:
    if(didstuff&did_fairy)
      return;
    break;
    case REFILL_MAGIC:
    if(didstuff&did_magic)
      return;
    break;
    case REFILL_ALL:
    if(didstuff&did_all)
      return;
  }

  if(fairyclk==0)
  {
    refill_what=type;
    refill_why=REFILL_FAIRY;
    StartRefill();
    action=freeze;
    holdclk=0;
    hopclk=0;
  }

  ++fairyclk;

  if(!refill() && ++holdclk>80)
  {
    action=none;
    switch(type)
    {
      case REFILL_LIFE:
      didstuff|=did_fairy; break;
      case REFILL_MAGIC:
      didstuff|=did_magic; break;
      case REFILL_ALL:
      didstuff|=did_all; break;
    }
    fairyclk=0;
  }
}

int touchcombo(int x,int y)
{
  switch(combobuf[MAPCOMBO(x,y)].type)
  {
    case cBSGRAVE:
    case cGRAVE:
    if(MAPFLAG(x,y)||MAPCOMBOFLAG(x,y))
    {
      break;
    }
    // fall through
    case cARMOS:
    {
      return combobuf[MAPCOMBO(x,y)].type;
    }
  }
  return 0;
}

void LinkClass::checktouchblk()
{
  if(toogam) return;

  if(!pushing)
    return;

  int tx=0,ty=-1;
  switch(dir)
  {
    case up:
    if(touchcombo(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:7)))
    {
      tx=x; ty=y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:7);
    }
    else if(touchcombo(x+8,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:7)))
      {
        tx=x+8; ty=y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:7);
      }
      break;
    case down:
    if(touchcombo(x,y+16))
    {
      tx=x; ty=y+16;
    }
    else if(touchcombo(x+8,y+16))
      {
        tx=x+8; ty=y+16;
      }
      break;
    case left:
    if(touchcombo(x-1,y+15))
    {
      tx=x-1; ty=y+15;
    }
    break;
    case right:
    if(touchcombo(x+16,y+15))
    {
      tx=x+16; ty=y+15;
    }
    break;
  }
  if(ty>=0)
  {
    ty&=0xF0;
    tx&=0xF0;
    int di = ty+(tx>>4);
    int gc=0;
    for (int i=0; i<guys.Count(); ++i)
    {
      if (((enemy*)guys.spr(i))->mainguy)
      {
        ++gc;
      }
    }
    if(di<176 && !guygrid[di] && gc<11)
    {
      guygrid[di]=1;
      int id=0;
      switch(combobuf[MAPCOMBO(tx,ty)].type)
      {
        case cARMOS: id=eARMOS; break;
        case cBSGRAVE:
        tmpscr->data[di]++;
        //fall through
        case cGRAVE:
        id=eGHINI2;
        break;
      }
      addenemy(tx,ty+3,id,0);
    }
  }
}

int LinkClass::nextcombo(int cx, int cy, int cdir)
{
  switch(cdir)
  {
    case up:    cy-=16; break;
    case down:  cy+=16; break;
    case left:  cx-=16; break;
    case right: cx+=16; break;
  }

  // off the screen
  if(cx<0 || cy<0 || cx>255 || cy>175)
  {
    int ns = nextscr(cdir);
	if(ns==0xFFFF) return 0;

    // want actual screen index, not game->maps[] index
    ns = (ns&127) + (ns>>7)*MAPSCRS;

    switch(cdir)
    {
      case up:    cy=160; break;
      case down:  cy=0; break;
      case left:  cx=240; break;
      case right: cx=0; break;
    }

    // from MAPCOMBO()
    int cmb = (cy&0xF0)+(cx>>4);
    if(cmb>175)
      return 0;
    return TheMaps[ns].data[cmb];                           // entire combo code
  }

  return MAPCOMBO(cx,cy);
}

int LinkClass::nextflag(int cx, int cy, int cdir, bool comboflag)
{
  switch(cdir)
  {
    case up:    cy-=16; break;
    case down:  cy+=16; break;
    case left:  cx-=16; break;
    case right: cx+=16; break;
  }

  // off the screen
  if(cx<0 || cy<0 || cx>255 || cy>175)
  {
    int ns = nextscr(cdir);
	if(ns==0xFFFF) return 0;

    // want actual screen index, not game->maps[] index
    ns = (ns&127) + (ns>>7)*MAPSCRS;

    switch(cdir)
    {
      case up:    cy=160; break;
      case down:  cy=0; break;
      case left:  cx=240; break;
      case right: cx=0; break;
    }

    // from MAPCOMBO()
    int cmb = (cy&0xF0)+(cx>>4);
    if(cmb>175)
      return 0;
    if (!comboflag)
    {
      return TheMaps[ns].sflag[cmb];                          // flag
    }
    else
    {
      return combobuf[TheMaps[ns].data[cmb]].flag;                          // flag
    }
  }

  if (comboflag)
  {
    return MAPCOMBOFLAG(cx,cy);
  }
  return MAPFLAG(cx,cy);
}

bool did_secret;

void LinkClass::checkspecial()
{
  checktouchblk();

  bool hasmainguy = hasMainGuy();                           // calculate it once

  if(!(loaded_enemies && !hasmainguy))
    did_secret=false;
  else
  {
    // after beating enemies

    // if room has traps, guys don't come back
    if(guys.idCount(eTRAP))
      setmapflag(mTMPNORET);

    // item
    if(hasitem)
    {
      int Item=tmpscr->item;
      if(getmapflag())
        Item=0;
      if(Item)
      {
        if(hasitem==1)
          sfx(WAV_CLEARED);
        additem(tmpscr->itemx,tmpscr->itemy+1,Item,ipONETIME+ipBIGRANGE+
                ((Item==iTriforce || (tmpscr->flags3&fHOLDITEM)) ? ipHOLDUP : 0));
      }
      hasitem=0;
    }

    // clear enemies and open secret
    if(!did_secret && (tmpscr->flags2&fCLEARSECRET))
    {
      hidden_entrance(0,true,true);
      if(tmpscr->flags4&fENEMYSCRTPERM && !isdungeon())
      {
        if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
      }
      if(!nosecretsounds)
      {
        sfx(WAV_SECRET);
      }
      did_secret=true;
    }
  }

  // doors
  bool has_shutters=false;
  for(int i=0; i<4; i++)
    if(tmpscr->door[i]==dSHUTTER)
      has_shutters=true;
    if(has_shutters)
  {
    if(opendoors==0 && loaded_enemies)
    {
      // if flag is set, open by pushing block instead
      if(!(tmpscr->flags&fSHUTTERS) && !hasmainguy)
        opendoors=12;
    }
    else if(opendoors<0)
        ++opendoors;
      else if((--opendoors)==0)
          openshutters();
  }

  // set boss flag when boss is gone
  if(loaded_enemies && tmpscr->enemyflags&efBOSS && !hasmainguy)
  {
    game->lvlitems[dlevel]|=liBOSS;
    stop_sfx(WAV_ROAR);
    stop_sfx(WAV_VADER);
    stop_sfx(WAV_DODONGO);
  }

  if(getmapflag(mCHEST))              // if special stuff done before
  {
    remove_chests((currscr>=128)?1:0,false);
  }

  if(getmapflag(mLOCKEDCHEST))              // if special stuff done before
  {
    remove_lockedchests((currscr>=128)?1:0,false);
  }

  if(getmapflag(mBOSSCHEST))              // if special stuff done before
  {
    remove_bosschests((currscr>=128)?1:0,false);
  }
}

void LinkClass::checkspecial2(int *ls)
{
  if(toogam) return;
  bool didstrig = false;

  //Checks for sensitive warps. Sensitive warps are used no matter where Link is or whether he's arrived on top of it or not
  for(int i=get_bit(quest_rules,qr_LTTPCOLLISION)?0:8;i<16;i+=get_bit(quest_rules,qr_LTTPCOLLISION)?15:7)
  {
    for(int j=0;j<16;j+=15)
    {
      int stype = COMBOTYPE(x+j,y+i);
      if(stype==cSWARPA)
      {
        if(tmpscr->flags5&fDIRECTSWARP)
        {
          didpit=true;
          pitx=x;pity=y;
        }
        sdir=dir;
        dowarp(0,0);
        return;
      }
      if(stype==cSWARPB)
      {
        if(tmpscr->flags5&fDIRECTSWARP)
        {
          didpit=true;
          pitx=x;
          pity=y;
        }
        sdir=dir;
        dowarp(0,1);
        return;
      }
      if(stype==cSWARPC)
      {
        if(tmpscr->flags5&fDIRECTSWARP)
        {
          didpit=true;
          pitx=x;
          pity=y;
        }
        sdir=dir;
        dowarp(0,2);
        return;
      }
      if(stype==cSWARPD)
      {
        if(tmpscr->flags5&fDIRECTSWARP)
        {
          didpit=true;
          pitx=x;
          pity=y;
        }
        sdir=dir;
        dowarp(0,3);
        return;
      }
      if(stype==cSWARPR)
      {
        if(tmpscr->flags5&fDIRECTSWARP)
        {
          didpit=true;
          pitx=x;
          pity=y;
        }
        sdir=dir;
        dowarp(0,rand()%4);
        return;
      }
      if(stype==cSTRIGNOFLAG || stype==cSTRIGFLAG)
      {
        if(stype==cSTRIGFLAG && !isdungeon())
        {
          if(!(tmpscr->flags5&fTEMPSECRETS))
          {
            setmapflag(mSECRET);
          }
          if(!didstrig)
          {
            hidden_entrance(0,true,false);
            didstrig = true;
          }
        }
        else
        {
          if(!didstrig) {hidden_entrance(0,true,true); didstrig = true;}
        }
      }
    }
  }

  // check if he's standing on a warp he just came out of
  if((int(y)&0xF8)==warpy)
  {
    if(x==warpx)
    {
      return;
    }
  }
  warpy=255;
  int tx=x;
  int ty=y;

  int flag=0;
  int flag2=0;
  int flag3=0;
  int type=0;
  int index = 0;
  //bool gotpit=false;

  int x1,x2,y1,y2;
  x1 = tx;
  x2 = tx+15;
  y1 = ty;
  y2 = ty+15;

  if(get_bit(quest_rules, qr_LTTPWALK))
  {
    x1 = tx+4;
    x2 = tx+11;
    y1 = ty+4;
    y2 = ty+11;
  }

  int types[4];
  types[0]=types[1]=types[2]=types[3]=-1;

  types[0] = MAPFLAG(x1,y1);
  types[1] = MAPFLAG(x1,y2);
  types[2] = MAPFLAG(x2,y1);
  types[3] = MAPFLAG(x2,y2);
  if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
	  flag = types[0];

  types[0] = MAPCOMBOFLAG(x1,y1);
  types[1] = MAPCOMBOFLAG(x1,y2);
  types[2] = MAPCOMBOFLAG(x2,y1);
  types[3] = MAPCOMBOFLAG(x2,y2);
  if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
	  flag2 = types[0];

  types[0] = MAPFFCOMBOFLAG(x1,y1);
  types[1] = MAPFFCOMBOFLAG(x1,y2);
  types[2] = MAPFFCOMBOFLAG(x2,y1);
  types[3] = MAPFFCOMBOFLAG(x2,y2);
  if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
	  flag3 = types[0];

  types[0] = COMBOTYPE(x1,y1);
  if(MAPFFCOMBO(x1,y1))
	  types[0] = FFCOMBOTYPE(x1,y1);
  types[1] = COMBOTYPE(x1,y2);
  if(MAPFFCOMBO(x1,y2))
	  types[1] = FFCOMBOTYPE(x1,y2);
  types[2] = COMBOTYPE(x2,y1);
  if(MAPFFCOMBO(x2,y1))
	  types[2] = FFCOMBOTYPE(x2,y1);
  types[3] = COMBOTYPE(x2,y2);
  if(MAPFFCOMBO(x2,y2))
	  types[3] = FFCOMBOTYPE(x2,y2);

  for(int i=0;i<4;i++)
  {
    if(types[i]==cCAVE) index=0;
    else if(types[i]==cCAVEB) {types[i]=cCAVE; index=1;}
    else if(types[i]==cCAVEC) {types[i]=cCAVE; index=2;}
    else if(types[i]==cCAVED) {types[i]=cCAVE; index=3;}
  
    if(types[i]==cPIT) index=0;
    else if(types[i]==cPITB) {types[i]=cPIT; index=1;}
    else if(types[i]==cPITC) {types[i]=cPIT; index=2;}
    else if(types[i]==cPITD) {types[i]=cPIT; index=3;}
  
    if(types[i]==cSTAIR) index=0;
    else if(types[i]==cSTAIRB) {types[i]=cSTAIR; index=1;}
    else if(types[i]==cSTAIRC) {types[i]=cSTAIR; index=2;}
    else if(types[i]==cSTAIRD) {types[i]=cSTAIR; index=3;}
    else if(types[i]==cSTAIRR) {types[i]=cSTAIR; index=rand()%4;}
  
    if(types[i]==cCAVE2) index=0;
    else if(types[i]==cCAVE2B) {types[i]=cCAVE2; index=1;}
    else if(types[i]==cCAVE2C) {types[i]=cCAVE2; index=2;}
    else if(types[i]==cCAVE2D) {types[i]=cCAVE2; index=3;}
  
    if(types[i]==cSWIMWARP) index=0;
    else if(types[i]==cSWIMWARPB) {types[i]=cSWIMWARP; index=1;}
    else if(types[i]==cSWIMWARPC) {types[i]=cSWIMWARP; index=2;}
    else if(types[i]==cSWIMWARPD) {types[i]=cSWIMWARP; index=3;}
  
    if(types[i]==cDIVEWARP) index=0;
    else if(types[i]==cDIVEWARPB) {types[i]=cDIVEWARP; index=1;}
    else if(types[i]==cDIVEWARPC) {types[i]=cDIVEWARP; index=2;}
    else if(types[i]==cDIVEWARPD) {types[i]=cDIVEWARP; index=3;}
  
    if(types[i]==cSTEP) types[i]=cSTEP;
    else if(types[i]==cSTEPSAME) types[i]=cSTEP;
    else if(types[i]==cSTEPALL) types[i]=cSTEP;
  }

  if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
	if(action!=freeze&&(!msgstr || msgpos>=72))
	  type = types[0];

  x1 = tx+4;
  x2 = tx+11;
  y1 = ty+4;
  y2 = ty+11;

  types[0] = COMBOTYPE(x1,y1);
  if(MAPFFCOMBO(x1,y1))
	  types[0] = FFCOMBOTYPE(x1,y1);
  types[1] = COMBOTYPE(x1,y2);
  if(MAPFFCOMBO(x1,y2))
	  types[1] = FFCOMBOTYPE(x1,y2);
  types[2] = COMBOTYPE(x2,y1);
  if(MAPFFCOMBO(x2,y1))
	  types[2] = FFCOMBOTYPE(x2,y1);
  types[3] = COMBOTYPE(x2,y2);
  if(MAPFFCOMBO(x2,y2))
	  types[3] = FFCOMBOTYPE(x2,y2);

  bool setsave=false;
  for(int i=0;i<4;i++)
  {
    if(types[i]==cSAVE) setsave=true;
    if(types[i]==cSAVE2) setsave=true;
  }
  if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
	if(setsave)
      type = types[0];

  x1 = tx+7;
  x2 = tx+8;
  y1 = ty+7+((get_bit(quest_rules,qr_LTTPCOLLISION))?0:4);
  y2 = ty+8+((get_bit(quest_rules,qr_LTTPCOLLISION))?0:4);

  types[0] = COMBOTYPE(x1,y1);
  if(MAPFFCOMBO(x1,y1))
	  types[0] = FFCOMBOTYPE(x1,y1);
  types[1] = COMBOTYPE(x1,y2);
  if(MAPFFCOMBO(x1,y2))
	  types[1] = FFCOMBOTYPE(x1,y2);
  types[2] = COMBOTYPE(x2,y1);
  if(MAPFFCOMBO(x2,y1))
	  types[2] = FFCOMBOTYPE(x2,y1);
  types[3] = COMBOTYPE(x2,y2);
  if(MAPFFCOMBO(x2,y2))
	  types[3] = FFCOMBOTYPE(x2,y2);

  for(int i=0;i<4;i++)
  {
    if(types[i]==cPIT) index=0;
    else if(types[i]==cPITB) {types[i]=cPIT; index=1;}
    else if(types[i]==cPITC) {types[i]=cPIT; index=2;}
    else if(types[i]==cPITD) {types[i]=cPIT; index=3;}
  }
  if(types[0]==cPIT||types[1]==cPIT||types[2]==cPIT||types[3]==cPIT)
    if(action!=freeze&&(!msgstr || msgpos>=72))
	  type=cPIT;

  /*

  if(get_bit(quest_rules, qr_LTTPWALK))
  {
    if((int(x)&7)<4)
      tx-=(int(x)&7);
    if((int(x)&7)>4)
      tx+=8-(int(x)&7);
    if((int(y)&7)<4)
      ty-=(int(y)&7);
    if((int(y)&7)>4)
      ty+=8-(int(y)&7);
  }*/

  if(type==cSAVE&&currscr<128)
    *ls=1;

  if(type==cSAVE2&&currscr<128)
    *ls=2;

  /*// check alignment for the rest of these
  if(int(ty)&7)
    return;
  if(int(tx)&7)
    return;*/

  

  if(flag==mfFAIRY||flag2==mfFAIRY||flag3==mfFAIRY)
  {
    fairycircle(REFILL_LIFE);
    if(fairyclk!=0) return;
  }

  if(flag==mfMAGICFAIRY||flag2==mfMAGICFAIRY||flag3==mfMAGICFAIRY)
  {
    fairycircle(REFILL_MAGIC);
    if(fairyclk!=0) return;
  }

  if(flag==mfALLFAIRY||flag2==mfALLFAIRY||flag3==mfALLFAIRY)
  {
    fairycircle(REFILL_ALL);
    if(fairyclk!=0) return;
  }

  if(flag==mfZELDA||flag2==mfZELDA||flag3==mfZELDA)
  {
    saved_Zelda();
    return;
  }
/*
  if(int(tx)&15)
  {
    if((COMBOTYPE(tx,ty)!=COMBOTYPE(tx+8,ty))&&
       (COMBOTYPE(tx,ty)!=cPIT)&&(COMBOTYPE(tx+8,ty)!=cPIT)&&
       (COMBOTYPE(tx,ty)!=cPITB)&&(COMBOTYPE(tx+8,ty)!=cPITB)&&
       (COMBOTYPE(tx,ty)!=cPITC)&&(COMBOTYPE(tx+8,ty)!=cPITC)&&
       (COMBOTYPE(tx,ty)!=cPITD)&&(COMBOTYPE(tx+8,ty)!=cPITD)&&
       (COMBOTYPE(tx,ty)!=cPITR)&&(COMBOTYPE(tx+8,ty)!=cPITR))
      return;
    if (COMBOTYPE(tx+8,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPIT)
    {
      type=cPIT;
      index=0;
      gotpit=true;
    }
    if (COMBOTYPE(tx+8,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITB)
    {
      type=cPIT;
      index=1;
      gotpit=true;
    }
    if (COMBOTYPE(tx+8,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITC)
    {
      type=cPIT;
      index=2;
      gotpit=true;
    }
    if (COMBOTYPE(tx+8,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITD)
    {
      type=cPIT;
      index=3;
      gotpit=true;
    }
    if (COMBOTYPE(tx+8,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITR)
    {
      type=cPIT;
      index=rand()%4;
      gotpit=true;
    }
  }

  if(int(ty)&15)
  {
    if((COMBOTYPE(tx,ty)!=COMBOTYPE(tx,ty+8))&&
       (COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))!=cPIT)&&(COMBOTYPE(tx,ty+8)!=cPIT)&&
       (COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))!=cPITB)&&(COMBOTYPE(tx,ty+8)!=cPITB)&&
       (COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))!=cPITC)&&(COMBOTYPE(tx,ty+8)!=cPITC)&&
       (COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))!=cPITD)&&(COMBOTYPE(tx,ty+8)!=cPITD)&&
       (COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))!=cPITR)&&(COMBOTYPE(tx,ty+8)!=cPITR))
      return;
    if ((COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPIT)||((COMBOTYPE(tx,ty+8)==cPIT)))
    {
      type=cPIT;
      index=0;
      gotpit=true;
    }
    if ((COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITB)||((COMBOTYPE(tx,ty+8)==cPITB)))
    {
      type=cPIT;
      index=1;
      gotpit=true;
    }
    if ((COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITC)||((COMBOTYPE(tx,ty+8)==cPITC)))
    {
      type=cPIT;
      index=2;
      gotpit=true;
    }
    if ((COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITD)||((COMBOTYPE(tx,ty+8)==cPITD)))
    {
      type=cPIT;
      index=3;
      gotpit=true;
    }
    if ((COMBOTYPE(tx,ty+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8))==cPITR)||((COMBOTYPE(tx,ty+8)==cPITR)))
    {
      type=cPIT;
      index=rand()%4;
      gotpit=true;
    }
  }*/

  if(type==cTRIGNOFLAG || type==cTRIGFLAG)
  {
    if(type==cTRIGFLAG && !isdungeon())
    {
      if(!(tmpscr->flags5&fTEMPSECRETS)) setmapflag(mSECRET);
      hidden_entrance(0,true,false);
    }
    else
      hidden_entrance(0,true,true);
  }

  if(type==cSTEP)
  {
    if(COMBOTYPE(tx+8,ty+8)==cSTEP)
    {
      tmpscr->data[((ty+8)&0xF0)+((tx+8)>>4)]++;
      reset_combo_animation(tmpscr->data[((ty+8)&0xF0)+((tx+8)>>4)]);
    }

    if(COMBOTYPE(tx+8,ty+8)==cSTEPSAME)
    {
      int stepc = tmpscr->data[((ty+8)&0xF0)+((tx+8)>>4)];
      for(int k=0;k<176;k++)
      {
        if(tmpscr->data[k]==stepc)
        {
          tmpscr->data[k]++;
          reset_combo_animation(tmpscr->data[k]);
        }
      }
    }

    if(COMBOTYPE(tx+8,ty+8)==cSTEPALL)
    {
      for(int k=0;k<176;k++)
      {
        if(
            (combobuf[tmpscr->data[k]].type==cSTEP)||
            (combobuf[tmpscr->data[k]].type==cSTEPSAME)||
            (combobuf[tmpscr->data[k]].type==cSTEPALL)||
            (combobuf[tmpscr->data[k]].type==cSTEPCOPY)
          )
        {
          tmpscr->data[k]++;
          reset_combo_animation(tmpscr->data[k]);
        }
      }
    }
  }

  if(type!=cCAVE && type!=cCAVE2 && type!=cSTAIR &&
     type!=cPIT && type!=cSWIMWARP && type!=cRESET &&
     !(type==cDIVEWARP && diveclk>30))
  {
    switch(flag)
    {
      case mfDIVE_ITEM:
      if(diveclk>30 && !getmapflag())
      {
        additem(x, y, tmpscr->catchall,
                ipONETIME + ipBIGRANGE + ipHOLDUP + ipNODRAW);
        if(!nosecretsounds)
        {
          sfx(WAV_SECRET);
        }
      }
      return;

      case mfRAFT:
      case mfRAFT_BRANCH:
      if((current_item(itype_raft, true)>=i_raft) && action!=rafting && action!=swimhit
         && action!=gothit && type==cDOCK)
      {
        if((isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true))))
        {
          action=rafting;
          if(!nosecretsounds)
          {
            sfx(WAV_SECRET);
          }
        }
      }
      return;

      default:
      break;
      //return;
    }
    switch(flag2)
    {
      case mfDIVE_ITEM:
      if(diveclk>30 && !getmapflag())
      {
        additem(x, y, tmpscr->catchall,
                ipONETIME + ipBIGRANGE + ipHOLDUP + ipNODRAW);
        if(!nosecretsounds)
        {
          sfx(WAV_SECRET);
        }
      }
      return;

      case mfRAFT:
      case mfRAFT_BRANCH:
      if((current_item(itype_raft, true)>=i_raft) && action!=rafting && action!=swimhit
         && action!=gothit && type==cDOCK)
      {
        if((isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true))))
        {
          action=rafting;
          if(!nosecretsounds)
          {
            sfx(WAV_SECRET);
          }
        }
      }
      return;

      default:
		  break;
      //return;
    }
	switch(flag3)
    {
      case mfDIVE_ITEM:
      if(diveclk>30 && !getmapflag())
      {
        additem(x, y, tmpscr->catchall,
                ipONETIME + ipBIGRANGE + ipHOLDUP + ipNODRAW);
        if(!nosecretsounds)
        {
          sfx(WAV_SECRET);
        }
      }
      return;

      case mfRAFT:
      case mfRAFT_BRANCH:
      if((current_item(itype_raft, true)>=i_raft) && action!=rafting && action!=swimhit
         && action!=gothit && type==cDOCK)
      {
        if((isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true))))
        {
          action=rafting;
          if(!nosecretsounds)
          {
            sfx(WAV_SECRET);
          }
        }
      }
      return;

      default:
      return;
    }
  }

  draw_screen(tmpscr, 0, 0);
  advanceframe();

  int t=(currscr<128)?0:1;
  if(type==cCAVE)
  {
    if (tmpscr[t].tilewarptype[0]!=wtPASS)
    {
      music_stop();
    }
    walkdown();
  }
  else if(type==cCAVE2)
  {
    if (tmpscr[t].tilewarptype[0]!=wtPASS)
    {
      music_stop();
    }
    walkup2();
  }

  if (type==cPIT)
  {
    didpit=true;
    pitx=x;
    pity=y;
  }

  if(dlevel==0 && currscr==129 && type==cSTAIR)
  {
    // "take any road you want"
    int dw = x<112 ? 1 : (x>136 ? 3 : 2);
    int code = WARPCODE(currdmap,homescr,dw);
    if(code!=-1)
    {
      currdmap = code>>8;
      dlevel  = DMaps[currdmap].level;
      currmap = DMaps[currdmap].map;
      homescr = (code&0xFF) + DMaps[currdmap].xoff;
      if(!isdungeon())
        setmapflag(mSECRET);
    }
    exitcave();
    return;
  }

  if(type==cRESET)
  {
    if(!(tmpscr->noreset&mSECRET)) unsetmapflag(mSECRET);
    if(!(tmpscr->noreset&mITEM)) unsetmapflag(mITEM);
    if(!(tmpscr->noreset&mBELOW)) unsetmapflag(mBELOW);
    if(!(tmpscr->noreset&mNEVERRET)) unsetmapflag(mNEVERRET);
    if(!(tmpscr->noreset&mCHEST)) unsetmapflag(mCHEST);
    if(!(tmpscr->noreset&mLOCKEDCHEST)) unsetmapflag(mLOCKEDCHEST);
    if(!(tmpscr->noreset&mBOSSCHEST)) unsetmapflag(mBOSSCHEST);
    if(!(tmpscr->noreset&mLOCKBLOCK)) unsetmapflag(mLOCKBLOCK);
    if(!(tmpscr->noreset&mBOSSLOCKBLOCK)) unsetmapflag(mBOSSLOCKBLOCK);
    if(isdungeon())
    {
      if(!(tmpscr->noreset&mDOOR_LEFT)) unsetmapflag(mDOOR_LEFT);
      if(!(tmpscr->noreset&mDOOR_RIGHT)) unsetmapflag(mDOOR_RIGHT);
      if(!(tmpscr->noreset&mDOOR_DOWN)) unsetmapflag(mDOOR_DOWN);
      if(!(tmpscr->noreset&mDOOR_UP)) unsetmapflag(mDOOR_UP);
    }
    //tmpscr->tilewarpdmap=currdmap;
    //tmpscr->tilewarpscr=homescr-(((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
    //tmpscr->tilewarptype=wtIWARP;
    didpit=true;
    pitx=x;
    pity=y;
    sdir=dir; dowarp(4,0);
  }
  else
  {
    sdir = dir; dowarp(0,index);
  }
}

int selectWlevel(int d)
{
  if(TriforceCount()==0)
    return 0;

  byte l = get_gamedata_wlevel(game);

  do
  {
    if(d==0 && (game->lvlitems[l+1] & liTRIFORCE))
      break;
    else if(d<0)
        l = (l==0) ? 7 : l-1;
      else
        l = (l==7) ? 0 : l+1;
  } while( !(game->lvlitems[l+1] & liTRIFORCE) );

  set_gamedata_wlevel(game, l);
  return l;
}

bool LinkClass::dowarp(int type, int index)
{
  byte wdmap=0,wscr=0,wtype=0,t=0;
  t=(currscr<128)?0:1;
  int wrindex = (tmpscr->warpreturnc>>(index*2))&3;
  //int lastent_org = lastentrance;
  //int lastdmap_org = lastentrace_dmap;
  switch(type)
  {
    case 0:                                                 // tile warp
    wtype = tmpscr[t].tilewarptype[index];
    wdmap = tmpscr[t].tilewarpdmap[index];
    wscr = tmpscr[t].tilewarpscr[index];
    break;
    case 1:                                                 // side warp
    wtype = tmpscr[t].sidewarptype[index];
    wdmap = tmpscr[t].sidewarpdmap[index];
    wscr = tmpscr[t].sidewarpscr[index];
    break;
    case 2:                                                 // whistle warp
    {
      wtype = wtWHISTLE;
      int level=0;
      if(blowcnt==0)
        level = selectWlevel(0);
      else
      {
        for(int i=0; i<abs(blowcnt); i++)
          level = selectWlevel(blowcnt);
      }
      wdmap = QMisc.wind[level].dmap;
      wscr = QMisc.wind[level].scr;
    }
    break;
    case 3:
    wtype = wtIWARP;
    wdmap = cheat_goto_dmap;
    wscr = cheat_goto_screen;
    break;
    case 4:
    wtype = wtIWARP;
    wdmap = currdmap;
    wscr = homescr-(((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
    break;
  }
  bool intradmap = (wdmap == currdmap);
  switch(wtype)
  {
    case wtCAVE:                                            // cave/item room
    ALLOFF();
    homescr=currscr;
    currscr=0x80;
    if(dlevel==0)                                         // cave
    {
      music_stop();
      kill_sfx();
      if(tmpscr->room==rWARP)
        currscr=0x81;
      loadlvlpal(10);
      bool b2 = COOLSCROLL&&
        ((combobuf[MAPCOMBO(x,y-16)].type==cCAVE)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2)||
         (combobuf[MAPCOMBO(x,y-16)].type==cCAVEB)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2B)||
         (combobuf[MAPCOMBO(x,y-16)].type==cCAVEC)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2C)||
         (combobuf[MAPCOMBO(x,y-16)].type==cCAVED)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2D));
      blackscr(30,b2?false:true);
      loadscr(0,currscr,up);
      loadscr(1,homescr,up);
      putscr(scrollbuf,0,0,tmpscr);
      dir=up;
      x=112;
      y=160;
      if (didpit)
      {
        didpit=false;
        x=pitx;
        y=pity;
      }
      reset_hookshot();
      stepforward(get_bit(quest_rules,qr_LTTPWALK)?5:6, false);
    }
    else                                                  // item room
    {
      stop_sfx(WAV_ROAR);
      stop_sfx(WAV_VADER);
      stop_sfx(WAV_DODONGO);
      draw_screen(tmpscr, 0, 0);
      if ((tmpscr->flags&fDARK) == 0)
      {
        fade(DMaps[currdmap].color,true,false,false);
      }
      blackscr(30,true);
      loadscr(0,currscr,down);
      loadscr(1,homescr,-1);
      dontdraw=true;
      draw_screen(tmpscr, 0, 0);
      fade(11,true,true,false);
      dir=down;
      x=48;
      y=0;
      // is this didpit check necessary?
      if (didpit)
      {
        didpit=false;
        x=pitx;
        y=pity;
      }
      reset_hookshot();
      dontdraw=false;
      stepforward(get_bit(quest_rules,qr_LTTPWALK)?16:18, false);
    }
    break;

    case wtPASS:                                            // passageway
    {
      stop_sfx(WAV_ROAR);
      stop_sfx(WAV_VADER);
      stop_sfx(WAV_DODONGO);
      ALLOFF();
      homescr=currscr;
      currscr=0x81;
      byte warpscr = wscr + DMaps[currdmap].xoff;
      draw_screen(tmpscr, 0, 0);
      fade(DMaps[currdmap].color,true,false,false);
      blackscr(30,true);
      loadscr(0,currscr,down);
      loadscr(1,homescr,-1);
      dontdraw=true;
      draw_screen(tmpscr, 0, 0);
      fade(11,true,true,false);
      dir=down;
      x=48;
      if( (homescr&15) > (warpscr&15) )
      {
        x=192;
      }
      if( (homescr&15) == (warpscr&15) )
      {
        if( (currscr>>4) > (warpscr>>4) )
        {
          x=192;
        }
      }
      // is this didpit check necessary?
      if (didpit)
      {
        didpit=false;
        x=pitx;
        y=pity;
      }
      warpx=x;
      y=0;
      reset_hookshot();
      dontdraw=false;
      stepforward(get_bit(quest_rules,qr_LTTPWALK)?16:18, false);
      newscr_clk=frame;
      activated_timed_warp=false;
    } break;

    case wtEXIT: // entrance/exit
    {
      ALLOFF();
      music_stop();
      kill_sfx();
      blackscr(30,false);
      currdmap = wdmap;
      dlevel=DMaps[currdmap].level;
      currmap=DMaps[currdmap].map;
      loadfullpal();
      ringcolor();
      loadlvlpal(DMaps[currdmap].color);
      //lastentrance_dmap = currdmap;
      homescr = currscr = wscr + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
      loadscr(0,currscr,-1);
      if (tmpscr->flags&fDARK)
      {
        if(get_bit(quest_rules,qr_FADE))
        {
          int dpos = 64;
          int lpos = 32;
          int last = CSET(5)-1;
    
          if(get_bit(quest_rules,qr_FADECS5))
          {
            last += 16;
            loadpalset(5,5);
          }
    
          loadlvlpal(DMaps[currdmap].color);
          byte *si = colordata + CSET(DMaps[currdmap].color*pdLEVEL+poFADE1)*3;
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
          loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
        }
        darkroom=true;
      }
      int wrx,wry;
      if(get_bit(quest_rules,qr_NOARRIVALPOINT))
      {
        wrx=tmpscr->warpreturnx[0];
        wry=tmpscr->warpreturny[0];
      }
      else
      {
        wrx=tmpscr->warparrivalx;
        wry=tmpscr->warparrivaly;
      }
      if(((wrx>0||wry>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))&&(!(tmpscr->flags6&fNOCONTINUEHERE)))
      {
        if (dlevel)
        {
          lastentrance = currscr;
        }
        else
        {
          lastentrance = DMaps[currdmap].cont + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
        }
        lastentrance_dmap = wdmap;
      }
      if(dlevel)
      {
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
        {
          x=tmpscr->warpreturnx[wrindex];
          y=tmpscr->warpreturny[wrindex];
        }
        else
        {
          x=tmpscr->warparrivalx;
          y=tmpscr->warparrivaly;
        }
      }
      else
      {
        x=tmpscr->warpreturnx[wrindex];
        y=tmpscr->warpreturny[wrindex];
      }
      if (didpit)
      {
        didpit=false;
        x=pitx;
        y=pity;
      }
      dir=down;
      if(x==0)   dir=right;
      if(x==240) dir=left;
      if(y==0)   dir=down;
      if(y==160) dir=up;
      if(dlevel)
      {
        // reset enemy kill counts
        for(int i=0; i<128; i++)
        {
          game->guys[(currmap<<7)+i] = 0;
          game->maps[(currmap<<7)+i] &= ~mTMPNORET;
        }
      }
      markBmap(dir^1);
      reset_hookshot();
      if(isdungeon())
      {
        openscreen();
        stepforward(get_bit(quest_rules,qr_LTTPWALK)?11:12, false);
      }
      else
      {
        if(!COOLSCROLL)
          openscreen();

        if((combobuf[MAPCOMBO(x,y-16)].type==cCAVE)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVEB)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVEC)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVED))
        {
          reset_pal_cycling();
          putscr(scrollbuf,0,0,tmpscr);
          walkup();
        }
        else if((combobuf[MAPCOMBO(x,y+16)].type==cCAVE2)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2B)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2C)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2D))
          {
            reset_pal_cycling();
            putscr(scrollbuf,0,0,tmpscr);
            walkdown2();
          }
          else if(COOLSCROLL)
            {
              openscreen();
            }
      }
      show_subscreen_life=true;
      show_subscreen_numbers=true;
      //play_DmapMusic();
      playLevelMusic();
      currcset=DMaps[currdmap].color;
      dointro();
      warpx=x;
      warpy=y;
      for(int i=0; i<6; i++)
        visited[i]=-1;
      break;
    }
    case wtSCROLL:                                          // scrolling warp
    {
      int c = DMaps[currdmap].color;
      currmap = DMaps[wdmap].map;
      lighting(4,dir);
      scrollscr(sdir, wscr+DMaps[wdmap].xoff, wdmap);
      reset_hookshot();
      currdmap = wdmap;
      dlevel = DMaps[currdmap].level;
      homescr = currscr = wscr + (((DMaps[wdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[wdmap].xoff);
      if (!intradmap)
      {
        int wrx,wry;
        if(get_bit(quest_rules,qr_NOARRIVALPOINT))
        {
          wrx=tmpscr->warpreturnx[0];
          wry=tmpscr->warpreturny[0];
        }
        else
        {
          wrx=tmpscr->warparrivalx;
          wry=tmpscr->warparrivaly;
        }
        if(((wrx>0||wry>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))&&(!get_bit(quest_rules,qr_NOSCROLLCONTINUE))&&(!(tmpscr->flags6&fNOCONTINUEHERE)))
        {
          if (dlevel)
          {
            lastentrance = currscr;
          }
          else
          {
            lastentrance = DMaps[currdmap].cont + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
          }
          lastentrance_dmap = wdmap;
        }
      }
      if(DMaps[currdmap].color != c)
      {
        loadlvlpal(DMaps[currdmap].color);
      }
      //play_DmapMusic();
      playLevelMusic();
      currcset=DMaps[currdmap].color;
      dointro();
    } break;

    case wtWHISTLE:                                         // whistle warp
    {
      currmap = DMaps[wdmap].map;
      scrollscr(right, wscr+DMaps[wdmap].xoff, wdmap);
      reset_hookshot();
      currdmap=wdmap;
      dlevel=DMaps[currdmap].level;
      loadlvlpal(DMaps[currdmap].color);
      //play_DmapMusic();
      playLevelMusic();
      currcset=DMaps[currdmap].color;
      dointro();
      action=inwind;
      int wry;
      if(get_bit(quest_rules,qr_NOARRIVALPOINT))
        wry=tmpscr->warpreturny[0];
      else wry=tmpscr->warparrivaly;
      Lwpns.add(new weapon((fix)0,(fix)(wry),wWind,1,0,right));
      whirlwind=255;
    } break;

    case wtIWARP:
    case wtIWARPBLK:
    case wtIWARPOPEN:
    case wtIWARPZAP:
    case wtIWARPWAVE:                                       // insta-warps
    {
      if (!(tmpscr->flags3&fIWARPFULLSCREEN))
      {
        ALLOFF();
        kill_sfx();
      }
      if(wtype==wtIWARPZAP)
      {
        zapout();
      }
      else if (wtype==wtIWARPWAVE)
        {
          wavyout();
        }
        else if(wtype!=wtIWARP)
          {
            bool b2 = COOLSCROLL&&
              ((combobuf[MAPCOMBO(x,y-16)].type==cCAVE)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2)||
               (combobuf[MAPCOMBO(x,y-16)].type==cCAVEB)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2B)||
               (combobuf[MAPCOMBO(x,y-16)].type==cCAVEC)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2C)||
               (combobuf[MAPCOMBO(x,y-16)].type==cCAVED)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVE2D));

            blackscr(30,b2?false:true);
          }

          int c = DMaps[currdmap].color;
      currdmap = wdmap;
      dlevel = DMaps[currdmap].level;
      currmap = DMaps[currdmap].map;

      ringcolor();
      if(DMaps[currdmap].color != c)
        loadlvlpal(DMaps[currdmap].color);

      homescr = currscr = wscr + DMaps[currdmap].xoff;

      loadscr(0,currscr,-1);
      putscr(scrollbuf,0,0,tmpscr);

      x = tmpscr->warpreturnx[wrindex];
      y = tmpscr->warpreturny[wrindex];
      if (didpit)
      {
        didpit=false;
        x=pitx;
        y=pity;
      }

      if(x==0)   dir=right;
      if(x==240) dir=left;
      if(y==0)   dir=down;
      if(y==160) dir=up;
      markBmap(dir^1);
      if(wtype==wtIWARPZAP)
      {
        zapin();
      }
      else if (wtype==wtIWARPWAVE)
        {
          wavyin();
        }
        if((combobuf[MAPCOMBO(x,y-16)].type==cCAVE)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVEB)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVEC)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVED))
      {
        reset_pal_cycling();
        putscr(scrollbuf,0,0,tmpscr);
        walkup();
      }
      else if((combobuf[MAPCOMBO(x,y+16)].type==cCAVE2)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2B)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2C)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2D))
        {
          reset_pal_cycling();
          putscr(scrollbuf,0,0,tmpscr);
          walkdown2();
        }
        else
        {
          if(wtype==wtIWARPOPEN)
          {
            openscreen();
          }
        }
      show_subscreen_life=true;
      show_subscreen_numbers=true;
      //play_DmapMusic();
      playLevelMusic();
      currcset=DMaps[currdmap].color;
      dointro();
      warpx=x;
      warpy=y;
    }
    break;


    case wtNOWARP:
    default:
    didpit=false;
    update_subscreens();
    return false;
  }

  if(action!=rafting && iswater(MAPCOMBO(x,y+8))
     && (current_item(itype_flippers, true)) && (action!=inwind))
  {
    hopclk=0xFF;
    action=swimming;
  }
  newscr_clk=frame;
  activated_timed_warp=false;
  eat_buttons();
  if(wtype!=wtIWARP) attackclk=0;
  didstuff=0;
  map_bkgsfx();
  loadside=dir^1;
  whistleclk=-1;
  if ((DMaps[currdmap].type&dmfCONTINUE) || (currdmap==0))
  {
    if (dlevel)
    {
      int wrx,wry;
      if(get_bit(quest_rules,qr_NOARRIVALPOINT))
      {
        wrx=tmpscr->warpreturnx[0];
        wry=tmpscr->warpreturny[0];
      }
      else
      {
        wrx=tmpscr->warparrivalx;
        wry=tmpscr->warparrivaly;
      }
      if ( (wtype == wtEXIT)
          || (((wtype == wtSCROLL) && !intradmap) && ((wrx>0 || wry>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))))
      {
        if(!(wtype==wtSCROLL)||!(get_bit(quest_rules,qr_NOSCROLLCONTINUE)))
        {
          set_gamedata_continue_scrn(game, homescr);
          //Z_message("continue_scrn = %02X\n e/e",get_gamedata_continue_scrn(game));
        }
      }
      else
      {
        if (currdmap != get_gamedata_continue_dmap(game))
        {
          set_gamedata_continue_scrn(game, DMaps[currdmap].cont + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff));
          //Z_message("continue_scrn = %02X\n dlevel",game->continue_scrn);
        }
      }
    }
    else
    {
      set_gamedata_continue_scrn(game, DMaps[currdmap].cont + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff));
      //Z_message("continue_scrn = %02X\n !dlevel",game->continue_scrn);
    }
    set_gamedata_continue_dmap(game, currdmap);
    lastentrance_dmap = currdmap;
    lastentrance = get_gamedata_continue_scrn(game);
    //Z_message("continue_map = %d\n",game->continue_dmap);
  }
  if (tmpscr->flags4&fAUTOSAVE)
  {
    set_gamedata_continue_scrn(game, homescr);
    set_gamedata_continue_dmap(game, currdmap);
    lastentrance_dmap = currdmap;
    lastentrance = get_gamedata_continue_scrn(game);
  }
  if (tmpscr->flags6&fCONTINUEHERE)
  {
    lastentrance_dmap = currdmap;
    lastentrance = homescr;
  }
  update_subscreens();
  memset(ffposx,0xFF,sizeof(short)*32);
  memset(ffposy,0xFF,sizeof(short)*32);
  memset(ffprvx,0xFF,sizeof(float)*32);
  memset(ffprvy,0xFF,sizeof(float)*32);
  return true;
}

bool LinkClass::check_cheat_warp()
{
  return false;
}

void LinkClass::exitcave()
{
  currscr=homescr;
  loadscr(0,currscr,255);                                   // bogus direction
  x = tmpscr->warpreturnx[0];
  y = tmpscr->warpreturny[0];
  if (didpit)
  {
    didpit=false;
    x=pitx;
    y=pity;
  }
  if(x+y == 0)
    x = y = 80;
  bool b = ((combobuf[MAPCOMBO(x,y-16)].type==cCAVE) || (combobuf[MAPCOMBO(x,y-16)].type==cCAVE2) ||
            (combobuf[MAPCOMBO(x,y-16)].type==cCAVEB) || (combobuf[MAPCOMBO(x,y-16)].type==cCAVE2B) ||
            (combobuf[MAPCOMBO(x,y-16)].type==cCAVEC) || (combobuf[MAPCOMBO(x,y-16)].type==cCAVE2C) ||
            (combobuf[MAPCOMBO(x,y-16)].type==cCAVED) || (combobuf[MAPCOMBO(x,y-16)].type==cCAVE2D)) && COOLSCROLL;
  blackscr(30,b?false:true);
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);
  lighting(2,dir);
  music_stop();
  kill_sfx();
  ALLOFF();
  putscr(scrollbuf,0,0,tmpscr);
  if((combobuf[MAPCOMBO(x,y-16)].type==cCAVE)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVEB)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVEC)||(combobuf[MAPCOMBO(x,y-16)].type==cCAVED))
  {
    walkup();
  }
  else if((combobuf[MAPCOMBO(x,y+16)].type==cCAVE2)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2B)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2C)||(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2D))
    {
      walkdown2();
    }
    show_subscreen_life=true;
  show_subscreen_numbers=true;
  //play_DmapMusic();
  playLevelMusic();
  currcset=DMaps[currdmap].color;
  dointro();
  newscr_clk=frame;
  activated_timed_warp=false;
  dir=down;
  warpx=x;
  warpy=y;
  eat_buttons();
  didstuff=0;
  map_bkgsfx();
  loadside=dir^1;
}



void LinkClass::stepforward(int steps, bool adjust)
{

  int tx=x;           //temp x
  int ty=y;           //temp y
  int tstep=0;        //temp single step distance
  int s=0;            //calculated step distance for all steps
  z3step=2;
  int sh=shiftdir;
  shiftdir=-1;

  for (int i=steps; i>0; --i)
  {
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
      tstep=z3step;
      z3step=(z3step%2)+1;
    }
    else
    {
      tstep=lsteps[int((dir<left)?ty:tx)&7];

      switch(dir)
      {
        case up:    ty-=tstep; break;
        case down:  ty+=tstep; break;
        case left:  tx-=tstep; break;
        case right: tx+=tstep; break;
      }
    }
    s+=tstep;
  }

  z3step=2;

  while(s>=0)
  {
    if(get_bit(quest_rules,qr_LTTPWALK))
    {
      if((dir<left?int(x)&7:int(y)&7)&&adjust==true)
      {
        walkable=false;
        shiftdir=dir<left?(int(x)&8?left:right):(int(y)&8?down:up);
      }
      else {s-=z3step; walkable=true;}
      move(dir);
      shiftdir=-1;
      draw_screen(tmpscr, 0, 0);
      advanceframe();
      if(Quit)
        return;


    }
    else
    {
      if (dir<left)
      {
        s-=lsteps[int(y)&7];
      }
      else
      {
        s-=lsteps[int(x)&7];
      }
      move(dir);
      draw_screen(tmpscr, 0, 0);
      advanceframe();
      if(Quit)
        return;
    }
  }
  draw_screen(tmpscr, 0, 0);
  eat_buttons();
  logic_counter=0;
  drawit=true;
  shiftdir=sh;
}

void LinkClass::walkdown() //entering cave
{
  if(COOLSCROLL)
  {
    close_black_opening(x+8, y+8+playing_field_offset, false);
  }
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  clk=0;
  //  int cmby=(int(y)&0xF0)+16;
  action=climbcoverbottom;
  climb_cover_x=int(x)&0xF0;
  climb_cover_y=(int(y)&0xF0)+16;

  guys.clear();
  chainlinks.clear();
  Lwpns.clear();
  Ewpns.clear();
  items.clear();

  for(int i=0; i<64; i++)
  {
    /*if (FrameSkip)
      {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
      }*/
    linkstep();
    if((i&3)==3)
      ++y;
    draw_screen(tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
  logic_counter=0;
  drawit=true;
  action=none;
}

void LinkClass::walkdown2() //exiting cave 2
{
  if(COOLSCROLL)
  {
    open_black_opening(x+8, y+8+playing_field_offset+16, false);
  }
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  clk=0;
  //  int cmby=int(y)&0xF0;
  action=climbcovertop;
  climb_cover_x=int(x)&0xF0;
  climb_cover_y=int(y)&0xF0;

  guys.clear();
  chainlinks.clear();
  Lwpns.clear();
  Ewpns.clear();
  items.clear();

  for(int i=0; i<64; i++)
  {
    /*if (FrameSkip)
      {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
      }*/
    linkstep();
    if((i&3)==3)
      ++y;
    draw_screen(tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
  logic_counter=0;
  drawit=true;
  action=none;
}

void LinkClass::walkup() //exiting cave
{
  if(COOLSCROLL)
  {
    open_black_opening(x+8, y+8+playing_field_offset-16, false);
  }
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  dir=down;
  clk=0;
  //  int cmby=int(y)&0xF0;
  action=climbcoverbottom;
  climb_cover_x=int(x)&0xF0;
  climb_cover_y=int(y)&0xF0;

  guys.clear();
  chainlinks.clear();
  Lwpns.clear();
  Ewpns.clear();
  items.clear();

  for(int i=0; i<64; i++)
  {
    /*if (FrameSkip)
      {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
      }*/
    linkstep();
    if((i&3)==0)
      --y;
    draw_screen(tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
  map_bkgsfx();
  loadside=dir^1;
  logic_counter=0;
  drawit=true;
  action=none;
}

void LinkClass::walkup2() //entering cave2
{
  if(COOLSCROLL)
  {
    close_black_opening(x+8, y+8+playing_field_offset, false);
  }
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  dir=up;
  clk=0;
  //  int cmby=int(y)&0xF0;
  action=climbcovertop;
  climb_cover_x=int(x)&0xF0;
  climb_cover_y=(int(y)&0xF0)-16;

  guys.clear();
  chainlinks.clear();
  Lwpns.clear();
  Ewpns.clear();
  items.clear();

  for(int i=0; i<64; i++)
  {
    /*if (FrameSkip)
      {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
      }*/
    linkstep();
    if((i&3)==0)
      --y;
    draw_screen(tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
  map_bkgsfx();
  loadside=dir^1;
  logic_counter=0;
  drawit=true;
  action=none;
}

void LinkClass::stepout()
{
  ALLOFF();
  draw_screen(tmpscr, 0, 0);
  fade(11,true,false,false);
  blackscr(30,true);
  ringcolor();
  if(currscr==129 && x!=warpx)
  {
    currdmap=tmpscr[1].tilewarpdmap[0];
    currmap=DMaps[currdmap].map;
    dlevel=DMaps[currdmap].level;
    homescr=tmpscr[1].tilewarpscr[0]+DMaps[currdmap].xoff;
  }
  currscr=homescr;
  loadscr(0,currscr,255);                                   // bogus direction
  draw_screen(tmpscr, 0, 0);
  if ((tmpscr->flags&fDARK) == 0)
  {
    fade(DMaps[currdmap].color,true,true,false);
  }
  else
  {
    if(get_bit(quest_rules,qr_FADE))
    {
      int dpos = 64;
      int lpos = 32;
      int last = CSET(5)-1;

      if(get_bit(quest_rules,qr_FADECS5))
      {
        last += 16;
        loadpalset(5,5);
      }

      loadlvlpal(DMaps[currdmap].color);
      byte *si = colordata + CSET(DMaps[currdmap].color*pdLEVEL+poFADE1)*3;
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
      loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
    }
  }
  x = tmpscr->warpreturnx[0];
  y = tmpscr->warpreturny[0];
  if (didpit)
  {
    didpit=false;
    x=pitx;
    y=pity;
  }
  if(x+y == 0)
    x = y = 80;
  dir=down;
  newscr_clk=frame;
  activated_timed_warp=false;
  didstuff=0;
  warpx=warpy=0;
  eat_buttons();
  markBmap(-1);
  map_bkgsfx();
  loadside=dir^1;
}

bool edge_of_dmap(int side)
{
  // needs fixin'
  // should check dmap style

  switch(side)
  {
    case up:    return currscr<16;
    case down:  return currscr>=112;
    case left:
    if((currscr&15)==0)
      return true;
    if ((DMaps[currdmap].type&dmfTYPE)!=dmOVERW)
      //    if(dlevel)
      return (((currscr&15)-DMaps[currdmap].xoff)<=0);
    break;
    case right:
    if((currscr&15)==15)
      return true;
    if ((DMaps[currdmap].type&dmfTYPE)!=dmOVERW)
      //    if(dlevel)
      return (((currscr&15)-DMaps[currdmap].xoff)>=7);
    break;
  }
  return false;
}

bool LinkClass::nextcombo_wf(int d)
{
  if(toogam || action!=swimming || hopclk==0)
    return false;

  // assumes Link is about to scroll screens

  int ns = nextscr(d);
  if(ns==0xFFFF)
    return false;

  // want actual screen index, not game->maps[] index
  ns = (ns&127) + (ns>>7)*MAPSCRS;

  int cx = x;
  int cy = y;

  switch(d)
  {
    case up:    cy=160; break;
    case down:  cy=0; break;
    case left:  cx=240; break;
    case right: cx=0; break;
  }

  // check lower half of combo
  cy += 8;

  // from MAPCOMBO()
  int cmb = (cy&0xF0)+(cx>>4);
  if(cmb>175)
    return true;

  newcombo c = combobuf[TheMaps[ns].data[cmb]];
  bool dried = iswater_type(c.type) && (whistleclk>=88);
  bool swim = iswater_type(c.type) && (current_item(itype_flippers, true));
  int b=1;

  if(cx&8) b<<=2;
  if(cy&8) b<<=1;

  if((c.walk&b) && !dried && !swim)
    return true;

  // next block (i.e. cnt==2)
  if(!(cx&8))
  {
    b<<=2;
  }
  else
  {
    c = combobuf[TheMaps[ns].data[++cmb]];
    dried = iswater_type(c.type) && (whistleclk>=88);
    swim = iswater_type(c.type) && (current_item(itype_flippers, true));
    b=1;
    if(cy&8)
    {
      b<<=1;
    }
  }

  return (c.walk&b) ? !dried && !swim : false;
}

bool LinkClass::nextcombo_solid(int d)
{
  if(toogam)
    return false;

  // assumes Link is about to scroll screens

  int ns = nextscr(d);
  if(ns==0xFFFF)
    return false;

  // want actual screen index, not game->maps[] index
  ns = (ns&127) + (ns>>7)*MAPSCRS;

  int cx = x;
  int cy = y;

  switch(d)
  {
    case up:    cy=160; break;
    case down:  cy=0; break;
    case left:  cx=240; break;
    case right: cx=0; break;
  }

  if(d==up) cy += 8;
  if(d==left||d==right) cy+=get_bit(quest_rules,qr_LTTPCOLLISION)?0:8;

  // from MAPCOMBO()

  for(int i=0;i<=(((get_bit(quest_rules,qr_LTTPCOLLISION))&&!(d==up||d==down))?((cy&7)?2:1):((cy&7)?1:0));cy+=8,i++)
  {
  int cmb = (cy&0xF0)+(cx>>4);
  if(cmb>175)
    return true;

  newcombo c = combobuf[TheMaps[ns].data[cmb]];
  bool dried = iswater_type(c.type) && (whistleclk>=88);
  bool swim = iswater_type(c.type) && (current_item(itype_flippers, true));
  int b=1;

  if(cx&8) b<<=2;
  if(cy&8) b<<=1;

  if((c.walk&b) && !dried && !swim)
    return true;

  // next block (i.e. cnt==2)
  if(!(cx&8))
  {
    b<<=2;
  }
  else
  {
    c = combobuf[TheMaps[ns].data[++cmb]];
    dried = iswater_type(c.type) && (whistleclk>=88);
    swim = iswater_type(c.type) && (current_item(itype_flippers, true));
    b=1;
    if(cy&8)
    {
      b<<=1;
    }
  }

  if((c.walk&b) && !dried && !swim)
    return true;

  cx+=8;
  if(cx&7)
  {
	if(!(cx&8))
    {
      b<<=2;
    }
    else
    {
      c = combobuf[TheMaps[ns].data[++cmb]];
      dried = iswater_type(c.type) && (whistleclk>=88);
      swim = iswater_type(c.type) && (current_item(itype_flippers, true));
      b=1;
      if(cy&8)
      {
        b<<=1;
      }
	}

	if((c.walk&b) && !dried && !swim)
    return true;
  }

  }
  return false;
}

void LinkClass::checkscroll()
{
  if(toogam)
  {
    if(x<0 && (currscr&15)==0) x=0;
    if(y<0 && currscr<16) y=0;
    if(x>240 && (currscr&15)==15) x=240;
    if(y>160 && currscr>=112) y=160;
  }

  if(y<0)
  {
    bool doit=true;
    y=0;
    if(nextcombo_wf(up))
      doit=false;
	if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)))
	{
	  if(nextcombo_solid(up))
	    doit=false;
	}
	if(doit)
	{
    if(currscr>=128)
      stepout();
    else if(tmpscr->flags2&wfUP)
      { sdir=up; dowarp(1,(tmpscr->sidewarpindex)&3); }
      else if(!edge_of_dmap(up))
        {
          scrollscr(up);
          if (tmpscr->flags4&fAUTOSAVE)
          {
            set_gamedata_continue_scrn(game, homescr);
            set_gamedata_continue_dmap(game, currdmap);
            lastentrance_dmap = currdmap;
            lastentrance = get_gamedata_continue_scrn(game);
          }
		  if (tmpscr->flags6&fCONTINUEHERE)
		  {
			lastentrance_dmap = currdmap;
            lastentrance = homescr;
		  }
        }
	}
  }
  if(y>160)
  {
	bool doit=true;
    y=160;
    if(nextcombo_wf(down))
      doit=false;
	if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)))
	{
	  if(nextcombo_solid(down))
	    doit=false;
	}
	if(doit)
	{
    if(currscr>=128)
      exitcave();
    else if(tmpscr->flags2&wfDOWN)
      { sdir=down; dowarp(1,(tmpscr->sidewarpindex>>2)&3); }
      else if(!edge_of_dmap(down))
        {
          scrollscr(down);
          if (tmpscr->flags4&fAUTOSAVE)
          {
            set_gamedata_continue_scrn(game, homescr);
            set_gamedata_continue_dmap(game, currdmap);
            lastentrance_dmap = currdmap;
            lastentrance = get_gamedata_continue_scrn(game);
          }
		  if (tmpscr->flags6&fCONTINUEHERE)
		  {
			lastentrance_dmap = currdmap;
            lastentrance = homescr;
		  }
        }
	}
  }
  if(x<0)
  {
    bool doit=true;
    x=0;
    if(nextcombo_wf(left))
      doit=false;
	if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)))
	{
	  if(nextcombo_solid(left))
	    doit=false;
	}
	if(doit)
	{
    if(tmpscr->flags2&wfLEFT)
    { sdir=left; dowarp(1,(tmpscr->sidewarpindex>>4)&3); }
    else if(!edge_of_dmap(left))
      {
        scrollscr(left);
        if (tmpscr->flags4&fAUTOSAVE)
        {
          set_gamedata_continue_scrn(game, homescr);
          set_gamedata_continue_dmap(game, currdmap);
          lastentrance_dmap = currdmap;
          lastentrance = get_gamedata_continue_scrn(game);
        }
		if (tmpscr->flags6&fCONTINUEHERE)
		  {
			lastentrance_dmap = currdmap;
            lastentrance = homescr;
		  }
      }
	}
  }
  if(x>240)
  {
    bool doit=true;
    x=240;
    if(nextcombo_wf(right))
      doit=false;
	if(get_bit(quest_rules, qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)))
	{
	  if(nextcombo_solid(right))
	    doit=false;
	}
	if(doit)
	{
    if(action==inwind)
      dowarp(2,0);
    else if(tmpscr->flags2&wfRIGHT)
      {sdir=right; dowarp(1,(tmpscr->sidewarpindex>>6)&3); }
      else if(!edge_of_dmap(right))
        {
          scrollscr(right);
          if (tmpscr->flags4&fAUTOSAVE)
          {
            set_gamedata_continue_scrn(game, homescr);
            set_gamedata_continue_dmap(game, currdmap);
            lastentrance_dmap = currdmap;
            lastentrance = get_gamedata_continue_scrn(game);
          }
		  if (tmpscr->flags6&fCONTINUEHERE)
		  {
			lastentrance_dmap = currdmap;
            lastentrance = homescr;
		  }
        }
	}
  }
}

// assumes current direction is in lastdir[3]
// compares directions with scr->path and scr->exitdir
bool LinkClass::checkmaze(mapscr *scr)
{
  if(!(scr->flags&fMAZE))
    return true;
  if(lastdir[3]==scr->exitdir)
    return true;
  for(int i=0; i<4; i++)
    if(lastdir[i]!=scr->path[i])
      return false;
    if(!nosecretsounds)
  {
    sfx(WAV_SECRET);
  }
  return true;
}

int LinkClass::lookahead(int destscr, int d)                       // Helper for scrollscr that gets next combo on next screen.
{
  // Can use destscr for scrolling warps,
  // but assumes currmap is correct.

  int s = currscr;
  int cx = x;
  int cy = y + 8;

  switch(d)
  {
    case up:    s-=16; cy=160; break;
    case down:  s+=16; cy=0; break;
    case left:  --s; cx=240; break;
    case right: ++s; cx=0; break;
  }

  if(destscr != -1)
    s = destscr;

  int combo = (cy&0xF0)+(cx>>4);
  if(combo>175)
    return 0;
  return TheMaps[currmap*MAPSCRS+s].data[combo];            // entire combo code
}

int LinkClass::lookaheadflag(int destscr, int d)
{                                                           // Helper for scrollscr that gets next combo on next screen.
                                                            // Can use destscr for scrolling warps,
                                                            // but assumes currmap is correct.

  int s = currscr;
  int cx = x;
  int cy = y + 8;

  switch(d)
  {
    case up:    s-=16; cy=160; break;
    case down:  s+=16; cy=0; break;
    case left:  --s; cx=240; break;
    case right: ++s; cx=0; break;
  }

  if(destscr != -1)
    s = destscr;

  int combo = (cy&0xF0)+(cx>>4);
  if(combo>175)
    return 0;
  if (!TheMaps[currmap*MAPSCRS+s].sflag[combo])
  {
    return combobuf[TheMaps[currmap*MAPSCRS+s].data[combo]].flag;           // flag
  }
  return TheMaps[currmap*MAPSCRS+s].sflag[combo];           // flag
}

void LinkClass::scrollscr(int dir, int destscr, int destdmap)
{
  screenscrolling=true;
  tmpscr[1] = tmpscr[0];
  memcpy(tmpscr3, tmpscr2, sizeof(mapscr)*6);
  mapscr *newscr = &tmpscr[0];
  mapscr *oldscr = &tmpscr[1];
  int sx=0, sy=0, tx=0, ty=0, tx2=0, ty2=0;
  int cx=0, step = (isdungeon() && !get_bit(quest_rules,qr_FASTDNGN)) ? 2 : 4;
  int scx = get_bit(quest_rules,qr_FASTDNGN) ? 30 : 0;

  for(int i=0; i<3; i++)
    lastdir[i]=lastdir[i+1];
  lastdir[3] = oldscr->flags&fMAZE ? dir : -1;

  actiontype lastaction = action;
  ALLOFF();

  int ahead = lookahead(destscr, dir);
  int aheadflag = lookaheadflag(destscr, dir);

  if (lastaction!=inwind)
  {
    if(lastaction==rafting && aheadflag==mfRAFT)
    {
      action=rafting;
    }
    else if(iswater(ahead) && (current_item(itype_flippers, true)))
      {
        if(lastaction==swimming)
        {
          action = swimming;
          hopclk = 0xFF;
        }
        else
        {
          action = hopping;
          hopclk = 2;
        }
      }
  }
  lstep=(lstep+6)%12;
  cx = scx;
  do
  {
    draw_screen(tmpscr, 0, 0);
    if(cx==scx)
      rehydratelake();
    advanceframe();
    if(Quit)
    {
      screenscrolling=false;
      return;
    }
    ++cx;
  } while(cx<32);

  if((DMaps[currdmap].type&dmfTYPE)==dmCAVE)
    markBmap(dir);

  switch(dir)
  {
    case up:
    if (fixed_door)
    {
      unsetmapflag(mSECRET);
    }
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
        currscr-=16;
      loadscr(0,currscr,dir);
    blit(scrollbuf,scrollbuf,0,0,0,176,256,176);
    putscr(scrollbuf,0,0,newscr);
    sy=176;
    cx=176/step;
    break;

    case down:
    if (fixed_door)
    {
      unsetmapflag(mSECRET);
    }
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
        currscr+=16;
      loadscr(0,currscr,dir);
    putscr(scrollbuf,0,176,newscr);
    cx=176/step;
    break;

    case left:
    if (fixed_door)
    {
      unsetmapflag(mSECRET);
    }
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
        --currscr;
      loadscr(0,currscr,dir);
    blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
    putscr(scrollbuf,0,0,newscr);
    sx=256;
    cx=256/step;
    break;

    case right:
    if (fixed_door)
    {
      unsetmapflag(mSECRET);
    }
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
        ++currscr;
      loadscr(0,currscr,dir);
    putscr(scrollbuf,256,0,newscr);
    cx=256/step;
    break;
  }

  fixed_door=false;
  lighting(2,dir);
  if(!(newscr->flags&fSEA))
    adjust_sfx(WAV_SEA,128,false);
  if(!(newscr->flags&fROAR))
  {
    adjust_sfx(WAV_ROAR,128,false);
    adjust_sfx(WAV_VADER,128,false);
    adjust_sfx(WAV_DODONGO,128,false);
  }

  while(cx>0)
  {
    switch(dir)
    {
      case up:    sy-=step; break;
      case down:  sy+=step; break;
      case left:  sx-=step; break;
      case right: sx+=step; break;
    }
    if (ladderx+laddery)
    {
      if(ladderdir==up)
      {
        ladderx = int(x);
        laddery = int(y);
      }
      else
      {
        ladderx = int(x);
        laddery = int(y);
      }
    }
	switch(dir)
	{
	  case up:
      if(newscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, newscr, 0, playing_field_offset, 2);
      if(oldscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, oldscr, 0, -176+playing_field_offset, 3);
      if(newscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, newscr, 0, playing_field_offset, 2);
      if(oldscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, oldscr, 0, -176+playing_field_offset, 3);
      putscr(scrollbuf, 0, 0, newscr);
      putscr(scrollbuf, 0, 176, oldscr);
      break;
	  case down:
      if(newscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, newscr, 0, -176+playing_field_offset, 2);
      if(oldscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, oldscr, 0, playing_field_offset, 3);
      if(newscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, newscr, 0, -176+playing_field_offset, 2);
      if(oldscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, oldscr, 0, playing_field_offset, 3);
      putscr(scrollbuf, 0, 0, oldscr);
      putscr(scrollbuf, 0, 176, newscr);
      break;
	  case left:
      if(newscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, newscr, 0, playing_field_offset, 2);
      if(oldscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, oldscr, -256, playing_field_offset, 3);
      if(newscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, newscr, 0, playing_field_offset, 2);
      if(oldscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, oldscr, -256, playing_field_offset, 3);
      putscr(scrollbuf, 0, 0, newscr);
      putscr(scrollbuf, 256, 0, oldscr);
      break;
	  case right:
      if(newscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, newscr, -256, playing_field_offset, 2);
      if(oldscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, oldscr, 0, playing_field_offset, 3);
      if(newscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, newscr, -256, playing_field_offset, 2);
      if(oldscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, oldscr, 0, playing_field_offset, 3);
      putscr(scrollbuf, 0, 0, oldscr);
      putscr(scrollbuf, 256, 0, newscr);
      break;
	}
	blit(scrollbuf,framebuf,sx,sy,0,playing_field_offset,256,168);
    switch(dir)
    {
      case up:    if(y<160) y+=step; break;
      case down:  if(y>0)   y-=step; break;
      case left:  if(x<240) x+=step; break;
      case right: if(x>0)   x-=step; break;
    }
    tx=sx;
    if (dir==right)
    {
      tx-=256;
    }
    ty=sy;
    if (dir==down)
    {
      ty-=176;
    }
    tx2=sx;
    if (dir==left)
    {
      tx2-=256;
    }
    ty2=sy;
    if (dir==up)
    {
      ty2-=176;
    }

    do_layer(framebuf,0, oldscr, tx2, ty2, 3);
    if(!(oldscr->flags7&fLAYER2BG)) do_layer(framebuf,1, oldscr, tx2, ty2, 3);
    do_layer(framebuf,0, newscr, tx, ty, 2);
    if(!(newscr->flags7&fLAYER2BG)) do_layer(framebuf,1, newscr, tx, ty, 2);
    do_layer(framebuf,-2, oldscr, tx2, ty2, 3);
    do_layer(framebuf,-2, newscr, tx, ty, 2);
    linkstep();
    if(!isdungeon()||get_bit(quest_rules,qr_FREEFORM))
    {
      draw_under(framebuf);
      draw(framebuf);
    }
    if(!(oldscr->flags7&fLAYER3BG)) do_layer(framebuf,2, oldscr, tx2, ty2, 3);
    do_layer(framebuf,3, oldscr, tx2, ty2, 3);
    do_layer(framebuf,-1, oldscr, tx2, ty2, 3);
    do_layer(framebuf,4, oldscr, tx2, ty2, 3);
    do_layer(framebuf,5, oldscr, tx2, ty2, 3);

    if(!(newscr->flags7&fLAYER3BG)) do_layer(framebuf,2, newscr, tx, ty, 2);
    do_layer(framebuf,3, newscr, tx, ty, 2);
    do_layer(framebuf,-1, newscr, tx, ty, 2);
    do_layer(framebuf,4, newscr, tx, ty, 2);
    do_layer(framebuf,5, newscr, tx, ty, 2);

    put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,false);

    advanceframe();
    if(Quit)
    {
      screenscrolling=false;
      return;
    }
    --cx;
  }

  screenscrolling=false;
  if(destdmap != -1)
    currdmap = destdmap;

  lighting(3,dir);
  homescr=currscr;
  putscr(scrollbuf,0,0,newscr);

  if((MAPFLAG(x,y)==mfRAFT||MAPCOMBOFLAG(x,y)==mfRAFT) && action!=rafting && hopclk==0)
  {
    if (!nosecretsounds)
    {
      sfx(WAV_SECRET);
    }
    action=rafting;
  }

  opendoors=0;
  markBmap(-1);

  if(isdungeon())
  {
    switch(tmpscr->door[dir^1])
    {
      case dOPEN:
      case dUNLOCKED:
      case dOPENBOSS:
      this->dir = dir;
      stepforward(get_bit(quest_rules,qr_LTTPWALK)?11:12, false);
      break;
      case dSHUTTER:
      case d1WAYSHUTTER:
      this->dir = dir;
      stepforward(get_bit(quest_rules,qr_LTTPWALK)?21:24, false);
      putdoor(0,dir^1,tmpscr->door[dir^1]);
      opendoors=-4;
      sfx(WAV_DOOR);
      break;
      default:
      this->dir = dir;
      stepforward(get_bit(quest_rules,qr_LTTPWALK)?21:24, false);
    }
  }

  if(action==scrolling)
    action=none;

  map_bkgsfx();
  if(newscr->flags2&fSECRET)
  {
    sfx(WAV_SECRET);
  }

  playLevelMusic();
  memset(ffposx,0xFF,sizeof(short)*32);
  memset(ffposy,0xFF,sizeof(short)*32);
  memset(ffprvx,0xFF,sizeof(float)*32);
  memset(ffprvy,0xFF,sizeof(float)*32);

  newscr_clk = frame;
  activated_timed_warp=false;
  loadside = dir^1;
  logic_counter=0;
}

/************************************/
/********  More Items Code  *********/
/************************************/

int Bweapon(int pos)
{
  int p=-1;
  for (int i=0; current_subscreen_active->objects[i].type!=ssoNULL; ++i)
  {
    if (current_subscreen_active->objects[i].type==ssoCURRENTITEM && current_subscreen_active->objects[i].d3==pos)
    {
      p=i;
      break;
    }
  }
  if (p==-1)
  {
    return 0;
  }

  switch (current_subscreen_active->objects[p].d1)
  {
    case ssiBRANG:
    if(current_item(itype_brang,true)==3)
    {
      return iFBrang;
    }
    if(current_item(itype_brang,true))
    {
      return current_item(itype_brang,true)-1+iBrang;
    }
    break;
    case ssiBOMB:
    if(current_item(itype_bomb,true))
    {
      return iBombs;
    }
    break;
    case ssiBOWANDARROW:
    case ssiARROW:
    if(current_item(itype_bow,true) && current_item(itype_arrow,true))
    {
      bool bow=(current_subscreen_active->objects[p].d1==ssiBOWANDARROW);
      if (current_item(itype_arrow,true)<3)
      {
        return current_item(itype_arrow,true)-1+iArrow+(bow?0xF000:0);
      }
      else
      {
        return iGArrow+(bow?0xF000:0);
      }
    }
    break;
    case ssiCANDLE:
    if(current_item(itype_candle,true))
    {
      return current_item(itype_candle,true)-1+iBCandle;
    }
    break;
    case ssiDINSFIRE:
    if(current_item(itype_dinsfire,true))
    {
      return iDinsFire;
    }
    break;
    case ssiWHISTLE:
    if(current_item(itype_whistle,true))
    {
      return iWhistle;
    }
    break;
    case ssiBAIT:
    if(current_item(itype_bait,true))
    {
      return iBait;
    }
    break;
    case ssiLETTERPOTION:
    if(current_item(itype_potion,true))
    {
      return current_item(itype_potion,true)-1+iBPotion;
    }
    if(current_item(itype_letter,true))
    {
      return iLetter;
    }
    break;
    case ssiPOTION:
    if(current_item(itype_potion,true))
    {
      return current_item(itype_potion,true)-1+iBPotion;
    }
    break;
    case ssiLETTER:
    if(current_item(itype_letter,true))
    {
      return iLetter;
    }
    break;
    case ssiWAND:
    if(current_item(itype_wand,true))
    {
      return iWand;
    }
    break;
    case ssiFARORESWIND:
    if(current_item(itype_faroreswind,true))
    {
      return iFaroresWind;
    }
    break;
    case ssiHOOKSHOT:
    if(current_item(itype_hookshot, true))
    {
      if(current_item(itype_hookshot, true)==2)
      {
        return iLongshot;
      }
      return iHookshot;
    }
    break;
    case ssiSBOMB:
    if(current_item(itype_sbomb,true))
    {
      return iSBomb;
    }
    break;
    case ssiLENS:
    if(current_item(itype_lens, true))
    {
      return iLens;
    }
    break;
    case ssiHAMMER:
    if(current_item(itype_hammer, true))
    {
      return iHammer;
    }
    break;
    case ssiNAYRUSLOVE:
    if(current_item(itype_nayruslove,true))
    {
      return iNayrusLove;
    }
    break;
  }
  return 0;
}

void selectAwpn(int step)
{
  // change this for selectable Awpn
  switch(current_item(itype_sword,true))
  {
    case 1:
    case 2:
    case 3:
    Awpn = current_item(itype_sword,true) - 1 + iSword; break;
    case 4:
    Awpn = iXSword; break;
    default:
    Awpn = 0;
  }
}

void selectBwpn(int xstep, int ystep)
{
  if((xstep==0)&&(ystep==0))
  {
    Bwpn=Bweapon(Bpos);
    update_subscr_items();
    if(Bwpn)
    {
      return;
    }
    xstep=1;
  }

  if((xstep==8)&&(ystep==8))
  {
    Bwpn=Bweapon(Bpos);
    update_subscr_items();
    if(Bwpn)
    {
      return;
    }
    xstep=-1;
  }

  int pos = Bpos;

  do
  {
    int p=-1;
    for (int i=0; current_subscreen_active->objects[i].type!=ssoNULL; ++i)
    {
      if (current_subscreen_active->objects[i].type==ssoCURRENTITEM)
      {
        if (current_subscreen_active->objects[i].d3==Bpos)
        {
          p=i;
          break;
        }
        else
        {
        }
      }
    }
    if (p!=-1)
    {
      if (xstep!=0)
      {
        Bpos=xstep<0?current_subscreen_active->objects[p].d6:current_subscreen_active->objects[p].d7;
      }
      else
      {
        Bpos=ystep<0?current_subscreen_active->objects[p].d4:current_subscreen_active->objects[p].d5;
      }
    }

    Bwpn = Bweapon(Bpos);
    if(Bwpn)
    {
      return;
    }
  } while(Bpos!=pos);

  if(!Bwpn)
    Bpos=0;
}

bool canget(int id)
{
  if(id==iSword && get_gamedata_maxlife(game)<swordhearts[0]*HP_PER_HEART)
    return false;
  if(id==iWSword && get_gamedata_maxlife(game)<swordhearts[1]*HP_PER_HEART)
    return false;
  if(id==iMSword && get_gamedata_maxlife(game)<swordhearts[2]*HP_PER_HEART)
    return false;
  if(id==iXSword && get_gamedata_maxlife(game)<swordhearts[3]*HP_PER_HEART)
    return false;
  return true;
}

void dospecialmoney(int index)
{
  int tmp=currscr>=128?1:0;
  switch(tmpscr[tmp].room)
  {
    case rINFO:                                             // pay for info
    if(get_gamedata_rupies(game) < abs(prices[index-1][0]))
      return;
    change_gamedata_drupy(game, -abs(prices[index-1][0]));
    msgstr = QMisc.info[tmpscr[tmp].catchall].str[index-1];
    msgclk=msgpos=0;
    rectfill(msgdisplaybuf, 0, 0, msgdisplaybuf->w, 80, 0);
    clear_bitmap(pricesdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
    items.del(0);
    for(int i=0; i<items.Count(); i++)
      ((item*)items.spr(i))->pickup=ipDUMMY;
    break;

    case rMONEY:                                            // secret money
    ((item*)items.spr(0))->pickup=ipDUMMY;
    change_gamedata_drupy(game, (prices[0][0]=tmpscr[tmp].catchall));
    putprices(false);
    setmapflag();
    break;

    case rGAMBLE:                                           // gamble
    {
      if(get_gamedata_rupies(game)<10) return;
      unsigned si=(rand()%24)*3;
      for(int i=0; i<3; i++)
        prices[i][0]=gambledat[si++];
      change_gamedata_drupy(game, prices[index-1][0]);
      putprices(true);
      for(int i=1; i<4; i++)
        ((item*)items.spr(i))->pickup=ipDUMMY;
    }break;

    case rBOMBS:
    if(get_gamedata_rupies(game)<abs(tmpscr[tmp].catchall))
      return;
    change_gamedata_drupy(game, -abs(tmpscr[tmp].catchall));
    setmapflag();
    change_gamedata_maxbombs(game, 4);
    game->items[itype_bomb]=get_gamedata_maxbombs(game);
    ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
    fadeclk=66;
    msgstr=0;
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    clear_bitmap(pricesdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
    //    putscr(scrollbuf,0,0,tmpscr);
    selectBwpn(0,0);
    break;

    case rARROWS:
    if(get_gamedata_rupies(game)<abs(tmpscr[tmp].catchall))
      return;
    change_gamedata_drupy(game, -abs(tmpscr[tmp].catchall));
    setmapflag();
    change_gamedata_maxarrows(game, 10);
    set_gamedata_arrows(game,get_gamedata_maxarrows(game));
    ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
    fadeclk=66;
    msgstr=0;
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    clear_bitmap(pricesdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
    //    putscr(scrollbuf,0,0,tmpscr);
    selectBwpn(0,0);
    break;

    case rSWINDLE:
    if(items.spr(index)->id==iRupy)
    {
      if(get_gamedata_rupies(game)<abs(tmpscr[tmp].catchall))
        return;
      change_gamedata_drupy(game, -abs(tmpscr[tmp].catchall));
    }
    else
    {
      if(get_gamedata_maxlife(game)<=HP_PER_HEART)
        return;
      set_gamedata_life(game, max(get_gamedata_life(game)-HP_PER_HEART,0));
      set_gamedata_maxlife(game, max(get_gamedata_maxlife(game)-HP_PER_HEART,(HP_PER_HEART)));
    }
    setmapflag();
    ((item*)items.spr(0))->pickup=ipDUMMY+ipFADE;
    ((item*)items.spr(1))->pickup=ipDUMMY+ipFADE;
    fadeclk=66;
    msgstr=0;
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    clear_bitmap(pricesdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
    //    putscr(scrollbuf,0,0,tmpscr);
    break;
  }
}

void computeMaxArrows(void)
{
	switch(game->items[itype_quiver])
	{
	case 3:
		set_gamedata_maxarrows(game,max(get_gamedata_maxarrows(game),99));
	case 2:
		set_gamedata_maxarrows(game,max(get_gamedata_maxarrows(game),60));
	case 1:
		set_gamedata_maxarrows(game,max(get_gamedata_maxarrows(game),30));
	}
}

void getitem(int id)
{
  switch(id)
  {
    case iRupy:         change_gamedata_drupy(game, 1);  break;
    case i5Rupies:      change_gamedata_drupy(game, 5);  break;
	case i10Rupies:     change_gamedata_drupy(game, 10); break;
    case i20Rupies:     change_gamedata_drupy(game, 20); break;
    case i50Rupies:     change_gamedata_drupy(game, 50); break;
	case i100Rupies:    change_gamedata_drupy(game, 100); break;
    case i200Rupies:    change_gamedata_drupy(game, 200); break;
    case iWallet500:    game->items[itype_wallet]|=i_swallet; break;
    case iWallet999:    game->items[itype_wallet]|=i_lwallet; break;
    case iBombs:        set_gamedata_bombs(game,min(get_gamedata_bombs(game)+4,get_gamedata_maxbombs(game))); break;
    case iSBomb:        set_gamedata_sbombs(game,min(get_gamedata_sbombs(game)+1,get_gamedata_maxbombs(game)>>2)); break;
    case i1ArrowAmmo:
    set_gamedata_arrows(game,min(get_gamedata_arrows(game)+1,get_gamedata_maxarrows(game)));
    break;
    case i5ArrowAmmo:
    set_gamedata_arrows(game,min(get_gamedata_arrows(game)+5,get_gamedata_maxarrows(game)));
    break;
    case i10ArrowAmmo:
    set_gamedata_arrows(game,min(get_gamedata_arrows(game)+10,get_gamedata_maxarrows(game)));
    break;
    case i30ArrowAmmo:
    set_gamedata_arrows(game,min(get_gamedata_arrows(game)+30,get_gamedata_maxarrows(game)));
    break;
    case iQuiver:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_quiver,true)<i_quiver)
      {
        game->items[itype_quiver]=(1<<(i_quiver-1));
      }
    }
    else
    {
      game->items[itype_quiver]|=(1<<(i_quiver-1));
    }
    //set_gamedata_maxarrows(game,max(get_gamedata_maxarrows(game),30));
	computeMaxArrows();
    break;
    case iQuiverL2:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_quiver,true)<i_quiverl2)
      {
        game->items[itype_quiver]=(1<<(i_quiverl2-1));
      }
    }
    else
    {
      game->items[itype_quiver]|=(1<<(i_quiverl2-1));
    }
    //set_gamedata_maxarrows(game,max(get_gamedata_maxarrows(game),60));
	computeMaxArrows();
    break;
    case iQuiverL3:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_quiver,true)<i_quiverl3)
      {
        game->items[itype_quiver]=(1<<(i_quiverl3-1));
      }
    }
    else
    {
      game->items[itype_quiver]|=(1<<(i_quiverl3-1));
    }
    //set_gamedata_maxarrows(game,max(get_gamedata_maxarrows(game),99));
	computeMaxArrows();
    break;
    case iClock:
    {
      setClock(watch=true);
      clock_zoras=0;
      if (get_bit(quest_rules,qr_TEMPCLOCKS))
      {
        clockclk=0;
      }
    } break;
    case iSword:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_sword,true)<i_sword)
      {
        game->items[itype_sword]=(1<<(i_sword-1));
      }
    }
    else
    {
      game->items[itype_sword]|=(1<<(i_sword-1));
    }
    break;
    case iWSword:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_sword,true)<i_wsword)
      {
        game->items[itype_sword]=(1<<(i_wsword-1));
      }
    }
    else
    {
      game->items[itype_sword]|=(1<<(i_wsword-1));
    }
    break;
    case iMSword:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_sword,true)<i_msword)
      {
        game->items[itype_sword]=(1<<(i_msword-1));
      }
    }
    else
    {
      game->items[itype_sword]|=(1<<(i_msword-1));
    }
    break;
    case iXSword:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_sword,true)<i_xsword)
      {
        game->items[itype_sword]=(1<<(i_xsword-1));
      }
    }
    else
    {
      game->items[itype_sword]|=(1<<(i_xsword-1));
    }
    break;
    case iKey:          if(get_gamedata_keys(game)<255) change_gamedata_keys(game, 1); break;
    case iLevelKey:		if(game->lvlkeys[dlevel]<255) game->lvlkeys[dlevel]++; break;
    case iBCandle:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_candle,true)<i_bcandle)
      {
        game->items[itype_candle]=(1<<(i_bcandle-1));
      }
    }
    else
    {
      game->items[itype_candle]|=(1<<(i_bcandle-1));
    }
    break;
    case iRCandle:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_candle,true)<i_rcandle)
      {
        game->items[itype_candle]=(1<<(i_rcandle-1));
      }
    }
    else
    {
      game->items[itype_candle]|=(1<<(i_rcandle-1));
    }
    break;
    case iArrow:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_arrow,true)<i_warrow)
      {
        game->items[itype_arrow]=(1<<(i_warrow-1));
      }
    }
    else
    {
      game->items[itype_arrow]|=(1<<(i_warrow-1));
    }
    break;
    case iSArrow:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_arrow,true)<i_sarrow)
      {
        game->items[itype_arrow]=(1<<(i_sarrow-1));
      }
    }
    else
    {
      game->items[itype_arrow]|=(1<<(i_sarrow-1));
    }
    break;
    case iGArrow:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_arrow,true)<i_garrow)
      {
        game->items[itype_arrow]=(1<<(i_garrow-1));
      }
    }
    else
    {
      game->items[itype_arrow]|=(1<<(i_garrow-1));
    }
    break;
    case iBRing:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_ring,true)<i_bring)
      {
        game->items[itype_ring]=(1<<(i_bring-1));
      }
    }
    else
    {
      game->items[itype_ring]|=(1<<(i_bring-1));
    }
    if(currscr<128 || dlevel)
    {
      ringcolor();
    }
    break;
    case iRRing:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_ring,true)<i_rring)
      {
        game->items[itype_ring]=(1<<(i_rring-1));
      }
    }
    else
    {
      game->items[itype_ring]|=(1<<(i_rring-1));
    }
    if(currscr<128 || dlevel)
    {
      ringcolor();
    }
    break;
    case iGRing:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_ring,true)<i_gring)
      {
        game->items[itype_ring]=(1<<(i_gring-1));
      }
    }
    else
    {
      game->items[itype_ring]|=(1<<(i_gring-1));
    }
    if(currscr<128 || dlevel)
    {
      ringcolor();
    }
    break;
    case iBrang:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_brang,true)<i_wbrang)
      {
        game->items[itype_brang]=(1<<(i_wbrang-1));
      }
    }
    else
    {
      game->items[itype_brang]|=(1<<(i_wbrang-1));
    }
    break;
    case iMBrang:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_brang,true)<i_mbrang)
      {
        game->items[itype_brang]=(1<<(i_mbrang-1));
      }
    }
    else
    {
      game->items[itype_brang]|=(1<<(i_mbrang-1));
    }
    break;
    case iFBrang:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_brang,true)<i_fbrang)
      {
        game->items[itype_brang]=(1<<(i_fbrang-1));
      }
    }
    else
    {
      game->items[itype_brang]|=(1<<(i_fbrang-1));
    }
    break;
    case iBPotion:      game->items[itype_potion]=min(current_item(itype_potion,true)+1,2); break;
    case iRPotion:      game->items[itype_potion]=2; break;
    case iBracelet:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_bracelet,true)<i_bracelet1)
      {
        game->items[itype_bracelet]=(1<<(i_bracelet1-1));
      }
    }
    else
    {
      game->items[itype_bracelet]|=(1<<(i_bracelet1-1));
    }
    break;
    case iRaft:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_raft,true)<i_raft)
      {
        game->items[itype_raft]=(1<<(i_raft-1));
      }
    }
    else
    {
      game->items[itype_raft]|=(1<<(i_raft-1));
    }
    break;
    case iLadder:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_ladder,true)<i_ladder)
      {
        game->items[itype_ladder]=(1<<(i_ladder-1));
      }
    }
    else
    {
      game->items[itype_ladder]|=(1<<(i_ladder-1));
    }
    break;
    case iBow:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_bow,true)<i_shortbow)
      {
        game->items[itype_bow]=(1<<(i_shortbow-1));
      }
    }
    else
    {
      game->items[itype_bow]|=(1<<(i_shortbow-1));
    }
    break;
    case iBow2:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_bow,true)<i_longbow)
      {
        game->items[itype_bow]=(1<<(i_longbow-1));
      }
    }
    else
    {
      game->items[itype_bow]|=(1<<(i_longbow-1));
    }
    break;
    case iBook:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_book,true)<i_book)
      {
        game->items[itype_book]=(1<<(i_book-1));
      }
    }
    else
    {
      game->items[itype_book]|=(1<<(i_book-1));
    }
    break;
    case iShield:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_shield,true)<i_largeshield)
      {
        game->items[itype_shield]=(1<<(i_largeshield-1));
      }
    }
    else
    {
      game->items[itype_shield]|=(1<<(i_largeshield-1));
    }
    break;
    case iMShield:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_shield,true)<i_mirrorshield)
      {
        game->items[itype_shield]=(1<<(i_mirrorshield-1));
      }
    }
    else
    {
      game->items[itype_shield]|=(1<<(i_mirrorshield-1));
    }
    break;
    case iMKey:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_magickey,true)<i_magickey)
      {
        game->items[itype_magickey]=(1<<(i_magickey-1));
      }
    }
    else
    {
      game->items[itype_magickey]|=(1<<(i_magickey-1));
    }
    break;
    case iMap:          game->lvlitems[dlevel]|=liMAP; break;
    case iCompass:      game->lvlitems[dlevel]|=liCOMPASS; break;
    case iBossKey:      game->lvlitems[dlevel]|=liBOSSKEY; break;
    case iLetter:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_letter,true)<i_letter)
      {
        game->items[itype_letter]=(1<<(i_letter-1));
      }
    }
    else
    {
      game->items[itype_letter]|=(1<<(i_letter-1));
    }
    break;
    case iBait:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_bait,true)<i_bait)
      {
        game->items[itype_bait]=(1<<(i_bait-1));
      }
    }
    else
    {
      game->items[itype_bait]|=(1<<(i_bait-1));
    }
    break;
    case iWand:         game->items[itype_wand]|=i_wand; break;
    case iWhistle:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_whistle,true)<i_recorder)
      {
        game->items[itype_whistle]=(1<<(i_recorder-1));
      }
    }
    else
    {
      game->items[itype_whistle]|=(1<<(i_recorder-1));
    }
    break;
    case iFairyStill:
      set_gamedata_life(game, min(get_gamedata_life(game)+(zinit.stationary_fairy_heart_percent?(get_gamedata_maxlife(game)*zinit.stationary_fairy_hearts/100):(get_gamedata_life(game)+(zinit.stationary_fairy_hearts*HP_PER_HEART))),get_gamedata_maxlife(game)));
      set_gamedata_magic(game, min(get_gamedata_magic(game)+(zinit.stationary_fairy_magic_percent?(get_gamedata_maxmagic(game)*zinit.stationary_fairy_magic/100):(get_gamedata_magic(game)+(zinit.stationary_fairy_magic*MAGICPERBLOCK))),get_gamedata_maxmagic(game)));
      break;
    case iFairyMoving:
      set_gamedata_life(game, min(get_gamedata_life(game)+(zinit.moving_fairy_heart_percent?(get_gamedata_maxlife(game)*zinit.moving_fairy_hearts/100):(get_gamedata_life(game)+(zinit.moving_fairy_hearts*HP_PER_HEART))),get_gamedata_maxlife(game)));
      set_gamedata_magic(game, min(get_gamedata_magic(game)+(zinit.moving_fairy_magic_percent?(get_gamedata_maxmagic(game)*zinit.moving_fairy_magic/100):(get_gamedata_magic(game)+(zinit.moving_fairy_magic*MAGICPERBLOCK))),get_gamedata_maxmagic(game)));
      break;
    case iAmulet:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_amulet,true)<i_amulet1)
      {
        game->items[itype_amulet]=(1<<(i_amulet1-1));
      }
    }
    else
    {
      game->items[itype_amulet]|=(1<<(i_amulet1-1));
    }
    break;
    case iL2Amulet:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_amulet,true)<i_amulet2)
      {
        game->items[itype_amulet]=(1<<(i_amulet2-1));
      }
    }
    else
    {
      game->items[itype_amulet]|=(1<<(i_amulet2-1));
    }
    break;
    case iFlippers:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_flippers,true)<i_flippers)
      {
        game->items[itype_flippers]=(1<<(i_flippers-1));
      }
    }
    else
    {
      game->items[itype_flippers]|=(1<<(i_flippers-1));
    }
    break;
    case iBoots:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_boots,true)<i_boots)
      {
        game->items[itype_boots]=(1<<(i_boots-1));
      }
    }
    else
    {
      game->items[itype_boots]|=(1<<(i_boots-1));
    }
    break;
    case iL2Bracelet:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_bracelet,true)<i_bracelet2)
      {
        game->items[itype_bracelet]=(1<<(i_bracelet2-1));
      }
    }
    else

    {
      game->items[itype_bracelet]|=(1<<(i_bracelet2-1));
    }
    break;
    case iHookshot:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_hookshot,true)<i_hookshot)
      {
        game->items[itype_hookshot]=(1<<(i_hookshot-1));
      }
    }
    else
    {
      game->items[itype_hookshot]|=(1<<(i_hookshot-1));
    }
    break;
    case iLongshot:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_hookshot,true)<i_longshot)
      {
        game->items[itype_hookshot]=(1<<(i_longshot-1));
      }
    }
    else
    {
      game->items[itype_hookshot]|=(1<<(i_longshot-1));
    }
    break;
    case iLens:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_lens,true)<i_lens)
      {
        game->items[itype_lens]=(1<<(i_lens-1));
      }
    }
    else
    {
      game->items[itype_lens]|=(1<<(i_lens-1));
    }
    break;
    case iHammer:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_hammer,true)<i_hammer)
      {
        game->items[itype_hammer]=(1<<(i_hammer-1));
      }
    }
    else
    {
      game->items[itype_hammer]|=(1<<(i_hammer-1));
    }
    break;

    case iMagicC:
    if(get_gamedata_maxmagic(game) < MAGICPERBLOCK*8)
      change_gamedata_maxmagic(game, MAGICPERBLOCK);

    case iSMagic:       change_gamedata_dmagic(game, MAGICPERBLOCK); break;
    case iLMagic:       set_gamedata_dmagic(game, MAGICPERBLOCK<<3); break;

    case iHCPiece:
    change_gamedata_HCpieces(game, 1);
    if(get_gamedata_HCpieces(game)<4)
      break;
    set_gamedata_HCpieces(game, 0);
    // fall through
    case iHeartC:
    if(get_gamedata_maxlife(game) < (get_bit(quest_rules,qr_24HC) ? 24*HP_PER_HEART : 16*HP_PER_HEART))
      change_gamedata_maxlife(game, HP_PER_HEART);
    // fall through
    case iHeart:        set_gamedata_life(game, min(get_gamedata_life(game)+HP_PER_HEART,get_gamedata_maxlife(game))); break;
    case iKillAll:      kill_em_all(); break;
    case iDinsFire:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_dinsfire,true)<i_dinsfire)
      {
        game->items[itype_dinsfire]=(1<<(i_dinsfire-1));
      }
    }
    else
    {
      game->items[itype_dinsfire]|=(1<<(i_dinsfire-1));
    }
    break;
    case iFaroresWind:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_faroreswind,true)<i_faroreswind)
      {
        game->items[itype_faroreswind]=(1<<(i_faroreswind-1));
      }
    }
    else
    {
      game->items[itype_faroreswind]|=(1<<(i_faroreswind-1));
    }
    break;
    case iNayrusLove:
    if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
    {
      if (current_item(itype_nayruslove,true)<i_nayruslove)
      {
        game->items[itype_nayruslove]=(1<<(i_nayruslove-1));
      }
    }
    else
    {
      game->items[itype_nayruslove]|=(1<<(i_nayruslove-1));
    }
    break;
  }

  selectBwpn(0,0);
  selectAwpn(0);

  switch(id)
  {
    case iRupy:
    case i5Rupies:
	case i10Rupies:
    case i20Rupies:
    case i50Rupies:
	case i100Rupies:
    case i200Rupies:
    sfx(WAV_CHIME);
    break;
    case iHeart:
    case iKey:
    sfx(WAV_PLINK);
    break;
    case iTriforce:
    case iBigTri:
    break;
    default:
    sfx(WAV_SCALE);
  }
}

void getdraggeditem(int j)
{
  getitem(items.spr(j)->id);
  items.del(j);
  for(int i=0; i<Lwpns.Count(); i++)
  {
    weapon *w = (weapon*)Lwpns.spr(i);
    if(w->dragging==j)
    {
      w->dragging=-1;
    }
    else if (w->dragging>j)
      {
        w->dragging-=1;
      }
  }
}

void LinkClass::checkitems()
{
  int tmp=currscr>=128?1:0;
  int index;
  if(get_bit(quest_rules,qr_LTTPWALK)) {
    index=items.hit(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),6,6);
  }
  else index=items.hit(x,y+(get_bit(quest_rules, qr_LTTPCOLLISION)?0:8),1,1);
  if(index==-1)
    return;

  // if (tmpscr[tmp].room==rSHOP && boughtsomething==true)
  //   return;

  int pickup = ((item*)items.spr(index))->pickup;
  int id = ((item*)items.spr(index))->id;

  if((pickup&ipTIMER) && (((item*)items.spr(index))->clk2 < 32))
    if((items.spr(index)->id!=iFairyMoving)&&(items.spr(index)->id!=iFairyMoving))
      // wait for it to stop flashing, doesn't check for other items yet
      return;

    if(pickup&ipENEMY)                                        // item was being carried by enemy
    hasitem=0;

  if(pickup&ipDUMMY)                                        // dummy item (usually a rupy)
  {
    if(pickup&ipMONEY)
      dospecialmoney(index);
    return;
  }

  if (get_bit(quest_rules,qr_NOPOTIONCOMBINE))
  {
    if ((id==iBPotion||id==iRPotion)&&
        (current_item(itype_potion,true)))
    {
      return;
    }
  }

  if(pickup&ipCHECK)                                        // check restrictions
    switch(tmpscr[tmp].room)
    {
      case rSP_ITEM:                                        // special item
      if(!canget(id))
        return;
      break;

      case rP_SHOP:                                         // potion shop
      if(msgpos<72)
        return;
      case rSHOP:                                           // shop
      if(get_gamedata_rupies(game)<abs(prices[index-1][0]))
        return;
      change_gamedata_drupy(game, -abs(prices[index-1][0]));
      boughtsomething=true;
      //make the other shop items untouchable after
      //you buy something
      int count = 0;
      for(int i=0; i<3; i++)
      {
        if(QMisc.shop[tmpscr[tmp].catchall].item[i])
        {
          ++count;
        }
      }
      for(int i=1; i<=count; i++)
      {
        ((item*)items.spr(i))->pickup=ipDUMMY+ipFADE;
      }
      break;
    }

  if(pickup&ipONETIME)                                      // set screen item flag for one-time-only items
    setmapflag();

  getitem(id);

  if(pickup&ipHOLDUP)
  {
    if(msgstr)
    {
      msgstr=0;
      clear_bitmap(msgdisplaybuf);
      //      set_clip_state(msgdisplaybuf, 1);
      clear_bitmap(pricesdisplaybuf);
      //      set_clip_state(pricesdisplaybuf, 1);
      //     putscr(scrollbuf,0,0,tmpscr);
    }

    fadeclk=66;

    if(id!=iBombs || action==swimming || get_bit(quest_rules,qr_BOMBHOLDFIX))
    {                                                       // don't hold up bombs unless swimming or the bomb hold fix quest rule is on
      if(action==swimming)
      {
        action=waterhold1;
      }
      else
      {
        action=landhold1;
      }

      if(((item*)items.spr(index))->twohand)
      {
        if(action==waterhold1)
        {
          action=waterhold2;
        }
        else
        {
          action=landhold2;
        }
      }

      holdclk=130;
      holditem=id;
      freeze_guys=true;
    }

    if(id!=iTriforce)
    {
      sfx(WAV_PICKUP);
    }
    items.del(index);
    for(int i=0; i<Lwpns.Count(); i++)
    {
      weapon *w = (weapon*)Lwpns.spr(i);
      if(w->dragging==index)
      {
        w->dragging=-1;
      }
      else if (w->dragging>index)
        {
          w->dragging-=1;
        }
    }
    // clear up shop stuff
    if((isdungeon()==0)&&(index!=0))
    {
      if (((item*)items.spr(0))->pickup&ipDUMMY)
      {
        items.del(0);
        for(int i=0; i<Lwpns.Count(); i++)
        {
          weapon *w = (weapon*)Lwpns.spr(i);
          if(w->dragging==0)
          {
            w->dragging=-1;
          }
          else if (w->dragging>0)
            {
              w->dragging-=1;
            }
        }
      }
      clear_bitmap(msgdisplaybuf);
      set_clip_state(msgdisplaybuf, 1);
      clear_bitmap(pricesdisplaybuf);
      set_clip_state(pricesdisplaybuf, 1);
    }
    //   items.del(index);
  }
  else
  {
    items.del(index);
    for(int i=0; i<Lwpns.Count(); i++)
    {
      weapon *w = (weapon*)Lwpns.spr(i);
      if(w->dragging==index)
      {
        w->dragging=-1;
      }
      else if (w->dragging>index)
        {
          w->dragging-=1;
        }
    }
    clear_bitmap(msgdisplaybuf);
    set_clip_state(msgdisplaybuf, 1);
    clear_bitmap(pricesdisplaybuf);
    set_clip_state(pricesdisplaybuf, 1);
  }
  if(id==iTriforce)
    getTriforce(iTriforce);
  if(id==iBigTri)
    getBigTri();
}

void LinkClass::StartRefill()
{
  if(!refilling)
  {
    refillclk=21;
    stop_sfx(WAV_ER);
    sfx(WAV_REFILL,128,true);
    refilling=true;
    if (((refill_why==REFILL_BPOTION)||(refill_why==REFILL_RPOTION))&&(!get_bit(quest_rules,qr_NONBUBBLEMEDICINE)))
      swordclk=0;
    if ((refill_why==REFILL_FAIRY)&&(!get_bit(quest_rules,qr_NONBUBBLEFAIRIES)))
      swordclk=0;
    if ((refill_why==REFILL_TRIFORCE)&&(!get_bit(quest_rules,qr_NONBUBBLETRIFORCE)))
      swordclk=0;
  }
}

bool LinkClass::refill()
{
  if(!refilling)
    return false;
  ++refillclk;
  int speed = get_bit(quest_rules,qr_FASTFILL) ? 6 : 22;
  int refill_heart_stop=get_gamedata_maxlife(game);
  int refill_magic_stop=get_gamedata_maxlife(game);
  if (refill_why==REFILL_BPOTION)
  {
    refill_heart_stop=min(get_gamedata_life(game)+(zinit.blue_potion_heart_percent?(get_gamedata_maxlife(game)*zinit.blue_potion_hearts/100):(get_gamedata_life(game)+(zinit.blue_potion_hearts*HP_PER_HEART))),get_gamedata_maxlife(game));
    refill_heart_stop=min(get_gamedata_life(game)+(zinit.blue_potion_magic_percent?(get_gamedata_maxlife(game)*zinit.blue_potion_magic/100):(get_gamedata_life(game)+(zinit.blue_potion_magic*MAGICPERBLOCK))),get_gamedata_maxmagic(game));
  }
  else if (refill_why==REFILL_RPOTION)
  {
    refill_heart_stop=min(get_gamedata_life(game)+(zinit.red_potion_heart_percent?(get_gamedata_maxlife(game)*zinit.red_potion_hearts/100):(get_gamedata_life(game)+(zinit.red_potion_hearts*HP_PER_HEART))),get_gamedata_maxlife(game));
    refill_heart_stop=min(get_gamedata_life(game)+(zinit.red_potion_magic_percent?(get_gamedata_maxlife(game)*zinit.red_potion_magic/100):(get_gamedata_life(game)+(zinit.red_potion_magic*MAGICPERBLOCK))),get_gamedata_maxmagic(game));
  }
  if(refillclk%speed == 0)
  {
    //   game->life&=0xFFC;
    switch (refill_what)
    {
      case REFILL_LIFE:
      set_gamedata_life(game, min(refill_heart_stop, (get_gamedata_life(game)+HP_PER_HEART/2)));
      if(get_gamedata_life(game)>=refill_heart_stop)
      {
        set_gamedata_life(game, refill_heart_stop);
        kill_sfx();
        sfx(WAV_MSG);
        refilling=false;
        return false;
      } break;
      case REFILL_MAGIC:
      set_gamedata_magic(game, min(refill_magic_stop, (get_gamedata_magic(game)+MAGICPERBLOCK/4)));
      if(get_gamedata_magic(game)>=refill_magic_stop)
      {
        set_gamedata_magic(game, refill_magic_stop);
        kill_sfx();
        sfx(WAV_MSG);
        refilling=false;
        return false;
      } break;
      case REFILL_ALL:
      set_gamedata_life(game, min(refill_heart_stop, (get_gamedata_life(game)+HP_PER_HEART/2)));
      set_gamedata_magic(game, min(refill_magic_stop, (get_gamedata_magic(game)+MAGICPERBLOCK/4)));
      if((get_gamedata_life(game)>=refill_heart_stop)&&(get_gamedata_magic(game)>=refill_magic_stop))
      {
        set_gamedata_life(game, refill_heart_stop);
        set_gamedata_magic(game, refill_magic_stop);
        kill_sfx();
        sfx(WAV_MSG);
        refilling=false;
        return false;
      } break;
    }
  }
  return true;
}

void LinkClass::getTriforce(int id)
{
  PALETTE flash_pal;
  for(int i=0; i<256; i++)
  {
    flash_pal[i] = get_bit(quest_rules,qr_FADE) ? _RGB(63,63,0) : _RGB(63,63,63);
  }

  //get rid off all sprites but Link
  guys.clear();
  items.clear();
  Ewpns.clear();
  Lwpns.clear();
  Sitems.clear();
  chainlinks.clear();
  decorations.clear();
  if (!COOLSCROLL)
  {
    show_subscreen_items=false;
  }

  sfx(WAV_SCALE);
  jukebox(ZC_MIDI_TRIFORCE);
  if (id==iTriforce)
  {
    game->lvlitems[dlevel]|=liTRIFORCE;
  }

  int f=0;
  int x=0;
  int curtain_x=0;
  int c=0;
  do
  {
    if(f==40)
    {
      ALLOFF();
      action=landhold2;                                      // have to reset this flag
    }
    if(f>=40 && f<88)
    {
      if(get_bit(quest_rules,qr_FADE))
      {
        if((f&3)==0)
        {
          fade_interpolate(RAMpal,flash_pal,RAMpal,42,0,CSET(6)-1);
          refreshpal=true;
        }
        if((f&3)==2)
        {
          loadpalset(0,0);
          loadpalset(1,1);
          loadpalset(5,5);
          if(currscr<128) loadlvlpal(DMaps[currdmap].color);
          else loadlvlpal(0xB);
        }
      }
      else
      {
        if((f&7)==0)
        {
          for(int cs=2; cs<5; cs++)
            for(int i=1; i<16; i++)
              RAMpal[CSET(cs)+i]=flash_pal[CSET(cs)+i];
          refreshpal=true;
        }
        if((f&7)==4)
        {
          if(currscr<128) loadlvlpal(DMaps[currdmap].color);
          else loadlvlpal(0xB);
          loadpalset(5,5);
        }
      }
    }

    if (id==iTriforce)
    {
      if(f==88)
      {
        refill_what=REFILL_ALL;
        refill_why=REFILL_TRIFORCE;
        StartRefill();
        refill();
      }

      if(f==89)
      {
        if(refill())
        {
          --f;
        }
      }
    }

    if(f>=208 && f<288)
    {
      ++x;
      switch(++c)
      {
        case 5: c=0;
        case 0:
        case 2:
        case 3: ++x; break;
      }
    }

    domoney();
    if (f<288)
    {
      curtain_x=x&0xF8;
      draw_screen_clip_rect_x1=curtain_x;
      draw_screen_clip_rect_x2=255-curtain_x;
      draw_screen_clip_rect_y1=0;
      draw_screen_clip_rect_y2=223;
      draw_screen_clip_rect_show_link=true;
      //draw_screen(tmpscr, 0, 0);
    }

    draw_screen(tmpscr, 0, 0);
    put_passive_subscr(framebuf,&QMisc,0,passive_subscreen_offset,false,false);

    advanceframe();
    ++f;
  } while((f<408)||(midi_pos > 0));

  action=none;
  draw_screen_clip_rect_x1=0;
  draw_screen_clip_rect_x2=255;
  draw_screen_clip_rect_y1=0;
  draw_screen_clip_rect_y2=223;
  draw_screen_clip_rect_show_link=true;
  sdir=dir; dowarp(1,0); //side warp
}

void red_shift()
{
  int tnum=176;
  // set up the new palette
  for(int i=CSET(2); i < CSET(4); i++)
  {
    int r = (i-CSET(2)) << 1;
    RAMpal[i+tnum].r = r;
    RAMpal[i+tnum].g = r >> 3;
    RAMpal[i+tnum].b = r >> 4;
  }

  // color scale the game screen
  for(int y=0; y<168; y++)
  {
    for(int x=0; x<256; x++)
    {
      int c = framebuf->line[y+playing_field_offset][x];
      int r = min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
      framebuf->line[y+playing_field_offset][x] = (c ? (r+tnum+CSET(2)) : 0);
    }
  }

  refreshpal = true;
}



void setup_red_screen_old()
{
  clear_bitmap(framebuf);
  rectfill(scrollbuf, 0, 0, 255, 167, 0);
  if(tmpscr->flags7&fLAYER2BG) do_layer(scrollbuf,1, tmpscr, 0, playing_field_offset, 2);
  if(tmpscr->flags7&fLAYER3BG) do_layer(scrollbuf,2, tmpscr, 0, playing_field_offset, 2);
  putscr(scrollbuf, 0, 0, tmpscr);
  blit(scrollbuf, framebuf, 0, 0, 0, playing_field_offset, 256, 168);
  do_layer(framebuf,0, tmpscr, 0, 0, 2);
  if(!(tmpscr->flags7&fLAYER2BG)) do_layer(framebuf,1, tmpscr, 0, 0, 2);
  do_layer(framebuf,-2, tmpscr, 0, 0, 2);

  if(!(msgdisplaybuf->clip))
  {
    masked_blit(msgdisplaybuf, framebuf,0,0,0,playing_field_offset, 256,168);
  }
  if(!(pricesdisplaybuf->clip))
  {
    masked_blit(pricesdisplaybuf, framebuf,0,0,0,playing_field_offset, 256,168);
  }
  //red shift
  // color scale the game screen
  for(int y=0; y<168; y++)
  {
    for(int x=0; x<256; x++)
    {
      int c = framebuf->line[y+playing_field_offset][x];
      int r = min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
      framebuf->line[y+playing_field_offset][x] = (c ? (r+CSET(2)) : 0);
    }
  }

  //  Link->draw(framebuf);
  blit(framebuf,scrollbuf, 0, playing_field_offset, 256, playing_field_offset, 256, 168);

  clear_bitmap(framebuf);

  if (!((tmpscr->layermap[2]==0||(tmpscr->flags7&fLAYER3BG))
        && tmpscr->layermap[3]==0
        && tmpscr->layermap[4]==0
        && tmpscr->layermap[5]==0
        && !overheadcombos(tmpscr)))
  {
    if(!(tmpscr->flags7&fLAYER3BG)) do_layer(framebuf,2, tmpscr, 0, 0, 2);
    do_layer(framebuf,3, tmpscr, 0, 0, 2);
    do_layer(framebuf,-1, tmpscr, 0, 0, 2);
    do_layer(framebuf,4, tmpscr, 0, 0, 2);
    do_layer(framebuf,5, tmpscr, 0, 0, 2);

    //do an AND masked blit for messages on top of layers
    if(!(msgdisplaybuf->clip) || !(pricesdisplaybuf->clip))
    {
      for(int y=0; y<168; y++)
      {
        for(int x=0; x<256; x++)
        {
          int c1 = framebuf->line[y+playing_field_offset][x];
          int c2 = msgdisplaybuf->line[y][x];
          int c3 = pricesdisplaybuf->line[y][x];

          if (c1 && c3)
          {
            framebuf->line[y+playing_field_offset][x] = c3;
          }
          else if (c1 && c2)
            {
              framebuf->line[y+playing_field_offset][x] = c2;
            }
        }
      }
    }

    //red shift
    // color scale the game screen
    for(int y=0; y<168; y++)
    {
      for(int x=0; x<256; x++)
      {
        int c = framebuf->line[y+playing_field_offset][x];
        int r = min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
        framebuf->line[y+playing_field_offset][x] = r+CSET(2);
      }
    }
  }

  blit(framebuf,scrollbuf, 0, playing_field_offset, 0, playing_field_offset, 256, 168);

  // set up the new palette
  for(int i=CSET(2); i < CSET(4); i++)
  {
    int r = (i-CSET(2)) << 1;
    RAMpal[i].r = r;
    RAMpal[i].g = r >> 3;
    RAMpal[i].b = r >> 4;
  }
  refreshpal = true;
}



void slide_in_color(int color)
{
  for(int i=1; i<16; i+=3)
  {
    RAMpal[CSET(2)+i+2] = RAMpal[CSET(2)+i+1];
    RAMpal[CSET(2)+i+1] = RAMpal[CSET(2)+i];
    RAMpal[CSET(2)+i]   = NESpal(color);
  }
  refreshpal=true;
}

void LinkClass::gameover()
{
  int f=0;

  action=none;
  Playing=false;
  if(!debug_enabled)
  {
    Paused=false;
  }
  set_gamedata_deaths(game, min(get_gamedata_deaths(game)+1,999));
  dir=down;
  music_stop();
  kill_sfx();
  attackclk=hclk=superman=0;

  //get rid off all sprites but Link
  guys.clear();
  items.clear();
  Ewpns.clear();
  Lwpns.clear();
  Sitems.clear();
  chainlinks.clear();
  decorations.clear();

  //in original Z1, Link marker vanishes at death.
  //code in subscr.cpp, put_passive_subscr checks the following value.
  //color 255 is a GUI color, so quest makers shouldn't be using this value.
  //Also, subscreen is static after death in Z1.
  int tmp_link_dot = QMisc.colors.link_dot;
  QMisc.colors.link_dot = 255;
  put_passive_subscr(scrollbuf, &QMisc, 256, passive_subscreen_offset, false, false);//save this and reuse it.
  QMisc.colors.link_dot = tmp_link_dot;

  do
  {
    if (f<254)
    {
      if(f<=32)
      {
        hclk=(32-f);
      }

      if(f>=62 && f<138)
      {
        switch((f-62)%20)
        {
          case 0:  dir=right;  break;
          case 5:  dir=up; break;
          case 10: dir=left;    break;
          case 15: dir=down;  break;
        }
        linkstep();
      }

      if(f>=194 && f<208)
      {
        if(f==194)
          action = dying;

        cs = wpnsbuf[iwDeath].csets&15;
        tile = wpnsbuf[iwDeath].tile;
        if(BSZ)
        {
          tile += (f-194)/3;
        }
        else if(f>=204)
          {
            ++tile;
          }
      }

      if(f==208)
        dontdraw = true;

      if(get_bit(quest_rules,qr_FADE))
      {
        if(f < 170)
        {
          if(f<60)
          {
            draw_screen(tmpscr, 0, 0);
            //reuse our static subscreen
            blit(scrollbuf,framebuf,256,0,0,0,256,passive_subscreen_height);
          }
          if(f==60)
          {
            red_shift();
            create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
            create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128, NULL);
          }

          if(f>=60 && f<=169)
          {
            draw_screen(tmpscr, 0, 0);
            //reuse our static subscreen
            blit(scrollbuf,framebuf,256,0,0,0,256,passive_subscreen_height);
            red_shift();

          }
          if(f>=139 && f<=169)//fade from red to black
          {
            fade_interpolate(RAMpal,black_palette,RAMpal, (f-138)<<1, 224, 255);
            create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
            create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128, NULL);
            refreshpal=true;
          }
        }
        else //f>=170
        {
          if(f==170)//make Link grayish
          {
            fade_interpolate(RAMpal,black_palette,RAMpal,64, 224, 255);
            for(int i=CSET(6); i < CSET(7); i++)
            {
              int g = (RAMpal[i].r + RAMpal[i].g + RAMpal[i].b)/3;
              RAMpal[i] = _RGB(g,g,g);
            }
            refreshpal = true;
          }

          //draw only link. otherwise black layers might cover him.
          rectfill(framebuf,0,playing_field_offset,255,167+playing_field_offset,0);
          draw(framebuf);
          blit(scrollbuf,framebuf,256,0,0,0,256,passive_subscreen_height);
        }
      }
      else //!qr_FADE
      {
        if(f==58)
        {
          for(int i = 0; i < 96; i++)
            tmpscr->cset[i] = 3;
          for(int j=0; j<6; j++)
            if (tmpscr->layermap[j]>0)
              for(int i=0; i<96; i++)
                tmpscr2[j].cset[i] = 3;
        }

        if(f==59)
        {
          for(int i = 96; i < 176; i++)
            tmpscr->cset[i] = 3;
          for(int j=0; j<6; j++)
            if (tmpscr->layermap[j]>0)
              for(int i=96; i<176; i++)
                tmpscr2[j].cset[i] = 3;
        }

        if(f==60)
        {
          for(int i=0; i<176; i++)
          {
            tmpscr->cset[i] = 2;
          }
          for(int j=0; j<6; j++)
            if (tmpscr->layermap[j]>0)
              for(int i=0; i<176; i++)
                tmpscr2[j].cset[i] = 2;

            for(int i=1; i<16; i+=3)
          {
            RAMpal[CSET(2)+i]   = NESpal(0x17);
            RAMpal[CSET(2)+i+1] = NESpal(0x16);
            RAMpal[CSET(2)+i+2] = NESpal(0x26);
          }
          refreshpal=true;
        }

        if(f==139)
          slide_in_color(0x06);
        if(f==149)
          slide_in_color(0x07);
        if(f==159)
          slide_in_color(0x0F);
        if(f==169)
        {
          slide_in_color(0x0F);
          slide_in_color(0x0F);
        }
        if(f==170)
        {
          for(int i=1; i<16; i+=3)
          {
            RAMpal[CSET(6)+i]   = NESpal(0x10);
            RAMpal[CSET(6)+i+1] = NESpal(0x30);
            RAMpal[CSET(6)+i+2] = NESpal(0x00);
            refreshpal = true;
          }
        }

        if(f < 169)
        {
          draw_screen(tmpscr, 0, 0);
          //reuse our static subscreen
          blit(scrollbuf,framebuf,256,0,0,0,256,passive_subscreen_height);
        }
        else
        { //draw only link. otherwise black layers might cover him.
          rectfill(framebuf,0,playing_field_offset,255,167+playing_field_offset,0);
          draw(framebuf);
          blit(scrollbuf,framebuf,256,0,0,0,256,passive_subscreen_height);
        }
      }
    }
    else if(f<350)//draw 'GAME OVER' text
      {
        clear_bitmap(framebuf);
        blit(scrollbuf,framebuf,256,0,0,0,256,passive_subscreen_height);
        textout_ex(framebuf,zfont,"GAME OVER",96,playing_field_offset+80,1,-1);
      }
      else
      {
        clear_bitmap(framebuf);
      }

    //SFX... put them all here
    switch (f)
    {
      case   0: sfx(WAV_OUCH,pan(int(x))); break;
      case  60: sfx(WAV_SPIRAL); break;
      case 194: sfx(WAV_MSG); break;
    }

    advanceframe();
    ++f;
  } while(f<353 && !Quit);

  action=none;
  dontdraw=false;
}


void LinkClass::ganon_intro()
{
  /*
    ************************
    * GANON INTRO SEQUENCE *
    ************************
    -25 DOT updates
    -24 LINK in
    0 TRIFORCE overhead - code begins at this point (f == 0)
    47 GANON in
    58 LIGHT step
    68 LIGHT step
    78 LIGHT step
    255 TRIFORCE out
    256 TRIFORCE in
    270 TRIFORCE out
    271 GANON out, LINK face up
    */
  loaded_guys=true;
  if(game->lvlitems[dlevel]&liBOSS)
  {
    return;
  }

  dir=down;
  action=landhold2;
  holditem=iTriforce;

  for(int f=0; f<271 && !Quit; f++)
  {
    if(f==47)
    {
      music_stop();
      stop_sfx(WAV_ROAR);
      sfx(WAV_GASP);
      sfx(WAV_GANON);
      if(current_item(itype_ring,true))
      {
        addenemy(160,96,eGANON,0);
      }
      else
      {
        addenemy(80,32,eGANON,0);
      }
    }
    if(f==48)
    {
      lighting(1,dir);
      f += 30;
    }

    //NES Z1, the triforce vanishes for one frame in two cases
    //while still showing Link's two-handed overhead sprite.
    if(f==255 || f==270)
    {
      holditem=-1;
    }
    if(f==256)
    {
      holditem=iTriforce;
    }

    draw_screen(tmpscr, 0, 0);
    advanceframe();
    if(rSbtn())
    {
      conveyclk=3;
      int tmp_subscr_clk = frame;
      dosubscr(&QMisc);
      newscr_clk += frame - tmp_subscr_clk;
    }

  }

  action=none;
  dir=up;
  if(!getmapflag() && (tunes[MAXMIDIS-1].midi))
    jukebox(MAXMIDIS-1);
  else
    //play_DmapMusic();
    playLevelMusic();
  currcset=DMaps[currdmap].color;
  dointro();
  cont_sfx(WAV_ROAR);
}

void LinkClass::saved_Zelda()
{
  Playing=Paused=false;
  action=won;
  Quit=qWON;
  hclk=0;
  x = 136;
  y = (isdungeon() && currscr<128) ? 75 : 73;
  dir=left;
}

void LinkClass::reset_hookshot()
{
  if (action!=rafting)
  {
    action=none;
  }
  hookshot_frozen=false;
  hookshot_used=false;
  pull_link=false;
  add_chainlink=false;
  del_chainlink=false;
  hs_fix=false;
  for (int i=0; i<chainlinks.Count(); i++)
  {
    chainlinks.del(chainlinks.idFirst(wHSChain));
  }
  hs_xdist=0;
  hs_ydist=0;
}

void LinkClass::reset_ladder()
{
  ladderx=laddery=0;
}

void LinkClass::check_conveyor()
{
  if (action==casting||inlikelike)
  {
    return;
  }
  int xoff,yoff;
  if (conveyclk<=0)
  {
    conveyor_flags=0;
    is_on_conveyor=false;
    int ctype;
    for (int i=0; i<4; i++)
    {
      switch (i)
      {
        case 0:
        ctype=(combobuf[MAPCOMBO(x+8,y+12-(get_bit(quest_rules, qr_LTTPCOLLISION)*4))].type);
        if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
        {
          set_bit(&conveyor_flags,ctype-cCVUP,1);
        }
        break;
        case 1:
        ctype=(combobuf[MAPCOMBO(x+8,y+12-(get_bit(quest_rules, qr_LTTPCOLLISION)*4))].type);
        if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
        {
          set_bit(&conveyor_flags,ctype-cCVUP,1);
        }
        break;
        case 2:
        ctype=(combobuf[MAPCOMBO(x+8,y+12-(get_bit(quest_rules, qr_LTTPCOLLISION)*4))].type);
        if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
        {
          set_bit(&conveyor_flags,ctype-cCVUP,1);
        }
        break;
        case 3:
        ctype=(combobuf[MAPCOMBO(x+8,y+12-(get_bit(quest_rules, qr_LTTPCOLLISION)*4))].type);
        if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
        {
          set_bit(&conveyor_flags,ctype-cCVUP,1);
        }
        break;
      }
      if (conveyor_flags!=0)
      {
        is_on_conveyor=true;
      }
    }
    for (int i=0; i<4; i++)
    {
      switch (i)
      {
        case 0:
        if(get_bit(&conveyor_flags,i)&&!walkflag(x,y+8-(get_bit(quest_rules, qr_LTTPCOLLISION)*8)-2,2,up))
        {
          int step=0;
          if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!get_bit(quest_rules,qr_LTTPWALK))
          {
            while(step<2)
            {
              yoff=((int)y-step)&7;
              if (!yoff) break;
              step++;
            }
          }
          else step=2;
          y=y-step;
          hs_starty-=step;
          for (int i=0; i<chainlinks.Count(); i++)
          {
            chainlinks.spr(i)->y-=step;
          }
          if (Lwpns.idFirst(wHookshot)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHookshot))->y-=step;
          }
          if (Lwpns.idFirst(wHSHandle)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->y-=step;
          }
        }
        break;
        case 1:
        if(get_bit(&conveyor_flags,i)&&!walkflag(x,y+15+2,2,down))
        {
          int step=0;
          if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!get_bit(quest_rules,qr_LTTPWALK))
          {
            while(step<2)
            {
              yoff=((int)y+step)&7;
              if (!yoff) break;
              step++;
            }
          }
          else step=2;
          y=y+step;
          hs_starty+=step;
          for (int i=0; i<chainlinks.Count(); i++)
          {
            chainlinks.spr(i)->y+=step;
          }
          if (Lwpns.idFirst(wHookshot)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=step;
          }
          if (Lwpns.idFirst(wHSHandle)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=step;
          }
        }
        break;
        case 2:
        if(get_bit(&conveyor_flags,i)&&!walkflag(x-int(lsteps[int(x)&7]),y+8-(get_bit(quest_rules, qr_LTTPCOLLISION)*8),1,left))
        {
          int step=0;
          if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!get_bit(quest_rules,qr_LTTPWALK))
          {
            while(step<2)
            {
              xoff=((int)x-step)&7;
              if (!xoff) break;
              step++;
            }
          }
          else step=2;
          x=x-step;
          hs_startx-=step;
          for (int i=0; i<chainlinks.Count(); i++)
          {
            chainlinks.spr(i)->x-=step;
          }
          if (Lwpns.idFirst(wHookshot)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHookshot))->x-=step;
          }
          if (Lwpns.idFirst(wHSHandle)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->x-=step;
          }
        }
        break;
        case 3:
        if(get_bit(&conveyor_flags,i)&&!walkflag(x+15+2,y+8-(get_bit(quest_rules, qr_LTTPCOLLISION)*8),1,right))
        {
          int step=0;
          if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!get_bit(quest_rules,qr_LTTPWALK))
          {
            while(step<2)
            {
              xoff=((int)x+step)&7;
              if (!xoff) break;
              step++;
            }
          }
          else step=2;
          x=x+step;
          hs_startx+=step;
          for (int i=0; i<chainlinks.Count(); i++)
          {
            chainlinks.spr(i)->x+=step;
          }
          if (Lwpns.idFirst(wHookshot)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=step;
          }
          if (Lwpns.idFirst(wHSHandle)>-1)
          {
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=step;
          }
        }
        break;
      }
    }
  }
}

void LinkClass::setNayrusLoveShieldClk(int newclk)
{
  NayrusLoveShieldClk=newclk;
  if (decorations.idCount(dNAYRUSLOVESHIELD)==0)
  {
    decoration *d;
    decorations.add(new dNayrusLoveShield(LinkX(), LinkY(), dNAYRUSLOVESHIELD, 0));
    decorations.spr(decorations.Count()-1)->misc=0;
    decorations.add(new dNayrusLoveShield(LinkX(), LinkY(), dNAYRUSLOVESHIELD, 0));
    d=(decoration *)decorations.spr(decorations.Count()-1);
    decorations.spr(decorations.Count()-1)->misc=1;
  }
}

int LinkClass::getNayrusLoveShieldClk()
{
  return NayrusLoveShieldClk;
}

/*** end of link.cc ***/
