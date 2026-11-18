#ifndef ZC_SCRIPTING_ZASM_CHECK_H_
#define ZC_SCRIPTING_ZASM_CHECK_H_

#include "base/check.h"

#include <cassert>

// For code that only ZASM the compiler never emits can reach (a quest file can hold any ZASM).
// These are CHECK, DCHECK and NOTREACHED, except in a fuzzing build (-DWANT_FUZZ=ON): there the
// fuzzer makes such ZASM all the time, so rather than report each as a crash these end the script
// run, and the zasm fuzz target drops the input (see src/zc/fuzz.cpp).
#ifdef ZC_FUZZ
[[noreturn]] void fuzz_reject_invalid_zasm();
// For outside of a script run (ex: compiling it), where there is nothing to end: the zasm fuzz
// target checks for this before it runs the script.
void fuzz_mark_invalid_zasm();
#define ZASM_CHECK(expr) do { if (!likely(expr)) fuzz_reject_invalid_zasm(); } while (0)
#define ZASM_DCHECK(expr) ZASM_CHECK(expr)
#define ZASM_NOTREACHED() fuzz_reject_invalid_zasm()
// Like ZASM_DCHECK, but carries on instead of ending the script run: for code that handles the
// failure itself, and that runs where jumping out isn't possible (ex: resetting a script after its
// run is over).
#define ZASM_DCHECK_CONTINUE(expr) do { if (!likely(expr)) fuzz_mark_invalid_zasm(); } while (0)
#else
#define ZASM_CHECK(expr) CHECK(expr)
#define ZASM_DCHECK(expr) DCHECK(expr)
#define ZASM_NOTREACHED() NOTREACHED()
#define ZASM_DCHECK_CONTINUE(expr) DCHECK(expr)
#endif

#endif
