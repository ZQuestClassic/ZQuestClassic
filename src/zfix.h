/*
 * Custom ZFix: because Allegro's zfix is not precise
 */


#ifndef ZFIX_H
#define ZFIX_H

typedef int32_t zslong;

class zfix
{
public:
	int16_t ipart, dpart;
	
	int getInt()
	{
		if(ipart < 0)
			return dpart >= 5000 ? ipart-1 : ipart;
		return dpart >= 5000 ? ipart+1 : ipart;
	}
	float getFloat()
	{
		if(ipart < 0)
			return ipart - (dpart / 10000.0);
		return ipart + (dpart / 10000.0);
	}
private:
	void update()
	{
		if(dpart < 0) dpart = -dpart;
		while(dpart > 9999)
		{
			if(ipart < 0) --ipart;
			else ++ipart;
			dpart -= 10000;
		}
	}
public:
	
	zfix() : ipart(0), dpart(0)											{}
	zfix(const zfix &x) : ipart(x.ipart), dpart(x.dpart)					{}
	explicit zfix(const int x) ipart(x), dpart(0)						{}
	explicit zfix(const long x) : ipart(x), dpart(0)		  			{}
	explicit zfix(const unsigned int x) : ipart(x), dpart(0)			{}
	explicit zfix(const unsigned long x) : ipart(x), dpart(0)			{}
	explicit zfix(const float x) : ipart(floor(x)), dpart(abs((x%1)*10000))	{}
	explicit zfix(const double x) : ipart(floor(x), dpart(abs((x%1)*10000))	{}
	explicit zfix(const int ip, const int dp)
	{
		ipart = ip;
		dpart = abs(dp);
		while(dpart > 9999) dpart /= 10; //cap to 4-digit
	}

	operator int() const						{ return getInt(); }
	operator long() const						{ return getInt(); }
	operator unsigned int() const				{ return getInt(); }
	operator unsigned long() const				{ return getInt(); }
	operator float() const						{ return getFloat(); }
	operator double() const						{ return getFloat(); }

	zfix& operator = (const zfix &x)			{ ipart = x.ipart; dpart = x.dpart; return *this; }
	zfix& operator = (const int x)				{ ipart = x; dpart = 0; return *this; }
	zfix& operator = (const long x)				{ ipart = x; dpart = 0; return *this; }
	zfix& operator = (const unsigned int x)		{ ipart = x; dpart = 0; return *this; }
	zfix& operator = (const unsigned long x)	{ ipart = x; dpart = 0; return *this; }
	zfix& operator = (const float x)			{ ipart = x; dpart = abs((x%1)*10000); return *this; }
	zfix& operator = (const double x)			{ ipart = x; dpart = abs((x%1)*10000); return *this; }

	zfix& operator +=  (const zfix x)
	{
		ipart += x.ipart;
		dpart += x.dpart;
		update();
	}
	zfix& operator +=  (const int x)			{ ipart += x; return *this; }
	zfix& operator +=  (const long x)			{ ipart += x; return *this; }
	zfix& operator +=  (const float x)
	{
		ipart += floor(x);
		dpart += (x%1)*10000;
		update();
	}
	zfix& operator +=  (const double x)
	{
		ipart += floor(x);
		dpart += (x%1)*10000;
		update();
	}

	zfix& operator -=  (const zfix x)
	{
		ipart -= x.ipart;
		dpart -= x.dpart;
		if(dpart > 9999)
		{
			--ipart;
			dpart -= 10000;
		}
	}
	zfix& operator -=  (const int x)			{ ipart -= x; return *this; }
	zfix& operator -=  (const long x)			{ ipart -= x; return *this; }
	zfix& operator -=  (const float x)
	{
		ipart -= floor(x);
		dpart -= (x%1)*10000;
		update();
	}
	zfix& operator -=  (const double x)
	{
		ipart -= floor(x);
		dpart -= (x%1)*10000;
		update();
	}
	/*
	zfix& operator *=  (const zfix x)		   { v = fixmul(v, x.v);		 return *this; }
	zfix& operator *=  (const int x)		   { v *= x;					 return *this; }
	zfix& operator *=  (const long x)		  { v *= x;					 return *this; }
	zfix& operator *=  (const float x)		 { v = ftofix(getFloat() * x);  return *this; }
	zfix& operator *=  (const double x)		{ v = ftofix(getFloat() * x);  return *this; }

	zfix& operator /=  (const zfix x)		   { v = fixdiv(v, x.v);		 return *this; }
	zfix& operator /=  (const int x)		   { v /= x;					 return *this; }
	zfix& operator /=  (const long x)		  { v /= x;					 return *this; }
	zfix& operator /=  (const float x)		 { v = ftofix(getFloat() / x);  return *this; }
	zfix& operator /=  (const double x)		{ v = ftofix(getFloat() / x);  return *this; }

	zfix& operator <<= (const int x)		   { v <<= x;		   return *this; }
	zfix& operator >>= (const int x)		   { v >>= x;		   return *this; }*/

	zfix& operator ++ ()						{ ++ipart; return *this; }
	zfix& operator -- ()						{ --ipart; return *this; }
	
	zfix operator ++ (int)						{ zfix t(ipart,dpart); ++ipart; return t; }
	zfix operator -- (int)						{ zfix t(ipart,dpart); --ipart; return t; }

	zfix operator - () const				{ ipart = -ipart; return *this; }

	inline friend zfix operator +  (const zfix x, const zfix y);
	inline friend zfix operator +  (const zfix x, const int y);
	inline friend zfix operator +  (const int x, const zfix y);
	inline friend zfix operator +  (const zfix x, const long y);
	inline friend zfix operator +  (const long x, const zfix y);
	inline friend zfix operator +  (const zfix x, const float y);
	inline friend zfix operator +  (const float x, const zfix y);
	inline friend zfix operator +  (const zfix x, const double y);
	inline friend zfix operator +  (const double x, const zfix y);

	inline friend zfix operator -  (const zfix x, const zfix y);
	inline friend zfix operator -  (const zfix x, const int y);
	inline friend zfix operator -  (const int x, const zfix y);
	inline friend zfix operator -  (const zfix x, const long y);
	inline friend zfix operator -  (const long x, const zfix y);
	inline friend zfix operator -  (const zfix x, const float y);
	inline friend zfix operator -  (const float x, const zfix y);
	inline friend zfix operator -  (const zfix x, const double y);
	inline friend zfix operator -  (const double x, const zfix y);
	/*
	inline friend zfix operator *  (const zfix x, const zfix y);
	inline friend zfix operator *  (const zfix x, const int y);
	inline friend zfix operator *  (const int x, const zfix y);
	inline friend zfix operator *  (const zfix x, const long y);
	inline friend zfix operator *  (const long x, const zfix y);
	inline friend zfix operator *  (const zfix x, const float y);
	inline friend zfix operator *  (const float x, const zfix y);
	inline friend zfix operator *  (const zfix x, const double y);
	inline friend zfix operator *  (const double x, const zfix y);

	inline friend zfix operator /  (const zfix x, const zfix y);
	inline friend zfix operator /  (const zfix x, const int y);
	inline friend zfix operator /  (const int x, const zfix y);
	inline friend zfix operator /  (const zfix x, const long y);
	inline friend zfix operator /  (const long x, const zfix y);
	inline friend zfix operator /  (const zfix x, const float y);
	inline friend zfix operator /  (const float x, const zfix y);
	inline friend zfix operator /  (const zfix x, const double y);
	inline friend zfix operator /  (const double x, const zfix y);

	inline friend zfix operator << (const zfix x, const int y);
	inline friend zfix operator >> (const zfix x, const int y);*/

	inline friend int operator == (const zfix x, const zfix y);
	inline friend int operator == (const zfix x, const int y);
	inline friend int operator == (const int x, const zfix y);
	inline friend int operator == (const zfix x, const long y);
	inline friend int operator == (const long x, const zfix y);
	inline friend int operator == (const zfix x, const float y);
	inline friend int operator == (const float x, const zfix y);
	inline friend int operator == (const zfix x, const double y);
	inline friend int operator == (const double x, const zfix y);

	inline friend int operator != (const zfix x, const zfix y);
	inline friend int operator != (const zfix x, const int y);
	inline friend int operator != (const int x, const zfix y);
	inline friend int operator != (const zfix x, const long y);
	inline friend int operator != (const long x, const zfix y);
	inline friend int operator != (const zfix x, const float y);
	inline friend int operator != (const float x, const zfix y);
	inline friend int operator != (const zfix x, const double y);
	inline friend int operator != (const double x, const zfix y);
	/*
	inline friend int operator <  (const zfix x, const zfix y);
	inline friend int operator <  (const zfix x, const int y);
	inline friend int operator <  (const int x, const zfix y);
	inline friend int operator <  (const zfix x, const long y);
	inline friend int operator <  (const long x, const zfix y);
	inline friend int operator <  (const zfix x, const float y);
	inline friend int operator <  (const float x, const zfix y);
	inline friend int operator <  (const zfix x, const double y);
	inline friend int operator <  (const double x, const zfix y);

	inline friend int operator >  (const zfix x, const zfix y);
	inline friend int operator >  (const zfix x, const int y);
	inline friend int operator >  (const int x, const zfix y);
	inline friend int operator >  (const zfix x, const long y);
	inline friend int operator >  (const long x, const zfix y);
	inline friend int operator >  (const zfix x, const float y);
	inline friend int operator >  (const float x, const zfix y);
	inline friend int operator >  (const zfix x, const double y);
	inline friend int operator >  (const double x, const zfix y);

	inline friend int operator <= (const zfix x, const zfix y);
	inline friend int operator <= (const zfix x, const int y);
	inline friend int operator <= (const int x, const zfix y);
	inline friend int operator <= (const zfix x, const long y);
	inline friend int operator <= (const long x, const zfix y);
	inline friend int operator <= (const zfix x, const float y);
	inline friend int operator <= (const float x, const zfix y);
	inline friend int operator <= (const zfix x, const double y);
	inline friend int operator <= (const double x, const zfix y);

	inline friend int operator >= (const zfix x, const zfix y);
	inline friend int operator >= (const zfix x, const int y);
	inline friend int operator >= (const int x, const zfix y);
	inline friend int operator >= (const zfix x, const long y);
	inline friend int operator >= (const long x, const zfix y);
	inline friend int operator >= (const zfix x, const float y);
	inline friend int operator >= (const float x, const zfix y);
	inline friend int operator >= (const zfix x, const double y);
	inline friend int operator >= (const double x, const zfix y);*/
};

#endif		  /* ifndef ZFIX_H */
