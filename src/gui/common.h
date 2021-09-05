#ifndef ZC_GUI_COMMON_H
#define ZC_GUI_COMMON_H

#include <string_view>

namespace gui
{

/* Finds the first character preceded by & and returns it. && is ignored.
 * Returns 0 if no character was found.
 */
int getAccelKey(const std::string_view text);

}

#endif
