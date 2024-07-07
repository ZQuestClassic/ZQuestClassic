#ifndef ZQ_STRINGS_H_
#define ZQ_STRINGS_H_

#include <map>
#include <cstdint>

struct DIALOG;

int32_t onStrings();
char *MsgString(int32_t index, bool show_number, bool pad_number);
const char *msgfontlist(int32_t index, int32_t *list_size);
const char *msgslist(int32_t index, int32_t *list_size);
const char *msgslist2(int32_t index, int32_t *list_size);
const char *msgslist3(int32_t index, int32_t *list_size);
int32_t msg_at_pos(int32_t pos);
char *strip_extra_spaces(char *string);
char* encode_msg_str(int32_t index);

extern DIALOG strlist_dlg[];

extern std::map<int32_t, int32_t> msglistcache;

#endif
