
#include "base/zdefs.h"
#include "zquest.h"
#include "zq_misc.h"
#include "zq_class.h"
#include "base/zsys.h"
#include "mainscreen.h"
#include "toolbox.h"
#include "base/gui.h"
#include <deque>

using std::string;
using std::vector;
using std::deque;
using std::to_string;
#include <chrono>
#include <thread>
void do_sleep(int32_t ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

extern int32_t current_combolist;
void animate_combos();
//A replacement for the old main UI, with re-organizable features

/////////////////////////
// Constants / Globals //
/////////////////////////
enum
{
	TB_DEV_DEBUG,
	TB_COMBO_COL1,
	TB_COMBO_COL2,
	TB_TEST,
	NUM_TB
};

Toolbox boxes[NUM_TB];
static std::deque<int32_t> focusOrder;
static std::deque<int32_t> drawOrder;
static int32_t mx, my, mb;
static int32_t m_curs_flags;
static int32_t cursor_dir = -1;
static int32_t lhov_id = -1;
static int32_t last_combolist = -1;
static clock_t tb_vsync = 0;
static int32_t gridsize = 16;
static bool show_grid = false;
enum {CDIR_MOVE = -2, CDIR_BASIC = -1, CDIR_UL_DR, CDIR_UR_DL, CDIR_L_R, CDIR_U_D};

void init_static_vars()
{
	mx = gui_mouse_x();
	my = gui_mouse_y();
	mb = 0;
	m_curs_flags = 0;
	cursor_dir = -1;
	lhov_id = -1;
	position_mouse_z(0);
	tb_vsync = clock();
}

//////////////////
// Toolbox Menu //
//////////////////
MENU* tbmenu = nullptr;
void update_tb_menu();
int32_t tbtoggle_combos_1()
{
	boxes[TB_COMBO_COL1].flags ^= TBF_VISIBLE;
	update_tb_menu();
	return D_O_K;
}
int32_t tbtoggle_combos_2()
{
	boxes[TB_COMBO_COL2].flags ^= TBF_VISIBLE;
	update_tb_menu();
	return D_O_K;
}
static MENU tbcombos_menu[] =
{
	{ " Combos 1", tbtoggle_combos_1, nullptr, 0, nullptr },
	{ " Combos 2", tbtoggle_combos_2, nullptr, 0, nullptr },
	{ nullptr, nullptr, nullptr, 0, nullptr }
};
MENU toolboxes_menu[] =
{
	{ "&Combos    ", nullptr, tbcombos_menu, 0, nullptr },
	{ "&Misc", nullptr, nullptr, 0, nullptr },
	{ nullptr, nullptr, nullptr, 0, nullptr }
};
void update_tb_menu()
{
	for(auto tb = 0; tb < NUM_TB; ++tb)
	{
		SETFLAG(tbmenu[tb].flags, D_SELECTED, boxes[tb].flags&TBF_VISIBLE);
	}
	SETFLAG(tbcombos_menu[0].flags, D_SELECTED, boxes[TB_COMBO_COL1].flags&TBF_VISIBLE);
	SETFLAG(tbcombos_menu[1].flags, D_SELECTED, boxes[TB_COMBO_COL2].flags&TBF_VISIBLE);
}
extern int32_t jwin_menu_selection;
int32_t update_toolbox()
{
	if(unsigned(jwin_menu_selection) >= NUM_TB) return D_O_K;
	auto tb = jwin_menu_selection;
	zprint2(" Updating toolbox %d \n", tb);
	boxes[tb].flags ^= TBF_VISIBLE;
	update_tb_menu();
	return D_O_K;
}
void init_tbmenu()
{
	static vector<string> names;
	static bool menu_init = false;
	if(menu_init) return;
	menu_init = true;
	for(auto& box : boxes)
	{
		names.push_back(box.title);
	}
	if(tbmenu) delete[] tbmenu;
	tbmenu = populate_menu_from_vec(names);
	for(auto q = 0; q < NUM_TB; ++q)
	{
		tbmenu[q].proc = update_toolbox;
	}
	toolboxes_menu[1].child = tbmenu;
}
////////////////
// Misc Stuff //
////////////////

void save_mainscreen_configs()
{
	
}
void load_mainscreen_configs()
{
	init_toolboxes();
}
MENU* populate_menu_from_vec(vector<string> const& mnu)
{
	MENU* ret = new MENU[mnu.size()+1];
	size_t q;
	for(q = 0; q < mnu.size(); ++q)
	{
		ret[q].text = const_cast<char*>(mnu[q].c_str());
		ret[q].proc = nullptr;
		ret[q].child = nullptr;
		ret[q].flags = 0;
		ret[q].dp = nullptr;
	}
	//terminator
	ret[q].text = nullptr;
	ret[q].proc = nullptr;
	ret[q].child = nullptr;
	ret[q].flags = 0;
	ret[q].dp = nullptr;
	return ret;
}
int32_t popup_menu_from_vec(vector<string> const& mnu, int32_t x, int32_t y, int32_t minw, int32_t minh)
{
	if(x < 0) x = gui_mouse_x();
	if(y < 0) y = gui_mouse_y();
	MENU *menu = populate_menu_from_vec(mnu);
	BGLOOP_START();
	BGLOOP_DRAW();
	cache_tb_cursor();
	FONT* of = font;
	font = lfont_l;
	int32_t m = popup_menu(menu, x, y, minw, minh);
	font = of;
	restore_tb_cursor();
	BGLOOP_END();
	delete[] menu;
	return m;
}

//////////////////////////
// Broadcasting / Focus //
//////////////////////////

int32_t broadcast_tb_message(int32_t msg, int32_t c)
{
	switch(msg)
	{
		case MG_MSG_KEY:
		case MG_MSG_CHAR:
		{
			for(int32_t q : focusOrder)
			{
				static int32_t char_force_q = -1;
				if(char_force_q > -1 && q != char_force_q)
					continue;
				char_force_q = -1;
				auto ret = boxes[q].msg(msg,c);
				if(ret == MG_RET_USECHAR)
					return q;
				if(ret == MG_RET_CHAR_WANTSMORE)
				{
					char_force_q = q;
					return -1;
				}
			}
			return -1;
		}
		default:
			for(int32_t q : drawOrder)
			{
				boxes[q].msg(msg,c);
			}
			break;
	}
	return 1;
}
#define MCFLAG_RESIZE_OR_MOVE 0x1
#define MCFLAG_DISABLED_CLICK 0x2
int32_t broadcast_mousefind()
{
	m_curs_flags = 0;
	for(auto it = drawOrder.rbegin(); it != drawOrder.rend(); ++it)
	{
		int32_t q = *it;
		auto ret = boxes[q].msg(MG_MSG_FIND_MOUSE_HOVER);
		switch(ret)
		{
			case MG_RET_CANRESIZE: m_curs_flags |= MCFLAG_RESIZE_OR_MOVE; return q;
			case MG_RET_CANCLICK: return q;
			case MG_RET_PINNEDMENU: //On a pinned menu's move area
			{
				if((gui_mouse_b()&3) == 2)
				{
					m_curs_flags |= MCFLAG_RESIZE_OR_MOVE;
					return q;
				}
				m_curs_flags |= MCFLAG_DISABLED_CLICK;
				return q;
			}
			case MG_RET_DISCLICK:
			{
				m_curs_flags |= MCFLAG_DISABLED_CLICK;
				return q;
			}
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
	if(boxes[id].flags & TBF_NO_REORDER) return;
	for(auto it = drawOrder.begin(); it < drawOrder.end(); ++it)
	{
		if(*it == id)
		{
			drawOrder.erase(it);
			break;
		}
	}
	drawOrder.push_back(id);
}

///////////
// VSync //
///////////

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

////////////////
// COMBOBOXES //
////////////////

int32_t cheap_getnum(char const* ptr, int32_t defval)
{
	cache_tb_cursor();
	BGLOOP_START();
	BGLOOP_DRAW();
	defval = getnumber(ptr,defval);
	BGLOOP_END();
	restore_tb_cursor();
	return defval;
}

void run_combosel_rc(int32_t x, int32_t y);
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
void dirty_comboboxes()
{
	if(current_combolist != last_combolist)
	{
		boxes[TB_COMBO_COL1].flags |= TBF_DIRTY;
		boxes[TB_COMBO_COL2].flags |= TBF_DIRTY;
		last_combolist = current_combolist;
	}
}

int32_t _combobox_proc(Toolbox* tb, int32_t msg, int32_t c, int32_t listnum)
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
				current_combolist = listnum;
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
					if(Combo == First[listnum]+pos_clicked) continue;
					Combo = First[listnum]+pos_clicked;
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
				current_combolist = listnum;
				dirty_comboboxes();
				int32_t offsx = gui_mouse_x()-(tb->x+tb->dx[0]);
				int32_t offsy = gui_mouse_y()-(tb->y+tb->dy[0]);
				int32_t pos_clicked = (offsx/(16*zf))+((offsy/(16*zf))*4);
				Combo = First[listnum]+pos_clicked;
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
			if(tb->scrollfactor < rows)
				rows = tb->scrollfactor;
			int32_t coffs = 4*rows;
			First[listnum] = vbound(First[listnum]+(coffs * -mz),0,MAXCOMBOS-c_per_pg);
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
			draw_combo_pane(tb->refbmp, 0, 0, cw, ch, listnum, zf<3);
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
			int32_t scale = round(double(tb->w - 8 + c) / (16.0*4));
			scale = vbound(scale,1,4);
			tb->zoomfactor = scale;
			return (8 + (16*4*scale)) - tb->w;
		}
		case MG_MSG_VSYNC:
		{
			tb->flags |= TBF_DIRTY;
			break;
		}
		case MG_MSG_BUILD_RCMENU:
		{
			tb->add_to_rcmenu("Resize");
			tb->add_to_rcmenu("");
			tb->baseproc(msg,c);
			break;
		}
		case MG_MSG_OPEN_RCMENU:
		{
			int32_t m = tb->popup_rclick_menu();
			switch(m)
			{
				case 0: //Resize
				{
					tb->zoomfactor = zf = vbound(cheap_getnum("Zoom (1x-4x)", zf), 1, 4);
					tb->w = (8 + (16*4*zf));
					tb->flags |= TBF_DIRTY;
					auto oldrows = int32_t(round((tb->h - 24)/(16.0*zf)));
					auto minrows = int32_t(round((tb->minh - 24)/(16.0*zf)));
					auto maxrows = int32_t(round((zq_screen_h - 24)/(16.0*zf)));
					char buf[32];
					sprintf(buf, "Tiles Tall (%d-%d)", minrows, maxrows);
					auto newrows = vbound(cheap_getnum(buf, oldrows),minrows,maxrows);
					if(oldrows == newrows) break;
					tb->h = (16*zf*newrows)+24;
					if(tb->y + tb->h >= zq_screen_h)
					{
						tb->y = zq_screen_h-tb->h;
					}
					tb->flags |= TBF_DIRTY;
					break;
				}
			}
			break;
		}
		default: ret = tb->baseproc(msg,c);
	}
	return ret;
}
void _init_combobox(int32_t listnum)
{
	static const int def_box_w = (16*4)+8;
	Toolbox& box = boxes[TB_COMBO_COL1+listnum];
	box.flags |= TBF_VISIBLE;
	box.pos(zq_screen_w-(def_box_w*(1+listnum)),0,def_box_w,16*24+24);
	box.minsz((16*4)+8,16*12+24);
	box.refbmp = create_bitmap_ex(8,4*16,36*16);
	box.title = "Combos " + to_string(listnum+1);
	box.scrollfactor = 5;
	box.proc = [&,listnum](Toolbox* tb,int32_t msg,int32_t c)
	{
		return _combobox_proc(tb,msg,c,listnum);
	};
}
void init_combobox()
{
	_init_combobox(0);
	_init_combobox(1);
}

//////////////////
// Debug Devkit //
//////////////////
void init_devkit()
{
	Toolbox& box = boxes[TB_DEV_DEBUG];
	box.title = "DevKit";
	box.pos(0,0,200,80);
	box.flags |= TBF_NO_RESIZE | TBF_NO_REORDER | TBF_VISIBLE;
	box.proc = [&](Toolbox* tb, int32_t msg, int32_t c)
	{
		switch (msg)
		{
			case MG_MSG_REDRAW_SELF:
			{
				tb->baseproc(msg,c);
				char buf[2048] = { 0 };
					Toolbox const& f_box = boxes[focusOrder.back()];
				char cbuf[4];
				if(tb->data[0])
					sprintf(cbuf,"'%c'",tb->data[0]);
				else strcpy(cbuf, "NUL");
				sprintf(buf, "GridSize: %d :: Mouse %03d,%03d"
					"\nFocused: %s, (%d,%d) (%d,%d)\n'%s' %s", gridsize, mx, my,
					f_box.title.c_str(), f_box.x, f_box.y, f_box.w, f_box.h,
					tb->dp ? (char*)tb->dp : "", cbuf);
				int32_t px = gui_textout_ln(tb->getBitmap(), lfont_l, (unsigned char*)buf,
					tb->w / 2, 26, vc(15), -1, 1);
				if(tb->w < 8+px)
				{
					tb->w = px + 8;
					tb->baseproc(msg,c);
					gui_textout_ln(tb->getBitmap(), lfont_l, (unsigned char*)buf,
						tb->w / 2, 26, vc(15), -1, 1);
				}
				break;
			}
			case MG_MSG_VSYNC:
				tb->flags |= TBF_DIRTY;
				break;
			case MG_MSG_KEY:
			{
				tb->dp = (void*)scancode_to_name(c);
				return MG_RET_CHAR_WANTSMORE;
			}
			case MG_MSG_CHAR:
			{
				tb->data[0] = c;
				return MG_RET_USECHAR;
			}
			case MG_MSG_BUILD_RCMENU:
			{
				tb->add_to_rcmenu("Grid Size");
				tb->add_to_rcmenu("Toggle Grid");
				tb->add_to_rcmenu("");
				tb->baseproc(msg,c);
				break;
			}
			case MG_MSG_MANAGE_MENU:
			{
				MENU* m = (MENU*)tb->dp;
				SETFLAG(m[1].flags, D_SELECTED, show_grid);
				break;
			}
			case MG_MSG_OPEN_RCMENU:
			{
				int32_t m = tb->popup_rclick_menu();
				switch(m)
				{
					case 0: //Gridsize
					{
						gridsize = vbound(cheap_getnum("Gridsize (8-50)", gridsize), 8, 50);
						break;
					}
					case 1: //Grid
					{
						show_grid = !show_grid;
						break;
					}
				}
				break;
			}
			default: return tb->baseproc(msg, c);
		}
		return MG_RET_OK;
	};
}
//Main initializer
void init_toolboxes()
{
	Toolbox::init_base_rcmenu();
	focusOrder.clear();
	drawOrder.clear();
	for(auto q = 0; q < NUM_TB; ++q)
	{
		focusOrder.push_back(q);
		drawOrder.push_back(q);
	}
	init_static_vars();
	init_combobox();
	init_devkit();
	
	Toolbox& box = boxes[TB_TEST];
	box.flags |= TBF_VISIBLE;
	box.pos(200,200,64,64);
	
	
	// Finalizing code
	broadcast_tb_message(MG_MSG_BUILD_RCMENU);
	init_tbmenu();
}

////////////////
////////////////

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

static int32_t cached_cursordir = -1;
void cache_tb_cursor()
{
	cached_cursordir = cursor_dir;
	setcurs(CDIR_BASIC);
}
void restore_tb_cursor()
{
	setcurs(cached_cursordir);
}

///////////////////
// Move / Resize //
///////////////////
int32_t snap_to_grid(int32_t val, int32_t gridwid, int32_t gridoffs = 0)
{
	val = (val-gridoffs)+gridwid/2;
	return val - (val%gridwid) + gridoffs;
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
			
			bool shift = key[KEY_LSHIFT] || key[KEY_RSHIFT];
			auto gx = shift ? snap_to_grid(mx-dx,gridsize) : mx-dx;
			auto gy = shift ? snap_to_grid(my-dy,gridsize,THEMENU_HEI) : my-dy;
			
			int32_t nx = vbound(gx,0,zq_screen_w-box.w);
			if(THEMENU_HEI > zq_screen_h-box.h)
				box.h = zq_screen_h-THEMENU_HEI;
			int32_t ny = vbound(gy,THEMENU_HEI,zq_screen_h-box.h);
			box.pos(nx,ny);
			box.flags |= TBF_DIRTY;
		}
		if(tb_draw_vsync() || (box.flags&TBF_DIRTY))
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
			bool shift = key[KEY_LSHIFT] || key[KEY_RSHIFT];
			auto gx = shift ? snap_to_grid(mx,gridsize) : mx;
			auto gy = shift ? snap_to_grid(my,gridsize,THEMENU_HEI) : my;
			int32_t lx = vbound(gx,0,zq_screen_w-1);
			int32_t ly = vbound(gy,0,zq_screen_h-1);
			if(dx < 0)
				box.rsz_left(box.x-lx);
			if(dy < 0)
				box.rsz_up(box.y-ly);
			if(shift)
			{
				--lx;
				--ly;
			}
			if(dx > 0)
				box.rsz_right(lx-(box.x+box.w-1));
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

//////////////////
// Main Handler //
//////////////////
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
		bool lclick = (mb&1) && !(omb&1);
		bool rclick = (mb&2) && !(omb&2);
		auto id = broadcast_mousefind();
		bool rsz = m_curs_flags & MCFLAG_RESIZE_OR_MOVE;
		bool dis = m_curs_flags & MCFLAG_DISABLED_CLICK;
		bool samehov = lhov_id == id;
		if(!samehov)
		{
			position_mouse_z(0);
			lhov_id = id;
		}
		if(dis || id < 0)
		{
			setcurs(CDIR_BASIC);
		}
		else if(rsz)
		{
			setcurs(dir_to_cdir(boxes[id].getResizeDir()));
			if(lclick) //lclick
			{
				focus(id);
				resize(id);
				position_mouse_z(0);
				runToolboxes();
				return;
			}
			else if(rclick) //rclick
			{
				if(boxes[id].getResizeDir() == CDIR_MOVE)
				{
					boxes[id].msg(MG_MSG_OPEN_RCMENU);
				}
			}
		}
		else
		{
			setcurs(CDIR_BASIC);
			if(lclick) //lclick
			{
				focus(id);
				boxes[id].msg(MG_MSG_CLICK);
			}
			if(rclick) //rclick
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
	if(keypressed())
	{
		int32_t scancode;
		auto ascii = ureadkey(&scancode);
		auto ind = broadcast_tb_message(MG_MSG_KEY, scancode);
		if(ind < 0)
		{
			ind = broadcast_tb_message(MG_MSG_CHAR, ascii);
		}
	}
	broadcast_tb_message(MG_MSG_IDLE);
	broadcast_tb_message(MG_MSG_REDRAW_BOX);
	all_mark_screen_dirty();
	sp_release_screen_all();
}


//!///////////
//! Testing //
//!///////////
bool IS_MSGUI_MODE = false;
extern bool close_button_quit;
extern DIALOG_PLAYER* main_dl_player;
void draw_the_menu();
void draw_grid(int32_t wid, int32_t hei, int32_t offsx, int32_t offsy, int32_t color)
{
	for(auto y = offsy%hei; y < zq_screen_h; y += hei)
	{
		line(screen,0,y,zq_screen_w,y,color);
	}
	for(auto x = offsx%wid; x < zq_screen_w; x += wid)
	{
		line(screen,x,0,x,zq_screen_h,color);
	}
}
void test_grid()
{
	if(!show_grid) return;
	sp_acquire_screen();
	const auto gs_thresh = 32;
	if(gridsize >= gs_thresh)
	{
		draw_grid(gridsize,gridsize,0,THEMENU_HEI,vc(15));
	}
	else
	{
		int32_t sz, q;
		for(q = 2, sz = 2*gridsize; sz < gs_thresh; sz = q*gridsize, ++q);
		for(int32_t p = 0; p < q; ++p)
		{
			draw_grid(sz,sz,p*gridsize,THEMENU_HEI+(p*gridsize),vc(15-p));
		}
	}
	sp_release_screen();
}
void test_mainscreen_gui()
{
	IS_MSGUI_MODE = true;
	while(key[KEY_G]);
	broadcast_tb_message(MG_MSG_REDRAW_SELF);
	while(true)
	{
		if(key[KEY_ESC]) break;
		sp_acquire_screen();
		scare_mouse();
		clear_to_color(screen, vc(0));
		test_grid();
		draw_the_menu();
		tb_draw_vsync();
		runToolboxes();
		unscare_mouse();
		if(lhov_id == -1)
		{
			update_tb_menu();
			if(!update_dialog(main_dl_player)) break;
		}
		sp_release_screen();
		do_sleep(1);
		if(close_button_quit)
		{
			close_button_quit=false;
			break;
		}
	}
	clear_to_color(screen, vc(0));
	while(key[KEY_ESC]);
	sp_release_screen_all();
	IS_MSGUI_MODE = false;
}

