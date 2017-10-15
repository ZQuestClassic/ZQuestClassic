#include "../precompiled.h"
#include "ZAsmVariables.h"
#include "ZAsmVariableTable.h"

#include <cassert>
#include <cstdlib>
#include <map>
#include <sstream>

#include "../zc_alleg.h"

#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_UNIX
#define strnicmp strncasecmp
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif

using namespace std;
using namespace ZAsm;

VariableDef::VariableDef(VariableId firstId, int count, string const& name)
	: firstId_(firstId), count_(count), name_(name)
{}

map<VariableId, VariableDef const*>& idMap()
{
	static map<VariableId, VariableDef const*> m;
	static bool initialized = false;
	if (!initialized)
	{		
#		define VARIABLE(START, COUNT, NAME)\
		for (int i = 0; i < COUNT; ++i)\
			/* eg. m[VariableId(0x000A + i)] = &ZAsm::VarDef_DATA; */\
			m[VariableId(START + i)] = &ZAsm::VarDef_##NAME;
		ZASM_VARIABLE_TABLE
#		undef VARIABLE
		initialized = true;
	}
	return m;
}

VariableDef const* ZAsm::getVariableDef(VariableId id)
{
	if (id < 0 || id >= ZAsm::VariableCount) return NULL;
	map<VariableId, VariableDef const*>& m = idMap();
	map<VariableId, VariableDef const*>::const_iterator it = m.find(id);
	if (it == m.end()) return NULL;
	return it->second;
}

map<string, VariableDef const*>& nameMap()
{
	static map<string, VariableDef const*> m;
	static bool initialized = false;
	if (!initialized)
	{
		// eg. m["DATA"] = &ZAsm::VarDef_DATA;
#		define VARIABLE(START, COUNT, NAME) m[#NAME] = &ZAsm::VarDef_##NAME;
		ZASM_VARIABLE_TABLE
#		undef VARIABLE
		initialized = true;
	}
	return m;
}

VariableDef const* ZAsm::getVariableDef(string const& name)
{
	map<string, VariableDef const*>& m = nameMap();
	map<string, VariableDef const*>::const_iterator it = m.find(name);
	if (it == m.end()) return NULL;
	return it->second;
}

// Variable

Variable Variable::Null;

Variable::Variable(VariableDef const* definition, int index)
	: definition_(definition), index_(index)
{
	assert(index_ >= 0);
	assert(index_ < definition->getCount());
}

VariableId Variable::getId() const
{
	return VariableId(definition_->getFirstId() + index_);
}

string Variable::toString() const
{
	if (!definition_) return "<NULL>";
	if (definition_->getCount() == 1) return definition_->getName();
	if (definition_ == &VarDef_D)
	{
		ostringstream out;
		out << "d" << index_;
		return out.str();
	}
	if (definition_ == &VarDef_A)
	{
		ostringstream out;
		out << "a" << index_ + 1;
		return out.str();
	}
	if (definition_ == &VarDef_GD)
	{
		ostringstream out;
		out << "gd" << index_;
		return out.str();
	}
	return "<UNKNOWN>";
}

Variable ZAsm::getVariable(VariableId id)
{
	VariableDef const* def = getVariableDef(id);
	if (!def) return Variable::Null;
	return Variable(def, id - def->getFirstId());
}

Variable ZAsm::getVariable(string const& name)
{
	// Singles.
	VariableDef const* def = getVariableDef(name);
	if (def) return Variable(def, 0);

	// A1 and A2
	if (name == "A1" || name == "a1") return Variable(&VarDef_A, 0);
	if (name == "A2" || name == "a2") return Variable(&VarDef_A, 1);

	// D0 through D7
	if ((name[0] == 'D' || name[0] == 'd')
	    && '0' <= name[1] && name[1] <= '7'
	    && name[2] == '\0')
		return Variable(&VarDef_D, name[1] - '0');

	// GD0 through GD255
	if (strnicmp(name.c_str(), "GD", 2) == 0)
	{
		int index = atoi(name.substr(2).c_str());
		if (0 <= index && index <= 255)
			return Variable(&VarDef_GD, index);
	}

	return Variable::Null;
}

// Declare Variable constructors. Example:
// Variable ZAsm::varD(int index) {
//     return Variable(&VarDef_D, index);}
#define VARIABLE(START, COUNT, NAME)\
Variable ZAsm::var##NAME(int index) { \
	return Variable(&VarDef_##NAME, index);}
ZASM_VARIABLE_TABLE
#undef VARIABLE

#undef ZASM_VARIABLE_TABLE
