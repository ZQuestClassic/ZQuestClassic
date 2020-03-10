
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
}

