//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  maps.cc
//
//  Map and screen scrolling stuff for zelda.cc
//
//--------------------------------------------------------

#ifndef _MAPS_H_
#define _MAPS_H_
#include "zdefs.h"

void clear_dmap(byte i);
void clear_dmaps();
int count_dmaps();
int isdungeon();
int MAPDATA(int x,int y);
int MAPCSET(int x,int y);
int MAPFLAG(int x,int y);
int COMBOTYPE(int x,int y);
int MAPDATA2(int layer,int x,int y);
int MAPCSET2(int layer,int x,int y);
int MAPFLAG2(int layer,int x,int y);
int COMBOTYPE2(int layer,int x,int y);
void setmapflag();
void unsetmapflag();
bool getmapflag();
void setmapflag(int flag);
void unsetmapflag(int flag);
bool getmapflag(int flag);
int WARPCODE(int dmap,int scr,int dw);
void update_combo_cycling();
bool iswater(int combo);
bool iswater_type(int type);
bool isstepable(int combo);                                 //can use ladder on it
bool hiddenstair(int tmp,bool redraw);                      // tmp = index of tmpscr[]
bool remove_lockblocks(int tmp,bool redraw);                // tmp = index of tmpscr[]
bool remove_bosslockblocks(int tmp,bool redraw);            // tmp = index of tmpscr[]
bool overheadcombos(mapscr *s);
void delete_fireball_shooter(mapscr *s, int i);
void hidden_entrance(int tmp,bool refresh,bool high16only=false);
bool findentrance(int x, int y, int flag, bool setflag);
bool hitcombo(int x, int y, int combotype);
bool hitflag(int x, int y, int flagtype);
int nextscr(int dir);
void bombdoor(int x,int y);
void do_scrolling_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, bool scrolling, int tempscreen);
void do_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, int tempscreen);
void draw_screen(mapscr* layer1, mapscr* layer2, int x1, int y1, int x2, int y2);

inline void draw_screen(mapscr* layer, int x, int y)
{
  draw_screen(layer, NULL, x, y, 0, 0);
}

void put_door(int t,int pos,int side,int type,bool redraw);
void over_door(int t, int pos,int side);
void putdoor(int t,int side,int door,bool redraw=true);
void showbombeddoor(int side);
void openshutters();
void loadscr(int tmp,int scr,int ldir);
void putscr(BITMAP* dest,int x,int y,mapscr* screen);
bool _walkflag(int x,int y,int cnt);
bool water_walkflag(int x,int y,int cnt);
bool hit_walkflag(int x,int y,int cnt);
void map_bkgsfx();

//extern FONT *lfont;
/****  View Map  ****/
extern int mapres;
void ViewMap();
int onViewMap();
#endif

/*** end of maps.cc ***/
