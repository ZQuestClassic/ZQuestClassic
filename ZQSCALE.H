#ifndef _ZQSCALE_H_
#define _ZQSCALE_H_

bool zqwin_set_scale(int scale, bool defer = false);
int zqwin_set_gfx_mode(int card, int w, int h, int v_w, int v_h);
void zq_set_mouse_range(int x1, int y1, int x2, int y2);

#ifdef ROMVIEW_SCALE
#define set_gfx_mode zqwin_set_gfx_mode
#endif
#endif
 
