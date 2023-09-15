#include "packfile.h"

bool pfwrite(const char *p,int32_t n,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		size_t len = strlen(p);
		if (len > n) len = n;
		int zeros = n - len;
		success=(pack_fwrite(p,len,f)==len);
		if (!success)
			return false;

		for (int i = 0; i < zeros; i++)
		{
			if (pack_putc(0, f)==EOF)
				return false;
		}
	}
	
	if(success)
	{
		writesize+=n;
	}
	
	return success;
}
