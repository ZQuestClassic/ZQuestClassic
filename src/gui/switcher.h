#ifndef ZC_GUI_SWITCHER_H
#define ZC_GUI_SWITCHER_H

#include "widget.h"
#include "dialog.h"
#include <vector>

namespace gui
{

/* A container that displays any one of multiple widgets. Used to dynamically
 * change the layout, e.g. to switch between a TextField and a DropDownList
 * when some selection is changed.
 *
 * XXX This one needs some work, but it'll require some changes further down.
 * And it may be better just to provide a way to replace a widget directly.
 */
class Switcher: public Widget
{
public:
    Switcher();
    void add(std::shared_ptr<Widget> child);

    // Makes the widget with the given index visible, hiding all others.
    void show(size_t index);

    // Returns the index of the currently visible widget.
    size_t getVisible() const;

    // Returns the widget at the given index, cast to the specified type.
    template<typename T>
    std::shared_ptr<T> get(size_t index)
    {
        auto ret=std::dynamic_pointer_cast<T>(widgets.at(index));
        assert(ret);
        return ret;
    }

private:
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<DialogRef> dialogs;
    size_t visible;

    void calculateSize() override;
    void arrange(int contX, int contY, int contW, int contH) override;
    void realize(DialogRunner& runner) override;
};

}

#endif
