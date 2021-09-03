#ifndef ZC_GUI_DIALOGREF_H
#define ZC_GUI_DIALOGREF_H

#include <cstddef>
struct DIALOG;

namespace gui
{

class DialogRunner;
/* References an item in the DialogRunner's DIALOG array, which isn't safe
 * to do directly because it might be reallocated as the vector grows.
 */
class DialogRef
{
public:
    DialogRef();
    DIALOG* operator->();
    const DIALOG* operator->() const;
    operator bool() const;

private:
    DialogRunner* owner;
    size_t index;

    DialogRef(DialogRunner* owner, size_t index);

    friend class DialogRunner;
};

}

#endif
