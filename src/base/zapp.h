#ifndef _ZAPP_H_
#define _ZAPP_H_

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
App get_app_id();
bool is_web();
bool is_ci();
void set_headless_mode();
bool is_headless();
double zc_get_monitor_scale();
std::pair<int, int> zc_get_default_display_size(int base_width, int base_height, int saved_width, int saved_height);
void zc_process_display_events();
void zapp_reporting_add_breadcrumb(const char* category, const char* message);
void zapp_reporting_set_tag(const char* key, const char* value);
void zapp_reporting_set_tag(const char* key, int value);

#endif
