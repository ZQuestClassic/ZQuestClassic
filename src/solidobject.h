#ifndef _SOLIDOBJECT_H_
#define _SOLIDOBJECT_H_

#include "base/zdefs.h"
#include "zfix.h"

class solid_object
{
public:
	zfix x, y, vx, vy; 
	int32_t hxsz,hysz;
	
	solid_object();
	~solid_object();
	solid_object(solid_object const& other);
	void setSolid(bool set);
	bool getSolid() const;
private:
	bool solid;
};



#endif

