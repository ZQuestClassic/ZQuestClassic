/*
 * Custom ZFix: because Allegro's zfix is not precise
 */


#ifndef ZFIX_H
#define ZFIX_H
#include <cstdint>
#include <math.h>
#include <limits>
#include <assert.h>
#define FIX_NAN 0
//std::numeric_limits<t>::quiet_NaN()

// Weird internal compiler error, but only for RelWithDebInfo 32bit...
// Also may be only VS2020
//
//     double angle = atan2(double(y-(Hero.y)),double(Hero.x-x))
//
// results in error:
//
//      fatal error C1001: Internal compiler error
//
// Can avoid by using temporary variables.
// There's a lot of these, so this macro helps by making drop-in replacement simple.
// Usages of this macro must define these variables in scope somewhere:
// double _MSVC2022_tmp1, _MSVC2022_tmp2;
#define atan2_MSVC2022_FIX(x, y) (\
    _MSVC2022_tmp1 = x,\
    _MSVC2022_tmp2 = y,\
    atan2(_MSVC2022_tmp1, _MSVC2022_tmp2)\
);

typedef int32_t ZLong;
typedef int64_t zint64;

class zfix;
inline zfix zslongToFix(ZLong val);
inline ZLong toZLong(float val);
inline ZLong toZLong(double val);
inline ZLong toZLong(int32_t val);
inline ZLong toZLong(int32_t val);
inline zfix floor(zfix const& fx);
inline zfix abs(zfix const& fx);
inline zfix pow(zfix const& fx, int exp);
inline zfix dist(zfix const& x1, zfix const& y1, zfix const& x2, zfix const& y2);

static int32_t zfixvbound(ZLong x, int32_t low, int32_t high)
{
	assert(low <= high);
	if(x<low) return low;
	if(x>high) return high;
	return x;
}
//inline int32_t vbound(int32_t x,int32_t low,int32_t high);
//inline float vbound(float x,float low,float high);



class zfix
{
public:
	ZLong val;
	
	int32_t sign() const
	{
		if(val < 0) return -1;
		if(val > 0) return 1;
		return 0;
	}
	zfix decsign() const
	{
		return zslongToFix(sign());
	}
	
	int32_t getInt() const
	{
		return val/10000L + zfixvbound((val%10000L)/5000L, (val < 0 ? -1 : 0),(val<0 ? 0 : 1));
	}
	double getFloat() const
	{
		return val/10000.0;
	}
	ZLong getZLong() const
	{
		return val;
	}
	int32_t getZLongDPart() const
	{
		return val%10000;
	}
	zfix getDPart() const
	{
		return zslongToFix(val%10000);
	}
	zfix& doDPart()
	{
		val %= 10000;
		return *this;
	}
	
	
	
	int32_t getFloor() const
	{
		int32_t v = val/10000L;
		if(val%10000 && val < 0) --v;
		return v;
	}
	zfix& doFloor()
	{
		val = getFloor() * 10000;
		return *this;
	}
	int32_t getCeil() const
	{
		int32_t v = val/10000L;
		if(val%10000 && val > 0) ++v;
		return v;
	}
	zfix& doCeil()
	{
		val = getCeil() * 10000;
		return *this;
	}
	
	int32_t getRound() const
	{
		int32_t dpart = val%10000;
		int32_t v = val/10000;
		if(val < 0)
		{
			if(dpart <= -5000) --v;
		}
		else
		{
			if(dpart >= 5000) ++v;
		}
		return v;
	}
	zfix& doRound()
	{
		val = getRound() * 10000;
		return *this;
	}
	
	zfix& doAbs()
	{
		val = abs(val);
		return *this;
	}
	zfix getAbs() const
	{
		zfix ret = copy();
		ret.doAbs();
		return ret;
	}
	
	
	int32_t getTrunc() const
	{
		return val/10000;
	}
	zfix& doTrunc()
	{
		val = getTrunc() * 10000;
		return *this;
	}
	
	int32_t getRoundAway() const
	{
		int32_t v = val/10000;
		if(val%10000)
		{
			v += sign();
		}
		return v;
	}
	zfix& doRoundAway()
	{
		val = getRoundAway() * 10000;
		return *this;
	}
	
	zfix& doBound(zfix low, zfix high)
	{
		if(low < high)
			return doBound(high,low);
		if(val < low.val)
			val = low.val;
		if(val > high.val)
			val = high.val;
		return *this;
	}
	zfix& doDecBound(int low_ipart, int low_dpart, int high_ipart, int high_dpart)
	{
		return doBound(zfix(low_ipart,low_dpart),zfix(high_ipart,high_dpart));
	}
public:
	
	zfix() : val(0)											{}
	zfix(const zfix &v) : val(v.val)						{}
	zfix(const int32_t v) : val(v*10000L)				{}
	zfix(const uint32_t v) : val(v*10000L)		{}
	zfix(const float v) : val(int32_t(v*10000L))			{}
	zfix(const double v) : val(int32_t(v*10000L))			{}
	explicit zfix(const int32_t ip, const int32_t dp)
	{
		val = ip*10000L + dp;
	}
	
	zfix copy() const							{ zfix t; t.val = val; return t; }
	
	operator int32_t() const						{ return getInt(); }
	operator uint32_t() const				{ return getInt(); }
	operator float() const						{ return getFloat(); }
	operator double() const						{ return getFloat(); }
	operator bool() const						{ return val!=0; }
	
	zfix& operator = (const zfix &fx)			{ val = fx.val; return *this; }
	zfix& operator = (const int32_t v)				{ val = v*10000L; return *this; }
	zfix& operator = (const uint32_t v)		{ val = v*10000L; return *this; }
	zfix& operator = (const float v)			{ val = int32_t(v*10000L); return *this; }
	zfix& operator = (const double v)			{ val = int32_t(v*10000L); return *this; }
	
	zfix& operator +=  (const zfix fx)	{ val += fx.val; return *this; }
	zfix& operator +=  (const int32_t v)	{ val += v*10000L; return *this; }
	zfix& operator +=  (const float v)	{ val += int32_t(v*10000L); return *this; }
	zfix& operator +=  (const double v)	{ val += int32_t(v*10000L); return *this; }
	
	zfix& operator -=  (const zfix fx)	{ val -= fx.val; return *this; }
	zfix& operator -=  (const int32_t v)	{ val -= v*10000L; return *this; }
	zfix& operator -=  (const float v)	{ val -= int32_t(v*10000L); return *this; }
	zfix& operator -=  (const double v)	{ val -= int32_t(v*10000L); return *this; }
	
	static int32_t longMul(int32_t a, int32_t b)	{ zint64 c = int64_t(a)*b; return (int32_t)(c/10000L);}
	zfix& operator *=  (const zfix fx)	{ val = longMul(val, fx.val); return *this; }
	zfix& operator *=  (const int32_t v)	{ val *= v; return *this; }
	zfix& operator *=  (const float v)	{ val = longMul(val, toZLong(v)); return *this; }
	zfix& operator *=  (const double v)	{ val = longMul(val, toZLong(v)); return *this; }
	
	static int32_t longDiv(int32_t a, int32_t b)	{ zint64 c = int64_t(a)*10000L; return (int32_t)(c/b); }
	zfix& operator /=  (const zfix fx)	{
		if(fx.val == 0) val = toZLong(FIX_NAN);
		else val = longDiv(val, fx.val); return *this; }
	zfix& operator /=  (const int32_t v)	{
		if(v == 0) val = toZLong(FIX_NAN);
		else val /= v; return *this; }
	zfix& operator /=  (const float v)	{
		if(toZLong(v) == 0) val = toZLong(FIX_NAN);
		else val = longDiv(val, toZLong(v)); return *this; }
	zfix& operator /=  (const double v)	{
		if(toZLong(v) == 0) val = toZLong(FIX_NAN);
		else val = longDiv(val, toZLong(v)); return *this; }
	
	zfix& operator <<= (const int32_t v)	{ val <<= v; return *this; }
	zfix& operator >>= (const int32_t v)	{ val >>= v; return *this; }
	
	zfix& operator ++ ()				{ val += 10000; return *this; }
	zfix& operator -- ()				{ val -= 10000; return *this; }
	bool operator ! ()				    { return !val; }
	
	zfix operator ++ (int32_t)				{ zfix t = copy(); val += 10000; return t; }
	zfix operator -- (int32_t)				{ zfix t = copy(); val -= 10000; return t; }
	
	zfix operator - () const			{ zfix t; t.val = -val; return t; }
	
	inline friend zfix operator +  (const zfix fx, const zfix fx2);
	inline friend zfix operator +  (const zfix fx, const int32_t v);
	inline friend zfix operator +  (const int32_t v, const zfix fy);
	inline friend zfix operator +  (const zfix fx, const int32_t v);
	inline friend zfix operator +  (const int32_t v, const zfix fy);
	inline friend zfix operator +  (const zfix fx, const float v);
	inline friend zfix operator +  (const float v, const zfix fy);
	inline friend zfix operator +  (const zfix fx, const double v);
	inline friend zfix operator +  (const double v, const zfix fy);
	
	inline friend zfix operator -  (const zfix fx, const zfix fx2);
	inline friend zfix operator -  (const zfix fx, const int32_t v);
	inline friend zfix operator -  (const int32_t v, const zfix fx);
	inline friend zfix operator -  (const zfix fx, const int32_t v);
	inline friend zfix operator -  (const int32_t v, const zfix fx);
	inline friend zfix operator -  (const zfix fx, const float v);
	inline friend zfix operator -  (const float v, const zfix fx);
	inline friend zfix operator -  (const zfix fx, const double v);
	inline friend zfix operator -  (const double v, const zfix fx);
	
	inline friend zfix operator *  (const zfix fx, const zfix fx2);
	inline friend zfix operator *  (const zfix fx, const int32_t v);
	inline friend zfix operator *  (const int32_t v, const zfix fx);
	inline friend zfix operator *  (const zfix fx, const int32_t v);
	inline friend zfix operator *  (const int32_t v, const zfix fx);
	inline friend zfix operator *  (const zfix fx, const float v);
	inline friend zfix operator *  (const float v, const zfix fx);
	inline friend zfix operator *  (const zfix fx, const double v);
	inline friend zfix operator *  (const double v, const zfix fx);
	
	inline friend zfix operator /  (const zfix fx, const zfix fx2);
	inline friend zfix operator /  (const zfix fx, const int32_t v);
	inline friend zfix operator /  (const int32_t v, const zfix fx);
	inline friend zfix operator /  (const zfix fx, const int32_t v);
	inline friend zfix operator /  (const int32_t v, const zfix fx);
	inline friend zfix operator /  (const zfix fx, const float v);
	inline friend zfix operator /  (const float v, const zfix fx);
	inline friend zfix operator /  (const zfix fx, const double v);
	inline friend zfix operator /  (const double v, const zfix fx);
	
	inline friend zfix operator << (const zfix fx, const int32_t v);
	inline friend zfix operator >> (const zfix fx, const int32_t v);
	
	inline friend int32_t operator == (const zfix fx, const zfix fx2);
	inline friend int32_t operator == (const zfix fx, const int32_t v);
	inline friend int32_t operator == (const int32_t v, const zfix fx);
	inline friend int32_t operator == (const zfix fx, const int32_t v);
	inline friend int32_t operator == (const int32_t v, const zfix fx);
	inline friend int32_t operator == (const zfix fx, const float v);
	inline friend int32_t operator == (const float v, const zfix fx);
	inline friend int32_t operator == (const zfix fx, const double v);
	inline friend int32_t operator == (const double v, const zfix fx);
	
	inline friend int32_t operator != (const zfix fx, const zfix fx2);
	inline friend int32_t operator != (const zfix fx, const int32_t v);
	inline friend int32_t operator != (const int32_t v, const zfix fx);
	inline friend int32_t operator != (const zfix fx, const int32_t v);
	inline friend int32_t operator != (const int32_t v, const zfix fx);
	inline friend int32_t operator != (const zfix fx, const float v);
	inline friend int32_t operator != (const float v, const zfix fx);
	inline friend int32_t operator != (const zfix fx, const double v);
	inline friend int32_t operator != (const double v, const zfix fx);
	
	inline friend int32_t operator <  (const zfix fx, const zfix fx2);
	inline friend int32_t operator <  (const zfix fx, const int32_t v);
	inline friend int32_t operator <  (const int32_t v, const zfix fx);
	inline friend int32_t operator <  (const zfix fx, const int32_t v);
	inline friend int32_t operator <  (const int32_t v, const zfix fx);
	inline friend int32_t operator <  (const zfix fx, const float v);
	inline friend int32_t operator <  (const float v, const zfix fx);
	inline friend int32_t operator <  (const zfix fx, const double v);
	inline friend int32_t operator <  (const double v, const zfix fx);
	
	inline friend int32_t operator >  (const zfix fx, const zfix fx2);
	inline friend int32_t operator >  (const zfix fx, const int32_t v);
	inline friend int32_t operator >  (const int32_t v, const zfix fx);
	inline friend int32_t operator >  (const zfix fx, const int32_t v);
	inline friend int32_t operator >  (const int32_t v, const zfix fx);
	inline friend int32_t operator >  (const zfix fx, const float v);
	inline friend int32_t operator >  (const float v, const zfix fx);
	inline friend int32_t operator >  (const zfix fx, const double v);
	inline friend int32_t operator >  (const double v, const zfix fx);
	
	inline friend int32_t operator <= (const zfix fx, const zfix fx2);
	inline friend int32_t operator <= (const zfix fx, const int32_t v);
	inline friend int32_t operator <= (const int32_t v, const zfix fx);
	inline friend int32_t operator <= (const zfix fx, const int32_t v);
	inline friend int32_t operator <= (const int32_t v, const zfix fx);
	inline friend int32_t operator <= (const zfix fx, const float v);
	inline friend int32_t operator <= (const float v, const zfix fx);
	inline friend int32_t operator <= (const zfix fx, const double v);
	inline friend int32_t operator <= (const double v, const zfix fx);
	
	inline friend int32_t operator >= (const zfix fx, const zfix fx2);
	inline friend int32_t operator >= (const zfix fx, const int32_t v);
	inline friend int32_t operator >= (const int32_t v, const zfix fx);
	inline friend int32_t operator >= (const zfix fx, const int32_t v);
	inline friend int32_t operator >= (const int32_t v, const zfix fx);
	inline friend int32_t operator >= (const zfix fx, const float v);
	inline friend int32_t operator >= (const float v, const zfix fx);
	inline friend int32_t operator >= (const zfix fx, const double v);
	inline friend int32_t operator >= (const double v, const zfix fx);
};


#include "zfix.inl"

#endif		  /* ifndef ZFIX_H */
