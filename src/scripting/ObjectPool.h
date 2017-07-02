#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <list>
#include <map>

class GameObject;

class ObjectPool
{
public:
	ObjectPool();
	int addToPool(GameObject *obj);
	void removeFromPool(int uid);

	GameObject *getFromUID(int uid);
	
private:
	int nextuid_;
	std::map<int, GameObject *> pool_;
};

class GameObject
{
public:
	GameObject(ObjectPool &pool);
	GameObject(const GameObject &other);

	virtual ~GameObject();

	int getUID() { return uid_; }
	
private:
	// not implemented
	GameObject &operator=(const GameObject &other);

	ObjectPool &pool_;
	int uid_;
};


#endif