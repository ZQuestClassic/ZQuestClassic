#include <string>

#ifndef _ZAPP_H_
#define _ZAPP_H_

void common_main_setup(int argc, char **argv);
bool is_in_osx_application_bundle();
std::string get_user_data_directory();
std::string get_user_data_path(std::string path);

#endif
