#ifndef _PACKFILE_H_
#define _PACKFILE_H_

#include "base/zc_alleg.h"
#include "base/ints.h"
#include "base/general.h"
#include "base/containers.h"
#include "base/zfix.h"
#include <string>
#include <vector>
#include <memory>

#define NEWALLEGRO

extern int32_t readsize, writesize;
extern bool fake_pack_writing;
#define new_return(x) {assert(x == 0); fake_pack_writing = false; return x; }

bool pfwrite(const char *p,int32_t n,PACKFILE *f);

INLINE bool pfwrite(const void *p,int32_t n,PACKFILE *f)
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

INLINE bool pfread(void *p,int32_t n,PACKFILE *f)
{
	bool success;
	
	success=(pack_fread(p,n,f)==n);
	
	if(success)
	{
		readsize+=n;
	}
	
	return success;
}

INLINE bool p_getc(void *p,PACKFILE *f)
{
	uint8_t *cp = (uint8_t *)p;
	int32_t c;
	
	if(!f) return false;
	
#ifdef NEWALLEGRO
	
	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
#else
	
	if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
	
#endif
	
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

INLINE bool p_putc(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		
#ifdef NEWALLEGRO
		
		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
#else
		
		if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
		
#endif
		
		pack_putc(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=1;
	}
	
	return success;
}

INLINE bool p_igetw(void *p,PACKFILE *f)
{
	int16_t *cp = (int16_t *)p;
	int32_t c;
	
	if(!f) return false;
	
#ifdef NEWALLEGRO
	
	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
#else
	
	if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
	
#endif
	
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

INLINE bool p_iputw(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		
#ifdef NEWALLEGRO
		
		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
#else
		
		if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
		
#endif
		
		pack_iputw(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=2;
	}
	
	return success;
}

INLINE bool p_igetl(void *p,PACKFILE *f)
{
	dword *cp = (dword *)p;
	int32_t c;
	
	if(!f) return false;
	
#ifdef NEWALLEGRO
	
	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
#else
	
	if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
	
#endif
	
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

INLINE bool p_igetzf(void *p,PACKFILE *f)
{
	zfix *cp = (zfix *)p;
	int32_t c;
	
	if(!f) return false;
	
#ifdef NEWALLEGRO
	
	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
#else
	
	if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
	
#endif
	
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

INLINE bool p_igetd(void *p, PACKFILE *f)
{
	int32_t temp;
	bool result = p_igetl(&temp,f);
	*(int32_t *)p=(int32_t)temp;
	return result;
}

// Floats are not serializable, do not use!
INLINE bool p_igetf_DO_NOT_USE(void *p,PACKFILE *f)
{
	if(!f) return false;
	
#ifdef NEWALLEGRO
	
	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
#else
	
	if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
	
#endif
	
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

INLINE bool p_iputl(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		
#ifdef NEWALLEGRO
		
		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
#else
		
		if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
		
#endif
		
		pack_iputl(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=4;
	}
	
	return success;
}

INLINE bool p_iputzf(zfix const& c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		
#ifdef NEWALLEGRO
		
		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
#else
		
		if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
		
#endif
		
		pack_iputl(c.getZLong(),f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=4;
	}
	
	return success;
}

INLINE bool p_mgetw(void *p,PACKFILE *f)
{
	int16_t *cp = (int16_t *)p;
	int32_t c;
	
	if(!f) return false;
	
#ifdef NEWALLEGRO
	
	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
#else
	
	if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
	
#endif
	
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

INLINE bool p_mputw(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		
#ifdef NEWALLEGRO
		
		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
#else
		
		if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
		
#endif
		
		pack_mputw(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=2;
	}
	
	return success;
}

INLINE bool p_mgetl(void *p,PACKFILE *f)
{
	dword *cp = (dword *)p;
	int32_t c;
	
	if(!f) return false;
	
#ifdef NEWALLEGRO
	
	if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
	
#else
	
	if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
	
#endif
	
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

INLINE bool p_mputl(int32_t c,PACKFILE *f)
{
	bool success=true;
	
	if(!fake_pack_writing)
	{
		if(!f) return false;
		
#ifdef NEWALLEGRO
		
		if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
		
#else
		
		if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
		
#endif
		
		pack_mputl(c,f);
		success=(pack_ferror(f)==0);
	}
	
	if(success)
	{
		writesize+=4;
	}
	
	return success;
}

INLINE bool p_getstr(std::string *str, size_t sz, PACKFILE *f)
{
	auto buf = std::make_unique<char[]>(sz + 1);
	buf[sz] = '\0';
	if (!pfread(buf.get(), sz, f))
		return false;
	*str = buf.get();
	return true;
}

INLINE bool p_getcstr(std::string *str, PACKFILE *f);
INLINE bool p_putcstr(std::string const& str, PACKFILE *f);
INLINE bool p_getwstr(std::string *str, PACKFILE *f);
INLINE bool p_putwstr(std::string const& str, PACKFILE *f);
template<typename T>
INLINE bool p_getcvec(std::vector<T> *vec, PACKFILE *f);
template<typename T>
INLINE bool p_putcvec(std::vector<T> const& vec, PACKFILE *f);
template<typename T>
INLINE bool p_getwvec(std::vector<T> *vec, PACKFILE *f);
template<typename T>
INLINE bool p_putwvec(std::vector<T> const& vec, PACKFILE *f);
template<typename T>
INLINE bool p_getlvec(std::vector<T> *vec, PACKFILE *f);
template<typename T>
INLINE bool p_putlvec(std::vector<T> const& vec, PACKFILE *f);

template<typename Sz,typename T>
INLINE bool p_getbvec(bounded_vec<Sz,T> *cont, PACKFILE *f);
template<typename Sz,typename T>
INLINE bool p_putbvec(bounded_vec<Sz,T> const& cont, PACKFILE *f);
template<typename Sz,typename T>
INLINE bool p_getbmap(bounded_map<Sz,T> *cont, PACKFILE *f);
template<typename Sz,typename T>
INLINE bool p_putbmap(bounded_map<Sz,T> const& cont, PACKFILE *f);

template<typename T>
INLINE bool p_getvar(T* ptr, PACKFILE *f)
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
INLINE bool p_putvar(T const& ptr, PACKFILE *f)
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
INLINE bool p_getvar(bounded_vec<Sz,T>* ptr, PACKFILE *f)
{
	return p_getbvec(ptr,f);
}
template<typename Sz,typename T>
INLINE bool p_putvar(bounded_vec<Sz,T> const& ptr, PACKFILE *f)
{
	return p_putbvec(ptr,f);
}

template<typename Sz,typename T>
INLINE bool p_getvar(bounded_map<Sz,T>* ptr, PACKFILE *f)
{
	return p_getbmap(ptr,f);
}
template<typename Sz,typename T>
INLINE bool p_putvar(bounded_map<Sz,T> const& ptr, PACKFILE *f)
{
	return p_putbmap(ptr,f);
}

template<typename T>
INLINE bool p_getvar(std::vector<T>* ptr, PACKFILE *f)
{
	return p_getlvec(ptr,f);
}
template<typename T>
INLINE bool p_putvar(std::vector<T> const& ptr, PACKFILE *f)
{
	return p_putlvec(ptr,f);
}

//

template<typename Sz,typename T>
INLINE bool p_getbvec(bounded_vec<Sz,T> *cont, PACKFILE *f)
{
	cont->clear();
	Sz sz = 0;
	if(!p_getvar(&sz,f))
		return false;
	cont->resize(sz);
	if(!p_getvar(&sz,f))
		return false;
	if(sz) //cont found
	{
		cont->reserve(sz);
		T dummy;
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getvar(&dummy,f))
				return false;
			(*cont)[q] = dummy;
		}
		cont->normalize();
	}
	return true;
}
template<typename Sz,typename T>
INLINE bool p_putbvec(bounded_vec<Sz,T> const& cont, PACKFILE *f)
{
	Sz sz = cont.size();
	if(!p_putvar(sz,f))
		return false;
	sz = cont.capacity();
	if(!p_putvar(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putvar(cont.at(q), f))
				return false;
		}
	}
	return true;
}

template<typename Sz,typename T>
INLINE bool p_getbmap(bounded_map<Sz,T> *cont, PACKFILE *f)
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
		T v;
		if(pairs)
		{
			while(count--)
			{
				if(!p_getvar(&k,f))
					return false;
				if(!p_getvar(&v,f))
					return false;
				cont[k] = v;
			}
		}
		else
		{
			for(k = 0; k < count; ++k)
			{
				if(!p_getvar(&v,f))
					return false;
				cont[k] = v;
			}
		}
		cont->normalize();
	}
	return true;
}
template<typename Sz,typename T>
INLINE bool p_putbmap(bounded_map<Sz,T> const& cont, PACKFILE *f)
{
	Sz sz = cont.size();
	if(!p_putvar(sz,f))
		return false;
	if(sz)
	{
		T dt;
		auto lkey = cont.lastKey();
		Sz writecnt = lkey ? *lkey+1 : 0;
		Sz cap = 0;
		for(auto [k,v] : cont.inner())
			if(k < sz && v != dt)
				++cap;
		bool pairs = (cap * (sizeof(T)+sizeof(Sz))) <= writecnt * sizeof(T);
		if(!p_putc(pairs ? 1 : 0, f))
			return false;
		if(!p_putvar(pairs ? cap : writecnt, f))
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
			for(Sz q = 0; q < writecnt; ++q)
				if(!p_putvar(cont[q], f))
					return false;
		}
	}
	return true;
}

//

// Reads `sz` bytes from `f` into `str`.
// `str` should be `sz + 1` bytes long.
// `str` will always be a null-terminated string.
INLINE bool p_getcstr(char *str, size_t sz, PACKFILE *f)
{
	size_t read = pack_fread(str, sz, f);
	bool success = read == sz;
	str[read] = '\0';
	if (success)
		readsize += read;
	return success;
}

INLINE bool p_getcstr(std::string *str, PACKFILE *f)
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
INLINE bool p_putcstr(std::string const& str, PACKFILE *f)
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
INLINE bool p_getwstr(std::string *str, PACKFILE *f)
{
	str->clear();
	word sz = 0;
	if(!p_igetw(&sz,f))
		return false;
	if(sz)
	{
		str->reserve(sz);
		auto buf = std::make_unique<char[]>(sz + 1);
		buf[sz] = '\0';
		if (!pfread(buf.get(), sz, f))
			return false;
		*str = buf.get();
	}
	return true;
}
INLINE bool p_putwstr(std::string const& str, PACKFILE *f)
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

template<typename T>
INLINE bool p_getcvec(std::vector<T> *vec, PACKFILE *f)
{
	vec->clear();
	byte sz = 0;
	if(!p_getc(&sz,f))
		return false;
	if(sz) //vec found
	{
		vec->reserve(sz);
		T dummy;
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
INLINE bool p_putcvec(std::vector<T> const& vec, PACKFILE *f)
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
INLINE bool p_getwvec(std::vector<T> *vec, PACKFILE *f)
{
	vec->clear();
	word sz = 0;
	if(!p_igetw(&sz,f))
		return false;
	if(sz) //vec found
	{
		vec->reserve(sz);
		T dummy;
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
INLINE bool p_putwvec(std::vector<T> const& vec, PACKFILE *f)
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
INLINE bool p_getlvec(std::vector<T> *vec, PACKFILE *f)
{
	vec->clear();
	dword sz = 0;
	if(!p_igetl(&sz,f))
		return false;
	if(sz) //vec found
	{
		vec->reserve(sz);
		T dummy;
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
INLINE bool p_putlvec(std::vector<T> const& vec, PACKFILE *f)
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

