#include "ffscript.h"
#include "zelda.h"
#include "link.h"
#include "guys.h"
#include "gamedata.h"
#include "zc_init.h"

#include <string.h>
#include <math.h>
#include <string>

#undef  max
#undef  min
#define max(a,b)  ((a)>(b)?(a):(b))
#define min(a,b)  ((a)<(b)?(a):(b))

using std::string;

extern sprite_list particles;
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

bool checkSprite(sprite *s)
{
	if(s==NULL)
	{
		al_trace("Script attempted to reference a nonexistent NPC!\n");
		return false;
	}
	return true;
}

long get_arg(long arg, byte i)
{
  int ret=0;
  int di;
  int mi;
  long *d[8];
  long *a[2];
  byte *sp;
  byte *ffc, *itemref, *iclass, *guyref, *lwpn, *gclass, *ewpn, *lclass, *eclass, global, linkref, scr;

  guyref=lwpn=ewpn=0;
  global=linkref=scr=0; //to get gcc to stop complaining aout unused variables
  ffc=itemref=iclass=lclass=eclass=gclass=sp=NULL;

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

  sprite *s = guys.spr(*guyref);
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
      ret=((tmpscr->ffwidth[*ffc]&63)+1)*10000; break;
    case FFCHEIGHT:
      ret=((tmpscr->ffheight[*ffc]&63)+1)*10000; break;
    case FFTWIDTH:
      ret=((tmpscr->ffwidth[*ffc]>>6)+1)*10000; break;
    case FFTHEIGHT:
      ret=((tmpscr->ffheight[*ffc]>>6)+1)*10000; break;
    case FFLINK:
      ret=(tmpscr->fflink[*ffc])*10000; break;
    case LINKX:
      ret=(int)(Link.getX())*10000; break;
    case LINKY:
      ret=(int)(Link.getY())*10000; break;
    case LINKZ:
      ret=(int)(Link.getZ())*10000; break;
    case LINKJUMP:
      ret=(int)(-Link.getFall()/GRAVITY)*10000; break;
    case LINKDIR:
      ret=(int)(Link.dir)*10000; break;
    case LINKHP:
      ret=(int)(game->get_life())*10000; break;
    case LINKMP:
      ret=(int)(game->get_magic())*10000; break;
    case LINKMAXHP:
      ret=(int)(game->get_maxlife())*10000; break;
    case LINKMAXMP:
      ret=(int)(game->get_maxmagic())*10000; break;
    case LINKACTION:
      ret=(int)(Link.getAction())*10000; break;
    case LINKITEMD:
      ret = game->item[(*d[0])/10000] ? 10000 : 0; break;
    case LINKSWORDJINX:
      ret = (int)(Link.getSwordClk())*10000; break;
    case LINKITEMJINX:
      ret = (int)(Link.getItemClk())*10000; break;
    case LINKCHARGED:
      ret = (int)(Link.isCharged())*10000; break;
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
    case ITEMZ:
      ret=((int)items.spr(*itemref)->z)*10000; break;
    case ITEMJUMP:
      ret=(-(int)items.spr(*itemref)->fall/GRAVITY)*10000; break;
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
		if(checkSprite(s))
			ret=((int)s->x)*10000; 
		break;
    case NPCY:
		if(checkSprite(s))
			ret=((int)s->y)*10000; 
		break;
    case NPCZ:
		if(checkSprite(s))
			ret=((int)s->z)*10000; 
		break;
    case NPCJUMP:
		if(checkSprite(s))
			ret=(-(int)s->fall/GRAVITY)*10000; 
		break;
    case NPCDIR:
		if(checkSprite(s))
			ret=(s->dir)*10000; 
		break;
    case NPCRATE:
		if(checkSprite(s))
			ret=((enemy*)(s))->rate*10000; 
		break;
    case NPCFRAMERATE:
		if(checkSprite(s))
			ret=((enemy*)(s))->frate*10000; 
		break;
    case NPCHALTRATE:
		if(checkSprite(s))
			ret=((enemy*)(s))->hrate*10000; 
		break;
    case NPCDRAWTYPE:
		if(checkSprite(s))
			ret=((enemy*)(s))->drawstyle*10000; 
		break;
    case NPCHP:
		if(checkSprite(s))
			ret=((enemy*)(s))->hp*10000; 
		break;
    case NPCID:
		if(checkSprite(s))
			ret=((enemy*)(s))->id*10000; 
		break;
    case NPCDP:
		if(checkSprite(s))
			ret=((enemy*)(s))->dp*10000; 
		break;
    case NPCWDP:
		if(checkSprite(s))
			ret=((enemy*)(s))->wdp*10000; 
		break;
    case NPCTILE:
		if(checkSprite(s))
			ret=((enemy*)(s))->o_tile*10000; 
		break;
    case NPCWEAPON:
		if(checkSprite(s))
			ret=((enemy*)(s))->wpn*10000; 
		break;
    case NPCITEMSET:
		if(checkSprite(s))
			ret=((enemy*)(s))->item_set*10000; 
		break;
    case NPCCSET:
		if(checkSprite(s))
			ret=((enemy*)(s))->dcset*10000; 
		break;
    case NPCBOSSPAL:
		if(checkSprite(s))
			ret=((enemy*)(s))->bosspal*10000; 
		break;
    case NPCBGSFX:
		if(checkSprite(s))
			ret=((enemy*)(s))->bgsfx*10000; 
		break;
    case NPCCOUNT:
      ret=guys.Count()*10000; break;
    case NPCEXTEND:
		if(checkSprite(s))
			ret=((enemy*)(s))->extend*10000; 
		break;
    case LWPNX:
      ret=(int)((weapon*)(Lwpns.spr(*lwpn)))->x*10000; break;
    case LWPNY:
      ret=(int)((weapon*)(Lwpns.spr(*lwpn)))->y*10000; break;
    case LWPNZ:
      ret=(int)((weapon*)(Lwpns.spr(*lwpn)))->z*10000; break;
    case LWPNJUMP:
      ret=-(int)(((weapon*)(Lwpns.spr(*lwpn)))->fall/GRAVITY)*10000; break;
    case LWPNDIR:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->dir*10000; break;
    case LWPNSTEP:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->step*10000; break;
    case LWPNANGLE:
      ret=int(((weapon*)(Lwpns.spr(*lwpn)))->angle*10000); break;
    case LWPNANGULAR:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->angular*10000; break;
    case LWPNDRAWTYPE:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->drawstyle*10000; break;
    case LWPNPOWER:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->power*10000; break;
    case LWPNDEAD:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->dead*10000; break;
    case LWPNID:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->id*10000; break;
    case LWPNTILE:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->tile*10000; break;
    case LWPNCSET:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->cs*10000; break;
    case LWPNFLASHCSET:
      ret=(((weapon*)(Lwpns.spr(*lwpn)))->o_cset>>4)*10000; break;
    case LWPNFRAMES:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->frames*10000; break;
    case LWPNFRAME:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->aframe*10000; break;
    case LWPNASPEED:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->o_speed*10000; break;
    case LWPNFLASH:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->flash*10000; break;
    case LWPNFLIP:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->flip*10000; break;
    case LWPNCOUNT:
      ret=Lwpns.Count()*10000; break;
    case LWPNEXTEND:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->extend*10000; break;
    case LWPNOTILE:
      ret=((weapon*)(Lwpns.spr(*lwpn)))->o_tile*10000; break;
    case LWPNOCSET:
      ret=(((weapon*)(Lwpns.spr(*lwpn)))->o_cset&15)*10000; break;
    case EWPNX:
      ret=(int)((weapon*)(Ewpns.spr(*ewpn)))->x*10000; break;
    case EWPNY:
      ret=(int)((weapon*)(Ewpns.spr(*ewpn)))->y*10000; break;
    case EWPNZ:
      ret=(int)((weapon*)(Ewpns.spr(*ewpn)))->z*10000; break;
    case EWPNJUMP:
      ret=-(int)(((weapon*)(Ewpns.spr(*ewpn)))->fall/GRAVITY)*10000; break;
    case EWPNDIR:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->dir*10000; break;
    case EWPNSTEP:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->step*10000; break;
    case EWPNANGLE:
      ret=int(((weapon*)(Ewpns.spr(*ewpn)))->angle*10000); break;
    case EWPNANGULAR:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->angular*10000; break;
    case EWPNDRAWTYPE:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->drawstyle*10000; break;
    case EWPNPOWER:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->power*10000; break;
    case EWPNDEAD:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->dead*10000; break;
    case EWPNID:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->id*10000; break;
    case EWPNTILE:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->tile*10000; break;
    case EWPNCSET:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->cs*10000; break;
    case EWPNFLASHCSET:
      ret=(((weapon*)(Ewpns.spr(*ewpn)))->o_cset>>4)*10000; break;
    case EWPNFRAMES:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->frames*10000; break;
    case EWPNFRAME:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->aframe*10000; break;
    case EWPNASPEED:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->o_speed*10000; break;
    case EWPNFLASH:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->flash*10000; break;
    case EWPNFLIP:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->flip*10000; break;
    case EWPNCOUNT:
      ret=Ewpns.Count()*10000; break;
    case EWPNEXTEND:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->extend*10000; break;
    case EWPNOTILE:
      ret=((weapon*)(Ewpns.spr(*ewpn)))->o_tile*10000; break;
    case EWPNOCSET:
      ret=(((weapon*)(Ewpns.spr(*ewpn)))->o_cset&15)*10000; break;
    case GAMEDEATHS:
      ret=game->get_deaths()*10000; break;
    case GAMECHEAT:
      ret=game->get_cheat()*10000; break;
    case GAMETIME:
      ret=game->get_time();  break;// Can't multiply by 10000 or the maximum result is too big
    case GAMETIMEVALID:
      ret=game->get_timevalid()?10000:0; break;
    case GAMEHASPLAYED:
      ret=game->get_hasplayed()?10000:0; break;
    case GAMEGUYCOUNT:
		mi = currmap*MAPSCRS+currscr;
      ret=game->guys[mi]*10000; break;
    case GAMECONTSCR:
      ret=game->get_continue_scrn()*10000; break;
    case GAMECONTDMAP:
      ret=game->get_continue_dmap()*10000; break;
    case GAMECOUNTERD:
      ret=game->get_counter((*(d[0]))/10000)*10000; break;
    case GAMEMCOUNTERD:
      ret=game->get_maxcounter((*(d[0]))/10000)*10000; break;
    case GAMEDCOUNTERD:
      ret=game->get_dcounter((*(d[0]))/10000)*10000; break;
    case GAMEGENERICD:
      ret=game->get_generic((*(d[0]))/10000)*10000; break;
    case GAMEITEMSD:
    ret=(game->item[(*(d[0]))/10000] ? 10000 : 0); break;
    case GAMELITEMSD:
      ret=game->lvlitems[(*(d[0]))/10000]*10000; break;
    case GAMELKEYSD:
      ret=game->lvlkeys[(*(d[0]))/10000]*10000; break;
    case GAMEMAPFLAGD:
		mi = currmap*MAPSCRS+currscr;
      ret=((game->maps[mi]>>((*(d[0]))/10000))&1)?10000:0; break;
    case GAMEMAPFLAGDD:
      ret=((game->maps[*(d[0])/10000]>>((*(d[1]))/10000))&1)?10000:0; break;
    case GAMEGUYCOUNTD:
      ret=game->guys[*(d[0])/10000]*10000; break;
    case CURMAP:
      ret=currmap*10000; break;
    case CURSCR:
      ret=currscr*10000; break;
    case CURDMAP:
      ret=currdmap*10000; break;
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
		di = ((get_currdmap())<<6) + ((get_currscr()>>4)<<3) + ((get_currscr()&15)-DMaps[get_currdmap()].xoff);
      ret=game->screen_d[di][*(d[0])/10000]; break;
    case GDD:
      ret=game->global_d[*(d[0])/10000]; break;
    case SDDD:
      ret=game->screen_d[*(d[0])/10000][*(d[1])/10000]; break;
    case SP:
      ret = (*sp)*10000; break;
    default:
	{
	  int k;
	  if(arg>=D(0)&&arg<=D(7))
	  {
	    k=arg-D(0); ret=*(d[k]); break;
	  }
	  else if(arg>=A(0)&&arg<=A(1))
	  {
	    k=arg-A(0); if(script_type!=SCRIPT_GLOBAL) ret=*(a[k]); break;
	  }
	  else if(arg>=SD(0)&&arg<=SD(7))
	  {
	    di = ((get_currdmap())<<6) + ((get_currscr()>>4)<<3) + ((get_currscr()&15)-DMaps[get_currdmap()].xoff);
	    k=arg-SD(0); ret=game->screen_d[di][k]; break;
	  }
	  else if(arg>=GD(0)&&arg<=GD(255))
	  {
	    k=arg-GD(0); ret=game->global_d[k]; break;
	  }
	  else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMECOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
	  {
	    k=(arg-GAMECOUNTER(0))/3; ret=game->get_counter(k)*10000; break;
	  }
	  else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEMCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
	  {
	    k=(arg-GAMECOUNTER(0))/3; ret=game->get_maxcounter(k)*10000; break;
	  }
	  else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEDCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
	  {
	    k=(arg-GAMECOUNTER(0))/3; ret=game->get_dcounter(k)*10000; break;
	  }
	  else if(arg>=GAMEMAPFLAG(0)&&arg<=GAMEMAPFLAG(31))
	  {
		  mi = currmap*MAPSCRS+currscr;
	    k=arg-GAMEMAPFLAG(0); ret=((game->maps[mi]>>k)&1)?10000:0; break;
	  }
	  else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOD(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
	  {
	    k=(arg-COMBOD(0))/3; ret=tmpscr->data[k]*10000; break;
	  }
	  else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOC(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
	  {
	    k=(arg-COMBOD(0))/3; ret=tmpscr->cset[k]*10000; break;
	  }
	  else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOF(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
	  {
	    k=(arg-COMBOD(0))/3; ret=tmpscr->sflag[k]*10000; break;
	  }
	  else if(arg>=GAMEGENERIC(0)&&arg<=GAMEGENERIC(255))
	  {
	    k=arg-GAMEGENERIC(0); ret=game->get_generic(k)*10000; break;
	  }
	  else if(arg>=GAMEITEMS(0)&&arg<=GAMEITEMS(255))
	  {
	    k=arg-GAMEITEMS(0); ret=(game->item[k] ? 10000 : 0); break;
	  }
	  else if(arg>=GAMELITEMS(0)&&arg<=GAMELITEMS(255))
	  {
	    k=arg-GAMELITEMS(0); ret=game->lvlitems[k]*10000; break;
	  }
	  else if(arg>=GAMELKEYS(0)&&arg<=GAMELKEYS(255))
	  {
	    k=arg-GAMELKEYS(0); ret=game->lvlkeys[k]*10000; break;
	  }
	  break;
	}
	/*
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
        if(arg==GAMECOUNTER(k)) { ret=game->get_counter(k)*10000; break; }
        if(arg==GAMEMCOUNTER(k)) { ret=game->get_maxcounter(k)*10000; break; }
        if(arg==GAMEDCOUNTER(k)) { ret=game->get_dcounter(k)*10000; break; }
        if(arg==GAMEMAPFLAG(k)) { ret=((game->maps[mi]>>k)&1)?10000:0; break; }
      }
      for(int k=0;k<256;k++)
      {
        if(arg==GD(k)) { ret=game->global_d[k]; break; }
        if(arg==GAMEGENERIC(k)) { ret=game->get_generic(k)*10000; break; }
        if(arg==GAMEITEMS(k)) { ret=(game->item[k] ? 10000 : 0); break; }
        if(arg==GAMELITEMS(k)) { ret=game->lvlitems[k]*10000; break; }
        if(arg==GAMELKEYS(k)) { ret=game->lvlkeys[k]*10000; break; }
      }
      for(int k=0;k<176;k++)
      {
        if(arg==COMBOD(k)) { ret=tmpscr->data[k]*10000; break; }
        if(arg==COMBOC(k)) { ret=tmpscr->cset[k]*10000; break; }
        if(arg==COMBOF(k)) { ret=tmpscr->sflag[k]*10000; break; }
      }
      break;*/
  }
  return ret;
}

void set_variable(long arg, byte i, long value)
{
  int di;
  int mi;
  long *d[8];
  long *a[2];
  byte *sp;
  byte *ffc, *itemref, *iclass, *guyref, *lwpn, *gclass, *ewpn, *lclass, *eclass, global, linkref, scr;
  gclass=lclass=eclass=0;

  guyref=lwpn=ewpn=0;
  global=linkref=scr=0; //to get gcc to stop complaining aout unused variables
  ffc=itemref=iclass=sp=NULL;

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
      //for(int j=0;j<8;j++)
      //{
      d[0] = &(tmpscr->d[i][0]);
	  d[1] = &(tmpscr->d[i][1]);
	  d[2] = &(tmpscr->d[i][2]);
	  d[3] = &(tmpscr->d[i][3]);
	  d[4] = &(tmpscr->d[i][4]);
	  d[5] = &(tmpscr->d[i][5]);
	  d[6] = &(tmpscr->d[i][6]);
	  d[7] = &(tmpscr->d[i][7]);

      //}
      //for(int j=0; j<2; j++)
      //{
      a[0] = &(tmpscr->a[i][0]);
	  a[1] = &(tmpscr->a[i][1]);
      //}
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
      //for(int j=0;j<8;j++)
      //{
      d[0] = &(items.spr(i)->d[0]);
	  d[1] = &(items.spr(i)->d[1]);
	  d[2] = &(items.spr(i)->d[2]);
	  d[3] = &(items.spr(i)->d[3]);
	  d[4] = &(items.spr(i)->d[4]);
	  d[5] = &(items.spr(i)->d[5]);
	  d[6] = &(items.spr(i)->d[6]);
	  d[7] = &(items.spr(i)->d[7]);
      //}
      //for(int j=0;j<2;j++)
      //{
      a[0] = &(items.spr(i)->a[0]);
	  a[1] = &(items.spr(i)->a[1]);
      //}
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
      //for(int j=0;j<8;j++)
      //{
      d[0] = &g_d[0];
	  d[1] = &g_d[1];
	  d[2] = &g_d[2];
	  d[3] = &g_d[3];
	  d[4] = &g_d[4];
	  d[5] = &g_d[5];
	  d[6] = &g_d[6];
	  d[7] = &g_d[7];
      //}
      //for(int j=0;j<2;j++)
      //{
      a[0] = NULL;
	  a[1] = NULL;
      //}
      break;
  }
  sprite *s = guys.spr(*guyref);

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
    tmpscr->ffwidth[*ffc]=((value/10000)-1)&63; break;
    case FFCHEIGHT:
      tmpscr->ffheight[*ffc]=((value/10000)-1)&63; break;
    case FFTWIDTH:
      tmpscr->ffwidth[*ffc]=(((value/10000)-1)&3)<<6; break;
    case FFTHEIGHT:
      tmpscr->ffheight[*ffc]=(((value/10000)-1)&3)<<6; break;
    case FFLINK:
      (tmpscr->fflink[*ffc])=vbound(value/10000,1,32); break;
    case LINKX:
      Link.setX(value/10000); break;
    case LINKY:
      Link.setY(value/10000); break;
    case LINKZ:
      Link.setZ(value/10000); break;
    case LINKJUMP:
      Link.setFall((-value*GRAVITY)/10000); break;
    case LINKDIR:
      Link.dir=value/10000; break;
    case LINKHP:
      game->set_life(value/10000); break;
    case LINKMP:
      game->set_magic(value/10000); break;
    case LINKMAXHP:
      game->set_maxlife(value/10000); break;
    case LINKMAXMP:
      game->set_maxmagic(value/10000); break;
    case LINKACTION:
      Link.setAction((actiontype)(value/10000)); break;
    case LINKITEMD:
      game->item[(*d[0])/10000] = (value == 0) ? false : true;
      resetItems(game);
      break;
    case LINKSWORDJINX:
      Link.setSwordClk(value/10000); break;
    case LINKITEMJINX:
      Link.setItemClk(value/10000); break;
/*
    case LINKCHARGED:
      break;
*/
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
    case ITEMZ:
      (items.spr(*itemref)->z)=(fix)(value/10000);
      if (items.spr(*itemref)->z<0)
  (items.spr(*itemref)->z)==0;
      break;
    case ITEMJUMP:
      (items.spr(*itemref)->fall)=(fix)(value*GRAVITY/10000); break;
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
      ((weapon*)(Lwpns.spr(*lwpn)))->x=(fix)value/10000; break;
    case LWPNY:
      ((weapon*)(Lwpns.spr(*lwpn)))->y=(fix)value/10000; break;
    case LWPNZ:
      ((weapon*)(Lwpns.spr(*lwpn)))->z=(fix)value/10000;
      if (((weapon*)(Lwpns.spr(*lwpn)))->z<0)
  (((weapon*)(Lwpns.spr(*lwpn)))->z)==0;
      break;
    case LWPNJUMP:
      ((weapon*)(Lwpns.spr(*lwpn)))->fall=(fix)value*GRAVITY/10000; break;
    case LWPNDIR:
      ((weapon*)(Lwpns.spr(*lwpn)))->dir=value/10000; break;
    case LWPNSTEP:
      ((weapon*)(Lwpns.spr(*lwpn)))->step=value/10000; break;
    case LWPNANGLE:
      ((weapon*)(Lwpns.spr(*lwpn)))->angle=value/10000; break;
    case LWPNANGULAR:
      ((weapon*)(Lwpns.spr(*lwpn)))->angular=(value/10000) != 0; break;
    case LWPNDRAWTYPE:
      ((weapon*)(Lwpns.spr(*lwpn)))->drawstyle=value/10000; break;
    case LWPNPOWER:
      ((weapon*)(Lwpns.spr(*lwpn)))->power=value/10000; break;
    case LWPNDEAD:
      ((weapon*)(Lwpns.spr(*lwpn)))->dead=value/10000; break;
    case LWPNID:
      ((weapon*)(Lwpns.spr(*lwpn)))->id=value/10000; break;
    case LWPNTILE:
      ((weapon*)(Lwpns.spr(*lwpn)))->tile=value/10000; break;
    case LWPNCSET:
      ((weapon*)(Lwpns.spr(*lwpn)))->cs=(value/10000)&15; break;
    case LWPNFLASHCSET:
      (((weapon*)(Lwpns.spr(*lwpn)))->o_cset)|=(value/10000)<<4; break;
    case LWPNFRAMES:
      ((weapon*)(Lwpns.spr(*lwpn)))->frames=value/10000; break;
    case LWPNFRAME:
      ((weapon*)(Lwpns.spr(*lwpn)))->aframe=value/10000; break;
    case LWPNASPEED:
      ((weapon*)(Lwpns.spr(*lwpn)))->o_speed=value/10000; break;
    case LWPNFLASH:
      ((weapon*)(Lwpns.spr(*lwpn)))->flash=value/10000; break;
    case LWPNFLIP:
      ((weapon*)(Lwpns.spr(*lwpn)))->flip=value/10000; break;
    case LWPNEXTEND:
      ((weapon*)(Lwpns.spr(*lwpn)))->extend=value/10000; break;
    case LWPNOTILE:
      ((weapon*)(Lwpns.spr(*lwpn)))->o_tile=value/10000; break;
    case LWPNOCSET:
      (((weapon*)(Lwpns.spr(*lwpn)))->o_cset)|=(value/10000)&15; break;
    case EWPNX:
      ((weapon*)(Ewpns.spr(*ewpn)))->x=(fix)value/10000; break;
    case EWPNY:
      ((weapon*)(Ewpns.spr(*ewpn)))->y=(fix)value/10000; break;
    case EWPNZ:
      ((weapon*)(Ewpns.spr(*ewpn)))->z=(fix)value/10000;
      if (((weapon*)(Ewpns.spr(*ewpn)))->z<0)
  ((weapon*)(Ewpns.spr(*ewpn)))->z=0;
      break;
    case EWPNJUMP:
      ((weapon*)(Ewpns.spr(*ewpn)))->fall=(fix)value*GRAVITY/10000; break;
    case EWPNDIR:
      ((weapon*)(Ewpns.spr(*ewpn)))->dir=value/10000; break;
    case EWPNSTEP:
      ((weapon*)(Ewpns.spr(*ewpn)))->step=value/10000; break;
    case EWPNANGLE:
      ((weapon*)(Ewpns.spr(*ewpn)))->angle=value/10000; break;
    case EWPNANGULAR:
      ((weapon*)(Ewpns.spr(*ewpn)))->angular=(value/10000)!=0; break;
    case EWPNDRAWTYPE:
      ((weapon*)(Ewpns.spr(*ewpn)))->drawstyle=value/10000; break;
    case EWPNPOWER:
      ((weapon*)(Ewpns.spr(*ewpn)))->power=value/10000; break;
    case EWPNDEAD:
      ((weapon*)(Ewpns.spr(*ewpn)))->dead=value/10000; break;
    case EWPNID:
      ((weapon*)(Ewpns.spr(*ewpn)))->id=value/10000; break;
    case EWPNTILE:
      ((weapon*)(Ewpns.spr(*ewpn)))->tile=value/10000; break;
    case EWPNCSET:
      ((weapon*)(Ewpns.spr(*ewpn)))->cs=(value/10000)&15; break;
    case EWPNFLASHCSET:
      (((weapon*)(Ewpns.spr(*ewpn)))->o_cset)|=(value/10000)<<4; break;
    case EWPNFRAMES:
      ((weapon*)(Ewpns.spr(*ewpn)))->frames=value/10000; break;
    case EWPNFRAME:
      ((weapon*)(Ewpns.spr(*ewpn)))->aframe=value/10000; break;
    case EWPNASPEED:
      ((weapon*)(Ewpns.spr(*ewpn)))->o_speed=value/10000; break;
    case EWPNFLASH:
      ((weapon*)(Ewpns.spr(*ewpn)))->flash=value/10000; break;
    case EWPNFLIP:
      ((weapon*)(Ewpns.spr(*ewpn)))->flip=value/10000; break;
    case EWPNEXTEND:
      ((weapon*)(Ewpns.spr(*ewpn)))->extend=value/10000; break;
    case EWPNOTILE:
      ((weapon*)(Ewpns.spr(*ewpn)))->o_tile=value/10000; break;
    case EWPNOCSET:
      (((weapon*)(Ewpns.spr(*ewpn)))->o_cset)|=(value/10000)&15; break;
    case NPCX:
		if(checkSprite(s))
			(s->x)=(fix)(value/10000); 
		break;
    case NPCY:
		if(checkSprite(s))
			(s->y)=(fix)(value/10000); 
		break;
    case NPCZ:
		if(checkSprite(s))
		{
			if(canfall(s->id))
			{
				(s->z)=(fix)(value/10000);
				if (s->z<0)
					(s->z)=0;
			}
		}
		break;
    case NPCJUMP:
		if(checkSprite(s))
		{
			if(canfall(s->id))
				(s->fall)=(fix)(value*GRAVITY/10000); 
		}
		break;
    case NPCDIR:
		if(checkSprite(s))
			(s->dir)=value/10000; 
		break;
    case NPCRATE:
		if(checkSprite(s))
            ((enemy*)(s))->rate=value/10000; 
		break;
    case NPCFRAMERATE:
		if(checkSprite(s))
            ((enemy*)(s))->frate=value/10000; 
		break;
    case NPCHALTRATE:
		if(checkSprite(s))
			((enemy*)(s))->hrate=value/10000; 
		break;
    case NPCDRAWTYPE:
		if(checkSprite(s))
			((enemy*)(s))->drawstyle=value/10000; 
		break;
    case NPCHP:
		if(checkSprite(s))
			((enemy*)(s))->hp=value/10000; 
		break;
    case NPCID:
		if(checkSprite(s))
			((enemy*)(s))->id=value/10000; 
		break;
    case NPCDP:
		if(checkSprite(s))
			((enemy*)(s))->dp=value/10000; 
		break;
    case NPCWDP:
		if(checkSprite(s))
			((enemy*)(s))->wdp=value/10000; 
		break;
    case NPCTILE:
		if(checkSprite(s))
			((enemy*)(s))->o_tile=value/10000; 
		break;
    case NPCWEAPON:
		if(checkSprite(s))
			((enemy*)(s))->wpn=value/10000; 
		break;
    case NPCITEMSET:
		if(checkSprite(s))
			((enemy*)(s))->item_set=value/10000; 
		break;
    case NPCCSET:
		if(checkSprite(s))
			((enemy*)(s))->dcset=value/10000; 
		break;
    case NPCBOSSPAL:
		if(checkSprite(s))
			((enemy*)(s))->bosspal=value/10000; 
		break;
    case NPCBGSFX:
		if(checkSprite(s))
			((enemy*)(s))->bgsfx=value/10000; 
		break;
    case NPCEXTEND:
		if(checkSprite(s))
			((enemy*)(s))->extend=value/10000; 
		break;
    case GAMEDEATHS:
      game->set_deaths(value/10000); break;
    case GAMECHEAT:
      game->set_cheat(value/10000); break;
    case GAMETIME:
      game->set_time(value); break; // Can't multiply by 10000 or the maximum result is too big
    case GAMETIMEVALID:
      game->set_timevalid((value/10000)?1:0); break;
    case GAMEHASPLAYED:
      game->set_hasplayed((value/10000)?1:0); break;
    case GAMEGUYCOUNT:
		 mi = currmap*MAPSCRS+currscr;
      game->guys[mi]=value/10000; break;
    case GAMECONTSCR:
      game->set_continue_scrn(value/10000); break;
    case GAMECONTDMAP:
      game->set_continue_dmap(value/10000); break;
    case GAMECOUNTERD:
      game->set_counter(value/10000, (*(d[0]))/10000); break;
    case GAMEMCOUNTERD:
      game->set_maxcounter(value/10000, (*(d[0]))/10000); break;
    case GAMEDCOUNTERD:
      game->set_dcounter(value/10000, (*(d[0]))/10000); break;
    case GAMEGENERICD:
      game->set_generic(value/10000, (*(d[0]))/10000); break;
    case GAMEITEMSD:
      game->item[(*(d[0]))/10000]= (value!=0); break;
    case GAMELITEMSD:
      game->lvlitems[(*(d[0]))/10000]=value/10000; break;
    case GAMELKEYSD:
      game->lvlkeys[(*(d[0]))/10000]=value/10000; break;
    case GAMEMAPFLAGD:
		 mi = currmap*MAPSCRS+currscr;
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
    case REFLWPN:
      *lwpn = value/10000;
    case REFLWPNCLASS:
      *lclass = value/10000;
    case REFEWPN:
      *ewpn = value/10000;
    case REFEWPNCLASS:
      *eclass = value/10000;
    case REFNPC:
      *guyref = value/10000;
    case REFNPCCLASS:
      *gclass = value/10000;
    case SDD:
		di = ((get_currdmap())<<6) + ((get_currscr()>>4)<<3) + ((get_currscr()&15)-DMaps[get_currdmap()].xoff);
      game->screen_d[di][*(d[0])/10000]=value; break;
    case GDD:
      game->global_d[*(d[0])/10000]=value; break;
    case SDDD:
      game->screen_d[*(d[0])/10000][*(d[1])/10000]=value; break;
    case SP:
      *sp = value/10000; break;
    default:
	{
	  int k;
	  if(arg>=D(0)&&arg<=D(7))
	  {
	    k=arg-D(0); *(d[k])=value; break;
	  }
	  else if(arg>=A(0)&&arg<=A(1))
	  {
	    k=arg-A(0); if(script_type!=SCRIPT_GLOBAL) *(a[k])=value; break;
	  }
	  else if(arg>=SD(0)&&arg<=SD(7))
	  {
		  di = ((get_currdmap())<<6) + ((get_currscr()>>4)<<3) + ((get_currscr()&15)-DMaps[get_currdmap()].xoff);
	    k=arg-SD(0); game->screen_d[di][k]=value; break;
	  }
	  else if(arg>=GD(0)&&arg<=GD(255))
	  {
	    k=arg-GD(0); game->global_d[k]=value; break;
	  }
	  else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMECOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
	  {
	    k=(arg-GAMECOUNTER(0))/3; game->set_counter(value/10000, k); break;
	  }
	  else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEMCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
	  {
	    k=(arg-GAMECOUNTER(0))/3; game->set_maxcounter(value/10000, k); break;
	  }
	  else if(((arg-GAMECOUNTER(0))%3)+GAMECOUNTER(0)==GAMEDCOUNTER(0)&&arg>=GAMECOUNTER(0)&&arg<=GAMEDCOUNTER(31))
	  {
	    k=(arg-GAMECOUNTER(0))/3; game->set_dcounter(value/10000, k); break;
	  }
	  else if(arg>=GAMEMAPFLAG(0)&&arg<=GAMEMAPFLAG(31))
	  {
		   mi = currmap*MAPSCRS+currscr;
	    k=arg-GAMEMAPFLAG(0); (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k):game->maps[mi]&=~(((value/10000)?1:0)<<(k)); break;
	  }
	  else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOD(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
	  {
	    k=(arg-COMBOD(0))/3; tmpscr->data[k]=(value/10000); break;
	  }
	  else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOC(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
	  {
	    k=(arg-COMBOD(0))/3; tmpscr->cset[k]=(value/10000); break;
	  }
	  else if(((arg-COMBOD(0))%3)+COMBOD(0)==COMBOF(0)&&arg>=COMBOD(0)&&arg<=COMBOF(175))
	  {
	    k=(arg-COMBOD(0))/3; tmpscr->sflag[k]=(value/10000); break;
	  }
	  else if(arg>=GAMEGENERIC(0)&&arg<=GAMEGENERIC(255))
	  {
	    k=arg-GAMEGENERIC(0); game->set_generic(value/10000, k); break;
	  }
	  else if(arg>=GAMEITEMS(0)&&arg<=GAMEITEMS(255))
	  {
	    k=arg-GAMEITEMS(0); game->item[k]= (value!=0); break;
	  }
	  else if(arg>=GAMELITEMS(0)&&arg<=GAMELITEMS(255))
	  {
	    k=arg-GAMELITEMS(0); game->lvlitems[k]=value/10000; break;
	  }
	  else if(arg>=GAMELKEYS(0)&&arg<=GAMELKEYS(255))
	  {
	    k=arg-GAMELKEYS(0); game->lvlkeys[k]=value/10000; break;
	  }
	  break;
	}

	 /* bool found=false;
      //for(int k=0;k<2;k++)
      //{
      if(arg==A(0)) { if(script_type!=SCRIPT_GLOBAL) *(a[0])=value; break; }
	  if(arg==A(1)) { if(script_type!=SCRIPT_GLOBAL) *(a[1])=value; break; }
      //}
      //for(int k=0;k<8;k++)
      //{
      if(arg==D(0)) { *(d[0])=value; break; }
      if(arg==SD(0)) { game->screen_d[di][0]=value; break; }
	  if(arg==D(1)) { *(d[1])=value; break; }
      if(arg==SD(1)) { game->screen_d[di][1]=value; break; }
	  if(arg==D(2)) { *(d[2])=value; break; }
      if(arg==SD(2)) { game->screen_d[di][2]=value; break; }
	  if(arg==D(3)) { *(d[3])=value; break; }
      if(arg==SD(3)) { game->screen_d[di][3]=value; break; }
	  if(arg==D(4)) { *(d[4])=value; break; }
      if(arg==SD(4)) { game->screen_d[di][4]=value; break; }
	  if(arg==D(5)) { *(d[5])=value; break; }
      if(arg==SD(5)) { game->screen_d[di][5]=value; break; }
	  if(arg==D(6)) { *(d[6])=value; break; }
      if(arg==SD(6)) { game->screen_d[di][6]=value; break; }
	  if(arg==D(7)) { *(d[7])=value; break; }
      if(arg==SD(7)) { game->screen_d[di][7]=value; break; }
      //}
      for(int k=0;k<4;k+=8)
      {
        if(arg==GAMECOUNTER(k)) { game->set_counter(value/10000, k); found=true; break; }
        if(arg==GAMEMCOUNTER(k)) { game->set_maxcounter(value/10000, k); found=true; break; }
        if(arg==GAMEDCOUNTER(k)) { game->set_dcounter(value/10000, k); found=true; break; }
        if(arg==GAMEMAPFLAG(k)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<k:game->maps[mi]&=~(((value/10000)?1:0)<<k); found=true; break;}

		if(arg==GAMECOUNTER(k+1)) { game->set_counter(value/10000, k+1); found=true; break; }
        if(arg==GAMEMCOUNTER(k+1)) { game->set_maxcounter(value/10000, k+1); found=true; break; }
        if(arg==GAMEDCOUNTER(k+1)) { game->set_dcounter(value/10000, k+1); found=true; break; }
        if(arg==GAMEMAPFLAG(k+1)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k+1):game->maps[mi]&=~(((value/10000)?1:0)<<(k+1)); found=true; break;}

		if(arg==GAMECOUNTER(k+2)) { game->set_counter(value/10000, k+2); found=true; break; }
        if(arg==GAMEMCOUNTER(k+2)) { game->set_maxcounter(value/10000, k+2); found=true; break; }
        if(arg==GAMEDCOUNTER(k+2)) { game->set_dcounter(value/10000, k+2); found=true; break; }
        if(arg==GAMEMAPFLAG(k+2)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k+2):game->maps[mi]&=~(((value/10000)?1:0)<<(k+2)); found=true; break;}

		if(arg==GAMECOUNTER(k+3)) { game->set_counter(value/10000, k+3); found=true; break; }
        if(arg==GAMEMCOUNTER(k+3)) { game->set_maxcounter(value/10000, k+3); found=true; break; }
        if(arg==GAMEDCOUNTER(k+3)) { game->set_dcounter(value/10000, k+3); found=true; break; }
        if(arg==GAMEMAPFLAG(k+3)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k+3):game->maps[mi]&=~(((value/10000)?1:0)<<(k+3)); found=true; break;}

		if(arg==GAMECOUNTER(k+4)) { game->set_counter(value/10000, k+4); found=true; break; }
        if(arg==GAMEMCOUNTER(k+4)) { game->set_maxcounter(value/10000, k+4); found=true; break; }
        if(arg==GAMEDCOUNTER(k+4)) { game->set_dcounter(value/10000, k+4); found=true; break; }
        if(arg==GAMEMAPFLAG(k+4)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k+4):game->maps[mi]&=~(((value/10000)?1:0)<<(k+4)); found=true; break;}

		if(arg==GAMECOUNTER(k+5)) { game->set_counter(value/10000, k+5); found=true; break; }
        if(arg==GAMEMCOUNTER(k+5)) { game->set_maxcounter(value/10000, k+5); found=true; break; }
        if(arg==GAMEDCOUNTER(k+5)) { game->set_dcounter(value/10000, k+5); found=true; break; }
        if(arg==GAMEMAPFLAG(k+5)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k+5):game->maps[mi]&=~(((value/10000)?1:0)<<(k+5)); found=true; break;}

		if(arg==GAMECOUNTER(k+6)) { game->set_counter(value/10000, k+6); found=true; break; }
        if(arg==GAMEMCOUNTER(k+6)) { game->set_maxcounter(value/10000, k+6); found=true; break; }
        if(arg==GAMEDCOUNTER(k+6)) { game->set_dcounter(value/10000, k+6); found=true; break; }
        if(arg==GAMEMAPFLAG(k+6)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k+6):game->maps[mi]&=~(((value/10000)?1:0)<<(k+6)); found=true; break;}

		if(arg==GAMECOUNTER(k+7)) { game->set_counter(value/10000, k+7); found=true; break; }
        if(arg==GAMEMCOUNTER(k+7)) { game->set_maxcounter(value/10000, k+7); found=true; break; }
        if(arg==GAMEDCOUNTER(k+7)) { game->set_dcounter(value/10000, k+7); found=true; break; }
        if(arg==GAMEMAPFLAG(k+7)) { (value/10000)?game->maps[mi]|=((value/10000)?1:0)<<(k+7):game->maps[mi]&=~(((value/10000)?1:0)<<(k+7)); found=true; break;}
      }
	  if(found==true) break;
      for(int k=0;k<32;k+=8)
      {
        if(arg==GD(k)) { game->global_d[k]=value; found=true; break; }
        if(arg==GAMEGENERIC(k)) { game->set_generic(value/10000, k); found=true; break; }
        if(arg==GAMEITEMS(k)) { game->item[k]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k)) { game->lvlitems[k]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k)) { game->lvlkeys[k]=value/10000; found=true; break; }

		if(arg==GD(k+1)) { game->global_d[k+1]=value; found=true; break; }
        if(arg==GAMEGENERIC(k+1)) { game->set_generic(value/10000, k+1); found=true; break; }
        if(arg==GAMEITEMS(k+1)) { game->item[k+1]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k+1)) { game->lvlitems[k+1]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k+1)) { game->lvlkeys[k+1]=value/10000; found=true; break; }

		if(arg==GD(k+2)) { game->global_d[k+2]=value; found=true; break; }
        if(arg==GAMEGENERIC(k+2)) { game->set_generic(value/10000, k+2); found=true; break; }
        if(arg==GAMEITEMS(k+2)) { game->item[k+2]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k+2)) { game->lvlitems[k+2]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k+2)) { game->lvlkeys[k+2]=value/10000; found=true; break; }

		if(arg==GD(k+3)) { game->global_d[k+3]=value; found=true; break; }
        if(arg==GAMEGENERIC(k+3)) { game->set_generic(value/10000, k+3); found=true; break; }
        if(arg==GAMEITEMS(k+3)) { game->item[k+3]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k+3)) { game->lvlitems[k+3]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k+3)) { game->lvlkeys[k+3]=value/10000; found=true; break; }

		if(arg==GD(k+4)) { game->global_d[k+4]=value; found=true; break; }
        if(arg==GAMEGENERIC(k+4)) { game->set_generic(value/10000, k+4); found=true; break; }
        if(arg==GAMEITEMS(k+4)) { game->item[k+4]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k+4)) { game->lvlitems[k+4]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k+4)) { game->lvlkeys[k+4]=value/10000; found=true; break; }

		if(arg==GD(k+5)) { game->global_d[k+5]=value; found=true; break; }
        if(arg==GAMEGENERIC(k+5)) { game->set_generic(value/10000, k+5); found=true; break; }
        if(arg==GAMEITEMS(k+5)) { game->item[k+5]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k+5)) { game->lvlitems[k+5]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k+5)) { game->lvlkeys[k+5]=value/10000; found=true; break; }

		if(arg==GD(k+6)) { game->global_d[k+6]=value; found=true; break; }
        if(arg==GAMEGENERIC(k+6)) { game->set_generic(value/10000, k+6); found=true; break; }
        if(arg==GAMEITEMS(k+6)) { game->item[k+6]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k+6)) { game->lvlitems[k+6]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k+6)) { game->lvlkeys[k+6]=value/10000; found=true; break; }

		if(arg==GD(k+7)) { game->global_d[k+7]=value; found=true; break; }
        if(arg==GAMEGENERIC(k+7)) { game->set_generic(value/10000, k+7); found=true; break; }
        if(arg==GAMEITEMS(k+7)) { game->item[k+7]= (value!=0); found=true; break; }
        if(arg==GAMELITEMS(k+7)) { game->lvlitems[k+7]=value/10000; found=true; break; }
        if(arg==GAMELKEYS(k+7)) { game->lvlkeys[k+7]=value/10000; found=true; break; }
      }
	  if(found==true) break;
      for(int k=0;k<22;k+=8)
      {
		if(arg==COMBOD(k)) { tmpscr->data[k]=(value/10000); found=true; break; }
		if(arg==COMBOC(k)) { tmpscr->cset[k]=(value/10000); found=true; break; }
		if(arg==COMBOF(k)) { tmpscr->sflag[k]=(value/10000); found=true; break; }

		if(arg==COMBOD(k+1)) { tmpscr->data[k+1]=(value/10000); found=true; break; }
		if(arg==COMBOC(k+1)) { tmpscr->cset[k+1]=(value/10000); found=true; break; }
		if(arg==COMBOF(k+1)) { tmpscr->sflag[k+1]=(value/10000); found=true; break; }

		if(arg==COMBOD(k+2)) { tmpscr->data[k+2]=(value/10000); found=true; break; }
		if(arg==COMBOC(k+2)) { tmpscr->cset[k+2]=(value/10000); found=true; break; }
		if(arg==COMBOF(k+2)) { tmpscr->sflag[k+2]=(value/10000); found=true; break; }

		if(arg==COMBOD(k+3)) { tmpscr->data[k+3]=(value/10000); found=true; break; }
		if(arg==COMBOC(k+3)) { tmpscr->cset[k+3]=(value/10000); found=true; break; }
		if(arg==COMBOF(k+3)) { tmpscr->sflag[k+3]=(value/10000); found=true; break; }

		if(arg==COMBOD(k+4)) { tmpscr->data[k+4]=(value/10000); found=true; break; }
		if(arg==COMBOC(k+4)) { tmpscr->cset[k+4]=(value/10000); found=true; break; }
		if(arg==COMBOF(k+4)) { tmpscr->sflag[k+4]=(value/10000); found=true; break; }

		if(arg==COMBOD(k+5)) { tmpscr->data[k+5]=(value/10000); found=true; break; }
		if(arg==COMBOC(k+5)) { tmpscr->cset[k+5]=(value/10000); found=true; break; }
		if(arg==COMBOF(k+5)) { tmpscr->sflag[k+5]=(value/10000); found=true; break; }

		if(arg==COMBOD(k+6)) { tmpscr->data[k+6]=(value/10000); found=true; break; }
		if(arg==COMBOC(k+6)) { tmpscr->cset[k+6]=(value/10000); found=true; break; }
		if(arg==COMBOF(k+6)) { tmpscr->sflag[k+6]=(value/10000); found=true; break; }

		if(arg==COMBOD(k+7)) { tmpscr->data[k+7]=(value/10000); found=true; break; }
		if(arg==COMBOC(k+7)) { tmpscr->cset[k+7]=(value/10000); found=true; break; }
		if(arg==COMBOF(k+7)) { tmpscr->sflag[k+7]=(value/10000); found=true; break; }
      }
      break;
	}*/
  }
}

void do_set(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  //long arg1;
  //long arg2;
  long temp;

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
  set_variable(*arg1,i,temp);
}

void do_trig(int script, word *pc, byte i, bool v, int type)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  //long arg1;
  //long arg2;
  long temp;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  set_variable(*arg1,i,temp2-temp);
}

void do_mult(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  //long arg1;
  //long arg2;
  long temp;
  long temp2;
  dword *scriptflag=NULL;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;
  v=v;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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

void do_clearsprites(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  //long arg1;
  //long arg2;
  double temp;
  int temp2;

  ////arg1 = ffscripts[script][*pc].arg1;
  //arg2 = ffscripts[script][*pc].arg2;

  if(v)
  {
    temp = *arg1;
  }
  else
  {
    temp=get_arg(*arg1,i);
  }
  temp=temp/10000.0;
  temp2=int(temp);
  switch (temp2)
  {
    case 0:
      guys.clear();
      break;
    case 1:
      items.clear();
      break;
    case 2:
      Ewpns.clear();
      break;
    case 3:
      Lwpns.clear();
      chainlinks.clear();
      break;
    case 4:
      decorations.clear();
      break;
    case 5:
      particles.clear();
      break;
  }
}

void do_drawing_command(int script, word *pc, byte i, int script_command)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;
  script_command=script_command;

  byte *sp=NULL;
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
  long temp2;

  temp=get_arg(*arg2,i);
  temp2=get_arg(*arg1,i);
  int j=0;
  for (; j<1000; ++j)
  {
    if (script_drawing_commands[j][0]==0)
    {
      break;
    }
  }
  if (j>=MAX_SCRIPT_DRAWING_COMMANDS)  //out of drawing command space
  {
    return;
  }
  script_drawing_commands[j][0]=script_command;
  script_drawing_commands[j][1]=read_stack(script, i, (*sp)+8);//get_arg(*arg1,i); //xy
  script_drawing_commands[j][2]=read_stack(script, i, (*sp)+7);//get_arg(*arg2,i); //color or tile/combo+cset for DRAWTILER and DRAWCOMBOR
  script_drawing_commands[j][3]=read_stack(script, i, (*sp)+6);
  script_drawing_commands[j][4]=read_stack(script, i, (*sp)+5);
  script_drawing_commands[j][5]=read_stack(script, i, (*sp)+4);
  script_drawing_commands[j][6]=read_stack(script, i, (*sp)+3);
  script_drawing_commands[j][7]=read_stack(script, i, (*sp)+2);
  script_drawing_commands[j][8]=read_stack(script, i, (*sp)+1);
  script_drawing_commands[j][9]=read_stack(script, i, *sp);
  switch(script_command)
  {
    case RECTR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+11);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+10);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+9);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+8);       //x2
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+7);       //y2
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+6);       //color
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+5);       //scale factor
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+4);       //rotation anchor x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+3);       //rotation anchor y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+2);      //rotation angle
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case CIRCLER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+10);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+9);       //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+8);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+7);       //radius
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+6);       //color
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+5);       //scale factor
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+4);       //rotation anchor x
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+3);       //rotation anchor y
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+2);       //rotation angle
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case ARCR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+13);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+12);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+11);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+10);      //radius
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+9);       //start angle
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+8);       //end angle
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+7);       //color
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+6);       //scale factor
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+5);       //rotation anchor x
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+4);      //rotation anchor y
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+3);      //rotation angle
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+2);      //closed
      script_drawing_commands[j][13]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][14]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case ELLIPSER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+11);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+10);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+9);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+8);       //radiusx
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+7);       //radiusy
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+6);       //color
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+5);       //scale factor
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+4);       //rotation anchor x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+3);       //rotation anchor y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+2);      //rotation angle
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+1);      //fill
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case LINER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+10);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+9);       //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+8);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+7);       //x2
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+6);       //y2
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+5);       //color
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+4);       //scale factor
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+3);       //rotation anchor x
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+2);       //rotation anchor y
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+1);      //rotation angle
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case PUTPIXELR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+7);       //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+6);       //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+5);       //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+4);       //color
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+3);       //rotation anchor x
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+2);       //rotation anchor y
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+1);       //rotation angle
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+0);       //opacity
      break;
    case DRAWTILER:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+13);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+12);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+11);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+10);      //tile
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+9);       //tile width
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+8);       //tile height
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+7);       //color (cset)
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+6);       //scale factor
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+5);       //rotation anchor x
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+4);      //rotation anchor y
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+3);      //rotation angle
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+2);      //flip
      script_drawing_commands[j][13]=read_stack(script, i, (*sp)+1);      //transparency
      script_drawing_commands[j][14]=read_stack(script, i, (*sp)+0);      //opacity
      break;
    case DRAWCOMBOR:
      script_drawing_commands[j][1]=read_stack(script, i, (*sp)+14);      //layer
      script_drawing_commands[j][2]=read_stack(script, i, (*sp)+13);      //x
      script_drawing_commands[j][3]=read_stack(script, i, (*sp)+12);      //y
      script_drawing_commands[j][4]=read_stack(script, i, (*sp)+11);      //combo
      script_drawing_commands[j][5]=read_stack(script, i, (*sp)+10);      //combo width
      script_drawing_commands[j][6]=read_stack(script, i, (*sp)+9);       //combo height
      script_drawing_commands[j][7]=read_stack(script, i, (*sp)+8);       //color (cset)
      script_drawing_commands[j][8]=read_stack(script, i, (*sp)+7);       //scale factor
      script_drawing_commands[j][9]=read_stack(script, i, (*sp)+6);       //rotation anchor x
      script_drawing_commands[j][10]=read_stack(script, i, (*sp)+5);      //rotation anchor y
      script_drawing_commands[j][11]=read_stack(script, i, (*sp)+4);      //rotation angle
      script_drawing_commands[j][12]=read_stack(script, i, (*sp)+3);      //frame
      script_drawing_commands[j][13]=read_stack(script, i, (*sp)+2);      //flip
      script_drawing_commands[j][14]=read_stack(script, i, (*sp)+1);      //transparency
      script_drawing_commands[j][15]=read_stack(script, i, (*sp)+0);      //opacity
      break;
  }
}

void do_push(int script, word *pc, byte i, bool v) {
  //these are here to bypass compiler warnings about unused arguments
  pc=pc;

  byte *sp=NULL;
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
  //these are here to bypass compiler warnings about unused arguments
  pc=pc;
  v=v;

  byte *sp=NULL;
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
  //these are here to bypass compiler warnings about unused arguments
  pc=pc;
  v=v;


  //long //arg1 = ffscripts[script][*pc].arg1;
  //long //arg2 = ffscripts[script][*pc].arg2;
  long sp = get_arg(*arg2,i)/10000;
  long val = read_stack(script,i,sp);
  set_variable(*arg1,i,val);
}

void do_storei(int script, word *pc, byte i, bool v) {
  //these are here to bypass compiler warnings about unused arguments
  pc=pc;
  v=v;


  //long //arg1 = ffscripts[script][*pc].arg1;
  //long //arg2 = ffscripts[script][*pc].arg2;
  long sp = get_arg(*arg2,i)/10000;
  long val = get_arg(*arg1,i);
  write_stack(script,i,sp,val);
}

void do_enqueue(int script, word *pc, byte i, bool v) {script=script; pc=pc; i=i; v=v; /*these are here to bypass compiler warnings about unused arguments*/}
void do_dequeue(int script, word *pc, byte i, bool v) {script=script; pc=pc; i=i; v=v; /*these are here to bypass compiler warnings about unused arguments*/}

void do_sfx(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;
  i=i;
  v=v;

  return;
}

void do_loaditem(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;
  i=i;
  v=v;

  return;
}

void do_loadnpc(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  long temp;
  if(v)
  {
    temp=*arg1;
  }
  else
  {
    temp=get_arg(*arg1,i)/10000;
  }
  switch(script_type)
  {
    case SCRIPT_FFC:
    (tmpscr->guyref[i])=temp;
    break;
    case SCRIPT_ITEM:
    (items.spr(i)->guyref)=temp;
    break;
    case SCRIPT_GLOBAL:
    (global_guy)=temp;
    break;
  }
}

void do_createlweapon(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  byte *wpnref=NULL;
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
  addLwpn(0,0,0,temp,0,0,0);
  *wpnref=Lwpns.Count()-1;
}

void do_createeweapon(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  byte *wpnref=NULL;
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
  addEwpn(0,0,0,temp,0,0,0);
  *wpnref=Ewpns.Count()-1;
}

void do_createitem(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  byte *itemref=NULL;
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

  //arg1 = ffscripts[script][*pc].arg1;

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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

  byte *guyref=NULL;
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
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;

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
  char tmp[100];
  sprintf(tmp,(temp < 0 ? "%06ld" : "%05ld"), temp);
  string s = tmp;
  s = s.substr(0, s.size()-4) + "." + s.substr(s.size()-4,4);
  al_trace("%s\n", s.c_str());
}

void do_tracenl(int script, word *pc, byte i, bool v)
{
  //these are here to bypass compiler warnings about unused arguments
  script=script;
  pc=pc;
  i=i;
  v=v;

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
  word *pc=NULL;
  word *ffs=NULL;
  dword *sflag=NULL;
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
        tmpscr->sidewarpdmap[0] = (*arg1)/10000;
        tmpscr->sidewarpscr[0] = (*arg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case WARPR:
        tmpscr->sidewarpdmap[0] = get_arg(*arg1,i)/10000;
        tmpscr->sidewarpscr[0] = get_arg(*arg2,i)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; break;
      case PITWARP:
        tmpscr->sidewarpdmap[0] = (*arg1)/10000;
        tmpscr->sidewarpscr[0] = (*arg2)/10000;
        tmpscr->sidewarptype[0] = wtIWARP;
        Link.ffwarp = true; Link.ffpit=true; break;
      case PITWARPR:
        tmpscr->sidewarpdmap[0] = get_arg(*arg1,i)/10000;
        tmpscr->sidewarpscr[0] = get_arg(*arg2,i)/10000;
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
      case CLEARSPRITESR:
        do_clearsprites(script, pc, i, false); break;
      case CLEARSPRITESV:
        do_clearsprites(script, pc, i, true); break;
      case RECTR:
      case CIRCLER:
      case ARCR:
      case ELLIPSER:
      case LINER:
      case PUTPIXELR:
      case DRAWTILER:
      case DRAWCOMBOR:
        do_drawing_command(script, pc, i, *command); break;
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

int ffscript_engine(bool preload)
{
  for(byte i=0;i<32;i++)
  {
    if(tmpscr->ffscript[i] && !(preload && !(tmpscr->ffflags[i]&ffPRELOAD)))
    {
      run_script(tmpscr->ffscript[i], i, SCRIPT_FFC);
    }
  }
  return 0;
}

void write_stack(int script, byte i, int sp, long value)
{
  long (*st)[256] = NULL;
  word *ffs=NULL;
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
  long (*st)[256]=NULL;
  word *ffs=NULL;
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



