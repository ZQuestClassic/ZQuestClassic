#include "zalleg/packfile.h"

// Some objects which are stored embedded within containers such as bounded_maps
// need version handling, but cannot have access to the version context they are being
// called from.
// For these objects, declare a 'packfile version', initialized to the current version.
// When loading old quest files, declare a PackfileVersionHandler with the appropriate
// version variable, and the value you want to treat it as. The handler will assign the
// version to the new value for you, and revert it when it is destructed.

int packfile_v_exported_variable = V_EXPORTED_VARIABLE;

PackfileVersionHandler::PackfileVersionHandler(int& ref, int new_ver)
	: ref(ref), old_ver(ref), new_ver(new_ver)
{
	ref = new_ver;
}
PackfileVersionHandler::~PackfileVersionHandler()
{
	ref = old_ver;
}

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
