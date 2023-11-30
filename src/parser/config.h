#include <string>
#include <map>

// Subset of allegro's config file, but without using any allegro code.
// Only reads "[Compiler]" section.
bool zscript_load_base_config(std::string path);
bool zscript_load_user_config(std::string path);

std::string zscript_get_config_string(std::string key, std::string def_value);
int zscript_get_config_int(std::string key, int def_value);
double zscript_get_config_double(std::string key, double def_value);
