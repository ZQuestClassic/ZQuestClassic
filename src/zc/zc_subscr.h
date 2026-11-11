#pragma once

void put_passive_subscr(BITMAP *dest,int32_t x,int32_t y,bool showtime,int32_t pos2);
void put_topsubscr(int32_t y, bool pulled_down);
void dosubscr(bool map_subscr);
void markBmap(int32_t dir, int32_t sc, int32_t dmap = -1);
void markBmap();
