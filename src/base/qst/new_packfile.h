#pragma once

bool new_fread(void* ptr, size_t size, ALLEGRO_FILE* fp)
{
	return al_fread(fp, ptr, size) == size;
}
bool new_fwrite(void const* ptr, size_t size, ALLEGRO_FILE* fp)
{
	return al_fwrite(fp, ptr, size);
}

template<typename T>
bool new_getv(T& ref, ALLEGRO_FILE* fp)
{
	return new_fread(&ref, sizeof(T), fp);
}
template<typename T>
bool new_putv(T const& ref, ALLEGRO_FILE* fp)
{
	return new_fwrite(&ref, sizeof(T), fp);
}

bool new_getc(bool& ref, ALLEGRO_FILE* fp)
{
	return new_getv(ref, fp);
}
bool new_getc(uint8_t& ref, ALLEGRO_FILE* fp)
{
	return new_getv(ref, fp);
}
bool new_getc(int8_t& ref, ALLEGRO_FILE* fp)
{
	return new_getv(ref, fp);
}
bool new_getw(uint16_t& ref, ALLEGRO_FILE* fp)
{
	return new_getv(ref, fp);
}
bool new_getw(int16_t& ref, ALLEGRO_FILE* fp)
{
	return new_getv(ref, fp);
}
bool new_getl(uint32_t& ref, ALLEGRO_FILE* fp)
{
	return new_getv(ref, fp);
}
bool new_getl(int32_t& ref, ALLEGRO_FILE* fp)
{
	return new_getv(ref, fp);
}

bool new_putb(bool val, ALLEGRO_FILE* fp)
{
	return new_putv(val ? 1 : 0, fp);
}
bool new_putc(uint8_t val, ALLEGRO_FILE* fp)
{
	return new_putv(val, fp);
}
bool new_putw(uint16_t val, ALLEGRO_FILE* fp)
{
	return new_putv(val, fp);
}
bool new_putl(uint32_t val, ALLEGRO_FILE* fp)
{
	return new_putv(val, fp);
}

bool new_getcstr(string& str, ALLEGRO_FILE* fp)
{
	str.clear();
	uint8_t sz;
	if (!new_getv(sz, fp))
		return false;
	str.resize(sz);
	return new_fread(str.data(), sz, fp);
}
bool new_getwstr(string& str, ALLEGRO_FILE* fp)
{
	str.clear();
	uint16_t sz;
	if (!new_getv(sz, fp))
		return false;
	str.resize(sz);
	return new_fread(str.data(), sz, fp);
}
bool new_getlstr(string& str, ALLEGRO_FILE* fp)
{
	str.clear();
	uint32_t sz;
	if (!new_getv(sz, fp))
		return false;
	str.resize(sz);
	return new_fread(str.data(), sz, fp);
}

bool new_putcstr(string const& str, ALLEGRO_FILE* fp)
{
	size_t sz = str.size();
	if (sz > (1<<8)-1)
		return false;
	if (!new_putc(sz, fp))
		return false;
	return new_fwrite(str.data(), sz, fp);
}
bool new_putwstr(string const& str, ALLEGRO_FILE* fp)
{
	size_t sz = str.size();
	if (sz > (1<<16)-1)
		return false;
	if (!new_putw(sz, fp))
		return false;
	return new_fwrite(str.data(), sz, fp);
}
bool new_putlstr(string const& str, ALLEGRO_FILE* fp)
{
	size_t sz = str.size();
	if (!new_putl(sz, fp))
		return false;
	return new_fwrite(str.data(), sz, fp);
}

