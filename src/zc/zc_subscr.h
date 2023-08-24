//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#ifndef _ZC_SUBSCR_H_
#define _ZC_SUBSCR_H_

void put_passive_subscr(BITMAP *dest,int32_t x,int32_t y,bool showtime,int32_t pos2);
void put_topsubscr(int32_t y, bool pulled_down);
void dosubscr();
void markBmap(int32_t dir, int32_t sc);
void markBmap(int32_t dir);
#endif
