#ifndef _MAPS_H_
#define _MAPS_H_

#include <optional>
#include <utility>
#include "base/handles.h"
// TODO: extract direction so this can be removed.
#include "base/zdefs.h"

struct newcombo;

#define DRIEDLAKE ((hero_scr->flags7 & fWHISTLEWATER) && (whistleclk>=88))

int32_t COMBOPOS_REGION_EXTENDED(int32_t pos, int32_t scr_dx, int32_t scr_dy);
int32_t COMBOPOS_REGION_EXTENDED(int32_t x, int32_t y);
int32_t COMBOPOS_REGION_EXTENDED_B(int32_t x, int32_t y);
int32_t COMBOX_REGION_EXTENDED(int32_t pos);
int32_t COMBOY_REGION_EXTENDED(int32_t pos);
int32_t COMBOPOS(int32_t x, int32_t y);
int32_t COMBOPOS_B(int32_t x, int32_t y);
int32_t COMBOX(int32_t pos);
int32_t COMBOY(int32_t pos);

// Input must be a valid world coordinate for the current region.
rpos_t COMBOPOS_REGION(int32_t x, int32_t y);
// If input not within the current region, returns rpos_t::None
rpos_t COMBOPOS_REGION_CHECK_BOUNDS(int32_t x, int32_t y);
// `rpos` must be a valid rpos for the current region.
int32_t RPOS_TO_POS(rpos_t rpos);
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr_dx, int32_t scr_dy);
rpos_t POS_TO_RPOS(int32_t pos, int32_t scr);
std::pair<int32_t, int32_t> COMBOXY_REGION(rpos_t rpos);
int32_t COMBOX_REGION(rpos_t rpos);
int32_t COMBOY_REGION(rpos_t rpos);

int32_t mapind(int32_t map, int32_t scr);
bool ffcIsAt(const ffc_handle_t& ffc_handle, int32_t x, int32_t y);

extern bool triggered_screen_secrets;
extern int32_t view_map_show_mode;

/*
    Z3-style scrolling is implemented via "Regions".
    
    Regions:

        - are a NxM set of screens within a single map
        - must be a rectangle (for now, at least)
		- are defined in the DMap Editor by setting an id, but other than grouping screens into the same region
		  this id currently has no purpose in-engine
		- the top-left screen is denoted at the `current_region_origin_screen_index`
		- can be uniquely identified as (dmap, current_region_origin_screen_index)
        - can border other regions or single screens, and supports scrolling
          between them (including side warps)
        - can be 1x1, but this only has significance for maze screens which
          will wrap the player around the screen
		- cannot overlap within the same dmap, though nothing prevents multiple dmaps from grouping
		  screens into regions in overlapping ways
        - when moving around a region with >1 screen width or height, by default the viewport
          will keep the player centered in the screen. When moving close to the region
          edge the camera bounds to the edges. This behavior is modified by `viewport_mode`,
		  which can be modified by scripts via `Viewport->`
        - `tmpscr` points to the entrance screen
        - `heroscr` is the screen index where the hero currently is, and updates as the player moves around. `hero_scr` is that screen object
		- `heroscr` is always a valid screen in the region
*/

// How large the current region is in pixels.
// If not currently in z3 scrolling mode, this is just the size of a single screen (256, 176).
extern int world_w, world_h;
// The "camera" in the above world-space coordinates.
// (viewport.x, viewport.y) is the point in world-space to draw as the top-left corner of the visible screen.
// In region mode, by default x and y are set such that the hero in the middle of the screen when possible, snapping
// to the region edges when not. This behavior is modified by `viewport_mode`.
//
// If not currently in z3 scrolling mode:
//  - x, y is 0
//  - w, h is 256, 176
extern viewport_t viewport;
extern ViewportMode viewport_mode;
// The screen offset from the region origin that the hero is currently standing in. If not currently
// in z3 scrolling mode, this is just 0.
extern int region_scr_dx, region_scr_dy;
// Number of screens in the current region.
extern int region_scr_count;
// Maximum value for 'rpos' in the current region. This is the number of possible combo positions, minus 1.
// If not currently in z3 scrolling mode, this is just 175.
extern rpos_t region_max_rpos;
// Number of unique values for 'rpos' in the current region. This is the number of possible combo positions.
// If not currently in z3 scrolling mode, this is just 176.
extern int region_num_rpos;
// TODO z3
extern int scrolling_maze_scr, scrolling_maze_state;
// TODO z3 ! this only works in mode '0' and if the scrolling region is 1x1...
// 0 to keep viewport frozen on maze screen and have hero appear to wrap around.
// 1 to keep hero in the center.
extern int scrolling_maze_mode;

struct region
{
	int region_id;
	int dmap;
	int origin_screen_index;
	// The screens size of the region that the hero is currently standing in. If not currently
	// in z3 scrolling mode, this is just 1.
	int screen_width, screen_height;
	int screen_count;
	// World coordinates.
	int width, height;
};
extern region current_region, scrolling_region;

int get_region_id(int dmap, int screen);
int get_current_region_id();
bool is_in_current_region(int scr);
bool is_valid_rpos(rpos_t rpos);
void z3_calculate_region(int dmap, int screen, region& region, int& region_scr_dx, int& region_scr_dy);
void z3_load_region(int screen, int dmap = -1);
// Returns a rpos_handle of the top-left position for every valid
// screen (including each layer) for the current region.
// The first item is the first element of the array of rpos_handles,
// and the second item is the number of elements.
std::tuple<const rpos_handle_t*, int> z3_get_current_region_handles();
void z3_clear_temporary_screens();
std::vector<mapscr*> z3_take_temporary_screens();
void z3_calculate_viewport(int dmap, int screen, int world_w, int world_h, int hero_x, int hero_y, viewport_t& viewport);
void z3_update_viewport();
void z3_update_heroscr();
bool edge_of_region(direction dir);
int get_screen_index_for_world_xy(int x, int y);
int get_screen_index_for_rpos(rpos_t rpos);
mapscr* get_screen_for_rpos(rpos_t rpos);
mapscr* get_screen_layer_for_rpos(rpos_t rpos, int layer);
rpos_handle_t get_rpos_handle(rpos_t rpos, int layer);
rpos_handle_t get_rpos_handle_for_world_xy(int x, int y, int layer);
rpos_handle_t get_rpos_handle_for_screen(int screen, int layer, int pos);
rpos_handle_t get_rpos_handle_for_screen(mapscr* scr, int screen, int layer, int pos);
void change_rpos_handle_layer(rpos_handle_t& rpos_handle, int layer);
combined_handle_t get_combined_handle_for_world_xy(int x, int y, int layer);
mapscr* get_screen_for_world_xy(int x, int y);
mapscr* get_screen_layer_for_xy_offset(int x, int y, int layer);
int z3_get_region_relative_dx(int screen);
int z3_get_region_relative_dx(int screen, int origin_screen_index);
int z3_get_region_relative_dy(int screen);
int z3_get_region_relative_dy(int screen, int origin_screen_index);
int get_region_screen_index_offset(int screen);
int get_screen_index_for_region_index_offset(int offset);
mapscr* get_screen_for_region_index_offset(int offset);
bool is_z3_scrolling_mode();
bool is_extended_height_mode();
const mapscr* get_canonical_scr(int map, int screen);
mapscr* get_scr(int map, int screen);
mapscr* get_scr_no_load(int map, int screen);
mapscr* get_layer_scr(int map, int screen, int layer);
mapscr* get_layer_scr_allow_scrolling(int map, int screen, int layer);
mapscr* get_layer_scr_for_xy(int x, int y, int layer);

ffc_handle_t get_ffc(int id);

std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen, int x, int y);
std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen);

void debugging_box(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void clear_dmap(word i);
void clear_dmaps();
int32_t count_dmaps();
int32_t isdungeon(int32_t dmap = -1, int32_t scr = -1);
bool canPermSecret(int32_t dmap, int32_t screen);
int32_t MAPCOMBO(int32_t x, int32_t y);
int32_t MAPCOMBO(const rpos_handle_t& rpos_handle);
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
int32_t MAPCSET2(int32_t layer,int32_t x,int32_t y);
int32_t MAPFLAG2(int32_t layer,int32_t x,int32_t y);
int32_t MAPCOMBOFLAG2(int32_t layer,int32_t x,int32_t y);
int32_t COMBOTYPE2(int32_t layer,int32_t x,int32_t y);

bool HASFLAG(int32_t flag, int32_t layer, rpos_t rpos);
bool HASFLAG_ANY(int32_t flag, rpos_t rpos);

//specific layers 1 to 6 -Z
int32_t MAPCOMBOL(int32_t layer,int32_t x,int32_t y);
int32_t MAPCSETL(int32_t layer,int32_t x,int32_t y);
int32_t MAPFLAGL(int32_t layer,int32_t x,int32_t y);
int32_t COMBOTYPEL(int32_t layer,int32_t x,int32_t y);
int32_t MAPCOMBOFLAGL(int32_t layer,int32_t x,int32_t y);

std::optional<ffc_handle_t> getFFCAt(int32_t x, int32_t y);
void eventlog_mapflags();

void setmapflag(mapscr* scr, int32_t screen, int32_t flag);
void setmapflag(int32_t screen, int32_t flag);
// TODO z3 remove
void setmapflag(int32_t flag);
void setmapflag_mi(int32_t mi2, int32_t flag);
void setmapflag_mi(mapscr* scr, int32_t mi2, int32_t flag);
void unsetmapflag(int32_t mi2, int32_t flag, bool anyflag=false);
void unsetmapflag(int32_t flag = 32,bool anyflag = false);
bool getmapflag(int32_t flag = 32); // 32 = mSPECIALITEM
bool getmapflag(int32_t screen, int32_t flag);

void setxmapflag(int32_t screen, uint32_t flag);
void setxmapflag_mi(int32_t mi2, uint32_t flag);
void unsetxmapflag(int32_t screen, uint32_t flag);
void unsetxmapflag_mi(int32_t mi2, uint32_t flag);
bool getxmapflag(int32_t screen, uint32_t flag);
bool getxmapflag_mi(int32_t mi2, uint32_t flag);

void setxdoor(uint mi2, uint dir, uint ind, bool state = true);
void setxdoor(uint dir, uint ind, bool state = true);
bool getxdoor(uint mi2, uint dir, uint ind);
bool getxdoor(uint dir, uint ind);

void set_doorstate_mi(uint mi,uint dir);
void set_doorstate(uint screen, uint dir);
void set_doorstate(uint dir);
void set_xdoorstate(uint mi,uint dir, uint ind);
void set_xdoorstate(uint dir, uint ind);

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
bool isdamage_type(int32_t type);
bool ispitfall(int32_t x, int32_t y);
int32_t getpitfall(int32_t x, int32_t y);
enum
{
	ICY_BLOCK,
	ICY_PLAYER,
	ICY_MAX
};
bool check_icy(newcombo const& cmb, int type);
int get_icy(int x, int y, int type);
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
bool check_hshot(int32_t layer, int32_t x, int32_t y, bool switchhook, rpos_t *out_rpos = nullptr, ffcdata **out_ffc = nullptr);
bool ishookshottable(int32_t bx, int32_t by);
bool reveal_hidden_stairs(mapscr *s, int32_t screen, bool redraw);
bool remove_screenstatecombos2(mapscr *s, int32_t screen, bool do_layers, int32_t what1, int32_t what2);

bool remove_xstatecombos(mapscr *s, int32_t scr, byte xflag, bool triggers = false);
bool remove_xstatecombos_mi(mapscr *s, int32_t scr, int32_t mi, byte xflag, bool triggers);
void clear_xstatecombos(mapscr *s, int32_t scr, bool triggers = false);
void clear_xstatecombos_mi(mapscr *s, int32_t scr, int32_t mi, bool triggers = false);

bool remove_xdoors(mapscr *s, int32_t scr, uint dir, uint ind, bool triggers = false);
bool remove_xdoors_mi(mapscr *s, int32_t scr, int32_t mi, uint dir, uint ind, bool triggers);
void clear_xdoors(mapscr *s, int32_t scr, bool triggers = false);
void clear_xdoors_mi(mapscr *s, int32_t scr, int32_t mi, bool triggers = false);

bool remove_lockblocks(mapscr* s, int32_t screen);
bool remove_bosslockblocks(mapscr* s, int32_t screen);
bool remove_chests(mapscr* s, int32_t screen);
bool remove_lockedchests(mapscr* s, int32_t screen);
bool remove_bosschests(mapscr* s, int32_t screen);

bool overheadcombos(mapscr *s);
void delete_fireball_shooter(const rpos_handle_t& rpos_handle);

enum TriggerSource {
	Unspecified,
	Singular,
	EnemiesScreenFlag,
	SecretsScreenState,
	Script,
	ItemsSecret,
	GenericCombo,
	LightTrigger,
	SCC,
	CheatTemp,
	CheatPerm,
};
void trigger_secrets_for_screen(TriggerSource source, int32_t screen, bool high16only=false, int32_t single=-1);
void trigger_secrets_for_screen(TriggerSource source, int32_t screen, mapscr *s, bool high16only=false, int32_t single=-1);
void trigger_secrets_for_screen_internal(int32_t screen, mapscr *s, bool do_combo_triggers, bool high16only, int32_t single);

void update_freeform_combos();
void update_slopes();
bool trigger_secrets_if_flag(int32_t x, int32_t y, int32_t flag, bool setflag);
bool triggerfire(int x, int y, bool setflag, bool any, bool strong, bool magic, bool divine);
bool hitcombo(int32_t x, int32_t y, int32_t combotype, byte layers = 0b0000001);
int gethitcombo(int32_t x, int32_t y, int32_t combotype, byte layers = 0b0000001);
bool hitflag(int32_t x, int32_t y, int32_t flagtype, byte layers = 0b0000001);
optional<int> nextscr(int map, int screen, int dir, bool normal);
optional<int> nextscr_mi(int mi, int dir, bool normal);
optional<int> nextscr(int dir, bool normal);
std::pair<int32_t, int32_t> nextscr2(int32_t dir);
void bombdoor(int32_t x,int32_t y);
bool lenscheck(mapscr* basescr, int layer);
void do_scrolling_layer(BITMAP *bmp, int32_t type, const screen_handle_t& screen_handle, int32_t x, int32_t y);
void do_layer_old(BITMAP *bmp, int32_t type, int32_t layer, mapscr* basescr, int32_t x, int32_t y, int32_t tempscreen, bool scrolling=false, bool drawprimitives=false);
void do_layer(BITMAP *bmp, int32_t type, const screen_handle_t& screen_handle, int32_t x, int32_t y, bool drawprimitives=false);
void put_walkflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr);
void do_walkflags(mapscr* layer,int32_t x, int32_t y, int32_t tempscreen);
void do_effectflags(mapscr* layer,int32_t x, int32_t y, int32_t tempscreen);
void do_primitives(BITMAP *bmp, int32_t type, int32_t x, int32_t y);
void do_script_draws(BITMAP *bmp, mapscr *layer, int32_t x, int32_t y, bool hideLayer7 = false);
void calc_darkroom_combos(int screen, int offx, int offy);
void draw_screen(bool showhero=true, bool runGeneric = false);
void put_door(BITMAP *dest,int32_t t,int32_t pos,int32_t side,int32_t type,bool redraw,bool even_walls=false);
void put_door(BITMAP *dest,mapscr* m,int32_t pos,int32_t side,int32_t type,bool redraw,bool even_walls=false);
void over_door(BITMAP *dest,int32_t t, int32_t pos,int32_t side);
void update_door(mapscr* m,int32_t side,int32_t door,bool even_walls=false);
void putdoor(BITMAP *dest,int32_t t,int32_t side,int32_t door,bool redraw=true,bool even_walls=false);
void showbombeddoor(BITMAP *dest, int32_t side);
void openshutters(mapscr* scr, int screen);
void load_a_screen_and_layers(int dmap, int map, int screen, int ldir);
void loadscr(int32_t destdmap, int32_t scr, int32_t ldir, bool overlay=false, bool no_x80_dir=false);
void clear_darkroom_bitmaps();
void loadscr2(int32_t tmp,int32_t scr,int32_t);
void loadscr_old(int32_t tmp,int32_t destdmap,int32_t scr,int32_t ldir,bool overlay=false);
void putscr(BITMAP* dest,int32_t x,int32_t y,mapscr* scr);
void putscrdoors(BITMAP *dest,int32_t x,int32_t y,mapscr* scr);
void putscrdoors(BITMAP *dest,int32_t x,int32_t y);
bool _walkflag_new(const mapscr* s0, const mapscr* s1, const mapscr* s2, zfix_round zx, zfix_round zy, zfix const& switchblockstate, bool is_temp_screens);
bool _walkflag(zfix_round x,zfix_round y,int32_t cnt);
bool _walkflag(zfix_round x,zfix_round y,int32_t cnt,zfix const& switchblockstate);
bool _effectflag(int32_t x,int32_t y,int32_t cnt, int32_t layer = -1, bool notLink = false);
bool _walkflag(zfix_round x,zfix_round y,int32_t cnt, mapscr* m);
bool _walkflag(zfix_round x,zfix_round y,int32_t cnt, mapscr* m, mapscr* s1, mapscr* s2);
bool _walkflag_layer(zfix_round x,zfix_round y, int32_t layer=-1, int32_t cnt=1);
bool _walkflag_layer(zfix_round x,zfix_round y,int32_t cnt, mapscr* m);
bool _walkflag_layer_scrolling(zfix_round zx,zfix_round zy,int32_t cnt, mapscr* m);
bool _effectflag_layer(int32_t x, int32_t y, int32_t layer=-1, int32_t cnt=1, bool notLink = false);
bool _effectflag_layer(int32_t x,int32_t y,int32_t cnt, mapscr* m, bool notLink = false);
bool water_walkflag(int32_t x,int32_t y,int32_t cnt);
bool water_walkflag(int32_t x,int32_t y);
bool hit_walkflag(int32_t x,int32_t y,int32_t cnt);
bool solpush_walkflag(int32_t x, int32_t y, int32_t cnt, solid_object const* ign = nullptr);
void map_bkgsfx(bool on);
void toggle_switches(dword flags, bool entry);
void toggle_switches(dword flags, bool entry, mapscr* m, int screen);
void toggle_gswitches(int32_t state, bool entry);
void toggle_gswitches(int32_t state, bool entry, mapscr* base_scr, int screen);
void toggle_gswitches(bool* states, bool entry, mapscr* base_scr, int screen);
void toggle_gswitches_load(mapscr* base_scr, int screen);
void run_gswitch_timers();
void onload_gswitch_timers();

//

/****  View Map  ****/
extern int32_t mapres;
bool displayOnMap(int32_t x, int32_t y);
void ViewMap();
int32_t onViewMap();

bool is_push(mapscr* m, int32_t pos);

enum class ScreenItemState {
	None,
	WhenKillEnemies,
	WhenTriggerSecrets,
	MustGiveToEnemy,
	CarriedByEnemy,
};

ScreenItemState screen_item_get_state(int screen);
void screen_item_set_state(int screen, ScreenItemState state);
void screen_item_clear_state(int screen);
void screen_item_clear_state();

optional<int32_t> get_combo(int x, int y, int maxlayer, bool ff, std::function<bool(newcombo const&)> proc);

#endif
