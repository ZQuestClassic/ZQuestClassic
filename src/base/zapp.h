#ifndef ZAPP_H_
#define ZAPP_H_

#include <optional>
#include <functional>
#include <string>
#include <vector>

enum App {
    undefined,
    zelda,
    zquest,
    launcher,
    zscript,
	updater,
};

void common_main_setup(App id, int argc, char **argv);
int32_t used_switch(int32_t argc,char *argv[],const char *s);
int32_t zapp_check_switch(const char *s, std::vector<const char*> arg_names = {});
int zapp_get_argc();
char** zapp_get_argv();
void zapp_replace_args(int argc, char **argv);
int32_t zapp_get_arg_int(int index);
std::string zapp_get_arg_string(int index);
std::string zapp_get_exe_folder_path();
// If `-name` flag is set, returns true
// If `-no-name` flag is set, returns false
// Otherwise returns std::nullopt
std::optional<bool> get_flag_bool(const char* name);
std::optional<int64_t> get_flag_int(const char* name);
std::optional<std::string> get_flag_string(const char* name);
App get_app_id();
bool is_web();
bool is_ci();
void set_headless_mode();
bool is_headless();
void zapp_set_crash_cb(std::function<void()> cb);
void zapp_reporting_add_breadcrumb(const char* category, const char* message);
void zapp_reporting_set_tag(const char* key, const char* value);
void zapp_reporting_set_tag(const char* key, int value);

bool is_exiting();
void set_is_exiting();
void zapp_exit(int code);

#endif
