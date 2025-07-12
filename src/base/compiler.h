#ifndef BASE_COMPILER_H_
#define BASE_COMPILER_H_

#if !defined(_DEBUG) && defined(__GNUC__)
  #define ZC_FORCE_INLINE inline __attribute__((__always_inline__))
#elif !defined(_DEBUG) && defined(_MSC_VER)
  #define ZC_FORCE_INLINE __forceinline
#else
  #define ZC_FORCE_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define ZC_FORMAT_PRINTF(format_idx, first_vararg_idx) __attribute__((format(printf, format_idx, first_vararg_idx)))
#else
	#define ZC_FORMAT_PRINTF(format_idx, first_vararg_idx)
#endif

#endif
