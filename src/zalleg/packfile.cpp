#include "zalleg/packfile.h"

// Some objects which are stored embedded within containers such as bounded_maps
// need version handling, but cannot have access to the version context they are being
// called from.
// For these objects, declare a 'packfile version', initialized to the current version.
// When loading old quest files, declare a PackfileVersionHandler with the appropriate
// version variable, and the value you want to treat it as. The handler will assign the
// version to the new value for you, and revert it when it is destructed.

int packfile_v_exported_variable = V_EXPORTED_VARIABLE;

PackfileVersionHandler::PackfileVersionHandler(int& ref, int new_ver)
	: ref(ref), old_ver(ref), new_ver(new_ver)
{
	ref = new_ver;
}
PackfileVersionHandler::~PackfileVersionHandler()
{
	ref = old_ver;
}

bool pfwrite(const char *p,int32_t n,PACKFILE *f)
{
	bool success = true;
	
	if (!fake_pack_writing)
	{
		size_t len = strlen(p);
		if (len > n) len = n;
		int zeros = n - len;
		success = (pack_fwrite(p,len,f) == len);
		if (!success)
			return false;

		for (int i = 0; i < zeros; i++)
		{
			if (pack_putc(0, f) == EOF)
				return false;
		}
	}
	
	if (success)
	{
		writesize += n;
	}
	
	return success;
}

bool pfwrite(const void *p,int32_t n,PACKFILE *f)
{
	bool success = true;
	
	if (!fake_pack_writing)
	{
		success = (pack_fwrite(p,n,f) == n);
	}
	
	if (success)
	{
		writesize += n;
	}
	
	return success;
}

bool pfread(void *p,int n,PACKFILE *f,bool allow_less)
{
	auto count = pack_fread(p,n,f);
	bool success = allow_less || count==n;
	
	if(success)
		readsize += count;
	
	return success;
}

bool p_getc(void *p,PACKFILE *f)
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

bool p_putc(int32_t c,PACKFILE *f)
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

bool p_igetw(void *p,PACKFILE *f)
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

bool p_iputw(int32_t c,PACKFILE *f)
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

bool p_igetl(void *p,PACKFILE *f)
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

bool p_igetzf(void *p,PACKFILE *f)
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

bool p_igetd(void *p, PACKFILE *f)
{
	int32_t temp;
	bool result = p_igetl(&temp,f);
	*(int32_t *)p=(int32_t)temp;
	return result;
}

// Floats are not serializable, do not use!
bool p_igetf_DO_NOT_USE(void *p,PACKFILE *f)
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

bool p_iputl(int32_t c,PACKFILE *f)
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

bool p_iputzf(zfix const& c,PACKFILE *f)
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

bool p_mgetw(void *p,PACKFILE *f)
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

bool p_mputw(int32_t c,PACKFILE *f)
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

bool p_mgetl(void *p,PACKFILE *f)
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

bool p_mputl(int32_t c,PACKFILE *f)
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

bool p_getstr(char *str, size_t sz, PACKFILE *f)
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

bool p_getstr(string *str, size_t sz, PACKFILE *f)
{
	auto buf = allocate_string_buffer(sz);
	if (!buf.has_value())
		return false;

	if (!pfread(buf->get(), sz, f))
		return false;
	*str = buf->get();
	return true;
}
bool p_putstr(char const* str, size_t sz, PACKFILE *f)
{
	return pfwrite(str,sz,f);
}
bool p_putstr(string const& str, size_t sz, PACKFILE *f)
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

bool p_getcstr(string *str, PACKFILE *f)
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
bool p_putcstr(string const& str, PACKFILE *f)
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
bool p_getwstr(string *str, PACKFILE *f)
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
bool p_putwstr(string const& str, PACKFILE *f)
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
bool p_getlstr(string *str, PACKFILE *f)
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
bool p_putlstr(string const& str, PACKFILE *f)
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

bool p_getbitstr(bitstring* ptr, PACKFILE *f)
{
	return p_getbvec(&ptr->inner(),f);
}
bool p_putbitstr(bitstring const& ptr, PACKFILE *f)
{
	return p_putbvec(ptr.inner(),f);
}

bool p_getvar(zfix* ptr, PACKFILE *f)
{
	return p_igetzf(ptr, f);
}
bool p_putvar(zfix const& ptr, PACKFILE *f)
{
	return p_iputzf(ptr, f);
}

bool p_getvar(string* ptr, PACKFILE *f)
{
	return p_getlstr(ptr,f);
}
bool p_putvar(string const& ptr, PACKFILE *f)
{
	return p_putlstr(ptr,f);
}


bool p_getvar(exported_variable* ptr, PACKFILE *f)
{
	*ptr = exported_variable();
	if (!p_getcstr(&ptr->name, f))
		return false;
	if (!p_getlstr(&ptr->helptext, f))
		return false;
	if (!p_getc(&ptr->btn_type, f))
		return false;
	if (packfile_v_exported_variable > 0)
	{
		if (!p_igetw(&ptr->engine_type, f))
			return false;
		if (!p_getc(&ptr->export_custom_type, f))
			return false;
		if (!p_getwmap(&ptr->custom_export_names, f))
			return false;
	}
	if (!p_igetzf(&ptr->min, f))
		return false;
	if (!p_igetzf(&ptr->max, f))
		return false;
	return true;
}
bool p_putvar(exported_variable const& ptr, PACKFILE *f)
{
	if (!p_putcstr(ptr.name, f))
		return false;
	if (!p_putlstr(ptr.helptext, f))
		return false;
	if (!p_putc(ptr.btn_type, f))
		return false;
	if (!p_iputw(word(ptr.engine_type), f))
		return false;
	if (!p_putc(byte(ptr.export_custom_type), f))
		return false;
	if (!p_putwmap(ptr.custom_export_names, f))
		return false;
	if (!p_iputzf(ptr.min, f))
		return false;
	if (!p_iputzf(ptr.max, f))
		return false;
	return true;
}

bool p_getvar(script_config_nosavescript* ptr, PACKFILE *f)
{
	if (!p_getarr(&ptr->run_args, f))
		return false;
	if (!p_getwmap(&ptr->inst_init, f))
		return false;
	return true;
}
bool p_putvar(script_config_nosavescript const& ptr, PACKFILE *f)
{
	if (!p_putarr(ptr.run_args, f))
		return false;
	if (!p_putwmap(ptr.inst_init, f))
		return false;
	return true;
}

bool p_getvar(script_config* ptr, PACKFILE *f)
{
	if (!p_igetw(&ptr->script, f))
		return false;
	return p_getvar((script_config_nosavescript*)ptr, f);
}
bool p_putvar(script_config const& ptr, PACKFILE *f)
{
	if (!p_iputw(ptr.script, f))
		return false;
	return p_putvar((script_config_nosavescript const&)ptr, f);
}

