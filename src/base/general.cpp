#include "general.h"
#include <assert.h>

void set_bit(byte *bitstr,int32_t bit,bool val)
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

void set_bitl(int32_t bitstr,int32_t bit,bool val)
{
	if (val)
		bitstr |= (1<<bit);
	else bitstr &= ~(1<<bit);
}

int32_t get_bitl(int32_t bitstr,int32_t bit)
{
    return bitstr&(1<<bit); //intel u32
}

int vbound(int val, int low, int high)
{
	if(low > high) zc_swap(low,high);
	if(val < low) return low;
	if(val > high) return high;
	return val;
}
double vbound(double val, double low, double high)
{
	if(low > high) zc_swap(low,high);
	if(val < low) return low;
	if(val > high) return high;
	return val;
}
zfix vbound(zfix val, zfix low, zfix high)
{
	if(low > high) zc_swap(low,high);
	if(val < low) return low;
	if(val > high) return high;
	return val;
}

int wrap(int x,int low,int high)
{
    while(x<low)
        x+=high-low+1;

    while(x>high)
        x-=high-low+1;

    return x;
}

