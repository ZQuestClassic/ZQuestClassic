#include "common.h"
#include "../zc_alleg.h"
#include "../zquest.h"

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


Size Size::em(size_t size)
{
	static const int em=text_height(is_large ? lfont_l : nfont);
	return Size(size*em);
}

Size Size::em(double size)
{
	static const int em=text_height(is_large ? lfont_l : nfont);
	return Size(size*em);
}

Size Size::largePixels(int size)
{
	return Size(is_large ? size : (size*2.0/3.0));
}

}
