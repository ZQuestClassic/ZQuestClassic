#include "solidobject.h"
#include "base/zdefs.h"
#include "sprite.h"
#include "hero.h"
#include "base/zc_math.h"

#ifdef IS_PLAYER
extern sprite_list guys;
extern HeroClass Hero;
#endif

using std::vector;

vector<solid_object*> solid_objects;
vector<slopedata> slopes;

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
	for(slopedata& s : slopes)
	{
		line(dest, x+s.x1, y+s.y1, x+s.x2, y+s.y2, makecol(255,85,85));
		if(s.slope > 0)
		{
			line(dest, x+s.x1, y+s.y1+1, x+s.x2-1, y+s.y2, makecol(255,85,85));
			line(dest, x+s.x1+1, y+s.y1, x+s.x2, y+s.y2-1, makecol(255,85,85));
		}
		else if(s.slope < 0)
		{
			line(dest, x+s.x1, y+s.y1-1, x+s.x2-1, y+s.y2, makecol(255,85,85));
			line(dest, x+s.x1+1, y+s.y1, x+s.x2, y+s.y2+1, makecol(255,85,85));
		}
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

bool check_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th)
{
	for(auto it = slopes.begin(); it != slopes.end(); ++it)
	{
		if (lineBoxCollision((*it).x1, (*it).y1, (*it).x2, (*it).y2, tx, ty, tw, th)) return true;
	}
	return false;
}

slopedata get_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th)
{
	for(auto it = slopes.begin(); it != slopes.end(); ++it)
	{
		if (lineBoxCollision((*it).x1, (*it).y1, (*it).x2, (*it).y2, tx, ty, tw, th)) return (*it);
	}
	slopedata s;
	return s;
}

bool check_slope(solid_object const* o)
{
	return check_slope(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hxsz + o->sxsz_ofs,
	               o->hysz + o->sysz_ofs);
}

slopedata get_slope(solid_object const* o)
{
	return get_slope(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hxsz + o->sxsz_ofs,
	               o->hysz + o->sysz_ofs);
}

void slope_push_int(slopedata s, solid_object* obj, zfix& dx, zfix& dy)
{
	zfix rx = obj->x+obj->hxofs+obj->sxofs, ry = obj->y+obj->hyofs+obj->syofs,
	rw = obj->hxsz+obj->sxsz_ofs, rh = obj->hysz+obj->sysz_ofs;
	zfix cx = rx + rw/2 - 1;
	zfix cy = ry + rh/2 - 1;
	double lineangle = atan2(double(s.y2-s.y1),double(s.x2-s.x1));
	double val = comparePointLine(cx, cy, s.x1, s.y1, s.x2, s.y2);
	if (sign(val) < 0)
	{
		lineangle -= PI/2;
	}
	else
	{
		lineangle += PI/2;
	}
	zfix orx = rx;
	zfix ory = ry;
	while(lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry, rw, rh))
	{
		rx += zc::math::Cos(lineangle);
		ry += zc::math::Sin(lineangle);
	}
	dx = (rx - orx);
	dy = (ry - ory);
}

solid_object::solid_object() : solid(false), in_solid_arr(false),
	ignore_solid_temp(false),
	hxsz(16), hysz(16), hxofs(0), hyofs(0),
	sxofs(0), syofs(0), sxsz_ofs(0), sysz_ofs(0),
	solidflags(0)
{}

solid_object::~solid_object()
{
	if(in_solid_arr)
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
}

void solid_object::solid_push_int(solid_object const* obj,zfix& dx, zfix& dy, int32_t& hdir)
{
	dx = dy = 0;
	if(is_unpushable()) return;
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
	bool sideview_slim = false;
	#define ZF_TO_INT(zf) (zf).getFloor()
	
	if(sideview_mode())
	{
		bool ride_platform = false;
		if(ZF_TO_INT(ry+rh) == ZF_TO_INT(obj_oy)) //correct y
		{
			int32_t cx1 = ZF_TO_INT(obj_ox), cx2 = ZF_TO_INT(obj_ox + obj_w);
			if(ZF_TO_INT(rw) > 4) //sanity
			{
				int32_t x1 = ZF_TO_INT(rx+4), x2 = ZF_TO_INT(rx+rw-4);
				if((x1 > cx1 && x1 < cx2)
				 || (x2 > cx1 && x2 < cx2)) //correct x
				{
					ride_platform = true;
				}
			}
			else
			{
				int32_t x1 = ZF_TO_INT(rx+rw/2);
				if(x1 > cx1 && x1 < cx2) //correct x
				{
					ride_platform = true;
				}
			}
		}
		if(ride_platform)
		{
			dx = odx;
			dy = ody;
			return;
		}
		if(ody && (!odx || ody < 0))
		{
			//Sideview solids use a 4-slimmer hitbox in some cases
			sideview_slim = true;
			sxofs += 4;
			sxsz_ofs -= 8;
		}
	}
	
	if(odx && ody)
	{
		//first lets see if the object is even in range; if not we can skip all these checks
		if (collide((odx > 0?obj_ox:obj_x), (ody > 0 ? obj_oy : obj_y), abs(odx)+obj_w, abs(ody)+obj_h))
		{
			//we're grabbing the corners of the object. These coordinates assume a rotrect either going upright or downleft.
			zfix leftx = obj_x;
			zfix topy = obj_y;
			zfix todx = odx;
			zfix ox, oy, nx, ny;
			zfix rightx = leftx + obj_w - 1;
			zfix bottomy = topy + obj_h - 1;
			if (odx > 0)
			{
				ox = leftx;
				nx = rightx;
			}
			else
			{
				ox = rightx;
				nx = leftx;
			}
			if (ody > 0)
			{
				oy = topy;
				ny = bottomy;
			}
			else
			{
				oy = bottomy;
				ny = topy;
			}
			if (ody > 0)
			{
				todx = -odx;
				leftx = obj_ox;
				topy = obj_oy;
			}
			zfix centerx = leftx + (obj_w/2)-1;
			zfix centery = topy + (obj_h/2)-1;
			rightx = leftx + obj_w - 1;
			bottomy = topy + obj_h - 1;

			
			//however, depending on the direction, we want to use different corners of the object.
			//thankfully, we can tell the direction by getting the signs of the velocity. if they're both equal, it's upleft and downright instead.
			
			zfix lefty = (sign(odx) == sign(ody)) ? bottomy : topy;
			zfix righty = (sign(odx) == sign(ody)) ? topy : bottomy;
			zfix side = 0;
			byte pdir = l_up; //topleft
			byte pdir2 = 0; //the dir the thing will actually be pushed in
			if (odx >= 0 && ody < 0) pdir = r_up; //topright
			if (odx < 0 && ody >= 0) pdir = l_down; //bottomleft
			if (odx >= 0 && ody >= 0) pdir = r_down; //bottomright
			zfix orx = rx;
			zfix ory = ry;
			while (true)
			{
				bool check = true;
				side = 0;
				if (lineBoxCollision(leftx, lefty, leftx+todx, lefty+abs(ody), rx, ry, rw, rh)) 
				{
					--side;
					check = false;
				}
				if (lineBoxCollision(rightx, righty, rightx+todx, righty+abs(ody), rx, ry, rw, rh))
				{
					++side;
					check = false;
				}
				if (check)
				{
					if (insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, leftx, lefty, leftx+todx, lefty+abs(ody), centerx, centery, centerx+todx, centery+abs(ody)))
					{
						--side;
						check = false;
					}
					if (insideRotRect(rx+((rw)/2)-1, ry+((rh)/2)-1, centerx, centery, centerx+todx, centery+abs(ody), rightx, righty, rightx+todx, righty+abs(ody)))
					{
						++side;
						check = false;
					}
				}
				if (check)
				{
					if (obj->collide(rx, ry, rw, rh))
					{
						double val = comparePointLine(rx+rw/2, ry+rh/2, ox, oy, nx, ny);
						if (abs(val) > 6) 
						{
							switch(pdir)
							{
								case r_up:
								case l_down:
									side = sign(val);
									break;
								case l_up:
								case r_down:
									side = -sign(val);
									break;
							}
						}
					}
					else break;
				}
				if (lineBoxCollision(ox, oy, nx, ny, rx+(rw/4), ry+(rh/4), rw/2, rh/2))
				{
					side = 0;
				}
				switch(pdir)
				{
					case l_up:
					{
						if (side < 0) pdir2 = left;
						else if (side > 0) pdir2 = up;
						else pdir2 = l_up;
						break;
					}
					case r_up:
					{
						if (side < 0) pdir2 = up;
						else if (side > 0) pdir2 = right;
						else pdir2 = r_up;
						break;
					}
					case l_down:
					{
						if (side < 0) pdir2 = left;
						else if (side > 0) pdir2 = down;
						else pdir2 = l_down;
						break;
					}
					case r_down:
					{
						if (side < 0) pdir2 = down;
						else if (side > 0) pdir2 = right;
						else pdir2 = r_down;
						break;
					}
					default:
						break;
				}
				switch(pdir2)
				{
					case up:
					{
						--ry;
						break;
					}
					case down:
					{
						++ry;
						break;
					}
					case left:
					{
						--rx;
						break;
					}
					case right:
					{
						++rx;
						break;
					}
					case l_up:
					{
						--rx;
						--ry;
						break;
					}
					case r_up:
					{
						++rx;
						--ry;
						break;
					}
					case l_down:
					{
						--rx;
						++ry;
						break;
					}
					case r_down:
					{
						++rx;
						++ry;
						break;
					}
					default:
						break;
				}
			}
			dx = rx - orx;
			dy = ry - ory;
			// if (collide(obj_x, obj_y, obj_w, obj_h))
			// {
				
				// if (abs(obj_x+sxofs - rx) > abs(obj_y + ((ry - obj_y) < 0? syofs:0) - ry))
				// {
					// if ((rx - obj_x) > 0) dx = obj_x + obj_w - rx;
					// else dx = obj_x - rw - rx;
				// }
				// else
				// {
					// if ((ry - obj_y) > 0) dy = obj_y + obj_h - ry;
					// else dy = obj_y - rh - ry;
				// }
			// }
		}
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
	if(sideview_slim)
	{
		sxofs -= 4;
		sxsz_ofs += 8;
	}
}

int32_t solid_object::push_dir() const
{
	zfix odx = x - old_x,
	     ody = y - old_y;
	int32_t xdir = -1, ydir = -1;
	if(odx > 0) xdir = right;
	else if(odx < 0) xdir = left;
	if(ody > 0) xdir = down;
	else if(ody < 0) xdir = up;
	return XY_DIR(xdir, ydir);
}

