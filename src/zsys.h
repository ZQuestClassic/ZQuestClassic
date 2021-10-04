//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zsys.h
//
//  System functions, etc.
//
//--------------------------------------------------------

#if defined(ALLEGRO_WINDOWS)
#include <conio.h>
#endif

#ifndef _ZSYS_H_
#define _ZSYS_H_

#include "zdefs.h"
#include "jwin.h"

#define FILENAME8_3   0
#define FILENAME8__   1
#define FILENAMEALL   2

extern int jwin_pal[jcMAX];

extern bool is_large;
extern void large_dialog(DIALOG *d);

extern volatile int dclick_status, dclick_time;

char *time_str_short(dword time);
char *time_str_short2(dword time);
char *time_str_med(dword time);
char *time_str_long(dword time);

void extract_name(char *path,char *name,int type);
void temp_name(char temporaryname[]);
char *zc_make_relative_filename(char *dest, const char *path, const char *filename, int size);
void chop_path(char *path);
int  used_switch(int argc,char *argv[],const char *s);
bool isinRect(int x,int y,int rx1,int ry1,int rx2,int ry2);

extern char zeldapwd[8];
extern char zquestpwd[8];
extern char datapwd[8];
extern char zcheat[4][8];

void resolve_password(char *pwd);

bool decode_007(byte *buf, dword size, dword key, word check1, word check2, int method);
void encode_007(byte *buf, dword size, dword key, word *check1, word *check2, int method);
int encode_file_007(const char *srcfile, const char *destfile, int key, const char *header, int method);
int decode_file_007(const char *srcfile, const char *destfile, const char *header, int method, bool packed, const char *password);
void copy_file(const char *src, const char *dest);

int  get_bit(byte const* bitstr,int bit);
void set_bit(byte *bitstr,int bit,byte val);
bool toggle_bit(byte *bitstr,int bit);

int  get_bitl(long bitstr,int bit);
void set_bitl(long bitstr,int bit,byte val);

void Z_error(const char *format,...);
void Z_message(const char *format,...);
void Z_title(const char *format,...);

int anim_3_4(int clk, int speed);

int bound(int &x,int low,int high);

enum {ssfmtBMP, ssfmtGIF, ssfmtJPG, ssfmtPNG, ssfmtPCX, ssfmtTGA, ssfmtMAX };
extern char *snapshotformat_str[ssfmtMAX][2];
extern const char *snapshotformatlist(int index, int *list_size);


/*0=normal, 1=jwin*/
void box_start(int style , const char *title, FONT *title_font, FONT *message_font, bool log, int w = -1, int h = -1, unsigned char scale = 1);
//void box_start();
void box_out(const char *msg);
void box_out_nl(const char *msg);
void box_save_x();
void box_load_x();
void box_eol();
void box_end(bool pause);
void box_pause();
void dither_rect(BITMAP *bmp, PALETTE *pal, int x1, int y1, int x2, int y2,
                 int src_color1, int src_color2, byte dest_color1,
                 byte dest_color2);
void dclick_check(void);
void lock_dclick_function();

void textout_shadow_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_centre_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_right_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_u_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_centre_u_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_right_u_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_o_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_centre_o_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_right_o_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_plus_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_centre_plus_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_right_plus_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_x_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_centre_x_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadow_right_x_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int shadow, int bg);
void textout_shadowed_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_centre_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_right_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_u_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_centre_u_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_right_u_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_o_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_centre_o_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_right_o_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);



void textout_shadowed_plus_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_centre_plus_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_right_plus_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_x_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_centre_x_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);
void textout_shadowed_right_x_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int shadow, int bg);




void textprintf_shadow_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_centre_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_right_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_u_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_centre_u_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_right_u_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_o_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_centre_o_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_right_o_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);





void textprintf_shadow_plus_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_centre_plus_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_right_plus_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_x_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_centre_x_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);
void textprintf_shadow_right_x_ex(BITMAP *bmp, const FONT *f, int x, int y, int shadow, int bg, const char *format, ...);



void textprintf_shadowed_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_centre_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_right_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_u_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_centre_u_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_right_u_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_o_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_centre_o_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_right_o_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);


void textprintf_shadowed_plus_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_centre_plus_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_right_plus_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_x_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_centre_x_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);
void textprintf_shadowed_right_x_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int shadow, int bg, const char *format, ...);

void copy_dialog(DIALOG **to, DIALOG *from);
void free_dialog(DIALOG **dlg);

void safe_rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);

int gcd(int a, int b);
int lcm(int a, int b);

void quit_game();

int zc_trace_handler(const char *);
void zc_trace_clear();

void sane_destroy_bitmap(BITMAP **bmp);

extern bool zconsole;

#endif                                                      // _ZSYS_H_

