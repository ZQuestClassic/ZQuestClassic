/**
 * A file for basic utility functions
 */
#ifndef _UTIL_H_
#define _UTIL_H_
#include "zdefs.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <stdlib.h>
#else
typedef int mode_t;
#endif
namespace util
{
	void upperstr(std::string& str);
	void lowerstr(std::string& str);
	void replchar(std::string& str, char from, char to);
	void replchar(char* buf, char from, char to);
	std::string get_ext(std::string const& path);
	bool valid_dir(std::string const& path);
	bool valid_file(std::string const& path);
	void regulate_path(char* buf);
	void regulate_path(std::string& buf);
	bool create_path(const char* path);
	char* zc_itoa(int value, char* str, int base);
	int zc_chmod(const char* path, mode_t mode);
}

#endif

