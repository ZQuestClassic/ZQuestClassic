#include "refInfo.h"
#include <cstring>

refInfo::refInfo()
{
    Clear();
}

void refInfo::Clear()
{
    pc=0;
    sp=0;
    scriptflag=0;
    ffcref=0;
    idata=0;
    itemref=0;
    guyref=0;
    lwpn=0;
    ewpn=0;
    std::memset(d, 0, 8 * sizeof(long));
    a[0]=0; // Should these reset to 10000?
    a[1]=0;
}
