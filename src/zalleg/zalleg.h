#include "zinfo.h"

struct DATAFILE;
enum App;

extern DATAFILE *sfxdata;
extern zcmodule moduledata;

void zalleg_setup_allegro(App id, int argc, char **argv);
void zalleg_create_window();

// TODO: move base/zc_alleg stuff here.
