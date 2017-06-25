#include "IWeapon.h"

#include "../../weapons.h"

weapon *IWeapon::getPtr()
{
	GameObject *obj = pool_.getFromUID(uid_);
	if (!obj)
		return NULL;
	weapon *result = dynamic_cast<weapon *>(obj);
	return result;
}