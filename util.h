#ifndef _UTIL_H_
#define _UTIL_H_

// Temporary!
#include "zsys.h"

inline int clamp(int x, int low, int high) { return vbound(x, low, high); }
int wrap(int x, int low, int high); // This is in zq_misc... Does it not exist in ZC, then?

#endif
