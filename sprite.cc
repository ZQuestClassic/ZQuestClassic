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


extern itemdata *itemsbuf;
extern wpndata  *wpnsbuf;


// this code needs some patching for use in zquest.cc

#ifdef  _ZQUEST_

int fadeclk = -1;
int frame = 8;

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
  int lasthit, lasthitclk;
  int dummy_int[10];
  bool dummy_bool[10];


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
    if(angular)
    {
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
  if(clk>=0)
    overtile16(dest,tile,sx,sy,cs,flip);
  else
  {
    int t  = wpnsbuf[iwSpawn].tile;
    int cs = wpnsbuf[iwSpawn].csets&15;
  #ifndef _ZQUEST_
    if(BSZ)
    {
      if(clk>=-10) t++;
      if(clk>=-5) t++;
    }
    else
  #endif
    {
      if(clk>=-12) t++;
      if(clk>=-6) t++;
    }
    overtile16(dest,t,x,sy,cs,0);
  }

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


#define SLMAX 255

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

  bool remove(sprite *s)
  // removes pointer from list but doesn't delete it
  {
    int j=0;
    for( ; j<count; j++)
      if(sprites[j] == s)
        goto gotit;

    return false;

    gotit:

    for(int i=j; i<count-1; i++)
      sprites[i]=sprites[i+1];

    count--;
    return true;
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

  // returns index of last sprite with matching id, -1 if none found
  int idLast(int id)
  {
    for(int i=count-1; i>=0; i--)
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
   word *di = &(tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]);
//   bcombo = ((*di)&0xFF)+(tmpscr->cpage<<8);
   bcombo = ((tmpscr->data[(int(y)&0xF0)+(int(x)>>4)])&0xFF)+(tmpscr->cpage<<8);
   tile = combobuf[bcombo].tile;
   flip = combobuf[bcombo].flip;
   cs = ((*di)&0x700)>>8;
   *di = tmpscr->under&0x7FF;
   putcombo(scrollbuf,x,y,*di,tmpscr->cpage);
   clk=32;
  }

  virtual bool animate(int index)
  {
   if(clk<=0)
     return false;

   move(0.5);

   if(--clk==0) {
    tmpscr->data[(int(y)&0xF0)+(int(x)>>4)]=cmb(bcombo,cs);
    tmpscr->sflag[(int(y)&0xF0)+(int(x)>>4)]=0;

    if(hiddenstair(0,true))
      sfx(WAV_SECRET,128);
    else
    {
      hidden_entrance(0,true,true);
      if((combobuf[bcombo].type == cPUSH_WAIT) ||
         (combobuf[bcombo].type == cPUSH_HW) ||
         (combobuf[bcombo].type == cPUSH_HW2))
        sfx(WAV_SECRET,128);
    }

    if(isdungeon() && tmpscr->flags&fSHUTTERS) {
     opendoors=8;
     }

    if(!isdungeon())
      if(combobuf[bcombo].type==cPUSH_HEAVY || combobuf[bcombo].type==cPUSH_HW
      || combobuf[bcombo].type==cPUSH_HEAVY2 || combobuf[bcombo].type==cPUSH_HW2)
        setmapflag(mSECRET);

    putcombo(scrollbuf,x,y,cmb(bcombo,cs),tmpscr->cpage);
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


byte boomframe[16] = {0,0,1,0,2,0,1,1,0,1,1,3,2,2,1,2};
byte bszboomflip[4] = {0,2,3,1};

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
  int power,type,dead,clk2,misc,misc2,ignorecombo;
  fix step;
  bool bounce, ignoreLink;
  word flash,wid,aframe,csclk;

  weapon(fix X,fix Y,int Id,int Type,int pow,int Dir) : sprite()
  {
   x=X; y=Y; id=Id; type=Type; power=pow; dir=max(Dir,0);
   clk=clk2=flip=misc=misc2=0;
   flash=wid=aframe=csclk=0;
   ignorecombo=-1;
   step=0;
   dead=-1;
   bounce=ignoreLink=false;
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
     LOADGFX(wWIND);
     clk=-14;
     step=2;
     break;
   case wBeam:
     step = 3;
     LOADGFX(game.sword-1+wSWORD);
     flash = 1;
     cs = 6;
     switch(dir) {
     case down:  flip=get_bit(QHeader.rules3,qr3_SWORDWANDFLIPFIX)?3:2;
     case up:    hyofs=2; hysz=12; break;
     case left:  flip=1;
     case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); hxofs=2; hxsz=12; yofs = (BSZ ? 59 : 57); break;
     }
     break;
   case wArrow:
     if (game.arrow<3) {
       LOADGFX(wARROW+type-1);
     } else {
       LOADGFX(wGARROW);
     }
     step=3;
     switch(dir) {
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
     step = (type==0)?.5:0;
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
     switch(dir) {
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
     break;

   case wHookshot:
     hookshot_used=true;
     LOADGFX(wHSHEAD);
     step = 4;
//     step = 0;
     clk2=256;
     switch(dir) {
     case down:  flip=2; xofs+=4;  yofs+=1; hyofs=2; hysz=12; break;
     case up:    yofs+=3; xofs-=5; hyofs=2; hysz=12; break;
     case left:  flip=1; tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs+=2; yofs=60; hxofs=2; hxsz=12; break;
     case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs-=2; yofs=60; hxofs=2; hxsz=12; break;
     }
     break;

   case wHSHandle:
     step = 0;
     LOADGFX(wHSHANDLE);
     switch(dir) {
     case down:  flip=2; xofs+=4;  yofs+=1; hyofs=2; hysz=12; break;
     case up:    yofs+=3; xofs-=5; hyofs=2; hysz=12; break;
     case left:  flip=1; tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs+=2; yofs=60; hxofs=2; hxsz=12; break;
     case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs-=2; yofs=60; hxofs=2; hxsz=12; break;
     }
     break;

   case wHSChain:
     LOADGFX(wHSCHAIN);
     step = 0;
     switch(dir) {
     case down:  xofs+=4;  yofs-=7;  break;
     case up:    xofs-=5;  yofs+=11; break;
     case left:  tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs+=10; yofs=60;  break;
     case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); xofs-=10; yofs=60;  break;
     }
     break;

   case ewBrang:
     hxofs=4; hxsz=8;
     wid = min(max(game.brang,1),3)-1+wBRANG;
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
     switch(dir) {
     case down:  flip=2;
     case up:    xofs=-4; hxsz=8; break;
     case left:  flip=1;
     case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); yofs=57; break;
     }
     break;
   case ewSword:
     LOADGFX(ewSWORD);
     step=3;
     switch(dir) {
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
     switch(dir) {
     case down:  flip=2;
     case up:    break;
     case left:  flip=1;
     case right: tile=wpnsbuf[wid].tile+((wpnsbuf[wid].frames>1)?wpnsbuf[wid].frames:1); yofs=57; break;
     }
     if (id==wRefMagic) {
       ignorecombo=(((int)y&0xF0)+((int)x>>4));
     }
     break;
   case ewFlame:
     LOADGFX(ewFLAME);
     if (dir==255) {
       step=2;
       seekLink();
     } else {
       if (dir>right) {
         step = .707;
       } else {
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
     if (type==0) {
       LOADGFX(wDINSFIRE1A);
       step = 4;
     }
     if (type==1) {
       LOADGFX(wDINSFIRE1B);
       step = 4;
     }
     if (type==2) {
       LOADGFX(wDINSFIRES1A);
     }
     if (type==3) {
       LOADGFX(wDINSFIRES1B);
     }
     break;
   default:
     tile=0; break;
   }
  }

  void LOADGFX(int wpn)
  {
    wid = wpn;
    flash = wpnsbuf[wid].misc&3;
    tile  = wpnsbuf[wid].tile;
    cs = wpnsbuf[wid].csets&15;
  }


  bool Dead() { return dead!=-1; }

  bool clip()
  {
   int c[4];
   int d=isdungeon();
/*
   if(id>wEnemyWeapons && id<wHammer && id!=ewBrang) {
     c[0] = d?32:16;
     c[1] = d?128:144;
     c[2] = d?32:16;
     c[3] = d?208:224;
     }
   else if (id==wHookshot) {
     c[0] = d?8:0;
     c[1] = d?168:176;
     c[2] = d?8:0;
     c[3] = d?248:256;
   } else {
     c[0] = d?32:16;
     c[1] = d?128:144;
     c[2] = d?32:16;
     c[3] = d?224:240;
   }
*/

   if(id>wEnemyWeapons && id<wHammer && id!=ewBrang) {
     c[0] = d?32:16;
     c[1] = d?128:144;
     c[2] = d?32:16;
     c[3] = d?208:224;
     }
   else if (id==wHookshot) {
     c[0] = d?8:0;
     c[1] = d?168:176;
     c[2] = d?8:0;
     c[3] = d?248:256;
   } else {
     c[0] = d?18:2;
     c[1] = d?144:160;
     c[2] = d?20:4;
     c[3] = d?220:236;
   }


   if(id>=wSSparkle && id<=wFSparkle) {
     c[0] = 0;
     c[1] = 176;
     c[2] = 0;
     c[3] = 256;
     }

   if(id==ewFlame) {
     c[0] = d?32:16;
     c[1] = d?128:144;
     c[2] = d?32:16;
     c[3] = d?208:224;
   }

   if(id==ewWind) {
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

   if(id>wEnemyWeapons && id<wHammer) {
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

  virtual bool animate(int index)
  {
   // do special timing stuff
   bool hooked=false;
   switch(id)
   {
   // Link's weapons
   case wWhistle:
     if(clk)       
       dead=1;
     break;

   case wWind:  if(type==1 && dead==-1 && x>=tmpscr->warpx2) dead=2; break;
   case wFire:
     if((type==0)||(type==3)) {
       if(clk==32) { step=0; lighting(1); }
       if(clk==94) { dead=1; findentrance(x,y,mfBURN,true); }
       }
     else {
       if(clk==1)  lighting(1);
       if(clk==80) { dead=1; findentrance(x,y,mfBURN,true); }
       }
     break;
   case wLitBomb:
   case wBomb:
     if(clk==40)
       id = wBomb;
     if(clk==41) {
       sfx(WAV_BOMB,pan(int(x)));
       findentrance(x,y,mfBOMB,true);
       hxofs=hyofs=-7;
       hxsz=hysz=30;
       }
     if(clk==65)
       hxofs=1000;
     if(clk==73) bombdoor(x,y);
     if(clk==77) dead=1;
     break;

   case wArrow:
     if (findentrance(x,y,mfARROW,true)) dead=4;
     if ((get_bit(QHeader.rules2,qr2_SASPARKLES+game.arrow-2)) && (game.arrow>=2)) {
       if (!(clk%(16>>(game.arrow)))) {
         arrow_x=x;
         arrow_y=y;
         add_asparkle=true;
       }
     }
     break;

   case wSSparkle:
       if(clk>=(((wpnsbuf[wSSPARKLE].frames) * (wpnsbuf[wSSPARKLE].speed))-1)) { dead=0; }
     break;

   case wGSparkle:
       if(clk>=(((wpnsbuf[wGSPARKLE].frames) * (wpnsbuf[wGSPARKLE].speed))-1)) { dead=0; }
     break;

   case wMSparkle:
       if(clk>=(((wpnsbuf[wMSPARKLE].frames) * (wpnsbuf[wMSPARKLE].speed))-1)) { dead=0; }
     break;

   case wFSparkle:
       if(clk>=(((wpnsbuf[wFSPARKLE].frames) * (wpnsbuf[wFSPARKLE].speed))-1)) { dead=0; }
     break;

   case wLitSBomb:
   case wSBomb:
     if(clk==40)
       id = wSBomb;
     if(clk==41) {
       sfx(WAV_BOMB,pan(int(x)));
       findentrance(x,y,mfBOMB,true);
       findentrance(x,y,mfSBOMB,true);
       hxofs=hyofs=-16;
       hxsz=hysz=48;
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

     if ((get_bit(QHeader.rules2,qr2_MBSPARKLES+game.brang-2)) && (game.brang>=2)) {
       if (!(clk%(16>>(game.brang)))) {
         brang_x=x-3;
         brang_y=y-3;
         add_bsparkle=true;
       }
     }


     break;

   case wHookshot:


     if (misc==0) {
       if (dir==up) {
         if ((combobuf[MAPCOMBO(x,y+6)].type==cHSGRAB)) {
           hooked=true;
         }
         if (!hooked && _walkflag(x,y+6,1) && !isstepable(MAPCOMBO(int(x),int(y+6)))) {
           dead=1;
         }
       }
     
       if (dir==down) {
         if (int(x)&8) {
           if ((combobuf[MAPCOMBO(x+16,y+9)].type==cHSGRAB)) {
             hooked=true;
           }
           if (!hooked && _walkflag(x+16,y+9,1) && !isstepable(MAPCOMBO(int(x+16),int(y+9)))) {
             dead=1;
           }
         } else {
           if ((combobuf[MAPCOMBO(x,y+9)].type==cHSGRAB)) {
             hooked=true;
           }
           if (!hooked && _walkflag(x,y+9,1) && !isstepable(MAPCOMBO(int(x),int(y+9)))) {
             dead=1;
           }
         }
       }
     
       if (dir==left) {
         if (int(y)&8) {
           if ((combobuf[MAPCOMBO(x+6,y+16)].type==cHSGRAB)) {
             hooked=true;
           }
           if (!hooked && _walkflag(x+6,y+16,1) && !isstepable(MAPCOMBO(int(x+6),int(y+16)))) {
             dead=1;
           }
         } else {
           if ((combobuf[MAPCOMBO(x+6,y)].type==cHSGRAB)) {
             hooked=true;
           }
           if (!hooked && _walkflag(x+6,y,1) && !isstepable(MAPCOMBO(int(x+6),int(y)))) {
             dead=1;
           }
         }
       }
     
       if (dir==right) {
         if (int(y)&8) {
           if ((combobuf[MAPCOMBO(x+9,y+16)].type==cHSGRAB)) {
             hooked=true;
           }
           if (!hooked && _walkflag(x+9,y+16,1) && !isstepable(MAPCOMBO(int(x+9),int(y+16)))) {
             dead=1;
           }
         } else {
           if ((combobuf[MAPCOMBO(x+9,y)].type==cHSGRAB)) {
             hooked=true;
           }
           if (!hooked && _walkflag(x+9,y,1) && !isstepable(MAPCOMBO(int(x+9),int(y)))) {
             dead=1;
           }
         }
       }
     }

     if (misc==0) {
       if( (((abs(hs_starty-y))/8)>(chainlinks_count)) ||
           (((abs(hs_startx-x))/8)>(chainlinks_count))){
           chainlink_x=x; chainlink_y=y;
           add_chainlink=true;
       }
     }

     if (misc==1) {
       if( (((abs(hs_starty-y))/8)<(chainlinks_count)) &&
           (((abs(hs_startx-x))/8)<(chainlinks_count))){
           del_chainlink=true;
       }
     }

     if (hooked==true) {
       misc=1;
       pull_link=true;
       step=0;
     }
     clk2++;
     if(clk==0) {             // delay a frame
       clk++;
       sfx(WAV_HOOKSHOT,pan(int(x)),true);
       return false;
       }

     if(dead==1) {
       dead=-1;
       misc=1;
       }

     if(misc==1) {  // returning
       if(abs(LinkY()-y)<9 && abs(LinkX()-x)<9) {
         hookshot_used=false;
         if (pull_link) {
           hs_fix=true;
         }
         pull_link=false;
         CatchBrang();
         stop_sfx(WAV_HOOKSHOT);
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
      switch (type) {
        case 0: //Din's Fire Rocket
          if (!(clk%(4))) {
            df_x=x-3;
            df_y=y-3;
            add_df1asparkle=true;
            add_df1bsparkle=false;
          }
          break;
        case 1: //Din's Fire Rocket return
          if (!(clk%(4))) {
            df_x=x-3;
            df_y=y-3;
            add_df1bsparkle=true;
            add_df1asparkle=false;
          }
          if (y>=casty) {
            dead=1;
            castnext=true;
          }
          break;
        case 2: //Din's Fire Rocket trail
          if(clk>=(((wpnsbuf[wDINSFIRES1A].frames) * (wpnsbuf[wDINSFIRES1A].speed))-1)) { dead=0; }
          break;
        case 3: //Din's Fire Rocket return trail
          if(clk>=(((wpnsbuf[wDINSFIRES1B].frames) * (wpnsbuf[wDINSFIRES1B].speed))-1)) { dead=0; }
          break;
      }
      break;

   case wRefMagic:
   case wMagic:
   case ewMagic:
     int checkx, checky;
     switch (dir) {
       case up: checkx=x; checky=y+8;
         break;
       case down: checkx=x; checky=y;
         break;
       case left: checkx=x+8; checky=y;
         break;
       case right: checkx=x; checky=y;
         break;
     }
     if (ignorecombo!=(((int)checky&0xF0)+((int)checkx>>4))) {
       if (hitcombo(checkx,checky,cMIRROR)) {
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
       if (hitcombo(checkx,checky,cMIRRORSLASH)) {
          id = wRefMagic;
          dir = 3-dir;
          if((dir==1)||(dir==2))
            flip ^= 3;
          tile=wpnsbuf[wid].tile;
          if (dir&2) {
            if (wpnsbuf[wid].frames>1) {
              tile+=wpnsbuf[wid].frames;
            } else {
              tile++;
            }
          }
          ignoreLink=false;
          ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
          y=(int)y&0xF0;
          x=(int)x&0xF0;
       }
       if (hitcombo(checkx,checky,cMIRRORBACKSLASH)) {
          id = wRefMagic;
          dir ^= 2;
          if(dir&1)
            flip ^= 2;
          else
            flip ^= 1;
          tile=wpnsbuf[wid].tile;
          if (dir&2) {
            if (wpnsbuf[wid].frames>1) {
              tile+=wpnsbuf[wid].frames;
            } else {
              tile++;
            }
          }
          ignoreLink=false;
          ignorecombo=(((int)checky&0xF0)+((int)checkx>>4));
          y=(int)y&0xF0;
          x=(int)x&0xF0;
       }
       if (hitcombo(checkx,checky,cMAGICPRISM)) {
          int newx, newy;
          newy=(int)y&0xF0;
          newx=(int)x&0xF0;
          for (int tdir=0; tdir<4; tdir++) {
            if (dir!=(tdir^1)) {
              addLwpn(newx,newy,wRefMagic,0,power*DAMAGE_MULTIPLIER,tdir);
            }
          }
          dead=0;
       }
       if (hitcombo(checkx,checky,cMAGICPRISM4)) {
          int newx, newy;
          newy=(int)y&0xF0;
          newx=(int)x&0xF0;
          for (int tdir=0; tdir<4; tdir++) {
            addLwpn(newx,newy,wRefMagic,0,power*DAMAGE_MULTIPLIER,tdir);
          }
          dead=0;
       }
       if (hitcombo(checkx,checky,cMAGICSPONGE)) {
          dead=0;
       }
     }
     break;
     
   // enemy weapons
   case wRefFireball:
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

   case ewFlame:
     if(clk==32)  { step=0; lighting(1); }
     if(clk==126) { dead=1; }
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

/*
    case wPhantom:
      if (type==0) { //Din's Fire Rocket up
        if (y<-32) {
          dead=1;
        } else {
          dead=0;
        }
      }
      if (type==1) { //Din's Fire Rocket return
        if (y>=casty) {
          dead=1;
          castnext=true;
        } else {
          dead=0;
        }
      }
      break;
*/
     
   }

   // move sprite, check clipping
   if(dead==-1 && clk>=0)
   {
     move(step);
     if(clip()) {
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
   clk++;
   if(dead>0)
     dead--;
   return dead==0;
  }


  virtual void onhit(bool clipped)
  {
    onhit(clipped, 0, -1);
  }

  virtual void onhit(bool clipped, int special, int linkdir)
  {
    if(special==2)   // hit Link's mirror shield
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
        special = 1; // check normal shield stuff
      }

    if(special==1)   // hit Link's shield
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


    switch(id) {
    case wLitBomb:  if(!clipped) dead=1;
    case wLitSBomb: if(!clipped) dead=1;
    case wWhistle:
    case wBomb:
    case wSBomb:
    case wBait:
    case wFire:
    case wPhantom: break; // don't worry about clipping or hits with these
    case ewFlame:  if (!clipped) dead=1; break;
    case wBeam:    dead=23; break;
    case wArrow:   dead=4; findentrance(x,y,mfARROW,true); break;
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
    case wHookshot:
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
       dead=1; break;  //remove the dead part to make the wand only die
                       //when clipped

    case ewWind:
      if (clipped) {
        if (misc==999) { // in enemy wind
          ewind_restart=true;
        }
        dead=1;
      }

      break;
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
     tile = wpnsbuf[wid].tile + aframe + (((!angular)&&(dir&2))?wpnsbuf[wid].frames:0);
   }

   // do special case stuff
   switch(id)
   {
    case wBeam: {
      if(dead==-1) break;
      // draw the beam thingies
      int ofs=23-dead;
      int f = frame&3;
      int type = wpnsbuf[wid].type;
      tile = wpnsbuf[wid].tile+2;
      if(type)
        cs = wpnsbuf[wid].csets>>4;
      if(type==3 && (f&2))
        tile++;
      if(!type || f==0 || (type>1 && f==3)) overtile16(dest,tile,x-2-ofs,y+54-ofs,cs,0);
      if(!type || f==2 || (type>1 && f==1)) overtile16(dest,tile,x+2+ofs,y+54-ofs,cs,1);
      if(!type || f==1 || (type>1 && f==2)) overtile16(dest,tile,x-2-ofs,y+58+ofs,cs,2);
      if(!type || f==3 || (type>1 && f==0)) overtile16(dest,tile,x+2+ofs,y+58+ofs,cs,3);
      } return; // don't draw sword

    case wBomb:
    case wSBomb:
      if(clk<=41) break;
      // draw the explosion
      tile = wpnsbuf[wBOOM].tile;
      cs = wpnsbuf[wBOOM].csets&15;
      if(clk>65)
        tile++;
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
      return; // don't draw bomb

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

int fairy_cnt = 0;


class item : public sprite {
public:
  int pickup,clk2,misc;
  int aclk,aframe;
  bool flash,twohand,anim;
  void check_conveyor();

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
    if(flash)
    {
      cs = itemsbuf[id].csets;
      if(frame&8)
        cs >>= 4;
      else
        cs &= 15;
    }
    if(anim)
    {
      int spd = itemsbuf[id].speed;
      if(aframe==0)
        spd *= itemsbuf[id].delay+1;

      if(++aclk >= spd)
      {
        aclk=0;
        if(++aframe >= itemsbuf[id].frames)
          aframe=0;

        tile = itemsbuf[id].tile + aframe;
      }
    }

#ifndef _ZQUEST_
    if(id==iFairy)
      movefairy(x,y,misc);
    item::check_conveyor();
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
    if(pickup&ipNODRAW)
      return;

    if(!(pickup&ipFADE) || fadeclk<0 || fadeclk&1)
      if(clk2>32 || (clk2&2)==0 || id==iFairy)
        sprite::draw(dest);
  }
};


item::item(fix X,fix Y,int i,int p,int c) : sprite()
{
  x=X; y=Y; id=i; pickup=p; clk=c;
  misc=clk2=0;
  aframe=aclk=0;
  anim=flash=twohand=false;

  if(id<0 || id>=iMax)
    return;

  tile = itemsbuf[id].tile;
  cs = itemsbuf[id].csets&15;

  if(itemsbuf[id].misc&1)
    flash=true;
  if(itemsbuf[id].misc&2)
    twohand=true;

  anim = itemsbuf[id].frames>0;

  if(pickup&ipBIGRANGE)
  {
    hxofs=-8;
    hxsz=17;
    hyofs=-4;
    hysz=20;
  }
  else if(pickup&ipBIGTRI)
  {
    hxofs=-8;
    hxsz=28;
    hyofs=-4;
    hysz=20;
  }
  else
  {
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



#ifndef _ZQUEST_
void item::check_conveyor()
{
  if (conveyclk<=0) {
    int ctype=(combobuf[MAPCOMBO(x+8,y+8)].type);
    if((ctype>=cCVUP) && (ctype<=cCVRIGHT)) {
      switch (ctype-cCVUP) {
        case up:
         if(!_walkflag(x,y+8-2,2)) {
            y=y-2;
          }
          break;
        case down:
         if(!_walkflag(x,y+15+2,2)) {
            y=y+2;
          }
          break;
        case left:
          if(!_walkflag(x-2,y+8,1)) {
            x=x-2;
          }
          break;
        case right:
          if(!_walkflag(x+15+2,y+8,1)) {
            x=x+2;
          }
          break;
      }
    }
  }
}
#endif


// easy way to draw an item

void putitem(BITMAP *dest,int x,int y,int item_id)
{
  item temp(x,y,item_id,0,0);
  temp.yofs=0;
  temp.animate(0);
  temp.draw(dest);
}



/*** end of sprite.cc ***/

