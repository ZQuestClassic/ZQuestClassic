
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
	
	string get_ext(string const& path)
	{
		size_t pos = path.find_last_of(".");
		if(pos == string::npos) return "";
		string ext = path.substr(pos);
		lowerstr(ext);
		return ext;
	}
	
	void regulate_path(char* buf)
	{
		for(int q = 0; buf[q]; ++q)
		{
#ifdef ALLEGRO_WINDOWS
			if (buf[q] == '/') buf[q] = '\\';
#else
			if (buf[q] == '\\') buf[q] = '/';
#endif
		}
	}
	
	int do_mkdir(const char* path, int mode)
	{
#ifdef ALLEGRO_WINDOWS
		return _mkdir(path);
#else
		return mkdir(path,mode);
#endif
	}

	bool create_path(const char* path)
	{
		char buf[2048] = {0};
		int q = 0;
		for(; path[q] && q < 2048; ++q)
		{
			buf[q] = path[q];
			if(path[q] == '/' || path[q] == '\\')
			{
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
}

