#include <stddef.h>
#include "zcmusic.h"
#include "base/zsys.h"
#include "base/zc_alleg.h"
#include <SDL2/SDL_mixer.h>
#include <filesystem>
#include <gme.h>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

int32_t zcmusic_bufsz = 64;
static std::vector<ZCMUSIC*> playlist;
static ALLEGRO_MUTEX *playlistmutex = NULL;

// TODO: dedupe this function.
ZCMUSIC* zcmusic_load_for_quest(char* filename, char* quest_path)
{
    char exe_path[PATH_MAX];
    get_executable_name(exe_path, PATH_MAX);
    auto exe_dir = std::filesystem::path(exe_path).parent_path();
    auto quest_dir = std::filesystem::path(quest_path).parent_path();

    for (int i = 0; i <= 5; i++)
    {
        std::filesystem::path dir;
        if (i == 0)
        {
            dir = exe_dir;
        }
        else if (i == 1)
        {
            dir = quest_dir;
        }
        else if (i == 2)
        {
            dir = exe_dir / std::filesystem::path(quest_path).filename();
            dir += "_music";
        }
        else if (i == 3)
        {
            dir = exe_dir / "music";
        }
        else if (i == 4)
        {
            dir = quest_dir / std::filesystem::path(quest_path).filename();
            dir += "_music";
        }
        else if (i == 5)
        {
            dir = quest_dir / "music";
        }

        auto path = dir / filename;
        if (!std::filesystem::exists(path))
            continue;

		ZCMUSIC *newzcmusic = zcmusic_load_file(path.string().c_str());
        if (newzcmusic)
            return newzcmusic;
    }

    return nullptr;
}

void zcm_extract_name(const char *path, char *name, int32_t type)
{
  int32_t l = (int32_t)strlen(path);
  int32_t i = l;

  while (i > 0 && path[i - 1] != '/' && path[i - 1] != '\\')
    --i;

  int32_t n = 0;

  if (type == FILENAME8__)
  {
    while (i < l && n < 8 && path[i] != '.')
      name[n++] = path[i++];
  }
  else if (type == FILENAME8_3)
  {
    while (i < l && n < 12)
      name[n++] = path[i++];
  }
  else
  {
    while (i < l)
      name[n++] = path[i++];
  }

  name[n] = 0;
}

bool zcmusic_init(int32_t flags)
{
  if (!playlistmutex) playlistmutex = al_create_mutex();

  if (Mix_OpenAudioDevice(22050, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 0, NULL, 0) < 0)
  {
    al_trace("Mix_OpenAudioDevice error: %s\n", Mix_GetError());
    return false;
  }

  return true;
}
bool zcmusic_poll(int32_t flags) {
  al_lock_mutex(playlistmutex);

  std::vector<ZCMUSIC*>::iterator b = playlist.begin();
  while(b != playlist.end())
  {
      switch((*b)->playing)
      {
      case ZCM_STOPPED:
          // if it has stopped, remove it from playlist;
          b = playlist.erase(b);
          break;
      case ZCM_PLAYING:
          (*b)->position++;
          b++;
          break;
      default:
          b++;
      }
  }

  al_unlock_mutex(playlistmutex);

  return true;
}
void zcmusic_exit() {}

ZCMUSIC *zcmusic_load_file(const char *filename)
{
#ifdef __EMSCRIPTEN__
  if (strncmp("/_quests/", filename, strlen("/_quests/")) == 0)
  {
    em_fetch_file(filename);
  }
#endif

  Mix_Music *mus = Mix_LoadMUS(filename);
  if (!mus)
  {
    al_trace("Mix_LoadMUS error: %s\n", Mix_GetError());
    return NULL;
  }

  ZCMUSIC *music = (ZCMUSIC *)malloc(sizeof(ZCMUSIC));
  zcm_extract_name(filename, music->filename, FILENAMEALL);
  music->filename[255] = '\0';
  music->track = 0;
  music->playing = ZCM_STOPPED;
  music->position = 0;
  music->mus = mus;

  return music;
}
ZCMUSIC *zcmusic_load_file_ex(const char *filename) { return NULL; }
bool zcmusic_play(ZCMUSIC *zcm, int32_t vol)
{
  if (Mix_PlayMusic(zcm->mus, -1) < 0)
  {
    zcm->playing = ZCM_STOPPED;
    al_trace("Mix_PlayMusic error: %s\n", Mix_GetError());
    return 1;
  }

  // In case it was paused.
  zcmusic_set_volume(zcm, vol);
  Mix_ResumeMusic();
  zcm->playing = ZCM_PLAYING;

  al_lock_mutex(playlistmutex);
  playlist.push_back(zcm);
  al_unlock_mutex(playlistmutex);

  return 0;
}
bool zcmusic_pause(ZCMUSIC *zcm, int32_t pause)
{
  if (zcm == NULL) return false;

  al_lock_mutex(playlistmutex);

  if (pause == ZCM_TOGGLE) {
    pause = (zcm->playing == ZCM_PAUSED) ? ZCM_RESUME : ZCM_PAUSE;
  }

  if (pause == ZCM_RESUME) {
    Mix_ResumeMusic();
    zcm->playing = ZCM_PLAYING;
  } else if (pause == ZCM_PAUSE) {
    Mix_PauseMusic();
    zcm->playing = ZCM_PAUSED;
  }

  al_unlock_mutex(playlistmutex);
  return true;
}
bool zcmusic_stop(ZCMUSIC *zcm)
{
  if (zcm == NULL) return false;

  al_lock_mutex(playlistmutex);

  Mix_HaltMusic();
  zcm->playing = 0;

  al_unlock_mutex(playlistmutex);
  return true;
}
bool zcmusic_set_volume(ZCMUSIC* zcm, int32_t vol)
{
	if (zcm == NULL) return false;

	// TODO

	return true;
}
void zcmusic_unload_file(ZCMUSIC *&zcm)
{
  if (zcm == NULL)
    return;

  al_lock_mutex(playlistmutex);

  for (auto it = playlist.begin(); it != playlist.end(); it++) {
    if (*it == zcm) {
      it = playlist.erase(it);
      break;
    }
  }

  Mix_FreeMusic(zcm->mus);
  zcm->mus = NULL;
  free(zcm);
  zcm = NULL;

  al_unlock_mutex(playlistmutex);
}
int32_t zcmusic_get_tracks(ZCMUSIC *zcm) {
  if (zcm == NULL)
    return 0;

  int result = Mix_GetNumTracks(zcm->mus);
  if (result == -1) return 0;
  return result;
}
std::string zcmusic_get_track_name(ZCMUSIC *zcm, int track) {
  if (zcm == NULL)
    return "";

  // return Mix_GetMusicTitleTag(zcm->mus, track);
  return Mix_GetMusicTitleTag(zcm->mus);
}
int32_t zcmusic_change_track(ZCMUSIC *zcm, int32_t tracknum) {
  Mix_StartTrack(zcm->mus, tracknum);
  return 0;
}

// TODO implement
int32_t zcmusic_get_curpos(ZCMUSIC *zcm) { return 0; }
void zcmusic_set_curpos(ZCMUSIC *zcm, int32_t value) {}
void zcmusic_set_speed(ZCMUSIC *zcm, int32_t value) {}
int32_t zcmusic_get_length(ZCMUSIC* zcm) { return 0; }
void zcmusic_set_loop(ZCMUSIC* zcm, double start, double end) {}
int32_t zcmusic_get_type(ZCMUSIC* zcm)
{
	return zcm->type;
}

// TODO implement
ZCMIXER* zcmixer_create()
{
	return nullptr;
}
void zcmixer_update(ZCMIXER* mix, int32_t basevol, int32_t uservol, bool oldscriptvol) {}
void zcmixer_exit(ZCMIXER* &mix) {}
