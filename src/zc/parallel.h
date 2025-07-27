#ifndef ZC_PARALLEL_H_
#define ZC_PARALLEL_H_

// The <execution> header defines compiler-provided execution policies, but is not always present.
// See: https://github.com/alugowski/poolSTL
#if __cplusplus >= 201603L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201603L)
#if __has_include(<execution>)
#ifndef POOLSTL_STD_SUPPLEMENT_NO_INCLUDE
// On GCC and Clang simply including <execution> requires linking with TBB.
// MinGW does not require TBB, but performance may be sequential. To keep parallel performance fallback to poolSTL.
#include <execution>
#endif
#endif
#endif

#define POOLSTL_STD_SUPPLEMENT
#include <poolstl/poolstl.hpp>

#endif
