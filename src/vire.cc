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