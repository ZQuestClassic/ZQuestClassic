#ifndef _MSC_VER
/*
 * This file is part of the Mingw32 package.
 *
 * unistd.h maps (roughly) to io.h
 */

#ifndef _UNISTD_H
#define _UNISTD_H

#include <io.h>
#include <process.h>

#define __UNISTD_GETOPT__
#include "getopt.h"
#undef __UNISTD_GETOPT__

#ifdef __cplusplus
extern "C" {
#endif

/* This is defined as a real library function to allow autoconf
   to verify its existence. */
int32_t ftruncate(int32_t, off_t);
/*__CRT_INLINE*/inline int32_t ftruncate(int32_t __fd, off_t __length)
{
    return _chsize(__fd, __length);
}

#ifdef __cplusplus
}
#endif

#endif /* _UNISTD_H */

#endif //_MSC_VER

