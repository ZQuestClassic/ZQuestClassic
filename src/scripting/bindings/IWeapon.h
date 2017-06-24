#ifndef IWEAPON_H
#define IWEAPON_H

#include "../ObjectPool.h"

class weapon;

class IWeapon
{
public:
	IWeapon(ObjectPool &pool, int uid) : pool_(pool), uid_(uid) {}

	// stopgap for now so that current ffscript.cpp compiles
	weapon *getPtr();

private:
	ObjectPool &pool_;
	int uid_;
};

#endif