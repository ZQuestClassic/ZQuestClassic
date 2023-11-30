/**
 * A file for basic utility functions
 */
#ifndef _UTIL_H_
#define _UTIL_H_

#include "base/zdefs.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <filesystem>
#include <vector>
#include <functional>
#include <algorithm>

#ifndef _WIN32
#include <stdlib.h>
#else
typedef int32_t mode_t;
#endif

namespace fs = std::filesystem;

namespace util
{
	void upperstr(std::string& str);
	void lowerstr(std::string& str);
	void trimstr(std::string& str);
	void sanitize(std::string& str);
	void sanitize_spaces_ok(std::string& str);
	size_t split(const std::string &txt, std::vector<std::string> &strs, char ch);
	std::vector<std::string> split_args(const std::string &str);
	std::string read_text_file(fs::path path);
	std::istream &portable_get_line(std::istream &is, std::string &t);
	std::string cropPath(std::string filepath);
	void replstr(std::string& str, std::string const& from, std::string const& to);
	void replace_first(std::string& s, std::string const& toReplace, std::string const& replaceWith);
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
	void safe_al_trace(std::string const& str);
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
	std::filesystem::path create_new_file_path(std::filesystem::path dir, std::string filename_prefix, std::string ext, bool force_suffix = false);
	std::string create_temp_file_path(std::string final_destination);

	template <typename T, std::size_t R, std::size_t C>
	inline void copy_2d_array(T source[R][C] , T dest[R][C])
	{
		std::copy(&source[0][0], &source[0][0] + (R*C)/sizeof(source[0][0]), &dest[0][0]);
	}
}

std::string dayextension(int32_t dy);
bool fileexists(const char *filename);

int32_t compare(int32_t a, int32_t b);

char const* get_snap_str();

void clear_clip_rect(BITMAP* bitmap);

int32_t zc_trace_handler(const char *);
void zc_trace_clear();

std::string QRHINT(std::vector<int> qrs);
std::string RULETMPL_HINT(std::vector<int> tmpls);
enum //special insert types
{
	INFOHINT_T_QR,
	INFOHINT_T_RULETMPL,
	INFOHINT_T_MAX
};

enum
{
	BSEARCH_FAIL,
	BSEARCH_SUCCEED,
	BSEARCH_CONTINUE_UP,
	BSEARCH_CONTINUE_DOWN,
	BSEARCH_CONTINUE_AWAY0,
	BSEARCH_CONTINUE_TOWARD0
};
int binary_search_int(int b1, int b2, std::function<int(int,int&)> proc, int defval = 0);
zfix binary_search_zfix(zfix b1, zfix b2, std::function<int(zfix,zfix&)> proc, zfix defval = 0);

#endif

