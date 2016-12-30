//--------------------------------------------------------
//  Zelda Classic
//
//  alleg_compat.cpp
//
//  Compatibility between allegro versions.
//
//--------------------------------------------------------

#include <allegro.h>

PACKFILE *pack_fopen_password(const char *filename, const char *mode, const char *password) {
	packfile_password(password);
	return pack_fopen(filename, mode);
}

uint64_t file_size_ex_password(const char *filename, const char *password) {
	packfile_password(password);
	return file_size_ex(filename);
}