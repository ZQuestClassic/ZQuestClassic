#include "solidobject.h"
#include "base/zdefs.h"
#include "sprite.h"
#include "hero.h"

#ifdef IS_PLAYER
extern sprite_list guys;
extern HeroClass Hero;
#endif

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

bool collide_object(int32_t tx, int32_t ty, int32_t tw, int32_t th, solid_object const* ign)
{
	for(auto it = solid_objects.begin(); it != solid_objects.end(); ++it)
	{
		if (*it == ign || *it == curobject) continue;
		if ((*it)->collide(tx, ty, tw, th))
			return true;
	}
	return false;
}

solid_object::solid_object() : solid(false), in_solid_arr(false),
	hxsz(16), hysz(16), hxofs(0), hyofs(0),
	sxofs(0), syofs(0), sxsz_ofs(0), sysz_ofs(0),
	solidflags(0)
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
	old_x = other.old_x;
	old_y = other.old_y;
	vx = other.vx;
	vy = other.vy;
	hxsz = other.hxsz;
	hysz = other.hysz;
	hxofs = other.hxofs;
	hyofs = other.hyofs;
	sxofs = other.sxofs;
	syofs = other.syofs;
	sxsz_ofs = other.sxsz_ofs;
	sysz_ofs = other.sysz_ofs;
	solidflags = other.solidflags;
	setSolid(other.solid);
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

bool solid_object::setSolid(bool set)
{
	solid = set;
	if(solid && !in_solid_arr)
	{
		solid_objects.push_back(this);
		in_solid_arr = true;
		return true;
	}
	else if(in_solid_arr && !solid)
	{
		remove_object(this);
		in_solid_arr = false;
		return true;
	}
	return false;
}
bool solid_object::getSolid() const
{
	return solid;
}
void solid_object::updateSolid()
{
	if(setSolid(solid))
		solid_update(false);
}
void solid_object::setTempNonsolid(bool set)
{
	ignore_solid_temp = set;
}
bool solid_object::getTempNonsolid() const
{
	return ignore_solid_temp;
}

bool solid_object::collide(solid_object const* o) const
{
	if(ignore_solid_temp) return false;
	return collide(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hxsz + o->sxsz_ofs,
	               o->hysz + o->sysz_ofs);
}
bool solid_object::collide(int32_t tx, int32_t ty, int32_t tw, int32_t th) const
{
	if(ignore_solid_temp) return false;
	int32_t rx = x+hxofs+sxofs, ry = y+hyofs+syofs;
	int32_t rw = hxsz+sxsz_ofs, rh = hysz+sysz_ofs;
	return tx+tw>rx && ty+th>ry &&
	       tx<rx+rw && ty<ry+rh;
}

void solid_object::putwalkflags(BITMAP *dest, int32_t tx, int32_t ty)
{
	if(ignore_solid_temp) return;
	tx += x.getFloor() + hxofs + sxofs;
	ty += y.getFloor() + hyofs + syofs;
	rectfill(dest, tx, ty, tx + hxsz-1 + sxsz_ofs,
	         ty + hysz-1 + sysz_ofs, makecol(255,85,85));
}

void solid_object::solid_update(bool push)
{
#ifdef IS_PLAYER
	if(push && solid)
	{
		if(x != old_x || y != old_y)
		{
			Hero.solid_push(this);
			guys.solid_push(this);
		}
	}
#endif
	old_x = x;
	old_y = y;
}
void solid_object::solid_push(solid_object* pusher)
{
	//Default behavior: Ignore
	//!TODO SOLIDPUSH Implement 'enemy::solid_push'
	//!TODO SOLIDPUSH finish 'HeroClass::solid_push' (good for 4-dir, needs diagonals)
}

void solid_object::solid_push_int(solid_object const* obj,zfix& dx, zfix& dy) const
{
	dx = dy = 0;
	zfix odx = obj->x - obj->old_x,
	     ody = obj->y - obj->old_y,
	     obj_x = obj->x + obj->hxofs + obj->sxofs,
	     obj_y = obj->y + obj->hyofs + obj->syofs,
	     obj_ox = obj->old_x + obj->hxofs + obj->sxsz_ofs,
	     obj_oy = obj->old_y + obj->hyofs + obj->sysz_ofs;
	int32_t obj_w = obj->hxsz + obj->sxsz_ofs,
	        obj_h = obj->hysz + obj->sysz_ofs;
	
	zfix rx = x+hxofs+sxofs, ry = y+hyofs+syofs,
		 rw = hxsz+sxsz_ofs, rh = hysz+sysz_ofs;
	
	if(odx && ody)
	{
		//!TODO SOLIDPUSH Diagonal Movement pushing
		return;
	}
	else if(odx)
	{
		if(odx > 0) //right
		{
			if(collide(obj_ox+obj_w, obj_oy, odx, obj_h)) //collided
			{
				dx = obj_x + obj_w - rx;
			}
		}
		else //left
		{
			if(collide(obj_x, obj_oy, -odx, obj_h)) //collided
			{
				dx = obj_x - rw - rx;
			}
		}
	}
	else if(ody)
	{
		if(ody > 0) //down
		{
			if(collide(obj_ox, obj_oy+obj_h, obj_w, ody)) //collided
			{
				dy = obj_y + obj_h - ry;
			}
		}
		else //up
		{
			if(collide(obj_ox, obj_y, obj_w, -ody)) //collided
			{
				dy = obj_y - rh - ry;
			}
		}
	}
}

