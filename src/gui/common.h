#ifndef ZC_GUI_COMMON_H
#define ZC_GUI_COMMON_H

#include <string>
#include <string_view>
#include <cstdint>

struct DIALOG;

/* Shows a tooltip below the given screen rectangle once the mouse has rested
 * there. Implemented by the editor's tooltip system; the player and launcher
 * link no-op stubs. See Widget::setTooltip.
 */
void gui_tooltip_show(std::string const& text, int32_t x, int32_t y, int32_t w, int32_t h);
void gui_tooltip_hide();

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
