#ifndef ZC_GUI_SWITCHER_H
#define ZC_GUI_SWITCHER_H

#include "widget.h"
#include "dialog.h"
#include "dialogRef.h"
#include <vector>

namespace gui
{

/* A container that displays any one of multiple widgets. Used to dynamically
 * change the layout, e.g. to switch between a TextField and a DropDownList
 * when some selection is changed.
 */
class Switcher: public Widget
{
public:
    Switcher();
    void add(std::shared_ptr<Widget> child);

    /* Makes the widget with the given index visible, hiding all others. */
    void switchTo(size_t index);

    /* Returns the index of the currently visible widget. */
    size_t getCurrentIndex() const;

    void setVisible(bool visible) override;

    /* Returns the widget at the given index, cast to the specified type. */
    template<typename T>
    std::shared_ptr<T> get(size_t index)
    {
        auto ret=std::dynamic_pointer_cast<T>(children.at(index).widget);
        assert(ret);
        return ret;
    }

private:
    /* Used to keep track of where in the array each child's DIALOGs are.
     * end is the index into alDialog of the last of the child's DIALOGs.
     */
    struct ChildInfo
    {
        std::shared_ptr<Widget> widget;
        int end;
    };

    std::vector<ChildInfo> children;
    DialogRef alDialog;
    size_t visibleChild;

    /* Sets or unsets D_HIDDEN for each of the child's DIALOGs. */
    void setChildVisible(size_t index, bool visible);
    void calculateSize() override;
    void arrange(int contX, int contY, int contW, int contH) override;
    void realize(DialogRunner& runner) override;
};

}

#endif
