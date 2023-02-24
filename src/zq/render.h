#ifndef _ZQ_RENDER_TREE_H_
#define _ZQ_RENDER_TREE_H_

#include "base/render.h"

ALLEGRO_BITMAP* get_overlay_bmp();
ALLEGRO_BITMAP* add_dlg_overlay();
void remove_dlg_overlay(ALLEGRO_BITMAP* bmp);
BITMAP* get_tooltip_bmp();
void set_dlg_transp(bool transp);
void zq_hide_screen(bool hidden);
void render_zq();

#endif
