#ifndef CLIB_NOTREACHED_H
#define CLIB_NOTREACHED_H

#include "check.h"

// equivalent to CEHCK(false)
#ifndef NOTREACHED
#define NOTREACHED() CHECK(0)
#endif

// alias `NOTREACHED()` because YOLO
#ifndef NOTREACHABLE
#define NOTREACHABLE NOTREACHED
#endif

#endif

