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

void put_ffcwalkflags(BITMAP *dest, int32_t x, int32_t y)
{
	for(auto it = solid_objects.begin(); it != solid_objects.end(); ++it)
	{
		(*it)->putwalkflags(dest, x, y);
	}
}

static solid_object* curobject = NULL;

void setCurObject(solid_object* obj)
{
	curobject = obj;
}

solid_object* getCurObject()
{
	return curobject;
}

bool collide_object(solid_object const* obj)
{
	for(auto it = solid_objects.begin(); it != solid_objects.end(); ++it)
	{
		if (*it == obj || *it == curobject) continue;
		if ((*it)->collide(obj))
			return true;
	}
	return false;
}

bool collide_object(int32_t tx, int32_t ty, int32_t tw, int32_t th)
{
	for(auto it = solid_objects.begin(); it != solid_objects.end(); ++it)
	{
		if (*it == curobject) continue;
		if ((*it)->collide(tx, ty, tw, th))
			return true;
	}
	return false;
}

solid_object::solid_object() : solid(false), hxsz(16), hysz(16), hxofs(0), hyofs(0), solidflags(0)
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
	hxofs = other.hxofs;
	hyofs = other.hyofs;
	solid = other.solid;
	solidflags = other.solidflags;
	if (solid) solid_objects.push_back(this);
	else remove_object(this);
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

bool solid_object::collide(solid_object const* o)
{
	return collide(o->x + o->hxofs, o->y + o->hyofs, o->hxsz, o->hysz);
}

bool solid_object::collide(int32_t tx, int32_t ty, int32_t tw, int32_t th)
{
	return tx+tw>x+hxofs &&
		ty+th>y+hyofs &&
		tx<x+hxofs+hxsz &&
		ty<y+hyofs+hysz;
}

void solid_object::putwalkflags(BITMAP *dest, int32_t tx, int32_t ty)
{
	tx += x.getFloor() + hxofs;
	ty += y.getFloor() + hyofs;
	rectfill(dest, tx, ty, tx + hxsz-1, ty + hysz-1, makecol(255,85,85));
}
