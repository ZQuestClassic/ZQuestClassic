/**
 * A file for basic utility functions
 */
#ifndef _UTIL_H_
#define _UTIL_H_
#include "zdefs.h"
#include <string>
#ifndef _WIN32
#include <stdlib.h>
#endif
namespace util
{
	void upperstr(std::string& str);
	void lowerstr(std::string& str);
	std::string get_ext(std::string const& path);
	void regulate_path(char* buf);
	bool create_path(const char* path);
	char* zc_itoa(int value, char* str, int base);
}

#endif

