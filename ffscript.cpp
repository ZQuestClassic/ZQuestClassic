#include "ffscript.h"
#include "zelda.h"
#include "link.h"
#include "guys.h"
#include "gamedata.h"

#include <string.h>
#include <math.h>

extern LinkClass Link;
enemy *script_npc;
item *script_item;
weapon *script_weapon;

int script_type=SCRIPT_GLOBAL;
word *command;
long *arg1;
long *arg2;

// gobal script variables
byte global_ffc=0;
byte global_item=0;
byte global_guy=0;
byte global_ewpn=0;
byte global_lwpn=0;
byte global_scr=0;
long g_d[8];
dword g_scriptflag=0;
long g_stack[256];
byte g_sp=0;
word g_pc=0;
word g_doscript=1;
byte global_itemclass=0;
byte global_lwpnclass=0;
byte global_ewpnclass=0;
byte global_guyclass=0;

long get_arg(long arg, byte i)
{
  int ret=0;
  int di = ((get_currdmap())<<6) + ((get_currscr()>>4)<<3) + ((get_currscr()&15)-DMaps[get_currdmap()].xoff);
  int mi = currmap*MAPSCRS+currscr;
  long *d[8];
  long *a[2];
  byte *sp;
  byte *ffc, *itemref, *iclass, *guyref, *lwpn, *gclass, *ewpn, *lclass, *eclass, global, linkref, scr;

  guyref=lwpn=ewpn=0;
  global=linkref=scr=0; //to get gcc to stop complaining aout unused variables
  
  switch(script_type)
  {
    case SCRIPT_FFC:
      ffc = &(tmpscr->ffcref[i]);
      itemref = &(tmpscr->itemref[i]);
      iclass = &(tmpscr->itemclass[i]);
      lwpn = &(tmpscr->lwpnref[i]);
      ewpn = &(tmpscr->ewpnref[i]);
      guyref = &(tmpscr->guyref[i]);
      gclass = &(tmpscr->guyclass[i]);
      lclass = &(tmpscr->lwpnclass[i]);
      eclass = &(tmpscr->ewpnclass[i]);
      sp = &(tmpscr->sp[i]);
      for(int j=0;j<8;j++)
      {
        d[j] = &(tmpscr->d[i][j]);
      }
      for(int j=0; j<2; j++)
      {
        a[j] = &(tmpscr->a[i][j]);
      }
      break;
    case SCRIPT_ITEM:
      ffc = &(items.spr(i)->ffcref);
      itemref = &(items.spr(i)->itemref);
      iclass = &(items.spr(i)->itemclass);
      lwpn = &(items.spr(i)->lwpnref);
      ewpn = &(items.spr(i)->ewpnref);
      guyref = &(items.spr(i)->guyref);
      gclass = &(items.spr(i)->guyclass);
      lclass = &(items.spr(i)->lwpnclass);
      eclass = &(items.spr(i)->ewpnclass);
      sp = &(items.spr(i)->sp);
      for(int j=0;j<8;j++)
      {
        d[j] = &(items.spr(i)->d[j]);
      }
      for(int j=0;j<2;j++)
      {
        a[j] = &(items.spr(i)->a[j]);
      }
      break;
    case SCRIPT_GLOBAL:
      ffc = &(global_ffc);
      itemref = &(global_item);
      iclass = &(global_itemclass);
      lwpn = &(global_lwpn);
      ewpn = &(global_ewpn);
      guyref = &(global_guy);
      gclass = &(global_guyclass);
      lclass = &(global_lwpnclass);
      eclass = &(global_ewpnclass);
      sp = &g_sp;
      for(int j=0;j<8;j++)
      {
        d[j] = &g_d[j];
      }
      for(int j=0;j<2;j++)
      {
        a[j] = NULL;
      }
      break;
  }

  switch(arg)
  {
    case DATA:
      ret=tmpscr->ffdata[*ffc]*10000; break;
    case FCSET:
      ret=tmpscr->ffcset[*ffc]*10000; break;
    case DELAY:
      ret=tmpscr->ffdelay[*ffc]*10000; break;
    case FX:
      ret=tmpscr->ffx[*ffc]; break;
    case FY:
      ret=tmpscr->ffy[*ffc]; break;
    case XD:
      ret=tmpscr->ffxdelta[*ffc]; break;
    case YD:
      ret=tmpscr->ffydelta[*ffc]; break;
    case XD2:
      ret=tmpscr->ffxdelta2[*ffc]; break;
    case YD2:
      ret=tmpscr->ffydelta2[*ffc]; break;
    case FFFLAGSD:
      ret=((tmpscr->ffflags[*ffc]>>((*(d[0]))/10000))&1)?10000:0; break;
    case FFCWIDTH:
      ret=((tmpscr->ffwidth[*ffc]&63)+1)*10000;
    case FFCHEIGHT:
      ret=((tmpscr->ffheight[*ffc]&63)+1)*10000;
    case FFTWIDTH:
      ret=((tmpscr->ffwidth[*ffc]>>6)+1)*10000;
    case FFTHEIGHT:
      ret=((tmpscr->ffheight[*ffc]>>6)+1)*10000;
    case FFLINK:
      ret=(tmpscr->fflink[*ffc])*10000;
    case LINKX:
      ret=(int)(Link.getX())*10000; break;
    case LINKY:
      ret=(int)(Link.getY())*10000; break;
    case LINKDIR:
      ret=(int)(Link.dir)*10000; break;
    case LINKHP:
      ret=(int)(get_gamedata_life())*10000; break;
    case LINKMP:
      ret=(int)(get_gamedata_magic())*10000; break;
    case LINKMAXHP:
      ret=(int)(get_gamedata_maxlife())*10000; break;
    case LINKMAXMP:
      ret=(int)(get_gamedata_maxmagic())*10000; break;
    case LINKACTION:
      ret=(int)(Link.getAction())*10000; break;
    case INPUTSTART:
      ret=control_state[6]?10000:0; break;
    case INPUTUP:
      ret=control_state[0]?10000:0; break;
    case INPUTDOWN:
      ret=control_state[1]?10000:0; break;
    case INPUTLEFT:
      ret=control_state[2]?10000:0; break;
    case INPUTRIGHT:
      ret=control_state[3]?10000:0; break;
    case INPUTA:
      ret=control_state[4]?10000:0; break;
    case INPUTB:
      ret=control_state[5]?10000:0; break;
    case INPUTL:
      ret=control_state[7]?10000:0; break;
    case INPUTR:
      ret=control_state[8]?10000:0; break;
    case ITEMX:
      ret=((int)items.spr(*itemref)->x)*10000; break;
    case ITEMY:
      ret=((int)items.spr(*itemref)->y)*10000; break;
    case ITEMDRAWTYPE:
      ret=items.spr(*itemref)->drawstyle*10000; break;
    case ITEMID:
      ret=items.spr(*itemref)->id*10000; break;
    case ITEMTILE:
      ret=((item *)(items.spr(*itemref)))->o_tile*10000; break;
    case ITEMCSET:
      ret=(((item *)(items.spr(*itemref)))->o_cset&15)*10000; break;
    case ITEMFLASHCSET:
      ret=(((item *)(items.spr(*itemref)))->o_cset>>4)*10000; break;
    case ITEMFRAMES:
      ret=((item *)(items.spr(*itemref)))->frames*10000; break;
    case ITEMFRAME:
      ret=((item *)(items.spr(*itemref)))->aframe*10000; break;
    case ITEMASPEED:
      ret=((item *)(items.spr(*itemref)))->o_speed*10000; break;
    case ITEMDELAY:
      ret=((item *)(items.spr(*itemref)))->o_delay*10000; break;
    case ITEMFLIP:
      ret=((item *)(items.spr(*itemref)))->flip*10000; break;
    case ITEMFLASH:
      ret=((item *)(items.spr(*itemref)))->flash*10000; break;
    case ITEMCOUNT:
      ret=(items.Count())*10000; break;
    case ICLASSFAMILY:
      ret=(itemsbuf[*iclass].family)*10000; break;
    case ICLASSFAMTYPE:
      ret=(itemsbuf[*iclass].fam_type)*10000; break;
    case ICLASSSETGAME:
      ret=(itemsbuf[*iclass].set_gamedata)?10000:0; break;
    case ICLASSAMOUNT:
      ret=(itemsbuf[*iclass].amount)*10000; break;
    case ICLASSSETMAX:
      ret=(itemsbuf[*iclass].setmax)*10000; break;
    case ICLASSMAX:
      ret=(itemsbuf[*iclass].max)*10000; break;
    case ICLASSCOUNTER:
      ret=(itemsbuf[*iclass].count)*10000; break;
    case ITEMEXTEND:
      ret=(items.spr(*itemref)->extend)*10000; break;
    case NPCX:
      ret=((int)guys.spr(*guyref)->x)*10000; break;
    case NPCY:
      ret=((int)guys.spr(*guyref)->y)*10000; break;
    case NPCDIR:
      ret=(guys.spr(*guyref)->dir)*10000; break;
    case NPCRATE:
      ret=((enemy*)(guys.spr(*guyref)))->rate*10000; break;
    case NPCFRAMERATE:
      ret=((enemy*)(guys.spr(*guyref)))->frate*10000; break;
    case NPCHALTRATE:
      ret=((enemy*)(guys.spr(*guyref)))->hrate*10000; break;
    case NPCDRAWTYPE:
      ret=((enemy*)(guys.spr(*guyref)))->drawstyle*10000; break;
    case NPCHP:
      ret=((enemy*)(guys.spr(*guyref)))->hp*10000; break;
    case NPCID:
      ret=((enemy*)(guys.spr(*guyref)))->id*10000; break;
    case NPCDP:
      ret=((enemy*)(guys.spr(*guyref)))->dp*10000; break;
    case NPCWDP:
      ret=((enemy*)(guys.spr(*guyref)))->wdp*10000; break;
    case NPCTILE:
      ret=((enemy*)(guys.spr(*guyref)))->o_tile*10000; break;
//    case NPCENEMY:
//      ret=((enemy*)(guys.spr(*guyref)))->*10000; break;
    case NPCWEAPON:
      ret=((enemy*)(guys.spr(*guyref)))->wpn*10000; break;
    case NPCITEMSET:
      ret=((enemy*)(guys.spr(*guyref)))->item_set*10000; break;
    case NPCCSET:
      ret=((enemy*)(guys.spr(*guyref)))->dcset*10000; break;
    case NPCBOSSPAL:
      ret=((enemy*)(guys.spr(*guyref)))->bosspal*10000; break;
    case NPCBGSFX:
      ret=((enemy*)(guys.spr(*guyref)))->bgsfx*10000; break;
    case NPCCOUNT:
      ret=guys.Count()*10000; break;
    case NPCEXTEND:
      ret=((enemy*)(guys.spr(*guyref)))->extend*10000; break;
    case LWPNX:
      ret=(int)((weapon*)(Lwpns.spr(*lwpn)))->x*10000;
    case LWPNY:
      ret=(int)((weapon*)(Lwpns.spr(*lwpn)))->y*10000;
    case LWPNDIR:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->dir*10000;
    case LWPNSTEP:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->step*10000;
    case LWPNANGLE:
      ret=int(((weapon*)(Lwpns.spr(*lwpn)))->angle*10000);
    case LWPNANGULAR:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->angular*10000;
    case LWPNDRAWTYPE:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->drawstyle*10000;
    case LWPNPOWER:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->power*10000;
    case LWPNDEAD:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->dead*10000;
    case LWPNID:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->id*10000;
    case LWPNTILE:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->tile*10000;
    case LWPNCSET:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->cs*10000;
    case LWPNFLASHCSET:
      ret=(((weapon*)(Lwpns.spr(*lwpn)))->o_cset>>4)*10000;
    case LWPNFRAMES:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->frames*10000;
    case LWPNFRAME:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->aframe*10000;
    case LWPNASPEED:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->o_speed*10000;
    case LWPNFLASH:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->flash*10000;
    case LWPNFLIP:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->flip*10000;
    case LWPNCOUNT:
      ret=Lwpns.Count()*10000;
    case LWPNEXTEND:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->extend*10000;
    case LWPNOTILE:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->o_tile*10000;
    case LWPNOCSET:
      ret=(((weapon*)(Lwpns.spr(*lwpn)))->o_cset&15)*10000;
    case EWPNX:
      ret=(int)((weapon*)(Ewpns.spr(*ewpn)))->x*10000;
    case EWPNY:
      ret=(int)((weapon*)(Ewpns.spr(*ewpn)))->y*10000;
    case EWPNDIR:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->dir*10000;
    case EWPNSTEP:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->step*10000;
    case EWPNANGLE:
      ret=int(((weapon*)(Ewpns.spr(*ewpn)))->angle*10000);
    case EWPNANGULAR:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->angular*10000;
    case EWPNDRAWTYPE:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->drawstyle*10000;
    case EWPNPOWER:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->power*10000;
    case EWPNDEAD:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->dead*10000;
    case EWPNID:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->id*10000;
    case EWPNTILE:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->tile*10000;
    case EWPNCSET:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->cs*10000;
    case EWPNFLASHCSET:
      ret=(((weapon*)(Ewpns.spr(*ewpn)))->o_cset>>4)*10000;
    case EWPNFRAMES:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->frames*10000;
    case EWPNFRAME:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->aframe*10000;
    case EWPNASPEED:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->o_speed*10000;
    case EWPNFLASH:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->flash*10000;
    case EWPNFLIP:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->flip*10000;
    case EWPNCOUNT:
      ret=Ewpns.Count()*10000;
    case EWPNEXTEND:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->extend*10000;
    case EWPNOTILE:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->o_tile*10000;
    case EWPNOCSET:
      ret=(((weapon*)(Ewpns.spr(*ewpn)))->o_cset&15)*10000;
    case GAMEDEATHS:
      ret=get_gamedata_deaths(game)*10000; break;
    case GAMECHEAT:
      ret=get_gamedata_cheat(game)*10000; break;
    case GAMETIME:
      ret=get_gamedata_time(game);  break;// Can't multiply by 10000 or the maximum result is too big
    case GAMETIMEVALID:
      ret=get_gamedata_timevalid(game)?10000:0; break;
    case GAMEHASPLAYED:
      ret=get_gamedata_hasplayed(game)?10000:0; break;
    case GAMEGUYCOUNT:
      ret=game->guys[mi]*10000; break;
    case GAMECONTSCR:
      ret=get_gamedata_continue_scrn(game)*10000; break;
    case GAMECONTDMAP:
      ret=get_gamedata_continue_dmap(game)*10000; break;
    case GAMECOUNTERD:
      ret=get_gamedata_counter(game, (*(d[0]))/10000)*10000; break;
    case GAMEMCOUNTERD:
      ret=get_gamedata_maxcounter(game, (*(d[0]))/10000)*10000; break;
    case GAMEDCOUNTERD:
      ret=get_gamedata_dcounter(game, (*(d[0]))/10000)*10000; break;
    case GAMEGENERICD:
      ret=get_gamedata_generic(game, (*(d[0]))/10000)*10000; break;
    case GAMEITEMSD:
      ret=game->items[(*(d[0]))/10000]*10000; break;
    case GAMELITEMSD:
      ret=game->lvlitems[(*(d[0]))/10000]*10000; break;
    case GAMELKEYSD:
      ret=game->lvlkeys[(*(d[0]))/10000]*10000; break;
    case GAMEMAPFLAGD:
      ret=((game->maps[mi]>>((*(d[0]))/10000))&1)?10000:0; break;
    case GAMEMAPFLAGDD:
      ret=((game->maps[*(d[0])/10000]>>((*(d[1]))/10000))&1)?10000:0; break;
    case GAMEGUYCOUNTD:
      ret=game->guys[*(d[0])/10000]*10000; break;
    case CURMAP:
      ret=currmap; break;
    case CURSCR:
      ret=currscr; break;
    case CURDMAP:
      ret=currdmap; break;
    case COMBODD:
      ret=tmpscr->data[(*(d[0]))/10000]*10000; break;
    case COMBOCD:
      ret=tmpscr->cset[(*(d[0]))/10000]*10000; break;
    case COMBOFD:
      ret=tmpscr->sflag[(*(d[0]))/10000]*10000; break;
    case COMBOTD:
      ret=combobuf[tmpscr->data[(*(d[0]))/10000]].type*10000; break;
    case COMBOID:
      ret=combobuf[tmpscr->data[(*(d[0]))/10000]].flag*10000; break;
    case COMBOSD:
      ret=(combobuf[tmpscr->data[(*(d[0]))/10000]].walk&15)*10000; break;
    case REFFFC:
      ret=(*ffc)*10000; break;
    case REFITEM:
      ret=(*itemref)*10000; break;
    case REFITEMCLASS:
      ret=(*iclass)*10000; break;
    case REFLWPN:
      ret=(*lwpn)*10000; break;
    case REFLWPNCLASS:
      ret=(*lclass)*10000; break;
    case REFEWPN:
      ret=(*ewpn)*10000; break;
    case REFEWPNCLASS:
      ret=(*eclass)*10000; break;
    case REFNPC:
      ret=(*guyref)*10000; break;
    case REFNPCCLASS:
      ret=(*gclass)*10000; break;
    case SDD:
      ret=game->screen_d[di][*(d[0])/10000];
    case GDD:
      ret=game->global_d[*(d[0])/10000];
    case SDDD:
      ret=game->screen_d[*(d[0])/10000][*(d[1])/10000];
    case SP:
      ret = (*sp)*10000; break;
    default:
      for(int k=0;k<2;k++)
      {
        if(arg==A(k)) { if(script_type!=SCRIPT_GLOBAL) ret=*(a[k]); break; }
      }
      for(int k=0;k<8;k++)
      {
        if(arg==D(k)) { ret=*(d[k]); break; }
        if(arg==SD(k)) { ret=game->screen_d[di][k]; break; }
      }
      for(int k=0;k<32;k++)
      {
        if(arg==GAMECOUNTER(k)) { ret=get_gamedata_counter(game, k)*10000; break; }
        if(arg==GAMEMCOUNTER(k)) { ret=get_gamedata_maxcounter(game, k)*10000; break; }
        if(arg==GAMEDCOUNTER(k)) { ret=get_gamedata_dcounter(game, k)*10000; break; }
        if(arg==GAMEMAPFLAG(k)) { ret=((game->maps[mi]>>k)&1)?10000:0; break; }
      }
      for(int k=0;k<256;k++)
      {
        if(arg==GD(k)) { ret=game->global_d[k]; break; }
        if(arg==GAMEGENERIC(k)) { ret=get_gamedata_generic(game, k)*10000; break; }
        if(arg==GAMEITEMS(k)) { ret=game->items[k]*10000; break; }
        if(arg==GAMELITEMS(k)) { ret=game->lvlitems[k]*10000; break; }
        if(arg==GAMELKEYS(k)) { ret=game->lvlkeys[k]*10000; break; }
      }
      for(int k=0;k<176;k++)
      {
        if(arg==COMBOD(k)) { ret=tmpscr->data[k]*10000; break; }
        if(arg==COMBOC(k)) { ret=tmpscr->cset[k]*10000; break; }
        if(arg==COMBOF(k)) { ret=tmpscr->sflag[k]*10000; break; }
      }
      break;
  }
  return ret;
}

void set_variable(long arg, byte i, long value)
{
  int di = ((get_currdmap())<<6) + ((get_currscr()>>4)<<3) + ((get_currscr()&15)-DMaps[get_currdmap()].xoff);
  int mi = currmap*MAPSCRS+currscr;
  long *d[8];
  long *a[2];
  byte *sp;
   byte *ffc, *itemref, *iclass, *guyref, *lwpn, *gclass, *ewpn, *lclass, *eclass, global, linkref, scr;

  guyref=lwpn=ewpn=0;
  global=linkref=scr=0; //to get gcc to stop complaining aout unused variables
  
  switch(script_type)
  {
    case SCRIPT_FFC:
      ffc = &(tmpscr->ffcref[i]);
      itemref = &(tmpscr->itemref[i]);
      iclass = &(tmpscr->itemclass[i]);
      lwpn = &(tmpscr->lwpnref[i]);
      ewpn = &(tmpscr->ewpnref[i]);
      guyref = &(tmpscr->guyref[i]);
      gclass = &(tmpscr->guyclass[i]);
      lclass = &(tmpscr->lwpnclass[i]);
      eclass = &(tmpscr->ewpnclass[i]);
      sp = &(tmpscr->sp[i]);
      for(int j=0;j<8;j++)
      {
        d[j] = &(tmpscr->d[i][j]);
      }
      for(int j=0; j<2; j++)
      {
        a[j] = &(tmpscr->a[i][j]);
      }
      break;
    case SCRIPT_ITEM:
      ffc = &(items.spr(i)->ffcref);
      itemref = &(items.spr(i)->itemref);
      iclass = &(items.spr(i)->itemclass);
      lwpn = &(items.spr(i)->lwpnref);
      ewpn = &(items.spr(i)->ewpnref);
      guyref = &(items.spr(i)->guyref);
      gclass = &(items.spr(i)->guyclass);
      lclass = &(items.spr(i)->lwpnclass);
      eclass = &(items.spr(i)->ewpnclass);
      sp = &(items.spr(i)->sp);
      for(int j=0;j<8;j++)
      {
        d[j] = &(items.spr(i)->d[j]);
      }
      for(int j=0;j<2;j++)
      {
        a[j] = &(items.spr(i)->a[j]);
      }
      break;
    case SCRIPT_GLOBAL:
      ffc = &(global_ffc);
      itemref = &(global_item);
      iclass = &(global_itemclass);
      lwpn = &(global_lwpn);
      ewpn = &(global_ewpn);
      guyref = &(global_guy);
      gclass = &(global_guyclass);
      lclass = &(global_lwpnclass);
      eclass = &(global_ewpnclass);
      sp = &g_sp;
      for(int j=0;j<8;j++)
      {
        d[j] = &g_d[j];
      }
      for(int j=0;j<2;j++)
      {
        a[j] = NULL;
      }
      break;
  }

  switch(arg)
  {
    case DATA:
      tmpscr->ffdata[*ffc]=value/10000; break;
    case FCSET:
      tmpscr->ffcset[*ffc]=value/10000; break;
    case DELAY:
      tmpscr->ffdelay[*ffc]=value/10000; break;
    case FX:
      tmpscr->ffx[*ffc]=value; break;
    case FY:
      tmpscr->ffy[*ffc]=value; break;
    case XD:
      tmpscr->ffxdelta[*ffc]=value; break;
    case YD:
      tmpscr->ffydelta[*ffc]=value; break;
    case XD2:
      tmpscr->ffxdelta2[*ffc]=value; break;
    case YD2:
      tmpscr->ffydelta2[*ffc]=value; break;
    case FFFLAGSD:
    (value/10000)?tmpscr->ffflags[*ffc]|=((value/10000)?1:0)<<((*(d[0]))/10000):tmpscr->ffflags[*ffc]&=~(((value/10000)?1:0)<<((*(d[0]))/10000)); break;
    case FFCWIDTH:
    tmpscr->ffwidth[*ffc]|=((value/10000)-1)&63;
    case FFCHEIGHT:
      tmpscr->ffheight[*ffc]|=((value/10000)-1)&63;
    case FFTWIDTH:
      tmpscr->ffwidth[*ffc]|=(((value/10000)-1)&3)<<6;
    case FFTHEIGHT:
      tmpscr->ffheight[*ffc]|=(((value/10000)-1)&3)<<6;
    case FFLINK:
      (tmpscr->fflink[*ffc])=vbound(value/10000,1,32);
    case LINKX:
      Link.setX(value/10000); break;
    case LINKY:
      Link.setY(value/10000); break;
    case LINKDIR:
      Link.dir=value/10000; break;
    case LINKHP:
      set_gamedata_life(value/10000); break;
    case LINKMP:
      set_gamedata_magic(value/10000); break;
    case LINKMAXHP:
      set_gamedata_maxlife(value/10000); break;
    case LINKMAXMP:
      set_gamedata_maxmagic(value/10000); break;
    case LINKACTION:
      Link.setAction((actiontype)(value/10000)); break;
    case INPUTSTART:
      control_state[6]=((value/10000)!=0)?true:false; break;
    case INPUTUP:
      control_state[0]=((value/10000)!=0)?true:false; break;
    case INPUTDOWN:
      control_state[1]=((value/10000)!=0)?true:false; break;
    case INPUTLEFT:
      control_state[2]=((value/10000)!=0)?true:false; break;
    case INPUTRIGHT:
      control_state[3]=((value/10000)!=0)?true:false; break;
    case INPUTA:
      control_state[4]=((value/10000)!=0)?true:false; break;
    case INPUTB:
      control_state[5]=((value/10000)!=0)?true:false; break;
    case INPUTL:
      control_state[7]=((value/10000)!=0)?true:false; break;
    case INPUTR:
      control_state[8]=((value/10000)!=0)?true:false; break;
    case ITEMX:
      (items.spr(*itemref)->x)=(fix)(value/10000); break;
    case ITEMY:
      (items.spr(*itemref)->y)=(fix)(value/10000); break;
    case ITEMDRAWTYPE:
      items.spr(*itemref)->drawstyle=value/10000; break;
    case ITEMID:
      items.spr(*itemref)->id=value/10000; break;
    case ITEMTILE:
      items.spr(*itemref)->tile=value/10000; break;
    case ITEMCSET:
      ((item *)(items.spr(*itemref)))->o_cset|=(value/10000)&15; break;
    case ITEMFLASHCSET:
      ((item *)(items.spr(*itemref)))->o_cset|=(value/10000)<<4; break;
    case ITEMFRAMES:
      ((item *)(items.spr(*itemref)))->frames=value/10000; break;
    case ITEMFRAME:
      ((item*)(items.spr(*itemref)))->aframe=value/10000; break;
    case ITEMASPEED:
      ((item *)(items.spr(*itemref)))->o_speed=value/10000; break;
    case ITEMDELAY:
      ((item *)(items.spr(*itemref)))->o_delay=value/10000; break;
    case ITEMFLIP:
      ((item *)(items.spr(*itemref)))->flip=(value/10000); break;
    case ITEMFLASH:
      ((item *)(items.spr(*itemref)))->flash=(value/10000)?1:0; break;
      /*case ITEMCOUNT:
      (items.Count())*10000; break;*/
    case ICLASSFAMILY:
      (itemsbuf[*iclass].family)=value/10000; break;
    case ICLASSFAMTYPE:
      (itemsbuf[*iclass].fam_type)=value/10000; break;
    case ICLASSSETGAME:
      (itemsbuf[*iclass].set_gamedata)=(value/10000)?1:0; break;
    case ICLASSAMOUNT:
      (itemsbuf[*iclass].amount)=value/10000; break;
    case ICLASSSETMAX:
      (itemsbuf[*iclass].setmax)=value/10000; break;
    case ICLASSMAX:
      (itemsbuf[*iclass].max)=value/10000; break;
    case ICLASSCOUNTER:
      (itemsbuf[*iclass].count)=value/10000; break;
    case ITEMEXTEND:
      (items.spr(*itemref)->extend)=value/10000; break;
    case LWPNX:
      ((weapon*)(Lwpns.spr(*lwpn)))->x=(fix)value/10000;
    case LWPNY:
      ((weapon*)(Lwpns.spr(*lwpn)))->y=(fix)value/10000;
    case LWPNDIR:
      ((weapon*)(Lwpns.spr(*lwpn)))->dir=value/10000;
    case LWPNSTEP:
      ((weapon*)(Lwpns.spr(*lwpn)))->step=value/10000;
    case LWPNANGLE:
      ((weapon*)(Lwpns.spr(*lwpn)))->angle=value/10000;
    case LWPNANGULAR:
      ((weapon*)(Lwpns.spr(*lwpn)))->angular=value/10000;
    case LWPNDRAWTYPE:
      ((weapon*)(Lwpns.spr(*lwpn)))->drawstyle=value/10000;
    case LWPNPOWER:
      ((weapon*)(Lwpns.spr(*lwpn)))->power=value/10000;
    case LWPNDEAD:
      ((weapon*)(Lwpns.spr(*lwpn)))->dead=value/10000;
    case LWPNID:
      ((weapon*)(Lwpns.spr(*lwpn)))->id=value/10000;
    case LWPNTILE:
      ((weapon*)(Lwpns.spr(*lwpn)))->tile=value/10000;
    case LWPNCSET:
      ((weapon*)(Lwpns.spr(*lwpn)))->cs=(value/10000)&15;
    case LWPNFLASHCSET:
      (((weapon*)(Lwpns.spr(*lwpn)))->o_cset)|=(value/10000)<<4;
    case LWPNFRAMES:
      ((weapon*)(Lwpns.spr(*lwpn)))->frames=value/10000;
    case LWPNFRAME:
      ((weapon*)(Lwpns.spr(*lwpn)))->aframe=value/10000;
    case LWPNASPEED:
      ((weapon*)(Lwpns.spr(*lwpn)))->o_speed=value/10000;
    case LWPNFLASH:
      ((weapon*)(Lwpns.spr(*lwpn)))->flash=value/10000;
    case LWPNFLIP:
      ((weapon*)(Lwpns.spr(*lwpn)))->flip=value/10000;
    case LWPNEXTEND:
      ((weapon*)(Lwpns.spr(*lwpn)))->extend=value/10000;
    case LWPNOTILE:
      ((weapon*)(Lwpns.spr(*lwpn)))->o_tile=value/10000;
    case LWPNOCSET:
      (((weapon*)(Lwpns.spr(*lwpn)))->o_cset)|=(value/10000)&15;
    case EWPNX:
      ((weapon*)(Ewpns.spr(*ewpn)))->x=(fix)value/10000;
    case EWPNY:
      ((weapon*)(Ewpns.spr(*ewpn)))->y=(fix)value/10000;
    case EWPNDIR:
      ((weapon*)(Ewpns.spr(*ewpn)))->dir=value/10000;
    case EWPNSTEP:
      ((weapon*)(Ewpns.spr(*ewpn)))->step=value/10000;
    case EWPNANGLE:
      ((weapon*)(Ewpns.spr(*ewpn)))->angle=value/10000;
    case EWPNANGULAR:
      ((weapon*)(Ewpns.spr(*ewpn)))->angular=value/10000;
    case EWPNDRAWTYPE:
      ((weapon*)(Ewpns.spr(*ewpn)))->drawstyle=value/10000;
    case EWPNPOWER:
      ((weapon*)(Ewpns.spr(*ewpn)))->power=value/10000;
    case EWPNDEAD:
      ((weapon*)(Ewpns.spr(*ewpn)))->dead=value/10000;
    case EWPNID:
      ((weapon*)(Ewpns.spr(*ewpn)))->id=value/10000;
    case EWPNTILE:
      ((weapon*)(Ewpns.spr(*ewpn)))->tile=value/10000;
    case EWPNCSET:
      ((weapon*)(Ewpns.spr(*ewpn)))->cs=(value/10000)&15;
    case EWPNFLASHCSET:
      (((weapon*)(Ewpns.spr(*ewpn)))->o_cset)|=(value/10000)<<4;
    case EWPNFRAMES:
      ((weapon*)(Ewpns.spr(*ewpn)))->frames=value/10000;
    case EWPNFRAME:
      ((weapon*)(Ewpns.spr(*ewpn)))->aframe=value/10000;
    case EWPNASPEED:
      ((weapon*)(Ewpns.spr(*ewpn)))->o_speed=value/10000;
    case EWPNFLASH:
      ((weapon*)(Ewpns.spr(*ewpn)))->flash=value/10000;
    case EWPNFLIP:
      ((weapon*)(Ewpns.spr(*ewpn)))->flip=value/10000;
    case EWPNEXTEND:
      ((weapon*)(Ewpns.spr(*ewpn)))->extend=value/10000;
    case EWPNOTILE:
      ((weapon*)(Ewpns.spr(*ewpn)))->o_tile=value/10000;
    case EWPNOCSET:
      (((weapon*)(Ewpns.spr(*ewpn)))->o_cset)|=(value/10000)&15;
    case NPCX:
      (guys.spr(*guyref)->x)=(fix)value/10000; break;
    case NPCY:
      (guys.spr(*guyref)->y)=(fix)value/10000; break;
    case NPCDIR:
      (guys.spr(*guyref)->dir)=value/10000; break;
    case NPCRATE:
      ((enemy*)(guys.spr(*guyref)))->rate=value/10000; break;
    case NPCFRAMERATE:
      ((enemy*)(guys.spr(*guyref)))->frate=value/10000; break;
    case NPCHALTRATE:
      ((enemy*)(guys.spr(*guyref)))->hrate=value/10000; break;
    case NPCDRAWTYPE:
      ((enemy*)(guys.spr(*guyref)))->drawstyle=value/10000; break;
    case NPCHP:
      ((enemy*)(guys.spr(*guyref)))->hp=value/10000; break;
    case NPCID:
      ((enemy*)(guys.spr(*guyref)))->id=value/10000; break;
    case NPCDP:
      ((enemy*)(guys.spr(*guyref)))->dp=value/10000; break;
    case NPCWDP:
      ((enemy*)(guys.spr(*guyref)))->wdp=value/10000; break;
    case NPCTILE:
      ((enemy*)(guys.spr(*guyref)))->o_tile=value/10000; break;
      /*case NPCENEMY:
      ret=((enemy*)(guys.spr(*guyref)))->*10000; break;*/
    case NPCWEAPON:
      ((enemy*)(guys.spr(*guyref)))->wpn=value/10000; break;
    case NPCITEMSET:
      ((enemy*)(guys.spr(*guyref)))->item_set=value/10000; break;
    case NPCCSET:
      ((enemy*)(guys.spr(*guyref)))->dcset=value/10000; break;
    case NPCBOSSPAL:
      ((enemy*)(guys.spr(*guyref)))->bosspal=value/10000; break;
    case NPCBGSFX:
      ((enemy*)(guys.spr(*guyref)))->bgsfx=value/10000; break;
      /*case NPCCOUNT:
      guys.Count()*10000; break;*/
    case NPCEXTEND:
      ((enemy*)(guys.spr(*guyref)))->extend=value/10000; break;
    case GAMEDEATHS:
      set_gamedata_deaths(game,value/10000); break;
    case GAMECHEAT:
      set_gamedata_cheat(game,value/10000); break;
    case GAMETIME:
      set_gamedata_time(game,value); break; // Can't multiply by 10000 or the maximum result is too big
    case GAMETIMEVALID:
      set_gamedata_timevalid(game, (value/10000)?1:0); break;
    case GAMEHASPLAYED:
      set_gamedata_hasplayed(game, (value/10000)?1:0); break;
    case GAMEGUYCOUNT:
      game->guys[mi]=value/10000; break;
    case GAMECONTSCR:
      set_gamedata_continue_scrn(game, value/10000); break;
    case GAMECONTDMAP:
      set_gamedata_continue_dmap(game, value/10000); break;
    case GAMECOUNTERD:
      set_gamedata_counter(game, value/10000, (*(d[0]))/10000); break;
    case GAMEMCOUNTERD:
      set_gamedata_maxcounter(game, value/10000, (*(d[0]))/10000); break;
    case GAMEDCOUNTERD:
      set_gamedata_dcounter(game, value/10000, (*(d[0]))/10000); break;
    case GAMEGENERICD:
      set_gamedata_generic(game, value/10000, (*(d[0]))/10000); break;
    case GAMEITEMSD:
      game->items[(*(d[0]))/10000]=value/10000; break;
    case GAMELITEMSD:
      game->lvlitems[(*(d[0]))/10000]=value/10000; break;
    case GAMELKEYSD:
      game->lvlkeys[(*(d[0]))/10000]=value/10000; break;
    case GAMEMAPFLAGD:
      (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<((*(d[0]))/10000):game->maps[mi]&=~(((value/10000)?1:0)<<((*(d[0]))/10000)); break;
    case GAMEMAPFLAGDD:
      (value/10000)?game->maps[*(d[0])/10000]|=((value/10000)?1:0)<<((*(d[1]))/10000):game->maps[*(d[0])/10000]&=~(((value/10000)?1:0)<<((*(d[1]))/10000)); break;
    case GAMEGUYCOUNTD:
      game->guys[*(d[0])/10000]=value/10000; break;
    case COMBODD:
      tmpscr->data[(*(d[0]))/10000]=value/10000; break;
    case COMBOCD:
      tmpscr->cset[(*(d[0]))/10000]=value/10000; break;
    case COMBOFD:
      tmpscr->sflag[(*(d[0]))/10000]=value/10000; break;
    case COMBOTD:
      combobuf[tmpscr->data[(*(d[0]))/10000]].type=value/10000; break;
    case COMBOID:
      combobuf[tmpscr->data[(*(d[0]))/10000]].flag=value/10000; break;
    case COMBOSD:
      combobuf[tmpscr->data[(*(d[0]))/10000]].walk=(value/10000)&15; break;
    case REFFFC:
      *ffc = value/10000; break;
    case REFITEM:
      *itemref = value/10000; break;
    case REFITEMCLASS:
      *iclass = value/10000; break;
    case SDD:
      game->screen_d[di][*(d[0])/10000]=value;
    case GDD:
      game->global_d[*(d[0])/10000]=value;
    case SDDD:
      game->screen_d[*(d[0])/10000][*(d[1])/10000]=value;
    case SP:
      *sp = value/10000; break;
    default:
      for(int k=0;k<2;k++)
      {
        if(arg==A(k)) { if(script_type!=SCRIPT_GLOBAL) *(a[k])=value; break; }
      }
      for(int k=0;k<8;k++)
      {
        if(arg==D(k)) { *(d[k])=value; break; }
        if(arg==SD(k)) { game->screen_d[di][k]=value; break; }
      }
      for(int k=0;k<32;k++)
      {
        if(arg==GAMECOUNTER(k)) { set_gamedata_counter(game, value/10000, k); break; }
        if(arg==GAMEMCOUNTER(k)) { set_gamedata_maxcounter(game, value/10000, k); break; }
        if(arg==GAMEDCOUNTER(k)) { set_gamedata_dcounter(game, value/10000, k); break; }
        if(arg==GAMEMAPFLAG(k)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<k:game->maps[mi]&=~(((value/10000)?1:0)<<k); break;}
      }
      for(int k=0;k<256;k++)
      {
        if(arg==GD(k)) { game->global_d[k]=value; break; }
        if(arg==GAMEGENERIC(k)) { set_gamedata_generic(game, value/10000, k); break; }
        if(arg==GAMEITEMS(k)) { game->items[k]=value/10000; break; }
        if(arg==GAMELITEMS(k)) { game->lvlitems[k]=value/10000; break; }
        if(arg==GAMELKEYS(k)) { game->lvlkeys[k]=value/10000; break; }
      }
      for(int k=0;k<176;k++)
      {
        if(arg==COMBOD(k)) tmpscr->data[k]=(value/10000);
        if(arg==COMBOC(k)) tmpscr->cset[k]=(value/10000);
        if(arg==COMBOF(k)) tmpscr->sflag[k]=(value/10000);
      }
      break;
  }
}

void do_set(int script, word *pc, byte i, bool v)
{
  ////long arg1;
  ////long arg2;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;
  ////arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  set_variable(*arg1,i,temp);
}

void do_trig(int script, word *pc, byte i, bool v, int type)
{
  ////long arg1;
  ////long arg2;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;
  ////arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  if(type==0)
  {
    double rangle = (temp/10000) * PI / 180.0;
    temp = (long)(sin(rangle)*10000);
  }
  else if(type==1) 
  {
    double rangle = (temp/10000) * PI / 180.0;
    temp = (long)(cos(rangle)*10000);
  }
  else if(type==2) 
  {
    double rangle = (temp/10000) * PI / 180.0;
    temp = (long)(tan(rangle)*10000);
  }
  set_variable(*arg1,i,temp);
}

void do_add(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp2=get_arg(*arg1,i);
  set_variable(*arg1,i,temp2+temp);
}

void do_sub(int script, word *pc, byte i, bool v)
{
  ////long arg1;
  //long arg2;
  long temp;
  long temp2;

  //arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp2=get_arg(*arg1,i);
  set_variable(*arg1,i,temp2-temp);
}

void do_mult(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  double temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp = temp/10000.0;
  temp2=get_arg(*arg1,i);
  set_variable(*arg1,i,(long)(temp2*temp));
}

void do_div(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  double temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp2=get_arg(*arg1,i);
  temp = temp/10000.0;
  set_variable(*arg1,i,(long)(temp2/temp));
}

void do_mod(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp = temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(temp2%temp)*10000);
}


void do_comp(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;
  dword *scriptflag;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  switch(script_type)
  {
  case SCRIPT_FFC:
    scriptflag = &(tmpscr->scriptflag[i]);
    break;
  case SCRIPT_ITEM:
    scriptflag = &(items.spr(i)->scriptflag);
    break;
  case SCRIPT_GLOBAL:
    scriptflag = &g_scriptflag;
    break;
  }

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp2=get_arg(*arg1,i);

  if(temp2 >= temp) *scriptflag |= MOREFLAG; else *scriptflag &= ~MOREFLAG;
  if(temp2 == temp) *scriptflag |= TRUEFLAG; else *scriptflag &= ~TRUEFLAG;
}

void do_loada(int script, word *pc, byte i, int a)
{
  //long arg1;
  //long arg2;
  long temp;
  int j;
  long *na[2];

  switch(script_type)
  {
  case SCRIPT_FFC:
    na[0] = &(tmpscr->a[i][0]);
    na[1] = &(tmpscr->a[i][1]);
    break;
  case SCRIPT_ITEM:
      na[0] = &(items.spr(i)->a[0]);
    na[1] = &(items.spr(i)->a[1]);
    break;
  case SCRIPT_GLOBAL:
    na[0] = NULL;
    na[1] = NULL;
    break;
  }

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(a) j = (*(na[1])/10000)-1;
  else j = (*(na[0])/10000)-1;

  temp=get_arg(*arg2,j);
  set_variable(*arg1,i,temp);
}

void do_seta(int script, word *pc, byte i, int a)
{
  //long arg1;
  //long arg2;
  long temp;
  int j;
  long *na[2];

  switch(script_type)
  {
  case SCRIPT_FFC:
    na[0] = &(tmpscr->a[i][0]);
    na[1] = &(tmpscr->a[i][1]);
    break;
  case SCRIPT_ITEM:
      na[0] = &(items.spr(i)->a[0]);
    na[1] = &(items.spr(i)->a[1]);
    break;
   case SCRIPT_GLOBAL:
    na[0] = NULL;
    na[1] = NULL;
    break;
  }

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(a) j = (*(na[1])/10000)-1;
  else j = (*(na[0])/10000)-1;

  temp=get_arg(*arg2,i);
  set_variable(*arg1,j,temp);
}

void do_abs(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  temp=get_arg(*arg1,i);
  set_variable(*arg1,i,abs(temp));
}

void do_min(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp2=get_arg(*arg1,i);
  set_variable(*arg1,i,min(temp2,temp));
}

void do_max(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp2=get_arg(*arg1,i);
  set_variable(*arg1,i,max(temp2,temp));
}


void do_rnd(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp2=get_arg(*arg1,i);
  set_variable(*arg1,i,rand()%temp);
}

void do_factorial(int script, word *pc, byte i, bool v)
{
  //long arg1;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    return;  //must factorial a register, not a value
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
    if (temp<2)
    {
      return;  //can't/don't need to factorial these
    }
  }
  temp2=1;
  for (long temp3=temp; temp>1; temp--)
  {
    temp2*=temp3;
  }
  set_variable(*arg1,i,temp2*10000);
}

void do_power(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  double temp;
  double temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp = temp/10000.0;
  temp2=get_arg(*arg1,i)/10000.0;
  set_variable(*arg1,i,((long)pow(temp2,temp))*10000);
}

void do_ipower(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  double temp;
  double temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp = 10000.0/temp;
  temp2=get_arg(*arg1,i)/10000.0;
  set_variable(*arg1,i,((long)pow(temp2,temp))*10000);
}

void do_and(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(temp2&temp)*10000);
}

void do_or(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(temp2|temp)*10000);
}

void do_xor(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(temp2^temp)*10000);
}

void do_nand(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(!(temp2&temp))*10000);
}

void do_nor(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffs2cripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(!(temp2|temp))*10000);
}

void do_xnor(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(!(temp2^temp))*10000);
}

void do_not(int script, word *pc, byte i, bool v)
{
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i);
  }
  temp=temp/10000;
  set_variable(*arg1,i,(!temp)*10000);
}

void do_lshift(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(temp2<<temp)*10000);
}

void do_rshift(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  long temp;
  long temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000;
  temp2=get_arg(*arg1,i)/10000;
  set_variable(*arg1,i,(temp2>>temp)*10000);
}

void do_sqroot(int script, word *pc, byte i, bool v)
{
  //long arg1;
  //long arg2;
  double temp;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg2;
  }
  else
  {
    temp=get_arg(*arg2,i);
  }
  temp=temp/10000.0;
  set_variable(*arg1,i,int((sqrt(temp))*10000));
}

void do_push(int script, word *pc, byte i, bool v) {
  //long //arg1 = ffscripts[script][*pc].arg1;
  byte *sp;
  switch(script_type)
  {
  case SCRIPT_FFC:
    sp = &(tmpscr->sp[i]);
    break;
  case SCRIPT_ITEM:
    sp = &(items.spr(i)->sp);
    break;
  case SCRIPT_GLOBAL:
    sp = &g_sp;
    break;
  }
  long temp;
  if(v)
    temp = *arg1;
  else
    temp = get_arg(*arg1,i);
  (*sp)--;
  write_stack(script,i,*sp,temp);
}

void do_pop(int script, word *pc, byte i, bool v) {
  //long //arg1 = ffscripts[script][*pc].arg1;
  byte *sp;
  switch(script_type)
  {
  case SCRIPT_FFC:
    sp = &(tmpscr->sp[i]);
    break;
  case SCRIPT_ITEM:
    sp = &(items.spr(i)->sp);
    break;
  case SCRIPT_GLOBAL:
    sp = &g_sp;
    break;
  }
  long temp = read_stack(script,i,*sp);
  (*sp)++;
  set_variable(*arg1,i,temp);
}

void do_loadi(int script, word *pc, byte i, bool v) {
  //long //arg1 = ffscripts[script][*pc].arg1;
  //long //arg2 = ffscripts[script][*pc].arg2;
  long sp = get_arg(*arg2,i)/10000;
  long val = read_stack(script,i,sp);
  set_variable(*arg1,i,val);
}

void do_storei(int script, word *pc, byte i, bool v) {
  //long //arg1 = ffscripts[script][*pc].arg1;
  //long //arg2 = ffscripts[script][*pc].arg2;
  long sp = get_arg(*arg2,i)/10000;
  long val = get_arg(*arg1,i);
  write_stack(script,i,sp,val);
}

void do_enqueue(int script, word *pc, byte i, bool v) {}
void do_dequeue(int script, word *pc, byte i, bool v) {}

void do_sfx(int script, word *pc, byte i, bool v)
{
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = (*arg1)/10000;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  sfx(temp);
}

void do_loadweapon(int script, word *pc, byte i, bool v)
{
  return;
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  script_weapon=(weapon*)Ewpns.spr(temp);
}

void do_loaditem(int script, word *pc, byte i, bool v)
{
  return;
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  script_item=(item*)items.spr(temp);
}

void do_loadnpc(int script, word *pc, byte i, bool v)
{
  return;
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  script_npc=(enemy*)guys.spr(temp);
}

void do_createlweapon(int script, word *pc, byte i, bool v)
{
  byte *wpnref;
  switch(script_type)
  {
  case SCRIPT_FFC:
    wpnref=&(tmpscr->lwpnref[i]);
  break;
  case SCRIPT_ITEM:
    wpnref=&(items.spr(i)->lwpnref);
  break;
  case SCRIPT_GLOBAL:
    wpnref=&(global_lwpn);
  break;
  }
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  addLwpn(0,0,temp,0,0,0);
  *wpnref=Lwpns.Count()-1;
}

void do_createeweapon(int script, word *pc, byte i, bool v)
{
  byte *wpnref;
  switch(script_type)
  {
  case SCRIPT_FFC:
    wpnref=&(tmpscr->ewpnref[i]);
  break;
  case SCRIPT_ITEM:
    wpnref=&(items.spr(i)->ewpnref);
  break;
  case SCRIPT_GLOBAL:
    wpnref=&(global_ewpn);
  break;
  }
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  addEwpn(0,0,temp,0,0,0);
  *wpnref=Ewpns.Count()-1;
}

void do_createitem(int script, word *pc, byte i, bool v)
{
  byte *itemref;
  switch(script_type)
  {
  case SCRIPT_FFC:
    itemref=&(tmpscr->itemref[i]);
  break;
  case SCRIPT_ITEM:
    itemref=&(items.spr(i)->itemref);
  break;
  case SCRIPT_GLOBAL:
    itemref=&(global_item);
  break;
  }
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  additem(0,0,temp,0);
  *itemref=items.Count()-1;
}

void do_createnpc(int script, word *pc, byte i, bool v)
{
  byte *guyref;
  switch(script_type)
  {
  case SCRIPT_FFC:
    guyref=&(tmpscr->guyref[i]);
  break;
  case SCRIPT_ITEM:
    guyref=&(items.spr(i)->guyref);
  break;
  case SCRIPT_GLOBAL:
    guyref=&(global_guy);
  break;
  }
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  addenemy(0,0,temp,-10);
  *guyref=guys.Count()-1;
}

void do_trace(int script, word *pc, byte i, bool v)
{
  //long arg1;
  long temp;

  ////arg1 = ffscripts[script][*pc].arg1;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i);
  }
  al_trace("%ld.%04d\n", temp/10000, abs(temp)%10000);
}

void do_tracenl(int script, word *pc, byte i, bool v)
{
  al_trace("\n");
}

// Let's do this
int run_script(int script, byte i, int stype)
{
  word scommand;

  scommand = 0;  //to get gcc to stop complaining aout unused variables
  //long arg1=0;
  //long arg2=0;
  //arg1=arg2; //to avoid unused variables warnings
  word *pc;
  word *ffs;
  dword *sflag;
  script_type = stype;
  switch(script_type)
  {
  case SCRIPT_FFC:
      pc = &(tmpscr->pc[i]);
    command = &(ffscripts[script][*pc].command);
    arg1 = &(ffscripts[script][*pc].arg1);
    arg2 = &(ffscripts[script][*pc].arg2);
    ffs = &(tmpscr->ffscript[i]);
    sflag = &(tmpscr->scriptflag[i]);
  tmpscr->ffcref[i]=i;
    break;
  case SCRIPT_ITEM:
    pc = &(items.spr(i)->pc);
    command = &(itemscripts[script][*pc].command);
    arg1 = &(itemscripts[script][*pc].arg1);
    arg2 = &(itemscripts[script][*pc].arg2);
    ffs = &(items.spr(i)->doscript);
    sflag = &(items.spr(i)->scriptflag);
  items.spr(i)->itemref = i;
    break;
  case SCRIPT_GLOBAL:
    pc = &g_pc;
    command = &(globalscripts[script][*pc].command);
      arg1 = &(globalscripts[script][*pc].arg1);
    arg2 = &(globalscripts[script][*pc].arg2);
    ffs = &g_doscript;
    sflag = &g_scriptflag;
    break;
  }

  bool increment = true;
  
  while(*ffs != 0 && (*command!=0xFFFF)&&(*command!=WAITFRAME))
  {
    if(key[KEY_ALT]&&key[KEY_F4])
  {
    quit_game();
    exit(101);
  }
  switch(*command)
    {
      case SETV:
        do_set(script, pc, i, true); break;
      case SETR:
        do_set(script, pc, i, false); break;
      case ADDV:
        do_add(script, pc, i, true); break;
      case ADDR:
        do_add(script, pc, i, false); break;
      case SUBV:
        do_sub(script, pc, i, true); break;
      case SUBR:
        do_sub(script, pc, i, false); break;
      case MULTV:
        do_mult(script, pc, i, true); break;
      case MULTR:
        do_mult(script, pc, i, false); break;
      case DIVV:
        do_div(script, pc, i, true); break;
      case DIVR:
        do_div(script, pc, i, false); break;
      case MODV:
        do_mod(script, pc, i, true); break;
      case MODR:
        do_mod(script, pc, i, false); break;
      case GOTO:
        *pc = *arg1; increment = false; break;
      case CHECKTRIG:
        break;
      case WARP:
        tmpscr->sidewarpdmap[0] = *arg1;
        tmpscr->sidewarpscr[0] = *arg2;
		tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
    case WARPR:
        tmpscr->sidewarpdmap[0] = get_arg(*arg1,i);
        tmpscr->sidewarpscr[0] = get_arg(*arg2,i);
		tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
    case PITWARP:
        tmpscr->sidewarpdmap[0] = *arg1;
        tmpscr->sidewarpscr[0] = *arg2;
		tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
    case PITWARPR:
        tmpscr->sidewarpdmap[0] = get_arg(*arg1,i);
        tmpscr->sidewarpscr[0] = get_arg(*arg2,i);
		tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case COMPAREV:
        do_comp(script, pc, i, true); break;
      case COMPARER:
        do_comp(script, pc, i, false); break;
      case GOTOTRUE:
        if(*sflag & TRUEFLAG)
        {*pc = *arg1; increment = false;} break;
      case GOTOFALSE:
        if(!(*sflag & TRUEFLAG))
        {*pc = *arg1; increment = false;} break;
      case GOTOMORE:
        if(*sflag & MOREFLAG)
        {*pc = *arg1; increment = false;} break;
      case GOTOLESS:
        if(!(*sflag & MOREFLAG))
        {*pc = *arg1; increment = false;} break;
      case LOAD1:
        do_loada(script, pc, i, 0); break;
      case LOAD2:
        do_loada(script, pc, i, 1); break;
      case SETA1:
        do_seta(script, pc, i, 0); break;
      case SETA2:
        do_seta(script, pc, i, 1); break;
      case QUIT:
        *ffs = 0; break;
      case SINV:
        do_trig(script, pc, i, true, 0); break;
      case SINR:
        do_trig(script, pc, i, false, 0); break;
      case COSV:
        do_trig(script, pc, i, true, 1); break;
      case COSR:
        do_trig(script, pc, i, false, 1); break; 
      case TANV:
        do_trig(script, pc, i, true, 2); break;
      case TANR:
        do_trig(script, pc, i, false, 2); break;
      case ABSR:
        do_abs(script, pc, i, false); break;
      case MINR:
        do_min(script, pc, i, false); break;
      case MINV:
        do_min(script, pc, i, true); break;
      case MAXR:
        do_max(script, pc, i, false); break;
      case MAXV:
        do_max(script, pc, i, true); break;
      case RNDR:
        do_rnd(script, pc, i, false); break;
      case RNDV:
        do_rnd(script, pc, i, true); break;
      case FACTORIAL:
        do_factorial(script, pc, i, false); break;
      case POWERR:
        do_power(script, pc, i, false); break;
      case POWERV:
        do_power(script, pc, i, true); break;
      case IPOWERR:
        do_ipower(script, pc, i, false); break;
      case IPOWERV:
        do_ipower(script, pc, i, true); break;
      case ANDR:
        do_and(script, pc, i, false); break;
      case ANDV:
        do_and(script, pc, i, true); break;
      case ORR:
        do_or(script, pc, i, false); break;
      case ORV:
        do_or(script, pc, i, true); break;
      case XORR:
        do_xor(script, pc, i, false); break;
      case XORV:
        do_xor(script, pc, i, true); break;
      case NANDR:
        do_nand(script, pc, i, false); break;
      case NANDV:
        do_nand(script, pc, i, true); break;
      case NORR:
        do_nor(script, pc, i, false); break;
      case NORV:
        do_nor(script, pc, i, true); break;
      case XNORR:
        do_xnor(script, pc, i, false); break;
      case XNORV:
        do_xnor(script, pc, i, true); break;
      case NOT:
        do_not(script, pc, i, false); break;
      case LSHIFTR:
        do_lshift(script, pc, i, false); break;
      case LSHIFTV:
        do_lshift(script, pc, i, true); break;
      case RSHIFTR:
        do_rshift(script, pc, i, false); break;
      case RSHIFTV:
        do_rshift(script, pc, i, true); break;
      case TRACER:
        do_trace(script, pc, i, false); break;
      case TRACEV:
        do_trace(script, pc, i, true); break;
      case TRACENL:
        do_tracenl(script, pc, i, false); break;
      case LOOP:
        if(get_arg(*arg2,i)>0)
        {
          *pc = *arg1;
          increment = false;
        }
        else
        {
          set_variable(*arg1,i,*arg1-1);
        }
        break;
      case PUSHR:
        do_push(script, pc, i, false); break;
      case PUSHV:
        do_push(script, pc, i, true); break;
      case POP:
        do_pop(script, pc, i, false); break;
      case ENQUEUER:
        do_enqueue(script, pc, i, false); break;
      case ENQUEUEV:
        do_enqueue(script, pc, i, true); break;
      case DEQUEUE:
        do_dequeue(script, pc, i, false); break;
      case PLAYSOUNDR:
        do_sfx(script, pc, i, false); break;
      case PLAYSOUNDV:
        do_sfx(script, pc, i, true); break;
      case LOADWEAPONR:
        do_loadweapon(script, pc, i, false); break;
      case LOADWEAPONV:
        do_loadweapon(script, pc, i, true); break;
      case LOADITEMR:
        do_loaditem(script, pc, i, false); break;
      case LOADITEMV:
        do_loaditem(script, pc, i, true); break;
      case LOADNPCR:
        do_loadnpc(script, pc, i, false); break;
      case LOADNPCV:
        do_loadnpc(script, pc, i, true); break;
      case CREATELWEAPONR:
        do_createlweapon(script, pc, i, false); break;
      case CREATELWEAPONV:
        do_createlweapon(script, pc, i, true); break;
      case CREATEEWEAPONR:
        do_createeweapon(script, pc, i, false); break;
      case CREATEEWEAPONV:
        do_createeweapon(script, pc, i, true); break;
      case CREATEITEMR:
        do_createitem(script, pc, i, false); break;
      case CREATEITEMV:
        do_createitem(script, pc, i, true); break;
      case CREATENPCR:
        do_createnpc(script, pc, i, false); break;
      case CREATENPCV:
        do_createnpc(script, pc, i, true); break;
      case LOADI:
        do_loadi(script,pc,i,true); break;
      case STOREI:
        do_storei(script,pc,i,true); break;
      case GOTOR:
        {
          long temp = *arg1;
          int tmp2 = (get_arg(temp,i)/10000)-1;
          *pc = tmp2;
          increment = false; 
        }
        break;
      case SQROOTV:
        do_sqroot(script,pc,i,true); break;
      case SQROOTR:
        do_sqroot(script,pc,i,false); break;
    }
    if(increment)
    {
      *pc+=1;
    }
    increment = true;
    switch(script_type)
    {
      case SCRIPT_FFC:
        command = &(ffscripts[script][*pc].command);
        arg1 = &(ffscripts[script][*pc].arg1);
        arg2 = &(ffscripts[script][*pc].arg2);
        break;
      case SCRIPT_ITEM:
        command = &(itemscripts[script][*pc].command);
        arg1 = &(itemscripts[script][*pc].arg1);
        arg2 = &(itemscripts[script][*pc].arg2);
        break;
      case SCRIPT_GLOBAL:
        command = &(globalscripts[script][*pc].command);
        arg1 = &(globalscripts[script][*pc].arg1);
        arg2 = &(globalscripts[script][*pc].arg2);
        break;
    }
  }
  if(*command==0xFFFF)
  {
    *ffs = 0;
  }
  else
  {
    *pc+=1;
  }
  return 0;
}

int ffscript_engine()
{
  for(byte i=0;i<32;i++)
  {
    if(tmpscr->ffscript[i])
    {
      run_script(tmpscr->ffscript[i], i, SCRIPT_FFC);
    }
  }
  return 0;
}

void write_stack(int script, byte i, int sp, long value)
{
  long (*st)[256];
  word *ffs;
  switch(script_type)
  {
    case SCRIPT_FFC:
      st = &(ffstack[i]);
      ffs = &(tmpscr->ffscript[i]);
      break;
    case SCRIPT_ITEM:
      st = &(items.spr(i)->stack);
      ffs = &(items.spr(i)->doscript);
      break;
    case SCRIPT_GLOBAL:
      st = &g_stack;
      ffs = &g_doscript;
      break;
  }

  if(sp == 0)
  {
    char tmp[200];
    sprintf(tmp, "Stack over or underflow: script %d\n", script);
    al_trace(tmp);
    *ffs=0;
  }
  (*st)[sp]=value;
}

int read_stack(int script, byte i, int sp)
{
  long (*st)[256];
  word *ffs;
  switch(script_type)
  {
    case SCRIPT_FFC:
      st = &(ffstack[i]);
      ffs = &(tmpscr->ffscript[i]);
      break;
    case SCRIPT_ITEM:
      st = &(items.spr(i)->stack);
      ffs = &(items.spr(i)->doscript);
      break;
    case SCRIPT_GLOBAL:
      st = &g_stack;
      ffs = &g_doscript;
      break;
  }
  if(sp == 0)
  {
    char tmp[200];
    sprintf(tmp, "Stack over or underflow: script %d\n", script);
    al_trace(tmp);
    *ffs=0;
  }
  return (*st)[sp];
}

