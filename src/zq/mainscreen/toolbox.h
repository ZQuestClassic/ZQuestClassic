
#ifndef ZQ_MS_TOOLBOX_H
#define ZQ_MS_TOOLBOX_H

#include "mainscreen.h"
#include <functional>

class Toolbox
{
public:
	Toolbox();
	~Toolbox();
	
	int32_t msg(int32_t m, int32_t c = 0);
	std::function<int32_t(Toolbox* tb,int32_t msg,int32_t c)> proc;
	//Saveables
	int32_t x, y, w, h;
	uint32_t flags;
	uint32_t zoomfactor;
	uint32_t scrollfactor;
	//Unsavables
	int32_t dx[10], dy[10], dw[10], dh[10];
	int32_t minw, minh, maxw, maxh;
	BITMAP* refbmp;
	std::string title;
	void *dp, *dp2;
	int32_t data[10];
	
	//RClick Menu
	void add_to_rcmenu(std::string const& name);
	void finish_rcmenu();
	static void init_base_rcmenu();
	int32_t popup_rclick_menu();
	
	//Default behavior functions
	int32_t mouse_in_d();
	int32_t getResizeDir();
	int32_t getResizeDir(int32_t mx, int32_t my);
	bool hovering(int32_t mx, int32_t my);
	void pos(int32_t nx, int32_t ny);
	void pos(int32_t nx, int32_t ny, int32_t nw, int32_t nh);
	void minsz(int32_t mw, int32_t mh);
	void maxsz(int32_t mw, int32_t mh);
	int32_t baseproc(int32_t msg,int32_t c = 0);
	void sanity();
	//Resizing
	void rsz_left(int32_t px);
	void rsz_right(int32_t px);
	void rsz_up(int32_t px);
	void rsz_down(int32_t px);
	//Private accessor functions
	BITMAP* getBitmap() {return intbmp;}
	BITMAP const* getBitmap() const {return intbmp;}
	void newBitmap(); //Ensure a clear bitmap of exactly the toolbox's WxH
	void newBitmap(uint32_t w, uint32_t h); //Ensure a clear bitmap of exactly WxH
	void clearBitmap(); //clear the existing bitmap
private:
	BITMAP* intbmp;
	
	static std::vector<std::string> basercmenu;
	std::vector<std::string> rcmenu;
	int32_t baserc_len;
	bool rcmenu_finalized;
	
	int32_t call(int32_t m, int32_t c = 0);
};


#define TBF_VISIBLE    0x00000001
#define TBF_DIRTY      0x00000002
#define TBF_DISABLED   0x00000004
#define TBF_PINNED     0x00000008
#define TBF_NO_REORDER 0x00000010
#define TBF_NO_RESIZE  0x00000020
#define TBF_DOCKED     0x00000040
#define TBF_NO_DOCKING 0x00000080


#define BGLOOP_START()\
BITMAP* restore_bg = create_bitmap_ex(8,screen->w,screen->h);\
do{\
	blit(screen,restore_bg,0,0,0,0,screen->w,screen->h);\
	sp_acquire_screen();\
	draw_docks();\
	broadcast_tb_message(MG_MSG_REDRAW_BOX);\
	sp_release_screen_all();\
	unscare_mouse();\
}while(false)
#define BGLOOP_DRAW()\
do{\
	sp_acquire_screen();\
	scare_mouse();\
	blit(restore_bg,screen,0,0,0,0,screen->w,screen->h);\
	draw_docks();\
	broadcast_tb_message(MG_MSG_REDRAW_BOX);\
	unscare_mouse();\
	all_mark_screen_dirty();\
	sp_release_screen();\
}while(false)
	
#define BGLOOP_DRAW1()\
do{\
	sp_acquire_screen();\
	scare_mouse();\
}while(false)
#define BGLOOP_DRAW2()\
do{\
	blit(restore_bg,screen,0,0,0,0,screen->w,screen->h);\
	draw_docks();\
	broadcast_tb_message(MG_MSG_REDRAW_BOX);\
}while(false)
#define BGLOOP_DRAW3()\
do{\
	unscare_mouse();\
	all_mark_screen_dirty();\
	sp_release_screen();\
}while(false)

#define BGLOOP_END()\
do{\
	sp_acquire_screen();\
	scare_mouse();\
	blit(restore_bg,screen,0,0,0,0,screen->w,screen->h);\
	destroy_bitmap(restore_bg);\
}while(false)

#endif

