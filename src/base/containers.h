#ifndef _CONTAINERS_H_
#define _CONTAINERS_H_

#include "base/headers.h"
#include <stdexcept>

template<class Sz,typename T>
class bounded_container
{
public:
	typedef Sz size_type;
	typedef T obj_type;
	
	bounded_container() : true_sz(0), default_val() {}
	bounded_container(size_type const& sz) : true_sz(sz), default_val() {}
	bounded_container(size_type const& sz, obj_type const& dv) : true_sz(sz), default_val(dv) {}
	virtual ~bounded_container() = default;
	
	virtual obj_type& at(size_type ind) = 0;
	virtual obj_type const& at(size_type ind) const = 0;
	virtual obj_type& operator[](size_type ind) = 0;
	virtual obj_type const& operator[](size_type ind) const = 0;
	virtual void clear() = 0;
	virtual void normalize() = 0;
	
	bool empty() const {return !true_sz;}
	size_type size() const {return true_sz;}
	void resize(size_type sz) {true_sz = sz;}
	
	obj_type const& defval() const {return default_val;}
protected:
	size_type true_sz;
	obj_type default_val;
};

template<class Sz,class T>
class bounded_vec : public bounded_container<Sz,T>
{
public:
	typedef Sz size_type;
	typedef T obj_type;
	typedef vector<obj_type> cont_t;
	typedef bounded_vec<size_type,obj_type> bound_t;
	
	bounded_vec() : bounded_container<Sz,T>(0) {}
	bounded_vec(size_type tsz) : bounded_container<Sz,T>(tsz) {}
	bounded_vec(size_type tsz, obj_type dv) : bounded_container<Sz,T>(tsz,dv) {}
	virtual ~bounded_vec() = default;
	
	obj_type& at(size_type ind)
	{
		if(ind < this->true_sz)
		{
			if(ind < cont.size())
				return cont.at(ind);
			cont.resize(ind+1,this->default_val);
			return cont.at(ind);
		}
		throw std::out_of_range("Bad bounded_vec access");
	}

	obj_type const& at(size_type ind) const
	{
		if(ind < this->true_sz)
		{
			if(ind < cont.size())
				return cont.at(ind);
			return this->default_val;
		}
		throw std::out_of_range("Bad bounded_vec access");
	}

	obj_type& operator[](size_type ind)
	{
		return at(ind);
	}

	obj_type const& operator[](size_type ind) const
	{
		return at(ind);
	}
	
	bounded_vec(bound_t const& other)
	{
		*this = other;
	}
	bound_t& operator=(bound_t const& other)
	{
		cont = other.cont;
		this->true_sz = other.true_sz;
		normalize();
		return *this;
	}
	bound_t& operator=(vector<obj_type> const& other)
	{
		cont = other;
		normalize();
		return *this;
	}
	
	cont_t const& inner() const {return cont;}
	obj_type& front() {return cont.front();}
	obj_type const& front() const {return cont.front();}
	obj_type& back() {return cont.at(this->true_sz-1);}
	obj_type const& back() const {return cont.at(this->true_sz-1);}
	obj_type const* data() const {return cont.data();}
	size_type capacity() const {return cont.size();}
	void reserve(size_type newcap) {cont.reserve(newcap);}
	virtual void clear() {cont.clear();}
	void swap(bounded_vec<size_type,obj_type>& other)
	{
		normalize();
		cont.swap(other.cont);
		zc_swap(this->true_sz, other.true_sz);
		normalize();
	}
	
	virtual void normalize();
	
	bool inner_empty() const {return cont.empty();}
	
	bool operator==(bound_t const& other)
	{
		return other.size() == this->size() && other.cont == this->cont;
	}
	bool operator!=(bound_t const& other)
	{
		return !(other == *this);
	}
private:
	cont_t cont;
};

template<class Sz,class T>
class bounded_map : public bounded_container<Sz,T>
{
public:
	typedef Sz size_type;
	typedef T obj_type;
	typedef map<size_type,obj_type> cont_t;
	typedef bounded_map<size_type,obj_type> bound_t;
	
	bounded_map() : bounded_container<Sz,T>(0) {}
	bounded_map(size_type tsz) : bounded_container<Sz,T>(tsz) {}
	bounded_map(size_type tsz, obj_type dv) : bounded_container<Sz,T>(tsz,dv) {}
	virtual ~bounded_map() = default;
	
	obj_type& at(size_type ind)
	{
		if(ind < this->true_sz)
		{
			if(cont.contains(ind))
				return cont[ind];
			return (cont[ind] = this->default_val);
		}
		throw std::out_of_range("Bad bounded_map access");
	}

	obj_type const& at(size_type ind) const
	{
		if(ind < this->true_sz)
		{
			if(cont.contains(ind))
				return cont.at(ind);
			return this->default_val;
		}
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
	
	bounded_map(bound_t const& other)
	{
		*this = other;
	}
	bound_t& operator=(bound_t const& other)
	{
		cont = other.cont;
		this->true_sz = other.true_sz;
		normalize();
		return *this;
	}
	bound_t& operator=(map<size_type,obj_type> const& other)
	{
		cont = other;
		normalize();
		return *this;
	}
	
	cont_t const& inner() const {return cont;}
	size_type capacity() const {return cont.size();}
	virtual void clear() {cont.clear();}
	
	virtual void normalize();
	
	bool inner_empty() const {return cont.empty();}
	
	bool erase(size_type const& key)
	{
		return cont.erase(key) > 0;
	}
	bool contains(size_type const& key) const
	{
		return key < this->true_sz && cont.contains(key);
	}
	
	optional<size_type> firstKey() const
	{
		for(auto it = cont.begin(); it != cont.end(); ++it)
		{
			if(it->first < this->true_sz)
				return it->first;
		}
		return nullopt;
	}
	optional<size_type> lastKey() const
	{
		for(auto it = cont.rbegin(); it != cont.rend(); ++it)
		{
			if(it->first < this->true_sz)
				return it->first;
		}
		return nullopt;
	}
private:
	cont_t cont;
};

template<class T>
void _do_normalize(T& v){}
template<class Sz,class T>
void _do_normalize(bounded_vec<Sz,T>& v)
{
	v.normalize();
}
template<class Sz,class T>
void _do_normalize(bounded_map<Sz,T>& v)
{
	v.normalize();
}

template<class Sz,class T>
void bounded_vec<Sz,T>::normalize()
{
	if(cont.size() > this->true_sz)
		cont.resize(this->true_sz);
	while(cont.size() && cont.back() == this->default_val)
		cont.pop_back();
	
	for(auto& v : cont)
		_do_normalize(v);
}
template<class Sz,class T>
void bounded_map<Sz,T>::normalize()
{
	for(auto it = cont.begin(); it != cont.end();)
	{
		if(it->second == this->default_val || unsigned(it->first) >= this->true_sz)
			it = cont.erase(it);
		else ++it;
	}
	
	for(auto& [k,v] : cont)
		_do_normalize(v);
}
#endif

