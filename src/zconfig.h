#ifndef ZC_ZCONFIG_H
#define ZC_ZCONFIG_H

char const* zc_get_standard_config_name();
void zc_set_config_standard();
bool zc_config_standard_exists();

int32_t zc_get_config(char const* header, char const* name, int32_t default_val);
double zc_get_config(char const* header, char const* name, double default_val);
char const* zc_get_config(char const* header, char const* name, char const* default_val);
void zc_set_config(char const* header, char const* name, int32_t val);
void zc_set_config(char const* header, char const* name, double default_val);
void zc_set_config(char const* header, char const* name, char const* val);

int32_t zc_get_config(char const* cfg_file, char const* header, char const* name, int32_t default_val);
double zc_get_config(char const* cfg_file, char const* header, char const* name, double default_val);
char const* zc_get_config(char const* cfg_file, char const* header, char const* name, char const* default_val);
void zc_set_config(char const* cfg_file, char const* header, char const* name, int32_t val);
void zc_set_config(char const* cfg_file, char const* header, char const* name, double default_val);
void zc_set_config(char const* cfg_file, char const* header, char const* name, char const* val);

#endif
