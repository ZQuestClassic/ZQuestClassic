#define ZCMF_DUH      0x00000001
#define ZCMF_OGG      0x00000002
#define ZCMF_MP3      0x00000004
//#define ZCMF_SPC			0x00000008

extern bool zcmusic_init(int flags = -1);
extern bool zcmusic_poll(int flags = -1);
extern void zcmusic_exit();

typedef struct ZCMUSICBASE
{
  int type;
  int playing;
} ZCMUSIC;

extern ZCMUSIC const * zcmusic_load_file(char *filename);
extern bool zcmusic_play(ZCMUSIC* zcm, int vol, int pan);
extern bool zcmusic_pause(ZCMUSIC* zcm, int pause);
extern bool zcmusic_stop(ZCMUSIC* zcm);
extern void zcmusic_unload_file(ZCMUSIC* zcm);
