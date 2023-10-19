#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

#include "base/ints.h"
#include "base/general.h"
#include <vector>
#include <map>
#include <stdexcept>

template<class Sz,class T>
class bounded_vec
{
public:
	typedef Sz size_type;
	typedef T obj_type;
	typedef std::vector<obj_type> cont_t;
	typedef bounded_vec<size_type,obj_type> bound_t;
	
	bounded_vec() : true_sz(0) {}
	bounded_vec(size_type tsz) : true_sz(tsz) {}
	
	obj_type& at(size_type ind)
	{
		if(ind < cont.size())
			return cont.at(ind);
		if(ind < true_sz)
		{
			cont.resize(ind+1);
			return cont.at(ind);
		}
		return cont.at(ind); //vector bound handling
	}

	obj_type const& at(size_type ind) const
	{
		if(ind < cont.size())
			return cont.at(ind);
		if(ind < true_sz)
		{
			static obj_type default_t;
			return default_t;
		}
		return cont.at(ind); //vector bound handling
	}

	obj_type& operator[](size_type ind)
	{
		return at(ind);
	}

	obj_type const& operator[](size_type ind) const
	{
		return at(ind);
	}
	
	bound_t& operator=(bounded_vec<size_type,obj_type> const& other)
	{
		cont = other.cont;
		true_sz = other.true_sz;
		return *this;
	}
	bound_t& operator=(std::vector<obj_type> const& other)
	{
		cont.clear();
		cont.insert(cont.end(), other.begin(), other.size() > true_sz ?
			other.begin()+true_sz : other.end());
		return *this;
	}
	
	cont_t const& inner() const {return cont;}
	obj_type& front() {return cont.front();}
	obj_type const& front() const {return cont.front();}
	obj_type& back() {return cont.at(true_sz-1);}
	obj_type const& back() const {return cont.at(true_sz-1);}
	obj_type const* data() const {return cont.data();}
	bool empty() const {return !true_sz;}
	size_type size() const {return true_sz;}
	size_type capacity() const {return cont.size();}
	void clear() {cont.clear();}
	void resize(size_type sz)
	{
		true_sz = sz;
		if(cont.size() > sz)
			cont.resize(sz);
	}
	void swap(bounded_vec<size_type,obj_type>& other)
	{
		cont.swap(other.cont);
		zc_swap(true_sz, other.true_sz);
	}
	
	bool inner_empty() const {return cont.empty();}
private:
	cont_t cont;
	size_type true_sz;
};

template<class Sz,class T>
class bounded_map
{
public:
	typedef Sz size_type;
	typedef T obj_type;
	typedef std::map<size_type,obj_type> cont_t;
	typedef bounded_map<size_type,obj_type> bound_t;
	
	bounded_map() : true_sz(0) {}
	bounded_map(size_type tsz) : true_sz(tsz) {}
	
	obj_type& at(size_type ind)
	{
		if(ind < true_sz)
			return cont[ind];
		throw std::out_of_range("Bad bounded_map access");
	}

	obj_type const& at(size_type ind) const
	{
		if(cont.contains(ind))
			return cont[ind];
		if(ind < true_sz)
			return default_t;
		throw std::out_of_range("Bad bounded_map access");
	}

	obj_type& operator[](size_type ind)
	{
		return at(ind);
	}

	obj_type const& operator[](size_type ind) const
	{
		return at(ind);
	}
	
	bound_t& operator=(bounded_map<size_type,obj_type> const& other)
	{
		cont = other.cont;
		true_sz = other.true_sz;
		normalize();
		return *this;
	}
	bound_t& operator=(std::map<size_type,obj_type> const& other)
	{
		cont = other;
		normalize();
		return *this;
	}
	
	cont_t const& inner() const {return cont;}
	bool empty() const {return !true_sz;}
	size_type size() const {return true_sz;}
	size_type capacity() const {return cont.size();}
	void clear() {cont.clear();}
	void resize(size_type sz)
	{
		bool shrunk = sz < true_sz;
		true_sz = sz;
		if(shrunk)
			normalize();
	}
	
	void normalize()
	{
		for(auto it = cont.begin(); it != cont.end();)
		{
			if(it->second == default_t || unsigned(it->first) >= true_sz)
				it = cont.erase(it);
			else ++it;
		}
	}
	
private:
	cont_t cont;
	size_type true_sz;
	static obj_type default_t;
};

#endif

