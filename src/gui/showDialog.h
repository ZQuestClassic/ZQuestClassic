#ifndef ZC_GUI_SHOWDIALOG_H
#define ZC_GUI_SHOWDIALOG_H

#include "dialogMessage.h"
#include "dialogRunner.h"
#include "helper.h"
#include <type_traits>

namespace gui
{

// Pick either the argument or non-argument version of handleMessage()
// based on which can be called.

template<typename T>
std::enable_if_t<
    std::is_invocable_v<
        decltype(&T::handleMessage), T&, typename T::Message, gui::MessageArg
    >, void>
showDialog(T& dlg)
{
    auto dr=DialogRunner();
    dr.runWithArg(dlg);
}

template<typename T>
std::enable_if_t<
    std::is_invocable_v<
        decltype(&T::handleMessage), T&, typename T::Message
    >, void>
showDialog(T& dlg)
{
    auto dr=DialogRunner();
    dr.runWithoutArg(dlg);
}

// This one just exists to produce a more helpful error message if neither
// version is correctly implemented. The enable_if is just to prevent
// additional errors from ambiguity with the two above.
template<typename T, bool b=false>
std::enable_if_t<
    not std::is_invocable_v<
        decltype(&T::handleMessage), T&, typename T::Message, gui::MessageArg>
    and not std::is_invocable_v<
        decltype(&T::handleMessage), T&, typename T::Message
    >, void>
showDialog(T& dlg)
{
    ZCGUI_STATIC_ASSERT(b,
        "No valid handleMessage() implementation found.\n"
        "You must implement one of the following:\n"
        "handleMessage([DialogClass]::Message, gui::EventArg)\n"
        "handleMessage([DialogClass]::Message)");
}

}

#endif
