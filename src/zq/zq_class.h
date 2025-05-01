#ifndef ZQ_CLASS_H_
#define ZQ_CLASS_H_

#include "base/zdefs.h"
#include "base/mapscr.h"
#include "base/containers.h"
#include <stdio.h>
#include <string_view>
#include <deque>
#include <memory>
#include <stack>
#include <array>

struct ZCSubscreen;

int32_t COMBOPOS(int32_t x, int32_t y);
int32_t COMBOPOS_B(int32_t x, int32_t y);
int32_t COMBOX(int32_t pos);
int32_t COMBOY(int32_t pos);

struct MapCursor
{
	int map;
	// The selected screen.
	// Always in bounds of the square starting at `viewscr` of size `size`.
	int screen;
	// The top-left screen of the currently visible screens.
	int viewscr;
	// The number of screens across currently visible. 1x1, 2x2, etc.
	int size = 1;

	bool operator==(const MapCursor&) const = default;

	void setScreen(int new_screen)
	{
		if (new_screen >= MAPSCRS)
			return;

		screen = new_screen;
		adjustViewScr();
	}

	void setSize(int new_size)
	{
		if (size == new_size)
			return;

		size = new_size;
		adjustViewScr();
	}

private:
	// Modify `viewscr` just enough to keep `screen` in bounds.
	void adjustViewScr()
	{
		int cx = screen % 16;
		int cy = screen / 16;
		int vx = viewscr % 16;
		int vy = viewscr / 16;

		if (cx < vx)
			vx = cx;
		else if (cx >= vx + size)
			vx = cx - size + 1;
		if (cy < vy)
			vy = cy;
		else if (cy >= vy + size)
			vy = cy - size + 1;

		viewscr = vx + vy * 16;
	}
};

struct ComboPosition
{
	int x, y;
	bool operator==(const ComboPosition&) const = default;
	ComboPosition operator+ (const ComboPosition & first) const
	{
		return {x + first.x, y + first.y};
	}
	ComboPosition operator+ (int pos) const
	{
		return {x + (pos%16), y + (pos/16)};
	}
	bool is_valid(const MapCursor& cursor)
	{
		int vx = cursor.viewscr % 16;
		int vy = cursor.viewscr / 16;
		int num_screens_width = std::min(cursor.size, 16 - vx);
		int max_height = vx <= 7 ? 9 : 8;
		int num_screens_height = std::min(cursor.size, max_height - vy);
		return x >= 0 && y >= 0 && x < num_screens_width * 16 && y < num_screens_height * 11;
	}
	int truncate() const
	{
		return x % 16 + (y % 11) * 16;
	}
	int screen_offset() const
	{
		return x / 16 + (y / 11) * 16;
	}
};

void set_preview_mode(int32_t prv);

class user_input_command
{
public:
    // The map, screen, and zoom settings that the user was on when command was created.
	MapCursor cursor;

    virtual void execute() = 0;
    virtual void undo() = 0;
    // Rough estimate of how much memory retained by the command, in terms of sizeof(mapscr).
    int size() { return 0; }
};

class list_command : public user_input_command
{
public:
    std::vector<std::shared_ptr<user_input_command>> commands;

    void execute();
    void undo();
    int size();
};

class set_combo_command : public user_input_command
{
public:
    int map, scr, pos, combo, cset;
    int prev_combo, prev_cset;

    void execute();
    void undo();
};

class set_ffc_command : public user_input_command
{
public:
	struct data_t {
		zfix x, y, vx, vy, ax, ay;
		int data, cset, delay, link, script;
		int tw, th;
		int ew, eh;
		ffc_flags flags;
		std::array<int, 8> initd;
		byte layer;
		bool operator==(data_t const&) const = default;
	};
	static data_t create_data(const ffcdata& ffc);

    int map, scr, i;
	data_t data, prev_data;

    void execute();
    void undo();
};

class set_flag_command : public user_input_command
{
public:
    int map, scr, pos, flag;
    int prev_flag;

    void execute();
    void undo();
};

class set_door_command : public user_input_command
{
public:
    int side, door;
    int prev_door;

    void execute();
    void undo();
};

class set_dcs_command : public user_input_command
{
public:
    int dcs;
    int prev_dcs;

    void execute();
    void undo();
};

enum PasteCommandType {
    ScreenAll,
    ScreenAllToEveryScreen,
    ScreenData,
    ScreenDoors,
    ScreenEnemies,
    ScreenFFCombos,
    ScreenGuy,
    ScreenLayers,
    ScreenPalette,
    ScreenPartial,
    ScreenPartialToEveryScreen,
    ScreenRoom,
    ScreenSecretCombos,
    ScreenUnderCombo,
    ScreenWarpLocations,
    ScreenWarps,
};

class paste_screen_command : public user_input_command
{
public:
    PasteCommandType type;
	int screen_index;
    std::shared_ptr<mapscr> screen;
    std::vector<std::shared_ptr<mapscr>> prev_screens;

    void execute();
    void undo();
    int size();

private:
    void perform(mapscr* to);
};

class set_screen_command : public user_input_command
{
public:
	int screen_index;
    std::shared_ptr<mapscr> screen;
    std::shared_ptr<mapscr> prev_screen;

    void execute();
    void undo();
    int size();
};

void reset_dmap(int32_t index);
void truncate_dmap_title(std::string& title);
bool setMapCount2(int32_t c);
class zmap
{
    mapscr *screens;
	MapCursor cursor;
    int32_t copymap;
    int32_t copyscr;
	optional<int32_t> warpbackmap, warpbackscreen;
    int32_t copyffc;
    uint8_t scrpos[MAXMAPS+1];
	uint8_t scrview[MAXMAPS+1];
	
	bounded_map<dword,int32_t> copyscrdata;
	
    mapscr copymapscr;
    mapscr prvscr; //NEW
    mapscr prvlayers[6];
    std::deque<std::shared_ptr<user_input_command>> undo_stack;
    std::stack<std::shared_ptr<user_input_command>> redo_stack;
    //int32_t prv_mode; //NEW
    int32_t prv_cmbcycle, prv_map, prv_scr, prv_freeze, prv_advance, prv_time; //NEW
    bool can_paste;
    // A screen which uses the current screen as a layer
    int32_t layer_target_map, layer_target_scr, layer_target_multiple;

	bool regions_dirty = true;
	region_ids_t current_map_region_ids;
	std::vector<region_description> region_descriptions;

public:

    zmap();

	void clear();
	void force_refr_pointer();
    bool CanUndo();
    bool CanRedo();
    bool CanPaste();
    int32_t  CopyScr();
    int32_t  getCopyFFC();
    void setCopyFFC(int32_t n);
    set_ffc_command::data_t getCopyFFCData();

    // Undo/Redo
    void StartListCommand();
    void FinishListCommand();
    void RevokeListCommand();
	bool InListCommand() const;
    void ExecuteCommand(std::shared_ptr<user_input_command> command, bool skip_execute = false);
    void UndoCommand();
    void RedoCommand();
    void ClearCommandHistory();
    void CapCommandHistory();
	void DoSetComboCommand(ComboPosition pos, int combo, int cset);
    void DoSetComboCommand(int map, int scr, int pos, int combo, int cset);
	void DoSetFFCCommand(int map, int scr, int i, set_ffc_command::data_t data);
	void DoSetFlagCommand(ComboPosition pos, int flag);
    void DoSetFlagCommand(int map, int scr, int pos, int flag);
    void DoPutDoorCommand(int side, int door, bool force = false);
    void DoSetDoorCommand(int scr, int side, int door);
    void DoSetDCSCommand(int dcs);
    void DoPasteScreenCommand(PasteCommandType type, int screen = -1);
    void DoClearScreenCommand(int scr);
    void DoTemplateCommand(int floorcombo, int floorcset, int scr);

    void Copy(int scr);
    void CopyFFC(int32_t screen, int32_t n);
    void Paste(const mapscr& copymapscr, int screen);
    void PasteAll(const mapscr& copymapscr, int screen);
    void PasteToAll(const mapscr& copymapscr);
    void PasteAllToAll(const mapscr& copymapscr);
    void PasteUnderCombo(const mapscr& copymapscr, int screen);
    void PasteSecretCombos(const mapscr& copymapscr, int screen);
    void PasteFFCombos(mapscr& copymapscr, int screen);
    void PasteWarps(const mapscr& copymapscr, int screen);
    void PasteScreenData(const mapscr& copymapscr, int screen);
    void PasteWarpLocations(const mapscr& copymapscr, int screen);
    void PasteDoors(const mapscr& copymapscr, int screen);
    void PasteLayers(const mapscr& copymapscr, int screen);
    void PasteRoom(const mapscr& copymapscr, int screen);
    void PasteGuy(const mapscr& copymapscr, int screen);
    void PastePalette(const mapscr& copymapscr, int screen);
    void PasteEnemies(const mapscr& copymapscr, int screen);
    void update_combo_cycling();
    void update_freeform_combos();
    int32_t getMapCount();
    bool isDungeon(int32_t scr);
    bool isstepable(int32_t combo);
    bool ishookshottable(int32_t bx, int32_t by, int32_t i);
    bool ishookshottable(int32_t map, int32_t screen, int32_t bx, int32_t by, int32_t i);
    int32_t warpindex(int32_t combo);
    bool clearall(bool validate);
    bool reset_templates(bool validate);
    bool clearmap(bool newquest);
    void clearscr(int32_t scr);
    int32_t  load(const char *path);
    int32_t  save(const char *path);
    int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t x,int32_t y);
    int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t pos);
    int32_t MAPCOMBO2(int32_t lyr,int32_t x,int32_t y, int32_t map = -1, int32_t screen = -1);
    int32_t MAPCOMBO(int32_t x,int32_t y, int32_t map = -1, int32_t screen = -1);
    int32_t MAPFLAG3(int32_t map, int32_t screen, int32_t layer, int32_t x,int32_t y);
    int32_t MAPFLAG3(int32_t map, int32_t screen, int32_t layer, int32_t pos);
    int32_t MAPFLAG2(int32_t lyr,int32_t x,int32_t y, int32_t map = -1, int32_t screen = -1);
    int32_t MAPFLAG(int32_t x,int32_t y, int32_t map = -1, int32_t screen = -1);
    void put_walkflags_layered(BITMAP *dest,int32_t x,int32_t y,int32_t pos,int32_t layer);
    void put_walkflags_layered_external(BITMAP *dest,int32_t x,int32_t y,int32_t pos,int32_t layer, int32_t map, int32_t screen);
    bool misaligned(int32_t map, int32_t screen, int32_t i, int32_t dir);
    void check_alignments(BITMAP* dest,int32_t x,int32_t y,int32_t scr=-1);
	void draw_darkness(BITMAP* dest, BITMAP* transdest);
    void draw(BITMAP *dest,int32_t x,int32_t y,int32_t flags,int32_t map,int32_t screen,int32_t hl_layer);
    void drawrow(BITMAP *dest,int32_t x,int32_t y,int32_t flags,int32_t c,int32_t map,int32_t scr);
    void drawcolumn(BITMAP *dest,int32_t x,int32_t y,int32_t flags,int32_t c,int32_t map,int32_t scr);
    void drawblock(BITMAP* dest,int32_t x,int32_t y,int32_t flags,int32_t c,int32_t map,int32_t scr);
    void drawstaticblock(BITMAP* dest,int32_t x,int32_t y);
    void drawstaticrow(BITMAP* dest,int32_t x,int32_t y);
    void drawstaticcolumn(BITMAP* dest,int32_t x,int32_t y);
    void draw_template(BITMAP *dest,int32_t x,int32_t y);
    void draw_template2(BITMAP *dest,int32_t x,int32_t y);
    void draw_secret(BITMAP *dest, int32_t pos);
    void draw_secret2(BITMAP *dest, int32_t pos);
    void scroll(int32_t dir, bool warp);
    MapCursor getCursor() const;
    void setCursor(MapCursor new_cursor);
    bool isValidPosition(ComboPosition pos) const;
    int getScreenForPosition(ComboPosition pos) const;
    mapscr *CurrScr();
    mapscr *Scr(int32_t scr);
	mapscr *Scr(ComboPosition pos);
	mapscr* Scr(ComboPosition pos, int layer);
	mapscr* ScrMakeValid(ComboPosition pos, int layer);
    mapscr *AbsoluteScr(int32_t scr);
    mapscr *AbsoluteScr(int32_t map, int32_t screen);
	mapscr *AbsoluteScrMakeValid(int32_t map, int32_t scr);
    int32_t  getCurrMap();
	void regions_mark_dirty();
	void regions_refresh();
	const std::vector<region_description>& get_region_descriptions();
	bool is_region(int screen);
    bool isDark(int scr);
	bool isValid(int32_t scr);
	bool isValid(int32_t map, int32_t scr);
    void setCurrMap(int32_t index);
    int32_t  getCurrScr();
    void setCurrScr(int32_t scr);
	int32_t getViewScr();
	void setViewSize(int32_t size);
	int32_t getViewSize();
    void setlayertarget();
    void setcolor(int color, mapscr* scr = nullptr);
    int32_t getcolor();
    void resetflags();
    word tcmbdat(int32_t pos);
    word tcmbcset(int32_t pos);
    int32_t tcmbflag(int32_t pos);
    word tcmbdat2(int32_t pos);
    word tcmbcset2(int32_t pos);
    int32_t tcmbflag2(int32_t pos);
    void put_door(BITMAP *dest,int32_t pos,int32_t side,int32_t type,int32_t xofs,int32_t yofs,bool ignorepos, int32_t scr);
    void over_door(BITMAP *dest,int32_t pos,int32_t side,int32_t xofs,int32_t yofs,bool ignorepos, int32_t scr);
    void TemplateAll();
    void Template(int32_t floorcombo, int32_t floorcset, int32_t scr);
    void putdoor(int32_t screen,int32_t side,int32_t door);
    
	void goto_dmapscr(int dmap, int scr);
	void goto_mapscr(int map, int scr);
	
	void dowarp(int32_t type, int32_t index);
    void prv_dowarp(int32_t type, int32_t index);
    void dowarp2(int32_t ring,int32_t index);
	
	void set_warpback();
	bool has_warpback();
	void warpback();
	
    void set_prvscr(int32_t map, int32_t scr);
    mapscr* get_prvscr();
    void prv_secrets(bool);
    int32_t get_prv_map()
    {
        return prv_map;
    }
    int32_t get_prv_scr()
    {
        return prv_scr;
    }
    void set_prvcmb(int32_t set)
    {
        prv_cmbcycle=set;
    }
    int32_t get_prvcmb()
    {
        return prv_cmbcycle;
    }
    void set_prvfreeze(int32_t set)
    {
        prv_freeze=set;
    }
    int32_t get_prvfreeze()
    {
        return prv_freeze;
    }
    void set_prvadvance(int32_t set)
    {
        prv_advance=set;
    }
    int32_t get_prvadvance()
    {
        return prv_advance;
    }
    void set_prvtime(int32_t set)
    {
        prv_time=set;
    }
    int32_t get_prvtime()
    {
        return prv_time;
    }
    int32_t getLayerTargetMap();
    int32_t getLayerTargetScr();
    int32_t getLayerTargetMultiple();
};

extern zmap Map;

extern const char *loaderror[];

void put_walkflags(BITMAP *dest,int32_t x,int32_t y,word cmbdat,int32_t layer);
void put_flags(BITMAP *dest,int32_t x,int32_t y,word cmbdat,int32_t cset,int32_t flags,int32_t sflag);
void put_flag(BITMAP* dest, int32_t x, int32_t y, int32_t flag);
void put_combo(BITMAP *dest,int32_t x,int32_t y,word cmbdat,int32_t cset,int32_t flags,int32_t sflag,int32_t scale);
void put_combo(BITMAP *dest,int32_t x,int32_t y,word cmbdat,int32_t cset,int32_t flags,int32_t sflag);
void put_engraving(BITMAP* dest, int32_t x, int32_t y, int32_t slot, int32_t scale);
void copy_mapscr(mapscr *dest, const mapscr *src);
void delete_mapscr(mapscr *dest);


#define DEFAULT_TILESET "tilesets/cambria.qst"

bool setMapCount2(int32_t c);
int32_t init_quest(std::string tileset_path);
void set_questpwd(std::string_view pwd, bool use_keyfile);
int32_t quest_access(const char *filename, zquestheader *hdr);
bool write_midi(MIDI *m,PACKFILE *f);
int32_t load_quest(const char *filename, bool show_progress = true);
int32_t load_tileset(const char *filename, dword tsetflags);
int32_t save_unencoded_quest(const char *filename, bool compressed, const char* afname = NULL);
int32_t save_quest(const char *filename, bool timed_save);

int32_t writemapscreen(PACKFILE *f, int32_t i, int32_t j);

bool load_msgstrs(const char *path, int32_t startstring);
bool save_msgstrs(const char *path);
bool save_msgstrs_text(const char *path);
int32_t writestrings(PACKFILE *f, word version, word build, word start_msgstr, word max_msgstrs);
int32_t writestrings_text(PACKFILE *f);

bool save_strings_tsv(const char *path);
bool load_strings_tsv(const char *path);
void parse_strings_tsv(std::string tsv);
int32_t writestrings_tsv(PACKFILE *f);

bool load_pals(const char *path, int32_t startcset);
bool save_pals(const char *path);
int32_t writecolordata(PACKFILE *f, word version, word build, word start_cset, word max_csets);

int32_t writecombos(PACKFILE *f, word version, word build, word start_combo, word max_combos);
int32_t writecomboaliases(PACKFILE *f, word version, word build);

int32_t writetiles(PACKFILE *f, word version, word build, int32_t start_tile, int32_t max_tiles);

int32_t writedmaps(PACKFILE *f, word version, word build, word start_dmap, word max_dmaps);

bool save_guys(const char *path);
bool load_guys(const char *path);

bool load_zgp(const char *path);
bool save_zgp(const char *path);

bool save_subscreen(const char *path, ZCSubscreen const& savefrom);
bool load_subscreen(const char *path, ZCSubscreen& loadto);

bool load_zqt(const char *path);
bool save_zqt(const char *path);

void center_zq_class_dialogs();

int32_t writeitems(PACKFILE *f, zquestheader *Header);
int32_t writeweapons(PACKFILE *f, zquestheader *Header);
int32_t writemisccolors(PACKFILE *f, zquestheader *Header);
int32_t writegameicons(PACKFILE *f, zquestheader *Header);
int32_t writedoorcombosets(PACKFILE *f, zquestheader *Header);
int32_t write_one_subscreen(PACKFILE *f, zquestheader *Header, int32_t i);
int32_t writeffscript(PACKFILE *f, zquestheader *Header);
int32_t writeffscript_old(PACKFILE *f, zquestheader *Header);
int32_t write_quest_zasm(PACKFILE *f);
int32_t write_one_ffscript(PACKFILE *f, zquestheader *Header, int32_t i, script_data *script);
int32_t write_one_ffscript_old(PACKFILE *f, zquestheader *Header, int32_t i, script_data *script);
int32_t writeitemdropsets(PACKFILE *f, zquestheader *Header);
int32_t writefavorites(PACKFILE *f, zquestheader *Header);

#endif
