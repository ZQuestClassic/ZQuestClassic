class eVire : public enemy {
public:
  eVire(zfix X,zfix Y,int Id,int Clk) : enemy(X,Y,Id,Clk)
  {
    dp=wdp=2;
    hp=4;
    cs=7;
    frate=20;
    dir=zc_rand()&3;
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
      zfix h = (31-clk3)*0.125 - 2.0;
      y+=h;
    }
  }
};

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