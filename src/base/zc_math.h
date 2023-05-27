#ifndef __zc_math_h_
#define __zc_math_h_

#include <math.h>
#include "sin1.h"
#include "zc/replay.h"

namespace zc
{


namespace math
{


inline bool IsPowerOfTwo(uint32_t x)
{
    return (x != 0 && !((x-1) & x));
}

inline uint32_t NextPowerOfTwo(uint32_t x)
{
    uint32_t y(1);
    
    while(y < x) y <<= 1;
    
    return y;
}

inline float ToDegrees(float radians)
{
    return (radians * 57.29578f);
}

inline float ToRadians(float degrees)
{
    return (degrees * 0.0174533f);
}

inline float Round(float x)
{
    return floorf(x + 0.5f);
}

inline float Lerp(float a, float b, float t)
{
    return (a + (b - a) * t);
}

inline float CalculateBezier(const float p1, const float t1, const float p2, float t)
{
    const float a = 1.0f - t;
    const float A = a * a;
    const float B = 2.0f * a * t;
    const float C = t * t;
    
    return ((p1 * A) + (t1 * B) + (p2 * C));
}

inline float CalculateBezier(const float p1, const float t1, const float t2, const float p2, float t)
{
    const float a = 1.0f - t;
    const float A = a * a * a;
    const float B = 3.0f * a * a * t;
    const float C = 3.0f * a * t * t;
    const float D = t * t * t;
    
    return ((p1 * A) + (t1 * B) + (t2 * C) + (p2 * D));
}

inline float CalculateBezier(const float p1, const float t1, const float t2, const float t3, const float p2, float t)
{
    const float a = 1.0f - t, b = a * a, c = t * t, d = (1.0f - t) * t;
    const float A = b * b;
    const float B = 4.0f * b * d;
    const float C = 6.0f * b * c;
    const float D = 4.0f * d * c;
    const float E = c * c;
    
    return ((p1 * A) + (t1 * B) + (t2 * C) + (t3 * D) + (p2 * E));
}

#define Q15 (1.0/(double)((1<<15)-1))
inline double Sin(double x)
{
	if (replay_is_active())
	{
		// x needs to be converted from radians -> angles -> sin1 domain
		x = x * (180/PI * 32768.0/360.0);
		x = (long)x % 0x8000;
		double r = sin1(x) * Q15;
		// round to 4 decimal places, otherwise some "critical" values
		// will be slightly off (ex: Cos(0) == 0.99996, instead of 1)
		if (replay_get_version() >= 4)
			return std::round(r * 10000.0) / 10000.0;
		return r;
	}
	else
		return std::sin(x);
}

inline double Cos(double x)
{
	if (replay_is_active())
	{
		x = x * (180/PI * 32768.0/360.0);
		x = (long)x % 0x8000;
		double r = cos1(x) * Q15;
		if (replay_get_version() >= 4)
			return std::round(r * 10000.0) / 10000.0;
		return r;
	}
	else
		return std::cos(x);
}

inline double Tan(double x)
{
	if (replay_is_active())
	{
		x = x * (180/PI * 32768.0/360.0);
		x = (long)x % 0x8000;
		double r = (sin1(x) * Q15) / (cos1(x) * Q15);
		if (replay_get_version() >= 4)
			return std::round(r * 10000.0) / 10000.0;
		return r;
	}
	else
		return std::tan(x);
}



} //namespace math


} //namespace zc

#endif
