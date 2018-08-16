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
		int defaultValue;
		Entry(string name = "", int defaultValue = 0)
			: name(name), defaultValue(defaultValue) {}
	};

	// Table holding option data.
	Entry entries[ID_END];
};

void CompileOption::initialize()
{
	static bool initialized = false;
	if (!initialized)
	{
#		define X(NAME, DEFAULT) \
		entries[ID_##NAME] = Entry(#NAME, DEFAULT);
#		include "CompileOption.xtable"
#		undef X
		initialized = true;
	}
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

optional<int> CompileOption::getDefault() const
{
	if (!isValid()) return nullopt;
	return entries[id_].defaultValue;
}
