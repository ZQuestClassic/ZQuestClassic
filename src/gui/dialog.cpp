#include "dialog.h"

using std::shared_ptr;

namespace gui
{

void DialogRunner::push(shared_ptr<Widget> owner, DIALOG dlg)
{
    widgets.push_back(owner);
    alDialog.push_back(std::move(dlg));
}

void DialogRunner::realize(shared_ptr<Widget> root)
{
    root->arrange(0, 0, 800, 600);
    root->realize(*this);
    alDialog.push_back({
        nullptr, // proc
        0, 0, 0, 0, // x, y, width, height
        0, 0, // fg, bg
        0, // key
        0, // flags
        0, 0, // d1, d2
        nullptr, nullptr, nullptr // dp1, dp2, dp3
    });
}

DialogRef DialogRunner::getAllegroDialog()
{
    size_t index=alDialog.size()-1;
    return DialogRef(this, index);
}



DialogRef::DialogRef(): owner(nullptr)
{}

DialogRef::DialogRef(DialogRunner* owner, size_t index): owner(owner),
    index(index)
{}

DIALOG* DialogRef::operator->()
{
    return &owner->alDialog[index];
}

const DIALOG* DialogRef::operator->() const
{
    return &owner->alDialog[index];
}

DialogRef::operator bool() const
{
    return owner;
}

}
