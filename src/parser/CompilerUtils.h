// Various utility functions for the compiler.

#ifndef ZSCRIPT_COMPILER_UTILS_H
#define ZSCRIPT_COMPILER_UTILS_H

#include <cassert>
#include <vector>
#include <set>
#include <sstream>
#include <string>
#include <cstdarg>

////////////////////////////////////////////////////////////////
// Strings

template <typename Type>
std::string to_string(Type val)
{
	std::ostringstream oss;
	oss << val;
	return oss.str();
}

int const formatBufferSize = 4096;

/*
namespace ZScript
{
	// as sprintf, but for strings.
	int const formatBufferSize = 4096;
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

// Empty optional instance.
struct nullopt_t
{
    struct init{};
    nullopt_t(init) {}
};
const nullopt_t nullopt((nullopt_t::init()));

template <class Type>
class optional : public SafeBool<optional<Type> >
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

	bool safe_bool() const {return has_value_;}
	
private:
	bool has_value_;
	union {char data[1 + (sizeof(Type) - 1) / sizeof(char)];};
};

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
		delete *it;
		*it = typename Container::value_type(); // techinically a pointer but this stuff is non-standard
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
		delete it->second;
		it->second = typename Map::mapped_type(); // a pointer, but this code is non-standard, should use nullptr later
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
			if (it == map.end()) return nullopt;
			Element const& element = it->second;
			return element;
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

#endif
