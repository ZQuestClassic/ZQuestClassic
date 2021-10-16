// Various utility functions for the compiler.

#ifndef ZSCRIPT_COMPILER_UTILS_H
#define ZSCRIPT_COMPILER_UTILS_H

// prevent compiler errors
#ifdef ASTDINT_H
#undef char
#undef uint8_t
#undef int16_t
#undef uint16_t
#undef int32_t
#undef uint32_t
#undef intptr_t
#undef uintptr_t
#endif // ASTDINT_H

#ifdef int64_t
#undef int64_t
#endif // int64_t

#ifdef uint64_t
#undef uint64_t
#endif // uint64_t

#ifdef new
#undef new
#endif // new

#if (__cplusplus < 201703L)
    #include <boost/optional.hpp>
#else
    #include <optional>
    using std::nullopt_t, std::nullopt;
#endif
#include <boost/type_traits.hpp>

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "zc_malloc.h"

namespace ZScript
{
    class DataTypeCustom;
}

////////////////////////////////////////////////////////////////
// Strings

template <typename Type>
std::string to_string(Type val)
{
	std::ostringstream oss;
	oss << val;
	return oss.str();
}

int32_t const formatBufferSize = 4096;

/*
namespace ZScript
{
	// as sprintf, but for strings.
	int32_t const formatBufferSize = 4096;
	std::string formatStr(std::string const* format, ...);
}
*/

class XTableHelper
{
	public:
	std::string formatStr(std::string const* format, ...);

	private:
};


////////////////////////////////////////////////////////////////
// No Copy Mixin

// Mixin that disables the auto-generated copy constructor and assignment
// operator.
class NoCopy
{
public:
	NoCopy() {}
	~NoCopy() {}
private:
	NoCopy(NoCopy const&);
	NoCopy& operator=(NoCopy const&);
};

////////////////////////////////////////////////////////////////
// Safe Bool Idiom

// Mixin for the safe bool idiom.
// To use, inherit publically from SafeBool<Own Class> and implement
// bool safe_bool() const;
template <typename Self>
class SafeBool
{
protected:
	typedef void (SafeBool::*bool_type) () const;
	void this_type_does_not_support_comparisons() const {}

	SafeBool() {}
	SafeBool(SafeBool const&) {}
	SafeBool& operator=(SafeBool const&) {return *this;}
	~SafeBool() {}

public:
	operator bool_type() const
	{
		return static_cast<Self const*>(this)->safe_bool()
			? &SafeBool::this_type_does_not_support_comparisons
			: NULL;
	}
};

// Disable ==.
template <typename T, typename U>
bool operator==(SafeBool<T> const& lhs, SafeBool<U> const& rhs) {
	lhs.this_type_does_not_support_comparisons(); // compile error
	return false;
}

// Disable !=
template <typename T, typename U>
bool operator!=(SafeBool<T> const& lhs, SafeBool<U> const& rhs) {
    lhs.this_type_does_not_support_comparisons(); // compile error
    return false;
}


////////////////////////////////////////////////////////////////
// Simple std::optional (from C++17).
// Soley adapts existing code with boost::optional


#if (__cplusplus < 201703L)
// Empty optional instance.
typedef boost::none_t nullopt_t;
const nullopt_t nullopt(boost::none);

template<typename T>
class optional : public SafeBool<optional<T> >
{
public:
	typedef T value_type;

	// Construct empty optional.
	optional() : data_() {}
	optional(nullopt_t) : data_(nullopt) {}
	// Construct with value.
	optional(const T& value) : data_(value) {}
	// Construct with value (eliminate double optional).
	optional(const optional& rhs)
	{
		if (rhs.data_.has_value()) {
			data_.emplace(*rhs);
		}
	}

	~optional()
	{
		data_.reset();
	}

	optional& operator=(nullopt_t)
	{
		data_.reset();
		return *this;
	}
	optional& operator=(const optional& rhs)
	{
		data_.emplace(*rhs);
		return *this;
	}

	const T* operator->() const
	{
		assert(data_.has_value());
		return &data_.value();
	}
	T* operator->()
	{
		assert(data_.has_value());
		return &data_.value();
	}
	const T& operator*() const
	{
		assert(data_.has_value());
		return data_.value();
	}

	T& operator*()
	{
		assert(data_.has_value());
		return data_.value();
	}

	bool has_value() const { return data_.has_value(); }
	const T& value() const
	{
		assert(data_.has_value());
		return data_.value();
	}
	T& value()
	{
		assert(data_.has_value());
		return data_.value();
	}

	template<typename U>
	T value_or(const U& v) const
	{
		return data_.value_or(v);
	}

	template <typename U>
	T value_or(U& v)
	{
		return data_.value_or(v);
	}

	// Destroys the value if present.
	void reset()
	{
		data_.reset();
	}

	bool safe_bool() const { return data_.has_value(); }

private:
	boost::optional<T> data_;
};
#else
template<typename T>
class optional : public std::optional<T>, public SafeBool<optional<T> > {
public:
    typedef T value_type;

	// Construct empty optional.
	optional() : std::optional<T>() {}
	optional(nullopt_t) : std::optional<T>(nullopt) {}
	// Construct with value.
	optional(const T& value) : std::optional<T>(value) {}
	// Construct with value (eliminate double optional).
	optional(const optional& rhs) : std::optional<T>(rhs) {}

	bool safe_bool() const { return this->has_value(); }
};
#endif // (__cplusplus < 201703L)



////////////////////////////////////////////////////////////////
// Containers

// Insert the contents of the second container into the first.
template <typename TargetContainer, typename SourceContainer>
void insertElements(TargetContainer& target, SourceContainer const& source)
{
	target.insert(source.begin(), source.end());
}

// Append the contents of the second container to the first.
template <typename TargetContainer, typename SourceContainer>
void appendElements(TargetContainer& target, SourceContainer const& source)
{
	target.insert(target.end(), source.begin(), source.end());
}

// Delete all the elements in a container.
template <typename Container>
void deleteElements(Container& container)
{
	typedef typename Container::value_type value_type;
	for (typename Container::iterator it = container.begin();
		it != container.end(); ++it) {
		delete_s(*it);
	}
}

// Return the only element of a container, or nothing.
template <typename Element, typename Container>
optional<Element> getOnly(Container const& container)
{
	typename Container::size_type size = container.size();
	if (size != 1) return nullopt;
	return container.front();
}

// Return a new container with all the elements in the original container
// cloned.
template <typename Container>
Container cloneElements(Container const& base)
{
	Container results;
	for (typename Container::const_iterator it = base.begin();
	     it != base.end(); ++it)
		results.push_back((*it)->clone());
	return results;
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
void deleteSeconds(Map& map)
{
	for (typename Map::iterator it = map.begin();
		it != map.end(); ++it) {
		delete_s(it->second);
	}
}

// Overwrite all key/value pairs in source onto target.
template <typename Map>
void overwritePairs(Map& target, Map const& source)
{
	for (typename Map::const_iterator it = source.begin();
	     it != source.end(); ++it)
		target[it->first] = it->second;
}

// Partial function specialization workaround.
namespace detail {
	template <typename Element, typename Map, typename Key>
	struct find_impl {
		static optional<Element> _(Map const& map, Key const& key)
		{
			typename Map::const_iterator it = map.find(key);
			if (it == map.end()) return optional<Element>(nullopt);
			Element const& element = it->second;
			return optional<Element>(element);
		}
	};

	template <typename Element, typename Key>
	struct find_impl<Element, std::set<Element>, Key> {
		static optional<Element> _(
				std::set<Element> const& set, Key const& key)
		{
			typename std::set<Element>::const_iterator it = set.find(key);
			if (it == set.end()) return nullopt;
			return *it;
		}
	};
}

// Find an element in a map.
template <typename Element, typename Map, typename Key>
optional<Element> find(Map const& map, Key const& key)
{
	return detail::find_impl<Element, Map, Key>::_(map, key);
}

////////////////////////////////////////////////////////////////
// Trees

template <typename Value, typename Node, typename ChildContainer>
Value findLargest(
		Node* root,
		ChildContainer (*getChildren)(Node*),
		Value (*accessor)(Node*))
{
	Value largest = (*accessor)(root);
	ChildContainer children = (*getChildren)(root);
	for (typename ChildContainer::const_iterator it = children.begin();
	     it != children.end(); ++it)
	{
		Node* child = *it;
		Value current = findLargest(child, getChildren, accessor);
		if (largest < current)
			largest = current;
	}
	return largest;
}

// HACK! This is to use mem_debug in PCH
#if (defined(_DEBUG) && defined(_MSC_VER) && defined(__trapperkeeper_h_) && defined(VLD_FORCE_ENABLE))
#if (VLD_FORCE_ENABLE == 0)
#undef DEBUG_NEW
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif // (VLD_FORCE_ENABLE == 0)
#endif // (defined(_DEBUG) && defined(_MSC_VER) && defined(__trapperkeeper_h_) && defined(VLD_FORCE_ENABLE))


#endif // !ZSCRIPT_COMPILER_UTILS_H
