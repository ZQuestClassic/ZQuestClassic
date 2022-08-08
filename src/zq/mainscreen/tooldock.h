
#ifndef ZQ_MS_TOOLDOC_H
#define ZQ_MS_TOOLDOC_H

#include "mainscreen.h"
#include "toolbox.h"
#include <functional>

void draw_docks();
class Tooldock
{
public:
	int32_t x,y;
	int32_t w,h;
	Tooldock();
	~Tooldock();
	
	std::vector<int32_t> docked_boxes;
	size_t size() const {return docked_boxes.size();}
	
	void insert(int32_t tbind, size_t pos);
	Toolbox const& get(size_t q);
	void draw();
	static void set_align();
private:
	size_t align;
};

void calculate_points();
void draw_points(int32_t color);
bool draw_point_outline(int32_t x, int32_t y);
bool click_into_point(int32_t tbind, int32_t x, int32_t y);

struct click_point
{
	int32_t x,y;
	int32_t w,h;
	size_t td_ind;
	size_t sub_ind;
	
	click_point(int32_t tx, int32_t ty, int32_t w, int32_t h,size_t td_ind, size_t sub_ind) :
		x(tx-(w/2)), y(ty-(h/2)), w(w), h(h),
		td_ind(td_ind), sub_ind(sub_ind)
	{
		//Force points in-bounds
		if(x < MAINSCR_X)
			x = MAINSCR_X;
		else if(x+w > MAINSCR_WID)
			x = MAINSCR_WID - w;
		if(y < MAINSCR_Y)
			y = MAINSCR_Y;
		else if(y+h > MAINSCR_HEI)
			y = MAINSCR_HEI - h;
	}
	
	bool collide(int32_t x, int32_t y);
	void fill(int32_t color);
	void outline();
};

#endif

