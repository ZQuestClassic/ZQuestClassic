#ifndef ZC_GUI_SHOWDIALOG_H
#define ZC_GUI_SHOWDIALOG_H

#include "dialogEvent.h"
#include "dialogRunner.h"
#include <type_traits>

namespace gui
{

// Pick either the argument or non-argument version of handleMessage()
// based on which can be called.

template<typename T>
std::enable_if_t<std::is_invocable_v<
    decltype(&T::handleMessage), T&, typename T::Message, gui::EventArg>, void>
showDialog(T& dlg)
{
    auto dr=DialogRunner();
    dr.runWithArg(dlg);
}

template<typename T>
std::enable_if_t<std::is_invocable_v<
    decltype(&T::handleMessage), T&, typename T::Message>, void>
showDialog(T& dlg)
{
    auto dr=DialogRunner();
    dr.runWithoutArg(dlg);
}

}

#endif
