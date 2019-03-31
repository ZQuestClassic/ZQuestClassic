#include "../precompiled.h" // Always first.

#include "CompileOption.h"
#include <map>
#include <vector>

//We need access to quest_rules, for option defaults. -V
#include "../zdefs.h"
#include "../zsys.h"

using std::map;
using std::string;
using namespace ZScript;

extern byte quest_rules[QUESTRULES_NEW_SIZE];
////////////////////////////////////////////////////////////////
// CompileOptionSetting

CompileOptionSetting CompileOptionSetting::Invalid(TYPE_UNSET);
CompileOptionSetting CompileOptionSetting::Default(TYPE_DEFAULT);
CompileOptionSetting CompileOptionSetting::Inherit(TYPE_INHERIT);

CompileOptionSetting::CompileOptionSetting()
	: type_(TYPE_UNSET), value_(0L)
{}

CompileOptionSetting::CompileOptionSetting(CompileOptionValue value)
	: type_(TYPE_VALUE), value_(value)
{}

bool CompileOptionSetting::operator==(CompileOptionSetting const& rhs) const
{
	if (type_ == TYPE_UNSET) return false;
	if (type_ != rhs.type_) return false;
	if (type_ != TYPE_VALUE) return true;
	return value_ == rhs.value_;
}

bool CompileOptionSetting::safe_bool() const
{
	return type_ != TYPE_UNSET;
}

optional<CompileOptionValue> CompileOptionSetting::getValue() const
{
	if (type_ != TYPE_VALUE) return nullopt;
	return value_;
}

std::string CompileOptionSetting::asString() const
{
	switch (type_)
	{
	case TYPE_DEFAULT: return "default";
	case TYPE_INHERIT: return "inherit";
	case TYPE_VALUE: return to_string(value_);
	default: return "INVALID";
	}
}

CompileOptionSetting::CompileOptionSetting(Type type)
	: type_(type), value_(0L)
{}

////////////////////////////////////////////////////////////////
// CompileOption

namespace // file local
{
	// Generate ids from table.
	enum Id
	{
		ID_START = -1,
#		define X(NAME, DEFAULTQR, DEFAULTVAL) \
		ID_##NAME,
#		define START_GLOBAL(NUM) \
		ID_GLOBAL = NUM,
#		include "CompileOption.xtable"
#		undef X
#		undef START_GLOBAL
		ID_END
	};

	// Holds a table entry.
	struct Entry
	{
		string name;
		CompileOptionValue defaultValue;
		int defaultqr;
		Entry(string name = "", int defaultQR = 0L, long defaultValue = 0L)
			: name(name), defaultValue(defaultValue), defaultqr(defaultQR) {}
	};

	// Table holding option data.
	Entry entries[ID_END];

	// Map of names to options.
	map<string, CompileOption> nameMap;
};

// Define static instance for each option.
#define X(NAME, DEFAULTQR, DEFAULTVAL) \
CompileOption CompileOption::OPT_##NAME(ID_##NAME);
#define START_GLOBAL(NUM) \
//
#include "CompileOption.xtable"
#undef X
#undef START_GLOBAL

CompileOption CompileOption::Invalid(-1);

void CompileOption::initialize()
{
	static bool initialized = false;
	if (initialized)
	{
		//Load updates to the default options, regardless -V
		for (int i = 0; i < ID_END; ++i)
		{
			if(i > ID_GLOBAL)
			{
				//Read from config ints.
			}
			else if(entries[i].defaultqr) //If this has a QR set in the xtable, use that
			{
				entries[i].defaultValue = get_bit(quest_rules, entries[i].defaultqr) ? 10000L : 0L;
			}
		}
	}
	else
	{
		// Fill entries table from xtable.
#		define X(NAME, DEFAULTQR, DEFAULTVAL) \
		entries[ID_##NAME] = Entry(#NAME, DEFAULTQR, DEFAULTVAL);
#		define START_GLOBAL(NUM) \
		//
#		include "CompileOption.xtable"
#		undef X
#		undef START_GLOBAL

		// Fill nameMap from entries table.
		for (int i = 0; i < ID_END; ++i)
		{
			if(i > ID_GLOBAL)
			{
				//Read from config ints.
			}
			else if(entries[i].defaultqr) //If this has a QR set in the xtable, use that
			{
				entries[i].defaultValue = get_bit(quest_rules, entries[i].defaultqr) ? 10000L : 0L;
			}
			nameMap[entries[i].name] = CompileOption(i);
		}
		
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

optional<CompileOptionValue> CompileOption::getDefault() const
{
	if (!isValid()) return nullopt;
	return entries[id_].defaultValue;
}

void CompileOption::setDefault(CompileOptionValue value)
{
	if(!isValid()) return;
	entries[id_].defaultValue = value;
}
