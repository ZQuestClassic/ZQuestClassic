//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_class.cc
//
//  Contains zmap class and other main code for ZQuest.
//
//--------------------------------------------------------

#ifndef _ZQ_CLASS_H_
#define _ZQ_CLASS_H_

#include "base/zdefs.h"
#include <stdio.h>
#include <string_view>

int32_t COMBOPOS(int32_t x, int32_t y);
int32_t COMBOX(int32_t pos);
int32_t COMBOY(int32_t pos);

void set_preview_mode(int32_t prv);

/************************/
/****** ZMAP class ******/
/************************/

void reset_dmap(int32_t index);
//void mapfix_0x166(mapscr *scr);
bool setMapCount2(int32_t c);
class zmap
{
    mapscr *screens;
    int32_t currmap,copymap;
    int32_t currscr,copyscr;
    int32_t copyffc;
    int32_t scrpos[MAXMAPS2+1];

    mapscr copymapscr;
    mapscr undomap[MAPSCRS+6];
    mapscr prvscr; //NEW
    mapscr prvlayers[6];
    //int32_t prv_mode; //NEW
    int32_t prv_cmbcycle, prv_map, prv_scr, prv_freeze, prv_advance, prv_time; //NEW
    bool can_undo,can_paste,can_undo_map,can_paste_map,screen_copy;
    // A screen which uses the current screen as a layer
    int32_t layer_target_map, layer_target_scr, layer_target_multiple;

public:

    zmap();
    ~zmap();
    bool CanUndo();
    bool CanPaste();
    int32_t  CopyScr();
    int32_t  getCopyFFC();
    void setCopyFFC(int32_t n);
    void Ugo();
    void Uhuilai();
    void Copy();
    void CopyFFC(int32_t n);
    void Paste();
    void PasteAll();
    void PasteToAll();
    void PasteAllToAll();
    void PasteUnderCombo();
    void PasteSecretCombos();
    void PasteFFCombos();
    void PasteOneFFC(int32_t i);
    void PasteWarps();
    void PasteScreenData();
    void PasteWarpLocations();
    void PasteDoors();
    void PasteLayers();
    void PasteRoom();
    void PasteGuy();
    void PastePalette();
    void PasteEnemies();
    void setCanPaste(bool _set);
    void setCanUndo(bool _set);
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
    void clearzcmap(int32_t map);
    int32_t  load(const char *path);
    int32_t  save(const char *path);
    int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t x,int32_t y);
    int32_t MAPCOMBO3(int32_t map, int32_t screen, int32_t layer, int32_t pos);
    int32_t MAPCOMBO2(int32_t lyr,int32_t x,int32_t y, int32_t map = -1, int32_t scr = -1);
    int32_t MAPCOMBO(int32_t x,int32_t y, int32_t map = -1, int32_t scr = -1);
    void put_walkflags_layered(BITMAP *dest,int32_t x,int32_t y,int32_t pos,int32_t layer);
    void put_walkflags_layered_external(BITMAP *dest,int32_t x,int32_t y,int32_t pos,int32_t layer, int32_t map, int32_t screen);
    bool misaligned(int32_t map, int32_t scr, int32_t i, int32_t dir);
    void check_alignments(BITMAP* dest,int32_t x,int32_t y,int32_t scr=-1);
	void draw_darkness(BITMAP* dest, BITMAP* transdest);
    void draw(BITMAP *dest,int32_t x,int32_t y,int32_t flags,int32_t map,int32_t scr);
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
    void scroll(int32_t dir);
    mapscr *CurrScr();
    mapscr *Scr(int32_t scr);
    mapscr *AbsoluteScr(int32_t scr);
    mapscr *AbsoluteScr(int32_t map, int32_t scr);
    int32_t  getCurrMap();
    bool isDark();
    void setCurrMap(int32_t index);
    int32_t  getCurrScr();
    void setCurrScr(int32_t scr);
    void setlayertarget();
    void setcolor(int32_t c);
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
    void Template(int32_t floorcombo, int32_t floorcset);
    void Template(int32_t floorcombo, int32_t floorcset, int32_t scr);
    void putdoor(int32_t side,int32_t door);
    void putdoor2(int32_t side,int32_t door);
    void putdoor(int32_t scr,int32_t side,int32_t door);
    void putdoor2(int32_t scr,int32_t side,int32_t door);
    void dowarp(int32_t type, int32_t index);
    void prv_dowarp(int32_t type, int32_t index);
    void dowarp2(int32_t ring,int32_t index);
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

//bool read_old_mapfile(mapscr *scr, FILE *f);
extern const char *loaderror[];

void put_walkflags(BITMAP *dest,int32_t x,int32_t y,word cmbdat,int32_t layer);
void put_flags(BITMAP *dest,int32_t x,int32_t y,word cmbdat,int32_t cset,int32_t flags,int32_t sflag);
void put_flag(BITMAP* dest, int32_t x, int32_t y, int32_t flag);
void put_combo(BITMAP *dest,int32_t x,int32_t y,word cmbdat,int32_t cset,int32_t flags,int32_t sflag);
void copy_mapscr(mapscr *dest, const mapscr *src);
void delete_mapscr(mapscr *dest);

/******************************/
/******** ZQuest stuff ********/
/******************************/

//word msg_count=0;
//const char zqsheader[30];

bool setMapCount2(int32_t c);
int32_t init_quest(const char *templatefile);
void set_questpwd(std::string_view pwd, bool use_keyfile);
int32_t quest_access(const char *filename, zquestheader *hdr, bool compressed);
bool write_midi(MIDI *m,PACKFILE *f);
int32_t load_quest(const char *filename, bool compressed, bool encrypted);
int32_t save_unencoded_quest(const char *filename, bool compressed, const char* afname = NULL);
int32_t save_quest(const char *filename, bool timed_save);

int32_t writemapscreen(PACKFILE *f, int32_t i, int32_t j);

bool load_msgstrs(const char *path, int32_t startstring);
bool save_msgstrs(const char *path);
bool save_msgstrs_text(const char *path);
int32_t writestrings(PACKFILE *f, word version, word build, word start_msgstr, word max_msgstrs);
int32_t writestrings_text(PACKFILE *f);

bool load_pals(const char *path, int32_t startcset);
bool save_pals(const char *path);
int32_t writecolordata(PACKFILE *f, miscQdata *Misc, word version, word build, word start_cset, word max_csets);

bool load_combos(const char *path, int32_t startcombo);
bool save_combos(const char *path);
int32_t writecombos(PACKFILE *f, word version, word build, word start_combo, word max_combos);
int32_t writecomboaliases(PACKFILE *f, word version, word build);

bool load_tiles(const char *path, int32_t starttile);
bool save_tiles(const char *path);
int32_t writetiles(PACKFILE *f, word version, word build, int32_t start_tile, int32_t max_tiles);

bool load_dmaps(const char *path, int32_t startdmap);
bool save_dmaps(const char *path);
int32_t writedmaps(PACKFILE *f, word version, word build, word start_dmap, word max_dmaps);

bool save_guys(const char *path);
bool load_guys(const char *path);

bool load_zgp(const char *path);
bool save_zgp(const char *path);

bool load_subscreen(const char *path);
bool save_subscreen(const char *path, bool *cancel);

bool load_zqt(const char *path);
bool save_zqt(const char *path);

void center_zq_class_dialogs();

int32_t writeitems(PACKFILE *f, zquestheader *Header);
int32_t writeweapons(PACKFILE *f, zquestheader *Header);
int32_t writemisccolors(PACKFILE *f, zquestheader *Header, miscQdata *Misc);
int32_t writegameicons(PACKFILE *f, zquestheader *Header, miscQdata *Misc);
int32_t writedoorcombosets(PACKFILE *f, zquestheader *Header);
int32_t write_one_subscreen(PACKFILE *f, zquestheader *Header, int32_t i);
int32_t writeffscript(PACKFILE *f, zquestheader *Header, bool keepdata);
int32_t write_one_ffscript(PACKFILE *f, zquestheader *Header, int32_t i, script_data **script);
int32_t writeitemdropsets(PACKFILE *f, zquestheader *Header);
int32_t writefavorites(PACKFILE *f, zquestheader *Header);
#endif
