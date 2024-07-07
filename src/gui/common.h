#ifndef ZC_GUI_COMMON_H_
#define ZC_GUI_COMMON_H_

#include <string_view>
#include <cstdint>

struct DIALOG;

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

}

#endif
