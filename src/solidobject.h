#ifndef _SOLIDOBJECT_H_
#define _SOLIDOBJECT_H_

#include "zfix.h"
#include "base/zc_alleg.h"

class solid_object;

bool collide_object(solid_object const* obj);
bool collide_object(int32_t tx, int32_t ty, int32_t tw, int32_t th);
void put_ffcwalkflags(BITMAP *dest, int32_t x, int32_t y);

class solid_object
{
public:
	zfix x, y, vx, vy; 
	int32_t hxsz,hysz,hxofs,hyofs;
	
	solid_object();
	~solid_object();
	solid_object(solid_object const& other);
	virtual void copy(solid_object const& other);
	solid_object& operator=(solid_object const& other);
	virtual void setSolid(bool set);
	bool getSolid() const;
	bool collide(solid_object const* other);
	bool collide(int32_t tx, int32_t ty, int32_t tw, int32_t th);
	void putwalkflags(BITMAP *dest, int32_t tx, int32_t ty);
protected:
	bool solid;
};



#endif

