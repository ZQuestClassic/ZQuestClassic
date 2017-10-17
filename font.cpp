/*         ______   ___    ___
  *        /\  _  \ /\_ \  /\_ \
  *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
  *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
  *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
  *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
  *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
  *                                           /\____/
  *                                           \_/__/
  *
  *      Grabber plugin for managing font objects.
  *
  *      By Shawn Hargreaves.
  *
  *      GRX font file reader by Mark Wodrich.
  *
  *      See readme.txt for copyright information.
  */
#ifndef __FONT_C__
#define __FONT_C__

#include <stdio.h>
#include <string.h>

#include "zc_alleg.h"
#include "allegro/internal/aintern.h"
#include "/allegro/tools/datedit.h"
#include "font.h"

extern FONT* create_font();

typedef unsigned char   byte;                               //0-255         ( 8 bits)
typedef unsigned short  word;                               //0-65535       (16 bits)
typedef unsigned long   dword;                              //0-4294967295  (32 bits)

extern byte *tilebuf;

// in tiles.cc
byte unpackbuf[256];

void unpack_tile(int tile)
{
  byte *si = tilebuf+((tile+1)<<7);
  int di;
  for(di=254; di>=0; di-=2)
  {
    --si;
    unpackbuf[di] = (*si) & 15;
    unpackbuf[di+1] = (*si) >> 4;
  }
}

static int convert_8x8_tiles_to_mono_font(FONT_GLYPH** gl, int start, int num)
{
  int w = 8, h = 8, i;

  for(i = 0; i < 32; i++)
  {
    int sx = ((w + 7) / 8), j;
    gl[i] = _al_malloc(sizeof(FONT_GLYPH) + sx * h);
    gl[i]->w = w;
    gl[i]->h = h;

    for(j = 0; j < sx * h; j++) gl[i]->dat[j] = 0;
  }
  for(i = 32; i < num+32; i++)
  {
    int tile=(((i-32)/16)*20)+((i-32)%16);

    int sx = ((w + 7) / 8), j, k;

    unpack_tile(start+tile);
    gl[i] = _al_malloc(sizeof(FONT_GLYPH) + sx * h);
    gl[i]->w = w;
    gl[i]->h = h;

    for(j = 0; j < sx * h; j++) gl[i]->dat[j] = 0;

    for(j = 0; j < h; j++)
    {
      for(k = 0; k < w; k++)
      {
        if(unpackbuf[j*16+k])
        {
          gl[i]->dat[(j * sx) + (k / 8)] |= 0x80 >> (k & 7);
        }
      }
    }
  }
  return 0;
}

static int convert_16x16_tiles_to_mono_font(FONT_GLYPH** gl, int start, int num)
{
  int w = 8, h = 8, i;

  for(i = 0; i < 32; i++)
  {
    int sx = ((w + 7) / 8), j;
    gl[i] = _al_malloc(sizeof(FONT_GLYPH) + sx * h);
    gl[i]->w = w;
    gl[i]->h = h;

    for(j = 0; j < sx * h; j++) gl[i]->dat[j] = 0;
  }
  for(i = 32; i < num+32; i++)
  {
    int tile=(((i-32)/16)*20)+((i-32)%16);

    int sx = ((w + 7) / 8), j, k;

    unpack_tile(start+tile);
    gl[i] = _al_malloc(sizeof(FONT_GLYPH) + sx * h);
    gl[i]->w = w;
    gl[i]->h = h;

    for(j = 0; j < sx * h; j++) gl[i]->dat[j] = 0;

    for(j = 0; j < h; j++)
    {
      for(k = 0; k < w; k++)
      {
        if(unpackbuf[j*16+k])
        {
          gl[i]->dat[(j * sx) + (k / 8)] |= 0x80 >> (k & 7);
        }
      }
    }
  }
  return 0;
}

/* import routine for the Allegro .pcx font format */
FONT* create_font()
{
  FONT *f;
  FONT_MONO_DATA* mf = _al_malloc(sizeof(FONT_MONO_DATA));

  f = _al_malloc(sizeof(FONT));

  mf->glyphs = _al_malloc(sizeof(FONT_GLYPH*) * (192));

  if( convert_8x8_tiles_to_mono_font(mf->glyphs, 10920, 192) )
  {
    free(mf->glyphs);
    free(mf);
    free(f);
    f = 0;
  }
  else
  {
    f->data = mf;
    f->vtable = font_vtable_mono;
    f->height = mf->glyphs[0]->h;
    mf->begin = 0;
    mf->end = 192;
    mf->next = 0;
  }

  //delete this.  only added to keep compiler from whining
  if( convert_16x16_tiles_to_mono_font(mf->glyphs, 10920, 192) )
  {
    free(mf->glyphs);
    free(mf);
    free(f);
    f = 0;
  }
  else
  {
    f->data = mf;
    f->vtable = font_vtable_mono;
    f->height = mf->glyphs[0]->h;
    mf->begin = 0;
    mf->end = 192;
    mf->next = 0;
  }

  return f;

}
#endif                                                      //__FONT_C__
 
