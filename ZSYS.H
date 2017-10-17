//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zsys.h
//
//  System functions, etc.
//
//--------------------------------------------------------

#ifdef ALLEGRO_DOS
//nothing to do here
#elif defined(ALLEGRO_WINDOWS)
#include <conio.h>
#elif defined(ALLEGRO_LINUX)
#endif

#ifndef _ZSYS_H_
#define _ZSYS_H_

#include "zdefs.h"
#include "jwin.h"

#define FILENAME8_3   0
#define FILENAME8__   1
#define FILENAMEALL   2

extern int jwin_pal[jcMAX];

extern volatile int dclick_status, dclick_time;

char *time_str_short(dword time);
char *time_str_med(dword time);
char *time_str_long(dword time);

void extract_name(char *path,char *name,int type);
void chop_path(char *path);
char *temp_name(char *s);
int  vbound(int x,int low,int high);
float vbound(float x,float low,float high);
int  used_switch(int argc,char *argv[],char *s);
bool isinRect(int x,int y,int rx1,int ry1,int rx2,int ry2);
void crash();

extern char zeldapwd[8];
extern char zquestpwd[8];
extern char datapwd[8];
extern char zcheat[4][8];

void resolve_password(char *pwd);

bool decode_007(byte *buf, dword size, dword key, word check1, word check2, int method);
void encode_007(byte *buf, dword size, dword key, word *check1, word *check2, int method);
int encode_file_007(char *srcfile, char *destfile, int key, char *header, int method);
int decode_file_007(char *srcfile, char *destfile, char *header, int method, bool packed);
void copy_file(char *src, char *dest);

int  get_bit(byte *bitstr,int bit);
void set_bit(byte *bitstr,int bit,byte val);

void Z_error(char *format,...);
void Z_message(char *format,...);
void Z_title(char *format,...);

int anim_3_4(int clk, int speed);

                                                            /*0=normal, 1=jwin*/
void box_start(int style , char *title, FONT *title_font, FONT *message_font, bool log);
//void box_start();
void box_out(char *msg);
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
#endif                                                      // _ZSYS_H_
