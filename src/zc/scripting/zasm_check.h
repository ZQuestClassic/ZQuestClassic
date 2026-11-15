#ifndef ZC_SCRIPTING_ZASM_CHECK_H_
#define ZC_SCRIPTING_ZASM_CHECK_H_

#include "base/check.h"

#include <cassert>

// For code that only ZASM the compiler never emits can reach (a quest file can hold any ZASM).
#define ZASM_CHECK(expr) CHECK(expr)
#define ZASM_DCHECK(expr) DCHECK(expr)
#define ZASM_NOTREACHED() NOTREACHED()

#endif
