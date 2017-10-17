/* Zelda Classic Music Library        	*/
/* ------------------------------------ */
/* Wrapper for transparently extending	*/
/* supported music file formats.      	*/

#ifndef _ZCMUSIC_H_
#define _ZCMUSIC_H_
#ifdef ZCM_DLL
#define ZCM_EXTERN extern __declspec(dllexport)
#else
#define ZCM_EXTERN extern
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define ZCMF_DUH      0x00000001
#define ZCMF_OGG      0x00000002
#define ZCMF_MP3      0x00000004
#define ZCMF_GME	  0x00000008

#define ZCM_PLAYING 1
#define ZCM_STOPPED 0
#define ZCM_PAUSED -1

#define ZCM_PAUSE   1
#define ZCM_RESUME  0
#define ZCM_TOGGLE -1

  ZCM_EXTERN char const * zcmusic_types;
  ZCM_EXTERN int zcmusic_bufsz;

  ZCM_EXTERN bool zcmusic_init(int flags = -1);
  ZCM_EXTERN bool zcmusic_poll(int flags = -1);
  ZCM_EXTERN void zcmusic_exit();

  typedef struct ZCMUSICBASE
  {
    int type;                                               // uses ZCMF defines
    int playing;                                            // -1 = paused, 0 = stopped, 1 = playing
  } ZCMUSIC;

  ZCM_EXTERN ZCMUSIC const * zcmusic_load_file(char *filename);
  ZCM_EXTERN bool zcmusic_play(ZCMUSIC* zcm, int vol, bool autopoll=0);
  ZCM_EXTERN bool zcmusic_pause(ZCMUSIC* zcm, int pause);
  ZCM_EXTERN bool zcmusic_stop(ZCMUSIC* zcm);
  ZCM_EXTERN void zcmusic_unload_file(ZCMUSIC* &zcm);
  ZCM_EXTERN int zcmusic_get_tracks(ZCMUSIC* zcm);
  ZCM_EXTERN int zcmusic_change_track(ZCMUSIC* zcm, int tracknum);

#ifdef __cplusplus
}                                                           // extern "C"
#endif

#undef ZCM_EXTERN
#endif
