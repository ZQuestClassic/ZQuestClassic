#include "parser/ParserHelper.h"
#include "parser/AST.h"
#include <map>

ParserScriptType ParserHelper::getScriptType(std::string name)
{
	if (name == "global")
		return ParserScriptType::global;
	else if (name == "ffc")
		return ParserScriptType::ffc;
	else if (name == "itemdata")
		return ParserScriptType::item;
	else if (name == "npc")
		return ParserScriptType::npc;
	else if (name == "lweapon")
		return ParserScriptType::lweapon;
	else if (name == "eweapon")
		return ParserScriptType::eweapon;
	else if (name == "player")
		return ParserScriptType::player;
	else if (name == "screendata")
		return ParserScriptType::screendata;
	else if (name == "dmapdata")
		return ParserScriptType::dmapdata;
	else if (name == "itemsprite")
		return ParserScriptType::itemsprite;
	else if (name == "subscreendata")
		return ParserScriptType::subscreendata;
	else if (name == "combodata")
		return ParserScriptType::combodata;
	else if (name == "generic")
		return ParserScriptType::genericscr;
	else
		return ParserScriptType::invalid;
}

bool ParserHelper::isValidIdentifier(std::string identifier)
{
	return getScriptType(identifier) == ParserScriptType::invalid;
}
