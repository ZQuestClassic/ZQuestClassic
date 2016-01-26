#ifndef _ZC_REFINFO_H_
#define _ZC_REFINFO_H_

#include "types.h"

class refInfo
{
public:
    dword pc; //current command offset
    
    long d[8]; //d registers
    long a[2]; //a regsisters (reference to another ffc on screen)
    byte sp; //stack pointer for current script
    dword scriptflag; //stores whether various operations were true/false etc.
    
    byte ffcref, idata; //current object pointers
    dword itemref, guyref, lwpn, ewpn;
    
    
    refInfo();
    void Clear();
};

#endif
