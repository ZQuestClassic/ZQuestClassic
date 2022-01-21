
#include "zdefs.h"
#include "jwinfsel.h"
#include "zsys.h"
extern volatile bool close_button_quit;
extern int32_t zq_screen_w, zq_screen_h;
extern bool is_large;
extern char temppath[4096], rootpath[4096];

bool getname(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename);
void launch_process(char const* relative_path);
