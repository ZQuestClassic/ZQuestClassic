//--------------------------------------------------------
//  Zelda Classic
//
//  alleg_compat.h
//
//  Compatibility between allegro versions.
//
//--------------------------------------------------------

#ifndef _ZC_ALLEG_COMPAT_H_
#define _ZC_ALLEG_COMPAT_H_

PACKFILE *pack_fopen_password(const char *filename, const char *mode, const char *password);
uint64_t file_size_ex_password(const char *filename, const char *password);

#endif // _ZC_ALLEG_COMPAT_H_