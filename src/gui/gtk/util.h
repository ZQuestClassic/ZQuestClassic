#ifndef _ZC_GUI_GTK_UTIL_H_
#define _ZC_GUI_GTK_UTIL_H_

#include <gtk/gtk.h>
#include <string>
struct KeyInput;
struct MouseInput;

bool translateKeyEvent(GdkEvent* event, KeyInput& out);
bool translateMouseEvent(GdkEvent* event, int scale, MouseInput& out);
std::string convertMnemonic(const std::string& str);

#endif
