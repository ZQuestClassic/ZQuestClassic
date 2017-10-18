#ifndef _ZC_FFC_H_
#define _ZC_FFC_H_

#include "refInfo.h"
#include "types.h"

//FF combo flags
#define ffOVERLAY       0x00000001
#define ffTRANS         0x00000002
#define ffSOLID         0x00000004
#define ffCARRYOVER     0x00000008
#define ffSTATIONARY    0x00000010
#define ffCHANGER       0x00000020 //Is a changer
#define ffPRELOAD       0x00000040 //Script is run before screen appears.
#define ffLENSVIS       0x00000080 //Invisible, but not to the Lens of Truth.
#define ffSCRIPTRESET	0x00000100 //Script resets when carried over.
#define ffETHEREAL      0x00000200 //Does not occlude combo and flags on the screen
#define ffIGNOREHOLDUP  0x00000400 //Updated even while Link is holding an item

//FF combo changer flags
#define ffSWAPNEXT      0x80000000 //Swap speed with next FFC
#define ffSWAPPREV      0x40000000 //Swap speed with prev. FFC
#define ffCHANGENEXT    0x20000000 //Increase combo ID
#define ffCHANGEPREV    0x10000000 //Decrease combo ID
#define ffCHANGETHIS    0x08000000 //Change combo/cset to this
#define ffCHANGESPEED   0x04000000 //Change speed to this (default, not implemented yet)

struct FFC
{
    enum copyType { copy_full, copy_pasteSingle, copy_carryOver };
    
    FFC();
    bool isVisible(bool lensActive) const;
    bool isUnderPoint(int px, int py) const;
    void clear();
    void copy(const FFC& other, copyType type);
    void applyChanger(FFC& changer);
    inline bool overlay() const { return (flags&ffOVERLAY)!=0; }
    inline bool carriesOver() const { return (flags&ffCARRYOVER)!=0; }
    
    inline int getXInt() const { return x/10000; }
    inline int getYInt() const { return y/10000; }
    
    inline word getCombo() const { return data; }
    inline void setCombo(word c) { data=c; }
    inline void modCombo(short amt) { data+=amt; }
    inline word getCSet() const { return cset; }
    inline void setCSet(word c) { cset=c; }
    
    inline dword getFlags() const { return flags; }
    
    // This sort of thing is why bit fields exist. :p
    inline void setEffectWidth(int w) { width=(width&0xC0)|(w-1); }
    inline void setEffectHeight(int h) { height=(height&0xC0)|(h-1); }
    inline byte getEffectWidth() const { return (width&0x3F)+1; }
    inline byte getEffectHeight() const { return (height&0x3F)+1; }
    
    inline void setTileWidth(int w) { width=((w-1)<<6)|(width&0x3F); }
    inline void setTileHeight(int h) { height=((h-1)<<6)|(height&0x3F); }
    inline byte getTileWidth() const { return (width>>6)+1; }
    inline byte getTileHeight() const { return (height>>6)+1; }
    
    inline word getScript() const { return script; }
    
//private: // Too much trouble. Probably will be for a while.
    word data;
    byte cset;
    word delay;
    long x, y;
    long xVel, yVel;
    long xAccel, yAccel;
    byte width, height;
    dword flags;
    byte link;
    
    word script;
    long misc[16];
    refInfo scriptData;
    long initd[8], inita[2];
    bool initialized;
    
private:
    short lastChangerX, lastChangerY;
    long prevX, prevY;
    
    bool canMove();
    void updateMovement(FFC* linked);
    
    /// Called when moving to see if the FFC has gone of the edge of the screen.
    void checkScreenEdges(bool wrap);
    
    friend class FFCSet;
};

/// The set of 32 FFCs on a single screen.
class FFCSet
{
public:
    FFCSet();
    
    void updateMovement(struct mapscr* tmpscr, bool holdingItem);
    inline void clear()
    {
        for(int i=0; i<32;  i++)
            ffcs[i].clear();
    }
    
    inline FFC& operator[](int index) { return ffcs[index]; }
    inline const FFC& operator[](int index) const { return ffcs[index]; }
    
    // For AngelScript
    inline FFC* getPtr(int index) { return ffcs+index; }
    
private:
    FFC ffcs[32];
    
    /// Called when a changer with a swap flag is triggered.
    void swapData(FFC& f1, FFC& f2);
};


class SparseFFCSetArray
{
public:
	enum { MAX_SCREENS = 65535 };

	/// Gets a screen ffcset. Can be null.
	FFCSet* GetFFCSet(int screenIndex)
	{
		//assert(screenIndex < MAX_SCREENS);
		return screenFFCSets[screenIndex];
	}

	/// Gets a screen ffcset. If it does not exist a new ffcset is created.
	FFCSet* GetOrCreateFFCSet(int screenIndex)
	{
		//assert(screenIndex < MAX_SCREENS);
		if(screenFFCSets[screenIndex] == 0)
			screenFFCSets[screenIndex] = new FFCSet();

		return screenFFCSets[screenIndex];
	}

	void Clear()
	{
		for(int i(0); i < MAX_SCREENS; ++i)
		{
			if(screenFFCSets[i])
			{
				delete screenFFCSets[i];
				screenFFCSets[i] = 0;
			}
		}
	}

	FFCSet blankFFCSet;
	FFCSet* screenFFCSets[MAX_SCREENS];
};


#endif
