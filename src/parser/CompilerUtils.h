// Various utility functions for the compiler.

#ifndef ZSCRIPT_COMPILER_UTILS_H
#define ZSCRIPT_COMPILER_UTILS_H

#include <cassert>

// Delete all the elements in a vector of pointers.
template<class Element>
void deleteElements(vector<Element*>& container)
{
	for (typename vector<Element*>::iterator it = container.begin();
		 it != container.end(); ++it)
		delete *it;
}

// Delete all the elements in a list of pointers.
template<class Element>
void deleteElements(list<Element*>& container)
{
	for (typename list<Element*>::iterator it = container.begin();
		 it != container.end(); ++it)
		delete *it;
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
class optional
{
public:
	// Construct empty optional. 
	optional() : has_value_(false) {}
	optional(nullopt_t) : has_value_(false) {}
	// Construct with value.
	optional(Type const& value) : has_value_(true), data(value) {}
	// Construct with value (eliminate double optional).
	optional(optional const& rhs) : has_value_(rhs.has_value_)
	{
		if (rhs.has_value_)
			new(&data) Type(rhs.data);
	}

	~optional()
	{
		if (has_value_)
			data.~Type();
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
			data = *rhs;
		return *this;
	}

	Type const* operator->() const {return &data;}
	Type* operator->() {return &data;}
	Type const& operator*() const {return data;}
	Type& operator*() {return data;}

	bool has_value() const {return has_value_;}
	Type const& value() const
	{
		assert(has_value_);
		return data;
	}
	Type& value()
	{
		assert(has_value_);
		return data;
	}

	template <typename U>
	Type value_or(U const& v) const
	{
		return has_value_ ? data : static_cast<Type>(v);
	}

	// Destroys the value if present.
	void reset()
	{
		if (has_value_) data.~Type();
		has_value_ = false;
	}
	
private:
	bool has_value_;
	union {Type data;};

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

#endif
