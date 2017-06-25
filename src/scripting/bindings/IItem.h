#ifndef IITEM_H
#define IITEM_H

#include "../ObjectPool.h"

class item;

class IItem
{
public:
	IItem(ObjectPool &pool, int uid) : pool_(pool), uid_(uid) {}

	// stopgap for now so that current ffscript.cpp compiles
	item *getPtr();

private:
	ObjectPool &pool_;
	int uid_;
};

#endif