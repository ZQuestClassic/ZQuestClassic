
#ifndef ZQ_MAINSCREEN_H
#define ZQ_MAINSCREEN_H
#include "base/zdefs.h"

#define MAINSCREEN_DEBUG 1

void do_sleep(int32_t ms);
void save_mainscreen_configs();
void load_mainscreen_configs();
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

#define MG_RET_OK                     0
#define MG_RET_CANRESIZE              1
#define MG_RET_CANCLICK               2

#endif

