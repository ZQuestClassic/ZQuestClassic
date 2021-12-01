#include "dialog_ref.h"
#include "dialog.h"
#include "dialog_runner.h"

namespace GUI
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

DIALOG& DialogRef::operator*()
{
	return owner->alDialog[index];
}

DIALOG& DialogRef::operator[](int32_t offset)
{
	return owner->alDialog.at(index+offset);
}

const DIALOG& DialogRef::operator[](int32_t offset) const
{
	return owner->alDialog.at(index+offset);
}

void DialogRef::message(int32_t msg, int32_t c)
{
	object_message(&owner->alDialog[index], msg, c);
}

void DialogRef::applyVisibility(bool visible, int32_t offs)
{
	if(visible)
	{
		owner->alDialog[index+offs].flags &= ~D_HIDDEN;
	}
	else
	{
		owner->alDialog[index+offs].flags |= D_HIDDEN;
	}
}

void DialogRef::applyDisabled(bool dis, int32_t offs)
{
	if(dis)
	{
		owner->alDialog[index+offs].flags |= D_DISABLED;
	}
	else
	{
		owner->alDialog[index+offs].flags &= ~D_DISABLED;
	}
}

}
