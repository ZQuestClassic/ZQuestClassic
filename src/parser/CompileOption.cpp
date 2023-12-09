#include "CompileOption.h"
#include "parser/config.h"
#include <map>
#include <vector>

//We need access to quest_rules, for option defaults. -Em
//#include "base/zdefs.h"
#include "base/zsys.h"
#include "base/qrs.h"

using std::map;
using std::string;
using namespace ZScript;

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

std::optional<CompileOptionValue> CompileOptionSetting::getValue() const
{
	if (type_ != TYPE_VALUE) return std::nullopt;
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
#		define X(NAME, DEFAULTQR, TYPE, DEFAULTVAL) \
		ID_##NAME,
#		include "CompileOption.xtable"
#		undef X
		ID_END
	};

	// Holds a table entry.
	struct Entry
	{
		string name;
		CompileOptionValue defaultValue;
		int32_t defaultqr, type;
		Entry(string name = "", int32_t defaultQR = 0, int32_t type = 0, int32_t defaultValue = 0L)
			: name(name), defaultValue(defaultValue), type(type), defaultqr(defaultQR) {}
	};

	// Table holding option data.
	Entry entries[ID_END];

	// Map of names to options.
	map<string, CompileOption> nameMap;
};

// Define static instance for each option.
#define X(NAME, DEFAULTQR, TYPE, DEFAULTVAL) \
CompileOption CompileOption::OPT_##NAME(ID_##NAME);
#include "CompileOption.xtable"
#undef X

CompileOption CompileOption::Invalid(-1);

void CompileOption::initialize()
{
	static bool initialized = false;
	if (!initialized)
	{
		// Fill entries table from xtable.
#		define X(NAME, DEFAULTQR, TYPE, DEFAULTVAL) \
		entries[ID_##NAME] = Entry(#NAME, DEFAULTQR, TYPE, DEFAULTVAL);
#		include "CompileOption.xtable"
#		undef X

		// Fill nameMap from entries table.
		for (int32_t i = 0; i < ID_END; ++i)
		{
			nameMap[entries[i].name] = CompileOption(i);
		}
		
		initialized = true;
	}
	//Update default values, always:
	updateDefaults();
	flush_config_file();
}

void CompileOption::updateDefaults()
{
	for (int32_t i = 0; i < ID_END; ++i)
	{
		switch(entries[i].type)
		{
			case OPTTYPE_QR:
				if(entries[i].defaultqr)
					entries[i].defaultValue = get_qr(entries[i].defaultqr) ? 10000L : 0L;
				break;
			
			case OPTTYPE_CONFIG:
			{
				int32_t val = zscript_get_config_int(entries[i].name, int32_t(entries[i].defaultValue/10000L));
				if(!zc_cfg_defaulted)
					entries[i].defaultValue = val * 10000L;
				break;
			}
			
			case OPTTYPE_CONFIG_FLOAT:
			{
				double val = zscript_get_config_double(entries[i].name, entries[i].defaultValue/10000.0);
				if(!zc_cfg_defaulted)
					entries[i].defaultValue = val * 10000L;
				break;
			}
		}
	}
}

std::optional<CompileOption> CompileOption::get(string const& name)
{
	map<string, CompileOption>::const_iterator it = nameMap.find(name);
	if (it == nameMap.end()) return std::nullopt;
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

std::optional<CompileOptionValue> CompileOption::getDefault() const
{
	if (!isValid()) return std::nullopt;
	return entries[id_].defaultValue;
}

void CompileOption::setDefault(CompileOptionValue value)
{
	if(!isValid()) return;
	entries[id_].defaultValue = value;
}
