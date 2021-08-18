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
	std::string cropPath(std::string filepath);
	void replchar(std::string& str, char from, char to);
	void replchar(char* buf, char from, char to);
	std::string get_ext(std::string const& path);
	bool valid_dir(std::string const& path);
	bool valid_file(std::string const& path);
	void regulate_path(char* buf);
	void regulate_path(std::string& buf);
	bool create_path(const char* path);
	char* zc_itoa(int value, char* str, int base = 10);
	long long zc_atoi64(const char *str);
	long long zc_xtoi64(const char *hexstr);
	int zc_xtoi(const char *hexstr);
	long ffparse2(const char *string);
	long ffparseX(const char *string);
	int xtoi(char *hexstr);
	int zc_chmod(const char* path, mode_t mode);
	bool checkPath(const char* path, const bool is_dir);
	void safe_al_trace(const char* str);
	int vbound(int x,int low,int high);
	float vbound(float x,float low,float high);
}

#endif

