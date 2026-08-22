#ifndef ZALLEG_PACKFILE_H_
#define ZALLEG_PACKFILE_H_

#include "zalleg/zalleg.h"
#include "base/ints.h"
#include "base/general.h"
#include "base/containers.h"
#include "base/zfix.h"
#include <memory>

extern int32_t readsize, writesize;
extern bool fake_pack_writing;
#define new_return(x) {assert(x == 0); fake_pack_writing = false; return x; }

#define V_EXPORTED_VARIABLE 1
extern int packfile_v_exported_variable;

struct PackfileVersionHandler
{
	int& ref;
	int old_ver;
	int new_ver;
	PackfileVersionHandler(int& ref, int new_ver);
	~PackfileVersionHandler();
};

bool pfwrite(const char *p,int32_t n,PACKFILE *f);
bool pfwrite(const void *p,int32_t n,PACKFILE *f);

bool pfread(void *p,int n,PACKFILE *f,bool allow_less = false);
bool p_getc(void *p,PACKFILE *f);
bool p_putc(int32_t c,PACKFILE *f);
bool p_igetw(void *p,PACKFILE *f);
bool p_iputw(int32_t c,PACKFILE *f);
bool p_igetl(void *p,PACKFILE *f);
bool p_igetzf(void *p,PACKFILE *f);
bool p_igetd(void *p, PACKFILE *f);
bool p_igetf_DO_NOT_USE(void *p,PACKFILE *f); // Floats are not serializable, do not use!
bool p_iputl(int32_t c,PACKFILE *f);
bool p_iputzf(zfix const& c,PACKFILE *f);
bool p_mgetw(void *p,PACKFILE *f);
bool p_mputw(int32_t c,PACKFILE *f);
bool p_mgetl(void *p,PACKFILE *f);
bool p_mputl(int32_t c,PACKFILE *f);

bool p_getstr(char *str, size_t sz, PACKFILE *f);
bool p_getstr(string *str, size_t sz, PACKFILE *f);
bool p_putstr(char const* str, size_t sz, PACKFILE *f);
bool p_putstr(string const& str, size_t sz, PACKFILE *f);

bool p_getcstr(string *str, PACKFILE *f);
bool p_putcstr(string const& str, PACKFILE *f);
bool p_getwstr(string *str, PACKFILE *f);
bool p_putwstr(string const& str, PACKFILE *f);
bool p_getlstr(string *str, PACKFILE *f);
bool p_putlstr(string const& str, PACKFILE *f);

bool p_getbitstr(bitstring* ptr, PACKFILE *f);
bool p_putbitstr(bitstring const& ptr, PACKFILE *f);

bool p_getvar(zfix* ptr, PACKFILE *f);
bool p_putvar(zfix const& ptr, PACKFILE *f);

bool p_getvar(string* ptr, PACKFILE *f);
bool p_putvar(string const& ptr, PACKFILE *f);

bool p_getvar(exported_variable* ptr, PACKFILE *f);
bool p_putvar(exported_variable const& ptr, PACKFILE *f);
bool p_getvar(script_config_nosavescript* ptr, PACKFILE *f);
bool p_putvar(script_config_nosavescript const& ptr, PACKFILE *f);
bool p_getvar(script_config* ptr, PACKFILE *f);
bool p_putvar(script_config const& ptr, PACKFILE *f);


template<typename T>
inline bool p_getcvec(vector<T> *vec, PACKFILE *f);
template<typename T>
inline bool p_putcvec(vector<T> const& vec, PACKFILE *f);
template<typename T>
inline bool p_getwvec(vector<T> *vec, PACKFILE *f);
template<typename T>
inline bool p_putwvec(vector<T> const& vec, PACKFILE *f);
template<typename T>
inline bool p_getlvec(vector<T> *vec, PACKFILE *f);
template<typename T>
inline bool p_putlvec(vector<T> const& vec, PACKFILE *f);

template<typename K, typename V>
inline bool p_getcmap(map<K, V> *mp, PACKFILE *f);
template<typename K, typename V>
inline bool p_putcmap(map<K, V> const& mp, PACKFILE *f);
template<typename K, typename V>
inline bool p_getwmap(map<K, V> *mp, PACKFILE *f);
template<typename K, typename V>
inline bool p_putwmap(map<K, V> const& mp, PACKFILE *f);
template<typename K, typename V>
inline bool p_getlmap(map<K, V> *mp, PACKFILE *f);
template<typename K, typename V>
inline bool p_putlmap(map<K, V> const& mp, PACKFILE *f);

template<uint_type Sz,typename T>
inline bool p_getbvec(bounded_vec<Sz,T> *cont, PACKFILE *f);
template<uint_type Sz,typename T>
inline bool p_putbvec(bounded_vec<Sz,T> const& cont, PACKFILE *f);
template<uint_type Sz,typename T>
inline bool p_getbmap(bounded_map<Sz,T> *cont, PACKFILE *f);
template<uint_type Sz,typename T>
inline bool p_putbmap(bounded_map<Sz,T> const& cont, PACKFILE *f);
template<typename T, size_t Sz>
inline bool p_getarr(T cont[Sz], PACKFILE *f);
template<typename T, size_t Sz>
inline bool p_putarr(T const (&cont)[Sz], PACKFILE *f);
template<typename T, size_t Sz>
inline bool p_getarr(std::array<T,Sz>* cont, PACKFILE *f);
template<typename T, size_t Sz>
inline bool p_putarr(std::array<T,Sz> const& cont, PACKFILE *f);
template<typename A, typename B>
inline bool p_getpair(std::pair<A,B>* cont, PACKFILE *f);
template<typename A, typename B>
inline bool p_putpair(std::pair<A,B> const& cont, PACKFILE *f);
template<typename A, typename B>
inline bool p_getpair(def_pair<A,B>* cont, PACKFILE *f);
template<typename A, typename B>
inline bool p_putpair(def_pair<A,B> const& cont, PACKFILE *f);

template<integral_type T>
inline bool p_getvar(T* ptr, PACKFILE *f)
{
	if constexpr (sizeof(T) == 1)
		return p_getc(ptr, f);
	else if constexpr (sizeof(T) == 2)
		return p_igetw(ptr, f);
	else if constexpr (sizeof(T) == 4)
		return p_igetl(ptr, f);
	return pfread((char*)ptr, sizeof(T), f);
}

template<integral_type T>
inline bool p_putvar(T const& ptr, PACKFILE *f)
{
	if constexpr (sizeof(T) == 1)
		return p_putc(ptr, f);
	else if constexpr (sizeof(T) == 2)
		return p_iputw(ptr, f);
	else if constexpr (sizeof(T) == 4)
		return p_iputl(ptr, f);
	return pfwrite((char const*)&ptr, sizeof(T), f);
}

template<uint_type Sz,typename T>
inline bool p_getvar(bounded_vec<Sz,T>* ptr, PACKFILE *f)
{
	return p_getbvec(ptr,f);
}
template<uint_type Sz,typename T>
inline bool p_putvar(bounded_vec<Sz,T> const& ptr, PACKFILE *f)
{
	return p_putbvec(ptr,f);
}

template<uint_type Sz,typename T>
inline bool p_getvar(bounded_map<Sz,T>* ptr, PACKFILE *f)
{
	return p_getbmap(ptr,f);
}
template<uint_type Sz,typename T>
inline bool p_putvar(bounded_map<Sz,T> const& ptr, PACKFILE *f)
{
	return p_putbmap(ptr,f);
}

template<typename T>
inline bool p_getvar(vector<T>* ptr, PACKFILE *f)
{
	return p_getlvec(ptr,f);
}
template<typename T>
inline bool p_putvar(vector<T> const& ptr, PACKFILE *f)
{
	return p_putlvec(ptr,f);
}

template<typename K, typename V>
inline bool p_getvar(map<K, V>* ptr, PACKFILE *f)
{
	return p_getlmap(ptr,f);
}
template<typename K, typename V>
inline bool p_putvar(map<K, V> const& ptr, PACKFILE *f)
{
	return p_putlmap(ptr,f);
}

template<typename T>
inline bool p_getvar(std::set<T>* ptr, PACKFILE *f)
{
	return p_getlset(ptr,f);
}
template<typename T>
inline bool p_putvar(std::set<T> const& ptr, PACKFILE *f)
{
	return p_putlset(ptr,f);
}

template<typename T>
inline bool p_getvar(bitstring* ptr, PACKFILE *f)
{
	return p_getbitstr(ptr,f);
}
template<typename T>
inline bool p_putvar(bitstring const& ptr, PACKFILE *f)
{
	return p_putbitstr(ptr,f);
}

template<typename T, size_t Sz>
inline bool p_getvar(std::array<T,Sz>* ptr, PACKFILE *f)
{
	return p_getarr(ptr,f);
}
template<typename T, size_t Sz>
inline bool p_putvar(std::array<T,Sz> const& ptr, PACKFILE *f)
{
	return p_putarr(ptr,f);
}

template<typename A, typename B>
inline bool p_getvar(std::pair<A,B>* ptr, PACKFILE *f)
{
	return p_getpair(ptr,f);
}
template<typename A, typename B>
inline bool p_putvar(std::pair<A,B> const& ptr, PACKFILE *f)
{
	return p_putpair(ptr,f);
}
template<typename A, typename B>
inline bool p_getvar(def_pair<A,B>* ptr, PACKFILE *f)
{
	return p_getpair(ptr,f);
}
template<typename A, typename B>
inline bool p_putvar(def_pair<A,B> const& ptr, PACKFILE *f)
{
	return p_putpair(ptr,f);
}

//

template<uint_type Sz,typename T>
inline bool p_getbvec(bounded_vec<Sz,T> *cont, PACKFILE *f)
{
	cont->clear();
	Sz sz = 0;
	if(!p_getvar(&sz,f))
		return false;
	cont->resize(sz);
	if(sz) //cont found
	{
		Sz count;
		byte pairs;
		if(!p_getc(&pairs,f))
			return false;
		if(!p_getvar(&count,f))
			return false;
		Sz k;
		T v = cont->defval();
		if(pairs)
		{
			while(count--)
			{
				if(!p_getvar(&k,f))
					return false;
				if(!p_getvar(&v,f))
					return false;
				(*cont)[k] = v;
			}
		}
		else
		{
			for(k = 0; k < count; ++k)
			{
				if(!p_getvar(&v,f))
					return false;
				(*cont)[k] = v;
			}
		}
		cont->normalize();
	}
	return true;
}
template<uint_type Sz,typename T>
inline bool p_putbvec(bounded_vec<Sz,T> const& cont, PACKFILE *f)
{
	Sz sz = cont.size();
	if(!p_putvar(sz,f))
		return false;
	if(sz)
	{
		T dt = cont.defval();
		Sz writecnt_v = cont.capacity();
		Sz writecnt_m = 0;
		for(Sz q = 0; q < writecnt_v; ++q)
			if(cont.get(q) != dt)
				++writecnt_m;
		bool pairs = (writecnt_m * (sizeof(T)+sizeof(Sz))) <= writecnt_v * sizeof(T);
		if(!p_putc(pairs ? 1 : 0, f))
			return false;
		if(!p_putvar(pairs ? writecnt_m : writecnt_v, f))
			return false;
		if(pairs)
		{
			for(Sz q = 0; q < writecnt_v; ++q)
			{
				if(cont.get(q) == dt) continue;
				if(!p_putvar(q, f))
					return false;
				if(!p_putvar(cont.get(q), f))
					return false;
			}
		}
		else
		{
			for(Sz q = 0; q < writecnt_v; ++q)
				if(!p_putvar(cont.get(q), f))
					return false;
		}
	}
	return true;
}

template<uint_type Sz,typename T>
inline bool p_getbmap(bounded_map<Sz,T> *cont, PACKFILE *f)
{
	cont->clear();
	Sz sz = 0;
	if(!p_getvar(&sz,f))
		return false;
	cont->resize(sz);
	if(sz) //cont found
	{
		Sz count;
		byte pairs;
		if(!p_getc(&pairs,f))
			return false;
		if(!p_getvar(&count,f))
			return false;
		Sz k;
		T v = cont->defval();
		if(pairs)
		{
			while(count--)
			{
				if(!p_getvar(&k,f))
					return false;
				if(!p_getvar(&v,f))
					return false;
				(*cont)[k] = v;
			}
		}
		else
		{
			for(k = 0; k < count; ++k)
			{
				if(!p_getvar(&v,f))
					return false;
				(*cont)[k] = v;
			}
		}
		cont->normalize();
	}
	return true;
}
template<uint_type Sz,typename T>
inline bool p_putbmap(bounded_map<Sz,T> const& cont, PACKFILE *f)
{
	Sz sz = cont.size();
	if(!p_putvar(sz,f))
		return false;
	if(sz)
	{
		T dt = cont.defval();
		auto lkey = cont.lastKey();
		Sz writecnt_v = lkey ? *lkey+1 : 0;
		Sz writecnt_m = 0;
		for(auto [k,v] : cont.inner())
			if(k < sz && v != dt)
				++writecnt_m;
		bool pairs = (writecnt_m * (sizeof(T)+sizeof(Sz))) <= writecnt_v * sizeof(T);
		if(!p_putc(pairs ? 1 : 0, f))
			return false;
		if(!p_putvar(pairs ? writecnt_m : writecnt_v, f))
			return false;
		if(pairs)
		{
			for(auto [k,v] : cont.inner())
			{
				if(k >= sz || v == dt) continue;
				if(!p_putvar(k, f))
					return false;
				if(!p_putvar(v, f))
					return false;
			}
		}
		else
		{
			for(Sz q = 0; q < writecnt_v; ++q)
				if(!p_putvar(cont.get(q), f))
					return false;
		}
	}
	return true;
}

template<typename T, size_t Sz>
inline bool p_getarr(std::array<T,Sz>* cont, PACKFILE *f)
{
	uint16_t sz;
	if(!p_igetw(&sz,f))
		return false;
	for(size_t q = 0; q < sz && q < Sz; ++q)
		if(!p_getvar(&((*cont)[q]), f))
			return false;
	return true;
}
template<typename T, size_t Sz>
inline bool p_putarr(std::array<T,Sz> const& cont, PACKFILE *f)
{
	if(!p_iputw(Sz,f))
		return false;
	for(size_t q = 0; q < Sz; ++q)
		if(!p_putvar(cont[q], f))
			return false;
	return true;
}

template<typename A, typename B>
inline bool p_getpair(std::pair<A,B>* cont, PACKFILE *f)
{
	if(!p_getvar(&(cont->first), f))
		return false;
	if(!p_getvar(&(cont->second), f))
		return false;
	return true;
}
template<typename A, typename B>
inline bool p_putpair(std::pair<A,B> const& cont, PACKFILE *f)
{
	if(!p_putvar(cont.first, f))
		return false;
	if(!p_putvar(cont.second, f))
		return false;
	return true;
}

template<typename A, typename B>
inline bool p_getpair(def_pair<A,B>* cont, PACKFILE *f)
{
	if(!p_getvar(&(cont->first), f))
		return false;
	if(!p_getvar(&(cont->second), f))
		return false;
	return true;
}
template<typename A, typename B>
inline bool p_putpair(def_pair<A,B> const& cont, PACKFILE *f)
{
	if(!p_putvar(cont.first, f))
		return false;
	if(!p_putvar(cont.second, f))
		return false;
	return true;
}
//

template<typename T>
inline bool p_getcvec(vector<T> *vec, PACKFILE *f)
{
	vec->clear();
	byte sz = 0;
	if(!p_getc(&sz,f))
		return false;
	if(sz) //vec found
	{
		vec->reserve(sz);
		T dummy = T();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&dummy,f))
				return false;
			vec->push_back(dummy);
		}
	}
	return true;
}
template<typename T>
inline bool p_putcvec(vector<T> const& vec, PACKFILE *f)
{
	byte sz = byte(zc_min(255,vec.size()));
	if(!p_putc(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putvar(vec.at(q), f))
				return false;
		}
	}
	return true;
}
template<typename T>
inline bool p_getwvec(vector<T> *vec, PACKFILE *f)
{
	vec->clear();
	word sz = 0;
	if(!p_igetw(&sz,f))
		return false;
	if(sz) //vec found
	{
		vec->reserve(sz);
		T dummy = T();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&dummy,f))
				return false;
			vec->push_back(dummy);
		}
	}
	return true;
}
template<typename T>
inline bool p_putwvec(vector<T> const& vec, PACKFILE *f)
{
	word sz = word(zc_min(65535,vec.size()));
	if(!p_iputw(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putvar(vec.at(q), f))
				return false;
		}
	}
	return true;
}
template<typename T>
inline bool p_getlvec(vector<T> *vec, PACKFILE *f)
{
	vec->clear();
	dword sz = 0;
	if(!p_igetl(&sz,f))
		return false;
	if(sz) //vec found
	{
		vec->reserve(sz);
		T dummy = T();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&dummy,f))
				return false;
			vec->push_back(dummy);
		}
	}
	return true;
}
template<typename T>
inline bool p_putlvec(vector<T> const& vec, PACKFILE *f)
{
	dword sz = vec.size();
	if(!p_iputl(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putvar(vec.at(q), f))
				return false;
		}
	}
	return true;
}

template<typename K, typename V>
inline bool p_getcmap(map<K, V> *mp, PACKFILE *f)
{
	mp->clear();
	byte sz = 0;
	if(!p_getc(&sz,f))
		return false;
	if(sz) //map found
	{
		K key = K();
		V val = V();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&key,f))
				return false;
			if(!p_getvar(&val,f))
				return false;
			(*mp)[key] = val;
		}
	}
	return true;
}
template<typename K, typename V>
inline bool p_putcmap(map<K, V> const& mp, PACKFILE *f)
{
	byte sz = byte(zc_min(255,mp.size()));
	if(!p_putc(sz,f))
		return false;
	if(sz)
	{
		for (auto& [k, v] : mp)
		{
			if(!p_putvar(k, f))
				return false;
			if(!p_putvar(v, f))
				return false;
			if (!--sz) break;
		}
	}
	return true;
}
template<typename K, typename V>
inline bool p_getwmap(map<K, V> *mp, PACKFILE *f)
{
	mp->clear();
	word sz = 0;
	if(!p_igetw(&sz,f))
		return false;
	if(sz) //map found
	{
		K key = K();
		V val = V();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&key,f))
				return false;
			if(!p_getvar(&val,f))
				return false;
			(*mp)[key] = val;
		}
	}
	return true;
}
template<typename K, typename V>
inline bool p_putwmap(map<K, V> const& mp, PACKFILE *f)
{
	word sz = word(zc_min(65535,mp.size()));
	if(!p_iputw(sz,f))
		return false;
	if(sz)
	{
		for (auto& [k, v] : mp)
		{
			if(!p_putvar(k, f))
				return false;
			if(!p_putvar(v, f))
				return false;
			if (!--sz) break;
		}
	}
	return true;
}
template<typename K, typename V>
inline bool p_getlmap(map<K, V> *mp, PACKFILE *f)
{
	mp->clear();
	dword sz = 0;
	if(!p_igetl(&sz,f))
		return false;
	if(sz) //map found
	{
		K key = K();
		V val = V();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&key,f))
				return false;
			if(!p_getvar(&val,f))
				return false;
			(*mp)[key] = val;
		}
	}
	return true;
}
template<typename K, typename V>
inline bool p_putlmap(map<K, V> const& mp, PACKFILE *f)
{
	dword sz = mp.size();
	if(!p_iputl(sz,f))
		return false;
	if(sz)
	{
		for (auto& [k, v] : mp)
		{
			if(!p_putvar(k, f))
				return false;
			if(!p_putvar(v, f))
				return false;
			if (!--sz) break;
		}
	}
	return true;
}

template<typename T>
inline bool p_getcset(std::set<T> *s, PACKFILE *f)
{
	s->clear();
	byte sz = 0;
	if(!p_getc(&sz,f))
		return false;
	if(sz) //set found
	{
		T dummy = T();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&dummy,f))
				return false;
			s->insert(dummy);
		}
	}
	return true;
}
template<typename T>
inline bool p_putcset(std::set<T> const& s, PACKFILE *f)
{
	byte sz = byte(zc_min(255,s.size()));
	if(!p_putc(sz,f))
		return false;
	if(sz)
	{
		for(T const& val : s)
		{
			if(!p_putvar(val, f))
				return false;
			if (!--sz) break;
		}
	}
	return true;
}
template<typename T>
inline bool p_getwset(std::set<T> *s, PACKFILE *f)
{
	s->clear();
	word sz = 0;
	if(!p_igetw(&sz,f))
		return false;
	if(sz) //set found
	{
		T dummy = T();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&dummy,f))
				return false;
			s->insert(dummy);
		}
	}
	return true;
}
template<typename T>
inline bool p_putwset(std::set<T> const& s, PACKFILE *f)
{
	word sz = word(zc_min(65535,s.size()));
	if(!p_iputw(sz,f))
		return false;
	if(sz)
	{
		for(T const& val : s)
		{
			if(!p_putvar(val, f))
				return false;
			if (!--sz) break;
		}
	}
	return true;
}
template<typename T>
inline bool p_getlset(std::set<T> *s, PACKFILE *f)
{
	s->clear();
	dword sz = 0;
	if(!p_igetl(&sz,f))
		return false;
	if(sz) //set found
	{
		T dummy = T();
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&dummy,f))
				return false;
			s->insert(dummy);
		}
	}
	return true;
}
template<typename T>
inline bool p_putlset(std::set<T> const& s, PACKFILE *f)
{
	dword sz = s.size();
	if(!p_iputl(sz,f))
		return false;
	if(sz)
	{
		for(T const& val : s)
		{
			if(!p_putvar(val, f))
				return false;
			if (!--sz) break;
		}
	}
	return true;
}

#endif
