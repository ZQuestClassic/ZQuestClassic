#ifndef ZC_GUI_COMMON_H
#define ZC_GUI_COMMON_H

#include <string_view>

struct DIALOG;

extern bool is_large;

namespace GUI
{


int newGUIProcImpl(int msg, DIALOG* d, int c, int (*base)(int, DIALOG*, int));

template<int (*PROC)(int, DIALOG*, int)>
int newGUIProc(int msg, DIALOG* d, int c)
{
	return newGUIProcImpl(msg, d, c, PROC);
}

/* Finds the first character preceded by & and returns it. && is ignored.
 * Returns 0 if no character was found.
 */
int getAccelKey(const std::string_view text);

/* Returns a in small mode, b in large mode. */
template<typename T>
inline T sized(T a, T b) noexcept
{
	return is_large ? b : a;
}

}

#endif
