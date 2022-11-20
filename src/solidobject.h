#ifndef _SOLIDOBJECT_H_
#define _SOLIDOBJECT_H_

#include "zfix.h"

class solid_object
{
public:
	zfix x, y, vx, vy; 
	int32_t hxsz,hysz;
	
	solid_object();
	~solid_object();
	solid_object(solid_object const& other);
	void copy(solid_object const& other);
	solid_object& operator=(solid_object const& other);
	virtual void setSolid(bool set);
	bool getSolid() const;
private:
	bool solid;
};



#endif

