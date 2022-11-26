#ifndef _SLOPES_H_
#define _SLOPES_H_

#include "zfix.h"
#include "ffc.h"
#include <map>

struct slope_object;

extern std::map<int32_t, slope_object> slopes;
void draw_slopes(BITMAP *dest, int32_t x, int32_t y, int32_t col);

struct slope_info
{
	newcombo const& cmb;
	zfix x1,y1,x2,y2;
	
	zfix slope() const {return (y2-y1)/(x2-x1);}
	zfix slipperiness() const {return zslongToFix(cmb.attributes[0]);}
	
	bool stairs()       const { return cmb.usrflags & cflag1; }
	bool ignorebottom() const { return cmb.usrflags & cflag2; }
	bool ignoretop()    const { return cmb.usrflags & cflag3; }
	bool ignoreleft()   const { return cmb.usrflags & cflag4; }
	bool ignoreright()  const { return cmb.usrflags & cflag5; }
	bool falldown()     const { return cmb.usrflags & cflag6; }
	bool ignore(double lineangle, bool canfall = false, bool onplatform = false) const;
	
	zfix getX(zfix const& y) const;
	zfix getY(zfix const& x) const;
	
	void draw(BITMAP* dest, int32_t x, int32_t y, int32_t col) const;
	
	slope_info(newcombo const& cmb, zfix const& xoffs = 0, zfix const& yoffs = 0);
};

struct slope_object
{
	ffcdata const* ffc;
	word const* cmbid;
	zfix ox1,oy1,ox2,oy2;
	int32_t id;
	zfix xoffs,yoffs;
	
	slope_info get_info() const;
	
	void updateslope();
	slope_object(word* cid = nullptr, ffcdata* ff = nullptr, int32_t id = -1, word cpos = 0);
};

int32_t check_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th, bool fallthrough = false);
int32_t check_new_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th, int32_t otx, int32_t oty, bool fallthrough = false);
slope_object const& get_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th);
slope_object const& get_new_slope(int32_t tx, int32_t ty, int32_t tw, int32_t th, int32_t otx, int32_t oty);
int32_t check_slope(solid_object* o, bool onlyNew = false);
slope_object const& get_slope(solid_object* o, bool onlyNew = false);
bool slide_slope(solid_object* obj, zfix& dx, zfix& dy, zfix& ID);
void slope_push_int(slope_info const& s, solid_object* obj, zfix& dx, zfix& dy, bool onplatform = false, bool fallthrough = false);

#endif

