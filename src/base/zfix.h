/*
 * Custom ZFix: because Allegro's zfix is not precise
 */
#ifndef ZFIX_H_
#define ZFIX_H_
#include <cstdint>
#include <math.h>
#include <limits>
#include <assert.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <fmt/format.h>

#define FIX_NAN 0

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
class zfix_round;
inline zfix zslongToFix(ZLong val);
inline ZLong toZLong(float val);
inline ZLong toZLong(double val);
inline ZLong toZLong(int32_t val);
inline ZLong toZLong(int32_t val);
inline zfix floor(zfix const& fx);
inline zfix abs(zfix const& fx);
inline zfix dist(zfix const& x1, zfix const& y1, zfix const& x2, zfix const& y2);

int vbound(int x,int low,int high);

enum roundType
{
	ROUND_NEAREST,
	ROUND_DOWN,
	ROUND_UP,
	ROUND_TO_0,
	ROUND_AWAY_0,
	
	ROUND_FLOOR = ROUND_DOWN,
	ROUND_CEIL = ROUND_UP,
	ROUND_TRUNC = ROUND_TO_0,
};
#define RoundDir(v) (v < 0 ? ROUND_FLOOR : ROUND_CEIL)

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
	zfix& doClamp(int low, int high)
	{
		int int_val = getInt();
		if (int_val > high) val = high * 10000L;
		else if (int_val < low) val = low * 10000L;
		return *this;
	}
	
	int32_t getInt() const
	{
		return val/10000L + vbound((val%10000L)/5000L, (val < 0 ? -1 : 0),(val<0 ? 0 : 1));
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
	
	virtual int32_t getRound() const
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
	virtual zfix& doRound()
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
		if(low > high)
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
	zfix& doMax(zfix min)
	{
		if (val < min.val)
			val = min.val;
		return *this;
	}
	zfix& doMin(zfix max)
	{
		if (val > max.val)
			val = max.val;
		return *this;
	}
	
	zfix& do_round(roundType rt);
	int32_t round(roundType rt) const
	{
		return copy().do_round(rt).getInt();
	}
	zfix_round rnd(roundType rt) const;
	
	std::string str() const
	{
		return fmt::format("{}{}.{:04}",val < 0 ? "-" : "",abs(val/10000),abs(val%10000));
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
	zfix& operator %=  (const zfix fx)	{
		if(fx.val == 0) val = toZLong(FIX_NAN);
		else val %= fx.val;
		return *this;}
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
	
	inline friend zfix operator %  (const zfix fx, const zfix fx2);
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
	
	inline friend zfix wrap_rad(zfix v);
	inline friend zfix wrap_deg(zfix v);
};

class zfix_round : public zfix
{
public:
	roundType rt;
	zfix_round() : zfix(0), rt(ROUND_TRUNC) {}
	zfix_round(int32_t v, roundType rt = ROUND_TRUNC) : zfix(v), rt(rt) {}
	zfix_round(zfix v, roundType rt = ROUND_TRUNC) : zfix(v), rt(rt) {}
	virtual int32_t getRound() const;
	virtual zfix& doRound();
};

//Implement fmt::format parsing for zfix type
template <> struct fmt::formatter<zfix>
{
	constexpr auto parse(fmt::format_parse_context& ctx) -> fmt::format_parse_context::iterator
	{
		return ctx.begin();
	}
	auto format(const zfix& number, fmt::format_context& ctx) const -> fmt::format_context::iterator
	{
		auto val = number.getZLong();
		return fmt::format_to(ctx.out(), "{0}.{1:04}", val/10000, abs(val%10000));
	}
};

inline zfix atozfix(char const* val)
{
	int ipart = 0;
	int dpart = 0;
	int poses = 4;
	int q = 0;
	bool neg = false;
	if((neg = (val[0] == '-')))
		++q;
	while(val[q] >= '0' && val[q] <= '9')
		ipart = (ipart*10)+(val[q++] - '0');
	if(val[q++] == '.')
	{
		while(val[q] >= '0' && val[q] <= '9' && poses)
		{
			dpart = (dpart*10) + (val[q]-'0');
			--poses;
			++q;
		}
		while(poses-- > 0)
			dpart *= 10;
	}
	if(neg)
		return -zfix(ipart,dpart);
	return zfix(ipart,dpart);
}
inline zfix operator ""_zf(unsigned long long int val)
{
	return zfix(int32_t(val));
}
inline zfix operator ""_zf(const char* val)
{
	return atozfix(val);
}
inline int32_t operator ""_zl(unsigned long long int val)
{
	return val*10000;
}
inline int32_t operator ""_zl(const char* val)
{
	return atozfix(val).getZLong();
}

static const zfix PI_ZF(3.1416_zf);
static const zfix PI2_ZF(2*PI_ZF);

inline ZLong toZLong(float val)
{
	return ZLong(val * 10000);
}
inline ZLong toZLong(double val)
{
	return ZLong(val * 10000);
}
inline ZLong toZLong(int32_t val)
{
	return ZLong(val * 10000);
}
inline zfix zslongToFix(ZLong val)
{
	zfix t;
	t.val = val;
	return t;
}
inline zfix floor(zfix const& fx)
{
	zfix t(fx);
	t.doFloor();
	return t;
}
inline zfix abs(zfix const& fx)
{
	zfix t(fx);
	t.doAbs();
	return t;
}
inline zfix sqrt(zfix const& fx) //rounding error... unavoidable?
{
	return zfix(sqrt(fx.getFloat()));
}

inline zfix dist(zfix const& x1, zfix const& y1, zfix const& x2, zfix const& y2)
{
	double xd = x2-x1;
	double yd = y2-y1;
	double d = xd*xd + yd*yd;
	return sqrt(d);
}

inline zfix operator +  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t += fx2;
	return t;
}
inline zfix operator +  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const int32_t v, const zfix fx)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const float v, const zfix fx)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const double v, const zfix fx)
{
	zfix t = fx.copy();
	t += v;
	return t;
}

inline zfix operator -  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t -= fx2;
	return t;
}
inline zfix operator -  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t -= v;
	return t;
}
inline zfix operator -  (const int32_t v, const zfix fx)
{
	zfix t(v);
	t -= fx;
	return t;
}
inline zfix operator -  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t -= v;
	return t;
}
inline zfix operator -  (const float v, const zfix fx)
{
	zfix t(v);
	t -= fx;
	return t;
}
inline zfix operator -  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t -= v;
	return t;
}
inline zfix operator -  (const double v, const zfix fx)
{
	zfix t(v);
	t -= fx;
	return t;
}

inline zfix operator *  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t *= fx2;
	return t;
}

inline zfix operator *  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const int32_t v, const zfix fx)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const float v, const zfix fx)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const double v, const zfix fx)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator %  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t %= fx2;
	return t;
}

inline zfix operator /  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t /= fx2;
	return t;
}

inline zfix operator /  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t /= v;
	return t;
}

inline zfix operator /  (const int32_t v, const zfix fx)
{
	zfix t(v);
	t /= fx;
	return t;
}

inline zfix operator /  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t /= v;
	return t;
}

inline zfix operator /  (const float v, const zfix fx)
{
	zfix t(v);
	t /= fx;
	return t;
}

inline zfix operator /  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t /= v;
	return t;
}

inline zfix operator /  (const double v, const zfix fx)
{
	zfix t(v);
	t /= fx;
	return t;
}

inline zfix operator << (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t <<= v;
	return t;
}

inline zfix operator >> (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t >>= v;
	return t;
}

inline int32_t operator == (const zfix fx, const zfix fx2)
{
	return fx.val == fx2.val;
}
inline int32_t operator == (const zfix fx, const int32_t v)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const int32_t v, const zfix fx)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const zfix fx, const float v)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const float v, const zfix fx)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const zfix fx, const double v)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const double v, const zfix fx)
{
	return fx.val == toZLong(v);
}

inline int32_t operator != (const zfix fx, const zfix fx2)
{
	return fx.val != fx2.val;
}
inline int32_t operator != (const zfix fx, const int32_t v)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const int32_t v, const zfix fx)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const zfix fx, const float v)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const float v, const zfix fx)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const zfix fx, const double v)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const double v, const zfix fx)
{
	return fx.val != toZLong(v);
}

inline int32_t operator <  (const zfix fx, const zfix fx2)
{
	return fx.val < fx2.val;
}
inline int32_t operator <  (const zfix fx, const int32_t v)
{
	return fx.val < toZLong(v);
}
inline int32_t operator <  (const int32_t v, const zfix fx)
{
	return toZLong(v) < fx.val;
}
inline int32_t operator <  (const zfix fx, const float v)
{
	return fx.val < toZLong(v);
}
inline int32_t operator <  (const float v, const zfix fx)
{
	return toZLong(v) < fx.val;
}
inline int32_t operator <  (const zfix fx, const double v)
{
	return fx.val < toZLong(v);
}
inline int32_t operator <  (const double v, const zfix fx)
{
	return toZLong(v) < fx.val;
}

inline int32_t operator >  (const zfix fx, const zfix fx2)
{
	return fx.val > fx2.val;
}
inline int32_t operator >  (const zfix fx, const int32_t v)
{
	return fx.val > toZLong(v);
}
inline int32_t operator >  (const int32_t v, const zfix fx)
{
	return toZLong(v) > fx.val;
}
inline int32_t operator >  (const zfix fx, const float v)
{
	return fx.val > toZLong(v);
}
inline int32_t operator >  (const float v, const zfix fx)
{
	return toZLong(v) > fx.val;
}
inline int32_t operator >  (const zfix fx, const double v)
{
	return fx.val > toZLong(v);
}
inline int32_t operator >  (const double v, const zfix fx)
{
	return toZLong(v) > fx.val;
}

inline int32_t operator <=  (const zfix fx, const zfix fx2)
{
	return fx.val <= fx2.val;
}
inline int32_t operator <=  (const zfix fx, const int32_t v)
{
	return fx.val <= toZLong(v);
}
inline int32_t operator <=  (const int32_t v, const zfix fx)
{
	return toZLong(v) <= fx.val;
}
inline int32_t operator <=  (const zfix fx, const float v)
{
	return fx.val <= toZLong(v);
}
inline int32_t operator <=  (const float v, const zfix fx)
{
	return toZLong(v) <= fx.val;
}
inline int32_t operator <=  (const zfix fx, const double v)
{
	return fx.val <= toZLong(v);
}
inline int32_t operator <=  (const double v, const zfix fx)
{
	return toZLong(v) <= fx.val;
}

inline int32_t operator >=  (const zfix fx, const zfix fx2)
{
	return fx.val >= fx2.val;
}
inline int32_t operator >=  (const zfix fx, const int32_t v)
{
	return fx.val >= toZLong(v);
}
inline int32_t operator >=  (const int32_t v, const zfix fx)
{
	return toZLong(v) >= fx.val;
}
inline int32_t operator >=  (const zfix fx, const float v)
{
	return fx.val >= toZLong(v);
}
inline int32_t operator >=  (const float v, const zfix fx)
{
	return toZLong(v) >= fx.val;
}
inline int32_t operator >=  (const zfix fx, const double v)
{
	return fx.val >= toZLong(v);
}
inline int32_t operator >=  (const double v, const zfix fx)
{
	return toZLong(v) >= fx.val;
}


inline zfix wrap_rad(zfix v)
{
	int half = PI_ZF.getZLong(), whole = half*2;
	if(v.val < 0)
		v.val = whole - ((-v.val)%whole);
	else v.val %= whole;
	if(v.val > half)
		v.val -= whole;
	return v;
}
inline zfix wrap_deg(zfix v)
{
	int half = 180*10000, whole = half*2;
	if(v.val < 0)
		v.val = whole - ((-v.val)%whole);
	else v.val %= whole;
	if(v.val > half)
		v.val -= whole;
	return v;
}
inline int wrap_zslong_rad(int v)
{
	int half = PI_ZF.getZLong(), whole = half*2;
	if(v < 0)
		v = whole - ((-v)%whole);
	else v %= whole;
	if(v > half)
		v -= whole;
	return v;
}
inline int wrap_zslong_deg(int v)
{
	int half = 180*10000, whole = half*2;
	if(v < 0)
		v = whole - ((-v)%whole);
	else v %= whole;
	if(v > half)
		v -= whole;
	return v;
}

#endif		  /* ifndef ZFIX_H */
