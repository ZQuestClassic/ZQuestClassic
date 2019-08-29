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
#include "zc_subscr.h"
#include "link.h"
#include "guys.h"
#include "ffscript.h"
#include <string.h>
#include "particles.h"

#define EPSILON 0.01 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))

extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations, particles;
extern movingblock mblock2;                                 //mblock[4]?
extern LinkClass Link;
int current_ffcombo=-1;

short ffposx[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
short ffposy[32]={-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                  -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
long ffprvx[32]={-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000};
long ffprvy[32]={-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000};

int draw_screen_clip_rect_x1=0;
int draw_screen_clip_rect_x2=255;
int draw_screen_clip_rect_y1=0;
int draw_screen_clip_rect_y2=223;
//bool draw_screen_clip_rect_show_link=true;
//bool draw_screen_clip_rect_show_guys=false;
bool checktrigger=false;

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
  {
    if(TheMaps[(currmap*MAPSCRS)+currscr].flags6&fCAVEROOM)
      return 0;
    return dlevel;
  }

  // dlevels that aren't dungeons are caves
  if(TheMaps[(currmap*MAPSCRS)+currscr].flags6&fDUNGEONROOM)
    return dlevel;
  return 0;
}

int MAPCOMBO(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return tmpscr->data[combo];                               // entire combo code
}

int MAPFFCOMBO(int x,int y)
{
  for(int i=0;i<32;i++)
  {
    if(x>=(tmpscr->ffx[i]/10000)&&x<(tmpscr->ffx[i]/10000)+(tmpscr->ffwidth[i]&63)+1)
	  if(y>=(tmpscr->ffy[i]/10000)&&y<(tmpscr->ffy[i]/10000)+(tmpscr->ffheight[i]&63)+1)
	    if(combobuf[tmpscr->ffdata[i]].type!=cCHANGE)
		  return tmpscr->ffdata[i];
  }
  return 0;
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
  return combobuf[MAPCOMBO(x,y)].type;
}

int FFCOMBOTYPE(int x,int y)
{
  return combobuf[MAPFFCOMBO(x,y)].type;
}

int MAPCOMBOFLAG(int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  return combobuf[tmpscr->data[combo]].flag;                               // entire combo code
}

int MAPFFCOMBOFLAG(int x,int y)
{
  for(int i=0;i<32;i++)
  {
    if(x>=(tmpscr->ffx[i]/10000)&&x<((tmpscr->ffx[i]/10000)+(tmpscr->ffwidth[i]&63)+1))
	  if(y>=(tmpscr->ffy[i]/10000)&&y<((tmpscr->ffy[i]/10000)+(tmpscr->ffheight[i]&63)+1))
	  {
	    if(combobuf[tmpscr->ffdata[i]].type!=cCHANGE)
		{
	    current_ffcombo = i;
		return combobuf[tmpscr->ffdata[i]].flag;
		}
	  }
  }
  current_ffcombo=-1;
  return 0;
}

int MAPCOMBO2(int layer,int x,int y)
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
  return combobuf[MAPCOMBO2(layer,x,y)].type;
}

int MAPCOMBOFLAG2(int layer,int x,int y)
{
  int combo = (y&0xF0)+(x>>4);
  if(combo>175)
    return 0;
  if (tmpscr2[layer].valid==0)
  {
    return 0;
  }
  return combobuf[tmpscr2[layer].data[combo]].flag;                        // entire combo code
}

// default is to set the item flag which depends on currscr
void setmapflag()
{
  game->maps[(currmap<<7)+homescr] |= ((currscr>=128) ? mBELOW : mITEM);

  byte nmap=tmpscr->nextmap;
  byte nscr=tmpscr->nextscr;
  while(/*(nmap!=0&&nscr!=0)&&*/(nmap!=0)&&!((nmap-1==currmap)&&(nscr==homescr))&&!(nscr>=128))
  {
    int temp = ((currscr>=128) ? (tmpscr->nocarry&mBELOW) : (tmpscr->nocarry&mITEM));
    if(temp!=((currscr>=128) ? mBELOW : mITEM))
      game->maps[((nmap-1)<<7)+nscr] |= ((currscr>=128) ? mBELOW : mITEM);
    int tmap = nmap;
    nmap=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextmap;
    nscr=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextscr;
    //Z_message("%d, %d\n", nmap, nscr);
  }
}

void unsetmapflag()
{
  if(!(tmpscr->flags4&fNOITEMRESET))
    game->maps[(currmap<<7)+homescr] &= ((currscr>=128) ? ~mBELOW : ~mITEM);
  byte nmap=tmpscr->nextmap;
  byte nscr=tmpscr->nextscr;
  while(/*(nmap!=0&&nscr!=0)&&*/(nmap!=0)&&(!((nmap-1==currmap)&&(nscr==homescr)))&&!(nscr>=128))
  {
    /*if(!(TheMaps[((nmap-1)*MAPSCRS)+nscr].flags4&fNOITEMRESET))*/
    int temp = ((currscr>=128) ? (tmpscr->nocarry&mBELOW) : (tmpscr->nocarry&mITEM));
    if(temp!=((currscr>=128) ? mBELOW : mITEM))
      game->maps[((nmap-1)<<7)+nscr] &= ((currscr>=128) ? ~mBELOW : ~mITEM);
    int tmap = nmap;
    nmap=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextmap;
    nscr=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextscr;
  }
}

bool getmapflag()
{
  return (game->maps[(currmap<<7)+homescr] & ((currscr>=128) ? mBELOW : mITEM))!=0;
}

// set specific flag
void setmapflag(int flag)
{
  game->maps[(currmap<<7)+homescr] |= flag;
  if(flag==mSECRET||flag==mITEM||flag==mBELOW||flag==mLOCKBLOCK||
     flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
  {
    if(!isdungeon()||(flag!=mSECRET)/*||(TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM)*/)
    {
      byte nmap=tmpscr->nextmap;
      byte nscr=tmpscr->nextscr;
      while(/*(nmap!=0&&nscr!=0)&&*/(nmap!=0)&&(!((nmap-1==currmap)&&(nscr==homescr)))&&!(nscr>=128))
      {
        if((tmpscr->nocarry&flag)!=flag)
          game->maps[((nmap-1)<<7)+nscr] |= flag;
        int tmap = nmap;
        nmap=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextmap;
        nscr=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextscr;
        //Z_message("%d, %d\n", nmap, nscr);
      }
    }
  }
}

void unsetmapflag(int flag)
{
  if(flag==mITEM || flag==mBELOW) {
    if(!(tmpscr->flags4&fNOITEMRESET))
      game->maps[(currmap<<7)+homescr] &= ~flag;
  }
  else game->maps[(currmap<<7)+homescr] &= ~flag;
  if(flag==mSECRET||flag==mITEM||flag==mBELOW||flag==mLOCKBLOCK||
     flag==mBOSSLOCKBLOCK||flag==mCHEST||flag==mBOSSCHEST||flag==mLOCKEDCHEST)
  {
    byte nmap=tmpscr->nextmap;
    byte nscr=tmpscr->nextscr;
    while(/*(nmap!=0&&nscr!=0)&&*/(nmap!=0)&&(!((nmap-1==currmap)&&(nscr==homescr)))&&!(nscr>=128))
    {
      /*if(flag==mITEM || flag==mBELOW) {
        if(!(TheMaps[((nmap-1)*MAPSCRS)+nscr].flags4&fNOITEMRESET))
        game->maps[((nmap-1)<<7)+nscr] &= ~flag;
        }
        else*/
      if((tmpscr->nocarry&flag)!=flag)
        game->maps[((nmap-1)<<7)+nscr] &= ~flag;
      int tmap = nmap;
      nmap=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextmap;
      nscr=TheMaps[((tmap-1)*MAPSCRS)+nscr].nextscr;
    }
  }
}

bool getmapflag(int flag)
{
  return (game->maps[(currmap<<7)+homescr] & flag) != 0;
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
      int c=tmpscr->data[i];
      int cs=tmpscr->cset[i];
      if (combobuf[c].type==cSPINTILE1)
      {
        addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+max(1,combobuf[c].frames));
      }
    }
  }

  for (int i=0; i<176; i++)
  {
    x=tmpscr->data[i];
    y=animated_combo_table2[x][0];
    //time to restart
    if ((animated_combo_table24[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
    {
      tmpscr->data[i]=combobuf[x].nextcombo;
      tmpscr->cset[i]=combobuf[x].nextcset;
      int c=tmpscr->data[i];
      int cs=tmpscr->cset[i];
      if (combobuf[c].type==cSPINTILE1)
      { 
        addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table2[c][1]+max(1,combobuf[c].frames));
      }
    }
  }

  for(int i=0;i<32;i++)
  {
	  x=tmpscr->ffdata[i];
	  y=animated_combo_table[x][0];
	  //time to restart
	  if ((animated_combo_table4[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
      {
        tmpscr->ffdata[i]=combobuf[x].nextcombo;
        tmpscr->ffcset[i]=combobuf[x].nextcset;
        int c=tmpscr->ffdata[i];
        int cs=tmpscr->ffcset[i];
        if (combobuf[c].type==cSPINTILE1)
        {
          addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+max(1,combobuf[c].frames));
        }
      }
  }

  for(int i=0;i<32;i++)
  {
	  x=tmpscr->ffdata[i];
	  y=animated_combo_table2[x][0];
	  //time to restart
	  if ((animated_combo_table24[y][1]>=combobuf[x].speed) &&
        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
        (combobuf[x].nextcombo!=0))
      {
        tmpscr->ffdata[i]=combobuf[x].nextcombo;
        tmpscr->ffcset[i]=combobuf[x].nextcset;
        int c=tmpscr->ffdata[i];
        int cs=tmpscr->ffcset[i];
        if (combobuf[c].type==cSPINTILE1)
        {
          addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table2[c][1]+max(1,combobuf[c].frames));
        }
      }
  }


  if(get_bit(quest_rules,qr_CMBCYCLELAYERS))
  {
    for (int j=0; j<6; j++)
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
          int c=(tmpscr2+j)->data[i];
          int cs=(tmpscr2+j)->cset[i];
          if (combobuf[c].type==cSPINTILE1)
          {
            addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+max(1,combobuf[c].frames));
          }
        }
      }
	  for (int i=0; i<176; i++)
      {
        x=(tmpscr2+j)->data[i];
        y=animated_combo_table2[x][0];
        //time to restart
        if ((animated_combo_table24[y][1]>=combobuf[x].speed) &&
            (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
            (combobuf[x].nextcombo!=0))
        {
          (tmpscr2+j)->data[i]=combobuf[x].nextcombo;
          (tmpscr2+j)->cset[i]=combobuf[x].nextcset;
          int c=(tmpscr2+j)->data[i];
          int cs=(tmpscr2+j)->cset[i];
          if (combobuf[c].type==cSPINTILE1)
          {
            addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table2[c][1]+max(1,combobuf[c].frames));
          }
        }
      }
    }
  }
}

bool iswater(int combo)
{
  int type = combobuf[combo].type;
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP ||
    type==cDIVEWARPB || type==cDIVEWARPC || type==cDIVEWARPD ||
    type==cSWIMWARPB || type==cSWIMWARPC || type==cSWIMWARPD;
}

bool iswater_type(int type)
{
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP ||
    type==cDIVEWARPB || type==cDIVEWARPC || type==cDIVEWARPD ||
    type==cSWIMWARPB || type==cSWIMWARPC || type==cSWIMWARPD;
}

bool isstepable(int combo)                                  //can use ladder on it
{
  int type = combobuf[combo].type;
  return type==cWATER || type==cSWIMWARP || type==cDIVEWARP || type==cLADDERONLY
    || type==cPIT || type==cPITB || type==cPITC || type==cPITD || type==cPITR || type==cLADDERHOOKSHOT
    || type==cSWIMWARPB || type==cSWIMWARPC || type==cSWIMWARPD
    || type==cDIVEWARPB || type==cDIVEWARPC || type==cDIVEWARPD;
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
  //these are here to bypass compiler warnings about unused arguments
  redraw=redraw;

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
  //these are here to bypass compiler warnings about unused arguments
  redraw=redraw;

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

bool remove_chests(int tmp,bool redraw)                 // tmp = index of tmpscr[]
{
  //these are here to bypass compiler warnings about unused arguments
  redraw=redraw;

  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  bool didit=false;

  for (int i=0; i<176; i++)
  {
    if ((combobuf[s->data[i]].type==cCHEST) ||
        (combobuf[s->data[i]].type==cCHEST2) )
    {
      s->data[i]++;
      didit=true;
    }
  }
  for (int j=0; j<6; j++)
  {
    for (int i=0; i<176; i++)
    {
      if ((combobuf[t[j].data[i]].type==cCHEST) ||
          (combobuf[t[j].data[i]].type==cCHEST2) )
      {
        t[j].data[i]++;
        didit=true;
      }
    }
  }
  return didit;
}

bool remove_lockedchests(int tmp,bool redraw)                 // tmp = index of tmpscr[]
{
  //these are here to bypass compiler warnings about unused arguments
  redraw=redraw;

  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  bool didit=false;

  for (int i=0; i<176; i++)
  {
    if ((combobuf[s->data[i]].type==cLOCKEDCHEST) ||
        (combobuf[s->data[i]].type==cLOCKEDCHEST2) )
    {
      s->data[i]++;
      didit=true;
    }
  }
  for (int j=0; j<6; j++)
  {
    for (int i=0; i<176; i++)
    {
      if ((combobuf[t[j].data[i]].type==cLOCKEDCHEST) ||
          (combobuf[t[j].data[i]].type==cLOCKEDCHEST2) )
      {
        t[j].data[i]++;
        didit=true;
      }
    }
  }
  return didit;
}

bool remove_bosschests(int tmp,bool redraw)                 // tmp = index of tmpscr[]
{
  //these are here to bypass compiler warnings about unused arguments
  redraw=redraw;

  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  bool didit=false;

  for (int i=0; i<176; i++)
  {
    if ((combobuf[s->data[i]].type==cBOSSCHEST) ||
        (combobuf[s->data[i]].type==cBOSSCHEST2) )
    {
      s->data[i]++;
      didit=true;
    }
  }
  for (int j=0; j<6; j++)
  {
    for (int i=0; i<176; i++)
    {
      if ((combobuf[t[j].data[i]].type==cBOSSCHEST) ||
          (combobuf[t[j].data[i]].type==cBOSSCHEST2) )
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

bool findtrigger(int scombo, bool ff)
{
	int checkflag=0;
	int iter;
	for(int j=0;j<(ff?32:176);j++)
	{
    if(ff)
    {
      checkflag=combobuf[tmpscr->ffdata[j]].flag;
      iter=1;
    }
    else
    {
      iter=2;
    }
    for(int i=0; i<iter;i++)
    {
      if(!ff)
      {
        checkflag = combobuf[tmpscr->data[j]].flag;
      }
      if(i==1&&!ff)
      {
        checkflag = tmpscr->sflag[j];
      }
      switch(checkflag)
      {
        case mfBCANDLE:
        case mfRCANDLE:
        case mfWANDFIRE:
        case mfDINSFIRE:
        case mfARROW:
        case mfSARROW:
        case mfGARROW:
        case mfSBOMB:
        case mfBOMB:
        case mfBRANG:
        case mfMBRANG:
        case mfFBRANG:
        case mfWANDMAGIC:
        case mfREFMAGIC:
        case mfREFFIREBALL:
        case mfSWORD:
        case mfWSWORD:
        case mfMSWORD:
        case mfXSWORD:
        case mfSWORDBEAM:
        case mfWSWORDBEAM:
        case mfMSWORDBEAM:
        case mfXSWORDBEAM:
        case mfHOOKSHOT:
        case mfWAND:
        case mfHAMMER:
        case mfSTRIKE:
          if(scombo!=j)
          return true;
        default:
        break;
      }
    }
	}
	return false;
}

void hidden_entrance(int tmp,bool refresh,bool high16only,int single)
{
  //these are here to bypass compiler warnings about unused arguments
  refresh=refresh;

  mapscr *s = tmpscr + tmp;
  mapscr *t = tmpscr2;
  int ft=0;
  for(int i=0; i<176; i++)
  {
    if(single>=0)
      if(i!=single)
        continue;
      bool putit;

    if((!high16only)||(single>=0))
    {
      for (int iter=0; iter<2; ++iter)
      {
        putit=true;
        int checkflag=combobuf[s->data[i]].flag;
        if (iter==1)
        {
          checkflag=s->sflag[i];
        }
        switch(checkflag)
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
          if(iter) s->sflag[i] = s->secretflag[ft];
          int c=s->data[i];
          int cs=s->cset[i];
          if (combobuf[c].type==cSPINTILE1)
          {
            addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+max(1,combobuf[c].frames));
          }
        }
      }

      for (int j=0; j<6; j++)
      {
        if(single>=0) continue;
        for (int iter=0; iter<2; ++iter)
        {
          putit=true;
          int checkflag=combobuf[t[j].data[i]].flag;
          if (iter==1)
          {
            checkflag=t[j].sflag[i];
          }
          switch(checkflag)
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
            if(iter) t[j].sflag[i] = t[j].secretflag[ft];
            int c=t[j].data[i];
            int cs=t[j].cset[i];
            if (combobuf[c].type==cSPINTILE1)
            {
              addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+max(1,combobuf[c].frames));
            }
          }
        }
      }
    }
  }

  for(int i=0; i<32; i++)
  {
	  if(single>=0)
      if(i+176!=single)
        continue;
      bool putit;

    if((!high16only)||(single>=0))
    {
      for (int iter=0; iter<1; ++iter)
      {
        putit=true;
        int checkflag=combobuf[s->ffdata[i]].flag;
        if (iter==1)
        {
          checkflag=s->sflag[i];
        }
        switch(checkflag)
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
          s->ffdata[i] = s->secretcombo[ft];
          s->ffcset[i] = s->secretcset[ft];
          if(iter) s->sflag[i] = s->secretflag[ft];
        }
      }
	}
  }

  if(checktrigger)
  {
    checktrigger=false;
    if(tmpscr->flags6&fTRIGGERF1631)
    {
		if(findtrigger(-1,false)) goto endhe;
		if(findtrigger(-1,true)) goto endhe;
    }
  }

  for(int i=0; i<176; i++)
  {
    if((!(s->flags2&fCLEARSECRET)&&(single<0)) || high16only || s->flags4&fENEMYSCRTPERM)
    {
      for (int iter=0; iter<2; ++iter)
      {
        int checkflag=combobuf[s->data[i]].flag;
        if (iter==1)
        {
          checkflag=s->sflag[i];
        }
        if((checkflag > 15)&&(checkflag < 32))
        {
          delete_fireball_shooter(s, i);
          s->data[i] = s->secretcombo[checkflag-16+4];
          s->cset[i] = s->secretcset[checkflag-16+4];
          if(iter) s->sflag[i] = s->secretflag[checkflag-16+4];
          //        putit = true;
        }
      }
      for (int j=0; j<6; j++)
      {
        for (int iter=0; iter<2; ++iter)
        {
          int checkflag=combobuf[t[j].data[i]].flag;
          if (iter==1)
          {
            checkflag=t[j].sflag[i];
          }
          if((checkflag > 15)&&(checkflag < 32))
          {
            t[j].data[i] = t[j].secretcombo[checkflag-16+4];
            t[j].cset[i] = t[j].secretcset[checkflag-16+4];
            if(iter) t[j].sflag[i] = t[j].secretflag[checkflag-16+4];
            //          putit = true;
          }
        }
      }
    }

    /*
      if(putit && refresh)
      putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
      */
  }

  for(int i=0; i<32; i++)
  {
	if((!(s->flags2&fCLEARSECRET)&&(single<0)) || high16only || s->flags4&fENEMYSCRTPERM)
    {
      for (int iter=0; iter<1; ++iter)
      {
        int checkflag=combobuf[s->ffdata[i]].flag;
        if (iter==1)
        {
          checkflag=s->sflag[i];
        }
        if((checkflag > 15)&&(checkflag < 32))
        {
          s->ffdata[i] = s->secretcombo[checkflag-16+4];
          s->ffcset[i] = s->secretcset[checkflag-16+4];
          if(iter) s->sflag[i] = s->secretflag[checkflag-16+4];
          //        putit = true;
        }
      }
	}
  }
    
endhe:
  if(tmpscr->flags4&fDISABLETIME)
  {
	  activated_timed_warp=true;
	  tmpscr->timedwarptics = 0;
  }
}






bool findentrance(int x, int y, int flag, bool setflag)
{
  bool foundflag=false;
  bool foundcflag=false;
  bool foundnflag=false;
  bool foundfflag=false;
  bool ffcombosingle = false;
  int ffcombos[4] = {-1, -1, -1, -1};
  bool single16=false;
  int scombo=-1;
  if(MAPFLAG(x,y)==flag || MAPFLAG(x+15,y)==flag ||
     MAPFLAG(x,y+15)==flag || MAPFLAG(x+15,y+15)==flag)
  {
    foundflag=true;
    foundnflag=true;
  }
  for (int i=0; i<6; i++)
  {
    if(MAPFLAG2(i,x,y)==flag || MAPFLAG2(i,x+15,y)==flag ||
       MAPFLAG2(i,x,y+15)==flag || MAPFLAG2(i,x+15,y+15)==flag)
    {
      foundflag=true;
      foundnflag=true;
    }
  }

  if(MAPCOMBOFLAG(x,y)==flag || MAPCOMBOFLAG(x+15,y)==flag ||
     MAPCOMBOFLAG(x,y+15)==flag || MAPCOMBOFLAG(x+15,y+15)==flag)
  {
    foundflag=true;
    foundcflag=true;
  }
  for (int i=0; i<6; i++)
  {
    if(MAPCOMBOFLAG2(i,x,y)==flag || MAPCOMBOFLAG2(i,x+15,y)==flag ||
       MAPCOMBOFLAG2(i,x,y+15)==flag || MAPCOMBOFLAG2(i,x+15,y+15)==flag)
    {
      foundflag=true;
      foundcflag=true;
    }
  }

  if(MAPFFCOMBOFLAG(x,y)==flag)
  {
	foundflag=true;
    foundfflag=true;
  }
  ffcombos[0] = current_ffcombo;

  if(MAPFFCOMBOFLAG(x+15,y)==flag)
  {
	foundflag=true;
    foundfflag=true;
  }
  ffcombos[1] = current_ffcombo;

  if(MAPFFCOMBOFLAG(x,y+15)==flag)
  {
	foundflag=true;
    foundfflag=true;
  }
  ffcombos[2] = current_ffcombo;

  if(MAPFFCOMBOFLAG(x+15,y+15)==flag)
  {
	foundflag=true;
    foundfflag=true;
  }
  ffcombos[3] = current_ffcombo;

  if (!foundflag)
  {
    return false;
  }

  if(foundnflag)
  {
    if((MAPCOMBOFLAG(x,y)==mfSINGLE)&&(MAPFLAG(x,y)==flag))
    {
      scombo=(y&0xF0)+(x>>4);
    }
    else if((MAPCOMBOFLAG(x,y)==mfSINGLE16)&&(MAPFLAG(x,y)==flag))
      {
        scombo=(y&0xF0)+(x>>4);
        single16=true;
      }
      else if((MAPCOMBOFLAG(x+15,y)==mfSINGLE)&&(MAPFLAG(x+15,y)==flag))
        {
          scombo=(y&0xF0)+((x+15)>>4);
        }
        else if((MAPCOMBOFLAG(x+15,y)==mfSINGLE16)&&(MAPFLAG(x+15,y)==flag))
          {
            scombo=(y&0xF0)+((x+15)>>4);
            single16=true;
          }
          else if((MAPCOMBOFLAG(x,y+15)==mfSINGLE)&&(MAPFLAG(x,y+15)==flag))
            {
              scombo=((y+15)&0xF0)+(x>>4);
            }
            else if((MAPCOMBOFLAG(x,y+15)==mfSINGLE16)&&(MAPFLAG(x,y+15)==flag))
              {
                scombo=((y+15)&0xF0)+(x>>4);
                single16=true;
              }
              else if((MAPCOMBOFLAG(x+15,y+15)==mfSINGLE)&&(MAPFLAG(x+15,y+15)==flag))
                {
                  scombo=((y+15)&0xF0)+((x+15)>>4);
                }
                else if((MAPCOMBOFLAG(x+15,y+15)==mfSINGLE16)&&(MAPFLAG(x+15,y+15)==flag))
                  {
                    scombo=((y+15)&0xF0)+((x+15)>>4);
                    single16=true;
                  }
  }

  if(foundcflag)
  {
    if((MAPFLAG(x,y)==mfSINGLE)&&(MAPCOMBOFLAG(x,y)==flag))
    {
      scombo=(y&0xF0)+(x>>4);
    }
    else if((MAPFLAG(x,y)==mfSINGLE16)&&(MAPCOMBOFLAG(x,y)==flag))
      {
        scombo=(y&0xF0)+(x>>4);
        single16=true;
      }
      else if((MAPFLAG(x+15,y)==mfSINGLE)&&(MAPCOMBOFLAG(x+15,y)==flag))
        {
          scombo=(y&0xF0)+((x+15)>>4);
        }
        else if((MAPFLAG(x+15,y)==mfSINGLE16)&&(MAPCOMBOFLAG(x+15,y)==flag))
          {
            scombo=(y&0xF0)+((x+15)>>4);
            single16=true;
          }
          else if((MAPFLAG(x,y+15)==mfSINGLE)&&(MAPCOMBOFLAG(x,y+15)==flag))
            {
              scombo=((y+15)&0xF0)+(x>>4);
            }
            else if((MAPFLAG(x,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG(x,y+15)==flag))
              {
                scombo=((y+15)&0xF0)+(x>>4);
                single16=true;
              }
              else if((MAPFLAG(x+15,y+15)==mfSINGLE)&&(MAPCOMBOFLAG(x+15,y+15)==flag))
                {
                  scombo=((y+15)&0xF0)+((x+15)>>4);
                }
                else if((MAPFLAG(x+15,y+15)==mfSINGLE16)&&(MAPCOMBOFLAG(x+15,y+15)==flag))
                  {
                    scombo=((y+15)&0xF0)+((x+15)>>4);
                    single16=true;
                  }
  }

  if(foundfflag)
  {
    if(((MAPFLAG(x,y)==mfSINGLE)||(MAPCOMBOFLAG(x,y)==mfSINGLE))&&(MAPFFCOMBOFLAG(x,y)==flag))
    {
      ffcombosingle=true;
    }
    else if(((MAPFLAG(x,y)==mfSINGLE16)||(MAPCOMBOFLAG(x,y)==mfSINGLE16))&&(MAPFFCOMBOFLAG(x,y)==flag))
      {
        ffcombosingle=true;
        single16=true;
      }
      else if(((MAPFLAG(x+15,y)==mfSINGLE)||(MAPCOMBOFLAG(x+15,y)==mfSINGLE))&&(MAPFFCOMBOFLAG(x+15,y)==flag))
        {
          ffcombosingle=true;
        }
        else if(((MAPFLAG(x+15,y)==mfSINGLE16)||(MAPCOMBOFLAG(x+15,y)==mfSINGLE16))&&(MAPFFCOMBOFLAG(x+15,y)==flag))
          {
            ffcombosingle=true;
            single16=true;
          }
          else if(((MAPFLAG(x,y+15)==mfSINGLE)||(MAPCOMBOFLAG(x,y+15)==mfSINGLE))&&(MAPFFCOMBOFLAG(x,y+15)==flag))
            {
              ffcombosingle=true;
            }
            else if(((MAPFLAG(x,y+15)==mfSINGLE16)||(MAPCOMBOFLAG(x,y+15)==mfSINGLE16))&&(MAPFFCOMBOFLAG(x,y+15)==flag))
              {
               ffcombosingle=true;
                single16=true;
              }
              else if(((MAPFLAG(x+15,y+15)==mfSINGLE)||(MAPCOMBOFLAG(x+15,y+15)==mfSINGLE))&&(MAPFFCOMBOFLAG(x+15,y+15)==flag))
                {
                  ffcombosingle=true;
                }
                else if(((MAPFLAG(x+15,y+15)==mfSINGLE16)||(MAPCOMBOFLAG(x+15,y+15)==mfSINGLE16))&&(MAPFFCOMBOFLAG(x+15,y+15)==flag))
                  {
                    ffcombosingle=true;
                    single16=true;
                  }
  }

  if(ffcombosingle)
  {
	  if(ffcombos[0]!=-1) scombo = ffcombos[0]+176;
	  else if(ffcombos[1]!=-1) scombo = ffcombos[1]+176;
	  else if(ffcombos[2]!=-1) scombo = ffcombos[2]+176;
	  else if(ffcombos[3]!=-1) scombo = ffcombos[3]+176;
  }

  if(scombo<0)
  {
	checktrigger=true;
    hidden_entrance(0,true);
  }
  else 
  {
    checktrigger=true;
	hidden_entrance(0,true,single16,scombo);
  }
  if (!nosecretsounds)
  {
    sfx(WAV_SECRET);
  }
  if(tmpscr->flags6&fTRIGGERFPERM)
  {
		if(findtrigger(-1,false)) setflag=false;
		if(findtrigger(-1,true)) setflag=false;
  }
  if(setflag && (!isdungeon()/*||TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM*/))
    if(!(tmpscr->flags5&fTEMPSECRETS))
      setmapflag(mSECRET);
  return true;
}

void update_freeform_combos()
{
  ffscript_engine();
  for(int i=0;i<32;i++)
  {
    if(combobuf[tmpscr->ffdata[i]].type!=cCHANGE && tmpscr->ffdata[i]!=0 && !(tmpscr->ffflags[i]&ffSTATIONARY))
    {
      if(tmpscr->fflink[i] ? !tmpscr->ffdelay[tmpscr->fflink[i]] : !tmpscr->ffdelay[i])
      {
        if(tmpscr->fflink[i]&&(tmpscr->fflink[i]-1)!=i)
        {
          ffprvx[i] = tmpscr->ffx[i];
          ffprvy[i] = tmpscr->ffy[i];
          tmpscr->ffx[i]+=tmpscr->ffxdelta[tmpscr->fflink[i]-1];
          tmpscr->ffy[i]+=tmpscr->ffydelta[tmpscr->fflink[i]-1];
        }
        else
        {
          ffprvx[i] = tmpscr->ffx[i];
          ffprvy[i] = tmpscr->ffy[i];
          tmpscr->ffx[i]+=tmpscr->ffxdelta[i];
          tmpscr->ffy[i]+=tmpscr->ffydelta[i];
          tmpscr->ffxdelta[i]+=tmpscr->ffxdelta2[i];
          tmpscr->ffydelta[i]+=tmpscr->ffydelta2[i];
          if(tmpscr->ffxdelta[i]>1280000) tmpscr->ffxdelta[i]=1280000;
          if(tmpscr->ffxdelta[i]<-1280000) tmpscr->ffxdelta[i]=-1280000;
          if(tmpscr->ffydelta[i]>1280000) tmpscr->ffydelta[i]=1280000;
          if(tmpscr->ffydelta[i]<-1280000) tmpscr->ffydelta[i]=-1280000;
        }
      }
      else
      {
        if(!tmpscr->fflink[i] || (tmpscr->fflink[i]-1)==i)
          tmpscr->ffdelay[i]--;
      }
      if(tmpscr->ffx[i]<-320000)
      {
        if(tmpscr->flags6&fWRAPAROUNDFF) 
        {
          tmpscr->ffx[i] = 2880000-(tmpscr->ffx[i]+320000);
          ffprvx[i] = tmpscr->ffx[i];
        }
        else
        {
          tmpscr->ffdata[i]=0;
          tmpscr->ffflags[i]&=~ffCARRYOVER;
        }
      }
      if(tmpscr->ffy[i]<-320000)
      {
        if(tmpscr->flags6&fWRAPAROUNDFF) 
        {
          tmpscr->ffy[i] = 2080000-(tmpscr->ffy[i]+320000);
          ffprvx[i] = tmpscr->ffx[i];
        }
        else
        {
          tmpscr->ffdata[i]=0;
          tmpscr->ffflags[i]&=~ffCARRYOVER;
        }
      }
      if(tmpscr->ffx[i]>=2880000)
      {
        if(tmpscr->flags6&fWRAPAROUNDFF) 
        {
          tmpscr->ffx[i] = tmpscr->ffx[i]-2880000-320000;
          ffprvy[i] = tmpscr->ffy[i];
        }
        else
        {
          tmpscr->ffdata[i]=0;
          tmpscr->ffflags[i]&=~ffCARRYOVER;
        }
      }
      if(tmpscr->ffy[i]>=2080000)
      {
        if(tmpscr->flags6&fWRAPAROUNDFF) 
        {
          tmpscr->ffy[i] = tmpscr->ffy[i]-2080000-320000;
          ffprvy[i] = tmpscr->ffy[i];
        }
        else
        {
          tmpscr->ffdata[i]=0;
          tmpscr->ffflags[i]&=~ffCARRYOVER;
        }
      }
  
      for(int j=0;j<32;j++)
      {
        if(i!=j)
        {
          if(combobuf[tmpscr->ffdata[j]].type==cCHANGE)
          {
            if((((tmpscr->ffx[j]/10000)!=ffposx[i])||((tmpscr->ffy[j]/10000)!=ffposy[i]))&&(tmpscr->fflink[i]==0))
            {
              if((isonline(tmpscr->ffx[i]/10000,tmpscr->ffy[i]/10000,ffprvx[i]/10000,ffprvy[i]/10000,tmpscr->ffx[j]/10000,tmpscr->ffy[j]/10000)||
                (tmpscr->ffx[i]==tmpscr->ffx[j])&&(tmpscr->ffy[i]==tmpscr->ffy[j]))&&(ffprvx[i]>-10000000&&ffprvy[i]>-10000000))
              {
                //tmpscr->ffdata[i]=tmpscr->ffdata[j];
                //tmpscr->ffcset[i]=tmpscr->ffcset[j];
                if(tmpscr->ffflags[j]&ffCHANGETHIS)
                {
                  tmpscr->ffdata[i] = tmpscr->ffdata[j];
                  tmpscr->ffcset[i] = tmpscr->ffcset[j];
                }
                if(tmpscr->ffflags[j]&ffCHANGENEXT)
                  tmpscr->ffdata[i]++;
                if(tmpscr->ffflags[j]&ffCHANGEPREV)
                  tmpscr->ffdata[i]--;
                tmpscr->ffdelay[i]=tmpscr->ffdelay[j];
                tmpscr->ffx[i]=tmpscr->ffx[j];
                tmpscr->ffy[i]=tmpscr->ffy[j];
                tmpscr->ffxdelta[i]=tmpscr->ffxdelta[j];
                tmpscr->ffydelta[i]=tmpscr->ffydelta[j];
                tmpscr->ffxdelta2[i]=tmpscr->ffxdelta2[j];
                tmpscr->ffydelta2[i]=tmpscr->ffydelta2[j];
                tmpscr->fflink[i]=tmpscr->fflink[j];
                tmpscr->ffwidth[i]=tmpscr->ffwidth[j];
                tmpscr->ffheight[i]=tmpscr->ffheight[j];
                if(tmpscr->ffflags[i]&ffCARRYOVER)
                  tmpscr->ffflags[i]=tmpscr->ffflags[j]|ffCARRYOVER;
                else tmpscr->ffflags[i]=tmpscr->ffflags[j];
                ffposx[i]=(short)(tmpscr->ffx[j]/10000);
                ffposy[i]=(short)(tmpscr->ffy[j]/10000);
       
                if(combobuf[tmpscr->ffdata[j]].flag>15 && combobuf[tmpscr->ffdata[j]].flag<32)
                {
                  tmpscr->ffdata[j]=tmpscr->secretcombo[combobuf[tmpscr->ffdata[j]].flag-16+4];
                }
                if((tmpscr->ffflags[j]&ffSWAPNEXT)||(tmpscr->ffflags[j]&ffSWAPPREV))
                {
                  int k=0;
                  if(tmpscr->ffflags[j]&ffSWAPNEXT)
                    k=j<31?j+1:0;
                  if(tmpscr->ffflags[j]&ffSWAPPREV)
                  k=j>0?j-1:31;
                  //swap(tmpscr->ffx[j],tmpscr->ffx[k]);
                  //swap(tmpscr->ffy[j],tmpscr->ffy[k]);
                  swap(tmpscr->ffdata[j],tmpscr->ffdata[k]);
                  swap(tmpscr->ffcset[j],tmpscr->ffcset[k]);
                  swap(tmpscr->ffdelay[j],tmpscr->ffdelay[k]);
                  swap(tmpscr->ffxdelta[j],tmpscr->ffxdelta[k]);
                  swap(tmpscr->ffydelta[j],tmpscr->ffydelta[k]);
                  swap(tmpscr->ffxdelta2[j],tmpscr->ffxdelta2[k]);
                  swap(tmpscr->ffydelta2[j],tmpscr->ffydelta2[k]);
                  swap(tmpscr->fflink[j],tmpscr->fflink[k]);
                  swap(tmpscr->ffwidth[j],tmpscr->ffwidth[k]);
                  swap(tmpscr->ffheight[j],tmpscr->ffheight[k]);
                  swap(tmpscr->ffflags[j],tmpscr->ffflags[k]);
                }
              }
            }
          }
        }
      }
    }
  }
}

bool hitcombo(int x, int y, int combotype)
{
  return (COMBOTYPE(x,y)==combotype);
}

bool hitflag(int x, int y, int flagtype)
{
  return (MAPFLAG(x,y)==flagtype||MAPCOMBOFLAG(x,y)==flagtype);
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

  int index = (tmpscr->sidewarpindex >> (dir*2))&3;
  if(tmpscr->sidewarptype[index] == 3)                                // scrolling warp
  {
    switch(dir)
    {
      case up:    if(!(tmpscr->flags2&wfUP))    goto skip; break;
      case down:  if(!(tmpscr->flags2&wfDOWN))  goto skip; break;
      case left:  if(!(tmpscr->flags2&wfLEFT))  goto skip; break;
      case right: if(!(tmpscr->flags2&wfRIGHT)) goto skip; break;
    }
    m = DMaps[tmpscr->sidewarpdmap[index]].map;
    s = tmpscr->sidewarpscr[index] + DMaps[tmpscr->sidewarpdmap[index]].xoff;
  }

  if(s<0||s>=128)
    return 0xFFFF;

skip:

  return (m<<7) + s;
}

void bombdoor(int x,int y)
{
  if(tmpscr->door[0]==dBOMB && isinRect(x,y,100,0,139,48))
  {
    tmpscr->door[0]=dBOMBED;
    putdoor(0,0,dBOMBED);
    game->maps[(currmap<<7)+homescr] |= 1;
        if(nextscr(up)!=0xFFFF) game->maps[nextscr(up)] |= 2;
      markBmap(-1);
  }
  if(tmpscr->door[1]==dBOMB && isinRect(x,y,100,112,139,176))
  {
    tmpscr->door[1]=dBOMBED;
    putdoor(0,1,dBOMBED);
    game->maps[(currmap<<7)+homescr] |= 2;
        if(nextscr(down)!=0xFFFF) game->maps[nextscr(down)] |= 1;
      markBmap(-1);
  }
  if(tmpscr->door[2]==dBOMB && isinRect(x,y,0,60,48,98))
  {
    tmpscr->door[2]=dBOMBED;
    putdoor(0,2,dBOMBED);
    game->maps[(currmap<<7)+homescr] |= 4;
        if(nextscr(left)!=0xFFFF) game->maps[nextscr(left)] |= 8;
      markBmap(-1);
  }
  if(tmpscr->door[3]==dBOMB && isinRect(x,y,192,60,240,98))
  {
    tmpscr->door[3]=dBOMBED;
    putdoor(0,3,dBOMBED);
    game->maps[(currmap<<7)+homescr] |= 8;
        if(nextscr(right)!=0xFFFF) game->maps[nextscr(right)] |= 4;
      markBmap(-1);
  }
}

void do_scrolling_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, bool scrolling, int tempscreen)
{
  if (!drawit)
  {
    return;
  }
  static int mf;
  switch (type)
  {
    case -2:                                                //push blocks
    for (int i=0; i<176; i++)
    {
      mf=layer->sflag[i];
      if (mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED ||
          ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS)))
      {
        overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
      }
      else
      {
        mf=combobuf[layer->data[i]].flag;
        if (mf==mfPUSHUD || mf==mfPUSH4 || mf==mfPUSHED ||
            ((mf>=mfPUSHLR)&&(mf<=mfPUSHRINS)))
        {
          overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
        }
      }
    }
    break;
    case -1:                                                //over combo
    for (int i=0; i<176; i++)
    {
      if (combobuf[layer->data[i]].type==cOVERHEAD)
      {
        overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
      }
      /*if (combobuf[layer->data[i]].type==cVERTICAL)
        {
        if(Link.getY() <= (i&0xF0))
        {
        overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,layer->data[i],layer->cset[i]);
        }
        }*/
    }
    break;
    case 0:
    //case 1:
    //case 2:
    case 3:
    case 4:
    case 5:
	if (TransLayers || layer->layeropacity[type]==255)
    {
      if (layer->layermap[type]>0)
      {
        if (scrolling)
        {
          if (layer->layeropacity[type]==255)
          {
            for (int i=0; i<176; i++)
            {
              overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
            }
          }
          else
          {
            for (int i=0; i<176; i++)
            {
              //                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr3[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
              overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
            }
          }
        }
        else
        {
          if (layer->layeropacity[type]==255)
          {
            for (int i=0; i<176; i++)
            {
              overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
            }
          }
          else
          {
            for (int i=0; i<176; i++)
            {
              overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
            }
          }
        }
      }
    }
    break;
	case 1:
	if (TransLayers || layer->layeropacity[type]==255)
    {
      if (layer->layermap[type]>0)
      {
        if (scrolling)
        {
          if (layer->layeropacity[type]==255)
          {
            for (int i=0; i<176; i++)
            {
			  if(layer->flags7&fLAYER2BG)
			    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
            }
          }
          else
          {
            for (int i=0; i<176; i++)
            {
              //                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr3[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
			  if(layer->flags7&fLAYER2BG)
			    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
            }
          }
        }
        else
        {
          if (layer->layeropacity[type]==255)
          {
            for (int i=0; i<176; i++)
            {
			  if(layer->flags7&fLAYER2BG)
				putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
            }
          }
          else
          {
            for (int i=0; i<176; i++)
            {
			  if(layer->flags7&fLAYER2BG)
				putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
            }
          }
        }
      }
    }
    break;
	case 2:
	if (TransLayers || layer->layeropacity[type]==255)
    {
      if (layer->layermap[type]>0)
      {
        if (scrolling)
        {
          if (layer->layeropacity[type]==255)
          {
            for (int i=0; i<176; i++)
            {
			  if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
			    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
            }
          }
          else
          {
            for (int i=0; i<176; i++)
            {
              //                overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr3[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
			  if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
			    putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
            }
          }
        }
        else
        {
          if (layer->layeropacity[type]==255)
          {
            for (int i=0; i<176; i++)
            {
			  if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
				putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
            }
          }
          else
          {
            for (int i=0; i<176; i++)
            {
			  if(layer->flags7&fLAYER3BG&&!(layer->flags7&fLAYER2BG))
				putcombo(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i]);
              else overcombotranslucent(bmp,((i&15)<<4)-x,(i&0xF0)+playing_field_offset-y,tempscreen==2?tmpscr2[type].data[i]:tmpscr3[type].data[i],tempscreen==2?tmpscr2[type].cset[i]:tmpscr3[type].cset[i],layer->layeropacity[type]);
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
  switch(type)
  {
    case -2:
      if (!show_layer_push)
      {
        return;
      }
      break;
    case -1:
      if (!show_layer_over)
      {
        return;
      }
      break;
    case 0:
      if (!show_layer_1)
      {
        return;
      }
      break;
    case 1:
      if (!show_layer_2)
      {
        return;
      }
      break;
    case 2:
      if (!show_layer_3)
      {
        return;
      }
      break;
    case 3:
      if (!show_layer_4)
      {
        return;
      }
      break;
    case 4:
      if (!show_layer_5)
      {
        return;
      }
      break;
    case 5:
      if (!show_layer_6)
      {
        return;
      }
      break;
  }
  do_scrolling_layer(bmp, type, layer, x, y, false, tempscreen);
}

void draw_screen(mapscr* layer1, mapscr* layer2, int x1, int y1, int x2, int y2)
{
	//1. Draw some layers onto temp_buf
	//2. Blit those layers onto framebuf with clipping
	//3. Draw some sprites onto temp_buf2
	//4. Blit temp_buf2 onto temp_buf and framebuf, without clipping
	//5. Draw some layers onto temp_buf
	//6. Blit temp_buf onto framebuf with clipping
	//7. Clear temp_buf2 and draw some flying sprites onto temp_buf2
	//8. Blit temp_buf2 onto framebuf and temp_buf, without clipping
	//9. Draw some layers onto temp_buf
	//10. Blit temp_buf onto framebuf with clipping
	//11. Clear temp_buf2 and draw some text on it
	//12. Blit temp_buf2 onto framebuf without clipping
	//good lord.
	clear_bitmap(framebuf);
	set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2); 
	clear_bitmap(temp_buf);
	clear_bitmap(temp_buf2);
  int cmby2=0;
  int pcounter;

  clear_bitmap(scrollbuf);
  if (layer2!=NULL)
  {
    if(layer2->flags7&fLAYER2BG) do_layer(scrollbuf,1, layer2, x2, y2+playing_field_offset, 3);
	if(layer2->flags7&fLAYER3BG) do_layer(scrollbuf,2, layer2, x2, y2+playing_field_offset, 3);
  }
  if(layer1->flags7&fLAYER2BG)
  {
    do_layer(scrollbuf,1, layer1, x1, y1+playing_field_offset, 2);
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==1)
      {
        particles.spr(pcounter)->draw(scrollbuf);
        //particles.del(pcounter);
      }
    }
  }
  if(layer1->flags7&fLAYER3BG)
  {
    do_layer(scrollbuf,2, layer1, x1, y1+playing_field_offset, 2);
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==2)
      {
        particles.spr(pcounter)->draw(scrollbuf);
        //particles.del(pcounter);
      }
    }
  }
  if (layer2!=NULL)
  {
    putscr(scrollbuf,x2,y2,layer2);
  }
  putscr(scrollbuf,x1,y1,layer1);
  blit(scrollbuf,temp_buf,0,0,0,playing_field_offset,256,168);

  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-3)
    {
      particles.spr(pcounter)->draw(temp_buf);
      //particles.del(pcounter);
    }
  }
  if(!(get_bit(quest_rules,qr_LAYER12UNDERCAVE)))
  {
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
        decorations.draw2(temp_buf,true);
      Link.draw(temp_buf);
      decorations.draw(temp_buf,true);
      overcombo(temp_buf,int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2+playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2),MAPCSET(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2));
      putcombo (temp_buf,int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      +playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      ),MAPCSET(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      ));
      if(int(Link.getX())&15)
      {
        overcombo(temp_buf,int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2+playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2),MAPCSET(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2));
        putcombo (temp_buf,int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      +playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      ),MAPCSET(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      ));
      }
    }
  }
  
  if (layer2!=NULL)
  {
    do_layer(temp_buf,0, layer2, x2, y2, 3);
    if(!(layer2->flags7&fLAYER2BG)) do_layer(temp_buf,1, layer2, x2, y2, 3);
    do_layer(temp_buf,-2, layer2, x2, y2, 3);
  }
  do_layer(temp_buf,0, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==0)
    {
      particles.spr(pcounter)->draw(temp_buf);
      //particles.del(pcounter);
    }
  }
  for(int i=31;i>=0;i--)
  {
    if(layer1->ffdata[i])
    {
      int c=combobuf[layer1->ffdata[i]].type;
      if(c!=cCHANGE)
      {
        if(!(layer1->ffflags[i]&ffOVERLAY))
        {
          for(int j=0;j<(layer1->ffheight[i]>>6)+1;j++)
          {
            for(int k=0;k<(layer1->ffwidth[i]>>6)+1;k++)
            {
              if(layer1->ffflags[i]&ffTRANS)
              {
                  overtiletranslucent16(temp_buf, combobuf[layer1->ffdata[i]].tile+(j*20)+(k), ((layer1->ffx[i]/10000)+(k*16)), ((layer1->ffy[i]/10000)+(j*16))+playing_field_offset, layer1->ffcset[i], combobuf[layer1->ffdata[i]].flip, 128);
              }
              else
              {
                  overtile16(temp_buf, combobuf[layer1->ffdata[i]].tile+(j*20)+(k), ((layer1->ffx[i]/10000)+(k*16)), ((layer1->ffy[i]/10000)+(j*16))+playing_field_offset, layer1->ffcset[i], combobuf[layer1->ffdata[i]].flip);
              }
            }
          }
        }
      }
    }
  }
  if(!(layer1->flags7&fLAYER2BG))
  {
    do_layer(temp_buf,1, layer1, x1, y1, 2);
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==1)
      {
        particles.spr(pcounter)->draw(temp_buf);
        //particles.del(pcounter);
      }
    }
  }
  if(get_bit(quest_rules,qr_LAYER12UNDERCAVE))
  {
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
        decorations.draw2(temp_buf,true);
      Link.draw(temp_buf);
      decorations.draw(temp_buf,true);
      overcombo(temp_buf,int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2+playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2),MAPCSET(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())+cmby2));
      putcombo (temp_buf,int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      +playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      ),MAPCSET(int(Link.getClimbCoverX()),int(Link.getClimbCoverY())      ));
      if(int(Link.getX())&15)
      {
        overcombo(temp_buf,int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2+playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2),MAPCSET(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())+cmby2));
        putcombo (temp_buf,int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      +playing_field_offset,MAPCOMBO(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      ),MAPCSET(int(Link.getClimbCoverX())+16,int(Link.getClimbCoverY())      ));
      }
    }
  }
  do_layer(temp_buf,-2, layer1, x1, y1, 2);

  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-2)
    {
      particles.spr(pcounter)->draw(temp_buf);
      //particles.del(pcounter);
    }
  }
  if(lensclk || (get_debug() && key[KEY_L]))
  {
    draw_lens_under(temp_buf);
  }

  blit(temp_buf, framebuf, 0, 0, 0, 0, 255, 223);
  
  if ((Link.getAction()!=climbcovertop)&&(Link.getAction()!=climbcoverbottom))
  {
    Link.draw_under(temp_buf2);
    mblock2.draw(temp_buf2);
    if (Link.isSwimming())
    {
      decorations.draw2(temp_buf2,true);
      Link.draw(temp_buf2);
      decorations.draw(temp_buf2,true);
    }
  }
  if(drawguys)
  {
    if(get_bit(quest_rules,qr_NOFLICKER) || (frame&1))
    {
      if (get_bit(quest_rules,qr_SHADOWS)&&((!get_bit(quest_rules,qr_SHADOWSFLICKER))||(get_bit(quest_rules,qr_SHADOWSFLICKER)&&(frame&1))))
      {
        guys.drawshadow(temp_buf2,get_bit(quest_rules,qr_TRANSSHADOWS) != 0,true);
      }
      guys.draw(temp_buf2,true);
      chainlinks.draw(temp_buf2,true);
      Lwpns.draw(temp_buf2,true);
      Ewpns.draw(temp_buf2,true);
      items.draw(temp_buf2,true);
    }
    else
    {
      if (get_bit(quest_rules,qr_SHADOWS)&&((!get_bit(quest_rules,qr_SHADOWSFLICKER))||(get_bit(quest_rules,qr_SHADOWSFLICKER)&&(frame&1))))
      {
        guys.drawshadow(temp_buf2,get_bit(quest_rules,qr_TRANSSHADOWS)!=0,true);
      }
      items.draw(temp_buf2,false);
      chainlinks.draw(temp_buf2,false);
      Lwpns.draw(temp_buf2,false);
      guys.draw(temp_buf2,false);
      Ewpns.draw(temp_buf2,false);
    }
    guys.draw2(temp_buf2,true);
  }
  if ((Link.getAction()!=climbcovertop)&& (Link.getAction()!=climbcoverbottom))
  {
    if (!Link.isSwimming())
    {
      decorations.draw2(temp_buf2,true);
      Link.draw(temp_buf2);
      decorations.draw(temp_buf2,true);
    }
  }

  // draw likelike over Link
  for(int i=0; i<guys.Count(); i++)
  {
    if(guys.spr(i)->id == eLIKE)
    {
      if(((eLikeLike*)guys.spr(i))->haslink)
      {
        guys.spr(i)->draw(temp_buf2);
      }
    }
    if(guys.spr(i)->id == eWALLM)
    {
      if(((eWallM*)guys.spr(i))->haslink)
      {
        guys.spr(i)->draw(temp_buf2);
      }
    }
  }
  set_clip_rect(framebuf,0,0,255,223); 
  masked_blit(temp_buf2, framebuf, 0, 0, 0, 0, 255, 223);
  masked_blit(temp_buf2, temp_buf, 0, 0, 0, 0, 255, 223);
  clear_bitmap(temp_buf2);

  if (layer2!=NULL)
  {
    if(!(layer2->flags7&fLAYER3BG)) do_layer(temp_buf,2, layer2, x2, y2, 3);
    do_layer(temp_buf,3, layer2, x2, y2, 3);
    do_layer(temp_buf,-1, layer2, x2, y2, 3);
  }


  if(!(layer1->flags7&fLAYER3BG))
  {
    do_layer(temp_buf,2, layer1, x1, y1, 2);
    for (pcounter=0;pcounter<particles.Count();pcounter++)
    {
      if (((particle*)particles.spr(pcounter))->layer==2)
      {
        particles.spr(pcounter)->draw(temp_buf);
        //particles.del(pcounter);
      }
    }
  }
  do_layer(temp_buf,3, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==3)
    {
      particles.spr(pcounter)->draw(temp_buf);
      //particles.del(pcounter);
    }
  }
  do_layer(temp_buf,-1, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==-1)
    {
      particles.spr(pcounter)->draw(temp_buf);
      //particles.del(pcounter);
    }
  }

  set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2); 
  blit(temp_buf, framebuf, 0, 0, 0, 0, 255, 223);
  
  for(int i=0; i<guys.Count(); i++)
  {
    if(isflier(guys.spr(i)->id))
    {
      guys.spr(i)->draw(temp_buf2);
    }
  }
	
  set_clip_rect(framebuf,0,0,255,223); 
  masked_blit(temp_buf2, framebuf, 0, 0, 0, 0, 255, 223);
  masked_blit(temp_buf2, temp_buf, 0, 0, 0, 0, 255, 223);
  clear_bitmap(temp_buf2);

  if (layer2!=NULL)
  {
    do_layer(temp_buf,4, layer2, x2, y2, 3);
    do_layer(temp_buf,5, layer2, x2, y2, 3);
  }

  do_layer(temp_buf,4, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==4)
    {
      particles.spr(pcounter)->draw(temp_buf);
      //particles.del(pcounter);
    }
  }
  for(int i=31;i>=0;i--)
  {
    if(layer1->ffdata[i])
    {
      int c=combobuf[layer1->ffdata[i]].type;
      if(c!=cCHANGE)
      {
        if(layer1->ffflags[i]&ffOVERLAY)
        {
          for(int j=0;j<(layer1->ffheight[i]>>6)+1;j++)
          {
            for(int k=0;k<(layer1->ffwidth[i]>>6)+1;k++)
            {
              if(layer1->ffflags[i]&ffTRANS)
              {
                  overtiletranslucent16(temp_buf, combobuf[layer1->ffdata[i]].tile+(j*20)+(k), ((layer1->ffx[i]/10000)+(k*16)), ((layer1->ffy[i]/10000)+(j*16))+playing_field_offset, layer1->ffcset[i], combobuf[layer1->ffdata[i]].flip, 128);
              }
              else
              {
                  overtile16(temp_buf, combobuf[layer1->ffdata[i]].tile+(j*20)+(k), ((layer1->ffx[i]/10000)+(k*16)), ((layer1->ffy[i]/10000)+(j*16))+playing_field_offset, layer1->ffcset[i], combobuf[layer1->ffdata[i]].flip);
              }
            }
          }
        }
      }
    }
  }
  do_layer(temp_buf,5, layer1, x1, y1, 2);
  for (pcounter=0;pcounter<particles.Count();pcounter++)
  {
    if (((particle*)particles.spr(pcounter))->layer==5)
    {
      particles.spr(pcounter)->draw(temp_buf);
      //particles.del(pcounter);
    }
  }

  set_clip_rect(framebuf,draw_screen_clip_rect_x1,draw_screen_clip_rect_y1,draw_screen_clip_rect_x2,draw_screen_clip_rect_y2); 
  blit(temp_buf, framebuf, 0, 0, 0, 0, 255, 223);
  
  if(!(msgdisplaybuf->clip))
  {
    masked_blit(msgdisplaybuf,temp_buf2,0,0,0,playing_field_offset,256,168);
  }
  if(!(pricesdisplaybuf->clip))
  {
    masked_blit(pricesdisplaybuf,temp_buf2,0,0,0,playing_field_offset,256,168);
  }
  
  masked_blit(temp_buf2, framebuf, 0, 0, 0,0,255, 233);
  set_clip_rect(framebuf,0,0,255,223);
  put_passive_subscr(framebuf, &QMisc, 0, passive_subscreen_offset, false, false);
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
    putcombo(framebuf,(7&15)<<4,(7&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_u[dt_bomb][0],
             DoorComboSets[d].doorcset_u[dt_bomb][0]);
    putcombo(framebuf,(8&15)<<4,(8&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_u[dt_bomb][1],
             DoorComboSets[d].doorcset_u[dt_bomb][1]);
    putcombo(framebuf,(23&15)<<4,(23&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_u[dt_bomb][2],
             DoorComboSets[d].doorcset_u[dt_bomb][2]);
    putcombo(framebuf,(24&15)<<4,(24&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_u[dt_bomb][3],
             DoorComboSets[d].doorcset_u[dt_bomb][3]);
    overcombo(framebuf,(39&15)<<4,(39&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_u[0],
              DoorComboSets[d].bombdoorcset_u[0]);
    overcombo(framebuf,(40&15)<<4,(40&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_u[1],
              DoorComboSets[d].bombdoorcset_u[1]);
    break;
    case down:
    putcombo(framebuf,(151&15)<<4,(151&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_d[dt_bomb][0],
             DoorComboSets[d].doorcset_d[dt_bomb][0]);
    putcombo(framebuf,(152&15)<<4,(152&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_d[dt_bomb][1],
             DoorComboSets[d].doorcset_d[dt_bomb][1]);
    putcombo(framebuf,(167&15)<<4,(167&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_d[dt_bomb][2],
             DoorComboSets[d].doorcset_d[dt_bomb][2]);
    putcombo(framebuf,(168&15)<<4,(168&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_d[dt_bomb][3],
             DoorComboSets[d].doorcset_d[dt_bomb][3]);
    overcombo(framebuf,(135&15)<<4,(135&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_d[0],
              DoorComboSets[d].bombdoorcset_d[0]);
    overcombo(framebuf,(136&15)<<4,(136&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_d[1],
              DoorComboSets[d].bombdoorcset_d[1]);
    break;
    case left:
    putcombo(framebuf,(64&15)<<4,(64&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_l[dt_bomb][0],
             DoorComboSets[d].doorcset_l[dt_bomb][0]);
    putcombo(framebuf,(65&15)<<4,(65&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_l[dt_bomb][1],
             DoorComboSets[d].doorcset_l[dt_bomb][1]);
    putcombo(framebuf,(80&15)<<4,(80&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_l[dt_bomb][2],
             DoorComboSets[d].doorcset_l[dt_bomb][2]);
    putcombo(framebuf,(81&15)<<4,(81&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_l[dt_bomb][3],
             DoorComboSets[d].doorcset_l[dt_bomb][3]);
    putcombo(framebuf,(96&15)<<4,(96&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_l[dt_bomb][4],
             DoorComboSets[d].doorcset_l[dt_bomb][4]);
    putcombo(framebuf,(97&15)<<4,(97&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_l[dt_bomb][5],
             DoorComboSets[d].doorcset_l[dt_bomb][5]);
    overcombo(framebuf,(66&15)<<4,(66&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_l[0],
              DoorComboSets[d].bombdoorcset_l[0]);
    overcombo(framebuf,(82&15)<<4,(82&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_l[1],
              DoorComboSets[d].bombdoorcset_l[1]);
    overcombo(framebuf,(98&15)<<4,(98&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_l[2],
              DoorComboSets[d].bombdoorcset_l[2]);
    break;
    case right:
    putcombo(framebuf,(78&15)<<4,(78&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_r[dt_bomb][0],
             DoorComboSets[d].doorcset_r[dt_bomb][0]);
    putcombo(framebuf,(79&15)<<4,(79&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_r[dt_bomb][1],
             DoorComboSets[d].doorcset_r[dt_bomb][1]);
    putcombo(framebuf,(94&15)<<4,(94&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_r[dt_bomb][2],
             DoorComboSets[d].doorcset_r[dt_bomb][2]);
    putcombo(framebuf,(95&15)<<4,(95&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_r[dt_bomb][3],
             DoorComboSets[d].doorcset_r[dt_bomb][3]);
    putcombo(framebuf,(110&15)<<4,(110&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_r[dt_bomb][4],
             DoorComboSets[d].doorcset_r[dt_bomb][4]);
    putcombo(framebuf,(111&15)<<4,(111&0xF0)+playing_field_offset,
             DoorComboSets[d].doorcombo_r[dt_bomb][5],
             DoorComboSets[d].doorcset_r[dt_bomb][5]);
    overcombo(framebuf,(77&15)<<4,(77&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_r[0],
              DoorComboSets[d].bombdoorcset_r[0]);
    overcombo(framebuf,(93&15)<<4,(93&0xF0)+playing_field_offset,
              DoorComboSets[d].bombdoorcombo_r[1],
              DoorComboSets[d].bombdoorcset_r[1]);
    overcombo(framebuf,(109&15)<<4,(109&0xF0)+playing_field_offset,
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
  for (word x=0; x<animated_combos2; x++)
  {
    if (combobuf[animated_combo_table24[x][0]].nextcombo!=0)
    {
      animated_combo_table24[x][1]=0;
    }
  }
  reset_combo_animations2();
  

  mapscr ffscr = tmpscr[tmp];

  tmpscr[tmp] = TheMaps[currmap*MAPSCRS+scr];

  if(tmp==0)
  {
    for(int i=0;i<32;i++)
    {
      if((ffscr.ffflags[i]&ffCARRYOVER)&&!(ffscr.flags5&fNOFFCARRYOVER))
      {
        tmpscr[tmp].ffdata[i] = ffscr.ffdata[i];
        tmpscr[tmp].ffx[i] = ffscr.ffx[i];
        tmpscr[tmp].ffy[i] = ffscr.ffy[i];
        tmpscr[tmp].ffxdelta[i] = ffscr.ffxdelta[i];
        tmpscr[tmp].ffydelta[i] = ffscr.ffydelta[i];
        tmpscr[tmp].ffxdelta2[i] = ffscr.ffxdelta2[i];
        tmpscr[tmp].ffydelta2[i] = ffscr.ffydelta2[i];
        tmpscr[tmp].fflink[i] = ffscr.fflink[i];
        tmpscr[tmp].ffdelay[i] = ffscr.ffdelay[i];
        tmpscr[tmp].ffcset[i] = ffscr.ffcset[i];
        tmpscr[tmp].ffwidth[i] = ffscr.ffwidth[i];
        tmpscr[tmp].ffheight[i] = ffscr.ffheight[i];
        tmpscr[tmp].ffflags[i] = ffscr.ffflags[i];
        tmpscr[tmp].ffscript[i] = ffscr.ffscript[i];
		for (int j=0; j<2; ++j)
        {
          tmpscr[tmp].a[i][j] = ffscr.a[i][j];
        }
		for (int j=0; j<8; ++j)
        {
          tmpscr[tmp].d[i][j] = ffscr.d[i][j];
        }
		tmpscr[tmp].pc[i] = ffscr.pc[i];
        tmpscr[tmp].scriptflag[i] = ffscr.scriptflag[i];
		tmpscr[tmp].ffcref[i] = ffscr.ffcref[i];
		tmpscr[tmp].sp[i] = ffscr.sp[i];
		
      }
	  else
	  {
		   for(int j=0; j<256; j++)
			{
				ffstack[i][j] = 0;
			}
	  }
    }
  }
  

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
  
  if(!isdungeon()/*||TheMaps[(currmap*MAPSCRS)+currscr].flags6&fTRIGGERFPERM*/)
  {
    if(game->maps[(currmap<<7)+scr]&mSECRET)               // if special stuff done before
    {
      hiddenstair(tmp,false);
      hidden_entrance(tmp,false);
    }
  }
  

  if(game->maps[(currmap<<7)+scr]&mLOCKBLOCK)              // if special stuff done before
  {
    remove_lockblocks(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
  {
    remove_bosslockblocks(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mCHEST)              // if special stuff done before
  {
    remove_chests(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mLOCKEDCHEST)              // if special stuff done before
  {
    remove_lockedchests(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSCHEST)              // if special stuff done before
  {
    remove_bosschests(tmp,false);
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
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dUNLOCKED;
          //          putit=false;
        }
        break;

        case dBOSS:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dOPENBOSS;
          //          putit=false;
        }
        break;

        case dBOMB:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
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


  for(int i=0; i<176; ++i)
  {
    int c=tmpscr[tmp].data[i];
    int cs=tmpscr[tmp].cset[i];
    if (combobuf[c].type==cSPINTILE1)
    {
      addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+max(1,combobuf[c].frames));
    }
  }
  for (int j=0; j<6; ++j)
  {
    if (tmpscr[tmp].layermap[j]>0)
    {
      mapscr *layerscreen=TheMaps+((tmpscr[tmp].layermap[j]-1)*MAPSCRS)+tmpscr[tmp].layerscreen[j];
      for(int i=0; i<176; ++i)
      {
        int c=layerscreen->data[i];
        int cs=layerscreen->cset[i];
        if (combobuf[c].type==cSPINTILE1)
        {
          addenemy((i&15)<<4,i&0xF0,(cs<<12)+eSPINTILE1,animated_combo_table[c][1]+max(1,combobuf[c].frames));
        }
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
    if(game->maps[(currmap<<7)+scr]&mSECRET)               // if special stuff done before
    {
      hiddenstair(tmp,false);
      hidden_entrance(tmp,false);
    }
  }

  if(game->maps[(currmap<<7)+scr]&mLOCKBLOCK)              // if special stuff done before
  {
    remove_lockblocks(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSLOCKBLOCK)          // if special stuff done before
  {
    remove_bosslockblocks(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mCHEST)              // if special stuff done before
  {
    remove_chests(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mLOCKEDCHEST)              // if special stuff done before
  {
    remove_lockedchests(tmp,false);
  }

  if(game->maps[(currmap<<7)+scr]&mBOSSCHEST)              // if special stuff done before
  {
    remove_bosschests(tmp,false);
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
/*
        if((ldir^1)==i)
        {
          tmpscr[tmp].door[i]=dOPENSHUTTER;
          //          putit=false;
        }
*/
        break;

        case dLOCKED:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dUNLOCKED;
          //          putit=false;
        }
        break;

        case dBOSS:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
        {
          tmpscr[tmp].door[i]=dOPENBOSS;
          //          putit=false;
        }
        break;

        case dBOMB:
        if(game->maps[(currmap<<7)+scr]&(1<<i))
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

void putscr(BITMAP* dest,int x,int y, mapscr* scrn)
{
  if(scrn->valid==0||!show_layer_0)
  {
    rectfill(dest,x,y,x+255,y+175,0);
    return;
  }
  for(int i=0; i<176; i++)
  {
    if(scrn->flags7&fLAYER2BG||scrn->flags7&fLAYER3BG)
	  overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,scrn->data[i],scrn->cset[i]);
    else putcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,scrn->data[i],scrn->cset[i]);
  }

  if(scrn->door[0]==dBOMBED)
  {
    over_door(0,39,up);
  }
  if(scrn->door[1]==dBOMBED)
  {
    over_door(0,135,down);
  }
  if(scrn->door[2]==dBOMBED)
  {
    over_door(0,66,left);
  }

  if(scrn->door[3]==dBOMBED)
  {
    over_door(0,77,right);
  }

}

bool _walkflag(int x,int y,int cnt)
{
  //  walkflagx=x; walkflagy=y;
  if(get_bit(quest_rules,qr_LTTPWALK))
  {
    if(x<0||y<0) return false;
    if(x>255) return false;
    if(x>247&&cnt==2) return false;
    if(y>175) return false;
  }
  else
  {
    if(x<0||y<0) return false;
    if(x>248) return false;
    if(x>240&&cnt==2) return false;
    if(y>168) return false;
  }
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
  if(get_bit(quest_rules,qr_LTTPWALK))
  {
    if(x<0||y<0) return false;
    if(x>255) return false;
    if(x>247&&cnt==2) return false;
    if(y>175) return false;
  }
  else
  {
    if(x<0||y<0) return false;
    if(x>248) return false;
    if(x>240&&cnt==2) return false;
    if(y>168) return false;
  }
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
  if(blockpath && y<((get_bit(quest_rules,qr_LTTPCOLLISION))?80:88))
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
  if(tmpscr->flags&fSEA)
  {
    cont_sfx(WAV_SEA);
  }
  else
  {
    adjust_sfx(WAV_SEA,128,false);
  }
  if(tmpscr->flags&fROAR && !(game->lvlitems[dlevel]&liBOSS))
  {
    if(tmpscr->flags3&fDODONGO)
    {
      cont_sfx(WAV_DODONGO);
      adjust_sfx(WAV_ROAR,128,false);
      adjust_sfx(WAV_VADER,128,false);
    }
    else if(tmpscr->flags2&fVADER)
    {
      cont_sfx(WAV_VADER);
      adjust_sfx(WAV_ROAR,128,false);
      adjust_sfx(WAV_DODONGO,128,false);
    }
    else
    {
      cont_sfx(WAV_ROAR);
      adjust_sfx(WAV_VADER,128,false);
      adjust_sfx(WAV_DODONGO,128,false);
    }
  }
  else
  {
    adjust_sfx(WAV_ROAR,128,false);
    adjust_sfx(WAV_VADER,128,false);
    adjust_sfx(WAV_DODONGO,128,false);
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

      if(!(game->maps[(currmap<<7)+s]&mVISITED))
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

		if((tmpscr+1)->flags7&fLAYER2BG) do_layer(scrollbuf, 1, tmpscr+1, -256, playing_field_offset, 2);
		if((tmpscr+1)->flags7&fLAYER3BG) do_layer(scrollbuf, 2, tmpscr+1, -256, playing_field_offset, 2);
        putscr(scrollbuf,256,0,tmpscr+1);
        do_layer(scrollbuf, 0, tmpscr+1, -256, playing_field_offset, 2);
        if(!((tmpscr+1)->flags7&fLAYER2BG)) do_layer(scrollbuf, 1, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf,-2, tmpscr+1, -256, playing_field_offset, 2);
        if(!((tmpscr+1)->flags7&fLAYER2BG)) do_layer(scrollbuf, 2, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf, 3, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf,-1, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf, 4, tmpscr+1, -256, playing_field_offset, 2);
        do_layer(scrollbuf, 5, tmpscr+1, -256, playing_field_offset, 2);

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
	load_control_state();
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
