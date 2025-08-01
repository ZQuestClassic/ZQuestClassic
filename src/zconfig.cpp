#include "base/zc_alleg.h"
#include "zconfig.h"
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
void zc_set_config(char const* header, char const* name, double default_val, App a)
{
	zc_push_config();
	zc_config_file(get_config_file_name(a));
	set_config_float(header, name, default_val);
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
void zc_set_config_basic(char const* header, char const* name, double default_val)
{
	set_config_float(header, name, default_val);
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
