#include "core/qrs.h"
#include "base/general.h"

byte quest_rules[QUESTRULES_NEW_SIZE];
byte extra_rules[EXTRARULES_SIZE];
bool _qrs_unpacked[qr_MAX];
map<QSTCompilerSetting, int> qst_compiler_settings;

void unpack_qrs()
{
	for (int i = 0; i < qr_MAX; i++)
		_qrs_unpacked[i] = get_bit(quest_rules,i)!=0;
}
void set_qr(int index,bool state,byte* qstptr)
{
	assert(index < qr_MAX);
	if (qstptr == nullptr)
		qstptr = quest_rules;
	set_bit(qstptr,index,state);
	if (qstptr == quest_rules)
		_qrs_unpacked[index] = state;
}

bool get_er(int index)
{
	return get_bit(extra_rules,index)!=0;
}
void set_er(int index,bool state)
{
	set_bit(extra_rules,index,state);
}

std::string to_string(QSTCompilerSetting setting)
{
	switch (setting)
	{
		case QSTCompilerSetting::DEFAULT_STATIC_SCRIPT_MEMBERS:
			return "DEFAULT_STATIC_SCRIPT_MEMBERS";
	}
	return "INVALID_SETTING";
}

