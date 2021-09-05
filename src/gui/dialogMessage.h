#ifndef ZC_GUI_DIALOGMESSAGE_H
#define ZC_GUI_DIALOGMESSAGE_H

#include <functional>
#include <string_view>
#include <variant>

namespace gui
{

// Just a simple variant typedef.
using MessageArg=std::variant<
    std::monostate,
    bool,
    int,
    std::string_view
>;

using MessageDispatcher=std::function<void(int, MessageArg)>;

}

#endif
