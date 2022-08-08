
#include "tooldock.h"
#include "jwin.h"
#include "zquest.h"
#include "base/gui.h"
#include "zq_misc.h"

using std::string;
using std::vector;
using std::to_string;

Tooldock docks[8];
static vector<click_point> points;
Tooldock::Tooldock() : x(0),y(0),
	w(0),h(0)
{}
Tooldock::~Tooldock(){}

Toolbox const& Tooldock::get(size_t q)
{
	return boxes[docked_boxes[q]];
}

void Tooldock::set_align()
{
	for(auto q = 0; q < 8; ++q)
		docks[q].align = q;
}

void Tooldock::insert(int32_t tbind, size_t pos)
{
	auto it = docked_boxes.begin();
	for(auto q = 0; q < pos; ++q) ++it;
	docked_boxes.insert(it, tbind);
	boxes[tbind].flags |= TBF_DOCKED;
	//Realign all docked boxes, and the dock itself
	if(size() == 1) //Just inserted first element
	{
		Toolbox& box = boxes[tbind];
		switch(align)
		{
			case up:
			{
				if(docks[l_up].size())
					x = docks[l_up].w;
				else x = MAINSCR_X;
				y = MAINSCR_Y;
				w = MAINSCR_WID;
				if(docks[l_up].size()) w -= docks[l_up].w;
				if(docks[r_up].size()) w -= docks[r_up].w;
				h = box.h;
				
				box.pos(x,y,w,h);
				break;
			}
			case down:
			{
				if(docks[l_down].size())
					x = docks[l_down].w;
				else x = MAINSCR_X;
				y = MAINSCR_END_Y - box.h;
				w = MAINSCR_WID;
				if(docks[l_down].size()) w -= docks[l_down].w;
				if(docks[r_down].size()) w -= docks[r_down].w;
				h = box.h;
				break;
			}
		}
	}
	else //Adding another element to an already-initialized dock
	{
		switch(align)
		{
			case up:
			{
				int32_t tx = x;
				int32_t tw = w/size();
				for(auto ind : docked_boxes)
				{
					Toolbox& box = boxes[ind];
					box.x = tx;
					box.y = y;
					box.w = tw;
					box.h = h;
					tx += tw;
				}
				break;
			}
			case down:
			{
				int32_t tx = x;
				int32_t tw = w/size();
				for(auto ind : docked_boxes)
				{
					Toolbox& box = boxes[ind];
					box.x = tx;
					box.y = y;
					box.w = tw;
					box.h = h;
					tx += tw;
				}
				break;
			}
		}
	}
	for(auto ind : docked_boxes)
		boxes[ind].flags |= TBF_DIRTY;
}

void Tooldock::draw()
{
	if(!size()) return;
	jwin_draw_win(screen,x,y,w,h,FR_WIN);
	
	for(auto ind : docked_boxes)
	{
		boxes[ind].msg(MG_MSG_REDRAW_DOCKED);
	}
}

void draw_docks()
{
	for(auto dock : docks)
		dock.draw();
}
bool click_point::collide(int32_t tx, int32_t ty)
{
	return (tx >= x && tx < (x+w) && ty >= y && ty < (y+h));
}
void click_point::fill(int32_t color)
{
	rectfill(screen, x, y, x+w-1, y+h-1, color);
}
void click_point::outline()
{
	dotted_rect(screen, x, y, x+w-1, y+h-1, vc(15), vc(0));
}

#define TB_POINT_SZ  16
#define TB_POINT_HLF 8
void calculate_points()
{
	points.clear();
	if(docks[up].size())
	{
		size_t len = docks[up].size();
		for(size_t q = 0; q < len; ++q)
		{
			Toolbox const& tb = docks[up].get(q);
			points.emplace_back(tb.x,tb.y+(tb.h/2),TB_POINT_SZ,TB_POINT_SZ,up,q);
		}
		Toolbox const& last = docks[up].get(len-1);
		points.emplace_back(last.x+last.w,last.y+(last.h/2),TB_POINT_SZ,TB_POINT_SZ,up,len);
	}
	else points.emplace_back(MAINSCR_CENTER_X,MAINSCR_Y+8,TB_POINT_SZ,TB_POINT_SZ,up,0);
	
	if(docks[down].size())
	{
		size_t len = docks[down].size();
		for(size_t q = 0; q < len; ++q)
		{
			Toolbox const& tb = docks[down].get(q);
			points.emplace_back(tb.x,tb.y+(tb.h/2),TB_POINT_SZ,TB_POINT_SZ,down,q);
		}
		Toolbox const& last = docks[down].get(len-1);
		points.emplace_back(last.x+last.w,last.y+(last.h/2),TB_POINT_SZ,TB_POINT_SZ,down,len);
	}
	else points.emplace_back(MAINSCR_CENTER_X,MAINSCR_END_Y-8,TB_POINT_SZ,TB_POINT_SZ,down,0);
}

void draw_points(int32_t color)
{
	for(auto point : points)
		point.fill(color);
}

bool draw_point_outline(int32_t x, int32_t y)
{
	for(auto point : points)
	{
		if(point.collide(x,y))
		{
			point.outline();
			return true;
		}
	}
	return false;
}

bool click_into_point(int32_t tbind, int32_t x, int32_t y)
{
	for(auto point : points)
	{
		if(point.collide(x,y))
		{
			docks[point.td_ind].insert(tbind, point.sub_ind);
			return true;
		}
	}
	return false;
}

