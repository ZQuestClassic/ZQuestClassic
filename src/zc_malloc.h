
#pragma once

#ifndef _zc_malloc_h
#define _zc_malloc_h

#ifdef HAS_BOOST
#include <boost/type_traits.hpp>
#endif // HAS_BOOST

#include <cassert>
#include <cstdlib>

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
#endif // defined(NDEBUG) && (ZC_DEBUG_MALLOC_ENABLED)


extern void *__zc_debug_malloc(size_t numBytes, const char* file, int line);
extern void  __zc_debug_free(void* p, const char* file, int line);
extern void  __zc_debug_malloc_free_print_memory_leaks();

#if (ZC_DEBUG_MALLOC_ENABLED != 0)
#define zc_malloc(x)	__zc_debug_malloc(x, __FILE__, __LINE__)
#define zc_free(x)		__zc_debug_free(x, __FILE__, __LINE__)
#else
#define zc_malloc	malloc
#define zc_free		free
#endif // (ZC_DEBUG_MALLOC_ENABLED != 0)

void __zc_always_assert(bool e, const char* expression, const char* file, int line);

/***
* Summary: Deallocates a pointer or raw dynamic array
* typeparam: Pointer - a pointer type
* param: p - Pointer or array to delete
* param: is_array - Set this to true if you are deleting an array, otherwise, leave it false
* remarks: Raw dynamic arrays have no size information so we cannot differentiate by type_trait
**/
template <typename Pointer>
inline void delete_s(Pointer p, bool is_array = false)
{
#ifdef HAS_BOOST
	BOOST_STATIC_ASSERT(boost::is_pointer<Pointer>::value);
#endif // HAS_BOOST

	if (is_array) {
		delete[] p;
	}
	else {
		delete p;
	}
	p = Pointer();
}

/***
* Summary: Same as std::free but nullifies the pointer, can be nullptr, e.g. (remove_pointer_t<Pointer>*)0
* Remarks: Never pass stack pointers or pointers allocated by "new"
*		   as the behavior is undefined (seg fault/crash) if the value passed was not returned
*		   before from std::malloc, std::calloc, std::realloc, and std::aligned_alloc (C11/C++17)
**/
template <typename Pointer>
inline void free_s(Pointer p)
{
#ifdef HAS_BOOST
	BOOST_STATIC_ASSERT(boost::is_pointer<Pointer>::value);
#endif // HAS_BOOST
	std::free(p);
	p = Pointer();
}

#endif // _zc_malloc_h


