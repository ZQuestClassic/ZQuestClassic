#ifndef ZC_GUI_SIZE_H
#define ZC_GUI_SIZE_H

#include <cstddef>

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
		return Size(static_cast<int>(t * static_cast<T>(emSize())));
    }

    template<typename T>
    static inline Size largePixels(T t) noexcept
    {
        return Size::sized(static_cast<int>(t));
    }

    template<typename T>
    static inline Size smallPixels(T t) noexcept
    {
        return Size::sized(static_cast<int>(t)*3/2);
    }

    template<typename T>
	static inline constexpr Size pixels(T t) noexcept
	{
		return Size(static_cast<int>(t));
	}

    /* Returns the size as the actual number of pixels. */
	inline constexpr int resolve() const
	{
		return value;
	}
	
	inline constexpr operator int() const noexcept
	{
		return value;
	}
	
	inline constexpr Size operator+(const int& other) const noexcept
	{
		return Size(value+other);
	}
	
	inline constexpr Size operator+(const Size& other) const noexcept
	{
		return Size(value+other.value);
	}

	inline constexpr bool operator<(const Size& other) const noexcept
	{
		return value < other.value;
	}

	inline constexpr bool operator>(const Size& other) const noexcept
	{
		return value > other.value;
	}

	inline constexpr bool operator==(const Size& other) const noexcept
	{
		return value == other.value;
	}

	inline constexpr bool operator!=(const Size& other) const noexcept
	{
		return value != other.value;
	}

private:
	int value;

	inline constexpr Size(int raw) noexcept: value(raw) {}
    static int emSize();
    static Size sized(int size) noexcept;
};

inline Size operator ""_em(unsigned long long size)
{
	return Size::em(size);
}

inline Size operator ""_em(long double size)
{
	return Size::em(size);
}

inline constexpr Size operator ""_px(unsigned long long size)
{
	return Size::pixels(size);
}

inline Size operator ""_lpx(unsigned long long size)
{
	return Size::largePixels(size);
}

inline Size operator ""_spx(unsigned long long size)
{
	return Size::smallPixels(size);
}

}

#endif
