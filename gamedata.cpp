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

word get_gamedata_counter(gamedata *g, byte c)
{
  return g->_counter[c];
}

void set_gamedata_counter(gamedata *g, word change, byte c)
{
  g->_counter[c]=change;
  return;
}

void change_gamedata_counter(gamedata *g, short change, byte c)
{
  g->_counter[c]+=change;
  return;
}

word get_gamedata_maxcounter(gamedata *g, byte c)
{
  return g->_maxcounter[c];
}

void set_gamedata_maxcounter(gamedata *g, word change, byte c)
{
  g->_maxcounter[c]=change;
  return;
}

void change_gamedata_maxcounter(gamedata *g, short change, byte c)
{
  g->_maxcounter[c]+=change;
  return;
}

short get_gamedata_dcounter(gamedata *g, byte c)
{
  return g->_dcounter[c];
}

void set_gamedata_dcounter(gamedata *g, short change, byte c)
{
  g->_dcounter[c]=change;
  return;
}

void change_gamedata_dcounter(gamedata *g, short change, byte c)
{
  g->_dcounter[c]+=change;
  return;
}

short get_gamedata_generic(gamedata *g, byte c)
{
  return g->_generic[c];
}

void set_gamedata_generic(gamedata *g, byte change, byte c)
{
  g->_generic[c]=change;
  return;
}

void change_gamedata_generic(gamedata *g, short change, byte c)
{
  g->_generic[c]+=change;
  return;
}

word get_gamedata_life(gamedata *g)
{
  return get_gamedata_counter(g, 0);
}
void set_gamedata_life(gamedata *g, word l)
{
  set_gamedata_counter(g, l, 0);
  set_gamedata_dcounter(g, l, 0);
  return;
}
void change_gamedata_life(gamedata *g, short l)
{
  change_gamedata_counter(g, l, 0);
  change_gamedata_dcounter(g, l, 0);
  return;
}

word get_gamedata_maxlife(gamedata *g)
{
  return get_gamedata_maxcounter(g, 0);
}
void set_gamedata_maxlife(gamedata *g, word m)
{
  set_gamedata_maxcounter(g, m, 0);
  return;
}
void change_gamedata_maxlife(gamedata *g, short m)
{
  change_gamedata_maxcounter(g, m, 0);
  return;
}

short get_gamedata_drupy(gamedata *g)
{
  return get_gamedata_dcounter(g, 1);
}
void set_gamedata_drupy(gamedata *g, short d)
{
  set_gamedata_dcounter(g, d, 1);
  return;
}
void change_gamedata_drupy(gamedata *g, short d)
{
  change_gamedata_dcounter(g, d, 1);
  return;
}

word get_gamedata_rupies(gamedata *g)
{
  return get_gamedata_counter(g, 1);
}
void set_gamedata_rupies(gamedata *g, word r)
{
  set_gamedata_counter(g, r, 1);
  return;
}
void change_gamedata_rupies(gamedata *g, short r)
{
  change_gamedata_counter(g, r, 1);
  return;
}

word get_gamedata_maxarrows(gamedata *g)
{
  return get_gamedata_maxcounter(g, 3);
}
void set_gamedata_maxarrows(gamedata *g, word a)
{
  set_gamedata_maxcounter(g, a, 3);
}
void change_gamedata_maxarrows(gamedata *g, short a)
{
  change_gamedata_maxcounter(g, a, 3);
  return;
}

word get_gamedata_arrows(gamedata *g)
{
  return get_gamedata_counter(g, 3);
}
void set_gamedata_arrows(gamedata *g, word a)
{
  set_gamedata_counter(g, a, 3);
  set_gamedata_dcounter(g, a, 3);
}
void change_gamedata_arrows(gamedata *g, short a)
{
  change_gamedata_counter(g, a, 3);
  change_gamedata_dcounter(g, a, 3);
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
  return get_gamedata_counter(g, 5);
}
void set_gamedata_keys(gamedata *g, byte k)
{
  set_gamedata_counter(g, k, 5);
  set_gamedata_dcounter(g, k, 5);
  return;
}
void change_gamedata_keys(gamedata *g, short k)
{
  change_gamedata_counter(g, k, 5);
  change_gamedata_dcounter(g, k, 5);
  return;
}

byte get_gamedata_bombs(gamedata *g)
{
  return get_gamedata_counter(g, 2);
}
void set_gamedata_bombs(gamedata *g, byte k)
{
  set_gamedata_counter(g, k, 2);
  set_gamedata_dcounter(g, k, 2);
  return;
}
void change_gamedata_bombs(gamedata *g, short k)
{
  change_gamedata_counter(g, k, 2);
  change_gamedata_dcounter(g, k, 2);
  return;
}

byte get_gamedata_maxbombs(gamedata *g)
{
  return get_gamedata_maxcounter(g, 2);
}
void set_gamedata_maxbombs(gamedata *g, byte b)
{
  set_gamedata_maxcounter(g, b, 2);
  set_gamedata_maxcounter(g,b>>2,6);
  return;
}
void change_gamedata_maxbombs(gamedata *g, short b)
{
  change_gamedata_maxcounter(g, b, 2);
  change_gamedata_maxcounter(g,b>>2,6);
  return;
}

byte get_gamedata_sbombs(gamedata *g)
{
  return get_gamedata_counter(g, 6);
}
void set_gamedata_sbombs(gamedata *g, byte k)
{
  set_gamedata_counter(g, k, 6);
  set_gamedata_dcounter(g, k, 6);
  return;
}
void change_gamedata_sbombs(gamedata *g, short k)
{
  change_gamedata_counter(g, k, 6);
  change_gamedata_dcounter(g, k, 6);
  return;
}

byte get_gamedata_wlevel(gamedata *g)
{
  return get_gamedata_generic(g, 3);
}
void set_gamedata_wlevel(gamedata *g, byte l)
{
  set_gamedata_generic(g, l, 3);
  return;
}
void change_gamedata_wlevel(gamedata *g, short l)
{
  change_gamedata_generic(g, l, 3);
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
  return get_gamedata_generic(g, 0);
}
void set_gamedata_HCpieces(gamedata *g, byte p)
{
  set_gamedata_generic(g, p, 0);
  return;
}
void change_gamedata_HCpieces(gamedata *g, short p)
{
  change_gamedata_generic(g, p, 0);
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
  return get_gamedata_maxcounter(g, 4);
}
void set_gamedata_maxmagic(gamedata *g, word m)
{
  set_gamedata_maxcounter(g, m, 4);
  return;
}
void change_gamedata_maxmagic(gamedata *g, short m)
{
  change_gamedata_maxcounter(g, m, 4);
  return;
}

word get_gamedata_magic(gamedata *g)
{
  return get_gamedata_counter(g, 4);
}
void set_gamedata_magic(gamedata *g, word m)
{
  set_gamedata_counter(g, m, 4);
  return;
}
void change_gamedata_magic(gamedata *g, short m)
{
  change_gamedata_counter(g, m, 4);
  return;
}

short get_gamedata_dmagic(gamedata *g)
{
  return get_gamedata_dcounter(g, 4);
}
void set_gamedata_dmagic(gamedata *g, short d)
{
  set_gamedata_dcounter(g, d, 4);
  return;
}
void change_gamedata_dmagic(gamedata *g, short d)
{
  change_gamedata_dcounter(g, d, 4);
  return;
}

byte get_gamedata_magicdrainrate(gamedata *g)
{
  return get_gamedata_generic(g, 1);
}
void set_gamedata_magicdrainrate(gamedata *g, byte r)
{
  set_gamedata_generic(g, r, 1);
  return;
}
void change_gamedata_magicdrainrate(gamedata *g, short r)
{
  change_gamedata_generic(g, (char)r, 1);
  return;
}

byte get_gamedata_canslash(gamedata *g)
{
  return get_gamedata_generic(g, 2);
}
void set_gamedata_canslash(gamedata *g, byte s)
{
  set_gamedata_generic(g, s, 2);
  return;
}
void change_gamedata_canslash(gamedata *g, short s)
{
  change_gamedata_generic(g, s, 2);
  return;
}

/*** end of gamedata.cpp ***/
