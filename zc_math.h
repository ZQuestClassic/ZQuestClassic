
#ifndef __zc_math_h_
#define __zc_math_h_

#include <math.h>

namespace zc
{


namespace math
{


inline bool IsPowerOfTwo( unsigned int x )
	{
		return (x != 0 && !((x-1) & x));
	}

inline unsigned int NextPowerOfTwo( unsigned int x )
	{
		unsigned int y(1);
		while( y < x ) y <<= 1;
		return y;
	}

inline float ToDegrees( float radians ) 
	{ 
		return (radians * 57.29578f); 
	}

inline float ToRadians( float degrees ) 
	{ 
		return (degrees * 0.0174533f); 
	}

inline float Round( float x )
	{
		return floorf(x + 0.5f);
	}

inline float Lerp( float a, float b, float t )
	{
		return (a + (b - a) * t);
	}

inline float CalculateBezier( const float p1, const float t1, const float p2, float t )
	{
		const float a = 1.0f - t;
		const float A = a * a;
		const float B = 2.0f * a * t;
		const float C = t * t;

		return ((p1 * A) + (t1 * B) + (p2 * C));
	}

inline float CalculateBezier( const float p1, const float t1, const float t2, const float p2, float t )
	{
		const float a = 1.0f - t;
		const float A = a * a * a;
		const float B = 3.0f * a * a * t;
		const float C = 3.0f * a * t * t;
		const float D = t * t * t;

		return ((p1 * A) + (t1 * B) + (t2 * C) + (p2 * D));
	}

inline float CalculateBezier( const float p1, const float t1, const float t2, const float t3, const float p2, float t )
	{
		const float a = 1.0f - t, b = a * a, c = t * t, d = (1.0f - t) * t;
		const float A = b * b;
		const float B = 4.0f * b * d;
		const float C = 6.0f * b * c;
		const float D = 4.0f * d * c;
		const float E = c * c;

		return ((p1 * A) + (t1 * B) + (t2 * C) + (t3 * D) + (p2 * E));
	}





} //namespace math


} //namespace zc

#endif