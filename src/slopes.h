#ifndef _SLOPES_H_
#define _SLOPES_H_

#include "zfix.h"
#include "ffc.h"
#include <map>

struct slope_object;

#define SLOPE_ID(index, layer) ((rpos_t)(region_num_rpos * (layer) + (index)))
extern std::map<rpos_t, slope_object> slopes;
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
	bool ignore(double lineangle, bool canfall = false, bool onplatform = false) const;
	
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
	rpos_t id;
	zfix xoffs,yoffs;
	
	slope_info get_info() const;
	
	void updateslope();
	slope_object(word* cid = nullptr, ffcdata* ff = nullptr, rpos_t id = rpos_t::NONE, word cpos = 0);
	slope_object(slope_object const& other) = delete;
	slope_object& operator=(slope_object const& other) = delete;
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

