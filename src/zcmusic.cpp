/* Zelda Classic Music Library         */
/* ------------------------------------*/
/* Wrapper for transparently extending */
/* supported music file formats.       */

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

//#include "precompiled.h" //always first


#include "zc_alleg.h" // Has to be there or else OS X Universal 10.4 complains of fix overload - Taku
#include <string.h>
#include <aldumb.h>
#include <alogg.h>
#include <almp3.h>

#include "zsys.h"
#include "zcmusic.h"
#include "zc_malloc.h"
#include "mutex.h"

#undef	int8_t
#undef	uint8_t
#undef	int16_t
#undef	uint16_t
#undef	int32_t
#undef	uint32_t
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
#define int8_t       signed char
#define uint8_t      unsigned char
#define int16_t      signed short
#define uint16_t     unsigned short
#define int32_t      signed int
#define uint32_t     unsigned int

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

int zcmusic_bufsz = 64;
static int zcmusic_bufsz_private = 64;

mutex playlistmutex;

typedef struct DUHFILE : public ZCMUSICBASE
{
    DUH *s;
    AL_DUH_PLAYER *p;
} DUHFILE;

typedef struct OGGFILE : public ZCMUSICBASE
{
    ALOGG_OGGSTREAM *s;
    PACKFILE *f;
    char *fname;
    int vol;
} OGGFILE;

typedef struct OGGEXFILE : public ZCMUSICBASE
{
    ALOGG_OGG *s;
    FILE *f;
    char *fname;
    int vol;
} OGGEXFILE;

typedef struct MP3FILE : public ZCMUSICBASE
{
    ALMP3_MP3STREAM *s;
    PACKFILE *f;
    char *fname;
    int vol;
} MP3FILE;

typedef struct GMEFILE : public ZCMUSICBASE
{
    AUDIOSTREAM *stream;
    class Music_Emu* emu;
    int samples;
} GMEFILE;

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h
#include <vector>
static std::vector<ZCMUSIC*> playlist;                      //yeah, I'm too lazy to do it myself
static int libflags = 0;

// forward declarations
OGGFILE *load_ogg_file(char *filename);
int poll_ogg_file(OGGFILE *ogg);
void unload_ogg_file(OGGFILE *ogg);
bool ogg_pause(OGGFILE *ogg);
bool ogg_resume(OGGFILE *ogg);
bool ogg_reset(OGGFILE *ogg);
void ogg_stop(OGGFILE *ogg);

OGGEXFILE *load_ogg_ex_file(char *filename);
int poll_ogg_ex_file(OGGEXFILE *ogg);
void unload_ogg_ex_file(OGGEXFILE *ogg);
bool ogg_ex_pause(OGGEXFILE *ogg);
bool ogg_ex_resume(OGGEXFILE *ogg);
bool ogg_ex_reset(OGGEXFILE *ogg);
void ogg_ex_stop(OGGEXFILE *ogg);
int ogg_ex_getpos(OGGEXFILE *ogg);
void ogg_ex_setpos(OGGEXFILE *ogg, int msecs);
void ogg_ex_setspeed(OGGEXFILE *ogg, int speed);

MP3FILE *load_mp3_file(char *filename);
int poll_mp3_file(MP3FILE *mp3);
void unload_mp3_file(MP3FILE *mp3);
bool mp3_pause(MP3FILE *mp3);
bool mp3_resume(MP3FILE *mp3);
bool mp3_reset(MP3FILE *mp3);
void mp3_stop(MP3FILE *mp3);

Music_Emu* gme_load_file(char* filename, char* ext);
int poll_gme_file(GMEFILE *gme);
int unload_gme_file(GMEFILE* gme);
int gme_play(GMEFILE *gme, int vol);


extern "C"
{
	void zcm_extract_name(char *path,char *name,int type)
	{
		int l=(int)strlen(path);
		int i=l;

		while(i>0 && path[i-1]!='/' && path[i-1]!='\\')
			--i;

		int n=0;

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
    
    bool zcmusic_init(int flags)                              /* = -1 */
    {
        zcmusic_bufsz_private = zcmusic_bufsz;
        
        if(flags & ZCMF_DUH)
        {
            dumb_register_packfiles();
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
        
        mutex_init(&playlistmutex);
        
        install_int_ex(zcmusic_autopoll, MSEC_TO_TIMER(25));
        return true;
    }
    
    bool zcmusic_poll(int flags)                              /* = -1 */
    {
        //lock mutex
        mutex_lock(&playlistmutex);
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
                    poll_mp3_file((MP3FILE*)*b);
                    break;
                    
                case ZCMF_GME:
                    if(((GMEFILE*)*b)->emu)
                        poll_gme_file((GMEFILE*)*b);
                        
                    break;
		    
		case ZCMF_OGGEX:
                    poll_ogg_ex_file((OGGEXFILE*)*b);
                    break;
                }
                
            case ZCM_PAUSED:
                b++;
            }
        }
        
        mutex_unlock(&playlistmutex);
//	setPackfilePassword(oldpwd);
//	if(oldpwd != NULL)
//		delete[] oldpwd;
        return true;
    }
    
    void zcmusic_exit()
    {
        //lock mutex
        mutex_lock(&playlistmutex);
        std::vector<ZCMUSIC*>::iterator b = playlist.begin();
        
        while(b != playlist.end())
        {
            zcmusic_unload_file(*b);
            b = playlist.erase(b);
        }
        
        playlist.clear();
        mutex_unlock(&playlistmutex);
        
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
    
    ZCMUSIC const * zcmusic_load_file(char *filename)
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
            
            p->fname = (char*)zc_malloc(strlen(filename)+1);
            
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
            
            p->fname = (char*)zc_malloc(strlen(filename)+1);
            
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
                d = dumb_load_mod(filename);
                
                if(!d) al_trace("MOD file '%s' not found.\n",filename);
            }
            
            if(d)
            {
                DUHFILE *p = (DUHFILE*)zc_malloc(sizeof(DUHFILE));
                
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
                
                emu=gme_load_file(filename, ext);
                
                if(emu)
                {
                    GMEFILE *p=(GMEFILE*)zc_malloc(sizeof(GMEFILE));
                    
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
    
    ZCMUSIC const * zcmusic_load_file_ex(char *filename)
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
	
	if((stricmp(ext,"ogg")==0) && (libflags & ZCMF_OGGEX))
        {
            OGGEXFILE *p = load_ogg_ex_file(filename);
            
            if(!p)
            {
                al_trace("OGG file '%s' not loaded.\n",filename);
                goto error;
            }
            
            p->fname = (char*)zc_malloc(strlen(filename)+1);
            
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
    
    bool zcmusic_play(ZCMUSIC* zcm, int vol) /* = FALSE */
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
        
        int ret = TRUE;
        
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
                    /*pan*/
                    almp3_adjust_mp3stream(((MP3FILE*)zcm)->s, vol, 128, 1000/*speed*/);
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
                    if(almp3_play_mp3stream(((MP3FILE*)zcm)->s, (zcmusic_bufsz_private*1024), vol, 128) != ALMP3_OK)
                        ret = FALSE;
                        
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
                mutex_lock(&playlistmutex);
                playlist.push_back(zcm);
                mutex_unlock(&playlistmutex);
            }
        }
        
        return ret!=0;
    }
    
    bool zcmusic_pause(ZCMUSIC* zcm, int pause = -1)
    {
        // This function suspends play of the music indicated
        // by 'zcm'. Passing 0 for pause will resume; passing
        // -1 (or if the default argument is invoked) will
        // toggle the current state; passing 1 will pause.
        if(zcm == NULL) return FALSE;
        
        mutex_lock(&playlistmutex);
        
        if(zcm->playing != ZCM_STOPPED)
        {
            int p = ZCM_PLAYING;
            
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
                            
                        break;
                    }
                    
                case ZCMF_OGG:
                    if(p == ZCM_PAUSED)
                        ogg_pause((OGGFILE*)zcm);
                    else
                        ogg_resume((OGGFILE*)zcm);
                        
                    break;
                    
                case ZCMF_MP3:
                    if(p == ZCM_PAUSED)
                        mp3_pause((MP3FILE*)zcm);
                    else
                        mp3_resume((MP3FILE*)zcm);
                        
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
                        
                        break;
                    }
                
		case ZCMF_OGGEX:
                    if(p == ZCM_PAUSED)
                        ogg_ex_pause((OGGEXFILE*)zcm);
                    else
                        ogg_ex_resume((OGGEXFILE*)zcm);
                        
                    break;
                }
            }
        }
        
        mutex_unlock(&playlistmutex);
        return TRUE;
    }
    
    bool zcmusic_stop(ZCMUSIC* zcm)
    {
        // this function will stop playback of 'zcm' and reset
        // the stream position to the beginning.
        if(zcm == NULL) return FALSE;
        
        mutex_lock(&playlistmutex);
        
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
            mp3_stop((MP3FILE*)zcm);
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
        
        mutex_unlock(&playlistmutex);
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
            mutex_lock(&playlistmutex);
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
            
            mutex_unlock(&playlistmutex);
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
                zc_free(zcm);
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
    
    int zcmusic_get_tracks(ZCMUSIC* zcm)
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
                int t=((GMEFILE*)zcm)->emu->track_count();
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
    
    int zcmusic_change_track(ZCMUSIC* zcm, int tracknum)
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
                mutex_lock(&playlistmutex);
                int t=((GMEFILE*)zcm)->emu->track_count();
                
                if(tracknum<0 || tracknum>=t)
                {
                    tracknum=0;
                }
                
                ((GMEFILE*)zcm)->emu->start_track(tracknum);
                zcm->track=tracknum;
                mutex_unlock(&playlistmutex);
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
    int zcmusic_get_curpos(ZCMUSIC* zcm)
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
    void zcmusic_set_curpos(ZCMUSIC* zcm, int value)
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
    void zcmusic_set_speed(ZCMUSIC* zcm, int value)
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
}                                                           // extern "C"

MP3FILE *load_mp3_file(char *filename)
{
    MP3FILE *p = NULL;
    PACKFILE *f = NULL;
    ALMP3_MP3STREAM *s = NULL;
    char *data = new char[(zcmusic_bufsz_private*512)];
    int len;
    
    if((p = (MP3FILE *)zc_malloc(sizeof(MP3FILE)))==NULL)
        goto error;
        
    if((f = pack_fopen_password(filename, F_READ,""))==NULL)
        goto error;
    
    // ID3 tags sometimes cause problems with almp3, so try to skip them
    if((len = pack_fread(data, 10, f)) <= 0)
        goto error;
    
    if(strncmp(data, "ID3", 3)==0)
    {
        int id3Size=10;
        
        id3Size=((data[6]&0x7F)<<21)|((data[7]&0x7F)<<14)|((data[8]&0x7F)<<7)|(data[9]&0x7F);
        pack_fseek(f, id3Size-10);
        if((len = pack_fread(data, (zcmusic_bufsz_private*512), f)) <= 0)
            goto error;
    }
    else // no ID3
    {
        //if((len = pack_fread(data+10, (zcmusic_bufsz_private*512)-10, f)) <= 0)
        if((len = pack_fread(data, (zcmusic_bufsz_private*512), f)) <= 0)
            goto error;
    }
    
    if((len = pack_fread(data, (zcmusic_bufsz_private*512), f)) <= 0)
        goto error;
    
    if(len < (zcmusic_bufsz_private*512))
    {
        if((s = almp3_create_mp3stream(data, len, TRUE))==NULL)
            goto error;
    }
    else
    {
        if((s = almp3_create_mp3stream(data, (zcmusic_bufsz_private*512), FALSE))==NULL)
            goto error;
    }
    
    p->f = f;
    p->s = s;
    delete[] data;
    return p;
    
error:

    if(f)
        pack_fclose(f);
        
    if(p)
        zc_free(p);
        
    delete[] data;
    return NULL;
}

int poll_mp3_file(MP3FILE *mp3)
{
    if(mp3 == NULL) return ALMP3_POLL_NOTPLAYING;
    
    if(mp3->s == NULL) return ALMP3_POLL_NOTPLAYING;
    
    char *data = (char *)almp3_get_mp3stream_buffer(mp3->s);
    
    if(data)
    {
        long len = pack_fread(data, (zcmusic_bufsz_private*512), mp3->f);
        
        if(len < (zcmusic_bufsz_private*512))
            almp3_free_mp3stream_buffer(mp3->s, len);
        else
            almp3_free_mp3stream_buffer(mp3->s, -1);
    }
    
    int ret = almp3_poll_mp3stream(mp3->s);
    
    if(ret != ALMP3_OK)
    {
        mp3_reset(mp3);
        almp3_play_mp3stream(mp3->s, (zcmusic_bufsz_private*1024), mp3->vol, 128);
        mp3->playing = ZCM_PLAYING;
    }
    
    return ret;
}

void unload_mp3_file(MP3FILE *mp3)
{
    if(mp3 != NULL)
    {
        if(mp3->f != NULL)
        {
            pack_fclose(mp3->f);
            mp3->f = NULL;
        }
        
        if(mp3->s != NULL)
        {
            AUDIOSTREAM* a = almp3_get_audiostream_mp3stream(mp3->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            almp3_destroy_mp3stream(mp3->s);
            mp3->s = NULL;
        }
        
        if(mp3->fname != NULL)
        {
            zc_free(mp3->fname);
            zc_free(mp3);
        }
    }
}

bool mp3_pause(MP3FILE *mp3)
{
    AUDIOSTREAM* a = NULL;
    
    if(mp3->s != NULL)
        a = almp3_get_audiostream_mp3stream(mp3->s);
        
    if(a != NULL)
    {
        voice_stop(a->voice);
        return true;
    }
    
    return false;
}

bool mp3_resume(MP3FILE *mp3)
{
    AUDIOSTREAM* a = NULL;
    
    if(mp3->s != NULL)
        a = almp3_get_audiostream_mp3stream(mp3->s);
        
    if(a != NULL)
    {
        voice_start(a->voice);
        return true;
    }
    
    return false;
}

bool mp3_reset(MP3FILE *mp3)
{
    if(mp3->fname != NULL)
    {
        if(mp3->f != NULL)
        {
            pack_fclose(mp3->f);
            mp3->f = NULL;
        }
        
        if(mp3->s != NULL)
        {
            AUDIOSTREAM* a = almp3_get_audiostream_mp3stream(mp3->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            almp3_destroy_mp3stream(mp3->s);
            mp3->s = NULL;
        }
        
        MP3FILE* tmp3 = load_mp3_file(mp3->fname);
        
        if(tmp3 != NULL)
        {
            mp3->playing = ZCM_STOPPED;
            mp3->s = tmp3->s;
            mp3->f = tmp3->f;
            zc_free(tmp3);
            return true;
        }
    }
    
    return false;
}

void mp3_stop(MP3FILE *mp3)
{
    if(mp3->fname != NULL)
    {
        if(mp3->f != NULL)
        {
            pack_fclose(mp3->f);
            mp3->f = NULL;
        }
        
        if(mp3->s != NULL)
        {
            AUDIOSTREAM* a = almp3_get_audiostream_mp3stream(mp3->s);
            
            if(a != NULL)
                voice_stop(a->voice);
                
            almp3_destroy_mp3stream(mp3->s);
            mp3->s = NULL;
        }
    }
}

OGGFILE *load_ogg_file(char *filename)
{
    OGGFILE *p = NULL;
    PACKFILE *f = NULL;
    ALOGG_OGGSTREAM *s = NULL;
    char *data = new char[(zcmusic_bufsz_private*512)];
    int len;
    
    if((p = (OGGFILE *)zc_malloc(sizeof(OGGFILE)))==NULL)
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
        zc_free(p);
        
    delete[] data;
    return NULL;
}

int poll_ogg_file(OGGFILE *ogg)
{
    if(ogg == NULL) return ALOGG_POLL_NOTPLAYING;
    
    if(ogg->s == NULL) return ALOGG_POLL_NOTPLAYING;
    
    char *data = (char *)alogg_get_oggstream_buffer(ogg->s);
    
    if(data)
    {
        long len = pack_fread(data, (zcmusic_bufsz_private*512), ogg->f);
        
        if(len < (zcmusic_bufsz_private*512))
            alogg_free_oggstream_buffer(ogg->s, len);
        else
            alogg_free_oggstream_buffer(ogg->s, -1);
    }
    
    int ret = alogg_poll_oggstream(ogg->s);
    
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
            zc_free(ogg->fname);
            zc_free(ogg);
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
            zc_free(togg);
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

OGGEXFILE *load_ogg_ex_file(char *filename) //!dimi: Start of og_ex. og_ex allows for seeking and getting total length of audio file.
{
    //OGGEXFILE *p = NULL;
    OGGEXFILE *p = (OGGEXFILE*)zc_malloc(sizeof(OGGEXFILE));
    FILE *f = fopen(filename, "rb");
    ALOGG_OGG *s = alogg_create_ogg_from_file(f);
    /*char *data = new char[(zcmusic_bufsz_private*512)];
    int len;*/
    
    /*if((p = (OGGEXFILE*)zc_malloc(sizeof(OGGEXFILE)))==NULL)
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
        zc_free(p);
        
    //delete[] data;
    return NULL;
}

int poll_ogg_ex_file(OGGEXFILE *ogg)
{
    if(ogg == NULL) return ALOGG_POLL_NOTPLAYING;
    
    if(ogg->s == NULL) return ALOGG_POLL_NOTPLAYING;
    
    int ret = alogg_poll_ogg(ogg->s);
    
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
            zc_free(ogg->fname);
            zc_free(ogg);
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
            zc_free(togg);
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

int ogg_ex_getpos(OGGEXFILE *ogg) //!dimi: both getpos and setpos are in milliseconds. This is so that you can (hopefully) use decimals in zscript to access sub-second values.
{
    if(ogg->s != NULL)
    {
	int baddebugtimes = alogg_get_pos_msecs_ogg(ogg->s);
	return baddebugtimes;
	
	return 0;
    }
    return 0; //if it is NULL, we still need to return a value. -Z
}

void ogg_ex_setpos(OGGEXFILE *ogg, int msecs)
{
    if(ogg->s != NULL)
    {
	alogg_seek_abs_msecs_ogg(ogg->s, msecs);
    }
}

void ogg_ex_setspeed(OGGEXFILE *ogg, int speed)
{
    if(ogg->s != NULL)
    {
	//alogg_adjust_ogg(ogg->s, ogg->vol, 128, speed, 1);
	alogg_stop_ogg(ogg->s);
	alogg_play_ex_ogg(ogg->s, (zcmusic_bufsz_private*1024), ogg->vol, 128, speed, 1);
    }
}

//!dimi: End of ogg_ex.

int poll_gme_file(GMEFILE* gme)
{
    unsigned char *p;
    p = (unsigned char*) get_audio_stream_buffer(gme->stream);
    
    if(p)
    {
        int samples=gme->samples;
        memset(p,0,4*samples);
        unsigned short *q=(unsigned short*) p;
        gme->emu->play((long) 2*samples,(short*)p);
        
        // Allegro only uses UNSIGNED samples ...
        for(int j=0; j<2*samples; ++j)
        {
            *q ^= 0x8000;
            q++;
        }
        
        free_audio_stream_buffer(gme->stream);
        return true;
    }
    
    return false;
}


Music_Emu* gme_load_file(char* filename, char* ext)
{
    Music_Emu* emu=NULL;
    
    if(stricmp(ext,"spc")==0) emu = new Spc_Emu;
    
    if(stricmp(ext,"gbs")==0) emu = new Gbs_Emu;
    
    if(stricmp(ext,"vgm")==0) emu = new Vgm_Emu;
    
    if(stricmp(ext,"nsf")==0) emu = new Nsf_Emu;
    
    if(stricmp(ext,"gym")==0) emu = new Gym_Emu;
    
    if(!emu) return NULL;
    
    Std_File_Reader in;
    
    const char* err = emu->set_sample_rate(DUH_SAMPLES);
    
    if(!err) err = in.open(filename);
    
    if(!err) err = emu->load(in);
    
    if(err)
    {
        delete emu;
        return NULL;
    }
    
    return emu;
}

int gme_play(GMEFILE *gme, int vol)
{
    gme->emu->start_track(0);
    int samples=512;
    int buf_size=2*DUH_SAMPLES/50;
    
    while(samples < buf_size) samples *= 2;
    
    gme->samples=samples;
    
    if(gme->playing != ZCM_STOPPED) stop_audio_stream(gme->stream);
    
    gme->stream = play_audio_stream(samples, 16, TRUE, DUH_SAMPLES, vol, 128);
    return true;
}

int unload_gme_file(GMEFILE* gme)
{
    if(gme!=NULL)
    {
        if(gme->emu != NULL)
        {
            zcmusic_stop(gme);
            delete gme->emu;
            gme->emu=NULL;
            zc_free(gme);
        }
    }
    
    return true;
}

