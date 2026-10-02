#ifndef _HOTKEY_H_
#define _HOTKEY_H_

#include "base/zc_alleg.h"
#include <string>

#define HOTKEY_FLAG_FILTER (KB_SHIFT_FLAG|KB_CTRL_FLAG|KB_COMMAND_FLAG|KB_ALT_FLAG)
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
// Hotkeys can also be bound to mouse buttons, stored as codes past the keyboard's
// (still within the 8 bits each binding is saved with). Left and right click are
// for editing, so only the middle button and up can be bound.
#define HOTKEY_MOUSE_FIRST_BUTTON 3
#define HOTKEY_MOUSE_LAST_BUTTON 16
#define HOTKEY_MOUSE_CODE(button) (KEY_MAX + (button))
#define HOTKEY_MOUSE_MIDDLE HOTKEY_MOUSE_CODE(3)
#define HOTKEY_MOUSE_BACK HOTKEY_MOUSE_CODE(4)
#define HOTKEY_MOUSE_FORWARD HOTKEY_MOUSE_CODE(5)
// The mouse button (1-based, as in Allegro) for a hotkey code, or 0 for a key.
int hotkey_mouse_button(int k);

std::string get_keystr(int key);
bool is_modkey(int c);
int get_mods(int mask = HOTKEY_FLAG_FILTER);

#endif

