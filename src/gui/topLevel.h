#ifndef ZC_GUI_TOPLEVEL_H
#define ZC_GUI_TOPLEVEL_H

#include "widget.h"
#include "key.h"
#include <vector>

struct DIALOG;

namespace gui
{

class DialogRunner;

class TopLevelWidget: public Widget
{
public:
    virtual ~TopLevelWidget() {}

    /* Add a keyboard shortcut. */
    template<typename T>
    inline void onKey(key::Key k, T message)
    {
        shortcuts.emplace_back(Shortcut {
            k.get(), static_cast<int>(message)
        });
    }

protected:
    /* Insert keyboard shortcut DIALOGs into the array. This should be called
     * by subclasses after inserting their own and their children's DIALOGs.
     */
    void realizeKeys(DialogRunner& runner);
    int onEvent(int event, MessageDispatcher sendMessage) override;

private:
    struct Shortcut
    {
        int key;
        int message;
    };

    std::vector<Shortcut> shortcuts;

    static int proc(int msg, DIALOG* d, int c);
};

}

#endif
