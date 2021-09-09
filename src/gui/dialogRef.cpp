#include "dialogRef.h"
#include "dialog.h"
#include "dialogRunner.h"

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

DIALOG& DialogRef::operator[](int offset)
{
	return owner->alDialog.at(index+offset);
}

const DIALOG& DialogRef::operator[](int offset) const
{
	return owner->alDialog.at(index+offset);
}

DialogRef::operator bool() const
{
	return owner;
}

}
