
#pragma once

#ifndef _zc_malloc_h
#define _zc_malloc_h

#include <assert.h>

// This should catch the following:
// -double deletions,
// -calls to free on unallocated memory,
// -memory leaks from malloc calls.
// -unanswered prayers to babylonian gods.


#define ZC_DEBUG_MALLOC_ENABLED 0


#if defined(NDEBUG) && (ZC_DEBUG_MALLOC_ENABLED)
#define ZC_MALLOC_ALWAYS_ASSERT(x) assert(x), __zc_always_assert(x, #x, __FILE__, __LINE__)
#else
#define ZC_MALLOC_ALWAYS_ASSERT(x) assert(x)
#endif


extern void *__zc_debug_malloc(size_t numBytes, const char* file, int line);
extern void  __zc_debug_free(void* p, const char* file, int line);
extern void  __zc_debug_malloc_free_print_memory_leaks();

#if (ZC_DEBUG_MALLOC_ENABLED != 0)
#define zc_malloc(x)	__zc_debug_malloc(x, __FILE__, __LINE__)
#define zc_free(x)		__zc_debug_free(x, __FILE__, __LINE__)
#else
#define zc_malloc	malloc
#define zc_free		free
#endif


void __zc_always_assert(bool e, const char* expression, const char* file, int line);

#endif


