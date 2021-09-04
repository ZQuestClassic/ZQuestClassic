#ifndef ZC_GUI_DIALOGEVENT_H
#define ZC_GUI_DIALOGEVENT_H

#include <functional>
#include <string_view>
#include <variant>

namespace gui
{

// Just a simple variant typedef.
using EventArg=std::variant<
    std::monostate,
    bool,
    int,
    std::string_view
>;

using MessageDispatcher=std::function<void(int, EventArg)>;

}

#endif
