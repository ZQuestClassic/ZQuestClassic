#ifndef _RENDER_TOOLTIP_H_
#define _RENDER_TOOLTIP_H_

#include <string>

void ttip_add(std::string text, int x, int y, float scale);
void ttip_add_highlight(int x, int y, int w, int h, int fw, int fh);
void ttip_set_highlight_thickness(int thickness);
void ttip_clear_timer();
void ttip_remove();

#endif
