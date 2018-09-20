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

#include "zdefs.h"
#include <stdio.h>

#define COMBOPOS(x,y) (((y)&0xF0)+((x)>>4))
#define COMBOX(pos) ((pos)%16*16)
#define COMBOY(pos) ((pos)&0xF0)

void set_preview_mode(int prv);

/************************/
/****** ZMAP class ******/
/************************/

void reset_dmap(int index);
//void mapfix_0x166(mapscr *scr);
bool setMapCount2(int c);
class zmap
{
    mapscr *screens;
    int currmap,copymap;
    int currscr,copyscr;
    int copyffc;
    int scrpos[MAXMAPS2+1];
    
    mapscr copymapscr;
    mapscr undomap[MAPSCRS+6];
    mapscr prvscr; //NEW
    mapscr prvlayers[6];
    //int prv_mode; //NEW
    int prv_cmbcycle, prv_map, prv_scr, prv_freeze, prv_advance, prv_time; //NEW
    bool can_undo,can_paste,can_undo_map,can_paste_map,screen_copy;
    // A screen which uses the current screen as a layer
    int layer_target_map, layer_target_scr, layer_target_multiple;
    
public:

    zmap();
    ~zmap();
    bool CanUndo();
    bool CanPaste();
    int  CopyScr();
    int  getCopyFFC();
    void Ugo();
    void Uhuilai();
    void Copy();
    void CopyFFC(int n);
    void Paste();
    void PasteAll();
    void PasteToAll();
    void PasteAllToAll();
    void PasteUnderCombo();
    void PasteSecretCombos();
    void PasteFFCombos();
    void PasteOneFFC(int i);
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
    int getMapCount();
    bool isDungeon(int scr);
    bool isstepable(int combo);
    bool ishookshottable(int bx, int by, int i);
    int warpindex(int combo);
    bool clearall(bool validate);
    bool reset_templates(bool validate);
    bool clearmap(bool newquest);
    void clearscr(int scr);
    void clearzcmap(int map);
    int  load(const char *path);
    int  save(const char *path);
    int MAPCOMBO2(int lyr,int x,int y, int map = -1, int scr = -1);
    int MAPCOMBO(int x,int y, int map = -1, int scr = -1);
    void put_walkflags_layered(BITMAP *dest,int x,int y,int pos,int layer);
    bool misaligned(int map, int scr, int i, int dir);
    void check_alignments(BITMAP* dest,int x,int y,int scr=-1);
    void draw(BITMAP *dest,int x,int y,int flags,int map,int scr);
    void drawrow(BITMAP *dest,int x,int y,int flags,int c,int map,int scr);
    void drawcolumn(BITMAP *dest,int x,int y,int flags,int c,int map,int scr);
    void drawblock(BITMAP* dest,int x,int y,int flags,int c,int map,int scr);
    void drawstaticblock(BITMAP* dest,int x,int y);
    void drawstaticrow(BITMAP* dest,int x,int y);
    void drawstaticcolumn(BITMAP* dest,int x,int y);
    void draw_template(BITMAP *dest,int x,int y);
    void draw_template2(BITMAP *dest,int x,int y);
    void draw_secret(BITMAP *dest, int pos);
    void draw_secret2(BITMAP *dest, int pos);
    void scroll(int dir);
    mapscr *CurrScr();
    mapscr *Scr(int scr);
    mapscr *AbsoluteScr(int scr);
    mapscr *AbsoluteScr(int map, int scr);
    int  getCurrMap();
    bool isDark();
    void setCurrMap(int index);
    int  getCurrScr();
    void setCurrScr(int scr);
    void setlayertarget();
    void setcolor(int c);
    int getcolor();
    void resetflags();
    word tcmbdat(int pos);
    word tcmbcset(int pos);
    int tcmbflag(int pos);
    word tcmbdat2(int pos);
    word tcmbcset2(int pos);
    int tcmbflag2(int pos);
    void put_door(BITMAP *dest,int pos,int side,int type,int xofs,int yofs,bool ignorepos, int scr);
    void over_door(BITMAP *dest,int pos,int side,int xofs,int yofs,bool ignorepos, int scr);
    void TemplateAll();
    void Template(int floorcombo, int floorcset);
    void Template(int floorcombo, int floorcset, int scr);
    void putdoor(int side,int door);
    void putdoor2(int side,int door);
    void putdoor(int scr,int side,int door);
    void putdoor2(int scr,int side,int door);
    void dowarp(int type, int index);
    void prv_dowarp(int type, int index);
    void dowarp2(int ring,int index);
    void set_prvscr(int map, int scr);
    mapscr* get_prvscr();
    void prv_secrets(bool);
    int get_prv_map()
    {
        return prv_map;
    }
    int get_prv_scr()
    {
        return prv_scr;
    }
    void set_prvcmb(int set)
    {
        prv_cmbcycle=set;
    }
    int get_prvcmb()
    {
        return prv_cmbcycle;
    }
    void set_prvfreeze(int set)
    {
        prv_freeze=set;
    }
    int get_prvfreeze()
    {
        return prv_freeze;
    }
    void set_prvadvance(int set)
    {
        prv_advance=set;
    }
    int get_prvadvance()
    {
        return prv_advance;
    }
    void set_prvtime(int set)
    {
        prv_time=set;
    }
    int get_prvtime()
    {
        return prv_time;
    }
    int getLayerTargetMap();
    int getLayerTargetScr();
    int getLayerTargetMultiple();
};

extern zmap Map;

//bool read_old_mapfile(mapscr *scr, FILE *f);
extern const char *loaderror[];

void put_walkflags(BITMAP *dest,int x,int y,word cmbdat,int layer);
void put_flags(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag);
void put_combo(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag);
void copy_mapscr(mapscr *dest, const mapscr *src);
void delete_mapscr(mapscr *dest);

/******************************/
/******** ZQuest stuff ********/
/******************************/

//word msg_count=0;
//const char zqsheader[30];

bool setMapCount2(int c);
int init_quest(const char *templatefile);
void set_questpwd(const char *pwd, bool use_keyfile);
int quest_access(const char *filename, zquestheader *hdr, bool compressed);
bool write_midi(MIDI *m,PACKFILE *f);
int load_quest(const char *filename, bool compressed, bool encrypted);
int save_unencoded_quest(const char *filename, bool compressed);
int save_quest(const char *filename, bool timed_save);

int writemapscreen(PACKFILE *f, int i, int j);

bool load_msgstrs(const char *path, int startstring);
bool save_msgstrs(const char *path);
bool save_msgstrs_text(const char *path);
int writestrings(PACKFILE *f, word version, word build, word start_msgstr, word max_msgstrs);
int writestrings_text(PACKFILE *f);

bool load_pals(const char *path, int startcset);
bool save_pals(const char *path);
int writecolordata(PACKFILE *f, miscQdata *Misc, word version, word build, word start_cset, word max_csets);

bool load_combos(const char *path, int startcombo);
bool save_combos(const char *path);
int writecombos(PACKFILE *f, word version, word build, word start_combo, word max_combos);
int writecomboaliases(PACKFILE *f, word version, word build);

bool load_tiles(const char *path, int starttile);
bool save_tiles(const char *path);
int writetiles(PACKFILE *f, word version, word build, int start_tile, int max_tiles);

bool load_dmaps(const char *path, int startdmap);
bool save_dmaps(const char *path);
int writedmaps(PACKFILE *f, word version, word build, word start_dmap, word max_dmaps);

bool save_guys(const char *path);
bool load_guys(const char *path);

bool load_zgp(const char *path);
bool save_zgp(const char *path);

bool load_subscreen(const char *path);
bool save_subscreen(const char *path, bool *cancel);

bool load_zqt(const char *path);
bool save_zqt(const char *path);

void center_zq_class_dialogs();

int writeitems(PACKFILE *f, zquestheader *Header);
int writeweapons(PACKFILE *f, zquestheader *Header);
int writemisccolors(PACKFILE *f, zquestheader *Header, miscQdata *Misc);
int writegameicons(PACKFILE *f, zquestheader *Header, miscQdata *Misc);
int writedoorcombosets(PACKFILE *f, zquestheader *Header);
int write_one_subscreen(PACKFILE *f, zquestheader *Header, int i);
int writeffscript(PACKFILE *f, zquestheader *Header, bool keepdata);
int write_one_ffscript(PACKFILE *f, zquestheader *Header, int i, ffscript **script);
int writeitemdropsets(PACKFILE *f, zquestheader *Header);
int writefavorites(PACKFILE *f, zquestheader *Header);
#endif
