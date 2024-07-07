#ifndef ZCMIXER_H_
#define ZCMIXER_H_

#include <cstdint>
#include "sound/zcmusic.h"

#if defined ZCM_DLL
#define ZCM_EXTERN extern __declspec(dllexport)
#elif defined ZCM_DLL_IMPORT
#define ZCM_EXTERN extern __declspec(dllimport)
#else
#define ZCM_EXTERN extern
#endif

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

ZCM_EXTERN ZCMIXER* zcmixer_create();
ZCM_EXTERN void zcmixer_update(ZCMIXER* mix, int32_t basevol, int32_t uservol, bool oldscriptvol);
ZCM_EXTERN void zcmixer_exit(ZCMIXER* &mix);

#undef ZCM_EXTERN
#endif
