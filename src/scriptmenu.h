#ifndef _Z_SCRIPTED_MENU_H_
#define _Z_SCRIPTED_MENU_H_

#include "zdefs.h"

enum ScriptMenuItemType
{
	SMI_TY_NUMBER,
	SMI_TY_BOOL,
	SMI_TY_TEXT
};
enum
{
	SMI_DATA_NUM_MINVAL,
	SMI_DATA_NUM_MAXVAL,
	SMI_DATA_NUM_SZ,
	
	SMI_DATA_TEXT_MAXCHARS,
	SMI_DATA_TEXT_SZ,
	
	SMI_DATA_SZ = SMI_DATA_NUM_SZ
};

struct ScriptMenuItem
{
	ScriptMenuItemType type;
	size_t index; //index in the vector- update this on removals and etc
	std::string name, info;
	
	ScriptMenuItem(ScriptMenuItemType type, size_t ind, std::string const& name);
	
	int getVal();
	bool setVal(int v);
	bool getStr(std::string& str);
	bool setStr(std::string const& str);
	
	void setBounds(int minval, int maxval);
	void setMaxChar(int maxchars);
	void setData(size_t indx, int val);
private:
	std::string strval;
	int val;
	int data[SMI_DATA_SZ];
	
	int boundVal(int v);
};
struct ScriptMenu
{
	std::vector<ScriptMenuItem> menu_items; //The actual items
	std::map<std::string, ScriptMenuItem*> map_items; //A map of names to items, for quick lookup
	
	ScriptMenuItem* add(ScriptMenuItemType ty, std::string const& name);
};

extern ScriptMenu scriptSettingsMenu;

#endif

