// Various utility functions for the compiler.

#ifndef ZSCRIPT_COMPILER_UTILS_H
#define ZSCRIPT_COMPILER_UTILS_H

#include <cassert>
#include <list>
#include <vector>

////////////////////////////////////////////////////////////////
// Simple std::optional (from C++17).

// Empty optional instance.
struct nullopt_t
{
    struct init{};
    nullopt_t(init) {}
};
const nullopt_t nullopt((nullopt_t::init()));

template <class Type>
class optional
{
public:
	// Construct empty optional. 
	optional() : has_value_(false) {}
	optional(nullopt_t) : has_value_(false) {}
	// Construct with value.
	optional(Type const& value) : has_value_(true)
	{
		new(&data) Type(value);
	}
	// Construct with value (eliminate double optional).
	optional(optional const& rhs) : has_value_(rhs.has_value_)
	{
		if (rhs.has_value_)
			new(&data) Type(*rhs);
	}

	~optional()
	{
		if (has_value_)
			reinterpret_cast<Type*>(&data)->~Type();
	}

	optional& operator=(nullopt_t)
	{
		reset();
		return *this;
	}
	optional& operator=(optional const& rhs)
	{
		if (has_value_ && !rhs.has_value_)
			reset();
		else if (!has_value_ && rhs.has_value_)
		{
			new(&data) Type(*rhs);
			has_value_ = true;
		}
		else if (has_value_ && rhs.has_value_)
			*reinterpret_cast<Type*>(&data) = *rhs;
		return *this;
	}

	Type const* operator->() const {
		return reinterpret_cast<Type const*>(&data);}
	Type* operator->() {
		return reinterpret_cast<Type*>(&data);}
	Type const& operator*() const {
		return *reinterpret_cast<Type const*>(&data);}
	Type& operator*() {
		return *reinterpret_cast<Type*>(&data);}

	bool has_value() const {return has_value_;}
	Type const& value() const
	{
		assert(has_value_);
		return *reinterpret_cast<Type const*>(&data);
	}
	Type& value()
	{
		assert(has_value_);
		return *reinterpret_cast<Type*>(&data);
	}

	template <typename U>
	Type const value_or(U const& v) const
	{
		return has_value_
			? *reinterpret_cast<Type const*>(&data)
			: *reinterpret_cast<Type const*>(&v);
	}

	template <typename U>
	Type value_or(U& v)
	{
		return has_value_
			? *reinterpret_cast<Type*>(&data)
			: *reinterpret_cast<Type*>(&v);
	}

	// Destroys the value if present.
	void reset()
	{
		if (has_value_) reinterpret_cast<Type>(data).~Type();
		has_value_ = false;
	}
	
private:
	bool has_value_;
	union {char data[1 + (sizeof(Type) - 1) / sizeof(char)];};

	// safe_bool idiom
private:
	typedef void (optional::*safe_bool)() const;
	void this_type_does_not_support_comparisons() const {}
public:
	operator safe_bool() const
    {
        return has_value_
			? &optional::this_type_does_not_support_comparisons
			: 0;
    }
};

////////////////////////////////////////////////////////////////
// Containers

// Append the contents of the second container to the first.
template <typename TargetContainer, typename SourceContainer>
void appendElements(TargetContainer& target, SourceContainer const& source)
{
	target.insert(target.end(), source.begin(), source.end());
}

// Delete all the elements in a container.
template <typename Container>
void deleteElements(Container const& container)
{
	for (typename Container::const_iterator it = container.begin();
	     it != container.end(); ++it)
		delete *it;
}

////////////////////////////////////////////////////////////////
// Maps

template <typename Value, typename Map>
std::vector<Value> getSeconds(Map const& map)
{
	std::vector<Value> seconds;
	for (typename Map::const_iterator it = map.begin();
	     it != map.end(); ++it)
		seconds.push_back(it->second);
	return seconds;
}

template <typename Map>
void deleteSeconds(Map const& map)
{
	for (typename Map::const_iterator it = map.begin(); it != map.end(); ++it)
		delete it->second;
}

template <typename Element, typename Map, typename Key>
optional<Element> find(Map const& map, Key const& key)
{
	typename Map::const_iterator it = map.find(key);
	if (it == map.end()) return nullopt;
	Element const& element = it->second;
	return element;
}
	
#endif
