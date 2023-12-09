#ifndef _HOTKEY_H_
#define _HOTKEY_H_

#include "base/zc_alleg.h"
#include <string>

#define HOTKEY_FLAG_FILTER (KB_SHIFT_FLAG|KB_CTRL_FLAG|KB_ALT_FLAG)
struct Hotkey
{
	int modflag[2];
	int hotkey[2];
	#undef check
	bool check(int k,int shifts,bool exact=false);
	int getval() const;
	void setval(int val);
	void setval(int ind,int k,int shifts);
	void setval(int k,int shifts,int k2,int shifts2);
	std::string get_name(int ind) const;
	bool operator==(Hotkey const& other);
	bool operator!=(Hotkey const& other);
};
std::string get_keystr(int key);
bool is_modkey(int c);
int get_mods(int mask = HOTKEY_FLAG_FILTER);

#endif

