/*
  subscr.cc
  Jeremy Craner, 1999
  Subscreen code for zelda.cc
*/


void blueframe(BITMAP *dest,int x,int y,int xsize,int ysize)
{
  puttile8(dest,12,x,y,0,0);
  puttile8(dest,16,((xsize-1)<<3)+x,y,0,0);
  puttile8(dest,13,x,((ysize-1)<<3)+y,0,0);
  puttile8(dest,17,((xsize-1)<<3)+x,((ysize-1)<<3)+y,0,0);
  for(int i=1; i+1<xsize; i++) {
    puttile8(dest,14,(i<<3)+x,y,0,0);
    puttile8(dest,15,(i<<3)+x,((ysize-1)<<3)+y,0,0);
    }
  for(int i=1; i+1<ysize; i++) {
    puttile8(dest,18,x,(i<<3)+y,0,0);
    puttile8(dest,19,((xsize-1)<<3)+x,(i<<3)+y,0,0);
    }
  for(int j=1; j+1<ysize; j++)
   for(int i=1; i+1<xsize; i++)
    puttile8(dest,22,(i<<3)+x,(j<<3)+y,0,0);
}



void drawgrid(BITMAP *dest,int fg,int bg)
{
 int si=0;
 for(int y=16; y<48; y+=4) {
   byte dl = DMaps[currdmap].grid[si];
   for(int x=16; x<76; x+=8) {
     rectfill(dest,x,y,x+6,y+2,(dl&0x80)?fg:bg);
     dl<<=1;
     }
   si++;
   }
}

void drawgrid_s(BITMAP *dest,int fg,int bg)
{
 int si=0;
 for(int y=16; y<48; y+=4) {
   byte dl = DMaps[currdmap].grid[si];
   for(int x=16; x<76; x+=8) {
     rectfill(dest,x,y,x+6,y+2,bg);
     if(dl&0x80)
       rectfill(dest,x+2,y,x+4,y+2,fg);
     dl<<=1;
     }
   si++;
   }
}


void drawdmap(BITMAP *dest)
{
 zcolors c=QMisc.colors;

 switch(DMaps[currdmap].type)
 {
 case dmOVERW:
   rectfill(dest,16,16,79,47,c.overw);
   break;

 case dmDNGN:
   rectfill(dest,16,16,79,47,c.dngn_bg);
   drawgrid(dest,c.dngn_fg,c.dngn_bg);
   break;

 case dmCAVE:
   rectfill(dest,16,16,79,47,c.cave_bg);
   drawgrid(dest,c.cave_fg,c.cave_bg);
   break;

 case dmBSOVERW:
   rectfill(dest,16,16,79,47,c.cave_bg);
   drawgrid_s(dest,c.bs_goal,c.bs_dk);
   break;

 case dmBSDNGN:
   rectfill(dest,16,16,79,47,c.bs_lt);
   drawgrid(dest,c.bs_dk,c.bs_lt);
   break;

 case dmBSCAVE:
   rectfill(dest,16,16,79,47,c.bs_lt);
   drawgrid(dest,c.bs_dk,c.bs_lt);
   break;
 }
}


void lifemeter(BITMAP *dest,int x,int y)
{
  for(int i=0; i<game.maxlife; i+=8) {
    int tile=0;
    if(i+4>=game.life) tile=2;
    if(i>=game.life) tile=4;
    puttile8(dest,tile,x,y,1,0);
    x+=8;
    if(((i>>3)&7)==7)
      {x-=64; y-=8;}
    }
}


void putxnum(BITMAP *dest,int x,int y,int num)
{
 text_mode(0);
 if(num>=100)
   textprintf(dest,zfont,x,y,QMisc.colors.text,"%d",num);
 else
   textprintf(dest,zfont,x,y,QMisc.colors.text,"X%d",num);
}



void putsubscr(BITMAP *dest,int x,int y)
{
  BITMAP *subscr = create_sub_bitmap(dest,x,y,256,56);
  clear(subscr);

  if(dlevel==0)
    drawdmap(subscr);
  else
  {
    if(dlevel<=9)
      textprintf(subscr,zfont,16,8,QMisc.colors.text,"LEVEL-%d",dlevel);
    if(game.lvlitems[dlevel]&liMAP)
      drawdmap(subscr);
    if(game.lvlitems[dlevel]&liCOMPASS)
    {
      int cs=9;
      if(((game.lvlitems[dlevel]&liTRIFORCE)==0) && (frame&16))
        cs--;
      overtile8(subscr,DOT,((DMaps[currdmap].compass&15)<<3)+18,((DMaps[currdmap].compass&0xF0)>>2)+16,cs,0);
    }
  }

  if(DMaps[currdmap].type==dmOVERW)
    overtile8(subscr,DOT,((homescr&15)<<2)+17,((homescr&0xF0)>>2)+16,6,0);
  else if(dlevel>0 && currscr<128)
    overtile8(subscr,DOT,(((homescr&15)-DMaps[currdmap].xoff)<<3)+18,((homescr&0xF0)>>2)+16,6,0);

  puttile8(subscr,6,88,16,1,0);
  puttile8(subscr,8,88,32,1,0);
  puttile8(subscr,10,88,40,0,0);
  putxnum(subscr,96,16,game.rupies);
  if(game.misc&iMKEY) textout(subscr,zfont,"XA",96,32,QMisc.colors.text);
  else putxnum(subscr,96,32,game.keys);
  putxnum(subscr,96,40,game.bombs);

  textout(subscr,zfont,"-LIFE-",184,16,QMisc.colors.caption);
  lifemeter(subscr,176,40);
  blueframe(subscr,120,16,3,4);
  blueframe(subscr,144,16,3,4);
  if(Bwpn)
    putitem(subscr,124,24,Bwpn);
  switch(game.sword) {
  case 1:
  case 2:
  case 3:
    putitem(subscr,148,24,game.sword-1+iSword); break;
  case 4:
    putitem(subscr,148,24,iXSword); break;
    break;
  }
  textout(subscr,zfont,"B",128,16,QMisc.colors.text);
  textout(subscr,zfont,"A",152,16,QMisc.colors.text);
  destroy_bitmap(subscr);
}


const byte tripiece[8*3] =
{
 112,112,0, 128,112,1, 96,128,0, 144,128,1,
 112,128,2, 112,128,1, 128,128,3, 128,128,0
};


void puttriframe(BITMAP *dest)
{
 line(dest,127,104,81,150,QMisc.colors.triframe);
 line(dest,128,104,174,150,QMisc.colors.triframe);
 hline(dest,80,151,175,QMisc.colors.triframe);
 line(dest,127,111,95,143,QMisc.colors.triframe);
 line(dest,128,111,160,143,QMisc.colors.triframe);
 hline(dest,95,144,160,QMisc.colors.triframe);

 for(int i=0; i<8; i++)
   if(game.lvlitems[i+1]&liTRIFORCE)
   {
     int tp = QMisc.triforce[i]-1;
     overtile16(dest,38,tripiece[tp*3],tripiece[tp*3+1],1,tripiece[tp*3+2]);
   }
}



void markBmap(int dir)
{
 if(DMaps[currdmap].type==dmOVERW)
   return;
 if(currscr>=128)
   return;

 byte drow = DMaps[currdmap].grid[currscr>>4];
 byte mask = 1 << (7-((currscr&15)-DMaps[currdmap].xoff));
 int di = ((currdmap-1)<<6) + ((currscr>>4)<<3) + ((currscr&15)-DMaps[currdmap].xoff);
 int code = 0;

 switch(DMaps[currdmap].type)
 {
 case dmDNGN:
 case dmBSDNGN:
   // check dmap
   if((drow&mask)==0)
     return;
   // calculate code
   for(int i=3; i>=0; i--) {
     code <<= 1;
     code += tmpscr[0].door[i]&1;
     }
   // mark the map
   game.bmaps[di] = code|128;
   break;

 default:
   game.bmaps[di] |= 128;
   if(dir>=0)
     game.bmaps[di] |= (1<<dir);
   break;
 }
}



const byte bmap[16*2] = {
0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,
0,3,0,0,0,3,0,0,0,0,3,0,0,0,3,0 };

void putBmap(BITMAP *dest)
{
 rectfill(dest,96,88,223,167,QMisc.colors.map);
 int si=0;
 for(int y=88; y<=88+72; y+=72)
  for(int x=96; x<96+128; x+=8) {
    if(bmap[si])
      puttile8(dest,21,x,y,1,bmap[si]-1);
    si++;
    }
 si=(currdmap-1)<<6;
 for(int y=96; y<160; y+=8)
  for(int x=128; x<192; x+=8) {
    if(game.bmaps[si]) {
      rectfill(dest,x+1,y+1,x+6,y+6,0);
      if(game.bmaps[si]&1) hline(dest,x+3,y,x+4,0);
      if(game.bmaps[si]&2) hline(dest,x+3,y+7,x+4,0);
      if(game.bmaps[si]&4) vline(dest,x,y+3,y+4,0);
      if(game.bmaps[si]&8) vline(dest,x+7,y+3,y+4,0);
      }
    si++;
    }
 if(currscr<128)
  overtile8(dest,DOT,(((homescr&15)-DMaps[currdmap].xoff)<<3)+130,((homescr&0xF0)>>1)+99,6,0);
}



void puttopsubscr()
{
  BITMAP *subscr = create_sub_bitmap(scrollbuf,0,0,256,174);
  clear(subscr);
  textout(subscr,zfont,"INVENTORY",32,24,QMisc.colors.caption);
  textout(subscr,zfont,"USE B BUTTON",16,72,QMisc.colors.text);
  textout(subscr,zfont,"FOR THIS",32,80,QMisc.colors.text);
  blueframe(subscr,56,40,4,4);
  blueframe(subscr,120,40,13,6);
  blit(scrollbuf,tmp_bmp,64,48,0,0,16,16);
  if(Bwpn) putitem(subscr,64,48,Bwpn);

  if(dlevel==0) {
    textout(subscr,zfont,"TRIFORCE",96,160,QMisc.colors.caption);
    puttriframe(subscr);
    }
  else {
    if(game.lvlitems[dlevel]&liMAP)
      putitem(subscr,44,112,21);
    if(game.lvlitems[dlevel]&liCOMPASS)
      putitem(subscr,44,152,22);
    textout(subscr,zfont,"MAP",40,96,QMisc.colors.caption);
    textout(subscr,zfont,"COMPASS",24,136,QMisc.colors.caption);
    putBmap(subscr);
    }

  if(game.misc&iRAFT)     putitem(subscr,128,24,iRaft);
  if(game.misc&iBOOK)     putitem(subscr,148,24,iBook);
  if(game.ring)           putitem(subscr,160,24,game.ring-1+iBRing);
  if(game.misc&iLADDER)   putitem(subscr,176,24,iLadder);
  if(game.misc&iMKEY)     putitem(subscr,192,24,iMKey);
  if(game.misc&iBRACELET) putitem(subscr,204,24,iBracelet);

  if(game.brang==1
    || game.brang==2)     putitem(subscr,128,48,game.brang-1+iBrang);
  if(game.brang==3)       putitem(subscr,128,48,iFBrang);
  if(game.bombs)          putitem(subscr,152,48,iBombs);
  if(game.arrow)          putitem(subscr,172,48,game.arrow-1+iArrow);
  if(game.misc&iBOW)      putitem(subscr,180,48,iBow);
  if(game.candle)         putitem(subscr,200,48,game.candle-1+iBCandle);

  if(game.whistle)        putitem(subscr,128,64,iWhistle);
  if(game.bait)           putitem(subscr,152,64,iBait);
  if(game.potion)         putitem(subscr,176,64,game.potion-1+iBPotion);
  else if(game.letter)    putitem(subscr,176,64,iLetter);
  if(game.wand)           putitem(subscr,200,64,iWand);

  destroy_bitmap(subscr);
}


void dosubscr()
{
pause_sfx(WAV_BRANG);
adjust_sfx(WAV_ER,128,false);
adjust_sfx(WAV_MSG,128,false);

blit(scrollbuf,scrollbuf,0,0,0,230,256,176);
puttopsubscr();

for(int y=174; y>=0; y-=3) {
  domoney();
  link.refill();
  putsubscr(scrollbuf,0,174);
  blit(scrollbuf,framebuf,0,y,0,0,256,224);
  advanceframe();
  if(Quit)
    return;
  }

bool done=false;
do {
  int pos=Bpos;
  if(rUp()||rDown()) selectBwpn(4);
  if(rLeft())  selectBwpn(-1);
  if(rRight()) selectBwpn(1);
  if(pos!=Bpos) {
    sfx(WAV_CHIME);
    blit(tmp_bmp,scrollbuf,0,0,64,48,16,16);
    putitem(scrollbuf,64,48,Bwpn);
    }
  domoney();
  link.refill();
  putsubscr(framebuf,0,174);
  blit(scrollbuf,framebuf,0,48,0,48,256,48);
  overtile16(framebuf,39,(Bpos&3)*24+128,((Bpos>>2)<<4)+48,((frame&8)>>3)+7,0);
  advanceframe();
  if(NESquit && Up() && cAbtn() && cBbtn())
    Quit=qQUIT;
  if(Quit)
    return;
  if(rSbtn())
    done=true;
  } while(!done);

for(int y=0; y<=174; y+=3) {
  domoney();
  link.refill();
  putsubscr(scrollbuf,0,174);
  blit(scrollbuf,framebuf,0,y,0,0,256,224);
  advanceframe();
  if(Quit)
    return;
  }

blit(scrollbuf,scrollbuf,0,230,0,0,256,176);
resume_sfx(WAV_BRANG);
}


/*** end of subscr.cc ***/
