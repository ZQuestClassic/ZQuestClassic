
#include "util.h"
#include <sys/stat.h>
using namespace std;

#define PATH_MODE		0755

namespace util
{
	void upperstr(string& str)
	{
		for(int q = str.size() - 1; q >= 0; --q)
			str[q] = toupper(str[q]);
	}

	void lowerstr(string& str)
	{
		for(int q = str.size() - 1; q >= 0; --q)
			str[q] = tolower(str[q]);
	}
	
	void replchar(std::string& str, char from, char to)
	{
		for(int q = str.size() - 1; q >= 0; --q)
		{
			if(str.at(q)==from)
				str[q] = to;
		}
	}
	
	void replchar(char* buf, char from, char to)
	{
		for(int i = 0; buf[i]!=0; ++i)
		{
			if(buf[i]==from)
				buf[i] = to;
		}
	}

	string get_ext(string const& path)
	{
		size_t dot_pos = path.find_last_of(".");
		if(dot_pos == string::npos) return "";
		size_t last_slash_pos = path.find_last_of("/\\");
		if(last_slash_pos != string::npos && last_slash_pos > dot_pos) return ""; //. found is in a dir name, not filename!
		string ext = path.substr(dot_pos);
		lowerstr(ext);
		return ext;
	}
	
	static bool valid_single_dir(string const& path)
	{
		if(path.find_first_of("<>|?*&^$#\"") != string::npos) return false; //Contains invalid chars
		size_t nonslash_pos = path.find_last_not_of("/\\");
		if(nonslash_pos == string::npos) return false; //blank or all slashes
		if(path[0] == '/' || path[0] == '\\') return false; //multiple consecutive slashes
		if(path.find_first_not_of("./\\") == string::npos) return false; //empty dirname
		if(path.find("..") == 0) return false; //cannot begin with >1 dot
		if(path.find("...") != string::npos) return false; //cannot contain >2 consecutive dots
		return true;
	}
	
	bool valid_dir(string const& path)
	{
		size_t pos = path.find_first_not_of("/\\");
		if(pos == string::npos) return false;
		while(pos < path.length())
		{
			size_t next_slash = path.find_first_of("/\\",pos);
			if(next_slash == string::npos) break;
			if(!valid_single_dir(path.substr(pos,next_slash-pos))) return false;
			pos = next_slash+1;
		}
		return true;
	}
	
	bool valid_file(string const& path)
	{
		if(path.find_first_of("<>|?*&^$#\"") != string::npos) return false; //Contains invalid chars
		size_t last_slash_pos = path.find_last_of("/\\");
		if(last_slash_pos == string::npos) last_slash_pos = 0;
		else
		{
			if(!valid_dir(path.substr(0,last_slash_pos))) return false;
			++last_slash_pos;
		}
		if(last_slash_pos == path.length() - 1) return false; //Ends in slash; empty filename
		string fname = path.substr(last_slash_pos);
		if(fname.find_first_of(":") != string::npos) return false; //invalid char
		if(fname.find_first_not_of(".") == string::npos) return false; //empty filename
		if(fname.find("..") == 0) return false; //cannot begin with >1 dot
		if(fname.find("...") != string::npos) return false; //cannot contain >2 consecutive dots
		return true;
	}
	
	void regulate_path(char* buf)
	{
		for(int q = 0; buf[q]; ++q)
		{
#ifdef _WIN32
			if (buf[q] == '/') buf[q] = '\\';
#else
			if (buf[q] == '\\') buf[q] = '/';
#endif
		}
	}
	
	void regulate_path(string& buf)
	{
		for(int q = 0; buf[q]; ++q)
		{
#ifdef _WIN32
			if (buf[q] == '/') buf[q] = '\\';
#else
			if (buf[q] == '\\') buf[q] = '/';
#endif
		}
	}
	
	int do_mkdir(const char* path, int mode)
	{
#ifdef _WIN32
		return _mkdir(path);
#else
		return mkdir(path,mode);
#endif
	}

	bool create_path(const char* path)
	{
		while((path[0] == '/' || path[0] == '\\') && path[0]) ++path; //trim leading slashes
		char buf[2048] = {0};
		int q = 0;
		int last_slash = -1;
		for(; path[q] && q < 2048; ++q)
		{
			buf[q] = path[q];
			if(path[q] == '/' || path[q] == '\\')
			{
				string strpath(buf+last_slash+1);
				if(!valid_single_dir(strpath))
				{
					return false; //Failure; invalid path
				}
				last_slash = q;
				if(strpath.find_first_of(":") != string::npos) continue; //Non-creatable; ex "C:\"
				struct stat info;
				if(stat( buf, &info ) != 0)
				{
					if (do_mkdir(buf, PATH_MODE) != 0 && errno != EEXIST)
						return false; //Failure; could not create
				}
				else if((info.st_mode & S_IFDIR)==0)
				{
					return false; //Hit failure; exists, but is not dir.
				}
			}
		}
		return q < 2048;
	}
	
	char* zc_itoa(int value, char* str, int base)
	{
#ifdef _WIN32
		return _itoa(value, str, base);
#else
		static char dig[] =
			"0123456789"
			"abcdefghijklmnopqrstuvwxyz";
		int n = 0, neg = 0;
		unsigned int v;
		char* p, *q;
		char c;
		if (base == 10 && value < 0) 
		{
			value = -value;
			neg = 1;
		}
		v = value;
		do 
		{
			str[n++] = dig[v%base];
			v /= base;
		} while (v);
		if (neg)
		{
			str[n++] = '-';
		}
		str[n] = '\0';
		for (p = str, q = p + n/2; p != q; ++p, --q)
			{
			c = *p, *p = *q, *q = c;
		}
		return str;
#endif
	}
	
	int zc_chmod(const char* path, mode_t mode)
	{
#ifdef _WIN32
		return _chmod(path,mode);
#else
		return chmod(path,mode);
#endif
	}

	bool checkPath(const char* path, const bool is_dir)
	{
		struct stat info;

		if(stat( path, &info ) != 0)
			return false;
		else return is_dir ? (info.st_mode & S_IFDIR)!=0 : (info.st_mode & S_IFDIR)==0;
	}
}

