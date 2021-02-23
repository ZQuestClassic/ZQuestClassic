//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  maps.cc
//
//  Map and screen scrolling stuff for zelda.cc
//
//--------------------------------------------------------


void clear_dmap(byte i) {
  DMaps[i].map=0;
  DMaps[i].level=0;
  DMaps[i].xoff=0;
  DMaps[i].compass=0;
  DMaps[i].color=0;
  DMaps[i].midi=0;
  DMaps[i].cont=0;
  DMaps[i].type=0;
  DMaps[i].grid[0]=0;
  DMaps[i].grid[1]=0;
  DMaps[i].grid[2]=0;
  DMaps[i].grid[3]=0;
  DMaps[i].grid[4]=0;
  DMaps[i].grid[5]=0;
  DMaps[i].grid[6]=0;
  DMaps[i].grid[7]=0;
}

void clear_dmaps() {
  for (byte i=0; i<=255; i++) {
    clear_dmap(i);
  }
}

int isdungeon()
{
  // overworlds should always be dlevel 0
  if(dlevel == 0)
    return 0;

  // dungeons can have any dlevel above 0
  if((DMaps[currdmap].type&dmfTYPE) == dmDNGN)
    return dlevel;

  // dlevels that aren't dungeons are caves
  return 0;
}


inline int MAPDATA(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return tmpscr->data[combo];  	// entire combo code
}


inline int MAPFLAG(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return tmpscr->sflag[combo]; 	// flag
}

/*
inline int FLAG(int combo)
{
  return combo >> 10+1;
}
*/

inline int MAPCOMBO(int x,int y)
{
  return (MAPDATA(x,y)&0xFF)+(tmpscr->cpage<<8);
}

inline int COMBOTYPE(int x,int y)
{
  return combobuf[MAPCOMBO(x,y)].type;
}

inline int cmb(int combo,int cset)
{
  return (combo&0xFF)+((cset&7)<<8);
}

// get template combo
inline int tcmb(int pos)
{
  return TheMaps[currmap*MAPSCRS+TEMPLATE].data[pos]&0xFF;
}

inline int tcmbdat(int pos)
{
  return TheMaps[currmap*MAPSCRS+TEMPLATE].data[pos];
}

inline int tcmbflag(int pos)
{
  return TheMaps[currmap*MAPSCRS+TEMPLATE].sflag[pos];
}


// default is to set the item flag which depends on currscr
inline void setmapflag()
{
  game.maps[(currmap<<7)+homescr] |= ((currscr>=128) ? mBELOW : mITEM);
}

inline void unsetmapflag()
{
  game.maps[(currmap<<7)+homescr] &= ((currscr>=128) ? ~mBELOW : ~mITEM);
}

inline bool getmapflag()
{
  return (game.maps[(currmap<<7)+homescr] & ((currscr>=128) ? mBELOW : mITEM))!=0;
}


// set specific flag
inline void setmapflag(int flag)
{
  game.maps[(currmap<<7)+homescr] |= flag;
}

inline void unsetmapflag(int flag)
{
  game.maps[(currmap<<7)+homescr] &= ~flag;
}

inline bool getmapflag(int flag)
{
  return (game.maps[(currmap<<7)+homescr] & flag) != 0;
}



int WARPCODE(int dmap,int scr,int dw)
// returns: -1 = not a warp screen
//          0+ = warp screen code ( high byte=dmap, low byte=scr )
{
  mapscr *s = TheMaps+ (DMaps[dmap].map*MAPSCRS+scr);
  if(s->room!=rWARP)
    return -1;

  int ring=s->catchall;
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



inline bool iswater(int combo)
{
  int type = combobuf[(combo&0xFF)+(tmpscr->cpage<<8)].type;
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP;
}

inline bool iswater_type(int type)
{
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP;
}

inline bool isstepable(int combo)
{
  int type = combobuf[(combo&0xFF)+(tmpscr->cpage<<8)].type;
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP || type==cLADDERONLY
      || type==cPIT || type==cHOOKSHOT;
}



bool hiddenstair(int tmp,bool redraw) // tmp = index of tmpscr[]
{
  mapscr *s = tmpscr + tmp;

  if(s->stairx || s->stairy)
  {
    int di = (s->stairy&0xF0)+(s->stairx>>4);
    s->data[di] = tcmbdat(s->secret[3]);
    s->sflag[di] = tcmbflag(s->secret[3]);
    if(redraw)
      putcombo(scrollbuf,s->stairx,s->stairy,s->data[di],s->cpage);
    return true;
  }
  return false;
}



void hidden_entrance(int tmp,bool refresh,bool high16only=false)
{
  mapscr *s = tmpscr + tmp;

  for(int i=0; i<176; i++)
  {
    bool putit = true;

    if(!high16only)
    {
      switch(s->sflag[i])
      {
      case mfBURN:  s->data[i] = tcmbdat(s->secret[0]); s->sflag[i] = tcmbflag(s->secret[0]); break;
      case mfARROW: s->data[i] = tcmbdat(s->secret[1]); s->sflag[i] = tcmbflag(s->secret[1]); break;
      case mfSBOMB:
      case mfBOMB:  s->data[i] = tcmbdat(s->secret[2]); s->sflag[i] = tcmbflag(s->secret[2]); break;
      default: putit = false; break;
      }
    }

    // if it's an enemies->secret screen, only do the high 16 if told to
    // that way you can have secret and burn/bomb entrance separately
    if(!(s->flags2&fCLEARSECRET) || high16only)
      if(((s->sflag[i]) > 15)&&((s->sflag[i]) < 32))
      {
        s->data[i] = tcmbdat(s->secret[(s->sflag[i])-16+4]);
        s->sflag[i] = tcmbflag(s->secret[(s->sflag[i])-16+4]);
        putit = true;
      }

    if(putit && refresh)
      putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cpage);
  }
}


bool findentrance(int x, int y, int flag, bool setflag)
{
  if(MAPFLAG(x,y-1)!=flag && MAPFLAG(x+15,y-1)!=flag &&
     MAPFLAG(x,y+15)!=flag && MAPFLAG(x+15,y+15)!=flag)
    return false;

  if(setflag && !isdungeon())
    setmapflag(mSECRET);
  hidden_entrance(0,true);
  sfx(WAV_SECRET);
  return true;
}

bool hitcombo(int x, int y, int combotype)
{
  return (COMBOTYPE(x,y)==combotype);
}

bool hitflag(int x, int y, int flagtype)
{
  return (MAPFLAG(x,y)==flagtype);
}

int nextscr(int dir)
{
  int m = currmap;
  int s = currscr;

  switch(dir)
  {
  case up:    s-=16; break;
  case down:  s+=16; break;
  case left:  s-=1;  break;
  case right: s+=1;  break;
  }

  // need to check for screens on other maps, 's' not valid, etc.

  if(tmpscr->warptype2 == 3) // scrolling warp
  {
    switch(dir)
    {
    case up:    if(!(tmpscr->flags2&wfUP))    goto skip; break;
    case down:  if(!(tmpscr->flags2&wfDOWN))  goto skip; break;
    case left:  if(!(tmpscr->flags2&wfLEFT))  goto skip; break;
    case right: if(!(tmpscr->flags2&wfRIGHT)) goto skip; break;
    }
    m = DMaps[tmpscr->warpdmap2].map;
    s = tmpscr->warpscr2 + DMaps[tmpscr->warpdmap2].xoff;
  }

skip:

  return (m<<7) + s;
}



void bombdoor(int x,int y)
{
  if(tmpscr->door[0]==dBOMB && isinRect(x,y,100,0,139,48))
  {
    tmpscr->door[0]=dBOMBED;
    putdoor(0,0,dBOMBED);
    game.maps[(currmap<<7)+homescr] |= 1;
    game.maps[nextscr(up)] |= 2;
    markBmap(-1);
  }
  if(tmpscr->door[1]==dBOMB && isinRect(x,y,100,112,139,176))
  {
    tmpscr->door[1]=dBOMBED;
    putdoor(0,1,dBOMBED);
    game.maps[(currmap<<7)+homescr] |= 2;
    game.maps[nextscr(down)] |= 1;
    markBmap(-1);
  }
  if(tmpscr->door[2]==dBOMB && isinRect(x,y,0,60,48,98))
  {
    tmpscr->door[2]=dBOMBED;
    putdoor(0,2,dBOMBED);
    game.maps[(currmap<<7)+homescr] |= 4;
    game.maps[nextscr(left)] |= 8;
    markBmap(-1);
  }
  if(tmpscr->door[3]==dBOMB && isinRect(x,y,192,60,240,98))
  {
    tmpscr->door[3]=dBOMBED;
    putdoor(0,3,dBOMBED);
    game.maps[(currmap<<7)+homescr] |= 8;
    game.maps[nextscr(right)] |= 4;
    markBmap(-1);
  }
}
/*
void do_overhead(int type) {
//  do_overhead(type, TheMaps+(currmap*MAPSCRS+currscr), 0, 0);
  do_overhead(type, tmpscr, 0, 0);
}
*/

void do_layer2(int type, mapscr* layer, int x, int y) {
//  return;
  //-1=over combo
  //3=ground over screen
  //5=sky over screen
  int layermap, layerscreen;

  layermap=layer->layermap[type]-1;
  if (layermap>-1) {
    layerscreen=layermap*MAPSCRS+layer->layerscreen[type];
  }
  switch (type) {
    case -1:
      for (int i=0; i<176; i++) {
        if (combobuf[((layer->data[i]&0xFF)+(layer->cpage<<8))].type==cOVERHEAD) {
          overcombo(framebuf,((i&15)<<4)-x,(i&0xF0)+56-y,layer->data[i],layer->cpage);
        }
      }
      break;
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      if (layermap>-1) {
        for (int i=0; i<176; i++) {
          overcombo(framebuf,((i&15)<<4)-x,(i&0xF0)+56-y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cpage);
        }
      }
      break;
  }
}


void put_vert_door(int t,int pos,int tmppos,int cs,int page,bool redraw)
{
  tmpscr[t].data[pos]   = cmb(tcmb(tmppos),cs);
  tmpscr[t].sflag[pos]  = 0;
  tmpscr[t].data[pos+1] = cmb(tcmb(tmppos+1),cs);
  tmpscr[t].sflag[pos+1]  = 0;

  if(redraw)
  {
    putcombo(scrollbuf,(pos&15)<<4,pos&0xF0,cmb(tcmb(tmppos),cs),page);
    putcombo(scrollbuf,((pos&15)<<4)+16,pos&0xF0,cmb(tcmb(tmppos+1),cs),page);
  }
}


void over_vert_door(int pos,int tmppos,int cs,int page)
{
  overcombo(scrollbuf,(pos&15)<<4,pos&0xF0,cmb(tcmb(tmppos),cs),page);
  overcombo(scrollbuf,((pos&15)<<4)+16,pos&0xF0,cmb(tcmb(tmppos+1),cs),page);
}



void put_horiz_door(int t,int pos,int tmppos,int cs,int page,bool redraw)
{
  tmpscr[t].data[pos]   = cmb(tcmb(tmppos),cs);
  tmpscr[t].sflag[pos]  = 0;
  tmpscr[t].data[pos+16] = cmb(tcmb(tmppos+16),cs);
  tmpscr[t].sflag[pos+16]  = 0;
  tmpscr[t].data[pos+32] = cmb(tcmb(tmppos+32),cs);
  tmpscr[t].sflag[pos+32]  = 0;

  if(redraw)
  {
    putcombo(scrollbuf,(pos&15)<<4,pos&0xF0,cmb(tcmb(tmppos),cs),page);
    putcombo(scrollbuf,((pos&15)<<4),(pos&0xF0)+16,cmb(tcmb(tmppos+16),cs),page);
    putcombo(scrollbuf,((pos&15)<<4),(pos&0xF0)+32,cmb(tcmb(tmppos+32),cs),page);
  }
}



void putdoor(int t,int side,int door,bool redraw=true)
{
 int page = tmpscr[t].cpage;

 switch(side) {
 case up:
   switch(door) {
   case dOPEN:
   case dUNLOCKED:    put_vert_door(t,23,53,2,page,redraw); break;
   case dLOCKED:      put_vert_door(t,23,39,2,page,redraw); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_vert_door(t,23,55,2,page,redraw); break;
   case dBOMBED:      put_vert_door(t,23,41,2,page,redraw);
                      if(redraw)
                        over_vert_door(39,57,2,page);
                      break;
   }
   break;

 case down:
   switch(door) {
   case dOPEN:
   case dUNLOCKED:    put_vert_door(t,151,117,2,page,redraw); break;
   case dLOCKED:      put_vert_door(t,151,135,2,page,redraw); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_vert_door(t,151,119,2,page,redraw); break;
   case dBOMBED:      put_vert_door(t,151,137,2,page,redraw);
                      if(redraw)
                        over_vert_door(135,121,2,page);
                      break;
   }
   break;

 case left:
   switch(door) {
   case dOPEN:
   case dUNLOCKED:    put_horiz_door(t,65,67,2,page,redraw); break;
   case dLOCKED:      put_horiz_door(t,65,68,2,page,redraw); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_horiz_door(t,65,69,2,page,redraw); break;
   case dBOMBED:      tmpscr[t].data[81] = cmb(tcmb(114),2);
                      tmpscr[t].sflag[81] = 0;
                      if(redraw)
                      {
                        putcombo(scrollbuf,16,80,cmb(tcmb(114),2),page);
                        overcombo(scrollbuf,32,80,cmb(tcmb(115),2),page);
                      }
                      break;
   }
   break;

 case right:
   switch(door) {
   case dOPEN:
   case dUNLOCKED:    put_horiz_door(t,78,76,2,page,redraw); break;
   case dLOCKED:      put_horiz_door(t,78,75,2,page,redraw); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_horiz_door(t,78,74,2,page,redraw); break;
   case dBOMBED:      tmpscr[t].data[94] = cmb(tcmb(125),2);
                      tmpscr[t].sflag[94] = 0;
                      if(redraw)
                      {
                        putcombo(scrollbuf,224,80,cmb(tcmb(125),2),page);
                        overcombo(scrollbuf,208,80,cmb(tcmb(124),2),page);
                      }
                      break;
   }
   break;
 }
}


void openshutters()
{
  for(int i=0; i<4; i++)
    if(tmpscr->door[i]==dSHUTTER)
    {
      putdoor(0,i,dOPEN);
      tmpscr->door[i]=dOPENSHUTTER;
    }
  sfx(WAV_DOOR,128);
}


void loadscr(int tmp,int scr,int ldir)
{
  tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];
  if(!isdungeon() && (game.maps[(currmap<<7)+scr]&mSECRET))
  { // if special stuff done before
    hiddenstair(tmp,false);
    hidden_entrance(tmp,false);
  }

  // check doors
  if(isdungeon())
  {
    for(int i=0; i<4; i++)
    {
      int door=tmpscr[tmp].door[i];
      bool putit=true;

      switch(door)
      {
      case d1WAYSHUTTER:
      case dSHUTTER:
        if((ldir^1)==i)
          putit=false;
        break;

      case dLOCKED:
        if(game.maps[(currmap<<7)+scr]&(1<<i))
        {
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
    putcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,screen->data[i],screen->cpage);

  if(screen->door[0]==dBOMBED)
  {
    overcombo(scrollbuf,112+x,32+y,cmb(tcmb(57),2),screen->cpage);
    overcombo(scrollbuf,128+x,32+y,cmb(tcmb(58),2),screen->cpage);
  }
  if(screen->door[1]==dBOMBED)
  {
    overcombo(scrollbuf,112+x,128+y,cmb(tcmb(121),2),screen->cpage);
    overcombo(scrollbuf,128+x,128+y,cmb(tcmb(122),2),screen->cpage);
  }
  if(screen->door[2]==dBOMBED)
    overcombo(scrollbuf,32+x,80+y,cmb(tcmb(115),2),screen->cpage);

  if(screen->door[3]==dBOMBED)
    overcombo(scrollbuf,208+x,80+y,cmb(tcmb(124),2),screen->cpage);
}



bool _walkflag(int x,int y,int cnt)
{
  if(x<0||y<0) return false;
  if(x>248) return false;
  if(x>240&&cnt==2) return false;
  if(y>168) return false;
  mapscr *s1, *s2;
  s1=(((*tmpscr).layermap[0]-1)>=0)?
       (TheMaps+((*tmpscr).layermap[0]-1)*MAPSCRS+((*tmpscr).layerscreen[0])):
       tmpscr;
  s2=(((*tmpscr).layermap[1]-1)>=0)?
       (TheMaps+((*tmpscr).layermap[1]-1)*MAPSCRS+((*tmpscr).layerscreen[1])):
       tmpscr;
//  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);

  int bx=(x>>4)+(y&0xF0);
  newcombo c = combobuf[(((*tmpscr).data[bx])&0xFF)+((*tmpscr).cpage<<8)];
  newcombo c1 = combobuf[(((*s1).data[bx])&0xFF)+((*s1).cpage<<8)];
  newcombo c2 = combobuf[(((*s2).data[bx])&0xFF)+((*s2).cpage<<8)];
  bool dried = (((iswater_type(c.type)) && (iswater_type(c1.type)) &&
                 (iswater_type(c2.type))) && (whistleclk>=88));
  int b=1;

  if(x&8) b<<=2;
  if(y&8) b<<=1;
  if(((c.walk&b) || (c1.walk&b) || (c2.walk&b)) && !dried)
    return true;
  if(cnt==1) return false;

  bx++;
  if(!(x&8))
    b<<=2;
  else
  {
    c  = combobuf[(((*tmpscr).data[bx])&0xFF)+((*tmpscr).cpage<<8)];
    c1 = combobuf[(((*s1).data[bx])&0xFF)+((*s1).cpage<<8)];
    c2 = combobuf[(((*s2).data[bx])&0xFF)+((*s2).cpage<<8)];
    dried = (((iswater_type(c.type)) && (iswater_type(c1.type)) &&
              (iswater_type(c2.type))) && (whistleclk>=88));
    b=1;
    if(y&8) b<<=1;
  }

  return ((c.walk&b)||(c1.walk&b)||(c2.walk&b)) ? !dried : false;
}



bool water_walkflag(int x,int y,int cnt)
{
  if(x<0||y<0) return false;
  if(x>248) return false;
  if(x>240&&cnt==2) return false;
  if(y>168) return false;

  int bx=(x>>4)+(y&0xF0);
  newcombo c = combobuf[((tmpscr->data[bx])&0xFF)+(tmpscr->cpage<<8)];
  int b=1;

  if(x&8) b<<=2;
  if(y&8) b<<=1;
  if((c.walk&b) && !iswater_type(c.type))
    return true;
  if(cnt==1) return false;

  if(x&8)
    b<<=2;
  else
  {
    c = combobuf[((tmpscr->data[++bx])&0xFF)+(tmpscr->cpage<<8)];
    b=1;
    if(y&8) b<<=1;
  }

  return (c.walk&b) ? !iswater_type(c.type) : false;
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
  if(tmpscr->flags&fSEA)   cont_sfx(WAV_SEA);
  if(tmpscr->flags&fROAR && !(game.lvlitems[dlevel]&liBOSS))
  {
    if(tmpscr->flags3&fDODONGO)
      cont_sfx(WAV_DODONGO);
    else if(tmpscr->flags2&fVADER)
      cont_sfx(WAV_VADER);
    else
      cont_sfx(WAV_ROAR);
  }
}



/****  View Map  ****/


BITMAP *mappic = NULL;
int    mapres = 0;


void ViewMap()
{
  static double scales[17] = {
    0.03125, 0.04419, 0.0625, 0.08839, 0.125, 0.177, 0.25, 0.3535,
    0.50, 0.707, 1.0, 1.414, 2.0, 2.828, 4.0, 5.657, 8.0 };

  int px = ((8-(currscr&15)) << 9)  - 256;
  int py = ((4-(currscr>>4)) * 352) - 176;
  int lx = ((currscr&15)<<8)  + LinkX()+8;
  int ly = ((currscr>>4)*176) + LinkY()+8;
  int sc = 6;

  bool done=false, redraw=true;

  if(!mappic)
  {
    mappic = create_bitmap((256*16)>>mapres,(176*8)>>mapres);

    if(!mappic)
    {
      system_pal();
      jwin_alert("View Map","Not enough memory.",NULL,NULL,"OK",NULL,13,27);
      game_pal();
      return;
    }
  }

  int layermap, layerscreen;
  int x2=256, y2=-56;

  // draw the map
  for(int y=0; y<8; y++)
    for(int x=0; x<16; x++)
    {
      int s = (y<<4) + x;

      if(!(game.maps[(currmap<<7)+s]&mVISITED))
        rectfill(scrollbuf, 256, 0, 511, 223, WHITE);
/*
      else if(s == currscr)
        blit(scrollbuf, scrollbuf, 0, 0, 256, 0, 256, 176);
*/
      else
      {
        loadscr(1,s,-1);
        putscr(scrollbuf, 256, 0, tmpscr+1);
//        TheMaps[currmap*MAPSCRS+s]


        for (int k=0; k<4; k++) {
          layermap=TheMaps[currmap*MAPSCRS+s].layermap[k]-1;
          if (layermap>-1) {
            layerscreen=layermap*MAPSCRS+TheMaps[currmap*MAPSCRS+s].layerscreen[k];
            for (int i=0; i<176; i++) {
              overcombo(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cpage);
            }
          }
        }

        for (int i=0; i<176; i++) {
          if (COMBOTYPE((i&15)<<4,i&0xF0)==cOVERHEAD) {
            overcombo(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,MAPDATA((i&15)<<4,i&0xF0),TheMaps[currmap*MAPSCRS+s].cpage);
          }
        }

        for (int k=4; k<6; k++) {
          layermap=TheMaps[currmap*MAPSCRS+s].layermap[k]-1;
          if (layermap>-1) {
            layerscreen=layermap*MAPSCRS+TheMaps[currmap*MAPSCRS+s].layerscreen[k];
            for (int i=0; i<176; i++) {
              overcombo(scrollbuf,((i&15)<<4)+x2,(i&0xF0)+56+y2,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cpage);
            }
          }
        }
      }
      stretch_blit(scrollbuf, mappic, 256, 0, 256, 176,
                   x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
    }

  clear_keybuf();

  // view it
  int delay = 0;
  static int show  = 3;

  do {
    int step = int(16.0/scales[sc]);
    step = (step>>1) + (step&1);
    bool r = cRbtn();

    if(cLbtn())
    {
      step <<= 2;
      delay = 0;
    }

    if(r)
    {
      if(rUp())    { py+=step; redraw=true; }
      if(rDown())  { py-=step; redraw=true; }
      if(rLeft())  { px+=step; redraw=true; }
      if(rRight()) { px-=step; redraw=true; }
    }
    else
    {
      if(Up())    { py+=step; redraw=true; }
      if(Down())  { py-=step; redraw=true; }
      if(Left())  { px+=step; redraw=true; }
      if(Right()) { px-=step; redraw=true; }
    }


    if(delay)
      delay--;
    else
    {
      bool a = cAbtn();
      bool b = cBbtn();
      if(a && !b)  { sc=min(sc+1,16); delay=8; redraw=true; }
      if(b && !a)  { sc=max(sc-1,0);  delay=8; redraw=true; }
    }

    if(rPbtn())
      show--;

    px = vbound(px,-4096,4096);
    py = vbound(py,-1408,1408);

    double scale = scales[sc];

    if(!redraw)
    {
      blit(scrollbuf,framebuf,256,0,0,0,256,224);
    }
    else
    {
      clear_to_color(framebuf,BLACK);
      stretch_blit(mappic,framebuf,0,0,mappic->w,mappic->h,
          int(256+(px-mappic->w)*scale)/2,int(224+(py-mappic->h)*scale)/2,
          int(mappic->w*scale),int(mappic->h*scale));

      blit(framebuf,scrollbuf,0,0,256,0,256,224);
      redraw=false;
    }


    int x = int(256+(px-((2048-lx)*2))*scale)/2;
    int y = int(224+(py-((704-ly)*2))*scale)/2;

    if(show&1)
    {
      line(framebuf,x-7,y-7,x+7,y+7,(frame&3)+252);
      line(framebuf,x+7,y-7,x-7,y+7,(frame&3)+252);
    }

    text_mode(BLACK);

    if(show&2 || r)
      textprintf(framebuf,font,224,216,WHITE,"%1.2f",scale);

    if(r)
    {
      textprintf(framebuf,font,0,208,WHITE,"m: %d %d",px,py);
      textprintf(framebuf,font,0,216,WHITE,"x: %d %d",x,y);
    }


    advanceframe();
    if(rSbtn())
      done = true;

  } while(!done && !Quit);
}



int onViewMap()
{
  if(Playing && currscr<128 && dlevel==0)
  {
    if(get_bit(QHeader.rules,qr1_VIEWMAP))
    {
      clear_to_color(framebuf,BLACK);
      text_mode(BLACK);
      textout_centre(framebuf,font,"Drawing map...",128,108,WHITE);
      advanceframe();
      ViewMap();
    }
  }
  return D_O_K;
}


/*** end of maps.cc ***/
