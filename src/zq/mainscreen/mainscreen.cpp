
#include "base/zdefs.h"
#include "zquest.h"
#include "zq_misc.h"
#include "zq_class.h"
#include "base/zsys.h"
#include "mainscreen.h"
#include "toolbox.h"
#include "base/gui.h"
#include <deque>

#include <chrono>
#include <thread>
void do_sleep(int32_t ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

extern int32_t current_combolist;
void animate_combos();
//A replacement for the old main UI, with re-organizable features

void save_mainscreen_configs()
{
	
}
void load_mainscreen_configs()
{
	init_toolboxes();
}

void draw_combo_pane(BITMAP* dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t list, bool thick);

enum
{
	TB_COMBO_COL1,
	TB_COMBO_COL2,
	TB_TEST,
	NUM_TB
};

Toolbox boxes[NUM_TB];
static std::deque<int32_t> focusOrder;
static int32_t mx, my, mb;
static bool m_can_resize;
static int32_t cursor_dir = -1;
static int32_t lhov_id = -1;
static int32_t last_combolist = -1;
static clock_t tb_vsync = 0;
enum {CDIR_MOVE = -2, CDIR_BASIC = -1, CDIR_UL_DR, CDIR_UR_DL, CDIR_L_R, CDIR_U_D};
void broadcast_tb_message(int32_t msg, int32_t c)
{
	for(int32_t q : focusOrder)
	{
		boxes[q].msg(msg,c);
	}
}

int32_t broadcast_mousefind()
{
	m_can_resize = false;
	for(auto it = focusOrder.rbegin(); it != focusOrder.rend(); ++it)
	{
		int32_t q = *it;
		auto ret = boxes[q].msg(MG_MSG_FIND_MOUSE_HOVER);
		switch(ret)
		{
			case MG_RET_CANRESIZE: m_can_resize = true; return q;
			case MG_RET_CANCLICK: return q;
		}
	}
	return -1;
}

void focus(int32_t id)
{
	for(auto it = focusOrder.begin(); it < focusOrder.end(); ++it)
	{
		if(*it == id)
		{
			focusOrder.erase(it);
			break;
		}
	}
	focusOrder.push_back(id);
}

bool run_tb_vsync()
{
	if(clock() > tb_vsync)
	{
		broadcast_tb_message(MG_MSG_VSYNC);
		tb_vsync = clock()+(CLOCKS_PER_SEC/60);
		return true;
	}
	return false;
}

bool tb_draw_vsync()
{
	if(run_tb_vsync())
	{
		animate_combos();
		return true;
	}
	return false;
}

void init_static_vars()
{
	mx = gui_mouse_x();
	my = gui_mouse_y();
	mb = 0;
	m_can_resize = false;
	cursor_dir = -1;
	position_mouse_z(0);
	tb_vsync = clock();
}

int32_t dir_to_cdir(int32_t dir)
{
	switch(dir)
	{
		case up: case down: return CDIR_U_D; break;
		case left: case right: return CDIR_L_R; break;
		case l_up: case r_down: return CDIR_UL_DR; break;
		case l_down: case r_up: return CDIR_UR_DL; break;
		case -2: return CDIR_MOVE;
		default: return CDIR_BASIC;
	}
}
void setcurs(int32_t cdir, bool force = false)
{
	if(!force && cursor_dir == cdir)
		return;
	cursor_dir = cdir;
	switch(cdir)
	{
		case CDIR_MOVE:
			set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][1]);
			zq_set_mouse_focus(8,8);
			break;
		case CDIR_L_R: case CDIR_U_D:
		case CDIR_UL_DR: case CDIR_UR_DL:
			set_mouse_sprite(mouse_bmp[MOUSE_BMP_RESIZE][cdir]);
			zq_set_mouse_focus(8,8);
			break;
		case CDIR_BASIC: default:
			set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
			zq_set_mouse_focus(1,1);
			break;
	}
}

void run_combosel_rc(int32_t x, int32_t y);
void dirty_comboboxes()
{
	if(current_combolist != last_combolist)
	{
		boxes[TB_COMBO_COL1].flags |= TBF_DIRTY;
		boxes[TB_COMBO_COL2].flags |= TBF_DIRTY;
		last_combolist = current_combolist;
	}
}
void init_combobox()
{
	static const int def_box_w = (16*4)+8;
	Toolbox& box = boxes[TB_COMBO_COL1];
	box.flags |= TBF_VISIBLE;
	box.pos(zq_screen_w-def_box_w,0,def_box_w,16*24+24);
	box.minsz((16*4)+8,16*12+24);
	box.refbmp = create_bitmap_ex(8,4*16,36*16);
	box.title = "Combos 1";
	box.proc = [&](Toolbox* tb,int32_t msg,int32_t c)
	{
		tb->sanity();
		int32_t ret = MG_RET_OK;
		//zoomfactor: 2 to 4, inclusive
		int32_t zf = vbound(tb->zoomfactor,1,4);
		switch(msg)
		{
			case MG_MSG_CLICK:
			{
				if(!tb->mouse_in_d())
				{
					current_combolist = 0;
					dirty_comboboxes();
					BGLOOP_START();
					int32_t offsx, offsy;
					while(gui_mouse_b()&1)
					{
						do_sleep(1);
						offsx = gui_mouse_x()-(tb->x+tb->dx[0]);
						offsy = gui_mouse_y()-(tb->y+tb->dy[0]);
						if(unsigned(offsx) >= tb->dw[0]) continue;
						if(unsigned(offsy) >= tb->dh[0]) continue;
						int32_t pos_clicked = (offsx/(16*zf))+((offsy/(16*zf))*4);
						if(Combo == First[0]+pos_clicked) continue;
						Combo = First[0]+pos_clicked;
						tb->msg(MG_MSG_REDRAW_SELF);
						BGLOOP_DRAW();
					}
					BGLOOP_END();
				}
				break;
			}
			case MG_MSG_RCLICK:
			{
				if(!tb->mouse_in_d())
				{
					current_combolist = 0;
					dirty_comboboxes();
					int32_t offsx = gui_mouse_x()-(tb->x+tb->dx[0]);
					int32_t offsy = gui_mouse_y()-(tb->y+tb->dy[0]);
					int32_t pos_clicked = (offsx/(16*zf))+((offsy/(16*zf))*4);
					Combo = First[0]+pos_clicked;
					BGLOOP_START();
					tb->msg(MG_MSG_REDRAW_SELF);
					BGLOOP_DRAW();
					run_combosel_rc(gui_mouse_x(),gui_mouse_y());
					BGLOOP_END();
				}
				break;
			}
			case MG_MSG_SCROLL:
			{
				auto mz = gui_mouse_z();
				int32_t c_per_pg = 4 * (36/zf);
				auto hei = tb->h - tb->dy[0] - 2;
				auto rows = hei/(16*zf);
				if(rows < 36/zf)
					c_per_pg = 4 * rows;
				First[0] = vbound(First[0]+(c_per_pg * -mz),0,MAXCOMBOS-c_per_pg);
				tb->flags |= TBF_DIRTY;
				position_mouse_z(0);
				break;
			}
			case MG_MSG_REDRAW_SELF:
			{
				tb->baseproc(msg,c);
				int32_t cw = 4, ch = 36/zf;
				int32_t sw = 16*cw, sh = 16*ch;
				tb->dw[0] = sw*zf;
				tb->dh[0] = 16*36;
				
				tb->dx[0] = 4;
				tb->dy[0] = 22;
				
				auto rows = (tb->h - 24)/(16*zf);
				if(rows<36/zf)
				{
					ch = rows;
					sh = 16*ch;
					tb->dh[0] = zf*sh;
				}
				clear_bitmap(tb->refbmp);
				draw_combo_pane(tb->refbmp, 0, 0, cw, ch, 0, zf<3);
				stretch_blit(tb->refbmp, tb->getBitmap(), 0, 0, sw, sh, tb->dx[0], tb->dy[0], tb->dw[0], tb->dh[0]);
				break;
			}
			case MG_MSG_RESZ_UP: case MG_MSG_RESZ_DOWN:
			{
				auto rows = int32_t(round((tb->h+c - 24)/(16.0*zf)));
				return (rows * (16*zf))+24-tb->h;
			}
			case MG_MSG_RESZ_LEFT: case MG_MSG_RESZ_RIGHT:
			{
				int32_t scale = (tb->w - 8 + c) / (16*4);
				scale = vbound(scale,1,4);
				tb->zoomfactor = scale;
				return (8 + (16*4*scale)) - tb->w;
			}
			case MG_MSG_VSYNC:
			{
				tb->flags |= TBF_DIRTY;
				break;
			}
			default: ret = tb->baseproc(msg,c);
		}
		return ret;
	};

	Toolbox& box2 = boxes[TB_COMBO_COL2];
	box2.flags |= TBF_VISIBLE;
	box2.pos(zq_screen_w-(def_box_w*2),0,def_box_w,16*24+24);
	box2.minsz((16*4)+8,16*12+24);
	box2.refbmp = create_bitmap_ex(8,4*16,36*16);
	box2.title = "Combos 2";
	box2.proc = [&](Toolbox* tb,int32_t msg,int32_t c)
	{
		tb->sanity();
		int32_t ret = MG_RET_OK;
		//zoomfactor: 2 to 4, inclusive
		int32_t zf = vbound(tb->zoomfactor,1,4);
		switch(msg)
		{
			case MG_MSG_CLICK:
			{
				if(!tb->mouse_in_d())
				{
					current_combolist = 1;
					dirty_comboboxes();
					BGLOOP_START();
					int32_t offsx, offsy;
					while(gui_mouse_b()&1)
					{
						do_sleep(1);
						offsx = gui_mouse_x()-(tb->x+tb->dx[0]);
						offsy = gui_mouse_y()-(tb->y+tb->dy[0]);
						if(unsigned(offsx) >= tb->dw[0]) continue;
						if(unsigned(offsy) >= tb->dh[0]) continue;
						int32_t pos_clicked = (offsx/(16*zf))+((offsy/(16*zf))*4);
						if(Combo == First[0]+pos_clicked) continue;
						Combo = First[0]+pos_clicked;
						tb->msg(MG_MSG_REDRAW_SELF);
						BGLOOP_DRAW();
					}
					BGLOOP_END();
				}
				break;
			}
			case MG_MSG_RCLICK:
			{
				if(!tb->mouse_in_d())
				{
					current_combolist = 1;
					dirty_comboboxes();
					int32_t offsx = gui_mouse_x()-(tb->x+tb->dx[0]);
					int32_t offsy = gui_mouse_y()-(tb->y+tb->dy[0]);
					int32_t pos_clicked = (offsx/(16*zf))+((offsy/(16*zf))*4);
					Combo = First[0]+pos_clicked;
					BGLOOP_START();
					tb->msg(MG_MSG_REDRAW_SELF);
					BGLOOP_DRAW();
					run_combosel_rc(gui_mouse_x(),gui_mouse_y());
					BGLOOP_END();
				}
				break;
			}
			case MG_MSG_SCROLL:
			{
				auto mz = gui_mouse_z();
				int32_t c_per_pg = 4 * (36/zf);
				auto hei = tb->h - tb->dy[0] - 2;
				auto rows = hei/(16*zf);
				if(rows < 36/zf)
					c_per_pg = 4 * rows;
				First[1] = vbound(First[1]+(c_per_pg * -mz),0,MAXCOMBOS-c_per_pg);
				tb->flags |= TBF_DIRTY;
				position_mouse_z(0);
				break;
			}
			case MG_MSG_REDRAW_SELF:
			{
				tb->baseproc(msg,c);
				int32_t cw = 4, ch = 36/zf;
				int32_t sw = 16*cw, sh = 16*ch;
				tb->dw[0] = sw*zf;
				tb->dh[0] = 16*36;
				
				tb->dx[0] = 4;
				tb->dy[0] = 22;
				
				auto rows = (tb->h - 24)/(16*zf);
				if(rows<36/zf)
				{
					ch = rows;
					sh = 16*ch;
					tb->dh[0] = zf*sh;
				}
				clear_bitmap(tb->refbmp);
				draw_combo_pane(tb->refbmp, 0, 0, cw, ch, 1, zf<3);
				stretch_blit(tb->refbmp, tb->getBitmap(), 0, 0, sw, sh, tb->dx[0], tb->dy[0], tb->dw[0], tb->dh[0]);
				break;
			}
			case MG_MSG_RESZ_UP: case MG_MSG_RESZ_DOWN:
			{
				auto rows = int32_t(round((tb->h+c - 24)/(16.0*zf)));
				return (rows * (16*zf))+24-tb->h;
			}
			case MG_MSG_RESZ_LEFT: case MG_MSG_RESZ_RIGHT:
			{
				int32_t scale = (tb->w - 8 + c) / (16*4);
				scale = vbound(scale,1,4);
				tb->zoomfactor = scale;
				return (8 + (16*4*scale)) - tb->w;
			}
			case MG_MSG_VSYNC:
			{
				tb->flags |= TBF_DIRTY;
				break;
			}
			default: ret = tb->baseproc(msg,c);
		}
		return ret;
	};
}
void init_toolboxes()
{
	focusOrder.clear();
	for(auto q = 0; q < NUM_TB; ++q)
		focusOrder.push_back(q);
	init_static_vars();
	init_combobox();
	Toolbox& box = boxes[TB_TEST];
	box.flags |= TBF_VISIBLE;
	box.pos(200,200,64,64);
}

void move_tb(int32_t id)
{
	Toolbox& box = boxes[id];
	mx = gui_mouse_x();
	my = gui_mouse_y();
	int32_t dx = mx - box.x, dy = my - box.y;
	BGLOOP_START();
	bool dirty = false;
	while(gui_mouse_b()&1) //while holding resize
	{
		if(mx != gui_mouse_x() || my != gui_mouse_y())
		{
			mx = gui_mouse_x();
			my = gui_mouse_y();
			int32_t nx = vbound(mx-dx,0,zq_screen_w-box.w);
			int32_t ny = vbound(my-dy,0,zq_screen_h-box.h);
			box.pos(nx,ny);
			dirty = true;
		}
		if(tb_draw_vsync() || dirty)
		{
			BGLOOP_DRAW();
			dirty = false;
		}
	}
	BGLOOP_END();
}
void resize(int32_t id)
{
	Toolbox& box = boxes[id];
	int32_t dir = box.getResizeDir();
	if(dir == CDIR_MOVE)
	{
		move_tb(id);
		return;
	}
	int32_t dx = 0, dy = 0;
	switch(dir)
	{
		case left: case l_up: case l_down:
			dx = -1; break;
		case right: case r_up: case r_down:
			dx = 1; break;
	}
	switch(dir)
	{
		case up: case l_up: case r_up:
			dy = -1; break;
		case down: case l_down: case r_down:
			dy = 1; break;
	}
	mx = gui_mouse_x();
	my = gui_mouse_y();
	BGLOOP_START();
	while(gui_mouse_b()&1) //while holding resize
	{
		if(mx != gui_mouse_x() || my != gui_mouse_y())
		{
			mx = gui_mouse_x();
			my = gui_mouse_y();
			int32_t lx = vbound(mx,0,zq_screen_w-1);
			int32_t ly = vbound(my,0,zq_screen_h-1);
			if(dx < 0)
				box.rsz_left(box.x-lx);
			if(dx > 0)
				box.rsz_right(lx-(box.x+box.w-1));
			if(dy < 0)
				box.rsz_up(box.y-ly);
			if(dy > 0)
				box.rsz_down(ly-(box.y+box.h-1));
			box.flags |= TBF_DIRTY;
		}
		if(tb_draw_vsync() || (box.flags&TBF_DIRTY))
		{
			BGLOOP_DRAW();
		}
	}
	BGLOOP_END();
}
void runToolboxes()
{
	sp_acquire_screen();
	run_tb_vsync();
	if(mx != gui_mouse_x() || my != gui_mouse_y() || mb != gui_mouse_b() || (lhov_id>-1 && gui_mouse_z()))
	{
		auto omx = mx, omy = my, omb = mb;
		mx = gui_mouse_x(); my = gui_mouse_y();
		mb = gui_mouse_b();
		//Mouse movement
		auto id = broadcast_mousefind();
		bool samehov = lhov_id == id;
		if(!samehov)
		{
			position_mouse_z(0);
			lhov_id = id;
		}
		if(id < 0)
		{
			setcurs(CDIR_BASIC);
		}
		else if(m_can_resize)
		{
			setcurs(dir_to_cdir(boxes[id].getResizeDir()));
			if(gui_mouse_b()&1) //lclick
			{
				focus(id);
				resize(id);
				position_mouse_z(0);
				runToolboxes();
				return;
			}
		}
		else
		{
			setcurs(CDIR_BASIC);
			if(!(omb&1) && (mb&1)) //lclick
			{
				focus(id);
				boxes[id].msg(MG_MSG_CLICK);
			}
			if(!(omb&2) && (mb&2)) //rclick
			{
				focus(id);
				boxes[id].msg(MG_MSG_RCLICK);
			}
			if(gui_mouse_z()) //scroll
			{
				focus(id);
				boxes[id].msg(MG_MSG_SCROLL);
			}
		}
	}
	broadcast_tb_message(MG_MSG_IDLE);
	broadcast_tb_message(MG_MSG_REDRAW_BOX);
	all_mark_screen_dirty();
	sp_release_screen_all();
}


extern bool close_button_quit;

void test_mainscreen_gui()
{
	while(key[KEY_G]);
	broadcast_tb_message(MG_MSG_REDRAW_SELF);
	while(true)
	{
		if(key[KEY_G]) break;
		BGLOOP_DRAW1();
		clear_to_color(screen, vc(0));
		tb_draw_vsync();
		runToolboxes();
		BGLOOP_DRAW3();
		do_sleep(1);
		if(close_button_quit)
		{
			close_button_quit=false;
			break;
		}
	}
	clear_to_color(screen, vc(0));
	while(key[KEY_G]);
	sp_release_screen_all();
}

void draw_combo_pane(BITMAP* dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t list, bool thick)
{
	for(int32_t i=0; i<(w*h); i++)
	{
		put_combo(dest,(i%w)*16+x,(i/w)*16+y,i+First[list],CSet,Flags&(cFLAGS|cWALK),0);
	}
	if(list != current_combolist) return;
	
	int32_t rect_pos=Combo-First[list];
	
	if((rect_pos>=0)&&(rect_pos<(First[list]+(w*h))))
	{
		int32_t rx = (rect_pos & (w - 1)) * 16 + x;
		int32_t ry = (rect_pos / w) * 16 + y;
		safe_rect(dest, rx + 0, ry + 0, rx + 15, ry + 15, 255);
		if(thick) safe_rect(dest, rx + 1, ry + 1, rx + 14, ry + 14, 255);
	}
}

