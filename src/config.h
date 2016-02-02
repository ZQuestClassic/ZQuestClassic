
#pragma once

// C++11
#if _MSC_VER <= 1600
	#define nullptr 0
	#define constexpr
#endif

// C++
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

// Assert
#ifdef _DEBUG
	#define StaticAssert(x) typedef int assert_##_LINE_ [(x) * 2 - 1];
	#define Assert(x) if(!(x)) DEBUGBREAK()
#else
	#define StaticAssert(x)
	#define Assert(x)
#endif


#define INVALID_CODE_PATH Assert(false)


#if defined(_MSC_VER)
	typedef __int8				int8;
	typedef __int16				int16;
	typedef __int32				int32;
	typedef __int64				int64;
	typedef unsigned __int8		uint8;
	typedef unsigned __int16	uint16;
	typedef unsigned __int32	uint32;
	typedef unsigned __int64	uint64;
#else
// should these use stdint?
	typedef char				int8;
	typedef short				int16;
	typedef int					int32;
	typedef long long			int64;
	typedef unsigned char		uint8;
	typedef unsigned short		uint16;
	typedef unsigned int		uint32;
	typedef unsigned long long	uint64;
#endif

