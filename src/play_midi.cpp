#include "play_midi.h"
#include "midi.h"

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#endif

#ifdef __EMSCRIPTEN__
MIDI *current_midi = NULL;
Mix_Music *current_mus = NULL;
bool has_opened_audio;

void on_music(void *udata, Uint8 *stream, int len)
{
  if (current_midi && current_mus)
    midi_pos = Mix_GetMusicBeat();
  else
    midi_pos = -1;
}

void on_music_finished()
{
  midi_pos = -1;
}
#endif

int play_midi_em(MIDI *midi, int32_t loop)
{
#ifdef __EMSCRIPTEN__
  if (!midi)
  {
    if (current_mus)
    {
      Mix_HaltMusic();
      current_midi = NULL;
      midi_pos = -1;
    }
    return 0;
  }

  if (current_midi == midi)
  {
    return 0;
  }

  if (!has_opened_audio)
  {
    if (Mix_OpenAudioDevice(22050, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 0, NULL, 0) < 0)
    {
      al_trace("Mix_OpenAudioDevice error: %s\n", Mix_GetError());
      return 1;
    }
    Mix_SetPostMix(on_music, NULL);
	Mix_HookMusicFinished(on_music_finished);
    has_opened_audio = true;
  }

  if (save_midi("/tmp/midi.mid", midi))
  {
    al_trace("save_midi error\n");
    return 1;
  }

  if (current_mus)
  {
    Mix_FreeMusic(current_mus);
    current_mus = NULL;
  }

  current_mus = Mix_LoadMUS("/tmp/midi.mid");
  if (!current_mus)
  {
    al_trace("Mix_LoadMUS error: %s\n", Mix_GetError());
    return 1;
  }

  if (Mix_PlayMusic(current_mus, loop == 0 ? 1 : -1) < 0)
  {
    al_trace("Mix_PlayMusic error: %s\n", Mix_GetError());
    return 1;
  }

  midi_pos = 0;
  current_midi = midi;
  return 0;
#else
  return 0;
#endif
}

int zc_play_midi(MIDI *midi, int loop)
{
#ifdef __EMSCRIPTEN__
  return play_midi_em(midi, loop);
#else
  return play_midi(midi, loop);
#endif
}

void zc_midi_pause()
{
#ifdef __EMSCRIPTEN__
  Mix_PauseMusic();
#else
  midi_pause();
#endif
}

void zc_midi_resume()
{
#ifdef __EMSCRIPTEN__
  Mix_ResumeMusic();
#else
  midi_resume();
#endif
}

void zc_stop_midi()
{
  zc_play_midi(NULL, false);
}

void zc_set_volume(int digi_volume, int midi_volume)
{
#ifdef __EMSCRIPTEN__
  // SDL_mixer volume is 0-128, but allegro is 0-255
  midi_volume /= 2;
  // Also, for some reason music is really loud compared to SFX, so dampen it
  Mix_VolumeMusic(midi_volume * 0.5);
#else
  set_volume(digi_volume, midi_volume);
#endif
}

void zc_midi_seek(int pos)
{
#ifdef __EMSCRIPTEN__
  Mix_SetMusicBeat(pos - 1);
#else
  midi_seek(pos);
#endif
}
