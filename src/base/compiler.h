#ifndef BASE_COMPILER_H_
#define BASE_COMPILER_H_

#if defined(__GNUC__)
  #define ZC_FORCE_INLINE inline __attribute__((__always_inline__))
#elif defined(_MSC_VER)
  #define ZC_FORCE_INLINE __forceinline
#else
  #define ZC_FORCE_INLINE inline
#endif

#endif
