#ifndef _ZAPP_H_
#define _ZAPP_H_

enum App {
    undefined,
    zelda,
    zquest,
    launcher,
    zscript,
};

void common_main_setup(App id, int argc, char **argv);
App get_app_id();
bool is_ci();
void set_headless_mode();
bool is_headless();
double zc_get_monitor_scale();
void zc_process_display_events();
void zapp_reporting_add_breadcrumb(const char* type, const char* message);
void zapp_reporting_set_tag(const char* key, const char* value);
void zapp_reporting_set_tag(const char* key, int value);

#endif
