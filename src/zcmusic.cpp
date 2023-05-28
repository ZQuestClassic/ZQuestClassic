/* Zelda Classic Music Library         */
/* ------------------------------------*/
/* Wrapper for transparently extending */
/* supported music file formats.       */

//#include "base/zc_alleg.h" // Has to be there or else OS X Universal 10.4 complains of fix overload - Taku
#include <string.h>

#ifdef _DEBUG
#ifdef _malloca
#undef _malloca
#endif
#endif

#include <aldumb.h>
#include <alogg.h>
#ifdef SOUND_LIBS_BUILT_FROM_SOURCE
#include <gme.h>
#endif

#include "base/zsys.h"
#include "base/util.h"
#include "zcmusic.h"
#include <filesystem>
#include <stdlib.h>
#include <allegro5/allegro_audio.h>

using namespace util;

#ifndef SOUND_LIBS_BUILT_FROM_SOURCE
//short of fixing gme, these warnings will always be there...
#pragma warning(disable:4512) //assignment operator could not be generated
#pragma warning(disable:4100) //unreferenced formal parameter
#include "Nsf_Emu.h"
#include "Gbs_Emu.h"
#include "Spc_Emu.h"
#include "Vgm_Emu.h"
#include "Gym_Emu.h"
#pragma warning(default:4100)
#pragma warning(default:4512)
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

// MSVC fix
#if _MSC_VER >= 1900
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }
#endif

// might consider replacing the following with user defined values from the
// 'sound' dialog in the player. This way, each person could tune it as needed.
#define DUH_CHANNELS  2                                     // stereo
#define DUH_SAMPLES   44100                                 //Hz
#define DUH_RESAMPLE  1

int32_t zcmusic_bufsz = 64;
static int32_t zcmusic_bufsz_private = 64;

ALLEGRO_MUTEX* playlistmutex = NULL;

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

typedef struct DUHFILE : public ZCMUSIC
{
    DUH *s;
    AL_DUH_PLAYER *p;
} DUHFILE;

typedef struct OGGFILE : public ZCMUSIC
{
    ALOGG_OGGSTREAM *s;
    PACKFILE *f;
    char *fname;
    int32_t vol;
} OGGFILE;

typedef struct OGGEXFILE : public ZCMUSIC
{
    ALOGG_OGG *s;
    FILE *f;
    char *fname;
    int32_t vol;
} OGGEXFILE;

typedef struct MP3FILE : public ZCMUSIC
{
    ALLEGRO_AUDIO_STREAM *s;
    char *fname;
    int32_t vol;
} MP3FILE;

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
OGGFILE *load_ogg_file(const char *filename);
int32_t poll_ogg_file(OGGFILE *ogg);
void unload_ogg_file(OGGFILE *ogg);
bool ogg_pause(OGGFILE *ogg);
bool ogg_resume(OGGFILE *ogg);
bool ogg_reset(OGGFILE *ogg);
void ogg_stop(OGGFILE *ogg);

OGGEXFILE *load_ogg_ex_file(const char *filename);
int32_t poll_ogg_ex_file(OGGEXFILE *ogg);
void unload_ogg_ex_file(OGGEXFILE *ogg);
bool ogg_ex_pause(OGGEXFILE *ogg);
bool ogg_ex_resume(OGGEXFILE *ogg);
bool ogg_ex_reset(OGGEXFILE *ogg);
void ogg_ex_stop(OGGEXFILE *ogg);
int32_t ogg_ex_getpos(OGGEXFILE *ogg);
void ogg_ex_setpos(OGGEXFILE *ogg, int32_t msecs);
void ogg_ex_setspeed(OGGEXFILE *ogg, int32_t speed);

MP3FILE *load_mp3_file(const char *filename);
void unload_mp3_file(MP3FILE *mp3);

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

void zcmusic_autopoll()
{
	zcmusic_poll();
}

bool zcmusic_init(int32_t flags)                              /* = -1 */
{
	zcmusic_bufsz_private = zcmusic_bufsz;
	
	if(flags & ZCMF_DUH)
	{
		dumb_register_stdfiles();
		dumb_resampling_quality = DUH_RESAMPLE;
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

if(flags & ZCMF_OGGEX)
	{
		libflags |= ZCMF_OGGEX;
	}
	
	if (!playlistmutex) playlistmutex = al_create_mutex();
	
	install_int_ex(zcmusic_autopoll, MSEC_TO_TIMER(25));
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
			case ZCMF_DUH:
				if(((DUHFILE*)*b)->p)
					al_poll_duh(((DUHFILE*)*b)->p);
					
				break;
				
			case ZCMF_OGG:
				poll_ogg_file((OGGFILE*)*b);
				break;
				
			case ZCMF_MP3:
				break;
				
			case ZCMF_GME:
				if(((GMEFILE*)*b)->emu)
					poll_gme_file((GMEFILE*)*b);
					
				break;
		
	case ZCMF_OGGEX:
				poll_ogg_ex_file((OGGEXFILE*)*b);
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
		dumb_exit();
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

if(libflags & ZCMF_OGGEX)
	{
		libflags ^= ZCMF_OGGEX;
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
	
	char *ext=get_extension(filename);
	
	if((stricmp(ext,"ogg")==0) && (libflags & ZCMF_OGG))
	{
		OGGFILE *p = load_ogg_file(filename);
		
		if(!p)
		{
			al_trace("OGG file '%s' not loaded.\n",filename);
			goto error;
		}
		
		p->fname = (char*)malloc(strlen(filename)+1);
		
		if(!p->fname)
		{
			unload_ogg_file(p);
			goto error;
		}
		
		strcpy(p->fname, filename);
		p->type = ZCMF_OGG;
		p->playing = ZCM_STOPPED;
		ZCMUSIC *music=(ZCMUSIC*)p;
		zcm_extract_name(filename, music->filename, FILENAMEALL);
		music->filename[255]='\0';
		music->track=0;
		return music;
	}
	
	if((stricmp(ext,"mp3")==0) && (libflags & ZCMF_MP3))
	{
		MP3FILE *p = load_mp3_file(filename);
		
		if(!p)
		{
			al_trace("MP3 file '%s' not found.\n",filename);
			goto error;
		}
		
		p->fname = (char*)malloc(strlen(filename)+1);
		
		if(!p->fname)
		{
			unload_mp3_file(p);
			goto error;
		}
		
		strcpy(p->fname, filename);
		p->type = ZCMF_MP3;
		p->playing = ZCM_STOPPED;
		ZCMUSIC *music=(ZCMUSIC*)p;
		zcm_extract_name(filename, music->filename, FILENAMEALL);
		music->filename[255]='\0';
		music->track=0;
		return music;
	}
	
	if(libflags & ZCMF_DUH)
	{
		DUH* d = NULL;
		
		if(stricmp(ext,"it")==0)
		{
			d = dumb_load_it(filename);
			
			if(!d) al_trace("IT file '%s' not found.\n",filename);
		}
		else if(stricmp(ext,"xm")==0)
		{
			d = dumb_load_xm(filename);
			
			if(!d) al_trace("XM file '%s' not found.\n",filename);
		}
		else if(stricmp(ext,"s3m")==0)
		{
			d = dumb_load_s3m(filename);
			
			if(!d) al_trace("S3M file '%s' not found.\n",filename);
		}
		else if(stricmp(ext,"mod")==0)
		{
#ifdef SOUND_LIBS_BUILT_FROM_SOURCE
			// No idea what this second arg does...
			d = dumb_load_mod(filename, 2);
#else
			d = dumb_load_mod(filename);
#endif
			
			if(!d) al_trace("MOD file '%s' not found.\n",filename);
		}
		
		if(d)
		{
			DUHFILE *p = (DUHFILE*)malloc(sizeof(DUHFILE));
			
			if(!p)
			{
				unload_duh(d);
				goto error;
			}
			
			p->type = ZCMF_DUH;
			p->playing = ZCM_STOPPED;
			p->s = d;
			p->p = NULL;
			ZCMUSIC *music=(ZCMUSIC*)p;
			zcm_extract_name(filename, music->filename, FILENAMEALL);
			music->filename[255]='\0';
			music->track=0;
			return music;
		}
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

ZCMUSIC * zcmusic_load_file_ex(const char *filename)
{
	if(filename == NULL)
	{
		return NULL;
	}
	
	al_trace("Trying to load %s\n", filename);
	
	if(strlen(filename)>255)
	{
		al_trace("Music file '%s' not loaded: filename too int32_t\n", filename);
		return NULL;
	}
	
	char *ext=get_extension(filename);

if((stricmp(ext,"ogg")==0) && (libflags & ZCMF_OGGEX))
	{
		OGGEXFILE *p = load_ogg_ex_file(filename);
		
		if(!p)
		{
			al_trace("OGG file '%s' not loaded.\n",filename);
			goto error;
		}
		
		p->fname = (char*)malloc(strlen(filename)+1);
		
		if(!p->fname)
		{
			unload_ogg_ex_file(p);
			goto error;
		}
		
		strcpy(p->fname, filename);
		p->type = ZCMF_OGGEX;
		p->playing = ZCM_STOPPED;
		ZCMUSIC *music=(ZCMUSIC*)p;
		zcm_extract_name(filename, music->filename, FILENAMEALL);
		music->filename[255]='\0';
		music->track=0;
		return music;
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
		switch(zcm->type & libflags)
		{
		case ZCMF_DUH:
			if(((DUHFILE*)zcm)->p != NULL)
				al_duh_set_volume(((DUHFILE*)zcm)->p, (float)vol / (float)255);
				
			break;
			
		case ZCMF_OGG:
			if(((OGGFILE*)zcm)->s != NULL)
			{
				/*pan*/
				alogg_adjust_oggstream(((OGGFILE*)zcm)->s, vol, 128, 1000/*speed*/);
				((OGGFILE*)zcm)->vol = vol;
			}
			
			break;
			
		case ZCMF_MP3:
			if(((MP3FILE*)zcm)->s != NULL)
			{
				al_set_audio_stream_gain(((MP3FILE*)zcm)->s, vol / 255.0);
				((MP3FILE*)zcm)->vol = vol;
			}
			
			break;
			
		case ZCMF_GME:
			// need to figure out volume switch
			break;
	
	case ZCMF_OGGEX:
			if(((OGGEXFILE*)zcm)->s != NULL)
			{
				/*pan*/
				alogg_adjust_ogg(((OGGEXFILE*)zcm)->s, vol, 128, 1000/*speed*/, true);
				((OGGEXFILE*)zcm)->vol = vol;
			}
			
			break;
			
		}
	}
	else
	{
		switch(zcm->type & libflags)
		{
		case ZCMF_DUH:
			if(((DUHFILE*)zcm)->s != NULL)
			{
				((DUHFILE*)zcm)->p = al_start_duh(((DUHFILE*)zcm)->s, DUH_CHANNELS, 0/*pos*/, ((float)vol) / (float)255, (zcmusic_bufsz_private*1024)/*bufsize*/, DUH_SAMPLES);
				ret = (((DUHFILE*)zcm)->p != NULL) ? TRUE : FALSE;
			}
			
			break;
			
		case ZCMF_OGG:
			if(((OGGFILE*)zcm)->s != NULL)
			{
				if(alogg_play_oggstream(((OGGFILE*)zcm)->s, (zcmusic_bufsz_private*1024), vol, 128) != ALOGG_OK)
					ret = FALSE;
					
				((OGGFILE*)zcm)->vol = vol;
		/*
		//Should be possible to establish loops for these file types. -Z
		((MP3FILE*)zcm)->loop_start = 0;
		((MP3FILE*)zcm)->loop_end = samp->len;
		*/
			}
			else
			{
				ret = FALSE;
			}
			
			break;
			
		case ZCMF_MP3:
			if(((MP3FILE*)zcm)->s != NULL)
			{
				ret = al_set_audio_stream_playing(((MP3FILE*)zcm)->s, true);
				al_set_audio_stream_gain(((MP3FILE*)zcm)->s, vol / 255.0);
				((MP3FILE*)zcm)->vol = vol;
		/*
		//Should be possible to establish loops for these file types. -Z
		((MP3FILE*)zcm)->loop_start = 0;
		((MP3FILE*)zcm)->loop_end = samp->len;
		*/
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
	
	case ZCMF_OGGEX:
			if(((OGGEXFILE*)zcm)->s != NULL)
			{
				if(alogg_play_ogg(((OGGEXFILE*)zcm)->s, (zcmusic_bufsz_private*1024), vol, 128) != ALOGG_OK)
					ret = FALSE;
					
				((OGGEXFILE*)zcm)->vol = vol;
		/*
		//Should be possible to establish loops for these file types. -Z
		((MP3FILE*)zcm)->loop_start = 0;
		((MP3FILE*)zcm)->loop_end = samp->len;
		*/
			}
			else
			{
				ret = FALSE;
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
			case ZCMF_DUH:
				if(((DUHFILE*)zcm)->p != NULL)
				{
					if(p == ZCM_PAUSED)
						al_pause_duh(((DUHFILE*)zcm)->p);
					else
						al_resume_duh(((DUHFILE*)zcm)->p);
						
				}
				break;
				
			case ZCMF_OGG:
				if(p == ZCM_PAUSED)
					ogg_pause((OGGFILE*)zcm);
				else
					ogg_resume((OGGFILE*)zcm);
					
				break;
				
			case ZCMF_MP3:
				al_set_audio_stream_playing(((MP3FILE*)zcm)->s, p != ZCM_PAUSED);
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
			
	case ZCMF_OGGEX:
				if(p == ZCM_PAUSED)
					ogg_ex_pause((OGGEXFILE*)zcm);
				else
					ogg_ex_resume((OGGEXFILE*)zcm);
					
				break;
			}
		}
	}
	
	al_unlock_mutex(playlistmutex);
	return TRUE;
}

bool zcmusic_stop(ZCMUSIC* zcm)
{
	// this function will stop playback of 'zcm' and reset
	// the stream position to the beginning.
	if(zcm == NULL) return FALSE;
	
	al_lock_mutex(playlistmutex);
	
	switch(zcm->type & libflags)
	{
	case ZCMF_DUH:
		if(((DUHFILE*)zcm)->p != NULL)
		{
			al_stop_duh(((DUHFILE*)zcm)->p);
			((DUHFILE*)zcm)->p = NULL;
			zcm->playing = ZCM_STOPPED;
		}
		
		break;
		
	case ZCMF_OGG:
		ogg_stop((OGGFILE*)zcm);
		break;
		
	case ZCMF_MP3:
		al_set_audio_stream_playing(((MP3FILE*)zcm)->s, false);
		al_seek_audio_stream_secs(((MP3FILE*)zcm)->s, 0);
		break;
		
	case ZCMF_GME:
		if(((GMEFILE*)zcm)->emu != NULL)
		{
			if(zcm->playing != ZCM_STOPPED) stop_audio_stream(((GMEFILE*)zcm)->stream);
			
			zcm->playing = ZCM_STOPPED;
		}
		
		break;
	
case ZCMF_OGGEX:
		ogg_ex_stop((OGGEXFILE*)zcm);
		break;
	}
	
	al_unlock_mutex(playlistmutex);
	return TRUE;
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
	case ZCMF_DUH:
		if(((DUHFILE*)zcm)->p != NULL)
		{
			zcmusic_stop(zcm);
			((DUHFILE*)zcm)->p = NULL;
		}
		
		if(((DUHFILE*)zcm)->s != NULL)
		{
			unload_duh(((DUHFILE*)zcm)->s);
			((DUHFILE*)zcm)->s = NULL;
			free(zcm);
		}
		
		break;
		
	case ZCMF_OGG:
		unload_ogg_file((OGGFILE*)zcm);
		break;
		
	case ZCMF_MP3:
		unload_mp3_file((MP3FILE*)zcm);
		break;
		
	case ZCMF_GME:
		unload_gme_file((GMEFILE*)zcm);
		break;
	
case ZCMF_OGGEX:
		unload_ogg_ex_file((OGGEXFILE*)zcm);
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
case ZCMF_OGGEX:
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
case ZCMF_OGGEX:
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
if(zcm == NULL) return 0;
	
	switch(zcm->type & libflags)
	{
	case ZCMF_OGGEX:
	return ogg_ex_getpos((OGGEXFILE*)zcm);
	break;
}

return 0;
}
void zcmusic_set_curpos(ZCMUSIC* zcm, int32_t value)
{
if(zcm == NULL) return;
	
	switch(zcm->type & libflags)
	{
	case ZCMF_OGGEX:
	ogg_ex_setpos((OGGEXFILE*)zcm, value);
	break;
}

return;
}
void zcmusic_set_speed(ZCMUSIC* zcm, int32_t value)
{
if(zcm == NULL) return;
	
	switch(zcm->type & libflags)
	{
	case ZCMF_OGGEX:
	ogg_ex_setspeed((OGGEXFILE*)zcm, value);
	break;
}

return;
}

MP3FILE *load_mp3_file(const char *filename)
{
    MP3FILE *p = NULL;

	if((p = (MP3FILE *)malloc(sizeof(MP3FILE)))==NULL)
        return NULL;

	al_reserve_samples(5);
	ALLEGRO_AUDIO_STREAM* stream = al_play_audio_stream(filename);
	if (!stream)
		goto error;
	al_set_audio_stream_playing(stream, false);
	al_set_audio_stream_playmode(stream, ALLEGRO_PLAYMODE_LOOP);

	p->s = stream;

	return p;

error:

    if(p)
        free(p);
        
    return NULL;
}

void unload_mp3_file(MP3FILE *mp3)
{
    if(mp3 != NULL)
    {
        if(mp3->s != NULL)
        {
			al_set_audio_stream_playing(mp3->s, false);
            mp3->s = NULL;
        }
        
        if(mp3->fname != NULL)
        {
            free(mp3->fname);
            free(mp3);
        }
    }
}

OGGFILE *load_ogg_file(const char *filename)
{
    OGGFILE *p = NULL;
    PACKFILE *f = NULL;
    ALOGG_OGGSTREAM *s = NULL;
    char *data = new char[(zcmusic_bufsz_private*512)];
    int32_t len;
    
    if((p = (OGGFILE *)malloc(sizeof(OGGFILE)))==NULL)
    {
        goto error;
    }
    
    if((f = pack_fopen_password(filename, F_READ,""))==NULL)
    {
        goto error;
    }
    
    if((len = pack_fread(data, (zcmusic_bufsz_private*512), f)) <= 0)
    {
        goto error;
    }
    
    if(len < (zcmusic_bufsz_private*512))
    {
        if((s = alogg_create_oggstream(data, len, TRUE))==NULL)
        {
            goto error;
        }
    }
    else
    {
        if((s = alogg_create_oggstream(data, (zcmusic_bufsz_private*512), FALSE))==NULL)
        {
            goto error;
        }
    }
    
    p->f = f;
    p->s = s;
    delete[] data;
    return p;
    
error:

    if(f)
        pack_fclose(f);
        
    if(p)
        free(p);
        
    delete[] data;
    return NULL;
}

int32_t poll_ogg_file(OGGFILE *ogg)
{
    if(ogg == NULL) return ALOGG_POLL_NOTPLAYING;
    
    if(ogg->s == NULL) return ALOGG_POLL_NOTPLAYING;
    
    char *data = (char *)alogg_get_oggstream_buffer(ogg->s);
    
    if(data)
    {
        int32_t len = pack_fread(data, (zcmusic_bufsz_private*512), ogg->f);
        
        if(len < (zcmusic_bufsz_private*512))
            alogg_free_oggstream_buffer(ogg->s, len);
        else
            alogg_free_oggstream_buffer(ogg->s, -1);
    }
    
    int32_t ret = alogg_poll_oggstream(ogg->s);
    
    if(ret != ALOGG_OK)
    {
        ogg_reset(ogg);
        alogg_play_oggstream(ogg->s, (zcmusic_bufsz_private*1024), ogg->vol, 128);
        ogg->playing = ZCM_PLAYING;
    }
    
    return ret;
}

void unload_ogg_file(OGGFILE *ogg)
{
    if(ogg != NULL)
    {
        if(ogg->f != NULL)
        {
            pack_fclose(ogg->f);
            ogg->f = NULL;
        }
        
        if(ogg->s != NULL)
        {
            AUDIOSTREAM* a = alogg_get_audiostream_oggstream(ogg->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            alogg_destroy_oggstream(ogg->s);
            ogg->s = NULL;
        }
        
        if(ogg->fname != NULL)
        {
            free(ogg->fname);
            free(ogg);
        }
    }
}

bool ogg_pause(OGGFILE *ogg)
{
    AUDIOSTREAM* a = NULL;
    
    if(ogg->s != NULL)
        a = alogg_get_audiostream_oggstream(ogg->s);
        
    if(a != NULL)
    {
        voice_stop(a->voice);
        return true;
    }
    
    return false;
}

bool ogg_resume(OGGFILE *ogg)
{
    AUDIOSTREAM* a = NULL;
    
    if(ogg->s != NULL)
        a = alogg_get_audiostream_oggstream(ogg->s);
        
    if(a != NULL)
    {
        voice_start(a->voice);
        return true;
    }
    
    return false;
}

bool ogg_reset(OGGFILE *ogg)
{
    if(ogg->fname != NULL)
    {
        if(ogg->f != NULL)
        {
            pack_fclose(ogg->f);
            ogg->f = NULL;
        }
        
        if(ogg->s != NULL)
        {
            AUDIOSTREAM* a = alogg_get_audiostream_oggstream(ogg->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            alogg_destroy_oggstream(ogg->s);
            ogg->s = NULL;
        }
        
        OGGFILE* togg = load_ogg_file(ogg->fname);
        
        if(togg != NULL)
        {
            ogg->playing = ZCM_STOPPED;
            ogg->s = togg->s;
            ogg->f = togg->f;
            free(togg);
            return true;
        }
    }
    
    return false;
}

void ogg_stop(OGGFILE *ogg)
{
    if(ogg->fname != NULL)
    {
        if(ogg->f != NULL)
        {
            pack_fclose(ogg->f);
            ogg->f = NULL;
        }
        
        if(ogg->s != NULL)
        {
            AUDIOSTREAM* a = alogg_get_audiostream_oggstream(ogg->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            alogg_destroy_oggstream(ogg->s);
            ogg->s = NULL;
        }
    }
}

OGGEXFILE *load_ogg_ex_file(const char *filename) //!dimi: Start of og_ex. og_ex allows for seeking and getting total length of audio file.
{
    //OGGEXFILE *p = NULL;
    OGGEXFILE *p = (OGGEXFILE*)malloc(sizeof(OGGEXFILE));
    FILE *f = fopen(filename, "rb");
    ALOGG_OGG *s = alogg_create_ogg_from_file(f);
    /*char *data = new char[(zcmusic_bufsz_private*512)];
    int32_t len;*/
    
    /*if((p = (OGGEXFILE*)malloc(sizeof(OGGEXFILE)))==NULL)
    {
        goto error;
    }*/
    
    if(!p)
    {
        goto error;
    }
    
    al_trace("oggex filename is: %s\n",filename);
    if(!f)
    {
        al_trace("oggex error at %s\n", "reading file");
        goto error;
    }
    
    /*if((len = pack_fread(data, (zcmusic_bufsz_private*512), f)) <= 0)
    {
        goto error;
    }*/
    
    /*if(len < (zcmusic_bufsz_private*512))
    {*/
        if(!s)
        {
	    al_trace("oggex error at %s\n", "checking alogg");
            goto error;
        }
    /*}
    else
    {
        if((s = alogg_create_ogg_from_buffer(data, (zcmusic_bufsz_private*512)))==NULL)
        {
            goto error;
        }
    }*/
    
    p->f = f;
    p->s = s;
    //delete[] data;
    return p;
    
error:

    if(f)
        fclose(f);
        
    if(p)
        free(p);
        
    //delete[] data;
    return NULL;
}

int32_t poll_ogg_ex_file(OGGEXFILE *ogg)
{
    if(ogg == NULL) return ALOGG_POLL_NOTPLAYING;
    
    if(ogg->s == NULL) return ALOGG_POLL_NOTPLAYING;
    
    int32_t ret = alogg_poll_ogg(ogg->s);
    
    if(ret != ALOGG_OK && ret != ALOGG_POLL_PLAYJUSTFINISHED && ret != ALOGG_POLL_NOTPLAYING)
    {
        if (ogg_ex_reset(ogg))
	{
		alogg_play_ogg(ogg->s, (zcmusic_bufsz_private*1024), ogg->vol, 128);
		ogg->playing = ZCM_PLAYING;
	}
    }
    else if (ret == ALOGG_POLL_PLAYJUSTFINISHED || ret == ALOGG_POLL_NOTPLAYING)
    {
	alogg_rewind_ogg(ogg->s);
	alogg_play_ogg(ogg->s, (zcmusic_bufsz_private*1024), ogg->vol, 128);
    }
    
    return ret;
}

void unload_ogg_ex_file(OGGEXFILE *ogg)
{
    if(ogg != NULL)
    {
        if(ogg->f != NULL)
        {
            fclose(ogg->f);
            ogg->f = NULL;
        }
        
        if(ogg->s != NULL)
        {
            AUDIOSTREAM* a = alogg_get_audiostream_ogg(ogg->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            alogg_destroy_ogg(ogg->s);
            ogg->s = NULL;
        }
        
        if(ogg->fname != NULL)
        {
            free(ogg->fname);
            free(ogg);
        }
    }
}

bool ogg_ex_pause(OGGEXFILE *ogg)
{
    AUDIOSTREAM* a = NULL;
    
    if(ogg->s != NULL)
        a = alogg_get_audiostream_ogg(ogg->s);
        
    if(a != NULL)
    {
        voice_stop(a->voice);
        return true;
    }
    
    return false;
}

bool ogg_ex_resume(OGGEXFILE *ogg)
{
    AUDIOSTREAM* a = NULL;
    
    if(ogg->s != NULL)
        a = alogg_get_audiostream_ogg(ogg->s);
        
    if(a != NULL)
    {
        voice_start(a->voice);
        return true;
    }
    
    return false;
}

bool ogg_ex_reset(OGGEXFILE *ogg)
{
    if(ogg->fname != NULL)
    {
        if(ogg->f != NULL)
        {
            fclose(ogg->f);
            ogg->f = NULL;
        }
        
        if(ogg->s != NULL)
        {
            AUDIOSTREAM* a = alogg_get_audiostream_ogg(ogg->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            alogg_destroy_ogg(ogg->s);
            ogg->s = NULL;
        }
        
        OGGEXFILE* togg = load_ogg_ex_file(ogg->fname);
        
        if(togg != NULL)
        {
            ogg->playing = ZCM_STOPPED;
            ogg->s = togg->s;
            ogg->f = togg->f;
            free(togg);
            return true;
        }
    }
    
    return false;
}

void ogg_ex_stop(OGGEXFILE *ogg)
{
    if(ogg->fname != NULL)
    {
        if(ogg->f != NULL)
        {
            fclose(ogg->f);
            ogg->f = NULL;
        }
        
        if(ogg->s != NULL)
        {
            AUDIOSTREAM* a = alogg_get_audiostream_ogg(ogg->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            alogg_destroy_ogg(ogg->s);
            ogg->s = NULL;
        }
    }
}

int32_t ogg_ex_getpos(OGGEXFILE *ogg) //!dimi: both getpos and setpos are in milliseconds. This is so that you can (hopefully) use decimals in zscript to access sub-second values.
{
    if(ogg->s != NULL)
    {
	int32_t baddebugtimes = alogg_get_pos_msecs_ogg(ogg->s);
	return baddebugtimes;
	
	return 0;
    }
    return 0; //if it is NULL, we still need to return a value. -Z
}

void ogg_ex_setpos(OGGEXFILE *ogg, int32_t msecs)
{
    if(ogg->s != NULL)
    {
	alogg_seek_abs_msecs_ogg(ogg->s, msecs);
    }
}

void ogg_ex_setspeed(OGGEXFILE *ogg, int32_t speed)
{
    if(ogg->s != NULL)
    {
	//alogg_adjust_ogg(ogg->s, ogg->vol, 128, speed, 1);
	alogg_stop_ogg(ogg->s);
	alogg_play_ex_ogg(ogg->s, (zcmusic_bufsz_private*1024), ogg->vol, 128, speed, 1);
    }
}

//!dimi: End of ogg_ex.

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

