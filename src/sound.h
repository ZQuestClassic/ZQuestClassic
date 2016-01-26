#ifndef _ZC_SOUND_H_
#define _ZC_SOUND_H_

bool try_zcmusic(char *filename, int track, int midi);
void jukebox(int index);
void jukebox(int index,int loop);
void play_DmapMusic();
void playLevelMusic();
bool get_midi_paused();
void set_midi_paused(bool paused);
void music_pause();
void music_resume();
void music_stop();
void master_volume(int dv,int mv);
void set_all_voice_volume(int sfx_volume);
void Z_init_sound();
void sfx_cleanup();
bool sfx_init(int index);
void sfx(int index,int pan,bool loop, bool restart = true);
bool sfx_allocated(int index);
void cont_sfx(int index);
void stop_sfx(int index);
void adjust_sfx(int index,int pan,bool loop);
void pause_sfx(int index);
void resume_sfx(int index);
void pause_all_sfx();
void resume_all_sfx();
void stop_sfx(int index);
void stop_item_sfx(int family);
void kill_sfx();
int pan(int x);

inline void sfx(int index)
{
    sfx(index, 128, false);
}

inline void sfx(int index,int pan)
{
    sfx(index, pan<0 ? 0 : (pan>255 ? 255 : pan), false);
}

#endif
