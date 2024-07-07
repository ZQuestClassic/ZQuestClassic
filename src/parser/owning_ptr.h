#ifndef ZSCRIPT_OWNING_PTR_H_
#define ZSCRIPT_OWNING_PTR_H_

////////////////////////////////////////////////////////////////
// owning_ptr - Owns its pointer, calls clone() on copy

template <class Type>
class owning_ptr
{
	// Safe bool idiom.
	typedef void (owning_ptr::*safe_bool_type) () const;
	void safe_bool_func() const {}
	
public:
	explicit owning_ptr(Type* p = nullptr) : data_(p) {}
	owning_ptr(owning_ptr const& other) : data_(other.clone()) {}
	template <class OtherType>
	owning_ptr(owning_ptr<OtherType> const& other)
			: data_(other.clone()) {}
	~owning_ptr() {delete data_;}
	owning_ptr& operator=(owning_ptr const& rhs)
	{
		if(data_)
			delete data_;
		data_ = rhs.clone();
		return *this;
	}
	owning_ptr& operator=(Type* pointer)
	{
		if(data_)
			delete data_;
		data_ = pointer;
		return *this;
	}

	// Return a clone of the contents, or nullptr if none.
	Type* clone() const
	{
		if (data_) return data_->clone();
		return nullptr;
	}

	operator safe_bool_type() const
	{
		return data_ ? &owning_ptr::safe_bool_func : nullptr;
	}

	Type* get() {return data_;}
	Type const* get() const {return data_;}
	Type& operator*() {return *data_;}
	Type const& operator*() const {return *data_;}
	Type* operator->() {return data_;}
	Type const* operator->() const {return data_;}
	Type* release()
	{
		Type* temp = data_;
		data_ = nullptr;
		return temp;
	}
	void reset(Type* p = nullptr)
	{
		if(data_)
			delete data_;
		data_ = p;
	}
	
private:
	Type* data_;
};

template <class Type, class Type2>
bool operator==(owning_ptr<Type> const& lhs, owning_ptr<Type2> const& rhs)
{
	return lhs.get() == rhs.get();
}

template <class Type, class Type2>
bool operator!=(owning_ptr<Type> const& lhs, owning_ptr<Type2> const& rhs)
{
	return lhs.get() != rhs.get();
}

template <class Type, class Type2>
bool operator==(owning_ptr<Type> const& lhs, Type2* rhs)
{
	return lhs.get() == rhs;
}

template <class Type, class Type2>
bool operator!=(owning_ptr<Type> const& lhs, Type2* rhs)
{
	return lhs.get() != rhs;
}

template <class Type, class Type2>
bool operator==(Type* lhs, owning_ptr<Type2> const& rhs)
{
	return lhs == rhs.get();
}

template <class Type, class Type2>
bool operator!=(Type* lhs, owning_ptr<Type2> const& rhs)
{
	return lhs != rhs.get();
}

#endif
