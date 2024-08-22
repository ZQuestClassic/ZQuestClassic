// See https://chromium.googlesource.com/chromium/src/+/HEAD/styleguide/c++/c++.md#check_dcheck_and-notreached

#ifndef BASE_CHECK_H_
#define BASE_CHECK_H_

#undef likely
#undef unlikely
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#  define likely(expr) (__builtin_expect(!!(expr), 1))
#  define unlikely(expr) (__builtin_expect(!!(expr), 0))
#else
#  define likely(expr) (1 == !!(expr))
#  define unlikely(expr) (0 == !!(expr))
#endif

#ifndef CHECK
# define CHECK(expr) if (!likely(expr)) { abort(); }
#endif

#ifdef DCHECK_IS_ON
#  define DCHECK(...) CHECK(__VA_ARGS__);
#else
#  define DCHECK(expr) ((void) 0);
#endif

#define DCHECK_RANGE_INCLUSIVE(x, low, high) DCHECK(x >= low && x <= high)
#define DCHECK_RANGE_EXCLUSIVE(x, low, high) DCHECK(x >= low && x < high)
#define DCHECK_LAYER_ZERO_INDEX(l) DCHECK(l >= 0 && l < 7)
#define DCHECK_LAYER_NEG1_INDEX(l) DCHECK(l >= -1 && l < 6)

#ifndef NOTREACHED
#define NOTREACHED() CHECK(0)
#endif

#endif
