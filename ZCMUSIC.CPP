/* Zelda Classic Music Library         */
/* ------------------------------------*/
/* Wrapper for transparently extending */
/* supported music file formats.       */

/* TODO: Make MP3s and OGGs loop       */

#include "zc_alleg.h"
#include <aldumb.h>
#include <alogg.h>
#include <almp3.h>
#include <alspc.h>

#include "zcmusic.h"

// might consider replacing the following with user defined values from the
// 'sound' dialog in the player. This way, each person could tune it as needed.
#define DATASZ  (1<<15)                                     /* (32768) amount of data to read from disk each time */
#define BUFFSZ   (1<<16)                                    /* (65536) size of audiostream buffer */
#define DUH_BUFFSZ    2<<10
#define DUH_CHANNELS  2                                     // stereo
#define DUH_SAMPLES   44100                                 //Hz
#define DUH_RESAMPLE  1

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

typedef struct MP3FILE : public ZCMUSICBASE
{
  ALMP3_MP3STREAM *s;
  PACKFILE *f;
  char *fname;
  int vol;
} MP3FILE;

typedef struct SPCFILE : public ZCMUSICBASE
{
  ALSPC_DATA *s;
  ALSPC_PLAYER *p;
} SPCFILE;

/*
typedef struct SPCFILE : public ZCMUSICBASE {
  AUDIOSTREAM* s;
  PACKFILE *f;
}
*/

void alspc_set_volume( ALSPC_PLAYER *p, float vol )
{
  return;
}

void alspc_pause( ALSPC_PLAYER *p )
{
  return;
}

void alspc_resume( ALSPC_PLAYER *p )
{
  return;
}

#define __GTHREAD_HIDE_WIN32API                             //prevent indirectly including windows.h
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

MP3FILE *load_mp3_file(char *filename);
int poll_mp3_file(MP3FILE *mp3);
void unload_mp3_file(MP3FILE *mp3);
bool mp3_pause(MP3FILE *mp3);
bool mp3_resume(MP3FILE *mp3);
bool mp3_reset(MP3FILE *mp3);

extern "C"
{

  bool zcmusic_init(int flags )                             /* = -1 */
  {
    if (flags & ZCMF_DUH)
    {
      dumb_register_packfiles();
      dumb_resampling_quality = DUH_RESAMPLE;
      libflags |= ZCMF_DUH;
    }
    if (flags & ZCMF_OGG)
    {
      libflags |= ZCMF_OGG;
    }
    if (flags & ZCMF_MP3)
    {
      libflags |= ZCMF_MP3;
    }
    if (flags & ZCMF_SPC)
    {
      alspc_install();
      libflags |= ZCMF_SPC;
    }
    return TRUE;
  }

  bool zcmusic_poll(int flags )                             /* = -1 */
  {
    std::vector<ZCMUSIC*>::iterator b = playlist.begin();
    while (b != playlist.end())
    {
      switch ((*b)->playing)
      {
        case ZCM_STOPPED:
          // if it has stopped, remove it from playlist;
          b = playlist.erase(b);
          break;
        case ZCM_PLAYING:
          switch ((*b)->type & flags & libflags)            // only poll those specified by 'flags'
          {
            case ZCMF_DUH:
              if( ((DUHFILE*)*b)->p )
                al_poll_duh( ((DUHFILE*)*b)->p );
              break;
            case ZCMF_OGG:
              poll_ogg_file( (OGGFILE*)*b );
              break;
            case ZCMF_MP3:
              poll_mp3_file( (MP3FILE*)*b );
              break;
            case ZCMF_SPC:
              if( ((SPCFILE*)*b)->p )
                alspc_poll( ((SPCFILE*)*b)->p );
              break;
          }
        case ZCM_PAUSED:
          b++;
      }
    }
    return TRUE;
  }

  void zcmusic_exit()
  {
    std::vector<ZCMUSIC*>::iterator b = playlist.begin();
    while (b != playlist.end())
    {
      zcmusic_unload_file(*b);
      b = playlist.erase(b);
    }
    playlist.clear();

    if (libflags & ZCMF_DUH)
    {
      dumb_exit();
      libflags ^= ZCMF_DUH;
    }

    if (libflags & ZCMF_OGG)
    {
      libflags ^= ZCMF_OGG;
    }

    if (libflags & ZCMF_MP3)
    {
      libflags ^= ZCMF_MP3;
    }

    if (libflags & ZCMF_SPC)
    {
      alspc_uninstall ();
      libflags ^= ZCMF_SPC;
    }

  }

  ZCMUSIC const * zcmusic_load_file(char *filename)
  {
    if (filename == NULL) return NULL;

    char *ext=get_extension(filename);

    if ((stricmp(ext,"ogg")==0) && (libflags & ZCMF_OGG))
    {
      OGGFILE *p = load_ogg_file(filename);
      if (!p)
      {
        goto error;
      }
      p->fname = (char*)malloc(strlen(filename)+1);
      if (!p->fname)
      {
        unload_ogg_file(p);
        goto error;
      }
      strcpy(p->fname, filename);
      p->type = ZCMF_OGG;
      p->playing = ZCM_STOPPED;
      return (ZCMUSIC*)p;
    }

    if ((stricmp(ext,"mp3")==0) && (libflags & ZCMF_MP3))
    {
      MP3FILE *p = load_mp3_file(filename);
      if (!p)
      {
        goto error;
      }
      p->fname = (char*)malloc(strlen(filename)+1);
      if (!p->fname)
      {
        unload_mp3_file(p);
        goto error;
      }
      strcpy(p->fname, filename);
      p->type = ZCMF_MP3;
      p->playing = ZCM_STOPPED;
      return (ZCMUSIC*)p;
    }

    if (libflags & ZCMF_DUH)
    {
      DUH* d = NULL;
      if (stricmp(ext,"it")==0)
      {
        d = dumb_load_it(filename);
      }
      else if (stricmp(ext,"xm")==0)
      {
        d = dumb_load_xm(filename);
      }
      else if (stricmp(ext,"s3m")==0)
      {
        d = dumb_load_s3m(filename);
      }
      else if (stricmp(ext,"mod")==0)
      {
        d = dumb_load_mod(filename);
      }
      if (d)
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
        return (ZCMUSIC*)p;
      }
    }

    if (libflags & ZCMF_SPC)
    {
      ALSPC_DATA* d = NULL;
      d = alspc_load(filename);
      if (d)
      {
        SPCFILE *p = (SPCFILE*)malloc(sizeof(SPCFILE));
        if(!p)
        {
          alspc_unload (d);
          goto error;
        }
        p->type = ZCMF_SPC;
        p->playing = ZCM_STOPPED;
        p->s = d;
        p->p = NULL;
        return (ZCMUSIC*)p;
      }
    }

    error:
    return NULL;
  }

  bool zcmusic_play(ZCMUSIC* zcm, int vol)
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
    if (zcm == NULL) return FALSE;
    int ret = TRUE;

    if(zcm->playing != ZCM_STOPPED)                         // adjust volume
    {
      switch(zcm->type & libflags)
      {
        case ZCMF_DUH:
          if ( ((DUHFILE*)zcm)->p != NULL )
            al_duh_set_volume( ((DUHFILE*)zcm)->p, (float)vol / (float)255);
          break;
        case ZCMF_OGG:
          if ( ((OGGFILE*)zcm)->s != NULL )
          {
                                                            /*pan*/
            alogg_adjust_oggstream( ((OGGFILE*)zcm)->s, vol, 128, 1000/*speed*/);
            ((OGGFILE*)zcm)->vol = vol;
          }
          break;
        case ZCMF_MP3:
          if ( ((MP3FILE*)zcm)->s != NULL )
          {
                                                            /*pan*/
            almp3_adjust_mp3stream( ((MP3FILE*)zcm)->s, vol, 128, 1000/*speed*/);
            ((MP3FILE*)zcm)->vol = vol;
          }
          break;
        case ZCMF_SPC:
          if ( ((SPCFILE*)zcm)->p != NULL )
          {
            alspc_set_volume( ((SPCFILE*)zcm)->p, (float)vol / (float)255);
          }
          break;
      }
    }
    else
    {
      switch(zcm->type & libflags)
      {
        case ZCMF_DUH:
          if ( ((DUHFILE*)zcm)->s != NULL )
          {
            ((DUHFILE*)zcm)->p = al_start_duh( ((DUHFILE*)zcm)->s, DUH_CHANNELS, 0/*pos*/, ((float)vol) / (float)255, DUH_BUFFSZ/*bufsize*/, DUH_SAMPLES);
            ret = (((DUHFILE*)zcm)->p != NULL) ? TRUE : FALSE;
          }
          break;
        case ZCMF_OGG:
          if ( ((OGGFILE*)zcm)->s != NULL )
          {
            if (alogg_play_oggstream(((OGGFILE*)zcm)->s, BUFFSZ, vol, 128) != ALOGG_OK)
              ret = FALSE;
            ((OGGFILE*)zcm)->vol = vol;
          }
          else
          {
            ret = FALSE;
          }
          break;
        case ZCMF_MP3:
          if ( ((MP3FILE*)zcm)->s != NULL )
          {
            if (almp3_play_mp3stream(((MP3FILE*)zcm)->s, BUFFSZ, vol, 128) != ALMP3_OK)
              ret = FALSE;
            ((MP3FILE*)zcm)->vol = vol;
          }
          else
          {
            ret = FALSE;
          }
          break;
        case ZCMF_SPC:
          if ( ((SPCFILE*)zcm)->s != NULL ) {
            ((SPCFILE*)zcm)->p = alspc_start( ((SPCFILE*)zcm)->s, 44100/*sampling_rate*/,vol,128/*pan*/,1/*stereo*/,1/*interploation*/);
            ret = (((SPCFILE*)zcm)->p != NULL) ? TRUE : FALSE;
          }
          break;
      }

      if (ret != FALSE)
      {
        zcm->playing = ZCM_PLAYING;
        playlist.push_back(zcm);
      }
    }

    return ret;
  }

  bool zcmusic_pause(ZCMUSIC* zcm, int pause = -1)
  {
    // This function suspends play of the music indicated
    // by 'zcm'. Passing 0 for pause will resume; passing
    // -1 (or if the default argument is invoked) will
    // toggle the current state; passing 1 will pause.
    if (zcm == NULL) return FALSE;
    if (zcm->playing != ZCM_STOPPED)
    {
      int p = ZCM_PLAYING;
      switch (pause)
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
      if (p != zcm->playing)                                // if the state has actually changed
      {
        zcm->playing = p;
        switch (zcm->type & libflags)
        {
          case ZCMF_DUH:
            if ( ((DUHFILE*)zcm)->p != NULL )
            {
              if (p == ZCM_PAUSED)
                al_pause_duh( ((DUHFILE*)zcm)->p );
              else
                al_resume_duh( ((DUHFILE*)zcm)->p );
              break;
            }
          case ZCMF_OGG:
            if (p == ZCM_PAUSED)
              ogg_pause( (OGGFILE*)zcm );
            else
              ogg_resume( (OGGFILE*)zcm );
            break;
          case ZCMF_MP3:
            if (p == ZCM_PAUSED)
              mp3_pause( (MP3FILE*)zcm );
            else
              mp3_resume( (MP3FILE*)zcm );
            break;
          case ZCMF_SPC:
            if ( ((SPCFILE*)zcm)->p != NULL )
            {
              if (p == ZCM_PAUSED)
                alspc_pause( ((SPCFILE*)zcm)->p );
              else
                alspc_resume( ((SPCFILE*)zcm)->p );
              break;
            }
        }
      }
    }
    return TRUE;
  }

  bool zcmusic_stop(ZCMUSIC* zcm)
  {
    // this function will stop playback of 'zcm' and reset
    // the stream position to the beginning.
    if (zcm == NULL) return FALSE;
    switch (zcm->type & libflags)
    {
      case ZCMF_DUH:
        if ( ((DUHFILE*)zcm)->p != NULL )
        {
          al_stop_duh( ((DUHFILE*)zcm)->p );
          ((DUHFILE*)zcm)->p = NULL;
          zcm->playing = ZCM_STOPPED;
        }
        break;
      case ZCMF_OGG:
        ogg_reset( (OGGFILE*)zcm );
        break;
      case ZCMF_MP3:
        mp3_reset( (MP3FILE*)zcm );
        break;
      case ZCMF_SPC:
        if ( ((SPCFILE*)zcm)->p != NULL )
        {
          alspc_stop( ((SPCFILE*)zcm)->p );
          ((SPCFILE*)zcm)->p = NULL;
          zcm->playing = ZCM_STOPPED;
        }
        break;
    }
    return TRUE;
  }

  void zcmusic_unload_file(ZCMUSIC* &zcm)
  {
    // this will unload and destroy all of the data/etc.
    // associated with 'zcm'. Also sets the pointer to
    // NULL so you don't try to use it later.
    if (zcm == NULL) return;

    // explicitly remove it from the playlist since we're
    // freeing the memory which holds the ZCM struct.
    // don't want to leave an soon-to-be invalid pointers
    // lying around to cause crashes.
    {
      std::vector<ZCMUSIC*>::iterator b = playlist.begin();
      while (b != playlist.end())
      {
        if (*b == zcm)
        {
          b = playlist.erase(b);
        }
        else
        {
          b++;
        }
      }
    }

    switch (zcm->type & libflags)
    {
      case ZCMF_DUH:
        if ( ((DUHFILE*)zcm)->p != NULL )
        {
          zcmusic_stop(zcm);
          ((DUHFILE*)zcm)->p = NULL;
        }
        if ( ((DUHFILE*)zcm)->s != NULL )
        {
          unload_duh( ((DUHFILE*)zcm)->s );
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
      case ZCMF_SPC:
        if ( ((SPCFILE*)zcm)->p != NULL )
        {
          zcmusic_stop(zcm);
          ((SPCFILE*)zcm)->p = NULL;
        }
        if ( ((SPCFILE*)zcm)->s != NULL )
        {
          alspc_unload( ((SPCFILE*)zcm)->s );
          ((SPCFILE*)zcm)->s = NULL;
          free(zcm);
        }
        break;
    }
    zcm = NULL;
    return;
  }

}                                                           // extern "C"

MP3FILE *load_mp3_file(char *filename)
{
  MP3FILE *p = NULL;
  PACKFILE *f = NULL;
  ALMP3_MP3STREAM *s = NULL;
  char data[DATASZ];
  int len;

  if (!(p = (MP3FILE *)malloc(sizeof(MP3FILE))))
    goto error;
  if (!(f = pack_fopen(filename, F_READ)))
    goto error;
  if ((len = pack_fread(data, DATASZ, f)) <= 0)
    goto error;
  if (len < DATASZ)
  {
    if (!(s = almp3_create_mp3stream(data, len, TRUE)))
      goto error;
  }
  else
  {
    if (!(s = almp3_create_mp3stream(data, DATASZ, FALSE)))
      goto error;
  }
  p->f = f;
  p->s = s;
  return p;

  error:
  pack_fclose(f);
  free(p);
  return NULL;
}

int poll_mp3_file(MP3FILE *mp3)
{
  if (mp3 == NULL) return ALMP3_POLL_NOTPLAYING;
  if (mp3->s == NULL) return ALMP3_POLL_NOTPLAYING;

  char *data = (char *)almp3_get_mp3stream_buffer(mp3->s);
  if (data)
  {
    long len = pack_fread(data, DATASZ, mp3->f);
    if (len < DATASZ)
      almp3_free_mp3stream_buffer(mp3->s, len);
    else
      almp3_free_mp3stream_buffer(mp3->s, -1);
  }
  int ret = almp3_poll_mp3stream(mp3->s);
  if (ret != ALMP3_OK)
  {
    mp3_reset(mp3);
    almp3_play_mp3stream(mp3->s, BUFFSZ, mp3->vol, 128);
    mp3->playing = ZCM_PLAYING;
  }
  return ret;
}

void unload_mp3_file(MP3FILE *mp3)
{
  if (mp3 != NULL)
  {
    if (mp3->f != NULL)
    {
      pack_fclose(mp3->f);
      mp3->f = NULL;
    }
    if (mp3->s != NULL)
    {
      almp3_destroy_mp3stream(mp3->s);
      mp3->s = NULL;
    }
    if (mp3->fname != NULL)
    {
      free(mp3->fname);
      free(mp3);
    }
  }
}

bool mp3_pause(MP3FILE *mp3)
{
  AUDIOSTREAM* a = NULL;
  if (mp3->s != NULL)
    a = almp3_get_audiostream_mp3stream(mp3->s);
  if (a != NULL)
  {
    voice_stop(a->voice);
    return true;
  }
  return false;
}

bool mp3_resume(MP3FILE *mp3)
{
  AUDIOSTREAM* a = NULL;
  if (mp3->s != NULL)
    a = almp3_get_audiostream_mp3stream(mp3->s);
  if (a != NULL)
  {
    voice_start(a->voice);
    return true;
  }
  return false;
}

bool mp3_reset(MP3FILE *mp3)
{
  if (mp3->fname != NULL)
  {
    if(mp3->f != NULL)
    {
      pack_fclose(mp3->f);
      mp3->f = NULL;
    }
    if(mp3->s != NULL)
    {
      almp3_destroy_mp3stream(mp3->s);
      mp3->s = NULL;
    }
    MP3FILE* tmp3 = load_mp3_file(mp3->fname);
    if (tmp3 != NULL)
    {
      mp3->playing = ZCM_STOPPED;
      mp3->s = tmp3->s;
      mp3->f = tmp3->f;
      free(tmp3);
      return true;
    }
  }
  return false;
}

OGGFILE *load_ogg_file(char *filename)
{
  OGGFILE *p = NULL;
  PACKFILE *f = NULL;
  ALOGG_OGGSTREAM *s = NULL;
  char data[DATASZ];
  int len;

  if (!(p = (OGGFILE *)malloc(sizeof(OGGFILE))))
  {
    goto error;
  }
  if (!(f = pack_fopen(filename, F_READ)))
  {
    goto error;
  }
  if ((len = pack_fread(data, DATASZ, f)) <= 0)
  {
    goto error;
  }
  if (len < DATASZ)
  {
    if (!(s = alogg_create_oggstream(data, len, TRUE)))
    {
      goto error;
    }
  }
  else
  {
    if (!(s = alogg_create_oggstream(data, DATASZ, FALSE)))
    {
      goto error;
    }
  }
  p->f = f;
  p->s = s;
  return p;

  error:
  pack_fclose(f);
  free(p);
  return NULL;
}

int poll_ogg_file(OGGFILE *ogg)
{
  if (ogg == NULL) return ALOGG_POLL_NOTPLAYING;
  if (ogg->s == NULL) return ALOGG_POLL_NOTPLAYING;

  char *data = (char *)alogg_get_oggstream_buffer(ogg->s);
  if (data)
  {
    long len = pack_fread(data, DATASZ, ogg->f);
    if (len < DATASZ)
      alogg_free_oggstream_buffer(ogg->s, len);
    else
      alogg_free_oggstream_buffer(ogg->s, -1);
  }
  int ret = alogg_poll_oggstream(ogg->s);
  if (ret != ALOGG_OK)
  {
    ogg_reset(ogg);
    alogg_play_oggstream(ogg->s, BUFFSZ, ogg->vol, 128);
    ogg->playing = ZCM_PLAYING;
  }
  return ret;
}

void unload_ogg_file(OGGFILE *ogg)
{
  if (ogg != NULL)
  {
    if (ogg->f != NULL)
    {
      pack_fclose(ogg->f);
      ogg->f = NULL;
    }
    if (ogg->s != NULL)
    {
      alogg_destroy_oggstream(ogg->s);
      ogg->s = NULL;
    }
    if (ogg->fname != NULL)
    {
      free(ogg->fname);
      free(ogg);
    }
  }
}

bool ogg_pause(OGGFILE *ogg)
{
  AUDIOSTREAM* a = NULL;
  if (ogg->s != NULL)
    a = alogg_get_audiostream_oggstream(ogg->s);
  if (a != NULL)
  {
    voice_stop(a->voice);
    return true;
  }
  return false;
}

bool ogg_resume(OGGFILE *ogg)
{
  AUDIOSTREAM* a = NULL;
  if (ogg->s != NULL)
    a = alogg_get_audiostream_oggstream(ogg->s);
  if (a != NULL)
  {
    voice_start(a->voice);
    return true;
  }
  return false;
}

bool ogg_reset(OGGFILE *ogg)
{
  if (ogg->fname != NULL)
  {
    if(ogg->f != NULL)
    {
      pack_fclose(ogg->f);
      ogg->f = NULL;
    }
    if(ogg->s != NULL)
    {
      alogg_destroy_oggstream(ogg->s);
      ogg->s = NULL;
    }
    OGGFILE* togg = load_ogg_file(ogg->fname);
    if (togg != NULL)
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
