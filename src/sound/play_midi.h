// This file only exists because there is no MIDI player interface for SDL/emscripten to use.
// See third_party/allegro_legacy/src/a5/midia5/sdl/sdl_midi.c
// Future work could remove this file if the above interface were fleshed out. Probably needs to
// use Timidity directly.
// For now, all original calls to allegro4 midi functions have been replaced with these,
// which will route to the original allegro4 api on all platforms except emscripten,
// which will instead load the MIDI file upfront and kick it over to SDL_Mixer.

#pragma once

#include <allegro/midi.h>

namespace midi::play_midi
{
    int play(MIDI* midi, int loop);
    void pause();
    void resume();
    void stop();
    void set_volume(int digi_volume, int midi_volume);
    void seek(int pos);
} // namespace midi::play_midi
