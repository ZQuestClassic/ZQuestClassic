#ifndef ZC_GUI_COMMON_H
#define ZC_GUI_COMMON_H

#include <string_view>

extern bool is_large;

namespace GUI
{

/* Finds the first character preceded by & and returns it. && is ignored.
 * Returns 0 if no character was found.
 */
int getAccelKey(const std::string_view text);

/* Returns a in small mode, b in large mode. */
template<typename T>
inline T sized(T a, T b)
{
	return is_large ? b : a;
}

}

#endif
