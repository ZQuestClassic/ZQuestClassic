/* See md5.c for explanation and copyright information.  */

/*
 * $FreeBSD: src/contrib/cvs/lib/md5.h,v 1.2 1999/12/11 15:10:02 peter Exp $
 */

#ifndef MD5_H
#define MD5_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Unlike previous versions of this code, uint32 need not be exactly
   32 bits, merely 32 bits or more.  Choosing a data type which is 32
   bits instead of 64 is not important; speed is considerably more
   important.  ANSI guarantees that "unsigned long" will be big enough,
   and always using it seems to have few disadvantages.  */
typedef unsigned long cvs_uint32;

typedef struct
{
    cvs_uint32 buf[4];
    cvs_uint32 bits[2];
    unsigned char in[64];
} cvs_MD5Context;

void cvs_MD5Init(cvs_MD5Context *context);
void cvs_MD5Update(cvs_MD5Context *context, unsigned char const *buf, unsigned len);
void cvs_MD5Final(unsigned char digest[16], cvs_MD5Context *context);
void cvs_MD5Transform(cvs_uint32 buf[4], const unsigned char in[64]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !MD5_H */
