
#include "zdefs.h"

const char months[13][13] =
{ 
	"Nonetober", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

char *VerStr(int32_t version)
{
    static char ver_str[12];
    sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
    return ver_str;
}

void set_config_standard()
{
	#ifdef IS_ZQUEST
	set_config_file("zquest.cfg");
	#elif defined IS_LAUNCHER
	set_config_file("zcl.cfg");
	#elif defined IS_PLAYER
	set_config_file("zc.cfg");
	#else
	set_config_file("ag.cfg");
	#endif
}

