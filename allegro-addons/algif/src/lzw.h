#ifndef _LZW_H_
#define _LZW_H_
#include <allegro.h>
int LZW_decode (PACKFILE * file, BITMAP *bmp);
void LZW_encode (PACKFILE * file, BITMAP *bmp);
#endif
