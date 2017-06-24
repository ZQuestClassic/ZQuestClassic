#include "ObjectPool.h"
#include <cassert>

using namespace std;

ObjectPool::ObjectPool() : nextuid_(0)
{	
}

int ObjectPool::addToPool(GameObject *obj)
{
	int uid = nextuid_++;
	if (nextuid_ == 0)
	{
		assert(!"Ran out of UIDs");
	}
	pool_[uid] = obj;
	return uid;
}

void ObjectPool::removeFromPool(int uid)
{
	map<int, GameObject *>::iterator it = pool_.find(uid);
	if (it != pool_.end())
		pool_.erase(it);
}

GameObject *ObjectPool::getFromUID(int uid)
{
	return pool_[uid];
}



GameObject::GameObject(ObjectPool &pool) : pool_(pool)
{
	uid_ = pool_.addToPool(this);
}

GameObject::GameObject(const GameObject &other) : pool_(other.pool_)
{
	uid_ = pool_.addToPool(this);
}

GameObject::~GameObject()
{
	pool_.removeFromPool(uid_);
}