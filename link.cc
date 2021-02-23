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

extern movingblock mblock[4];
extern sprite_list items,Lwpns,Ewpns,guys, chainlinks;

const byte lsteps[2][8] = {{1,1,2,1,1,2,1,1},
                           {8,8,8,8,8,8,8,8}};

enum { did_fairy=1, did_candle=2, did_whistle=4 };


class LinkClass : public sprite {
  bool autostep,superman,refilling,inlikelike,inwallm;
  int attackclk,ladderx,laddery,pushing,fairyclk,refillclk,hclk;
  int warpx,warpy,holdclk,holditem,attack,swordclk,didstuff,blowcnt;
  byte skipstep,lstep,hopclk,diveclk,whirlwind;
  byte hitdir,ladderdir,lastdir[3];
  actiontype action;
  int hshandle_id, hshead_id;

  void movelink();
  void move(int d);
  void hitlink(int hit);
  int  nextcombo(int cx,int cy,int cdir);
  int  nextflag(int cx,int cy,int cdir);
  bool nextcombo_wf();
  int  lookahead(int destscr = -1);
  int  lookaheadflag(int destscr = -1);
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
  void do_rafting();
  void do_hopping();
  bool walkflag(int wx,int wy,int cnt,byte d);
  bool checkmaze(mapscr *scr);
  void scrollscr(int dir,int destscr = -1, int destdmap = -1);
  void walkdown();
  void walkup();
  void walkdown2();
  void walkup2();
  bool dowarp(int type);
  void exitcave();
  void stepout();
  void masked_draw(BITMAP *dest);
  void getTriforce();
  void checkstab();
  void fairycircle();
  void StartRefill(int refill_why);
  int  EwpnHit();
  int  LwpnHit();
  void gameover();
  void ganon_intro();
  void saved_Zelda();
  void check_slash_block(int bx, int by);
//  bool check_pound_block(int bx, int by);
  void check_pound_block(int bx, int by);
  void check_conveyor();
  bool check_cheat_warp();
  
public:

  LinkClass() : sprite() { init(); }
  void init();
  virtual void draw(BITMAP* dest);
  virtual bool animate(int index);

  void linkstep() { lstep = lstep<(BSZ?27:11) ? lstep+1 : 0; }
  void stepforward(int steps);
  void draw_under(BITMAP* dest);

  // called by ALLOFF()
  void resetflags(bool all)
  {
    refilling=inlikelike=inwallm=false;
    blowcnt=whirlwind=hclk=fairyclk=didstuff=0;
    if(swordclk>0 || all)
      swordclk=0;
    hopclk=0;
    attackclk=0;
    diveclk=0;
    action=none;
  }

  void Freeze() { action=freeze; }
  void unfreeze() { if(action==freeze) action=none; }
  void beatlikelike() { inlikelike=false; }
  fix  getX()   { return x; }
  fix  getY()   { return y; }
  int  getDir() { return dir; }
  int  getClk() { return clk; }
  void reset_hookshot();
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
  int  getAction() { return action; };
  bool isDiving() { return (diveclk>30); }
  bool isSwimming() { return ((action==swimming)||(action==swimhit)||
                              (action==swimhold1)||(action==swimhold2)||
                              (hopclk==0xFF)); }
};


void LinkClass::init()
{
  hookshot_used=false;
  hookshot_frozen=false;
  dir = up;
  x=tmpscr->warpx2;
  y=tmpscr->warpy2;
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
}


void LinkClass::draw_under(BITMAP* dest)
{
  if(action==rafting){
    if (((dir==left) || (dir==right)) && (get_bit(QHeader.rules2,qr2_RLFIX))) {

      overtilerotate16(dest, itemsbuf[iRaft].tile, x, y+60,
        itemsbuf[iRaft].csets&15, (itemsbuf[iRaft].misc>>2)&3, 3);
    } else {
      overtile16(dest, itemsbuf[iRaft].tile, x, y+60,
        itemsbuf[iRaft].csets&15, (itemsbuf[iRaft].misc>>2)&3);
    }
  }

  if(ladderx+laddery) {

    if (((ladderdir==left)||(ladderdir==right)) && (get_bit(QHeader.rules2,qr2_RLFIX))) {
      overtilerotate16(dest, itemsbuf[iLadder].tile, ladderx, laddery+56,
        itemsbuf[iLadder].csets&15, (itemsbuf[iLadder].misc>>2)&3, 3);
    } else {
      overtile16(dest, itemsbuf[iLadder].tile, ladderx, laddery+56,
        itemsbuf[iLadder].csets&15, (itemsbuf[iLadder].misc>>2)&3);
    }
  }
}


void LinkClass::draw(BITMAP* dest)
{
  int swimspr = wpnsbuf[iwSwim].tile;

  yofs = (!BSZ && isdungeon() && currscr<128) ? 54 : 56;
  cs = ((hclk>>1)&3) + 6;
  if(superman)
    cs = (((~frame)>>1)&3)+6;

  if(attackclk || action==attacking)
  {
    if(attackclk>4)
    {
      if(attack==wSword || attack==wWand)// ||
//         attack==wHammer)
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
          t = wpnsbuf[game.sword-1].tile;
          cs = wpnsbuf[game.sword-1].csets&15;
        }

        switch(dir)
        {
        case up:
          wx=-1; wy=-12;
          if(attackclk==13) wy+=4;
          if(attackclk==14) wy+=8;
          break;
        case down:
          f=get_bit(QHeader.rules3,qr3_SWORDWANDFLIPFIX)?3:2; wy=11;
          if(attackclk==13) wy-=4;
          if(attackclk==14) wy-=8;
          break;
        case left:
          f=1; wx=-11; t++;
          if(attackclk==13) wx+=4;
          if(attackclk==14) wx+=8;
          break;
        case right:
          wx=11; t++;
          if(attackclk==13) wx-=4;
          if(attackclk==14) wx-=8;
          break;
        }

        if(BSZ || (isdungeon() && currscr<128))
          wy+=2;
        if(isdungeon() && currscr<128)
        {
          BITMAP *sub = create_sub_bitmap(dest,16,72,224,144);
          overtile16(sub,t,x+wx-16,y+wy-(72-yofs),cs,f);
          destroy_bitmap(sub);
        }
        else
          overtile16(dest,t,x+wx,y+yofs+wy,cs,f);
      }
    }

    if(attackclk<7 || ((attack==wSword || attack==wWand) && attackclk<13)
                     || ((attack==wHammer) && attackclk<=30))
    {
      flip=0;
      if(BSZ)
      {
        switch(dir) {
        case up:    tile=31; break;
        case down:  tile=27; break;
        case left:  flip=1;
        case right: tile=23; break;
        }
      }
      else
      {
        switch(dir) {
        case up:    tile=27; break;
        case down:  tile=26; break;
        case left:  flip=1;
        case right: tile=25; break;
        }
      }
      if((attack==wHammer) && (attackclk<13))
      {
        switch(dir) {
        case up:    tile=7; break;
        case down:  tile=6; break;
        case left:  flip=1;
        case right: tile=5; break;
        }
      }
      masked_draw(dest);
      if (attack!=wHammer) {
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

      switch(dir) {
        case up:
          wx=-1; wy=-15;
          if(attackclk>=13) {
            wx-=1; wy+=1; t++;
          }
          if(attackclk>=15) {
            t++;
          }
          break;
        case down:
          wx=3;   wy=-14;  t+=3;
          if(attackclk>=13) {
            wy+=16;
            t++;
          }
          if(attackclk>=15) {
            wx-=1; wy+=12;
            t++;
          }
          break;
        case left:
          wx=0;   wy=-14;  t+=6; f=1;
          if(attackclk>=13) {
            wx-=7; wy+=8;
            t++;
          }
          if(attackclk>=15) {
            wx-=8; wy+=8;
            t++;
          }
          break;
        case right:
          wx=0;  wy=-14;  t+=6;
          if(attackclk>=13) {
            wx+=7; wy+=8;
            t++;
          }
          if(attackclk>=15) {
            wx+=8; wy+=8;
            t++;
          }
          break;
      }

      if(BSZ || (isdungeon() && currscr<128))
        wy+=2;
      if(isdungeon() && currscr<128)
      {
        BITMAP *sub = create_sub_bitmap(dest,16,72,224,144);
        overtile16(sub,t,x+wx-16,y+wy-(72-yofs),cs,f);
        destroy_bitmap(sub);
      }
      else
        overtile16(dest,t,x+wx,y+yofs+wy,cs,f);
        if (attackclk==15) {
          sfx(WAV_HAMMER,pan(int(x)));
        }
      return;
    }




  }


/*
  if(BSZ)
  {
    if( (dir!=up) && ((game.misc&iSHIELD)||(game.misc&iMSHIELD)) )
    {
      switch(dir) {
      case down:  tile=11; break;
      case left:  flip++;
      case right: tile=8; break;
      }
      if(game.misc&iMSHIELD)
        tile+=6;
    }
    else
    {
      switch(dir) {
      case up:    tile=28; break;
      case down:  tile=24; break;
      case left:  flip++; break;
      }
    }
    tile += anim_3_4(lstep,7);
  }
  else if(action==swimming || action==swimhit || hopclk==0xFF)
  {
    switch(dir)
    {
    case up:    tile = swimspr + 4; break;
    case down:  tile = swimspr + 2; break;
    case left:  flip++; // go on
    case right: tile = swimspr; break;
    }
    if(lstep>=6)
      tile++;
    if(diveclk>30)
      tile = swimspr + 8 + ((frame>>3) & 1);
  }
  else
  {
    switch(dir)
    {
    case up:    tile=24; if(lstep>=6) flip++; break;
    case down:  tile=22; if(lstep>=6) tile++; break;
    case left:  flip++; if(lstep>=6) tile++; break;
    case right: if(lstep>=6) tile++;
    }
    if( (dir!=up) && (game.misc&iMSHIELD) )
      tile-=10;
    else if( (dir!=up) && (game.misc&iSHIELD) )
      tile+=10;
  }
*/


  if (action!=casting) {
  tile=20;
  flip=0;
  if(BSZ)
  {
    if(action==swimming || action==swimhit || hopclk==0xFF)
    {
      switch(dir)
      {
      case up:    tile = swimspr + 6; break;
      case down:  tile = swimspr + 3; break;
      case left:  flip++; // go on
      case right: tile = swimspr; break;
      }
      tile += anim_3_4(lstep,7);
/*
      if(lstep>=6)
        tile++;
*/
      if(diveclk>30)
        tile = swimspr + 11 + ((frame>>3) % 3);
    }
    else
    {
      if( (dir!=up) && ((game.misc&iSHIELD)||(game.misc&iMSHIELD)) )
      {
        switch(dir) {
        case down:  tile=11; break;
        case left:  flip++;
        case right: tile=8; break;
        }
        if(game.misc&iMSHIELD)
          tile+=6;
      }
      else
      {
        switch(dir) {
        case up:    tile=28; break;
        case down:  tile=24; break;
        case left:  flip++; break;
        }
      }
      tile += anim_3_4(lstep,7);
    }
  }
  else //!BSZ animation
  {
    if(action==swimming || action==swimhit || hopclk==0xFF)
    {
      switch(dir)
      {
      case up:    tile = swimspr + 4; break;
      case down:  tile = swimspr + 2; break;
      case left:  flip++; // go on
      case right: tile = swimspr; break;
      }
      if(lstep>=6)
        tile++;
      if(diveclk>30)
        tile = swimspr + 8 + ((frame>>3) & 1);
    }
    else
    {
      switch(dir)
      {
        case up:    tile=24; if(lstep>=6) flip++; break;
        case down:  tile=22; if(lstep>=6) tile++; break;
        case left:  flip++; if(lstep>=6) tile++; break;
        case right: if(lstep>=6) tile++;
      }
      if( (dir!=up) && (game.misc&iMSHIELD) )
        tile-=10;
      else if( (dir!=up) && (game.misc&iSHIELD) )
        tile+=10;
    }
  }
  }


  yofs = (!BSZ && isdungeon() && currscr<128) ? 54 : 56;
  if(action==won)
    yofs=54;

  if(action==holding1 || action==holding2)
  {
    yofs = BSZ ? 56 : 54;
    if(BSZ)
      tile = (action==holding1)?33:32;
    else
      tile = (action==holding1)?34:29;
    flip = 0;
    putitem(dest,x-((action==holding1)?4:0),y+yofs-16,holditem);
  }
  else if(action==swimhold1 || action==swimhold2)
  {
    if(BSZ)
      tile = (action==swimhold1) ? 33 : 32;
    else
      tile = (action==swimhold1) ? swimspr+7 : swimspr+6;
    flip = 0;
    putitem(dest,x-((action==swimhold1)?4:0),y+yofs-12,holditem);
  }

  masked_draw(dest);

  if((didstuff&did_fairy)||fairyclk==0||(get_bit(QHeader.rules3,qr3_NOHEARTRING)))
    return;

  double a = fairyclk*2*PI/80 + (PI/2);
  int hearts=0;
  int htile = QHeader.data_flags[ZQ_TILES] ? 2 : 0;
  do {
    int nx=125;
    if (get_bit(QHeader.rules3,qr3_HEARTRINGFIX)) {
      nx=x;
    }
    int ny=88;
    if (get_bit(QHeader.rules3,qr3_HEARTRINGFIX)) {
      ny=y;
    }
    double tx = cos(a)*53  +nx;
    double ty = -sin(a)*53 +ny+56;
    overtile8(dest,htile,int(tx),int(ty),1,0);
    a-=PI/4;
    hearts++;
  } while(a>PI/2 && hearts<8);
}



void LinkClass::masked_draw(BITMAP* dest)
{
 if(isdungeon() && currscr<128 &&
  (x<16 || x>224 || y<18 || y>146))
  { // clip under doorways
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

  if((attack==wHammer) && (attackclk<15))
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
    if(guys.spr(i)->hit(wx,wy,wxsz,wysz) || (attack==wWand && guys.spr(i)->hit(x,y-8,16,24))
                                         || (attack==wHammer && guys.spr(i)->hit(x,y-8,16,24)))
    {
      int h = hit_enemy(i,attack,(attack==wSword)
                        ? ((1*DAMAGE_MULTIPLIER)<<(game.sword-1)) :
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
  for(int j=0; j<items.Count(); j++) {
    if(((item*)items.spr(j))->pickup & ipTIMER)
     if(((item*)items.spr(j))->clk2 >= 32) {
      if(items.spr(j)->hit(wx,wy,wxsz,wysz) || (attack==wWand && items.spr(j)->hit(x,y-8,16,24))
                                            || (attack==wHammer && items.spr(j)->hit(x,y-8,16,24))) {
        getitem(items.spr(j)->id);
        items.del(j);
        j--;
        }
      }
    }


  if(attack==wSword) {
    for (int q=0; q<176; q++) {
      set_bit(screengrid,q,0);
    }
    // cutable blocks
    if(dir==up && (int(x)&15)==0)
    {
      check_slash_block(x,wy);
      check_slash_block(x,wy+8);
    }
    if(dir==up && (int(x)&15)==8)
    {
      check_slash_block(x,wy);
      check_slash_block(x,wy+8);
      check_slash_block(x+8,wy);
      check_slash_block(x+8,wy+8);
    }
    if(dir==down && (int(x)&15)==0)
    {
      check_slash_block(x,wy+wysz-8);
      check_slash_block(x,wy+wysz);
    }
    if(dir==down && (int(x)&15)==8)
    {
      check_slash_block(x,wy+wysz-8);
      check_slash_block(x,wy+wysz);
      check_slash_block(x+8,wy+wysz-8);
      check_slash_block(x+8,wy+wysz);
    }
    if(dir==left)
    {
      check_slash_block(wx,y+8);
      check_slash_block(wx+8,y+8);
    }
    if(dir==right)
    {
      check_slash_block(wx+wxsz,y+8);
      check_slash_block(wx+wxsz-8,y+8);
    }
  } else if ((attack==wHammer) && (attackclk==15)) {
    // poundable blocks

    for (int q=0; q<176; q++) {
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
/*
    if(dir==up && (int(x)&15)==0)
    {
      if (check_pound_block(x,wy)) alert("1a",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x,wy+8)) alert("1b",NULL,NULL,"O&K",NULL,'k',0);
    }
    if(dir==up && (int(x)&15)==8)
    {
      if (check_pound_block(x,wy)) alert("2a",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x,wy+8)) alert("2b",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x+8,wy)) alert("2c",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x+8,wy+8)) alert("2d",NULL,NULL,"O&K",NULL,'k',0);
    }
    if(dir==down && (int(x)&15)==0)
    {
      if (check_pound_block(x,wy+wysz-8)) alert("3a",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x,wy+wysz)) alert("3b",NULL,NULL,"O&K",NULL,'k',0);
    }
    if(dir==down && (int(x)&15)==8)
    {
      if (check_pound_block(x,wy+wysz-8)) alert("4a",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x,wy+wysz)) alert("4b",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x+8,wy+wysz-8)) alert("4c",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(x+8,wy+wysz)) alert("4d",NULL,NULL,"O&K",NULL,'k',0);
    }
    if(dir==left)
    {
      if (check_pound_block(wx,y+8)) alert("5a",NULL,NULL,"O&K",NULL,'k',0);
      if (check_pound_block(wx+8,y+8)) alert("5b",NULL,NULL,"O&K",NULL,'k',0);
    }
    if(dir==right)
    {
      if (check_pound_block(wx+wxsz,y+8)) alert("6a",NULL,NULL,"O&K",NULL,'k',0);
      if ((int(x)&15)>7)
        if (check_pound_block(wx+wxsz-8,y+8)) alert("6b",NULL,NULL,"O&K",NULL,'k',0);
    }
*/
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

  if(type!=cSLASH && type!=cSLASHITEM)
    return;

  if(i > 175)
    return;

  if (get_bit(screengrid,i)) {
    return;
  }

  mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);

  if((flag >= 16)&&(flag <= 31))
  {
    s->data[i] = tcmbdat(s->secret[(s->sflag[i])-16+4]);
    s->sflag[i] = tcmbflag(s->secret[(s->sflag[i])-16+4]);
  }
  else if(flag == mfARMOS_SECRET)
  {
    s->data[i] = tcmbdat(s->secret[3]);
    s->sflag[i] = tcmbflag(s->secret[3]);
    sfx(WAV_SECRET);
  }
  else
  {
    s->data[i] = s->under&0x7FFF;
    s->sflag[i] = 0;
    set_bit(screengrid,i,1);
//    pausenow=true;
  }

  if(flag==mfARMOS_ITEM && !getmapflag())
  {
    items.add(new item(bx, by, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
    sfx(WAV_SECRET);
  }
  else if(type==cSLASHITEM)
  {
    int it=-1;
    int r=rand()%100;

    if(r<15)      it=iHeart;      // 15%
    else if(r<35) it=iRupy;      // 20%

    if(it!=-1)
      items.add(new item(bx, by, it, ipBIGRANGE + ipTIMER, 0));
  }

  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cpage);
}

void LinkClass::check_pound_block(int bx, int by)
{
  bx &= 0xF0;
  by &= 0xF0;

  int type = COMBOTYPE(bx,by);
  int flag = MAPFLAG(bx,by);
  int i = (bx>>4) + by;

  if(type!=cPOUND)
    return;

  if(i > 175)
    return;


  if (get_bit(screengrid,i)) {
    return;
  }

  mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);

  if((flag >= 16)&&(flag <= 31))
  {
    s->data[i] = tcmbdat(s->secret[(s->sflag[i])-16+4]);
    s->sflag[i] = tcmbflag(s->secret[(s->sflag[i])-16+4]);
  }
  else if(flag == mfARMOS_SECRET)
  {
    s->data[i] = tcmbdat(s->secret[3]);
    s->sflag[i] = tcmbflag(s->secret[3]);
    sfx(WAV_SECRET);
  }
  else
  {
    s->data[i]+=1;
    set_bit(screengrid,i,1);
  }

  if(flag==mfARMOS_ITEM && !getmapflag())
  {
    items.add(new item(bx, by, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
    sfx(WAV_SECRET);
  }
  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cpage);
  return;
}

/*
bool LinkClass::check_pound_block(int bx, int by)
{
  bx &= 0xF0;
  by &= 0xF0;

  int type = COMBOTYPE(bx,by);
  int flag = MAPFLAG(bx,by);
  int i = (bx>>4) + by;

  if(type!=cPOUND)
    return false;

  if(i > 175)
    return false;

  mapscr *s = tmpscr + ((currscr>=128) ? 1 : 0);

  if(flag >= 16)
  {
    s->data[i] = tcmbdat(s->secret[(s->sflag[i])-16+4]);
    s->sflag[i] = tcmbflag(s->secret[(s->sflag[i])-16+4]);
  }
  else if(flag == mfARMOS_SECRET)
  {
    s->data[i] = tcmbdat(s->secret[3]);
    s->sflag[i] = tcmbflag(s->secret[3]);
    sfx(WAV_SECRET);
  }
  else
  {
    s->data[i]+=1;
  }

  if(flag==mfARMOS_ITEM && !getmapflag())
  {
    items.add(new item(bx, by, tmpscr->catchall, ipONETIME + ipBIGRANGE + ipHOLDUP, 0));
    sfx(WAV_SECRET);
  }
  putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cpage);
  return true;
}

*/

int LinkClass::EwpnHit()
{
  for(int i=0; i<Ewpns.Count(); i++)
    if(Ewpns.spr(i)->hit(x+7,y+7,2,2))
    {
      weapon *ew = (weapon*)(Ewpns.spr(i));
      bool hitshield=false;
      if ((ew->ignoreLink)==true)
        break;
      if (ew->id==ewWind) {
        xofs=1000;
        action=freeze;
        ew->misc=999; // in enemy wind
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
        return i;

      switch(ew->id)
      {
      case ewFireball:
      case ewMagic:
      case ewSword:
        if(!((game.misc&iSHIELD)||(game.misc&iMSHIELD)) || ew->type)
        break;
      case ewFlame:
        if(!(game.misc&iMSHIELD))
          return i;
        break;
      }

      int oldid = ew->id;
      ew->onhit(false, (game.misc&iMSHIELD) ? 2 : 1, dir);
      if(ew->id != oldid) // changed type from ewX to wX
      {
        ew->power*=DAMAGE_MULTIPLIER;
        Lwpns.add(ew);
        Ewpns.remove(ew);
      }
      if (ew->id==wRefMagic) {
        ew->ignoreLink=true;
        ew->ignorecombo=-1;
      }

      sfx(WAV_CHINK,pan(int(x)));
    }
  return -1;
}

int LinkClass::LwpnHit() //only here to check magic hits
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
      if(!hitshield || action==attacking || action==swimming || hopclk==0xFF)
        return i;

      switch(lw->id)
      {
      case wMagic:
      case wRefMagic:
        if(!((game.misc&iSHIELD)||(game.misc&iMSHIELD)) || lw->type)
          return i;
        break;
      }

      lw->onhit(false, (game.misc&iMSHIELD) ? 2 : 1, dir);
      if (lw->id==wRefMagic) {
        lw->ignoreLink=true;
        lw->ignorecombo=-1;
      }

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
 if(hclk<39 && action==swimhit)
   action=swimming;

 if(hclk>=40 && action==gothit)
 {
   for(int i=0; i<4; i++)
   {
     switch(hitdir) {
     case up:    if(hit_walkflag(x,y+7,2))    action=none; else y--; break;
     case down:  if(hit_walkflag(x,y+16,2))   action=none; else y++; break;
     case left:  if(hit_walkflag(x-1,y+8,1))  action=none; else x--; break;
     case right: if(hit_walkflag(x+16,y+8,1)) action=none; else x++; break;
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

   if (!get_bit(QHeader.rules3,qr3_FIREPROOFLINK)) {
     if(s->id==wFire && (superman ? s->hit(x+7,y+7,2,2) : s->hit(this))&&
        ((weapon*)(Lwpns.spr(i)))->type<3)
     {
       game.life = max(game.life-((HP_PER_HEART/2)>>game.ring),0);
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

   if (get_bit(QHeader.rules3,qr3_OUCHBOMBS)) {
     if(((s->id==wBomb)||(s->id==wSBomb)) && (superman ? s->hit(x+7,y+7,2,2) : s->hit(this)))
     {
       game.life = max(game.life-((4*HP_PER_HEART/2)>>game.ring),0);
       if (s->id==wSBomb) {
         game.life = max(game.life-((12*HP_PER_HEART/2)>>game.ring),0);
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
/*
   if((s->id==wRefMagic)||(s->id==wMagic))
   {
     if ((game.misc&iMSHIELD)&&(s->dir=dir^1)) {
       s->dir=s->dir^1;
     } else {
       game.life = max(game.life-((((weapon*)(Lwpns.spr(i)))->power*=DAMAGE_MULTIPLIER)>>game.ring),0);
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
*/
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
     game.life = max(game.life-(ewpn_dp(hit)>>game.ring),0);
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
     game.life = max(game.life-(ewpn_dp(hit)>>game.ring),0);
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

 int ctype1(combobuf[MAPCOMBO(x,y+8)].type);
 int ctype2(combobuf[MAPCOMBO(x+8,y+8)].type);
 int ctype=max(ctype1, ctype2);

 if((ctype>=cDAMAGE1) && (ctype<=cDAMAGE4))
 {
   if((!(game.misc2&iBOOTS)) ||
     ((get_bit(QHeader.rules4,qr4_MAGICBOOTS))&&
      (game.magic+game.dmagic<BOOTSDRAINAMOUNT*game.magicdrainrate))) {
     game.life = max((game.life-((HP_PER_HEART<<2)>>(cDAMAGE4-ctype))),0);
     hitdir = (dir^1);
     if(action!=rafting && action!=freeze)
       action=gothit;
     if(action==swimming || hopclk==0xFF)
       action=swimhit;
     hclk=48;
     sfx(WAV_OUCH,pan(int(x)));
     return;
   } else if (get_bit(QHeader.rules4,qr4_MAGICBOOTS)) {
     if (!(magicdrainclk%BOOTSDRAINSPEED)) {
       game.magic-=BOOTSDRAINAMOUNT*game.magicdrainrate;
     }
   }

 }



}


void LinkClass::hitlink(int hit)
{
   if(superman)
     return;

   game.life = max(game.life-(enemy_dp(hit)>>game.ring),0);
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
   }
}



// returns true when game over
bool LinkClass::animate(int index)
{
  if (pausenow) {
    jwin_alert("Slash","Point B",NULL,NULL,"OK",NULL,13,27);
  }
  if ((watch==true) && (get_bit(QHeader.rules3,qr3_TEMPCLOCKS))) {
      clockclk++;
      if (clockclk==256) {
        if (cheat_superman==false) {
          setClock(false);
        }
        watch=false;
        for (int zoras=0; zoras<clock_zoras; zoras++) {
          addenemy(0,0,eZORA,0);
        }
      }
  }
  if (hookshot_frozen==true) {
  if (hookshot_used==true) {
    action=freeze;
    if (pull_link==true) {
      sprite *t;
      int i;
      for(i=0; i<Lwpns.Count() && (Lwpns.spr(i)->id!=wHSHandle); i++);
      t = Lwpns.spr(i);
      for(i=0; i<Lwpns.Count(); i++)
      {
        sprite *s = Lwpns.spr(i);
        if(s->id==wHookshot) {
          if ((s->y)>y) {
            y+=4;
            t->y+=4;
            hs_starty+=4;
          }
          if ((s->y)<y) {
            y-=4;
            t->y-=4;
            hs_starty-=4;
          }
          if ((s->x)>x) {
            x+=4;
            t->x+=4;
            hs_startx+=4;
          }
          if ((s->x)<x) {
            x-=4;
            t->x-=4;
            hs_startx-=4;
          }
        }
      }
    }
  } else {
    Lwpns.del(Lwpns.idFirst(wHSHandle));
    reset_hookshot();
  }
  if (hs_fix) {
    if (dir==up) {
      y=int(y+7)&0xF0;
    }
    if (dir==down) {
      y=int(y+7)&0xF0;
    }
    if (dir==left) {
      x=int(x+7)&0xF0;
    }
    if (dir==right) {
      x=int(x+7)&0xF0;
    }
    hs_fix=false;
  }

  }
  if(rLbtn())         selectBwpn(-1);
  else if(rRbtn())    selectBwpn(1);
  if(rPbtn())         onViewMap();

  // make the flames from the wand
  if(wand_dead)
  {
    wand_dead=false;
    if(game.misc&iBOOK && Lwpns.idCount(wFire)<2)
    {
      Lwpns.add(new weapon(wand_x,wand_y,wFire,1,1*DAMAGE_MULTIPLIER,0));
      sfx(WAV_FIRE,pan(wand_x));
    }
  }

  if(add_asparkle)
  {
    if (game.arrow>=2) {
      Lwpns.add(new weapon(int(arrow_x+(rand()%7)),
                           int(arrow_y+(rand()%7)),
                           wSSparkle+(game.arrow-2),0,0,0));
    }
    add_asparkle=false;
  }

  if(add_bsparkle)
  {
    if (game.brang>=2) {
      Lwpns.add(new weapon(int(brang_x+(rand()%7)),
                           int(brang_y+(rand()%7)),
                           wMSparkle+(game.brang-2),0,0,0));
    }
    add_bsparkle=false;
  }

  if(add_df1asparkle)
  {
    Lwpns.add(new weapon(int(df_x+(rand()%7)),
                         int(df_y+(rand()%7)),
                         wPhantom,2,0,0));
    add_df1asparkle=false;
  }

  if(add_df1bsparkle)
  {
    Lwpns.add(new weapon(int(df_x+(rand()%7)),
                         int(df_y+(rand()%7)),
                         wPhantom,3,0,0));
    add_df1bsparkle=false;
  }



  if (add_chainlink) {
    chainlinks.add(new weapon(chainlink_x, chainlink_y, wHSChain, 0,0,dir));
    chainlinks_count++;
    add_chainlink=false;
  }

  if (del_chainlink) {
    if (pull_link==false) {
      chainlinks.del(chainlinks.idLast(wHSChain));
    } else {
      chainlinks.del(chainlinks.idFirst(wHSChain));
    }
    chainlinks_count--;
    del_chainlink=false;
  }

  checkhit();
  if(game.life<=0)
  {
    gameover();
    return true;
  }

  if(swordclk>0)
    swordclk--;

  if(inwallm)
  {
    attackclk=0;
    linkstep();
    if(CarryLink()==false)
      restart_level();
    return false;
  }

  if (ewind_restart) {
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
  freeze_guys=false; // reset this flag, set it again if holding
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
    if (magictype==mgc_none) {
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
    movelink(); // call the main movement routine
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
    checkpushblock();
    if (hookshot_frozen==false) {
      checkspecial();
    }
    if(action==won)
      return true;
  }

  if(frame - newscr_clk == 1)
  {
    if(tmpscr->room==rGANON)
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

  if(isdungeon() && action!=freeze && loaded_guys && !inlikelike)
  {
    if(((dtype==dBOMBED)?Up():dir==up)
      && x==120 && y<=32 && tmpscr->door[0]==dtype)
      { walk=true; dir=up; }

    if(((dtype==dBOMBED)?Down():dir==down)
      && x==120 && y>=128 && tmpscr->door[1]==dtype)
      { walk=true; dir=down; }

    if(((dtype==dBOMBED)?Left():dir==left)
      && x<=32 && y==80 && tmpscr->door[2]==dtype)
      { walk=true; dir=left; }

    if(((dtype==dBOMBED)?Right():dir==right)
      && x>=208 && y==80 && tmpscr->door[3]==dtype)
      { walk=true; dir=right; }
  }
  if(walk)
  {
    hclk=0;
    drawguys=false;
    if(dtype==dWALK)
      sfx(WAV_SECRET);
    stepforward(25);
    action=scrolling;
    pushing=false;
  }

  if(game.life<=(HP_PER_HEART)) {
    if (heart_beep) {
      cont_sfx(WAV_ER);
    } else {
      if (heart_beep_timer==-1) {
        heart_beep_timer=70;
      }
      if (heart_beep_timer>0) {
        heart_beep_timer--;
        cont_sfx(WAV_ER);
      } else {
        stop_sfx(WAV_ER);
      }
    }
  } else {
    heart_beep_timer=-1;
    stop_sfx(WAV_ER);
  }

  if(rSbtn()) {
    conveyclk=3;
    dosubscr();
  }

  checkstab();
  if (pausenow) {
    jwin_alert("Slash","Point A",NULL,NULL,"OK",NULL,13,27);
  }

  check_conveyor();

  check_cheat_warp();
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
bool use_hookshot=true;

switch(wpn) {

 case iRPotion:
 case iBPotion:
  game.potion--;
  Bwpn=0;
  refill_what=REFILL_LIFE;
  StartRefill(REFILL_POTION);
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
  if(dir==up || dir==right)
    blowcnt++;
  else
    blowcnt--;
 
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
    whistleclk=0; // signal to start drying lake or doing other stuff
  else if(dlevel==0 && TriforceCount())
    Lwpns.add(new weapon(0,y,wWind,0,0,right));
  return false;


 case iBombs: {
  if(Lwpns.idCount(wLitBomb))
    return false;
  if(!debug)
    game.bombs--;
  selectBwpn(8);
  Lwpns.add(new weapon(wx,wy,wBomb,0,4*DAMAGE_MULTIPLIER,dir));
  sfx(WAV_PLACE,pan(wx));
  } break;


 case iSBomb: {
  if(Lwpns.idCount(wLitSBomb))
    return false;
  if(!debug)
    game.sbombs--;
  selectBwpn(8);
  Lwpns.add(new weapon(wx,wy,wSBomb,0,16*DAMAGE_MULTIPLIER,dir));
  sfx(WAV_PLACE,pan(wx));
  } break;


 case iWand:
  if(Lwpns.idCount(wMagic))
    return false;
  if((get_bit(QHeader.rules4,qr4_ENABLEMAGIC))&&
     ((game.magic+game.dmagic)<WANDDRAINAMOUNT*game.magicdrainrate)&&
     ((get_bit(QHeader.rules4,qr4_MAGICWAND))))
    return false;
  if(Lwpns.idCount(wBeam))
    Lwpns.del(Lwpns.idFirst(wBeam));
  Lwpns.add(new weapon(wx,wy,wMagic,0,2*DAMAGE_MULTIPLIER,dir));
  if (get_bit(QHeader.rules4,qr4_MAGICWAND))
    game.magic-=(WANDDRAINAMOUNT*game.magicdrainrate);
  sfx(WAV_WAND,pan(wx));
  break;


 case iSword:
  if(Lwpns.idCount(wBeam)||Lwpns.idCount(wMagic))
    return false;
//  Lwpns.add(new weapon(wx,wy,wBeam,0,((get_bit(QHeader.rules2,qr2_BEAMHALFPWR))?1:2)<<(game.sword-1),dir));
//  ((x<<z)>>y)
//  ((DAMAGE_MULTIPLIER<<(game.sword-1))>>(get_bit(QHeader.rules2,qr2_BEAMHALFPWR)))
//  x=DAMAGE_MULTIPLIER
//  y=(game.sword-1)
//  z=(get_bit(QHeader.rules2,qr2_BEAMHALFPWR))
  Lwpns.add(new weapon(wx,wy,wBeam,0,((DAMAGE_MULTIPLIER<<(game.sword-1))>>(get_bit(QHeader.rules2,qr2_BEAMHALFPWR))),dir));
  sfx(WAV_BEAM,pan(wx));
  break;

 case iBCandle: if(didstuff&did_candle) return false;
 case iRCandle:
  if(Lwpns.idCount(wFire)>=2)
    return false;
  if((get_bit(QHeader.rules4,qr4_ENABLEMAGIC))&&
     ((game.magic+game.dmagic)<CANDLEDRAINAMOUNT*game.magicdrainrate)&&
     ((get_bit(QHeader.rules4,qr4_MAGICCANDLE))))
    return false;
  didstuff|=did_candle;
  Lwpns.add(new weapon(wx,wy,wFire,0,1*DAMAGE_MULTIPLIER,dir));
  if (get_bit(QHeader.rules4,qr4_MAGICCANDLE))
    game.magic-=(CANDLEDRAINAMOUNT*game.magicdrainrate);
  sfx(WAV_FIRE,pan(wx));
  attack=wFire;
/*
  if((get_bit(QHeader.rules4,qr4_ENABLEMAGIC))&&
     ((game.magic+game.dmagic)<DINSFIREDRAINAMOUNT*game.magicdrainrate))
    return false;
   game.magic-=(DINSFIREDRAINAMOUNT*game.magicdrainrate);
   action=casting;
   magictype=mgc_dinsfire;
*/
   break;


 case iGArrow:
 case iSArrow:
 case iArrow:
  if(Lwpns.idCount(wArrow))
    return false;
  if(game.drupy+game.rupies<=0)
    return false;
  game.drupy--;
  Lwpns.add(new weapon(wx,wy,wArrow,game.arrow,(1*DAMAGE_MULTIPLIER)<<game.arrow,dir));
  sfx(WAV_ARROW,pan(wx));
  break;


 case iBait:
  if(Lwpns.idCount(wBait))
    return false;
  if(tmpscr->room==rGRUMBLE && !getmapflag()) {
    items.add(new item(wx,wy,iBait,ipDUMMY+ipFADE,0));
    fadeclk=66;
    msgstr=0;
    clear(msgdisplaybuf);
//    putscr(scrollbuf,0,0,tmpscr);
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
  Lwpns.add(new weapon(wx,wy,wBrang,game.brang,(game.brang*DAMAGE_MULTIPLIER),dir));
  break;

 case iHookshot:
  if(Lwpns.idCount(wHookshot))
    return false;
  if (dir==up) {
    if ((combobuf[MAPCOMBO(x,y-7)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }
     
  if (dir==down) {
    if (int(x)&8) {
      if ((combobuf[MAPCOMBO(x+16,y+23)].type==cHSGRAB)) {
        use_hookshot=false;
      }
    } else if ((combobuf[MAPCOMBO(x,y+23)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }

  if (dir==left) {
    if (int(y)&8) {
      if ((combobuf[MAPCOMBO(x-7,y+16)].type==cHSGRAB)) {
        use_hookshot=false;
      }
    } else if ((combobuf[MAPCOMBO(x-7,y)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }
     
  if (dir==right) {
    if (int(y)&8) {
      if ((combobuf[MAPCOMBO(x+23,y+16)].type==cHSGRAB)) {
        use_hookshot=false;
      }
    } else if ((combobuf[MAPCOMBO(x+23,y)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }

/*
  if (dir==up) {
    if ((combobuf[MAPCOMBO(x,y+6)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }
     
  if (dir==down) {
    if (int(x)&8) {
      if ((combobuf[MAPCOMBO(x+16,y+9)].type==cHSGRAB)) {
        use_hookshot=false;
      }
    } else if ((combobuf[MAPCOMBO(x,y+9)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }

  if (dir==left) {
    if (int(y)&8) {
      if ((combobuf[MAPCOMBO(x+6,y+16)].type==cHSGRAB)) {
        use_hookshot=false;
      }
    } else if ((combobuf[MAPCOMBO(x+6,y)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }
     
  if (dir==right) {
    if (int(y)&8) {
      if ((combobuf[MAPCOMBO(x+9,y+16)].type==cHSGRAB)) {
        use_hookshot=false;
      }
    } else if ((combobuf[MAPCOMBO(x+9,y)].type==cHSGRAB)) {
      use_hookshot=false;
    }
  }
*/

  if (use_hookshot) {
    if (dir==up) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x,y-20,wHookshot,0,0,dir));
      hs_startx=x; hs_starty=y-20;
    }
    if (dir==down) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x,y+20,wHookshot,0,0,dir));
      hs_startx=x; hs_starty=y+20;
    }
    if (dir==left) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x-20,y,wHookshot,0,0,dir));
      hs_startx=x-20; hs_starty=y;
    }
    if (dir==right) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x+20,y,wHookshot,0,0,dir));
      hs_startx=x+20; hs_starty=y;
    }
/*
    if (dir==up) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x,y-20,wHookshot,0,0,dir));
      hs_startx=x; hs_starty=y-20;
    }
    if (dir==down) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x,y+20,wHookshot,0,0,dir));
      hs_startx=x; hs_starty=y+20;
    }
    if (dir==left) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x-20,y,wHookshot,0,0,dir));
      hs_startx=x-20; hs_starty=y;
    }
    if (dir==right) {
      hookshot_used=true;
      Lwpns.add(new weapon(wx,wy,wHSHandle,0,0,dir));
      Lwpns.add(new weapon(x+20,y,wHookshot,0,0,dir));
      hs_startx=x+20; hs_starty=y;
    }
*/
    hookshot_frozen=true;
  }
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

  if(attack==wCatching && attackclk>4) {
    if(Up()||Down()||Left()||Right()) {
      lstep = s;
      return false;
      }
    }
  else if(attack!=wWand && attack!=wSword
       && attack!=wHammer && attackclk>7) {
    if(Up()||Down()||Left()||Right()) {
      lstep = s;
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
    if(attack==wSword && game.life+((HP_PER_HEART/2)-1)>=game.maxlife
      && (!get_bit(QHeader.rules2,qr2_NOSBEAM+game.sword-1)))
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
 if(action==attacking || action==hopping || action==swimming)
   return false;
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


inline bool isRaftFlag(int flag)
{
  return (flag==mfRAFT || flag==mfRAFT_BRANCH);
}


void do_lens()
{

//  if(cBbtn() && lensclk==0 && (lenscnt > 0 || game.drupy+game.rupies > 0))
//  ((get_bit(QHeader.rules4,qr4_ENABLEMAGIC))? (game.drupy+game.rupies > 0) : (game.dmagic+game.magic > 0))
  if(cBbtn() && lensclk==0 && (lenscnt > 0 ||
    ((get_bit(QHeader.rules4,qr4_ENABLEMAGIC)) ?
    (game.dmagic+game.magic > 0) :
    (game.drupy+game.rupies > 0))))
  {
    if(lenscnt)
      lenscnt--;
    else
    {
      if (get_bit(QHeader.rules4,qr4_ENABLEMAGIC)) {
        if (!(magicdrainclk%LENSDRAINSPEED)) {
          game.magic-=LENSDRAINAMOUNT*game.magicdrainrate;
        }
      } else {
        game.drupy-=LENSDRAINAMOUNT/2;
      }
      lenscnt = 4;
    }
    lensclk = 3;
  }
}


void LinkClass::do_hopping()
{
  if(Bwpn==iLens)
    do_lens();

  if(hopclk == 0xFF)    // swimming
  {
    if(diveclk>0)
      diveclk--;
    else if(rAbtn())
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
  else  // hopping in or out (need to separate the cases...)
  {
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
      int xofs = int(x)&15;
      int yofs = int(y)&15;
      int s = 1 + (frame&1);
      switch(dir)
      {
      case up:    if(yofs<3 || yofs>13) y--; else y-=s; break;
      case down:  if(yofs<3 || yofs>13) y++; else y+=s; break;
      case left:  if(xofs<3 || xofs>13) x--; else x-=s; break;
      case right: if(xofs<3 || xofs>13) x++; else x+=s; break;
      }
    }
  }
}



void LinkClass::do_rafting()
{

  if(Bwpn==iLens)
    do_lens();

  linkstep();
  if(!(int(x)&15) && !(int(y)&15))
  {
    if(MAPFLAG(x,y)==mfRAFT_BRANCH)
    {
      if(dir!=down && Up() && isRaftFlag(nextflag(x,y,up)))
      {
        dir = up;
        goto skip;
      }
      if(dir!=up && Down() && isRaftFlag(nextflag(x,y,down)))
      {
        dir = down;
        goto skip;
      }
      if(dir!=right && Left() && isRaftFlag(nextflag(x,y,left)))
      {
        dir = left;
        goto skip;
      }
      if(dir!=left && Right() && isRaftFlag(nextflag(x,y,right)))
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
  case up:    y--; break;
  case down:  y++; break;
  case left:  x--; break;
  case right: x++; break;
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
   diveclk--;
 }
 else if(action==swimming)
 {
   if(rAbtn())
     diveclk=80;
 }

 if(action==rafting)
 {
   do_rafting();
   if(action==rafting)
     return;
 }

 if(can_attack() && game.sword>0 && swordclk==0 && rAbtn())
 {
   action=attacking;
   attack=wSword;
   attackclk=0;
   sfx(WAV_SWORD,pan(int(x)));
 }

 if(Bwpn==iLens)
 {
   do_lens();
 }
 else if(can_attack() && rBbtn())
 {
   if(Bwpn==iWand)
   {
     action=attacking;
     attack=wWand;
     attackclk=0;
   } else if(Bwpn==iHammer) {
     action=attacking;
     attack=wHammer;
     attackclk=0;
   }
   else if(startwpn(Bwpn))
   {
     if (action==casting) {
       return;
     } else {
       action=attacking;
       attackclk=0;
       attack=none;
       if(Bwpn==iBrang || Bwpn==iMBrang || Bwpn==iFBrang)
         attack=wBrang;
     }
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
// int xstep=(lsteps[gofast][int(x)&7]);
// int ystep=lsteps[gofast][int(y)&7];
       if(int(x)&8) {
         if(!walkflag(x,y+8-(lsteps[gofast][int(y)&7]),2,up))
           move(up);
         else action=none; }
//       else if(!walkflag(x+8,y+9,1,up))
       else if(!walkflag(x,y+8-(lsteps[gofast][int(y)&7]),2,up))
         move(up);
       else action=none;
       }
     if(dir==down) {
       if(int(x)&8) {
         if(!walkflag(x,y+15+(lsteps[gofast][int(y)&7]),2,down))
           move(down);
         else action=none; }
//       else if(!walkflag(x+8,y+16,1,down))
       else if(!walkflag(x,y+15+(lsteps[gofast][int(y)&7]),2,down))
         move(down);
       else action=none;
       }
     if(dir==left) {
       if(!walkflag(x-(lsteps[gofast][int(x)&7]),y+8,1,left))
         move(left);
       else action=none;
       }
     if(dir==right) {
       if(!walkflag(x+15+(lsteps[gofast][int(x)&7]),y+8,1,right))
         move(right);
       else action=none;
       }
     return;
     }
   }

 if((action!=swimming)&&(action!=casting)) {
   action=none;
 }

 if(isdungeon() && (x<=26 || x>=214))
   goto LEFTRIGHT;

 // make it easier to get in left & right doors
 if(isdungeon() && Left() && x==32 && y==80 && !walkflag(x-(lsteps[gofast][int(x)&7]),y+8,1,left))
 {
   move(left);
   return;
 }
 if(isdungeon() && Right() && x==208 && y==80 && !walkflag(x+15+(lsteps[gofast][int(x)&7]),y+8,1,right))
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
   } else {
     if((isdungeon())||(int(x)&8)) {
       if(!walkflag(x,y+8-(lsteps[gofast][int(y)&7]),2,up)) {
         move(up);
         return;
       }
     } else {
//       if(!walkflag(x,y+7,1,up)) {
       if(!walkflag(x,y+8-(lsteps[gofast][int(y)&7]),2,up)) {
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
   } else {
     if((isdungeon())||(int(x)&8)) {
       if(!walkflag(x,y+15+(lsteps[gofast][int(y)&7]),2,down)) {
         move(down);
         return;
       }
     } else {
//       if(!walkflag(x,y+16,1,down)) {
       if(!walkflag(x,y+15+(lsteps[gofast][int(y)&7]),2,down)) {
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
   }
   return;
 }

LEFTRIGHT:

 if(isdungeon() && (y<=26 || y>=134))
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
   } else {
     if(!walkflag(x-(lsteps[gofast][int(x)&7]),y+8,1,left))
       move(left);
     else if( !Up() && !Down() ) {
       pushing=push+1;
       dir=left;
       if(action!=swimming)
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
   } else {
     if(!walkflag(x+15+(lsteps[gofast][int(x)&7]),y+8,1,right))
       move(right);
     else if( !Up() && !Down() ) {
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
 int xstep=lsteps[gofast][int(x)&7];
 int ystep=lsteps[gofast][int(y)&7];
 if(combobuf[MAPCOMBO(x+7,y+8)].type==cWALKSLOW) {
   if(d<left) { if(ystep>1) { skipstep^=1; ystep=skipstep; } }
   else         if(xstep>1) { skipstep^=1; xstep=skipstep; }
   }
 switch(d) {
   case up:    dy-=ystep; break;
   case down:  dy+=ystep; break;
   case left:  dx-=xstep; break;
   case right: dx+=xstep; break;
   }
 dir=d;
 linkstep();
 if(action!=swimming)
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
 if(isdungeon() && currscr<128 && wy<40 && (x!=120 || _walkflag(120,24,2)) )
   return true;

 bool wf = _walkflag(wx,wy,cnt);

 if(action==swimming)
 {
/*   bool wf2 = false;

   switch(d)
   {
   case up:    wf2 = _walkflag(wx,wy-8,cnt); break;
   case down:  wf2 = _walkflag(wx,wy+8,cnt); break;
   case left:  wf2 = _walkflag(wx-8,wy,cnt); break;
   case right: wf2 = _walkflag(wx+8,wy,cnt); break;
   }
*/
   if(!wf)
   {
     // hop out of the water
     hopclk = 16;
   }
   else
   {
     bool wtrx  = iswater(MAPCOMBO(wx,wy));
     bool wtrx8 = iswater(MAPCOMBO(x+8,wy));

     if((d>=left && wtrx) || (d<=down && wtrx && wtrx8))
     {
       // swim
       hopclk = 0xFF;
       return false;
     }
   }
 }
 else if(ladderx+laddery)  // ladder is being used
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
 else if(wf)
 {
   // see if it's a good spot for the ladder or for swimming
   bool wtrx  = iswater(MAPCOMBO(wx,wy));
   bool wtrx8 = iswater(MAPCOMBO(x+8,wy));
   bool flgx  = _walkflag(wx,wy,1) && !wtrx;
   bool flgx8 = _walkflag(x+8,wy,1) && !wtrx8;

   // check if he can swim
   if(game.misc2&iFLIPPERS)
   {
     if((d>=left && wtrx) || (d<=down && wtrx && wtrx8))
     {
       hopclk = 16;
       return false;
     }
   }

   // check if he can use the ladder
   if(game.misc&iLADDER && (tmpscr->flags&fLADDER || isdungeon()))
   {
     // add ladder combos
     wtrx  = isstepable(MAPCOMBO(wx,wy));
     wtrx8 = isstepable(MAPCOMBO(x+8,wy));
     flgx  = _walkflag(wx,wy,1) && !wtrx;
     flgx8 = _walkflag(x+8,wy,1) && !wtrx8;

     if((d>=left && wtrx) || (d<=down && ((wtrx && !flgx8) || (wtrx8 && !flgx))) )
     {
       if( ((int(y)+15) < wy) || ((int(y)+8) > wy) )
         ladderdir = up;
       else
         ladderdir = left;

       if(ladderdir==up) {
         ladderx = int(x)&0xF8;
         laddery = wy&0xF0; }
       else {
         ladderx = wx&0xF0;
         laddery = int(y)&0xF8; }
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

 if( (t==cPUSH_WAIT || t==cPUSH_HW || t==cPUSH_HW2) && (pushing<16 || hasMainGuy()) ) return;
 if( (t==cPUSH_HEAVY || t==cPUSH_HW) && !(game.misc&iBRACELET) ) return;
 if( (t==cPUSH_HEAVY2 || t==cPUSH_HW2) && !(game.misc2&iL2BRACELET) ) return;

 bool doit=false;

 if((f==mfPUSH2 && dir<=down) || f==mfPUSH4)
   doit=true;

 if(get_bit(QHeader.rules,qr1_SOLIDBLK)) {
   switch(dir) {
   case up:    if(_walkflag(bx,by-8,2))    doit=false; break;
   case down:  if(_walkflag(bx,by+24,2))   doit=false; break;
   case left:  if(_walkflag(bx-16,by+8,2)) doit=false; break;
   case right: if(_walkflag(bx+16,by+8,2)) doit=false; break;
   }
 }

 if(doit) {
   tmpscr->sflag[(by&0xF0)+(bx>>4)]=0;
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
 if(!isdungeon()) return;
 if(pushing!=8) return;
 if(tmpscr->door[dir]!=dLOCKED) return;

 int si = (currmap<<7) + currscr;
 int di = nextscr(dir);

 switch(dir) {
 case up:
  if(y!=32 || x!=120) return;
  if(usekey()) {
    putdoor(0,up,dUNLOCKED);
    tmpscr->door[0]=dUNLOCKED;
    game.maps[si] |= 1;
    game.maps[di] |= 2;
    }
  else return;
  break;
 case down:
  if(y!=128 || x!=120) return;
  if(usekey()) {
    putdoor(0,down,dUNLOCKED);
    tmpscr->door[1]=dUNLOCKED;
    game.maps[si] |= 2;
    game.maps[di] |= 1;
    }
  else return;
  break;
 case left:
  if(y!=80 || x!=32) return;
  if(usekey()) {
    putdoor(0,left,dUNLOCKED);
    tmpscr->door[2]=dUNLOCKED;
    game.maps[si] |= 4;
    game.maps[di] |= 8;
    }
  else return;
  break;
 case right:
  if(y!=80 || x!=208) return;
  if(usekey()) {
    putdoor(0,right,dUNLOCKED);
    tmpscr->door[3]=dUNLOCKED;
    game.maps[si] |= 8;
    game.maps[di] |= 4;
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
   refill_what=REFILL_LIFE;
   StartRefill(REFILL_FAIRY);
   action=freeze;
   holdclk=0;
   hopclk=0;
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
   if(MAPFLAG(x,y))
     break;
   // fall through
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
    return TheMaps[ns].data[cmb];  	// entire combo code
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
    return TheMaps[ns].sflag[cmb];  	// flag
  }

  return MAPFLAG(cx,cy);
}




bool did_secret;


void LinkClass::checkspecial()
{
 checktouchblk();

 bool hasmainguy = hasMainGuy(); // calculate it once

 if(!(loaded_enemies && !hasmainguy))
   did_secret=false;
 else
 {
  // after beating enemies

  // if room has traps, guys don't come back
  if(guys.idCount(eTRAP))
    setmapflag(mTMPNORET);

  // item
  if(hasitem) {
    int Item=tmpscr->item;
    if(getmapflag())
      Item=0;
    if(Item) {
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
    sfx(WAV_SECRET);
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
    opendoors++;
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


/*
 if (!get_bit(QHeader.rules3,qr3_LINKEDCOMBOS)) {
   if(int(y)&15)
   {
  //  if(type!=cCAVE || combobuf[MAPCOMBO(x+16,y)].type!=cCAVE)
    if(combobuf[MAPCOMBO(x,y+8)].type!=cPIT)
      return;
    type=cPIT;
   }
 }
*/
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

/*
 // only allow disalignment with extra wide doors
 // from here on...
 if(int(x)&15)
 {
  if((type==cPIT)||(combobuf[MAPCOMBO(x+8,y)].type==cPIT))
    type=cPIT;
 }

 if (!get_bit(QHeader.rules3,qr3_LINKEDCOMBOS)) {
   if(int(x)&15)
   {
  //  if(type!=cCAVE || combobuf[MAPCOMBO(x+16,y)].type!=cCAVE)
    if((type!=cCAVE || combobuf[MAPCOMBO(x+16,y)].type!=cCAVE) && type!=cPIT)
      return;
   }
 }
*/


 if(int(x)&15)
 {
  if((COMBOTYPE(x,y)!=COMBOTYPE(x+8,y))&&
     (COMBOTYPE(x,y)!=cPIT)&&(COMBOTYPE(x+8,y)!=cPIT))
    return;
  if (COMBOTYPE(x+8,y+8)==cPIT) {
    type=cPIT;
  }
 }

 if(int(y)&15)
 {
  if((COMBOTYPE(x,y)!=COMBOTYPE(x,y+8))&&
     (COMBOTYPE(x,y+8)!=cPIT))
    return;
  if (COMBOTYPE(x,y+8)==cPIT) {
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
      sfx(WAV_SECRET);
    }
    return;

  case mfRAFT:
  case mfRAFT_BRANCH:
    if((game.misc&iRAFT) && action!=rafting && action!=swimhit
       && action!=gothit && type==cDOCK)
    {
      if(isRaftFlag(nextflag(x,y,dir)))
      {
        action=rafting;
        sfx(WAV_SECRET);
      }
    }
    return;

  default:
    return;
  }
 }


 putsubscr(framebuf,0,0);
 blit(scrollbuf,framebuf,0,0,0,56,256,168);
 draw(framebuf);
 do_layer2(0, tmpscr, 0, 0);
 do_layer2(1, tmpscr, 0, 0);
 do_layer2(2, tmpscr, 0, 0);
 do_layer2(3, tmpscr, 0, 0);
 do_layer2(-1, tmpscr, 0, 0);
 for(int i=0; i<guys.Count(); i++)
 {
   if(isflier(guys.spr(i)->id)) {
       guys.spr(i)->draw(framebuf);
   }
 }
 do_layer2(4, tmpscr, 0, 0);
 do_layer2(5, tmpscr, 0, 0);
 advanceframe();

 if(type==cCAVE)
 {
//  alert("cave",NULL,NULL,"O&K",NULL,'k',0);
  stop_midi();
  walkdown();
 } else if(type==cCAVE2)
 {
//  alert("cave2",NULL,NULL,"O&K",NULL,'k',0);
  stop_midi();
  walkup2();
 }

 if (type==cPIT) {
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

  do {
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
   wtype = wtWHISTLE;
   int level;
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

 switch(wtype)
 {
 case wtCAVE: // cave/item room
  ALLOFF();
  homescr=currscr;
  currscr=0x80;
  if(dlevel==0) {  // cave
    stop_midi();
    kill_sfx();
    if(tmpscr->room==rWARP)
      currscr=0x81;
    loadlvlpal(10);
    blackscr(30,(COOLSCROLL&&(combobuf[MAPCOMBO(x,y-16)].type==cCAVE))?false:true);
    loadscr(0,currscr,up);
    loadscr(1,homescr,up);
    putscr(scrollbuf,0,0,tmpscr);
    dir=up;
    x=112;
    y=160;
    if (didpit) {
      didpit=false;
      x=pitx;
      y=pity;
    }
    reset_hookshot();
    stepforward(6); 
    }
  else { // item room
    stop_sfx(WAV_ROAR);
    stop_sfx(WAV_VADER);
    stop_sfx(WAV_DODONGO);
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    fade(DMaps[currdmap].color,true,false,false);
    blackscr(30,true);
    loadscr(0,currscr,down);
    loadscr(1,homescr,-1);
    putscr(scrollbuf,0,0,tmpscr);
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer2(0, tmpscr, 0, 0);
    do_layer2(1, tmpscr, 0, 0);
    do_layer2(2, tmpscr, 0, 0);
    do_layer2(3, tmpscr, 0, 0);
    do_layer2(-1, tmpscr, 0, 0);
    for(int i=0; i<guys.Count(); i++)
    {
      if(isflier(guys.spr(i)->id)) {
          guys.spr(i)->draw(framebuf);
      }
    }
    do_layer2(4, tmpscr, 0, 0);
    do_layer2(5, tmpscr, 0, 0);
    fade(11,true,true,false);
    dir=down;
    x=48;
    y=0;
    if (didpit) {
      didpit=false;
      x=pitx;
      y=pity;
    }
    reset_hookshot();
    stepforward(18);
    }
  break;

 case wtPASS: { // passageway
  stop_sfx(WAV_ROAR);
  stop_sfx(WAV_VADER);
  stop_sfx(WAV_DODONGO);
  ALLOFF();
  homescr=currscr;
  currscr=0x81;
  byte warpscr = wscr + DMaps[currdmap].xoff;
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  fade(DMaps[currdmap].color,true,false,false);
  blackscr(30,true);
  loadscr(0,currscr,down);
  loadscr(1,homescr,-1);
  putscr(scrollbuf,0,0,tmpscr);
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  do_layer2(0, tmpscr, 0, 0);
  do_layer2(1, tmpscr, 0, 0);
  do_layer2(2, tmpscr, 0, 0);
  do_layer2(3, tmpscr, 0, 0);
  do_layer2(-1, tmpscr, 0, 0);
  for(int i=0; i<guys.Count(); i++)
  {
    if(isflier(guys.spr(i)->id)) {
        guys.spr(i)->draw(framebuf);
    }
  }
  do_layer2(4, tmpscr, 0, 0);
  do_layer2(5, tmpscr, 0, 0);
  fade(11,true,true,false);
  dir=down;
  x=48;
  if( (homescr&15) > (warpscr&15) )
    x=192;
  if( (homescr&15) == (warpscr&15) )
    if( (currscr>>4) > (warpscr>>4) )
      x=192;
  if (didpit) {
    didpit=false;
    x=pitx;
    y=pity;
  }
  warpx=x;
  y=0;
  reset_hookshot();
  stepforward(18);
  newscr_clk=frame;
  } break;

 case wtEXIT: // entrance/exit
  ALLOFF();
  stop_midi();
  kill_sfx();
  blackscr(30,false);
  currdmap = wdmap;
  dlevel=DMaps[currdmap].level;
  currmap=DMaps[currdmap].map;
  loadfullpal();
  ringcolor();
  loadlvlpal(DMaps[currdmap].color);
  lastentrance = homescr = currscr = wscr + DMaps[currdmap].xoff;
  loadscr(0,currscr,-1);
  if(dlevel) {
    x=tmpscr->warpx2;
    y=tmpscr->warpy2;
    }
  else {
    x=tmpscr->warpx;
    y=tmpscr->warpy;
    }
  if (didpit) {
    didpit=false;
    x=pitx;
    y=pity;
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

    if(combobuf[MAPCOMBO(x,y-16)].type==cCAVE)
    {
      reset_pal_cycling();
      putscr(scrollbuf,0,0,tmpscr);
      walkup();
    } else if(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2)
    {
      reset_pal_cycling();
      putscr(scrollbuf,0,0,tmpscr);
      walkdown2();
    }
    else if(COOLSCROLL)
      openscreen();
  }
  play_DmapMidi();
  currcset=DMaps[currdmap].color;
  dointro();
  warpx=x;
  warpy=y;
  for(int i=0; i<6; i++)
    visited[i]=-1;
  break;

 case wtSCROLL: { // scrolling warp
  int c = DMaps[currdmap].color;
  currmap = DMaps[wdmap].map;
  lighting(4);
  scrollscr(dir, wscr+DMaps[wdmap].xoff, wdmap);
  reset_hookshot();
  if (currdmap!=wdmap) {
    lastentrance = homescr = currscr = wscr + DMaps[currdmap].xoff;
  }
  currdmap = wdmap;
  dlevel = DMaps[currdmap].level;
  if(DMaps[currdmap].color != c)
    loadlvlpal(DMaps[currdmap].color);
  play_DmapMidi();
  currcset=DMaps[currdmap].color;
  dointro();
  } break;

 case wtWHISTLE: { // whistle warp
  currmap = DMaps[wdmap].map;
  scrollscr(right, wscr+DMaps[wdmap].xoff, wdmap);
  reset_hookshot();
  currdmap=wdmap;
  dlevel=DMaps[currdmap].level;
  loadlvlpal(DMaps[currdmap].color);
  play_DmapMidi();
  currcset=DMaps[currdmap].color;
  dointro();
  action=inwind;
  Lwpns.add(new weapon(0,tmpscr->warpy2,wWind,1,0,right));
  whirlwind=255;
  } break;

 case wtIWARP:
 case wtIWARPBLK:
 case wtIWARPOPEN:
 case wtIWARPZAP: { // insta-warps
  ALLOFF();
  kill_sfx();
  if(wtype==wtIWARPZAP)
    zapout();
  else if(wtype!=wtIWARP)
    blackscr(30,(COOLSCROLL&&(combobuf[MAPCOMBO(x,y-16)].type==cCAVE))?false:true);

  int c = DMaps[currdmap].color;
  currdmap = wdmap;
  dlevel = DMaps[currdmap].level;
  currmap = DMaps[currdmap].map;

  ringcolor();
  if(DMaps[currdmap].color != c)
    loadlvlpal(DMaps[currdmap].color);

  homescr = currscr = wscr + DMaps[currdmap].xoff;
  loadscr(0,currscr,-1);
  reset_hookshot();
  putscr(scrollbuf,0,0,tmpscr);
  x = tmpscr->warpx;
  y = tmpscr->warpy;
  if (didpit) {
    didpit=false;
    x=pitx;
    y=pity;
  }
  dir=down;
  if(x==0)   dir=right;
  if(x==240) dir=left;
  if(y==0)   dir=down;
  if(y==160) dir=up;
  markBmap(dir^1);
  if(wtype==wtIWARPZAP)
    zapin();
  if(combobuf[MAPCOMBO(x,y-16)].type==cCAVE)
  {
    reset_pal_cycling();
    putscr(scrollbuf,0,0,tmpscr);
    walkup();
  } else if(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2)
  {
    reset_pal_cycling();
    putscr(scrollbuf,0,0,tmpscr);
    walkdown2();
  }
  else
  {
    if(wtype==wtIWARPOPEN)
      openscreen();
  }
  play_DmapMidi();
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

 if(action!=rafting && iswater(MAPCOMBO(x,y+8))
   && (game.misc2&iFLIPPERS) && (action!=inwind))
 {
   hopclk=0xFF;
   action=swimming;
 }
 newscr_clk=frame;
 eat_buttons();
 attackclk=0;
 didstuff=0;
 map_bkgsfx();
 loadside=dir^1;
 whistleclk=-1;
 if ((DMaps[currdmap].type&dmfCONTINUE) ||
     (currdmap==0)){
   game.continue_dmap=currdmap;
 };
 return true;
}

bool LinkClass::check_cheat_warp()
{
  return false;
}



void LinkClass::exitcave()
{
 currscr=homescr;
 loadscr(0,currscr,255);  // bogus direction
 x = tmpscr->warpx;
 y = tmpscr->warpy;
 if (didpit) {
   didpit=false;
   x=pitx;
   y=pity;
 }
 if(x+y == 0)
   x = y = 80;
 bool b = (combobuf[MAPCOMBO(x,y-16)].type==cCAVE) && COOLSCROLL;
 blackscr(30,b?false:true);
 ringcolor();
 loadlvlpal(DMaps[currdmap].color);
 lighting(2);
 stop_midi();
 kill_sfx();
 ALLOFF();
 putscr(scrollbuf,0,0,tmpscr);
 if(combobuf[MAPCOMBO(x,y-16)].type==cCAVE)
   walkup();
 if(combobuf[MAPCOMBO(x,y+16)].type==cCAVE2)
   walkdown2();
 play_DmapMidi();
 currcset=DMaps[currdmap].color;
 dointro();
 newscr_clk=frame;
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
 while(steps>0) {
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  draw(framebuf);
  move(dir);
  do_layer2(0, tmpscr, 0, 0);
  do_layer2(1, tmpscr, 0, 0);
  do_layer2(2, tmpscr, 0, 0);
  do_layer2(3, tmpscr, 0, 0);
  do_layer2(-1, tmpscr, 0, 0);
  for(int i=0; i<guys.Count(); i++)
  {
    if(isflier(guys.spr(i)->id)) {
        guys.spr(i)->draw(framebuf);
    }
  }
  do_layer2(4, tmpscr, 0, 0);
  do_layer2(5, tmpscr, 0, 0);
  advanceframe();
  if(Quit)
    return;
  steps--;
  }
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  do_layer2(0, tmpscr, 0, 0);
  do_layer2(1, tmpscr, 0, 0);
  do_layer2(2, tmpscr, 0, 0);
  do_layer2(3, tmpscr, 0, 0);
  do_layer2(-1, tmpscr, 0, 0);
  for(int i=0; i<guys.Count(); i++)
  {
    if(isflier(guys.spr(i)->id)) {
        guys.spr(i)->draw(framebuf);
    }
  }
  do_layer2(4, tmpscr, 0, 0);
  do_layer2(5, tmpscr, 0, 0);
  eat_buttons();
}



void LinkClass::walkdown()
{
  if(COOLSCROLL)
    close_black_circle();
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  clk=0;
  int cmby=(int(y)&0xF0)+16;
  for(int i=0; i<66; i++)
  {
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    draw(framebuf);
    overcombo(framebuf,int(x)&0xF0,cmby-16+56,MAPDATA(x,cmby-16),tmpscr->cpage);
    putcombo(framebuf,int(x)&0xF0,cmby+56,MAPDATA(x,cmby),tmpscr->cpage);
    if(int(x)&15)
    {
      overcombo(framebuf,int(x+16)&0xF0,cmby-16+56,MAPDATA(x+16,cmby-16),tmpscr->cpage);
      putcombo(framebuf,int(x+16)&0xF0,cmby+56,MAPDATA(x+16,cmby),tmpscr->cpage);
    }
    linkstep();
    if((i&3)==3)
      y++;
    do_layer2(0, tmpscr, 0, 0);
    do_layer2(1, tmpscr, 0, 0);
    do_layer2(2, tmpscr, 0, 0);
    do_layer2(3, tmpscr, 0, 0);
    do_layer2(-1, tmpscr, 0, 0);
    for(int i=0; i<guys.Count(); i++)
    {
      if(isflier(guys.spr(i)->id)) {
          guys.spr(i)->draw(framebuf);
      }
    }
    do_layer2(4, tmpscr, 0, 0);
    do_layer2(5, tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
}

void LinkClass::walkdown2()
{
  if(COOLSCROLL)
    open_black_circle();
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  clk=0;
  int cmby=int(y)&0xF0;
  for(int i=0; i<66; i++)
  {
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    draw(framebuf);
    overcombo(framebuf,int(x)&0xF0,cmby+16+56,MAPDATA(x,cmby+16),tmpscr->cpage);
    putcombo(framebuf,int(x)&0xF0,cmby+56,MAPDATA(x,cmby),tmpscr->cpage);
    if(int(x)&15)
    {
      overcombo(framebuf,int(x+16)&0xF0,cmby+16+56,MAPDATA(x+16,cmby+16),tmpscr->cpage);
      putcombo(framebuf,int(x+16)&0xF0,cmby+56,MAPDATA(x+16,cmby),tmpscr->cpage);
    }
    linkstep();
    if((i&3)==3)
      y++;
    do_layer2(0, tmpscr, 0, 0);
    do_layer2(1, tmpscr, 0, 0);
    do_layer2(2, tmpscr, 0, 0);
    do_layer2(3, tmpscr, 0, 0);
    do_layer2(-1, tmpscr, 0, 0);
    for(int i=0; i<guys.Count(); i++)
    {
      if(isflier(guys.spr(i)->id)) {
          guys.spr(i)->draw(framebuf);
      }
    }
    do_layer2(4, tmpscr, 0, 0);
    do_layer2(5, tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
}


void LinkClass::walkup()
{
  if(COOLSCROLL)
    open_black_circle();
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  dir=down;
  clk=0;
  int cmby=int(y)&0xF0;
  for(int i=0; i<64; i++)
  {
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    draw(framebuf);
    overcombo(framebuf,int(x)&0xF0,cmby-16+56,MAPDATA(x,cmby-16),tmpscr->cpage);
    putcombo(framebuf,int(x)&0xF0,cmby+56,MAPDATA(x,cmby),tmpscr->cpage);
    if(int(x)&15)
    {
      overcombo(framebuf,int(x+16)&0xF0,cmby-16+56,MAPDATA(x+16,cmby-16),tmpscr->cpage);
      putcombo(framebuf,int(x+16)&0xF0,cmby+56,MAPDATA(x+16,cmby),tmpscr->cpage);
    }
    linkstep();
    if((i&3)==0)
      y--;
    do_layer2(0, tmpscr, 0, 0);
    do_layer2(1, tmpscr, 0, 0);
    do_layer2(2, tmpscr, 0, 0);
    do_layer2(3, tmpscr, 0, 0);
    do_layer2(-1, tmpscr, 0, 0);
    for(int i=0; i<guys.Count(); i++)
    {
      if(isflier(guys.spr(i)->id)) {
          guys.spr(i)->draw(framebuf);
      }
    }
    do_layer2(4, tmpscr, 0, 0);
    do_layer2(5, tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
  map_bkgsfx();
  loadside=dir^1;
}

void LinkClass::walkup2()
{
  if(COOLSCROLL)
    close_black_circle();
  hclk=0;
  stop_sfx(WAV_BRANG);
  sfx(WAV_STAIRS,pan(int(x)));
  dir=up;
  clk=0;
  int cmby=int(y)&0xF0;
  for(int i=0; i<64; i++)
  {
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    draw(framebuf);
    overcombo(framebuf,int(x)&0xF0,cmby+56,MAPDATA(x,cmby),tmpscr->cpage);
    putcombo(framebuf,int(x)&0xF0,cmby-16+56,MAPDATA(x,cmby-16),tmpscr->cpage);
    if(int(x)&15)
    {
      overcombo(framebuf,int(x+16)&0xF0,cmby+56,MAPDATA(x+16,cmby),tmpscr->cpage);
      putcombo(framebuf,int(x+16)&0xF0,cmby-16+56,MAPDATA(x+16,cmby-16),tmpscr->cpage);
    }
    linkstep();
    if((i&3)==0)
      y--;
    do_layer2(0, tmpscr, 0, 0);
    do_layer2(1, tmpscr, 0, 0);
    do_layer2(2, tmpscr, 0, 0);
    do_layer2(3, tmpscr, 0, 0);
    do_layer2(-1, tmpscr, 0, 0);
    for(int i=0; i<guys.Count(); i++)
    {
      if(isflier(guys.spr(i)->id)) {
          guys.spr(i)->draw(framebuf);
      }
    }
    do_layer2(4, tmpscr, 0, 0);
    do_layer2(5, tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      break;
  }
  map_bkgsfx();
  loadside=dir^1;
}


void LinkClass::stepout()
{
 ALLOFF();
 putsubscr(framebuf,0,0);
 blit(scrollbuf,framebuf,0,0,0,56,256,168);
 fade(11,true,false,false);
 blackscr(30,true);
 ringcolor();
 if(currscr==129 && x!=warpx)
 {
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
 do_layer2(0, tmpscr, 0, 0);
 do_layer2(1, tmpscr, 0, 0);
 do_layer2(2, tmpscr, 0, 0);
 do_layer2(3, tmpscr, 0, 0);
 do_layer2(-1, tmpscr, 0, 0);
 for(int i=0; i<guys.Count(); i++)
 {
   if(isflier(guys.spr(i)->id)) {
       guys.spr(i)->draw(framebuf);
   }
 }
 do_layer2(4, tmpscr, 0, 0);
 do_layer2(5, tmpscr, 0, 0);
 fade(DMaps[currdmap].color,true,true,false);
 x = tmpscr->warpx;
 y = tmpscr->warpy;
 if (didpit) {
   didpit=false;
   x=pitx;
   y=pity;
 }
 if(x+y == 0)
   x = y = 80;
 dir=down;
 newscr_clk=frame;
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

  newcombo c = combobuf[ (TheMaps[ns].data[cmb]&0xFF) + (TheMaps[ns].cpage<<8) ];
  bool dried = iswater_type(c.type) && (whistleclk>=88);
  bool swim = iswater_type(c.type) && (game.misc2&iFLIPPERS);
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
    c = combobuf[ (TheMaps[ns].data[++cmb]&0xFF) + (TheMaps[ns].cpage<<8) ];
    dried = iswater_type(c.type) && (whistleclk>=88);
    swim = iswater_type(c.type) && (game.misc2&iFLIPPERS);
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
 sfx(WAV_SECRET);
 return true;
}



int LinkClass::lookahead(int destscr = -1)
{ // Helper for scrollscr that gets next combo on next screen.
  // Can use destscr for scrolling warps,
  // but assumes currmap is correct.

  int s = currscr;
  int cx = x;
  int cy = y + 8;

  switch(dir)
  {
  case up:    s-=16; cy=160; break;
  case down:  s+=16; cy=0; break;
  case left:  s--; cx=240; break;
  case right: s++; cx=0; break;
  }

  if(destscr != -1)
    s = destscr;

  int combo = (cy&0xF0)+(cx>>4);
  if(combo>175)
    return 0;
  return TheMaps[currmap*MAPSCRS+s].data[combo];  // entire combo code
}

int LinkClass::lookaheadflag(int destscr = -1)
{ // Helper for scrollscr that gets next combo on next screen.
  // Can use destscr for scrolling warps,
  // but assumes currmap is correct.

  int s = currscr;
  int cx = x;
  int cy = y + 8;

  switch(dir)
  {
  case up:    s-=16; cy=160; break;
  case down:  s+=16; cy=0; break;
  case left:  s--; cx=240; break;
  case right: s++; cx=0; break;
  }

  if(destscr != -1)
    s = destscr;

  int combo = (cy&0xF0)+(cx>>4);
  if(combo>175)
    return 0;
  return TheMaps[currmap*MAPSCRS+s].sflag[combo];  // flag
}



void LinkClass::scrollscr(int dir, int destscr = -1, int destdmap = -1)
{
  tmpscr[1] = tmpscr[0];
  mapscr *newscr = &tmpscr[0];
  mapscr *oldscr = &tmpscr[1];
  int sx=0, sy=0, tx=0, ty=0, tx2=0, ty2=0;
  int cx=0, step = (isdungeon() && !get_bit(QHeader.rules,qr1_FASTDNGN)) ? 2 : 4;
  int scx = get_bit(QHeader.rules,qr1_FASTDNGN) ? 30 : 0;

  for(int i=0; i<3; i++)
    lastdir[i]=lastdir[i+1];
  lastdir[3] = oldscr->flags&fMAZE ? dir : -1;

  actiontype lastaction = action;
  ALLOFF();

  int ahead = lookahead(destscr);
  int aheadflag = lookaheadflag(destscr);

  if (lastaction!=inwind) {
  if(lastaction==rafting && aheadflag==mfRAFT)
  {
    action=rafting;
  }
  else if(iswater(ahead) && (game.misc2&iFLIPPERS))
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
  do {
/*
    system_pal();
    jwin_alert("moving",NULL,NULL,NULL,"OK",NULL,13,27);
    game_pal();
*/
    putsubscr(framebuf,0,0);
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    do_layer2(0, tmpscr, 0, 0);
    do_layer2(1, tmpscr, 0, 0);
    if(isdungeon()==0)
    {
      draw_under(framebuf);
      draw(framebuf);
    }
    if(cx==scx)
      rehydratelake();
    do_layer2(2, tmpscr, 0, 0);
    do_layer2(3, tmpscr, 0, 0);
    do_layer2(-1, tmpscr, 0, 0);
    for(int i=0; i<guys.Count(); i++)
    {
      if(isflier(guys.spr(i)->id)) {
          guys.spr(i)->draw(framebuf);
      }
    }
    do_layer2(4, tmpscr, 0, 0);
    do_layer2(5, tmpscr, 0, 0);
    advanceframe();
    if(Quit)
      return;
    cx++;
    } while(cx<32);

  if((DMaps[currdmap].type&dmfTYPE)==dmCAVE)
    markBmap(dir);

  switch(dir) {
   case up:
    if (fixed_door) {
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
    if (fixed_door) {
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
    if (fixed_door) {
      unsetmapflag(mSECRET);
    }
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
    if (fixed_door) {
      unsetmapflag(mSECRET);
    }
    if(destscr!=-1)
      currscr=destscr;
    else if(checkmaze(oldscr))
      currscr++;
    loadscr(0,currscr,dir);
    putscr(scrollbuf,256,0,newscr);
    cx=256/step;
    break;
   }


  fixed_door=false;
  lighting(2);
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
//    putsubscr(framebuf,0,0);
    if (ladderx+laddery) {
       if(ladderdir==up) {
         ladderx = int(x);
         laddery = int(y); }
       else {
         ladderx = int(x);
         laddery = int(y); }
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
    if (dir==right) {
      tx-=256;
    }
    ty=sy;
    if (dir==down) {
      ty-=176;
    }
    tx2=sx;
    if (dir==left) {
      tx2-=256;
    }
    ty2=sy;
    if (dir==up) {
      ty2-=176;
    }

    do_layer2(0, oldscr, tx2, ty2);
    do_layer2(1, oldscr, tx2, ty2);
    do_layer2(0, newscr, tx, ty);
    do_layer2(1, newscr, tx, ty);
    linkstep();
    if(isdungeon()==0)
    {
      draw_under(framebuf);
      draw(framebuf);
    }
  do_layer2(2, oldscr, tx2, ty2);
  do_layer2(3, oldscr, tx2, ty2);
  do_layer2(-1, oldscr, tx2, ty2);
  do_layer2(4, oldscr, tx2, ty2);
  do_layer2(5, oldscr, tx2, ty2);

  do_layer2(2, newscr, tx, ty);
  do_layer2(3, newscr, tx, ty);
  do_layer2(-1, newscr, tx, ty);
  do_layer2(4, newscr, tx, ty);
  do_layer2(5, newscr, tx, ty);

  putsubscr(framebuf,0,0);

    advanceframe();
    if(Quit)
      return;
    cx--;
  }

  if(destdmap != -1)
    currdmap = destdmap;

  lighting(3);
  homescr=currscr;
  putscr(scrollbuf,0,0,newscr);

  if(MAPFLAG(x,y)==mfRAFT && action!=rafting && hopclk==0)
  {
    sfx(WAV_SECRET);
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
    sfx(WAV_SECRET);

  newscr_clk = frame;
  loadside = dir^1;

}




/************************************/
/********  More Items Code  *********/
/************************************/


int Bweapon(int pos)
{
  switch(pos)
  {
  case 0: if(game.brang==3) return iFBrang;
          if(game.brang) return game.brang-1+iBrang; break;
  case 1: if(game.bombs) return iBombs; break;
  case 2: if((game.misc&iBOW) && game.arrow){
            if (game.arrow<3) {
              return game.arrow-1+iArrow;
            } else {
              return iGArrow;
            }
          } break;
  case 3: if(game.candle) return game.candle-1+iBCandle; break;
  case 4: if(game.whistle) return iWhistle; break;
  case 5: if(game.bait) return iBait; break;
  case 6: if(game.potion) return game.potion-1+iBPotion;
          if(game.letter) return iLetter; break;
  case 7: if(game.wand) return iWand; break;
  case 8: if(game.misc2&iHOOKSHOT) return iHookshot; break;
  case 9: if(game.sbombs) return iSBomb; break;
  case 10: if(game.misc2&iLENS) return iLens; break;
  case 11: if(game.misc2&iHAMMER) return iHammer; break;
  }

  return 0;
}


void selectAwpn(int step)
{
  // change this for selectable Awpn
  switch(game.sword)
  {
  case 1:
  case 2:
  case 3:
    Awpn = game.sword - 1 + iSword; break;
  case 4:
    Awpn = iXSword; break;
  default:
    Awpn = 0;
  }
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

 int pos = Bpos;
 int cnt = NEWSUBSCR ? 12 : 8;

 do {
   Bpos += step;

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
 switch(tmpscr[tmp].room) {
  case rINFO:  // pay for info
    if(game.rupies < abs(prices[index-1]))
      return;
    game.drupy -= abs(prices[index-1]);
    msgstr = QMisc.info[tmpscr[tmp].catchall].str[index-1];
    msgclk=msgpos=0;
    rectfill(msgdisplaybuf, 0, 0, msgdisplaybuf->w, 80, 0);
/*
    for(int i=0; i<80; i++)
      putcombo(scrollbuf,((i&15)<<4),(i&0xF0),tmpscr->data[i],tmpscr->cpage);
*/
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
    clear(msgdisplaybuf);
//    putscr(scrollbuf,0,0,tmpscr);
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
      game.life = max(game.life-HP_PER_HEART,0);
      game.maxlife = max(game.maxlife-HP_PER_HEART,(3*HP_PER_HEART));
    }
    setmapflag();
    ((item*)items.spr(0))->pickup=ipDUMMY+ipFADE;
    ((item*)items.spr(1))->pickup=ipDUMMY+ipFADE;
    fadeclk=66;
    msgstr=0;
    clear(msgdisplaybuf);
//    putscr(scrollbuf,0,0,tmpscr);
    break;
  }
}


void getitem(int id)
{
  switch(id)
  {
  case iRupy:       game.drupy++;   break;
  case i5Rupies:    game.drupy+=5;  break;
  case i20Rupies:   game.drupy+=20; break;
  case i50Rupies:   game.drupy+=50; break;
  case i200Rupies:  game.drupy+=200; break;
  case iWallet500:  game.wallet=max(game.wallet,1); break;
  case iWallet999:  game.wallet=max(game.wallet,2); break;
  case iBombs:      game.bombs=min(game.bombs+4,game.maxbombs); break;
  case iSBomb:      game.sbombs=min(game.sbombs+1,game.maxbombs>>2); break;
  case iClock:      {
                      setClock(watch=true);
                      clock_zoras=0;
                      if (get_bit(QHeader.rules3,qr3_TEMPCLOCKS)) {
                          clockclk=0;
                      }
                    } break;
  case iSword:      game.sword=max(game.sword,1); break;
  case iWSword:     game.sword=max(game.sword,2); break;
  case iMSword:     game.sword=max(game.sword,3); break;
  case iXSword:     game.sword=max(game.sword,4); break;
  case iKey:        if(game.keys<255) game.keys++; break;
  case iBCandle:    game.candle=max(game.candle,1); break;
  case iRCandle:    game.candle=2; break;
  case iArrow:      game.arrow=max(game.arrow,1); break;
  case iSArrow:     game.arrow=max(game.arrow,2); break;
  case iGArrow:     game.arrow=max(game.arrow,3); break;
  case iBRing:      game.ring=max(game.ring,1); if(currscr<128 || dlevel) ringcolor(); break;
  case iRRing:      game.ring=max(game.ring,2); if(currscr<128 || dlevel) ringcolor(); break;
  case iGRing:      game.ring=max(game.ring,3); if(currscr<128 || dlevel) ringcolor(); break;
  case iBrang:      game.brang=max(game.brang,1); break;
  case iMBrang:     game.brang=max(game.brang,2); break;
  case iFBrang:     game.brang=max(game.brang,3); break;
  case iBPotion:    game.potion=min(game.potion+1,2); break;
  case iRPotion:    game.potion=2; break;
  case iBracelet:   game.misc|=iBRACELET; break;
  case iRaft:       game.misc|=iRAFT; break;
  case iLadder:     game.misc|=iLADDER; break;
  case iBow:        game.misc|=iBOW; break;
  case iBook:       game.misc|=iBOOK; break;
  case iShield:     game.misc|=iSHIELD; break;
  case iMShield:    game.misc|=iMSHIELD; break;
  case iMKey:       game.misc|=iMKEY; break;
  case iMap:        game.lvlitems[dlevel]|=liMAP; break;
  case iCompass:    game.lvlitems[dlevel]|=liCOMPASS; break;
  case iLetter:     game.letter=1; break;
  case iBait:       game.bait=1; break;
  case iWand:       game.wand=1; break;
  case iWhistle:    game.whistle=1; break;
  case iFairy:      game.life=min(game.life+(3*HP_PER_HEART),game.maxlife); break;
  case iCross:      game.misc2|=iCROSS; break;
  case iFlippers:   game.misc2|=iFLIPPERS; break;
  case iBoots:      game.misc2|=iBOOTS; break;
  case iL2Bracelet: game.misc2|=iL2BRACELET; break;
  case iHookshot:   game.misc2|=iHOOKSHOT; break;
  case iLens:       game.misc2|=iLENS; break;
  case iHammer:     game.misc2|=iHAMMER; break;

  case iMagicC:
    if(game.maxmagic < MAGICPERBLOCK*8)
       game.maxmagic+=MAGICPERBLOCK;

  case iSMagic:     game.dmagic+=MAGICPERBLOCK; break;
  case iLMagic:     game.dmagic=MAGICPERBLOCK*8; break;

  case iHCPiece:
    if(++game.HCpieces<4)
      break;
    game.HCpieces = 0;
    // fall through
  case iHeartC:
    if(game.maxlife < (get_bit(QHeader.rules,qr1_24HC) ? 24*HP_PER_HEART : 16*HP_PER_HEART))
       game.maxlife+=HP_PER_HEART;
    // fall through
  case iHeart:     game.life=min(game.life+HP_PER_HEART,game.maxlife); break;
  }

  selectBwpn(0);
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
    sfx(WAV_SCALE1);
  }
}


void LinkClass::checkitems()
{
 int tmp=currscr>=128?1:0;
 int index=items.hit(x,y+8,1,1);
 if(index==-1)
   return;

 if (tmpscr[tmp].room==rSHOP && boughtsomething==true)
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
      boughtsomething=true;
      break;
    }


 if(pickup&ipONETIME)  // set screen item flag for one-time-only items
  setmapflag();

 getitem(id);

 if(pickup&ipHOLDUP)
 {
   if(msgstr) {
     msgstr=0;
     clear(msgdisplaybuf);
//     putscr(scrollbuf,0,0,tmpscr);
     }

   fadeclk=66;

   if(id!=iBombs || action==swimming)
   { // don't hold up bombs unless swimming
     if(action==swimming)
       action=swimhold1;
     else
       action=holding1;

     if(((item*)items.spr(index))->twohand)
     {
       if(action==swimhold1)
         action=swimhold2;
       else
         action=holding2;
     }

     holdclk=130;
     holditem=id;
     freeze_guys=true;
   }

   if(id!=iTriforce)
     sfx(WAV_PICKUP);
   items.del(index);

   // clear up shop stuff
   if(isdungeon()==0 && ((item*)items.spr(0))->pickup&ipDUMMY)
   {
     items.del(0);
     clear(msgdisplaybuf);
//     putscr(scrollbuf,0,0,tmpscr);
   }
 }
 else
 {
   items.del(index);
 }

 if(id==iTriforce)
   getTriforce();
 if(id==iBigTri)
   getBigTri();
}

void LinkClass::StartRefill(int refill_why)
{
 if(!refilling) {
   refillclk=21;
   stop_sfx(WAV_ER);
   sfx(WAV_REFILL,128,true);
   refilling=true;
   if ((refill_why==REFILL_POTION)&&(!get_bit(QHeader.rules4,qr4_NONBUBBLEMEDICINE)))
     swordclk=0;
   if ((refill_why==REFILL_FAIRY)&&(!get_bit(QHeader.rules4,qr4_NONBUBBLEFAIRIES)))
     swordclk=0;
   if ((refill_why==REFILL_TRIFORCE)&&(!get_bit(QHeader.rules4,qr4_NONBUBBLETRIFORCE)))
     swordclk=0;
   }
}

bool LinkClass::refill()
{
 if(!refilling)
   return false;
 refillclk++;
 int speed = get_bit(QHeader.rules,qr1_FASTFILL) ? 6 : 22;
 if(refillclk%speed == 0)
 {
//   game.life&=0xFFC;
   switch (refill_what) {
     case REFILL_LIFE:
       game.life+=HP_PER_HEART/2;
       if(game.life>=game.maxlife)
       {
         game.life=game.maxlife;
         kill_sfx();
         sfx(WAV_MSG);
         refilling=false;
         return false;
       } break;
     case REFILL_MAGIC:
       game.magic+=MAGICPERBLOCK/16;
       if(game.magic>=game.maxmagic)
       {
         game.magic=game.maxmagic;
         kill_sfx();
         sfx(WAV_MSG);
         refilling=false;
         return false;
       } break;
     case REFILL_ALL:
       game.life+=HP_PER_HEART/2;
       game.magic+=MAGICPERBLOCK/16;
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


void LinkClass::getTriforce()
{
 PALETTE flash_pal;
 for(int i=0; i<256; i++)
   flash_pal[i] = get_bit(QHeader.rules,qr1_FADE) ? _RGB(63,63,0) : _RGB(63,63,63);

 sfx(WAV_SCALE1);
 jukebox(MUSIC_TRIFORCE);
 game.lvlitems[dlevel]|=liTRIFORCE;

 int f=0;
 int x=0;
 int c=0;
 do {
  if(f==40)
  {
    ALLOFF();
    action=holding2; // have to reset this flag
  }
  if(f>=40 && f<88)
  {
    if(get_bit(QHeader.rules,qr1_FADE))
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

  if(f==88)
  {
    refill_what=REFILL_ALL;
    StartRefill(REFILL_TRIFORCE);
    refill();
  }

  if(f==89)
    if(refill())
      f--;

  if(f>=208 && f<288)
  {
    x++;
    switch(++c)
    {
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

  do_layer2(0, tmpscr, 0, 0);
  do_layer2(1, tmpscr, 0, 0);
  do_layer2(2, tmpscr, 0, 0);
  do_layer2(3, tmpscr, 0, 0);
  do_layer2(-1, tmpscr, 0, 0);
  for(int i=0; i<guys.Count(); i++)
  {
    if(isflier(guys.spr(i)->id)) {
        guys.spr(i)->draw(framebuf);
    }
  }
  do_layer2(4, tmpscr, 0, 0);
  do_layer2(5, tmpscr, 0, 0);

  advanceframe();
  f++;
 } while(f<408);

 action=none;
 dowarp(1);
}



void setup_red_screen()
{
  // color scale the game screen
  for(int y=0; y<168; y++)
    for(int x=0; x<256; x++)
    {
      int c = scrollbuf->line[y][x];
      int r = min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
      scrollbuf->line[y][x] = r+CSET(2);
    }

  // set up the new palette
  for(int i=CSET(2); i<CSET(4); i++)
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

 Playing=Paused=false;
 game.deaths=min(game.deaths+1,999);
 dir=down;
 stop_midi();
 kill_sfx();
 sfx(WAV_OUCH,pan(int(x)));
 hclk=48;
 attackclk=0;

 do {
   if(hclk)
     hclk--;

   if(f<32)
     cs=(((32-f)>>1)&3)+6;

   if(f==64)
   {
     if(get_bit(QHeader.rules,qr1_FADE))
       setup_red_screen();
     else
     {
       for(int i=0; i<176; i++)
       {
         tmpscr->data[i] &= 0x00FF;
         tmpscr->data[i] |= 0x0200;
       }
       putscr(scrollbuf,0,0,tmpscr);
       for(int i=1; i<16; i+=3)
       {
         RAMpal[CSET(2)+i]   = NESpal(0x17);
         RAMpal[CSET(2)+i+1] = NESpal(0x16);
         RAMpal[CSET(2)+i+2] = NESpal(0x26);
       }
       refreshpal=true;
     }
     sfx(WAV_SPIRAL);
   }
   if(f>64 && f<=144)
   {
     switch((f-64)%20) {
     case 0:  dir=down;  break;
     case 5:  dir=right; break;
     case 10: dir=up;    break;
     case 15: dir=left;  break;
     }
     linkstep();
   }

   if(get_bit(QHeader.rules,qr1_FADE))
   {
     if(f>=144 && f<=208)
     {
       fade_interpolate(RAMpal,black_palette,RAMpal,f-144,CSET(2),CSET(4)-1);
       refreshpal = true;
     }
     if(f==174)
     {
       for(int i=CSET(6); i<CSET(7); i++)
       {
         int g = (RAMpal[i].r + RAMpal[i].g + RAMpal[i].b)/3;
         RAMpal[i] = _RGB(g,g,g);
       }
     }
   }
   else
   {
     if(f==144)
       slide_in_color(0x06);
     if(f==154)
       slide_in_color(0x07);
     if(f==164)
       slide_in_color(0x0F);
     if(f==174)
     {
       slide_in_color(0x0F);
       slide_in_color(0x0F);
       for(int i=1; i<16; i+=3)
       {
         RAMpal[CSET(6)+i]   = NESpal(0x10);
         RAMpal[CSET(6)+i+1] = NESpal(0x30);
         RAMpal[CSET(6)+i+2] = NESpal(0x00);
       }
     }
   }

   if(f==204)
     sfx(WAV_MSG);

   putsubscr(framebuf,0,0);
   blit(scrollbuf,framebuf,0,0,0,56,256,168);

   if(f<204)
     draw(framebuf);
   else
   {
     cs = wpnsbuf[iwDeath].csets&15;
     tile = wpnsbuf[iwDeath].tile;
     if(BSZ)
       tile += (f-204)/3;
     else if(f>=214)
       tile++;
     if(f<=218)
       sprite::draw(framebuf);
   }


   if(f>=266)
   {
     text_mode(-1);
     textout(framebuf,zfont,"GAME OVER",96,136,1);
   }

   if (f<266) {
     do_layer2(0, tmpscr, 0, 0);
     do_layer2(1, tmpscr, 0, 0);
     do_layer2(2, tmpscr, 0, 0);
     do_layer2(3, tmpscr, 0, 0);
     do_layer2(-1, tmpscr, 0, 0);
     for(int i=0; i<guys.Count(); i++)
     {
       if(isflier(guys.spr(i)->id)) {
           guys.spr(i)->draw(framebuf);
       }
     }
     do_layer2(4, tmpscr, 0, 0);
     do_layer2(5, tmpscr, 0, 0);
   }
   advanceframe();
   f++;
 } while(f<=368 && !Quit);
}


void LinkClass::ganon_intro()
{
  loaded_guys=true;
  if(game.lvlitems[dlevel]&liBOSS)
    return;

  dir=down;
  action=holding2;
  holditem=iTriforce;

  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);

  for(int f=0; f<288 && !Quit; f++)
  {
    if(f==48)
    {
      stop_midi();
      stop_sfx(WAV_ROAR);
      sfx(WAV_GASP);
      sfx(WAV_GANON);
      if(game.ring)
        addenemy(160,96,eGANON,0);
      else
        addenemy(80,32,eGANON,0);
      guys.draw(framebuf,false);
      lighting(1);
    }

    draw(framebuf);
    advanceframe();
    if(rSbtn())
    {
      conveyclk=3;
      dosubscr();
      guys.draw(framebuf,false);
    }
  }

  action=none;
  dir=up;
  if(!getmapflag() && (tunes[MAXMUSIC-1].midi))
    jukebox(MAXMUSIC-1);
  else
    play_DmapMidi();
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
    action=none;
    hookshot_frozen=false;
    hookshot_used=false;
    pull_link=false;
    add_chainlink=false;
    del_chainlink=false;
    hs_fix=false;
    for (int i=0; i<chainlinks_count; i++) {
      chainlinks.del(chainlinks.idFirst(wHSChain));
    }
    chainlinks_count=0;
}


void LinkClass::check_conveyor()
{
  if (action==casting) {
    return;
  }
  was_on_conveyor=is_on_conveyor;
  if (conveyclk<=0) {
    int ctype=(combobuf[MAPCOMBO(x+7,y+7)].type);
    if((ctype>=cCVUP) && (ctype<=cCVRIGHT)) {
      is_on_conveyor=true;
      switch (ctype-cCVUP) {
        case up:
         if(!walkflag(x,y+8-2,2,up)) {
            y=y-2;
            hs_starty-=2;
            for (int i=0; i<chainlinks_count; i++) {
              chainlinks.spr(i)->y-=2;
            }
            Lwpns.spr(Lwpns.idFirst(wHookshot))->y-=2;
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->y-=2;
          }
          break;
        case down:
         if(!walkflag(x,y+15+2,2,down)) {
            y=y+2;
            hs_starty+=2;
            for (int i=0; i<chainlinks_count; i++) {
              chainlinks.spr(i)->y+=2;
            }
            Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=2;
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=2;
          }
          break;
        case left:
          if(!walkflag(x-(lsteps[gofast][int(x)&7]),y+8,1,left)) {
            x=x-2;
            hs_startx-=2;
            for (int i=0; i<chainlinks_count; i++) {
              chainlinks.spr(i)->x-=2;
            }
            Lwpns.spr(Lwpns.idFirst(wHookshot))->x-=2;
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->x-=2;
          }
          break;
        case right:
          if(!walkflag(x+15+2,y+8,1,right)) {
            x=x+2;
            hs_startx+=2;
            for (int i=0; i<chainlinks_count; i++) {
              chainlinks.spr(i)->x+=2;
            }
            Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=2;
            Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=2;
          }
          break;
      }
    } else {
      is_on_conveyor=false;
    }
  }
  if (was_on_conveyor && !is_on_conveyor) {
    if (dir<left) {
      x=int(x+7)&0xF8;
    }
    if (dir>down) {
      y=int(y+7)&0xF8;
    }
  }
}

/*** end of link.cc ***/
