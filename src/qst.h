//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  qst.h
//
//  Code for loading '.qst' files in ZC and ZQuest.
//
//--------------------------------------------------------

#ifndef _ZC_QST_H_
#define _ZC_QST_H_

#include "zdefs.h"
#include "subscr.h"
#include "zscriptversion.h"


#include "ffscript.h"
extern FFScript ffengine;

// define these in main code
//extern bool init_tiles(bool validate);
//extern bool init_combos(bool validate);

#define checkstatus(ret)\
          switch (ret)  \
        {               \
          case 0:       \
          break;        \
          case qe_invalid:\
          goto invalid; \
          break;        \
          default:      \
          pack_fclose(f);\
          if(!oldquest) \
            delete_file(tmpfilename);\
          return ret;   \
          break;        \
        }

extern bool bad_version(int ver);

enum
{
    qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
    qe_missing, qe_internal, qe_pwd, qe_match, qe_minver,
    qe_nomem, qe_debug, qe_cancel
};

enum
{
    zgp_tiles, zgp_combos, zgp_pals, zgp_items, zgp_wpns, zgp_misc,
    zgp_maps, zgp_doors
};

enum
{
    skip_header, skip_rules, skip_strings, skip_misc,
    skip_tiles, skip_combos, skip_comboaliases, skip_csets,
    skip_maps, skip_dmaps, skip_doors, skip_items,
    skip_weapons, skip_colors, skip_icons, skip_initdata,
    skip_guys, skip_linksprites, skip_subscreens, skip_ffscript,
    skip_sfx, skip_midis, skip_cheats, skip_itemdropsets,
    skip_favorites, skip_max
};

extern const char *qst_error[];
extern word msg_count;
extern word door_combo_set_count;
extern int favorite_combos[MAXFAVORITECOMBOS];
extern int favorite_comboaliases[MAXFAVORITECOMBOALIASES];

char *VerStr(int version);
char *ordinal(int num);

PACKFILE *open_quest_file(int *open_error, const char *filename, char *deletefilename, bool compressed, bool encrypted, bool show_progress);
PACKFILE *open_quest_template(zquestheader *Header, char *deletefilename, bool validate);

void clear_combo(int i);
void clear_combos();
void pack_combos();
void fix_maps(mapscr *buf,int cnt);
//void reset_midi(zcmidi_ *m);
//void reset_midis(zcmidi_ *m);
void reset_scr(int scr);
//bool reset_items();
//bool reset_wpns();

int get_qst_buffers();
void del_qst_buffers();
int count_dmaps();
int count_shops(miscQdata *Misc);
int count_infos(miscQdata *Misc);
int count_warprings(miscQdata *Misc);
int count_palcycles(miscQdata *Misc);

void port250QuestRules();
void portCandleRules();
void portBombRules();

int loadquest(const char *filename, zquestheader *Header,
              miscQdata *Misc, zctune *tunes, bool show_progress, bool compressed, bool encrypted, bool keepall, byte *skip_flags);

char *byte_conversion(int number, int format);
char *byte_conversion2(int number1, int number2, int format1, int format2);

bool valid_zqt(PACKFILE *f);
bool valid_zqt(const char *filename);
bool reset_mapstyles(bool validate, miscQdata *Misc);
bool reset_items(bool validate, zquestheader *Header);
bool reset_guys();
bool reset_wpns(bool validate, zquestheader *Header);
bool reset_doorcombosets(bool validate, zquestheader *Header);
bool init_tiles(bool validate, zquestheader *Header);
bool init_colordata(bool validate, zquestheader *Header, miscQdata *Misc);
bool init_combos(bool validate, zquestheader *Header);

void get_questpwd(char *encrypted_pwd, short pwdkey, char *pwd);
bool check_questpwd(zquestheader *Header, char *pwd);

void update_guy_1(guydata *tempguy);
void initMsgStr(MsgStr *str);
void init_msgstrings(int start, int end);

int copyquest(PACKFILE *f);
int readheader(PACKFILE *f, zquestheader *Header, bool keepdata);
int readrules(PACKFILE *f, zquestheader *Header, bool keepdata);
int readstrings(PACKFILE *f, zquestheader *Header, bool keepdata);
int readdoorcombosets(PACKFILE *f, zquestheader *Header, bool keepdata);
int readdmaps(PACKFILE *f, zquestheader *Header, word version, word build, word start_dmap, word max_dmaps, bool keepdata);
int readmisccolors(PACKFILE *f, zquestheader *Header, miscQdata *Misc, bool keepdata);
int readgameicons(PACKFILE *f, zquestheader *Header, miscQdata *Misc, bool keepdata);
int readmisc(PACKFILE *f, zquestheader *Header, miscQdata *Misc, bool keepdata);
int readitems(PACKFILE *f, word version, word build, bool keepdata, bool zgpmode=false);
int readweapons(PACKFILE *f, zquestheader *Header, bool keepdata);
int readguys(PACKFILE *f, zquestheader *Header, bool keepdata);
int readmapscreen(PACKFILE *f, zquestheader *Header, mapscr *temp_mapscr, zcmap *temp_map, word version);
int readmaps(PACKFILE *f, zquestheader *Header, bool keepdata);
int readcombos(PACKFILE *f, zquestheader *Header, word version, word build, word start_combo, word max_combos, bool keepdata);
int readcomboaliases(PACKFILE *f, zquestheader *Header, word version, word build, bool keepdata);
int readcolordata(PACKFILE *f, miscQdata *Misc, word version, word build, word start_cset, word max_csets, bool keepdata);
int readtiles(PACKFILE *f, tiledata *buf, zquestheader *Header, word version, word build, word start_tile, int max_tiles, bool from_init, bool keepdata);
int readtunes(PACKFILE *f, zquestheader *Header, zctune *tunes, bool keepdata);
int readcheatcodes(PACKFILE *f, zquestheader *Header, bool keepdata);
int readinitdata(PACKFILE *f, zquestheader *Header, bool keepdata);
int readsubscreens(PACKFILE *f, zquestheader *Header, bool keepdata);
int read_one_subscreen(PACKFILE *f, zquestheader *Header, bool keepdata, int i, word s_version, word s_cversion);
int readffscript(PACKFILE *f, zquestheader *Header, bool keepdata);
int read_one_ffscript(PACKFILE *f, zquestheader *Header, bool keepdata, int i, word s_version, word s_cversion, ffscript **script);
int readsfx(PACKFILE *f, zquestheader *Header, bool keepdata);
int readitemdropsets(PACKFILE *f, word version, word build, bool keepdata);
int readfavorites(PACKFILE *f, int, word, bool keepdata);


int get_version_and_build(PACKFILE *f, word *version, word *build);
bool find_section(PACKFILE *f, long section_id_requested);


INLINE int skipheader(PACKFILE *f, zquestheader *Header)
{
    return readheader(f, Header, false);
}
INLINE int skiptiles(PACKFILE *f, tiledata *buf, zquestheader *Header, word version, word build, word start_tile, word max_tiles)
{
    return readtiles(f, buf, Header, version, build, start_tile, max_tiles, false, false);
}
INLINE int skipcombos(PACKFILE *f, zquestheader *Header, word version, word build, word start_combo, word max_combos)
{
    return readcombos(f, Header, version, build, start_combo, max_combos, false);
}
INLINE int skipcolordata(PACKFILE *f, zquestheader *Header, miscQdata *Misc, word version, word build, word start_cset, word max_csets)
{
    (void)Header;
    return readcolordata(f, Misc, version, build, start_cset, max_csets, false);
}
INLINE int skipstrings(PACKFILE *f, zquestheader *Header, word version, word build, word start_string, word max_strings)
{
    (void)max_strings;
    (void)start_string;
    (void)build;
    (void)version;
    return readstrings(f, Header, false);
}
INLINE int skipdmaps(PACKFILE *f, zquestheader *Header, word version, word build, word start_dmap, word max_dmaps)
{
    return readdmaps(f, Header, version, build, start_dmap, max_dmaps, false);
}

extern void delete_combo_aliases();
void reset_subscreen(subscreen_group *tempss);
void reset_subscreens();
int setupsubscreens();
void setupsfx();
void reset_itembuf(itemdata *item, int id);
void reset_itemname(int id);
void reset_weaponname(int i);
void init_guys(int guyversion);
void init_item_drop_sets();
void init_favorites();

#endif                                                      // _ZC_QST_H_

