#include "general.h"

void set_bit(byte *bitstr,int32_t bit,byte val)
{
    bitstr += bit>>3;
    byte mask = 1 << (bit&7);
    
    if(val)
        *bitstr |= mask;
    else
        *bitstr &= ~mask;
}

int32_t get_bit(byte const *bitstr,int32_t bit)
{
    bitstr += bit>>3;
    return ((*bitstr) >> (bit&7))&1;
}

bool toggle_bit(byte *bitstr,int32_t bit)
{
	bitstr += bit>>3;
    byte mask = 1 << (bit&7);
	*bitstr ^= mask;
	return *bitstr&mask;
}

void set_bitl(int32_t bitstr,int32_t bit,byte val)
{
	if (val)
		bitstr |= (1<<bit);
	else bitstr &= ~(1<<bit);
}

int32_t get_bitl(int32_t bitstr,int32_t bit)
{
    return bitstr&(1<<bit); //intel u32
}

