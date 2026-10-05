#ifndef BASE_LAZY_DEFAULT_H_
#define BASE_LAZY_DEFAULT_H_

#include <memory>

// Holds a value of T that reads as `*Default` until it is first written to, and only then
// allocates storage of its own. For large members of large arrays (e.g. every combo) that
// almost always keep their default value.
//
// Use `get()` to read and `mut()` to write. After filling the value in bulk (e.g. reading a
// quest), call `release_if_default()` to give back storage that ended up equal to the default.
template <typename T, const T* Default>
class lazy_default
{
public:
	lazy_default() = default;
	lazy_default(const lazy_default& other)
		: value(other.value ? std::make_unique<T>(*other.value) : nullptr) {}
	lazy_default(lazy_default&&) noexcept = default;

	lazy_default& operator=(const lazy_default& other)
	{
		if (this != &other)
			value = other.value ? std::make_unique<T>(*other.value) : nullptr;
		return *this;
	}
	lazy_default& operator=(lazy_default&&) noexcept = default;

	// Assigning the default value releases any storage.
	lazy_default& operator=(const T& v)
	{
		if (v == *Default)
			value.reset();
		else if (value)
			*value = v;
		else
			value = std::make_unique<T>(v);
		return *this;
	}

	const T& get() const
	{
		return value ? *value : *Default;
	}

	T& mut()
	{
		if (!value)
			value = std::make_unique<T>(*Default);
		return *value;
	}

	void reset()
	{
		value.reset();
	}

	void release_if_default()
	{
		if (value && *value == *Default)
			value.reset();
	}

	bool operator==(const lazy_default& other) const
	{
		return get() == other.get();
	}

private:
	std::unique_ptr<T> value;
};

#endif
