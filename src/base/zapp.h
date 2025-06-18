#ifndef ZAPP_H_
#define ZAPP_H_

#include <optional>
#include <functional>
#include <string>
#include <utility>

enum App {
    undefined,
    zelda,
    zquest,
    launcher,
    zscript,
	updater,
};

void common_main_setup(App id, int argc, char **argv);
std::optional<bool> get_flag_bool(const char* name);
std::optional<int> get_flag_int(const char* name);
std::optional<std::string> get_flag_string(const char* name);
App get_app_id();
bool is_web();
bool is_ci();
void set_headless_mode();
bool is_headless();
void zapp_setup_icon();
std::pair<int, int> zc_get_default_display_size(int base_width, int base_height, int saved_width, int saved_height, int max_scale = 3);
void zc_process_display_events();
void zapp_set_crash_cb(std::function<void()> cb);
void zapp_reporting_add_breadcrumb(const char* category, const char* message);
void zapp_reporting_set_tag(const char* key, const char* value);
void zapp_reporting_set_tag(const char* key, int value);

bool is_exiting();
void set_is_exiting();

#endif
