#ifndef SLOPES_H_
#define SLOPES_H_

#include "base/ints.h"
#include "base/zc_alleg.h"
#include "base/zfix.h"
#include <map>

class ffcdata;
class solid_object;
struct newcombo;
struct slope_object;

extern std::map<int32_t, slope_object> slopes;
void draw_slopes(BITMAP *dest, int32_t x, int32_t y, int32_t col);
void draw_slopes_a5(int32_t x, int32_t y, ALLEGRO_COLOR col);

struct slope_info
{
	zfix x1,y1,x2,y2;
	
	zfix slope()        const;
	zfix slipperiness() const;
	
	bool stairs()       const;
	bool ignorebottom() const;
	bool ignoretop()    const;
	bool ignoreleft()   const;
	bool ignoreright()  const;
	bool falldown()     const;
	bool ignore(zfix sinangle, zfix cosangle, bool canfall = false, bool onplatform = false) const;
	
	zfix getX(zfix const& y) const;
	zfix getY(zfix const& x) const;
	
	void draw(BITMAP* dest, int32_t x, int32_t y, int32_t col) const;
	void draw_a5(int32_t x, int32_t y, ALLEGRO_COLOR col) const;
	
	slope_info(newcombo const& cmb, zfix const& xoffs = 0, zfix const& yoffs = 0);
	slope_info();
private:
	newcombo const* cmb;
	slope_info(slope_info const& other) = delete;
	slope_info& operator=(slope_info const& other) = delete;
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
	slope_object(slope_object const& other) = delete;
	slope_object& operator=(slope_object const& other) = delete;
};

zfix check_slope(zfix tx, zfix ty, zfix tw, zfix th, bool fallthrough = false, bool platformonly = false);
zfix check_new_slope(zfix tx, zfix ty, zfix tw, zfix th, zfix otx, zfix oty, bool fallthrough = false, bool platformonly = false, zfix ID = 0);
zfix LinePointDist(slope_info const& s, zfix tx, zfix ty);
slope_object const& get_slope(zfix tx, zfix ty, zfix tw, zfix th);
slope_object const& get_new_slope(zfix tx, zfix ty, zfix tw, zfix th, zfix otx, zfix oty);
zfix check_slope(solid_object* o, bool onlyNew = false);
slope_object const& get_slope(solid_object* o, bool onlyNew = false);
bool slide_slope(solid_object* obj, zfix& dx, zfix& dy, zfix& ID);
void slope_push_int(slope_info const& s, solid_object* obj, zfix& dx, zfix& dy, bool onplatform = false, bool fallthrough = false);

#endif
