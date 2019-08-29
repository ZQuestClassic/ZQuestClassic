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


void set_preview_mode(int prv);

/************************/
/****** ZMAP class ******/
/************************/

void reset_dmap(int index);
//void mapfix_0x166(mapscr *scr);
bool setMapCount2(int c,bool reload);
class zmap
{
  mapscr *screens;
  int currmap,copymap;
  int currscr,copyscr;
  int scrpos[MAXMAPS2+1];

  mapscr copymapscr;
  mapscr undomap[128];
  mapscr prvscr; //NEW
  mapscr prvlayers[6];
  //int prv_mode; //NEW
  int prv_cmbcycle, prv_map, prv_scr, prv_freeze, prv_advance, prv_time; //NEW
  bool can_undo,can_paste,can_undo_map,can_paste_map,screen_copy;

public:

  zmap();
  bool CanUndo();
  bool CanPaste();
  int  CopyScr();
  void Ugo();
  void Uhuilai();
  void Copy();
  void Paste();
  void PasteAll();
  void PasteToAll();
  void PasteAllToAll();
  void PasteUnderCombo();
  void PasteSecretCombos();
  void PasteFFCombos();
  void PasteWarps();
  void PasteScreenData();
  void PasteWarpLocations();
  void PasteDoors();
  void PasteLayers();
  void PasteRoom();
  void PasteGuy();
  void PasteEnemies();
  void update_combo_cycling();
  void update_freeform_combos();
  int getMapCount();
  bool isDungeon(int scr);
  bool clearall(bool validate);
  bool reset_templates(bool validate);
  bool clearmap(bool newquest);
  void clearscr(int scr);
  void savescreen();
  int  load(char *path);
  int  save(char *path);
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
  void setcolor(int c);
  int getcolor();
  void resetflags();
  word tcmbdat(int pos);
  word tcmbcset(int pos);
  int tcmbflag(int pos);
  word tcmbdat2(int pos);
  word tcmbcset2(int pos);
  int tcmbflag2(int pos);
  void put_door(BITMAP *dest,int pos,int side,int type,int xofs,int yofs,bool ignorepos);
  void over_door(BITMAP *dest,int pos,int side,int xofs,int yofs,bool ignorepos);
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
  int get_prv_map() {return prv_map;}
  int get_prv_scr() {return prv_scr;}
  void set_prvcmb(int set) {prv_cmbcycle=set;}
  int get_prvcmb() {return prv_cmbcycle;}
  void set_prvfreeze(int set) {prv_freeze=set;}
  int get_prvfreeze() {return prv_freeze;}
  void set_prvadvance(int set) {prv_advance=set;}
  int get_prvadvance() {return prv_advance;}
  void set_prvtime(int set) {prv_time=set;}
  int get_prvtime() {return prv_time;}
};

extern zmap Map;

//bool read_old_mapfile(mapscr *scr, FILE *f);
extern char *loaderror[];

void put_walkflags(BITMAP *dest,int x,int y,word cmbdat);
void put_flags(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag);
void put_combo(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag);
void over_combo_translucent(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int opacity);
void over_combo(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags);

/******************************/
/******** ZQuest stuff ********/
/******************************/

//word msg_count=0;
//const char zqsheader[30];

char *MsgString(int index);
void reset_msgstr(int index);
void init_msgstrs();
bool setMapCount2(int c, bool reload);
int init_quest();
void set_questpwd(char *pwd, bool use_keyfile);
bool quest_access(char *filename);
bool write_midi(MIDI *m,PACKFILE *f);
int load_quest(char *filename, bool compressed);
int save_unencoded_quest(char *filename, bool compressed);
int save_quest(char *filename, bool version_210);

bool load_msgstrs(char *path, int startstring);
bool save_msgstrs(char *path);
int writestrings(PACKFILE *f, word version, word build, word start_msgstr, word max_msgstrs);

bool load_pals(char *path, int startcset);
bool save_pals(char *path);
int writecolordata(PACKFILE *f, miscQdata *misc, word version, word build, word start_cset, word max_csets);

bool load_combos(char *path, int startcombo);
bool save_combos(char *path);
int writecombos(PACKFILE *f, word version, word build, word start_combo, word max_combos);
int writecomboaliases(PACKFILE *f, word version, word build);

bool load_tiles(char *path, int starttile);
bool save_tiles(char *path);
int writetiles(PACKFILE *f, word version, word build, word start_tile, word max_tiles);

bool load_dmaps(char *path, int startdmap);
bool save_dmaps(char *path);
int writedmaps(PACKFILE *f, word version, word build, word start_dmap, word max_dmaps);

bool load_zgp(char *path);
bool save_zgp(char *path);

bool load_subscreen(char *path);
bool save_subscreen(char *path, bool *cancel);

bool load_zqt(char *path);
bool save_zqt(char *path);

void center_zq_class_dialogs();

int writeitems(PACKFILE *f, zquestheader *header);
int writeweapons(PACKFILE *f, zquestheader *header);
int writemisccolors(PACKFILE *f, zquestheader *header, miscQdata *misc);
int writegameicons(PACKFILE *f, zquestheader *header, miscQdata *misc);
int writedoorcombosets(PACKFILE *f, zquestheader *header);
int write_one_subscreen(PACKFILE *f, zquestheader *header, int i);
int writeffscript(PACKFILE *f, zquestheader *header, bool keepdata);
int write_one_ffscript(PACKFILE *f, zquestheader *header, int i, ffscript **script);

#define ZELDA_VERSION_210       0x0210                          //version of the program
#define VERSION_BUILD_210       0                               //build number of this version

#define V_HEADER_210        1
#define V_RULES_210         1
#define V_STRINGS_210       1
#define V_MISC_210          1
#define V_TILES_210         1
#define V_COMBOS_210        1
#define V_CSETS_210         1
#define V_MAPS_210          2
#define V_DMAPS_210         1
#define V_DOORS_210         1
#define V_ITEMS_210         1
#define V_WEAPONS_210       1
#define V_COLORS_210        1
#define V_ICONS_210         1
#define V_INITDATA_210      1
#define V_GUYS_210          1
#define V_MIDIS_210         1
#define V_CHEATS_210        1
#define V_SAVEGAME_210      1

#define CV_HEADER_210       1
#define CV_RULES_210        1
#define CV_STRINGS_210      1
#define CV_MISC_210         1
#define CV_TILES_210        1
#define CV_COMBOS_210       1
#define CV_CSETS_210        1
#define CV_MAPS_210         2
#define CV_DMAPS_210        1
#define CV_DOORS_210        1
#define CV_ITEMS_210        1
#define CV_WEAPONS_210      1
#define CV_COLORS_210       1
#define CV_ICONS_210        1
#define CV_INITDATA_210     1
#define CV_GUYS_210         1
#define CV_MIDIS_210        1
#define CV_CHEATS_210       1
#define CV_SAVEGAME_210     1


int save_unencoded_quest_210(char *filename, bool compressed);
#endif
