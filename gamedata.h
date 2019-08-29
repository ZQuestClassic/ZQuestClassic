//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zelda.h
//
//  Definitions, function prototypes, etc. for zelda.cc
//
//--------------------------------------------------------

#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include "zdefs.h"
char *get_gamedata_name(gamedata *g);
void set_gamedata_name(gamedata *g, char *n);

byte get_gamedata_quest(gamedata *g);
void set_gamedata_quest(gamedata *g, byte q);
void change_gamedata_quest(gamedata *g, short q);

word get_gamedata_counter(gamedata *g, byte c);
void set_gamedata_counter(gamedata *g, word change, byte c);
void change_gamedata_counter(gamedata *g, short change, byte c);

word get_gamedata_maxcounter(gamedata *g, byte c);
void set_gamedata_maxcounter(gamedata *g, word change, byte c);
void change_gamedata_maxcounter(gamedata *g, short change, byte c);

short get_gamedata_dcounter(gamedata *g, byte c);
void set_gamedata_dcounter(gamedata *g, short change, byte c);
void change_gamedata_dcounter(gamedata *g, short change, byte c);

word get_gamedata_life(gamedata *g);
void set_gamedata_life(gamedata *g, word l);
void change_gamedata_life(gamedata *g, short l);

word get_gamedata_maxlife(gamedata *g);
void set_gamedata_maxlife(gamedata *g, word m);
void change_gamedata_maxlife(gamedata *g, short m);

short get_gamedata_drupy(gamedata *g);
void set_gamedata_drupy(gamedata *g, short d);
void change_gamedata_drupy(gamedata *g, short d);

word get_gamedata_rupies(gamedata *g);
void set_gamedata_rupies(gamedata *g, word r);
void change_gamedata_rupies(gamedata *g, short r);

word get_gamedata_maxarrows(gamedata *g);
void set_gamedata_maxarrows(gamedata *g, word a);
void change_gamedata_maxarrows(gamedata *g, short a);

word get_gamedata_arrows(gamedata *g);
void set_gamedata_arrows(gamedata *g, word a);
void change_gamedata_arrows(gamedata *g, short a);

word get_gamedata_deaths(gamedata *g);
void set_gamedata_deaths(gamedata *g, word d);
void change_gamedata_deaths(gamedata *g, short d);

byte get_gamedata_keys(gamedata *g);
void set_gamedata_keys(gamedata *g, byte k);
void change_gamedata_keys(gamedata *g, short k);

byte get_gamedata_bombs(gamedata *g);
void set_gamedata_bombs(gamedata *g, byte k);
void change_gamedata_bombs(gamedata *g, short k);

byte get_gamedata_maxbombs(gamedata *g);
void set_gamedata_maxbombs(gamedata *g, byte b);
void change_gamedata_maxbombs(gamedata *g, short b);

byte get_gamedata_sbombs(gamedata *g);
void set_gamedata_sbombs(gamedata *g, byte k);
void change_gamedata_sbombs(gamedata *g, short k);

byte get_gamedata_wlevel(gamedata *g);
void set_gamedata_wlevel(gamedata *g, byte l);
void change_gamedata_wlevel(gamedata *g, short l);

byte get_gamedata_cheat(gamedata *g);
void set_gamedata_cheat(gamedata *g, byte c);
void change_gamedata_cheat(gamedata *g, short c);

byte get_gamedata_hasplayed(gamedata *g);
void set_gamedata_hasplayed(gamedata *g, byte p);
void change_gamedata_hasplayed(gamedata *g, short p);

dword get_gamedata_time(gamedata *g);
void set_gamedata_time(gamedata *g, dword t);
void change_gamedata_time(gamedata *g, long long t);

byte get_gamedata_timevalid(gamedata *g);
void set_gamedata_timevalid(gamedata *g, byte t);
void change_gamedata_timevalid(gamedata *g, short t);

byte get_gamedata_HCpieces(gamedata *g);
void set_gamedata_HCpieces(gamedata *g, byte p);
void change_gamedata_HCpieces(gamedata *g, short p);

byte get_gamedata_continue_scrn(gamedata *g);
void set_gamedata_continue_scrn(gamedata *g, byte s);
void change_gamedata_continue_scrn(gamedata *g, short s);

byte get_gamedata_continue_dmap(gamedata *g);
void set_gamedata_continue_dmap(gamedata *g, byte d);
void change_gamedata_continue_dmap(gamedata *g, short d);

word get_gamedata_maxmagic(gamedata *g);
void set_gamedata_maxmagic(gamedata *g, word m);
void change_gamedata_maxmagic(gamedata *g, short m);

word get_gamedata_magic(gamedata *g);
void set_gamedata_magic(gamedata *g, word m);
void change_gamedata_magic(gamedata *g, short m);

short get_gamedata_dmagic(gamedata *g);
void set_gamedata_dmagic(gamedata *g, short d);
void change_gamedata_dmagic(gamedata *g, short d);

byte get_gamedata_magicdrainrate(gamedata *g);
void set_gamedata_magicdrainrate(gamedata *g, byte r);
void change_gamedata_magicdrainrate(gamedata *g, short r);

byte get_gamedata_canslash(gamedata *g);
void set_gamedata_canslash(gamedata *g, byte s);
void change_gamedata_canslash(gamedata *g, short s);

short get_gamedata_generic(gamedata *g, byte c);
void set_gamedata_generic(gamedata *g, byte change, byte c);
void change_gamedata_generic(gamedata *g, short change, byte c);

#endif
/*** end of gamedata.h ***/
