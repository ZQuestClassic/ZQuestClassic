

#pragma once

#include <math.h>


#define ZC_EPSILON		0.00001f
#define ZC_PI			3.141592653589f
#define ZC_TWOPI		6.28318530f
#define ZC_PI			3.141592653589f
#define ZC_PI_OVER_2	1.570796326794f

//todo: better clamp function
//#define ZC_CLAMP(v, min, max) if((v) < (min)) v = (min); else if((v) > (max)) v = (max)


struct Vector2i;
struct Vector2f;
struct Rect;


inline float Min(float a, float b) { return (a < b) ? a : b; }
inline float Max(float a, float b) { return (a > b) ? a : b; }
inline int32 Min(int32 a, int32 b) { return (a < b) ? a : b; }
inline int32 Max(int32 a, int32 b) { return (a > b) ? a : b; }

inline float Clamp(float x, float min, float max) { return (x < min ? min : (x > max ? max : x)); }
inline int32 Clamp(int32 x, int32 min, int32 max) { return (x < min ? min : (x > max ? max : x)); }
inline float Clamp01(float x) { return (x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x)); }

template <class T> inline
void Swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

inline bool IsPowerOfTwo(int32 value)
{
	return (value && (value & (value - 1)) == 0);
}

inline int32 SnapToGrid(int32 x, int32 grid)
{
	if(grid == 0)
		return x;

	return (x / grid) * grid;
}

inline float SnapToGrid(float x, float grid)
{
	if(grid == 0.f)
		return x;

	return floorf(x / grid) * grid;
}

inline float ToDegrees(float radianValue)
{
	return radianValue * (180.f / 3.141592653589f);
}

inline float ToRadians(float degreeValue)
{
	return degreeValue * (3.141592653589f / 180.f);
}

inline float DistanceSquared(float x1, float y1, float x2, float y2)
{
	float a = x1 - x2;
	float b = y1 - y2;
	return a * a + b * b;
}

inline float Distance(float x1, float y1, float x2, float y2)
{
	float a = x1 - x2;
	float b = y1 - y2;
	return sqrtf(float((a * a) + (b * b)));
}

inline float Lerp(float x, float y, float t)
{
	return x + (y - x) * t;
}

inline float SmoothStep(float x, float y, float t)
{
	t = (t * t) * (3.0f - (2.0f * t));
	return x + (y - x) * t;
}



struct Vector2i
{
	int32 x;
	int32 y;

	Vector2i() : x(0), y(0) {}
	Vector2i(int32 value) : x(value), y(value) {}
	Vector2i(int32 x, int32 y) : x(x), y(y) {}
	Vector2i(const Vector2f& value);

	Vector2i& operator = (int32 n) { x = n; y = n; return *this; }
	Vector2i& operator = (const Vector2f& value) { *this = Vector2i(value); return *this; }

	Vector2i operator - () const { return Negate(); }
	//Vector2i operator + () const { return Abs(); }

	Vector2i& operator ++ () { ++x; ++y; return *this; }
	Vector2i& operator -- () { --x; --y; return *this; }
	Vector2i  operator ++ (int32) { Vector2i tmp(*this); x++; y++; return tmp; }
	Vector2i  operator -- (int32) { Vector2i tmp(*this); x--; y--; return tmp; }

	Vector2i operator * (const Vector2i& p)	const { return Vector2i(x * p.x, y * p.y); }
	Vector2i operator / (const Vector2i& p)	const { return Vector2i(x / p.x, y / p.y); }
	Vector2i operator + (const Vector2i& p)	const { return Vector2i(x + p.x, y + p.y); }
	Vector2i operator - (const Vector2i& p)	const { return Vector2i(x - p.x, y - p.y); }
	Vector2i operator * (int32 value) const { return Vector2i(x * value, y * value); }
	Vector2i operator / (int32 value) const { return Vector2i(x / value, y / value); }
	Vector2i operator + (int32 value) const { return Vector2i(x + value, y + value); }
	Vector2i operator - (int32 value) const { return Vector2i(x - value, y - value); }

	inline friend Vector2i operator * (int32 value, const Vector2i& p) { return Vector2i(value * p.x, value * p.y); }
	inline friend Vector2i operator / (int32 value, const Vector2i& p) { return Vector2i(value / p.x, value / p.y); }
	inline friend Vector2i operator + (int32 value, const Vector2i& p) { return Vector2i(value + p.x, value + p.y); }
	inline friend Vector2i operator - (int32 value, const Vector2i& p) { return Vector2i(value - p.x, value - p.y); }

	Vector2i& operator *= (const Vector2i& p) { x *= p.x; y *= p.y; return *this; }
	Vector2i& operator /= (const Vector2i& p) { x /= p.x; y /= p.y; return *this; }
	Vector2i& operator += (const Vector2i& p) { x += p.x; y += p.y; return *this; }
	Vector2i& operator -= (const Vector2i& p) { x -= p.x; y -= p.y; return *this; }
	Vector2i& operator *= (int32 value) { x *= value; y *= value; return *this; }
	Vector2i& operator /= (int32 value) { x /= value; y /= value; return *this; }
	Vector2i& operator += (int32 value) { x += value; y += value; return *this; }
	Vector2i& operator -= (int32 value) { x -= value; y -= value; return *this; }

	bool operator ==(const Vector2i& p) const { return (x == p.x && y == p.y); }
	bool operator !=(const Vector2i& p) const { return (x != p.x && y != p.y); }

	Vector2i Abs() const { return Vector2i(abs(x), abs(y)); }
	Vector2i Negate() const { return Vector2i(-x, -y);	}

	void Clamp(const Vector2i& min, const Vector2i& max)
	{
		x = ::Clamp(x, min.x, max.x);
		y = ::Clamp(y, min.y, max.y);
	}

	Vector2i Clamped(const Vector2i& min, const Vector2i& max)
	{
		return Vector2i(::Clamp(x, min.x, max.x), ::Clamp(y, min.y, max.y));
	}

	static const Vector2i Zero;
	static const Vector2i One;
};


struct Vector2f
{
	float x, y;

	Vector2f() : x(0.0f), y(0.0f) {}
	Vector2f(float value) : x(value), y(value) {}
	Vector2f(float x, float y) : x(x), y(y) {}
	Vector2f(const Vector2i& value) : x(float(value.x)), y(float(value.y)) {}

	Vector2f& operator = (float value) { x = value; y = value; return *this; }
	Vector2f& operator = (const Vector2i& value) { *this = Vector2f(value); return *this; }

	Vector2f operator - () const { return Vector2f(-x, -y);}

	Vector2f& operator ++ () { ++x; ++y; return *this; }
	Vector2f& operator -- () { --x; --y; return *this; }
	Vector2f operator ++ (int) { Vector2f tmp(*this); x++; y++; return tmp; }
	Vector2f operator -- (int) { Vector2f tmp(*this); x--; y--; return tmp; }

	Vector2f operator * (const Vector2f& v) const { return Vector2f(x * v.x, y * v.y); }
	Vector2f operator / (const Vector2f& v) const { return Vector2f(x / v.x, y / v.y); }
	Vector2f operator + (const Vector2f& v) const { return Vector2f(x + v.x, y + v.y); }
	Vector2f operator - (const Vector2f& v) const { return Vector2f(x - v.x, y - v.y); }
	Vector2f operator * (float value) const { return Vector2f(x * value, y * value); }
	Vector2f operator / (float value) const { return Vector2f(x / value, y / value); }
	Vector2f operator + (float value) const { return Vector2f(x + value, y + value); }
	Vector2f operator - (float value) const { return Vector2f(x - value, y - value); }

	friend Vector2f operator * (float value, const Vector2f& v) { return Vector2f(value * v.x, value * v.y); }
	friend Vector2f operator / (float value, const Vector2f& v) { return Vector2f(value / v.x, value / v.y); }
	friend Vector2f operator + (float value, const Vector2f& v) { return Vector2f(value + v.x, value + v.y); }
	friend Vector2f operator - (float value, const Vector2f& v) { return Vector2f(value - v.x, value - v.y); }

	Vector2f& operator *= (const Vector2f& v) { x *= v.x; y *= v.y; return *this; }
	Vector2f& operator /= (const Vector2f& v) { x /= v.x; y /= v.y; return *this; }
	Vector2f& operator += (const Vector2f& v) { x += v.x; y += v.y; return *this; }
	Vector2f& operator -= (const Vector2f& v) { x -= v.x; y -= v.y; return *this; }
	Vector2f& operator *= (float value) { x *= value; y *= value; return *this; }
	Vector2f& operator /= (float value) { x /= value; y /= value; return *this; }
	Vector2f& operator += (float value) { x += value; y += value; return *this; }
	Vector2f& operator -= (float value) { x -= value; y -= value; return *this; }

	bool operator == (const Vector2f& v) const { return x == v.x && y == v.y; } //fuzzy equals...
	bool operator != (const Vector2f& v) const { return !(*this == v); }

	//bool Equals(const Vector2& v, float epsilon = Math::Epsilon) const;

	static const Vector2f Zero;
	static const Vector2f One;

	float Dot(const Vector2f& v) const { return x * v.x + y * v.y; }
	float Cross(const Vector2f& v) const { return x * v.y - y * v.x; }
	float LengthSquared() const { return x * x + y * y; }
	float Length() const { return sqrtf(x * x + y * y); }
	float DistanceSquared(const Vector2f& v) const { return (*this - v).LengthSquared(); }
	float Distance(const Vector2f& v) const { return (*this - v).Length();}

	float Angle() const { return atan2f(y, x); }
	float Angle(const Vector2f& to) const
	{
		return atan2f(to.y - y, to.x - x);
	}

	//Vector2 Polar(const Vector2& dist, float theta) const;

	Vector2f Direction(const Vector2f& to) const
	{
		const Vector2f distance = *this - to;
		return distance / distance.Length();
	}

	Vector2f Rotated(float a) const;
	Vector2f Rotated(float a, const Vector2f& origin) const;
	void Rotate(float a);
	void Rotate(float a, const Vector2f& origin);

	float MinElement() const { return (x < y ? x : y); }
	float MaxElement() const { return (x > y ? x : y); }

	Vector2f Abs() const { return Vector2f(fabsf(x), fabsf(y)); }
	Vector2f Ceil() const { return Vector2f(ceilf(x), ceilf(y)); }
	Vector2f Floor() const { return Vector2f(floorf(x), floorf(y)); }
	Vector2f Round() const { return Vector2f(floorf(x + 0.5f), floorf(y + 0.5f)); }
	Vector2f Negate() const { return Vector2f(-x, -y); }

	Vector2f PerpendicularLeft() const { return Vector2f(y, -x); }
	Vector2f PerpendicularRight() const { return Vector2f(-y, x); }

	//Vector2f Unit() const { return Normalized(); }
	Vector2f Normalized() const
	{
		float lengthSquared = x * x + y * y;
		if(lengthSquared != 0.f)
		{
			float inv = 1.f / sqrtf(lengthSquared);
			return Vector2f(x * inv, y * inv);
		}

		return *this;
	}

	void Normalize() { *this = Normalized(); }

	Vector2f Reflect(const Vector2f& unit) const
	{
		return Vector2f(unit * (2.f * Dot(unit)) - *this);
	}

	Vector2f Refract(const Vector2f& unit, float eta) const;

	void Clamp(Vector2f min, Vector2f max)
	{
		x = ::Clamp(x, min.x, max.x);
		y = ::Clamp(y, min.y, max.y);
	}

	Vector2f Clamp01() const { return Vector2f(::Clamp01(x), ::Clamp01(y)); }
	Vector2f Clamped(const Vector2f& min, const Vector2f& max) const
	{
		return Vector2f(
			::Clamp(x, min.x, max.x),
			::Clamp(y, min.y, max.y));
	}

	Vector2f Lerp(Vector2f p2, float t) const
	{
		return Vector2f(
			x + (p2.x - x) * t,
			y + (p2.y - y) * t);
	}

	Vector2f SmoothStep(Vector2f v, float t) const
	{
		return Lerp(v, (t * t) * (3.0f - (2.0f * t)));
	}
};


inline float Dot(const Vector2f& a, const Vector2f& b) { return a.Dot(b); }
inline float Length(const Vector2f& a) { return a.Length(); }
inline float LengthSquared(const Vector2f& a) { return a.LengthSquared(); }
inline float Distance(const Vector2f& a, const Vector2f& b) { return a.Distance(b); }
inline float DistanceSquared(const Vector2f& a, const Vector2f& b) { return a.DistanceSquared(b); }
inline Vector2f Normalize(const Vector2f& a) { return a.Normalized(); }
inline Vector2f Abs(const Vector2f& a) { return a.Abs(); }
inline Vector2f Min(const Vector2f& a, const Vector2f& b) { return Vector2f(Min(a.x, b.x), Min(a.y, b.y)); }
inline Vector2f Max(const Vector2f& a, const Vector2f& b) { return Vector2f(Max(a.x, b.x), Max(a.y, b.y)); }
inline Vector2f Clamp(const Vector2f& a, float min, float max) { return a.Clamped(min, max); }
inline Vector2f Clamp(const Vector2f& a, const Vector2f& min, const Vector2f& max) { return a.Clamped(min, max); }
inline Vector2f Clamp01(const Vector2f& a) { return a.Clamp01(); }
inline Vector2f Lerp(const Vector2f& a, const Vector2f& b, float t) { return a.Lerp(b, t); }
inline Vector2f SmoothStep(const Vector2f& a, const Vector2f& b, float t) { return a.SmoothStep(b, t); }



struct Rect
{
	union
	{
		struct { int32 x, y, width, height; };
		struct { Vector2i position, size; };
	};

	Rect() : position(), size() {}
	Rect(const Vector2i& position, const Vector2i& size) : position(position), size(size) {}
	Rect(int32 x, int32 y, int32 width, int32 height) : position(x, y), size(width, height) {}

	int32& operator [](int32 i) { return *(&x + i); }
	int32 operator [](int32 i) const { return *(&x + i); }

	bool operator ==(const Rect& r) const { return (position == r.position && size == r.size); }
	bool operator !=(const Rect& r) const { return (position != r.position || size != r.size); }

	Rect operator + (const Vector2i &p) const { return Rect(position + p, size); }
	Rect& operator += (const Vector2i &p) { position += p; return *this; }

	int32 Left() const { return x; }
	int32 Right() const { return x + width; }
	int32 Top() const { return y; }
	int32 Bottom() const { return y + height; }

	Vector2i TopLeft() const { return position; }
	Vector2i TopRight() const { return Vector2i(x + width, y); }
	Vector2i BottomLeft() const { return Vector2i(x, y + height); }
	Vector2i BottomRight() const { return position + size; }
	Vector2i Center() const { return position + (size / 2); }

	//void Translate(const Vector2i& value) { position += value; }

	int32 Area() const { return width * height; }
	bool Empty() const { return width == 0 && height == 0; }

	bool Intersects(const Rect& r) const
	{
		return(r.x < x + width && x < r.x + r.width && r.y < y + height && y < r.y + r.height);
	}

	bool Contains(const Vector2i& p) const
	{
		return (x <= p.x && x + width > p.x && y <= p.y && y + height > p.y);
	}

	bool Contains(const Rect& r) const
	{
		return (r.x >= x && r.y >= y && r.x + r.width <= x + width && r.y + r.height <= y + height);
	}

	void Merge(const Rect& rect)
	{
		if(rect.position.x < position.x) position.x = rect.position.x;
		if(rect.position.y < position.y) position.y = rect.position.y;

		int xoff = rect.Right() - Right();
		int yoff = rect.Bottom() - Bottom();
		if(xoff > 0) width += xoff;
		if(yoff > 0) height += yoff;
	}

	void Clamp(const Rect& boundingRect)
	{
		::Clamp(x, boundingRect.x, boundingRect.width);
		::Clamp(y, boundingRect.y, boundingRect.height);
		::Clamp(width, boundingRect.x, boundingRect.width);
		::Clamp(height, boundingRect.y, boundingRect.height);
	}

	static const Rect Zero;
	static const Rect One;

};





inline Vector2i::Vector2i(const Vector2f& value)
	: x((int32)value.x), y((int32)value.y)
{
}


inline Vector2f RotatePoint(const Vector2f& p, const Vector2f& rot)
{
	return Vector2f((rot.x * p.x) - (rot.y * p.y),
		(rot.y * p.x) + (rot.x * p.y));
}

inline Vector2f RotatePoint(const Vector2f& p, const Vector2f& rot, const Vector2f& center)
{
	const Vector2f v = (p - center);
	return Vector2f((rot.x * v.x) - (rot.y * v.y) + center.x, (rot.y * v.x) + (rot.x * v.y) + center.y);
}

inline Vector2f ScalePoint(const Vector2f& p, const Vector2f& scale, const Vector2f& center)
{
	return (scale * (p - center)) + center;
}

inline Vector2f RotateScalePoint(const Vector2f& p, const Vector2f& rot, const Vector2f& scale, const Vector2f& center)
{
	const Vector2f v = (scale * (p - center));
	return Vector2f((rot.x * v.x) - (rot.y * v.y) + center.x, (rot.y * v.x) + (rot.x * v.y) + center.y);
}

inline float WrapValue(float x, float min, float max, float amount)
{
	if(x < min) x += amount;
	else if(x > max) x -= amount;
	return x;
}

inline float WrapAngle(float radians) //Wraps radian value towards -PI and PI.
{
	return WrapValue(radians, -ZC_PI, ZC_PI, ZC_TWOPI);
}

inline float WrapDegrees(float degrees) //Wraps degree value towards 0 and 360.
{
	return WrapValue(degrees, 0.0f, 360.0f, 360.0f);
}

inline float ClampAngle(float radians) // Clamps radian value between -PI and PI
{
	radians = fmodf(radians, ZC_TWOPI);
	return WrapAngle(radians);
}

inline float ClampDegrees(float degrees) // Clamps degree value between 0 and 360
{
	return fmodf(degrees, 360.f);
}

inline float FindDeltaAngle(float a, float b)
{
	return WrapAngle(b - a);
}

float DistancetoLineSegment(const Vector2f& a, const Vector2f& b, const Vector2f& point);






