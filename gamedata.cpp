//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zelda.cc
//
//  Main code for Zelda Classic. Originally written in
//  SPHINX C--, now rewritten in DJGPP with Allegro.
//
//--------------------------------------------------------

#include <stdio.h>
#include "zc_alleg.h"
#include "zdefs.h"

char *get_gamedata_name(gamedata *g)
{
  return g->_name;
}
void set_gamedata_name(gamedata *g, char *n)
{
  sprintf(g->_name, n);
  return;
}

byte get_gamedata_quest(gamedata *g)
{
  return g->_quest;
}
void set_gamedata_quest(gamedata *g, byte q)
{
  g->_quest=q;
  return;
}
void change_gamedata_quest(gamedata *g, short q)
{
  g->_quest+=q;
  return;
}

word get_gamedata_life(gamedata *g)
{
  return g->_life;
}
void set_gamedata_life(gamedata *g, word l)
{
  g->_life=l;
  return;
}
void change_gamedata_life(gamedata *g, short l)
{
  g->_life+=l;
  return;
}

word get_gamedata_maxlife(gamedata *g)
{
  return g->_maxlife;
}
void set_gamedata_maxlife(gamedata *g, word m)
{
  g->_maxlife=m;
  return;
}
void change_gamedata_maxlife(gamedata *g, short m)
{
  g->_maxlife+=m;
  return;
}

short get_gamedata_drupy(gamedata *g)
{
  return g->_drupy;
}
void set_gamedata_drupy(gamedata *g, short d)
{
  g->_drupy=d;
  return;
}
void change_gamedata_drupy(gamedata *g, short d)
{
  g->_drupy+=d;
  return;
}

word get_gamedata_rupies(gamedata *g)
{
  return g->_rupies;
}
void set_gamedata_rupies(gamedata *g, word r)
{
  g->_rupies=r;
  return;
}
void change_gamedata_rupies(gamedata *g, short r)
{
  g->_rupies+=r;
  return;
}

word get_gamedata_maxarrows(gamedata *g)
{
  return g->_maxarrows;
}
void set_gamedata_maxarrows(gamedata *g, word a)
{
  g->_maxarrows=a;
}
void change_gamedata_maxarrows(gamedata *g, short a)
{
  g->_maxarrows+=a;
  return;
}

word get_gamedata_arrows(gamedata *g)
{
  return g->_arrows;
}
void set_gamedata_arrows(gamedata *g, word a)
{
	g->_arrows=a;
}
void change_gamedata_arrows(gamedata *g, short a)
{
  g->_arrows+=a;
  return;
}

word get_gamedata_deaths(gamedata *g)
{
  return g->_deaths;
}
void set_gamedata_deaths(gamedata *g, word d)
{
  g->_deaths=d;
  return;
}
void change_gamedata_deaths(gamedata *g, short d)
{
  g->_deaths+=d;
  return;
}

byte get_gamedata_keys(gamedata *g)
{
  return g->_keys;
}
void set_gamedata_keys(gamedata *g, byte k)
{
  g->_keys=k;
  return;
}
void change_gamedata_keys(gamedata *g, short k)
{
  g->_keys+=k;
  return;
}

byte get_gamedata_bombs(gamedata *g)
{
  return g->items[itype_bomb];
}
void set_gamedata_bombs(gamedata *g, byte k)
{
  g->items[itype_bomb]=k;
  return;
}
void change_gamedata_bombs(gamedata *g, short k)
{
  g->items[itype_bomb]+=k;
  return;
}

byte get_gamedata_maxbombs(gamedata *g)
{
  return g->_maxbombs;
}
void set_gamedata_maxbombs(gamedata *g, byte b)
{
  g->_maxbombs=b;
  return;
}
void change_gamedata_maxbombs(gamedata *g, short b)
{
  g->_maxbombs+=b;
  return;
}

byte get_gamedata_sbombs(gamedata *g)
{
  return g->items[itype_sbomb];
}
void set_gamedata_sbombs(gamedata *g, byte k)
{
  g->items[itype_sbomb]=k;
  return;
}
void change_gamedata_sbombs(gamedata *g, short k)
{
  g->items[itype_sbomb]+=k;
  return;
}

byte get_gamedata_wlevel(gamedata *g)
{
  return g->_wlevel;
}
void set_gamedata_wlevel(gamedata *g, byte l)
{
  g->_wlevel=l;
  return;
}
void change_gamedata_wlevel(gamedata *g, short l)
{
  g->_wlevel+=l;
  return;
}

byte get_gamedata_cheat(gamedata *g)
{
  return g->_cheat;
}
void set_gamedata_cheat(gamedata *g, byte c)
{
  g->_cheat=c;
  return;
}
void change_gamedata_cheat(gamedata *g, short c)
{
  g->_cheat+=c;
  return;
}

byte get_gamedata_hasplayed(gamedata *g)
{
  return g->_hasplayed;
}
void set_gamedata_hasplayed(gamedata *g, byte p)
{
  g->_hasplayed=p;
  return;
}
void change_gamedata_hasplayed(gamedata *g, short p)
{
  g->_hasplayed+=p;
  return;
}

dword get_gamedata_time(gamedata *g)
{
  return g->_time;
}
void set_gamedata_time(gamedata *g, dword t)
{
  g->_time=t;
  return;
}
void change_gamedata_time(gamedata *g, long long t)
{
  g->_time+=t;
  return;
}

byte get_gamedata_timevalid(gamedata *g)
{
  return g->_timevalid;
}
void set_gamedata_timevalid(gamedata *g, byte t)
{
  g->_timevalid=t;
  return;
}
void change_gamedata_timevalid(gamedata *g, short t)
{
  g->_timevalid+=t;
  return;
}

byte get_gamedata_HCpieces(gamedata *g)
{
  return g->_HCpieces;
}
void set_gamedata_HCpieces(gamedata *g, byte p)
{
  g->_HCpieces=p;
  return;
}
void change_gamedata_HCpieces(gamedata *g, short p)
{
  g->_HCpieces+=p;
  return;
}

byte get_gamedata_continue_scrn(gamedata *g)
{
  return g->_continue_scrn;
}
void set_gamedata_continue_scrn(gamedata *g, byte s)
{
  g->_continue_scrn=s;
  return;
}
void change_gamedata_continue_scrn(gamedata *g, short s)
{
  g->_continue_scrn+=s;
  return;
}

byte get_gamedata_continue_dmap(gamedata *g)
{
  return g->_continue_dmap;
}
void set_gamedata_continue_dmap(gamedata *g, byte d)
{
  g->_continue_dmap=d;
  return;
}
void change_gamedata_continue_dmap(gamedata *g, short d)
{
  g->_continue_dmap+=d;
  return;
}

word get_gamedata_maxmagic(gamedata *g)
{
  return g->_maxmagic;
}
void set_gamedata_maxmagic(gamedata *g, word m)
{
  g->_maxmagic=m;
  return;
}
void change_gamedata_maxmagic(gamedata *g, short m)
{
  g->_maxmagic+=m;
  return;
}

word get_gamedata_magic(gamedata *g)
{
  return g->_magic;
}
void set_gamedata_magic(gamedata *g, word m)
{
  g->_magic=m;
  return;
}
void change_gamedata_magic(gamedata *g, short m)
{
  g->_magic+=m;
  return;
}

short get_gamedata_dmagic(gamedata *g)
{
  return g->_dmagic;
}
void set_gamedata_dmagic(gamedata *g, short d)
{
  g->_dmagic=d;
  return;
}
void change_gamedata_dmagic(gamedata *g, short d)
{
  g->_dmagic+=d;
  return;
}

byte get_gamedata_magicdrainrate(gamedata *g)
{
  return g->_magicdrainrate;
}
void set_gamedata_magicdrainrate(gamedata *g, byte r)
{
  g->_magicdrainrate=r;
  return;
}
void change_gamedata_magicdrainrate(gamedata *g, short r)
{
  g->_magicdrainrate+=r;
  return;
}

byte get_gamedata_canslash(gamedata *g)
{
  return g->_canslash;
}
void set_gamedata_canslash(gamedata *g, byte s)
{
  g->_canslash=s;
  return;
}
void change_gamedata_canslash(gamedata *g, short s)
{
  g->_canslash+=s;
  return;
}

/*** end of gamedata.cpp ***/
