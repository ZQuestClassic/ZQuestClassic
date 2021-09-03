#ifndef ZC_GUI_DIALOGEVENT_H
#define ZC_GUI_DIALOGEVENT_H

#include <functional>
#include <variant>

namespace gui
{

// Just a simple variant typedef.
using EventArg=std::variant<
    std::monostate,
    int
>;

using MessageDispatcher=std::function<void(int, EventArg)>;

}

#endif
