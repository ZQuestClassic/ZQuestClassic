#ifndef ZALLEG_ZALLEG_H_
#define ZALLEG_ZALLEG_H_

#include "zinfo.h"

struct DATAFILE;
enum App;

extern DATAFILE *sfxdata;
extern bool sound_was_installed;

void zalleg_setup_allegro(App id, int argc, char **argv);
void zalleg_create_window();
void zalleg_wait_for_all_keys_up();

// TODO: move base/zc_alleg stuff here.

#endif
