#include "hotkey.h"
#include "base/util.h"
#include <sstream>

bool Hotkey::check(int k,int shifts,bool exact)
{
	for(int q = 0; q <= 1; ++q)
	{
		int s = shifts & (exact ? HOTKEY_FLAG_FILTER : modflag[q]);
		if(hotkey[q] == k && modflag[q] == s)
			return true;
	}
	return false;
}
int Hotkey::getval() const
{
	return (hotkey[0]&0xFF)<<0 |
		(modflag[0]&HOTKEY_FLAG_FILTER)<<8 |
		(hotkey[1]&0xFF)<<16 |
		(modflag[1]&HOTKEY_FLAG_FILTER)<<24;
}
void Hotkey::setval(int val)
{
	hotkey[0] = (val>>0)&0xFF;
	modflag[0] = (val>>8)&HOTKEY_FLAG_FILTER;
	hotkey[1] = (val>>16)&0xFF;
	modflag[1] = (val>>24)&HOTKEY_FLAG_FILTER;
	for(int q = 0; q < 2; ++q)
		if(modflag[q] & KB_COMMAND_FLAG)
		{
			modflag[q] &= ~KB_COMMAND_FLAG;
			modflag[q] |= KB_CTRL_FLAG;
		}
}
void Hotkey::setval(int ind,int k,int shifts)
{
	if(shifts & KB_COMMAND_FLAG)
	{
		shifts &= ~KB_COMMAND_FLAG;
		shifts |= KB_CTRL_FLAG;
	}
	hotkey[ind] = k&0xFF;
	modflag[ind] = shifts&HOTKEY_FLAG_FILTER;
}
void Hotkey::setval(int k,int shifts,int k2,int shifts2)
{
	if(shifts & KB_COMMAND_FLAG)
	{
		shifts &= ~KB_COMMAND_FLAG;
		shifts |= KB_CTRL_FLAG;
	}
	if(shifts2 & KB_COMMAND_FLAG)
	{
		shifts2 &= ~KB_COMMAND_FLAG;
		shifts2 |= KB_CTRL_FLAG;
	}
	hotkey[0] = k&0xFF;
	modflag[0] = shifts&HOTKEY_FLAG_FILTER;
	hotkey[1] = k2&0xFF;
	modflag[1] = shifts2&HOTKEY_FLAG_FILTER;
}
std::string Hotkey::get_name(int ind) const
{
	if(unsigned(ind) > 1)
		return "";
	if(!hotkey[ind])
		return "(None)";
	std::ostringstream oss;
	if(modflag[ind] & KB_CTRL_FLAG)
		oss << "Ctrl+";
	if(modflag[ind] & KB_ALT_FLAG)
		oss << "Alt+";
	if(modflag[ind] & KB_SHIFT_FLAG)
		oss << "Shift+";
	oss << get_keystr(hotkey[ind]);
	return oss.str();
}
bool Hotkey::operator==(Hotkey const& other)
{
	for(int q = 0; q < 2; ++q)
	{
		if(hotkey[q] != other.hotkey[q])
			return false;
		if(modflag[q] != other.modflag[q])
			return false;
	}
	return true;
}
bool Hotkey::operator!=(Hotkey const& other)
{
	return !(*this == other);
}

const char *key_str[] =
{
	"(None)",       "A",             "B",             "C",
	"D",            "E",             "F",             "G",
	"H",            "I",             "J",             "K",
	"L",            "M",             "N",             "O",
	"P",            "Q",             "R",             "S",
	"T",            "U",             "V",             "W",
	"X",            "Y",             "Z",             "0",
	"1",            "2",             "3",             "4",
	"5",            "6",             "7",             "8",
	"9",            "Num 0",         "Num 1",         "Num 2",
	"Num 3",        "Num 4",         "Num 5",         "Num 6",
	"Num 7",        "Num 8",         "Num 9",         "F1",
	"F2",           "F3",            "F4",            "F5",
	"F6",           "F7",            "F8",            "F9",
	"F10",          "F11",           "F12",           "Esc",
	"~",            "-",             "=",             "Backspace",
	"Tab",          "{",             "}",             "Enter",
	":",            "Quote",         "\\",            "\\ (2)",
	",",            ".",             "/",             "Space",
	"Insert",       "Delete",        "Home",          "End",
	"Page Up",      "Page Down",     "Left",          "Right",
	"Up",           "Down",          "Num /",         "Num *",
	"Num -",        "Num +",         "Num delete",    "Num enter",
	"Print screen", "Pause",         "Abnt C1",       "Yen",
	"Kana",         "Convert",       "No Convert",    "At",
	"Circumflex",   ": (2)",         "Kanji",         "Num =",
	"Back quote",   ";",             "Command",       "Unknown (0)",
	"Unknown (1)",  "Unknown (2)",   "Unknown (3)",   "Unknown (4)",
	"Unknown (5)",  "Unknown (6)",   "Unknown (7)",   "Left Shift",
	"Right Shift",  "Left Control",  "Right Control", "Alt",
	"Alt GR",       "Left Win",      "Right Win",     "Menu",
	"Scroll Lock",  "Number Lock",   "Caps Lock",     "MAX"
};
std::string get_keystr(int key)
{
	if(unsigned(key) > KEY_MAX) return "";
	std::string str(key_str[key]);
	util::trimstr(str);
	return str;
}
bool is_modkey(int c)
{
	switch(c)
	{
		case KEY_LSHIFT:
		case KEY_RSHIFT:
		case KEY_LCONTROL:
		case KEY_RCONTROL:
		case KEY_COMMAND:
		case KEY_ALT:
		case KEY_ALTGR:
			return true;
	}
	return false;
}
int get_mods(int mask)
{
	int shifts = key_shifts;
	if(shifts&KB_COMMAND_FLAG)
	{
		shifts &= ~KB_COMMAND_FLAG;
		shifts |= KB_CTRL_FLAG;
	}
	return shifts&mask;
}

