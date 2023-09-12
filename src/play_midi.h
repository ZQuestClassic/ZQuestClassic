// This file only exists because there is no MIDI player interface for SDL/emscripten to use.
// See third_party/allegro_legacy/src/a5/midia5/sdl/sdl_midi.c
// Future work could remove this file if the above interface were fleshed out. Probably needs to
// use Timidity directly.
// For now, all original calls to allegro4 midi functions have been replaced with these,
// which will route to the original allegro4 api on all platforms except emscripten,
// which will instead load the MIDI file upfront and kick it over to SDL_Mixer.

#ifndef __play_midi_h_
#define __play_midi_h_

#include "base/zc_alleg.h"

int zc_play_midi(MIDI *midi, int loop);
void zc_midi_pause();
void zc_midi_resume();
void zc_stop_midi();
void zc_set_volume(int digi_volume, int midi_volume);
void zc_midi_seek(int pos);

#endif
