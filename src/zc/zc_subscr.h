//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#ifndef _ZC_SUBSCR_H_
#define _ZC_SUBSCR_H_

struct miscQdata;

void put_topsubscr(miscQdata *misc, int32_t y, bool pulled_down);
void dosubscr(miscQdata *misc);
void markBmap(int32_t dir, int32_t sc);
void markBmap(int32_t dir);
#endif
