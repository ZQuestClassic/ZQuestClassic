#include "scriptmenu.h"

ScriptMenuItem::ScriptMenuItem(ScriptMenuItemType type, size_t ind, std::string const& name)
	: index(ind), type(type), name(name)
{
	switch(type)
	{
		case SMI_TY_BOOL:
			break;
		case SMI_TY_NUMBER:
			data[SMI_DATA_NUM_MINVAL] = 0;
			data[SMI_DATA_NUM_MAXVAL] = -1;
			break;
		case SMI_TY_TEXT:
			data[SMI_DATA_TEXT_MAXCHARS] = 1024;
			break;
	}
}

int ScriptMenuItem::getVal()
{
	return boundVal(val);
}
bool ScriptMenuItem::setVal(int v)
{
	val = boundVal(v);
}
bool ScriptMenuItem::getStr(std::string& str)
{
	if(type == SMI_TY_TEXT)
	{
		str = strval;
		return true;
	}
	return false;
}
bool ScriptMenuItem::setStr(std::string const& str)
{
	if(type == SMI_TY_TEXT)
	{
		strval = str;
		return true;
	}
	return false;
}
void ScriptMenuItem::setBounds(int minval, int maxval)
{
	if(type == SMI_TY_NUMBER)
	{
		data[SMI_DATA_NUM_MINVAL] = minval;
		data[SMI_DATA_NUM_MAXVAL] = maxval;
	}
}
void ScriptMenuItem::setMaxChar(int maxchars)
{
	if(type == SMI_TY_TEXT)
	{
		data[SMI_DATA_TEXT_MAXCHARS] = (maxchars < 1 || maxchars > 1023) ? 1024 : maxchars;
	}
}
void ScriptMenuItem::setData(size_t indx, int val)
{
	switch(type)
	{
		case SMI_TY_NUMBER:
			switch(indx)
			{
				case SMI_DATA_NUM_MINVAL:
				case SMI_DATA_NUM_MAXVAL:
					data[indx] = val;
					break;
			}
			break;
		case SMI_TY_BOOL:
			break;
		case SMI_TY_TEXT:
			if(indx == SMI_DATA_TEXT_MAXCHARS)
				setMaxChar(val);
			break;
	}
}
int ScriptMenuItem::boundVal(int v)
{
	switch(type)
	{
		case SMI_TY_BOOL:
			return v ? 1 : 0;
		case SMI_TY_NUMBER:
			if(data[SMI_DATA_NUM_MINVAL] <= data[SMI_DATA_NUM_MAXVAL])
			{
				if(v < data[SMI_DATA_NUM_MINVAL])
					return data[SMI_DATA_NUM_MINVAL];
				if(v > data[SMI_DATA_NUM_MAXVAL])
					return data[SMI_DATA_NUM_MAXVAL];
			}
			return v;
		default:
			return 0;
	}
}

ScriptMenuItem* ScriptMenu::add(ScriptMenuItemType ty, std::string const& name)
{
	if(map_items.find(name) == map_items.end()) //unique name
	{
		ScriptMenuItem* ptr = &menu_items.emplace_back(ty,menu_items.size(),name);
		map_items[name] = ptr;
		return ptr;
	}
	return nullptr; //duplicate name, fail
}