#include "qrs.h"
#include "general.h"

byte quest_rules[QUESTRULES_NEW_SIZE];
byte extra_rules[EXTRARULES_SIZE];

bool get_qr(int index,byte* qrptr)
{
	if(!qrptr) qrptr = quest_rules;
	return get_bit(qrptr,index)!=0;
}
void set_qr(int index,bool state,byte* qrptr)
{
	if(!qrptr) qrptr = quest_rules;
	set_bit(qrptr,index,state);
}
bool get_er(int index)
{
	return get_qr(index,extra_rules);
}
void set_er(int index,bool state)
{
	set_qr(index,state,extra_rules);
}

