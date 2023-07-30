//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  ending.cc
//
//  Ending code for ZQuest Classic.
//
//--------------------------------------------------------

#ifndef _ENDING_H_
#define _ENDING_H_

#include <cstdint>

void inc_quest();
void noproc();
void put_triforce();
void putendmsg(const char *s,int32_t x,int32_t y,int32_t speed,void(proc)());
void brick(int32_t x,int32_t y);
void endingpal();
void ending();
void ending_scripted();
#endif
 
