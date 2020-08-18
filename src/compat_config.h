/***
* Author: Samer Charif
* Desciption: Mainly used for suppressing compiler warnings and to 
*             ease in new lanaguage features when the time comes
* Date: 8/18/2020
**/

/* keywords */
#if (__cplusplus >= 201103L)
/* constexpr is a keyword */
#else
#define constexpr const
#endif /* (__cplusplus >= 201103L) */

/***
* Attributes - C++17 is checked first because those are where the standard defines them
**/
#if (__cplusplus >= 201704L)
#define _MAYBE_UNSUED(x) x [[maybe_unused]]
#define _FALLTHROUGH [[fallthrough]]
#elif defined(__GNUC__) || defined(__clang__)
#define _MAYBE_UNSUED(x) x __attribute__((unused))
#define _FALLTHROUGH __attribute__((fallthrough))
#elif defined(_MSC_VER)
#define _MAYBE_UNUSED(x) static_cast<void>(x)
#define _FALLTHROUGH __fallthrough /* annotates depending on your compiler, sometimes does nothing */
#else
#define _MAYBE_UNUSED(x) static_cast<void>(x)
#define _FALLTHROUGH /* no standard way to do it */
#endif /* (__cplusplus >= 201704L) */

