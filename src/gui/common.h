#ifndef ZC_GUI_COMMON_H
#define ZC_GUI_COMMON_H

#include <string_view>
#include <cstdint>

struct DIALOG;

extern bool is_large;
#define USE_READABLE_FONT zc_get_config("gui","bolder_font",0)
#define GUI_READABLE_FONT (USE_READABLE_FONT ? sized(nfont, lfont) : GUI_DEF_FONT)

namespace GUI
{


int32_t newGUIProcImpl(int32_t msg, DIALOG* d, int32_t c, int32_t (*base)(int32_t, DIALOG*, int32_t));

template<int32_t (*PROC)(int32_t, DIALOG*, int32_t)>
int32_t newGUIProc(int32_t msg, DIALOG* d, int32_t c)
{
	return newGUIProcImpl(msg, d, c, PROC);
}

/* Finds the first character preceded by & and returns it. && is ignored.
 * Returns 0 if no character was found.
 */
int32_t getAccelKey(const std::string_view text);

/* Returns a in small mode, b in large mode. */
template<typename T>
inline T sized(T a, T b) noexcept
{
	return is_large ? b : a;
}

}

#endif
