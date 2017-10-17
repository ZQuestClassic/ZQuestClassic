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
#include "decorations.h"
#include <stdio.h>

extern int draw_screen_clip_rect_x1;
extern int draw_screen_clip_rect_x2;
extern int draw_screen_clip_rect_y1;
extern int draw_screen_clip_rect_y2;
extern bool draw_screen_clip_rect_show_link;


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
    (action==swimhold1)||(action==swimhold2)||
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
  x=tmpscr->warparrivalx;
  y=tmpscr->warparrivaly;
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
  yofs=56;
  cs=6;
  pushing=fairyclk=0;
  id=0;
  inlikelike=superman=inwallm=false;
  blowcnt=whirlwind=0;
  hopclk=diveclk=0;
  conveyor_flags=0;
  drunkclk=0;
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
      overtile16(dest, itemsbuf[iLadder].tile, ladderx, laddery+56,
        itemsbuf[iLadder].csets&15, rotate_value((itemsbuf[iRaft].misc>>2)&3)^3);
    }
    else
    {
      overtile16(dest, itemsbuf[iLadder].tile, ladderx, laddery+56,
        itemsbuf[iLadder].csets&15, (itemsbuf[iLadder].misc>>2)&3);
    }
  }
}

void LinkClass::draw(BITMAP* dest)
{
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

  bool useltm=get_bit(quest_rules,qr_FULLLTM);

  yofs = (!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 54 : 56;
  detail_int[0]=yofs;
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
    if(attackclk>4)
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

        switch(dir)
        {
          case up:
            wx=-1; wy=-12;
            if (!game.canslash || attack!=wSword)
            {
              if(attackclk==13) wy+=4;
              if(attackclk==14) wy+=8;
            }
            break;
          case down:
            f=get_bit(quest_rules,qr_SWORDWANDFLIPFIX)?3:2; wy=11;
            if (!game.canslash || attack!=wSword)
            {
              if(attackclk==13) wy-=4;
              if(attackclk==14) wy-=8;
            }
            break;
          case left:
            f=1; wx=-11; ++t;
            if (!game.canslash || attack!=wSword)
            {
              if(attackclk==13) wx+=4;
              if(attackclk==14) wx+=8;
            }
            break;
          case right:
            wx=11; ++t;
            if (!game.canslash || attack!=wSword)
            {
              if(attackclk==13) wx-=4;
              if(attackclk==14) wx-=8;
            }
            break;
        }

        if (game.canslash && attack==wSword && attackclk<11)
        {
          switch(dir)
          {
            case up:
              wx=8; wy=0;
              ++t; f=0;                                     //starts pointing right
              if(attackclk>=8)
              {
                wy-=8;
                t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
                cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
                f=0;
              }
              break;
            case down:
              wx=-8; wy=0;
              ++t; f=1;                                     //starts pointing left
              if(attackclk>=8)
              {
                wy+=8;
                t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
                cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
                ++t;
                f=0;
              }
              break;
            case left:
              wx=0; wy=-8;
              --t; f=0;                                     //starts pointing up
              if(attackclk>=8)
              {
                wx-=8;
                t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
                cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
                t+=2;
                f=0;
              }
              break;
            case right:
              wx=0; wy=-8;
              --t; f=1;                                     //starts pointing up
              if(attackclk>=8)
              {
                wx+=8;
                t = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].tile;
                cs = wpnsbuf[wSWORDSLASH+current_item(itype_sword,true)-1].csets&15;
                t+=3;
                f=0;
              }
              break;
          }
        }

        if(BSZ || ((isdungeon() && currscr<128) && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)))
        {
          wy+=2;
        }
        if(isdungeon() && currscr<128)
        {
          BITMAP *sub = create_sub_bitmap(dest,16,72,224,144);
          overtile16(sub,t,x+wx-16,y+wy-(72-yofs),cs,f);
          destroy_bitmap(sub);
        }
        else
        {
          overtile16(dest,t,x+wx,y+yofs+wy,cs,f);
        }
      }
    }

    if(attackclk<7 || ((attack==wSword || attack==wWand) && attackclk<13) || ((attack==wHammer) && attackclk<=30))
    {
      linktile(&tile, &flip, ls_stab, dir, zinit.linkwalkstyle);

      if(game.canslash && (attack==wSword) && (attackclk<7))
      {
        linktile(&tile, &flip, ls_slash, dir, zinit.linkwalkstyle);
      }
      if((attack==wHammer) && (attackclk<13))
      {
        linktile(&tile, &flip, ls_pound, dir, zinit.linkwalkstyle);
      }
      masked_draw(dest);
      if (attack!=wHammer)
      {
        return;
      }
    }

    if(attack==wHammer)
    {
      int wy=1;
      int wx=1;
      int f=0,t,cs;
      t = wpnsbuf[wHammer].tile;
      cs = wpnsbuf[wHammer].csets&15;

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
      if(isdungeon() && currscr<128)
      {
        BITMAP *sub = create_sub_bitmap(dest,16,72,224,144);
        overtile16(sub,t,x+wx-16,y+wy-(72-yofs),cs,f);
        destroy_bitmap(sub);
      } else
      overtile16(dest,t,x+wx,y+yofs+wy,cs,f);

      if (attackclk==15)
      {
        sfx(WAV_HAMMER,pan(int(x)));
      }
      return;
    }
  }

  if (action!=casting)
  {
    switch (zinit.linkwalkstyle)
    {
      case 0:                                               //normal
        if(action==swimming || action==swimhit || hopclk==0xFF)
        {
          linktile(&tile, &flip, ls_swim, dir, zinit.linkwalkstyle);
          if (lstep>=6)
          {
            ++tile;
          }
          if (diveclk>30)
          {
            linktile(&tile, &flip, ls_dive, dir, zinit.linkwalkstyle);
            tile+=((frame>>3) & 1);
          }
        }
        else
        {
          linktile(&tile, &flip, ls_walk, dir, zinit.linkwalkstyle);
          if (dir>up)
          {
            useltm=true;
          }
          if (lstep>=6)
          {
            if (dir==up)
            {
              ++flip;
            }
            else
            {
              ++tile;
            }
          }
        }
        break;
      case 1:                                               //BS
        if(action==swimming || action==swimhit || hopclk==0xFF)
        {
          linktile(&tile, &flip, ls_swim, dir, zinit.linkwalkstyle);
          tile += anim_3_4(lstep,7);
          if(diveclk>30)
          {
            linktile(&tile, &flip, ls_dive, dir, zinit.linkwalkstyle);
            tile += anim_3_4(lstep,7);
          }
        }
        else
        {
          linktile(&tile, &flip, ls_walk, dir, zinit.linkwalkstyle);
          if (dir>up)
          {
            useltm=true;
          }
          tile += anim_3_4(lstep,7);
        }
        break;
      case 2:                                               //4-frame
        break;
      default:
        break;
    }
  }

  yofs = (!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 54 : 56;
  if(action==won)
  {
    yofs=54;
  }

  if(action==holding1 || action==holding2)
  {
//    yofs = BSZ ? 56 : 54;
    useltm=get_bit(quest_rules,qr_FULLLTM);

    yofs = (!BSZ && isdungeon() && currscr<128 && !get_bit(quest_rules,qr_LINKDUNGEONPOSFIX)) ? 54 : 56;
    linktile(&tile, &flip, (action==holding1)?ls_hold1:ls_hold2, dir, zinit.linkwalkstyle);
    if (get_bit(quest_rules,qr_HOLDITEMANIMATION))
    {
      putitem2(dest,x-((action==holding1)?4:0),y+yofs-16,holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
    }
    else
    {
      putitem(dest,x-((action==holding1)?4:0),y+yofs-16,holditem);
    }
  }
  else if(action==swimhold1 || action==swimhold2)
  {
    useltm=get_bit(quest_rules,qr_FULLLTM);
    linktile(&tile, &flip, (action==swimhold1)?ls_swimhold1:ls_swimhold2, dir, zinit.linkwalkstyle);
    if (get_bit(quest_rules,qr_HOLDITEMANIMATION))
    {
      putitem2(dest,x-((action==swimhold1)?4:0),y+yofs-12,holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
    }
    else
    {
      putitem(dest,x-((action==swimhold1)?4:0),y+yofs-12,holditem);
    }
  }

  if (useltm&&action!=casting)
  {
    tile+=item_tile_mod();
  }
  tile+=dmap_tile_mod();
  if (!(get_bit(quest_rules,qr_LINKFLICKER)&&((superman||hclk)&&(frame&1))))
  {
    masked_draw(dest);
  }
  if((didstuff&did_fairy)||fairyclk==0||(get_bit(quest_rules,qr_NOHEARTRING)))
    return;

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
    double ty = -sin(a)*53 +ny+56;
    overtile8(dest,htile,int(tx),int(ty),1,0);
    a-=PI/4;
    ++hearts;
  } while(a>PI/2 && hearts<8);
}

void LinkClass::masked_draw(BITMAP* dest)
{
  if(isdungeon() && currscr<128 && (x<16 || x>224 || y<18 || y>146) && !get_bit(quest_rules,qr_FREEFORM))
  {                                                         // clip under doorways
    BITMAP *sub=create_sub_bitmap(dest,16,72,224,144);
    if(sub!=NULL)
    {
      yofs -= 72;
      xofs -= 16;
      sprite::draw(sub);
      xofs=0;
      destroy_bitmap(sub);
    }
  }
  else
    sprite::draw(dest);
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

  if (game.canslash && attack==wSword && attackclk<11)
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
          hitlink(i);
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
    if(dir==up && (int(x)&15)==0)
    {
      check_slash_block(wx,wy);
      check_slash_block(wx,wy+8);
    }
    if(dir==up && (int(x)&15)==8)
    {
      check_slash_block(wx,wy);
      check_slash_block(wx,wy+8);
      check_slash_block(wx+8,wy);
      check_slash_block(wx+8,wy+8);
    }
    if(dir==down && (int(x)&15)==0)
    {
      check_slash_block(wx,wy+wysz-8);
      check_slash_block(wx,wy+wysz);
    }
    if(dir==down && (int(x)&15)==8)
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
    if(dir==up && (int(x)&15)==8)
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
    if(dir==down && (int(x)&15)==8)
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
    if(dir==up && (int(x)&15)==8)
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
    if(dir==down && (int(x)&15)==8)
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
  bx &= 0xF0;
  by &= 0xF0;

  int type = COMBOTYPE(bx,by);
  int flag = MAPFLAG(bx,by);
  int i = (bx>>4) + by;

  if(type!=cSLASH && type!=cSLASHITEM && type!=cBUSH && type!=cFLOWERS &&
    type!=cTALLGRASS && (flag<mfSWORD || flag>mfXSWORD) &&
    flag!=mfSTRIKE)
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
  else
  {
    s->data[i] = s->undercombo;
    s->cset[i] = s->undercset;
    s->sflag[i] = 0;
    set_bit(screengrid,i,1);
    //    pausenow=true;
  }

  if(flag==mfARMOS_ITEM && !getmapflag())
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
  bx &= 0xF0;
  by &= 0xF0;

  int flag = MAPFLAG(bx,by);
  int i = (bx>>4) + by;

  if(flag!=mfWAND)
    return;

  if(i > 175)
    return;

  mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);

  findentrance(bx,by,mfWAND,true);
  findentrance(bx,by,mfSTRIKE,true);

  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
}

void LinkClass::check_pound_block(int bx, int by)
{
  bx &= 0xF0;
  by &= 0xF0;

  int type = COMBOTYPE(bx,by);
  int flag = MAPFLAG(bx,by);
  int i = (bx>>4) + by;

  if(type!=cPOUND && flag!=mfHAMMER && flag!=mfSTRIKE)
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
        switch(hitdir)
        {
          case up:    if(hit_walkflag(x,y+7,2))    action=none; else --y; break;
          case down:  if(hit_walkflag(x,y+16,2))   action=none; else ++y; break;
          case left:  if(hit_walkflag(x-1,y+8,1))  action=none; else --x; break;
          case right: if(hit_walkflag(x+16,y+8,1)) action=none; else ++x; break;
        }
      }
    }
  }

  if(hclk>0 || inlikelike || action==inwind || inwallm || diveclk>30
    || (action==hopping && hopclk<255) )
  {
    return;
  }

  for(int i=0; i<Lwpns.Count(); i++)
  {
    sprite *s = Lwpns.spr(i);

    if (!get_bit(quest_rules,qr_FIREPROOFLINK))
    {
      if(s->id==wFire && (superman ? s->hit(x+7,y+7,2,2) : s->hit(this))&&
        ((weapon*)(Lwpns.spr(i)))->type<3)
      {
        if(NayrusLoveShieldClk<=0)
        {
          game.life = max(game.life-((HP_PER_HEART/2)>>current_item(itype_ring,true)),0);
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
          game.life = max(game.life-((4*HP_PER_HEART/2)>>current_item(itype_ring,true)),0);
          if (s->id==wSBomb)
          {
            game.life = max(game.life-((12*HP_PER_HEART/2)>>current_item(itype_ring,true)),0);
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

  int hit = GuyHit(x+7,y+7,2,2);
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
      game.life = max(game.life-(lwpn_dp(hit)>>current_item(itype_ring,true)),0);
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
      game.life = max(game.life-(ewpn_dp(hit)>>current_item(itype_ring,true)),0);
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

  int ctype1(combobuf[MAPDATA(x,y+8)].type);
  int ctype2(combobuf[MAPDATA(x+8,y+8)].type);
  int ctype=max(ctype1, ctype2);

  if((ctype>=cDAMAGE1) && (ctype<=cDAMAGE4))
  {
    if((!(current_item(itype_boots, true))) || ((get_bit(quest_rules,qr_MAGICBOOTS))&&(game.magic+game.dmagic<BOOTSDRAINAMOUNT*game.magicdrainrate)))
    {
      if(NayrusLoveShieldClk<=0)
      {
        game.life = max((game.life-((HP_PER_HEART<<2)>>(cDAMAGE4-ctype))),0);
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
        game.magic-=BOOTSDRAINAMOUNT*game.magicdrainrate;
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
    game.life = max(game.life-(enemy_dp(hit)>>current_item(itype_ring,true)),0);
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
      GrabLink(hit);
      inwallm=true;
      action=none;
      break;

    case eBUBBLE:
      if(swordclk>=0)
        swordclk=150;
      break;

    case eRBUBBLE:
      swordclk=-1;
      break;

    case eBBUBBLE:
      swordclk=0;
      break;

    case eIBUBBLE:
      if(itemclk>=0)
        itemclk=150;
      break;

    case eRIBUBBLE:
      itemclk=-1;
      break;

    case eBIBUBBLE:
      itemclk=0;
      break;
  }
}

// returns true when game over
bool LinkClass::animate(int index)
{
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
  if (!is_on_conveyor)
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
  if(game.life<=0)
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

    case holding1:
    case holding2:
      if(--holdclk == 0)
        action=none;
      else
        freeze_guys=true;
      break;

    case swimhold1:
    case swimhold2:
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
  if((abs(laddery-int(y))>=16) || (abs(ladderx-int(x))>=16))
  {
    ladderx = laddery = 0;
  }

  // check lots of other things
  checkscroll();
  if(action!=inwind)
  {
    checkitems();
    checklocked();
    checklockblock();
    checkbosslockblock();
    checkpushblock();
    if (hookshot_frozen==false)
    {
      checkspecial();
    }
    if(action==won)
    {
      return true;
    }
  }

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


  if((!activated_timed_warp) && (tmpscr->timedwarptics>0) &&
    (frame - newscr_clk >= tmpscr->timedwarptics))
  {
    activated_timed_warp=true;
    dowarp(1);
  }

  // walk through bombed doors and fake walls
  bool walk=false;
  int dtype=dBOMBED;
  if(pushing>=24) dtype=dWALK;

  if(isdungeon() && action!=freeze && loaded_guys && !inlikelike)
  {
    if(((dtype==dBOMBED)?DrunkUp():dir==up) && x==120 && y<=32 && tmpscr->door[0]==dtype)
    {
      walk=true;
      dir=up;
    }

    if(((dtype==dBOMBED)?DrunkDown():dir==down) && x==120 && y>=128 && tmpscr->door[1]==dtype)
    {
      walk=true;
      dir=down;
    }

    if(((dtype==dBOMBED)?DrunkLeft():dir==left) && x<=32 && y==80 && tmpscr->door[2]==dtype)
    {
      walk=true;
      dir=left;
    }

    if(((dtype==dBOMBED)?DrunkRight():dir==right) && x>=208 && y==80 && tmpscr->door[3]==dtype)
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
    stepforward(29);
    action=scrolling;
    pushing=false;
  }

  if(game.life<=(HP_PER_HEART))
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
/*
  if(rSbtn())
  {
    conveyclk=3;
    dosubscr();
  }
*/
  if(rSbtn())
  {
    conveyclk=3;
    int tmp_subscr_clk = frame;
    dosubscr();
    newscr_clk += frame - tmp_subscr_clk;
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

  switch(wpn)
  {

    case iRPotion:
    case iBPotion:
      //  --game.potion;
      game.items[itype_potion]=game.items[itype_potion]>>1;
      Bwpn=0;

      refill_what=REFILL_ALL;
      StartRefill(REFILL_POTION);
      while(refill())
      {
        putsubscr(framebuf,0,0);
        advanceframe();
      }

      selectBwpn(0, 0);
      return false;

    case iLetter:
      //  if(game.letter==1 && currscr==128 && tmpscr[1].room==rP_SHOP) {
      if(current_item(itype_letter,true)==i_letter &&
        tmpscr[currscr<128?0:1].room==rP_SHOP &&
        tmpscr[currscr<128?0:1].guy &&
//        ((currscr<128&&isdungeon())||(currscr>=128&&!isdungeon()))
        ((currscr<128&&dlevel)||(currscr>=128&&!isdungeon()))
        )
      {
        game.items[itype_letter]|=i_letter_used;
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
      if(!debug)
        --game.items[itype_bomb];
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
      if(!debug)
        --game.items[itype_sbomb];
      selectBwpn(8,8);
      Lwpns.add(new weapon((fix)wx,(fix)wy,wSBomb,0,16*DAMAGE_MULTIPLIER,dir));
      sfx(WAV_PLACE,pan(wx));
    } break;

    case iWand:
      if(Lwpns.idCount(wMagic))
        return false;
      if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
        ((game.magic+game.dmagic)<WANDDRAINAMOUNT*game.magicdrainrate)&&
        ((get_bit(quest_rules,qr_MAGICWAND))))
        return false;
      if(Lwpns.idCount(wBeam))
        Lwpns.del(Lwpns.idFirst(wBeam));
      Lwpns.add(new weapon((fix)wx,(fix)wy,wMagic,0,2*DAMAGE_MULTIPLIER,dir));
      if (get_bit(quest_rules,qr_MAGICWAND))
        game.magic-=(WANDDRAINAMOUNT*game.magicdrainrate);
      sfx(WAV_WAND,pan(wx));

      /* Fireball Wand
        Lwpns.add(new weapon((fix)wx,(fix)wy,ewFireball,0,2*DAMAGE_MULTIPLIER,dir));
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
      Lwpns.spr(Lwpns.Count()-1)->id=wRefFireball;
      Lwpns.spr(Lwpns.Count()-1)->clk=16;
      ((weapon*)Lwpns.spr(Lwpns.Count()-1))->step=3.5;
      Lwpns.spr(Lwpns.Count()-1)->dummy_bool[0]=true; //homing
      if (get_bit(quest_rules,qr_MAGICWAND))
      game.magic-=(WANDDRAINAMOUNT*game.magicdrainrate);
      sfx(WAV_WAND,pan(wx));
      */
      break;

    case iSword:
      if(Lwpns.idCount(wBeam)||Lwpns.idCount(wMagic))
        return false;
      //  Lwpns.add(new weapon(wx,wy,wBeam,0,((get_bit(quest_rules,qr_BEAMHALFPWR))?1:2)<<(game.sword-1),dir));
      //  ((x<<z)>>y)
      //  ((DAMAGE_MULTIPLIER<<(game.sword-1))>>(get_bit(quest_rules,qr_BEAMHALFPWR)))
      //  x=DAMAGE_MULTIPLIER
      //  y=(game.sword-1)
      //  z=(get_bit(quest_rules,qr_BEAMHALFPWR))


      //  Lwpns.add(new weapon(wx,wy,wBeam,0,((DAMAGE_MULTIPLIER<<(game.sword-1))>>(get_bit(quest_rules,qr_BEAMHALFPWR))),dir));
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
        ((game.magic+game.dmagic)<CANDLEDRAINAMOUNT*game.magicdrainrate)&&
        ((get_bit(quest_rules,qr_MAGICCANDLE))))
        return false;
      didstuff|=did_candle;
      Lwpns.add(new weapon((fix)wx,(fix)wy,wFire,(wpn==iBCandle)?0:1,1*DAMAGE_MULTIPLIER,dir));
      if (get_bit(quest_rules,qr_MAGICCANDLE))
        game.magic-=(CANDLEDRAINAMOUNT*game.magicdrainrate);
      sfx(WAV_FIRE,pan(wx));
      attack=wFire;
      break;

    case iGArrow:
    case iSArrow:
    case iArrow:
      if(Lwpns.idCount(wArrow))
        return false;
      if(game.drupy+game.rupies<=0)
        return false;
      --game.drupy;
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
        game.items[itype_bait]=0;
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
      if(Lwpns.idCount(wHookshot))
        return false;
      if (dir==up)
      {
        if ((combobuf[MAPDATA(x,y-7)].type==cHSGRAB)||
          (_walkflag(x+2,y+4,1) && !isstepable(MAPDATA(int(x),int(y+4)))))
        {
          use_hookshot=false;
        }
      }

      if (dir==down)
      {
        if (int(x)&8)
        {
          if ((combobuf[MAPDATA(x+16,y+23)].type==cHSGRAB))
          {
            use_hookshot=false;
          }
        }
        else if ((combobuf[MAPDATA(x,y+23)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
      }

      if (dir==left)
      {
        if (int(y)&8)
        {
          if ((combobuf[MAPDATA(x-7,y+16)].type==cHSGRAB))
          {
            use_hookshot=false;
          }
        }
        else if ((combobuf[MAPDATA(x-7,y)].type==cHSGRAB))
        {
          use_hookshot=false;
        }
      }

      if (dir==right)
      {
        if (int(y)&8)
        {
          if ((combobuf[MAPDATA(x+23,y+16)].type==cHSGRAB))
          {
            use_hookshot=false;
          }
        }
        else if ((combobuf[MAPDATA(x+23,y)].type==cHSGRAB))
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
        ((game.magic+game.dmagic)<DINSFIREDRAINAMOUNT*game.magicdrainrate))
        return false;
      game.magic-=(DINSFIREDRAINAMOUNT*game.magicdrainrate);
      action=casting;
      magictype=mgc_dinsfire;
      break;
    case iFaroresWind:
      if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
        ((game.magic+game.dmagic)<FARORESWINDDRAINAMOUNT*game.magicdrainrate))
        return false;
      game.magic-=(FARORESWINDDRAINAMOUNT*game.magicdrainrate);
      action=casting;
      magictype=mgc_faroreswind;
      break;
    case iNayrusLove:
      if((get_bit(quest_rules,qr_ENABLEMAGIC))&&
        ((game.magic+game.dmagic)<NAYRUSLOVEDRAINAMOUNT*game.magicdrainrate))
        return false;
      game.magic-=(NAYRUSLOVEDRAINAMOUNT*game.magicdrainrate);
      action=casting;
      magictype=mgc_nayruslove;
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
      templife=templife*game.maxlife;
      templife=templife/100;
    }
    else
    {
      templife*=HP_PER_HEART;
    }

    if(attack==wSword && game.life+((HP_PER_HEART/2)-1)>=templife)
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
  //  if(cBbtn() && lensclk==0 && (lenscnt > 0 || game.drupy+game.rupies > 0))
  //  ((get_bit(quest_rules,qr_ENABLEMAGIC))? (game.drupy+game.rupies > 0) : (game.dmagic+game.magic > 0))
  if(DrunkcBbtn() && lensclk==0 && (lenscnt > 0 ||
    ((get_bit(quest_rules,qr_ENABLEMAGIC)) ?
    (game.dmagic+game.magic > 0) :
    (game.drupy+game.rupies > 0))))
  {
    if(lenscnt)
      --lenscnt;
    else
    {
      if (get_bit(quest_rules,qr_ENABLEMAGIC))
      {
        if (!(magicdrainclk%LENSDRAINSPEED))
        {
          game.magic-=LENSDRAINAMOUNT*game.magicdrainrate;
        }
      }
      else
      {
        game.drupy-=LENSDRAINAMOUNT/2;
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

    if(!(int(x)&7) && !(int(y)&7))
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
    if(dir<left ? !(int(x)&7) && !(int(y)&15) : !(int(x)&15) && !(int(y)&7))
    {
      action = none;
      hopclk = 0;
      diveclk = 0;
      if(iswater(MAPDATA(int(x),int(y)+8)))
      {
        // hopped in
        action = swimming;
      }
    }
    else
    {
      linkstep();
      linkstep();
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

void LinkClass::do_rafting()
{

  if(Bwpn==iLens&&itemclk==0)
    do_lens();

  linkstep();
  if(!(int(x)&15) && !(int(y)&15))
  {
    if(MAPFLAG(x,y)==mfRAFT_BRANCH)
    {
      if(dir!=down && DrunkUp() && isRaftFlag(nextflag(x,y,up)))
      {
        dir = up;
        goto skip;
      }
      if(dir!=up && DrunkDown() && isRaftFlag(nextflag(x,y,down)))
      {
        dir = down;
        goto skip;
      }
      if(dir!=right && DrunkLeft() && isRaftFlag(nextflag(x,y,left)))
      {
        dir = left;
        goto skip;
      }
      if(dir!=left && DrunkRight() && isRaftFlag(nextflag(x,y,right)))
      {
        dir = right;
        goto skip;
      }
    }

    if(!isRaftFlag(nextflag(x,y,dir)))
    {
      if(dir<left)
      {
        if(isRaftFlag(nextflag(x,y,right)))
          dir=right;
        else if(isRaftFlag(nextflag(x,y,left)))
          dir=left;
        else if(y>0 && y<160)
          action=none;
      }
      else
      {
        if(isRaftFlag(nextflag(x,y,down)))
          dir=down;
        else if(isRaftFlag(nextflag(x,y,up)))
          dir=up;
        else if(x>0 && x<240)
          action=none;
      }
    }
  }

  skip:

  switch(dir)
  {
    case up:    --y; break;
    case down:  ++y; break;
    case left:  --x; break;
    case right: ++x; break;
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
    if(DrunkrAbtn())
      diveclk=80;
  }

  if(action==rafting)
  {
    do_rafting();
    if(action==rafting)
      return;
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
              ((game.magic+game.dmagic)<CANDLEDRAINAMOUNT*game.magicdrainrate)&&
              ((get_bit(quest_rules,qr_MAGICCANDLE))))) {
             didstuff|=did_candle;
             Lwpns.add(new weapon((fix)wx,(fix)wy,wFire,4,4*DAMAGE_MULTIPLIER,0));
             if (!(frame%24)&&(get_bit(quest_rules,qr_MAGICCANDLE))) {
               game.magic-=(CANDLEDRAINAMOUNT*game.magicdrainrate);
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
      return;
    }
    autostep=false;
    if(xoff||yoff)
    {
      if(dir==up)
      {
        // int xstep=(lsteps[int(x)&7]);
        // int ystep=lsteps[int(y)&7];
        if(int(x)&8)
        {
          if(!walkflag(x,y+8-(lsteps[int(y)&7]),2,up))
            move(up);
          else action=none;
        }
        //       else if(!walkflag(x+8,y+9,1,up))
        else if(!walkflag(x,y+8-(lsteps[int(y)&7]),2,up))
          move(up);
        else action=none;
      }
      if(dir==down)
      {
        if(int(x)&8)
        {
          if(!walkflag(x,y+15+(lsteps[int(y)&7]),2,down))
            move(down);
          else action=none;
        }
        //       else if(!walkflag(x+8,y+16,1,down))
        else if(!walkflag(x,y+15+(lsteps[int(y)&7]),2,down))
          move(down);
        else action=none;
      }
      if(dir==left)
      {
        if(!walkflag(x-(lsteps[int(x)&7]),y+8,1,left))
          move(left);
        else action=none;
      }
      if(dir==right)
      {
        if(!walkflag(x+15+(lsteps[int(x)&7]),y+8,1,right))
          move(right);
        else action=none;
      }
      return;
    }
  }

  if((action!=swimming)&&(action!=casting))
  {
    action=none;
  }

  if(isdungeon() && (x<=26 || x>=214) && !get_bit(quest_rules,qr_FREEFORM))
    goto LEFTRIGHT;

  // make it easier to get in left & right doors
  if(isdungeon() && DrunkLeft() && x==32 && y==80 && !walkflag(x-(lsteps[int(x)&7]),y+8,1,left))
  {
    move(left);
    return;
  }
  if(isdungeon() && DrunkRight() && x==208 && y==80 && !walkflag(x+15+(lsteps[int(x)&7]),y+8,1,right))
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
          move(dir);
        else if(xoff>=6)
          move(right);
        else if(xoff>=1)
          move(left);
      }
    }
    else
    {
      if((isdungeon())||(int(x)&8))
      {
        if(!walkflag(x,y+8-(lsteps[int(y)&7]),2,up))
        {
          move(up);
          return;
        }
      }
      else
      {
        //       if(!walkflag(x,y+7,1,up)) {
        if(!walkflag(x,y+8-(lsteps[int(y)&7]),2,up))
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
          linkstep();
        return;
      } else goto LEFTRIGHT;
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
    }
    else
    {
      if((isdungeon())||(int(x)&8))
      {
        if(!walkflag(x,y+15+(lsteps[int(y)&7]),2,down))
        {
          move(down);
          return;
        }
      }
      else
      {
        //       if(!walkflag(x,y+16,1,down)) {
        if(!walkflag(x,y+15+(lsteps[int(y)&7]),2,down))
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
          linkstep();
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
          move(dir);
        else if(yoff>=6)
          move(down);
        else if(yoff>=1)
          move(up);
      }
    }
    else
    {
      if(!walkflag(x-(lsteps[int(x)&7]),y+8,1,left))
        move(left);
      else if( !DrunkUp() && !DrunkDown() )
      {
        pushing=push+1;
        dir=left;
        if(action!=swimming)
          linkstep();
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
          move(dir);
        else if(yoff>=6)
          move(down);
        else if(yoff>=1)
          move(up);
      }
    }
    else
    {
      if(!walkflag(x+15+(lsteps[int(x)&7]),y+8,1,right))
        move(right);
      else if( !DrunkUp() && !DrunkDown() )
      {
        pushing=push+1;
        dir=right;
        if(action!=swimming)
          linkstep();
        return;
      }
    }
  }

  /* Use this code for free movement
   if(Up()) {
       if((isdungeon())||(int(x)&8)) {
         if(!walkflag(x,y+7,2,up)) {
           move(up);
           return;
         }
       } else {
  //       if(!walkflag(x,y+7,1,up)) {
         if(!walkflag(x,y+7,2,up)) {
           move(up);
  return;
  }
  }

  if( !Left() && !Right() ) {
  pushing=push+1;
  dir=up;
  if(action!=swimming)
  linkstep();
  return;
  } else goto LEFTRIGHT;
  return;
  }

  if(Down()) {
  if((isdungeon())||(int(x)&8)) {
  if(!walkflag(x,y+16,2,down)) {
  move(down);
  return;
  }
  } else {
  //       if(!walkflag(x,y+16,1,down)) {
  if(!walkflag(x,y+16,2,down)) {
  move(down);
  return;
  }
  }
  if( !Left() && !Right() ) {
  pushing=push+1;
  dir=down;
  if(action!=swimming)
  linkstep();
  return;
  }
  else goto LEFTRIGHT;
  return;
  }

  LEFTRIGHT:

  if(isdungeon() && (y<=26 || y>=134))
  return;

  if(Left()) {
  if(!walkflag(x-1,y+8,1,left))
  move(left);
  else if( !Up() && !Down() ) {
  pushing=push+1;
  dir=left;
  if(action!=swimming)
  linkstep();
  return;
  }
  return;
  }

  if(Right()) {
  if(!walkflag(x+16,y+8,1,right))
  move(right);
  else if( !Up() && !Down() ) {
  pushing=push+1;
  dir=right;
  if(action!=swimming)
  linkstep();
  return;
  }
  }
  */

}

void LinkClass::move(int d)
{
  if(inlikelike)
    return;
  int dx=0,dy=0;
  int xstep=lsteps[int(x)&7];
  int ystep=lsteps[int(y)&7];
  // xstep=ystep=0;
  if(combobuf[MAPDATA(x+7,y+8)].type==cWALKSLOW)
  {
    if(d<left)
    {
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
  switch(d)
  {
    case up:    dy-=ystep; break;
    case down:  dy+=ystep; break;
    case left:  dx-=xstep; break;
    case right: dx+=xstep; break;
  }
  dir=d;
  linkstep();
  if(action!=swimming)
    action=walking;
  sprite::move((fix)dx,(fix)dy);
}

bool LinkClass::walkflag(int wx,int wy,int cnt,byte d)
{
  if(toogam) return false;
  if(blockpath && wy<88)
    return true;

  // for(int i=0; i<4; i++)
  if(mblock2.clk && mblock2.hit(wx,wy,d<=down?16:1,1))
    return true;

  if(isdungeon() && currscr<128 && wy<40 && (x!=120 || _walkflag(120,24,2))
    && !get_bit(quest_rules,qr_FREEFORM) )
    return true;

  bool wf = _walkflag(wx,wy,cnt);

  if(action==swimming)
  {
    if(!wf)
    {
      // hop out of the water
      hopclk = 16;
    }
    else
    {
      bool wtrx  = iswater(MAPDATA(wx,wy));
      bool wtrx8 = iswater(MAPDATA(x+8,wy));

      if((d>=left && wtrx) || (d<=down && wtrx && wtrx8))
      {
        // swim
        hopclk = 0xFF;
        return false;
      }
    }
  }
  else if(ladderx+laddery)                                  // ladder is being used
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
  else if(wf)
  {
    // see if it's a good spot for the ladder or for swimming
    bool wtrx  = iswater(MAPDATA(wx,wy));
    bool wtrx8 = iswater(MAPDATA(x+8,wy));
    bool flgx  = _walkflag(wx,wy,1) && !wtrx;
    bool flgx8 = _walkflag(x+8,wy,1) && !wtrx8;

    // check if he can swim
    if(current_item(itype_flippers,true))
    {
      if((d>=left && wtrx) || (d<=down && wtrx && wtrx8))
      {
        hopclk = 16;
        return false;
      }
    }

    // check if he can use the ladder
    if(can_use_item(itype_ladder, i_ladder) && (tmpscr->flags&fLADDER || isdungeon()))
    {
      // add ladder combos
      wtrx  = isstepable(MAPDATA(wx,wy));
      wtrx8 = isstepable(MAPDATA(x+8,wy));
      flgx  = _walkflag(wx,wy,1) && !wtrx;
      flgx8 = _walkflag(x+8,wy,1) && !wtrx8;

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
  return wf;
}

void LinkClass::checkpushblock()
{
  if(toogam) return;
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
      break;
    case down:
      if (y>128)
      {
        return;
      }
      else
      {
        by+=16;
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
  int t = combobuf[MAPDATA(bx,by)].type;

  if( (t==cPUSH_WAIT || t==cPUSH_HW || t==cPUSH_HW2) && (pushing<16 || hasMainGuy()) ) return;
  if( (t==cPUSH_HEAVY || t==cPUSH_HW) && (current_item(itype_bracelet,true)<1) ) return;
  if( (t==cPUSH_HEAVY2 || t==cPUSH_HW2) && (current_item(itype_bracelet,true)<2) ) return;

  if(get_bit(quest_rules,qr_HESITANTPUSHBLOCKS)&&(pushing<4)) return;

  bool doit=false;

  if (((f==mfPUSHUD || f==mfPUSHUDNS|| f==mfPUSHUDINS) && dir<=down) ||
    ((f==mfPUSHLR || f==mfPUSHLRNS|| f==mfPUSHLRINS) && dir>=left) ||
    ((f==mfPUSHU || f==mfPUSHUNS || f==mfPUSHUINS) && dir==up) ||
    ((f==mfPUSHD || f==mfPUSHDNS || f==mfPUSHDINS) && dir==down) ||
    ((f==mfPUSHL || f==mfPUSHLNS || f==mfPUSHLINS) && dir==left) ||
    ((f==mfPUSHR || f==mfPUSHRNS || f==mfPUSHRINS) && dir==right) ||
    f==mfPUSH4 || f==mfPUSH4NS || f==mfPUSH4INS)
    doit=true;

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
    case up:    if(MAPFLAG(bx,by-8)==mfNOBLOCKS)    doit=false; break;
    case down:  if(MAPFLAG(bx,by+24)==mfNOBLOCKS)   doit=false; break;
    case left:  if(MAPFLAG(bx-16,by+8)==mfNOBLOCKS) doit=false; break;
    case right: if(MAPFLAG(bx+16,by+8)==mfNOBLOCKS) doit=false; break;
  }

  if(doit)
  {
    //   for(int i=0; i<1; i++)
    if (!blockmoving)
    {
      tmpscr->sflag[(by&0xF0)+(bx>>4)]=0;
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
    if(game.keys==0)
      return false;
    --game.keys;
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
  if ((combobuf[MAPDATA(bx,by)].type==cLOCKBLOCK)||
    (combobuf[MAPDATA(bx2,by)].type==cLOCKBLOCK))
  {
    found=true;
  }
  if (!found)
  {
    for (int i=0; i<2; i++)
    {
      if ((combobuf[MAPDATA2(i,bx,by)].type==cLOCKBLOCK)||
        (combobuf[MAPDATA2(i,bx2,by)].type==cLOCKBLOCK))
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
  if ((combobuf[MAPDATA(bx,by)].type==cBOSSLOCKBLOCK)||
    (combobuf[MAPDATA(bx2,by)].type==cBOSSLOCKBLOCK))
  {
    found=true;
  }
  if (!found)
  {
    for (int i=0; i<2; i++)
    {
      if ((combobuf[MAPDATA2(i,bx,by)].type==cBOSSLOCKBLOCK)||
        (combobuf[MAPDATA2(i,bx2,by)].type==cBOSSLOCKBLOCK))
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

  if(!(game.lvlitems[dlevel]&liBOSSKEY)) return;
  setmapflag(mBOSSLOCKBLOCK);
  remove_bosslockblocks((currscr>=128)?1:0,true);
  sfx(WAV_DOOR);
}

void LinkClass::checklocked()
{
  if(toogam) return;
  if(!isdungeon()) return;
  if(pushing!=8) return;
  if((tmpscr->door[dir]!=dLOCKED) && (tmpscr->door[dir]!=dBOSS)) return;

  int si = (currmap<<7) + currscr;
  int di = nextscr(dir);

  switch(dir)
  {
    case up:
      if(y>32 || x!=120) return;
      if (tmpscr->door[dir]==dLOCKED)
      {
        if(usekey())
        {
          putdoor(0,up,dUNLOCKED);
          tmpscr->door[0]=dUNLOCKED;
          game.maps[si] |= 1;
          game.maps[di] |= 2;
        } else return;
      }
      else if (tmpscr->door[dir]==dBOSS)
      {
        if(game.lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,up,dOPENBOSS);
          tmpscr->door[0]=dOPENBOSS;
          game.maps[si] |= 1;
          game.maps[di] |= 2;
        } else return;
      }
      break;
    case down:
      if(y!=128 || x!=120) return;
      if (tmpscr->door[dir]==dLOCKED)
      {
        if(usekey())
        {
          putdoor(0,down,dUNLOCKED);
          tmpscr->door[1]=dUNLOCKED;
          game.maps[si] |= 2;
          game.maps[di] |= 1;
        } else return;
      }
      else if (tmpscr->door[dir]==dBOSS)
      {
        if(game.lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,down,dOPENBOSS);
          tmpscr->door[1]=dOPENBOSS;
          game.maps[si] |= 2;
          game.maps[di] |= 1;
        } else return;
      }
      break;
    case left:
      if(y!=80 || x!=32) return;

      if (tmpscr->door[dir]==dLOCKED)
      {
        if(usekey())
        {
          putdoor(0,left,dUNLOCKED);
          tmpscr->door[2]=dUNLOCKED;
          game.maps[si] |= 4;
          game.maps[di] |= 8;
        } else return;
      }
      else if (tmpscr->door[dir]==dBOSS)
      {
        if(game.lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,left,dOPENBOSS);
          tmpscr->door[2]=dOPENBOSS;
          game.maps[si] |= 4;
          game.maps[di] |= 8;
        } else return;
      }
      break;
    case right:
      if(y!=80 || x!=208) return;
      if (tmpscr->door[dir]==dLOCKED)
      {
        if(usekey())
        {
          putdoor(0,right,dUNLOCKED);
          tmpscr->door[3]=dUNLOCKED;
          game.maps[si] |= 8;
          game.maps[di] |= 4;
        } else return;
      }
      else if (tmpscr->door[dir]==dBOSS)
      {
        if(game.lvlitems[dlevel]&liBOSSKEY)
        {
          putdoor(0,right,dOPENBOSS);
          tmpscr->door[3]=dOPENBOSS;
          game.maps[si] |= 8;
          game.maps[di] |= 4;
        } else return;
      }
  }
  sfx(WAV_DOOR);
  markBmap(-1);
}

void LinkClass::fairycircle()
{
  if(didstuff&did_fairy)
    return;

  if(fairyclk==0)
  {
    refill_what=REFILL_LIFE;
    StartRefill(REFILL_FAIRY);
    action=freeze;
    holdclk=0;
    hopclk=0;
  }

  ++fairyclk;

  if(!refill() && ++holdclk>80)
  {
    action=none;
    didstuff|=did_fairy;
  }
}

int touchcombo(int x,int y)
{
  switch(combobuf[MAPDATA(x,y)].type)
  {
    case cBSGRAVE:
    case cGRAVE:
      if(MAPFLAG(x,y))
      {
        break;
      }
      // fall through
    case cARMOS:
      {
        return combobuf[MAPDATA(x,y)].type;
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
      if(touchcombo(x,y+7))
      {
        tx=x; ty=y+7;
      }
      else if(touchcombo(x+8,y+7))
      {
        tx=x+8; ty=y+7;
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
      switch(combobuf[MAPDATA(tx,ty)].type)
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

    // want actual screen index, not game.maps[] index
    ns = (ns&127) + (ns>>7)*MAPSCRS;

    switch(cdir)
    {
      case up:    cy=160; break;
      case down:  cy=0; break;
      case left:  cx=240; break;
      case right: cx=0; break;
    }

    // from MAPDATA()
    int cmb = (cy&0xF0)+(cx>>4);
    if(cmb>175)
      return 0;
    return TheMaps[ns].data[cmb];                           // entire combo code
  }

  return MAPDATA(cx,cy);
}

int LinkClass::nextflag(int cx, int cy, int cdir)
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

    // want actual screen index, not game.maps[] index
    ns = (ns&127) + (ns>>7)*MAPSCRS;

    switch(cdir)
    {
      case up:    cy=160; break;
      case down:  cy=0; break;
      case left:  cx=240; break;
      case right: cx=0; break;
    }

    // from MAPDATA()
    int cmb = (cy&0xF0)+(cx>>4);
    if(cmb>175)
      return 0;
    return TheMaps[ns].sflag[cmb];                          // flag
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
    game.lvlitems[dlevel]|=liBOSS;
    stop_sfx(WAV_ROAR);
    stop_sfx(WAV_VADER);
    stop_sfx(WAV_DODONGO);
  }

  if(toogam) return;

  // check if he's standing on a warp he just came out of
  if((int(y)&0xF8)==warpy)
    if(x==warpx)
      return;
  warpy=255;

  // check alignment for the rest of these
  if(int(y)&7)
    return;
  if(int(x)&7)
    return;

  int flag = MAPFLAG(x,y);
  int type = COMBOTYPE(x,y);

  if(flag==mfFAIRY)
  {
    fairycircle();
    return;
  }

  if(flag==mfZELDA)
  {
    saved_Zelda();
    return;
  }

  if(int(x)&15)
  {
    if((COMBOTYPE(x,y)!=COMBOTYPE(x+8,y))&&
      (COMBOTYPE(x,y)!=cPIT)&&(COMBOTYPE(x+8,y)!=cPIT))
      return;
    if (COMBOTYPE(x+8,y+8)==cPIT)
    {
      type=cPIT;
    }
  }

  if(int(y)&15)
  {
    if((COMBOTYPE(x,y)!=COMBOTYPE(x,y+8))&&
      (COMBOTYPE(x,y+8)!=cPIT))
      return;
    if (COMBOTYPE(x,y+8)==cPIT)
    {
      type=cPIT;
    }
  }

  if(type==cTRIGNOFLAG || type==cTRIGFLAG)
  {
    if(type==cTRIGFLAG && !isdungeon())
    {
      setmapflag(mSECRET);
      hidden_entrance(0,true,false);
    }
    else
      hidden_entrance(0,true,true);
  }

  if(type!=cCAVE && type!=cCAVE2 && type!=cSTAIR &&
    type!=cPIT && type!=cSWIMWARP && !(type==cDIVEWARP && diveclk>30))
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
          if(isRaftFlag(nextflag(x,y,dir)))
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

  if(type==cCAVE)
  {
    music_stop();
    walkdown();
  } else if(type==cCAVE2)
  {
    music_stop();
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

  dowarp(0);
}

int selectWlevel(int d)
{
  if(TriforceCount()==0)
    return 0;

  byte l = game.wlevel;

  do
  {
    if(d==0 && (game.lvlitems[l+1] & liTRIFORCE))
      break;
    else if(d<0)
      l = (l==0) ? 7 : l-1;
    else
      l = (l==7) ? 0 : l+1;
  } while( !(game.lvlitems[l+1] & liTRIFORCE) );

  game.wlevel = l;
  return l;
}

bool LinkClass::dowarp(int type)
{
  byte wdmap=0,wscr=0,wtype=0,t=0;
  t=(currscr<128)?0:1;
  switch(type)
  {
    case 0:                                                 // tile warp
      wtype = tmpscr[t].tilewarptype;
      wdmap = tmpscr[t].tilewarpdmap;
      wscr = tmpscr[t].tilewarpscr;
      break;
    case 1:                                                 // side warp
      wtype = tmpscr[t].sidewarptype;
      wdmap = tmpscr[t].sidewarpdmap;
      wscr = tmpscr[t].sidewarpscr;
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
        blackscr(30,(COOLSCROLL&&((combobuf[MAPDATA(x,y-16)].type==cCAVE)||(combobuf[MAPDATA(x,y-16)].type==cCAVE2)))?false:true);
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
        stepforward(6);
      }
      else                                                  // item room
      {
        stop_sfx(WAV_ROAR);
        stop_sfx(WAV_VADER);
        stop_sfx(WAV_DODONGO);
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
        y=0;
        // is thid didpit check necessary?
        if (didpit)
        {
          didpit=false;
          x=pitx;
          y=pity;
        }
        reset_hookshot();
        dontdraw=false;
        stepforward(18);
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
      // is thid didpit check necessary?
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
      stepforward(18);
      newscr_clk=frame;
      activated_timed_warp=false;
    } break;

    case wtEXIT:                                            // entrance/exit
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
      lastentrance_dmap = currdmap;
      homescr = currscr = wscr + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
      if (dlevel)
      {
        lastentrance = currscr;
      }
      else
      {
        lastentrance = DMaps[currdmap].cont + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
      }
      loadscr(0,currscr,-1);
      if(dlevel)
      {
        x=tmpscr->warparrivalx;
        y=tmpscr->warparrivaly;
      }
      else
      {
        x=tmpscr->warpreturnx;
        y=tmpscr->warpreturny;
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
          game.guys[(currmap<<7)+i] = 0;
          game.maps[(currmap<<7)+i] &= ~mTMPNORET;
        }
      }
      markBmap(dir^1);
      reset_hookshot();
      if(isdungeon())
      {
        openscreen();
        stepforward(12);
      }
      else
      {
        if(!COOLSCROLL)
          openscreen();

        if(combobuf[MAPDATA(x,y-16)].type==cCAVE)
        {
          reset_pal_cycling();
          putscr(scrollbuf,0,0,tmpscr);
          walkup();
        }
        else if(combobuf[MAPDATA(x,y+16)].type==cCAVE2)
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
      play_DmapMusic();
      currcset=DMaps[currdmap].color;
      dointro();
      warpx=x;
      warpy=y;
      for(int i=0; i<6; i++)
        visited[i]=-1;
      break;

    case wtSCROLL:                                          // scrolling warp
    {
      int c = DMaps[currdmap].color;
      currmap = DMaps[wdmap].map;
      lighting(4,dir);
      scrollscr(dir, wscr+DMaps[wdmap].xoff, wdmap);
      reset_hookshot();
      currdmap = wdmap;
      dlevel = DMaps[currdmap].level;
      homescr = currscr = wscr + (((DMaps[wdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[wdmap].xoff);
      if (!intradmap)
      {
        if(!get_bit(quest_rules,qr_NOSCROLLCONTINUE))
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
        loadlvlpal(DMaps[currdmap].color);
      play_DmapMusic();
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
      play_DmapMusic();
      currcset=DMaps[currdmap].color;
      dointro();
      action=inwind;
      Lwpns.add(new weapon((fix)0,(fix)(tmpscr->warparrivaly),wWind,1,0,right));
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
        blackscr(30,(COOLSCROLL&&((combobuf[MAPDATA(x,y-16)].type==cCAVE)||(combobuf[MAPDATA(x,y-16)].type==cCAVE2)))?false:true);
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

      x = tmpscr->warpreturnx;
      y = tmpscr->warpreturny;
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
      if(combobuf[MAPDATA(x,y-16)].type==cCAVE)
      {
        reset_pal_cycling();
        putscr(scrollbuf,0,0,tmpscr);
        walkup();
      }
      else if(combobuf[MAPDATA(x,y+16)].type==cCAVE2)
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
      play_DmapMusic();
      currcset=DMaps[currdmap].color;
      dointro();
      warpx=x;
      warpy=y;
    }
    break;


    case wtNOWARP:
    default:
      return false;
  }

  if(action!=rafting && iswater(MAPDATA(x,y+8))
    && (current_item(itype_flippers, true)) && (action!=inwind))
  {
    hopclk=0xFF;
    action=swimming;
  }
  newscr_clk=frame;
  activated_timed_warp=false;
  eat_buttons();
  attackclk=0;
  didstuff=0;
  map_bkgsfx();
  loadside=dir^1;
  whistleclk=-1;
  if ((DMaps[currdmap].type&dmfCONTINUE) || (currdmap==0))
  {
    if (dlevel)
    {
      if ( (wtype == wtEXIT)
          || (((wtype == wtSCROLL) && !intradmap)))
      {
        if(!(wtype==wtSCROLL)||!(get_bit(quest_rules,qr_NOSCROLLCONTINUE)))
        {
          game.continue_scrn=homescr;
          //Z_message("continue_scrn = %02X\n e/e",get_gamedata_continue_scrn(game));
        }
      }
      else
      {
        if (currdmap != game.continue_dmap)
        {
          game.continue_scrn=DMaps[currdmap].cont + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
          //Z_message("continue_scrn = %02X\n dlevel",game->continue_scrn);
        }
      }
    }
    else
    {
      game.continue_scrn=DMaps[currdmap].cont + (((DMaps[currdmap].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[currdmap].xoff);
      //Z_message("continue_scrn = %02X\n !dlevel",game->continue_scrn);
    }
    game.continue_dmap=currdmap;
    lastentrance_dmap = currdmap;
    lastentrance = game.continue_scrn;
    //Z_message("continue_map = %d\n",game.continue_dmap);
  }
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
  x = tmpscr->warpreturnx;
  y = tmpscr->warpreturny;
  if (didpit)
  {
    didpit=false;
    x=pitx;
    y=pity;
  }
  if(x+y == 0)
    x = y = 80;
  bool b = ((combobuf[MAPDATA(x,y-16)].type==cCAVE) || (combobuf[MAPDATA(x,y-16)].type==cCAVE2)) && COOLSCROLL;
  blackscr(30,b?false:true);
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);
  lighting(2,dir);
  music_stop();
  kill_sfx();
  ALLOFF();
  putscr(scrollbuf,0,0,tmpscr);
  if(combobuf[MAPDATA(x,y-16)].type==cCAVE)
  {
    walkup();
  }
  else if(combobuf[MAPDATA(x,y+16)].type==cCAVE2)
  {
    walkdown2();
  }
  show_subscreen_life=true;
  show_subscreen_numbers=true;
  play_DmapMusic();
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



void LinkClass::stepforward(int steps)
{

  int tx=x;           //temp x
  int ty=y;           //temp y
  int tstep=0;        //temp single step distance
  int s=0;            //calculated step distance for all steps

  for (int i=steps; i>0; --i)
  {
    tstep=lsteps[int((dir<left)?ty:tx)&7];

    switch(dir)
    {
      case up:    ty-=tstep; break;
      case down:  ty+=tstep; break;
      case left:  tx-=tstep; break;
      case right: tx+=tstep; break;
    }
    s+=tstep;
  }

    while(s>=0)
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
  draw_screen(tmpscr, 0, 0);
  eat_buttons();
  logic_counter=0;
  drawit=true;
}

void LinkClass::walkdown() //entering cave
{
  if(COOLSCROLL)
  {
    close_black_opening(x+8, y+8+56, false);
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
    if (FrameSkip)
    {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
    }
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
    open_black_opening(x+8, y+8+56+16, false);
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
    if (FrameSkip)
    {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
    }
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
    open_black_opening(x+8, y+8+56-16, false);
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
    if (FrameSkip)
    {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
    }
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
    close_black_opening(x+8, y+8+56, false);
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
    if (FrameSkip)
    {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
    }
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
    currdmap=tmpscr[1].tilewarpdmap;
    currmap=DMaps[currdmap].map;
    dlevel=DMaps[currdmap].level;
    homescr=tmpscr[1].tilewarpscr+DMaps[currdmap].xoff;
  }
  currscr=homescr;
  loadscr(0,currscr,255);                                   // bogus direction
  draw_screen(tmpscr, 0, 0);
  fade(DMaps[currdmap].color,true,true,false);
  x = tmpscr->warpreturnx;
  y = tmpscr->warpreturny;
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

bool LinkClass::nextcombo_wf()
{
  if(toogam || action!=swimming || hopclk==0)
    return false;

  // assumes Link is about to scroll screens

  int ns = nextscr(dir);

  // want actual screen index, not game.maps[] index
  ns = (ns&127) + (ns>>7)*MAPSCRS;

  int cx = x;
  int cy = y;

  switch(dir)
  {
    case up:    cy=160; break;
    case down:  cy=0; break;
    case left:  cx=240; break;
    case right: cx=0; break;
  }

  // check lower half of combo
  cy += 8;

  // from MAPDATA()
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
    y=0;
    if(nextcombo_wf())
      return;
    if(currscr>=128)
      stepout();
    else if(tmpscr->flags2&wfUP)
      dowarp(1);
    else if(!edge_of_dmap(up))
      scrollscr(up);
  }
  if(y>160)
  {
    y=160;
    if(nextcombo_wf())
      return;
    if(currscr>=128)
      exitcave();
    else if(tmpscr->flags2&wfDOWN)
      dowarp(1);
    else if(!edge_of_dmap(down))
      scrollscr(down);
  }
  if(x<0)
  {
    x=0;
    if(nextcombo_wf())
      return;
    if(tmpscr->flags2&wfLEFT)
      dowarp(1);
    else if(!edge_of_dmap(left))
      scrollscr(left);
  }
  if(x>240)
  {
    x=240;
    if(nextcombo_wf())
      return;
    if(action==inwind)
      dowarp(2);
    else if(tmpscr->flags2&wfRIGHT)
      dowarp(1);
    else if(!edge_of_dmap(right))
      scrollscr(right);
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

int LinkClass::lookahead(int destscr)                       // Helper for scrollscr that gets next combo on next screen.
{
  // Can use destscr for scrolling warps,
  // but assumes currmap is correct.

  int s = currscr;
  int cx = x;
  int cy = y + 8;

  switch(dir)
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

int LinkClass::lookaheadflag(int destscr)
{                                                           // Helper for scrollscr that gets next combo on next screen.
  // Can use destscr for scrolling warps,
  // but assumes currmap is correct.

  int s = currscr;
  int cx = x;
  int cy = y + 8;

  switch(dir)
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

  int ahead = lookahead(destscr);
  int aheadflag = lookaheadflag(destscr);

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
        hopclk = 16;
      }
    }
  }
  lstep=(lstep+6)%12;
  cx = scx;
  do
  {
    if (FrameSkip)
    {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
    }
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
    if (FrameSkip)
    {
      logic_counter=0;
      while(logic_counter==0)
      {
      }
      drawit=true;
    }
    switch(dir)
    {
      case up:    sy-=step; break;
      case down:  sy+=step; break;
      case left:  sx-=step; break;
      case right: sx+=step; break;
    }
    //    putsubscr(framebuf,0,0);
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
    blit(scrollbuf,framebuf,sx,sy,0,56,256,168);
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

    //    linkstep();
    //    draw_screen(newscr, oldscr, tx, ty, tx2, ty2);

    do_layer(framebuf, 0, oldscr, tx2, ty2, 3);
    do_layer(framebuf, 1, oldscr, tx2, ty2, 3);
    do_layer(framebuf, 0, newscr, tx, ty, 2);
    do_layer(framebuf, 1, newscr, tx, ty, 2);
    do_layer(framebuf, -2, oldscr, tx2, ty2, 3);
    do_layer(framebuf, -2, newscr, tx, ty, 2);
    linkstep();
    if(!isdungeon()||get_bit(quest_rules,qr_FREEFORM))
    {
      draw_under(framebuf);
      draw(framebuf);
    }
    do_layer(framebuf, 2, oldscr, tx2, ty2, 3);
    do_layer(framebuf, 3, oldscr, tx2, ty2, 3);
    do_layer(framebuf, -1, oldscr, tx2, ty2, 3);
    do_layer(framebuf, 4, oldscr, tx2, ty2, 3);
    do_layer(framebuf, 5, oldscr, tx2, ty2, 3);

    do_layer(framebuf, 2, newscr, tx, ty, 2);
    do_layer(framebuf, 3, newscr, tx, ty, 2);
    do_layer(framebuf, -1, newscr, tx, ty, 2);
    do_layer(framebuf, 4, newscr, tx, ty, 2);
    do_layer(framebuf, 5, newscr, tx, ty, 2);

    putsubscr(framebuf,0,0);

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

  if(MAPFLAG(x,y)==mfRAFT && action!=rafting && hopclk==0)
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
        stepforward(12);
        break;
      case dSHUTTER:
      case d1WAYSHUTTER:
        stepforward(24);
        putdoor(0,dir^1,tmpscr->door[dir^1]);
        opendoors=-4;
        sfx(WAV_DOOR);
        break;
      default:
        stepforward(24);
    }
  }

  if(action==scrolling)
    action=none;

  map_bkgsfx();
  if(newscr->flags2&fSECRET)
  {
    sfx(WAV_SECRET);
  }

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

  switch(zinit.subscreen)
  {
    case 0:                                                 //original
    case 1:                                                 //revision 1 ("New Subscreen")
      switch(pos)
      {
        case 0: if(current_item(itype_brang,true)==3) return iFBrang;
        if(current_item(itype_brang,true)) return current_item(itype_brang,true)-1+iBrang; break;
        case 1: if(current_item(itype_bomb,true)) return iBombs; break;
        case 2: if(current_item(itype_bow,true) && current_item(itype_arrow,true))
        {
          if (current_item(itype_arrow,true)<3)
          {
            return current_item(itype_arrow,true)-1+iArrow;
          }
          else
          {
            return iGArrow;
          }
        } break;
        case 3: if(current_item(itype_candle,true)) return current_item(itype_candle,true)-1+iBCandle; break;
        case 4: if(current_item(itype_whistle,true)) return iWhistle; break;
        case 5: if(current_item(itype_bait,true)) return iBait; break;
        case 6: if(current_item(itype_potion,true)) return current_item(itype_potion,true)-1+iBPotion;
        if(current_item(itype_letter,true)) return iLetter; break;
        case 7: if(current_item(itype_wand,true)) return iWand; break;
        case 8: if(current_item(itype_hookshot, true)) return iHookshot; break;
        case 9: if(current_item(itype_sbomb,true)) return iSBomb; break;
        case 10: if(current_item(itype_lens, true)) return iLens; break;
        case 11: if(current_item(itype_hammer, true)) return iHammer; break;
      }
      break;
    case 2:                                                 //revision 2 (1.92 beta 168)
      switch(pos)
      {
        case 0: if(current_item(itype_brang,true)==3) return iFBrang;
        if(current_item(itype_brang,true)) return current_item(itype_brang,true)-1+iBrang; break;
        case 1: if(current_item(itype_bomb,true)) return iBombs; break;
        case 2: if(current_item(itype_bow,true) && current_item(itype_arrow,true))
        {
          if (current_item(itype_arrow,true)<3)
          {
            return current_item(itype_arrow,true)-1+iArrow;
          }
          else
          {
            return iGArrow;
          }
        } break;
        case 3: if(current_item(itype_candle,true)) return current_item(itype_candle,true)-1+iBCandle; break;
        case 4: if(current_item(itype_dinsfire,true)) return iDinsFire; break;
        case 5: if(current_item(itype_whistle,true)) return iWhistle; break;
        case 6: if(current_item(itype_bait,true)) return iBait; break;
        case 7: if(current_item(itype_potion,true)) return current_item(itype_potion,true)-1+iBPotion;
        if(current_item(itype_letter,true)) return iLetter; break;
        case 8: if(current_item(itype_wand,true)) return iWand; break;
        case 9: if(current_item(itype_faroreswind,true)) return iFaroresWind; break;
        case 10: if(current_item(itype_hookshot, true)) return iHookshot; break;
        case 11: if(current_item(itype_sbomb,true)) return iSBomb; break;
        case 12: if(current_item(itype_lens, true)) return iLens; break;
        case 13: if(current_item(itype_hammer, true)) return iHammer; break;
        case 14: if(current_item(itype_nayruslove,true)) return iNayrusLove; break;
        case 15: break;
        case 16: break;
        case 17: break;
        case 18: break;
        case 19: break;
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
    if(Bwpn)
      return;
    xstep=1;
  }

  if((xstep==8)&&(ystep==8))
  {
    Bwpn=Bweapon(Bpos);
    if(Bwpn)
      return;
    xstep=-1;
  }

  int pos = Bpos;
  int cnt=0;
  switch(zinit.subscreen)
  {
    case 0:                                                 //original
      cnt=8;
      break;
    case 1:                                                 //revision 1 ("New Subscreen")
      cnt=12;
      break;
    case 2:                                                 //revision 2 (1.92 beta 168)
      cnt=20;
      break;
  }
  // int cnt = NEWSUBSCR ? 12 : 8;

  do
  {
    switch(zinit.subscreen)
    {
      case 0:                                               //original
        Bpos += xstep;
        Bpos += (ystep*4);
        break;
      case 1:                                               //revision 1 ("New Subscreen")
        Bpos += xstep;
        Bpos += (ystep*4);
        break;
      case 2:                                               //revision 2 (1.92 beta 168)
        Bpos += xstep;
        Bpos += (ystep*5);
        break;
    }

    while(Bpos<0)
      Bpos+=cnt;
    while(Bpos>=cnt)
      Bpos-=cnt;

    Bwpn = Bweapon(Bpos);
    if(Bwpn)
      return;
  } while(Bpos!=pos);

  if(!Bwpn)
    Bpos=0;
}

bool canget(int id)
{
  if(id==iSword && game.maxlife<swordhearts[0]*HP_PER_HEART)
    return false;
  if(id==iWSword && game.maxlife<swordhearts[1]*HP_PER_HEART)
    return false;
  if(id==iMSword && game.maxlife<swordhearts[2]*HP_PER_HEART)
    return false;
  if(id==iXSword && game.maxlife<swordhearts[3]*HP_PER_HEART)
    return false;
  return true;
}

void dospecialmoney(int index)
{
  int tmp=currscr>=128?1:0;
  switch(tmpscr[tmp].room)
  {
    case rINFO:                                             // pay for info
      if(game.rupies < abs(prices[index-1][0]))
        return;
      game.drupy -= abs(prices[index-1][0]);
      msgstr = QMisc.info[tmpscr[tmp].catchall].str[index-1];
      msgclk=msgpos=0;
      rectfill(msgdisplaybuf, 0, 0, msgdisplaybuf->w, 80, 0);
      for(int i=1; i<4; i++)
        ((item*)items.spr(i))->pickup=ipDUMMY;
      break;

    case rMONEY:                                            // secret money
      ((item*)items.spr(0))->pickup=ipDUMMY;
      game.drupy += (prices[0][0]=tmpscr[tmp].catchall);
      putprices(false);
      setmapflag();
      break;

    case rGAMBLE:                                           // gamble
    {
      if(game.rupies<10) return;
      unsigned si=(rand()%24)*3;
      for(int i=0; i<3; i++)
        prices[i][0]=gambledat[si++];
      game.drupy+=prices[index-1][0];
      putprices(true);
      for(int i=1; i<4; i++)
        ((item*)items.spr(i))->pickup=ipDUMMY;
    }break;

    case rBOMBS:
      if(game.rupies<abs(tmpscr[tmp].catchall))
        return;
      game.drupy -= abs(tmpscr[tmp].catchall);
      setmapflag();
      game.maxbombs+=4;
      game.items[itype_bomb]=game.maxbombs;
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
        if(game.rupies<abs(tmpscr[tmp].catchall))
          return;
        game.drupy -= abs(tmpscr[tmp].catchall);
      }
      else
      {
        if(game.maxlife<=HP_PER_HEART)
          return;
        game.life = max(game.life-HP_PER_HEART,0);
        game.maxlife = max(game.maxlife-HP_PER_HEART,(HP_PER_HEART));
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

void getitem(int id)
{
  switch(id)
  {
    case iRupy:         ++game.drupy;   break;
    case i5Rupies:      game.drupy+=5;  break;
    case i20Rupies:     game.drupy+=20; break;
    case i50Rupies:     game.drupy+=50; break;
    case i200Rupies:    game.drupy+=200; break;
    case iWallet500:    game.items[itype_wallet]|=i_swallet; break;
    case iWallet999:    game.items[itype_wallet]|=i_lwallet; break;
    case iBombs:        game.items[itype_bomb]=min(game.items[itype_bomb]+4,game.maxbombs); break;
    case iSBomb:        game.items[itype_sbomb]=min(game.items[itype_sbomb]+1,game.maxbombs>>2); break;
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
          game.items[itype_sword]=(1<<(i_sword-1));
        }
      }
      else
      {
        game.items[itype_sword]|=(1<<(i_sword-1));
      }
      break;
    case iWSword:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_sword,true)<i_wsword)
        {
          game.items[itype_sword]=(1<<(i_wsword-1));
        }
      }
      else
      {
        game.items[itype_sword]|=(1<<(i_wsword-1));
      }
      break;
    case iMSword:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_sword,true)<i_msword)
        {
          game.items[itype_sword]=(1<<(i_msword-1));
        }
      }
      else
      {
        game.items[itype_sword]|=(1<<(i_msword-1));
      }
      break;
    case iXSword:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_sword,true)<i_xsword)
        {
          game.items[itype_sword]=(1<<(i_xsword-1));
        }
      }
      else
      {
        game.items[itype_sword]|=(1<<(i_xsword-1));
      }
      break;
    case iKey:          if(game.keys<255) ++game.keys; break;
    case iBCandle:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_candle,true)<i_bcandle)
        {
          game.items[itype_candle]=(1<<(i_bcandle-1));
        }
      }
      else
      {
        game.items[itype_candle]|=(1<<(i_bcandle-1));
      }
      break;
    case iRCandle:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_candle,true)<i_rcandle)
        {
          game.items[itype_candle]=(1<<(i_rcandle-1));
        }
      }
      else
      {
        game.items[itype_candle]|=(1<<(i_rcandle-1));
      }
      break;
    case iArrow:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_arrow,true)<i_warrow)
        {
          game.items[itype_arrow]=(1<<(i_warrow-1));
        }
      }
      else
      {
        game.items[itype_arrow]|=(1<<(i_warrow-1));
      }
      break;
    case iSArrow:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_arrow,true)<i_sarrow)
        {
          game.items[itype_arrow]=(1<<(i_sarrow-1));
        }
      }
      else
      {
        game.items[itype_arrow]|=(1<<(i_sarrow-1));
      }
      break;
    case iGArrow:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_arrow,true)<i_garrow)
        {
          game.items[itype_arrow]=(1<<(i_garrow-1));
        }
      }
      else
      {
        game.items[itype_arrow]|=(1<<(i_garrow-1));
      }
      break;
    case iBRing:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_ring,true)<i_bring)
        {
          game.items[itype_ring]=(1<<(i_bring-1));
        }
      }
      else
      {
        game.items[itype_ring]|=(1<<(i_bring-1));
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
          game.items[itype_ring]=(1<<(i_rring-1));
        }
      }
      else
      {
        game.items[itype_ring]|=(1<<(i_rring-1));
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
          game.items[itype_ring]=(1<<(i_gring-1));
        }
      }
      else
      {
        game.items[itype_ring]|=(1<<(i_gring-1));
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
          game.items[itype_brang]=(1<<(i_wbrang-1));
        }
      }
      else
      {
        game.items[itype_brang]|=(1<<(i_wbrang-1));
      }
      break;
    case iMBrang:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_brang,true)<i_mbrang)
        {
          game.items[itype_brang]=(1<<(i_mbrang-1));
        }
      }
      else
      {
        game.items[itype_brang]|=(1<<(i_mbrang-1));
      }
      break;
    case iFBrang:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_brang,true)<i_fbrang)
        {
          game.items[itype_brang]=(1<<(i_fbrang-1));
        }
      }
      else
      {
        game.items[itype_brang]|=(1<<(i_fbrang-1));
      }
      break;
    case iBPotion:      game.items[itype_potion]=min(current_item(itype_potion,true)+1,2); break;
    case iRPotion:      game.items[itype_potion]=2; break;
    case iBracelet:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_bracelet,true)<i_bracelet1)
        {
          game.items[itype_bracelet]=(1<<(i_bracelet1-1));
        }
      }
      else
      {
        game.items[itype_bracelet]|=(1<<(i_bracelet1-1));
      }
      break;
    case iRaft:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_raft,true)<i_raft)
        {
          game.items[itype_raft]=(1<<(i_raft-1));
        }
      }
      else
      {
        game.items[itype_raft]|=(1<<(i_raft-1));
      }
      break;
    case iLadder:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_ladder,true)<i_ladder)
        {
          game.items[itype_ladder]=(1<<(i_ladder-1));
        }
      }
      else
      {
        game.items[itype_ladder]|=(1<<(i_ladder-1));
      }
      break;
    case iBow:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_bow,true)<i_shortbow)
        {
          game.items[itype_bow]=(1<<(i_shortbow-1));
        }
      }
      else
      {
        game.items[itype_bow]|=(1<<(i_shortbow-1));
      }
      break;
    case iBow2:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_bow,true)<i_longbow)
        {
          game.items[itype_bow]=(1<<(i_longbow-1));
        }
      }
      else
      {
        game.items[itype_bow]|=(1<<(i_longbow-1));
      }
      break;
    case iBook:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_book,true)<i_book)
        {
          game.items[itype_book]=(1<<(i_book-1));
        }
      }
      else
      {
        game.items[itype_book]|=(1<<(i_book-1));
      }
      break;
    case iShield:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_shield,true)<i_largeshield)
        {
          game.items[itype_shield]=(1<<(i_largeshield-1));
        }
      }
      else
      {
        game.items[itype_shield]|=(1<<(i_largeshield-1));
      }
      break;
    case iMShield:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_shield,true)<i_mirrorshield)
        {
          game.items[itype_shield]=(1<<(i_mirrorshield-1));
        }
      }
      else
      {
        game.items[itype_shield]|=(1<<(i_mirrorshield-1));
      }
      break;
    case iMKey:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_magickey,true)<i_magickey)
        {
          game.items[itype_magickey]=(1<<(i_magickey-1));
        }
      }
      else
      {
        game.items[itype_magickey]|=(1<<(i_magickey-1));
      }
      break;
    case iMap:          game.lvlitems[dlevel]|=liMAP; break;
    case iCompass:      game.lvlitems[dlevel]|=liCOMPASS; break;
    case iBossKey:      game.lvlitems[dlevel]|=liBOSSKEY; break;
    case iLetter:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_letter,true)<i_letter)
        {
          game.items[itype_letter]=(1<<(i_letter-1));
        }
      }
      else
      {
        game.items[itype_letter]|=(1<<(i_letter-1));
      }
      break;
    case iBait:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_bait,true)<i_bait)
        {
          game.items[itype_bait]=(1<<(i_bait-1));
        }
      }
      else
      {
        game.items[itype_bait]|=(1<<(i_bait-1));
      }
      break;
    case iWand:         game.items[itype_wand]|=i_wand; break;
    case iWhistle:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_whistle,true)<i_recorder)
        {
          game.items[itype_whistle]=(1<<(i_recorder-1));
        }
      }
      else
      {
        game.items[itype_whistle]|=(1<<(i_recorder-1));
      }
      break;
    case iFairyStill:
    case iFairyMoving:  game.life=min(game.life+(3*HP_PER_HEART),game.maxlife); break;
    //  case iCross:      game.misc2|=iCROSS; break;
    case iAmulet:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_amulet,true)<i_amulet1)
        {
          game.items[itype_amulet]=(1<<(i_amulet1-1));
        }
      }
      else
      {
        game.items[itype_amulet]|=(1<<(i_amulet1-1));
      }
      break;
    case iL2Amulet:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_amulet,true)<i_amulet2)
        {
          game.items[itype_amulet]=(1<<(i_amulet2-1));
        }
      }
      else
      {
        game.items[itype_amulet]|=(1<<(i_amulet2-1));
      }
      break;
    case iFlippers:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_flippers,true)<i_flippers)
        {
          game.items[itype_flippers]=(1<<(i_flippers-1));
        }
      }
      else
      {
        game.items[itype_flippers]|=(1<<(i_flippers-1));
      }
      break;
    case iBoots:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_boots,true)<i_boots)
        {
          game.items[itype_boots]=(1<<(i_boots-1));
        }
      }
      else
      {
        game.items[itype_boots]|=(1<<(i_boots-1));
      }
      break;
    case iL2Bracelet:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_bracelet,true)<i_bracelet2)
        {
          game.items[itype_bracelet]=(1<<(i_bracelet2-1));
        }
      }
      else

      {
        game.items[itype_bracelet]|=(1<<(i_bracelet2-1));
      }
      break;
    case iHookshot:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_hookshot,true)<i_hookshot)
        {
          game.items[itype_hookshot]=(1<<(i_hookshot-1));
        }
      }
      else
      {
        game.items[itype_hookshot]|=(1<<(i_hookshot-1));
      }
      break;
    case iLens:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_lens,true)<i_lens)
        {
          game.items[itype_lens]=(1<<(i_lens-1));
        }
      }
      else
      {
        game.items[itype_lens]|=(1<<(i_lens-1));
      }
      break;
    case iHammer:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_hammer,true)<i_hammer)
        {
          game.items[itype_hammer]=(1<<(i_hammer-1));
        }
      }
      else
      {
        game.items[itype_hammer]|=(1<<(i_hammer-1));
      }
      break;

    case iMagicC:
      if(game.maxmagic < MAGICPERBLOCK*8)
        game.maxmagic+=MAGICPERBLOCK;

    case iSMagic:       game.dmagic+=MAGICPERBLOCK; break;
    case iLMagic:       game.dmagic=MAGICPERBLOCK*8; break;

    case iHCPiece:
      if(++game.HCpieces<4)
        break;
      game.HCpieces = 0;
      // fall through
    case iHeartC:
      if(game.maxlife < (get_bit(quest_rules,qr_24HC) ? 24*HP_PER_HEART : 16*HP_PER_HEART))
        game.maxlife+=HP_PER_HEART;
      // fall through
    case iHeart:        game.life=min(game.life+HP_PER_HEART,game.maxlife); break;
    case iKillAll:      kill_em_all(); break;
    case iDinsFire:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_dinsfire,true)<i_dinsfire)
        {
          game.items[itype_dinsfire]=(1<<(i_dinsfire-1));
        }
      }
      else
      {
        game.items[itype_dinsfire]|=(1<<(i_dinsfire-1));
      }
      break;
    case iFaroresWind:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_faroreswind,true)<i_faroreswind)
        {
          game.items[itype_faroreswind]=(1<<(i_faroreswind-1));
        }
      }
      else
      {
        game.items[itype_faroreswind]|=(1<<(i_faroreswind-1));
      }
      break;
    case iNayrusLove:
      if (!get_bit(quest_rules,qr_KEEPOLDITEMS))
      {
        if (current_item(itype_nayruslove,true)<i_nayruslove)
        {
          game.items[itype_nayruslove]=(1<<(i_nayruslove-1));
        }
      }
      else
      {
        game.items[itype_nayruslove]|=(1<<(i_nayruslove-1));
      }
      break;
  }

  selectBwpn(0,0);
  selectAwpn(0);

  switch(id)
  {
    case iRupy:
    case i5Rupies:
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
  int index=items.hit(x,y+8,1,1);
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
      if(game.rupies<abs(prices[index-1][0]))
        return;
      game.drupy -= abs(prices[index-1][0]);
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
        action=swimhold1;
      }
      else
      {
        action=holding1;
      }

      if(((item*)items.spr(index))->twohand)
      {
        if(action==swimhold1)
        {
          action=swimhold2;
        }
        else
        {
          action=holding2;
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

void LinkClass::StartRefill(int refill_why)
{
  if(!refilling)
  {
    refillclk=21;
    stop_sfx(WAV_ER);
    sfx(WAV_REFILL,128,true);
    refilling=true;
    if ((refill_why==REFILL_POTION)&&(!get_bit(quest_rules,qr_NONBUBBLEMEDICINE)))
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
  if(refillclk%speed == 0)
  {
    //   game.life&=0xFFC;
    switch (refill_what)
    {
      case REFILL_LIFE:
        game.life=min(game.maxlife, (game.life+HP_PER_HEART/2));
        if(game.life>=game.maxlife)
        {
          game.life=game.maxlife;
          kill_sfx();
          sfx(WAV_MSG);
          refilling=false;
          return false;
        } break;
      case REFILL_MAGIC:
        game.magic=min(game.maxmagic, (game.magic+MAGICPERBLOCK/4));
        if(game.magic>=game.maxmagic)
        {
          game.magic=game.maxmagic;
          kill_sfx();
          sfx(WAV_MSG);
          refilling=false;
          return false;
        } break;
      case REFILL_ALL:
        game.life=min(game.maxlife, (game.life+HP_PER_HEART/2));
//        game.magic=min(game.maxmagic, (game.magic+MAGICPERBLOCK/4));
        game.magic=min(game.maxmagic, (game.magic+MAGICPERBLOCK/2));
        if((game.life>=game.maxlife)&&(game.magic>=game.maxmagic))
        {
          game.life=game.maxlife;
          game.magic=game.maxmagic;
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

  sfx(WAV_SCALE);
  jukebox(MUSIC_TRIFORCE);
  if (id==iTriforce)
  {
    game.lvlitems[dlevel]|=liTRIFORCE;
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
      action=holding2;                                      // have to reset this flag
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
        StartRefill(REFILL_TRIFORCE);
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
      draw_screen(tmpscr, 0, 0);
    }

    draw_screen(tmpscr, 0, 0);
    putsubscr(framebuf,0,0);

    advanceframe();
    ++f;
  } while((f<408)||(midi_pos > 0));

  action=none;
  draw_screen_clip_rect_x1=0;
  draw_screen_clip_rect_x2=255;
  draw_screen_clip_rect_y1=0;
  draw_screen_clip_rect_y2=223;
  draw_screen_clip_rect_show_link=true;
  dowarp(1); //side warp
}

void setup_red_screen()
{
  clear_bitmap(framebuf);
  rectfill(scrollbuf, 0, 0, 255, 167, 0);
  putscr(scrollbuf, 0, 0, tmpscr);
  blit(scrollbuf, framebuf, 0, 0, 0, 56, 256, 168);
  do_layer(framebuf, 0, tmpscr, 0, 0, 2);
  do_layer(framebuf, 1, tmpscr, 0, 0, 2);
  do_layer(framebuf, -2, tmpscr, 0, 0, 2);

  if(!(msgdisplaybuf->clip))
  {
    masked_blit(msgdisplaybuf, framebuf,0,0,0,56, 256,168);
  }
  if(!(pricesdisplaybuf->clip))
  {
    masked_blit(pricesdisplaybuf, framebuf,0,0,0,56, 256,168);
  }
  //red shift
  // color scale the game screen
  for(int y=0; y<168; y++)
  {
    for(int x=0; x<256; x++)
    {
      int c = framebuf->line[y+56][x];
      int r = min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
      framebuf->line[y+56][x] = (c ? (r+CSET(2)) : 0);
    }
  }

//  Link->draw(framebuf);
  blit(framebuf,scrollbuf, 0, 56, 256, 56, 256, 168);

  clear_bitmap(framebuf);

  if (!(tmpscr->layermap[2]==0
        && tmpscr->layermap[3]==0
        && tmpscr->layermap[4]==0
        && tmpscr->layermap[5]==0
        && !overheadcombos(tmpscr)))
  {
    do_layer(framebuf, 2, tmpscr, 0, 0, 2);
    do_layer(framebuf, 3, tmpscr, 0, 0, 2);
    do_layer(framebuf, -1, tmpscr, 0, 0, 2);
    do_layer(framebuf, 4, tmpscr, 0, 0, 2);
    do_layer(framebuf, 5, tmpscr, 0, 0, 2);

    //do an AND masked blit for messages on top of layers
    if(!(msgdisplaybuf->clip) || !(pricesdisplaybuf->clip))
    {
      for(int y=0; y<168; y++)
      {
        for(int x=0; x<256; x++)
        {
          int c1 = framebuf->line[y+56][x];
          int c2 = msgdisplaybuf->line[y][x];
          int c3 = pricesdisplaybuf->line[y][x];

          if (c1 && c3)
          {
            framebuf->line[y+56][x] = c3;
          }
          else if (c1 && c2)
          {
            framebuf->line[y+56][x] = c2;
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
        int c = framebuf->line[y+56][x];
        int r = min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
        framebuf->line[y+56][x] = r+CSET(2);
      }
    }
  }

  blit(framebuf,scrollbuf, 0, 56, 0, 56, 256, 168);

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
  game.deaths=min(game.deaths+1,999);
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
  //code in subscr.cpp, putsubscr checks the following value.
  //color 255 is a GUI color, so quest makers shouldn't be using this value.
  //Also, subscreen is static after death in Z1.
  int tmp_link_dot = QMisc.colors.link_dot;
  QMisc.colors.link_dot = 255;
  putsubscr(scrollbuf, 256, 0);//save this and reuse it.
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
            blit(scrollbuf,framebuf,256,0,0,0,256,56);
          }
          if(f==60)
            setup_red_screen();

          if(f>=60 && f<=169)
          {
            blit(scrollbuf,framebuf,256,0,0,0,256,224);
            draw(framebuf);
            masked_blit(scrollbuf,framebuf,0,56,0,56,256,168);
          }
          if(f>=139 && f<=169)//fade from red to black
          {
            fade_interpolate(RAMpal,black_palette,RAMpal, (f-138)<<1, CSET(2), CSET(4)-1);
            refreshpal=true;
          }
        }
        else //f>=170
        {
          if(f==170)//make Link grayish
          {
            fade_interpolate(RAMpal,black_palette,RAMpal,64, CSET(2), CSET(4)-1);
            for(int i=CSET(6); i < CSET(7); i++)
            {
              int g = (RAMpal[i].r + RAMpal[i].g + RAMpal[i].b)/3;
              RAMpal[i] = _RGB(g,g,g);
            }
            refreshpal = true;
          }

          //draw only link. otherwise black layers might cover him.
          rectfill(framebuf,0,56,255,223,0);
          draw(framebuf);
          blit(scrollbuf,framebuf,256,0,0,0,256,56);
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
          blit(scrollbuf,framebuf,256,0,0,0,256,56);
        }
        else
        { //draw only link. otherwise black layers might cover him.
          rectfill(framebuf,0,56,255,223,0);
          draw(framebuf);
          blit(scrollbuf,framebuf,256,0,0,0,256,56);
        }
      }
    }
    else if(f<350)//draw 'GAME OVER' text
    {
      clear_bitmap(framebuf);
      blit(scrollbuf,framebuf,256,0,0,0,256,56);
      textout_ex(framebuf,zfont,"GAME OVER",96,136,1,-1);
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
  if(game.lvlitems[dlevel]&liBOSS)
  {
    return;
  }

  dir=down;
  action=holding2;
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
/*
    if(rSbtn())
    {
      conveyclk=3;
      dosubscr();
      //      guys.draw(framebuf,false);
    }
*/
    if(rSbtn())
    {
      conveyclk=3;
      int tmp_subscr_clk = frame;
      dosubscr();
      newscr_clk += frame - tmp_subscr_clk;
    }

  }

  action=none;
  dir=up;
  if(!getmapflag() && (tunes[MAXMUSIC-1].midi))
    jukebox(MAXMUSIC-1);
  else
    play_DmapMusic();
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
          ctype=(combobuf[MAPDATA(x+4,y+12)].type);
          if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
          {
            set_bit(&conveyor_flags,ctype-cCVUP,1);
          }
          break;
        case 1:
          ctype=(combobuf[MAPDATA(x+11,y+12)].type);
          if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
          {
            set_bit(&conveyor_flags,ctype-cCVUP,1);
          }
          break;
        case 2:
          ctype=(combobuf[MAPDATA(x+4,y+15)].type);
          if((ctype>=cCVUP) && (ctype<=cCVRIGHT))
          {
            set_bit(&conveyor_flags,ctype-cCVUP,1);
          }
          break;
        case 3:
          ctype=(combobuf[MAPDATA(x+11,y+15)].type);
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
          if(get_bit(&conveyor_flags,i)&&!walkflag(x,y+8-2,2,up))
          {
            y=y-2;
            hs_starty-=2;
            for (int i=0; i<chainlinks.Count(); i++)
            {
              chainlinks.spr(i)->y-=2;
            }
            if (Lwpns.idFirst(wHookshot)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHookshot))->y-=2;
            }
            if (Lwpns.idFirst(wHSHandle)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHSHandle))->y-=2;
            }
          }
          break;
        case 1:
          if(get_bit(&conveyor_flags,i)&&!walkflag(x,y+15+2,2,down))
          {
            y=y+2;
            hs_starty+=2;
            for (int i=0; i<chainlinks.Count(); i++)
            {
              chainlinks.spr(i)->y+=2;
            }
            if (Lwpns.idFirst(wHookshot)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=2;
            }
            if (Lwpns.idFirst(wHSHandle)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=2;
            }
          }
          break;
        case 2:
          if(get_bit(&conveyor_flags,i)&&!walkflag(x-(lsteps[int(x)&7]),y+8,1,left))
          {
            x=x-2;
            hs_startx-=2;
            for (int i=0; i<chainlinks.Count(); i++)
            {
              chainlinks.spr(i)->x-=2;
            }
            if (Lwpns.idFirst(wHookshot)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHookshot))->x-=2;
            }
            if (Lwpns.idFirst(wHSHandle)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHSHandle))->x-=2;
            }
          }
          break;
        case 3:
          if(get_bit(&conveyor_flags,i)&&!walkflag(x+15+2,y+8,1,right))
          {
            x=x+2;
            hs_startx+=2;
            for (int i=0; i<chainlinks.Count(); i++)
            {
              chainlinks.spr(i)->x+=2;
            }
            if (Lwpns.idFirst(wHookshot)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=2;
            }
            if (Lwpns.idFirst(wHSHandle)>-1)
            {
              Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=2;
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
