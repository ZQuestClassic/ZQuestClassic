#ifndef PACKFILE_H_
#define PACKFILE_H_

#include "base/zc_alleg.h"
#include "base/ints.h"
#include "base/general.h"
#include "base/containers.h"
#include "base/zfix.h"
#include <string>
#include <vector>
#include <memory>

extern int32_t readsize, writesize;
extern bool fake_pack_writing;
#define new_return(x) {assert(x == 0); fake_pack_writing = false; return x; }

bool pfwrite(const char *p,int32_t n,PACKFILE *f);

inline bool pfwrite(const void *p,int32_t n,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		success=(pack_fwrite(p,n,f)==n);
	}
	
	if(success)
	{
		writesize+=n;
	}
	
	return success;
}

inline bool pfread(void *p,int n,PACKFILE *f,bool allow_less = false)
{
	auto count = pack_fread(p,n,f);
	bool success = allow_less || count==n;
	
	if(success)
		readsize += count;
	
	return success;
}

inline bool p_getc(void *p,PACKFILE *f)
{
	uint8_t *cp = (uint8_t *)p;
	int32_t c;
	
	if(!f) return false;
	

	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
	if(pack_feof(f))
	{
		return false;
	}
	
	c = pack_getc(f);
	
	if(pack_ferror(f))
	{
		return false;
	}
	
	*cp = c;
	
	readsize+=1;
	return true;
}

inline bool p_putc(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		

		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
		pack_putc(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=1;
	}
	
	return success;
}

inline bool p_igetw(void *p,PACKFILE *f)
{
	int16_t *cp = (int16_t *)p;
	int32_t c;
	
	if(!f) return false;
	

	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
	if(pack_feof(f))
	{
		return false;
	}
	
	c = pack_igetw(f);
	
	if(pack_ferror(f))
	{
		return false;
	}
	
	*cp = c;
	
	readsize+=2;
	return true;
}

inline bool p_iputw(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		

		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
		pack_iputw(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=2;
	}
	
	return success;
}

inline bool p_igetl(void *p,PACKFILE *f)
{
	dword *cp = (dword *)p;
	int32_t c;
	
	if(!f) return false;
	

	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
	if(pack_feof(f))
	{
		return false;
	}
	
	c = pack_igetl(f);
	
	if(pack_ferror(f))
	{
		return false;
	}
	
	*cp = c;
	
	readsize+=4;
	return true;
}

inline bool p_igetzf(void *p,PACKFILE *f)
{
	zfix *cp = (zfix *)p;
	int32_t c;
	
	if(!f) return false;
	

	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
	if(pack_feof(f))
	{
		return false;
	}
	
	c = pack_igetl(f);
	
	if(pack_ferror(f))
	{
		return false;
	}
	
	*cp = zslongToFix(c);
	
	readsize+=4;
	return true;
}

inline bool p_igetd(void *p, PACKFILE *f)
{
	int32_t temp;
	bool result = p_igetl(&temp,f);
	*(int32_t *)p=(int32_t)temp;
	return result;
}

// Floats are not serializable, do not use!
inline bool p_igetf_DO_NOT_USE(void *p,PACKFILE *f)
{
	if(!f) return false;
	

	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
	if(pack_feof(f))
	{
		return false;
	}
	
	byte tempfloat[sizeof(float)];
	
	if(!pfread(tempfloat,sizeof(float),f))
		return false;
		
	memset(p, 0,sizeof(float));
#ifdef ALLEGRO_MACOSX
	
	for(int32_t i=0; i<(int32_t)sizeof(float); i++)
	{
		((byte *)p)[i] = tempfloat[i];
	}
	
#else
	
	for(int32_t i=0; i<(int32_t)sizeof(float); i++)
	{
		((byte *)p)[sizeof(float)-i-1] = tempfloat[i];
	}
	
#endif
	
	readsize += sizeof(float);
	return true;
}

inline bool p_iputl(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		

		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
		pack_iputl(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=4;
	}
	
	return success;
}

inline bool p_iputzf(zfix const& c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		

		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
		pack_iputl(c.getZLong(),f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=4;
	}
	
	return success;
}

inline bool p_mgetw(void *p,PACKFILE *f)
{
	int16_t *cp = (int16_t *)p;
	int32_t c;
	
	if(!f) return false;
	

	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
	if(pack_feof(f))
	{
		return false;
	}
	
	c = pack_mgetw(f);
	
	if(pack_ferror(f))
	{
		return false;
	}
	
	*cp = c;
	
	readsize+=2;
	return true;
}

inline bool p_mputw(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		

		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
		pack_mputw(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=2;
	}
	
	return success;
}

inline bool p_mgetl(void *p,PACKFILE *f)
{
	dword *cp = (dword *)p;
	int32_t c;
	
	if(!f) return false;
	

	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
	if(pack_feof(f))
	{
		return false;
	}
	
	c = pack_mgetl(f);
	
	if(pack_ferror(f))
	{
		return false;
	}
	
	*cp = c;
	
	readsize+=4;
	return true;
}

inline bool p_mputl(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		

		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
		pack_mputl(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=4;
	}
	
	return success;
}

inline bool p_getstr(char *str, size_t sz, PACKFILE *f)
{
	size_t read = pack_fread(str, sz, f);
	bool success = read == sz;
	str[read] = '\0';
	if (success)
		readsize += read;
	return success;
}

static std::optional<std::unique_ptr<char[]>> allocate_string_buffer(size_t sz)
{
	// A 100+ MB string is far beyond a reasonable size. File must be corrupt.
	if (sz >= 1e+8)
		return std::nullopt;

	auto buf2 = std::make_unique<char[]>(sz + 1);
	buf2[sz] = '\0';
	return buf2;
}

inline bool p_getstr(string *str, size_t sz, PACKFILE *f)
{
	auto buf = allocate_string_buffer(sz);
	if (!buf.has_value())
		return false;

	if (!pfread(buf->get(), sz, f))
		return false;
	*str = buf->get();
	return true;
}
inline bool p_putstr(char const* str, size_t sz, PACKFILE *f)
{
	return pfwrite(str,sz,f);
}
inline bool p_putstr(string const& str, size_t sz, PACKFILE *f)
{
	if(str.size() < sz)
	{
		if(!pfwrite(str.data(),str.size(),f))
			return false;
		for(int q = str.size(); q < sz; ++q)
			if(!p_putc(0,f))
				return false;
		return true;
	}
	return pfwrite(str.data(),sz,f);
}

inline bool p_getcstr(string *str, PACKFILE *f);
inline bool p_putcstr(string const& str, PACKFILE *f);
inline bool p_getwstr(string *str, PACKFILE *f);
inline bool p_putwstr(string const& str, PACKFILE *f);
inline bool p_getlstr(string *str, PACKFILE *f);
inline bool p_putlstr(string const& str, PACKFILE *f);
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

template<typename Sz,typename T>
inline bool p_getbvec(bounded_vec<Sz,T> *cont, PACKFILE *f);
template<typename Sz,typename T>
inline bool p_putbvec(bounded_vec<Sz,T> const& cont, PACKFILE *f);
template<typename Sz,typename T>
inline bool p_getbmap(bounded_map<Sz,T> *cont, PACKFILE *f);
template<typename Sz,typename T>
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

bool p_getbitstr(bitstring* ptr, PACKFILE *f);
bool p_putbitstr(bitstring const& ptr, PACKFILE *f);

template<typename T>
inline bool p_getvar(T* ptr, PACKFILE *f)
{
	switch(auto sz = sizeof(T))
	{
		case 1:
			return p_getc(ptr,f);
		case 2:
			return p_igetw(ptr,f);
		case 4:
			return p_igetl(ptr,f);
		default:
			return pfread((char*)ptr,sz,f);
	}
}

template<typename T>
inline bool p_putvar(T const& ptr, PACKFILE *f)
{
	switch(auto sz = sizeof(T))
	{
		case 1:
			return p_putc(ptr,f);
		case 2:
			return p_iputw(ptr,f);
		case 4:
			return p_iputl(ptr,f);
		default:
			return pfwrite((char const*)&ptr,sz,f);
	}
}


template<typename Sz,typename T>
inline bool p_getvar(bounded_vec<Sz,T>* ptr, PACKFILE *f)
{
	return p_getbvec(ptr,f);
}
template<typename Sz,typename T>
inline bool p_putvar(bounded_vec<Sz,T> const& ptr, PACKFILE *f)
{
	return p_putbvec(ptr,f);
}

template<typename Sz,typename T>
inline bool p_getvar(bounded_map<Sz,T>* ptr, PACKFILE *f)
{
	return p_getbmap(ptr,f);
}
template<typename Sz,typename T>
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

template<typename Sz,typename T>
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
template<typename Sz,typename T>
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
			if(cont[q] != dt)
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
				if(cont[q] == dt) continue;
				if(!p_putvar(q, f))
					return false;
				if(!p_putvar(cont[q], f))
					return false;
			}
		}
		else
		{
			for(Sz q = 0; q < writecnt_v; ++q)
				if(!p_putvar(cont[q], f))
					return false;
		}
	}
	return true;
}

template<typename Sz,typename T>
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
template<typename Sz,typename T>
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
				if(!p_putvar(cont[q], f))
					return false;
		}
	}
	return true;
}

inline bool p_getbitstr(bitstring* ptr, PACKFILE *f)
{
	return p_getbvec(&ptr->inner(),f);
}
inline bool p_putbitstr(bitstring const& ptr, PACKFILE *f)
{
	return p_putbvec(ptr.inner(),f);
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

inline bool p_getcstr(string *str, PACKFILE *f)
{
	byte sz = 0;
	if(!p_getc(&sz,f))
	{
		str->clear();
		return false;
	}

	if(sz) //string found
	{
		str->reserve(sz);
		char dummy;
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getc(&dummy,f))
				return false;
			str->push_back(dummy);
		}
	}

	return true;
}
inline bool p_putcstr(string const& str, PACKFILE *f)
{
	byte sz = byte(zc_min(255,str.size()));
	if(!p_putc(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putc(str.at(q),f))
				return false;
		}
	}
	return true;
}
inline bool p_getwstr(string *str, PACKFILE *f)
{
	str->clear();
	word sz = 0;
	if(!p_igetw(&sz,f))
		return false;
	if(sz)
	{
		str->reserve(sz);
		auto buf = allocate_string_buffer(sz);
		if (!buf.has_value())
			return false;
		if (!pfread(buf->get(), sz, f))
			return false;
		*str = buf->get();
	}
	return true;
}
inline bool p_putwstr(string const& str, PACKFILE *f)
{
	word sz = word(zc_min(65535,str.size()));
	if(!p_iputw(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putc(str.at(q),f))
				return false;
		}
	}
	return true;
}
inline bool p_getlstr(string *str, PACKFILE *f)
{
	str->clear();
	dword sz = 0;
	if(!p_igetl(&sz,f))
		return false;
	if(sz)
	{
		str->reserve(sz);
		auto buf = allocate_string_buffer(sz);
		if (!buf.has_value())
			return false;
		if (!pfread(buf->get(), sz, f))
			return false;
		*str = buf->get();
	}
	return true;
}
inline bool p_putlstr(string const& str, PACKFILE *f)
{
	dword sz = word(zc_min(UINT32_MAX,str.size()));
	if(!p_iputl(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putc(str.at(q),f))
				return false;
		}
	}
	return true;
}

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

#endif

