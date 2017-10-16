/*
  link.cc
  Jeremy Craner, 1999
  Link's code for zelda.cc
*/

extern movingblock mblock[4];
extern sprite_list items,Lwpns,Ewpns,guys;

const byte lsteps[8] = {1,1,2,1,1,2,1,1};

enum { did_fairy=1, did_candle=2, did_whistle=4 };


class LinkClass : public sprite {
  bool Adown,Bdown,autostep,superman,refilling,inlikelike,inwallm;
  int attackclk,ladderx,laddery,pushing,fairyclk,refillclk,hclk;
  int warpx,warpy,holdclk,holditem,attack,swordclk,didstuff;
  byte skipstep,lstep,whirlwind;
  actiontype action;
  byte hitdir,ladderdir,lastdir[3];

  void movelink();
  void move(int d);
  void hitlink(int hit);
  void checkhit();
  void checkscroll();
  void checkspecial();
  void checkpushblock();
  void checktouchblk();
  void checklocked();
  void checkitems();
  bool startwpn(int wpn);
  bool doattack();
  bool can_attack();
  bool walkflag(int wx,int wy,int cnt,byte d);
  bool checkmaze(mapscr *scr);
  void scrollscr(int dir);
  void scrollscr(int dir,int destscr);
  void walkdown();
  void walkup();
  bool dowarp(int type);
  void exitcave();
  void stepout();
  void masked_draw(BITMAP *dest);
  void getTriforce();
  void checkstab();
  void fairycircle();
  void StartRefill();
  int  EwpnHit();
  void gameover();
  void ganon_intro();
  void saved_Zelda();

public:

  LinkClass() : sprite() { init(); }
  void init();
  virtual void draw(BITMAP* dest);
  virtual bool animate(int index);

  void linkstep() { lstep = lstep<11 ? lstep+1 : 0; }
  void stepforward(int steps);
  void draw_under(BITMAP* dest);

  // called by ALLOFF()
  void resetflags(bool all)
  {
    refilling=inlikelike=inwallm=false;
    whirlwind=hclk=fairyclk=didstuff=0;
    if(swordclk>0 || all)
      swordclk=0;
    action=none;
  }

  void Freeze() { action=freeze; }
  void unfreeze() { if(action==freeze) action=none; }
  void beatlikelike() { inlikelike=false; }
  fix  getX()   { return x; }
  fix  getY()   { return y; }
  int  getDir() { return dir; }
  bool refill();
  void Catch()
  {
    if(!inwallm && (action==none || action==walking))
    {
      action=attacking;
      attackclk=0;
      attack=wCatching;
    }
  }
  bool getClock() { return superman; }
  void setClock(bool state) { superman=state; }
};


void LinkClass::init()
{
  dir = up;
  x=tmpscr[0].warpx2;
  y=tmpscr[0].warpy2;
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
  whirlwind=0;
}



void LinkClass::draw_under(BITMAP* dest)
{
  if(action==rafting)
    overtile16(dest,71,x,y+60,6,0);

  if(ladderx+laddery)
    overtile16(dest,76,ladderx,laddery+56,6,0);
}


void LinkClass::draw(BITMAP* dest)
{
  yofs=(dungeon() && currscr<128) ? 54 : 56;
  cs = ((hclk>>1)&3) + 6;
  if(superman)
    cs = (((~frame)>>1)&3)+6;

  if(attackclk || action==attacking) {
    if(attackclk>4) {
      if(attack==wSword || attack==wWand) {
        int wy=1;
        int wx=1;
        int t=79;
        int f=0;
        int cs=game.sword-1+6;

        switch(dir) {
        case up:
          t=41; wx=-1; wy=-12;
          if(attackclk==13) wy+=4;
          if(attackclk==14) wy+=8;
          break;
        case down:
          t=41; f=2; wy=11;
          if(attackclk==13) wy-=4;
          if(attackclk==14) wy-=8;
          break;
        case left:
          f=1; wx=-11;
          if(attackclk==13) wx+=4;
          if(attackclk==14) wx+=8;
          break;
        case right:
          wx=11;
          if(attackclk==13) wx-=4;
          if(attackclk==14) wx-=8;
          break;
        }

        if(dungeon())
          if(currscr<128)
            wy+=2;
        if(attack==wWand) {
          cs=7;
          if(t==41)  t=60;
          if(t==79)  t=81;
          }
        if(dungeon() && currscr<128) {
          BITMAP *sub=create_sub_bitmap(dest,16,72,224,144);
          overtile16(sub,t,x+wx-16,y+wy-18,cs,f);
          destroy_bitmap(sub);
          }
        else
          overtile16(dest,t,x+wx,y+yofs+wy,cs,f);
        }
      }
    if(attackclk<7 || ((attack==wSword || attack==wWand) && attackclk<13)) {
      flip=0;
      switch(dir) {
        case up:    tile=27; break;
        case down:  tile=26; break;
        case left:  tile=25; flip=1; break;
        case right: tile=25; break;
        }
      masked_draw(dest);
      return;
      }
    }

  tile=20;
  flip=0;
  switch(dir) {
  case up:    tile=24; if(lstep>=6) flip++; break;
  case down:  tile=22; if(lstep>=6) tile++; break;
  case left:  flip++; if(lstep>=6) tile++; break;
  case right: if(lstep>=6) tile++;
  }
  if( (dir!=up) && (game.misc&iSHIELD) )
    tile+=10;

  yofs=(dungeon() && currscr<128) ? 54 : 56;

  if(action==holding1 || action==holding2) {
    yofs=54;
    tile=(action==holding1)?34:29;
    flip=0;
    putitem(dest,x-((action==holding1)?4:0),y+38,holditem);
    }
  masked_draw(dest);

  if((didstuff&did_fairy)||fairyclk==0)
    return;

  double a = fairyclk*2*PI/80 + (PI/2);
  int hearts=0;
  do {
    double tx = cos(a)*53  +125;
    double ty = -sin(a)*53 +88+56;
    overtile8(dest,0,int(tx),int(ty),8,0);
    a-=PI/4;
    hearts++;
  } while(a>PI/2 && hearts<8);
}



void LinkClass::masked_draw(BITMAP* dest)
{
 if(dungeon() && currscr<128 &&
  (x<16 || x>224 || y<18 || y>146))
  { // clip under doorways
   BITMAP *sub=create_sub_bitmap(dest,16,72,224,144);
   if(sub!=NULL)
   {
    yofs=-18;
    xofs=-16;
    sprite::draw(sub);
    xofs=0;
    destroy_bitmap(sub);
   }
  }
 else
   sprite::draw(dest);
}



// separate case for sword/wand only
// the main weapon checking is in the global function check_collisions()
void LinkClass::checkstab()
{
  if(action!=attacking || (attack!=wSword && attack!=wWand) || attackclk<=4)
    return;

  int wx,wy,wxsz,wysz;
  switch(dir) {
  case up:    wx=x+1;  wy=y-12; wxsz=14; wysz=20; break;
  case down:  wx=x+1;  wy=y+8;  wxsz=14; wysz=20; break;
  case left:  wx=x-12; wy=y+2;  wxsz=20; wysz=14; break;
  case right: wx=x+8;  wy=y+2;  wxsz=20; wysz=14; break;
  }
  for(int i=0; i<guys.Count(); i++)
  {
    if(guys.spr(i)->hit(wx,wy,wxsz,wysz) || (attack==wWand && guys.spr(i)->hit(x,y-8,16,24)))
    {
      int h = hit_enemy(i,attack,(attack==wSword) ? (1<<(game.sword-1)) : 2, wx,wy,dir);
      if(h>0 && hclk==0 && !inlikelike)
      {
        if(GuyHit(i,x+7,y+7,2,2)!=-1)
          hitlink(i);
      }
      if(h==2)
        break;
    }
  }
  for(int j=0; j<items.Count(); j++) {
    if(((item*)items.spr(j))->pickup & ipTIMER)
     if(((item*)items.spr(j))->clk2 >= 32) {
      if(items.spr(j)->hit(wx,wy,wxsz,wysz) || (attack==wWand && items.spr(j)->hit(x,y-8,16,24))) {
        getitem(items.spr(j)->id);
        items.del(j);
        j--;
        }
      }
    }
}



int LinkClass::EwpnHit()
{
  for(int i=0; i<Ewpns.Count(); i++)
    if(Ewpns.spr(i)->hit(x+7,y+7,2,2)) {
      weapon *ew = (weapon*)(Ewpns.spr(i));
      bool hitshield=false;
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
      if(!hitshield || action==attacking)
        return i;
      switch(ew->id) {
      case ewFireball:
      case ewMagic:
      case ewSword:
        if(!(game.misc&iSHIELD) || ew->type)
          return i;
        break;
      }
      ew->onhit(false,1);
      sfx(WAV_CHINK,pan(int(x)));
    }
  return -1;
}



void LinkClass::checkhit()
{
 if(hclk>0)
   hclk--;

 if(hclk<39 && action==gothit)
   action=none;
 if(hclk>=40 && action==gothit) {
   for(int i=0; i<4; i++) {
     switch(hitdir) {
     case up:    if(hit_walkflag(x,y+7,2))    action=none; else y--; break;
     case down:  if(hit_walkflag(x,y+16,2))   action=none; else y++; break;
     case left:  if(hit_walkflag(x-1,y+8,1))  action=none; else x--; break;
     case right: if(hit_walkflag(x+16,y+8,1)) action=none; else x++; break;
     }
   }
 }

 if(hclk>0 || inlikelike || action==inwind || inwallm)
   return;

 for(int i=0; i<Lwpns.Count(); i++) {
   sprite *s = Lwpns.spr(i);
   if(s->id==wFire && (superman ? s->hit(x+7,y+7,2,2) : s->hit(this)) ) {
     game.life = max(game.life-(4>>game.ring),0);
     hitdir = s->hitdir(x,y,16,16,dir);
     if(action!=rafting && action!=freeze)
       action=gothit;
     hclk=48;
     sfx(WAV_OUCH,pan(int(x)));
     return;
     }
   if(hclk==0 && s->id==wWind && s->hit(x+7,y+7,2,2)) {
     xofs=1000;
     action=inwind;
     attackclk=0;
     return;
     }
   }

 if(action==rafting || action==freeze || superman)
   return;

 int hit = GuyHit(x+7,y+7,2,2);
 if(hit!=-1)
 {
   hitlink(hit);
   return;
 }

 hit = EwpnHit();
 if(hit!=-1) {
   game.life = max(game.life-(ewpn_dp(hit)>>game.ring),0);
   hitdir = Ewpns.spr(hit)->hitdir(x,y,16,16,dir);
   ((weapon*)Ewpns.spr(hit))->onhit(false);
   action=gothit;
   hclk=48;
   sfx(WAV_OUCH,pan(int(x)));
   return;
   }

}


void LinkClass::hitlink(int hit)
{
   if(superman)
     return;

   game.life = max(game.life-(enemy_dp(hit)>>game.ring),0);
   hitdir = guys.spr(hit)->hitdir(x,y,16,16,dir);
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
   }
}



// returns true when game over
bool LinkClass::animate(int index)
{
  // make the flames from the wand
  if(wand_dead) {
    wand_dead=false;
    if(game.misc&iBOOK && Lwpns.idCount(wFire)<2) {
      Lwpns.add(new weapon(wand_x,wand_y,wFire,1,1,0));
      sfx(WAV_FIRE,pan(wand_x));
      }
    }

  checkhit();
  if(game.life<=0) {
    gameover();
    return true;
    }

  if(swordclk>0)
    swordclk--;

  if(inwallm)
  {
    linkstep();
    if(CarryLink()==false)
      restart_level();
    return false;
  }

  // get user input or do other animation
  freeze_guys=false; // reset flag, set it if holding
  switch(action)
  {
  case gothit:
    if(attackclk)
      if(!doattack())
        attackclk=0;

  case freeze:
  case scrolling:
    break;

  case holding1:
  case holding2:
    if(--holdclk == 0)
      action=none;
    else
      freeze_guys=true;
    break;
  case inwind: {
    int i=Lwpns.idFirst(wWind);
    if(i<0) {
      if(whirlwind==255) {
        action=none;
        xofs=0;
        whirlwind=0;
        dir=right;
        lstep=0;
        }
      else
        x=241;
      }
    else {
      x=Lwpns.spr(i)->x;
      y=Lwpns.spr(i)->y;
      }
    }
    break;

  default:
    movelink(); // call the main movement routine
  }

  // check for ladder removal
  if((abs(laddery-int(y))>=16) || (abs(ladderx-int(x))>=16)) {
    ladderx=laddery=0;
    }

  // check lots of other things
  checkscroll();
  if(action!=inwind) {
    checkitems();
    checklocked();
    checkpushblock();
    checkspecial();
    if(action==won)
      return true;
    }

  if(frame - newscr_clk == 1)
  {
    if(tmpscr[0].room==rGANON)
      ganon_intro();
    else
      loadguys();
  }
  if(frame - newscr_clk == 2)
    loadenemies();
  if(frame - newscr_clk > 2 && !loaded_enemies)
    loadenemies();


  // walk through bombed doors and fake walls
  bool walk=false;
  int dtype=dBOMBED;
  if(pushing>=24) dtype=dWALK;

  if(dungeon() && action!=freeze && loaded_guys && !inlikelike) {
    if(((dtype==dBOMBED)?Up():dir==up)
      && x==120 && y<=32 && tmpscr[0].door[0]==dtype)
      { walk=true; dir=up; }

    if(((dtype==dBOMBED)?Down():dir==down)
      && x==120 && y>=128 && tmpscr[0].door[1]==dtype)
      { walk=true; dir=down; }

    if(((dtype==dBOMBED)?Left():dir==left)
      && x<=32 && y==80 && tmpscr[0].door[2]==dtype)
      { walk=true; dir=left; }

    if(((dtype==dBOMBED)?Right():dir==right)
      && x>=208 && y==80 && tmpscr[0].door[3]==dtype)
      { walk=true; dir=right; }
    }
  if(walk) {
    hclk=0;
    drawguys=false;
    if(dtype==dWALK)
      sfx(WAV_SECRET);
    stepforward(25);
    action=scrolling;
    pushing=false;
    }

  if(game.life<=8)
    cont_sfx(WAV_ER);
  else stop_sfx(WAV_ER);

  if(rSbtn())
    dosubscr();

  checkstab();
  return false;
}


bool LinkClass::startwpn(int wpn) // an item index
{
if((dir==up && y<24) || (dir==down && y>128) ||
   (dir==left && x<32) || (dir==right && x>208))
   return false;

int wx=x;
int wy=y;
switch(dir) {
 case up:    wy-=16; break;
 case down:  wy+=16; break;
 case left:  wx-=16; break;
 case right: wx+=16; break;
 }

switch(wpn) {

 case iRPotion:
 case iBPotion:
  game.potion--;
  Bwpn=0;
  StartRefill();
  while(refill()) {
    putsubscr(framebuf,0,0);
    advanceframe();
    }
  selectBwpn(0);
  return false;


 case iLetter:
  if(game.letter==1 && currscr==128 && tmpscr[1].room==rP_SHOP) {
    game.letter|=2;
    setupscreen();
    action=none;
    }
  return false;


 case iWhistle:
  sfx(WAV_WHISTLE);
  for(int i=0; i<150; i++) {
    advanceframe();
    if(Quit)
      return false;
    }
  Lwpns.add(new weapon(wx,wy,wWhistle,0,0,dir));
  if((didstuff&did_whistle) || currscr>=128)
    return false;
  didstuff|=did_whistle;
  if(tmpscr[0].flags&fWHISTLE)
    whistleclk=0; // signal to start drying lake or doing other stuff
  else if(dlevel==0 && TriforceCount()) {
    Lwpns.add(new weapon(0,y,wWind,0,0,right));
    whirlwind=dir;
    }
  return false;


 case iBombs: {
  if(Lwpns.idCount(wLitBomb))
    return false;
  if(!debug)
    game.bombs--;
  selectBwpn(8);
  Lwpns.add(new weapon(wx,wy,wBomb,0,4,dir));
  sfx(WAV_PLACE,pan(wx));
  } break;


 case iWand:
  if(Lwpns.idCount(wMagic))
    return false;
  if(Lwpns.idCount(wBeam))
    Lwpns.del(Lwpns.idFirst(wBeam));
  Lwpns.add(new weapon(wx,wy,wMagic,0,2,dir));
  sfx(WAV_WAND,pan(wx));
  break;


 case iSword:
  if(Lwpns.idCount(wBeam)||Lwpns.idCount(wMagic))
    return false;
  Lwpns.add(new weapon(wx,wy,wBeam,0,1<<(game.sword-1),dir));
  sfx(WAV_BEAM,pan(wx));
  break;


 case iBCandle: if(didstuff&did_candle) return false;
 case iRCandle:
  if(Lwpns.idCount(wFire)>=2)
    return false;
  didstuff|=did_candle;
  Lwpns.add(new weapon(wx,wy,wFire,0,1,dir));
  sfx(WAV_FIRE,pan(wx));
  attack=wFire;
  break;


 case iSArrow:
 case iArrow:
  if(Lwpns.idCount(wArrow))
    return false;
  if(game.drupy+game.rupies<=0)
    return false;
  game.drupy--;
  Lwpns.add(new weapon(wx,wy,wArrow,game.arrow,game.arrow<<1,dir));
  sfx(WAV_ARROW,pan(wx));
  break;


 case iBait:
  if(Lwpns.idCount(wBait))
    return false;
  if(tmpscr[0].room==rGRUMBLE && !getmapflag()) {
    items.add(new item(wx,wy,iBait,ipDUMMY+ipFADE,0));
    fadeclk=66;
    msgstr=0;
    putscr(scrollbuf,0,0,tmpscr);
    setmapflag();
    game.bait=0;
    selectBwpn(0);
    sfx(WAV_SECRET);
    return false;
    }
  Lwpns.add(new weapon(wx,wy,wBait,0,0,dir));
  break;


 case iBrang:
 case iMBrang:
 case iFBrang:
  if(Lwpns.idCount(wBrang))
    return false;
  Lwpns.add(new weapon(wx,wy,wBrang,game.brang,game.brang,dir));
  break;


 default:
  return false;
 }

return true;
}



bool LinkClass::doattack()
{
  if(attackclk>=14)
    return false;

  if(attack==wCatching && attackclk>4) {
    if(Up()||Down()||Left()||Right()) {
      lstep=11;
      return false;
      }
    }
  else if(attack!=wWand && attack!=wSword && attackclk>7) {
    if(Up()||Down()||Left()||Right()) {
      lstep=11;
      return false;
      }
    }
  lstep=0;
  attackclk++;
  if(attackclk==2 && attack==wBrang) {
    if(Up() && !Left() && !Right() && !Down())  dir=up;
    if(!Up() && !Left() && !Right() && Down())  dir=down;
    if(!Up() && Left() && !Right() && !Down())  dir=left;
    if(!Up() && !Left() && Right() && !Down())  dir=right;
    }
  if(attackclk==13) {
    if(attack==wSword && game.life+3>=game.maxlife)
      startwpn(iSword);
    if(attack==wWand)
      startwpn(iWand);
    }
  if(attackclk==14)
    lstep=11;
  return true;
}



bool LinkClass::can_attack()
{
 if(action==attacking) return false;
 int r=(dungeon())?16:0;
 switch(dir) {
 case up:
 case down:  return !( y<(8+r) || y>(152-r) );
 case left:
 case right: return !( x<(8+r) || x>(232-r) );
 }
 return true;
}


void LinkClass::movelink()
{
 int xoff=int(x)&7;
 int yoff=int(y)&7;
 int push=pushing;
 pushing=0;

 if(action==rafting) {
  linkstep();
  if(dir==up) {
    y--;
    return; }
  else {
    y++;
    if(MAPFLAG(x,y+14)==0)
      action=none;
    else return; }
  }

 if(rAbtn() && can_attack() && game.sword>0 && swordclk==0) {
   action=attacking;
   attack=wSword;
   attackclk=0;
   sfx(WAV_SWORD,pan(int(x)));
   }

 if(rBbtn() && can_attack()) {
   if(Bwpn==iWand) {
     action=attacking;
     attack=wWand;
     attackclk=0;
     }
   else if(startwpn(Bwpn)) {
     action=attacking;
     attackclk=0;
     attack=none;
     if(Bwpn==iBrang || Bwpn==iMBrang)
       attack=wBrang;
     }
   }

 if(attackclk || action==attacking) {
   if(!inlikelike && attackclk>4 && (attackclk&3)==0) {
     if(xoff==0) {
       if(Up()) dir=up;
       if(Down()) dir=down;
       }
     if(yoff==0) {
       if(Left()) dir=left;
       if(Right()) dir=right;
       }
     }
   if(doattack())
     return;
   action=none;
   attackclk=0;
   }

 if(action==walking) {
   if(!Up() && !Down() && !Left() && !Right() && !autostep) {
     action=none;
     return;
     }
   autostep=false;
   if(xoff||yoff) {
     if(dir==up) {
       if(int(x)&8) {
         if(!walkflag(x,y+7,2,up))
           move(up);
         else action=none; }
       else if(!walkflag(x+8,y+9,1,up))
         move(up);
       else action=none;
       }
     if(dir==down) {
       if(int(x)&8) {
         if(!walkflag(x,y+16,2,down))
           move(down);
         else action=none; }
       else if(!walkflag(x+8,y+16,1,down))
         move(down);
       else action=none;
       }
     if(dir==left) {
       if(!walkflag(x-1,y+8,1,left))
         move(left);
       else action=none;
       }
     if(dir==right) {
       if(!walkflag(x+16,y+8,1,right))
         move(right);
       else action=none;
       }
     return;
     }
   }

 action=none;
 if(dungeon() && (x<=26 || x>=214))
   goto LEFTRIGHT;

 // make it easier to get in left & right doors
 if(dungeon() && Left() && x==32 && y==80 && !walkflag(x-1,y+8,1,left))
 {
   move(left);
   return;
 }
 if(dungeon() && Right() && x==208 && y==80 && !walkflag(x+16,y+8,1,right))
 {
   move(right);
   return;
 }

 if(Up()) {
   if(xoff) {
     if(dir!=up&&dir!=down) {
       if(xoff>2&&xoff<6)
         move(dir);
       else if(xoff>=6)
         move(right);
       else if(xoff>=1)
         move(left);
       }
     }
   else {
     if((dungeon())||(int(x)&8)) {
       if(!walkflag(x,y+7,2,up)) {
         move(up);
         return;
         }
       }
     else {
       if(!walkflag(x+8,y+7,1,up)) {
         move(up);
         return;
         }
       }
     if( !Left() && !Right() ) {
       pushing=push+1;
       dir=up;
       linkstep();
       return;
       }
     else goto LEFTRIGHT;
     }
   return;
   }

 if(Down()) {
   if(xoff) {
     if(dir!=up&&dir!=down) {
       if(xoff>2&&xoff<6)
         move(dir);
       else if(xoff>=6)
         move(right);
       else if(xoff>=1)
         move(left);
       }
     }
   else {
     if((dungeon())||(int(x)&8)) {
       if(!walkflag(x,y+16,2,down)) {
         move(down);
         return;
         }
       }
     else {
       if(!walkflag(x+8,y+16,1,down)) {
         move(down);
         return;
         }
       }
     if( !Left() && !Right() ) {
       pushing=push+1;
       dir=down;
       linkstep();
       return;
       }
     else goto LEFTRIGHT;
     }
   return;
   }

LEFTRIGHT:

 if(dungeon() && (y<=26 || y>=134))
   return;

 if(Left()) {
   if(yoff) {
     if(dir!=left&&dir!=right) {
       if(yoff>2&&yoff<6)
         move(dir);
       else if(yoff>=6)
         move(down);
       else if(yoff>=1)
         move(up);
       }
     }
   else {
     if(!walkflag(x-1,y+8,1,left))
       move(left);
     else if( !Up() && !Down() ) {
       pushing=push+1;
       dir=left;
       linkstep();
       return;
       }
     }
   return;
   }

 if(Right()) {
   if(yoff) {
     if(dir!=left&&dir!=right) {
       if(yoff>2&&yoff<6)
         move(dir);
       else if(yoff>=6)
         move(down);
       else if(yoff>=1)
         move(up);
       }
     }
   else {
     if(!walkflag(x+16,y+8,1,right))
       move(right);
     else if( !Up() && !Down() ) {
       pushing=push+1;
       dir=right;
       linkstep();
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
 if(MAPCOMBO(x+7,y+8)==4) {
   if(d<left) { if(ystep==2) { skipstep^=1; ystep=skipstep; } }
   else         if(xstep==2) { skipstep^=1; xstep=skipstep; }
   }
 switch(d) {
   case up:    dy-=ystep; break;
   case down:  dy+=ystep; break;
   case left:  dx-=xstep; break;
   case right: dx+=xstep; break;
   }
 dir=d;
 linkstep();
 action=walking;
 sprite::move(dx,dy);
}




bool LinkClass::walkflag(int wx,int wy,int cnt,byte d)
{
 if(toogam) return false;
 if(blockpath && wy<88)
   return true;
 for(int i=0; i<4; i++)
   if(mblock[i].clk && mblock[i].hit(wx,wy,d<=down?16:1,1))
     return true;
 if(dungeon() && currscr<128 && wy<40 && (x!=120 || _walkflag(120,24,2)) )
   return true;

 bool wf = _walkflag(wx,wy,cnt);
 if(ladderx+laddery)
 {
  if((d&2)==ladderdir) // same direction
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
 else if(wf) {
  if(iswater(MAPCOMBO(wx,wy)) || (d<=down && iswater(MAPCOMBO(x+8,wy))) ) {
    if(game.misc&iLADDER && (dungeon() || tmpscr[0].flags&fLADDER)) {
      if((int(y)+15)<wy)
        ladderdir=up;
      else if((int(y)+8)>wy)
        ladderdir=up;
      else ladderdir=left;
      if(ladderdir==up) {
        ladderx=int(x)&0xF8;
        laddery=wy&0xF0; }
      else {
        ladderx=wx&0xF0;
        laddery=int(y)&0xF8; }
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
 if(y<16) return;

 int bx = int(x)&0xF0;
 int by = (int(y)&0xF0);
 switch(dir) {
  case up: break;
  case down:  by+=16; break;
  case left:  bx-=16; if(int(y)&8) by+=16; break;
  case right: bx+=16; if(int(y)&8) by+=16; break;
  }

 int f = MAPFLAG(bx,by);
 int t = combobuf[MAPCOMBO(bx,by)].type;

 if( (t==cPUSH_WAIT || t==cPUSH_HW) && (pushing<16 || hasMainGuy()) ) return;
 if( (t==cPUSH_HEAVY || t==cPUSH_HW) && !(game.misc&iBRACELET) ) return;

 bool doit=false;

 if((f==mfPUSH2 && dir<=down) || f==mfPUSH4)
   doit=true;

 if(get_bit(QHeader.rules,qrSOLIDBLK)) {
   switch(dir) {
   case up:    if(_walkflag(bx,by-8,2))    doit=false; break;
   case down:  if(_walkflag(bx,by+24,2))   doit=false; break;
   case left:  if(_walkflag(bx-16,by+8,2)) doit=false; break;
   case right: if(_walkflag(bx+16,by+8,2)) doit=false; break;
   }
 }

 if(doit) {
   for(int i=0; i<4; i++)
     if(mblock[i].clk<=0) {
       mblock[i].push(bx,by,dir);
       break;
       }
  }
}




bool usekey()
{
 if(!(game.misc&iMKEY)) {
  if(game.keys==0)
    return false;
  game.keys--;
  }
 return true;
}


void LinkClass::checklocked()
{
 if(toogam) return;
 if(!dungeon()) return;
 if(pushing!=8) return;
 if(tmpscr[0].door[dir]!=dLOCKED) return;
 switch(dir) {
 case up:
  if(y!=32 || x!=120) return;
  if(usekey()) {
    putdoor(0,up,dUNLOCKED);
    tmpscr[0].door[0]=dUNLOCKED;
    game.maps[(currmap<<7)+currscr]    |= 1;
    game.maps[(currmap<<7)+currscr-16] |= 2;
    }
  else return;
  break;
 case down:
  if(y!=128 || x!=120) return;
  if(usekey()) {
    putdoor(0,down,dUNLOCKED);
    tmpscr[0].door[1]=dUNLOCKED;
    game.maps[(currmap<<7)+currscr]    |= 2;
    game.maps[(currmap<<7)+currscr+16] |= 1;
    }
  else return;
  break;
 case left:
  if(y!=80 || x!=32) return;
  if(usekey()) {
    putdoor(0,left,dUNLOCKED);
    tmpscr[0].door[2]=dUNLOCKED;
    game.maps[(currmap<<7)+currscr]   |= 4;
    game.maps[(currmap<<7)+currscr-1] |= 8;
    }
  else return;
  break;
 case right:
  if(y!=80 || x!=208) return;
  if(usekey()) {
    putdoor(0,right,dUNLOCKED);
    tmpscr[0].door[3]=dUNLOCKED;
    game.maps[(currmap<<7)+currscr]   |= 8;
    game.maps[(currmap<<7)+currscr+1] |= 4;
    }
  else return;
  }
 sfx(WAV_DOOR);
 markBmap(-1);
}



void LinkClass::fairycircle()
{
 if(didstuff&did_fairy)
   return;

 if(fairyclk==0) {
   StartRefill();
   action=freeze;
   holdclk=0;
   }

 fairyclk++;

 if(!refill() && ++holdclk>80) {
   action=none;
   didstuff|=did_fairy;
   }
}


int touchcombo(int x,int y)
{
 switch(combobuf[MAPCOMBO(x,y)].type) {
 case cGRAVE:
 case cARMOS:
  return combobuf[MAPCOMBO(x,y)].type;
 }
 return 0;
}


void LinkClass::checktouchblk()
{
 if(toogam) return;

 if(!pushing)
   return;

 int tx,ty=-1;
 switch(dir) {
 case up:
   if(touchcombo(x,y+7)) {
     tx=x; ty=y+7; }
   else if(touchcombo(x+8,y+7)) {
     tx=x+8; ty=y+7; }
   break;
 case down:
   if(touchcombo(x,y+16)) {
     tx=x; ty=y+16; }
   else if(touchcombo(x+8,y+16)) {
     tx=x+8; ty=y+16; }
   break;
 case left:
   if(touchcombo(x-1,y+15)) {
     tx=x-1; ty=y+15; }
   break;
 case right:
   if(touchcombo(x+16,y+15)) {
     tx=x+16; ty=y+15; }
   break;
 }
 if(ty>=0) {
   ty&=0xF0;
   tx&=0xF0;
   int di = ty+(tx>>4);
   if(di<176 && !guygrid[di] && guys.Count()<11) {
     guygrid[di]=1;
     int id=0;
     switch(combobuf[MAPCOMBO(tx,ty)].type) {
      case cARMOS: id=eARMOS; break;
      case cGRAVE: id=eGHINI2; break;
      }
     addenemy(tx,ty+3,id,0);
     }
   }
}




void LinkClass::checkspecial()
{
checktouchblk();

bool hasmainguy = hasMainGuy(); // calculate it once

// after beating enemies
if(loaded_enemies && !hasmainguy) {

  // if room has traps, guys don't come back
  if(guys.idCount(eTRAP))
    setmapflag(mfTMPNORET);

  // item
  if(hasitem) {
    int Item=tmpscr[0].item;
    if(getmapflag())
      Item=0;
    if(Item) {
      if(hasitem==1)
        sfx(WAV_CLEARED);
      additem(tmpscr[0].itemx,tmpscr[0].itemy+1,Item,ipONETIME+ipBIGRANGE+
        ((Item==iTriforce) ? ipHOLDUP : 0));
      }
    hasitem=0;
    }
  }


// doors
bool has_shutters=false;
for(int i=0; i<4; i++)
 if(tmpscr[0].door[i]==dSHUTTER)
   has_shutters=true;
if(has_shutters)
 if(opendoors==0 && loaded_enemies) {
   // if flag is set, open by pushing block instead
   if(!(tmpscr[0].flags&fSHUTTERS) && !hasmainguy)
     opendoors=12;
   }
 else if(opendoors<0)
   opendoors++;
 else if((--opendoors)==0)
   openshutters();

// set boss flag when boss is gone
if(loaded_enemies && tmpscr[0].enemyflags&efBOSS && !hasmainguy) {
  game.lvlitems[dlevel]|=liBOSS;
  stop_sfx(WAV_ROAR);
  stop_sfx(WAV_VADER);
  }


if(toogam) return;

// check if he's standing on a warp he just came out of
if((int(y)&0xF8)==warpy)
  if(x==warpx)
    return;
warpy=255;
if(int(y)&15)
  return;
if(int(x)&7)
  return;

int flag = MAPFLAG(x,y);
int type = combobuf[MAPCOMBO(x,y)].type;

if(flag==mfFAIRY) {
  fairycircle();
  return;
  }
if(flag==mfZELDA) {
  saved_Zelda();
  return;
  }

if(int(x)&15) {	// only allow disalignment with extra wide doors
  if(type!=cCAVE || combobuf[MAPCOMBO(x+16,y)].type!=cCAVE)
    return;
  }

switch(flag) {
  case mfRAFT:
    if(game.misc&iRAFT && action!=rafting) {
      action=rafting;
      sfx(WAV_SECRET);
      }
    return;
  case 0:
    if(type==cCAVE || type==cSTAIR)
      break;
    return;
  default:
    return;
  }

putsubscr(framebuf,0,0);
blit(scrollbuf,framebuf,0,0,0,56,256,168);
draw(framebuf);
advanceframe();

if(type==cCAVE) {
  stop_midi();
  walkdown();
  }

if(dlevel==0 && currscr==129 && type==cSTAIR) {
  int dw = x<112 ? 1 : (x>136 ? 3 : 2);
  int code = WARPCODE(currdmap,homescr,dw);
  if(code!=-1)
  {
    currdmap = code>>8;
    dlevel  = DMaps[currdmap].level;
    currmap = DMaps[currdmap].map;
    homescr = (code&0xFF) + DMaps[currdmap].xoff;
    game.maps[(currmap<<7)+homescr]|=1;
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
  do {
    if(d==up || d==right)
      l=(l==7)?0:l+1;
    else
      l=(l==0)?7:l-1;
  } while(!(game.lvlitems[l+1]&liTRIFORCE));
  game.wlevel=l;
  return l;
}


bool LinkClass::dowarp(int type)
{
 byte wdmap,wscr,wtype,t;
 t=(currscr<128)?0:1;
 switch(type) {
 case 0:  // tile warp
   wtype = tmpscr[t].warptype;
   wdmap = tmpscr[t].warpdmap;
   wscr = tmpscr[t].warpscr;
   break;
 case 1:  // side warp
   wtype = tmpscr[t].warptype2;
   wdmap = tmpscr[t].warpdmap2;
   wscr = tmpscr[t].warpscr2;
   break;
 case 2: { // whistle warp
   wtype = 4;
   int level = selectWlevel(whirlwind);
   wdmap = QMisc.wind[level].dmap;
   wscr = QMisc.wind[level].scr;
   }
   break;
 }

 switch(wtype) {
 case 0: // cave/item room
  ALLOFF();
  homescr=currscr;
  currscr=0x80;
  if(dlevel==0) {  // cave
    stop_midi();
    kill_sfx();
    if(tmpscr[0].room==rWARP)
      currscr=0x81;
    loadlvlpal(10);
    blackscr(30,true);
    loadscr(0,currscr,up);
    loadscr(1,homescr,up);
    putscr(scrollbuf,0,0,tmpscr);
    dir=up;
    x=112;
    y=160;
    stepforward(6); 
    }
  else { // item room
    stop_sfx(WAV_ROAR);
    stop_sfx(WAV_VADER);
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    fade(DMaps[currdmap].color,true,false);
    blackscr(30,true);
    loadscr(0,currscr,down);
    loadscr(1,homescr,-1);
    putscr(scrollbuf,0,0,tmpscr);
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    fade(11,true,true);
    dir=down;
    x=48;
    y=0;
    stepforward(18);
    }
  break;

 case 1: { // passageway
  stop_sfx(WAV_ROAR);
  stop_sfx(WAV_VADER);
  ALLOFF();
  homescr=currscr;
  currscr=0x81;
  byte warpscr = wscr + DMaps[currdmap].xoff;
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  fade(DMaps[currdmap].color,true,false);
  blackscr(30,true);
  loadscr(0,currscr,down);
  loadscr(1,homescr,-1);
  putscr(scrollbuf,0,0,tmpscr);
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  fade(11,true,true);
  dir=down;
  x=48;
  if( (homescr&15) > (warpscr&15) )
    x=192;
  if( (homescr&15) == (warpscr&15) )
    if( (currscr>>4) > (warpscr>>4) )
      x=192;
  warpx=x;
  y=0;
  stepforward(18);
  newscr_clk=frame;
  } break;

 case 2: // entrance/exit
  ALLOFF();
  stop_midi();
  kill_sfx();
  blackscr(30,false);
  currdmap = wdmap;
  dlevel=DMaps[currdmap].level;
  currmap=DMaps[currdmap].map;
  loadlvlpal(DMaps[currdmap].color);
  lastentrance = homescr = currscr = wscr + DMaps[currdmap].xoff;
  loadscr(0,currscr,-1);
  openscreen();
  if(dlevel) {
    x=tmpscr[0].warpx2;
    y=tmpscr[0].warpy2;
    }
  else {
    x=tmpscr[0].warpx;
    y=tmpscr[0].warpy;
    }
  dir=down;
  if(x==0)   dir=right;
  if(x==240) dir=left;
  if(y==0)   dir=down;
  if(y==160) dir=up;
  if(dlevel) {
    // reset enemy kill counts
    for(int i=0; i<128; i++) {
      game.guys[(currmap<<7)+i] = 0;
      game.maps[(currmap<<7)+i] &= 0x7F; // reset mfTMPNORET flag
      }
    }
  markBmap(dir^1);
  if(dungeon())
    stepforward(12);
  else {
    if(combobuf[MAPCOMBO(x,y-16)].type==cCAVE)
      walkup();
    }
  play_DmapMidi();
  warpx=x;
  warpy=y;
  for(int i=0; i<6; i++)
    visited[i]=-1;
  break;

 case 3: { // scrolling warp
  currmap = DMaps[wdmap].map;
  scrollscr(dir,wscr+DMaps[wdmap].xoff);
  currdmap=wdmap;
  dlevel=DMaps[currdmap].level;
  loadlvlpal(DMaps[currdmap].color);
  play_DmapMidi();
  } break;

 case 4: { // whistle warp
  currmap = DMaps[wdmap].map;
  scrollscr(right,wscr+DMaps[wdmap].xoff);
  currdmap=wdmap;
  dlevel=DMaps[currdmap].level;
  loadlvlpal(DMaps[currdmap].color);
  play_DmapMidi();
  action=inwind;
  Lwpns.add(new weapon(0,tmpscr[0].warpy2,wWind,1,0,right));
  whirlwind=255;
  } break;

 default:
  return false;
 }

 newscr_clk=frame;
 Adown=Bdown=true;
 didstuff=0;
 map_bkgsfx();
 loadside=dir^1;
 whistleclk=-1;
 return true;
}



void LinkClass::exitcave()
{
 blackscr(30,true);
 loadlvlpal(DMaps[currdmap].color);
 ringcolor();
 stop_midi();
 kill_sfx();
 ALLOFF();
 currscr=homescr;
 loadscr(0,currscr,255);  // bogus direction
 putscr(scrollbuf,0,0,tmpscr);
 x=tmpscr[0].warpx;
 y=tmpscr[0].warpy;
 if(combobuf[MAPCOMBO(x,y-16)].type==cCAVE)
   walkup();
 play_DmapMidi();
 newscr_clk=frame;
 dir=down;
 warpx=x;
 warpy=y;
 Adown=Bdown=true;
 didstuff=0;
 map_bkgsfx();
 loadside=dir^1;
}


void LinkClass::stepforward(int steps)
{
 while(steps>0) {
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  draw(framebuf);
  move(dir);
  advanceframe();
  if(Quit)
    return;
  steps--;
  }
 blit(scrollbuf,framebuf,0,0,0,56,256,168);
 Adown=Bdown=true;
}



void LinkClass::walkdown()
{
hclk=0;
stop_sfx(WAV_BRANG);
sfx(WAV_STAIRS,pan(int(x)));
clk=0;
int cmby=(int(y)&0xF0)+16;
for(int i=0; i<66; i++) {
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  draw(framebuf);
  overcombo(framebuf,x,cmby-16+56,MAPDATA(x,cmby-16));
  putcombo(framebuf,x,cmby+56,MAPDATA(x,cmby));
  linkstep();
  if((i&3)==3)
    y++;
  advanceframe();
  if(Quit)
    return;
  }
}


void LinkClass::walkup()
{
hclk=0;
stop_sfx(WAV_BRANG);
sfx(WAV_STAIRS,pan(int(x)));
dir=down;
clk=0;
int cmby=int(y)&0xF0;
for(int i=0; i<64; i++) {
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  draw(framebuf);
  overcombo(framebuf,x,cmby-16+56,MAPDATA(x,cmby-16));
  putcombo(framebuf,x,cmby+56,MAPDATA(x,cmby));
  linkstep();
  if((i&3)==0)
    y--;
  advanceframe();
  if(Quit)
    return;
  }
map_bkgsfx();
loadside=dir^1;
}


void LinkClass::stepout()
{
 ALLOFF();
 putsubscr(framebuf,0,0);
 blit(scrollbuf,framebuf,0,0,0,56,256,168);
 fade(11,true,false);
 blackscr(30,true);
 ringcolor();
 if(currscr==129 && x!=warpx) {
   currdmap=tmpscr[1].warpdmap;
   currmap=DMaps[currdmap].map;
   dlevel=DMaps[currdmap].level;
   homescr=tmpscr[1].warpscr+DMaps[currdmap].xoff;
   }
 currscr=homescr;
 loadscr(0,currscr,255);  // bogus direction
 putscr(scrollbuf,0,0,tmpscr);
 putsubscr(framebuf,0,0);
 blit(scrollbuf,framebuf,0,0,0,56,256,168);
 fade(DMaps[currdmap].color,true,true);
 x=tmpscr[0].warpx;
 y=tmpscr[0].warpy;
 dir=down;
 newscr_clk=frame;
 didstuff=0;
 warpx=warpy=0;
 Adown=Bdown=true;
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
    if(dlevel)
      return (((currscr&15)-DMaps[currdmap].xoff)<=0);
    break;
  case right:
    if((currscr&15)==15)
      return true;
    if(dlevel)
      return (((currscr&15)-DMaps[currdmap].xoff)>=7);
    break;
  }
  return false;
}


void LinkClass::checkscroll()
{
 if(toogam) {
   if(x<0 && (currscr&15)==0) x=0;
   if(y<0 && currscr<16) y=0;
   if(x>240 && (currscr&15)==15) x=240;
   if(y>160 && currscr>=112) y=160;
   }
 if(y<0) {
   y=0;
   if(tmpscr[0].flags2&wfUP || edge_of_dmap(up))
     dowarp(1);
   else if(currscr<128)
     scrollscr(up);
   else
     stepout();
   }
 if(y>160) {
   y=160;
   if(currscr>=128)
     exitcave();
   else if(tmpscr[0].flags2&wfDOWN || edge_of_dmap(down))
     dowarp(1);
   else
     scrollscr(down);
   }
 if(x<0) {
   x=0;
   if(tmpscr[0].flags2&wfLEFT || edge_of_dmap(left))
     dowarp(1);
   else
     scrollscr(left);
   }
 if(x>240) {
   x=240;
   if(action==inwind)
     dowarp(2);
   else if(tmpscr[0].flags2&wfRIGHT || edge_of_dmap(right))
     dowarp(1);
   else
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
 sfx(WAV_SECRET);
 return true;
}


inline void LinkClass::scrollscr(int dir)
{
  scrollscr(dir,-1);
}


void LinkClass::scrollscr(int dir,int destscr)
{
  tmpscr[1]=tmpscr[0];
  mapscr* newscr=&tmpscr[0];
  mapscr* oldscr=&tmpscr[1];
  int sx=0, sy=0;
  int cx=0, step = dungeon() ? 2 : 4;

  for(int i=0; i<3; i++)
    lastdir[i]=lastdir[i+1];
  lastdir[3] = oldscr->flags&fMAZE ? dir : -1;

  ALLOFF();
  lstep=(lstep+6)%12;
  cx=0;
  do {
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    if(dungeon()==0)
      draw(framebuf);
    if(cx==0)
      rehydratelake();
    advanceframe();
    if(Quit)
      return;
    cx++;
    } while(cx<32);

  if(!(DMaps[currdmap].type==dmDNGN || DMaps[currdmap].type==dmBSDNGN))
    markBmap(dir);

  switch(dir) {
   case up:
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
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
      currscr+=16;
    loadscr(0,currscr,dir);
    putscr(scrollbuf,0,176,newscr);
    cx=176/step;
    break;

   case left:
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
      currscr--;
    loadscr(0,currscr,dir);
    blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
    putscr(scrollbuf,0,0,newscr);
    sx=256;
    cx=256/step;
    break;

   case right:
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
      currscr++;
    loadscr(0,currscr,dir);
    putscr(scrollbuf,256,0,newscr);
    cx=256/step;
    break;
   }

  lighting(2);
  if(!(newscr->flags&fSEA))
    adjust_sfx(WAV_SEA,128,false);
  if(!(newscr->flags&fROAR)) {
    adjust_sfx(WAV_ROAR,128,false);
    adjust_sfx(WAV_VADER,128,false);
    }

  while(cx>0) {
    switch(dir) {
      case up:    sy-=step; break;
      case down:  sy+=step; break;
      case left:  sx-=step; break;
      case right: sx+=step; break;
      }
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,sx,sy,0,56,256,168);
    switch(dir) {
      case up:    if(y<160) y+=step; break;
      case down:  if(y>0)   y-=step; break;
      case left:  if(x<240) x+=step; break;
      case right: if(x>0)   x-=step; break;
      }
    linkstep();
    if(dungeon()==0)
      draw(framebuf);
    advanceframe();
    if(Quit)
      return;
    cx--;
    }

  lighting(3);
  homescr=currscr;
  putscr(scrollbuf,0,0,newscr);
  if(action==rafting) action=none;
  if(MAPFLAG(x,y)==11) {
    action=rafting;
    sfx(WAV_SECRET);
    }
  opendoors=0;
  markBmap(-1);

  if(dungeon()) {
    switch(tmpscr[0].door[dir^1]) {
     case dOPEN:
     case dUNLOCKED:
       stepforward(12);
       break;
     case dSHUTTER:
     case d1WAYSHUTTER:
       stepforward(24);
       putdoor(0,dir^1,tmpscr[0].door[dir^1]);
       opendoors=-4;
       sfx(WAV_DOOR);
       break;
     default:
       stepforward(24);
     }
    }
  if(action==scrolling) action=none;

  map_bkgsfx();
  if(newscr->flags2&fSECRET)
    sfx(WAV_SECRET);

  newscr_clk=frame;
  loadside=dir^1;
}




/************************************/
/********  More Items Code  *********/
/************************************/


int Bweapon(int pos)
{
 switch(pos) {
 case 0: if(game.brang==3) return iFBrang;
         return game.brang?game.brang-1+iBrang:0;
 case 1: return game.bombs?iBombs:0;
 case 2: return ((game.misc&iBOW)&&game.arrow)?game.arrow-1+iArrow:0;
 case 3: return game.candle?game.candle-1+iBCandle:0;
 case 4: return game.whistle?iWhistle:0;
 case 5: return game.bait?iBait:0;
 case 6: return game.potion?game.potion-1+iBPotion:game.letter?iLetter:0;
 case 7: return game.wand?iWand:0;
 }
 return 0;
}


void selectBwpn(int step)
{
 if(step==0) {
   Bwpn=Bweapon(Bpos);
   if(Bwpn)
     return;
   step=1;
   }
 if(step==8) {
   Bwpn=Bweapon(Bpos);
   if(Bwpn)
     return;
   step=-1;
   }
 int pos=Bpos;
 do {
  Bpos+=step;
  Bpos&=7;
  Bwpn=Bweapon(Bpos);
  if(Bwpn)
    return;
  } while(Bpos!=pos);
 if(!Bwpn)
   Bpos=0;
}


bool canget(int id)
{
 if(id==iWSword && game.maxlife<5*8)
   return false;
 if(id==iMSword && game.maxlife<12*8)
   return false;
 return true;
}



void dospecialmoney(int index)
{
 int tmp=currscr>=128?1:0;
 switch(tmpscr[tmp].room) {
  case rINFO:  // pay for info
    if(game.rupies<prices[index-1])
      return;
    game.drupy+=prices[index-1];
    msgstr = QMisc.info[tmpscr[tmp].catchall].str[index-1];
    msgclk=msgpos=0;
    for(int i=0; i<80; i++)
      putcombo(scrollbuf,((i&15)<<4),(i&0xF0),tmpscr->data[i]);
    for(int i=1; i<4; i++)
      ((item*)items.spr(i))->pickup=ipDUMMY;
    break;

  case rMONEY:  // secret money
    ((item*)items.spr(0))->pickup=ipDUMMY;
    game.drupy += (prices[0]=tmpscr[tmp].catchall);
    putprices(false);
    setmapflag();
    break;

  case rGAMBLE: {  // gamble
    if(game.rupies<10) return;
    unsigned si=(rand()%24)*3;
    for(int i=0; i<3; i++)
      prices[i]=gambledat[si++];
    game.drupy+=prices[index-1];
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
    game.bombs=game.maxbombs;
    ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
    fadeclk=66;
    msgstr=0;
    putscr(scrollbuf,0,0,tmpscr);
    selectBwpn(0);
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
      game.life = max(game.life-8,0);
      game.maxlife = max(game.maxlife-8,24);
    }
    setmapflag();
    ((item*)items.spr(0))->pickup=ipDUMMY+ipFADE;
    ((item*)items.spr(1))->pickup=ipDUMMY+ipFADE;
    fadeclk=66;
    msgstr=0;
    putscr(scrollbuf,0,0,tmpscr);
    break;
  }
}


void getitem(int id)
{
 switch(id) {
  case iRupy:      game.drupy++;   break;
  case i5Rupies:   game.drupy+=5;  break;
  case i20Rupies:  game.drupy+=20; break;
  case i50Rupies:  game.drupy+=50; break;
  case iHeartC:    if(game.maxlife>=16*8) break; game.maxlife+=8;
  case iHeart:     game.life=min(game.life+8,game.maxlife); break;
  case iBombs:     game.bombs=min(game.bombs+4,game.maxbombs); break;
  case iClock:     watch=true; setClock(true); break;
  case iSword:     game.sword=max(game.sword,1); break;
  case iWSword:    game.sword=max(game.sword,2); break;
  case iMSword:    game.sword=max(game.sword,3); break;
  case iKey:       if(game.keys<255) game.keys++; break;
  case iBCandle:   game.candle=max(game.candle,1); break;
  case iRCandle:   game.candle=2; break;
  case iArrow:     game.arrow=max(game.arrow,1); break;
  case iSArrow:    game.arrow=2; break;
  case iBRing:     game.ring=max(game.ring,1); if(currscr<128 || dlevel) ringcolor(); break;
  case iRRing:     game.ring=2; if(currscr<128 || dlevel) ringcolor(); break;
  case iBrang:     game.brang=max(game.brang,1); break;
  case iMBrang:    game.brang=2; break;
  case iBPotion:   game.potion=min(game.potion+1,2); break;
  case iRPotion:   game.potion=2; break;
  case iBracelet:  game.misc|=iBRACELET; break;
  case iRaft:      game.misc|=iRAFT; break;
  case iLadder:    game.misc|=iLADDER; break;
  case iBow:       game.misc|=iBOW; break;
  case iBook:      game.misc|=iBOOK; break;
  case iShield:    game.misc|=iSHIELD; break;
  case iMKey:      game.misc|=iMKEY; break;
  case iMap:       game.lvlitems[dlevel]|=liMAP; break;
  case iCompass:   game.lvlitems[dlevel]|=liCOMPASS; break;
  case iLetter:    game.letter=1; break;
  case iBait:      game.bait=1; break;
  case iWand:      game.wand=1; break;
  case iWhistle:   game.whistle=1; break;
  case iFairy:     game.life=min(game.life+24,game.maxlife); break;
 }
 selectBwpn(0);
 switch(id) {
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
    sfx(WAV_SCALE1);
 }
}


void LinkClass::checkitems()
{
 int tmp=currscr>=128?1:0;
 int index=items.hit(x,y+8,1,1);
 if(index==-1)
   return;

 int pickup = ((item*)items.spr(index))->pickup;
 int id = ((item*)items.spr(index))->id;

 if((pickup&ipTIMER) && (((item*)items.spr(index))->clk2 < 32))
   if(items.spr(index)->id!=iFairy)
     // wait for it to stop flashing, doesn't check for other items yet
     return;

 if(pickup&ipENEMY)     // item was being carried by enemy
   hasitem=0;

 if(pickup&ipDUMMY) {   // dummy item (usually a rupy)
  if(pickup&ipMONEY)
    dospecialmoney(index);
  return;
  }

 if(pickup&ipCHECK)  	 // check restrictions
   switch(tmpscr[tmp].room) {
    case rSP_ITEM:  // special item
      if(!canget(id))
        return;
      break;

    case rP_SHOP:  // potion shop
      if(msgpos<72)
        return;
    case rSHOP:   // shop
      if(game.rupies<abs(prices[index-1]))
        return;
      game.drupy -= abs(prices[index-1]);
      break;
    }


 if(pickup&ipONETIME)  // set screen item flag for one-time-only items
  setmapflag();

 getitem(id);

 if(pickup&ipHOLDUP) {
   if(msgstr) {
     msgstr=0;
     putscr(scrollbuf,0,0,tmpscr);
     }

   fadeclk=66;

   if(id!=iBombs) {	// don't hold up bombs
     action=holding1;
     if(((item*)items.spr(index))->twohand)
       action=holding2;
     holdclk=130;
     holditem=id;
     freeze_guys=true;
     }
   if(id!=iTriforce)
     sfx(WAV_PICKUP);
   items.del(index);
   // clear up shop stuff
   if(dungeon()==0 && ((item*)items.spr(0))->pickup&ipDUMMY) {
     items.del(0);
     putscr(scrollbuf,0,0,tmpscr);
     }
   }
 else
   items.del(index);

 if(id==iTriforce)
   getTriforce();
 if(id==iBigTri)
   getBigTri();
}

void LinkClass::StartRefill()
{
 if(!refilling) {
   refillclk=21;
   stop_sfx(WAV_ER);
   sfx(WAV_REFILL,128,true);
   refilling=true;
   swordclk=0;
   }
}

bool LinkClass::refill()
{
 if(!refilling)
   return false;
 refillclk++;
 if(refillclk%22==0) {
   game.life&=0xFFC;
   game.life+=4;
   if(game.life>=game.maxlife) {
     game.life=game.maxlife;
     kill_sfx();
     sfx(WAV_MSG);
     refilling=false;
     return false;
     }
   }
 return true;
}


void LinkClass::getTriforce()
{
 sfx(WAV_SCALE1);
 jukebox(MUSIC_TRIFORCE);
 game.lvlitems[dlevel]|=liTRIFORCE;

 int f=0;
 int x=0;
 int c=0;
 do {
  if(f==40) {
    ALLOFF();
    action=holding2; // have to reset this flag
    }
  if(f>=40 && f<88) {
    if((f&7)==0) {
      for(int cs=2; cs<5; cs++)
        for(int i=1; i<16; i++)
          RAMpal[CSET(cs)+i]=_RGB(63,63,63);
      refreshpal=true;
      }
    if((f&7)==4) {
      if(currscr<128) loadlvlpal(DMaps[currdmap].color);
      else loadlvlpal(0xB);
      }
    }
  if(f==88) {
    StartRefill();
    refill();
    }
  if(f==89)
    if(refill())
      f--;
  if(f>=208 && f<288) {
    x++;
    switch(++c) {
      case 5: c=0;
      case 0:
      case 2:
      case 3: x++; break;
      }
    rectfill(scrollbuf,0,0,x-1,175,0);
    rectfill(scrollbuf,256-x,0,255,175,0);
    }

  domoney();
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  draw(framebuf);

  advanceframe();
  f++;
 } while(f<408);

 action=none;
 dowarp(1);
}


void slide_in_color(int color)
{
  for(int i=1; i<16; i+=3) {
    RAMpal[CSET(2)+i+2] = RAMpal[CSET(2)+i+1];
    RAMpal[CSET(2)+i+1] = RAMpal[CSET(2)+i];
    RAMpal[CSET(2)+i]   = NESpal(color);
    }
  refreshpal=true;
}


void LinkClass::gameover()
{
 Playing=Paused=false;
 game.deaths=min(game.deaths+1,999);
 dir=down;
 int f=0;
 stop_midi();
 kill_sfx();
 sfx(WAV_OUCH,pan(int(x)));
 do {
   if(f<32) {
     cs=(((32-f)>>1)&3)+6;
     }

   if(f==64) {
     for(int i=0; i<176; i++) {
       tmpscr[0].data[i] &= 0x01FF;
       tmpscr[0].data[i] |= 0x0400;
       }
     putscr(scrollbuf,0,0,tmpscr);
     for(int i=1; i<16; i+=3) {
       RAMpal[CSET(2)+i]   = NESpal(0x17);
       RAMpal[CSET(2)+i+1] = NESpal(0x16);
       RAMpal[CSET(2)+i+2] = NESpal(0x26);
       }
     refreshpal=true;
     sfx(WAV_SPIRAL);
     }
   if(f>64 && f<=144) {
     switch((f-64)%20) {
     case 0:  dir=down;  break;
     case 5:  dir=right; break;
     case 10: dir=up;    break;
     case 15: dir=left;  break;
     }
     linkstep();
     }
   if(f==144)
     slide_in_color(0x06);
   if(f==154)
     slide_in_color(0x07);
   if(f==164)
     slide_in_color(0x0F);
   if(f==174) {
     slide_in_color(0x0F);
     slide_in_color(0x0F);
     for(int i=1; i<16; i+=3) {
       RAMpal[CSET(6)+i]   = NESpal(0x10);
       RAMpal[CSET(6)+i+1] = NESpal(0x30);
       RAMpal[CSET(6)+i+2] = NESpal(0x00);
       }
     }

   if(f==204)
     sfx(WAV_MSG);

   putsubscr(framebuf,0,0);
   blit(scrollbuf,framebuf,0,0,0,56,256,168);

   tile=20;
   flip=0;
   switch(dir) {
   case up:    tile=24; if(lstep>=6) flip++; break;
   case down:  tile=22; if(lstep>=6) tile++; break;
   case left:  flip++; if(lstep>=6) tile++; break;
   case right: if(lstep>=6) tile++;
   }
   if( (dir!=up) && (game.misc&iSHIELD) )
     tile+=10;

   yofs=(dungeon() && currscr<128) ? 54 : 56;

   if(f>=204) {
     cs=7;
     tile=66;
     }
   if(f>=214)
     tile=67;

   if(f<=218)
     sprite::draw(framebuf);

   if(f>=266)
     textout(framebuf,zfont,"GAME OVER",96,136,1);

   advanceframe();
   f++;
 } while(f<=368 && !Quit);
}


void LinkClass::ganon_intro()
{
  loaded_guys=true;
  if(game.lvlitems[dlevel]&liBOSS)
    return;

  int level = DMaps[currdmap].color;

  dir=down;
  action=holding2;
  holditem=iTriforce;

  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);

  for(int f=0; f<288 && !Quit; f++)
  {
    switch(f)
    {
    case 48:
      stop_midi();
      stop_sfx(WAV_ROAR);
      sfx(WAV_GASP);
      sfx(WAV_GANON);
      if(game.ring)
        addenemy(160,96,eGANON,0);
      else
        addenemy(80,32,eGANON,0);
      guys.draw(framebuf,false);
      break;

    case 58: loadfadepal(level*pdLEVEL+poFADE2); break;
    case 68: loadfadepal(level*pdLEVEL+poFADE1); break;
    case 78: loadlvlpal(level); darkroom=false; break;
    }
    draw(framebuf);
    advanceframe();
    if(rSbtn())
    {
      dosubscr();
      guys.draw(framebuf,false);
    }
  }

  action=none;
  dir=up;
  play_DmapMidi();
  cont_sfx(WAV_ROAR);
}


void LinkClass::saved_Zelda()
{
  Playing=Paused=false;
  action=won;
  Quit=qWON;
  hclk=0;
  x=136;
  y=(dungeon() && currscr<128) ? 75 : 73;
  dir=left;
}


/*** end of link.cc ***/
