//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  maps.cc
//
//  Map and screen scrolling stuff for zelda.cc
//
//--------------------------------------------------------

#include "maps.h"
#include "zelda.h"
#include "tiles.h"
#include "sprite.h"
#include "jwin.h"
#include "zsys.h"
#include "subscr.h"
#include "link.h"
#include "guys.h"
#include <string.h>
#include "particles.h"

extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;
extern movingblock mblock2;                                 //mblock[4]?
extern LinkClass Link;

int draw_screen_clip_rect_x1=0;
int draw_screen_clip_rect_x2=255;
int draw_screen_clip_rect_y1=0;
int draw_screen_clip_rect_y2=223;
bool draw_screen_clip_rect_show_link=true;
bool draw_screen_clip_rect_show_guys=false;

void clear_dmap(byte i)
{
  memset(&DMaps[i],0,sizeof(dmap));
}

void clear_dmaps()
{
  for (int i=0; i<=255; i++)
  {
    clear_dmap(i);
  }
}

int isdungeon()
{
  // overworlds should always be dlevel 0
  /*
    if(dlevel == 0)
      return 0;
  */

  // dungeons can have any dlevel above 0
  if((DMaps[currdmap].type&dmfTYPE) == dmDNGN)
    return dlevel;

  // dlevels that aren't dungeons are caves
  return 0;
}

int MAPDATA(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return tmpscr->data[combo];                               // entire combo code
}

int MAPCSET(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return tmpscr->cset[combo];                               // entire combo code
}

int MAPFLAG(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return tmpscr->sflag[combo];                              // flag
}

int COMBOTYPE(int x,int y)
{
  return combobuf[MAPDATA(x,y)].type;
}

int MAPDATA2(int layer,int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  if (tmpscr2[layer].valid==0)
  {
    return 0;
  }
  return tmpscr2[layer].data[combo];                        // entire combo code
}

int MAPCSET2(int layer,int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  if (tmpscr2[layer].valid==0)
  {
    return 0;
  }
  return tmpscr2[layer].cset[combo];                        // entire combo code
}

int MAPFLAG2(int layer,int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  if (tmpscr2[layer].valid==0)
  {
    return 0;
  }
  return tmpscr2[layer].sflag[combo];                       // flag
}

int COMBOTYPE2(int layer,int x,int y)
{
  if (tmpscr2[layer].valid==0)
  {
    return 0;
  }
  return combobuf[MAPDATA2(layer,x,y)].type;
}

// default is to set the item flag which depends on currscr
void setmapflag()
{
  game.maps[(currmap<<7)+homescr] |= ((currscr>=128) ? mBELOW : mITEM);
}

void unsetmapflag()
{
  game.maps[(currmap<<7)+homescr] &= ((currscr>=128) ? ~mBELOW : ~mITEM);
}

bool getmapflag()
{
  return (game.maps[(currmap<<7)+homescr] & ((currscr>=128) ? mBELOW : mITEM))!=0;
}

// set specific flag
void setmapflag(int flag)
{
  game.maps[(currmap<<7)+homescr] |= flag;
}

void unsetmapflag(int flag)
{
  game.maps[(currmap<<7)+homescr] &= ~flag;
}

bool getmapflag(int flag)
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

void update_combo_cycling()
{
  int x,y;
  for (int i=0; i<176; i++)
  {
    x=tmpscr->data[i];
    y=animated_combo_table[x][0];
    //time to restart
    if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
      (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
      (combobuf[x].nextcombo!=0))
    {
      tmpscr->data[i]=combobuf[x].nextcombo;
      tmpscr->cset[i]=combobuf[x].nextcset;
    }
  }

  for (int j=0; j>6; j++)
  {
    for (int i=0; i<176; i++)
    {
      x=(tmpscr2+j)->data[i];
      y=animated_combo_table[x][0];
      //time to restart
      if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
      {
        (tmpscr2+j)->data[i]=combobuf[x].nextcombo;
        (tmpscr2+j)->cset[i]=combobuf[x].nextcset;
      }
    }
  }
}

bool iswater(int combo)
{
  int type = combobuf[combo].type;
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP;
}

bool iswater_type(int type)
{
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP;
}

bool isstepable(int combo)                                  //can use ladder on it
{
  int type = combobuf[combo].type;
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP || type==cLADDERONLY
    || type==cPIT || type==cLADDERHOOKSHOT;
}

bool hiddenstair(int tmp,bool redraw)                       // tmp = index of tmpscr[]
{
  mapscr *s = tmpscr + tmp;

  if(s->stairx || s->stairy)
  {
    int di = (s->stairy&0xF0)+(s->stairx>>4);
    s->data[di] = s->secretcombo[sSTAIRS];
    s->cset[di] = s->secretcset[sSTAIRS];
    s->sflag[di] = s->secretflag[sSTAIRS];
    if(redraw)
      putcombo(scrollbuf,s->stairx,s->stairy,s->data[di],s->cset[di]);
    return true;
  }
  return false;
}

bool remove_lockblocks(int tmp,bool redraw)                 // tmp = index of tmpscr[]
{
  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  bool didit=false;

  for (int i=0; i<176; i++)
  {
    if ((combobuf[s->data[i]].type==cLOCKBLOCK) ||
      (combobuf[s->data[i]].type==cLOCKBLOCK2) )
    {
      s->data[i]++;
      didit=true;
    }
  }
  for (int j=0; j<6; j++)
  {
    for (int i=0; i<176; i++)
    {
      if ((combobuf[t[j].data[i]].type==cLOCKBLOCK) ||
        (combobuf[t[j].data[i]].type==cLOCKBLOCK2) )
      {
        t[j].data[i]++;
        didit=true;
      }
    }
  }
  return didit;
}

bool remove_bosslockblocks(int tmp,bool redraw)             // tmp = index of tmpscr[]
{
  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  bool didit=false;

  for (int i=0; i<176; i++)
  {
    if ((combobuf[s->data[i]].type==cBOSSLOCKBLOCK) ||
      (combobuf[s->data[i]].type==cBOSSLOCKBLOCK2) )
    {
      s->data[i]++;
      didit=true;
    }
  }
  for (int j=0; j<6; j++)
  {
    for (int i=0; i<176; i++)
    {
      if ((combobuf[t[j].data[i]].type==cBOSSLOCKBLOCK) ||
        (combobuf[t[j].data[i]].type==cBOSSLOCKBLOCK2) )
      {
        t[j].data[i]++;
        didit=true;
      }
    }
  }
  return didit;
}

bool overheadcombos(mapscr *s)
{
  for (int i=0; i<176; i++)
  {
    if (combobuf[s->data[i]].type==cOVERHEAD)
    {
      return true;
    }
  }
  return false;
}

void delete_fireball_shooter(mapscr *s, int i)
{
  int cx=0, cy=0;
  int ct=combobuf[s->data[i]].type;
  if (ct!=cL_STATUE && ct!=cR_STATUE && ct!=cC_STATUE)
    return;

  switch (ct)
  {
    case cL_STATUE:
      cx=((i&15)<<4)+4;
      cy=(i&0xF0)+7;
      break;
    case cR_STATUE:
      cx=((i&15)<<4)-8;
      cy=(i&0xF0)-1;
      break;
    case cC_STATUE:
      cx=((i&15)<<4);
      cy=(i&0xF0);
      break;
  }
  for (int j=0; j<guys.Count(); j++)
  {
    if ((int(guys.spr(j)->x)==cx)&&(int(guys.spr(j)->y)==cy))
    {
      guys.del(j);
    }
  }
}

void hidden_entrance(int tmp,bool refresh,bool high16only)
{
  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  int ft=0;
  for(int i=0; i<176; i++)
  {
    bool putit = true;

    if(!high16only)
    {
      switch(s->sflag[i])
      {
        case mfBCANDLE:
          ft=sBCANDLE;
          break;
        case mfRCANDLE:
          ft=sRCANDLE;
          break;
        case mfWANDFIRE:
          ft=sWANDFIRE;
          break;
        case mfDINSFIRE:
          ft=sDINSFIRE;
          break;
        case mfARROW:
          ft=sARROW;
          break;
        case mfSARROW:
          ft=sSARROW;
          break;
        case mfGARROW:
          ft=sGARROW;
          break;
        case mfSBOMB:
          ft=sSBOMB;
          break;
        case mfBOMB:
          ft=sBOMB;
          break;
        case mfBRANG:
          ft=sBRANG;
          break;
        case mfMBRANG:
          ft=sMBRANG;
          break;
        case mfFBRANG:
          ft=sFBRANG;
          break;
        case mfWANDMAGIC:
          ft=sWANDMAGIC;
          break;
        case mfREFMAGIC:
          ft=sREFMAGIC;
          break;
        case mfREFFIREBALL:
          ft=sREFFIREBALL;
          break;
        case mfSWORD:
          ft=sSWORD;
          break;
        case mfWSWORD:
          ft=sWSWORD;
          break;
        case mfMSWORD:
          ft=sMSWORD;
          break;
        case mfXSWORD:
          ft=sXSWORD;
          break;
        case mfSWORDBEAM:
          ft=sSWORDBEAM;
          break;
        case mfWSWORDBEAM:
          ft=sWSWORDBEAM;
          break;
        case mfMSWORDBEAM:
          ft=sMSWORDBEAM;
          break;
        case mfXSWORDBEAM:
          ft=sXSWORDBEAM;
          break;
        case mfHOOKSHOT:
          ft=sHOOKSHOT;
          break;
        case mfWAND:
          ft=sWAND;
          break;
        case mfHAMMER:

          ft=sHAMMER;
          break;
        case mfSTRIKE:
          ft=sSTRIKE;
          break;
        default:
          putit = false;
          break;
      }
      if (putit)
      {
        delete_fireball_shooter(s, i);
        s->data[i] = s->secretcombo[ft];
        s->cset[i] = s->secretcset[ft];
        s->sflag[i] = s->secretflag[ft];
      }

      for (int j=0; j<6; j++)
      {
        putit=true;
        switch(t[j].sflag[i])
        {
          case mfBCANDLE:
            ft=sBCANDLE;
            break;
          case mfRCANDLE:
            ft=sRCANDLE;
            break;
          case mfWANDFIRE:
            ft=sWANDFIRE;
            break;
          case mfDINSFIRE:
            ft=sDINSFIRE;
            break;
          case mfARROW:
            ft=sARROW;
            break;
          case mfSARROW:
            ft=sSARROW;
            break;
          case mfGARROW:
            ft=sGARROW;
            break;
          case mfSBOMB:
            ft=sSBOMB;
            break;
          case mfBOMB:
            ft=sBOMB;
            break;
          case mfBRANG:
            ft=sBRANG;
            break;
          case mfMBRANG:
            ft=sMBRANG;
            break;
          case mfFBRANG:
            ft=sFBRANG;
            break;
          case mfWANDMAGIC:
            ft=sWANDMAGIC;
            break;
          case mfREFMAGIC:
            ft=sREFMAGIC;
            break;
          case mfREFFIREBALL:
            ft=sREFFIREBALL;
            break;
          case mfSWORD:
            ft=sSWORD;
            break;
          case mfWSWORD:
            ft=sWSWORD;
            break;
          case mfMSWORD:
            ft=sMSWORD;
            break;
          case mfXSWORD:
            ft=sXSWORD;
            break;
          case mfSWORDBEAM:
            ft=sSWORDBEAM;
            break;
          case mfWSWORDBEAM:
            ft=sWSWORDBEAM;
            break;
          case mfMSWORDBEAM:
            ft=sMSWORDBEAM;
            break;
          case mfXSWORDBEAM:
            ft=sXSWORDBEAM;
            break;
          case mfHOOKSHOT:
            ft=sHOOKSHOT;
            break;
          case mfWAND:
            ft=sWAND;
            break;
          case mfHAMMER:
            ft=sHAMMER;
            break;
          case mfSTRIKE:
            ft=sSTRIKE;
            break;
          default:
            putit = false;
            break;
        }
        if (putit)
        {

          t[j].data[i] = t[j].secretcombo[ft];
          t[j].cset[i] = t[j].secretcset[ft];
          t[j].sflag[i] = t[j].secretflag[ft];
        }
      }
    }

    // if it's an enemies->secret screen, only do the high 16 if told to
    // that way you can have secret and burn/bomb entrance separately
    if(!(s->flags2&fCLEARSECRET) || high16only)
    {
      if((s->sflag[i] > 15)&&(s->sflag[i] < 32))
      {
        delete_fireball_shooter(s, i);
        s->data[i] = s->secretcombo[(s->sflag[i])-16+4];
        s->cset[i] = s->secretcset[(s->sflag[i])-16+4];
        s->sflag[i] = s->secretflag[(s->sflag[i])-16+4];
        //        putit = true;
      }
      for (int j=0; j<6; j++)
      {
        if((t[j].sflag[i] > 15)&&(t[j].sflag[i] < 32))
        {
          t[j].data[i] = t[j].secretcombo[(t[j].sflag[i])-16+4];
          t[j].cset[i] = t[j].secretcset[(t[j].sflag[i])-16+4];
          t[j].sflag[i] = t[j].secretflag[(t[j].sflag[i])-16+4];
          //          putit = true;
        }
      }
    }

    /*
        if(putit && refresh)
          putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
    */
  }
}

bool findentrance(int x, int y, int flag, bool setflag)
{
  bool foundflag=false;
  if(MAPFLAG(x,y)==flag || MAPFLAG(x+15,y)==flag ||
    MAPFLAG(x,y+15)==flag || MAPFLAG(x+15,y+15)==flag)
  {
    foundflag=true;
  }
  for (int i=0; i<6; i++)
  {
    if(MAPFLAG2(i,x,y)==flag || MAPFLAG2(i,x+15,y)==flag ||
      MAPFLAG2(i,x,y+15)==flag || MAPFLAG2(i,x+15,y+15)==flag)
    {
      foundflag=true;
    }
  }

  if (!foundflag)
  {
    return false;
  }
  if(setflag && !isdungeon())
    setmapflag(mSECRET);
  hidden_entrance(0,true);
  if (!nosecretsounds)
  {
    sfx(WAV_SECRET);
  }
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

  if(tmpscr->sidewarptype == 3)                                // scrolling warp
  {
    switch(dir)
    {
      case up:    if(!(tmpscr->flags2&wfUP))    goto skip; break;
      case down:  if(!(tmpscr->flags2&wfDOWN))  goto skip; break;
      case left:  if(!(tmpscr->flags2&wfLEFT))  goto skip; break;
      case right: if(!(tmpscr->flags2&wfRIGHT)) goto skip; break;
    }
    m = DMaps[tmpscr->sidewarpdmap].map;
    s = tmpscr->sidewarpscr + DMaps[tmpscr->sidewarpdmap].xoff;
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

void do_scrolling_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, bool scrolling, int tempscreen)
{
  if (!drawit)
  {
    return;
  }
  int i;
  static int mf;
  mapscr *tscr;
  switch (type)
  {
    case -2:                                                //push blocks
      for (i=0; i<176; i++)
      {
        mf=layer->sflag[i];
        if (mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED ||
          ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS)))
        {
          overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+56-y,layer->data[i],layer->cset[i]);
        }
      }
      break;
    case -1:                                                //over combo
      for (i=0; i<176; i++)
      {
        if (combobuf[layer->data[i]].type==cOVERHEAD)
        {
          overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+56-y,layer->data[i],layer->cset[i]);
        }
      }
      break;
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      if (tempscreen==2)
      {
        tscr=tmpscr2+type;
      }
      else
      {
        tscr=tmpscr3+type;
      }
      if (TransLayers || layer->layeropacity[type]==255)
      {
        if (layer->layermap[type]>0)
        {
          if (scrolling)
          {
            if (layer->layeropacity[type]==255)
            {
              for (i=0; i<176; i++)
              {
                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+56-y,tscr->data[i],tscr->cset[i]);
              }
            }
            else
            {
              for (int i=0; i<176; i++)
              {
                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+56-y,tscr->data[i],tscr->cset[i],layer->layeropacity[type]);
              }
            }
          }
          else
          {
            if (layer->layeropacity[type]==255)
            {
              for (i=0; i<176; i++)
              {
                overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+56-y,tscr->data[i],tscr->cset[i]);
              }
            }
            else
            {
              for (i=0; i<176; i++)
              {
                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+56-y,tscr->data[i],tscr->cset[i],layer->layeropacity[type]);
              }
            }
          }
        }
      }
      break;
  }
}

void do_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, int tempscreen)
{
  do_scrolling_layer(bmp, type, layer, x, y, false, tempscreen);
}

void draw_screen(mapscr* layer1, mapscr* layer2, int x1, int y1, int x2, int y2)
{
  int cmby2=0;
  int pcounter;

  if (layer2!=NULL)
  {
    putscr(scrollbuf,x2,y2,layer2);
  }
  putscr(scrollbuf,x1,y1,layer1);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-3)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  if ((Link.getAction()==climbcovertop)||(Link.getAction()==climbcoverbottom))
  {
    if (Link.getAction()==climbcovertop)
    {
      cmby2=16;
    }
    else if (Link.getAction()==climbcoverbottom)
    {
      cmby2=-16;
    }
    decorations.draw2(framebuf,true);
    Link.draw(framebuf);
    decorations.draw(framebuf,true);
    overcombo(framebuf,int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2+56,MAPDATA(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2),MAPCSET(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2));
    putcombo (framebuf,int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      +56,MAPDATA(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      ),MAPCSET(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      ));
    if(int(Link.getX())&15)
    {
      overcombo(framebuf,int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2+56,MAPDATA(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2),MAPCSET(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2));
      putcombo (framebuf,int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      +56,MAPDATA(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      ),MAPCSET(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      ));
    }
  }
  if (layer2!=NULL)
  {
    do_layer(framebuf, 0, layer2, x2, y2, 3);
    do_layer(framebuf, 1, layer2, x2, y2, 3);
    do_layer(framebuf, -2, layer2, x2, y2, 3);
  }
  do_layer(framebuf, 0, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==0)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  do_layer(framebuf, 1, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==1)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  do_layer(framebuf, -2, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-2)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  if(lensclk || (debug && key[KEY_L]))
  {
    draw_lens_under();
  }

  if(!(msgdisplaybuf->clip))
  {
    masked_blit(msgdisplaybuf,framebuf,0,0,0,56,256,168);
  }
  if(!(pricesdisplaybuf->clip))
  {
    masked_blit(pricesdisplaybuf,framebuf,0,0,0,56,256,168);
  }

  if ((Link.getAction()!=climbcovertop)&&(Link.getAction()!=climbcoverbottom))
  {
    Link.draw_under(framebuf);
    mblock2.draw(framebuf);
    if (Link.isSwimming())
    {
      decorations.draw2(framebuf,true);
      Link.draw(framebuf);
      decorations.draw(framebuf,true);
    }
  }
  if(drawguys)
  {
    if(get_bit(quest_rules,qr_NOFLICKER) || (frame&1))
    {
      if (get_bit(quest_rules,qr_SHADOWS)&&((!get_bit(quest_rules,qr_SHADOWSFLICKER))||(get_bit(quest_rules,qr_SHADOWSFLICKER)&&(frame&1))))
      {
        guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS),true);
      }
      guys.draw(framebuf,true);
      chainlinks.draw(framebuf,true);
      Lwpns.draw(framebuf,true);
      Ewpns.draw(framebuf,true);
      items.draw(framebuf,true);
    }
    else
    {
      if (get_bit(quest_rules,qr_SHADOWS)&&((!get_bit(quest_rules,qr_SHADOWSFLICKER))||(get_bit(quest_rules,qr_SHADOWSFLICKER)&&(frame&1))))
      {
        guys.drawshadow(framebuf,get_bit(quest_rules,qr_TRANSSHADOWS),true);
      }
      items.draw(framebuf,false);
      chainlinks.draw(framebuf,false);
      Lwpns.draw(framebuf,false);
      guys.draw(framebuf,false);
      Ewpns.draw(framebuf,false);
    }
    guys.draw2(framebuf,true);
  }
  if ((Link.getAction()!=climbcovertop)&& (Link.getAction()!=climbcoverbottom))
  {
    if (!Link.isSwimming())
    {
      if (draw_screen_clip_rect_show_link==true)
      {
        set_clip_rect(framebuf, 0, 0, 255, 223);
      }
      decorations.draw2(framebuf,true);
      Link.draw(framebuf);
      decorations.draw(framebuf,true);
    }
  }

  // draw likelike over Link
  for(int i=0; i<guys.Count(); i++)
  {
    if(guys.spr(i)->id == eLIKE)
    {
      if(((eLikeLike*)guys.spr(i))->haslink)
      {
        guys.spr(i)->draw(framebuf);
      }
    }
    if(guys.spr(i)->id == eWALLM)
    {
      if(((eWallM*)guys.spr(i))->haslink)
      {
        guys.spr(i)->draw(framebuf);
      }
    }
  }

  if (layer2!=NULL)
  {
    do_layer(framebuf, 2, layer2, x2, y2, 3);
    do_layer(framebuf, 3, layer2, x2, y2, 3);
    do_layer(framebuf, -1, layer2, x2, y2, 3);
  }


  //at this point save screen for messages comparison later
  //but only if the next layer group will draw to the screen
  //and we actually have messages
  if ((tmpscr->layermap[2]!=0 || tmpscr->layermap[3]!=0 || overheadcombos(tmpscr))
      && (!(msgdisplaybuf->clip) || !(pricesdisplaybuf->clip)))
  {
    blit(framebuf,scrollbuf,0,56,0,0,256,168);
  }

  do_layer(framebuf, 2, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==2)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  do_layer(framebuf, 3, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==3)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  do_layer(framebuf, -1, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-1)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }

 //before drawing fliers, compare messages to saved screen.
 //pixels in the framebuf which don't match the saved screen
 //are redrawn from the message bitmaps.
 //the screen was saved *after* sprites were drawn, so sprites will
 //only be covered by layers, not by messages.
 //(though messages appear on top of all layers now, so they
 // *can* cover sprites that are under layers. got that ;-P )
  if ((tmpscr->layermap[2]!=0 || tmpscr->layermap[3]!=0 || overheadcombos(tmpscr))
      && (!(msgdisplaybuf->clip) || !(pricesdisplaybuf->clip)))
  {
    for(int y=0; y<168; y++)
    {
      for(int x=0; x<256; x++)
      {
        int c0 = scrollbuf->line[y][x];
        int c1 = framebuf->line[y+56][x];
        int c2 = msgdisplaybuf->line[y][x];
        int c3 = pricesdisplaybuf->line[y][x];

        if (c3 && (c0 != c1))
        {
          framebuf->line[y+56][x] = c3;
        }
        else if (c2 && (c0 != c1))
        {
          framebuf->line[y+56][x] = c2;
        }
      }
    }
  }

  for(int i=0; i<guys.Count(); i++)
  {
    if(isflier(guys.spr(i)->id))
    {
      guys.spr(i)->draw(framebuf);
    }
  }
  if (layer2!=NULL)
  {
    do_layer(framebuf, 4, layer2, x2, y2, 3);
    do_layer(framebuf, 5, layer2, x2, y2, 3);
  }

  //at this point save screen for messages comparison later
  //but only if the next layer group will draw to the screen
  if ((tmpscr->layermap[4]!=0 || tmpscr->layermap[5]!=0)
      && (!(msgdisplaybuf->clip) || !(pricesdisplaybuf->clip)))
  {
    blit(framebuf,scrollbuf,0,56,0,0,256,168);
  }

  do_layer(framebuf, 4, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==4)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  do_layer(framebuf, 5, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==5)
    {
      particles.spr(pcounter)->draw(framebuf);
      //particles.del(pcounter);
    }
  }
  //particles.clear();

  //before clipping, compare messages to saved screen.
  //pixels in the framebuf which don't match the saved screen
  //are redrawn from the message bitmaps
  if ((tmpscr->layermap[4]!=0 || tmpscr->layermap[5]!=0)
      && (!(msgdisplaybuf->clip) || !(pricesdisplaybuf->clip)))
  {
    for(int y=0; y<168; y++)
    {
      for(int x=0; x<256; x++)
      {
        int c0 = scrollbuf->line[y][x];
        int c1 = framebuf->line[y+56][x];
        int c2 = msgdisplaybuf->line[y][x];
        int c3 = pricesdisplaybuf->line[y][x];

        if (c3 && (c0 != c1))
        {
          framebuf->line[y+56][x] = c3;
        }
        else if (c2 && (c0 != c1))
        {
          framebuf->line[y+56][x] = c2;
        }
      }
    }
  }

  if (draw_screen_clip_rect_x1>0||draw_screen_clip_rect_y1>0||draw_screen_clip_rect_x2<255||draw_screen_clip_rect_y2<223)
  {
    rectfill(scrollbuf, 0,0, 255,223, 0);
    if(drawguys && draw_screen_clip_rect_show_guys)
    {
      guys.draw(scrollbuf,true);
      guys.draw2(scrollbuf,true);
    }
    if (draw_screen_clip_rect_show_link==true)
    {
      decorations.draw2(scrollbuf,true);
      Link.draw(scrollbuf);
      decorations.draw(scrollbuf,true);
    }
    if(drawguys && draw_screen_clip_rect_show_guys)
    {
    // draw likelike over Link
    if(draw_screen_clip_rect_show_link)
      for(int i=0; i<guys.Count(); i++)
      {
        if(guys.spr(i)->id == eLIKE)
        {
          if(((eLikeLike*)guys.spr(i))->haslink)
          {
            guys.spr(i)->draw(framebuf);
          }
        }
        else if(guys.spr(i)->id == eWALLM)
        {
          if(((eWallM*)guys.spr(i))->haslink)
          {
            guys.spr(i)->draw(framebuf);
          }
        }
      }

      for(int i=0; i<guys.Count(); i++)
      {
        if(isflier(guys.spr(i)->id))
        {
          guys.spr(i)->draw(scrollbuf);
        }
      }
    }
    if(draw_screen_clip_rect_x1)     blit( scrollbuf, framebuf, 0,                          0,                          0,                          0,                          draw_screen_clip_rect_x1,                            224                          );
    if(draw_screen_clip_rect_x2<255) blit( scrollbuf, framebuf, draw_screen_clip_rect_x2+1, 0,                          draw_screen_clip_rect_x2+1, 0,                          255-draw_screen_clip_rect_x2,                        224                          );
    if(draw_screen_clip_rect_y1)     blit( scrollbuf, framebuf, draw_screen_clip_rect_x1,   0,                          draw_screen_clip_rect_x1,   0,                          draw_screen_clip_rect_x2-draw_screen_clip_rect_x1+1, draw_screen_clip_rect_y1     );
    if(draw_screen_clip_rect_y2<223) blit( scrollbuf, framebuf, draw_screen_clip_rect_x1,   draw_screen_clip_rect_y2+1, draw_screen_clip_rect_x1,   draw_screen_clip_rect_y2+1, draw_screen_clip_rect_x2-draw_screen_clip_rect_x1+1, 223-draw_screen_clip_rect_y1 );
  }
  putsubscr(framebuf, 0, 0);
}

void put_door(int t,int pos,int side,int type,bool redraw)
{
  int d=tmpscr[t].door_combo_set;
  switch (type)
  {
    case dt_pass:
    case dt_lock:
    case dt_shut:
    case dt_boss:
    case dt_olck:
    case dt_osht:
    case dt_obos:
    case dt_bomb:
      switch (side)
      {
        case up:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_u[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_u[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_u[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_u[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_u[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_u[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_u[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_u[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          if(redraw)
          {
            putcombo(scrollbuf,(pos&15)<<4,pos&0xF0,
              DoorComboSets[d].doorcombo_u[type][0],
              DoorComboSets[d].doorcset_u[type][0]);
            putcombo(scrollbuf,((pos&15)<<4)+16,pos&0xF0,
              DoorComboSets[d].doorcombo_u[type][1],
              DoorComboSets[d].doorcset_u[type][1]);
          }
          break;
        case down:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_d[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_d[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_d[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_d[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_d[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_d[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_d[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_d[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          if(redraw)
          {
            putcombo(scrollbuf,(pos&15)<<4,(pos&0xF0)+16,
              DoorComboSets[d].doorcombo_d[type][2],
              DoorComboSets[d].doorcset_d[type][2]);
            putcombo(scrollbuf,((pos&15)<<4)+16,(pos&0xF0)+16,
              DoorComboSets[d].doorcombo_d[type][3],
              DoorComboSets[d].doorcset_d[type][3]);
          }
          break;
        case left:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_l[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_l[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_l[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_l[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_l[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_l[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_l[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_l[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          tmpscr[t].data[pos+32]   = DoorComboSets[d].doorcombo_l[type][4];
          tmpscr[t].cset[pos+32]   = DoorComboSets[d].doorcset_l[type][4];
          tmpscr[t].sflag[pos+32]  = 0;
          tmpscr[t].data[pos+32+1]   = DoorComboSets[d].doorcombo_l[type][5];
          tmpscr[t].cset[pos+32+1]   = DoorComboSets[d].doorcset_l[type][5];
          tmpscr[t].sflag[pos+32+1]  = 0;
          if(redraw)
          {
            putcombo(scrollbuf,(pos&15)<<4,pos&0xF0,
              DoorComboSets[d].doorcombo_l[type][0],
              DoorComboSets[d].doorcset_l[type][0]);
            putcombo(scrollbuf,(pos&15)<<4,(pos&0xF0)+16,
              DoorComboSets[d].doorcombo_l[type][2],
              DoorComboSets[d].doorcset_l[type][2]);
            putcombo(scrollbuf,(pos&15)<<4,(pos&0xF0)+32,
              DoorComboSets[d].doorcombo_l[type][4],
              DoorComboSets[d].doorcset_l[type][4]);
          }
          break;
        case right:
          tmpscr[t].data[pos]   = DoorComboSets[d].doorcombo_r[type][0];
          tmpscr[t].cset[pos]   = DoorComboSets[d].doorcset_r[type][0];
          tmpscr[t].sflag[pos]  = 0;
          tmpscr[t].data[pos+1]   = DoorComboSets[d].doorcombo_r[type][1];
          tmpscr[t].cset[pos+1]   = DoorComboSets[d].doorcset_r[type][1];
          tmpscr[t].sflag[pos+1]  = 0;
          tmpscr[t].data[pos+16]   = DoorComboSets[d].doorcombo_r[type][2];
          tmpscr[t].cset[pos+16]   = DoorComboSets[d].doorcset_r[type][2];
          tmpscr[t].sflag[pos+16]  = 0;
          tmpscr[t].data[pos+16+1]   = DoorComboSets[d].doorcombo_r[type][3];
          tmpscr[t].cset[pos+16+1]   = DoorComboSets[d].doorcset_r[type][3];
          tmpscr[t].sflag[pos+16+1]  = 0;
          tmpscr[t].data[pos+32]   = DoorComboSets[d].doorcombo_r[type][4];
          tmpscr[t].cset[pos+32]   = DoorComboSets[d].doorcset_r[type][4];
          tmpscr[t].sflag[pos+32]  = 0;
          tmpscr[t].data[pos+32+1]   = DoorComboSets[d].doorcombo_r[type][5];
          tmpscr[t].cset[pos+32+1]   = DoorComboSets[d].doorcset_r[type][5];
          tmpscr[t].sflag[pos+32+1]  = 0;
          if(redraw)
          {
            putcombo(scrollbuf,(pos&15)<<4,pos&0xF0,
              DoorComboSets[d].doorcombo_r[type][0],
              DoorComboSets[d].doorcset_r[type][0]);
            putcombo(scrollbuf,(pos&15)<<4,(pos&0xF0)+16,
              DoorComboSets[d].doorcombo_r[type][2],
              DoorComboSets[d].doorcset_r[type][2]);
            putcombo(scrollbuf,(pos&15)<<4,(pos&0xF0)+32,
              DoorComboSets[d].doorcombo_r[type][4],
              DoorComboSets[d].doorcset_r[type][4]);
          }
          break;
      }
      break;
    case dt_wall:
    case dt_walk:
    default:
      break;
  }
}

void over_door(int t, int pos,int side)
{
  int d=tmpscr[t].door_combo_set;
  int x=(pos&15)<<4;
  int y=(pos&0xF0);
  switch (side)
  {
    case up:
      overcombo2(scrollbuf,x,y,
        DoorComboSets[d].bombdoorcombo_u[0],
        DoorComboSets[d].bombdoorcset_u[0]);
      overcombo2(scrollbuf,x+16,y,
        DoorComboSets[d].bombdoorcombo_u[1],
        DoorComboSets[d].bombdoorcset_u[1]);
      break;
    case down:
      overcombo2(scrollbuf,x,y,
        DoorComboSets[d].bombdoorcombo_d[0],
        DoorComboSets[d].bombdoorcset_d[0]);
      overcombo2(scrollbuf,x+16,y,
        DoorComboSets[d].bombdoorcombo_d[1],
        DoorComboSets[d].bombdoorcset_d[1]);
      break;
    case left:
      overcombo2(scrollbuf,x,y,
        DoorComboSets[d].bombdoorcombo_l[0],
        DoorComboSets[d].bombdoorcset_l[0]);
      overcombo2(scrollbuf,x,y+16,
        DoorComboSets[d].bombdoorcombo_l[1],
        DoorComboSets[d].bombdoorcset_l[1]);
      overcombo2(scrollbuf,x,y+16,
        DoorComboSets[d].bombdoorcombo_l[2],
        DoorComboSets[d].bombdoorcset_l[2]);
      break;
    case right:
      overcombo2(scrollbuf,x,y,
        DoorComboSets[d].bombdoorcombo_r[0],
        DoorComboSets[d].bombdoorcset_r[0]);
      overcombo2(scrollbuf,x,y+16,
        DoorComboSets[d].bombdoorcombo_r[1],
        DoorComboSets[d].bombdoorcset_r[1]);
      overcombo2(scrollbuf,x,y+16,
        DoorComboSets[d].bombdoorcombo_r[2],
        DoorComboSets[d].bombdoorcset_r[2]);
      break;
  }
}

void putdoor(int t,int side,int door,bool redraw)
{
  /*
  #define dWALL           0  //  000    0
  #define dBOMB           6  //  011    0
  #define              8  //  100    0
  enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
  */

  int doortype;
  switch (door)
  {
    case dOPEN:
      doortype=dt_pass;
      break;
    case dLOCKED:
      doortype=dt_lock;
      break;
    case dUNLOCKED:
      doortype=dt_olck;
      break;
    case d1WAYSHUTTER:
    case dSHUTTER:
      doortype=dt_shut;
      break;
    case dOPENSHUTTER:
      doortype=dt_osht;
      break;
    case dBOSS:
      doortype=dt_boss;
      break;
    case dOPENBOSS:
      doortype=dt_obos;
      break;
    case dBOMBED:
      doortype=dt_bomb;
      break;
    default:
      return;
  }

  switch(side)
  {
    case up:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(t,39,side);
          }
        default:
          put_door(t,7,side,doortype,redraw);
          break;
      }
      break;
    case down:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(t,135,side);
          }
        default:
          put_door(t,151,side,doortype,redraw);
          break;
      }
      break;
    case left:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(t,66,side);
          }
        default:
          put_door(t,64,side,doortype,redraw);
          break;
      }
      break;
    case right:
      switch(door)
      {
        case dBOMBED:
          if(redraw)
          {
            over_door(t,77,side);
          }
        default:
          put_door(t,78,side,doortype,redraw);
          break;
      }
      break;
  }
}

void showbombeddoor(int side)
{
  int d=tmpscr->door_combo_set;
  switch(side)
  {
    case up:
      putcombo(framebuf,(7&15)<<4,(7&0xF0)+56,
        DoorComboSets[d].doorcombo_u[dt_bomb][0],
        DoorComboSets[d].doorcset_u[dt_bomb][0]);
      putcombo(framebuf,(8&15)<<4,(8&0xF0)+56,
        DoorComboSets[d].doorcombo_u[dt_bomb][1],
        DoorComboSets[d].doorcset_u[dt_bomb][1]);
      putcombo(framebuf,(23&15)<<4,(23&0xF0)+56,
        DoorComboSets[d].doorcombo_u[dt_bomb][2],
        DoorComboSets[d].doorcset_u[dt_bomb][2]);
      putcombo(framebuf,(24&15)<<4,(24&0xF0)+56,
        DoorComboSets[d].doorcombo_u[dt_bomb][3],
        DoorComboSets[d].doorcset_u[dt_bomb][3]);
      overcombo(framebuf,(39&15)<<4,(39&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_u[0],
        DoorComboSets[d].bombdoorcset_u[0]);
      overcombo(framebuf,(40&15)<<4,(40&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_u[1],
        DoorComboSets[d].bombdoorcset_u[1]);
      break;
    case down:
      putcombo(framebuf,(151&15)<<4,(151&0xF0)+56,
        DoorComboSets[d].doorcombo_d[dt_bomb][0],
        DoorComboSets[d].doorcset_d[dt_bomb][0]);
      putcombo(framebuf,(152&15)<<4,(152&0xF0)+56,
        DoorComboSets[d].doorcombo_d[dt_bomb][1],
        DoorComboSets[d].doorcset_d[dt_bomb][1]);
      putcombo(framebuf,(167&15)<<4,(167&0xF0)+56,
        DoorComboSets[d].doorcombo_d[dt_bomb][2],
        DoorComboSets[d].doorcset_d[dt_bomb][2]);
      putcombo(framebuf,(168&15)<<4,(168&0xF0)+56,
        DoorComboSets[d].doorcombo_d[dt_bomb][3],
        DoorComboSets[d].doorcset_d[dt_bomb][3]);
      overcombo(framebuf,(135&15)<<4,(135&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_d[0],
        DoorComboSets[d].bombdoorcset_d[0]);
      overcombo(framebuf,(136&15)<<4,(136&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_d[1],
        DoorComboSets[d].bombdoorcset_d[1]);
      break;
    case left:
      putcombo(framebuf,(64&15)<<4,(64&0xF0)+56,
        DoorComboSets[d].doorcombo_l[dt_bomb][0],
        DoorComboSets[d].doorcset_l[dt_bomb][0]);
      putcombo(framebuf,(65&15)<<4,(65&0xF0)+56,
        DoorComboSets[d].doorcombo_l[dt_bomb][1],
        DoorComboSets[d].doorcset_l[dt_bomb][1]);
      putcombo(framebuf,(80&15)<<4,(80&0xF0)+56,
        DoorComboSets[d].doorcombo_l[dt_bomb][2],
        DoorComboSets[d].doorcset_l[dt_bomb][2]);
      putcombo(framebuf,(81&15)<<4,(81&0xF0)+56,
        DoorComboSets[d].doorcombo_l[dt_bomb][3],
        DoorComboSets[d].doorcset_l[dt_bomb][3]);
      putcombo(framebuf,(96&15)<<4,(96&0xF0)+56,
        DoorComboSets[d].doorcombo_l[dt_bomb][4],
        DoorComboSets[d].doorcset_l[dt_bomb][4]);
      putcombo(framebuf,(97&15)<<4,(97&0xF0)+56,
        DoorComboSets[d].doorcombo_l[dt_bomb][5],
        DoorComboSets[d].doorcset_l[dt_bomb][5]);
      overcombo(framebuf,(66&15)<<4,(66&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_l[0],
        DoorComboSets[d].bombdoorcset_l[0]);
      overcombo(framebuf,(82&15)<<4,(82&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_l[1],
        DoorComboSets[d].bombdoorcset_l[1]);
      overcombo(framebuf,(98&15)<<4,(98&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_l[2],
        DoorComboSets[d].bombdoorcset_l[2]);
      break;
    case right:
      putcombo(framebuf,(78&15)<<4,(78&0xF0)+56,
        DoorComboSets[d].doorcombo_r[dt_bomb][0],
        DoorComboSets[d].doorcset_r[dt_bomb][0]);
      putcombo(framebuf,(79&15)<<4,(79&0xF0)+56,
        DoorComboSets[d].doorcombo_r[dt_bomb][1],
        DoorComboSets[d].doorcset_r[dt_bomb][1]);
      putcombo(framebuf,(94&15)<<4,(94&0xF0)+56,
        DoorComboSets[d].doorcombo_r[dt_bomb][2],
        DoorComboSets[d].doorcset_r[dt_bomb][2]);
      putcombo(framebuf,(95&15)<<4,(95&0xF0)+56,
        DoorComboSets[d].doorcombo_r[dt_bomb][3],
        DoorComboSets[d].doorcset_r[dt_bomb][3]);
      putcombo(framebuf,(110&15)<<4,(110&0xF0)+56,
        DoorComboSets[d].doorcombo_r[dt_bomb][4],
        DoorComboSets[d].doorcset_r[dt_bomb][4]);
      putcombo(framebuf,(111&15)<<4,(111&0xF0)+56,
        DoorComboSets[d].doorcombo_r[dt_bomb][5],
        DoorComboSets[d].doorcset_r[dt_bomb][5]);
      overcombo(framebuf,(77&15)<<4,(77&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_r[0],
        DoorComboSets[d].bombdoorcset_r[0]);
      overcombo(framebuf,(93&15)<<4,(93&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_r[1],
        DoorComboSets[d].bombdoorcset_r[1]);
      overcombo(framebuf,(109&15)<<4,(109&0xF0)+56,
        DoorComboSets[d].bombdoorcombo_r[2],
        DoorComboSets[d].bombdoorcset_r[2]);
      break;
  }
}

void openshutters()
{
  for(int i=0; i<4; i++)
    if(tmpscr->door[i]==dSHUTTER)
  {
    putdoor(0,i,dOPENSHUTTER);
    tmpscr->door[i]=dOPENSHUTTER;
  }
  sfx(WAV_DOOR,128);
}

void loadscr(int tmp,int scr,int ldir)
{
  //  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
  for (word x=0; x<animated_combos; x++)
  {
    if (combobuf[animated_combo_table4[x][0]].nextcombo!=0)
    {
      animated_combo_table4[x][1]=0;
    }
  }
  tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];
  if (tmp==0)
  {
    for (int i=0; i<6; i++)
    {
      if (tmpscr[tmp].layermap[i]>0)
      {
        tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];
      }
      else
      {
        memset(tmpscr2+i,0,sizeof(mapscr));
      }
    }
  }
  if(!isdungeon())
  {
    if(game.maps[(currmap<<7)+scr]&mSECRET)               // if special stuff done before
    {
      hiddenstair(tmp,false);
      hidden_entrance(tmp,false);
    }
  }

  if(game.maps[(currmap<<7)+scr]&mLOCKBLOCK)              // if special stuff done before
  {
    remove_lockblocks(tmp,false);
  }

  if(game.maps[(currmap<<7)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
  {
    remove_bosslockblocks(tmp,false);
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
          {
            tmpscr[tmp].door[i]=dOPENSHUTTER;
            //          putit=false;
          }
          break;

        case dLOCKED:
          if(game.maps[(currmap<<7)+scr]&(1<<i))
          {
            tmpscr[tmp].door[i]=dUNLOCKED;
            //          putit=false;
          }
          break;

        case dBOSS:
          if(game.maps[(currmap<<7)+scr]&(1<<i))
          {
            tmpscr[tmp].door[i]=dOPENBOSS;
            //          putit=false;
          }
          break;

        case dBOMB:
          if(game.maps[(currmap<<7)+scr]&(1<<i))
          {
            tmpscr[tmp].door[i]=dBOMBED;
          }
          break;
      }

      if(putit)
        putdoor(tmp,i,tmpscr[tmp].door[i],false);
      if (door==dSHUTTER||door==d1WAYSHUTTER)
      {
        tmpscr[tmp].door[i]=door;
      }

    }
  }
}

void loadscr2(int tmp,int scr,int ldir)
{
  //these are here to bypass compiler warnings about unused arguments
  ldir=ldir;

  //  introclk=intropos=msgclk=msgpos=dmapmsgclk=0;
  for (word x=0; x<animated_combos; x++)
  {
    if (combobuf[animated_combo_table4[x][0]].nextcombo!=0)
    {
      animated_combo_table4[x][1]=0;
    }
  }
  tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];
  if (tmp==0)
  {
    for (int i=0; i<6; i++)
    {
      if (tmpscr[tmp].layermap[i]>0)
      {
        tmpscr2[i]=TheMaps[(tmpscr[tmp].layermap[i]-1)*MAPSCRS+tmpscr[tmp].layerscreen[i]];
      }
      else
      {
        memset(tmpscr2+i,0,sizeof(mapscr));
      }
    }
  }
  if(!isdungeon())
  {
    if(game.maps[(currmap<<7)+scr]&mSECRET)               // if special stuff done before
    {
      hiddenstair(tmp,false);
      hidden_entrance(tmp,false);
    }
  }

  if(game.maps[(currmap<<7)+scr]&mLOCKBLOCK)              // if special stuff done before
  {
    remove_lockblocks(tmp,false);
  }

  if(game.maps[(currmap<<7)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
  {
    remove_bosslockblocks(tmp,false);
  }

//  if(game.maps[(currmap<<7)+scr]&mCHEST)              // if special stuff done before
//  {
//    remove_chests(tmp,false);
//  }
//
//  if(game.maps[(currmap<<7)+scr]&mLOCKEDCHEST)              // if special stuff done before
//  {
//    remove_lockedchests(tmp,false);
//  }
//
//  if(game.maps[(currmap<<7)+scr]&mBOSSCHEST)              // if special stuff done before
//  {
//    remove_bosschests(tmp,false);
//  }

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
/*
        if((ldir^1)==i)
        {
          tmpscr[tmp].door[i]=dOPENSHUTTER;
          //          putit=false;
        }
*/
        break;

        case dLOCKED:
        if(game.maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dUNLOCKED;
          //          putit=false;
        }
        break;

        case dBOSS:
        if(game.maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dOPENBOSS;
          //          putit=false;
        }
        break;

        case dBOMB:
        if(game.maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dBOMBED;
        }
        break;
      }

      if(putit)
      {
        putdoor(tmp,i,tmpscr[tmp].door[i],false);
      }
      if (door==dSHUTTER||door==d1WAYSHUTTER)
      {
        tmpscr[tmp].door[i]=door;
      }

    }
  }
}

void putscr(BITMAP* dest,int x,int y,mapscr* screen)
{
  if(screen->valid==0)
  {
    rectfill(dest,x,y,x+255,y+175,0);
    return;
  }
  for(int i=0; i<176; i++)
    putcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,screen->data[i],screen->cset[i]);

  if(screen->door[0]==dBOMBED)
  {
    over_door(0,39,up);
  }
  if(screen->door[1]==dBOMBED)
  {
    over_door(0,135,down);
  }
  if(screen->door[2]==dBOMBED)
  {
    over_door(0,66,left);
  }

  if(screen->door[3]==dBOMBED)
  {
    over_door(0,77,right);
  }

}

bool _walkflag(int x,int y,int cnt)
{
  //  walkflagx=x; walkflagy=y;
  if(x<0||y<0) return false;
  if(x>248) return false;
  if(x>240&&cnt==2) return false;
  if(y>168) return false;
  mapscr *s1, *s2;
  /*
    s1=(((*tmpscr).layermap[0]-1)>=0)?
         (TheMaps+((*tmpscr).layermap[0]-1)*MAPSCRS+((*tmpscr).layerscreen[0])):
         tmpscr;
    s2=(((*tmpscr).layermap[1]-1)>=0)?
         (TheMaps+((*tmpscr).layermap[1]-1)*MAPSCRS+((*tmpscr).layerscreen[1])):
         tmpscr;
  */
  s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
  s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;
  //  s2=TheMaps+((*tmpscr).layermap[1]-1)MAPSCRS+((*tmpscr).layerscreen[1]);

  int bx=(x>>4)+(y&0xF0);
  newcombo c = combobuf[tmpscr->data[bx]];
  newcombo c1 = combobuf[s1->data[bx]];
  newcombo c2 = combobuf[s2->data[bx]];
  bool dried = (((iswater_type(c.type)) && (iswater_type(c1.type)) &&
    (iswater_type(c2.type))) && (whistleclk>=88));
  int b=1;

  if(x&8) b<<=2;
  if(y&8) b<<=1;
  if(((c.walk&b) || (c1.walk&b) || (c2.walk&b)) && !dried)
    return true;
  if(cnt==1) return false;

  ++bx;
  if(!(x&8))
    b<<=2;
  else
  {
    c  = combobuf[tmpscr->data[bx]];
    c1 = combobuf[s1->data[bx]];
    c2 = combobuf[s2->data[bx]];
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

  mapscr *s1, *s2;
  /*
    s1=(((*tmpscr).layermap[0]-1)>=0)?
         (TheMaps+((*tmpscr).layermap[0]-1)*MAPSCRS+((*tmpscr).layerscreen[0])):
         tmpscr;
    s2=(((*tmpscr).layermap[1]-1)>=0)?
         (TheMaps+((*tmpscr).layermap[1]-1)*MAPSCRS+((*tmpscr).layerscreen[1])):
         tmpscr;
  */
  s1=(((*tmpscr).layermap[0]-1)>=0)?tmpscr2:tmpscr;
  s2=(((*tmpscr).layermap[1]-1)>=0)?tmpscr2+1:tmpscr;

  int bx=(x>>4)+(y&0xF0);
  newcombo c = combobuf[tmpscr->data[bx]];
  newcombo c1 = combobuf[s1->data[bx]];
  newcombo c2 = combobuf[s2->data[bx]];
  int b=1;

  if(x&8) b<<=2;
  if(y&8) b<<=1;
  if((c.walk&b) && !iswater_type(c.type))
    return true;
  if((c1.walk&b) && !iswater_type(c1.type))
    return true;
  if((c2.walk&b) && !iswater_type(c2.type))
    return true;
  if(cnt==1) return false;

  if(x&8)
    b<<=2;
  else
  {
    c = combobuf[tmpscr->data[++bx]];
    c1 = combobuf[s1->data[bx]];
    c2 = combobuf[s2->data[bx]];
    b=1;
    if(y&8) b<<=1;
  }

  return (c.walk&b) ? !iswater_type(c.type) :
  (c1.walk&b) ? !iswater_type(c1.type) :
  (c2.walk&b) ? !iswater_type(c2.type) :false;
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
  //  for(int i=0; i<4; i++)
  if(mblock2.clk && mblock2.hit(x,y,cnt*8,1))
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

//BITMAP *mappic = NULL;
int mapres = 0;

void ViewMap()
{
  mapscr tmpscr_b[2];
  mapscr tmpscr_c[6];
  for (int i=0; i<6; ++i)
  {
    memcpy(&(tmpscr_c[i]), &(tmpscr2[i]), sizeof(mapscr));
    if (i>=2)
    {
      continue;
    }
    memcpy(&(tmpscr_b[i]), &(tmpscr[i]), sizeof(mapscr));
  }
  BITMAP* mappic = NULL;
  static double scales[17] =
  {
    0.03125, 0.04419, 0.0625, 0.08839, 0.125, 0.177, 0.25, 0.3535,
    0.50, 0.707, 1.0, 1.414, 2.0, 2.828, 4.0, 5.657, 8.0
  };

  int px = ((8-(currscr&15)) << 9)  - 256;
  int py = ((4-(currscr>>4)) * 352) - 176;
  int lx = ((currscr&15)<<8)  + LinkX()+8;
  int ly = ((currscr>>4)*176) + LinkY()+8;
  int sc = 6;

  bool done=false, redraw=true;

  mappic = create_bitmap_ex(8,(256*16)>>mapres,(176*8)>>mapres);

  if(!mappic)
  {
    system_pal();
    jwin_alert("View Map","Not enough memory.",NULL,NULL,"OK",NULL,13,27,lfont);
    game_pal();
    return;
  }

  // draw the map
  for(int y=0; y<8; y++)
  {
    for(int x=0; x<16; x++)
    {
      int s = (y<<4) + x;

      if(!(game.maps[(currmap<<7)+s]&mVISITED))
      {
        rectfill(scrollbuf, 256, 0, 511, 223, WHITE);
      }
      else
      {
        loadscr2(1,s,-1);
        for (int i=0; i<6; i++)
        {
          if (tmpscr[1].layermap[i]>0)
          {
            tmpscr2[i]=TheMaps[(tmpscr[1].layermap[i]-1)*MAPSCRS+tmpscr[1].layerscreen[i]];
          }
          else
          {
            memset(tmpscr2+i,0,sizeof(mapscr));
          }
        }

//		if((tmpscr+1)->flags7&fLAYER2BG) do_layer(scrollbuf, 1, tmpscr+1, -256, 56, 2);
//		if((tmpscr+1)->flags7&fLAYER3BG) do_layer(scrollbuf, 2, tmpscr+1, -256, 56, 2);
        putscr(scrollbuf,256,0,tmpscr+1);
        do_layer(scrollbuf, 0, tmpscr+1, -256, 56, 2);
        do_layer(scrollbuf, 1, tmpscr+1, -256, 56, 2);
        do_layer(scrollbuf,-2, tmpscr+1, -256, 56, 2);
        do_layer(scrollbuf, 2, tmpscr+1, -256, 56, 2);
        do_layer(scrollbuf, 3, tmpscr+1, -256, 56, 2);
        do_layer(scrollbuf,-1, tmpscr+1, -256, 56, 2);
        do_layer(scrollbuf, 4, tmpscr+1, -256, 56, 2);
        do_layer(scrollbuf, 5, tmpscr+1, -256, 56, 2);

      }
      stretch_blit(scrollbuf, mappic, 256, 0, 256, 176, x<<(8-mapres), (y*176)>>mapres, 256>>mapres, 176>>mapres);
    }
  }
  clear_keybuf();

  // view it
  int delay = 0;
  static int show  = 3;

  do
  {
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
      --delay;
    else
    {
      bool a = cAbtn();
      bool b = cBbtn();
      if(a && !b)  { sc=min(sc+1,16); delay=8; redraw=true; }
      if(b && !a)  { sc=max(sc-1,0);  delay=8; redraw=true; }
    }

    if(rPbtn())
      --show;

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

    //    text_mode(BLACK);

    if(show&2 || r)
      textprintf_ex(framebuf,font,224,216,WHITE,BLACK,"%1.2f",scale);

    if(r)
    {
      textprintf_ex(framebuf,font,0,208,WHITE,BLACK,"m: %d %d",px,py);
      textprintf_ex(framebuf,font,0,216,WHITE,BLACK,"x: %d %d",x,y);
    }

    advanceframe();
    if(rSbtn())
      done = true;

  } while(!done && !Quit);
  destroy_bitmap(mappic);
  loadscr2(0,currscr,-1);
  for (int i=0; i<6; ++i)
  {
    memcpy(&(tmpscr2[i]), &(tmpscr_c[i]), sizeof(mapscr));
    if (i>=2)
    {
      continue;
    }
    memcpy(&(tmpscr[i]), &(tmpscr_b[i]), sizeof(mapscr));
  }

}

int onViewMap()
{
  if(Playing && currscr<128 && dlevel==0)
  {
    if(get_bit(quest_rules,qr_VIEWMAP))
    {
      clear_to_color(framebuf,BLACK);
      //      text_mode(BLACK);
      textout_centre_ex(framebuf,font,"Drawing map...",128,108,WHITE,BLACK);
      advanceframe();
      ViewMap();
    }
  }
  return D_O_K;
}


/*** end of maps.cc ***/
