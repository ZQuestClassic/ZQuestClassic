//--------------------------------------------------------
//  Zelda Classic
//
//  alleg_compat.cpp
//
//  Compatibility between allegro versions.
//
//--------------------------------------------------------

#include "base/allegro_wrapper.h"

PACKFILE *pack_fopen_password(const char *filename, const char *mode, const char *password)
{
	packfile_password(password);
	PACKFILE *new_pf = pack_fopen(filename, mode);
	packfile_password("");
	return new_pf;
}

uint64_t file_size_ex_password(const char *filename, const char *password)
{
	packfile_password(password);
	uint64_t new_pf = file_size_ex(filename);
	packfile_password("");
	return new_pf;
}

