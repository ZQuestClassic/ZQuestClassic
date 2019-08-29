#ifndef ZSYSSIMPLE_H
#define ZSYSSIMPLE_H

struct FONT;

void box_out(const char *msg);
void box_start(int style, char *title, FONT *title_font, FONT *message_font, bool log);
void box_end(bool pause);
void box_eol();

#endif
