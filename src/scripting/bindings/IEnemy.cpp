#include "IEnemy.h"

#include "../../guys.h"

enemy *IEnemy::getPtr()
{
	GameObject *obj = pool_.getFromUID(uid_);
	if (!obj)
		return NULL;
	enemy *result = dynamic_cast<enemy *>(obj);
	return result;
}