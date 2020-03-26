/**
 * A file for basic utility functions
 */
#ifndef _UTIL_H_
#define _UTIL_H_
#include "zdefs.h"

namespace util
{
	void upperstr(std::string& str);
	void lowerstr(std::string& str);
	std::string get_ext(std::string const& path);
}

#endif

