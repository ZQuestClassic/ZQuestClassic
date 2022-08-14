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

#include "base/zdefs.h"
#include <functional>

#define DRIEDLAKE ((tmpscr.flags7 & fWHISTLEWATER) && (whistleclk>=88))

int32_t COMBOPOS_REGION_EXTENDED(int32_t pos, int32_t scr_dx, int32_t scr_dy);
int32_t COMBOPOS_REGION_EXTENDED(int32_t x, int32_t y);
int32_t COMBOX_REGION_EXTENDED(int32_t pos);
int32_t COMBOY_REGION_EXTENDED(int32_t pos);
int32_t COMBOPOS(int32_t x, int32_t y);
int32_t COMBOX(int32_t pos);
int32_t COMBOY(int32_t pos);

rpos_t COMBOPOS_REGION(int32_t x, int32_t y);
int32_t RPOS_TO_POS(rpos_t rpos);
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr_dx, int32_t scr_dy);
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr);
void COMBOXY_REGION(rpos_t rpos, int32_t& out_x, int32_t& out_y);
int32_t COMBOX_REGION(rpos_t rpos);
int32_t COMBOY_REGION(rpos_t rpos);

int32_t mapind(int32_t map, int32_t scr);

extern bool triggered_screen_secrets;

/*
    Z3-style scrolling is implemented via "regions".
    
    Regions:

        - are a NxM set of screens within a single map
        - must be a rectangle (for now, at least)
        - can border other regions or single screens, and supports scrolling
          between them (including side warps)
        - can be 1x1, but this only has significance for maze screens which
          will wrap the player around the screen
        - when moving around a region with >1 screen width or height, the viewport
          will keep the player centered in the screen. When moving close to the region
          edge the camera bounds to the edges
        - tmpscr points to the entrance screen
        - currscr is the screen where the hero currently is
*/

// How large the current region is. If not currently in z3 scrolling mode, this is just the size
// of a single screen.
extern int world_w, world_h;
// The camera offset in the above world-space coordinates. If not currently in z3 scrolling mode,
// this is just 0. Otherwise it attempts to center the hero in the middle of the camera, snapping
// to the region edges.
extern int viewport_x, viewport_y;
extern int viewport_y_offset;
// The screen offset from the region origin that the hero is currently standing in. If not currently
// in z3 scrolling mode, this is just 0.
extern int region_scr_dx, region_scr_dy;
// The screens size of the region that the hero is currently standing in. If not currently
// in z3 scrolling mode, this is just 1.
extern int region_scr_width, region_scr_height;
// Maximum value for 'rpos' in a region. Number of combo positions in a region, minus 1. If not currently
// in z3 scrolling mode, this is just 175.
extern rpos_t region_max_rpos;
// TODO z3
extern int scrolling_maze_scr, scrolling_maze_state;
// TODO z3: this only works in mode '0' and if the scrolling region is 1x1...
// 0 to keep viewport frozen on maze screen and have hero appear to wrap around.
// 1 to keep hero in the center.
extern int scrolling_maze_mode;

int z3_get_region_id(int scr);
void z3_calculate_region(int scr, int& origin_scr, int& region_scr_width, int& region_scr_height, int& region_scr_dx, int& region_scr_dy, int& world_w, int& world_h);
void z3_load_region();
void z3_clear_temporary_screens();
void z3_calculate_viewport(mapscr* scr, int world_w, int world_h, int hero_x, int hero_y, int& viewport_x, int& viewport_y);
void z3_update_viewport();
void z3_update_currscr();
bool edge_of_region(direction dir);
int z3_get_scr_index_for_xy_offset(int x, int y);
int z3_get_scr_index_for_rpos(rpos_t rpos);
mapscr* z3_get_mapscr_for_rpos(rpos_t rpos);
pos_handle z3_get_pos_handle(rpos_t rpos, int layer);
pos_handle z3_get_pos_handle_for_world_xy(int x, int y, int layer);
mapscr* z3_get_scr_for_world_xy(int x, int y);
mapscr* z3_get_mapscr_layer_for_xy_offset(int x, int y, int layer);
void for_every_screen_in_region(const std::function <void (mapscr*, int, unsigned int, unsigned int)>& fn);
int z3_get_origin_scr();
int z3_get_region_relative_dx(int scr);
int z3_get_region_relative_dy(int scr);
bool is_z3_scrolling_mode();
mapscr* get_scr(int map, int screen);
mapscr* get_layer_scr(int map, int screen, int layer);
mapscr* get_layer_scr_for_xy(int x, int y, int layer);

void debugging_box(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void clear_dmap(word i);
void clear_dmaps();
int32_t count_dmaps();
int32_t isdungeon(int32_t dmap = -1, int32_t scr = -1);
bool canPermSecret(int32_t dmap = -1, int32_t scr = -1);
int32_t MAPCOMBO(int32_t x, int32_t y);
int32_t MAPCOMBO(const pos_handle& pos_handle);
int32_t MAPCOMBOzq(int32_t x,int32_t y);
int32_t MAPFFCOMBO(int32_t x,int32_t y);
int32_t MAPCSET(int32_t x,int32_t y);
int32_t MAPFLAG(int32_t x,int32_t y);
int32_t MAPCOMBOFLAG(int32_t x,int32_t y);
int32_t MAPFFCOMBOFLAG(int32_t x,int32_t y);
int32_t COMBOTYPE(int32_t x,int32_t y);
int32_t FFCOMBOTYPE(int32_t x, int32_t y);
int32_t FFORCOMBO(int32_t x, int32_t y);
int32_t FFORCOMBOTYPE(int32_t x, int32_t y);
int32_t FFORCOMBO_L(int32_t layer, int32_t x, int32_t y);
int32_t FFORCOMBOTYPE_L(int32_t layer, int32_t x, int32_t y);
int32_t MAPCOMBO2(int32_t layer,int32_t x,int32_t y);
int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets = false);
int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, rpos_t rpos, bool secrets = false);
int32_t MAPCOMBO3(mapscr *m, int32_t map, int32_t screen, int32_t layer, int32_t pos, bool secrets = false);
int32_t MAPCSET2(int32_t layer,int32_t x,int32_t y);
int32_t MAPFLAG2(int32_t layer,int32_t x,int32_t y);
int32_t MAPCOMBOFLAG2(int32_t layer,int32_t x,int32_t y);
int32_t COMBOTYPE2(int32_t layer,int32_t x,int32_t y);

bool HASFLAG(int32_t flag, int32_t layer, int32_t pos);
bool HASFLAG_ANY(int32_t flag, int32_t pos);

//specific layers 1 to 6 -Z
int32_t MAPCOMBOL(int32_t layer,int32_t x,int32_t y);
int32_t MAPCSETL(int32_t layer,int32_t x,int32_t y);
int32_t MAPFLAGL(int32_t layer,int32_t x,int32_t y);
int32_t COMBOTYPEL(int32_t layer,int32_t x,int32_t y);
int32_t MAPCOMBOFLAGL(int32_t layer,int32_t x,int32_t y);

int32_t getFFCAt(int32_t x, int32_t y);
void eventlog_mapflags();

void setmapflag2(mapscr* scr, int32_t screen, int32_t flag);
void setmapflag(mapscr* scr, int32_t mi2, int32_t flag);
void setmapflag(int32_t mi2, int32_t flag);
void setmapflag(int32_t flag = 32); // 32 = mSPECIALITEM
void unsetmapflag(int32_t mi2, int32_t flag, bool anyflag=false);
void unsetmapflag(int32_t flag = 32,bool anyflag = false);
bool getmapflag(int32_t flag = 32); // 32 = mSPECIALITEM
bool getmapflag(int32_t screen, int32_t flag);

void setxmapflag2(int32_t screen, int32_t flag);
void setxmapflag(int32_t mi2, uint32_t flag);
void setxmapflag(uint32_t flag);
void unsetxmapflag(int32_t mi2, uint32_t flag);
void unsetxmapflag(uint32_t flag);
bool getxmapflag2(int32_t screen_index, uint32_t flag);
bool getxmapflag(int32_t mi2, uint32_t flag);
bool getxmapflag(uint32_t flag);

int32_t WARPCODE(int32_t dmap,int32_t scr,int32_t dw);
void update_combo_cycling();
bool isSVLadder(int32_t x, int32_t y);
bool isSVPlatform(int32_t x, int32_t y);
bool checkSVLadderPlatform(int32_t x, int32_t y);
bool iswater(int32_t combo);
int32_t iswaterex_z3(int32_t combo, int32_t layer, int32_t x, int32_t y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true, bool ShallowCheck = false, bool hero = true);
int32_t iswaterex(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true, bool ShallowCheck = false, bool hero = true);
int32_t iswaterexzq(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true);
bool iswater_type(int32_t type);
bool ispitfall(int32_t combo);
bool ispitfall_type(int32_t type);
bool ispitfall(int32_t x, int32_t y);
int32_t getpitfall(int32_t x, int32_t y);
bool isGrassType(int32_t type);
bool isFlowersType(int32_t type);
bool isBushType(int32_t type);
bool isGenericType(int32_t type);
bool isSlashType(int32_t type);
bool isCuttableNextType(int32_t type);
bool isTouchyType(int32_t type);
bool isCuttableType(int32_t type);
bool isCuttableItemType(int32_t type);
bool isstepable(int32_t combo);                                 //can use ladder on it
bool isHSComboFlagType(int32_t type);
bool isHSGrabbable(newcombo const& cmb);
bool isSwitchHookable(newcombo const& cmb);
rpos_t check_hshot(int32_t layer, int32_t x, int32_t y, bool switchhook);
bool ishookshottable(int32_t bx, int32_t by);
bool ishookshottable(int32_t map, int32_t screen, int32_t bx, int32_t by);
bool hiddenstair(int32_t tmp, bool redraw);                      // tmp = index of tmpscr[]
bool hiddenstair2(mapscr *s, bool redraw);                      
bool remove_screenstatecombos2(mapscr *s, int32_t screen_index, bool do_layers, int32_t what1, int32_t what2);

bool remove_xstatecombos_old(int32_t tmp, byte xflag);
bool remove_xstatecombos_old(int32_t tmp, int32_t mi, byte xflag);
bool remove_xstatecombos2(mapscr *s, int32_t scr, byte xflag);
bool remove_xstatecombos2(mapscr *s, int32_t scr, int32_t mi, byte xflag);
void clear_xstatecombos_old(int32_t tmp);
void clear_xstatecombos_old(int32_t tmp, int32_t mi);
// void clear_xstatecombos2(mapscr *s, int32_t scr);
void clear_xstatecombos2(mapscr *s, int32_t scr, int32_t mi);

bool remove_lockblocks(mapscr* s, int32_t screen_index);
bool remove_bosslockblocks(mapscr* s, int32_t screen_index);
bool remove_chests(mapscr* s, int32_t screen_index);
bool remove_lockedchests(mapscr* s, int32_t screen_index);
bool remove_bosschests(mapscr* s, int32_t screen_index);

bool overheadcombos(mapscr *s);
void delete_fireball_shooter(mapscr *s, int32_t i);
void trigger_secrets_for_screen(int32_t screen, bool high16only=false, int32_t single=-1);
void hidden_entrance(int32_t tmp, bool refresh, bool high16only=false, int32_t single=-1);
void hidden_entrance2(int32_t screen_index, mapscr *s, bool do_layers, bool high16only=false, int32_t single=-1);
void update_freeform_combos();
bool trigger_secrets_if_flag(int32_t x, int32_t y, int32_t flag, bool setflag);
bool hitcombo(int32_t x, int32_t y, int32_t combotype);
bool hitflag(int32_t x, int32_t y, int32_t flagtype);
int32_t nextscr(int32_t dir);
void bombdoor(int32_t x,int32_t y);
void do_scrolling_layer(BITMAP *bmp, int32_t type, int32_t map, int32_t scr, int32_t layer, mapscr* basescr, int32_t x, int32_t y, bool scrolling, int32_t tempscreen);
void do_layer(BITMAP *bmp, int32_t type, int32_t layer, mapscr* basescr, int32_t x, int32_t y, int32_t tempscreen, bool scrolling=false, bool drawprimitives=false);
void do_layer(BITMAP *bmp, int32_t type, int32_t map, int32_t scr, int32_t layer, mapscr* basescr, int32_t x, int32_t y, int32_t tempscreen, bool scrolling = false, bool drawprimitives=false);
void put_walkflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr);
void do_walkflags(BITMAP *dest,mapscr* layer,int32_t x, int32_t y, int32_t tempscreen);
void do_effectflags(BITMAP *dest,mapscr* layer,int32_t x, int32_t y, int32_t tempscreen);
void do_primitives(BITMAP *bmp, int32_t type, mapscr *layer, int32_t x, int32_t y);
void do_script_draws(BITMAP *bmp, mapscr *layer, int32_t x, int32_t y, bool hideLayer7 = false);
void calc_darkroom_combos(int screen, int offx, int offy, bool scrolling = false);
void draw_screen(mapscr* this_screen, bool showhero=true, bool runGeneric = false);
void put_door(BITMAP *dest,int32_t t,int32_t pos,int32_t side,int32_t type,bool redraw,bool even_walls=false);
void over_door(BITMAP *dest,int32_t t, int32_t pos,int32_t side);
void putdoor(BITMAP *dest,int32_t t,int32_t side,int32_t door,bool redraw=true,bool even_walls=false);
void showbombeddoor(BITMAP *dest, int32_t side);
void openshutters();
void load_a_screen_and_layers(int dmap, int map, int screen);
void loadscr(int32_t destdmap, int32_t scr, int32_t ldir, bool overlay=false, bool no_x80_dir=false);
void loadscr2(int32_t tmp,int32_t scr,int32_t);
void loadscr_old(int32_t tmp,int32_t destdmap,int32_t scr,int32_t ldir,bool overlay);
void putscr(BITMAP* dest,int32_t x,int32_t y,mapscr* screen);
void putscrdoors(BITMAP *dest,int32_t x,int32_t y,mapscr* screen);
bool _walkflag(int32_t x,int32_t y,int32_t cnt);
bool _walkflag(int32_t x,int32_t y,int32_t cnt,zfix const& switchblockstate);
bool _effectflag(int32_t x,int32_t y,int32_t cnt, int32_t layer = -1);
bool _walkflag(int32_t x,int32_t y,int32_t cnt, mapscr* m);
bool _walkflag(int32_t x,int32_t y,int32_t cnt, mapscr* m, mapscr* s1, mapscr* s2);
bool _walkflag_layer(int32_t x, int32_t y, int32_t layer=-1, int32_t cnt=1);
bool _walkflag_layer(int32_t x,int32_t y,int32_t cnt, mapscr* m);
bool _effectflag_layer(int32_t x, int32_t y, int32_t layer=-1, int32_t cnt=1);
bool _effectflag_layer(int32_t x,int32_t y,int32_t cnt, mapscr* m);
bool water_walkflag(int32_t x,int32_t y,int32_t cnt);
bool hit_walkflag(int32_t x,int32_t y,int32_t cnt);
void map_bkgsfx(bool on);
void toggle_switches(dword flags, bool entry);
void toggle_switches(dword flags, bool entry, mapscr* m, int screen_index);

//
void doDarkroomCircle(int32_t cx, int32_t cy, byte glowRad,BITMAP* dest=NULL,BITMAP* transdest=NULL);
void doDarkroomCone(int32_t sx, int32_t sy, byte glowRad, int32_t dir, BITMAP* dest=NULL,BITMAP* transdest=NULL);

//extern FONT *lfont;
/****  View Map  ****/
extern int32_t mapres;
bool displayOnMap(int32_t x, int32_t y);
void ViewMap();
int32_t onViewMap();

//extern bool FuckIAlreadyDrewThatAlready[ 7 ];
#endif

/*** end of maps.cc ***/

