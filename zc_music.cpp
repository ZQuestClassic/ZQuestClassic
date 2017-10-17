#include "zc_alleg.h"
#include <aldumb.h>
#include <alogg.h>
#include <almp3.h>
//#include <libspc.h>

#include "zc_music.h"

// might consider replacing the following with a user defined value from the
// 'sound' dialog in the player. This way, each person could tune it as needed.
#define DATASZ  (1<<15)                                     /* (32768) amount of data to read from disk each time */
#define BUFSZ   (1<<16)                                     /* (65536) size of audiostream buffer */

typedef struct DUHFILE : public ZCMUSICBASE
{
  DUH *s;
  AL_DUH_PLAYER *p;
} DUHFILE;

typedef struct OGGFILE : public ZCMUSICBASE
{
  //alogg_stream *s;
  ALOGG_OGGSTREAM *s;
  PACKFILE *f;
} OGGFILE;

typedef struct MP3FILE : public ZCMUSICBASE
{
  ALMP3_MP3STREAM *s;
  PACKFILE *f;
} MP3FILE;
/*
typedef struct SPCFILE : public ZCMUSICBASE {
  AUDIOSTREAM* s;
  PACKFILE *f;
}
*/

//forward declarations
MP3FILE *load_mp3_file(char *filename);
int poll_mp3_file(MP3FILE *mp3);
void unload_mp3_file(MP3FILE *mp3);

OGGFILE *load_ogg_file(char *filename);
int poll_ogg_file(MP3FILE *mp3);
void unload_ogg_file(MP3FILE *mp3);

#define BITMAP WINDOWSBITMAP                                //hack because <vector> indirectly includes windows.h
#include <vector>
#undef BITMAP
static std::vector<ZCMUSIC*> playlist;                      //yeah, I'm too lazy to do it myself

ZCMUSIC const * zcmusic_load_file(char *filename)
{
  char *ext=get_extension(filename);

  /*
   */

  if (stricmp(ext,"ogg")==0)
  {
    /*
        ALOGG_OGGSTREAM *s = alogg_start_streaming(filename, BUFSZ);
        if (!s)
          goto error;
        OGGFILE *of = (OGGFILE*)malloc(sizeof(OGGFILE));
        if (!of) {
          alogg_stop_streaming(s);
          goto error;
        }
        of->type = ZCMF_OGG;
        of->playing = FALSE;
    of->s = s;
    return (ZCMUSIC*)of;
    */
    OGGFILE *p = load_ogg_file(filename);
    if (!p)
      goto error;
    p->type = ZCMF_OGG;
    p->playing = FALSE;
    return (ZCMUSIC*)p;
  }

  if (stricmp(ext,"mp3")==0)
  {
    MP3FILE *p = load_mp3_file(filename);
    if (!p)
      goto error;
    p->type = ZCMF_MP3;
    p->playing = FALSE;
    return (ZCMUSIC*)p;
  }

  // TODO: Add the DUH loading functions.

  error:
  return NULL;
}

bool zcmusic_play(ZCMUSIC* zcm, int vol, int pan)
{
  // the OGG and MP3 libraries require polling
  // of individual streams, so here we must keep
  // record of each OGG and MP3 file which is
  // playing, so we can iterate over all of them
  // when zcmusic_poll() is called.
  //
  // In addition, any music library which actually
  // has a 'play' function or similar will be
  // called from here.
  int ret = TRUE;

  switch(zcm->type)
  {
    case ZCMF_DUH:
                                                            /*n_channels*/
      ((DUHFILE*)zcm)->p = al_start_duh(((DUHFILE*)zcm)->s, 2, 0/*pos*/,
        1.0, DATASZ/*bufsize*/, 44100/*freq*/);             /*volume*/
      ret = (int)(((DUHFILE*)zcm)->p);
      break;
    case ZCMF_OGG:
      if (alogg_play_oggstream(((OGGFILE*)zcm)->s, BUFSZ, vol, pan) == ALOGG_OK)
        playlist.push_back(zcm);
      else
        ret = FALSE;
      break;
    case ZCMF_MP3:
      if (almp3_play_mp3stream(((MP3FILE*)zcm)->s, BUFSZ, vol, pan) == ALMP3_OK)
        playlist.push_back(zcm);
      else
        ret = FALSE;
      break;
  }

  if (ret) zcm->playing = 1;
  return ret;
}

bool zcmusic_pause(ZCMUSIC* zcm, int pause = -1)
{
  // This function suspends play of the music indicated
  // by 'zcm'. Passing 0 for pause will resume; passing
  // <0 (or if the default argument is invoked) will
  // toggle the current state; passing >0 will pause.
  return TRUE;
}

bool zcmusic_stop(ZCMUSIC* zcm)
{
  // this function will stop playback of 'zcm' and reset
  // the stream position to the beginning.
  return TRUE;
}

void zcmusic_unload_file(ZCMUSIC* &zcm)
{
  // this will unload and destroy all of the data/etc.
  // associated with 'zcm'. Also sets the pointer to
  // NULL so you don't try to use it later.
  return;
}

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
  char *data;
  long len;

  data = (char *)almp3_get_mp3stream_buffer(mp3->s);
  if (data)
  {
    len = pack_fread(data, DATASZ, mp3->f);
    if (len < DATASZ)
      almp3_free_mp3stream_buffer(mp3->s, len);
    else
      almp3_free_mp3stream_buffer(mp3->s, -1);
  }

  return almp3_poll_mp3stream(mp3->s);
}

void unload_mp3_file(MP3FILE *mp3)
{
  if (mp3)
  {
    pack_fclose(mp3->f);
    almp3_destroy_mp3stream(mp3->s);
    free(mp3);
  }
}

OGGFILE *load_ogg_file(char *filename)
{
  OGGFILE *p = NULL;
  PACKFILE *f = NULL;
  ALOGG_OGGSTREAM *s = NULL;
  char data[DATASZ];
  int len;

  if (!(p = (OGGFILE *)malloc(sizeof(OGGFILE))))
    goto error;
  if (!(f = pack_fopen(filename, F_READ)))
    goto error;
  if ((len = pack_fread(data, DATASZ, f)) <= 0)
    goto error;
  if (len < DATASZ)
  {
    if (!(s = alogg_create_oggstream(data, len, TRUE)))
      goto error;
  }
  else
  {
    if (!(s = alogg_create_oggstream(data, DATASZ, FALSE)))
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

int poll_ogg_file(OGGFILE *ogg)
{
  char *data;
  long len;

  data = (char *)alogg_get_oggstream_buffer(ogg->s);
  if (data)
  {
    len = pack_fread(data, DATASZ, ogg->f);
    if (len < DATASZ)
      alogg_free_oggstream_buffer(ogg->s, len);
    else
      alogg_free_oggstream_buffer(ogg->s, -1);
  }

  return alogg_poll_oggstream(ogg->s);
}

void unload_ogg_file(OGGFILE *ogg)
{
  if (ogg)
  {
    pack_fclose(ogg->f);
    alogg_destroy_oggstream(ogg->s);
    free(ogg);
  }
}
