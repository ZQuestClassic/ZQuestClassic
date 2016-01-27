//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  ending.cc
//
//  Ending code for Zelda Classic.
//
//--------------------------------------------------------

#ifndef _ENDING_H_
#define _ENDING_H_

void inc_quest();
void noproc();
void put_triforce();
void putendmsg(const char *s,int x,int y,int speed,void(proc)());
void brick(int x,int y);
void endingpal();
void ending();
#endif
 
