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
zfix vbound(zfix val, int low, zfix high)
{
	if(low > high) zc_swap(low,high);
	if(val < low) return low;
	if(val > high) return high;
	return val;
}
zfix vbound(zfix val, zfix low, int high)
{
	if(low > high) zc_swap(low,high);
	if(val < low) return low;
	if(val > high) return high;
	return val;
}
zfix vbound(zfix val, int low, int high)
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

int edef_to_wtype(int edef, bool fuzzy)
{
	switch(edef)
	{
		case edefBRANG: return wBrang;
		case edefBOMB: return wBomb;
		case edefSBOMB: return wSBomb;
		case edefARROW: return wArrow;
		case edefFIRE: return wFire;
		case edefWAND: return wWand;
		case edefMAGIC: return wMagic;
		case edefHOOKSHOT: return wHookshot;
		case edefHAMMER: return wHammer;
		case edefSWORD: return wSword;
		case edefBEAM: return wBeam;
		case edefREFBEAM: return wRefBeam;
		case edefREFMAGIC: return wRefMagic;
		case edefREFBALL: return wRefFireball;
		case edefREFROCK: return wRefRock;
		case edefSTOMP: return wStomp;
		case edefBYRNA: return wCByrna;
		case edefSCRIPT: return fuzzy ? wScript1 : -1;
		case edef_UNIMPL: return -1;
		case edefQUAKE: return -1;
		case edefSCRIPT01: return wScript1;
		case edefSCRIPT02: return wScript2;
		case edefSCRIPT03: return wScript3;
		case edefSCRIPT04: return wScript4;
		case edefSCRIPT05: return wScript5;
		case edefSCRIPT06: return wScript6;
		case edefSCRIPT07: return wScript7;
		case edefSCRIPT08: return wScript8;
		case edefSCRIPT09: return wScript9;
		case edefSCRIPT10: return wScript10;
		case edefICE: return wIce;
		case edefBAIT: return wBait;
		case edefWIND: return wWind;
		case edefSPARKLE: return wSSparkle;
		case edefSONIC: return wSound;
		case edefWhistle: return wWhistle;
		case edefSwitchHook: return wSwitchHook;
		case edefTHROWN: return wThrown;
		case edefREFARROW: return wRefArrow;
		case edefREFFIRE: return wRefFire;
		case edefREFFIRE2: return wRefFire2;
	}
	return -1;
}
int wtype_to_edef(int wtype)
{
	switch(wtype)
	{
		case wBrang: return edefBRANG;
		case wBomb: return edefBOMB;
		case wSBomb: return edefSBOMB;
		case wArrow: return edefARROW;
		case wFire: return edefFIRE;
		case wWand: return edefWAND;
		case wMagic: return edefMAGIC;
		case wHookshot: return edefHOOKSHOT;
		case wHammer: return edefHAMMER;
		case wSword: return edefSWORD;
		case wBeam: return edefBEAM;
		case wRefBeam: return edefREFBEAM;
		case wRefMagic: return edefREFMAGIC;
		case wRefFireball: return edefREFBALL;
		case wRefRock: return edefREFROCK;
		case wStomp: return edefSTOMP;
		case wCByrna: return edefBYRNA;
		case wScript1: return edefSCRIPT01;
		case wScript2: return edefSCRIPT02;
		case wScript3: return edefSCRIPT03;
		case wScript4: return edefSCRIPT04;
		case wScript5: return edefSCRIPT05;
		case wScript6: return edefSCRIPT06;
		case wScript7: return edefSCRIPT07;
		case wScript8: return edefSCRIPT08;
		case wScript9: return edefSCRIPT09;
		case wScript10: return edefSCRIPT10;
		case wIce: return edefICE;
		case wBait: return edefBAIT;
		case wWind: return edefWIND;
		case wSSparkle: return edefSPARKLE;
		case wSound: return edefSONIC;
		case wWhistle: return edefWhistle;
		case wSwitchHook: return edefSwitchHook;
		case wThrown: return edefTHROWN;
		case wRefArrow: return edefREFARROW;
		case wRefFire: return edefREFFIRE;
		case wRefFire2: return edefREFFIRE2;
	}
	return -1;
}
bool unimpl_edef(int edef)
{
	switch(edef)
	{
		case edef_UNIMPL: case edefQUAKE:
			return true;
	}
	return false;
}

direction X_DIR(int32_t dir)
{
	dir = NORMAL_DIR(dir);
	if(dir < 0) return dir_invalid;
	return xDir[dir];
}
direction Y_DIR(int32_t dir)
{
	dir = NORMAL_DIR(dir);
	if(dir < 0) return dir_invalid;
	return yDir[dir];
}
direction XY_DIR(int32_t xdir, int32_t ydir)
{
	if(X_DIR(xdir) < 0) return NORMAL_DIR(ydir);
	if(Y_DIR(ydir) < 0) return NORMAL_DIR(xdir);
	switch(X_DIR(xdir))
	{
		case right:
			switch(Y_DIR(ydir))
			{
				case up: return r_up;
				case down: return r_down;
			}
			break;
		case left:
			switch(Y_DIR(ydir))
			{
				case up: return l_up;
				case down: return l_down;
			}
			break;
	}
	return dir_invalid;
}
direction GET_XDIR(zfix const& sign)
{
	if(sign < 0) return left;
	if(sign) return right;
	return dir_invalid;
}
direction GET_YDIR(zfix const& sign)
{
	if(sign < 0) return up;
	if(sign) return down;
	return dir_invalid;
}
direction GET_DIR(zfix const& dx, zfix const& dy)
{
	return XY_DIR(GET_XDIR(dx), GET_YDIR(dy));
}

direction XY_DELTA_TO_DIR(int32_t dx, int32_t dy)
{
	if (dx == 0 && dy == 0) return dir_invalid;
	if (dx == 1 && dy == 0) return right;
	if (dx == -1 && dy == 0) return left;
	if (dx == 0 && dy == 1) return down;
	if (dx == 0 && dy == -1) return up;
	return dir_invalid;
}

bool viewport_t::intersects_with(int x, int y, int w, int h) const
{
	int l = x;
	int r = x + w;
	int t = y;
	int b = y + h;
	return left() <= r && right() > l && top() <= b && bottom() > t;
}

bool viewport_t::contains_point(int x0, int y0) const
{
	return left() <= x0 && right() > x0 && top() <= y0 && bottom() > y0;
}

bool viewport_t::contains_or_on(const viewport_t& other) const
{
	// The `=` equality portions of all these conditions makes this more than just a "contains" check.
	return other.right() <= right() && other.left() >= left() && other.top() >= top() && other.bottom() <= bottom();
}

int32_t viewport_t::visible_height(bool show_bottom_8px) const
{
	return h - (show_bottom_8px ? 0 : 8);
}

int32_t viewport_t::left() const
{
	return x;
}

int32_t viewport_t::right() const
{
	return x + w;
}

int32_t viewport_t::top() const
{
	return y;
}

int32_t viewport_t::bottom() const
{
	return y + h;
}
