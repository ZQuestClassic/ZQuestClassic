/*
 * Custom ZFix: because Allegro's zfix is not precise
 */


#ifndef ZFIX_H
#define ZFIX_H
#include "zdefs.h"
#include <math.h>
#include <limits>
#define FIX_NAN 0
//std::numeric_limits<t>::quiet_NaN()

typedef int32_t ZLong;
typedef long long zint64;

class zfix;
inline zfix zslongToFix(ZLong val);
inline ZLong toZLong(float val);
inline ZLong toZLong(double val);
inline ZLong toZLong(int val);
inline ZLong toZLong(long val);
inline zfix floor(zfix fx);
inline zfix abs(zfix fx);

static int zfixvbound(ZLong x, int low, int high)
{
	assert(low <= high);
	if(x<low) return low;
	if(x>high) return high;
	return x;
}
//inline int vbound(int x,int low,int high);
//inline float vbound(float x,float low,float high);



class zfix
{
public:
	ZLong val;

	long getInt() const
	{
		return val/10000L + zfixvbound((val%10000L)/5000L,-1,1);
	}
	double getFloat() const
	{
		return val/10000.0;
	}
	ZLong getZLong() const
	{
		return val;
	}
	int getDPart() const
	{
		return val%10000;
	}
	
	
	
	zfix& doFloor()
	{
		val = (val / 10000) * 10000;
		return *this;
	}
	long getFloor() const
	{
		return val / 10000L;
	}
	
	zfix& doRound()
	{
		if ((val % 10000) >= 5000) val = ((val / 10000)+1) * 10000;
		else val = (val / 10000) * 10000;
		return *this;
	}
	long getRound() const
	{
		if ((val % 10000) >= 5000) return ((val / 10000)+1);
		else return (val / 10000);
	}
	
	zfix& doAbs()
	{
		val = abs(val);
		return *this;
	}
public:
	
	zfix() : val(0)											{}
	zfix(const zfix &v) : val(v.val)						{}
	explicit zfix(const int v) : val(v*10000L)				{}
	explicit zfix(const long v) : val(v*10000L)				{}
	explicit zfix(const unsigned int v) : val(v*10000L)		{}
	explicit zfix(const unsigned long v) : val(v*10000L)	{}
	explicit zfix(const float v) : val(v*10000L)			{}
	explicit zfix(const double v) : val(v*10000L)			{}
	explicit zfix(const int ip, const int dp)
	{
		val = ip*10000L + dp;
	}
	
	zfix copy() const							{ zfix t; t.val = val; return t; }
	
	operator int() const						{ return getInt(); }
	operator long() const						{ return getInt(); }
	operator unsigned int() const				{ return getInt(); }
	operator unsigned long() const				{ return getInt(); }
	operator float() const						{ return getFloat(); }
	operator double() const						{ return getFloat(); }
	
	zfix& operator = (const zfix &fx)			{ val = fx.val; return *this; }
	zfix& operator = (const int v)				{ val = v*10000L; return *this; }
	zfix& operator = (const long v)				{ val = v*10000L; return *this; }
	zfix& operator = (const unsigned int v)		{ val = v*10000L; return *this; }
	zfix& operator = (const unsigned long v)	{ val = v*10000L; return *this; }
	zfix& operator = (const float v)			{ val = v*10000L; return *this; }
	zfix& operator = (const double v)			{ val = v*10000L; return *this; }
	
	zfix& operator +=  (const zfix fx)	{ val += fx.val; return *this; }
	zfix& operator +=  (const int v)	{ val += v*10000L; return *this; }
	zfix& operator +=  (const long v)	{ val += v*10000L; return *this; }
	zfix& operator +=  (const float v)	{ val += v*10000L; return *this; }
	zfix& operator +=  (const double v)	{ val += v*10000L; return *this; }
	
	zfix& operator -=  (const zfix fx)	{ val -= fx.val; return *this; }
	zfix& operator -=  (const int v)	{ val -= v*10000L; return *this; }
	zfix& operator -=  (const long v)	{ val -= v*10000L; return *this; }
	zfix& operator -=  (const float v)	{ val -= v*10000L; return *this; }
	zfix& operator -=  (const double v)	{ val -= v*10000L; return *this; }
	
	static long longMul(long a, long b)	{ zint64 c = a*b; return (long)(c/10000L);}
	zfix& operator *=  (const zfix fx)	{ val = longMul(val, fx.val); return *this; }
	zfix& operator *=  (const int v)	{ val *= v; return *this; }
	zfix& operator *=  (const long v)	{ val *= v; return *this; }
	zfix& operator *=  (const float v)	{ val = longMul(val, toZLong(v)); return *this; }
	zfix& operator *=  (const double v)	{ val = longMul(val, toZLong(v)); return *this; }
	
	static long longDiv(long a, long b)	{ zint64 c = a*10000L; return (long)(c/b); }
	zfix& operator /=  (const zfix fx)	{
		if(fx.val == 0) val = toZLong(FIX_NAN);
		else val = longDiv(val, fx.val); return *this; }
	zfix& operator /=  (const int v)	{
		if(v == 0) val = toZLong(FIX_NAN);
		else val /= v; return *this; }
	zfix& operator /=  (const long v)	{
		if(v == 0) val = toZLong(FIX_NAN);
		else val /= v; return *this; }
	zfix& operator /=  (const float v)	{
		if(toZLong(v) == 0) val = toZLong(FIX_NAN);
		else val = longDiv(val, toZLong(v)); return *this; }
	zfix& operator /=  (const double v)	{
		if(toZLong(v) == 0) val = toZLong(FIX_NAN);
		else val = longDiv(val, toZLong(v)); return *this; }
	
	zfix& operator <<= (const int v)	{ val <<= v; return *this; }
	zfix& operator >>= (const int v)	{ val >>= v; return *this; }
	
	zfix& operator ++ ()				{ val += 10000; return *this; }
	zfix& operator -- ()				{ val -= 10000; return *this; }
	
	zfix operator ++ (int)				{ zfix t = copy(); val += 10000; return t; }
	zfix operator -- (int)				{ zfix t = copy(); val -= 10000; return t; }
	
	zfix operator - () const			{ zfix t; t.val = -val; return t; }
	
	inline friend zfix operator +  (const zfix fx, const zfix fx2);
	inline friend zfix operator +  (const zfix fx, const int v);
	inline friend zfix operator +  (const int v, const zfix fy);
	inline friend zfix operator +  (const zfix fx, const long v);
	inline friend zfix operator +  (const long v, const zfix fy);
	inline friend zfix operator +  (const zfix fx, const float v);
	inline friend zfix operator +  (const float v, const zfix fy);
	inline friend zfix operator +  (const zfix fx, const double v);
	inline friend zfix operator +  (const double v, const zfix fy);
	
	inline friend zfix operator -  (const zfix fx, const zfix fx2);
	inline friend zfix operator -  (const zfix fx, const int v);
	inline friend zfix operator -  (const int v, const zfix fx);
	inline friend zfix operator -  (const zfix fx, const long v);
	inline friend zfix operator -  (const long v, const zfix fx);
	inline friend zfix operator -  (const zfix fx, const float v);
	inline friend zfix operator -  (const float v, const zfix fx);
	inline friend zfix operator -  (const zfix fx, const double v);
	inline friend zfix operator -  (const double v, const zfix fx);
	
	inline friend zfix operator *  (const zfix fx, const zfix fx2);
	inline friend zfix operator *  (const zfix fx, const int v);
	inline friend zfix operator *  (const int v, const zfix fx);
	inline friend zfix operator *  (const zfix fx, const long v);
	inline friend zfix operator *  (const long v, const zfix fx);
	inline friend zfix operator *  (const zfix fx, const float v);
	inline friend zfix operator *  (const float v, const zfix fx);
	inline friend zfix operator *  (const zfix fx, const double v);
	inline friend zfix operator *  (const double v, const zfix fx);
	
	inline friend zfix operator /  (const zfix fx, const zfix fx2);
	inline friend zfix operator /  (const zfix fx, const int v);
	inline friend zfix operator /  (const int v, const zfix fx);
	inline friend zfix operator /  (const zfix fx, const long v);
	inline friend zfix operator /  (const long v, const zfix fx);
	inline friend zfix operator /  (const zfix fx, const float v);
	inline friend zfix operator /  (const float v, const zfix fx);
	inline friend zfix operator /  (const zfix fx, const double v);
	inline friend zfix operator /  (const double v, const zfix fx);
	
	inline friend zfix operator << (const zfix fx, const int v);
	inline friend zfix operator >> (const zfix fx, const int v);
	
	inline friend int operator == (const zfix fx, const zfix fx2);
	inline friend int operator == (const zfix fx, const int v);
	inline friend int operator == (const int v, const zfix fx);
	inline friend int operator == (const zfix fx, const long v);
	inline friend int operator == (const long v, const zfix fx);
	inline friend int operator == (const zfix fx, const float v);
	inline friend int operator == (const float v, const zfix fx);
	inline friend int operator == (const zfix fx, const double v);
	inline friend int operator == (const double v, const zfix fx);
	
	inline friend int operator != (const zfix fx, const zfix fx2);
	inline friend int operator != (const zfix fx, const int v);
	inline friend int operator != (const int v, const zfix fx);
	inline friend int operator != (const zfix fx, const long v);
	inline friend int operator != (const long v, const zfix fx);
	inline friend int operator != (const zfix fx, const float v);
	inline friend int operator != (const float v, const zfix fx);
	inline friend int operator != (const zfix fx, const double v);
	inline friend int operator != (const double v, const zfix fx);
	
	inline friend int operator <  (const zfix fx, const zfix fx2);
	inline friend int operator <  (const zfix fx, const int v);
	inline friend int operator <  (const int v, const zfix fx);
	inline friend int operator <  (const zfix fx, const long v);
	inline friend int operator <  (const long v, const zfix fx);
	inline friend int operator <  (const zfix fx, const float v);
	inline friend int operator <  (const float v, const zfix fx);
	inline friend int operator <  (const zfix fx, const double v);
	inline friend int operator <  (const double v, const zfix fx);
	
	inline friend int operator >  (const zfix fx, const zfix fx2);
	inline friend int operator >  (const zfix fx, const int v);
	inline friend int operator >  (const int v, const zfix fx);
	inline friend int operator >  (const zfix fx, const long v);
	inline friend int operator >  (const long v, const zfix fx);
	inline friend int operator >  (const zfix fx, const float v);
	inline friend int operator >  (const float v, const zfix fx);
	inline friend int operator >  (const zfix fx, const double v);
	inline friend int operator >  (const double v, const zfix fx);
	
	inline friend int operator <= (const zfix fx, const zfix fx2);
	inline friend int operator <= (const zfix fx, const int v);
	inline friend int operator <= (const int v, const zfix fx);
	inline friend int operator <= (const zfix fx, const long v);
	inline friend int operator <= (const long v, const zfix fx);
	inline friend int operator <= (const zfix fx, const float v);
	inline friend int operator <= (const float v, const zfix fx);
	inline friend int operator <= (const zfix fx, const double v);
	inline friend int operator <= (const double v, const zfix fx);
	
	inline friend int operator >= (const zfix fx, const zfix fx2);
	inline friend int operator >= (const zfix fx, const int v);
	inline friend int operator >= (const int v, const zfix fx);
	inline friend int operator >= (const zfix fx, const long v);
	inline friend int operator >= (const long v, const zfix fx);
	inline friend int operator >= (const zfix fx, const float v);
	inline friend int operator >= (const float v, const zfix fx);
	inline friend int operator >= (const zfix fx, const double v);
	inline friend int operator >= (const double v, const zfix fx);
};


#include "zfix.inl"

#endif		  /* ifndef ZFIX_H */
