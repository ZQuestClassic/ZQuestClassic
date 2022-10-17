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
void zc_install_mouse_event_handler();
void zc_process_mouse_events();

#endif
