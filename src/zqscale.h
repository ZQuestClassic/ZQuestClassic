#ifndef _ZQSCALE_H_
#define _ZQSCALE_H_

bool zqwin_set_scale(int32_t scale, bool defer = false);
int32_t zqwin_set_gfx_mode(int32_t card, int32_t w, int32_t h, int32_t v_w, int32_t v_h);
void zq_set_mouse_range(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

#ifdef ROMVIEW_SCALE
#define set_gfx_mode zqwin_set_gfx_mode
#endif
#endif
 
