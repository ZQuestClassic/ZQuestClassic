/*
  guys.cc
  by Jeremy Craner, 1999
  "Guys" code (and other related stuff) for zelda.cc
*/

bool repaircharge;
int itemindex;


bool hasBoss();

bool can_do_clock()
{
  if(watch || hasBoss()) return false;
  if(items.idFirst(iClock)>=0) return false;
  return true;
}


bool m_walkflag(int x,int y,int special)
{
  int yg = (special==spw_floater)?8:0;

  if(x<16 || y<16-yg || x>=240 || y>=160)
    return true;

  if(dungeon()) {
    if(y<32-yg || y>=144)
      return true;
    if(x<32 || x>=224)
      if(special!=spw_door)  // walk in door way
        return true;
    }

  switch(special) {
  case spw_clipright: if(x>=208) return true; break;
  case spw_floater: return false;
  }

  x&=(special==spw_halfstep)?(~7):(~15);
  y&=(special==spw_halfstep)?(~7):(~15);

  if(special==spw_water)
    return water_walkflag(x,y+8,1) && water_walkflag(x+8,y+8,1);

  return _walkflag(x,y+8,1) && _walkflag(x+8,y+8,1);
}


int link_on_wall()
{
  int lx = link.getX();
  int ly = link.getY();
  if(lx>=48 && lx<=192)
  {
    if(ly==32)  return up+1;
    if(ly==128) return down+1;
  }
  if(ly>=48 && ly<=112)
  {
    if(lx==32)  return left+1;
    if(lx==208) return right+1;
  }
  return 0;
}

inline bool tooclose(int x,int y,int d)
{
  return (abs(int(LinkX())-x)<d && abs(int(LinkY())-y)<d);
}







/**********************************/
/*******  Enemy Base Class  *******/
/**********************************/


class enemy : public sprite {
public:
  int  clk2,clk3,stunclk,hclk,sclk;
  int  hp,dp,wdp,frate,fading,misc;
  fix  step;
  byte movestatus,item_set,grumble;
  bool superman,mainguy,leader,itemguy,bhit,count_enemy,dying,scored;

  enemy(fix X,fix Y,int Id,int Clk) : sprite()
  {
    x=X; y=Y; id=Id; clk=Clk;
    fading=misc=clk2=clk3=stunclk=hclk=sclk=0;
    grumble=movestatus=0;
    item_set=1;
    yofs=54;
    hp=1;
    frate=256;
    dying=leader=superman=itemguy=bhit=scored=false;
    canfreeze=count_enemy=mainguy=true;
    step=0;
    dir=rand()&3;
  }

  virtual ~enemy() {}

  // basic animation code that all derived classes should call
  // the one with an index is the one that is called by
  // the guys sprite list; index is the enemy's index in the list
  virtual bool animate(int index)
  {
    // clk is incremented here
    if(++clk>=frate)
      clk=0;

    // hit and death handling
    if(hclk>0)
      hclk--;
    if(stunclk>0)
      stunclk--;

    if(!dying && hp<=0)
    {
      dying=true;
      if(fading==fade_flash_die)
        clk2=19+18*4;
      else {
        clk2=19;
        if(fading!=fade_blue_poof)
          fading=0;
        }
      if(itemguy)
      {
        hasitem=0;
        item_set=0;
      }
      if(currscr<128 && count_enemy)
        game.guys[(currmap<<7)+currscr]-=1;
    }

    scored=false;

    // returns true when enemy is defeated
    return Dead();
  }

  // to allow for different sfx on defeating enemy
  virtual void death_sfx()
  {
    sfx(WAV_EDEAD,pan(int(x)));
  }
  // supplemental animation code that all derived classes should call
  // as a return value for animate
  // handles the death animation and returns true when enemy is finished
  virtual bool Dead()
  {
    if(dying) {
      clk2--;
      if(clk2==18 && hp>-1000)   // not killed by ringleader
        death_sfx();
      if(clk2==0) {
        if(leader)
          kill_em_all();
        leave_item();
        return true;
        }
      }
    return false;
  }

  virtual void move(fix dx,fix dy)
  {
    if(!watch)
    {
      x+=dx;
      y+=dy;
    }
  }
  virtual void move(fix s)
  {
    if(!watch)
      sprite::move(s);
  }

  void leave_item()
  {
    int i=-1;
    int r=rand()%100;

    switch(item_set)
    {
      case isDEFAULT:
        if(r<3)       i=iFairy;      // 3%
        else if(r<8)  i=i5Rupies;    // 5%
        else if(r<20) i=iHeart;      // 12%
        else if(r<40) i=iRupy;       // 20%
        break;                       // 60%

      case isBOMBS:
        if(r<2)       i=iFairy;      // 2%
        if(r<6)       i=can_do_clock()?iClock:-1;      // 4%
        else if(r<16) i=iRupy;       // 10%
        else if(r<20) i=i5Rupies;    // 4%
        else if(r<30) i=iHeart;      // 10%
        else if(r<42) i=iBombs;      // 12%
        break;                       // 58%

      case isMONEY:
        if(r<3)       i=iFairy;      // 3%
        else if(r<8)  i=can_do_clock()?iClock:-1;      // 5%
        else if(r<23) i=i5Rupies;    // 15%
        else if(r<33) i=iHeart;      // 10%
        else if(r<55) i=iRupy;       // 22%
        break;                       // 45%

      case isLIFE:
        if(r<8)       i=iFairy;      // 8%
        else if(r<16) i=iRupy;       // 8%
        else if(r<48) i=iHeart;      // 32%
        break;                       // 52%

      case isBOMB100: i=iBombs; break;
    }
    if(i!=-1)
      items.add(new item(x,y,i,ipBIGRANGE+ipTIMER,0));
  }

  // auomatically kill off enemy (for rooms with ringleaders)
  virtual void kickbucket()
  {
    if(!superman)
      hp=-1000;  // don't call death_sfx()
  }

  // take damage or ignore it
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || clk<0 || hclk>0 || superman)
      return 0;
    switch(wpnId) {
    case wLitBomb:
    case wBait:
    case wWhistle:
    case wWind:
      return 0;
    case wBrang:
      if(!bhit)
      {
        stunclk=160;
        break;
      }
    default:
      hp-=power;
      hclk=33;
      if((dir&2)==(wpnDir&2))
        sclk=(wpnDir<<8)+16;
    }
    if(wpnId==wBrang && hp<=0)
      fading=fade_blue_poof;
    sfx(WAV_EHIT,pan(int(x)));
    return 1;
  }

  // base drawing function to be used by all derived classes instead of
  // sprite::draw()
  virtual void draw(BITMAP *dest)
  {
    if(fading==fade_invisible || (fading==fade_flicker && (clk&1)))
      return;

    int cshold=cs;
    if(dying) {
      if(clk2>=19) {
        if(!(clk2&2))
          sprite::draw(dest);
        return;
        }
      flip = 0;
      if(clk2>6 && clk2<=12)
        tile=67;
      else tile=66;
      if(fading==fade_blue_poof)
        cs=7;
      else
        cs=(((clk2+5)>>1)&3)+6;
      }
    else if(hclk>0 && (hclk<33 || id==eGANON))
      cs=(((hclk-1)>>1)&3)+6;
    sprite::draw(dest);
    cs=cshold;
  }

  // similar to the overblock function--can do up to a 32x32 sprite
  void drawblock(BITMAP *dest,int mask)
  {
    int thold=tile;
    int t1=tile;
    int t2=tile+20;
    int t3=tile+1;
    int t4=tile+21;
    switch(mask) {
    case 1:
      enemy::draw(dest);
      break;
    case 3:
      if(flip&2)
        swap(t1,t2);
      tile=t1; enemy::draw(dest);
      tile=t2; yofs+=16; enemy::draw(dest); yofs-=16;
      break;
    case 5:
      t2=tile+1;
      if(flip&1)
        swap(t1,t2);
      tile=t1; enemy::draw(dest);
      tile=t2; xofs+=16; enemy::draw(dest); xofs-=16;
      break;
    case 15:
      if(flip&1) {
        swap(t1,t3);
        swap(t2,t4);
        }
      if(flip&2) {
        swap(t1,t2);
        swap(t3,t4);
        }
      tile=t1; enemy::draw(dest);
      tile=t2; yofs+=16; enemy::draw(dest); yofs-=16;
      tile=t3; xofs+=16; enemy::draw(dest);
      tile=t4; yofs+=16; enemy::draw(dest); xofs-=16; yofs-=16;
      break;
    }
    tile=thold;
  }

  void masked_draw(BITMAP *dest,int mx,int my,int mw,int mh)
  {
    BITMAP *sub=create_sub_bitmap(dest,mx,my,mw,mh);
    if(sub!=NULL)
    {
      xofs-=mx;
      yofs-=my;
      enemy::draw(sub);
      xofs+=mx;
      yofs+=my;
      destroy_bitmap(sub);
    }
    else
      enemy::draw(dest);
  }


  // override hit detection to check for invicibility, stunned, etc
  virtual bool hit(sprite *s)
  {
   return (dying || hclk>0) ? false : sprite::hit(s);
  }
  virtual bool hit(int tx,int ty,int txsz,int tysz)
  {
   return (dying || hclk>0) ? false : sprite::hit(tx,ty,txsz,tysz);
  }
  virtual bool hit(weapon *w)
  {
   return (dying || hclk>0) ? false : sprite::hit(w);
  }


//                         --==**==--

//   Movement routines that can be used by derived classes as needed

//                         --==**==--


  void fix_coords()
  {
    x=fix((int(x)&0xF0)+((int(x)&8)?16:0));
    y=fix((int(y)&0xF0)+((int(y)&8)?16:0));
  }

  // returns true if next step is ok, false if there is something there
  bool canmove(int ndir,fix s,int special,int dx1,int dy1,int dx2,int dy2)
  {
    bool ok;
    switch(ndir) {
    case 8:
    case up:     ok = !m_walkflag(x,y+dy1-s,special); break;
    case 12:
    case down:   ok = !m_walkflag(x,y+dy2+s,special); break;
    case 14:
    case left:   ok = !m_walkflag(x+dx1-s,y+8,special); break;
    case 10:
    case right:  ok = !m_walkflag(x+dx2+s,y+8,special); break;
    case 9:
    case r_up:   ok = !m_walkflag(x,y+dy1-s,special) && !m_walkflag(x+dx2+s,y+8,special); break;
    case 11:
    case r_down: ok = !m_walkflag(x,y+dy2+s,special) && !m_walkflag(x+dx2+s,y+8,special); break;
    case 13:
    case l_down: ok = !m_walkflag(x,y+dy2+s,special) && !m_walkflag(x+dx1-s,y+8,special); break;
    case 15:
    case l_up:   ok = !m_walkflag(x,y+dy1-s,special) && !m_walkflag(x+dx1-s,y+8,special); break;
    default: db=99; ok=true;
    }
    return ok;
  }
  bool canmove(int ndir,fix s,int special)
  {
    return canmove(ndir,s,special,0,-8,15,15);
  }
  bool canmove(int ndir,int special)
  {
    return canmove(ndir,1,special,0,-8,15,15);
  }
  bool canmove(int ndir)
  {
    return canmove(ndir,1,spw_none,0,-8,15,15);
  }

  // 8-directional
  void newdir_8(int rate,int special,int dx1,int dy1,int dx2,int dy2)
  {
    int ndir;
    // can move straight, check if it wants to turn
    if(canmove(dir,step,special,dx1,dy1,dx2,dy2))
    {
      int r=rand();
      if(rate>0 && !(r%rate))
      {
        ndir = ((dir+((r&64)?-1:1))&7)+8;
        if(canmove(ndir,step,special,dx1,dy1,dx2,dy2))
          dir=ndir;
        else
        {
          ndir = ((dir+((r&64)?1:-1))&7)+8;
          if(canmove(ndir,step,special,dx1,dy1,dx2,dy2))
            dir=ndir;
        }
        if(dir==ndir)
        {
          x.v&=0xFFFF0000;
          y.v&=0xFFFF0000;
        }
      }
      return;
    }
    // can't move straight, must turn
    do {
      ndir=(rand()&7)+8;
    } while(!canmove(ndir,step,special,dx1,dy1,dx2,dy2));
    dir=ndir;
    x.v&=0xFFFF0000;
    y.v&=0xFFFF0000;
  }
  void newdir_8(int rate,int special)
  {
    newdir_8(rate,special,0,-8,15,15);
  }

  // makes the enemy slide backwards when hit
  // sclk: first byte is clk, second byte is dir
  bool slide()
  {
    if(sclk==0 || hp<=0)
      return false;
    if((sclk&255)==16 && !canmove(sclk>>8,12,0)) {
      sclk=0;
      return false;
      }
    sclk--;
    switch(sclk>>8) {
    case up:    y-=4; break;
    case down:  y+=4; break;
    case left:  x-=4; break;
    case right: x+=4; break;
    }
    if(!canmove(sclk>>8,0,0)) {
      switch(sclk>>8) {
      case up:
      case down:
        if( (int(y)&15) > 7 )
          y=(int(y)&0xF0)+16;
        else
          y=(int(y)&0xF0);
        break;
      case left:
      case right:
        if( (int(x)&15) > 7 )
          x=(int(x)&0xF0)+16;
        else
          x=(int(x)&0xF0);
        break;
      }
      sclk=0;
      clk3=0;
     }
    if((sclk&255)==0)
      sclk=0;
    return true;
  }


  bool fslide()
  {
    if(sclk==0 || hp<=0)
      return false;
    if((sclk&255)==16 && !canmove(sclk>>8,12,spw_floater)) {
      sclk=0;
      return false;
      }
    sclk--;
    switch(sclk>>8) {
    case up:    y-=4; break;
    case down:  y+=4; break;
    case left:  x-=4; break;
    case right: x+=4; break;
    }
    if(!canmove(sclk>>8,0,spw_floater)) {
      switch(sclk>>8) {
      case up:
      case down:
        if( (int(y)&15) > 7 )
          y=(int(y)&0xF0)+16;
        else
          y=(int(y)&0xF0);
        break;
      case left:
      case right:
        if( (int(x)&15) > 7 )
          x=(int(x)&0xF0)+16;
        else
          x=(int(x)&0xF0);
        break;
      }
      sclk=0;
      clk3=0;
     }
    if((sclk&255)==0)
      sclk=0;
    return true;
  }


  // changes enemy's direction, checking restrictions
  // rate:   0 = no random changes, 16 = always random change
  // homing: 0 = none, 256 = always
  // grumble 0 = none, 4 = strongest appetite
  void newdir(int rate,int homing,int special)
  {
    int ndir;
    if(grumble && (rand()&3)<grumble)
    {
      int w = Lwpns.idFirst(wBait);
      if(w>=0)
      {
        int bx = Lwpns.spr(w)->x;
        int by = Lwpns.spr(w)->y;
        if(abs(int(y)-by)>14)
        {
          ndir = (by<y) ? up : down;
          if(canmove(ndir,special))
          {
            dir=ndir;
            return;
          }
        }
        ndir = (bx<x) ? left : right;
        if(canmove(ndir,special))
        {
          dir=ndir;
          return;
        }
      }
    }
    if((rand()&255)<homing)
    {
      ndir = lined_up(8);
      if(ndir>=0 && canmove(ndir,special))
      {
        dir=ndir;
        return;
      }
    }
    do {
      int r=rand();
      if((r&15)<rate)
        ndir=(r>>4)&3;
      else
        ndir=dir;
    } while(!canmove(ndir,special));
    dir=ndir;
  }
  void newdir()
  {
    newdir(4,0,spw_none);
  }

  fix distance_left()
  {
    int a=x.v>>16;
    int b=y.v>>16;

    switch(dir)
    {
    case up:    return fix(b&0xF);
    case down:  return fix(16-(b&0xF));
    case left:  return fix(a&0xF);
    case right: return fix(16-(a&0xF));
    }
    return 0;
  }

  // keeps walking around
  void constant_walk(int rate,int homing,int special)
  {
    if(slide())
      return;
    if(clk<0 || dying || stunclk || watch)
      return;
    if(clk3<=0)
    {
      fix_coords();
      newdir(rate,homing,special);
      clk3=int(16.0/step);
    }
    else if(scored)
    {
      dir^=1;
      clk3=int(16.0/step)-clk3;
    }
    clk3--;
    move(step);
  }
  void constant_walk()
  {
    constant_walk(4,0,spw_none);
  }

  int pos(int x,int y)
  {
   return (y<<8)+x;
  }

  // for variable step rates
  void variable_walk(int rate,int homing,int special)
  {
    if(slide())
      return;
    if(clk<0 || dying || stunclk || watch)
      return;
    if((int(x)&15)==0 && (int(y)&15)==0 && clk3!=pos(x,y))
    {
      fix_coords();
      newdir(rate,homing,special);
      clk3=pos(x,y);
    }
    move(step);
  }


  // pauses for a while after it makes a complete move (to a new square)
  void halting_walk(int rate,int homing,int special,int hrate, int haltcnt)
  {
    if(sclk && clk2)
      clk3=0;
    if(slide() || clk<0 || dying || stunclk || watch)
      return;
    if(clk2>0) {
      clk2--;
      return;
      }
    if(clk3<=0)
    {
      fix_coords();
      newdir(rate,homing,special);
      clk3=int(16.0/step);
      if(clk2<0)
        clk2=0;
      else if((rand()&15)<hrate)
      {
        clk2=haltcnt;
        return;
      }
    }
    else if(scored)
    {
      dir^=1;
      clk3=int(16.0/step)-clk3;
    }
    clk3--;
    move(step);
  }


  // 8-directional movement, aligns to 8 pixels
  void constant_walk_8(int rate,int special)
  {
    if(clk<0 || dying || stunclk || watch)
      return;
    if(clk3<=0) {
      newdir_8(rate,special);
      clk3=int(8.0/step);
      }
    clk3--;
    move(step);
  }

  // 8-directional movement, no alignment
  void variable_walk_8(int rate,int newclk,int special)
  {
    if(clk<0 || dying || stunclk || watch)
      return;
    if(!canmove(dir,step,special))
      clk3=0;
    if(clk3<=0) {
      newdir_8(rate,special);
      clk3=newclk;
      }
    clk3--;
    move(step);
  }

  // same as above but with variable enemy size
  void variable_walk_8(int rate,int newclk,int special,int dx1,int dy1,int dx2,int dy2)
  {
    if(clk<0 || dying || stunclk || watch)
      return;
    if(!canmove(dir,step,special,dx1,dy1,dx2,dy2))
      clk3=0;
    if(clk3<=0) {
      newdir_8(rate,special,dx1,dy1,dx2,dy2);
      clk3=newclk;
      }
    clk3--;
    move(step);
  }

  // the variable speed floater movement
  // ms is max speed
  // ss is step speed
  // s is step count
  // p is pause count
  // g is graduality :)
  void floater_walk(int rate,int newclk,fix ms,fix ss,int s,int p, int g)
  {
    clk2++;
    switch(movestatus)
    {
    case 0: // paused
      if(clk2>=p)
      {
        movestatus=1;
        clk2=0;
      }
      break;

    case 1: // speeding up
      if(clk2<g*s)
      {
        if(!((clk2-1)%g))
          step+=ss;
      }
      else
      {
        movestatus=2;
        clk2=0;
      }
      break;

    case 2: // normal
      step=ms;
      if(clk2>48 && !(rand()%768))
      {
        step=ss*s;
        movestatus=3;
        clk2=0;
      }
      break;

    case 3: // slowing down
      if(clk2<=g*s)
      {
        if(!(clk2%g))
          step-=ss;
      }
      else
      {
        movestatus=0;
        clk2=0;
      }
      break;
    }
    variable_walk_8(movestatus==2?rate:0,newclk,spw_floater);
  }

  void floater_walk(int rate,int newclk,fix s)
  {
    floater_walk(rate,newclk,s,0.125,3,80,32);
  }


  // Checks if enemy is lined up with Link. If so, returns direction Link is
  // at as compared to enemy. Returns -1 if not lined up. Range is inclusive.
  int lined_up(int range)
  {
    int lx = link.getX();
    int ly = link.getY();
    if(abs(lx-int(x))<=range) {
      if(ly<y)
        return up;
      return down;
      }
    if(abs(ly-int(y))<=range) {
      if(lx<x)
        return left;
      return right;
      }
    return -1;
  }

  // returns true if Link is within 'range' pixels of the enemy
  bool LinkInRange(int range)
  {
    int lx = link.getX();
    int ly = link.getY();
    return abs(lx-int(x))<=range && abs(ly-int(y))<=range;
  }

  // place the enemy in line with Link (red wizzrobes)
  void place_on_axis(bool floater)
  {
    int lx=min(max(int(link.getX())&0xF0,32),208);
    int ly=min(max(int(link.getY())&0xF0,32),128);
    int pos=rand()%23;
    int tried=0;
    bool last_resort,placed=false;

    do {
      if(pos<14)
        { x=(pos<<4)+16; y=ly; }
      else
        { x=lx; y=((pos-14)<<4)+16; }

      if(x<32 || y<32 || x>=224 || y>=144)
        last_resort=false;
      else
        last_resort=true;

      if(abs(lx-int(x))>16 || abs(ly-int(y))>16) {
        if(!m_walkflag(x,y,1))
          placed=true;
        else if(floater && last_resort && !iswater(MAPDATA(x,y)))
          placed=true;
        }

      if(!placed && tried>=22 && last_resort)
        placed=true;
      tried++;
      pos=(pos+3)%23;
    } while(!placed);

    if(y==ly)
      dir=(x<lx)?right:left;
    else
      dir=(y<ly)?down:up;
    clk2=tried;
  }
};



/********************************/
/*********  Guy Class  **********/
/********************************/


// good guys, fires, fairy, and other non-enemies
// based on enemy class b/c guys in dungeons act sort of like enemies
// also easier to manage all the guys this way
class guy : public enemy {
public:
  guy(fix X,fix Y,int Id,int Clk,bool mg) : enemy(X,Y,Id,Clk)
  {
    mainguy=mg;
    superman=dlevel?false:true;
    canfreeze=false;
    bhit=true;
    dp=wdp=hp=1;
    dir=down;
    yofs=56;
    hxofs=2;
    hxsz=12;
    hysz=17;
    cs=8;
    switch(id) {
    case gABEI:   tile=dlevel?87:84; break;
    case gAMA:    tile=85; break;
    case gDUDE:   tile=86; cs=6; break;
    case gMOBLIN: tile=116; break;
    case gFIRE:   tile=65;  frate=12; superman=true; break;
    case gFAIRY:  tile=63;  frate=8; break;
    case gGORIYA: tile=130; break;
    case gZELDA:  tile=35; cs=6; break;
    }
  }
  virtual bool animate(int index)
  {
    if(mainguy) {
      if(clk==0)
        setupscreen();
      else if(clk>0)   // keep clk from becoming 0 again
        clk=1;
      }
    if(mainguy && fadeclk==0)
      return true;
    hp=256; // good guys never die...
    if(hclk && !clk2) { // but if they get hit...
      clk2++; // only do this once
      addenemy(72,68,eFBALL,0);
      addenemy(168,68,eFBALL,0);
      }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
   switch(id) {
   case gFIRE:   flip = clk>=6 ?  1 :  0; break;
   case gFAIRY:  tile = clk>=4 ? 63 : 64; break;
   }
   if(!mainguy || fadeclk<0 || fadeclk&1)
     enemy::draw(dest);
  }
};



/*******************************/
/*********   Enemies   *********/
/*******************************/



class eFire : public enemy {
public:
  eFire(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    bhit=true;
    dp=wdp=hp=1;
    cs=8;
    tile=65;
    frate=12;
    item_set=0;
  }
  virtual void draw(BITMAP *dest)
  {
    flip = clk>=6 ? 1 : 0;
    enemy::draw(dest);
  }
};



void removearmos(int ax,int ay)
{
 ax&=0xF0;
 ay&=0xF0;
 int cd = (ax>>4)+ay;
 int f = tmpscr[0].data[cd]>>12;

 tmpscr[0].data[cd]=tmpscr[0].under;

 switch(f) {
 case mfARMOS_STAIR:
   tmpscr[0].data[cd]|=tcmb(42);
   sfx(WAV_SECRET);
   break;
 case mfARMOS_ITEM:
   if(!getmapflag()) {
     additem(ax,ay,tmpscr[0].catchall,ipONETIME+ipBIGRANGE);
     sfx(WAV_SECRET);
     }
   break;
 }
 putcombo(scrollbuf,ax,ay,tmpscr[0].data[cd]);
}


class eArmos : public enemy {
public:

  eArmos(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,0)
  {
    hp=3;
    dp=wdp=2;
    cs=8;
    tile=88+(rand()&1);
    superman=true;
    fading=fade_flicker;
    step=(rand()&1)?1.5:.5;
    count_enemy=false;
    item_set=0;
    if(canmove(down,8,spw_none)) {
      dir=down;
      clk3=int(13.0/step);
      }
    else
      dir=-1;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(misc)
      constant_walk(6,128,0);
    if(misc==0 && clk>=60) {
      misc=1;
      superman=false;
      fading=0;
      removearmos(x,y);
      clk=-1;
      if(dir==-1) {
        dir=0;
        y.v&=0xF00000;
        }
      }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    if(misc) {
      if(dir==up)
        tile = clk&8 ? 91:90;
      else
        tile = clk&8 ? 89:88;
      }
    enemy::draw(dest);
  }
};


class eGhini : public enemy {
public:

  eGhini(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    hp=9;
    dp=wdp=2;
    cs=7;
    tile=224;
    step=.5;
    if(id==eGHINI2) {
      fading=fade_flicker;
      superman=true;
      count_enemy=false;
      dir=12;
      step=0;
      movestatus=1;
      clk=0;
      }
    else
      leader=true;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(id==eGHINI1) {
      if(clk>=0)
        constant_walk(4,0,0);
      }
    if(id==eGHINI2) {
      if(misc)
      {
        if(clk>160)
          misc=2;
        floater_walk((misc==1)?0:4,12,0.625,0.0625,10,120,10);
      }
      else if(clk>=60)
      {
        misc=1;
        clk3=32;
        fading=0;
        guygrid[(int(y)&0xF0)+(int(x)>>4)]=0;
        }
      }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    switch(dir) {
    case 8:
    case 9:
    case up: tile=110; flip=0; break;
    case 15: tile=110; flip=1; break;
    case 10:
    case 11:
    case right: tile=109; flip=1; break;
    default:
      tile=109; flip=0; break;
    }
    enemy::draw(dest);
  }
  virtual void kickbucket()
  {
    hp=-1000;  // don't call death_sfx()
  }
};


class eTektite : public enemy {
public:
  int t;

  eTektite(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=hp=1;
    cs=8;
    if(id==eBTEK) { cs=7; item_set=isMONEY; }
    dir=down;
    misc=1;
    clk=-15*(rand()%3+1);
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(clk>=0 && !stunclk && (!watch || misc==0))
     switch(misc) {
     case 0:  // normal
      if(!(rand()%((id==eRTEK)?24:80))) {
        misc=1;
        clk2=32;
        }
      break;
     case 1:  // waiting to pounce
      if(--clk2<=0) {
        int r=rand();
        misc=2;
        step=-2.5;              // initial speed
        clk3=(r&1)+2;           // left or right
        clk2=(r&31)+10;         // flight time
        if(y<32)  clk2+=2;      // make them come down from top of screen
        if(y>112) clk2-=2;      // make them go back up
        t=9-((r&31)>>3);        // time before gravity kicks in
        }
      break;
     case 2:  // in flight
      move(step);
      t--;
      if(t<0 && step<2.5)
        step+=.5;
      if(x<=16)  clk3=right;
      if(x>=224) clk3=left;
      x+=(clk3==left)?-1:1;
      if((--clk2<=0 && y>=16) || y>=144)
        if(rand()%((id==eRTEK)?3:16))
          clk=misc=0;
        else {
          misc=1;
          clk2=0;
          }
      break;
     }
    if(stunclk && (clk&31)==1)
      clk=0;
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    if(misc==0)
      tile = clk&16 ? 104:103;
    else if(misc==1)
      tile = 103;
    else tile=104;
    enemy::draw(dest);
  }
};


class eItemFairy : public enemy {
  int ox,oy;

public:
  eItemFairy(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=hp=1;
    superman=true;
    dir=8;
    step=0.5;
    hxofs=1000;
    mainguy=false;
    count_enemy=false;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(clk>32)
      misc=1;
    bool w=watch;
    watch=false;
    variable_walk_8(misc?3:0,8,spw_floater);
    watch=w;
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
  }
};




class ePeahat : public enemy {
  int ox,oy;

public:
  ePeahat(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=1;
    hp=2;
    cs=8;
    superman=true;
    dir=8;
    step=0;
    movestatus=1;
    tile=102;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(clk>96)
      misc=1;
    floater_walk(misc?4:0,8,0.625,0.0625,10,80,16);
    superman=movestatus;
    stunclk=0;
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    int nx = real_x(x);
    int ny = real_y(y);
    if((ox!=nx || oy!=ny))
      tile=203-tile;
    ox = nx;
    oy = ny;
    enemy::draw(dest);
  }
};


class eLeever : public enemy {
public:
  eLeever(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=2;
    hp=4;
    if(id==eRLEV) { hp=2; dp=1; misc=-1; clk-=16; item_set=isMONEY; }
    frate=40;
    step=0.5;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(clk>=0 && !slide())
     switch(id) {
     case eRLEV:
      switch(misc) {
      case -1: {
       int active=0;
       for(int i=0; i<guys.Count(); i++)
         if(guys.spr(i)->id==eRLEV && ((enemy*)guys.spr(i))->misc>=0)
           active++;
       if(active<2)
         misc=0;
       } break;
      case 0: {
       int s=0;
       for(int i=0; i<guys.Count(); i++)
         if(guys.spr(i)->id==eRLEV && ((enemy*)guys.spr(i))->misc==1)
           s++;
       if(s>0)
         break;
       int d=rand()&1;
       if(LinkDir()>=left)
         d+=2;
       if(canplace(d) || canplace(d^1)) {
         misc=1;
         clk2=0;
         clk=0;
         }
       } break;
      case 1: if(++clk2>16) misc=2; break;
      case 2: if(++clk2>24) misc=3; break;
      case 3: if(stunclk) break; if(scored) dir^=1; if(!canmove(dir)) misc=4; else move(0.5); break;
      case 4: if(--clk2==16) { misc=5; clk=8; } break;
      case 5: if(--clk2==0)  misc=0; break;
      }
      break;

     case eBLEV:
      step=0.167;
      clk2++;
           if(clk2<32)    misc=1;
      else if(clk2<48)    misc=2;
      else if(clk2<300) { misc=3; step=0.5; }
      else if(clk2<316)   misc=2;
      else if(clk2<412)   misc=1;
      else if(clk2<540) { misc=0; step=0; }
      else clk2=0;
      if(clk2==48) clk=0;
      variable_walk(8,0,0);
     }
    hxofs=(misc>=2)?0:1000;
    return enemy::animate(index);
  }
  bool canplace(int d)
  {
    int nx=LinkX();
    int ny=LinkY();

    if(d<left) ny&=0xF0;
    else       nx&=0xF0;

    switch(d) {
    case up:    ny-=32; break;
    case down:  ny+=32; if(ny-LinkY()<32) ny+=16; break;
    case left:  nx-=32; break;
    case right: nx+=32; if(nx-LinkX()<32) nx+=16; break;
    }
    if(m_walkflag(nx,ny,spw_halfstep))
      return false;
    x=nx;
    y=ny;
    dir=d^1;
    return true;
  }
  virtual void draw(BITMAP *dest)
  {
    cs = (id==eBLEV)?7:8;
    switch(misc) {
    case -1:
    case 0: return;
    case 1:
    case 5: tile = (clk&8) ? 97:96; cs=8; break;
    case 2:
    case 4: tile = 98; break;
    case 3: tile = ((clk/5)&1) ? 100:99; break;
    }
    enemy::draw(dest);
  }
};


class eGel : public enemy {
public:
  eGel(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=hp=1;
    cs=9;
    step=1.0;
    bhit=true;
    clk2=-1;
    item_set=0;
    if(id>=0x1000)
      count_enemy=false;
    hxsz=hysz=14;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(clk>=0)
      switch(id>>12) {
      case 0: halting_walk(4,64,spw_none,16,((rand()&7)<<3)+2); break;
      case 1:
        if(misc==1) {
          dir=up;
          step=8;
          }
        if(misc<=2) {
          move(step);
          if(!canmove(dir,0,0))
            dir=down;
          }
        if(misc==3) {
          if(canmove(right,16,0))
            x+=16;
          }
        misc++;
        break;
      case 2:
        if(misc==1) {
          dir=down;
          step=8;
          }
        if(misc<=2)
          move(step);
        if(misc==3) {
          if(canmove(left,16,0))
            x-=16;
          }
        misc++;
        break;
      }
    if(misc>=4) {
      id&=0xFFF;
      step=1;
      if(x<32) x=32;
      if(x>208) x=208;
      if(y<32) y=32;
      if(y>128) y=128;
      }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    tile = clk&2 ? 121:120;
    enemy::draw(dest);
  }
};


class eZol : public enemy {
public:
  eZol(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=hp=2;
    cs=9;
    step=16.0/40.0;
    clk2=-1;
    item_set=isMONEY;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(hp==1 && !slide()) {
      guys.add(new eGel(x,y,eGEL+0x1000,-21));
      guys.add(new eGel(x,y,eGEL+0x2000,-22));
      return true;
      }
    else
      halting_walk(4,144,spw_none,16,(rand()&7)<<4);
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    tile = clk&8 ? 137:138;
    enemy::draw(dest);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    int ret=enemy::takehit(wpnId,power,wpnx,wpny,wpnDir);
    if(sclk)
      sclk+=128;
    return ret;
  }
};

int wallm_load_clk=0;

class eWallM : public enemy {
public:
  bool haslink;

  eWallM(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=1;
    hp=2;
    cs=7;
    step=16.0/40.0;
    haslink=false;
    item_set=isMONEY;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    hxofs=1000;
    if(misc==0)
    {
     if(frame-wallm_load_clk>80 && !watch && clk>=0)
     {
      int wall=link_on_wall();
      int wallm_cnt=0;
      for(int i=0; i<guys.Count(); i++)
        if(guys.spr(i)->id==eWALLM)
        {
          register int m=((enemy*)guys.spr(i))->misc;
          if(m && ((enemy*)guys.spr(i))->clk3==wall^1)
            wallm_cnt++;
        }
      if(wall>0)
      {
        wall--;
        misc=1;
        clk2=0;
        clk3=wall^1;
        wallm_load_clk=frame;
        if(wall<=down)
        {
          if(LinkDir()==left)
            dir=right;
          else
            dir=left;
        }
        else
        {
          if(LinkDir()==up)
            dir=down;
          else
            dir=up;
        }
        switch(wall)
        {
        case up:    y=0;   break;
        case down:  y=160; break;
        case left:  x=0;   break;
        case right: x=240; break;
        }
        switch(dir)
        {
        case up:    y=LinkY()+48-(wallm_cnt&1)*12; flip=wall&1;           break;
        case down:  y=LinkY()-48+(wallm_cnt&1)*12; flip=((wall&1)^1)+2;   break;
        case left:  x=LinkX()+48-(wallm_cnt&1)*12; flip=(wall==up?2:0)+1; break;
        case right: x=LinkX()-48+(wallm_cnt&1)*12; flip=(wall==up?2:0);   break;
        }
      }
     }
    }
    else
      wallm_crawl();
    return enemy::animate(index);
  }

  void wallm_crawl()
  {
    hxofs=0;
    if(slide())
      return;
    if(dying || watch || (!haslink && stunclk))
      return;
    clk2++;
    misc=(clk2/40)+1;
    switch(misc)
    {
    case 1:
    case 2: swap(dir,clk3); move(step); swap(dir,clk3); break;
    case 3:
    case 4:
    case 5: move(step); break;
    case 6:
    case 7: swap(dir,clk3); dir^=1; move(step); dir^=1; swap(dir,clk3); break;
    default: misc=0; break;
    }
  }

  void grablink()
  {
    haslink=true;
  }
  virtual void draw(BITMAP *dest)
  {
    tile = clk&8 ? 128:129;
    if(haslink) tile=128;
    if(misc>0)
      masked_draw(dest,16,72,224,144);
  }
};


class eTrap : public enemy {
public:
  int homex,homey;

  eTrap(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    homex=x;
    homey=y;
    dp=wdp=2;
    hp=1;
    cs=7;
    tile=122;
    superman=true;
    mainguy=false;
    count_enemy=false;
  }
  virtual bool animate(int index)
  {
    if(clk<0)
      return enemy::animate(index);

    if(misc==0) { // waiting
      int d=lined_up(15);
      if(d!=-1 && trapmove(d)) {
        dir=d;
        misc=1;
        clk2=(dir==down)?3:0;
        }
      }
    if(misc==1) { // charging
      clk2=(clk2+1)&3;
      step=(clk2==3)?1:2;
      if(!trapmove(dir) || clip()) {
        misc=2;
        dir=dir^1;
        }
      else
        sprite::move(step);
      }
    if(misc==2) { // retreating
      step=(++clk2&1)?1:0;
      if(int(x)==homex && int(y)==homey) {
        x=homex;
        y=homey;
        misc=0;
        }
      else
        sprite::move(step);
      }

    return enemy::animate(index);
  }

  bool trapmove(int ndir)
  {
    if(get_bit(QHeader.rules,qrMEANTRAPS))
    {
      if(tmpscr[0].flags2&fFLOATTRAPS)
        return canmove(ndir,1,spw_floater);
      return canmove(ndir,1,spw_water);
    }

    if(homey==80 && ndir<left)
      return false;
    if(homey<80 && ndir==up)
      return false;
    if(homey>80 && ndir==down)
      return false;
    if(homex<128 && ndir==left)
      return false;
    if(homex>128 && ndir==right)
      return false;
    return true;
  }

  bool clip()
  {
    switch(dir) {
    case up:    if(homey>80 && y<=86) return true; break;
    case down:  if(homey<80 && y>=80) return true; break;
    case left:  if(homex>128 && x<=124) return true; break;
    case right: if(homex<120 && x>=116) return true; break;
    }
    return false;
  }
};


class eRock : public enemy {
public:
  eRock(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=hp=1;
    cs=8;
    superman=true;
    mainguy=false;
    frate=12;
    clk2=clk;
    hxofs=hyofs=-2;
    hxsz=hysz=20;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(++clk2==0) // start it
    {
      x=rand()&0xF0;
      y=0;
      clk3=0;
      clk2=rand()&15;
    }
    if(clk2>16)   // move it
    {
      if(clk3<=0)             // start bounce
      {
        dir=rand()&1;
        if(x<32)  dir=1;
        if(x>208) dir=0;
      }
      if(clk3<13+16)
      {
        x += dir ? 1 : -1;
        if(clk3<2) y-=2;
        else if(clk3<5) y--;
        else if(clk3<8) ;
        else if(clk3<11) y++;
        else y+=2;

        clk3++;
      }
      else if(y<176)
        clk3=0;               // next bounce
      else
        clk2 = -(rand()&63);  // back to top
    }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    if(clk>=0)
    {
      tile = clk<6 ? 118:119;
      enemy::draw(dest);
    }
  }
};


// Helper for launching fireballs from statues, etc.
// It's invisible and can't be hit.
// Pass the range value through the clk variable in the constuctor.
class eFBall : public enemy {
public:
  eFBall(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=1;
    hp=1;
    mainguy=false;
    count_enemy=false;
    superman=true;
    hxofs=1000;
    hclk=clk;   // the "no fire" range
    clk=96;
  }
  virtual bool animate(int index)
  {
    if(++clk>80) {
      unsigned r=rand();
      if(!(r&63) && !LinkInRange(hclk)) {
        addEwpn(x,y,ewFireball,0,wdp,0);
        if(!((r>>7)&15))
          addEwpn(x-4,y,ewFireball,0,wdp,0);
        clk=0;
        }
      }
    return false;
  }
  virtual void draw(BITMAP *dest)
  {
  }
};


class eZora : public enemy {
public:
  eZora(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,0)
  {
    dp=wdp=1;
    hp=2;
    cs=9;
    mainguy=false;
    count_enemy=false;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();

    if(watch)
      return true;

    switch(clk) {
    case 0: { // reposition him
      int t=0;
      int pos=rand()%160 + 16;
      bool placed=false;
      while(!placed && t<160) {
        if(iswater(tmpscr[0].data[pos]) && (pos&15)>0 && (pos&15)<15) {
          x=(pos&15)<<4;
          y=pos&0xF0;
          hp=2;       // refill life each time
          hxofs=1000; // avoid hit detection
          stunclk=0;
          placed=true;
          }
        pos+=19;
        if(pos>=176)
          pos-=160;
        t++;
        }
      if(!placed || whistleclk>=88) // can't place him, he's gone
        return true;
      } break;
    case 35:    dir=(link.y+8<y)?up:down; hxofs=0; break;
    case 35+19: addEwpn(x,y,ewFireball,0,wdp,0); break;
    case 35+66: hxofs=1000; break;
    case 198:   clk=-1; break;
    }

    return enemy::animate(index);
  }

  virtual void draw(BITMAP *dest)
  {
    if(clk<3)
      return;
    int dl;
    if(clk<36) {
      dl=clk+5;
      goto waves;
      }
    if(clk<36+66)
      tile=(dir==up)?113:112;
    else {
      dl=clk-36-66;
      waves:
      tile=((dl/11)&1)+96;
      }
    enemy::draw(dest);
  }
};


class eStalfos : public enemy {
public:
  bool doubleshot;

  eStalfos(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=1;
    wdp=4;
    hp=2;
    cs=8;
    tile=125;
    step=0.5;
    doubleshot=false;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(id==eSTALFOS)
      constant_walk(4,128,0);
    else
    {
      halting_walk(4,128,0,4,48);
      if(clk2==16 && sclk==0 && !stunclk && !watch)
        Ewpns.add(new weapon(x,y,ewSword,0,wdp,dir));
      if(clk2==1 && !doubleshot && !(rand()&15)) {
        newdir(4,128,0);
        clk2=48;
        doubleshot=true;
        }
      if(clk2==0)
        doubleshot=false;
    }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    flip = clk&8 ? 1:0;
    enemy::draw(dest);
  }
};


class eGibdo : public enemy {
public:
  eGibdo(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=4;
    hp=7;
    cs=7;
    tile=136;
    step=0.5;
    item_set=isBOMBS;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    constant_walk(4,128,0);
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    flip = clk&8 ? 1:0;
    enemy::draw(dest);
  }
};


class eBubble : public enemy {
public:
  eBubble(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=0;
    hp=1;
    tile=111;
    superman=true;
    mainguy=false;
    step=1.0;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    constant_walk();
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    flip = clk&1 ? 1:0;
    switch(id)
    {
    case eRBUBBLE: cs=8; break;
    case eBBUBBLE: cs=7; break;
    default: cs=(frame&3)+6; break;
    }
    enemy::draw(dest);
  }
};


class eRope : public enemy {
public:
  bool charging;

  eRope(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=hp=1;
    if(id==eROPE2)
      hp=4;
    cs=8;
    step=0.5;
    charging=false;
    item_set=isMONEY;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    charge_attack();
    return enemy::animate(index);
  }
  void charge_attack()
  {
    if(slide())
      return;
    if(clk<0 || dir<0 || stunclk || watch)
      return;

    if(clk3<=0) {
      x=fix(int(x)&0xF0);
      y=fix(int(y)&0xF0);
      if(!charging) {
        int ldir = lined_up(7);
        if(ldir!=-1 && canmove(ldir)) {
          dir=ldir;
          charging=true;
          step=1.5;
          }
        else newdir(4,0,0);
        }
      if(!canmove(dir)) {
        newdir();
        charging=false;
        step=0.5;
        }
      clk3=int(16.0/step);
      }

    move(step);
    clk3--;
  }
  virtual void draw(BITMAP *dest)
  {
    tile = clk&8 ? 126:127;
    flip = dir==left ? 1:0;
    if(id==eROPE2)
      cs=(frame&3)+6;
    enemy::draw(dest);
  }
};




class eKeese : public enemy {
  int ox,oy,c;

public:
  eKeese(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=hp=1;
    cs=9;
    if(id==eKEESE1) cs=7;
    if(id==eKEESE2) cs=8;
    bhit=true;
    dir=(rand()&7)+8;
    step=0;
    movestatus=1;
    tile=124;
    c=0;
    item_set=0;
    hxofs=2;
    hxsz=12;
    hyofs=4;
    hysz=8;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    floater_walk(2,8,0.625,0.0625,10,120,16);
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    int nx = real_x(x);
    int ny = real_y(y);
    if(ox!=nx || oy!=ny)
      c++;
    if(c>=2)
    {
      tile=247-tile;
      c=0;
    }
    ox = nx;
    oy = ny;
    enemy::draw(dest);
  }
};


class eVire : public enemy {
public:
  eVire(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=2;
    hp=4;
    cs=7;
    frate=20;
    dir=rand()&3;
    step=0.5;
    item_set=isBOMBS;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(hp>0 && hp<4 && !fslide()) {
      int kids = guys.Count();
      guys.add(new eKeese(x,y,eKEESE2,-24));
      guys.add(new eKeese(x,y,eKEESE2,-24));
      ((enemy*)guys.spr(kids))->count_enemy = false;
      ((enemy*)guys.spr(kids+1))->count_enemy = false;
      return true;
      }
    vire_hop();
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    tile = (clk>=10) ? 165:166;
    if(dir==up) tile+=2;
    enemy::draw(dest);
  }
  void vire_hop()
  {
    if(slide())
      return;
    if(clk<0 || dying || stunclk || watch)
      return;
    if(clk3<=0)
    {
      fix_coords();
      newdir(4,64,spw_none);
      clk3=32;
    }
    clk3--;
    move(step);
    if(dir>=left)
    {
      fix h = (31-clk3)*0.125 - 2.0;
      y+=h;
    }
  }
};



class ePolsVoice : public enemy {
public:
  fix fy;

  ePolsVoice(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=4;
    hp=10;
    cs=6;
    step=1.0;
    fy=y;
    item_set=isMONEY;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    polsvoice_hop();
    return enemy::animate(index);
  }
  void polsvoice_hop()
  {
    if(slide())
      return;
    if(clk<0 || dying || stunclk || watch)
      return;
    swap(fy,y);
    if(clk3<=0)
    {
      fix_coords();
      if(clk2<=0 || !canmove(dir,1,spw_floater))
        newdir(4,32,spw_floater);
      if(clk2<=0)
      {
        fy=y;
        if(!canmove(dir,2,spw_none) || m_walkflag(x,y,spw_none) || !(rand()%3))
          clk2=32;
      }
      clk3=32;
    }
    clk3--;
    swap(fy,y);
    if(clk3&1)
    {
      if(dir==up)   fy-=step;
      if(dir==down) fy+=step;
      move(step);
      clk2--;
      if(clk2>0)
      {
        fix h = (32-clk2)*0.25 - 4.0;
        if(h<-2.5) h=-2.5;
        if(h> 2.5) h=2.5;
        y+=h;
      }
    }
  }
  virtual void draw(BITMAP *dest)
  {
    tile = clk&8 ? 134:135;
    enemy::draw(dest);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || clk<0 || hclk>0 || superman)
      return 0;

    switch(wpnId) {
    case wBomb:
    case wLitBomb:
    case wFire:
    case wBait:
    case wWhistle:
    case wWind:
      return 0;
    case wBrang:
    case wMagic:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    case wArrow:
      hp=0;
      break;
    default:
      hp-=power;
    }
    hclk=33;
    sfx(WAV_EHIT,pan(int(x)));
    return wpnId==wArrow ? 0 : 1;  // arrow keeps going
  }
};



class eLikeLike : public enemy {
public:
  bool haslink;

  eLikeLike(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=2;
    hp=9;
    cs=8;
    haslink=false;
    step=0.5;
    item_set=0;
  }
  virtual bool animate(int index)
  {
    if(hp<=0 && haslink) {
      link.beatlikelike();
      haslink=false;
      }
    if(dying)
      return Dead();

    if(haslink) {
      clk2++;
      if(clk2==95)
        game.misc &= ~iSHIELD;
      if((clk&0x18)==8) // stop its animation on the middle frame
        clk--;
      }
    else
      constant_walk(4,64,spw_none);
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    switch(clk&0x18) {
    case 0:  tile=162; break;
    case 8:
    case 24: tile=163; break;
    case 16: tile=164; break;
    }
    enemy::draw(dest);
  }
  void eatlink()
  {
    haslink=true;
    x=link.getX();
    y=link.getY();
    clk2=0;
  }
};



class eOctorok : public enemy {
public:
  eOctorok(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    hp=dp=wdp=1;
    cs=8;
    grumble=1;
    if(id==eBOCTO1 || id==eBOCTO2)
    {
      hp=2; cs=7;
      item_set=isBOMBS;
      grumble=2;
    }
    if(id==eROCTO2 || id==eBOCTO2)
      step=1.0;
    else
      step=0.5;
    frate=12;
  }

  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    halting_walk(2+(4*step),128*step,0,(id==eBOCTO1 || id==eBOCTO2)?5:3,48);
    if(clk2==16 && sclk==0 && !stunclk && !watch)
      Ewpns.add(new weapon(x,y,ewRock,0,wdp,dir));
    return enemy::animate(index);
  }

  virtual void draw(BITMAP *dest)
  {
    switch(dir) {
    case up:    flip=2; tile=(clk>=6)?93:92; break;
    case down:  flip=0; tile=(clk>=6)?93:92; break;
    case left:  flip=0; tile=(clk>=6)?95:94; break;
    case right: flip=1; tile=(clk>=6)?95:94; break;
    }
    enemy::draw(dest);
  }
};



class eMolblin : public enemy {
public:
  bool doubleshot;

  eMolblin(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=1;
    hp=2; cs=8;
    grumble=3;
    if(id==eBMOLBLIN)
    {
      hp=3; cs=9;
      item_set=isBOMBS;
      grumble=4;
    }
    frate=12;
    step=0.5;
  }

  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    halting_walk((id==eBMOLBLIN)?8:5,128,0,(id==eBMOLBLIN)?6:4,48);
    if(clk2==16 && sclk==0 && !stunclk && !watch)
      Ewpns.add(new weapon(x,y,ewArrow,3,wdp,dir));
    if(clk2==1 && id==eBMOLBLIN && !doubleshot && !(rand()&15)) {
      newdir(8,128,0);
      clk2=48;
      doubleshot=true;
      }
    if(clk2==0)
      doubleshot=false;
    return enemy::animate(index);
  }

  virtual void draw(BITMAP *dest)
  {
    switch(dir) {
    case up:    tile=117; flip=(clk>=6)?1:0; break;
    case down:  tile=116; flip=(clk>=6)?1:0; break;
    case left:  flip=1; tile=(clk>=6)?115:114; break;
    case right: flip=0; tile=(clk>=6)?115:114; break;
    }
    enemy::draw(dest);
  }
};


class eLynel : public enemy {
public:
  bool doubleshot;

  eLynel(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=2; wdp=4;
    hp=4; cs=8;
    item_set=isMONEY;
    if(id==eBLYNEL)
    {
      hp=6; cs=7; dp=4;
      item_set=isBOMBS;
    }
    frate=12;
    step=0.5;
  }

  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    halting_walk(6,128,0,4,48);
    if(clk2==16 && sclk==0 && !stunclk && !watch)
      Ewpns.add(new weapon(x,y,ewSword,0,wdp,dir));
    if(clk2==1 && !doubleshot && !(rand()&15)) {
      newdir(6,128,0);
      clk2=48;
      doubleshot=true;
      }
    if(clk2==0)
      doubleshot=false;
    return enemy::animate(index);
  }

  virtual void draw(BITMAP *dest)
  {
    switch(dir) {
    case up:    tile=108; flip=(clk>=6)?1:0; break;
    case down:  tile=107; flip=(clk>=6)?1:0; break;
    case left:  flip=1; tile=(clk>=6)?106:105; break;
    case right: flip=0; tile=(clk>=6)?106:105; break;
    }
    enemy::draw(dest);
  }
};


class eGoriya : public enemy {
public:
  eGoriya(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=1; wdp=2;
    hp=3; cs=8;
    item_set=isBOMBS;
    grumble=4;
    if(id==eBGORIYA)
    {
      hp=5; cs=7; dp=2;
      item_set=isLIFE;
    }
    frate=12;
    step=0.5;
  }


  virtual bool animate(int index)
  {
    if(dying)
    {
      KillWeapon();
      return Dead();
    }
    if(!WeaponOut())
    {
      halting_walk(6,128,0,(id==eRGORIYA)?5:6,48);
      if(clk2==16 && sclk==0 && !stunclk && !watch)
      {
        misc=index+100;
        Ewpns.add(new weapon(x,y,ewBrang,misc,wdp,dir));
      }
    }
    else if(clk2>2)
      clk2--;
    return enemy::animate(index);
  }

  bool WeaponOut()
  {
    for(int i=0; i<Ewpns.Count(); i++)
      if(((weapon*)Ewpns.spr(i))->type==misc && Ewpns.spr(i)->id==ewBrang)
        return true;
    return false;
  }

  void KillWeapon()
  {
    for(int i=0; i<Ewpns.Count(); i++)
      if(((weapon*)Ewpns.spr(i))->type==misc && Ewpns.spr(i)->id==ewBrang)
        Ewpns.del(i);
  }

  virtual void draw(BITMAP *dest)
  {
    switch(dir) {
    case up:    tile=131; flip=(clk>=6)?1:0; break;
    case down:  tile=130; flip=(clk>=6)?1:0; break;
    case left:  flip=1; tile=(clk>=6)?133:132; break;
    case right: flip=0; tile=(clk>=6)?133:132; break;
    }
    enemy::draw(dest);
  }
};


class eDarknut : public enemy {
public:
  eDarknut(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=2;
    hp=4; cs=8;
    step=0.5;
    item_set=isBOMBS;
    if(id==eBDKNUT)
    {
      hp=8; cs=7; dp=4; step=0.667; item_set=isLIFE;
    }
  }

  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(id==eRDKNUT)
      constant_walk(5,144,0);
    else
      constant_walk(4,160,0);
    return enemy::animate(index);
  }

  virtual void draw(BITMAP *dest)
  {
    switch(dir) {
    case up:    tile=142; flip=(clk&8)?1:0; break;
    case down:  flip=0; tile=(clk&8)?140:141; break;
    case left:  flip=1; tile=(clk&8)?143:144; break;
    case right: flip=0; tile=(clk&8)?143:144; break;
    }
    enemy::draw(dest);
  }

  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || clk<0 || hclk>0 || superman)
      return 0;

    switch(wpnId) {
    case wLitBomb:
    case wBait:
    case wWhistle:
    case wFire:
    case wWind:
      return 0;
    }

    if(wpnId!=wBomb && wpnDir==(dir^1)) {
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
      }

    switch(wpnId) {
    case wArrow:
    case wBrang:
    case wMagic:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    case wBomb:
      if(wpnDir==(dir^1))
        break;
    default:
      hp-=power;
    }
    hclk=33;
    if((dir&2)==(wpnDir&2))
      sclk=(wpnDir<<8)+16;
    sfx(WAV_EHIT,pan(int(x)));
    return 1;
  }
};


class eWizzrobe : public enemy {
public:
  bool flying;

  eWizzrobe(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    switch(id) {
    case eRWIZ: dp=2; wdp=8; hp=4;  cs=8; hxofs=1000;
                fading=fade_invisible; clk+=220+14; item_set=isBOMBS; break;
    case eBWIZ: dp=4; wdp=4; hp=10; cs=7; step=0.5;
                dir=(loadside==right)?right:left; misc=-3;
                break;
    }
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(id==eRWIZ)
    {
      if(watch)
      {
        fading=0;
        hxofs=0;
      }
      else switch(clk)
      {
       case 0:
         place_on_axis(true);
         fading=fade_flicker;
         hxofs=0;
         break;
       case 64:  fading=0; break;
       case 83:
         addEwpn(x,y,ewMagic,0,wdp,dir);
         sfx(WAV_WAND,pan(int(x)));
         break;
       case 128: fading=fade_flicker; break;
       case 146: fading=fade_invisible; hxofs=1000; break;
       case 220: clk=-1; break;
      }
    }
    else
      wizzrobe_attack();
    return enemy::animate(index);
  }

  void wizzrobe_attack()
  {
    if(slide())
      return;
    if(clk<0 || dying || stunclk || watch)
      return;
    if(clk3<=0 || ((clk3&31)==0 && !canmove(dir,1,spw_door) && !misc))
    {
      fix_coords();
      switch(misc)
      {
      case 1:
        if(!m_walkflag(x,y,spw_door))
          misc=0;
        else
        {
          clk3=16;
          if(!canmove(dir,1,spw_floater))
            newdir(4,0,spw_floater);
        }
        break;

      case 2:
       {
        int jx=x;
        int jy=y;
        int jdir=-1;
        switch(rand()&7)
        {
        case 0: jx-=32; jy-=32; jdir=15; break;
        case 1: jx+=32; jy-=32; jdir=9;  break;
        case 2: jx+=32; jy+=32; jdir=11; break;
        case 3: jx-=32; jy+=32; jdir=13; break;
        }
        if(jdir>0 && jx>=32 && jx<=208 && jy>=32 && jy<=128)
        {
          misc=3;
          clk3=32;
          dir=jdir;
          break;
        }
       }

      case 3:
        dir&=3;
        misc=0;

      case 0:
        newdir(4,64,spw_floater);

      default:
        if(!canmove(dir,1,spw_door))
        {
          if(canmove(dir,15,spw_floater))
          {
            misc=1;
            clk3=16;
          }
          else
          {
            newdir(4,64,spw_floater);
            misc=0;
            clk3=32;
          }
        }
        else
        {
          clk3=32;
        }
        break;
      }
      if(misc<0)
        misc++;
    }
    clk3--;
    switch(misc)
    {
    case 1:
    case 3:  step=1.0; break;
    case 2:  step=0;   break;
    default: step=0.5; break;
    }
    move(step);
    if(id==eBWIZ && misc<=0 && clk3==28)
    {
      if(lined_up(8) == dir)
      {
        addEwpn(x,y,ewMagic,0,wdp,dir);
        sfx(WAV_WAND,pan(int(x)));
      }
    }
    if(misc==0 && (rand()&127)==0)
      misc=2;
    if(misc==2 && clk3==4)
      fix_coords();
  }

  virtual void draw(BITMAP *dest)
  {
    if(id==eBWIZ && (misc==1 || misc==3) && (clk3&1) && hp>0 && !watch)  // phasing
      return;

    if(id==eRWIZ)
      tile=(frame&4)?170:169;
    else
      tile=(clk&8)?170:169;
    switch(dir) {
    case 9:
    case 15:
    case up:
      if(id==eRWIZ)
        tile=(frame&4)?172:171;
      else
        tile=(clk&8)?172:171;
      break;
    case 13:
    case left:  flip=1; break;
    default:    flip=0; break;
    }
    enemy::draw(dest);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || clk<0 || hclk>0 || superman)
      return 0;

    switch(wpnId) {
    case wFire:
    case wArrow:
    case wWand:
    case wLitBomb:
    case wBait:
    case wWhistle:
    case wWind:
      return 0;
    case wBrang:
    case wMagic:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    default:
      hp-=power;
    }
    hclk=33;
    sfx(WAV_EHIT,pan(int(x)));
    return 1;
  }
};




/*********************************/
/**********   Bosses   ***********/
/*********************************/


class eDodongo : public enemy {
public:
  eDodongo(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=2;
    hp=8;
    cs=8;
    step=0.5;
    fading=fade_flash_die;
    cont_sfx(WAV_ROAR);
  }

  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(clk2) {    // ate a bomb
      if(--clk2==0)
        hp-=misc; // store bomb's power in misc
      }
    else
      constant_walk(4,64,spw_clipright);
    hxsz = (dir<=down) ? 16 : 32;
    return enemy::animate(index);
  }

  virtual void draw(BITMAP *dest)
  {
    if(clk<0) {
      enemy::draw(dest);
      return;
      }

    int fr = stunclk>0 ? 16 : 8;

    if(!dying && clk2>0 && clk2<=64) {
      // bloated
      switch(dir) {
      case up:    tile=158; flip=0; break;
      case down:  tile=156; flip=0; break;
      case left:
        flip=1;
        tile=153;
        xofs=16;
        enemy::draw(dest);
        xofs=0;
        tile++;
        break;
      case right:
        flip=0;
        tile=154;
        xofs=16;
        enemy::draw(dest);
        xofs=0;
        tile--;
        break;
      }
    }
    else if(!dying || clk2>19) {
      // normal
      switch(dir) {
      case up:    tile=157; flip=(clk&fr)?1:0; break;
      case down:  tile=155; flip=(clk&fr)?1:0; break;
      case left:
        flip=1;
        tile=(clk&fr)?151:149;
        xofs=16;
        enemy::draw(dest);
        xofs=0;
        tile++;
        break;
      case right:
        flip=0;
        tile=(clk&fr)?152:150;
        xofs=16;
        enemy::draw(dest);
        xofs=0;
        tile--;
        break;
      }
    }
    enemy::draw(dest);
  }

  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || clk<0 || clk2>0 || superman)
      return 0;

    switch(wpnId) {
    case wFire:
    case wBait:
    case wWhistle:
    case wWind:
      return 0;
    case wLitBomb:
      if(abs(wpnx-((dir==right)?x+16:x)) > 7 || abs(wpny-y) > 7)
        return 0;
      clk2=96;
      misc=power;
      return 1;
    case wBomb:
      if(abs(wpnx-((dir==right)?x+16:x)) > 8 || abs(wpny-y) > 8)
        return 0;
      stunclk=160;
      return 1;
    case wSword:
    case wBeam:
      if(stunclk) {
        sfx(WAV_EHIT,pan(int(x)));
        hp=0;
        item_set=isBOMB100;
        fading=0;   // don't flash
        return 1;
        }
    default:
      sfx(WAV_CHINK,pan(int(x)));
    }
    return 1;
  }
  virtual void death_sfx()
  {
    if(guys.idCount(eDODONGO)<2)  // count self
      stop_sfx(WAV_ROAR);
    sfx(WAV_GASP,pan(int(x)));
  }
};



class eAquamentus : public enemy {
public:

  eAquamentus(fix X,fix Y,int Id,int Clk) : enemy(176,64,Id,Clk)
  {
    dp=2;
    wdp=1;
    hp=6;
    cs=csBOSS;
    loadpalset(csBOSS,pSprite(spAQUA));
    yofs=57;
    clk3=32;
    clk2=0;
    dir=left;
    cont_sfx(WAV_ROAR);
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(--clk3==0) {
      addEwpn(x+4,y,ewFireball,0,wdp,up+1);
      addEwpn(x+4,y,ewFireball,0,wdp,0);
      addEwpn(x+4,y,ewFireball,0,wdp,down+1);
      }
    if(clk3<-80 && !(rand()&63))
      clk3=32;
    if(!((clk+1)&63)) {
      int d=(rand()%3)+1;
      if(d>=left)
        dir=d;
      if(x<=136)
        dir=right;
      if(x>=200)
        dir=left;
      }
    if(clk>=-1 && !((clk+1)&7)) {
      if(dir==left) x-=1;
      else x+=1;
      }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    xofs=0;
    if(clk<0 || dying) {
      enemy::draw(dest);
      return;
      }
    // face
    tile=(clk3>0)?226:228; enemy::draw(dest);
    // tail
    tile=(clk&16)?227:229; xofs=16;  enemy::draw(dest);
    // body
    yofs+=16;
    xofs=0;  tile=(clk&16)?246:248; enemy::draw(dest);
    xofs=16; tile=(clk&16)?247:249; enemy::draw(dest);
    yofs-=16;
  }
  virtual bool hit(weapon *w)
  {
   switch(w->id) {
   case wBeam:
   case wMagic: hysz=32;
   }
   bool ret = (dying || hclk>0) ? false : sprite::hit(w);
   hysz=16;
   return ret;
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || clk<0 || hclk>0 || superman)
      return 0;
    switch(wpnId) {
    case wFire:
    case wLitBomb:
    case wBait:
    case wWhistle:
    case wWind:
      return 0;
    case wBrang:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    default:
      hp-=power;
      hclk=33;
    }
    sfx(WAV_EHIT,pan(int(x)));
    sfx(WAV_GASP,pan(int(x)));
    return 1;
  }
  virtual void death_sfx()
  {
    setmapflag(mfNEVERRET);
    stop_sfx(WAV_ROAR);
  }
};



class eGohma : public enemy {
public:

  eGohma(fix X,fix Y,int Id,int Clk) : enemy(128,48,Id,0)
  {
    dp=4;
    wdp=2;
    switch(id) {
    case eRGOHMA: hp=2; cs=8; break;
    case eBGOHMA: hp=6; cs=7; break;
    }
    hxofs=-16;
    hxsz=48;
    yofs=57;
    cont_sfx(WAV_ROAR);
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if((clk&63)==0) {
      if(clk&64)
        dir^=1;
      else
        dir=rand()%3+1;
      }
    if((clk&63)==3)
      addEwpn(x,y+2,ewFireball,1,wdp,0);
    if(clk&1)
      move(1);
    if(++clk3>=400)
      clk3=0;
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    if(clk<0 || dying) {
      enemy::draw(dest);
      return;
      }

    // left side
    xofs=-16;
    flip=0;
    if(clk&16) tile=180;
    else { tile=182; flip=1; }
    enemy::draw(dest);

    // right side
    xofs=16;
    tile=(180+182)-tile;
    enemy::draw(dest);

    // body
    xofs=0;
    if(clk3<16)
      tile=184;
    else if(clk3<116)
      tile=185;
    else if(clk3<132)
      tile=184;
    else tile=((clk3-132)&8)?183:181;
    enemy::draw(dest);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || clk<0 || hclk>0 || superman)
      return 0;
    switch(wpnId) {
    case wFire:
    case wLitBomb:
    case wBomb:
    case wBait:
    case wWhistle:
    case wWind:
      return 0;
    case wArrow:
      if(wpnDir==up && abs(int(x)-wpnx)<6 && clk3>=16 && clk3<116) {
        hp-=power;
        hclk=33;
        break;
        }
    default:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    }
    sfx(WAV_EHIT,pan(int(x)));
    sfx(WAV_GASP,pan(int(x)));
    return 1;
  }
  virtual void death_sfx()
  {
    setmapflag(mfNEVERRET);
    stop_sfx(WAV_ROAR);
  }
};



class eLilDig : public enemy {
public:

  eLilDig(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=4;
    wdp=2;
    hp=6;
    cs=csBOSS;
    step=0.25;
    frate=12;
    count_enemy=false;
  }
  virtual bool animate(int index)
  {
    if(dying) {
      if(clk2==1)
        setmapflag(mfNEVERRET);
      return Dead();
      }
    if(misc<=128) {
      if(!(++misc&31))
        step+=0.25;
      }
    variable_walk_8(2,16,spw_floater);
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    tile = (clk>=6)?161:160;
    enemy::draw(dest);
  }
};



class eBigDig : public enemy {
public:

  eBigDig(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=4;
    wdp=2;
    superman=true;
    hxofs=hyofs=-8;
    hxsz=hysz=32;
    cs=csBOSS;
    loadpalset(csBOSS,pSprite(spDIG));
    step=.25;
    cont_sfx(WAV_VADER);
    frate=6;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    switch(misc) {
    case 0: variable_walk_8(2,16,spw_floater,-8,-16,23,23); break;
    case 1: misc++; break;
    case 2: guys.add(new eLilDig(x,y,id+0x1000,-14));
      if(id==eDIG3) {
        guys.add(new eLilDig(x,y,id+0x1000,-15));
        guys.add(new eLilDig(x,y,id+0x1000,-15));
        }
      stop_sfx(WAV_VADER);
      sfx(WAV_GASP,pan(int(x)));
      return true;
    }
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    if(misc || clk<0) {
      tile=160;
      enemy::draw(dest);
      return;
      }

    tile=(clk>=3)?230:232;
    flip=(clk&1)?1:0;
    xofs-=8; yofs-=8;
    drawblock(dest,15);
    xofs+=8; yofs+=8;
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(wpnId==wWhistle && misc==0)
      misc=1;
    return 0;
  }
};





class eGanon : public enemy {
public:
  int Stunclk;

  eGanon(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=8;
    wdp=2;
    hp=15;
    cs=csBOSS;
    loadpalset(csBOSS,pSprite(spGANON));
    hxofs=hyofs=8;
    step=1;
    clk2=70;
    tile=260;
    misc=-1;
    mainguy=!getmapflag();
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();

    switch(misc)
    {
    case -1: misc=0;
    case 0:
      if(++clk2>72 && !(rand()&3))
      {
        addEwpn(x,y,ewFireball,1,wdp,0);
        clk2=0;
      }
      Stunclk=0;
      constant_walk(5,72,spw_none);
      break;

    case 1:
    case 2:
      if(--Stunclk<=0)
      {
        int r=rand();
        if(r&1)
        {
          y=96;
          if(r&2)
            x=160;
          else
            x=48;
          if(tooclose(x,y,48))
            x=208-x;
        }
        loadpalset(csBOSS,pSprite(spGANON));
        misc=0;
      }
      break;
    case 3:
      if(hclk>0)
        break;
      misc=4;
      clk=0;
      hxofs=1000;
      loadpalset(9,pSprite(spPILE));
      stop_midi();
      stop_sfx(WAV_ROAR);
      sfx(WAV_GASP,pan(int(x)));
      sfx(WAV_GANON);
      items.add(new item(x+8,y+8,iPile,ipDUMMY,0));
      break;

    case 4:
      if(clk>=80)
      {
        misc=5;
        if(getmapflag())
        {
          game.lvlitems[dlevel]|=liBOSS;
          return true;
        }
        sfx(WAV_CLEARED);
        items.add(new item(x+8,y+8,iBigTri,ipBIGTRI,0));
      }
      break;
    }
    return enemy::animate(index);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    switch(misc)
    {
    case 0:
      if(wpnId!=wSword)
        return 0;
      hp-=power;
      if(hp>0)
      {
        misc=1;
        Stunclk=64;
      }
      else
      {
        loadpalset(csBOSS,pSprite(spBROWN));
        misc=2;
        Stunclk=284;
        hp=15;
      }
      sfx(WAV_EHIT,pan(int(x)));
      sfx(WAV_GASP,pan(int(x)));
      return 1;

    case 2:
      if(wpnId!=wArrow || game.arrow<2)
        return 0;
      misc=3;
      hclk=81;
      loadpalset(9,pSprite(spBROWN));
      return 1;
    }
    return 0;
  }
  virtual void draw(BITMAP *dest)
  {
    switch(misc)
    {
    case 0:
      if((clk&3)==3)
        tile=(rand()%5)*2+260;
      if(db!=999)
        break;
    case 2:
      if(Stunclk<64 && (Stunclk&1))
        break;
    case -1:
    case 1:
    case 3:
      drawblock(dest,15);
      break;
    }
  }
  virtual void draw2(BITMAP *dest)
  {
    if(misc!=4)
      return;

    if(clk&1)
    {
      draw_flash(dest);
      draw_guts(dest);
    }
    else
    {
      draw_guts(dest);
      draw_flash(dest);
    }
  }
  void draw_guts(BITMAP *dest)
  {
    int c = min(clk>>3,8);
    tile = clk<24 ? 74 : 75;
    overtile16(dest,tile,x+8,y+c+56,9,0);
    overtile16(dest,tile,x+8,y+16-c+56,9,0);
    overtile16(dest,tile,x+c,y+8+56,9,0);
    overtile16(dest,tile,x+16-c,y+8+56,9,0);
    overtile16(dest,tile,x+c,y+c+56,9,0);
    overtile16(dest,tile,x+16-c,y+c+56,9,0);
    overtile16(dest,tile,x+c,y+16-c+56,9,0);
    overtile16(dest,tile,x+16-c,y+16-c+56,9,0);
  }
  void draw_flash(BITMAP *dest)
  {
    int c = clk-(clk>>2);
    cs = (frame&3)+6;
    overtile16(dest,194,x+8,y+8-clk+56,cs,0);
    overtile16(dest,194,x+8,y+8+clk+56,cs,2);
    overtile16(dest,195,x+8-clk,y+8+56,cs,0);
    overtile16(dest,195,x+8+clk,y+8+56,cs,1);
    overtile16(dest,196,x+8-c,y+8-c+56,cs,0);
    overtile16(dest,196,x+8+c,y+8-c+56,cs,1);
    overtile16(dest,196,x+8-c,y+8+c+56,cs,2);
    overtile16(dest,196,x+8+c,y+8+c+56,cs,3);
  }
};


void getBigTri()
{
  sfx(WAV_CLEARED);
  guys.clear();

  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  link.draw_under(framebuf);
  items.draw(framebuf,false);
  link.draw(framebuf);

  for(int f=0; f<24*8 && !Quit; f++)
  {
    if((f&7)==0)
    {
      for(int cs=2; cs<5; cs++)
        for(int i=1; i<16; i++)
          RAMpal[CSET(cs)+i]=_RGB(63,63,63);
      refreshpal=true;
    }
    if((f&7)==4)
      loadlvlpal(DMaps[currdmap].color);
  advanceframe();
  }

  loadpalset(9,pSprite(spPILE));
  play_DmapMidi();
  game.lvlitems[dlevel]|=liBOSS;
  setmapflag();
}




/**********************************/
/***  Multiple-Segment Enemies  ***/
/**********************************/


// segment manager
class eMoldorm : public enemy {
public:
  int segcnt;

  // tell it the segment count through the clk param
  eMoldorm(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    x=128; y=48;
    dir=(rand()&7)+8;
    step=0.5;
    superman=true;
    fading=fade_invisible;
    hxofs=1000;
    segcnt=clk;
    clk=0;
    id=guys.Count();
    yofs=56;
    tile=57;
    cs=7;
  }
  virtual bool animate(int index)
  {
    if(clk2) {
      if(--clk2 == 0) {
        leave_item();
        setmapflag(mfNEVERRET);
        return true;
        }
      }
    else {
      constant_walk_8(1,spw_floater);
      misc=dir;

      for(int i=index+1; i<index+segcnt+1; i++) {
        if(((enemy*)guys.spr(i))->hp <= 0) {
          for(int j=i; j<index+segcnt; j++) {
            swap(((enemy*)guys.spr(j))->hp,((enemy*)guys.spr(j+1))->hp);
            swap(((enemy*)guys.spr(j))->hclk,((enemy*)guys.spr(j+1))->hclk);
            }
          ((enemy*)guys.spr(i))->hclk=33;
          segcnt--;
          }
        }
      if(segcnt==0) {
        clk2=19;
        x=guys.spr(index+1)->x;
        y=guys.spr(index+1)->y;
        }
      }
    return false;
  }
};


// segment class
class esMoldorm : public enemy {
public:

  esMoldorm(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    x=128; y=48;
    hp=2;
    dp=wdp=1;
    yofs=56;
    tile=57;
    cs=8;
    hyofs=4;
    hxsz=hysz=8;
    hxofs=1000;
    step=0.5;
    item_set=0;
    mainguy=count_enemy=false;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(clk>=0) {
      hxofs=4;
      if(!(clk&15)) {
        misc=dir;
        dir=((enemy*)guys.spr(index-1))->misc;
        }
      sprite::move(step);
      }
    return enemy::animate(index);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(enemy::takehit(wpnId,power,wpnx,wpny,wpnDir))
      return 2;  // force it to wait a frame before checking sword attacks again
    return 0;
  }
  virtual void draw(BITMAP *dest)
  {
   if(clk>=0)
     enemy::draw(dest);
  }
  virtual void death_sfx()
  {
    sfx(WAV_GASP,pan(int(x)));
  }
};




// segment manager
class eLanmola : public enemy {
public:
  int segcnt;

  // tell it the segment count through the clk param
  eLanmola(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    x=64; y=80;
    dir=up;
    step=(id==eRCENT)?1:2;
    superman=true;
    fading=fade_invisible;
    hxofs=1000;
    segcnt=clk;
    clk=0;
    id=guys.Count();
    tile=173;
    cs=9;
  }
  virtual bool animate(int index)
  {
    if(clk2) {
      if(--clk2 == 0) {
        leave_item();
        return true;
        }
      return false;
      }

    constant_walk(8,0,spw_none);
    misc=dir;

    for(int i=index+1; i<index+segcnt+1; i++) {
      if(((enemy*)guys.spr(i))->hp <= 0) {
        for(int j=i; j<index+segcnt; j++) {
          swap(((enemy*)guys.spr(j))->hp,((enemy*)guys.spr(j+1))->hp);
          swap(((enemy*)guys.spr(j))->hclk,((enemy*)guys.spr(j+1))->hclk);
          }
        ((enemy*)guys.spr(i))->hclk=33;
        segcnt--;
        }
      }
    if(segcnt==0) {
      clk2=19;
      x=guys.spr(index+1)->x;
      y=guys.spr(index+1)->y;
      setmapflag(mfTMPNORET);
      }
    return enemy::animate(index);
  }
};


// segment class
class esLanmola : public enemy {
public:
  int crate;

  esLanmola(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    x=64; y=80;
    hp=2;
    dp=wdp=4;
    tile=(id<0x2000)?173:174;
    cs=(id&0xFFF)==eRCENT?8:7;
    hxofs=1000;
    hxsz=8;
    step=(id&0xFFF)==eRCENT?1:2;
    crate=(id&0xFFF)==eRCENT?7:3;
    mainguy=false;
    count_enemy=(id<0x2000)?true:false;
    item_set=0;
  }
  virtual bool animate(int index)
  {
    if(dying) {
      xofs=0;
      return Dead();
      }
    if(clk>=0) {
      hxofs=4;
      if(!(clk&crate)) {
        misc=dir;
        dir=((enemy*)guys.spr(index-1))->misc;
        }
      sprite::move(step);
      }
    return enemy::animate(index);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(enemy::takehit(wpnId,power,wpnx,wpny,wpnDir))
      return 2;  // force it to wait a frame before checking sword attacks again
    return 0;
  }
  virtual void draw(BITMAP *dest)
  {
   if(clk>=0)
     enemy::draw(dest);
  }
};




class eManhandla : public enemy {
public:
  char arm[4]; // side the arm is on
  int armcnt;

  eManhandla(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,0)
  {
    dp=wdp=2;
    hp=1;
    superman=true;
    dir=(rand()&7)+8;
    step=0.5;
    cs=7;
    tile=190;
    armcnt=4;
    for(int i=0; i<4; i++)
      arm[i]=i;
    cont_sfx(WAV_VADER);
    fading=fade_blue_poof;
  }

  virtual bool animate(int index)
  {
    if(dying)
      return Dead();

    // check arm status, move dead ones to end of group
    for(int i=0; i<armcnt; i++) {
      if(((enemy*)guys.spr(index+i+1))->dying) {
        for(int j=i; j<armcnt-1; j++) {
          swap(arm[j],arm[j+1]);
          guys.swap(index+j+1,index+j+2);
          }
        armcnt--;
        }
      }
    // move or die
    if(armcnt==0) {
      setmapflag(mfNEVERRET);
      hp=0;
      }
    else {
      step=(5-armcnt)*0.5;
      int dx1=0, dy1=-8, dx2=15, dy2=15;
      for(int i=0; i<armcnt; i++) {
        switch(arm[i]) {
         case 0: dy1=-24; break;
         case 1: dy2=31;  break;
         case 2: dx1=-16; break;
         case 3: dx2=31;  break;
         }
        }
      variable_walk_8(2,16,spw_floater,dx1,dy1,dx2,dy2);
      for(int i=0; i<armcnt; i++) {
        fix dx=0,dy=0;
        switch(arm[i]) {
         case 0: dy=-16; break;
         case 1: dy=16;  break;
         case 2: dx=-16; break;
         case 3: dx=16;  break;
         }
        guys.spr(index+i+1)->x = x+dx;
        guys.spr(index+i+1)->y = y+dy;
        }
      }
    return enemy::animate(index);
  }

  virtual void death_sfx()
  {
    stop_sfx(WAV_VADER);
    sfx(WAV_GASP,pan(int(x)));
  }

  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying)
      return 0;
    switch(wpnId) {
    case wLitBomb:
    case wBait:
    case wWhistle:
    case wFire:
    case wWind:
      return 0;
    case wBrang: sfx(WAV_CHINK,pan(int(x))); break;
    default:     sfx(WAV_EHIT,pan(int(x)));
    }
    return 1;
  }
};




class esManhandla : public enemy {
public:

  // use clk param to tell which arm it is
  esManhandla(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    id=misc=clk;
    clk=0;
    dp=wdp=2;
    hp=4;
    cs=7;
    mainguy=count_enemy=false;
    item_set=0;
    switch(misc) {
    case down: flip=2;
    case up: break;
    case right: flip=1;
    case left: break;
    }
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    if(--clk2<=0) {
      clk2=unsigned(rand())%5+5;
      clk3^=1;
      }
    if(!(rand()&127))
      addEwpn(x,y,ewFireball,1,wdp,0);
    return enemy::animate(index);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(dying || hclk>0)
      return 0;
    switch(wpnId) {
    case wLitBomb:
    case wBait:
    case wWhistle:
    case wFire:
    case wWind:
      return 0;
    case wBrang: sfx(WAV_CHINK,pan(int(x))); break;
    default:
      hp-=power;
      hclk=33;
      sfx(WAV_EHIT,pan(int(x)));
      sfx(WAV_GASP,pan(int(x)));
    }
    return 1;
  }

  virtual void draw(BITMAP *dest)
  {
    switch(misc) {
    case up:
    case down: tile=(clk3)?188:189; break;
    case left:
    case right: tile=(clk3)?186:187; break;
    }
    enemy::draw(dest);
  }
};




class eGleeok : public enemy {
public:

  eGleeok(fix X,fix Y,int Id,int Clk) : enemy(120,48,Id,Clk)
  {
    misc=clk;  // total head count
    clk3=clk;  // live head count
    clk=0;
    clk2=60;   // fire ball clock
    dp=wdp=2;
    hp=(misc-1)*6+10;
    cs=csBOSS;
    loadpalset(csBOSS,pSprite(spGLEEOK));
    hxofs=4;
    hxsz=8;
    frate=17*4;
    cont_sfx(WAV_ROAR);
    fading=fade_blue_poof;
  }

  virtual bool animate(int index)
  {
    if(dying)
      return Dead();

    for(int i=0; i<clk3; i++) {
      enemy *head = ((enemy*)guys.spr(index+i+1));
      if(head->hclk) {
        if(hclk==0) {
          hp -= 1000 - head->hp;
          hclk = 33;
          sfx(WAV_GASP);
          sfx(WAV_EHIT,pan(int(head->x)));
          }
        head->hp = 1000;
        head->hclk = 0;
        }
      }

    if(hp<=(clk3-1)*6) {
      ((enemy*)guys.spr(index+clk3))->misc = -1;  // give signal to fly off
      hp=(--clk3)*6;
      }

    if(++clk2>72 && !(rand()&3)) {
      int i=rand()%misc;
      enemy *head = ((enemy*)guys.spr(index+i+1));
      addEwpn(head->x,head->y,ewFireball,1,wdp,0);
      clk2=0;
      }

    if(hp<=0) {
      for(int i=0; i<misc; i++)
        ((enemy*)guys.spr(index+i+1))->misc = -2;  // give the signal to disappear
      setmapflag(mfNEVERRET);
      }
    return enemy::animate(index);
  }

  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    return 0;
  }

  virtual void death_sfx()
  {
    stop_sfx(WAV_ROAR);
    sfx(WAV_GASP,pan(int(x)));
  }

  virtual void draw(BITMAP *dest)
  {
    if(dying) {
      enemy::draw(dest);
      return;
      }

    int f=clk/17;

    // body
    xofs=-8; yofs=32;
    switch(f) {
    case 0: tile=220; break;
    case 2: tile=224; break;
    default: tile=222; break;
    }
    enemy::drawblock(dest,15);
  }
  virtual void draw2(BITMAP *dest)
  {
    // the neck stub
    xofs=0;
    yofs=56;
    tile=145;
    if(hp>0)
      sprite::draw(dest);
  }
};


// head class
class esGleeok : public enemy {
public:
  int nx[4],ny[4];

  esGleeok(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    x=120; y=70;
    hxofs=4;
    hxsz=8;
    yofs=56;
    clk2=clk;    // how long to wait before moving first time
    clk=0;
    hp=1000;
    dp=wdp=2;
    tile=146;
    cs=csBOSS;
    step=1;
    mainguy=count_enemy=false;
    item_set=0;
    dir=rand();
    clk3=((dir&2)>>1)+2; // left or right
    dir&=1;              // up or down
    for(int i=0; i<4; i++) {
      nx[i]=124;
      ny[i]=i*6+48;
      }
  }

  virtual bool animate(int index)
  {
    switch(misc) {
    case 0:    // live head
      if(++clk2>=0 && !(clk2&3)) {
        if(y<=56) dir=down;
        if(y>=80) dir=up;
        if(y<=58 && !(rand()&31))  // y jig
          dir^=1;
        sprite::move(step);
        if(clk2>=4) {
          clk3^=1;
          clk2=-4;
          }
        else {
          if(x<=96) clk3=right;
          if(x>=144) clk3=left;
          if(y<=72 && !(rand()&15))
            clk3^=1;               // x jig
          else {
            if(y<=64 && !(rand()&31))
              clk3^=1;             // x switch back
            clk2=-4;
            }
          }
        swap(dir,clk3);
        sprite::move(step);
        swap(dir,clk3);

        for(int i=1; i<4; i++) {
          nx[i] = ( ( (4-i)*120 + i*x ) >>2 ) + rand()%3 + 3;
          ny[i] = ( ( (4-i)*48 + i*y ) >>2 ) + ((i>0) ? rand()%3 - 1 : 0);
          }
        }
      break;
    case 1:    // flying head
      if(clk>=0) {
        variable_walk_8(2,9,spw_floater);
//        if(++clk2>=80 && !(rand()%63)) {
//          addEwpn(x,y,ewFireball,1,wdp,0);
//          clk2=0;
//          }
        }
      break;
    // the following are messages sent from the main guy...
    case -1: { // got chopped off
      misc=1;
      superman=true;
      hxofs=xofs=0;
      hxsz=16;
      cs=8;
      clk=-24;
      clk2=40;
      dir=(rand()&7)+8;
      step=8.0/9.0;
      } break;
    case -2:   // the big guy is dead
      return true;
    }
    return enemy::animate(index);
  }

  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(clk<0 || hclk>0 || superman)
      return 0;
    switch(wpnId) {
    case wArrow:
    case wMagic:
    case wBrang:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    case wWand:
    case wBeam:
    case wSword:
      hp-=power;
      hclk=33;
      return 2;  // force it to wait a frame before checking sword attacks again
    }
    return 0;
  }
  virtual void draw(BITMAP *dest)
  {
    switch(misc) {
    case 0:
      for(int i=1; i<4; i++)
        overtile16(dest,145,nx[i]-4,ny[i]+56,cs,0);
      break;
    case 1:
      tile=(clk&1)?147:148;
      break;
    }
  }
  virtual void draw2(BITMAP *dest)
  {
    enemy::draw(dest);
  }
};




class ePatra : public enemy {
public:
  int flycnt,loopcnt;

  ePatra(fix X,fix Y,int Id,int Clk) : enemy(128,48,Id,Clk)
  {
    dp=wdp=4;
    hp=11;
    cs=7;
    dir=(rand()&7)+8;
    step=0.25;
    tile=191;
    cont_sfx(WAV_VADER);
    flycnt=8;
    loopcnt=0;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    variable_walk_8(3,8,spw_floater);
    if(++clk2>84)
    {
      clk2=0;
      if(loopcnt)
        loopcnt--;
      else
      {
        if(id==ePATRA1)
        {
          if((misc%6)==0)
            loopcnt=3;
        }
        else
        {
          if((misc&7)==0)
            loopcnt=4;
        }
      }
      misc++;
    }
    for(int i=index+1; i<index+flycnt+1; i++)
    {
      if(((enemy*)guys.spr(i))->hp <= 0)
      {
        for(int j=i; j<index+flycnt; j++)
          guys.swap(j,j+1);
        flycnt--;
      }
      else
      {
        int pos = ((enemy*)guys.spr(i))->misc;
        double a = (clk2-pos*10.5)*PI/42;

        if(id==ePATRA1)
        {
          if(loopcnt>0)
          {
            guys.spr(i)->x =  cos(a+PI/2)*56 - sin(pos*PI/4)*28;
            guys.spr(i)->y = -sin(a+PI/2)*56 + cos(pos*PI/4)*28;
          }
          else
          {
            guys.spr(i)->x =  cos(a+PI/2)*28;
            guys.spr(i)->y = -sin(a+PI/2)*28;
          }
        }
        else
        {
          if(loopcnt>0)
          {
            guys.spr(i)->x =  cos(a+PI/2)*42;
            guys.spr(i)->y = (-sin(a+PI/2)-cos(pos*PI/4))*21;
          }
          else
          {
            guys.spr(i)->x =  cos(a+PI/2)*42;
            guys.spr(i)->y = -sin(a+PI/2)*42;
          }
        }
        guys.spr(i)->x += x;
        guys.spr(i)->y += y;
      }
    }
    if(hp<=0)
      setmapflag(mfNEVERRET);
    return enemy::animate(index);
  }
  virtual void draw(BITMAP *dest)
  {
    flip = (clk&2)>>1;
    enemy::draw(dest);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(clk<0 || hclk>0 || superman)
      return 0;
    switch(wpnId) {
    case wArrow:
    case wMagic:
    case wBrang:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    case wWand:
    case wBeam:
    case wSword:
      if(flycnt)
        return 0;
      hp-=power;
      hclk=33;
      sfx(WAV_EHIT,pan(int(x)));
      sfx(WAV_GASP,pan(int(x)));
      return 1;
    }
    return 0;
  }
  virtual void death_sfx()
  {
    stop_sfx(WAV_VADER);
    sfx(WAV_GASP,pan(int(x)));
  }
};


// segment class
class esPatra : public enemy {
public:

  esPatra(fix X,fix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    misc=clk;
    clk = -((misc*21)>>1)-1;
    hp=6;
    dp=wdp=4;
    yofs=56;
    cs=8;
    hyofs=2;
    hxsz=hysz=12;
    hxofs=1000;
    item_set=0;
    mainguy=count_enemy=false;
  }
  virtual bool animate(int index)
  {
    if(dying)
      return Dead();
    hxofs=4;
    return enemy::animate(index);
  }
  virtual int takehit(int wpnId,int power,int wpnx,int wpny,int wpnDir)
  {
    if(clk<0 || hclk>0 || superman)
      return 0;
    switch(wpnId) {
    case wArrow:
    case wMagic:
    case wBrang:
      sfx(WAV_CHINK,pan(int(x)));
      return 1;
    case wWand:
    case wBeam:
    case wSword:
      hp-=power;
      hclk=33;
      sfx(WAV_EHIT,pan(int(x)));
      return 1;
    }
    return 0;
  }
  virtual void draw(BITMAP *dest)
  {
   tile = (clk&2) ? 192 : 193;
   if(clk>=0)
     enemy::draw(dest);
  }
};





/**********************************/
/**********  Misc Code  ***********/
/**********************************/


inline void addEwpn(int x,int y,int id,int type,int power,int dir)
{
 Ewpns.add(new weapon(x,y,id,type,power,dir));
}

inline int enemy_dp(int index)
{
 return (((enemy*)guys.spr(index))->dp)<<2;
}

inline int ewpn_dp(int index)
{
 return (((weapon*)Ewpns.spr(index))->power)<<2;
}

inline int hit_enemy(int index,int wpnId,int power,int wpnx,int wpny,int dir)
{
 return ((enemy*)guys.spr(index))->takehit(wpnId,power,wpnx,wpny,dir);
}

inline void enemy_scored(int index)
{
 ((enemy*)guys.spr(index))->scored=true;
}

inline void addguy(int x,int y,int id,int clk,bool mainguy)
{
 guys.add(new guy(x,y+(dungeon()?1:0),id,clk,mainguy));
}

inline void additem(int x,int y,int id,int pickup)
{
 items.add(new item(x,y,id,pickup,0));
}

inline void additem(int x,int y,int id,int pickup,int clk)
{
 items.add(new item(x,y,id,pickup,clk));
}

void kill_em_all()
{
  for(int i=0; i<guys.Count(); i++)
    ((enemy*)guys.spr(i))->kickbucket();
}


// For Link's hit detection. Don't count them if they are stunned.
int GuyHit(int tx,int ty,int txsz,int tysz)
{
  for(int i=0; i<guys.Count(); i++)
    if(guys.spr(i)->hit(tx,ty,txsz,tysz))
      if(((enemy*)guys.spr(i))->stunclk==0)
        return i;
  return -1;
}

// For Link's hit detection. Count them if they are dying.
int GuyHit(int index,int tx,int ty,int txsz,int tysz)
{
  enemy *e = (enemy*)guys.spr(index);
  if(e->hp > 0)
    return -1;

  bool d = e->dying;
  int hc = e->hclk;
  e->dying = false;
  e->hclk = 0;
  bool hit = e->hit(tx,ty,txsz,tysz);
  e->dying = d;
  e->hclk = hc;

  return hit ? index : -1;
}


bool hasMainGuy()
{
 for(int i=0; i<guys.Count(); i++)
  if(((enemy*)guys.spr(i))->mainguy)
    return true;
 return false;
}

void EatLink(int index)
{
  ((eLikeLike*)guys.spr(index))->eatlink();
}

void GrabLink(int index)
{
  ((eWallM*)guys.spr(index))->grablink();
}

bool CarryLink()
{
  for(int i=0; i<guys.Count(); i++)
    if(guys.spr(i)->id==eWALLM)
      if(((eWallM*)guys.spr(i))->haslink)
      {
        link.x=guys.spr(i)->x;
        link.y=guys.spr(i)->y;
        return ((eWallM*)guys.spr(i))->misc > 0;
      }
  return false;
}

void movefairy(fix &x,fix &y,int misc)
{
  int i = guys.idFirst(eITEMFAIRY+0x1000+misc);
  x = guys.spr(i)->x;
  y = guys.spr(i)->y;
}


void killfairy(int misc)
{
  int i = guys.idFirst(eITEMFAIRY+0x1000+misc);
  guys.del(i);
}



void addenemy(int x,int y,int id,int clk)
{
 sprite *e;
 switch(id) {
 case eROCTO1:
 case eROCTO2:
 case eBOCTO1:
 case eBOCTO2:    e = new eOctorok(x,y,id,clk); break;
 case eRMOLBLIN:
 case eBMOLBLIN:  e = new eMolblin(x,y,id,clk); break;
 case eRLYNEL:
 case eBLYNEL:    e = new eLynel(x,y,id,clk); break;
 case eRLEV:
 case eBLEV:      e = new eLeever(x,y,id,clk); break;
 case eRTEK:
 case eBTEK:      e = new eTektite(x,y,id,clk); break;
 case ePEAHAT:    e = new ePeahat(x,y,id,clk); break;
 case eZORA:      e = new eZora(x,y,id,clk); break;
 case eROCK:      e = new eRock(x,y,id,clk); break;
 case eARMOS:     e = new eArmos(x,y,id,clk); break;
 case eGHINI1:
 case eGHINI2:    e = new eGhini(x,y,id,clk); break;
 case eKEESE1:
 case eKEESE2:
 case eKEESE3:    e = new eKeese(x,y,id,clk); break;
 case eTRAP:      e = new eTrap(x,y,id,clk); break;
 case eSTALFOS:
 case eSTALFOS2:  e = new eStalfos(x,y,id,clk); break;
 case eGEL:       e = new eGel(x,y,id,clk); break;
 case eZOL:       e = new eZol(x,y,id,clk); break;
 case eROPE:
 case eROPE2:     e = new eRope(x,y,id,clk); break;
 case eVIRE:      e = new eVire(x,y,id,clk); break;
 case eBBUBBLE:
 case eRBUBBLE:
 case eBUBBLE:    e = new eBubble(x,y,id,clk); break;
 case eLIKE:      e = new eLikeLike(x,y,id,clk); break;
 case eGIBDO:     e = new eGibdo(x,y,id,clk); break;
 case ePOLSV:     e = new ePolsVoice(x,y,id,clk); break;
 case eRGORIYA:
 case eBGORIYA:   e = new eGoriya(x,y,id,clk); break;
 case eRDKNUT:
 case eBDKNUT:    e = new eDarknut(x,y,id,clk); break;
 case eRWIZ:
 case eBWIZ:      e = new eWizzrobe(x,y,id,clk); break;
 case eFBALL:     e = new eFBall(x,y,id,clk); break;
 case eWALLM:     e = new eWallM(x,y,id,clk); break;
 case eDODONGO:   e = new eDodongo(x,y,id,clk); break;
 case eAQUAM:     e = new eAquamentus(x,y,id,clk); break;
 case eMOLDORM:   e = new eMoldorm(x,y,id,5); break;
 case eMANHAN:    e = new eManhandla(x,y,id,clk); break;
 case eGLEEOK2:   e = new eGleeok(x,y,id,2); break;
 case eGLEEOK3:   e = new eGleeok(x,y,id,3); break;
 case eGLEEOK4:   e = new eGleeok(x,y,id,4); break;
 case eDIG1:
 case eDIG3:      e = new eBigDig(x,y,id,clk); break;
 case eRGOHMA:
 case eBGOHMA:    e = new eGohma(x,y,id,clk); break;
 case eRCENT:
 case eBCENT:     e = new eLanmola(x,y,id,5); break;
 case ePATRA1:
 case ePATRA2:    e = new ePatra(x,y,id,clk); break;
 case eGANON:     e = new eGanon(x,y,id,clk); break;
 case eITEMFAIRY: e = new eItemFairy(x,y,id+clk,0); break;
 case eFIRE:      e = new eFire(x,y,id,clk); break;
 default:         e = new enemy(x,y,-1,clk); break;
 }
 guys.add(e);

 // add segments of segmented enemies
 int c=0;
 switch(id) {

 case eMOLDORM:
   for(int i=0; i<5; i++)
     guys.add(new esMoldorm(x,y,id+0x1000,-(i<<4)));
   break;

 case eRCENT:
 case eBCENT: {
   int shft = (id==eRCENT)?3:2;
   guys.add(new esLanmola(x,y,id+0x1000,0));
   for(int i=1; i<5; i++)
     guys.add(new esLanmola(x,y,id+0x2000,-(i<<shft)));
   } break;

 case eMANHAN:
   for(int i=0; i<4; i++)
     guys.add(new esManhandla(x,y,id+0x1000,i));
   break;

 case eGLEEOK4: guys.add(new esGleeok(x,y,id+0x1000,c)); c-=48;
 case eGLEEOK3: guys.add(new esGleeok(x,y,id+0x1000,c)); c-=48;
 case eGLEEOK2: guys.add(new esGleeok(x,y,id+0x1000,c)); c-=48;
                guys.add(new esGleeok(x,y,id+0x1000,c));
                break;

 case ePATRA1:
 case ePATRA2:
   for(int i=0; i<8; i++)
     guys.add(new esPatra(x,y,id+0x1000,i));
   break;
 }
}


bool checkpos(int id)
{
 switch(id) {
 case eRTEK:
 case eBTEK: return false;
 }
 return true;
}


void addfires()
{
 addguy(72,64,gFIRE,-17,false);
 addguy(168,64,gFIRE,-18,false);
}




void loadguys()
{
if(loaded_guys)
 return;

loaded_guys=true;

byte Guy=0,Item=0;
repaircharge=false;
itemindex=-1;
hasitem=0;

if(currscr>=128)
  Guy=tmpscr[1].guy;
else {
  Guy=tmpscr[0].guy;
  if(dlevel==0) {
    if(Guy==gFAIRY) {
      sfx(WAV_SCALE1);
      addguy(120,62,gFAIRY,-14,false);
      return;
      }
    else Guy=0;
    }
  }
if(tmpscr[0].room==rZELDA)
{
  addguy(120,72,Guy,-15,true);
  guys.spr(0)->hxofs=1000;
  addenemy(128,96,eFIRE,-15);
  addenemy(112,96,eFIRE,-15);
  addenemy(96,120,eFIRE,-15);
  addenemy(144,120,eFIRE,-15);
  return;
}
if(Guy) {
  addfires();
  if(currscr>=128)
    if(getmapflag())
      Guy=0;
  switch(tmpscr[0].room) {
   case rSP_ITEM:
   case rGRUMBLE:
   case rBOMBS:
   case rSWINDLE:
     if(getmapflag())
       Guy=0;
     break;
   case rTRIFORCE:
     if(TriforceCount()==8)
       Guy=0;
     break;
   }
  if(Guy) {
    blockpath=true;
    if(currscr<128)
      sfx(WAV_SCALE1);
    addguy(120,64,Guy,dlevel?-15:startguy[rand()&7],true);
    link.Freeze();
    }
  }

if(currscr<128) {
  Item=tmpscr[0].item;
  if(getmapflag())
    Item=0;
  if(Item) {
    if(tmpscr[0].flags&fITEM)
      hasitem=1;
    else if(tmpscr[0].enemyflags&efCARRYITEM)
      hasitem=2;
    else
      additem(tmpscr[0].itemx,tmpscr[0].itemy+1,Item,ipONETIME+ipBIGRANGE+
        ((Item==iTriforce) ? ipHOLDUP : 0));
    }
  }
else if(dlevel) {
  Item=tmpscr[1].catchall;
  if(getmapflag())
    Item=0;
  if(Item)
    additem(120,81,Item,ipBIGRANGE+ipONETIME+ipHOLDUP);
  }

if(tmpscr[0].room==r10RUPIES && !getmapflag()) {
  setmapflag();
  for(int i=0; i<10; i++)
    additem(ten_rupies_x[i],ten_rupies_y[i],0,ipBIGRANGE,-14);
  }
}




bool hasBoss()
{
 for(int i=0; i<guys.Count(); i++)
 {
  switch(guys.spr(i)->id)
  {
  case eGLEEOK2:
  case eGLEEOK3:
  case eGLEEOK4:
  case eMOLDORM:
  case eMANHAN:
  case eRCENT:
  case eBCENT:
  case ePATRA1:
  case ePATRA2:
    return true;
  }
 }
 return false;
}


bool slowguy(int id)
{
 switch(id) {
 case eROCTO1:
 case eBOCTO1:
 case eROCTO2:
 case eBOCTO2:
 case eRLEV:
 case eBLEV:
 case eROCK:
   return true;
 }
 return false;
}


bool countguy(int id)
{
 switch(id) {
 case eROCK:
 case eZORA:
 case eTRAP:
   return false;
 }
 return true;
}


bool ok2add(int id)
{
 switch(id) {
 case eARMOS:
 case eGHINI2:
 case eTRAP:
 case eGANON:
   return false;
 case eGLEEOK2:
 case eGLEEOK3:
 case eGLEEOK4:
 case eMOLDORM:
 case eMANHAN:
 case eAQUAM:
 case eRGOHMA:
 case eBGOHMA:
 case eDIG1:
 case eDIG3:
 case ePATRA1:
 case ePATRA2:
   return !getmapflag(mfNEVERRET);
 }
 if(!get_bit(QHeader.rules,qrNOTMPNORET))
   return !getmapflag(mfTMPNORET);
 return true;
}




void load_default_enemies()
{
  wallm_load_clk=frame-80;

  if(tmpscr[0].enemyflags&efZORA)
    addenemy(0,0,eZORA,0);

  if(tmpscr[0].enemyflags&efTRAP4)
  {
    addenemy(32,32,eTRAP,-14);
    addenemy(208,32,eTRAP,-14);
    addenemy(32,128,eTRAP,-14);
    addenemy(208,128,eTRAP,-14);
  }

  if(tmpscr[0].enemyflags&efTRAP2)
  {
    addenemy(64,80,eTRAP,-14);
    addenemy(176,80,eTRAP,-14);
  }

  if(tmpscr[0].enemyflags&efROCKS)
  {
    addenemy(0,0,eROCK,-14);
    addenemy(0,0,eROCK,-14);
    addenemy(0,0,eROCK,-14);
  }

  if(tmpscr[0].enemyflags&efFIREBALLS)
   for(int y=0; y<176; y+=16)
     for(int x=0; x<256; x+=16)
     {
       int ctype = combobuf[MAPCOMBO(x,y)].type;
       if(ctype==cL_STATUE)
         addenemy(x+4,y+7,eFBALL,24);
       if(ctype==cR_STATUE)
         addenemy(x-8,y-1,eFBALL,24);
     }
}



int sle_x,sle_y,sle_cnt,sle_clk;

void nsp()
// moves sle_x and sle_y to the next position
{
  if(sle_x==0)
  {
    if(sle_y<160)
      sle_y+=16;
    else
      sle_x+=16;
  }
  else if(sle_y==160)
  {
    if(sle_x<240)
      sle_x+=16;
    else
      sle_y-=16;
  }
  else if(sle_x==240)
  {
    if(sle_y>0)
      sle_y-=16;
    else
      sle_x-=16;
  }
  else if(sle_y==0)
  {
    if(sle_x>0)
      sle_x-=16;
    else
      sle_y+=16;
  }
}


int next_side_pos()
// moves sle_x and sle_y to the next available position
// returns the direction the enemy should face
{
  bool blocked;
  int c=0;
  do {
    nsp();
    blocked = _walkflag(sle_x,sle_y,2) || _walkflag(sle_x,sle_y+8,2);
    if(++c>50)
      return -1;
  } while(blocked);

  int dir=0;
  if(sle_x==0)    dir=right;
  if(sle_y==0)    dir=down;
  if(sle_x==240)  dir=left;
  if(sle_y==168)  dir=up;
  return dir;
}


int vhead=0;


void side_load_enemies()
{
  if(sle_clk==0)
  {
    sle_cnt = 0;
    int guycnt = 0;
    short s = (currmap<<7)+currscr;
    bool beenhere=false;
    bool reload=true;

    load_default_enemies();

    for(int i=0; i<6; i++)
      if(visited[i]==s)
        beenhere=true;

    if(!beenhere)
    {
      visited[vhead]=s;
      vhead = (vhead+1)%6;
    }
    else if(game.guys[s]==0)
    {
      sle_cnt=0;
      reload=false;
    }

    if(reload)
    {
      sle_cnt = game.guys[s];
      if(sle_cnt==0)
      {
        while(sle_cnt<10 && tmpscr[0].enemy[sle_cnt]!=0)
          sle_cnt++;
      }
    }

    for(int i=0; !countguy(tmpscr[0].enemy[i]) && sle_cnt<10; i++)
      sle_cnt++;

    for(int i=0; i<sle_cnt; i++)
      if(countguy(tmpscr[0].enemy[i]))
        guycnt++;

    game.guys[s] = guycnt;
  }

  if( (++sle_clk+8)%24 == 0)
  {
    int dir = next_side_pos();
    if(dir==-1 || tooclose(sle_x,sle_y,32))
      return;
    addenemy(sle_x,sle_y,tmpscr[0].enemy[--sle_cnt],0);
    guys.spr(guys.Count()-1)->dir = dir;
  }

  if(sle_cnt<=0)
    loaded_enemies=true;
}





void loadenemies()
{
  if(loaded_enemies)
    return;

  if(tmpscr[0].pattern==1)  // enemies enter from sides
  {
    side_load_enemies();
    return;
  }

loaded_enemies=true;

// do enemies that are always loaded
load_default_enemies();


// dungeon basements

static byte dngn_enemy_x[4] = {32,96,144,208};

if(currscr>=128) {
  if(dlevel==0) return;
  for(int i=0; i<4; i++)
    addenemy(dngn_enemy_x[i],96,tmpscr[0].enemy[i]?tmpscr[0].enemy[i]:eKEESE1,-14-i);
  return;
  }

// check if it's been long enough to reload all enemies

int loadcnt = 10;
short s = (currmap<<7)+currscr;
bool beenhere=false;
bool reload=true;

for(int i=0; i<6; i++)
  if(visited[i]==s)
    beenhere=true;

if(!beenhere)
{
  visited[vhead]=s;
  vhead = (vhead+1)%6;
}
else if(game.guys[s]==0)
{
  loadcnt=0;
  reload=false;
}

if(reload)
{
  loadcnt = game.guys[s];
  if(loadcnt==0)
    loadcnt=10;
}

for(int i=0; !countguy(tmpscr[0].enemy[i]) && loadcnt<10; i++)
  loadcnt++;


// check if it's the dungeon boss and it has been beaten before
if(tmpscr[0].enemyflags&efBOSS && game.lvlitems[dlevel]&liBOSS)
  return;


// load enemies

int set=loadside*9;
int pos=rand()%9;
int clk=-15,x,y,fastguys=0;
int i=0,guycnt=0;

for( ; i<loadcnt && tmpscr[0].enemy[i]>0; i++) {
  int t=0;
  do {
    if(pos>=9)
      pos-=9;
    x=stx0[set+pos];
    y=sty0[set+pos];
    pos++;
    t++;
    } while(checkpos(tmpscr[0].enemy[i]) && (_walkflag(x,y+8,2) || tooclose(x,y,40)) && t<11);

  int c=clk;
  if(!slowguy(tmpscr[0].enemy[i]))
    fastguys++;
  else if(fastguys>0)
    c=-15*(i-fastguys+2);
  else
    c=-15*(i+1);

  if(ok2add(tmpscr[0].enemy[i])) {
    addenemy(x,y,tmpscr[0].enemy[i],c);
    if(countguy(tmpscr[0].enemy[i]))
      guycnt++;
    }
  else
    loadcnt++;

  if(i==0 && tmpscr[0].enemyflags&efLEADER) {
    int index = guys.idFirst(tmpscr[0].enemy[i]);
    ((enemy*)guys.spr(index))->leader = true;
    }
  if(i==0 && hasitem==2) {
    int index = guys.idFirst(tmpscr[0].enemy[i]);
    ((enemy*)guys.spr(index))->itemguy = true;
    }

  clk--;
  }
game.guys[s] = guycnt;
}




void moneysign()
{
 additem(48,108,iRupy,ipDUMMY);
 textout(scrollbuf,zfont,"X",64,112,CSET(0)+1);
}


void putprices(bool sign)
{
 // refresh what's under the prices
 for(int i=5; i<12; i++)
   putcombo(scrollbuf,i<<4,112,tmpscr[0].data[112+i]);

 int step=32;
 int x=80;
 if(prices[2]==0)
   step<<=1;
 if(prices[1]==0)
   x=112;
 for(int i=0; i<3; i++)
  if(prices[i]) {
    char buf[8];
    sprintf(buf,sign?"%+3d":"%3d",prices[i]);
    int l=strlen(buf);
    text_mode(-1);
    textout(scrollbuf,zfont,buf,x-(l>3?(l-3)<<3:0),112,CSET(0)+1);
    x+=step;
    }
}



void setupscreen()
{
int t=currscr<128?0:1;
byte str=tmpscr[t].str;

for(int i=0; i<3; i++)
  prices[i]=0;

switch(tmpscr[t].room) {
 case rSP_ITEM:  // special item
  additem(120,89,tmpscr[t].catchall,ipONETIME+ipHOLDUP+ipCHECK);
  break;

 case rINFO:  // pay for info
  for(int i=0; i<3;i++)
    prices[i] = -(QMisc.info[tmpscr[t].catchall].price[i]);
  moneysign();
  additem(88,89,iRupy,ipMONEY+ipDUMMY);
  additem(120,89,iRupy,ipMONEY+ipDUMMY);
  additem(152,89,iRupy,ipMONEY+ipDUMMY);
  break;

 case rMONEY:  // secret money
  additem(120,89,iRupy,ipONETIME+ipDUMMY+ipMONEY);
  break;

 case rGAMBLE:  // gambling
  prices[0]=prices[1]=prices[2]=-10;
  moneysign();
  additem(88,89,iRupy,ipMONEY+ipDUMMY);
  additem(120,89,iRupy,ipMONEY+ipDUMMY);
  additem(152,89,iRupy,ipMONEY+ipDUMMY);
  break;

 case rREPAIR:  // door repair
  setmapflag();
  repaircharge=true;
  break;

 case rRP_HC:  // heart container or red potion
  additem(88,89,iRPotion,ipONETIME+ipHOLDUP+ipFADE);
  additem(152,89,iHeartC,ipONETIME+ipHOLDUP+ipFADE);
  break;

 case rP_SHOP:  // potion shop
  if(game.letter&2) {
    moneysign();
    prices[0]=40;
    prices[1]=68;
    additem(88,89,iBPotion,ipHOLDUP+ipCHECK+ipFADE);
    additem(152,89,iRPotion,ipHOLDUP+ipCHECK+ipFADE);
    }
  else
    str=0;
  break;

 case rSHOP: {  // shop
  moneysign();
  for(int i=0; i<3; i++) {
    additem((i<<5)+88,89, QMisc.shop[tmpscr[t].catchall].item[i], ipHOLDUP+ipCHECK+ipFADE);
    prices[i] = QMisc.shop[tmpscr[t].catchall].price[i];
    }
  } break;

 case rBOMBS:  // more bombs
  additem(120,89,iRupy,ipDUMMY+ipMONEY);
  prices[0]=-tmpscr[t].catchall;
  break;

 case rSWINDLE:  // leave heart container or money
  additem(88,89,iHeartC,ipDUMMY+ipMONEY);
  additem(152,89,iRupy,ipDUMMY+ipMONEY);
  prices[0]=-1;
  prices[1]=-tmpscr[t].catchall;
  break;
 }

putprices(false);

if(str) {
  msgstr=str;
  msgclk=msgpos=0;
  }
else
 link.unfreeze();
}



bool moneysfx=false;


void putmsg()
{
 if(!msgstr || msgpos>=72 || (msgclk++)%6<5)
   return;

 if(msgpos == 0) {
   while(MsgStrings[msgstr].s[msgpos]==' ')
     msgpos++;
   }

// if(!moneysfx)
//   cont_sfx(WAV_MSG);
 sfx(WAV_MSG);

 text_mode(-1);
 textprintf(scrollbuf,zfont,((msgpos%24)<<3)+32,((msgpos/24)<<3)+40,CSET(0)+1,
   "%c",MsgStrings[msgstr].s[msgpos]);
 text_mode(0);

 msgpos++;

 if(MsgStrings[msgstr].s[msgpos]==' ' && MsgStrings[msgstr].s[msgpos+1]==' ')
   while(MsgStrings[msgstr].s[msgpos]==' ')
     msgpos++;

 if(msgpos>=72) {
   link.unfreeze();
//   adjust_sfx(WAV_MSG,128,false);
   if(repaircharge)
     game.drupy-=tmpscr[1].catchall;
   }
}


void domoney()
{
 if(game.drupy==0) {
   moneysfx=false;
   return;
   }
 if(frame&1) {
  if(game.drupy>0) {
    if(game.rupies<255) {
      game.rupies++;
      game.drupy--;
      }
    else game.drupy=0;
    }
  else {
    if(game.rupies>0) {
      game.rupies--;
      game.drupy++;
      }
    else game.drupy=0;
    }
  moneysfx=true;
  }
 if(moneysfx)
  sfx(WAV_MSG);
}




/***  Collision detection & handling  ***/


void check_collisions()
{
 for(int i=0; i<Lwpns.Count(); i++) {
   weapon *w = (weapon*)Lwpns.spr(i);
   if(!(w->Dead())) {
     for(int j=0; j<guys.Count(); j++) {
       if(((enemy*)guys.spr(j))->hit(w)) {
         int h = ((enemy*)guys.spr(j)) -> takehit(w->id,w->power,w->x,w->y,w->dir);
         if(h)
           w->onhit(false);
         if(h==2)
           break;
         }
       if(w->Dead())
         break;
       }
     if(w->id == wBrang || w->id == wArrow)
       for(int j=0; j<items.Count(); j++)
         if(items.spr(j)->hit(w))
           if(((item*)items.spr(j))->pickup & ipTIMER)
            if(((item*)items.spr(j))->clk2 >= 32) {
             getitem(items.spr(j)->id);
             items.del(j);
             j--;
             }
     }
   }
}


// messy code to do the enemy-carrying-the-item thing


int guyindex=0;

void roaming_item()
{
  if(hasitem!=2 || !loaded_enemies)
    return;

  if(guys.Count()==0) {  // Only case is when you clear al the guys and
    return;              // leave w/out getting item, then return to room.
    }                    // We'll let LinkClass::checkspecial() handle it.

  if(itemindex==-1) {
    int Item=tmpscr[0].item;
    if(getmapflag())
      Item=0;
    if(Item) {
      itemindex=items.Count();
      additem(0,0,Item,ipONETIME+ipBIGRANGE+((Item==iTriforce) ? ipHOLDUP : 0));
      }
    else
      hasitem=0;

    for(int i=0; i<guys.Count(); i++)
      if(((enemy*)guys.spr(i))->itemguy)
        guyindex=i;
    }

  items.spr(itemindex)->x = guys.spr(guyindex)->x;
  items.spr(itemindex)->y = guys.spr(guyindex)->y - 2;
}


/*** end of guys.cc ***/
