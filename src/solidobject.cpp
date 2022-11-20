#include "solidobject.h"
#include "base/zdefs.h"

using std::vector;

vector<solid_object*> solid_objects;

bool remove_object(solid_object* obj)
{
    bool ret = false;
    for(auto it = solid_objects.begin(); it != solid_objects.end();)
    {
        if(*it == obj)
        {
            it = solid_objects.erase(it);
            // return true; //don't keep iterating - optimization
            ret = true;
        }
        else
            ++it;
    }
    return ret;
}

solid_object::solid_object() : solid(false), hxsz(16), hysz(16)
{}

solid_object::~solid_object()
{
	if(solid)
	{
	    remove_object(this);
	}
}

void solid_object::copy(solid_object const& other)
{
	x = other.x;
	y = other.y;
	vx = other.vx;
	vy = other.vy;
	hxsz = other.hxsz;
	hysz = other.hysz;
	solid = other.solid;
	if (solid) solid_objects.push_back(this);
}

solid_object::solid_object(solid_object const& other) 
{
	copy(other);
}

solid_object& solid_object::operator=(solid_object const& other)
{
	copy(other);
	return *this;
}

void solid_object::setSolid(bool set)
{
	if (set == solid) return;
	if (solid = set)
	{
		solid_objects.push_back(this);
	}
	else
	{
		remove_object(this);
	}
}

bool solid_object::getSolid() const
{
	return solid;
}

