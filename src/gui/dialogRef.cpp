#include "dialogRef.h"
#include "dialog.h"

namespace gui
{

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
