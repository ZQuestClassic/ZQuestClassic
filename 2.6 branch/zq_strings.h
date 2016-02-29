#ifndef _ZQ_STRINGS_H_
#define _ZQ_STRINGS_H_

#include <map>
struct DIALOG;

int onStrings();
char *MsgString(int index, bool show_number, bool pad_number);
const char *msgfontlist(int index, int *list_size);
const char *msgslist(int index, int *list_size);
const char *msgslist2(int index, int *list_size);
int msg_at_pos(int pos);
char *strip_extra_spaces(char *string);
void encode_msg_str(int index);

extern DIALOG strlist_dlg[];
extern DIALOG editmsg_dlg[];
extern DIALOG editmsg_help_dlg[];

extern std::map<int, int> msglistcache;

#endif
