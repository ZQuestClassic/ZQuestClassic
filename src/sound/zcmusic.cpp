/* ZQuest Classic Music Library         */
/* ------------------------------------*/
/* Wrapper for transparently extending */
/* supported music file formats.       */

#include "base/zc_alleg.h" // TODO: why do we get "_malloca macro redefinition" in Windows debug builds without this include?
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

#ifdef _DEBUG
#ifdef _malloca
#undef _malloca
#endif
#endif

#include "allegro/unicode.h"

#include <gme.h>

#include "base/zsys.h"
#include "base/util.h"
#include "sound/zcmusic.h"
#include <filesystem>
#include <stdlib.h>
#include <allegro5/allegro_audio.h>
#include <cctype>
#include <string>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

using namespace util;
using std::pair;
using std::make_pair;

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

// MSVC fix
#if _MSC_VER >= 1900
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#endif

#define DUH_CHANNELS  2                                     // stereo
#define DUH_SAMPLES   44100                                 //Hz
#define DUH_RESAMPLE  1

namespace {

ALLEGRO_MUTEX* playlistmutex;

std::filesystem::path resolve_case_insensitive(const std::filesystem::path& base, const std::string& target)
{
	std::filesystem::path exact_path = base / target;

	if (std::filesystem::exists(exact_path))
		return exact_path;

	std::string target_lower = target;
	ustrlwr(target_lower.data());

	if (std::filesystem::exists(base) && std::filesystem::is_directory(base))
	{
		for (const auto& entry : std::filesystem::directory_iterator(base))
		{
			std::u8string entry_lower_u8 = entry.path().filename().u8string();
			std::string entry_lower(entry_lower_u8.begin(), entry_lower_u8.end());
			ustrlwr(entry_lower.data());

			if (entry_lower == target_lower)
				return entry.path();
		}
	}

	return exact_path;
}

} // end namespace

char const * zcmusic_types = "it;mod;mp3;ogg;s3m;spc;gym;nsf;gbs;vgm;xm";

pair<ZCMUSIC*,ZCM_Error> zcmusic_load_for_quest(const char* filename, const char* quest_path)
{
	if (!al_is_audio_installed())
	{
		static bool logged_no_audio;
		if (!logged_no_audio)
		{
			logged_no_audio = true;
			al_trace("Not loading music: audio is not installed\n");
		}
		return make_pair(nullptr, ZCM_E_NO_AUDIO);
	}
    char exe_path[PATH_MAX];
    get_executable_name(exe_path, PATH_MAX);
    auto exe_dir = std::filesystem::path(exe_path).parent_path();
    auto quest_dir = std::filesystem::path(quest_path).parent_path();
	// A quest path with no directory component should search relative to the
	// current directory (this is how the bundled title music is found).
	if (quest_dir.empty())
		quest_dir = ".";

	std::vector<std::filesystem::path> dirs;
	auto add_dir = [&](const std::filesystem::path& dir) {
		std::error_code ec;
		if (!std::filesystem::is_directory(dir, ec))
			return;
		if (std::find(dirs.begin(), dirs.end(), dir) == dirs.end())
			dirs.push_back(dir);
	};

	std::string music_folder_name = std::filesystem::path(quest_path).filename().string() + "_music";
	add_dir(exe_dir);
	add_dir(quest_dir);
	add_dir(resolve_case_insensitive(exe_dir, music_folder_name));
	add_dir(resolve_case_insensitive(exe_dir, "music"));
	add_dir(resolve_case_insensitive(quest_dir, music_folder_name));
	add_dir(resolve_case_insensitive(quest_dir, "music"));

	// Last resort: the quest directory's immediate subfolders, so a music
	// pack extracted into its own folder (what most archive tools do by
	// default) still works.
	std::error_code ec;
	for (const auto& entry : std::filesystem::directory_iterator(quest_dir, ec))
	{
		if (!entry.is_directory(ec))
			continue;
		if (entry.path().filename().string().starts_with('.'))
			continue;
		add_dir(entry.path());
	}

	bool any_existed = false;

	for (const auto& dir : dirs)
	{
		auto path = resolve_case_insensitive(dir, filename);
		if (!std::filesystem::exists(path))
			continue;

		any_existed = true;

		ZCMUSIC *newzcmusic = zcmusic_load_file(path.string().c_str());
		if (newzcmusic)
			return make_pair(newzcmusic, ZCM_E_OK);
	}

	if (any_existed)
	{
		al_trace("Music file '%s' was found, but failed to load (quest: '%s')\n", filename, quest_path);
	}
	else
	{
		al_trace("Music file '%s' not found in any music folder (quest: '%s'). Searched:\n", filename, quest_path);
		for (const auto& dir : dirs)
			al_trace("    %s\n", dir.string().c_str());
	}

    return make_pair(nullptr, any_existed ? ZCM_E_ERROR : ZCM_E_NOT_FOUND);
}

// used for allegro streamed music (ogg, mp3)
typedef struct ALSTREAMFILE : public ZCMUSIC
{
	ALLEGRO_AUDIO_STREAM *s;
    char *fname;
    int32_t vol;
    // Where playback would be if it stayed locked 1:1 to the 60fps game logic:
    // advanced by speed/60 per zcmusic_poll and wrapped at the loop points.
    // zcmusic_correct_pause_drift seeks the stream here on unpause.
    double frame_clock;
    double speed;
    double loop_start, loop_end;                            // seconds; loop_end <= 0 = end of file
    double length;
} ALSTREAMFILE;

typedef struct GMEFILE : public ZCMUSIC
{
    AUDIOSTREAM *stream;
    struct Music_Emu* emu;
    int32_t samples;
} GMEFILE;

#include <vector>
static std::vector<ZCMUSIC*> playlist;                      //yeah, I'm too lazy to do it myself
static int32_t libflags = 0;

// forward declarations
ALSTREAMFILE *load_alstream_file(const char *filename);
void unload_alstream_file(ALSTREAMFILE *als);

int32_t stream_getpos(ALSTREAMFILE* als);
void stream_setpos(ALSTREAMFILE* als, int32_t msecs);
void stream_setspeed(ALSTREAMFILE* als, int32_t speed);
int32_t stream_getlength(ALSTREAMFILE* als);
void stream_setloop(ALSTREAMFILE* als, double start, double end);

Music_Emu* gme_load_file(const char* filename, const char* ext);
int32_t poll_gme_file(GMEFILE *gme);
int32_t unload_gme_file(GMEFILE* gme);
int32_t gme_play(GMEFILE *gme, int32_t vol);


void zcm_extract_name(const char *path,char *name,int32_t type)
{
	int32_t l=(int32_t)strlen(path);
	int32_t i=l;

	while(i>0 && path[i-1]!='/' && path[i-1]!='\\')
		--i;

	int32_t n=0;

	if(type==FILENAME8__)
	{
		while(i<l && n<8 && path[i]!='.')
			name[n++]=path[i++];
	}
	else if(type==FILENAME8_3)
	{
		while(i<l && n<12)
			name[n++]=path[i++];
	}
	else
	{
		while(i<l)
			name[n++]=path[i++];
	}

	name[n]=0;
}

bool zcmusic_init(int32_t flags)                              /* = -1 */
{
	if(flags & ZCMF_DUH)
	{
		libflags |= ZCMF_DUH;
	}
	
	if(flags & ZCMF_OGG)
	{
		libflags |= ZCMF_OGG;
	}
	
	if(flags & ZCMF_MP3)
	{
		libflags |= ZCMF_MP3;
	}
	
	if(flags & ZCMF_GME)
	{
		libflags |= ZCMF_GME;
	}

	if (!playlistmutex) playlistmutex = al_create_mutex();
	return true;
}

bool zcmusic_poll(int32_t flags)                              /* = -1 */
{
	al_lock_mutex(playlistmutex);
	//do all kinds of gymnastics to get around Allegro stupidity
//	char *oldpwd = getCurPackfilePassword();
//	setPackfilePassword(NULL);
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
			
			switch((*b)->type & flags & libflags)             // only poll those specified by 'flags'
			{
			case ZCMF_OGG:
			case ZCMF_MP3:
			case ZCMF_DUH:
			{
				ALSTREAMFILE* als = (ALSTREAMFILE*)*b;
				als->frame_clock += als->speed / 60.0;
				double end = als->loop_end > 0 ? als->loop_end : als->length;
				if(end > als->loop_start && als->frame_clock >= end)
					als->frame_clock = als->loop_start + fmod(als->frame_clock - als->loop_start, end - als->loop_start);
				break;
			}

			case ZCMF_GME:
				if(((GMEFILE*)*b)->emu)
					poll_gme_file((GMEFILE*)*b);
					
				break;
			}
			[[fallthrough]];
		case ZCM_PAUSED:
			b++;
		}
	}
	
	al_unlock_mutex(playlistmutex);
//	setPackfilePassword(oldpwd);
//	if(oldpwd != NULL)
//		delete[] oldpwd;
	return true;
}

void zcmusic_exit()
{
	al_lock_mutex(playlistmutex);
	std::vector<ZCMUSIC*>::iterator b = playlist.begin();
	
	while(b != playlist.end())
	{
		zcmusic_unload_file(*b);
		b = playlist.erase(b);
	}
	
	playlist.clear();
	al_unlock_mutex(playlistmutex);
	
	if(libflags & ZCMF_DUH)
	{
		libflags ^= ZCMF_DUH;
	}
	
	if(libflags & ZCMF_OGG)
	{
		libflags ^= ZCMF_OGG;
	}
	
	if(libflags & ZCMF_MP3)
	{
		libflags ^= ZCMF_MP3;
	}
	
	if(libflags & ZCMF_GME)
	{
		libflags ^= ZCMF_GME;
	}
}

ZCMUSIC * zcmusic_load_file(const char *filename)
{
	if(filename == NULL)
	{
		return NULL;
	}
	
	al_trace("Trying to load %s\n", filename);

	if(strlen(filename)>255)
	{
		al_trace("Music file '%s' not loaded: filename too long\n", filename);
		return NULL;
	}

#ifdef __EMSCRIPTEN__
	if (em_is_lazy_file(filename))
		em_fetch_file(filename);
#endif

	char *ext=get_extension(filename);
	
	if((stricmp(ext,"ogg")==0) && (libflags & ZCMF_OGG))
	{
		ALSTREAMFILE *p = load_alstream_file(filename);
		
		if(!p)
		{
			al_trace("OGG file '%s' not loaded.\n",filename);
			goto error;
		}
		
		p->fname = (char*)malloc(strlen(filename)+1);
		
		if(!p->fname)
		{
			unload_alstream_file(p);
			goto error;
		}
		
		strcpy(p->fname, filename);
		p->type = ZCMF_OGG;
		p->playing = ZCM_STOPPED;
		p->fadeoutframes = 0;
		p->fadevolume = 10000;
		ZCMUSIC *music=(ZCMUSIC*)p;
		zcm_extract_name(filename, music->filename, FILENAMEALL);
		music->filename[255]='\0';
		music->track=0;
		return music;
	}
	
	if((stricmp(ext,"mp3")==0) && (libflags & ZCMF_MP3))
	{
		ALSTREAMFILE *p = load_alstream_file(filename);
		
		if(!p)
		{
			al_trace("MP3 file '%s' not found.\n",filename);
			goto error;
		}
		
		p->fname = (char*)malloc(strlen(filename)+1);
		
		if(!p->fname)
		{
			unload_alstream_file(p);
			goto error;
		}
		
		strcpy(p->fname, filename);
		p->type = ZCMF_MP3;
		p->playing = ZCM_STOPPED;
		p->fadeoutframes = 0;
		p->fadevolume = 10000;
		ZCMUSIC *music=(ZCMUSIC*)p;
		zcm_extract_name(filename, music->filename, FILENAMEALL);
		music->filename[255]='\0';
		music->track=0;
		return music;
	}
	
	if(((stricmp(ext,"it")==0) || (stricmp(ext,"s3m")==0) || (stricmp(ext,"mod")==0)|| (stricmp(ext,"xm")==0)) && libflags & ZCMF_DUH)
	{
		ALSTREAMFILE *p = load_alstream_file(filename);
		
		if(!p)
		{
			al_trace("Tracker file '%s' not found.\n",filename);
			goto error;
		}
		
		p->fname = (char*)malloc(strlen(filename)+1);
		
		if(!p->fname)
		{
			unload_alstream_file(p);
			goto error;
		}
		
		strcpy(p->fname, filename);
		p->type = ZCMF_DUH;
		p->playing = ZCM_STOPPED;
		p->fadeoutframes = 0;
		p->fadevolume = 10000;
		ZCMUSIC *music=(ZCMUSIC*)p;
		zcm_extract_name(filename, music->filename, FILENAMEALL);
		music->filename[255]='\0';
		music->track=0;
		return music;
	}
	
	if((libflags & ZCMF_GME))
	{
		if((stricmp(ext,"spc")==0) || (stricmp(ext,"gbs")==0) || (stricmp(ext,"vgm")==0)|| (stricmp(ext,"gym")==0)|| (stricmp(ext,"nsf")==0))
		{
		
			Music_Emu *emu;
			
			emu=gme_load_file(std::string(filename).c_str(), ext);
			
			if(emu)
			{
				GMEFILE *p=(GMEFILE*)malloc(sizeof(GMEFILE));
				
				if(!p) return NULL;
				
				p->type = ZCMF_GME;
				p->playing = ZCM_STOPPED;
				p->emu = emu;
				p->fadeoutframes = 0;
				p->fadevolume = 10000;
				ZCMUSIC *music=(ZCMUSIC*)p;
				zcm_extract_name(filename, music->filename, FILENAMEALL);
				music->filename[255]='\0';
				music->track=0;
				return music;
			}
			else al_trace("%s file '%s' not found.\n",ext,filename);
			
		}
	}
	
error:
	return NULL;
}

bool zcmusic_play(ZCMUSIC* zcm, int32_t vol) /* = FALSE */
{
	// the libraries require polling
	// of individual streams, so here we must keep
	// record of each file which is
	// playing, so we can iterate over all of them
	// when zcmusic_poll() is called.
	//
	// In addition, any music library which actually
	// has a 'play' function or similar will be
	// called from here.
	
	if(zcm == NULL) return FALSE;
	
	int32_t ret = TRUE;
	
	if(zcm->playing != ZCM_STOPPED)                         // adjust volume
	{
		ret = zcmusic_set_volume(zcm, vol);
	}
	else
	{
		if (vol > 255) vol = 255;
		if (vol < 0) vol = 0;

		switch(zcm->type & libflags)
		{
		case ZCMF_OGG:
		case ZCMF_MP3:
		case ZCMF_DUH:
			if (((ALSTREAMFILE*)zcm)->s != NULL)
			{
				al_set_audio_stream_gain(((ALSTREAMFILE*)zcm)->s, vol / 255.0);
				((ALSTREAMFILE*)zcm)->vol = vol;
				ret = al_set_audio_stream_playing(((ALSTREAMFILE*)zcm)->s, true);
				// Playback always begins at the start of the file here (a fresh
				// load, or zcmusic_stop rewound it) - not at the decode position,
				// which has already buffered ahead.
				((ALSTREAMFILE*)zcm)->frame_clock = 0;
			}
			else
			{
				ret = FALSE;
			}
			
			break;
			
		case ZCMF_GME:
			if(((GMEFILE*)zcm)->emu != NULL)
			{
				gme_play((GMEFILE*) zcm, vol);
			}
			
			break;
			
		}
		
		if(ret != FALSE)
		{
			zcm->position=0;
			zcm->playing = ZCM_PLAYING;
			al_lock_mutex(playlistmutex);
			playlist.push_back(zcm);
			al_unlock_mutex(playlistmutex);
		}
	}
	
	return ret!=0;
}

bool zcmusic_pause(ZCMUSIC* zcm, int32_t pause = -1)
{
	// This function suspends play of the music indicated
	// by 'zcm'. Passing 0 for pause will resume; passing
	// -1 (or if the default argument is invoked) will
	// toggle the current state; passing 1 will pause.
	if(zcm == NULL) return FALSE;
	
	al_lock_mutex(playlistmutex);
	
	if(zcm->playing != ZCM_STOPPED)
	{
		int32_t p = ZCM_PLAYING;
		
		switch(pause)
		{
		case ZCM_TOGGLE:
			p = (zcm->playing == ZCM_PAUSED) ? ZCM_PLAYING : ZCM_PAUSED;
			break;
			
		case ZCM_RESUME:
			p = ZCM_PLAYING;
			break;
			
		case ZCM_PAUSE:
			p = ZCM_PAUSED;
			break;
		}
		
		if(p != zcm->playing)                                 // if the state has actually changed
		{
			zcm->playing = p;
			
			switch(zcm->type & libflags)
			{
			case ZCMF_OGG:
			case ZCMF_MP3:
			case ZCMF_DUH:
				al_set_audio_stream_playing(((ALSTREAMFILE*)zcm)->s, p != ZCM_PAUSED);
				break;
				
			case ZCMF_GME:
				if(((GMEFILE*)zcm)->emu != NULL)
				{
					if(p == ZCM_PAUSED)
					{
						voice_stop(((GMEFILE*)zcm)->stream->voice);
					}
					else
					{
						voice_start(((GMEFILE*)zcm)->stream->voice);
					}
					
				}
				break;
			}
		}
	}
	
	al_unlock_mutex(playlistmutex);
	return TRUE;
}

bool zcmusic_correct_pause_drift(ZCMUSIC* zcm)
{
	// Pausing an Allegro stream throws away its decoded-but-unplayed buffers
	// (up to ~0.2s), so unpausing skips that audio and the music lands a
	// little further ahead of the 60fps game logic on every pause. While
	// still paused, seek back to where the frame count says playback should
	// be; the jump is inaudible across the pause's silence.
	if(zcm == NULL) return false;
	if(zcm->playing != ZCM_PAUSED) return false;

	switch(zcm->type & libflags)
	{
	case ZCMF_OGG:
	case ZCMF_MP3:
	case ZCMF_DUH:
		break;

	default:
		return false;
	}

	ALSTREAMFILE* als = (ALSTREAMFILE*)zcm;
	if(als->s == NULL) return false;

	al_lock_mutex(playlistmutex);
	double actual = al_get_audio_stream_position_secs(als->s);
	bool ret = false;

	// A large mismatch means the tracked position lost the plot (uncapped
	// fps, heavy lag, unknown stream length): adopt the stream's position
	// as the new baseline instead of making an audible jump.
	if(fabs(actual - als->frame_clock) <= 0.5)
	{
		ret = al_seek_audio_stream_secs(als->s, als->frame_clock);

		// The stream's feeder thread may have decoded one more fragment of
		// pre-seek audio right as the pause landed; pausing again flushes it.
		if(ret)
		{
			al_set_audio_stream_playing(als->s, false);
			al_trace("music resync on unpause: %+dms\n", int32_t((als->frame_clock - actual) * 1000));
		}
	}

	if(!ret)
		als->frame_clock = actual;

	al_unlock_mutex(playlistmutex);
	return ret;
}

bool zcmusic_stop(ZCMUSIC* zcm)
{
	// this function will stop playback of 'zcm' and reset
	// the stream position to the beginning.
	if (zcm == NULL) return FALSE;
	
	al_lock_mutex(playlistmutex);
	
	switch (zcm->type & libflags)
	{
	case ZCMF_OGG:
	case ZCMF_MP3:
	case ZCMF_DUH:
		al_set_audio_stream_playing(((ALSTREAMFILE*)zcm)->s, false);
		al_seek_audio_stream_secs(((ALSTREAMFILE*)zcm)->s, 0);
		break;
		
	case ZCMF_GME:
		if (((GMEFILE*)zcm)->emu != NULL)
		{
			if (zcm->playing != ZCM_STOPPED) stop_audio_stream(((GMEFILE*)zcm)->stream);
		}
		
		break;
	}

	zcm->playing = ZCM_STOPPED;
	
	al_unlock_mutex(playlistmutex);
	return TRUE;
}

bool zcmusic_set_volume(ZCMUSIC* zcm, int32_t vol)
{
	if(zcm == NULL) return FALSE;

	if (vol > 255) vol = 255;
	if (vol < 0) vol = 0;
	
	if(zcm->playing != ZCM_STOPPED)                         // adjust volume
	{
		switch(zcm->type & libflags)
		{
		case ZCMF_OGG:
		case ZCMF_MP3:
		case ZCMF_DUH:
			if(((ALSTREAMFILE*)zcm)->s != NULL)
			{
				al_set_audio_stream_gain(((ALSTREAMFILE*)zcm)->s, vol / 255.0);
				((ALSTREAMFILE*)zcm)->vol = vol;
			}
			
			break;
			
		case ZCMF_GME:
		{
			auto stream = ((GMEFILE*)zcm)->stream;
			voice_set_volume(stream->voice, vol);
		}
		break;
		}
	}
	
	return true;
}

void zcmusic_unload_file(ZCMUSIC* &zcm)
{
	// this will unload and destroy all of the data/etc.
	// associated with 'zcm'. Also sets the pointer to
	// NULL so you don't try to use it later.
	if(zcm == NULL) return;
	
	// explicitly remove it from the playlist since we're
	// freeing the memory which holds the ZCM struct.
	// don't want to leave an soon-to-be invalid pointers
	// lying around to cause crashes.
	{
		al_lock_mutex(playlistmutex);
		std::vector<ZCMUSIC*>::iterator b = playlist.begin();
		
		while(b != playlist.end())
		{
			if(*b == zcm)
			{
				b = playlist.erase(b);
			}
			else
			{
				b++;
			}
		}
		
		al_unlock_mutex(playlistmutex);
	}
	
	switch(zcm->type & libflags)
	{
	case ZCMF_OGG:
	case ZCMF_MP3:
	case ZCMF_DUH:
		unload_alstream_file((ALSTREAMFILE*)zcm);
		break;
		
	case ZCMF_GME:
		unload_gme_file((GMEFILE*)zcm);
		break;
}
	
	zcm = NULL;
	return;
}

int32_t zcmusic_get_tracks(ZCMUSIC* zcm)
{
	if(zcm == NULL) return 0;
	
	switch(zcm->type & libflags)
	{
	case ZCMF_DUH:
	case ZCMF_OGG:
	case ZCMF_MP3:
		return 0;
		break;
		
	case ZCMF_GME:
		if(((GMEFILE*)zcm)->emu != NULL)
		{
			int32_t t= gme_track_count(((GMEFILE*)zcm)->emu);
			return (t>1)?t:0;
		}
		else
		{
			return 0;
		}
		
		break;
	}
	
	return 0;
}

int32_t zcmusic_change_track(ZCMUSIC* zcm, int32_t tracknum)
{
	if(zcm == NULL) return -1;
	
	switch(zcm->type & libflags)
	{
	case ZCMF_DUH:
	case ZCMF_OGG:
	case ZCMF_MP3:
		return -1;
		break;
		
	case ZCMF_GME:
		if(((GMEFILE*)zcm)->emu != NULL)
		{
			al_lock_mutex(playlistmutex);
			int32_t t= gme_track_count(((GMEFILE*)zcm)->emu);
			
			if(tracknum<0 || tracknum>=t)
			{
				tracknum=0;
			}
			
			gme_start_track(((GMEFILE*)zcm)->emu, tracknum);

			zcm->track=tracknum;
			al_unlock_mutex(playlistmutex);
			return tracknum;
		}
		else
		{
			return -1;
		}
		
		break;
	}
	
	return 0;
}

std::string zcmusic_get_track_name(ZCMUSIC *zcm, int track)
{
	return "";
}

int32_t zcmusic_get_curpos(ZCMUSIC* zcm)
{
	if(zcm == NULL) return -10000;
	
		switch(zcm->type & libflags)
		{
		case ZCMF_OGG:
		case ZCMF_MP3:
		case ZCMF_DUH:
			return stream_getpos((ALSTREAMFILE*)zcm);
	}

	return -10000;
}
void zcmusic_set_curpos(ZCMUSIC* zcm, int32_t value)
{
	if(zcm == NULL) return;
	
		switch(zcm->type & libflags)
		{
		case ZCMF_OGG:
		case ZCMF_MP3:
		case ZCMF_DUH:
			stream_setpos((ALSTREAMFILE*)zcm, value);
			break;
	}

	return;
}
void zcmusic_set_speed(ZCMUSIC* zcm, int32_t value)
{
	if(zcm == NULL) return;
	
		switch(zcm->type & libflags)
		{
		case ZCMF_OGG:
		case ZCMF_MP3:
		case ZCMF_DUH:
			stream_setspeed((ALSTREAMFILE*)zcm, value);
			break;
	}

	return;
}

int32_t zcmusic_get_length(ZCMUSIC* zcm)
{
	if (zcm == NULL) return -10000;

	switch (zcm->type & libflags)
	{
	case ZCMF_OGG:
	case ZCMF_MP3:
	case ZCMF_DUH:
		return stream_getlength((ALSTREAMFILE*)zcm);
		break;
	}

	return -10000;
}

void zcmusic_set_loop(ZCMUSIC* zcm, double start, double end)
{
	if (zcm == NULL) return;

	switch (zcm->type & libflags)
	{
	case ZCMF_OGG:
	case ZCMF_MP3:
	case ZCMF_DUH:
		stream_setloop((ALSTREAMFILE*)zcm, start, end);
		break;
	}

	return;
}

int32_t zcmusic_get_type(ZCMUSIC* zcm)
{
	return zcm->type & libflags;
}


ALSTREAMFILE *load_alstream_file(const char *filename)
{
	ALSTREAMFILE *p = NULL;

	if((p = (ALSTREAMFILE*)malloc(sizeof(ALSTREAMFILE)))==NULL)
        return NULL;

	al_reserve_samples(5);

	ALLEGRO_AUDIO_STREAM* stream = al_load_audio_stream(filename, 4, 2048);
	if (!stream)
		goto error;
	if (!al_attach_audio_stream_to_mixer(stream, al_get_default_mixer()))
	{
		al_destroy_audio_stream(stream);
		goto error;
	}

	al_set_audio_stream_playing(stream, false);
	al_set_audio_stream_playmode(stream, ALLEGRO_PLAYMODE_LOOP);

	p->s = stream;
	p->fadeoutframes = 0;
	p->frame_clock = 0;
	p->speed = 1.0;
	p->loop_start = p->loop_end = 0;
	p->length = al_get_audio_stream_length_secs(stream);

	return p;

error:

    if(p)
        free(p);
        
    return NULL;
}

void unload_alstream_file(ALSTREAMFILE *als)
{
    if(als != NULL)
    {
        if(als->s != NULL)
        {
			al_set_audio_stream_playing(als->s, false);
			al_destroy_audio_stream(als->s);
			als->s = NULL;
        }
        
        if(als->fname != NULL)
        {
            free(als->fname);
        }
		free(als);
    }
}

int32_t stream_getpos(ALSTREAMFILE* als)
{
	if (als->s != NULL)
	{
		return int32_t(al_get_audio_stream_position_secs(als->s) * 10000.0);
	}
	return -10000;
}

void stream_setpos(ALSTREAMFILE* als, int32_t msecs)
{
	if (als->s != NULL)
	{
		// A seek alone leaves ~0.2s of already-decoded audio queued, which
		// would play out before the jump is heard. Stopping the stream
		// discards that queue (any fragment decoded post-seek is fine), so
		// the seek takes effect right away and playback stays aligned with
		// the frame clock.
		bool was_playing = al_get_audio_stream_playing(als->s);
		al_seek_audio_stream_secs(als->s, double(msecs) / 10000.0);
		al_set_audio_stream_playing(als->s, false);
		if (was_playing)
			al_set_audio_stream_playing(als->s, true);
		als->frame_clock = double(msecs) / 10000.0;
	}
}

void stream_setspeed(ALSTREAMFILE* als, int32_t speed)
{
	if (als->s != NULL)
	{
		// Allegro rejects speeds <= 0, leaving the stream at its old speed;
		// only track a speed it accepted.
		if (al_set_audio_stream_speed(als->s, float(speed / 10000.0)))
			als->speed = speed / 10000.0;
	}
}

int32_t stream_getlength(ALSTREAMFILE* als)
{
	if (als->s != NULL)
	{
		return int32_t(al_get_audio_stream_length_secs(als->s) * 10000.0);
	}
	return -10000;
}

void stream_setloop(ALSTREAMFILE* als, double start, double end)
{
	if (als->s != NULL)
	{
		// No loop set
		if (start == 0.0 && end == 0.0)
			return;
		// Don't allow end point before start point
		if (end < start)
			end = double(stream_getlength(als) / 10000.0);
		al_set_audio_stream_loop_secs(als->s, start, end);
		als->loop_start = start;
		als->loop_end = end;
	}
}

int32_t poll_gme_file(GMEFILE* gme)
{
    uint8_t *p;
    p = (uint8_t*) get_audio_stream_buffer(gme->stream);
    
    if(p)
    {
        int32_t samples=gme->samples;
        memset(p,0,4*samples);
        uint16_t *q=(uint16_t*) p;
        gme_play(gme->emu, (int32_t) 2*samples, (int16_t*)p);

        // The legacy mixer plays each voice at half amplitude (6dB of
        // per-voice headroom for panning), leaving this centered music
        // stream quieter than the OGG/MP3 streams that play through
        // Allegro 5 at full gain. Sound effects and tracker music still
        // share that mixer on this branch, so its per-voice volume can't
        // be raised wholesale; double just this stream's samples instead.
        int16_t *s=(int16_t*) p;
        for(int32_t j=0; j<2*samples; ++j)
        {
            int32_t v = 2 * (int32_t) s[j];
            if(v > 32767) v = 32767;
            else if(v < -32768) v = -32768;
            s[j] = (int16_t) v;
        }

        // Allegro only uses UNSIGNED samples ...
        for(int32_t j=0; j<2*samples; ++j)
        {
            *q ^= 0x8000;
            q++;
        }
        
        free_audio_stream_buffer(gme->stream);
        return true;
    }
    
    return false;
}


Music_Emu* gme_load_file(const char* filename, const char* ext)
{
    Music_Emu* emu=NULL;
    gme_err_t err = gme_open_file(filename, &emu, DUH_SAMPLES);
    if (err)
    {
        gme_delete(emu);
        return NULL;
    }

    gme_set_autoload_playback_limit(emu, false);

    return emu;
}

int32_t gme_play(GMEFILE *gme, int32_t vol)
{
    gme_start_track(gme->emu, 0);
    int32_t samples=512;
    int32_t buf_size=2*DUH_SAMPLES/50;
    
    while(samples < buf_size) samples *= 2;
    
    gme->samples=samples;
    
    if(gme->playing != ZCM_STOPPED) stop_audio_stream(gme->stream);
    
    gme->stream = play_audio_stream(samples, 16, TRUE, DUH_SAMPLES, vol, 128);
    return true;
}

int32_t unload_gme_file(GMEFILE* gme)
{
    if(gme!=NULL)
    {
        if(gme->emu != NULL)
        {
            zcmusic_stop(gme);
            gme_delete(gme->emu);
            gme->emu=NULL;
            free(gme);
        }
    }
    
    return true;
}
