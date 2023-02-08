#ifndef ZC_GUI_SIZE_H
#define ZC_GUI_SIZE_H

#include <cstddef>
#include <cstdint>

namespace GUI
{

/* A simple wrapper to force sizes into units. */
class Size
{
public:
    // These are templates to resolve annoying ambiguity issues.

    template<typename T>
    static inline Size em(T t)
    {
		return Size(static_cast<int32_t>(t * static_cast<T>(emSize())));
    }

    template<typename T>
	static inline constexpr Size pixels(T t) noexcept
	{
		return Size(static_cast<int32_t>(t));
	}

    /* Returns the size as the actual number of pixels. */
	inline constexpr int32_t resolve() const
	{
		return value;
	}
	
	inline constexpr operator int32_t() const noexcept
	{
		return value;
	}
	
	inline constexpr Size operator+(const int32_t v) const noexcept
	{
		return Size(value+v);
	}
	
	inline constexpr Size operator+(const Size& other) const noexcept
	{
		return Size(value+other.value);
	}
	
	inline constexpr Size operator*(const int32_t v) const noexcept
	{
		return Size(value*v);
	}
	
	inline constexpr Size operator*(const double v) const noexcept
	{
		return Size(int32_t(value*v));
	}
	
	inline constexpr Size operator*(const Size& other) const noexcept
	{
		return Size(value*other.value);
	}
	template<typename T>
	inline friend Size operator*(const T v, const Size s);
	template<typename T>
	inline friend Size operator*(const Size s, const T v);
	
	inline constexpr Size operator/(const int32_t v) const noexcept
	{
		return Size(value/v);
	}
	
	inline constexpr Size operator/(const double v) const noexcept
	{
		return Size(int32_t(value/v));
	}
	
	inline constexpr Size operator/(const Size& other) const noexcept
	{
		return Size(value/other.value);
	}
	inline friend Size operator/(const int32_t v, const Size s);
	
	inline constexpr Size operator-(const Size& other) const noexcept
	{
		return Size(value-other.value);
	}
	inline friend Size operator-(const Size s, const int32_t v);
	inline friend Size operator-(const int32_t v, const Size s);
	inline constexpr bool operator<(const Size& other) const noexcept
	{
		return value < other.value;
	}
	inline constexpr bool operator<(int32_t val) const noexcept
	{
		return value < val;
	}
	inline constexpr bool operator<=(const Size& other) const noexcept
	{
		return value <= other.value;
	}
	inline constexpr bool operator<=(int32_t val) const noexcept
	{
		return value <= val;
	}

	inline constexpr bool operator>(const Size& other) const noexcept
	{
		return value > other.value;
	}
	inline constexpr bool operator>(int32_t val) const noexcept
	{
		return value > val;
	}
	inline constexpr bool operator>=(const Size& other) const noexcept
	{
		return value >= other.value;
	}
	inline constexpr bool operator>=(int32_t val) const noexcept
	{
		return value >= val;
	}

	inline constexpr bool operator==(const Size& other) const noexcept
	{
		return value == other.value;
	}

	inline constexpr bool operator!=(const Size& other) const noexcept
	{
		return value != other.value;
	}

	inline Size& operator-=(Size const& other)
	{
		value -= other.value;
		return *this;
	}
	inline Size& operator-=(int32_t val)
	{
		value -= val;
		return *this;
	}
	inline Size& operator+=(Size const& other)
	{
		value += other.value;
		return *this;
	}
	inline Size& operator+=(int32_t val)
	{
		value += val;
		return *this;
	}

private:
	int32_t value;

	inline constexpr Size(int32_t raw) noexcept: value(raw) {}
    static int32_t emSize();
};

inline Size operator ""_em(unsigned long long int size)
{
	return Size::em(size);
}

inline Size operator ""_em(long double size)
{
	return Size::em(size);
}

inline constexpr Size operator ""_px(unsigned long long int size)
{
	return Size::pixels(size);
}

inline Size operator-(const Size s, const int32_t v)
{
	return Size(s.value - v);
}

inline Size operator-(const int32_t v, const Size s)
{
	return Size(v - s.value);
}

template<typename t>
inline Size operator*(const t v, const Size s)
{
	return Size(static_cast<int32_t>(v)*s.value);
}
template<typename t>
inline Size operator*(const Size s, const t v)
{
	return Size(static_cast<int32_t>(v)*s.value);
}

inline Size operator/(const int32_t v, const Size s)
{
	return Size(v/s.value);
}

}

#endif
