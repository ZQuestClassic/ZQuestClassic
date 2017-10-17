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

// define these in main code
//extern bool init_tiles(bool validate);
//extern bool init_combos(bool validate);

#define checkstatus(ret)\
  switch (ret) \
  { \
    case 0:\
    break;\
    case qe_invalid:\
    goto invalid;\
    break;\
    default:\
      pack_fclose(f);\
      if(!oldquest)\
        delete_file(tmpfilename);\
        return ret;\
        break;\
  }

extern bool bad_version(int ver);

enum
{
  qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
  qe_missing, qe_internal, qe_pwd, qe_match, qe_minver,
  qe_nomem, qe_debug
};

enum
{
  zgp_tiles, zgp_combos, zgp_pals, zgp_items, zgp_wpns, zgp_misc,
  zgp_maps, zgp_doors
};

enum
{
  skip_header, skip_rules, skip_strings, skip_misc, skip_tiles, skip_combos, skip_comboaliases, skip_csets, skip_maps, skip_dmaps, skip_doors, skip_items, skip_weapons, skip_colors, skip_icons, skip_initdata, skip_guys, skip_linksprites, skip_subscreens, skip_ffscript, skip_sfx, skip_midis, skip_cheats, skip_max
};

extern char *qst_error[];
extern word msg_count;
extern word door_combo_set_count;

char *VerStr(int version);
char *ordinal(int num);

PACKFILE *open_quest_file(int *open_error, char *filename, char *deletefilename, bool compressed, bool show_progress);
PACKFILE *open_quest_template(zquestheader *header, char *deletefilename, bool validate);

void clear_combo(int i);
void clear_combos();
void pack_combos();
void fix_maps(mapscr *buf,int cnt);
void reset_midi(music *m);
void reset_midis(music *m);
void reset_scr(int scr);
//bool reset_items();
//bool reset_wpns();

void get_qst_buffers();
int count_dmaps();
int count_shops(miscQdata *misc);
int count_infos(miscQdata *misc);
int count_warprings(miscQdata *misc);
int count_palcycles(miscQdata *misc);
int count_windwarps(miscQdata *misc);
int loadquest(char *filename, zquestheader *Header, miscQdata *Misc, music *midis, bool show_progress, bool compressed, bool encrypted, bool keepall, byte *skip_flags);
int loadquest2(char *filename, zquestheader *header,miscQdata *misc, music *midis);

char *byte_conversion(int number, int format);
char *byte_conversion2(int number1, int number2, int format1, int format2);

bool valid_zqt(PACKFILE *f);
bool valid_zqt(char *filename);
bool reset_mapstyles(bool validate, zquestheader *header, miscQdata *misc);
bool reset_items(bool validate, zquestheader *header);
bool reset_wpns(bool validate, zquestheader *header);
bool reset_doorcombosets(bool validate, zquestheader *header);
bool init_tiles(bool validate, zquestheader *header);
bool init_colordata(bool validate, zquestheader *header, miscQdata *misc);
bool init_combos(bool validate, zquestheader *header);

int readheader(PACKFILE *f, zquestheader *header, bool keepdata);
int readrules(PACKFILE *f, zquestheader *header, bool keepdata);
int readstrings(PACKFILE *f, zquestheader *header, bool keepdata);
int readdoorcombosets(PACKFILE *f, zquestheader *header, bool keepdata);
int readdmaps(PACKFILE *f, zquestheader *header, word version, word build, word start_dmap, word max_dmaps, bool keepdata);
int readmisc(PACKFILE *f, zquestheader *header, miscQdata *misc, bool keepdata);
int readitems(PACKFILE *f, word version, word build, bool keepdata);
int readweapons(PACKFILE *f, zquestheader *header, bool keepdata);
int readguys(PACKFILE *f, zquestheader *header, bool keepdata);
int readmapscreen(PACKFILE *f, zquestheader *header, mapscr *temp_mapscr);
int readmaps(PACKFILE *f, zquestheader *header, bool keepdata);
int readcombos(PACKFILE *f, zquestheader *header, word version, word build, word start_combo, word max_combos, bool keepdata);
int readcolordata(PACKFILE *f, miscQdata *misc, word version, word build, word start_cset, word max_csets, bool keepdata);
int readtiles(PACKFILE *f, byte *buf, zquestheader *header, word version, word build, word start_tile, word max_tiles, bool from_init, bool keepdata);
int readmidis(PACKFILE *f, zquestheader *header, music *midis, bool keepdata);
int readcheatcodes(PACKFILE *f, zquestheader *header, bool keepdata);
int readinitdata(PACKFILE *f, zquestheader *header, bool keepdata);


int get_version_and_build(PACKFILE *f, word *version, word *build);
bool find_section(PACKFILE *f, long section_id_requested);


inline int skipheader(PACKFILE *f, zquestheader *header)
{
  return readheader(f, header, false);
}
inline int skiptiles(PACKFILE *f, byte *buf, zquestheader *header, word version, word build, word start_tile, word max_tiles)
{
  return readtiles(f, buf, header, version, build, start_tile, max_tiles, false, false);
}
inline int skipcombos(PACKFILE *f, zquestheader *header, word version, word build, word start_combo, word max_combos)
{
  return readcombos(f, header, version, build, start_combo, max_combos, false);
}
inline int skipcolordata(PACKFILE *f, zquestheader *header, miscQdata *misc, word version, word build, word start_cset, word max_csets)
{
  return readcolordata(f, misc, version, build, start_cset, max_csets, false);
}
inline int skipstrings(PACKFILE *f, zquestheader *header, word version, word build, word start_string, word max_strings)
{
  return readstrings(f, header, false);
}
inline int skipdmaps(PACKFILE *f, zquestheader *header, word version, word build, word start_dmap, word max_dmaps)
{
  return readdmaps(f, header, version, build, start_dmap, max_dmaps, false);
}


#endif                                                      // _ZC_QST_H_
