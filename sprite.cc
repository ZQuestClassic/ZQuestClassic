/*
  sprite.cc
  Jeremy Craner, 1999
  - sprite:      base class for the guys and enemies in zelda.cc
  - movingblock: the moving block class
  - sprite_list: main container class for different groups of sprites
  - item:        items class
*/


// this code needs some patching for use in zquest.cc

#ifdef  _ZQUEST_

#define fadeclk  -1
#define frame    8

#endif


/**********************************/
/******* Sprite Base Class ********/
/**********************************/

class sprite {
public:
  fix x,y;
  int tile,cs,flip,clk;
  fix xofs,yofs;
  int hxofs,hyofs,hxsz,hysz;
  int id,dir;
  bool angular,canfreeze;
  double angle;

  sprite()
  { x=y=tile=cs=flip=clk=xofs=hxofs=hyofs=0;
    id=-1;
    hxsz=hysz=16;
    yofs=56;
    dir=down;
    angular=canfreeze=false;
  }
  sprite(fix X,fix Y,int T,int CS,int F,int Clk,int Yofs):
    x(X),y(Y),tile(T),cs(CS),flip(F),clk(Clk),yofs(Yofs)
  {
    hxsz=hysz=16;
    hxofs=hyofs=xofs=0;
    id=-1;
    dir=down;
    angular=canfreeze=false;
  }
  virtual ~sprite() {}
  virtual void draw(BITMAP* dest);    // main layer
  virtual void draw2(BITMAP* dest) {} // top layer for special needs
  virtual bool animate(int index) { return false; }
  int real_x(fix fx)
  {
    int rx=fx.v>>16;
    switch(dir)
    {
    case 9:
    case 13:
      if(fx.v&0xFFFF)
        rx++;
      break;
    }
    return rx;
  }
  int real_y(fix fy)
  {
    return fy.v>>16;
  }
  virtual bool hit(sprite *s)
  {
   if(id<0 || s->id<0 || clk<0) return false;
   return hit(s->x+s->hxofs,s->y+s->hyofs,s->hxsz,s->hysz);
  }
  virtual bool hit(int tx,int ty,int txsz,int tysz)
  {
   if(id<0 || clk<0) return false;
   return (tx+txsz>x+hxofs && ty+tysz>y+hyofs && tx<x+hxofs+hxsz
           && ty<y+hyofs+hysz);
  }
  virtual int hitdir(int tx,int ty,int txsz,int tysz,int dir)
  {
   int cx1=x+hxofs+(hxsz>>1);
   int cy1=y+hyofs+(hysz>>1);
   int cx2=tx+(txsz>>1);
   int cy2=ty+(tysz>>1);
   if(dir>=left && abs(cy1-cy2)<=8)
     return (cx2-cx1<0)?left:right;
   return (cy2-cy1<0)?up:down;
  }

  virtual void move(fix dx,fix dy)
  {
    x+=dx; y+=dy;
  }
  virtual void move(fix s)
  {
    if(angular) {
      x += cos(angle)*s;
      y += sin(angle)*s;
      return;
      }

    switch(dir) {
    case 8:
    case up:     y-=s; break;
    case 12:
    case down:   y+=s; break;
    case 14:
    case left:   x-=s; break;
    case 10:
    case right:  x+=s; break;
    case 15:
    case l_up:   x-=s; y-=s; break;
    case 9:
    case r_up:   x+=s; y-=s; break;
    case 13:
    case l_down: x-=s; y+=s; break;
    case 11:
    case r_down: x+=s; y+=s; break;
    }
  }
};


void sprite::draw(BITMAP* dest)
{
 int sx = real_x(x+xofs);
 int sy = real_y(y+yofs);

 if(id<0)
  return;
 if(clk<0) {
  int t=73;
  if(clk>=-12) t++;
  if(clk>=-6) t++;
  overtile16(dest,t,x,sy,7,0);
  }
 else
  overtile16(dest,tile,sx,sy,cs,flip);
 #ifndef _ZQUEST_
 if(debug && key[KEY_O])
   rectfill(dest,x+hxofs,sy+hyofs,x+hxofs+hxsz-1,sy+hyofs+hysz-1,vc(id));
 #endif
}





/***************************************************************************/

#ifndef _ZQUEST_   // skip this stuff if we're compiling zquest.cc



/**********************************/
/********** Sprite List ***********/
/**********************************/


#define SLMAX 64

class sprite_list {
  sprite *sprites[SLMAX];
  int count;

public:
  sprite_list() : count(0) {}
  void clear() { while(count>0) del(0); }

  sprite *spr(int index)
  {
    if(index<0 || index>=count)
      return NULL;
    return sprites[index];
  }

  bool swap(int a,int b)
  {
    if(a<0 || a>=count || b<0 || b>=count)
      return false;
    sprite *c = sprites[a];
    sprites[a] = sprites[b];
    sprites[b] = c;
    return true;
  }

  bool add(sprite *s)
  {
    if(count>=SLMAX)
    {
      delete s;
      return false;
    }
    sprites[count++]=s;
    return true;
  }

  void draw(BITMAP* dest,bool lowfirst)
  { switch(lowfirst) {
    case true:
      for(int i=0; i<count; i++)
       sprites[i]->draw(dest);
      break;
    case false:
      for(int i=count-1; i>=0; i--)
       sprites[i]->draw(dest);
      break;
    }
  }

  void draw2(BITMAP* dest,bool lowfirst)
  { switch(lowfirst) {
    case true:
      for(int i=0; i<count; i++)
       sprites[i]->draw2(dest);
      break;
    case false:
      for(int i=count-1; i>=0; i--)
       sprites[i]->draw2(dest);
      break;
    }
  }

  void animate()
  { int i=0;
    while(i<count) {
     if(!(freeze_guys && sprites[i]->canfreeze))
       if(sprites[i]->animate(i)) {
         del(i);
         i--;
         }
     i++;
     }
  }

  bool del(int j)
  {
    if(j<0||j>=count)
      return false;

    delete sprites[j];
    for(int i=j; i<count-1; i++)
      sprites[i]=sprites[i+1];
    count--;
    return true;
  }

  int Count() { return count; }

  int hit(sprite *s)
  {
    for(int i=0; i<count; i++)
     if(sprites[i]->hit(s))
      return i;
    return -1;
  }

  int hit(int x,int y,int xsize, int ysize)
  {
    for(int i=0; i<count; i++)
     if(sprites[i]->hit(x,y,xsize,ysize))
      return i;
    return -1;
  }

  // returns the number of sprites with matching id
  int idCount(int id)
  {
    int c=0;
    for(int i=0; i<count; i++)
     if(sprites[i]->id == id)
      c++;
    return c;
  }

  // returns index of first sprite with matching id, -1 if none found
  int idFirst(int id)
  {
    for(int i=0; i<count; i++)
     if(sprites[i]->id == id)
      return i;
    return -1;
  }
};



/**********************************/
/********** Moving Block **********/
/**********************************/


class movingblock : public sprite {
public:
  int bcombo;

  movingblock() : sprite()
  {
    id=1;
  }

  void push(fix bx,fix by,int d)
  {
   x=bx; y=by; dir=d;
   word *di = &tmpscr[0].data[(int(y)&0xF0)+(int(x)>>4)];
   bcombo = (*di)&0x1FF;
   tile = combobuf[bcombo].tile;
   flip = combobuf[bcombo].flip;
   cs = ((*di)&0xE00)>>9;
   *di = tmpscr[0].under&0xFFF;
   putcombo(scrollbuf,x,y,*di);
   clk=32;
  }

  virtual bool animate(int index)
  {
   if(clk<=0)
     return false;

   move(0.5);

   if(--clk==0) {
    tmpscr[0].data[(int(y)&0xF0)+(int(x)>>4)]=cmb(bcombo,cs,0);

    if(hiddenstair(0,true))
      sfx(WAV_SECRET,128);
    else if(combobuf[bcombo].type == cPUSH_WAIT)
      sfx(WAV_SECRET,128);
    else if(combobuf[bcombo].type == cPUSH_HW)
      sfx(WAV_SECRET,128);

    if(dungeon() && tmpscr[0].flags&fSHUTTERS) {
     opendoors=8;
     }

    if(dungeon()==0)
     if(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW)
      game.maps[(currmap<<7)+currscr]|=1;

    putcombo(scrollbuf,x,y,cmb(bcombo,cs,0));
    }
   return false;
  }

  virtual void draw(BITMAP *dest)
  {
   if(clk)
     sprite::draw(dest);
  }
};



/**************************************/
/***********  Weapon Class  ***********/
/**************************************/


byte boomframe[16] = {51,0,52,0,53,0,52,1,51,1,52,3,53,2,52,2};

class weapon : public sprite {

  void seekLink()
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

public:
  int power,type,dead,clk2,misc,misc2;
  fix step;
  bool flash,bounce;

  weapon(fix X,fix Y,int Id,int Type,int pow,int Dir) : sprite()
  {
   x=X; y=Y; id=Id; type=Type; power=pow; dir=Dir;
   clk=clk2=flip=misc=misc2=step=0;
   dead=-1;
   flash=bounce=false;
   yofs=54;

   if(id>wEnemyWeapons)
     canfreeze=true;

   switch(id) {
   case wWhistle:
     xofs=1000;     // don't show
     x=y=hxofs=hyofs=0;
     hxsz=hysz=255; // hit the whole screen
     break;
   case wWind:
     flash=true;
     clk=-14;
     step=2;
     break;
   case wBeam:
     flash=true;
     step=3;
     switch(dir) {
     case down:  flip=2;
     case up:    tile=41; hyofs=2; hysz=12; break;
     case left:  flip=1;
     case right: tile=79; yofs=57; hxofs=2; hxsz=12; break;
     }
     break;
   case wArrow:
     cs=type+5;
     step=3;
     switch(dir) {
     case down:  flip=2;
     case up:    tile=45; hyofs=2; hysz=12; break;
     case left:  flip=1;
     case right: tile=80; yofs=57; hyofs=2; hysz=14; hxofs=2; hxsz=12; break;
     }
     break;
   case wFire:
     tile=65; step=(type==0)?.5:0; cs=8;
     hxofs=hyofs=1; hxsz=hysz=14;
     break;
   case wBomb:
     tile=50; cs=7; hxofs=hyofs=4; hxsz=hysz=8; id = wLitBomb;
     break;
   case wBait:
     tile=42; cs=8;
     break;
   case wMagic:
     flash=true;
     step=2.5;
     switch(dir) {
     case down:  flip=2;
     case up:    tile=77; hyofs=2; hysz=12; break;
     case left:  flip=1;
     case right: tile=78; yofs=57; hxofs=2; hxsz=12; break;
     }
     break;
   case wBrang:
     cs=type+5;
     if(type>=2)
       clk2=256;
     hxofs=4; hxsz=7;
     hyofs=2; hysz=11;
     break;

   case ewBrang:
     hxofs=4; hxsz=8;
     break;

   case ewFireball:
     flash=true;
     tile=57; step=1.75;
     misc=dir-1;
     seekLink();
     break;
   case ewRock:
     cs=6; tile=139; hxofs=4; hxsz=8; step=3;
     break;
   case ewArrow:
     cs=type+5;
     step=2;
     switch(dir) {
     case down:  flip=2;
     case up:    tile=45; xofs=-4; hxsz=8; break;
     case left:  flip=1;
     case right: tile=80; yofs=57; break;
     }
     break;
   case ewSword:
     flash=true;
     step=3;
     switch(dir) {
     case down:  flip=2;
     case up:    tile=41; break;
     case left:  flip=1;
     case right: tile=79; yofs=57; break;
     }
     break;
   case ewMagic:
     flash=true;
     step=3;
     switch(dir) {
     case down:  flip=2;
     case up:    tile=77; break;
     case left:  flip=1;
     case right: tile=78; yofs=57; break;
     }
     break;
   default:
     tile=0; break;
   }
  }

  bool Dead() { return dead!=-1; }

  bool clip()
  {
   int c[4];
   int d=dungeon();
   if(id>wEnemyWeapons && id!=ewBrang) {
     c[0] = d?32:16;
     c[1] = d?128:144;
     c[2] = d?32:16;
     c[3] = d?208:224;
     }
   else {
     c[0] = d?18:2;
     c[1] = d?144:160;
     c[2] = d?20:4;
     c[3] = d?220:236;
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

  virtual bool animate(int index)
  {
   // do special timing stuff
   switch(id) {
   // Link's weapons
   case wWhistle: if(clk) dead=1; break;
   case wWind:  if(type==1 && dead==-1 && x>=tmpscr[0].warpx2) dead=2; break;
   case wFire:
     if(type==0) {
       if(clk==32) { step=0; lighting(1); }
       if(clk==94) { dead=1; doburn(x,y); }
       }
     else {
       if(clk==1)  lighting(1);
       if(clk==80) { dead=1; doburn(x,y); }
       }
     break;
   case wLitBomb:
   case wBomb:
     if(clk==40)
       id = wBomb;
     if(clk==41) {
       sfx(WAV_BOMB,pan(int(x)));
       dobomb(x,y);
       hxofs=hyofs=-7;
       hxsz=hysz=30;
       }
     if(clk==65)
       hxofs=1000;
     if(clk==73) bombdoor(x,y);
     if(clk==77) dead=1;
     break;
   case wBait:
     if(clk==16*2*24) dead=1;
     break;

   case wBrang:
     clk2++;
     if(clk2==36)
       misc=1;
     if(clk2>18 && clk2<52)
       step=1;
     else if(misc)
       step=2;
     else
       step=3;

     if(clk==0) {             // delay a frame
       clk++;
       sfx(WAV_BRANG,pan(int(x)),true);
       return false;
       }

     if(clk==1) {             // then check directional input
       if(Up()) {
         dir=up;
         if(Left())  dir=l_up;
         if(Right()) dir=r_up;
         }
       else if(Down()) {
         dir=down;
         if(Left())  dir=l_down;
         if(Right()) dir=r_down;
         }
       else if(Left())
         dir=left;
       else if(Right())
         dir=right;
       }

     if(dead==1) {
       dead=-1;
       misc=1;
       cs=type+5;
       }

     if(misc==1) {  // returning
       if(abs(LinkY()-y)<7 && abs(LinkX()-x)<7) {
         CatchBrang();
         stop_sfx(WAV_BRANG);
         return true;
         }
       seekLink();
       }

     adjust_sfx(WAV_BRANG,pan(int(x)),true);
     break;

   // enemy weapons
   case ewFireball:
     switch(misc) {
     case up:    y-=.5; break;
     case down:  y+=.5; break;
     case left:  x-=.5; break;
     case right: x+=.5; break;
     }
     if(clk<16)
     {
       clk++;
       if(dead>0)
         dead--;
       return dead==0;
     }
     break;

   case ewBrang:
     if(clk==0)
       misc2=(dir<left)?y:x; // save home position

     clk2++;
     if(clk2==45) {
       misc=1;
       dir^=1;
       }
     if(clk2>27 && clk2<61)
       step=1;
     else if(misc)
       step=2;
     else
       step=3;

     if(dead==1) {
       dead=-1;
       misc=1;
       dir^=1;
       }

     if(misc==1) // returning
       switch(dir) {
       case up:    if(y<misc2) return true; break;
       case down:  if(y>misc2) return true; break;
       case left:  if(x<misc2) return true; break;
       case right: if(x>misc2) return true; break;
       }
     break;
   }

   // move sprite, check clipping
   if(dead==-1 && clk>=0) {
     move(step);
     if(clip())
       onhit(true);
     else if(id==ewRock) {
       if(_walkflag(x,y,2) || _walkflag(x,y+8,2))
         onhit(true);
       }
     }

   if(bounce)
     switch(dir) {
     case up:    x-=1; y+=2; break;
     case down:  x+=1; y-=2; break;
     case left:  x+=2; y-=1; break;
     case right: x-=2; y-=1; break;
     }

   // update clocks
   clk++;
   if(dead>0)
     dead--;
   return dead==0;
  }


  virtual void onhit(bool clipped)
  {
    onhit(clipped,0);
  }

  virtual void onhit(bool clipped,int special)
  {
    if(special==1)   // hit Link's shield
      switch(id) {
      case ewMagic:
      case ewArrow:
      case ewSword:
      case ewRock:
        bounce=true;
        dead=16;
        return;
      case ewBrang:
        if(misc==0) {
          clk2=256;
          misc=1;
          dir^=1;
          }
        return;
      }
    switch(id) {
    case wLitBomb: if(!clipped) dead=1;
    case wWhistle:
    case wBomb:
    case wBait:
    case wFire:    break; // don't worry about clipping or hits with these ones
    case wBeam:    dead=23; break;
    case wArrow:   dead=4; break;
    case ewArrow:  dead=clipped?4:1; break;
    case wWind:
      if(x>=240)
        dead=2;
      break;
    case wBrang:
      if(misc==0) {
        clk2=256;
        if(clipped)
          dead=4;
        else
          misc=1;
        }
      break;
    case ewBrang:
      if(misc==0) {
        clk2=256;
        dead=4;
        }
      break;
    case wMagic: wand_dead=true; wand_x=x; wand_y=y; // set some global flags
      // dead=1; break;
    default: dead=1;
    }
  }

  // override hit detection to check for invicibility, etc
  virtual bool hit(sprite *s)
  {
   if(id==ewBrang && misc)
     return false;
   return (dead!=-1) ? false : sprite::hit(s);
  }
  virtual bool hit(int tx,int ty,int txsz,int tysz)
  {
   if(id==ewBrang && misc)
     return false;
   return (dead!=-1) ? false : sprite::hit(tx,ty,txsz,tysz);
  }


  virtual void draw(BITMAP *dest)
  {
   if(flash) cs=(frame&3)+6;
   // do special case stuff
   switch(id) {
    case wBeam: {
      if(dead==-1) break;
      // draw the beam thingies
      int ofs=23-dead;
      overtile16(dest,196,x-2-ofs,y+54-ofs,cs,0);
      overtile16(dest,196,x+2+ofs,y+54-ofs,cs,1);
      overtile16(dest,196,x-2-ofs,y+58+ofs,cs,2);
      overtile16(dest,196,x+2+ofs,y+58+ofs,cs,3);
      } return; // don't draw sword

    case wBomb:
      if(clk<=41) break;
      // draw the explosion
      tile=clk<=65?73:74;
      overtile16(dest,tile,x+((clk&1)?7:-7),y+yofs-13,cs,0);
      overtile16(dest,tile,x,y+yofs,cs,0);
      overtile16(dest,tile,x+((clk&1)?-14:14),y+yofs,cs,0);
      overtile16(dest,tile,x+((clk&1)?-7:7),y+yofs+14,cs,0);
      if(debug && key[KEY_F5])
        rectfill(dest,x+hxofs,y+hyofs+yofs,
          x+hxofs+hxsz-1,y+hyofs+hysz-1+yofs,vc(id));
      return; // don't draw bomb

    case wArrow:
    case ewArrow:
      if(dead>0 && !bounce) { cs=7; tile=54; flip=0; }
      break;

    case wFire:
      flip=(clk&4)>>2;
      break;

    case ewBrang:
      cs=min(max(game.brang,1),3)+5;
    case wBrang:
      tile = boomframe[clk&0xE];
      flip = boomframe[(clk&0xE)+1];
      if(dead>0) { cs=7; tile=54; flip=0; }
      break;

    case wWind:
      tile=(dead==-1)?83:65;
      flip^=1;
      break;
    }
   // draw it
   sprite::draw(dest);
  }
};




#endif   // ifndef _ZQUEST_


/***************************************************************************/



/**********************************/
/**********  Item Class  **********/
/**********************************/

// items data format:
//   low byte:  tile
//   high byte: aatfcccc (a: animation frames, t:two hands, f:flash, c:cset)

const byte dat_items[iMax*2] = {
// 0-4  : rupee,    blue rupee,  heart,    bombs,    clock
          49,0x17,  49,0x07,     82,0x17,  50,0x07,  68,0x08,
// 5-9  : sword,    white sword,  magic sword,  shield,   key
          41,0x06,  41,0x07,      59,0x08,      40,0x06,  48,0x08,
// 10-14: blue candle,  red candle,  letter,   arrow,    silver arrow
          44,0x07,      44,0x08,     61,0x07,  45,0x06,  45,0x07,
// 15-19: bow,      bait,     blue ring,  red ring,  power bracelet
          46,0x06,  42,0x08,  58,0x07,    58,0x08,   62,0x08,
// 20-24: triforce,  map,      compass,  wood boomerang,  magic boomerang
          72,0x37,   61,0x08,  70,0x08,  51,0x06,         51,0x07,
// 25-29: wand,     raft,     ladder,   heart container,  blue potion
          60,0x07,  71,0x26,  76,0x26,  69,0x28,          55,0x07,
// 30-34: red potion,  whistle,  magic book,  magic key,  fairy,
          55,0x08,     43,0x08,  56,0x08,     47,0x08,    63,0x48,
// 35-39: fire brang,  Excalibur,  mirror shield,  20 rupies,  50 rupies
          51,0x08,     59,0x07,    40,0x07,        49,0x0A,    49,0x0B,

// 40:    pile,      big triforce
          176,0x09,  175,0x37
};


int fairy_cnt = 0;


class item : public sprite {
public:
  int pickup,clk2,misc;
  byte anim;
  bool flash,twohand;

  item(fix X,fix Y,int i,int p,int c);

  #ifndef _ZQUEST_
  virtual ~item()
  {
    if(id==iFairy && misc>0)
      killfairy(misc);
  }
  #endif

  virtual bool animate(int index)
  {
    if((++clk)>=0x8000)
      clk=0x7000;
    if(flash) {
      cs=(dat_items[(id<<1)+1])&15;
      if(frame&8)
        cs++;
      }
    if(anim==1)
      tile=dat_items[id<<1]+((clk&4)>>2);

    #ifndef _ZQUEST_
    if(id==iFairy)
      movefairy(x,y,misc);
    #endif

    if(fadeclk==0)
      return true;
    if(pickup&ipTIMER)
      if(++clk2 == 512)
        return true;
    return false;
  }
  virtual void draw(BITMAP *dest)
  {
    if((pickup&ipFADE)==0 || fadeclk<0 || fadeclk&1)
      if(clk2>32 || (clk2&2)==0 || id==iFairy)
        sprite::draw(dest);
  }
};


item::item(fix X,fix Y,int i,int p,int c) : sprite()
{
  x=X; y=Y; id=i; pickup=p; clk=c;
  misc=clk2=0;
  anim=0;
  flash=twohand=false;

  int si=id<<1;
  tile=dat_items[si];
  cs=dat_items[si+1]&15;
  if(dat_items[si+1]&0x10)
    flash=true;
  if(dat_items[si+1]&0x20)
    twohand=true;
  anim = dat_items[si+1]>>6;

  if(pickup&ipBIGRANGE) {
   hxofs=-8;
   hxsz=17;
   hyofs=-4;
   hysz=20;
   }
  else if(pickup&ipBIGTRI) {
   hxofs=-8;
   hxsz=28;
   hyofs=-4;
   hysz=20;
   }
  else {
   hxsz=1;
   hyofs=4;
   hysz=12;
   }

  #ifndef _ZQUEST_
  if(id==iFairy)
  {
    misc = ++fairy_cnt;
    addenemy(x,y,eITEMFAIRY,misc+0x1000);
    sfx(WAV_SCALE1);
    movefairy(x,y,misc);
  }
  #endif
}





// easy way to draw an item

void putitem(BITMAP* dest,int x,int y,int item_id)
{
 item temp(x,y,item_id,0,0);
 temp.yofs=0;

 if(temp.flash)
  if(frame&8)
   temp.cs++;

 if(temp.anim)
  if(frame&4)
   temp.tile++;

 temp.draw(dest);
}



/*** end of sprite.cc ***/

