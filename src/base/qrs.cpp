#include "qrs.h"
#include "general.h"

byte quest_rules[QUESTRULES_NEW_SIZE];
byte extra_rules[EXTRARULES_SIZE];

bool get_qr(int index)
{
	return get_bit(quest_rules,index)!=0;
}
void set_qr(int index,bool state)
{
	set_bit(quest_rules,index,state);
}
bool get_er(int index)
{
	return get_bit(extra_rules,index)!=0;
}
void set_er(int index,bool state)
{
	set_bit(extra_rules,index,state);
}

