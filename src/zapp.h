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

#endif
