
#pragma once

// C++11
// No idea, but this may need greater accuracy on different platforms. Water as needed.
#if __cplusplus > 199711L
	#define ZC_HAS_CPP0X 1
	#define ZC_HAS_CPP11 1
#else
	#define ZC_HAS_CPP0X 0
	#define ZC_HAS_CPP11 0
#endif

#if defined(_MSC_VER)
	#define _HAS_ITERATOR_DEBUGGING 0
	#define _HAS_EXCEPTIONS 0
	#define _SECURE_SCL 0
#endif

// DebugBreak
#if defined(_MSC_VER)
	#define DEBUGBREAK() __debugbreak()
#elif defined(__GNUC__)
	#define DEBUGBREAK() __builtin_trap()
#else
	#define DEBUGBREAK() __asm { int 3 }
#endif

// Assert ...because <cassert> is pure crap. We WANT to:
// a) break into the debugger at the exact line the assert was triggered,
// b) log it for future reference, and importantly
// c) do NOT exit the application in order to find control flow bugs that exist in release builds.
#ifdef _DEBUG
	#define StaticAssert(x) typedef int assert_##_LINE_ [(x) * 2 - 1];
	#define Assert(x) if(!(x)) { DEBUGBREAK(); }
#else
	#define StaticAssert(x)
	#define Assert(x)
#endif

#define INVALID_CODE_PATH Assert(false)
#define GLOBAL_PTR


#if defined(_MSC_VER)
	typedef __int8				int8;
	typedef __int16				int16;
	typedef __int32				int32;
	typedef __int64				int64;
	typedef unsigned __int8		uint8;
	typedef unsigned __int16	uint16;
	typedef unsigned __int32	uint32;
	typedef unsigned __int64	uint64;
	typedef __int8				i8;
	typedef __int16				i16;
	typedef __int32				i32;
	typedef __int64				i64;
	typedef unsigned __int8		u8;
	typedef unsigned __int16	u16;
	typedef unsigned __int32	u32;
	typedef unsigned __int64	u64;
#else
	typedef int8_t		int8;
	typedef int16_t		int16;
	typedef int32_t		int32;
	typedef int64_t		int64;
	typedef uint8_t		uint8;
	typedef uint16_t	uint16;
	typedef uint32_t	uint32;
	typedef uint64_t	uint64;
	typedef int8_t		i8;
	typedef int16_t		i16;
	typedef int32_t		i32;
	typedef int64_t		i64;
	typedef uint8_t		u8;
	typedef uint16_t	u16;
	typedef uint32_t	u32;
	typedef uint64_t	u64;
#endif

#if defined(_MSC_VER) //Intel also
	#define RESTRICT	__restrict
	#define LIKELY
	#define UNLIKELY
	#define FORCEINLINE	__forceinline
	#define NOINLINE	noinline
#else
	#define RESTRICT	restrict //is this valid only in C99 in gcc? idk.
	#define LIKELY
	#define UNLIKELY
	#define FORCEINLINE	inline
	#define NOINLINE	
#endif

#if (ZC_HAS_CPP11)
	// already defined
#else
	#define nullptr 0
	#define constexpr
#endif

#ifndef NULL
#define NULL 0
#endif

//todo: log defines
#define ScriptLog Printf
#define CompileLog Printf
#define ErrorLog Printf


