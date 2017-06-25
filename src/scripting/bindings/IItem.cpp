#include "IItem.h"

#include "../../items.h"

item *IItem::getPtr()
{
	GameObject *obj = pool_.getFromUID(uid_);
	if (!obj)
		return NULL;
	item *result = dynamic_cast<item *>(obj);
	return result;
}