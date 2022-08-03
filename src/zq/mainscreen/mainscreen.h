
#ifndef ZQ_MAINSCREEN_H
#define ZQ_MAINSCREEN_H
#include "base/zdefs.h"

#define MAINSCREEN_DEBUG 1

void do_sleep(int32_t ms);
void save_mainscreen_configs();
void load_mainscreen_configs();
MENU* populate_menu_from_vec(std::vector<std::string> const& mnu);
int32_t popup_menu_from_vec(std::vector<std::string> const& mnu, int32_t x = -1, int32_t y = -1, int32_t minw = 0, int32_t minh = 0);
void cache_tb_cursor();
void restore_tb_cursor();
void init_toolboxes();
void broadcast_tb_message(int32_t msg, int32_t c = 0);
void runToolboxes();
void test_mainscreen_gui();

#define MG_MSG_CLICK                  0 //Clicked inside the proc
#define MG_MSG_RCLICK                 1 //RClicked inside the proc
#define MG_MSG_FIND_MOUSE_HOVER       2 //Check if mouse can click/resize the proc
#define MG_MSG_REDRAW_SELF            3 //Redraw the personal internal bitmap
#define MG_MSG_REDRAW_BOX             4 //Redraw the proc to the screen
#define MG_MSG_SCROLL                 5 //Scrollwheeled
#define MG_MSG_IDLE                   6 //Idling
#define MG_MSG_RESZ_UP                7 //Resizing (top handle)
#define MG_MSG_RESZ_DOWN              8 //Resizing (bottom handle)
#define MG_MSG_RESZ_LEFT              9 //Resizing (left handle)
#define MG_MSG_RESZ_RIGHT            10 //Resizing (right handle)
#define MG_MSG_VSYNC                 11 //Sent approx. 60 times per second
#define MG_MSG_BUILD_RCMENU          12 //A signal to initialize the Right-Click menu
#define MG_MSG_OPEN_RCMENU           13 //A signal to open the Right-Click menu
#define MG_MSG_MANAGE_MENU           14 //A signal to manage a MENU* object (stored in dp)

#define MG_RET_OK                     0
#define MG_RET_CANRESIZE              1
#define MG_RET_CANCLICK               2
#define MG_RET_PINNEDMENU             3
#define MG_RET_DISCLICK               4


extern bool IS_MSGUI_MODE;
#endif

