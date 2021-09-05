#ifndef ZC_GUI_WINDOW_H
#define ZC_GUI_WINDOW_H

#include "topLevel.h"
#include <memory>
#include <string>

namespace gui
{

class Window: public TopLevelWidget
{
public:
    Window();
    void setTitle(std::string newTitle);
    void setContent(std::shared_ptr<Widget> newContent);
    template<typename T>
    void onClose(T m)
    {
        closeMessage=static_cast<int>(m);
    }

private:
    std::shared_ptr<Widget> content;
    std::string title;
    int closeMessage;

    void arrange(int contX, int contY, int contW, int contH) override;
    void realize(DialogRunner& runner) override;
    int onEvent(int event, MessageDispatcher sendMessage) override;
};

}

#endif
