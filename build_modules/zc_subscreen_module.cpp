
#include "precompiled.h"

#define USE_ENCRYPTION
//misc
//#include "callback.cpp"
#include "encryptionEnabled.cpp"
//#include "encryptionDisabled.cpp"


//subscreen
#include "subscr.cpp"
#include "zc_subscr.cpp"
#include "md5.cpp"


//sound
#include "midi.cpp"
#include "sfxAllegro.cpp"
#include "sfxClass.cpp"
#include "sfxManager.cpp"


// Some zc files requires zquest headers and that causes problems when unity building,
// so we keep them chained up in here and just feed them fish heads.
#include "gui.cpp"
#include "zc_sys.cpp"

int zcmusic_bufsz = 64;