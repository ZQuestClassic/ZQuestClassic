/*
  maps.cc
  Jeremy Craner, 1999
  Map and screen scrolling stuff for zelda.cc
*/


int dungeon()
{
 switch(dlevel)
 {
 case 0:
 case 0xA:
 case 0xB:
 case 0xC:
   return 0;
 }
 return dlevel;
}


inline int MAPDATA(int x,int y)
{
 int combo=(y&0xF0)+(x>>4);
 return tmpscr[0].data[combo];  	// entire combo code
}


inline int MAPFLAG(int x,int y)
{
 return MAPDATA(x,y) >> 12;
}


inline word cmb(int combo,int cset,int flags)
{
 return (combo&0x1FF)+((cset&7)<<9)+(flags<<12);
}


int WARPCODE(int dmap,int scr,int dw)
// returns: -1 = not a warp screen
//          0+ = warp screen code ( high byte=dmap, low byte=scr )
{
 if(TheMaps[DMaps[dmap].map*130+scr].room!=rWARP)
   return -1;

 int ring=TheMaps[DMaps[dmap].map*130+scr].catchall;
 int size=QMisc.warp[ring].size;
 if(size==0)
   return -1;

 int index=-1;
 for(int i=0; i<size; i++)
   if(dmap==QMisc.warp[ring].dmap[i] && scr==QMisc.warp[ring].scr[i])
     index=i;

 if(index==-1)
   return -1;

 index = (index+dw)%size;
 return (QMisc.warp[ring].dmap[index] << 8) + QMisc.warp[ring].scr[index];
}



bool iswater(int combo)
{
 return combobuf[combo&0x1FF].type == cWATER;
}


bool hiddenstair(int tmp,bool redraw) // index of tmpscr[]
{
 if(tmpscr[tmp].stairx || tmpscr[tmp].stairy) {
  int di=(tmpscr[tmp].stairy&0xF0)+(tmpscr[tmp].stairx>>4);
  if(dlevel) {
    tmpscr[tmp].data[di] &= 0x0E00;
    tmpscr[tmp].data[di] |= tcmb(43);
    }
  else {
    tmpscr[tmp].data[di] &= 0x0E00;
    tmpscr[tmp].data[di] |= tcmb(42);
    }
  if(redraw)
    putcombo(scrollbuf,tmpscr[tmp].stairx,tmpscr[tmp].stairy,tmpscr[tmp].data[di]);
  return true;
  }
 return false;
}


void hidden_entrance(int tmp,bool refresh)
{
 for(int i=0; i<176; i++) {
   if((tmpscr[tmp].data[i]>>12)==mfBURN_S) {  // burnt tree -> stairs
     tmpscr[tmp].data[i] &= 0x0E00;
     tmpscr[tmp].data[i] |= tcmb(42);
     if(refresh)
       putcombo(scrollbuf,(i&15)<<4,i&0xF0,tmpscr[tmp].data[i]);
     }
   if((tmpscr[tmp].data[i]>>12)==mfBOMB_C) {  // bombed rock -> cave
     tmpscr[tmp].data[i] &= 0x0E00;
     tmpscr[tmp].data[i] |= tcmb(51);
     if(refresh)
       putcombo(scrollbuf,(i&15)<<4,i&0xF0,tmpscr[tmp].data[i]);
     }
   }
}


void findentrance(int x,int y,int flag)
{
 if(MAPFLAG(x,y-1)!=flag && MAPFLAG(x+15,y-1)!=flag &&
    MAPFLAG(x,y+15)!=flag && MAPFLAG(x+15,y+15)!=flag)
   return;
 game.maps[(currmap<<7)+homescr]|=1;
 hidden_entrance(0,true);
 sfx(WAV_SECRET);
}


void doburn(int x,int y)
{
 findentrance(x,y,mfBURN_S);
}

void dobomb(int x,int y)
{
 findentrance(x,y,mfBOMB_C);
}

void bombdoor(int x,int y)
{
 if(tmpscr[0].door[0]==dBOMB && isinRect(x,y,100,0,139,48)) {
  tmpscr[0].door[0]=dBOMBED;
  putdoor(0,0,dBOMBED);
  game.maps[(currmap<<7)+homescr]    |= 1;
  game.maps[(currmap<<7)+homescr-16] |= 2;
  markBmap(-1);
  }
 if(tmpscr[0].door[1]==dBOMB && isinRect(x,y,100,112,139,176)) {
  tmpscr[0].door[1]=dBOMBED;
  putdoor(0,1,dBOMBED);
  game.maps[(currmap<<7)+homescr]    |= 2;
  game.maps[(currmap<<7)+homescr+16] |= 1;
  markBmap(-1);
  }
 if(tmpscr[0].door[2]==dBOMB && isinRect(x,y,0,60,48,98)) {
  tmpscr[0].door[2]=dBOMBED;
  putdoor(0,2,dBOMBED);
  game.maps[(currmap<<7)+homescr]   |= 4;
  game.maps[(currmap<<7)+homescr-1] |= 8;
  markBmap(-1);
  }
 if(tmpscr[0].door[3]==dBOMB && isinRect(x,y,192,60,240,98)) {
  tmpscr[0].door[3]=dBOMBED;
  putdoor(0,3,dBOMBED);
  game.maps[(currmap<<7)+homescr]   |= 8;
  game.maps[(currmap<<7)+homescr+1] |= 4;
  markBmap(-1);
  }
}



// default is to set the item flag which depends on currscr
inline void setmapflag()
{
 game.maps[(currmap<<7)+homescr] |= ((currscr>=128) ? 32 : 16);
}

inline bool getmapflag()
{
 return (game.maps[(currmap<<7)+homescr] & ((currscr>=128) ? 32 : 16))!=0;
}


// set specific flag
inline void setmapflag(int flag)
{
 game.maps[(currmap<<7)+homescr] |= flag;
}

inline bool getmapflag(int flag)
{
 return (game.maps[(currmap<<7)+homescr] & flag) != 0;
}

// get template combo
inline word tcmb(int pos)
{
 return TheMaps[TEMPLATE].data[pos]&0x1FF;
}


void putdoor(int t,int side,int door,bool redraw=true)
{
 switch(side) {
 case up:
   switch(door) {
   case dOPEN:
   case dUNLOCKED: tmpscr[t].data[23]=cmb(tcmb(53),2,0); tmpscr[t].data[24]=cmb(tcmb(54),2,0); break;
   case dLOCKED:   tmpscr[t].data[23]=cmb(tcmb(39),2,0); tmpscr[t].data[24]=cmb(tcmb(40),2,0); break;
   case d1WAYSHUTTER:
   case dSHUTTER:  tmpscr[t].data[23]=cmb(tcmb(55),2,0); tmpscr[t].data[24]=cmb(tcmb(56),2,0); break;
   case dBOMBED:   tmpscr[t].data[23]=cmb(tcmb(71),2,0); tmpscr[t].data[24]=cmb(tcmb(72),2,0); break;
   }
   if(redraw) {
     putcombo(scrollbuf,112,16,tmpscr[0].data[23]);
     putcombo(scrollbuf,128,16,tmpscr[0].data[24]);
   }
   break;

 case down:
   switch(door) {
   case dOPEN:
   case dUNLOCKED: tmpscr[t].data[151]=cmb(tcmb(117),2,0); tmpscr[t].data[152]=cmb(tcmb(118),2,0); break;
   case dLOCKED:   tmpscr[t].data[151]=cmb(tcmb(135),2,0); tmpscr[t].data[152]=cmb(tcmb(136),2,0); break;
   case d1WAYSHUTTER:
   case dSHUTTER:  tmpscr[t].data[151]=cmb(tcmb(119),2,0); tmpscr[t].data[152]=cmb(tcmb(120),2,0); break;
   case dBOMBED:   tmpscr[t].data[151]=cmb(tcmb(103),2,0); tmpscr[t].data[152]=cmb(tcmb(104),2,0); break;
   }
   if(redraw) {
     putcombo(scrollbuf,112,144,tmpscr[0].data[151]);
     putcombo(scrollbuf,128,144,tmpscr[0].data[152]);
   }
   break;

 case left:
   switch(door) {
   case dOPEN:
   case dUNLOCKED: tmpscr[t].data[81]=cmb(tcmb(83),2,0); break;
   case dLOCKED:   tmpscr[t].data[81]=cmb(tcmb(68),2,0); break;
   case d1WAYSHUTTER:
   case dSHUTTER:  tmpscr[t].data[81]=cmb(tcmb(84),2,0); break;
   case dBOMBED:   tmpscr[t].data[81]=cmb(tcmb(100),2,0); break;
   }
   if(redraw)
     putcombo(scrollbuf,16,80,tmpscr[0].data[81]);
   break;

 case right:
   switch(door) {
   case dOPEN:
   case dUNLOCKED: tmpscr[t].data[94]=cmb(tcmb(92),2,0); break;
   case dLOCKED:   tmpscr[t].data[94]=cmb(tcmb(75),2,0); break;
   case d1WAYSHUTTER:
   case dSHUTTER:  tmpscr[t].data[94]=cmb(tcmb(91),2,0); break;
   case dBOMBED:   tmpscr[t].data[94]=cmb(tcmb(107),2,0); break;
   }
   if(redraw)
     putcombo(scrollbuf,224,80,tmpscr[0].data[94]);
   break;
 }
}


void openshutters()
{
 for(int i=0; i<4; i++)
  if(tmpscr[0].door[i]==dSHUTTER) {
    putdoor(0,i,dOPEN);
    tmpscr[0].door[i]=dOPENSHUTTER;
    }
 sfx(WAV_DOOR,128);
}


void loadscr(int tmp,int scr,int ldir)
{
 tmpscr[tmp] = TheMaps[currmap*130+scr];
 if(dlevel==0 && game.maps[(currmap<<7)+scr]&1) { // if special stuff done before
   hiddenstair(tmp,false);
   hidden_entrance(tmp,false);
  }
 // check doors
 if(dlevel) {
  for(int i=0; i<4; i++) {
    int door=tmpscr[tmp].door[i];
    bool putit=true;
    switch(door) {
     case d1WAYSHUTTER:
     case dSHUTTER:
      if((ldir^1)==i)
        putit=false;
      break;
     case dLOCKED:
      if(game.maps[(currmap<<7)+scr]&(1<<i)) {
        tmpscr[tmp].door[i]=dUNLOCKED;
        putit=false;
        }
      break;
     case dBOMB:
      if(game.maps[(currmap<<7)+scr]&(1<<i))
        tmpscr[tmp].door[i]=dBOMBED;
      break;
     }
    if(putit)
      putdoor(tmp,i,tmpscr[tmp].door[i],false);
    }
  }
}




void putscr(BITMAP* dest,int x,int y,mapscr* screen)
{
  if(screen->valid==0) {
    rectfill(dest,x,y,x+255,y+175,0);
    return;
    }
  for(int i=0; i<176; i++)
    putcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,screen->data[i]);
}



bool _walkflag(int x,int y,int cnt)
{
 if(x<0||y<0) return false;
 if(x>248) return false;
 if(x>240&&cnt==2) return false;
 if(y>168) return false;

 int bx=(x>>4)+(y&0xF0);
 combo c = combobuf[(tmpscr[0].data[bx])&0x1FF];
 bool dried = (c.type==cWATER) && (whistleclk>=88);
 int b=1;

 if(x&8) b<<=2;
 if(y&8) b<<=1;
 if((c.walk&b) && !dried)
   return true;
 if(cnt==1) return false;

 if(x&8) {
   c = combobuf[(tmpscr[0].data[++bx])&0x1FF];
   dried = (c.type==cWATER) && (whistleclk>=88);
   b=1;
   if(y&8) b<<=1;
   }
 else b<<=2;
 return (c.walk&b) ? !dried : false;
}



bool water_walkflag(int x,int y,int cnt)
{
 if(x<0||y<0) return false;
 if(x>248) return false;
 if(x>240&&cnt==2) return false;
 if(y>168) return false;

 int bx=(x>>4)+(y&0xF0);
 combo c = combobuf[(tmpscr[0].data[bx])&0x1FF];
 int b=1;

 if(x&8) b<<=2;
 if(y&8) b<<=1;
 if((c.walk&b) && (c.type!=cWATER))
   return true;
 if(cnt==1) return false;

 if(x&8) {
   c = combobuf[(tmpscr[0].data[++bx])&0x1FF];
   b=1;
   if(y&8) b<<=1;
   }
 else b<<=2;
 return (c.walk&b) ? (c.type!=cWATER) : false;
}



bool hit_walkflag(int x,int y,int cnt)
{
 if(dlevel)
   if(x<32 || y<40 || (x+(cnt-1)*8)>=224 || y>=144)
     return true;
 if(blockpath && y<88)
   return true;
 if(x<16 || y<16 || (x+(cnt-1)*8)>=240 || y>=160)
   return true;
 for(int i=0; i<4; i++)
   if(mblock[i].clk && mblock[i].hit(x,y,cnt*8,1))
     return true;
 return _walkflag(x,y,cnt);
}


void map_bkgsfx()
{
  if(tmpscr[0].flags&fSEA)   cont_sfx(WAV_SEA);
  if(tmpscr[0].flags&fROAR && !(game.lvlitems[dlevel]&liBOSS))
    cont_sfx((tmpscr[0].flags2&fVADER)?WAV_VADER:WAV_ROAR);
}


/*** end of maps.cc ***/
