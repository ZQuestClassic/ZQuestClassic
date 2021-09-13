#include "common.h"

namespace GUI
{

int getAccelKey(const std::string_view text)
{
	bool lastWasAmpersand=false;
	for(auto& c: text)
	{
		if(c=='&')
		{
			if(lastWasAmpersand)
				lastWasAmpersand=false;
			else
				lastWasAmpersand=true;
		}
		else if(lastWasAmpersand)
			return c;
	}
	return 0;
}

}
