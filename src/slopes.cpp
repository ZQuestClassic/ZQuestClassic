#include "base/zdefs.h"
#include "slopes.h"
#include "zc/maps.h"
#include "base/zc_math.h"
#include "sprite.h"
#include "base/combo.h"

std::map<int32_t, slope_object> slopes;

slope_info::slope_info(newcombo const& cmb, zfix const& xoffs, zfix const& yoffs)
	: cmb(&cmb)
{
	x1 = xoffs + int32_t(cmb.attrishorts[0]);
	y1 = yoffs + int32_t(cmb.attrishorts[1]);
	x2 = xoffs + int32_t(cmb.attrishorts[2]);
	y2 = yoffs + int32_t(cmb.attrishorts[3]);
	
	if(x1 > x2)
	{
		zc_swap(x1,x2);
		zc_swap(y1,y2);
	}
}

slope_info::slope_info()
	: cmb(nullptr)
{}

zfix slope_info::getX(zfix const& y) const
{
	zfix b = y1 - slope() * x1;
	return (y - b)/slope();
}
zfix slope_info::getY(zfix const& x) const
{
	zfix b = y1 - slope() * x1;
	return (x*slope())+b;
}

void slope_info::draw(BITMAP* dest, int32_t x, int32_t y, int32_t col) const
{
	if(!cmb) return;
	line(dest, x+x1, y+y1, x+x2, y+y2, col);
	zfix const& sl = slope();
	if(sl > 0)
	{
		line(dest, x+x1-1, y+y1, x+x2, y+y2+1, col);
		line(dest, x+x1, y+y1-1, x+x2+1, y+y2, col);
	}
	else if(sl < 0)
	{
		line(dest, x+x1-1, y+y1, x+x2, y+y2-1, col);
		line(dest, x+x1, y+y1+1, x+x2+1, y+y2, col);
	}
}
void slope_info::draw_a5(int32_t x, int32_t y, ALLEGRO_COLOR col) const
{
	if(!cmb) return;
	al_draw_line(x+x1, y+y1, x+x2, y+y2, col, 1);
	zfix const& sl = slope();
	if(sl > 0)
	{
		al_draw_line(x+x1-1, y+y1, x+x2, y+y2+1, col, 1);
		al_draw_line(x+x1, y+y1-1, x+x2+1, y+y2, col, 1);
	}
	else if(sl < 0)
	{
		al_draw_line(x+x1-1, y+y1, x+x2, y+y2-1, col, 1);
		al_draw_line(x+x1, y+y1+1, x+x2+1, y+y2, col, 1);
	}
}

void draw_slopes(BITMAP *dest, int32_t x, int32_t y, int32_t col)
{
	for(auto& p : slopes)
		p.second.get_info().draw(dest,x,y,col);
}
void draw_slopes_a5(int32_t x, int32_t y, ALLEGRO_COLOR col)
{
	for(auto& p : slopes)
		p.second.get_info().draw_a5(x,y,col);
}

slope_info slope_object::get_info() const
{
	bool ff = ffc && ffc->getLoaded();
	word const* id = ff ? &ffc->getData() : cmbid;
	if(!id) return slope_info();
	newcombo const& cmb = combobuf[*id];
	if(cmb.type != cSLOPE)
		return slope_info();
	
	return slope_info(cmb, ff ? ffc->x : xoffs, ff ? ffc->y : yoffs);
}

void slope_object::updateslope()
{
	slope_info const& inf = get_info();
	ox1 = inf.x1;
	oy1 = inf.y1;
	ox2 = inf.x2;
	oy2 = inf.y2;
}

slope_object::slope_object(word* cid, ffcdata* ff, int32_t id, word cpos)
	: cmbid(cid), ffc(ff), id(id)
{
	if(ffc)
	{
		xoffs = ffc->x;
		yoffs = ffc->y;
		cmbid = &(ffc->getData());
	}
	else
	{
		xoffs = COMBOX(cpos);
		yoffs = COMBOY(cpos);
	}
}

zfix slope_info::slope()        const { return (y2-y1)/(x2-x1); }
zfix slope_info::slipperiness() const { return cmb ? zslongToFix(cmb->attributes[0]) : zfix(0); }
bool slope_info::stairs()       const { return cmb && (cmb->usrflags & cflag1); }
bool slope_info::ignorebottom() const { return cmb && (cmb->usrflags & cflag2); }
bool slope_info::ignoretop()    const { return cmb && (cmb->usrflags & cflag3); }
bool slope_info::ignoreleft()   const { return cmb && (cmb->usrflags & cflag4); }
bool slope_info::ignoreright()  const { return cmb && (cmb->usrflags & cflag5); }
bool slope_info::falldown()     const { return cmb && (cmb->usrflags & cflag6); }
bool slope_info::ignore(double lineangle, bool canfall, bool onplatform) const
{
	if(!cmb) return true;
	auto sinangle = zc::math::Sin(lineangle);
	auto cosangle = zc::math::Cos(lineangle);
	if (sinangle < 0 && ((onplatform && stairs())
		|| ignoretop() || (canfall && falldown())))
		return true;
	if (sinangle > 0 && (ignorebottom() || stairs()))
		return true;
	if (cosangle < 0 && ignoreleft())
		return true;
	if (cosangle > 0 && ignoreright())
		return true;
	return false;
}

int32_t check_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th, bool fallthrough)
{
	for (auto const& p : slopes)
	{
		slope_info const& s = p.second.get_info();
		if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, tx, ty, tw, th))
		{
			zfix cx = tx + tw/2 - 1;
			zfix cy = ty + th/2 - 1;
			double lineangle = atan2(double(s.y2-s.y1),double(s.x2-s.x1));
			double val = comparePointLine(cx, cy, s.x1, s.y1, s.x2, s.y2);
			if (val < 0)
			{
				lineangle -= PI/2;
			}
			else
			{
				lineangle += PI/2;
			}
			if(s.ignore(lineangle, fallthrough)) continue;
			int32_t ret = sign(zc::math::Sin(lineangle));
			return ret?ret:1;
		}
	}
	return 0;
}

int32_t check_new_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th, int32_t otx, int32_t oty, bool fallthrough)
{
	for(auto const& p : slopes)
	{
		slope_object const& obj = p.second;
		slope_info const& s = obj.get_info();
		if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, tx, ty, tw, th) && !lineBoxCollision(obj.ox1, obj.oy1, obj.ox2, obj.oy2, otx, oty, tw, th))
		{
			zfix cx = tx + tw/2 - 1;
			zfix cy = ty + th/2 - 1;
			double lineangle = atan2(double(s.y2-s.y1),double(s.x2-s.x1));
			double val = comparePointLine(cx, cy, s.x1, s.y1, s.x2, s.y2);
			if (val < 0)
			{
				lineangle -= PI/2;
			}
			else
			{
				lineangle += PI/2;
			}
			if(s.ignore(lineangle, fallthrough)) continue;
			int32_t ret = sign(zc::math::Sin(lineangle));
			return ret?ret:1;
		}
	}
	return 0;
}

slope_object const& get_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th)
{
	for (auto const& p : slopes)
	{
		slope_object const& obj = p.second;
		slope_info const& s = obj.get_info();
		if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, tx, ty, tw, th)) 
		{
			zfix cx = tx + tw/2 - 1;
			zfix cy = ty + th/2 - 1;
			double lineangle = atan2(double(s.y2-s.y1),double(s.x2-s.x1));
			double val = comparePointLine(cx, cy, s.x1, s.y1, s.x2, s.y2);
			if (val < 0)
			{
				lineangle -= PI/2;
			}
			else
			{
				lineangle += PI/2;
			}
			if(s.ignore(lineangle)) continue;
			return obj;
		}
	}
	static slope_object obj;
	return obj;
}

slope_object const& get_new_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th, int32_t otx, int32_t oty)
{
	for(auto const& p : slopes)
	{
		slope_object const& obj = p.second;
		slope_info const& s = obj.get_info();
		if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, tx, ty, tw, th)
			&& !lineBoxCollision(obj.ox1, obj.oy1, obj.ox2, obj.oy2, otx, oty, tw, th))
		{
			zfix cx = tx + tw/2 - 1;
			zfix cy = ty + th/2 - 1;
			double lineangle = atan2(double(s.y2-s.y1),double(s.x2-s.x1));
			double val = comparePointLine(cx, cy, s.x1, s.y1, s.x2, s.y2);
			if (val < 0)
			{
				lineangle -= PI/2;
			}
			else
			{
				lineangle += PI/2;
			}
			if(s.ignore(lineangle)) continue;
			return obj;
		}
	}
	static slope_object obj;
	return obj;
}

int32_t check_slope(solid_object* o, bool onlyNew)
{
	if (onlyNew) return check_new_slope(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hit_width + o->sxsz_ofs,
	               o->hit_height + o->sysz_ofs,
		       o->old_x + o->hxofs + o->sxofs,
	               o->old_y + o->hyofs + o->syofs);
	else return check_slope(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hit_width + o->sxsz_ofs,
	               o->hit_height + o->sysz_ofs);
}

slope_object const& get_slope(solid_object* o, bool onlyNew)
{
	if (!onlyNew) return get_slope(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hit_width + o->sxsz_ofs,
	               o->hit_height + o->sysz_ofs);
	else return get_new_slope(o->x + o->hxofs + o->sxofs,
	               o->y + o->hyofs + o->syofs,
	               o->hit_width + o->sxsz_ofs,
	               o->hit_height + o->sysz_ofs,
		       o->old_x + o->hxofs + o->sxofs,
	               o->old_y + o->hyofs + o->syofs);
}

bool slide_slope(solid_object* obj, zfix& dx, zfix& dy, zfix& ID)
{
	if (!obj->sideview_mode()) return false;
	zfix tx = obj->x+obj->hxofs+obj->sxofs, ty = obj->y+obj->hyofs+obj->syofs,
		tw = obj->hit_width+obj->sxsz_ofs, th = obj->hit_height+obj->sysz_ofs;
		
	dx = dy = 0;
	zfix otx = tx, oty = ty;
	
	for (auto const& p : slopes)
	{
		slope_object const& obj = p.second;
		slope_info const& s = obj.get_info();
		if (s.stairs() && ID != 0 && s.slope() != ID) continue;
		if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, tx, ty+1, tw, th))
		{
			zfix cx = tx + tw/2 - 1;
			zfix cy = ty + th/2 - 1;
			double lineangle = atan2(double(s.y2-s.y1),double(s.x2-s.x1));
			double val = comparePointLine(cx, cy, s.x1, s.y1, s.x2, s.y2);
			if (val < 0)
			{
				lineangle -= PI/2;
			}
			else
			{
				lineangle += PI/2;
			}
			auto sinangle = zc::math::Sin(lineangle);
			auto cosangle = zc::math::Cos(lineangle);
			if (sign(sinangle) <= 0)
			{
				dx += (s.x1 - obj.ox1);
				dy += (s.y1 - obj.oy1);
				ID = s.slope();
				if (s.slipperiness())
				{
					int32_t xdir = sign(cosangle);
					if (xdir)
					{
						ty += s.slipperiness();
						int32_t my = zc_min(s.y1, s.y2);
						if (int32_t(oty + th) < my) return false;
						dx += (s.getX(ty) - s.getX(oty));
						dy += ty - oty;
					}
				}
				return true;
			}
			return false;
		}
	}
	return false;
}

void slope_push_int(slope_info const& s, solid_object* obj, zfix& dx, zfix& dy, bool onplatform, bool fallthrough)
{
	bool disabledY = (dy == -1);
	bool disabledX = (dx == -1);
	zfix rx = obj->x+obj->hxofs+obj->sxofs, ry = obj->y+obj->hyofs+obj->syofs,
	rw = obj->hit_width+obj->sxsz_ofs, rh = obj->hit_height+obj->sysz_ofs;
	zfix orx = rx;
	zfix ory = ry;
	zfix cx = rx + rw/2 - 1;
	zfix cy = ry + rh/2 - 1;
	double lineangle = atan2(double(s.y2-s.y1),double(s.x2-s.x1));
	double val = comparePointLine(cx, cy, s.x1, s.y1, s.x2, s.y2);
	if (val < 0)
	{
		lineangle -= PI/2;
	}
	else
	{
		lineangle += PI/2;
	}
	zfix sinangle = zc::math::Sin(lineangle);
	zfix cosangle = zc::math::Cos(lineangle);
	if(s.ignore(lineangle, fallthrough, onplatform)) return;
	if (obj->sideview_mode() && sinangle < 0)
	{
		while(lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry, rw, rh))
		{
			onplatform = true;
			--ry;
		}
	}
	else
	{
		zfix mx = cosangle;
		zfix my = sinangle;
		if (disabledX && !my) return;
		if (disabledY && !mx) return;
		if (mx && my)
		{
			while (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry, rw, rh))
			{
				if (!disabledX) rx += cosangle;
				if (!disabledY) ry += sinangle;
			}
		}
		else
		{
			if (s.y1 == s.y2 && s.x1 != s.x2)
			{
				if (!my || disabledY) return;
				while (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry, rw, rh))
				{
					ry += sinangle;
				}
			}
			if (s.x1 == s.x2 && s.y1 != s.y2)
			{
				if (!mx || disabledX) return;
				while (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry, rw, rh))
				{
					rx += cosangle;
				}
			}
		}
	}
	dx += (rx - orx);
	dy += (ry - ory);
}




