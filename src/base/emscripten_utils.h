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
void em_fetch_file(const char *path);
QueryParams get_query_params();
void em_mark_initializing_status();
void em_mark_ready_status();

#endif
