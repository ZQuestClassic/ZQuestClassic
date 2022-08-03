
#include "toolbox.h"
#include "jwin.h"
#include "zquest.h"
#include "base/gui.h"

#define RESZ_MARGIN 5

Toolbox::Toolbox() : x(0), y(0),
	w(64), h(64), flags(0), intbmp(nullptr),
	minw(32), minh(32), refbmp(nullptr),
	zoomfactor(1), title("UNSET_TITLE")
{
	memset(dx, 0, sizeof(dx));
	memset(dy, 0, sizeof(dy));
	memset(dw, 0, sizeof(dw));
	memset(dh, 0, sizeof(dh));
}

Toolbox::~Toolbox()
{
	if(intbmp)
		destroy_bitmap(intbmp);
	if(refbmp)
		destroy_bitmap(refbmp);
}

int32_t Toolbox::mouse_in_d()
{
	for(auto q = 0; q < 10; ++q)
	{
		if(!dw[q] || !dh[q]) continue;
		if(unsigned(gui_mouse_x()-(x+dx[q])) >= dw[q]) continue;
		if(unsigned(gui_mouse_y()-(y+dy[q])) >= dh[q]) continue;
		return q;
	}
	return -1;
}
int32_t Toolbox::getResizeDir()
{
	return getResizeDir(gui_mouse_x(), gui_mouse_y());
}
int32_t Toolbox::getResizeDir(int32_t mx, int32_t my)
{
	bool tmar = abs(my-y) < RESZ_MARGIN;
	bool dmar = abs(my-(y+h-1)) < RESZ_MARGIN;
	bool lmar = abs(mx-x) < RESZ_MARGIN;
	bool rmar = abs(mx-(x+w-1)) < RESZ_MARGIN;
	if(tmar)
	{
		if(lmar)
			return l_up;
		else if(rmar)
			return r_up;
		else if(!(mx < x || mx > (x+w-1)))
			return up;
	}
	if(dmar)
	{
		if(lmar)
			return l_down;
		else if(rmar)
			return r_down;
		else if(!(mx < x || mx > (x+w-1)))
			return down;
	}
	if(lmar && !(my < y || my > (y+h-1)))
		return left;
	if(rmar && !(my < y || my > (y+h-1)))
		return right;
	auto dy = my-y;
	if(dy > 0 && dy < 18 && !(mx < x || mx > (x+w-1)))
		return -2;
	return -1;
}

bool Toolbox::hovering(int32_t mx, int32_t my)
{
	if(mx-x >= w) return false;
	if(mx-x < 0) return false;
	if(my-y >= h) return false;
	if(my-y < 0) return false;
	return true;
}

void Toolbox::pos(int32_t nx, int32_t ny)
{
	x = nx;
	y = ny;
}
void Toolbox::pos(int32_t nx, int32_t ny, int32_t nw, int32_t nh)
{
	x = nx;
	y = ny;
	w = std::max(minw,nw);
	h = std::max(minh,nh);
}
void Toolbox::minsz(int32_t mw, int32_t mh)
{
	minw = std::max(32,mw);
	minh = std::max(32,mh);
	if(w < minw) w = minw;
	if(h < minh) h = minh;
	flags |= TBF_DIRTY;
}

void Toolbox::rsz_left(int32_t px)
{
	if(w+px < minw)
	{
		px = minw-w;
	}
	px = msg(MG_MSG_RESZ_LEFT,px);
	x -= px;
	w += px;
	sanity();
}
void Toolbox::rsz_right(int32_t px)
{
	if(w+px < minw)
	{
		px = minw-w;
	}
	px = msg(MG_MSG_RESZ_RIGHT,px);
	w += px;
	sanity();
}
void Toolbox::rsz_up(int32_t px)
{
	if(h+px < minh)
	{
		px = minh-h;
	}
	px = msg(MG_MSG_RESZ_UP,px);
	y -= px;
	h += px;
	sanity();
}
void Toolbox::rsz_down(int32_t px)
{
	if(h+px < minh)
	{
		px = minh-h;
	}
	px = msg(MG_MSG_RESZ_DOWN,px);
	h += px;
	sanity();
}
//private
int32_t Toolbox::call(int32_t m,int32_t c)
{
	if(proc)
		return proc(this, m, c);
	else return baseproc(m, c);
}
//public wrapper
int32_t Toolbox::msg(int32_t m,int32_t c)
{
	switch(m)
	{
		case MG_MSG_REDRAW_BOX:
			if(!(flags&TBF_VISIBLE))
				return MG_RET_OK;
			if(flags&TBF_DIRTY)
			{
				call(MG_MSG_REDRAW_SELF);
				flags &= ~TBF_DIRTY;
			}
			break;
	}
	return call(m,c);
}
int32_t Toolbox::baseproc(int32_t msg,int32_t c)
{
	sanity();
	int32_t ret = MG_RET_OK;
	bool vis = flags&TBF_VISIBLE;
	bool dis = flags&TBF_DISABLED;
	switch(msg)
	{
		case MG_MSG_CLICK:
		case MG_MSG_IDLE:
			break; //No default behavior
		//Complex default behavior
		case MG_MSG_FIND_MOUSE_HOVER:
		{
			if(!vis) break;
			uint32_t mx = gui_mouse_x();
			uint32_t my = gui_mouse_y();
			
			if(getResizeDir(mx,my) != -1)
				ret = MG_RET_CANRESIZE;
			else if(!dis && hovering(mx,my))
				ret = MG_RET_CANCLICK;
			break;
		}
		case MG_MSG_REDRAW_SELF:
		{
			newBitmap();
			set_clip_rect(intbmp, 0, 0, w, h);
			jwin_draw_win(intbmp,0,0,w,h,FR_WIN);
			jwin_draw_titlebar(intbmp, 3, 3, zc_max(0,w-6), zc_min(h,18), title.c_str(), false, false);
			set_clip_rect(intbmp, 4, 20, w-4, h-4);
			break;
		}
		case MG_MSG_REDRAW_BOX:
		{
			masked_stretch_blit(intbmp, screen, 0, 0, intbmp->w, intbmp->h, x, y, w, h);
			break;
		}
		//Basic default behavior
		case MG_MSG_SCROLL:
		{
			//Eat the scroll input
			position_mouse_z(0);
			break;
		}
		case MG_MSG_RESZ_UP: case MG_MSG_RESZ_DOWN:
		case MG_MSG_RESZ_LEFT: case MG_MSG_RESZ_RIGHT:
			return c;
		//Debug Behavior
		#if MAINSCREEN_DEBUG > 0
		case MG_MSG_RCLICK:
		{
			int32_t d = mouse_in_d();
			if(d > -1)
			{
				int32_t tx = x+dx[d], ty = y+dy[d];
				BGLOOP_START();
				BGLOOP_DRAW1();
				rectfill(screen, tx, ty, tx+dw[d]-1, ty+dh[d]-1, vc(12));
				BGLOOP_DRAW3();
				while(gui_mouse_b()&2)
				{
					do_sleep(1);
				}
				BGLOOP_END();
			}
			else
			{
				BGLOOP_START();
				BITMAP* tmp = create_bitmap_ex(8,w,h);
				clear_to_color(tmp, vc(11));
				for(auto q = 0; q < 10; ++q)
				{
					if(!dw[q] || !dh[q]) continue;
					rectfill(tmp, dx[q], dy[q], dx[q]+dw[q]-1, dy[q]+dh[q]-1, 0);
				}
				BGLOOP_DRAW1();
				masked_blit(tmp,screen,0,0,x,y,w,h);
				BGLOOP_DRAW3();
				destroy_bitmap(tmp);
				while(gui_mouse_b()&2)
				{
					do_sleep(1);
				}
				BGLOOP_END();
			}
			break;
		}
		#endif
	}
	return ret;
}

void Toolbox::sanity()
{
	if(w<1) w = 1;
	if(h<1) h = 1;
	if(!intbmp)
	{
		newBitmap();
		flags |= TBF_DIRTY;
	}
	if(x+w > zq_screen_w)
		x = zq_screen_w - w;
	if(y+h > zq_screen_h)
		y = zq_screen_h - h;
}
void Toolbox::newBitmap()
{
	newBitmap(w,h);
}
void Toolbox::newBitmap(uint32_t wid,uint32_t hei)
{
	if(!wid) wid = 1;
	if(!hei) hei = 1;
	if(intbmp)
	{
		if(intbmp->w == wid && intbmp->h == hei)
		{
			clear_bitmap(intbmp);
			return;
		}
		destroy_bitmap(intbmp);
	}
	intbmp = create_bitmap_ex(8,wid,hei);
	clear_bitmap(intbmp);
}
void Toolbox::clearBitmap()
{
	if(intbmp)
		clear_bitmap(intbmp);
}

