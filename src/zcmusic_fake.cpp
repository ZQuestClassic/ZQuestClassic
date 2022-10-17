#include <stddef.h>
#include "zcmusic.h"

int32_t zcmusic_bufsz = 64;

bool zcmusic_init(int32_t flags) { return false; }
bool zcmusic_poll(int32_t flags) { return false; }
void zcmusic_exit() {}

ZCMUSIC const *zcmusic_load_file(char *filename) { return NULL; }
ZCMUSIC const *zcmusic_load_file_ex(char *filename) { return NULL; }
bool zcmusic_play(ZCMUSIC *zcm, int32_t vol) { return false; }
bool zcmusic_pause(ZCMUSIC *zcm, int32_t pause) { return false; }
bool zcmusic_stop(ZCMUSIC *zcm) { return false; }
void zcmusic_unload_file(ZCMUSIC *&zcm) {}
int32_t zcmusic_get_tracks(ZCMUSIC *zcm) { return 0; }
int32_t zcmusic_change_track(ZCMUSIC *zcm, int32_t tracknum) { return 0; }
int32_t zcmusic_get_curpos(ZCMUSIC *zcm) { return 0; }
void zcmusic_set_curpos(ZCMUSIC *zcm, int32_t value) {}
void zcmusic_set_speed(ZCMUSIC *zcm, int32_t value) {}
