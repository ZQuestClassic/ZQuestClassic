#ifndef ZC_COMMANDS_H_
#define ZC_COMMANDS_H_

#include "base/ints.h"

void zplayer_handle_commands();
void do_dump_qrs_command(const char* quest_path);
void do_dump_screen_command(const char* quest_path, int map, int screen);
void do_dump_dmaps_command(const char* quest_path);

#endif
