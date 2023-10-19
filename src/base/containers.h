#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

#include "base/ints.h"
#include "base/general.h"
#include <vector>

template<class T>
class bounded_vec
{
public:
	typedef T obj_type;
	
	bounded_vec() : true_sz(0) {}
	bounded_vec(size_t tsz) : true_sz(tsz) {}
	
	T& at(size_t ind)
	{
		if(ind < vec.size())
			return vec.at(ind);
		if(ind < true_sz)
		{
			vec.resize(ind+1);
			return vec.at(ind);
		}
		return vec.at(ind); //vector bound handling
	}

	T const& at(size_t ind) const
	{
		if(ind < vec.size())
			return vec.at(ind);
		if(ind < true_sz)
		{
			static T default_t;
			return default_t;
		}
		return vec.at(ind); //vector bound handling
	}

	T& operator[](size_t ind)
	{
		return at(ind);
	}

	T const& operator[](size_t ind) const
	{
		return at(ind);
	}
	
	bounded_vec<T>& operator=(bounded_vec<T> const& other)
	{
		vec = other.vec;
		true_sz = other.true_sz;
		return *this;
	}
	bounded_vec<T>& operator=(std::vector<T> const& other)
	{
		vec.clear();
		vec.insert(vec.end(), other.begin(), other.size() > true_sz ?
			other.begin()+true_sz : other.end());
		return *this;
	}
	
	T& front() {return vec.front();}
	T const& front() const {return vec.front();}
	T& back() {return vec.at(true_sz-1);}
	T const& back() const {return vec.at(true_sz-1);}
	T const* data() const {return vec.data();}
	bool empty() const {return !true_sz;}
	size_t size() const {return true_sz;}
	size_t capacity() const {return vec.size();}
	void clear() {vec.clear();}
	void resize(size_t sz)
	{
		true_sz = sz;
		if(vec.size() > sz)
			vec.resize(sz);
	}
	void swap(bounded_vec<T>& other)
	{
		vec.swap(other.vec);
		zc_swap(true_sz, other.true_sz);
	}
	
	bool inner_empty() const {return vec.empty();}
private:
	std::vector<T> vec;
	size_t true_sz;
};

#endif

