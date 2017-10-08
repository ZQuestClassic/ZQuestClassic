// Various utility functions for the compiler.

#ifndef ZSCRIPT_COMPILER_UTILS_H
#define ZSCRIPT_COMPILER_UTILS_H

#include <cassert>
#include <cstddef>
#include <vector>
#include <map>
#include <set>
#include <sstream>

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
// To use, inherit publically from safe_bool<Own Class> and implement
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
		if (has_value_) return *reinterpret_cast<Type*>(&data);
		return *reinterpret_cast<Type*>(&v);
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
// Strings

// Lets you append to the string until it becomes too long.
class LimitString
{
public:
	LimitString() : limit(64), capped(false) {}
	LimitString(std::size_t const& limit) : limit(limit), capped(false) {}
	
	template<typename Value> LimitString& operator<<(Value const& val)
	{
		if (!capped)
		{
			std::ostringstream temp;
			temp << val;
			if (data.str().size() + temp.str().size() > limit)
			{
				data << "...";
				capped =  true;
			}
			else data << temp.str();
		}
		return *this;
	}

	operator std::string() const {return data.str();}
	bool isCapped() const {return capped;}
	std::string str() const {return data.str();}

private:
	std::size_t limit;
	std::ostringstream data;
	bool capped;
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
void deleteElements(Container const& container)
{
	for (typename Container::const_iterator it = container.begin();
	     it != container.end(); ++it)
		delete *it;
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
// Vectors

// Easily construct vectors.
template <typename Element>
class VectorBuilder
{
public:
	VectorBuilder& operator<<(Element rhs) {
		data.push_back(rhs); return *this;}
	operator std::vector<Element>() const {return data;}
	
private:
	std::vector<Element> data;
};

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
	for (typename Map::const_iterator it = map.begin();
	     it != map.end(); ++it)
		delete it->second;
}

// Overwrite all key/value pairs in source onto target.
template <typename Map>
void overwritePairs(Map& target, Map const& source)
{
	for (typename Map::const_iterator it = source.begin();
	     it != source.end(); ++it)
		target[it->first] = it->second;
}

// Find an element in a map.
template <typename Element, typename Key, typename Compare, typename Alloc>
optional<Element> find(
		std::map<Key, Element, Compare, Alloc> const& container,
		Key const& key)
{
	typedef std::map<Key, Element, Compare, Alloc> Map;
	typename Map::const_iterator it = container.find(key);
	if (it == container.end()) return nullopt;
	Element const& element = it->second;
	return element;
}

////////////////////////////////////////////////////////////////
// Sets

// Find an element in a set.
template <typename Element, typename Compare, typename Alloc>
optional<Element> find(
		std::set<Element, Compare, Alloc> const& container,
		Element const& key)
{
	typedef std::set<Element, Compare, Alloc> Set;
	typename Set::const_iterator it = container.find(key);
	if (it == container.end()) return nullopt;
	Element const& element = *it;
	return element;
}

// Insert into a set.
template <typename Element, typename Source>
void insert(std::set<Element> const& target, Source const& source)
{
	target.insert(source.begin(), source.end());
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
