#include "slopes.h"
#include "base/combo.h"
#include "base/zc_math.h"
#include "ffc.h"
#include "sprite.h"
#include "zc/maps.h"

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
	if (x2 != x1) x2 += 0.9999_zf; 
	if (y1 > y2) y1 += 0.9999_zf;
	else if (y1 < y2) y2 += 0.9999_zf;
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
	word const* id = ff ? &ffc->data : cmbid;
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
	: ffc(ff), cmbid(cid), id(id)
{
	if(ffc)
	{
		xoffs = ffc->x;
		yoffs = ffc->y;
		cmbid = &(ffc->data);
	}
	else
	{
		xoffs = COMBOX(cpos);
		yoffs = COMBOY(cpos);
	}
}

zfix slope_info::slope()        const { return (y2-y1)/(x2-x1); }
zfix slope_info::slipperiness() const { return cmb ? zslongToFix(cmb->attributes[0]) : 0_zf; }
bool slope_info::stairs()       const { return cmb && (cmb->usrflags & cflag1); }
bool slope_info::ignorebottom() const { return cmb && (cmb->usrflags & cflag2); }
bool slope_info::ignoretop()    const { return cmb && (cmb->usrflags & cflag3); }
bool slope_info::ignoreleft()   const { return cmb && (cmb->usrflags & cflag4); }
bool slope_info::ignoreright()  const { return cmb && (cmb->usrflags & cflag5); }
bool slope_info::falldown()     const { return cmb && (cmb->usrflags & cflag6); }
bool slope_info::ignore(zfix sinangle, zfix cosangle, bool canfall, bool onplatform) const
{
	if(!cmb) return true;
	if (sinangle <= 0 && ((onplatform && stairs())
		|| ignoretop() || (canfall && falldown())))
		return true;
	if (sinangle >= 0 && (ignorebottom() || stairs()))
		return true;
	if (cosangle <= 0 && ignoreleft())
		return true;
	if (cosangle >= 0 && ignoreright())
		return true;
	return false;
}

zfix check_slope(zfix tx, zfix ty, zfix tw, zfix th, bool fallthrough, bool platformonly)
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
			zfix sinangle = zc::math::Sin(lineangle);
			zfix cosangle = zc::math::Cos(lineangle);
			if (s.x2 == s.x1) sinangle = 0;
			if (s.y2 == s.y1) cosangle = 0;
			if(s.ignore(sinangle, cosangle, fallthrough)) continue;
			zfix ret = sign(zc::math::Sin(lineangle));
			if (ret < 0 || !platformonly) return ret?ret:1_zf;
		}
	}
	return 0;
}

zfix check_new_slope(zfix tx, zfix ty, zfix tw, zfix th, zfix otx, zfix oty, bool fallthrough, bool platformonly, zfix ID)
{
	for(auto const& p : slopes)
	{
		slope_object const& obj = p.second;
		slope_info const& s = obj.get_info();
		if (s.stairs() && ID != 0 && s.slope() != ID) continue;
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
			bool staircheck = false;
			zfix sinangle = zc::math::Sin(lineangle);
			zfix cosangle = zc::math::Cos(lineangle);
			if (s.x2 == s.x1) sinangle = 0;
			if (s.y2 == s.y1) cosangle = 0;
			if(s.ignore(sinangle, cosangle, fallthrough)) continue;
			zfix ret = sign(zc::math::Sin(lineangle));
			if (ret < 0 || !platformonly) return ret?ret:1_zf;
		}
	}
	return 0;
}

zfix LinePointDist(slope_info const& s, zfix tx, zfix ty)
{
	zfix cx;
	zfix cy;
	if (s.x1 == s.x2)
	{
		cx = s.x1;
		cy = vbound(ty, s.y1, s.y2);
	}
	else if (s.y1 == s.y2)
	{
		cx = vbound(tx, s.x1, s.x2);
		cy = s.y1;
	}
	else
	{
		zfix b = s.y1 - s.slope() * s.x1;
		zfix slope2 = -1*(1_zf/s.slope());
		zfix b2 = ty - (tx*slope2);
		cx = (b2 - b) / (s.slope() - slope2);
		cx = vbound(cx, s.x1, s.x2);
		cy = (s.slope()*cx) + b; //y = mx + b
	}
	return (zfix)sqrt(pow((double)abs(tx-cx),2)+pow((double)abs(ty-cy),2));
}

slope_object const& get_slope(zfix tx, zfix ty, zfix tw, zfix th)
{
	zfix dist = 99999;
	static slope_object def_slope;
	slope_object const* ret = &def_slope;
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
			zfix sinangle = zc::math::Sin(lineangle);
			zfix cosangle = zc::math::Cos(lineangle);
			if (s.x2 == s.x1) sinangle = 0;
			if (s.y2 == s.y1) cosangle = 0;
			if(s.ignore(sinangle, cosangle)) continue;
			zfix slopedist = LinePointDist(s, cx, cy);
			if (slopedist > dist) continue;
			dist = slopedist;
			ret = &p.second;
		}
	}
	return *ret;
}

slope_object const& get_new_slope(zfix tx, zfix ty, zfix tw, zfix th, zfix otx, zfix oty)
{
	zfix dist = 99999;
	static slope_object def_slope;
	slope_object const* ret = &def_slope;
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
			zfix sinangle = zc::math::Sin(lineangle);
			zfix cosangle = zc::math::Cos(lineangle);
			if (s.x2 == s.x1) sinangle = 0;
			if (s.y2 == s.y1) cosangle = 0;
			if(s.ignore(sinangle, cosangle)) continue;
			zfix slopedist = LinePointDist(s, cx, cy);
			if (slopedist > dist) continue;
			dist = slopedist;
			ret = &p.second;
		}
	}
	return *ret;
}

zfix check_slope(solid_object* o, bool onlyNew)
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
		slope_object const& obj2 = p.second;
		slope_info const& s = obj2.get_info();
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
			if (s.x1 == s.x2) sinangle = 0; //Stupid hack because apparently double isn't precise enough to handle -PI
			if (s.y1 == s.y2) cosangle = 0; //Just to be on the safe side lets do it with cos too -Deedee
			if (sign(sinangle) <= 0)
			{
				dx += (s.x1 - obj2.ox1);
				dy += (s.y1 - obj2.oy1);
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
	if (s.x2 == s.x1) sinangle = 0; //Same as slide_slope; I don't trust double's ability to handle PI without getting it slightly wrong.
	if (s.y2 == s.y1) cosangle = 0; //I've only seen it with sin but I don't want to come back here if cos breaks -Deedee
	if(s.ignore(sinangle, cosangle, fallthrough, onplatform)) return;
	if (obj->sideview_mode() && sinangle < 0)
	{
		while(lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry, rw, rh))
		{
			onplatform = true;
			--ry;
		}
		zfix ty = 0;
		ty = binary_search_zfix(0_zf, 1_zf, [&](zfix val, zfix& retval)
		{
			if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry + val, rw, rh))
			{
				return BSEARCH_CONTINUE_TOWARD0;
			}
			else 
			{
				retval = val;
				return BSEARCH_CONTINUE_AWAY0;
			}
		});
		ry += ty;
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
			zfix rx2 = rx - cosangle;
			zfix ry2 = ry - sinangle;
			zfix percent = 0;
			percent = binary_search_zfix(0.0001_zf, 1_zf, [&](zfix val, zfix& retval)
			{
				if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, zc::math::Lerp(rx2, rx, val), zc::math::Lerp(ry2, ry, val), rw, rh))
				{
					return BSEARCH_CONTINUE_AWAY0;
				}
				else 
				{
					retval = val;
					return BSEARCH_CONTINUE_TOWARD0;
				}
			});
			rx = zc::math::Lerp(rx2, rx, percent);
			ry = zc::math::Lerp(ry2, ry, percent);
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
				zfix rdy = -sinangle;
				zfix ty = 0;
				ty = binary_search_zfix(0, rdy, [&](zfix val, zfix& retval)
				{
					if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry + val, rw, rh))
					{
						return BSEARCH_CONTINUE_TOWARD0;
					}
					else 
					{
						retval = val;
						return BSEARCH_CONTINUE_AWAY0;
					}
				});
				ry += ty;
			}
			if (s.x1 == s.x2 && s.y1 != s.y2)
			{
				if (!mx || disabledX) return;
				while (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx, ry, rw, rh))
				{
					rx += cosangle;
				}
				zfix rdx = -cosangle;
				zfix tx = 0;
				tx = binary_search_zfix(0, rdx, [&](zfix val, zfix& retval)
				{
					if (lineBoxCollision(s.x1, s.y1, s.x2, s.y2, rx + val, ry, rw, rh))
					{
						return BSEARCH_CONTINUE_TOWARD0;
					}
					else 
					{
						retval = val;
						return BSEARCH_CONTINUE_AWAY0;
					}
				});
				rx += tx;
			}
		}
	}
	dx += (rx - orx);
	dy += (ry - ory);
}
