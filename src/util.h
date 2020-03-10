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
	void replchar(std::string& str, char from, char to);
	void replchar(char* buf, char from, char to);
}

#endif

