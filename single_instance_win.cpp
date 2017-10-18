//#include "zc_alleg.h"
#include <windows.h>
#include "single_instance.h"

bool is_only_instance(const char* name)
{
    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, name);
	
    if (!hMutex)
	{
		hMutex = CreateMutex(0, 0, name);
		return true;
	}
    else
		return false;
}