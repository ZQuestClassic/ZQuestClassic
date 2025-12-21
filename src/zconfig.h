#ifndef ZC_ZCONFIG_H
#define ZC_ZCONFIG_H

#include "base/zapp.h"
#include <optional>

char const* get_config_file_name(App a = App::undefined);
char const* get_config_base_name(App a = App::undefined);
void zc_config_file(char const* fl);
void zc_push_config();
void zc_pop_config();

extern bool zc_cfg_defaulted;
int32_t zc_get_config(char const* header, char const* name, int32_t default_val, App a = App::undefined);
int32_t zc_get_config_hex(char const* header, char const* name, int32_t default_val, App a = App::undefined);
double zc_get_config(char const* header, char const* name, double default_val, App a = App::undefined);
char const* zc_get_config(char const* header, char const* name, char const* default_val, App a = App::undefined);
void zc_set_config(char const* header, char const* name, int32_t val, App a = App::undefined);
void zc_set_config_hex(char const* header, char const* name, int32_t val, App a = App::undefined);
void zc_set_config(char const* header, char const* name, double default_val, App a = App::undefined);
void zc_set_config(char const* header, char const* name, char const* val, App a = App::undefined);

int32_t zc_get_config_basic(char const* header, char const* name, int32_t default_val);
int32_t zc_get_config_basic_hex(char const* header, char const* name, int32_t default_val);
double zc_get_config_basic(char const* header, char const* name, double default_val);
char const* zc_get_config_basic(char const* header, char const* name, char const* default_val);
void zc_set_config_basic(char const* header, char const* name, int32_t val);
void zc_set_config_basic_hex(char const* header, char const* name, int32_t val);
void zc_set_config_basic(char const* header, char const* name, double default_val);
void zc_set_config_basic(char const* header, char const* name, char const* val);

struct zc_a5_cfg_iterator;
struct ALLEGRO_CONFIG;
struct ALLEGRO_CONFIG_SECTION;
struct ALLEGRO_CONFIG_ENTRY;
struct zc_a5_cfg
{
	ALLEGRO_CONFIG* data;
	std::string path;
	bool dirty;
	
	zc_a5_cfg();
	zc_a5_cfg(std::string const& path);
	~zc_a5_cfg();
	
	void destroy();
	
	bool is_valid() const;
	
	void re_load(std::string const& new_path);
	bool load();
	bool save();
	
	void add_comment(const char* header, const char* comment);
	void add_section(const char* header);
	bool remove_section(const char* header);
	bool remove_key(const char* header, const char* name);
	std::optional<int> get_config_int(char const* header, char const* name);
	std::optional<double> get_config_float(char const* header, char const* name);
	std::optional<std::string> get_config(char const* header, char const* name);
	void set_config(char const* header, char const* name, int val);
	void set_config(char const* header, char const* name, double val);
	void set_config(char const* header, char const* name, char const* val);
	
	zc_a5_cfg_iterator iterate_sections();
	zc_a5_cfg_iterator iterate_entries(const char* header);
};
struct zc_a5_cfg_iterator
{
	ALLEGRO_CONFIG_SECTION** section_it = nullptr;
	ALLEGRO_CONFIG_ENTRY** entry_it = nullptr;
	std::optional<std::string> value = std::nullopt;
	
	std::optional<std::string> operator*(); // peek()
	zc_a5_cfg_iterator& operator++(); // advance + return
	zc_a5_cfg_iterator operator++(int); // returns a copy, then advances
	operator bool() const; // is_valid()
	
	bool advance(); // advances to the next section/entry, returns false if advanced past the last entry
	std::optional<std::string> peek(); // returns the current section/entry
	std::optional<std::string> next(); // returns the current section/entry AND advances to the next
	bool is_valid() const;
private:
	zc_a5_cfg_iterator(zc_a5_cfg_iterator const& other) = default;
	zc_a5_cfg_iterator(ALLEGRO_CONFIG const *data);
	zc_a5_cfg_iterator(ALLEGRO_CONFIG const *data, const char* header);
	
	friend struct zc_a5_cfg;
};

#endif
