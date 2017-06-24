#ifndef IENEMY_H
#define IENEMY_H

#include "../ObjectPool.h"

class enemy;

class IEnemy
{
public:
	IEnemy(ObjectPool &pool, int uid) : pool_(pool), uid_(uid) {}

	// stopgap for now so that current ffscript.cpp compiles
	enemy *getPtr();

private:
	ObjectPool &pool_;
	int uid_;
};

#endif