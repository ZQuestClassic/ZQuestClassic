#include "play_midi.h"
#include "midi.h"
#include <base/zc_alleg.h>

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#endif

namespace midi::play_midi
{
#ifdef __EMSCRIPTEN__
  namespace
  {
    MIDI* current_midi = nullptr;
    Mix_Music* current_mus = nullptr;
    bool has_opened_audio;

    void on_music(void* udata, Uint8* stream, int len)
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

    int play_midi_em(MIDI* const midi, const int32_t loop)
    {
      if (!midi)
      {
        if (current_mus)
        {
          Mix_HaltMusic();
          current_midi = nullptr;
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
        if (Mix_OpenAudioDevice(22050, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 0, nullptr, 0) < 0)
        {
          al_trace("Mix_OpenAudioDevice error: %s\n", Mix_GetError());
          return 1;
        }
        Mix_SetPostMix(on_music, nullptr);
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
        current_mus = nullptr;
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
    }
  } // namespace
#endif

  int play(MIDI* const midi, const int loop)
  {
#ifdef __EMSCRIPTEN__
    return play_midi_em(midi, loop);
#else
    return ::play_midi(midi, loop);
#endif
  }

  void pause()
  {
#ifdef __EMSCRIPTEN__
    Mix_PauseMusic();
#else
    ::midi_pause();
#endif
  }

  void resume()
  {
#ifdef __EMSCRIPTEN__
    Mix_ResumeMusic();
#else
    ::midi_resume();
#endif
  }

  void stop()
  {
    play(nullptr, false);
  }

  void set_volume(const int digi_volume, int midi_volume)
  {
#ifdef __EMSCRIPTEN__
    if (!has_opened_audio) return;
    // SDL_mixer volume is 0-128, but allegro is 0-255
    midi_volume /= 2;
    // Also, for some reason music is really loud compared to SFX, so dampen it
    Mix_VolumeMusic(midi_volume * 0.5);
#else
    ::set_volume(digi_volume, midi_volume);
#endif
  }

  void seek(int pos)
  {
#ifdef __EMSCRIPTEN__
    Mix_SetMusicBeat(pos - 1);
#else
    ::midi_seek(pos);
#endif
  }
} // namespace midi::play_midi
