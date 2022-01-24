#ifndef ZC_ZCONFIG_H
#define ZC_ZCONFIG_H

#ifdef IS_ZQUEST
#define STANDARD_CFG "zquest.cfg"
#elif defined IS_LAUNCHER
#define STANDARD_CFG "zcl.cfg"
#elif defined IS_PLAYER
#define STANDARD_CFG "zc.cfg"
#elif defined IS_PARSER
#define STANDARD_CFG "zscript.cfg"
#else
#define STANDARD_CFG "ag.cfg"
#endif
void set_config_standard();

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

