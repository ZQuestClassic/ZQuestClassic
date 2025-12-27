#include "base/zc_alleg.h"
#include "zconfig.h"
#include "base/zsys.h"
#include <string>

#if defined(IS_PLAYER) or defined(IS_EDITOR)
#include "base/jwinfsel.h"
#endif

char const* get_config_file_name(App a)
{
	if(a == App::undefined)
		a = get_app_id();
	switch (a)
	{
#ifdef __EMSCRIPTEN__
		case App::zelda: return "local/zc.cfg";
		case App::zquest: return "local/zquest.cfg";
		case App::zscript: return "local/zscript.cfg";
#else
		case App::zelda: return "zc.cfg";
		case App::zquest: return "zquest.cfg";
		case App::zscript: return "zscript.cfg";
#endif
		case App::launcher: return "zcl.cfg";
		default: abort();
	}
}
char const* get_config_base_name(App a)
{
	if(a == App::undefined)
		a = get_app_id();
	switch (a)
	{
		case App::zelda: return "base_config/zc.cfg";
		case App::zquest: return "base_config/zquest.cfg";
		case App::zscript: return "base_config/zscript.cfg";
		case App::launcher: return "base_config/zcl.cfg";
		default: abort();
	}
}

void zc_config_file(char const* fl)
{
	flush_config_file();
	set_config_file(fl);
}

void zc_push_config()
{
	push_config_state();
}

void zc_pop_config()
{
	flush_config_file();
	pop_config_state();
}

std::string qst_cfg_header_from_path(std::string path)
{
#if defined(IS_PLAYER) or defined(IS_EDITOR)
	path = relativize_path(path);
	util::replchar(path, '[', '_');
	util::replchar(path, ']', '_');
	util::replchar(path, ' ', '_');
	util::replchar(path, '\\', '/');
	return path;
#else
	return "";
#endif
}

static char cfg_str[2048];
bool zc_cfg_defaulted = false;

int32_t zc_get_config(char const* header, char const* name, int32_t default_val, App a)
{
	zc_push_config();
	zc_config_file(get_config_base_name(a));
	auto def = get_config_int(header,name,default_val);
	zc_config_file(get_config_file_name(a));
	auto ret = get_config_int(header,name,def);
	zc_cfg_defaulted = get_config_defaulted();
	zc_pop_config();
	return ret;
}
int32_t zc_get_config_hex(char const* header, char const* name, int32_t default_val, App a)
{
	zc_push_config();
	zc_config_file(get_config_base_name(a));
	auto def = get_config_hex(header,name,default_val);
	zc_config_file(get_config_file_name(a));
	auto ret = get_config_hex(header,name,def);
	zc_cfg_defaulted = get_config_defaulted();
	zc_pop_config();
	return ret;
}
double zc_get_config(char const* header, char const* name, double default_val, App a)
{
	zc_push_config();
	zc_config_file(get_config_base_name(a));
	auto def = get_config_float(header,name,default_val);
	zc_config_file(get_config_file_name(a));
	auto ret = get_config_float(header,name,def);
	zc_cfg_defaulted = get_config_defaulted();
	zc_pop_config();
	return ret;
}
char const* zc_get_config(char const* header, char const* name, char const* default_val, App a)
{
	zc_push_config();
	zc_config_file(get_config_base_name(a));
	std::string def = get_config_string(header,name,default_val?default_val:"");
	zc_config_file(get_config_file_name(a));
	char const* ret = get_config_string(header,name,def.c_str());
	zc_cfg_defaulted = get_config_defaulted();
	if(!ret)
		cfg_str[0] = 0;
	else strcpy(cfg_str, ret);
	zc_pop_config();
	return cfg_str;
}
void zc_set_config(char const* header, char const* name, int32_t val, App a)
{
	zc_push_config();
	zc_config_file(get_config_file_name(a));
	set_config_int(header,name,val);
	zc_pop_config();
}
void zc_set_config_hex(char const* header, char const* name, int32_t val, App a)
{
	zc_push_config();
	zc_config_file(get_config_file_name(a));
	set_config_hex(header,name,val);
	zc_pop_config();
}
void zc_set_config(char const* header, char const* name, double val, App a)
{
	zc_push_config();
	zc_config_file(get_config_file_name(a));
	set_config_float(header, name, val);
	zc_pop_config();
}
void zc_set_config(char const* header, char const* name, char const* val, App a)
{
	zc_push_config();
	zc_config_file(get_config_file_name(a));
	set_config_string(header,name,val);
	zc_pop_config();
}

int32_t zc_get_config_basic(char const* header, char const* name, int32_t default_val)
{
	return get_config_int(header,name,default_val);;
}
int32_t zc_get_config_basic_hex(char const* header, char const* name, int32_t default_val)
{
	return get_config_hex(header,name,default_val);;
}
double zc_get_config_basic(char const* header, char const* name, double default_val)
{
	return get_config_float(header,name,default_val);
}
char const* zc_get_config_basic(char const* header, char const* name, char const* default_val)
{
	char const* ret = get_config_string(header,name,default_val);
	if(!ret)
		cfg_str[0] = 0;
	else strcpy(cfg_str, ret);
	return cfg_str;
}
void zc_set_config_basic(char const* header, char const* name, int32_t val)
{
	set_config_int(header,name,val);
}
void zc_set_config_basic_hex(char const* header, char const* name, int32_t val)
{
	set_config_hex(header,name,val);
}
void zc_set_config_basic(char const* header, char const* name, double val)
{
	set_config_float(header, name, val);
}
void zc_set_config_basic(char const* header, char const* name, char const* val)
{
	set_config_string(header,name,val);
}

bool is_feature_enabled(const char* flag_switch, const char* config_header, const char* config_name, bool def)
{
	auto flag = get_flag_bool(flag_switch);
	bool enabled = flag.value_or(zc_get_config(config_header, config_name, def));
	return enabled;
}


zc_a5_cfg::zc_a5_cfg()
	: data(nullptr), path(), dirty(false)
{}

zc_a5_cfg::zc_a5_cfg(string const& path)
	: data(nullptr), path(path), dirty(true)
{
	if (!load())
		data = al_create_config();
}
zc_a5_cfg::~zc_a5_cfg()
{
	destroy();
}

void zc_a5_cfg::destroy()
{
	if (data)
	{
		save();
		al_destroy_config(data);
		data = nullptr;
		dirty = false;
		path.clear();
	}
}

bool zc_a5_cfg::is_valid() const
{
	return data != nullptr;
}

void zc_a5_cfg::re_load(string const& new_path)
{
	path = new_path;
	dirty = true;
	if (data)
	{
		al_destroy_config(data);
		data = nullptr;
	}
	if (!load())
		data = al_create_config();
}

bool zc_a5_cfg::load()
{
	if (path.empty()) return false;
	dirty = true;
	if (data) al_destroy_config(data);
	data = al_load_config_file(path.c_str());
	if (!data)
		return false;
	dirty = false;
	return true;
}

bool zc_a5_cfg::save()
{
	if (path.empty()) return false;
	if (!data) return false;
	if (!dirty) return true; // nothing to save
	if (al_save_config_file(path.c_str(), data))
	{
		dirty = false;
		return true;
	}
	Z_error("Failed to save config file '%s'!\n", path.c_str());
	return false;
}

void zc_a5_cfg::add_comment(const char* header, const char* comment)
{
	if (!data) return;
	dirty = true;
	al_add_config_comment(data, header, comment);
}

void zc_a5_cfg::add_section(const char* header)
{
	if (!data) return;
	dirty = true;
	al_add_config_section(data, header);
}

bool zc_a5_cfg::remove_section(const char* header)
{
	if (!data) return false;
	if (al_remove_config_section(data, header))
	{
		dirty = true;
		return true;
	}
	return false;
}

bool zc_a5_cfg::remove_key(const char* header, const char* name)
{
	if (!data) return false;
	if (al_remove_config_key(data, header, name))
	{
		dirty = true;
		return true;
	}
	return false;
}

optional<int> zc_a5_cfg::get_config_int(char const* header, char const* name)
{
	if (!data) return std::nullopt;
	const char* str = al_get_config_value(data, header, name);
	if (!str) return std::nullopt;
	return atoi(str);
}
optional<double> zc_a5_cfg::get_config_float(char const* header, char const* name)
{
	if (!data) return std::nullopt;
	const char* str = al_get_config_value(data, header, name);
	if (!str) return std::nullopt;
	return std::stod(str);
}
optional<string> zc_a5_cfg::get_config(char const* header, char const* name)
{
	if (!data) return std::nullopt;
	const char* str = al_get_config_value(data, header, name);
	if (!str) return std::nullopt;
	return str;
}
void zc_a5_cfg::set_config(char const* header, char const* name, int val)
{
	if (!data) return;
	dirty = true;
	al_set_config_value(data, header, name, std::to_string(val).c_str());
}
void zc_a5_cfg::set_config(char const* header, char const* name, double val)
{
	if (!data) return;
	dirty = true;
	al_set_config_value(data, header, name, std::to_string(val).c_str());
}
void zc_a5_cfg::set_config(char const* header, char const* name, char const* val)
{
	if (!data) return;
	if (!val)
	{
		remove_key(header, name);
		return;
	}
	dirty = true;
	al_set_config_value(data, header, name, val);
}

zc_a5_cfg_iterator zc_a5_cfg::iterate_sections()
{
	return zc_a5_cfg_iterator(data);
}
zc_a5_cfg_iterator zc_a5_cfg::iterate_entries(const char* header)
{
	return zc_a5_cfg_iterator(data, header);
}
zc_a5_cfg_iterator::operator bool() const
{
	return is_valid();
}


optional<string> zc_a5_cfg_iterator::operator*()
{
	return peek();
}
zc_a5_cfg_iterator& zc_a5_cfg_iterator::operator++()
{
	advance();
	return *this;
}
zc_a5_cfg_iterator zc_a5_cfg_iterator::operator++(int)
{
	zc_a5_cfg_iterator ret(*this);
	advance();
	return ret;
}
optional<string> zc_a5_cfg_iterator::peek()
{
	return value;
}
bool zc_a5_cfg_iterator::advance()
{
	const char* str = nullptr;
	if (section_it)
		str = al_get_next_config_section(&section_it);
	else if (entry_it)
		str = al_get_next_config_entry(&entry_it);
	if (str)
		value = string(str);
	else value = std::nullopt;
	return is_valid();
}
optional<string> zc_a5_cfg_iterator::next()
{
	auto ret = value;
	advance();
	return ret;
}
bool zc_a5_cfg_iterator::is_valid() const
{
	return value != std::nullopt;
}
zc_a5_cfg_iterator::zc_a5_cfg_iterator(ALLEGRO_CONFIG const *data)
	: section_it(nullptr), entry_it(nullptr), value(std::nullopt)
{
	const char* str = al_get_first_config_section(data, &section_it);
	if (str)
		value = string(str);
	else value = std::nullopt;
}
zc_a5_cfg_iterator::zc_a5_cfg_iterator(ALLEGRO_CONFIG const *data, const char* header)
	: section_it(nullptr), entry_it(nullptr), value(std::nullopt)
{
	const char* str = al_get_first_config_entry(data, header, &entry_it);
	if (str)
		value = string(str);
	else value = std::nullopt;
}

