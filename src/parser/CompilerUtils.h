// Various utility functions for the compiler.

#ifndef ZSCRIPT_COMPILER_UTILS_H_
#define ZSCRIPT_COMPILER_UTILS_H_

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

#include <optional>

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <set>
#include <sstream>
#include <string>
#include <vector>

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
	for (typename Container::iterator it = container.begin();
		it != container.end(); ++it) {
		delete *it;
		*it = typename Container::value_type();
	}
}

// Return the only element of a container, or nothing.
template <typename Element, typename Container>
std::optional<Element> getOnly(Container const& container)
{
	typename Container::size_type size = container.size();
	if (size != 1) return std::nullopt;
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
		delete it->second;
		it->second = typename Map::mapped_type();
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
		static std::optional<Element> _(Map const& map, Key const& key)
		{
			typename Map::const_iterator it = map.find(key);
			if (it == map.end()) return std::optional<Element>(std::nullopt);
			Element const& element = it->second;
			return std::optional<Element>(element);
		}
	};

	template <typename Element, typename Key>
	struct find_impl<Element, std::set<Element>, Key> {
		static std::optional<Element> _(
				std::set<Element> const& set, Key const& key)
		{
			typename std::set<Element>::const_iterator it = set.find(key);
			if (it == set.end()) return std::nullopt;
			return *it;
		}
	};
}

// Find an element in a map.
template <typename Element, typename Map, typename Key>
std::optional<Element> find(Map const& map, Key const& key)
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


#endif // !ZSCRIPT_COMPILER_UTILS_H
