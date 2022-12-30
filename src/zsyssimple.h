#ifndef ZSYSSIMPLE_H
#define ZSYSSIMPLE_H

#include <cstdint>

struct FONT;

void box_out(const char *msg);
void box_out_nl(const char *msg);
void box_start(int32_t style, const char *title, FONT *title_font, FONT *message_font, bool log, int32_t w = -1, int32_t h = -1, uint8_t scale = 1);
void box_end(bool pause);
void box_eol();
void Z_message(char const*format,...);

#endif
 
