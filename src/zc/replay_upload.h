#ifndef REPLAY_UPLOAD_H_
#define REPLAY_UPLOAD_H_

#include <optional>
#include <string>

bool replay_upload_auto_enabled();
int replay_upload();
std::optional<std::string> replay_upload(std::string file_path);
int replay_upload_auto();
void replay_upload_clear_cache();

#endif
