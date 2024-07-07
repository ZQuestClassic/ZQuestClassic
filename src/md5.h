/* See md5.c for explanation and copyright information.  */

/*
 * $FreeBSD: src/contrib/cvs/lib/md5.h,v 1.2 1999/12/11 15:10:02 peter Exp $
 */

#ifndef BASE_MD5_H_
#define BASE_MD5_H_

#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif

/* Unlike previous versions of this code, uint32 need not be exactly
   32 bits, merely 32 bits or more.  Choosing a data type which is 32
   bits instead of 64 is not important; speed is considerably more
   important.  ANSI guarantees that "uint32_t" will be big enough,
   and always using it seems to have few disadvantages.  */
typedef uint32_t cvs_uint32;

typedef struct
{
    cvs_uint32 buf[4];
    cvs_uint32 bits[2];
    uint8_t in[64];
} cvs_MD5Context;

void cvs_MD5Init(cvs_MD5Context *context);
void cvs_MD5Update(cvs_MD5Context *context, uint8_t const *buf, unsigned len);
void cvs_MD5Final(uint8_t digest[16], cvs_MD5Context *context);
void cvs_MD5Transform(cvs_uint32 buf[4], const uint8_t in[64]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
