#ifndef _BASE_FILES_H_
#define _BASE_FILES_H_

#include "base/jwinfsel.h"
#include <optional>
#include <string>

extern char temppath[4096];

// When set to true, the methods in this file will always directly open the OS file select dialog.
// When false, instead the classic jwinfsel allegro 4 dialog will be used. However, the user may still select
// a button in that dialog to use the native dialog.
void set_always_use_native_file_dialog(bool active);

// ext is semicolon separated list of file types. Ex: qst;qsu
//   Note: space and comma is allowed to, since that is what allegro 4 does in `parse_extension_string` despite only documenting that `;` is supported.
// Prefer `list` to provide labels associated with a group of extensions. When `list` is set, `ext` is ignored.
std::optional<std::string> prompt_for_existing_file(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename = true);
std::optional<std::string> prompt_for_existing_folder(std::string prompt, std::string initial_path, std::string ext = "");
std::optional<std::string> prompt_for_new_file(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename = true);

// Same as above, but return value is a bool and the result is placed in a global `temppath`. Prefer the other methods.
bool prompt_for_existing_file_compat(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename = true);
bool prompt_for_new_file_compat(std::string prompt, std::string ext, EXT_LIST *list, std::string initial_path, bool usefilename = true);

#endif
