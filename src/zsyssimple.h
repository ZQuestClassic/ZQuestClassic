#ifndef ZSYSSIMPLE_H
#define ZSYSSIMPLE_H

struct FONT;

void box_out(const char *msg);
void box_start(int style, const char *title, FONT *title_font, FONT *message_font, bool log);
void box_end(bool pause);
void box_eol();
void Z_message(char *format,...);

extern bool gotoless_not_equal; // Used by BuildVisitors.cpp

#endif
 
