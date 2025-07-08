#ifndef MAPS_H_
#define MAPS_H_

#include <optional>
#include <utility>
#include "base/compiler.h"
#include "base/general.h"
#include "base/handles.h"
#include "sprite.h"
#include "zc/zelda.h"

struct newcombo;
struct BITMAP;
class solid_object;

// How large the current region is in pixels.
// If not currently in a scrolling region, this is just the size of a single screen (256, 176).
extern int world_w, world_h;
// The "camera" in the above world-space coordinates.
// (viewport.x, viewport.y) is the point in world-space to draw as the top-left corner of the visible screen.
// In region mode, by default x and y are set such that the hero in the middle of the screen when possible, snapping
// to the region edges when not. This behavior is modified by `viewport_mode`.
//
// If not currently in a scrolling region:
//  - x, y is 0
//  - w, h is 256, 176
extern viewport_t viewport;
extern ViewportMode viewport_mode;
// The screen offset from the region origin that the hero is currently standing in. If not currently
// in a scrolling region, this is just 0.
extern int region_scr_dx, region_scr_dy;
// Number of screens in the current region.
extern int region_scr_count;
// Maximum value for 'rpos' in the current region. This is the number of possible combo positions, minus 1.
// If not currently in a scrolling region, this is just 175.
extern rpos_t region_max_rpos;
// Number of unique values for 'rpos' in the current region. This is the number of possible combo positions.
// If not currently in a scrolling region, this is just 176.
extern int region_num_rpos;

/*
    z3-style scrolling is implemented via "Regions".

	Regions:

        - are a NxM set of screens within a single map
        - even 1x1 (one screen) areas are regions. A region containing a single screen is called
		  a "non-scrolling region", otherwise it is a "scrolling region"
        - must be a rectangle
		- are defined in the Region Editor by assigning each screen an id, but other than
		  grouping screens into the same region
		  this id currently has no purpose in-engine
        - can border other regions or single screens, and supports scrolling between them
		  (including side warps)
        - when moving around a scrolling region, by default the viewport is updated to keep
		  the player drawn in the center of the screen. When moving close to the region edge,
		  the viewport bounds to the edges. This behavior is modified by `viewport_mode`, which
		  can be modified by scripts via `Viewport->`
		- the top-left screen is loaded as `cur_screen` / `origin_scr`
        - `hero_screen` / `hero_scr` is the screen where the hero currently is, and updates as the
		  player moves around.
*/
struct region_t
{
	int region_id;
	int map;
	int origin_screen;
	int origin_screen_x;
	int origin_screen_y;
	// The screens size of the region that the hero is currently standing in. If not currently
	// in a non-scrolling region, this is just 1.
	int screen_width, screen_height;
	int screen_count;
	// World coordinates.
	int width, height;
};
extern region_t cur_region, scrolling_region;

void maps_init_game_vars();

#define DRIEDLAKE ((hero_scr->flags7 & fWHISTLEWATER) && (whistleclk>=88))

ZC_FORCE_INLINE bool is_valid_rpos(rpos_t rpos)
{
	return (int)rpos >= 0 && rpos <= region_max_rpos;
}

ZC_FORCE_INLINE bool is_in_world_bounds(int x, int y)
{
	return x >= 0 && y >= 0 && x < world_w && y < world_h;
}

ZC_FORCE_INLINE int get_region_relative_dx(int screen, int origin_screen)
{
	return screen % 16 - origin_screen % 16;
}
ZC_FORCE_INLINE int get_region_relative_dx(int screen)
{
	return get_region_relative_dx(screen, cur_screen);
}
ZC_FORCE_INLINE int get_region_relative_dy(int screen, int origin_screen)
{
	return screen / 16 - origin_screen / 16;
}
ZC_FORCE_INLINE int get_region_relative_dy(int screen)
{
	return get_region_relative_dy(screen, cur_screen);
}

// `rpos` must be valid for the current region.
ZC_FORCE_INLINE int32_t RPOS_TO_POS(rpos_t rpos)
{
	DCHECK(is_valid_rpos(rpos));
	return static_cast<int32_t>(rpos)%176;
}
ZC_FORCE_INLINE rpos_t POS_TO_RPOS(int32_t pos, int32_t scr_dx, int32_t scr_dy)
{
	// Ignore this invariant when drawing the map, for now.
	// See replay: enigma_of_basilischi_island_basilse_2_of_2.zplay frame 48860 (remove_xstatecombos_mi)
	DCHECK(ViewingMap || (scr_dx >= 0 && scr_dy >= 0 && scr_dx < cur_region.width && scr_dy < cur_region.height));
	DCHECK_RANGE_EXCLUSIVE(pos, 0, 176);
	return static_cast<rpos_t>((scr_dx + scr_dy * cur_region.screen_width)*176 + pos);
}
ZC_FORCE_INLINE rpos_t POS_TO_RPOS(int32_t pos, int32_t screen)
{
	DCHECK_RANGE_EXCLUSIVE(pos, 0, 176);
	return POS_TO_RPOS(pos, get_region_relative_dx(screen), get_region_relative_dy(screen));
}

// Converts world coordinates to a combo pos (0-175). Input must be a valid screen coordinate.
int32_t COMBOPOS(int32_t x, int32_t y);
// Bounded - if input is not within the bounds of a single screen, returns -1.
int32_t COMBOPOS_B(int32_t x, int32_t y);
int32_t COMBOX(int32_t pos);
int32_t COMBOY(int32_t pos);

// Converts world coordinates to a combo rpos. Input must be a valid world coordinate for the current region.
rpos_t COMBOPOS_REGION(int32_t x, int32_t y);
// Bounded - if input is not within the current region, returns rpos_t::None.
rpos_t COMBOPOS_REGION_B(int32_t x, int32_t y);
std::pair<int32_t, int32_t> COMBOXY_REGION(rpos_t rpos);
int32_t COMBOX_REGION(rpos_t rpos);
int32_t COMBOY_REGION(rpos_t rpos);
// Same as COMBOXY_REGION, but not multiplied by 16.
rpos_t COMBOPOS_REGION_INDEX(int32_t x, int32_t y);
std::pair<int32_t, int32_t> COMBOXY_REGION_INDEX(rpos_t rpos);

int32_t mapind(int32_t map, int32_t scr);
bool ffcIsAt(const ffc_handle_t& ffc_handle, int32_t x, int32_t y);

extern int32_t view_map_show_mode;

struct maze_state_t {
	bool active;
	bool lost;
	bool can_get_lost;
	bool loopy;
	mapscr* scr;
	int exit_screen;
	byte transition_wipe;
	int last_check_herox;
	int last_check_heroy;
	direction enter_dir = dir_invalid;
};

extern maze_state_t maze_state;

// Holds the last solved screen maze, only used within a scrolling region to remember
// that a maze was solved.
extern int scrolling_maze_last_solved_screen;

int get_region_id(int map, int screen);
int get_current_region_id();
bool is_in_current_region(int map, int screen);
bool is_in_current_region(int screen);
bool is_in_current_region(mapscr* scr);
bool is_in_scrolling_region(int screen);
void calculate_region(int map, int screen, region_t& region, int& region_scr_dx, int& region_scr_dy);
void load_region(int dmap, int screen);
// Returns a rpos_handle of the top-left position for every valid
// screen (including each layer) for the current region.
// The first item is the first element of the array of rpos_handles,
// and the second item is the number of elements.
std::tuple<const rpos_handle_t*, int> get_current_region_handles();
std::tuple<const rpos_handle_t*, int> get_current_region_handles(mapscr* scr);
void mark_current_region_handles_dirty();
void clear_temporary_screens();
std::vector<mapscr*> take_temporary_scrs();
void calculate_viewport(viewport_t& viewport, int dmap, int screen, int world_w, int world_h, int x, int y);
sprite* get_viewport_sprite();
void set_viewport_sprite(sprite* spr);
void update_viewport();
void update_heroscr();
mapscr* determine_hero_screen_from_coords();
bool edge_of_region(direction dir);
int get_screen_for_world_xy(int x, int y);
int get_screen_for_rpos(rpos_t rpos);
mapscr* get_scr_for_rpos(rpos_t rpos);
mapscr* get_scr_for_rpos_layer(rpos_t rpos, int layer);
rpos_handle_t get_rpos_handle(rpos_t rpos, int layer);
rpos_handle_t get_rpos_handle_for_world_xy(int x, int y, int layer);
rpos_handle_t get_rpos_handle_for_screen(int screen, int layer, int pos);
rpos_handle_t get_rpos_handle_for_scr(mapscr* scr, int layer, int pos);
void change_rpos_handle_layer(rpos_handle_t& rpos_handle, int layer);
combined_handle_t get_combined_handle_for_world_xy(int x, int y, int layer);
mapscr* get_scr_for_world_xy(int x, int y);
mapscr* get_scr_for_world_xy_layer(int x, int y, int layer);


int get_region_screen_offset(int screen);
int get_screen_for_region_index_offset(int offset);
mapscr* get_scr_for_region_index_offset(int offset);
bool is_in_scrolling_region();
bool is_extended_height_mode();
mapscr* get_scr(int map, int screen);
mapscr* get_scr(int screen);
mapscr* get_scr_maybe(int map, int screen);
mapscr* get_scr_layer(int map, int screen, int layer);
mapscr* get_scr_layer(int screen, int layer);
mapscr* get_scr_layer_valid(int screen, int layer);

mapscr* get_scr_current_region_dir(int screen, direction dir);

ffc_handle_t get_ffc_handle(ffc_id_t id);

std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen, int x, int y);
std::pair<int32_t, int32_t> translate_screen_coordinates_to_world(int screen);

void debugging_box(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void clear_dmap(word i);
void clear_dmaps();
int32_t count_dmaps();
int32_t isdungeon();
int32_t isdungeon(int32_t screen);
int32_t isdungeon(int32_t dmap, int32_t screen);
bool canPermSecret(int32_t dmap, int32_t screen);
int32_t MAPCOMBO(int32_t x, int32_t y);
int32_t MAPCOMBO(const rpos_handle_t& rpos_handle);
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
void handle_region_load_trigger();
std::optional<mapscr> load_temp_mapscr_and_apply_secrets(int32_t map, int32_t screen, int32_t layer, bool secrets, bool secrets_do_replay_comment = true);
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

void setmapflag(mapscr* scr, int32_t flag);
void setmapflag_homescr(int32_t flag);
void setmapflag_mi(int32_t mi, int32_t flag);
void setmapflag_mi(mapscr* scr, int32_t mi, int32_t flag);
void unsetmapflag_mi(mapscr* scr, int32_t mi, int32_t flag, bool anyflag=false);
void unsetmapflag_mi(int32_t mi, int32_t flag, bool anyflag=false);
void unsetmapflag_home(int32_t flag = 32,bool anyflag = false);
void unsetmapflag(mapscr* scr, int32_t flag, bool anyflag = false);
bool getmapflag(int32_t screen, int32_t flag);
bool getmapflag(mapscr* scr, int32_t flag);

void setxmapflag(int32_t screen, uint32_t flag);
void setxmapflag_mi(int32_t mi, uint32_t flag);
void unsetxmapflag(int32_t screen, uint32_t flag);
void unsetxmapflag_mi(int32_t mi, uint32_t flag);
bool getxmapflag(int32_t screen, uint32_t flag);
bool getxmapflag_mi(int32_t mi, uint32_t flag);

void setxdoor_mi(uint mi, uint dir, uint ind, bool state = true);
bool getxdoor_mi(uint mi, uint dir, uint ind);
bool getxdoor(int32_t screen, uint dir, uint ind);

void set_doorstate_mi(uint mi,uint dir);
void set_doorstate(uint screen, uint dir);
void set_xdoorstate_mi(uint mi,uint dir, uint ind, bool state = true);
void set_xdoorstate(int32_t screen,uint dir, uint ind, bool state = true);

int32_t WARPCODE(int32_t dmap,int32_t screen,int32_t dw);
void update_combo_cycling();
bool isSVLadder(int32_t x, int32_t y);
bool isSVPlatform(int32_t x, int32_t y);
bool checkSVLadderPlatform(int32_t x, int32_t y);
bool iswater(int32_t combo);
int32_t iswaterex_z3(int32_t combo, int32_t layer, int32_t x, int32_t y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true, bool ShallowCheck = false, bool hero = true, optional<combined_handle_t>* out_handle = nullptr);
int32_t iswaterex(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true, bool ShallowCheck = false, bool hero = true, optional<combined_handle_t>* out_handle = nullptr);
int32_t iswaterexzq(int32_t combo, int32_t map, int32_t screen, int32_t layer, int32_t x, int32_t y, bool secrets = true, bool fullcheck = false, bool LayerCheck = true);
bool iswater_type(int32_t type);
bool ispitfall(int32_t combo);
bool ispitfall_type(int32_t type);
bool isdamage_type(int32_t type);
bool ispitfall(int32_t x, int32_t y);
int32_t getpitfall(int32_t x, int32_t y);
optional<combined_handle_t> get_pitfall_handle(int32_t x, int32_t y);
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

bool remove_xstatecombos(const std::array<screen_handle_t, 7>& screen_handles, byte xflag, bool triggers = false);
bool remove_xstatecombos_mi(const std::array<screen_handle_t, 7>& screen_handles, int32_t mi, byte xflag, bool triggers);
void clear_xstatecombos(const std::array<screen_handle_t, 7>& screen_handles, bool triggers = false);
void clear_xstatecombos_mi(const std::array<screen_handle_t, 7>& screen_handles, int32_t mi, bool triggers = false);

bool remove_xdoors(const std::array<screen_handle_t, 7>& screen_handles, uint dir, uint ind, bool triggers = false);
bool remove_xdoors_mi(const std::array<screen_handle_t, 7>& screen_handles, int32_t mi, uint dir, uint ind, bool triggers);
void clear_xdoors(const std::array<screen_handle_t, 7>& screen_handles, bool triggers = false);
void clear_xdoors_mi(const std::array<screen_handle_t, 7>& screen_handles, int32_t mi, bool triggers = false);

std::array<screen_handle_t, 7> create_screen_handles_one(mapscr* base_scr);
std::array<screen_handle_t, 7> create_screen_handles(mapscr* base_scr);

bool remove_screenstatecombos2(const std::array<screen_handle_t, 7>& screen_handles, bool do_layers, int32_t what1, int32_t what2);
bool remove_lockblocks(const std::array<screen_handle_t, 7>& screen_handles);
bool remove_bosslockblocks(const std::array<screen_handle_t, 7>& screen_handles);
bool remove_chests(const std::array<screen_handle_t, 7>& screen_handles);
bool remove_lockedchests(const std::array<screen_handle_t, 7>& screen_handles);
bool remove_bosschests(const std::array<screen_handle_t, 7>& screen_handles);

void delete_fireball_shooter(const rpos_handle_t& rpos_handle);

enum class TriggerSource {
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
void trigger_secrets_for_screen(TriggerSource source, mapscr* scr, bool high16only=false, int32_t single=-1);
void trigger_secrets_for_screen(TriggerSource source, int32_t screen, bool high16only=false, int32_t single=-1);
void trigger_secrets_for_screen_internal(const std::array<screen_handle_t, 7>& screen_handles, bool from_active_screen, bool high16only, int32_t single, bool do_replay_comment);

void update_freeform_combos();
void update_slopes();
bool trigger_secrets_if_flag(int32_t x, int32_t y, int32_t flag, bool setflag);
bool hitcombo(int32_t x, int32_t y, int32_t combotype, byte layers = 0b0000001);
int gethitcombo(int32_t x, int32_t y, int32_t combotype, byte layers = 0b0000001);
bool hitflag(int32_t x, int32_t y, int32_t flagtype, byte layers = 0b0000001);
optional<int> nextscr(int screen, int dir);
optional<int> nextscr_mi(int mi, int dir);
std::pair<int32_t, int32_t> nextscr2(int32_t dir);
std::pair<int32_t, int32_t> nextscr2(int m, int s, int32_t dir);
void bombdoor(int32_t x,int32_t y);
bool lenscheck(mapscr* scr, int layer);
void do_ffc_layer(BITMAP* bmp, int32_t layer, const screen_handle_t& screen_handle, int32_t x, int32_t y);
void do_scrolling_layer(BITMAP *bmp, int32_t type, const screen_handle_t& screen_handle, int32_t x, int32_t y);
void do_layer(BITMAP *bmp, int32_t type, const screen_handle_t& screen_handle, int32_t x, int32_t y);
void do_layer_primitives(BITMAP *bmp, int32_t layer);
void put_walkflags(BITMAP *dest,int32_t x,int32_t y,int32_t xofs,int32_t yofs, word cmbdat,int32_t lyr);
void do_walkflags(const std::array<screen_handle_t, 7>& screen_handles ,int32_t x, int32_t y);
void do_walkflags(int32_t x, int32_t y);
void do_effectflags(mapscr* scr, int32_t x, int32_t y);
void do_primitives(BITMAP *bmp, int32_t type);
void do_primitives(BITMAP *bmp, int32_t type, int32_t x, int32_t y);
void do_script_draws(BITMAP *bmp, mapscr *scr, int32_t x, int32_t y, bool hideLayer7 = false);
void calc_darkroom_combos(mapscr* scr, int offx, int offy);
void calc_darkroom_ffcs(mapscr* scr, int offx, int offy);
void draw_screen(bool showhero=true, bool runGeneric = false);
void put_door(mapscr* scr, BITMAP *dest, int32_t pos, int32_t side, int32_t type, bool redraw, bool even_walls=false);
void update_door(mapscr* scr, int32_t side, int32_t door, bool even_walls=false);
void putdoor(mapscr* scr, BITMAP *dest, int32_t side, int32_t door, bool redraw=true, bool even_walls=false);
void showbombeddoor(mapscr* scr, BITMAP *dest, int32_t side, int32_t offx, int32_t offy);
void openshutters(mapscr* scr);
void loadscr(int32_t destdmap, int32_t screen, int32_t ldir, bool origin_screen_overlay=false, bool no_x80_dir=false);
void clear_darkroom_bitmaps();
bool is_dark(const mapscr* scr);
bool scrolling_is_dark(const mapscr* scr);
bool is_any_dark();
std::array<mapscr, 7> loadscr2(int32_t screen);
void loadscr_old(int32_t destdmap, int32_t screen, int32_t ldir, bool overlay);
void putscr(mapscr* scr, BITMAP* dest, int32_t x, int32_t y);
void putscrdoors(mapscr* scr, BITMAP *dest, int32_t x, int32_t y);
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
void toggle_switches(dword flags, bool entry, const screen_handles_t& screen_handles);
void toggle_gswitches(int32_t state, bool entry);
void toggle_gswitches(int32_t state, bool entry, const screen_handles_t& screen_handles);
void toggle_gswitches(bool* states, bool entry, const screen_handles_t& screen_handles);
void toggle_gswitches_load(const screen_handles_t& screen_handles);
void run_gswitch_timers();
void onload_gswitch_timers();

//

/****  View Map  ****/
extern int32_t mapres;
bool displayOnMap(int32_t x, int32_t y);
void ViewMap();
int32_t onViewMap();

bool is_push(mapscr* m, int32_t pos);

enum class ScreenItemState
{
	None,
	WhenKillEnemies,
	WhenTriggerSecrets,
	MustGiveToEnemy,
	CarriedByEnemy,
};

struct screen_state_t
{
	ScreenItemState item_state;
	bool loaded_enemies;
	bool did_enemy_secret;
	bool triggered_secrets;
	int open_doors;
};

std::map<int, screen_state_t>& get_screen_states();
screen_state_t& get_screen_state(int screen);
void clear_screen_states();
void screen_item_set_state(int screen, ScreenItemState state);

void mark_visited(int screen);

#endif
