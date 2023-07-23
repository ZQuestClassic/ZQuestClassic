#include "qrs.h"

byte quest_rules[QUESTRULES_NEW_SIZE];
byte extra_rules[EXTRARULES_SIZE];

int32_t get_bit(byte const* bitstr,int32_t bit);
void set_bit(byte *bitstr,int32_t bit,byte val);

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

