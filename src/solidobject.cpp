#include "solidobject.h"
#include "base/zdefs.h"
#include "sprite.h"
#include "zc/hero.h"
#include "base/zc_math.h"
#include "slopes.h"
#include "base/mapscr.h"
#include "iter.h"

#ifdef IS_PLAYER
extern sprite_list guys;
extern HeroClass Hero;
#endif

static solid_object* curobject = NULL;

template <class F>
static bool for_every_solid_object(const F& fn)
{
#ifdef IS_PLAYER
	bool should_continue = true;
	for_some_ffcs_in_region([&](const ffc_handle_t& ffc_handle) {
		if (ffc_handle.ffc->flags & ffCHANGER) return true;

		solid_object* obj = ffc_handle.ffc;
		if (!obj->getSolid()) return true;

		if (fn(obj)) {
			should_continue = false;
			return false;
		}

		return true;
	});
	if (!should_continue) return true;

	for (int i = 0; i < guys.Count(); i++)
	{
		solid_object* obj = guys.spr(i);
		if (!obj->getSolid()) continue;
		if (fn(obj)) return true;
	}
	
	if(mblock2.getSolid())
	{
		if(fn(&mblock2)) return true;
	}
#endif

	return false;
}

void draw_solid_objects(BITMAP *dest, int32_t x, int32_t y, int32_t col)
{
	for_every_solid_object([&](solid_object* obj) {
		obj->draw(dest, x, y, col);
		return false;
	});
}
void draw_solid_objects_a5(int32_t x, int32_t y, ALLEGRO_COLOR col)
{
	for_every_solid_object([&](solid_object* obj) {
		obj->draw_a5(x, y, col);
		return false;
	});
}

bool collide_object(solid_object const* collide_with_obj)
{
	return for_every_solid_object([&](solid_object* obj) {
		if (collide_with_obj == obj) return false;
		return obj->collide(collide_with_obj);
	});
}

bool collide_object(zfix tx, zfix ty, zfix tw, zfix th, solid_object const* ign)
{
	return for_every_solid_object([&](solid_object* obj) {
		if (obj == ign || obj == curobject) return false;
		return obj->collide(tx, ty, tw, th);
	});
}

void setCurObject(solid_object* obj)
{
	curobject = obj;
}

solid_object* getCurObject()
{
	return curobject;
}

bool solid_object::setSolid(bool set)
{
	bool changed = solid != set;
	solid = set;
	// TODO is this still needed?
	return changed;
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
	               o->hit_width + o->sxsz_ofs,
	               o->hit_height + o->sysz_ofs);
}
bool solid_object::collide(zfix tx, zfix ty, zfix tw, zfix th) const
{
	if(ignore_solid_temp) return false;
	zfix rx = x+hxofs+sxofs, ry = y+hyofs+syofs;
	zfix rw = hit_width+sxsz_ofs, rh = hit_height+sysz_ofs;
	return tx+tw>rx && ty+th>ry &&
	       tx<rx+rw && ty<ry+rh;
}
bool solid_object::collide_old(zfix tx, zfix ty, zfix tw, zfix th) const
{
	if(ignore_solid_temp) return false;
	zfix rx = old_x+hxofs+sxofs, ry = old_y+hyofs+syofs;
	zfix rw = hit_width+sxsz_ofs, rh = hit_height+sysz_ofs;
	return tx+tw>rx && ty+th>ry &&
	       tx<rx+rw && ty<ry+rh;
}

void solid_object::draw(BITMAP *dest, int32_t tx, int32_t ty, int32_t col)
{
	if(ignore_solid_temp) return;
	tx += x.getFloor() + hxofs + sxofs;
	ty += y.getFloor() + hyofs + syofs;
	rectfill(dest, tx, ty, tx + hit_width-1 + sxsz_ofs,
	         ty + hit_height-1 + sysz_ofs, col);
}
void solid_object::draw_a5(int32_t tx, int32_t ty, ALLEGRO_COLOR col)
{
	if(ignore_solid_temp) return;
	tx += x.getFloor() + hxofs + sxofs;
	ty += y.getFloor() + hyofs + syofs;
	al_draw_filled_rectangle(tx, ty, tx + hit_width + sxsz_ofs,
	         ty + hit_height + sysz_ofs, col);
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
	old_x2 = old_x;
	old_y2 = old_y;
	old_x = x;
	old_y = y;
}
void solid_object::solid_push(solid_object* pusher)
{
	//Default behavior: Ignore
}

void solid_object::solid_push_int(solid_object const* obj,zfix& dx, zfix& dy, int32_t& hdir, bool can_platform)
{
	dx = dy = 0; hdir = -1;
	if(is_unpushable()) return;
	zfix odx = obj->x - obj->old_x,
	     ody = obj->y - obj->old_y,
	     obj_x = obj->x + obj->hxofs + obj->sxofs,
	     obj_y = obj->y + obj->hyofs + obj->syofs,
	     obj_ox = obj->old_x + obj->hxofs + obj->sxsz_ofs,
	     obj_oy = obj->old_y + obj->hyofs + obj->sysz_ofs;
	int32_t obj_w = obj->hit_width + obj->sxsz_ofs,
	        obj_h = obj->hit_height + obj->sysz_ofs;
	
	zfix rx = x+hxofs+sxofs, ry = y+hyofs+syofs,
		 rw = hit_width+sxsz_ofs, rh = hit_height+sysz_ofs;
	bool sideview_slim = false;
	#define ZF_TO_INT(zf) (zf).getFloor()
	
	if(sideview_mode())
	{
		bool ride_platform = false;
		if(can_platform && ZF_TO_INT(ry+rh) == ZF_TO_INT(obj_oy)) //correct y
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

			int SCL = 1;
			// zfix maxd = zc_max(abs(odx), abs(ody));
			// if (maxd > 1000)
				// SCL = maxd / 20;

			int maxreps = 1000;
			while (true)
			{
				if (--maxreps < 0) break;
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
						ry -= SCL;
						break;
					}
					case down:
					{
						ry += SCL;
						break;
					}
					case left:
					{
						rx -= SCL;
						break;
					}
					case right:
					{
						rx += SCL;
						break;
					}
					case l_up:
					{
						rx -= SCL;
						ry -= SCL;
						break;
					}
					case r_up:
					{
						rx += SCL;
						ry -= SCL;
						break;
					}
					case l_down:
					{
						rx -= SCL;
						ry += SCL;
						break;
					}
					case r_down:
					{
						rx += SCL;
						ry += SCL;
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
	if(dx || dy)
		hdir = XY_DIR(GET_YDIR(dy), GET_XDIR(dx));
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

