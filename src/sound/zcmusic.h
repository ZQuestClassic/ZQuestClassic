/* ZQuest Classic Music Library        	*/
/* ------------------------------------ */
/* Wrapper for transparently extending	*/
/* supported music file formats.      	*/

#ifndef _ZCMUSIC_H_
#define _ZCMUSIC_H_

#include <cstdint>
#include <string>

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#endif

#define ZCMF_DUH      0x00000001
#define ZCMF_OGG      0x00000002
#define ZCMF_MP3      0x00000004
#define ZCMF_GME	  0x00000008

#define ZCM_PLAYING 1
#define ZCM_STOPPED 0
#define ZCM_PAUSED -1

#define ZCM_PAUSE   1
#define ZCM_RESUME  0
#define ZCM_TOGGLE -1

constexpr char * zcmusic_types = "it;mod;mp3;ogg;s3m;spc;gym;nsf;gbs;vgm;xm";

typedef struct
{
    int32_t type;                                               // uses ZCMF defines
    int32_t playing;                                            // -1 = paused, 0 = stopped, 1 = playing
    int32_t position;                                           // Only needed to sync Triforce jingle
    char filename[256];
    int32_t track;
    int32_t fadevolume;
    int32_t fadeoutframes;
#ifdef __EMSCRIPTEN__
    Mix_Music* mus;
#endif
} ZCMUSIC;

ZCMUSIC* zcmusic_load_for_quest(const char* filename, const char* quest_path);

bool zcmusic_init(int32_t flags = -1);
bool zcmusic_poll(int32_t flags = -1);
void zcmusic_exit();

ZCMUSIC * zcmusic_load_file(const char *filename);
bool zcmusic_play(ZCMUSIC* zcm, int32_t vol);
bool zcmusic_pause(ZCMUSIC* zcm, int32_t pause);
bool zcmusic_stop(ZCMUSIC* zcm);
bool zcmusic_set_volume(ZCMUSIC* zcm, int32_t vol);
void zcmusic_unload_file(ZCMUSIC* &zcm);
int32_t zcmusic_get_tracks(ZCMUSIC* zcm);
int32_t zcmusic_change_track(ZCMUSIC* zcm, int32_t tracknum);
std::string zcmusic_get_track_name(ZCMUSIC* zcm, int32_t tracknum);
int32_t zcmusic_get_curpos(ZCMUSIC* zcm);
void zcmusic_set_curpos(ZCMUSIC* zcm, int32_t value);
void zcmusic_set_speed(ZCMUSIC* zcm, int32_t value);
int32_t zcmusic_get_length(ZCMUSIC* zcm);
void zcmusic_set_loop(ZCMUSIC* zcm, double start, double end);
int32_t zcmusic_get_type(ZCMUSIC* zcm);

#undef ZCM_EXTERN
#endif