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

extern int dlevel;

// Debug variables: these log certain operations on gamedata when active. 
// Should help me debug those item bugs.

// #define DEBUG_GD_ITEMS
// #define DEBUG_GD_COUNTERS
// #define DEBUG_GD_HCP

char *gamedata::get_name()
{
  return _name;
}
void gamedata::set_name(char *n)
{
  sprintf(_name, n);
  return;
}

byte gamedata::get_quest()
{
  return _quest;
}
void gamedata::set_quest(byte q)
{
  _quest=q;
  return;
}
void gamedata::change_quest(short q)
{
  _quest+=q;
  return;
}

word gamedata::get_counter(byte c)
{
  return _counter[c];
}

void gamedata::set_counter(word change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
	al_trace("Changing counter %i from %i to %i\n", c, _counter[c], change);
#endif
  _counter[c]=change;
  return;
}

void gamedata::change_counter(short change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
	al_trace("Changing counter %i from %i by + %i\n", c, _counter[c], change);
#endif
  _counter[c]+=change;
  return;
}

word gamedata::get_maxcounter(byte c)
{
  return _maxcounter[c];
}

void gamedata::set_maxcounter(word change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
	al_trace("Changing max counter %i from %i to %i\n", c, _maxcounter[c], change);
#endif
  _maxcounter[c]=change;
  return;
}

void gamedata::change_maxcounter(short change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
	al_trace("Changing max counter %i from %i by +%i\n", c, _maxcounter[c], change);
#endif
  _maxcounter[c]+=change;
  return;
}

short gamedata::get_dcounter(byte c)
{
  return _dcounter[c];
}

void gamedata::set_dcounter(short change, byte c)
{
#ifdef DEBUG_GD_COUNTERS
	al_trace("Changing D counter %i from %i to %i\n", c, _maxcounter[c], change);
#endif
  _dcounter[c]=change;
  return;
}

void gamedata::change_dcounter(short change, byte c)
{
 #ifdef DEBUG_GD_COUNTERS
	al_trace("Changing D counter %i from %i to %i\n", c, _maxcounter[c], change);
#endif
  _dcounter[c]+=change;
  return;
}

short gamedata::get_generic(byte c)
{
  return _generic[c];
}

void gamedata::set_generic(byte change, byte c)
{
  _generic[c]=change;
  return;
}

void gamedata::change_generic(short change, byte c)
{
  _generic[c]+=change;
  return;
}

word gamedata::get_life()
{
  return get_counter(0);
}
void gamedata::set_life(word l)
{
  set_counter(l, 0);
  set_dcounter(l, 0);
  return;
}
void gamedata::change_life(short l)
{
  change_counter(l, 0);
  change_dcounter(l, 0);
  return;
}

word gamedata::get_maxlife()
{
  return get_maxcounter(0);
}
void gamedata::set_maxlife(word m)
{
  set_maxcounter(m, 0);
  return;
}
void gamedata::change_maxlife(short m)
{
  change_maxcounter(m, 0);
  return;
}

short gamedata::get_drupy()
{
  return get_dcounter(1);
}
void gamedata::set_drupy(short d)
{
  set_dcounter(d, 1);
  return;
}
void gamedata::change_drupy(short d)
{
  change_dcounter(d, 1);
  return;
}

word gamedata::get_rupies()
{
  return get_counter(1);
}
void gamedata::set_rupies(word r)
{
  set_counter(r, 1);
  return;
}
void gamedata::change_rupies(short r)
{
  change_counter(r, 1);
  return;
}

word gamedata::get_maxarrows()
{
  return get_maxcounter(3);
}
void gamedata::set_maxarrows(word a)
{
  set_maxcounter(a, 3);
}
void gamedata::change_maxarrows(short a)
{
  change_maxcounter(a, 3);
  return;
}

word gamedata::get_arrows()
{
  return get_counter(3);
}
void gamedata::set_arrows(word a)
{
  set_counter(a, 3);
  set_dcounter(a, 3);
}
void gamedata::change_arrows(short a)
{
  change_counter(a, 3);
  change_dcounter(a, 3);
  return;
}

word gamedata::get_deaths()
{
  return _deaths;
}
void gamedata::set_deaths(word d)
{
  _deaths=d;
  return;
}
void gamedata::change_deaths(short d)
{
  _deaths+=d;
  return;
}

byte gamedata::get_keys()
{
  return get_counter(5);
}
void gamedata::set_keys(byte k)
{
  set_counter(k, 5);
  set_dcounter(k, 5);
  return;
}
void gamedata::change_keys(short k)
{
  change_counter(k, 5);
  change_dcounter(k, 5);
  return;
}

byte gamedata::get_bombs()
{
  return get_counter(2);
}
void gamedata::set_bombs(byte k)
{
  set_counter(k, 2);
  set_dcounter(k, 2);
  return;
}
void gamedata::change_bombs(short k)
{
  change_counter(k, 2);
  change_dcounter(k, 2);
  return;
}

byte gamedata::get_maxbombs()
{
  return get_maxcounter(2);
}
void gamedata::set_maxbombs(byte b)
{
  set_maxcounter(b, 2);
  set_maxcounter(b>>2,6);
  return;
}
void gamedata::change_maxbombs(short b)
{
  change_maxcounter(b, 2);
  change_maxcounter(b>>2,6);
  return;
}

byte gamedata::get_sbombs()
{
  return get_counter(6);
}
void gamedata::set_sbombs(byte k)
{
  set_counter(k, 6);
  set_dcounter(k, 6);
  return;
}
void gamedata::change_sbombs(short k)
{
  change_counter(k, 6);
  change_dcounter(k, 6);
  return;
}

word gamedata::get_wlevel()
{
  return get_generic(3);
}
void gamedata::set_wlevel(word l)
{
  set_generic(l, 3);
  return;
}
void gamedata::change_wlevel(short l)
{
  change_generic(l, 3);
  return;
}

byte gamedata::get_cheat()
{
  return _cheat;
}
void gamedata::set_cheat(byte c)
{
  _cheat=c;
  return;
}
void gamedata::change_cheat(short c)
{
  _cheat+=c;
  return;
}

byte gamedata::get_hasplayed()
{
  return _hasplayed;
}
void gamedata::set_hasplayed(byte p)
{
  _hasplayed=p;
  return;
}
void gamedata::change_hasplayed(short p)
{
  _hasplayed+=p;
  return;
}

dword gamedata::get_time()
{
  return _time;
}
void gamedata::set_time(dword t)
{
  _time=t;
  return;
}
void gamedata::change_time(long long t)
{
  _time+=t;
  return;
}

byte gamedata::get_timevalid()
{
  return _timevalid;
}
void gamedata::set_timevalid(byte t)
{
  _timevalid=t;
  return;
}
void gamedata::change_timevalid(short t)
{
  _timevalid+=t;
  return;
}

byte gamedata::get_HCpieces()
{
  return get_generic(0);
}
void gamedata::set_HCpieces(byte p)
{
#ifdef DEBUG_GD_HCP
	al_trace("Setting HCP to %i\n",p);
#endif

  set_generic(p, 0);
  return;
}
void gamedata::change_HCpieces(short p)
{
#ifdef DEBUG_GD_HCP
	al_trace("Changing HCP by %i\n",p);
#endif
  change_generic(p, 0);
  return;
}

byte gamedata::get_continue_scrn()
{
  return _continue_scrn;
}
void gamedata::set_continue_scrn(byte s)
{
  _continue_scrn=s;
  return;
}
void gamedata::change_continue_scrn(short s)
{
  _continue_scrn+=s;
  return;
}

word gamedata::get_continue_dmap()
{
  return _continue_dmap;
}
void gamedata::set_continue_dmap(word d)
{
  _continue_dmap=d;
  return;
}
void gamedata::change_continue_dmap(short d)
{
  _continue_dmap+=d;
  return;
}


word gamedata::get_maxmagic()
{
  return get_maxcounter(4);
}
void gamedata::set_maxmagic(word m)
{
  set_maxcounter(m, 4);
  return;
}
void gamedata::change_maxmagic(short m)
{
  change_maxcounter(m, 4);
  return;
}

word gamedata::get_magic()
{
  return get_counter(4);
}
void gamedata::set_magic(word m)
{
  set_counter(m, 4);
  return;
}
void gamedata::change_magic(short m)
{
  change_counter(m, 4);
  return;
}

short gamedata::get_dmagic()
{
  return get_dcounter(4);
}
void gamedata::set_dmagic(short d)
{
  set_dcounter(d, 4);
  return;
}
void gamedata::change_dmagic(short d)
{
  change_dcounter(d, 4);
  return;
}

byte gamedata::get_magicdrainrate()
{
  return get_generic(1);
}
void gamedata::set_magicdrainrate(byte r)
{
  set_generic(r, 1);
  return;
}
void gamedata::change_magicdrainrate(short r)
{
  change_generic((char)r, 1);
  return;
}

byte gamedata::get_canslash()
{
  return get_generic(2);
}
void gamedata::set_canslash(byte s)
{
  set_generic(s, 2);
  return;
}
void gamedata::change_canslash(short s)
{
  change_generic(s, 2);
  return;
}

byte gamedata::get_lkeys()
{
	return lvlkeys[dlevel];
}

bool gamedata::get_item(int id)
{
	return item[id];
}

void gamedata::set_item (int id, bool value)
{
#ifdef DEBUG_GD_ITEMS
	if ( !(value == 0 && item[id] ==0 ) )
	al_trace("Setting item id=%i from %i to %i\n", id, item[id], value);
#endif
	item[id]=value;
}

/*** end of gamedata.cpp ***/
