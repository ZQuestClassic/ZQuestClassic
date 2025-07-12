#ifndef SUBSCR_H_
#define SUBSCR_H_

#include "base/compiler.h"
#include "new_subscr.h"
#include "base/zc_alleg.h"

#define ssflagSHOWGRID  1
#define ssflagSHOWINVIS 2

extern bool show_subscreen_dmap_dots;
extern bool show_subscreen_numbers;
extern bool show_subscreen_items;
extern bool show_subscreen_life;

void textout_styled_aligned_ex(BITMAP *bmp, const FONT *f, const char *s, int32_t x, int32_t y, int32_t textstyle, int32_t alignment, int32_t color, int32_t shadow, int32_t bg);
void frame2x2(BITMAP *dest,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h,int32_t flip,bool overlay,bool trans);
void drawgrid(BITMAP *dest,int32_t x,int32_t y,int32_t c1,int32_t c2);
void draw_block(BITMAP *dest,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h);
void lifemeter(BITMAP *dest,int32_t x,int32_t y,int32_t tile,bool bs_style);
void magicmeter(BITMAP *dest,int32_t x,int32_t y);
void buttonitem(BITMAP *dest, int32_t button, int32_t x, int32_t y);
void putxnum(BITMAP *dest,int32_t x,int32_t y,int32_t num,FONT *tempfont,int32_t color,int32_t shadowcolor,int32_t bgcolor,int32_t textstyle,bool usex,int32_t digits,bool infinite,char idigit);
void defaultcounters(BITMAP *dest, int32_t x, int32_t y, FONT *tempfont, int32_t color, int32_t shadowcolor, int32_t bgcolor, bool usex, int32_t textstyle, int32_t digits, char idigit);
void counter(BITMAP *dest, int32_t x, int32_t y, FONT *tempfont, int32_t color, int32_t shadowcolor, int32_t bgcolor, int32_t alignment, int32_t textstyle, int32_t digits, char idigit, bool showzero, int32_t itemtype1, int32_t itemtype2, int32_t itemtype3, int32_t infiniteitem, bool onlyselected);

INLINE void putdot(BITMAP *dest,int32_t x,int32_t y,int32_t c)
{
    rectfill(dest,x,y,x+2,y+2,c);
}

// subscreen default types
enum { ssdtOLD, ssdtNEWSUBSCR, ssdtREV2, ssdtBSZELDA, ssdtBSZELDAMODIFIED, ssdtBSZELDAENHANCED, ssdtBSZELDACOMPLETE, ssdtZ3, ssdtMAX };

enum { sssFULLPUSH, sssFULLSLIDEDOWN, sssMAX };

struct sso_struct
{
    char *s;
    int32_t i;
};

extern sso_struct bisso[widgMAX];





/****  Subscr items code  ****/
extern subscreen_object default_subscreen_active[ssdtMAX][2][66];
extern subscreen_object default_subscreen_passive[ssdtMAX][2][21];
extern subscreen_object z3_active_a[80];
extern subscreen_object z3_passive_a[66];
extern subscreen_object z3_active_ab[82];
extern subscreen_object z3_passive_ab[75];
extern std::vector<ZCSubscreen> subscreens_active, subscreens_passive, subscreens_overlay;
extern ZCSubscreen *new_subscreen_active;
extern ZCSubscreen *new_subscreen_passive;
extern ZCSubscreen *new_subscreen_overlay;
extern int new_sub_indexes[3];
extern bool subscreen_open;
extern int active_sub_yoff;

const byte tripiece[2][8][3] =
{
    //  112,112,0, 128,112,1, 96,128,0, 144,128,1,
    //  112,128,2, 112,128,1, 128,128,3, 128,128,0
    {
        //old style
        {32,8,0},
        {48,8,1},
        {16,24,0},
        {64,24,1},
        {32,24,2},
        {32,24,1},
        {48,24,3},
        {48,24,0}
    },
    {
        //bs style
        {32,8,0},
        {56,8,1},
        {8,56,0},
        {80,56,1},
        {32,56,2},
        {32,56,1},
        {56,56,3},
        {56,56,0}
    }
};
const byte bmap_original[2][16] =
{
    {0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0},
    {0,3,0,0,0,3,0,0,0,0,3,0,0,0,3,0}
};
const byte bmap_bs[2][12] =
{
    {0,0,1,0,0,1,0,0,1,0,1,0},
    {0,3,0,0,3,0,0,3,0,3,0,0}
};
const int32_t fringe[8] = { 6,2,4,7,6,8,7,5 };
const byte bmaptiles_old[8*5] =
{
    0, 1, 2, 3, 2, 3, 3, 4,
    20,21,22,23,22,23,23,24,
    20,21,22,23,22,23,23,24,
    20,21,22,23,22,23,23,24,
    40,41,42,43,42,43,43,44
};

const byte bmaptiles_original[5][8] =
{
    {0, 1, 2, 3, 2, 3, 3, 4},
    {20,21,22,23,22,23,23,24},
    {20,21,22,23,22,23,23,24},
    {20,21,22,23,22,23,23,24},
    {40,41,42,43,42,43,43,44}
};

const byte bmaptiles_bs[5][6] =
{
    {0, 1, 2, 3, 3, 4},
    {20,21,22,23,23,24},
    {20,21,22,23,23,24},
    {20,21,22,23,23,24},
    {40,41,42,43,43,44}
};

void refresh_subscr_buttonitems();
void animate_subscr_buttonitems();
void refresh_subscr_items();
void kill_subscr_items();
int32_t stripspaces(char *source, char *target, int32_t stop);
void puttriframe(BITMAP *dest, int32_t x, int32_t y, int32_t triframecolor, int32_t numbercolor, int32_t triframetile, int32_t triframecset, int32_t triforcetile, int32_t triforcecset, bool showframe, bool showpieces, bool largepieces);
void puttriforce(BITMAP *dest, int32_t x, int32_t y, int32_t tile, int32_t cset, int32_t w, int32_t h, int32_t flip, bool overlay, bool trans, int32_t trinum);
void draw_block(BITMAP *dest,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h);
void draw_block_flip(BITMAP *dest,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h,int32_t flip,bool overlay,bool trans);
void putBmap(BITMAP *dest, int32_t x, int32_t y,bool showmap, bool showrooms, bool showhero, int32_t roomcolor, int32_t herocolor, bool large);
void textout_styled_aligned_ex(BITMAP *bmp, const FONT *f, const char *s, int32_t x, int32_t y, int32_t textstyle, int32_t alignment, int32_t color, int32_t shadow, int32_t bg);
ZC_FORMAT_PRINTF(10, 11)
void textprintf_styled_aligned_ex(BITMAP *bmp, const FONT *f, int32_t x, int32_t y, int32_t textstyle, int32_t alignment, int32_t color, int32_t shadow, int32_t bg, const char *format, ...);
void update_subscreens(int32_t dmap=-1);
void show_custom_subscreen(BITMAP *dest, ZCSubscreen *subscr, int32_t xofs, int32_t yofs, bool showtime, int32_t pos2);

void purge_blank_subscreen_objects(SubscrPage& pg);
int32_t subscreen_cset(int32_t c1, int32_t c2);

void sso_bounding_box(BITMAP *bmp, SubscrWidget* widg, int32_t color);


bool findWeaponWithParent(int32_t id, int32_t type);
int32_t countWeaponWithParent(int32_t id, int32_t type);

#endif
