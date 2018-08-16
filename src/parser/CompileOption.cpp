#include "../precompiled.h" // Always first.

#include "CompileOption.h"
#include <map>
#include <vector>

using std::map;
using std::string;
using namespace ZScript;

namespace // file local
{
	// Generate ids from table.
	enum Id
	{
		ID_START = -1,
#		define X(NAME, DEFAULT) \
		ID_##NAME,
#		include "CompileOption.xtable"
#		undef X
		ID_END
	};

	// Holds a table entry.
	struct Entry
	{
		string name;
		long defaultValue;
		Entry(string name = "", long defaultValue = 0)
			: name(name), defaultValue(defaultValue) {}
	};

	// Table holding option data.
	Entry entries[ID_END];

	// Map of names to options.
	map<string, CompileOption> nameMap;
};

// Define static instance for each option.
#define X(NAME, DEFAULT) \
CompileOption CompileOption::NAME(ID_##NAME);
#include "CompileOption.xtable"
#undef X

CompileOption CompileOption::Invalid(-1);

void CompileOption::initialize()
{
	static bool initialized = false;
	if (!initialized)
	{
		// Fill entries table from xtable.
#		define X(NAME, DEFAULT) \
		entries[ID_##NAME] = Entry(#NAME, DEFAULT);
#		include "CompileOption.xtable"
#		undef X

		// Fill nameMap from entries table.
		for (int i = 0; i < ID_END; ++i)
			nameMap[entries[i].name] = CompileOption(i);
		
		initialized = true;
	}
}

optional<CompileOption> CompileOption::get(string const& name)
{
	map<string, CompileOption>::const_iterator it = nameMap.find(name);
	if (it == nameMap.end()) return nullopt;
	return it->second;
}

bool CompileOption::isValid() const
{
	return ID_START < id_ && id_ < ID_END;
}

std::string* CompileOption::getName() const
{
	if (!isValid()) return NULL;
	return &entries[id_].name;
}

optional<long> CompileOption::getDefault() const
{
	if (!isValid()) return nullopt;
	return entries[id_].defaultValue;
}
