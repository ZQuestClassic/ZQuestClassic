/**
 * A file for basic utility functions
 */
#ifndef _UTIL_H_
#define _UTIL_H_
#include "base/zdefs.h"
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
	void trimstr(std::string& str);
	std::string cropPath(std::string filepath);
	void replstr(std::string& str, std::string const& from, std::string const& to);
	std::string escape_characters(std::string const& str);
	std::string disallow_escapes(std::string const& str);
	std::string unescape_characters(std::string const& str);
	void replchar(std::string& str, char from, char to);
	void replchar(char* buf, char from, char to);
	void removechar(std::string& str, char to_delete);
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
	bool zc_isalpha(int c);
	std::string escape_string(char const* str);
	std::string escape_string(std::string const& str);
	std::string unescape_string(char const* str);
	std::string unescape_string(std::string const& str);
	size_t escstr_size(char const* str);
	std::string stringify_vector(std::vector<int32_t> const& vec, bool dec);
	void unstringify_vector(std::vector<int32_t>& vec, std::string const& str, bool dec);
	void unstringify_vector(std::vector<int32_t>& vec, char const* str, bool dec);
	size_t vecstr_size(char const* str);
}
int32_t vbound(int32_t val, int32_t low, int32_t high);
double vbound(double val, double low, double high);

std::string dayextension(int32_t dy);
bool fileexists(const char *filename);

int32_t compare(int32_t a, int32_t b);

char const* get_snap_str();

//#define vbound(x,low,high) (x<low?low:(x>high?high:x))

#endif

