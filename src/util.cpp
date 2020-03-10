
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
}

