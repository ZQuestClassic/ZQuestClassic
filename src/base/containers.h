#ifndef CONTAINERS_H_
#define CONTAINERS_H_

#include "base/headers.h"
#include <array>
#include <stdexcept>

struct SuperSet
{
	std::set<int>* get(int v)
	{
		for(auto& s : superset)
		{
			if(s.contains(v))
				return &s;
		}
		return nullptr;
	}
	std::set<int> const* get(int v) const
	{
		for(auto& s : superset)
		{
			if(s.contains(v))
				return &s;
		}
		return nullptr;
	}
	std::set<int>* get_add(int v)
	{
		if(auto s = get(v))
			return s;
		if(banned_vals.contains(v))
			return nullptr;
		auto& s = superset.emplace_back();
		s.insert(v);
		return &s;
	}
	std::set<int>* add_to(int targ, int v)
	{
		if(banned_vals.contains(targ))
			return get(v);
		if(banned_vals.contains(v))
			return get(targ);
		
		auto s = get_add(targ);
		if(auto s2 = get(v))
		{
			if(s != s2)
				merge(*s,*s2);
		}
		else s->insert(v);
		return s;
	}
	std::set<int>* add(std::set<int> const& s)
	{
		std::set<int>* ptr = nullptr;
		for(int v : s)
		{
			if(banned_vals.contains(v))
				continue;
			if(ptr)
			{
				if(auto ptr2 = get(v))
				{
					if(ptr != ptr2)
						merge(*ptr, *ptr2);
				}
				else
				{
					ptr->insert(v);
				}
			}
			else
			{
				ptr = get_add(v);
			}
		}
		return ptr;
	}
	void clear()
	{
		superset.clear();
	}
	size_t size() const
	{
		size_t sz = 0;
		for(auto& s : superset)
			sz += s.size();
		return sz;
	}
	bool empty() const
	{
		return superset.empty();
	}
	bool remove(std::set<int>& s)
	{
		for(auto it = superset.begin(); it != superset.end();)
		{
			std::set<int>& s2 = *it;
			if(&s == &s2)
			{
				superset.erase(it);
				return true;
			}
			else ++it;
		}
		return false;
	}
	vector<std::set<int> const*> subset(std::set<int> const& s) const
	{
		vector<std::set<int> const*> ret;
		for(auto& s2 : superset)
		{
			for(auto v : s)
				if(s2.contains(v))
				{
					ret.emplace_back(&s2);
					break;
				}
		}
		return ret;
	}
	template<typename T>
	vector<std::set<int> const*> subset(std::map<int,T> const& s) const
	{
		vector<std::set<int> const*> ret;
		for(auto& s2 : superset)
		{
			for(auto p : s)
				if(s2.contains(p.first))
				{
					ret.emplace_back(&s2);
					break;
				}
		}
		return ret;
	}
	std::set<int> banned_vals = {0};
private:
	vector<std::set<int>> superset;
	void merge(std::set<int>& dest, std::set<int>& src)
	{
		dest.insert(src.begin(), src.end());
		remove(src);
	}
};

template<typename Sz,typename T>
class bounded_container
{
public:
	typedef Sz size_type;
	typedef T value_type;
	
	bounded_container() : true_sz(0), default_val() {}
	bounded_container(size_type const& sz) : true_sz(sz), default_val() {}
	bounded_container(size_type const& sz, value_type const& dv) : true_sz(sz), default_val(dv) {}
	virtual ~bounded_container() = default;
	
	virtual value_type& at(size_type ind) = 0;
	virtual value_type const& at(size_type ind) const = 0;
	virtual value_type& operator[](size_type ind) = 0;
	virtual value_type const& operator[](size_type ind) const = 0;
	virtual void clear() = 0;
	virtual void normalize() = 0;
	
	bool empty() const {return !true_sz;}
	size_type size() const {return true_sz;}
	void resize(size_type sz) {true_sz = sz;}
	
	value_type const& defval() const {return default_val;}
	
	void copy_to(value_type* arr, size_type sz)
	{
		for(size_type q = 0; q < sz; ++q)
		{
			if(q >= size())
				arr[q] = default_val;
			else arr[q] = at(q);
		}
	}
protected:
	size_type true_sz;
	value_type default_val;
};

template<typename Sz,typename T>
class bounded_map : public bounded_container<Sz,T>
{
public:
	typedef Sz size_type;
	typedef T value_type;
	typedef map<size_type,value_type> cont_t;
	typedef bounded_map<size_type,value_type> bound_t;
	
	bounded_map() : bounded_container<Sz,T>(0) {}
	bounded_map(size_type tsz) : bounded_container<Sz,T>(tsz) {}
	bounded_map(size_type tsz, value_type dv) : bounded_container<Sz,T>(tsz,dv) {}
	virtual ~bounded_map() = default;
	
	value_type& at(size_type ind)
	{
		if(ind < this->true_sz)
		{
			if(cont.contains(ind))
				return cont[ind];
			return (cont[ind] = this->default_val);
		}
		throw std::out_of_range("Bad bounded_map access");
	}

	value_type const& at(size_type ind) const
	{
		if(ind < this->true_sz)
		{
			if(cont.contains(ind))
				return cont.at(ind);
			return this->default_val;
		}
		throw std::out_of_range("Bad bounded_map access");
	}

	value_type& operator[](size_type ind)
	{
		return at(ind);
	}

	value_type const& operator[](size_type ind) const
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
		this->default_val = other.default_val;
		normalize();
		return *this;
	}
	bound_t& operator=(map<size_type,value_type> const& other)
	{
		cont = other;
		normalize();
		return *this;
	}
	bound_t& operator=(vector<value_type> const& other)
	{
		cont.clear();
		for(size_t q = 0; q < other.size(); ++q)
		{
			if(other[q] != this->default_val)
				cont[q] = other[q];
		}
		normalize();
		return *this;
	}
	
	cont_t const& inner() const {return cont;}
	cont_t& mut_inner() {return cont;}
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
	
	bool operator==(bound_t const& other) const
	{
		if(other.size() != this->size())
			return false;
		if(other.default_val != this->default_val)
			return false;
		map<size_type,bool> keys;
		for(auto [k,v] : cont)
			keys[k] = true;
		for(auto [k,v] : other.cont)
			keys[k] = true;
		for(auto [k,b] : keys)
			if(other[k] != (*this)[k])
				return false;
		return true;
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

template<typename Sz,typename T>
class bounded_vec : public bounded_container<Sz,T>
{
public:
	typedef Sz size_type;
	typedef T value_type;
	typedef vector<value_type> cont_t;
	typedef bounded_vec<size_type,value_type> bound_t;
	
	bounded_vec() : bounded_container<Sz,T>(0) {}
	bounded_vec(size_type tsz) : bounded_container<Sz,T>(tsz) {}
	bounded_vec(size_type tsz, value_type dv) : bounded_container<Sz,T>(tsz,dv) {}
	virtual ~bounded_vec() = default;
	
	value_type& at(size_type ind)
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

	value_type const& at(size_type ind) const
	{
		if(ind < this->true_sz)
		{
			if(ind < cont.size())
				return cont.at(ind);
			return this->default_val;
		}
		throw std::out_of_range("Bad bounded_vec access");
	}

	value_type& operator[](size_type ind)
	{
		return at(ind);
	}

	value_type const& operator[](size_type ind) const
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
		this->default_val = other.default_val;
		normalize();
		return *this;
	}
	bound_t& operator=(vector<value_type> const& other)
	{
		cont = other;
		normalize();
		return *this;
	}
	
	cont_t const& inner() const {return cont;}
	value_type& front() {return cont.front();}
	value_type const& front() const {return cont.front();}
	value_type& back() {return cont.at(this->true_sz-1);}
	value_type const& back() const {return cont.at(this->true_sz-1);}
	value_type const* data() const {return cont.data();}
	size_type capacity() const {return cont.size();}
	void reserve(size_type newcap) {cont.reserve(newcap);}
	virtual void clear() {cont.clear();}
	void swap(bounded_vec<size_type,value_type>& other)
	{
		normalize();
		cont.swap(other.cont);
		zc_swap(this->true_sz, other.true_sz);
		normalize();
	}
	
	virtual void normalize();
	
	bool inner_empty() const {return cont.empty();}
	
	bool operator==(bound_t const& other) const
	{
		if(other.size() != this->size())
			return false;
		if(other.default_val != this->default_val)
			return false;
		size_t cap = std::max(other.capacity(), this->capacity());
		for(size_t q = 0; q < cap; ++q)
			if(other[q] != (*this)[q])
				return false;
		return true;
	}
	
	bounded_map<size_type,value_type> as_bmap() const
	{
		bounded_map<size_type,value_type> ret(this->true_sz, this->default_val);
		ret = cont; //overloaded for std::vector
		return ret;
	}
private:
	cont_t cont;
};

class bitstring
{
public:
	typedef size_t index_t;
	typedef word storage_t;
	static constexpr storage_t max_bytes = 65535;
	
	bool get(index_t ind) const
	{
		return cont[ind/8] & (1 << ind%8);
	}
	void set(index_t ind, bool state)
	{
		if(state)
			cont[ind/8] |= (1 << ind%8);
		else if(get(ind))
			cont[ind/8] &= ~(1 << ind%8);
	}
	void toggle(index_t ind)
	{
		cont[ind/8] ^= (1 << ind%8);
	}
	index_t length() const
	{
		return cont.inner().size() * 8;
	}
	void normalize() {cont.normalize();}
	void clear() {cont.clear();}
	bounded_vec<storage_t,byte>& inner() {return cont;}
	bounded_vec<storage_t,byte> const& inner() const {return cont;}
	bool operator==(bitstring const& other) const = default;
private:
	bounded_vec<storage_t,byte> cont {max_bytes};
};

class bitstring_long
{
public:
	typedef uint64_t index_t;
	typedef uint32_t storage_t;
	static constexpr storage_t max_bytes = 4294967295;
	
	bool get(index_t ind) const
	{
		return cont[ind/8] & (1 << ind%8);
	}
	void set(index_t ind, bool state)
	{
		if(state)
			cont[ind/8] |= (1 << ind%8);
		else if(get(ind))
			cont[ind/8] &= ~(1 << ind%8);
	}
	void toggle(index_t ind)
	{
		cont[ind/8] ^= (1 << ind%8);
	}
	index_t length() const
	{
		return cont.inner().size() * 8;
	}
	void normalize() {cont.normalize();}
	void clear() {cont.clear();}
	bounded_vec<storage_t,byte>& inner() {return cont;}
	bounded_vec<storage_t,byte> const& inner() const {return cont;}
	bool operator==(bitstring_long const& other) const = default;
private:
	bounded_vec<storage_t,byte> cont {max_bytes};
};

template<typename A,typename B>
class def_pair
{
	typedef def_pair<A,B> pair_t;
public:
	def_pair() : first(), second(){}
	def_pair(A const& first) : first(first), second(){}
	def_pair(A const& first, B const& second) : first(first), second(second){}
	def_pair(A&& first) : first(std::move(first)), second(){}
	def_pair(A&& first, B&& second) : first(std::move(first)), second(std::move(second)){}
	def_pair(A const& first, B&& second) : first(first), second(std::move(second)){}
	def_pair(A&& first, B const& second) : first(std::move(first)), second(second){}
	bool operator==(pair_t const& other) const = default;
	
	A first;
	B second;
};

template<typename T>
void _do_normalize(T& v){}
template<>
inline void _do_normalize<bitstring>(bitstring& v)
{
	v.normalize();
}
template<typename Sz,typename T>
void _do_normalize(bounded_vec<Sz,T>& v)
{
	v.normalize();
}
template<typename Sz,typename T>
void _do_normalize(bounded_map<Sz,T>& v)
{
	v.normalize();
}


template<typename Sz,typename T>
void bounded_vec<Sz,T>::normalize()
{
	if(cont.size() > this->true_sz)
		cont.resize(this->true_sz);
	while(cont.size() && cont.back() == this->default_val)
		cont.pop_back();
	
	for(auto& v : cont)
		_do_normalize(v);
}
template<typename Sz,typename T>
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

