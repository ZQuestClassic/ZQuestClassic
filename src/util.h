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
typedef int32_t mode_t;
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
	char* zc_itoa(int32_t value, char* str, int32_t base = 10);
	int64_t zc_atoi64(const char *str);
	int64_t zc_xtoi64(const char *hexstr);
	int32_t zc_xtoi(const char *hexstr);
	int32_t ffparse2(const char *string);
	int32_t ffparseX(const char *string);
	int32_t xtoi(char *hexstr);
	int32_t zc_chmod(const char* path, mode_t mode);
	bool checkPath(const char* path, const bool is_dir);
	void safe_al_trace(const char* str);
}
int32_t vbound(int32_t val, int32_t low, int32_t high);
double vbound(double val, double low, double high);

//#define vbound(x,low,high) (x<low?low:(x>high?high:x))

#endif

