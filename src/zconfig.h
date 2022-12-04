#ifndef ZC_ZCONFIG_H
#define ZC_ZCONFIG_H

#include "base/zapp.h"

char const* get_config_file_name(App a = App::undefined);
char const* get_config_base_name(App a = App::undefined);
void zc_config_file(char const* fl);
void zc_push_config();
void zc_pop_config();

extern bool zc_cfg_defaulted;
int32_t zc_get_config(char const* header, char const* name, int32_t default_val, App a = App::undefined);
double zc_get_config(char const* header, char const* name, double default_val, App a = App::undefined);
char const* zc_get_config(char const* header, char const* name, char const* default_val, App a = App::undefined);
void zc_set_config(char const* header, char const* name, int32_t val, App a = App::undefined);
void zc_set_config(char const* header, char const* name, double default_val, App a = App::undefined);
void zc_set_config(char const* header, char const* name, char const* val, App a = App::undefined);

int32_t zc_get_config_basic(char const* header, char const* name, int32_t default_val);
double zc_get_config_basic(char const* header, char const* name, double default_val);
char const* zc_get_config_basic(char const* header, char const* name, char const* default_val);
void zc_set_config_basic(char const* header, char const* name, int32_t val);
void zc_set_config_basic(char const* header, char const* name, double default_val);
void zc_set_config_basic(char const* header, char const* name, char const* val);

#endif
