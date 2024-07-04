#ifndef _ZCMIXER_H_
#define _ZCMIXER_H_

#include <cstdint>
#include "sound/zcmusic.h"

typedef struct
{
    ZCMUSIC *newtrack;
    ZCMUSIC *oldtrack;

    int32_t fadeinframes;
    int32_t fadeinmaxframes;
    int32_t fadeindelay;

    int32_t fadeoutframes;
    int32_t fadeoutmaxframes;
    int32_t fadeoutdelay;
} ZCMIXER;

ZCMIXER* zcmixer_create();
void zcmixer_update(ZCMIXER* mix, int32_t basevol, int32_t uservol, bool oldscriptvol);
void zcmixer_exit(ZCMIXER* &mix);

#undef ZCM_EXTERN
#endif
