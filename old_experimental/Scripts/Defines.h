
#pragma once

// Reserved Keywords
#define new
#define delete

// Built-Ins...(for now)
#define String string
#define vec2 Vector2f
#define vec2i Vector2i

// Constants
#define EPSILON			0.00001f
#define PI				3.141592653589f
#define TWOPI			6.28318530f
#define TO_RADIANS(x)	((x) * (PI/180.0f))
#define TO_DEGREES(x)	((x) * (180.0f/PI))
#define PI				3.141592653589f
#define PI_OVER_2		1.570796326794f

#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401


// Built-In Function Registrar. (This is my current documentation!!!)
#define Print(s) Print(s)
#define Waitframes(x) Waitframes(x)
#define Waitframe() Waitframes(1)
#define ThrowException(x) ThrowException(x)
#define Rand Rand


// Debugging Macros
#ifdef DEBUG
	#define Log(x) Print(x)
	#define DebugPrint(x) Print(x)
	#define Assert(x) if(!x){ ThrowException(#x); } // Assert is a debug mode only macro.
	#define throw(x) ThrowException(x);

#else
	#define Log(x)
	#define Assert(x)
	#define DebugPrint(x)
	#define throw(x) ThrowException(x); // Throws even when not in debug mode.

#endif




