
#include "util.h"
using namespace std;

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
}

