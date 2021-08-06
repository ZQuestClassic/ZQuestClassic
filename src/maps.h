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

#define DRIEDLAKE ((tmpscr->flags7 & fWHISTLEWATER) && (whistleclk>=88))
#define COMBOPOS(x,y) (((y)&0xF0)+((x)>>4))
void debugging_box(int x1, int y1, int x2, int y2);
void clear_dmap(word i);
void clear_dmaps();
int count_dmaps();
int isdungeon(int dmap = -1, int scr = -1);
bool canPermSecret(int dmap = -1, int scr = -1);
int MAPCOMBO(int x,int y);
int MAPCOMBOzq(int x,int y);
int MAPFFCOMBO(int x,int y);
int MAPCSET(int x,int y);
int MAPFLAG(int x,int y);
int MAPCOMBOFLAG(int x,int y);
int MAPFFCOMBOFLAG(int x,int y);
int COMBOTYPE(int x,int y);
int FFCOMBOTYPE(int x, int y);
int FFORCOMBO(int x, int y);
int FFORCOMBOTYPE(int x, int y);
int FFORCOMBO_L(int layer, int x, int y);
int FFORCOMBOTYPE_L(int layer, int x, int y);
int MAPCOMBO2(int layer,int x,int y);
int MAPCOMBOX(int pos);
int MAPCOMBOY(int pos);
int MAPCOMBO3(int map, int screen, int layer,int x,int y, bool secrets = false);
int MAPCOMBO3(int map, int screen, int layer,int pos, bool secrets = false);
int MAPCSET2(int layer,int x,int y);
int MAPFLAG2(int layer,int x,int y);
int MAPCOMBOFLAG2(int layer,int x,int y);
int COMBOTYPE2(int layer,int x,int y);

//specific layers 1 to 6 -Z
int MAPCOMBOL(int layer,int x,int y);
int MAPCSETL(int layer,int x,int y);
int MAPFLAGL(int layer,int x,int y);
int COMBOTYPEL(int layer,int x,int y);
int MAPCOMBOFLAGL(int layer,int x,int y);

int getFFCAt(int x, int y);
void eventlog_mapflags();
void setmapflag(int mi2, int flag);
void setmapflag(int flag = 32); // 32 = mBELOW
void unsetmapflag(int mi2, int flag, bool anyflag=false);
void unsetmapflag(int flag = 32);
bool getmapflag(int flag = 32); // 32 = mBELOW
int WARPCODE(int dmap,int scr,int dw);
void update_combo_cycling();
bool isSVLadder(int x, int y);
bool isSVPlatform(int x, int y);
bool checkSVLadderPlatform(int x, int y);
bool iswater(int combo);
int iswaterex(int combo, int map, int screen, int layer, int x, int y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true, bool ShallowCheck = false);
int iswaterexzq(int combo, int map, int screen, int layer, int x, int y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true);
bool iswater_type(int type);
bool ispitfall(int combo);
bool ispitfall_type(int type);
bool ispitfall(int x, int y);
int getpitfall(int x, int y);
bool isGrassType(int type);
bool isFlowersType(int type);
bool isBushType(int type);
bool isGenericType(int type);
bool isSlashType(int type);
bool isCuttableNextType(int type);
bool isTouchyType(int type);
bool isCuttableType(int type);
bool isCuttableItemType(int type);
bool isstepable(int combo);                                 //can use ladder on it
bool ishookshottable(int bx, int by);
bool ishookshottable(int map, int screen, int bx, int by);
bool hiddenstair(int tmp, bool redraw);                      // tmp = index of tmpscr[]
bool hiddenstair2(mapscr *s, bool redraw);                      
bool remove_screenstatecombos2(mapscr *s, mapscr *t, int what1, int what2);
bool remove_lockblocks(int tmp);                // tmp = index of tmpscr[]
bool remove_bosslockblocks(int tmp);            // tmp = index of tmpscr[]
bool remove_chests(int tmp);                    // tmp = index of tmpscr[]
bool remove_lockedchests(int tmp);              // tmp = index of tmpscr[]
bool remove_bosschests(int tmp);                // tmp = index of tmpscr[]
bool overheadcombos(mapscr *s);
void delete_fireball_shooter(mapscr *s, int i);
void hidden_entrance(int tmp,bool refresh, bool high16only=false,int single=-1);
void hidden_entrance2(mapscr *s, mapscr *t, bool high16only=false,int single=-1);
void update_freeform_combos();
bool findentrance(int x, int y, int flag, bool setflag);
bool hitcombo(int x, int y, int combotype);
bool hitflag(int x, int y, int flagtype);
int nextscr(int dir);
void bombdoor(int x,int y);
void do_scrolling_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, bool scrolling, int tempscreen);
void do_layer(BITMAP *bmp, int type, mapscr* layer, int x, int y, int tempscreen, bool scrolling = false, bool drawprimitives=false);
void put_walkflags(BITMAP *dest,int x,int y,int xofs,int yofs, word cmbdat,int lyr);
void do_walkflags(BITMAP *dest,mapscr* layer,int x, int y, int tempscreen);
void do_effectflags(BITMAP *dest,mapscr* layer,int x, int y, int tempscreen);
void do_primitives(BITMAP *bmp, int type, mapscr *layer, int x, int y);
void do_script_draws(BITMAP *bmp, mapscr *layer, int x, int y, bool hideLayer7 = false);
void draw_screen(mapscr* this_screen, bool showlink=true);
/*
  INLINE void do_scrolling_layer(int type, mapscr* layer, int x, int y, bool scrolling, int tempscreen)
  {
  do_scrolling_layer(framebuf, type, layer, x, y, scrolling, tempscreen);
  }

  INLINE void do_layer(int type, mapscr* layer, int x, int y, int tempscreen)
  {
  do_layer(framebuf, type, layer, x, y, tempscreen);
  }
  */
void put_door(BITMAP *dest,int t,int pos,int side,int type,bool redraw,bool even_walls=false);
void over_door(BITMAP *dest,int t, int pos,int side);
void putdoor(BITMAP *dest,int t,int side,int door,bool redraw=true,bool even_walls=false);
void showbombeddoor(BITMAP *dest, int side);
void openshutters();
void loadscr2(int tmp,int scr,int);
void loadscr(int tmp,int destdmap,int scr,int ldir,bool overlay);
void putscr(BITMAP* dest,int x,int y,mapscr* screen);
void putscrdoors(BITMAP *dest,int x,int y,mapscr* screen);
bool _walkflag(int x,int y,int cnt);
bool _effectflag(int x,int y,int cnt, int layer = -1);
bool _walkflag(int x,int y,int cnt, mapscr* m);
bool _walkflag(int x,int y,int cnt, mapscr* m, mapscr* s1, mapscr* s2);
bool _walkflag_layer(int x,int y,int cnt, mapscr* m);
bool _effectflag_layer(int x,int y,int cnt, mapscr* m);
bool water_walkflag(int x,int y,int cnt);
bool hit_walkflag(int x,int y,int cnt);
void map_bkgsfx(bool on);

//extern FONT *lfont;
/****  View Map  ****/
extern int mapres;
bool displayOnMap(int x, int y);
void ViewMap();
int onViewMap();

//extern bool FuckIAlreadyDrewThatAlready[ 7 ];
#endif

/*** end of maps.cc ***/

