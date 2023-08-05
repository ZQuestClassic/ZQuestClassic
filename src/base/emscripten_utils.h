#ifndef __emscripten_utils_h_
#define __emscripten_utils_h_

#include <emscripten/emscripten.h>
#include <string>

struct QueryParams
{
  std::string quest;
};

void em_init_fs();
void em_sync_fs();
void em_fetch_file(std::string path);
bool em_is_lazy_file(std::string path);
std::string get_initial_file_dialog_folder();
QueryParams get_query_params();
void em_mark_initializing_status();
void em_mark_ready_status();
bool em_is_mobile();
void em_open_test_mode(const char* qstpath, int dmap, int scr, int retsquare);
int em_compile_zscript(const char* script_path, const char* console_path, const char* qr);

#endif
