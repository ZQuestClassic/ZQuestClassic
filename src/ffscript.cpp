#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <deque>
//#include <algorithm>
#include <string>
#include <sstream>
#include <math.h>
#include <cstdio>

#include "ffasm.h"
#include "zc_sys.h"
#include "zc_math.h"
#include "zc_array.h"
#include "ffscript.h"
FFScript FFCore;
zquestheader ZCheader;
//miscQdata *Misc;

#include "zelda.h"
#include "link.h"
#include "guys.h"

#include "gamedata.h"
#include "zc_init.h"
#include "zsys.h"
#include "title.h"
#include "mem_debug.h"
#include "zscriptversion.h"

#include "pal.h"
#include "zdefs.h"
#include "zq_class.h"
#include "rendertarget.h" //Needed for LoadBitmap

#include "zc_custom.h"

#ifdef _FFDEBUG
#include "ffdebug.h"
#endif

#include "debug.h"

#define zc_max(a,b)  ((a)>(b)?(a):(b))
/*template <typename T>
T zc_max(T a, T b)
{
	return (a > b) ? a : b;
}*/

#define zc_min(a,b)  ((a)<(b)?(a):(b))

#ifdef _MSC_VER
#pragma warning ( disable : 4800 ) //int to bool town. population: lots.
#endif

#define NUMSHOPS 256
#define NUMINFOSHOPS 511

//! New datatype vars for 2.54:

//spritedata sp->member


using std::string;

extern sprite_list particles;
extern LinkClass Link;
extern char *guy_string[];
extern int skipcont;
extern std::map<int, std::pair<string,string> > ffcmap;
extern std::map<int, std::pair<string,string> > itemmap;
extern std::map<int, std::pair<string,string> > globalmap;

PALETTE tempgreypal; //Palettes go here. This is used for Greyscale() / Monochrome()

FFScript ffengine;

byte FF_rules[512]; //For Migration of Quest Rules, and Scritp Engine Rules
long FF_link_tile;	//Overrides for the tile used when blitting Limk to the bitmap; and a var to hold a script-set action/
byte FF_link_action; //This way, we can make safe replicas of internal Link actions to be set by script. 
	
long FF_screenbounds[4]; //edges of the screen, left, right, top, bottom used for where to scroll. 
long FF_screen_dimensions[4]; //height, width, displaywidth, displayheight
long FF_subscreen_dimensions[4];
long FF_eweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic eweapon removal. 
long FF_lweapon_removal_bounds[4]; //left, right, top, bottom coordinates for automatic lweapon removal. 
long FF_clocks[FFSCRIPTCLASS_CLOCKS]; //Will be used for Linkaction, anims, and so forth 
byte ScriptDrawingRules[SCRIPT_DRAWING_RULES];
long FF_UserMidis[NUM_USER_MIDI_OVERRIDES]; //MIDIs to use for Game Over, and similar to override system defaults. 
    
miscQdata *misc;

//We gain some speed by not passing as arguments
long sarg1 = 0;
long sarg2 = 0;
refInfo *ri = NULL;
ffscript *curscript = NULL;

static int numInstructions; // Used to detect hangs
static bool scriptCanSave = true;
byte curScriptType;
word curScriptNum;

//Global script data
refInfo globalScriptData;
word g_doscript = 1;
bool global_wait = false;

//Item script data
refInfo itemScriptData;

//The stacks
//This is where we need to change the formula. These stacks need to be variable in some manner
//to permit adding additional scripts to them, without manually sizing them in advance. - Z
long(*stack)[256] = NULL;
long ffc_stack[32][256];
long global_stack[256];
long item_stack[256];
long ffmisc[32][16];
refInfo ffcScriptData[32];

void clear_ffc_stack(const byte i)
{
    memset(ffc_stack[i], 0, 256 * sizeof(long));
}

void clear_global_stack()
{
    memset(global_stack, 0, 256 * sizeof(long));
}

//ScriptHelper
class SH
{

///-----------------------------//
//           Errors             //
///-----------------------------//

public:

    enum __Error
    {
        _NoError, //OK!
        _Overflow, //script array too small
        _InvalidPointer, //passed NULL pointer or similar
        _OutOfBounds, //library array out of bounds
        _InvalidSpriteUID //bad npc, ffc, etc.
    };
    
#define INVALIDARRAY localRAM[0]  //localRAM[0] is never used
    
    //only if the player is messing with their pointers...
    static ZScriptArray& InvalidError(const long ptr)
    {
        Z_scripterrlog("Invalid pointer (%i) passed to array (don't change the values of your array pointers)\n", ptr);
        return INVALIDARRAY;
    }
    
    static void write_stack(const int stackoffset, const long value)
    {
        if(stackoffset == 0)
        {
            Z_scripterrlog("Stack over or underflow, stack pointer = %ld\n", stackoffset);
            return;
        }
        
        (*stack)[stackoffset] = value;
    }
    
    static long read_stack(const int stackoffset)
    {
        if(stackoffset == 0)
        {
            Z_scripterrlog("Stack over or underflow, stack pointer = %ld\n", stackoffset);
            return -10000;
        }
        
        return (*stack)[stackoffset];
    }
    
    static INLINE long get_arg(long arg, bool v)
    {
        return v ? arg : get_register(arg);
    }
};

///----------------------------//
//           Misc.             //
///----------------------------//

//Miscellaneous Helper
class MiscH : public SH
{

public:

};

byte flagpos;
int ornextflag(bool flag)
{
    int f = (flag?1:0)<<flagpos;
    flagpos++;
    return f;
}

long get_screenflags(mapscr *m, int flagset)
{
    int f=0;
    flagpos = 0;
    
    switch(flagset)
    {
    case 0: // Room Type
        f = ornextflag(m->flags6&1)  | ornextflag(m->flags6&2) | ornextflag(m->flags7&8);
        break;
        
    case 1: // View
        f = ornextflag(m->flags3&8)  | ornextflag(m->flags7&16) | ornextflag(m->flags3&16)
            | ornextflag(m->flags3&64) | ornextflag(m->flags7&2)  | ornextflag(m->flags7&1)
            | ornextflag(m->flags&4);
        break;
        
    case 2: // Secrets
        f = ornextflag(m->flags&1)   | ornextflag(m->flags5&16) | ornextflag(m->flags6&4)
            | ornextflag(m->flags6&32);
        break;
        
    case 3: // Warp
        f = ornextflag(m->flags5&4)  | ornextflag(m->flags5&8)  | ornextflag(m->flags&64)
            | ornextflag(m->flags8&64) | ornextflag(m->flags3&32);
        break;
        
    case 4: // Item
        f = ornextflag(m->flags3&1)  | ornextflag(m->flags7&4);
        break;
        
    case 5: // Combo
        f = ornextflag((m->flags2>>4)&2)| ornextflag(m->flags3&2)| ornextflag(m->flags5&2)
            | ornextflag(m->flags6&64);
        break;
        
    case 6: // Save
        f = ornextflag(m->flags4&64) | ornextflag(m->flags4&128) | ornextflag(m->flags6&8)
            | ornextflag(m->flags6&16);
        break;
        
    case 7: // FFC
        f = ornextflag(m->flags6&128)| ornextflag(m->flags5&128);
        break;
        
    case 8: // Whistle
        f = ornextflag(m->flags&16)  | ornextflag(m->flags7&64)  | ornextflag(m->flags7&128);
        break;
        
    case 9: // Misc
        f = ornextflag(m->flags&32)  | ornextflag(m->flags5&64)  | m->flags8<<2;
        break;
    }
    
    return f*10000;
}

long get_screeneflags(mapscr *m, int flagset)
{
    int f=0;
    flagpos = 0;
    
    switch(flagset)
    {
    case 0:
        f = m->enemyflags&0x1F;
        break;
        
    case 1:
        f = ornextflag(m->enemyflags&32) | ornextflag(m->enemyflags&64) | ornextflag(m->flags3&4)
            | ornextflag(m->enemyflags&128)| ornextflag((m->flags2>>4)&4);
        break;
        
    case 2:
        f = ornextflag(m->flags3&128)    | ornextflag(m->flags&2)       | ornextflag((m->flags2>>4)&8)
            | ornextflag(m->flags4&16);
        break;
    }
    
    return f*10000;
}


///------------------------------------------------//
//           Bounds Checking Functions             //
///------------------------------------------------//

//Bounds Checker
class BC : public SH
{
public:

    static INLINE int checkMapID(const long ID, const char * const str)
    {
        //return checkBounds(ID, 0, map_count-1, str);
        if(ID < 0 || ID > map_count-1)
        {
            Z_scripterrlog("Invalid value (%i) passed to '%s'\n", ID+1, str);
            return _OutOfBounds;
        }
        
        return _NoError;
    }
    
    static INLINE int checkDMapID(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, MAXDMAPS-1, str);
    }
    
    static INLINE int checkComboPos(const long pos, const char * const str)
    {
        return checkBounds(pos, 0, 175, str);
    }
    
    static INLINE int checkTile(const long pos, const char * const str)
    {
        return checkBounds(pos, 0, NEWMAXTILES-1, str);
    }
    
    static INLINE int checkCombo(const long pos, const char * const str)
    {
        return checkBounds(pos, 0, MAXCOMBOS-1, str);
    }
    
    static INLINE int checkMisc(const long a, const char * const str)
    {
        return checkBounds(a, 0, 15, str);
    }
    
     static INLINE int checkMisc32(const long a, const char * const str)
    {
        return checkBounds(a, 0, 31, str);
    }
    
    static INLINE int checkMessage(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, msg_strings_size-1, str);
    }
    
    static INLINE int checkLayer(const long layer, const char * const str)
    {
        return checkBounds(layer, 0, 6, str);
    }
    
    static INLINE int checkFFC(const long ffc, const char * const str)
    {
        return checkBounds(ffc, 0, MAXFFCS-1, str);
    }
    
    static INLINE int checkGuyIndex(const long index, const char * const str)
    {
        return checkBounds(index, 0, guys.Count()-1, str);
    }
    
    static INLINE int checkItemIndex(const long index, const char * const str)
    {
        return checkBounds(index, 0, items.Count()-1, str);
    }
    
    static INLINE int checkEWeaponIndex(const long index, const char * const str)
    {
        return checkBounds(index, 0, Ewpns.Count()-1, str);
    }
    
    static INLINE int checkLWeaponIndex(const long index, const char * const str)
    {
        return checkBounds(index, 0, Lwpns.Count()-1, str);
    }
    
    static INLINE int checkGuyID(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, MAXGUYS-1, str);
    }
    
    static INLINE int checkItemID(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, ITEMCNT-1, str);
    }
    
    static INLINE int checkWeaponID(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, WPNCNT-1, str);
    }
    
    static INLINE int checkWeaponMiscSprite(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, MAXWPNS-1, str);
    }
    
    static INLINE int checkSFXID(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, WAV_COUNT-1, str);
    }
    
    static INLINE int checkBounds(const long n, const long boundlow, const long boundup, const char * const funcvar)
    {
        if(n < boundlow || n > boundup)
        {
            Z_scripterrlog("Invalid value (%i) passed to '%s'\n", n, funcvar);
            return _OutOfBounds;
        }
        
        return _NoError;
    }
    
    static INLINE int checkUserArrayIndex(const long index, const dword size)
    {
        if(index < 0 || index >= long(size))
        {
            Z_scripterrlog("Invalid index (%ld) to local array of size %ld\n", index, size);
            return _OutOfBounds;
        }
        
        return _NoError;
    }
};

///------------------------------------------------//
//           Pointer Handling Functions          //
///------------------------------------------------//
//MUST call AND check load functions before trying to use other functions

//Guy Helper
class GuyH : public SH
{

public:
    static int loadNPC(const long eid, const char * const funcvar)
    {
        tempenemy = (enemy *) guys.getByUID(eid);
        
        if(tempenemy == NULL)
        {
            Z_scripterrlog("Invalid NPC with UID %ld passed to %s\nNPCs on screen have UIDs ", eid, funcvar);
            
            for(word i = 0; i < guys.Count(); i++)
                Z_scripterrlog("%ld ", guys.spr(i)->getUID());
                
            Z_scripterrlog("\n");
            return _InvalidSpriteUID;
        }
        
        return _NoError;
    }
    
    static INLINE enemy *getNPC()
    {
        return tempenemy;
    }
    
    // Currently only used in a context where the enemy is known to be valid,
    // so there's no need to print an error
    static int getNPCIndex(const long eid)
    {
        for(word i = 0; i < guys.Count(); i++)
        {
            if(guys.spr(i)->getUID() == eid)
                return i;
        }
        
        return -1;
    }
    
    static long getNPCDMisc(const byte a)
    {
        switch(a)
        {
        case 0: return tempenemy->dmisc1;
        case 1: return tempenemy->dmisc2;
        case 2: return tempenemy->dmisc3;
        case 3: return tempenemy->dmisc4;
        case 4: return tempenemy->dmisc5;
        case 5: return tempenemy->dmisc6;
        case 6: return tempenemy->dmisc7;
        case 7: return tempenemy->dmisc8;
        case 8: return tempenemy->dmisc9;
        case 9: return tempenemy->dmisc10;
        case 10: return tempenemy->dmisc11;
        case 11: return tempenemy->dmisc12;
        case 12: return tempenemy->dmisc13;
        case 13: return tempenemy->dmisc14;
        case 14: return tempenemy->dmisc15;
        case 15: return tempenemy->dmisc16;
        case 16: return tempenemy->dmisc17;
        case 17: return tempenemy->dmisc18;
        case 18: return tempenemy->dmisc19;
        case 19: return tempenemy->dmisc20;
        case 20: return tempenemy->dmisc21;
        case 21: return tempenemy->dmisc22;
        case 22: return tempenemy->dmisc23;
        case 23: return tempenemy->dmisc24;
        case 24: return tempenemy->dmisc25;
        case 25: return tempenemy->dmisc26;
        case 26: return tempenemy->dmisc27;
        case 27: return tempenemy->dmisc28;
        case 28: return tempenemy->dmisc29;
        case 29: return tempenemy->dmisc30;
        case 30: return tempenemy->dmisc31;
        case 31: return tempenemy->dmisc32;
        }
        
        return 0;
    }
    
    static bool hasLink()
    {
        if(tempenemy->family == eeWALLM)
            return ((eWallM *) tempenemy)->haslink;
            
        if(tempenemy->family == eeWALK)
            return ((eStalfos *) tempenemy)->haslink;
            
        return false;
    }
    
    static long getMFlags()
    {
        flagpos = 5;
        // Must be in the same order as in the Enemy Editor pane
        return (tempenemy->flags&0x1F)
               | ornextflag(tempenemy->flags&(lens_only))
               | ornextflag(tempenemy->flags2&(guy_flashing))
               | ornextflag(tempenemy->flags2&(guy_blinking))
               | ornextflag(tempenemy->flags2&(guy_transparent))
               | ornextflag(tempenemy->flags&(inv_front))
               | ornextflag(tempenemy->flags&(inv_left))
               | ornextflag(tempenemy->flags&(inv_right))
               | ornextflag(tempenemy->flags&(inv_back))
               | ornextflag(tempenemy->flags&(guy_bkshield));
    }
    
    static INLINE void clearTemp()
    {
        tempenemy = NULL;
    }
    
private:

    static enemy *tempenemy;
};

enemy *GuyH::tempenemy = NULL;

//Item Helper
class ItemH : public SH
{

public:
    static int loadItem(const long iid, const char * const funcvar)
    {
        tempitem = (item *) items.getByUID(iid);
        
        if(tempitem == NULL)
        {
            Z_scripterrlog("Invalid item with UID %ld passed to %s\nItems on screen have UIDs ", iid, funcvar);
            
            for(word i = 0; i < items.Count(); i++)
                Z_scripterrlog("%ld ", items.spr(i)->getUID());
                
            Z_scripterrlog("\n");
            return _InvalidSpriteUID;
        }
        
        return _NoError;
    }
    
    static INLINE item* getItem()
    {
        return tempitem;
    }
    
    static INLINE void clearTemp()
    {
        tempitem = NULL;
    }
    
private:

    static item *tempitem;
};

item *ItemH::tempitem = NULL;

//LWeapon Helper
class LwpnH : public SH
{

public:
    static int loadWeapon(const long wid, const char * const funcvar)
    {
        tempweapon = (weapon *) Lwpns.getByUID(wid);
        
        if(tempweapon == NULL)
        {
            Z_scripterrlog("Invalid lweapon with UID %ld passed to %s\nLWeapons on screen have UIDs ", wid, funcvar);
            
            for(word i = 0; i < Lwpns.Count(); i++)
                Z_scripterrlog("%ld ", Lwpns.spr(i)->getUID());
                
            Z_scripterrlog("\n");
            return _InvalidSpriteUID;
        }
        
        return _NoError;
    }
    
    static INLINE weapon *getWeapon()
    {
        return tempweapon;
    }
    
    static INLINE void clearTemp()
    {
        tempweapon = NULL;
    }
    
private:

    static weapon *tempweapon;
};

weapon *LwpnH::tempweapon = NULL;

//EWeapon Helper
class EwpnH : public SH
{

public:
    static int loadWeapon(const long wid, const char * const funcvar)
    {
        tempweapon = (weapon *) Ewpns.getByUID(wid);
        
        if(tempweapon == NULL)
        {
            Z_scripterrlog("Invalid eweapon with UID %ld passed to %s\nEWeapons on screen have UIDs ", wid, funcvar);
            
            for(word i = 0; i < Ewpns.Count(); i++)
                Z_scripterrlog("%ld ", Ewpns.spr(i)->getUID());
                
            Z_scripterrlog("\n");
            return _InvalidSpriteUID;
        }
        
        return _NoError;
    }
    
    static INLINE weapon *getWeapon()
    {
        return tempweapon;
    }
    
    static INLINE void clearTemp()
    {
        tempweapon = NULL;
    }
    
private:

    static weapon *tempweapon;
};

weapon *EwpnH::tempweapon = NULL;

void clearScriptHelperData()
{
    GuyH::clearTemp();
    ItemH::clearTemp();
    LwpnH::clearTemp();
    EwpnH::clearTemp();
}

///---------------------------------------------//
//           Array Helper Functions           //
///---------------------------------------------//

//Array Helper
class ArrayH : public SH
{
public:

    //Returns a reference to the correct array based on pointer passed
    static ZScriptArray& getArray(const long ptr)
    {
        if(ptr <= 0)
            return InvalidError(ptr);
            
        if(ptr >= MAX_ZCARRAY_SIZE) //Then it's a global
        {
            dword gptr = ptr - MAX_ZCARRAY_SIZE;
            
            if(gptr > game->globalRAM.size())
                return InvalidError(ptr);
                
            return game->globalRAM[gptr];
        }
        else
        {
            if(localRAM[ptr].Size() == 0)
                return InvalidError(ptr);
                
            return localRAM[ptr];
        }
    }
    
    static size_t getSize(const long ptr)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return size_t(-1);
            
        return a.Size();
    }
    
    //Can't you get the std::string and then check its length?
    static int strlen(const long ptr)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return -1;
            
        word count;
        
        for(count = 0; BC::checkUserArrayIndex(count, a.Size()) == _NoError && a[count] != '\0'; count++) ;
        
        return count;
    }
    
    //Returns values of a zscript array as an std::string.
    static void getString(const long ptr, string &str, word num_chars = 256)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
        {
            str.clear();
            return;
        }
        
        str.clear();
        
        for(word i = 0; BC::checkUserArrayIndex(i, a.Size()) == _NoError && a[i] != '\0' && num_chars != 0; i++)
        {
            str += char(a[i] / 10000);
            num_chars--;
        }
    }
    
    //Like getString but for an array of longs instead of chars. *(arrayPtr is not checked for validity)
    static void getValues(const long ptr, long* arrayPtr, word num_values)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return;
            
        for(word i = 0; BC::checkUserArrayIndex(i, a.Size()) == _NoError && num_values != 0; i++)
        {
            arrayPtr[i] = (a[i] / 10000);
            num_values--;
        }
    }
    
    //Get element from array
    static INLINE long getElement(const long ptr, const long offset)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return -10000;
            
        if(BC::checkUserArrayIndex(offset, a.Size()) == _NoError)
            return a[offset];
        else
            return -10000;
    }
    
    //Set element in array
    static INLINE void setElement(const long ptr, const long offset, const long value)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return;
            
        if(BC::checkUserArrayIndex(offset, a.Size()) == _NoError)
            a[offset] = value;
    }
    
    //Puts values of a zscript array into a client <type> array. returns 0 on success. Overloaded
    template <typename T>
    static int getArray(const long ptr, T *refArray)
    {
        return getArray(ptr, getArray(ptr).Size(), 0, 0, 0, refArray);
    }
    
    template <typename T>
    static int getArray(const long ptr, const word size, T *refArray)
    {
        return getArray(ptr, size, 0, 0, 0, refArray);
    }
    
    template <typename T>
    static int getArray(const long ptr, const word size, word userOffset, const word userStride, const word refArrayOffset, T *refArray)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return _InvalidPointer;
            
        word j = 0, k = userStride;
        
        for(word i = 0; j < size; i++)
        {
            if(i >= a.Size())
                return _Overflow;
                
            if(userOffset-- > 0)
                continue;
                
            if(k > 0)
                k--;
            else if(BC::checkUserArrayIndex(i, a.Size()) == _NoError)
            {
                refArray[j + refArrayOffset] = T(a[i]);
                k = userStride;
                j++;
            }
        }
        
        return _NoError;
    }
    
    
    static int setArray(const long ptr, const string s2)
    {
        ZScriptArray &a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return _InvalidPointer;
            
        word i;
        
        for(i = 0; i < s2.size(); i++)
        {
            if(i >= a.Size())
            {
                a.Back() = '\0';
                return _Overflow;
            }
            
            if(BC::checkUserArrayIndex(i, a.Size()) == _NoError)
                a[i] = s2[i] * 10000;
        }
        
        if(BC::checkUserArrayIndex(i, a.Size()) == _NoError)
            a[i] = '\0';
            
        return _NoError;
    }
    
    //Puts values of a client <type> array into a zscript array. returns 0 on success. Overloaded
    template <typename T>
    static int setArray(const long ptr, const word size, T *refArray)
    {
        return setArray(ptr, size, 0, 0, 0, refArray);
    }
    
    template <typename T>
    static int setArray(const long ptr, const word size, word userOffset, const word userStride, const word refArrayOffset, T *refArray)
    {
        ZScriptArray& a = getArray(ptr);
        
        if(a == INVALIDARRAY)
            return _InvalidPointer;
            
        word j = 0, k = userStride;
        
        for(word i = 0; j < size; i++)
        {
            if(i >= a.Size())
                return _Overflow; //Resize?
                
            if(userOffset-- > 0)
                continue;
                
            if(k > 0)
                k--;
            else if(BC::checkUserArrayIndex(i, a.Size()) == _NoError)
            {
                a[i] = long(refArray[j + refArrayOffset]) * 10000;
                k = userStride;
                j++;
            }
        }
        
        return _NoError;
    }
};

// Called when leaving a screen; deallocate arrays created by FFCs that aren't carried over
void deallocateArray(const long ptrval)
{
    if(ptrval<=0 || ptrval >= MAX_ZCARRAY_SIZE)
        Z_scripterrlog("Script tried to deallocate memory at invalid address %ld\n", ptrval);
    else
    {
        arrayOwner[ptrval] = 255;
        
        if(localRAM[ptrval].Size() == 0)
            Z_scripterrlog("Script tried to deallocate memory that was not allocated at address %ld\n", ptrval);
        else
        {
            word size = localRAM[ptrval].Size();
            localRAM[ptrval].Clear();
            
            // If this happens once per frame, it can drown out every other message. -L
            //Z_eventlog("Deallocated local array with address %ld, size %d\n", ptrval, size);
            size = size;
        }
    }
}

item *checkItem(long iid)
{
    item *s = (item *)items.getByUID(iid);
    
    if(s == NULL)
    {
        Z_eventlog("Script attempted to reference a nonexistent item!\n");
        Z_eventlog("You were trying to reference an item with UID = %ld; Items on screen are UIDs ", iid);
        
        for(int i=0; i<items.Count(); i++)
        {
            Z_eventlog("%ld ", items.spr(i)->getUID());
        }
        
        Z_eventlog("\n");
        return NULL;
    }
    
    return s;
}

weapon *checkLWpn(long eid, const char *what)
{
    weapon *s = (weapon *)Lwpns.getByUID(eid);
    
    if(s == NULL)
    {
    
        Z_eventlog("Script attempted to reference a nonexistent LWeapon!\n");
        Z_eventlog("You were trying to reference the %s of an LWeapon with UID = %ld; LWeapons on screen are UIDs ", what, eid);
        
        for(int i=0; i<Lwpns.Count(); i++)
        {
            Z_eventlog("%ld ", Lwpns.spr(i)->getUID());
        }
        
        Z_eventlog("\n");
        return NULL;
    }
    
    return s;
}

weapon *checkEWpn(long eid, const char *what)
{
    weapon *s = (weapon *)Ewpns.getByUID(eid);
    
    if(s == NULL)
    {
    
        Z_eventlog("Script attempted to reference a nonexistent EWeapon!\n");
        Z_eventlog("You were trying to reference the %s of an EWeapon with UID = %ld; EWeapons on screen are UIDs ", what, eid);
        
        for(int i=0; i<Ewpns.Count(); i++)
        {
            Z_eventlog("%ld ", Ewpns.spr(i)->getUID());
        }
        
        Z_eventlog("\n");
        return NULL;
    }
    
    return s;
}


int get_screen_d(long index1, long index2)
{
    if(index2 < 0 || index2 > 7)
    {
        Z_scripterrlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", index1);
        return 0;
    }
    
    return game->screen_d[index1][index2];
}

void set_screen_d(long index1, long index2, int val)
{
    if(index2 < 0 || index2 > 7)
    {
        Z_scripterrlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", index1);
        return;
    }
    
    game->screen_d[index1][index2] = val;
}

// If scr is currently being used as a layer, return that layer no.
int whichlayer(long scr)
{
    for(int i = 0; i < 6; i++)
    {
        if(scr == (tmpscr->layermap[i] - 1) * MAPSCRS + tmpscr->layerscreen[i])
            return i;
    }
    
    return -1;
}

sprite *s;



long get_register(const long arg)
{

    long ret = 0;
	
	//Macros
	
	#define GET_SPRITEDATA_VAR_INT(member, str) \
	{ \
		if(ri->spritesref < 0 || ri->spritesref > (MAXWPNS-1) )    \
		{ \
			ret = -10000; \
			Z_scripterrlog("Invalid Sprite ID passed to spritedata->%s: %d\n", (ri->spritesref*10000), str);\
		} \
		else \
			ret = (wpnsbuf[ri->spritesref].member * 10000); \
	}
    
    switch(arg)
    {
	    
	//Debug->Null()
    case DONULL: 
	ret = 0;
	break;
    
    //debug ri->d[]
    case DEBUGD:
    {
	int a = ri->d[0] / 10000;
	ret = ri->d[a] * 10000;
	break;
    }
    
///----------------------------------------------------------------------------------------------------//
//FFC Variables
    case DATA:
        ret = tmpscr->ffdata[ri->ffcref]*10000;
        break;
        
    case FFSCRIPT:
        ret = tmpscr->ffscript[ri->ffcref]*10000;
        break;
        
    case FCSET:
        ret = tmpscr->ffcset[ri->ffcref]*10000;
        break;
        
    case DELAY:
        ret = tmpscr->ffdelay[ri->ffcref]*10000;
        break;
        
    case FX:
        ret = tmpscr->ffx[ri->ffcref];
        break;
        
    case FY:
        ret = tmpscr->ffy[ri->ffcref];
        break;
        
    case XD:
        ret = tmpscr->ffxdelta[ri->ffcref];
        break;
        
    case YD:
        ret = tmpscr->ffydelta[ri->ffcref];
        break;
    case FFCID:
        ret=(ri->ffcref*10000)+10000;
        break;
        
    case XD2:
        ret = tmpscr->ffxdelta2[ri->ffcref];
        break;
        
    case YD2:
        ret = tmpscr->ffydelta2[ri->ffcref];
        break;
        
    case FFFLAGSD:
        ret=((tmpscr->ffflags[ri->ffcref] >> (ri->d[0] / 10000))&1) ? 10000 : 0;
        break;
        
    case FFCWIDTH:
        ret=((tmpscr->ffwidth[ri->ffcref]&0x3F)+1)*10000;
        break;
        
    case FFCHEIGHT:
        ret=((tmpscr->ffheight[ri->ffcref]&0x3F)+1)*10000;
        break;
        
    case FFTWIDTH:
        ret=((tmpscr->ffwidth[ri->ffcref]>>6)+1)*10000;
        break;
        
    case FFTHEIGHT:
        ret=((tmpscr->ffheight[ri->ffcref]>>6)+1)*10000;
        break;
        
    case FFLINK:
        ret=(tmpscr->fflink[ri->ffcref])*10000;
        break;
        
    case FFMISCD:
    {
        int a = ri->d[0] / 10000;
        
        if(BC::checkMisc(a, "ffc->Misc") != SH::_NoError)
            ret = -10000;
        else
            ret = ffmisc[ri->ffcref][a];
    }
    break;
    
    case FFINITDD:
    {
        int a = ri->d[0] / 10000;
        
        if(BC::checkBounds(a, 0, 7, "ffc->InitD") != SH::_NoError)
            ret = -10000;
        else
            ret = tmpscr->initd[ri->ffcref][a];
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//Link's Variables
    case LINKX:
        ret = long(Link.getX()) * 10000;
        break;

        
    case LINKY:
        ret = long(Link.getY()) * 10000;
        break;
        
    case LINKZ:
        ret = long(Link.getZ()) * 10000;
        break;
        
    case LINKJUMP:
        // -fall/100*10000, but doing it that way screwed up the result
        ret = long(-Link.getFall()) * 100;
        break;
        
    case LINKDIR:
        ret=(int)(Link.dir)*10000;
        break;
        
    case LINKHITDIR:
        ret=(int)(Link.getHitDir())*10000;
        break;
        
    case LINKHP:
        ret=(int)(game->get_life())*10000;
        break;
        
    case LINKMP:
        ret=(int)(game->get_magic())*10000;
        break;
        
    case LINKMAXHP:
        ret=(int)(game->get_maxlife())*10000;
        break;
        
    case LINKMAXMP:
        ret=(int)(game->get_maxmagic())*10000;
        break;
        
    case LINKACTION:
    {
	//Z_scripterrlog("The present FFCore action is: %d\n", FFCore.getLinkAction());
	//int act = FFCore.getLinkAction() * 10000;
	ret = FFCore.getLinkAction() * 10000;
	//Z_scripterrlog("The present 'action' return value is: %d\n", act);
	//ret = act;
        //ret=(int)(Link.getAction())*10000;
        break;
    }
        
    case LINKHELD:
        ret = (int)(Link.getHeldItem())*10000;
        break;
        
    case LINKITEMD:
        ret = game->item[vbound(ri->d[0]/10000, 0, MAXITEMS-1)] ? 10000 : 0;
        break;
        
    case LINKEQUIP:
        ret = ((Awpn&0xFF)|((Bwpn&0xFF)<<8))*10000;
        break;
        
    case LINKINVIS:
        ret = (((int)(Link.getDontDraw())) ? 10000 : 0);
        break;
        
    case LINKINVINC:
        ret = (int)(Link.scriptcoldet)*10000;
        break;
        
    case LINKLADDERX:
        ret=(int)(Link.getLadderX())*10000;
        break;
        
    case LINKLADDERY:
        ret=(int)(Link.getLadderY())*10000;
        break;
        
    case LINKSWORDJINX:
        ret = (int)(Link.getSwordClk())*10000;
        break;
        
    case LINKITEMJINX:
        ret = (int)(Link.getItemClk())*10000;
        break;
        
    case LINKDRUNK:
        ret = (int)(Link.DrunkClock())*10000;
        break;
        
    case LINKMISCD:
        ret = (int)(Link.miscellaneous[vbound(ri->d[0]/10000,0,31)]); //Was this buffed before? -Z
        break;
    
    
    case LINKHITBY:
    {
	    int indx = ri->d[0]/10000;
	    switch(indx)
	    {
		    //screen indices of objects
		case 0:
		case 1:
		case 2:
		case 3:
		{
			ret = (int)(Link.gethitLinkUID(indx))* 10000;
			break;
		}
		//uids of objects
		case 5:
		case 6:
		case 7:
		case 8:
		{
			ret = (int)(Link.gethitLinkUID(vbound(ri->d[0]/10000,0,3))); //do not multiply by 10000! UIDs are not *10000!
			break;
			
		}
		default: { al_trace("Invalid index passed to Link->HitBy[%d] /n", indx); ret = -1; break; }
	    }
        break;
    }
    case LINKDEFENCE:
        ret = (int)(Link.get_defence(vbound(ri->d[0]/10000,0,255)))* 10000;
        break;
        
        
    case LINKHXOFS:
        ret = (int)(Link.hxofs)*10000;
        break;
        
    case LINKHYOFS:
        ret = (int)(Link.hyofs)*10000;
        break;
        
    case LINKXOFS:
        ret = (int)(Link.xofs)*10000;
        break;
        
    case LINKYOFS:
        ret = (int)(Link.yofs-playing_field_offset)*10000;
        break;
        
    case LINKZOFS:
        ret = (int)(Link.zofs)*10000;
        break;
        
    case LINKHXSZ:
        ret = (int)(Link.hxsz)*10000;
        break;
        
    case LINKHYSZ:
        ret = (int)(Link.hysz)*10000;
        break;
        
    case LINKHZSZ:
        ret = (int)(Link.hzsz)*10000;
        break;
        
    case LINKTXSZ:
        ret = (int)(Link.txsz)*10000;
        break;
        
    case LINKTYSZ:
        ret = (int)(Link.tysz)*10000;
        break;
        
    case LINKTILE:
        ret = (int)(Link.tile)*10000;
        break;
        
    case LINKFLIP:
        ret = (int)(Link.flip)*10000;
        break;
    
    case LINKINVFRAME:
	ret = (int)Link.getHClk()*10000;
	break;
    
    case LINKCANFLICKER:
        ret= Link.getCanLinkFlicker()?10000:0;
        break;
    case LINKHURTSFX:
	ret = (int)Link.getHurtSFX()*10000;
	break;
    
    /*
    case LINKUSINGITEM:
	ret = (int)Link.getDirectItem()*10000;
        break;
    
    case LINKUSINGITEMA:
	ret = (int)Link.getDirectItemA()*10000;
        break;
    
    case LINKUSINGITEMB:
	ret = (int)Link.getDirectItemB()*10000;
        break;
    */
        
    case LINKEATEN:
	ret=(int)Link.getEaten()*10000;
	break;
    case LINKPUSH:
	ret=(int)Link.getPushing()*10000;
	break;
    case LINKSTUN:
	ret=(int)Link.StunClock()*10000;
	break;
    case LINKSCRIPTTILE:
	ret=script_link_sprite*10000;
	break;
    case LINKSCRIPFLIP:
	ret=script_link_flip*10000;
	break;
        
        
    case LINKITEMB:
	    //Link->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
	ret = Bwpn*10000;
	break;
    
    case LINKITEMA:
	    //Link->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
	ret = Awpn *10000;
	break;
    
    case LINKDIAG:
	ret=Link.getDiagMove()?10000:0;
	break;
    
    case LINKBIGHITBOX:
	ret=Link.getBigHitbox()?10000:0;
	break;
    
    
///----------------------------------------------------------------------------------------------------//
//Input States
    case INPUTSTART:
        ret=control_state[6]?10000:0;
        break;
        
    case INPUTMAP:
        ret=control_state[9]?10000:0;
        break;
        
    case INPUTUP:
        ret=control_state[0]?10000:0;
        break;
        
    case INPUTDOWN:
        ret=control_state[1]?10000:0;
        break;
        
    case INPUTLEFT:
        ret=control_state[2]?10000:0;
        break;
        
    case INPUTRIGHT:
        ret=control_state[3]?10000:0;
        break;
        
    case INPUTA:
        ret=control_state[4]?10000:0;
        break;
        
    case INPUTB:
        ret=control_state[5]?10000:0;
        break;
        
    case INPUTL:
        ret=control_state[7]?10000:0;
        break;
        
    case INPUTR:
        ret=control_state[8]?10000:0;
        break;
        
    case INPUTEX1:
        ret=control_state[10]?10000:0;
        break;
        
    case INPUTEX2:
        ret=control_state[11]?10000:0;
        break;
        
    case INPUTEX3:
        ret=control_state[12]?10000:0;
        break;
        
    case INPUTEX4:
        ret=control_state[13]?10000:0;
        break;
        
    case INPUTAXISUP:
        ret=control_state[14]?10000:0;
        break;
        
    case INPUTAXISDOWN:
        ret=control_state[15]?10000:0;
        break;
        
    case INPUTAXISLEFT:
        ret=control_state[16]?10000:0;
        break;
        
    case INPUTAXISRIGHT:
        ret=control_state[17]?10000:0;
        break;
        
   case INPUTMOUSEX:
    {
        int leftOffset=(resx/2)-(128*screen_scale);
        ret=((gui_mouse_x()-leftOffset)/screen_scale)*10000;
        break;
    }
    
    case INPUTMOUSEY:
    {
        int topOffset=(resy/2)-((112-playing_field_offset)*screen_scale);
        ret=((gui_mouse_y()-topOffset)/screen_scale)*10000;
        break;
    }
    
    case INPUTMOUSEZ:
        ret=(gui_mouse_z())*10000;
        break;
        
    case INPUTMOUSEB:
        ret=(gui_mouse_b())*10000;
        break;
    
    case INPUTPRESSSTART:
        ret=button_press[6]?10000:0;
        break;
        
    case INPUTPRESSMAP:
        ret=button_press[9]?10000:0;
        break;
        
    case INPUTPRESSUP:
        ret=button_press[0]?10000:0;
        break;
        
    case INPUTPRESSDOWN:
        ret=button_press[1]?10000:0;
        break;
        
    case INPUTPRESSLEFT:
        ret=button_press[2]?10000:0;
        break;
        
    case INPUTPRESSRIGHT:
        ret=button_press[3]?10000:0;
        break;
        
    case INPUTPRESSA:
        ret=button_press[4]?10000:0;
        break;
        
    case INPUTPRESSB:
        ret=button_press[5]?10000:0;
        break;
        
    case INPUTPRESSL:
        ret=button_press[7]?10000:0;
        break;
        
    case INPUTPRESSR:
        ret=button_press[8]?10000:0;
        break;
        
    case INPUTPRESSEX1:
        ret=button_press[10]?10000:0;
        break;
        
    case INPUTPRESSEX2:
        ret=button_press[11]?10000:0;
        break;
        
    case INPUTPRESSEX3:
        ret=button_press[12]?10000:0;
        break;
        
    case INPUTPRESSEX4:
        ret=button_press[13]?10000:0;
        break;
        
    case INPUTPRESSAXISUP:
        ret=button_press[14]?10000:0;
        break;
        
    case INPUTPRESSAXISDOWN:
        ret=button_press[15]?10000:0;
        break;
        
    case INPUTPRESSAXISLEFT:
        ret=button_press[16]?10000:0;
        break;
        
    case INPUTPRESSAXISRIGHT:
        ret=button_press[17]?10000:0;
        break;
        
    case BUTTONPRESS:
		// DUkey, DDkey, DLkey, DRkey, Akey, Bkey, Skey, Lkey, Rkey, Pkey, Exkey1, Exkey2, Exkey3, Exkey4 };
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		ret = button_press[button]?10000:0;
		
	}
	break;

	case BUTTONINPUT:
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		ret=control_state[button]?10000:0;
		
	}
	break;

	case BUTTONHELD:
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		ret = button_hold[button]?10000:0;
	}
	break;

	case KEYPRESS:
	{	//Game->KeyPressed[], read-only
		//if ( !keypressed() ) break; //Don;t return values set by setting Link->Input/Press
		//hmm...no, this won;t return properly for modifier keys. 
		int keyid = ri->d[0]/10000;
		//key = vbound(key,0,n);
		bool pressed = key[keyid] != 0;
		ret = pressed?10000:0;
	}
	break;

	case READKEY:
	{
		//Game->ReadKey(int key), also clears it. 
		int keyid = ri->d[0]/10000;
		bool pressed = ReadKey(keyid);
		ret = pressed?10000:0;
	}
	break;

	case JOYPADPRESS:
	{
		//Checks if a press is from the joypad, not keyboard. 
		int button = ri->d[0]/10000;
		ret = joybtn(button)?10000:0;
	}
	break;
	
	case MOUSEARR:
	{	
		int indx = (ri->d[0]/10000);
		int rv;
		switch (indx)
		{
			case 0: //MouseX
			{
				int leftOffset=(resx/2)-(128*screen_scale);
				rv=((gui_mouse_x()-leftOffset)/screen_scale)*10000;
				break;	
			}
			case 1: //MouseY
			{
				int topOffset=(resy/2)-((112-playing_field_offset)*screen_scale);
				rv=((gui_mouse_y()-topOffset)/screen_scale)*10000;
				break;
				
			}
			case 2: //MouseZ
			{
				rv=(gui_mouse_z())*10000;
				break;
			}
			case 3: //Left Click
			{
				rv=((gui_mouse_b()&0x1))*10000;
				break;
			}
			case 4: //Right Click
			{
				rv=((gui_mouse_b()&0x2))*10000;
				break;
			}
			case 5: //Middle Click
			{
				rv=((gui_mouse_b()&0x4))*10000;
				break;
			}
			default:
			{
				Z_scripterrlog("Invalid index passed to Input->Mouse[]: %d\n", indx);
				rv = -10000;
				break;
			}
		}
			
		//bool pressed = key[keyid] != 0;
		//ret = pressed?10000:0;
		ret = rv;
	}
	break;
        
///----------------------------------------------------------------------------------------------------//
//Item Variables
    case ITEMX:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)((item*)(s))->x)*10000;
        }
        
        break;
	
    case ITEMSCRIPTUID:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)((item*)(s))->script_UID); //Literal, not *10000
        }
        
        break;
        
    case ITEMY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)((item*)(s))->y)*10000;
        }
        
        break;
        
    case ITEMZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)((item*)(s))->z)*10000;
        }
        
        break;
        
    case ITEMJUMP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret = long((-((item*)(s))->fall / fix(100.0))  * 10000);
        }
        
        break;
        
    case ITEMDRAWTYPE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->drawstyle*10000;
        }
        
        break;
        
    case ITEMID:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->id*10000;
        }
        
        break;
        
    case ITEMTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->tile*10000;
        }
        
        break;
	
    case ITEMPSTRING:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->pstring*10000;
        }
        
        break;
    case ITEMPSTRINGFLAGS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->pickup_string_flags*10000;
        }
        
        break;
    case ITEMOVERRIDEFLAGS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->overrideFLAGS*10000;
        }
        
        break;
        
    case ITEMOTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->o_tile*10000;
        }
        
        break;
        
    case ITEMCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->o_cset&15)*10000;
        }
        
        break;
        
    case ITEMFLASHCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->o_cset>>4)*10000;
        }
        
        break;
        
    case ITEMFRAMES:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->frames*10000;
        }
        
        break;
        
    case ITEMFRAME:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->aframe*10000;
        }
        
        break;
	
    case ITEMACLK:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->aclk*10000;
        }
        
        break;    
	
    case ITEMASPEED:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->o_speed*10000;
        }
        
        break;
        
    case ITEMDELAY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->o_delay*10000;
        }
        
        break;
        
    case ITEMFLIP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->flip*10000;
        }
        
        break;
        
    case ITEMFLASH:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->flash*10000;
        }
        
        break;
        
    case ITEMHXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hxofs)*10000;
        }
        
        break;
        
    case ITEMHYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hyofs)*10000;
        }
        
        break;
        
    case ITEMXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)(((item*)(s))->xofs))*10000;
        }
        
        break;
        
    case ITEMYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)(((item*)(s))->yofs-playing_field_offset))*10000;
        }
        
        break;
        
    case ITEMZOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)(((item*)(s))->zofs))*10000;
        }
        
        break;
        
    case ITEMHXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hxsz)*10000;
        }
        
        break;
        
    case ITEMHYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hysz)*10000;
        }
        
        break;
        
    case ITEMHZSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hzsz)*10000;
        }
        
        break;
        
    case ITEMTXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->txsz)*10000;
        }
        
        break;
        
    case ITEMTYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->tysz)*10000;
        }
        
        break;
        
    case ITEMCOUNT:
        ret=(items.Count())*10000;
        break;
        
    case ITEMEXTEND:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->extend*10000;
        }
        
        break;
        
    case ITEMPICKUP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->pickup*10000;
        }
        
        break;
        
    case ITEMMISCD:
        if(0!=(s=checkItem(ri->itemref)))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            ret=(((item*)(s))->miscellaneous[a]);
            break;
        }
        
///----------------------------------------------------------------------------------------------------//
//Itemdata Variables
	
	
    case IDATAUSEWPN:
        ret=(itemsbuf[ri->idata].useweapon)*10000;
        break;
    case IDATAUSEDEF:
        ret=(itemsbuf[ri->idata].usedefence)*10000;
        break;
    case IDATAWRANGE:
        ret=(itemsbuf[ri->idata].weaprange)*10000;
        break;
    case IDATAMAGICTIMER:
        ret=(itemsbuf[ri->idata].magiccosttimer)*10000;
        break;
    case IDATAUSEMVT:
    {
	long a = vbound((ri->d[0] / 10000),0,(ITEM_MOVEMENT_PATTERNS-1));
	    
        ret=(itemsbuf[ri->idata].weap_pattern[a])*10000;
        
    }
    break;
    
    case IDATADURATION:
        ret=(itemsbuf[ri->idata].weapduration)*10000;
        break;
    
    case IDATADUPLICATES:
        ret=(itemsbuf[ri->idata].duplicates)*10000;
        break;
    case IDATADRAWLAYER:
        ret=(itemsbuf[ri->idata].drawlayer)*10000;
        break;
    case IDATACOLLECTFLAGS:
        ret=(itemsbuf[ri->idata].collectflags)*10000;
        break;
    case IDATAWEAPONSCRIPT:
        ret=(itemsbuf[ri->idata].weaponscript)*10000;
        break;
    case IDATAMISCD:
    {
	    
	int a = vbound((ri->d[0] / 10000),0,31);
        ret=(itemsbuf[ri->idata].wpn_misc_d[a])*10000;
    }
    break;
    case IDATAWPNINITD:
    {
	    
	int a = vbound((ri->d[0] / 10000),0,7);
        ret=(itemsbuf[ri->idata].weap_initiald[a])*10000;
    }
    break;
    case IDATAWEAPHXOFS:
        ret=(itemsbuf[ri->idata].weap_hxofs)*10000;
        break;
    case IDATAWEAPHYOFS:
        ret=(itemsbuf[ri->idata].weap_hyofs)*10000;
        break;
    case IDATAWEAPHXSZ:
        ret=(itemsbuf[ri->idata].weap_hxsz)*10000;
        break;
    case IDATAWEAPHYSZ:
        ret=(itemsbuf[ri->idata].weap_hysz)*10000;
        break;
    case IDATAWEAPHZSZ:
        ret=(itemsbuf[ri->idata].weap_hzsz)*10000;
        break;
    case IDATAWEAPXOFS:
        ret=(itemsbuf[ri->idata].weap_xofs)*10000;
        break;
    case IDATAWEAPYOFS:
        ret=(itemsbuf[ri->idata].weap_yofs)*10000;
        break;
     case IDATAHXOFS:
        ret=(itemsbuf[ri->idata].hxofs)*10000;
        break;
    case IDATAHYOFS:
        ret=(itemsbuf[ri->idata].hyofs)*10000;
        break;
    case IDATAHXSZ:
        ret=(itemsbuf[ri->idata].hxsz)*10000;
        break;
    case IDATAHYSZ:
        ret=(itemsbuf[ri->idata].hysz)*10000;
        break;
    case IDATAHZSZ:
        ret=(itemsbuf[ri->idata].hzsz)*10000;
        break;
    case IDATADXOFS:
        ret=(itemsbuf[ri->idata].xofs)*10000;
        break;
    case IDATADYOFS:
        ret=(itemsbuf[ri->idata].yofs)*10000;
        break;
    case IDATATILEW:
        ret=(itemsbuf[ri->idata].tilew)*10000;
        break;
    case IDATATILEH:
        ret=(itemsbuf[ri->idata].tileh)*10000;
        break;
    case IDATAPICKUP:
        ret=(itemsbuf[ri->idata].pickup)*10000;
        break;
    case IDATAOVERRIDEFL:
        ret=(itemsbuf[ri->idata].overrideFLAGS)*10000;
        break;

    case IDATATILEWWEAP:
        ret=(itemsbuf[ri->idata].weap_tilew)*10000;
        break;
    case IDATATILEHWEAP:
        ret=(itemsbuf[ri->idata].weap_tileh)*10000;
        break;
    case IDATAOVERRIDEFLWEAP:
        ret=(itemsbuf[ri->idata].weapoverrideFLAGS)*10000;
        break;

    
    
    case IDATAFAMILY:
        ret=(itemsbuf[ri->idata].family)*10000;
        break;
        
    case IDATALEVEL:
        ret=(itemsbuf[ri->idata].fam_type)*10000;
        break;
        
    case IDATAKEEP:
        ret=(itemsbuf[ri->idata].flags & ITEM_GAMEDATA)?10000:0;
        break;
        
    case IDATAAMOUNT:
        ret=(itemsbuf[ri->idata].amount)*10000;
        break;
        
    case IDATASETMAX:
        ret=(itemsbuf[ri->idata].setmax)*10000;
        break;
        
    case IDATAMAX:
        ret=(itemsbuf[ri->idata].max)*10000;
        break;
        
    case IDATACOUNTER:
        ret=(itemsbuf[ri->idata].count)*10000;
        break;
        
    case IDATAUSESOUND:
        ret=(itemsbuf[ri->idata].usesound)*10000;
        break;
        
    case IDATAPOWER:
        ret=(itemsbuf[ri->idata].power)*10000;
        break;
        
    //2.54
	//Get the ID of an item.
        case IDATAID:
        ret=ri->idata*10000;
        break;
    
	//Get the script assigned to an item (active)
    case IDATASCRIPT:
        ret=(itemsbuf[ri->idata].script)*10000;
        break;
    //Get the ->Attributes[] of an item
    case IDATAATTRIB:
    {
	    int index = vbound(ri->d[0]/10000,0,9);
		switch(index){
		    case 0:
			ret=(itemsbuf[ri->idata].misc1)*10000;
		    break;
		    case 1:
			ret=(itemsbuf[ri->idata].misc2)*10000; break;
		    case 2:
			ret=(itemsbuf[ri->idata].misc3)*10000; break;
		    case 3:
			ret=(itemsbuf[ri->idata].misc4)*10000; break;
		    case 4:
			ret=(itemsbuf[ri->idata].misc5)*10000; break;
		    case 5:
			ret=(itemsbuf[ri->idata].misc6)*10000; break;
		    case 6:
			ret=(itemsbuf[ri->idata].misc7)*10000; break;
		    case 7:
			ret=(itemsbuf[ri->idata].misc8)*10000; break;
		    case 8:
			ret=(itemsbuf[ri->idata].misc9)*10000; break;
		    case 9:
			ret=(itemsbuf[ri->idata].misc10)*10000; break;
		    default: 
			   ret = -10000; break;
		}
		   
        break;
	
	}
		//Get the ->Sprite[] of an item.
	case IDATASPRITE: {
	    int index = vbound(ri->d[0]/10000,0,9);
		switch(index){
		    case 0:
			ret=(itemsbuf[ri->idata].wpn)*10000;
		    break;
		    case 1:
			ret=(itemsbuf[ri->idata].wpn2)*10000; break;
		    case 2:
			ret=(itemsbuf[ri->idata].wpn3)*10000; break;
		    case 3:
			ret=(itemsbuf[ri->idata].wpn4)*10000; break;
		    case 4:
			ret=(itemsbuf[ri->idata].wpn5)*10000; break;
		    case 5:
			ret=(itemsbuf[ri->idata].wpn6)*10000; break;
		    case 6:
			ret=(itemsbuf[ri->idata].wpn7)*10000; break;
		    case 7:
			ret=(itemsbuf[ri->idata].wpn8)*10000; break;
		    case 8:
			ret=(itemsbuf[ri->idata].wpn9)*10000; break;
		    case 9:
			ret=(itemsbuf[ri->idata].wpn10)*10000; break;
		    default: 
			   ret = -10000; break;
		}
		   
		break;
	}
	//Link TIle modifier
    case IDATALTM:
        ret=(itemsbuf[ri->idata].ltm)*10000;
        break;
    //Pickup script
    case IDATAPSCRIPT:
        ret=(itemsbuf[ri->idata].collect_script)*10000;
        break;
    //Pickup string
    case IDATAPSTRING:
        ret=(itemsbuf[ri->idata].pstring)*10000;
        break;
    //Magic cost
     case IDATAMAGCOST:
        ret=(itemsbuf[ri->idata].magic)*10000;
        break;
     //cost counter ref
     case IDATACOSTCOUNTER:
        ret=(itemsbuf[ri->idata].cost_counter)*10000;
        break;
     //Min Hearts to Pick Up
     case IDATAMINHEARTS:
        ret=(itemsbuf[ri->idata].pickup_hearts)*10000;
        break;
     //Tile used by the item
     case IDATATILE:
        ret=(itemsbuf[ri->idata].tile)*10000;
        break;
     //itemdata->Flash
     case IDATAMISC:
        ret=(itemsbuf[ri->idata].misc)*10000;
        break;
     //->CSet
     case IDATACSET:
        ret=(itemsbuf[ri->idata].csets)*10000;
        break;
     //->A.Frames
     case IDATAFRAMES:
        ret=(itemsbuf[ri->idata].frames)*10000;
        break;
     /*
     case IDATAFRAME:
        ret=(itemsbuf[ri->idata].frame)*10000;
        break;
    */ 
     //->A.Speed
     case IDATAASPEED:
        ret=(itemsbuf[ri->idata].speed)*10000;
        break;
     //->Delay
     case IDATADELAY:
        ret=(itemsbuf[ri->idata].delay)*10000;
        break;
     // teo of this item upgrades
      case IDATACOMBINE:
        ret=(itemsbuf[ri->idata].flags & ITEM_COMBINE)?10000:0;
        break;
      //Use item, and get the lower level one
      case IDATADOWNGRADE:
        ret=(itemsbuf[ri->idata].flags & ITEM_DOWNGRADE)?10000:0;
        break;
      //->Flags[5]
      case IDATAFLAGS: {
	    int index = vbound(ri->d[0]/10000,0,4);
		switch(index){
		    case 0:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG1)?10000:0;
		    break;
		    case 1:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG2)?10000:0; break;
		    case 2:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG3)?10000:0; break;
		    case 3:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG4)?10000:0; break;
		    case 4:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG5)?10000:0; break;
		    case 5:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG6)?10000:0; break;
		   case 6:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG7)?10000:0; break;
		   case 7:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG8)?10000:0; break;
		   case 8:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG9)?10000:0; break;
		   case 9:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG10)?10000:0; break;
		   case 10:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG11)?10000:0; break;
		   case 11:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG12)?10000:0; break;
		   case 12:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG13)?10000:0; break;
		   case 13:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG14)?10000:0; break;
		   case 14:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG15)?10000:0; break;
		   case 15:
			ret=(itemsbuf[ri->idata].flags & ITEM_FLAG16)?10000:0; break;
		   
		   
		    default: 
			   ret = 0; break;
		}
		   
		break;
	}
		
	//->Keep Old
      case IDATAKEEPOLD:
        ret=(itemsbuf[ri->idata].flags & ITEM_KEEPOLD)?10000:0;
        break;
      //Use rupees instead of magic
      case IDATARUPEECOST:
        ret=(itemsbuf[ri->idata].flags & ITEM_RUPEE_MAGIC)?10000:0;
        break;
      //Can be eaten
      case IDATAEDIBLE:
        ret=(itemsbuf[ri->idata].flags & ITEM_EDIBLE)?10000:0;
        break;
      //Not int he editor, could become flags[6], but I'm reserving this one for other item uses. 
      case IDATAFLAGUNUSED:
        ret=(itemsbuf[ri->idata].flags & ITEM_UNUSED)?10000:0;
        break;
      //Gain lower level items when collected
      case IDATAGAINLOWER:
        ret=(itemsbuf[ri->idata].flags & ITEM_GAINOLD)?10000:0;
        break;
	//Unchanged from master
    case IDATAINITDD:
    {
        int a = ri->d[0] / 10000;
        
        if(BC::checkBounds(a, 0, 7, "itemdata->InitD") != SH::_NoError)
            ret = -10000;
        else
            ret = itemsbuf[ri->idata].initiald[a];
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//NPC Variables

//Reduces accessing integer members to one line
#define GET_NPC_VAR_INT(member, str) \
{ \
	if(GuyH::loadNPC(ri->guyref, str) != SH::_NoError) \
		ret = -10000; \
	else \
		ret = GuyH::getNPC()->member * 10000; \
}

    case NPCDIR:
        if(GuyH::loadNPC(ri->guyref, "npc->Dir") != SH::_NoError)
            ret = -10000;
        else
            ret = zc_max(GuyH::getNPC()->dir * 10000, 0);
            
        break;
        
    case NPCRATE:
        GET_NPC_VAR_INT(rate, "npc->Rate") break;
        
    case NPCHOMING:
        GET_NPC_VAR_INT(homing, "npc->Homing") break;
        
    case NPCFRAMERATE:
        GET_NPC_VAR_INT(frate, "npc->ASpeed") break;
        
    case NPCHALTRATE:
        GET_NPC_VAR_INT(hrate, "npc->HaltRate") break;
        
    case NPCDRAWTYPE:
        GET_NPC_VAR_INT(drawstyle, "npc->DrawStyle") break;
        
    case NPCHP:
        GET_NPC_VAR_INT(hp, "npc->HP") break;
        
    case NPCCOLLDET:
        GET_NPC_VAR_INT(scriptcoldet, "npc->ColDetection") break;
        
    case NPCSTUN:
        GET_NPC_VAR_INT(stunclk, "npc->Stun") break;
        
    case NPCHUNGER:
        GET_NPC_VAR_INT(grumble, "npc->Hunger") break;
    
    case NPCWEAPSPRITE:
        GET_NPC_VAR_INT(wpnsprite, "npc->WeaponSprite") break;
        
    case NPCTYPE:
        GET_NPC_VAR_INT(family, "npc->Type") break;
        
    case NPCDP:
        GET_NPC_VAR_INT(dp, "npc->Damage") break;
        
    case NPCWDP:
        GET_NPC_VAR_INT(wdp, "npc->WeaponDamage") break;
        
    case NPCOTILE:
        GET_NPC_VAR_INT(o_tile, "npc->OriginalTile") break;
        
    case NPCTILE:
        GET_NPC_VAR_INT(tile, "npc->Tile") break;
        
    case NPCWEAPON:
        GET_NPC_VAR_INT(wpn, "npc->Weapon") break;
        
    case NPCITEMSET:
        GET_NPC_VAR_INT(item_set, "npc->ItemSet") break;
        
    case NPCCSET:
        GET_NPC_VAR_INT(cs, "npc->CSet") break;
        
    case NPCBOSSPAL:
        GET_NPC_VAR_INT(bosspal, "npc->BossPal") break;
        
    case NPCBGSFX:
        GET_NPC_VAR_INT(bgsfx, "npc->SFX") break;
        
    case NPCEXTEND:
        GET_NPC_VAR_INT(extend, "npc->Extend") break;
        
    case NPCHXOFS:
        GET_NPC_VAR_INT(hxofs, "npc->HitXOffset") break;
        
    case NPCHYOFS:
        GET_NPC_VAR_INT(hyofs, "npc->HitYOffset") break;
        
    case NPCHXSZ:
        GET_NPC_VAR_INT(hxsz, "npc->HitWidth") break;
        
    case NPCHYSZ:
        GET_NPC_VAR_INT(hysz, "npc->HitHeight") break;
        
    case NPCHZSZ:
        GET_NPC_VAR_INT(hzsz, "npc->HitZHeight") break;
        
    case NPCTXSZ:
        GET_NPC_VAR_INT(txsz, "npc->TileWidth") break;
        
    case NPCTYSZ:
        GET_NPC_VAR_INT(tysz, "npc->TileHeight") break;
        
//And fix
#define GET_NPC_VAR_FIX(member, str) \
{ \
	if(GuyH::loadNPC(ri->guyref, str) != SH::_NoError) \
		ret = -10000; \
	else \
		ret = (long(GuyH::getNPC()->member) * 10000); \
}

    case NPCX:
        GET_NPC_VAR_FIX(x, "npc->X") break;
        
    case NPCY:
        GET_NPC_VAR_FIX(y, "npc->Y") break;
        
    case NPCZ:
        GET_NPC_VAR_FIX(z, "npc->Z") break;
        
    case NPCXOFS:
        GET_NPC_VAR_FIX(xofs, "npc->DrawXOffset") break;
        
    case NPCYOFS:
        GET_NPC_VAR_FIX(yofs, "npc->DrawYOffset") ret-=playing_field_offset*10000;
        break;
        
    case NPCZOFS:
        GET_NPC_VAR_FIX(zofs, "npc->DrawZOffset") break;
        
        //These variables are all different to the templates (casting for jump and step is slightly non-standard)
    case NPCJUMP:
        if(GuyH::loadNPC(ri->guyref, "npc->Jump") != SH::_NoError)
            ret = -10000;
        else
            ret = (long(-GuyH::getNPC()->fall / fix(100.0)) * 10000);
            
        break;
        
    case NPCSTEP:
        if(GuyH::loadNPC(ri->guyref, "npc->Step") != SH::_NoError)
            ret = -10000;
        else
            ret = long(GuyH::getNPC()->step * fix(100.0)) * 10000;
            
        break;
	
        
    case NPCID:
        if(GuyH::loadNPC(ri->guyref, "npc->ID") != SH::_NoError)
            ret = -10000;
        else
            ret = (GuyH::getNPC()->id & 0xFFF) * 10000;
            
        break;
	
    case NPCISCORE:
        if(GuyH::loadNPC(ri->guyref, "npc->isCore") != SH::_NoError)
            ret = -10000;
        else
            ret = ((GuyH::getNPC()->isCore) ? 10000 : 0);
            
        break;
	
    case NPCSCRIPTUID:
        if(GuyH::loadNPC(ri->guyref, "npc->ScriptUID") != SH::_NoError)
            ret = -10000;
        else
            ret = ((GuyH::getNPC()->getScriptUID())); //literal, not *10000
            
        break;
        
    case NPCMFLAGS:
        if(GuyH::loadNPC(ri->guyref, "npc->MiscFlags") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getMFlags() * 10000;
            
        break;
        
        //Indexed (two checks)
    case NPCDEFENSED:
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Defense[]") != SH::_NoError ||
                BC::checkBounds(a, 0, (edefLAST255), "npc->Defense[]") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPC()->defense[a] * 10000;
    }
    break;
    
    case NPCHITBY:
    {
        int indx = ri->d[0] / 10000;

        if(GuyH::loadNPC(ri->guyref, "npc->HitBy[]") != SH::_NoError )
	{
            ret = -10000; break;
	}
        else
	{
		switch(indx)
		{
			//screen indixes
			case 0:
			case 1:
			case 2:
			case 3:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			{
				ret = GuyH::getNPC()->hitby[indx] * 10000; // * 10000; //do not multiply by 10000! UIDs are not *10000!
				break;
			}
			//UIDs
			case 4:
			case 5:
			case 6:
			case 7:
			{
				ret = GuyH::getNPC()->hitby[indx]; // * 10000; //do not multiply by 10000! UIDs are not *10000!
				break;
			}
			default: { al_trace("Invalid index used for npc->HitBy[%d]. /n", indx); ret = -10000; break; }
		}
		break;
	}
    }
    
    //2.fuure compat.
    
    case NPCSCRDEFENSED:
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->ScriptDefense") != SH::_NoError ||
                BC::checkBounds(a, 0, edefSCRIPTDEFS_MAX, "npc->ScriptDefense") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPC()->defense[a+edefSCRIPT01] * 10000;
    }
    break;
    
    
    case NPCMISCD:
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Misc") != SH::_NoError ||
                BC::checkMisc32(a, "npc->Misc") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPC()->miscellaneous[a];
    }
    break;
    
    case NPCDD: //Fized the size of this array. There are 15 total attribs, [0] to [14], not [0] to [9]. -Z
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Attributes") != SH::_NoError ||
                BC::checkBounds(a, 0, 15, "npc->Attributes") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPCDMisc(a) * 10000;
    }
    break;
    
        case NPCINVINC:
        if(GuyH::loadNPC(ri->guyref, "npc->InvFrames") != SH::_NoError)
            ret = -10000;
        else
	ret = (int)GuyH::getNPC()->hclk * 10000;
            
        break;
	
    case NPCHASITEM:
        if(GuyH::loadNPC(ri->guyref, "npc->HasItem") != SH::_NoError)
            ret = 0;
        else
	ret = GuyH::getNPC()->itemguy?10000:0;
            
        break;
	
    case NPCRINGLEAD:
        if(GuyH::loadNPC(ri->guyref, "npc->Ringleader") != SH::_NoError)
            ret = 0;
        else
	ret = GuyH::getNPC()->leader?10000:0;
            
        break;
	
	case NPCSUPERMAN:
        if(GuyH::loadNPC(ri->guyref, "npc->Invincible") != SH::_NoError)
            ret = -10000;
        else
	ret = (int)GuyH::getNPC()->superman * 10000;
            
        break;
    
    case NPCSHIELD:
	{
		int indx = ri->d[0];
		if(GuyH::loadNPC(ri->guyref, "npc->Shield[]") == SH::_NoError)
		{
			switch(indx)
			{
				case 0:
				{
					ret = ((GuyH::getNPC()->flags&inv_front) ? 10000 : 0);
					break;
				}
				case 1:
				{
					ret = ((GuyH::getNPC()->flags&inv_left) ? 10000 : 0);
					break;
				}
				case 2:
				{
					ret = ((GuyH::getNPC()->flags&inv_right) ? 10000 : 0);
					break;
				}
				case 3:
				{
					ret = ((GuyH::getNPC()->flags&inv_back) ? 10000 : 0);
					break;
				}
				case 4: //shield can be broken
				{
					ret = ((GuyH::getNPC()->flags&guy_bkshield) ? 10000 : 0);
					break;
				}
				default:
				{
					Z_scripterrlog("Invalid Array Index passed to npc->Shield[]: %d\n", indx); 
					break;
				}
			}
		}
	}
	
	case NPCFROZENTILE:
        GET_NPC_VAR_INT(frozentile, "npc->FrozenTile") break;
	
	case NPCFROZENCSET:
        GET_NPC_VAR_INT(frozencset, "npc->FrozenCSet") break;
	
	case NPCFROZEN:
        GET_NPC_VAR_INT(frozenclock, "npc->Frozen") break;
    
///----------------------------------------------------------------------------------------------------//
//LWeapon Variables
    case LWPNX:
        if(0!=(s=checkLWpn(ri->lwpn,"X")))
            ret=((int)((weapon*)(s))->x)*10000;
            
        break;
        
    case LWPNY:
        if(0!=(s=checkLWpn(ri->lwpn,"Y")))
            ret=((int)((weapon*)(s))->y)*10000;
            
        break;
        
    case LWPNZ:
        if(0!=(s=checkLWpn(ri->lwpn,"Z")))
            ret=((int)((weapon*)(s))->z)*10000;
            
        break;
        
    case LWPNJUMP:
        if(0!=(s=checkLWpn(ri->lwpn,"Jump")))
            ret = long((-((weapon*)(s))->fall / fix(100.0)) * 100000);
            
        break;
        
    case LWPNDIR:
        if(0!=(s=checkLWpn(ri->lwpn,"Dir")))
            ret=((weapon*)(s))->dir*10000;
            
        break;
        
    case LWPNSTEP:
        if(0!=(s=checkLWpn(ri->lwpn,"Step")))
            ret=(int)((float)((weapon*)s)->step * 1000000.0);
            
        break;
        
    case LWPNANGLE:
        if(0!=(s=checkLWpn(ri->lwpn,"Angle")))
            ret=(int)(((weapon*)(s))->angle*10000);
            
        break;
        
    case LWPNANGULAR:
        if(0!=(s=checkLWpn(ri->lwpn,"Angular")))
            ret=((weapon*)(s))->angular*10000;
            
        break;
        
    case LWPNBEHIND:
        if(0!=(s=checkLWpn(ri->lwpn,"Behind")))
            ret=((weapon*)(s))->behind*10000;
            
        break;
        
    case LWPNDRAWTYPE:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawStyle")))
            ret=((weapon*)(s))->drawstyle*10000;
            
        break;
        
    case LWPNPOWER:
        if(0!=(s=checkLWpn(ri->lwpn,"Damage")))
            ret=((weapon*)(s))->power*10000;
            
        break;
/*
    case LWPNRANGE:
        if(0!=(s=checkLWpn(ri->lwpn,"Range")))
            ret=((weapon*)(s))->scriptrange*10000;
            
        break;
*/        
    case LWPNDEAD:
        if(0!=(s=checkLWpn(ri->lwpn,"DeadState")))
            ret=((weapon*)(s))->dead*10000;
            
        break;
        
    case LWPNID:
        if(0!=(s=checkLWpn(ri->lwpn,"ID")))
            ret=((weapon*)(s))->id*10000;
            
        break;
        
    case LWPNTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"Tile")))
            ret=((weapon*)(s))->tile*10000;
            
        break;
        
    case LWPNCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"CSet")))
            ret=((weapon*)(s))->cs*10000;
            
        break;
        
    case LWPNFLASHCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"FlashCSet")))
            ret=(((weapon*)(s))->o_cset>>4)*10000;
            
        break;
        
    case LWPNFRAMES:
        if(0!=(s=checkLWpn(ri->lwpn,"NumFrames")))
            ret=((weapon*)(s))->frames*10000;
            
        break;
        
    case LWPNFRAME:
        if(0!=(s=checkLWpn(ri->lwpn,"Frame")))
            ret=((weapon*)(s))->aframe*10000;
            
        break;
        
    case LWPNASPEED:
        if(0!=(s=checkLWpn(ri->lwpn,"ASpeed")))
            ret=((weapon*)(s))->o_speed*10000;
            
        break;
        
    case LWPNFLASH:
        if(0!=(s=checkLWpn(ri->lwpn,"Flash")))
            ret=((weapon*)(s))->flash*10000;
            
        break;
        
    case LWPNFLIP:
        if(0!=(s=checkLWpn(ri->lwpn,"Flip")))
            ret=((weapon*)(s))->flip*10000;
            
        break;
        
    case LWPNCOUNT:
        ret=Lwpns.Count()*10000;
        break;
        
    case LWPNEXTEND:
        if(0!=(s=checkLWpn(ri->lwpn,"Extend")))
            ret=((weapon*)(s))->extend*10000;
            
        break;
        
    case LWPNOTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalTile")))
            ret=((weapon*)(s))->o_tile*10000;
            
        break;
        
    case LWPNOCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalCSet")))
            ret=(((weapon*)(s))->o_cset&15)*10000;
            
        break;
        
    case LWPNHXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitXOffset")))
            ret=(((weapon*)(s))->hxofs)*10000;
            
        break;
        
    case LWPNHYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitYOffset")))
            ret=(((weapon*)(s))->hyofs)*10000;
            
        break;
        
    case LWPNXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawXOffset")))
            ret=((int)(((weapon*)(s))->xofs))*10000;
            
        break;
        
    case LWPNYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawYOffset")))
            ret=((int)(((weapon*)(s))->yofs-playing_field_offset))*10000;
            
        break;
        
    case LWPNZOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawZOffset")))
            ret=((int)(((weapon*)(s))->zofs))*10000;
            
        break;
        
    case LWPNHXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitWidth")))
            ret=(((weapon*)(s))->hxsz)*10000;
            
        break;
        
    case LWPNHYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitHeight")))
            ret=(((weapon*)(s))->hysz)*10000;
            
        break;
        
    case LWPNHZSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitZHeight")))
            ret=(((weapon*)(s))->hzsz)*10000;
            
        break;
        
    case LWPNTXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileWidth")))
            ret=(((weapon*)(s))->txsz)*10000;
            
        break;
        
    case LWPNTYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileHeight")))
            ret=(((weapon*)(s))->tysz)*10000;
            
        break;
        
    case LWPNMISCD:
        if(0!=(s=checkLWpn(ri->lwpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            ret=(((weapon*)(s))->miscellaneous[a]);
        }
        
        break;
        
    case LWPNCOLLDET:
        if(0!=(s=checkLWpn(ri->lwpn,"CollDetection")))
            ret=(((weapon*)(s))->scriptcoldet)*10000;
            
        break;
	
    case LWPNPARENT:
        if(0!=(s=checkLWpn(ri->lwpn,"Parent")))
            ret=(((weapon*)(s))->parentitem)*10000;
            
        break;

    case LWPNLEVEL:
        if(0!=(s=checkLWpn(ri->lwpn,"Level")))
            ret=(((weapon*)(s))->type)*10000;
            
        break;
	
    case LWEAPONSCRIPTUID:
	if(0!=(s=checkLWpn(ri->lwpn,"ScriptUID")))
            ret=(((weapon*)(s))->script_UID); //literal, not *10000
            
        break;
	
    
        
///----------------------------------------------------------------------------------------------------//
//EWeapon Variables
    case EWPNX:
        if(0!=(s=checkEWpn(ri->ewpn, "X")))
            ret=((int)((weapon*)(s))->x)*10000;
            
        break;
        
    case EWPNY:
        if(0!=(s=checkEWpn(ri->ewpn, "Y")))
            ret=((int)((weapon*)(s))->y)*10000;
            
        break;
        
    case EWPNZ:
        if(0!=(s=checkEWpn(ri->ewpn, "Z")))
            ret=((int)((weapon*)(s))->z)*10000;
            
        break;
        
    case EWPNJUMP:
        if(0!=(s=checkEWpn(ri->ewpn, "Jump")))
            ret = long((-((weapon*)(s))->fall / fix(100.0)) * 100000);
            
        break;
        
    case EWPNDIR:
        if(0!=(s=checkEWpn(ri->ewpn, "Dir")))
            ret=((weapon*)(s))->dir*10000;
            
        break;
        
    case EWPNSTEP:
        if(0!=(s=checkEWpn(ri->ewpn, "Step")))
            ret=(int)((float)((weapon*)s)->step * 1000000.0);
            
        break;
        
    case EWPNANGLE:
        if(0!=(s=checkEWpn(ri->ewpn,"Angle")))
            ret=(int)(((weapon*)(s))->angle*10000);
            
        break;
        
    case EWPNANGULAR:
        if(0!=(s=checkEWpn(ri->ewpn,"Angular")))
            ret=((weapon*)(s))->angular*10000;
            
        break;
        
    case EWPNBEHIND:
        if(0!=(s=checkEWpn(ri->ewpn,"Behind")))
            ret=((weapon*)(s))->behind*10000;
            
        break;
        
    case EWPNDRAWTYPE:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawStyle")))
            ret=((weapon*)(s))->drawstyle*10000;
            
        break;
        
    case EWPNPOWER:
        if(0!=(s=checkEWpn(ri->ewpn,"Damage")))
            ret=((weapon*)(s))->power*10000;
            
        break;
        
    case EWPNDEAD:
        if(0!=(s=checkEWpn(ri->ewpn,"DeadState")))
            ret=((weapon*)(s))->dead*10000;
            
        break;
        
    case EWPNID:
        if(0!=(s=checkEWpn(ri->ewpn,"ID")))
            ret=((weapon*)(s))->id*10000;
            
        break;
        
    case EWPNTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"Tile")))
            ret=((weapon*)(s))->tile*10000;
            
        break;
        
    case EWPNCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"CSet")))
            ret=((weapon*)(s))->cs*10000;
            
        break;
        
    case EWPNFLASHCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"FlashCSet")))
            ret=(((weapon*)(s))->o_cset>>4)*10000;
            
        break;
        
    case EWPNFRAMES:
        if(0!=(s=checkEWpn(ri->ewpn,"NumFrames")))
            ret=((weapon*)(s))->frames*10000;
            
        break;
        
    case EWPNFRAME:
        if(0!=(s=checkEWpn(ri->ewpn,"Frame")))
            ret=((weapon*)(s))->aframe*10000;
            
        break;
        
    case EWPNASPEED:
        if(0!=(s=checkEWpn(ri->ewpn,"ASpeed")))
            ret=((weapon*)(s))->o_speed*10000;
            
        break;
        
    case EWPNFLASH:
        if(0!=(s=checkEWpn(ri->ewpn,"Flash")))
            ret=((weapon*)(s))->flash*10000;
            
        break;
        
    case EWPNFLIP:
        if(0!=(s=checkEWpn(ri->ewpn,"Flip")))
            ret=((weapon*)(s))->flip*10000;
            
        break;
        
    case EWPNCOUNT:
        ret=Ewpns.Count()*10000;
        break;
        
    case EWPNEXTEND:
        if(0!=(s=checkEWpn(ri->ewpn,"Extend")))
            ret=((weapon*)(s))->extend*10000;
            
        break;
        
    case EWPNOTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalTile")))
            ret=((weapon*)(s))->o_tile*10000;
            
        break;
        
    case EWPNOCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalCSet")))
            ret=(((weapon*)(s))->o_cset&15)*10000;
            
        break;
        
    case EWPNHXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitXOffset")))
            ret=(((weapon*)(s))->hxofs)*10000;
            
        break;
        
    case EWPNHYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitYOffset")))
            ret=(((weapon*)(s))->hyofs)*10000;
            
        break;
        
    case EWPNXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawXOffset")))
            ret=((int)(((weapon*)(s))->xofs))*10000;
            
        break;
        
    case EWPNYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawYOffset")))
            ret=((int)(((weapon*)(s))->yofs-playing_field_offset))*10000;
            
        break;
        
    case EWPNZOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawZOffset")))
            ret=((int)(((weapon*)(s))->zofs))*10000;
            
        break;
        
    case EWPNHXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitWidth")))
            ret=(((weapon*)(s))->hxsz)*10000;
            
        break;
        
    case EWPNHYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitHeight")))
            ret=(((weapon*)(s))->hysz)*10000;
            
        break;
        
    case EWPNHZSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitZHeight")))
            ret=(((weapon*)(s))->hzsz)*10000;
            
        break;
        
    case EWPNTXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileWidth")))
            ret=(((weapon*)(s))->txsz)*10000;
            
        break;
        
    case EWPNTYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileHeight")))
            ret=(((weapon*)(s))->tysz)*10000;
            
        break;
        
    case EWPNMISCD:
        if(0!=(s=checkEWpn(ri->ewpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            ret=(((weapon*)(s))->miscellaneous[a]);
        }
        
        break;
        
    case EWPNCOLLDET:
        if(0!=(s=checkEWpn(ri->ewpn,"CollDetection")))
            ret=(((weapon*)(s))->scriptcoldet)*10000;
            
        break;
	
    case EWPNPARENT:
        if(0!=(s=checkEWpn(ri->ewpn, "Parent")))
            ret=(((weapon*)(s))->parentid)*10000;
            
        break;
	
    case EWEAPONSCRIPTUID:
        if(0!=(s=checkEWpn(ri->ewpn, "ScriptUID")))
            ret=(((weapon*)(s))->script_UID); //literal, not *10000
            
        break;
	
	/*
	case LWEAPONSCRIPTUID:
	if(0!=(s=checkLWpn(ri->lwpn,"ScriptUID")))
            ret=(((weapon*)(s))->getScriptUID()); //literal, not *10000
            
        break;
    case EWEAPONSCRIPTUID:
	if(0!=(s=checkLWpn(ri->ewpn,"ScriptUID")))
            ret=(((weapon*)(s))->getScriptUID()); //literal, not *10000
            
        break;
	*/
        
///----------------------------------------------------------------------------------------------------//
//Game Info
	
	
    case GAMESUBSCHEIGHT:
    {
	ret = passive_subscreen_height*10000;    
    }
    break;
    
    case GAMEPLAYFIELDOFS:
	ret = playing_field_offset*10000;
    break;
    
    case PASSSUBOFS:
	ret = passive_subscreen_offset * 10000;
    break;
    
    case GETPIXEL:
        ret=FFCore.do_getpixel();
        break;

    
    case ZELDAVERSION:
        ret = ZC_VERSION; //Do *not* multiply by 10,000!
        break;
    case ZELDABUILD:
	ret = (int)VERSION_BUILD*10000;
	break;
    case ZELDABETA:
	ret = (int)VERSION_BETA*10000;
	break;
    case GAMEDEATHS:
        ret=game->get_deaths()*10000;
        break;
        
    case GAMECHEAT:
        ret=game->get_cheat()*10000;
        break;
        
    case GAMETIME:
        ret=game->get_time();
        break;// Can't multiply by 10000 or the maximum result is too big
        
    case GAMETIMEVALID:
        ret=game->get_timevalid()?10000:0;
        break;
        
    case GAMEHASPLAYED:
        ret=game->get_hasplayed()?10000:0;
        break;
    
    case TYPINGMODE:
        ret=FFCore.kb_typing_mode?10000:0;
        break;
        
    case GAMESTANDALONE:
        ret=standalone_mode?10000:0;
        break;
        
    case GAMEGUYCOUNT:
    {
        int mi = (currmap*MAPSCRSNORMAL)+(ri->d[0]/10000);
        ret=game->guys[mi]*10000;
    }
    break;
    
    case GAMECONTSCR:
        ret=game->get_continue_scrn()*10000;
        break;
        
    case GAMECONTDMAP:
        ret=game->get_continue_dmap()*10000;
        break;
        
    case GAMEENTRSCR:
        ret=lastentrance*10000;
        break;
        
    case GAMEENTRDMAP:
        ret=lastentrance_dmap*10000;
        break;
        
    case GAMECOUNTERD:
        ret=game->get_counter((ri->d[0])/10000)*10000;
        break;
        
    case GAMEMCOUNTERD:
        ret=game->get_maxcounter((ri->d[0])/10000)*10000;
        break;
        
    case GAMEDCOUNTERD:
        ret=game->get_dcounter((ri->d[0])/10000)*10000;
        break;
        
    case GAMEGENERICD:
        ret=game->get_generic((ri->d[0])/10000)*10000;
        break;
    
    case GAMEMISC:
    {
	int indx = ri->d[0]/10000;
	if ( indx < 0 || indx > 31 )
	{
		ret = -10000;
		Z_scripterrlog("Invalud index used to access Game->Misc: %d\n", indx);
	}
	else
	{
		ret = QMisc.questmisc[indx];
	}
	break;
    }
        
    case GAMEITEMSD:
        ret=(game->item[(ri->d[0])/10000] ? 10000 : 0);
        break;
    case DISABLEDITEM:
	ret = (game->items_off[(ri->d[0])/10000] ? 10000 : 0);
	break;
        
    case GAMELITEMSD:
        ret=game->lvlitems[(ri->d[0])/10000]*10000;
        break;
        
    case GAMELKEYSD:
        ret=game->lvlkeys[(ri->d[0])/10000]*10000;
        break;
        
    case SCREENSTATED:
    {
        int mi =(currmap*MAPSCRSNORMAL)+currscr;
        ret=((game->maps[mi]>>((ri->d[0]/10000)))&1)?10000:0;
    }
    break;
    
    case SCREENSTATEDD:
    {
        // Gah! >:(  Screen state is stored in game->maps, which uses 128 screens per map,
        // but the compiler multiplies the map number by 136, so it has to be corrected here.
        // Yeah, the compiler could be fixed, but that wouldn't cover existing quests...
        int mi = ri->d[0] / 10000;
        mi -= 8*((ri->d[0] / 10000) / MAPSCRS);
        
        if(BC::checkMapID(mi>>7, "Game->GetScreenState") == SH::_NoError)
            ret=(game->maps[mi] >> (ri->d[1] / 10000) & 1) ? 10000 : 0;
        else
            ret=0;
            
        break;
    }
    
    case GAMEGUYCOUNTD:
        ret=game->guys[(currmap * MAPSCRSNORMAL) + (ri->d[0] / 10000)]*10000;
        break;
        
    case CURMAP:
        ret=(1+currmap)*10000;
        break;
        
    case CURSCR:
        ret=currscr*10000;
        break;
        
    case GETMIDI:
        ret=(currmidi-(ZC_MIDI_COUNT-1))*10000;
        break;
        
    case CURDSCR:
    {
        int di = (get_currscr()-DMaps[get_currdmap()].xoff);
        ret=(DMaps[get_currdmap()].type==dmOVERW ? currscr : di)*10000;
    }
    break;
    
    case GAMEMAXMAPS:
	ret = (map_count-1)*10000;
	break;
    case GAMENUMMESSAGES:
	ret = (msg_count-1) * 10000; 
	break;
    
    case CURDMAP:
        ret=currdmap*10000;
        break;
        
    case CURLEVEL:
        ret=DMaps[get_currdmap()].level*10000;
        break;
        
    case GAMECLICKFREEZE:
        ret=disableClickToFreeze?0:10000;
        break;
        
    
    case NOACTIVESUBSC:
	ret=Link.stopSubscreenFalling()?10000:0;
	break;
        
///----------------------------------------------------------------------------------------------------//
//DMap Information

#define GET_DMAP_VAR(member, str) \
{ \
    int ID = ri->d[0] / 10000; \
    if(BC::checkDMapID(ID, str) != SH::_NoError) \
        ret = -10000; \
    else \
        ret = DMaps[ID].member * 10000; \
}

    case DMAPFLAGSD:
        GET_DMAP_VAR(flags,   "Game->DMapFlags")    break;
        
    case DMAPLEVELD:
        GET_DMAP_VAR(level,   "Game->DMapLevel")    break;
        
    case DMAPCOMPASSD:
        GET_DMAP_VAR(compass, "Game->DMapCompass")  break;
        
    case DMAPCONTINUED:
        GET_DMAP_VAR(cont,    "Game->DMapContinue") break;
    
    case DMAPLEVELPAL:
	GET_DMAP_VAR(color,   "Game->DMapPalette")    break; 
        
    case DMAPOFFSET:
        GET_DMAP_VAR(xoff,    "Game->DMapOffset")   break;
        
    case DMAPMAP:
    {
        int ID = ri->d[0] / 10000;
        
        if(BC::checkDMapID(ID, "Game->DMapMap") != SH::_NoError)
            ret = -10000;
        else
            ret = (DMaps[ID].map+1) * 10000;
            
        break;
    }
    
    case DMAPMIDID:
    {
        int ID = ri->d[0] / 10000;
        
        if(BC::checkDMapID(ID, "Game->DMapMIDI") == SH::_NoError)
        {
            // Based on play_DmapMusic
            switch(DMaps[ID].midi)
            {
            case 2:
                ret = -60000;
                break; // Dungeon
                
            case 3:
                ret = -30000;
                break; // Level 9
                
            case 1:
                ret = -20000;
                break; // Overworld
                
            case 0:
                ret = 0;
                break; // None
                
            default:
                ret = (DMaps[ID].midi - 3) * 10000;
            }
        }
        else
            ret = -10000; // Which is valid, but whatever.
            
        break;
    }
    
///----------------------------------------------------------------------------------------------------//
//Screen->ComboX
#define GET_COMBO_VAR(member, str) \
{ \
int pos = ri->d[0] / 10000; \
if(BC::checkComboPos(pos, str) != SH::_NoError) \
    ret = -10000; \
else \
    ret = tmpscr->member[pos]*10000; \
}

    case COMBODD:
        GET_COMBO_VAR(data,  "Screen->ComboD") break;
        
    case COMBOCD:
        GET_COMBO_VAR(cset,  "Screen->ComboC") break;
        
    case COMBOFD:
        GET_COMBO_VAR(sflag, "Screen->ComboF") break;
        
#define GET_COMBO_VAR_BUF(member, str) \
{ \
    int pos = ri->d[0] / 10000; \
    if(BC::checkComboPos(pos, str) != SH::_NoError) \
        ret = -10000; \
    else \
        ret = combobuf[tmpscr->data[pos]].member * 10000; \
}
        
    case COMBOTD:
        GET_COMBO_VAR_BUF(type, "Screen->ComboT") break;
        
    case COMBOID:
        GET_COMBO_VAR_BUF(flag, "Screen->ComboI") break;
        
    case COMBOSD:
    {
        int pos = ri->d[0] / 10000;
        
        if(BC::checkComboPos(pos, "Screen->ComboS") != SH::_NoError)
            ret = -10000;
        else
            ret = (combobuf[tmpscr->data[pos]].walk & 0xF) * 10000;
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//Game->GetComboX
    
    case COMBODDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        int layr = whichlayer(scr);
        
        if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
        {
            if(scr==(currmap*MAPSCRS+currscr))
                ret=tmpscr->data[pos]*10000;
            else if(layr>-1)
                ret=tmpscr2[layr].data[pos]*10000;
            else ret=TheMaps[scr].data[pos]*10000;
        }
        else
            ret = -10000;
    }
    break;
    
    case COMBOCDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        int layr = whichlayer(scr);
        
        if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
        {
            if(scr==(currmap*MAPSCRS+currscr))
                ret=tmpscr->cset[pos]*10000;
            else if(layr>-1)
                ret=tmpscr2[layr].cset[pos]*10000;
            else ret=TheMaps[scr].cset[pos]*10000;
        }
        else
            ret = -10000;
    }
    break;
    
    case COMBOFDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        int layr = whichlayer(scr);
        
        if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
        {
            if(scr==(currmap*MAPSCRS+currscr))
                ret=tmpscr->sflag[pos]*10000;
            else if(layr>-1)
                ret=tmpscr2[layr].sflag[pos]*10000;
            else ret=TheMaps[scr].sflag[pos]*10000;
        }
        else
            ret = -10000;
    }
    break;
    
    case COMBOTDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        int layr = whichlayer(scr);
        
        if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
        {
            if(scr==(currmap*MAPSCRS+currscr))
                ret=combobuf[tmpscr->data[pos]].type*10000;
            else if(layr>-1)
                ret=combobuf[tmpscr2[layr].data[pos]].type*10000;
            else ret=combobuf[
                             TheMaps[scr].data[pos]].type*10000;
        }
        else
            ret = -10000;
    }
    break;
    
    case COMBOIDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        int layr = whichlayer(scr);
        
        if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
        {
            if(scr==(currmap*MAPSCRS+currscr))
                ret=combobuf[tmpscr->data[pos]].flag*10000;
            else if(layr>-1)
                ret=combobuf[tmpscr2[layr].data[pos]].flag*10000;
            else ret=combobuf[TheMaps[scr].data[pos]].flag*10000;
        }
        else
            ret = -10000;
    }
    break;
    
    case COMBOSDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        int layr = whichlayer(scr);
        
        if(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)
        {
            if(scr==(currmap*MAPSCRS+currscr))
                ret=(combobuf[tmpscr->data[pos]].walk&15)*10000;
            else if(layr>-1)
                ret=(combobuf[tmpscr2[layr].data[pos]].walk&15)*10000;
            else ret=(combobuf[TheMaps[scr].data[pos]].walk&15)*10000;
        }
        else
            ret = -10000;
    }
    break;
    

   
    
///----------------------------------------------------------------------------------------------------//
//Screen Information
    
    	#define	GET_SCREENDATA_VAR_INT32(member, str) \
	{ \
		ret = (tmpscr->member *10000); \
	} \

	#define	GET_SCREENDATA_VAR_INT16(member, str) \
	{ \
		ret = (tmpscr->member *10000); \
	} \

	#define	GET_SCREENDATA_VAR_BYTE(member, str) \
	{ \
		ret = (tmpscr->member *10000); \
	} \
	
	#define GET_SCREENDATA_VAR_INDEX32(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		ret = (tmpscr->member[indx] *10000); \
	} \
	
	#define GET_SCREENDATA_VAR_INDEX16(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		ret = (tmpscr->member[indx] *10000); \
	} \
	
	#define GET_SCREENDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		ret = (tmpscr->member[indx] *10000); \
	} \
	
	//byte
	#define GET_SCREENDATA_LAYER_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to Screen->%s[]: %d\n", (indx), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (tmpscr->member[indx] *10000); \
		} \
	} \
	
	
	#define GET_SCREENDATA_BOOL_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to Screen->%s[]: %d\n", (indx), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (tmpscr->member[indx]?10000:0); \
		} \
	} \
	
	
	#define GET_SCREENDATA_FLAG(member, str, indexbound) \
	{ \
		long flag =  (value/10000);  \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		ret = (tmpscr->member&flag) ? 10000 : 0); \
	} \


case SCREENDATAVALID:		GET_SCREENDATA_VAR_BYTE(valid, "Valid"); break;		//b
case SCREENDATAGUY: 		GET_SCREENDATA_VAR_BYTE(guy, "Guy"); break;		//b
case SCREENDATASTRING:		GET_SCREENDATA_VAR_INT32(str, "String"); break;		//w
case SCREENDATAROOM: 		GET_SCREENDATA_VAR_BYTE(room, "RoomType");	break;		//b
case SCREENDATAITEM: 		GET_SCREENDATA_VAR_BYTE(item, "Item"); break;		//b
case SCREENDATAHASITEM: 		GET_SCREENDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
case SCREENDATATILEWARPTYPE: 	GET_SCREENDATA_BYTE_INDEX(tilewarptype, "TileWarpType", 3); break;	//b, 4 of these
case SCREENDATATILEWARPOVFLAGS: 	GET_SCREENDATA_VAR_BYTE(tilewarpoverlayflags, "TileWarpOverlayFlags"); break;	//b, tilewarpoverlayflags
case SCREENDATADOORCOMBOSET: 	GET_SCREENDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
case SCREENDATAWARPRETX:	 	GET_SCREENDATA_BYTE_INDEX(warpreturnx, "WarpReturnX", 3); break;	//b, 4 of these
case SCREENDATAWARPRETY:	 	GET_SCREENDATA_BYTE_INDEX(warpreturny, "WarpReturnY", 3); break;	//b, 4 of these
case SCREENDATAWARPRETURNC: 	GET_SCREENDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
case SCREENDATASTAIRX: 		GET_SCREENDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
case SCREENDATASTAIRY: 		GET_SCREENDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
case SCREENDATAITEMX:		GET_SCREENDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
case SCREENDATAITEMY:		GET_SCREENDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
case SCREENDATACOLOUR: 		GET_SCREENDATA_VAR_INT32(color, "CSet"); break;	//w
case SCREENDATAENEMYFLAGS: 	GET_SCREENDATA_VAR_BYTE(enemyflags, "EnemyFlags");	break;	//b
case SCREENDATADOOR: 		GET_SCREENDATA_BYTE_INDEX(door, "Door", 3); break;	//b, 4 of these
case SCREENDATATILEWARPDMAP: 	GET_SCREENDATA_VAR_INDEX32(tilewarpdmap, "TileWarpDMap", 3); break;	//w, 4 of these
case SCREENDATATILEWARPSCREEN: 	GET_SCREENDATA_BYTE_INDEX(tilewarpscr, "TileWarpScreen", 3); break;	//b, 4 of these
case SCREENDATAEXITDIR: 		GET_SCREENDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
case SCREENDATAENEMY: 		GET_SCREENDATA_VAR_INDEX32(enemy, "Enemy", 9); break;	//w, 10 of these
case SCREENDATAPATTERN: 		GET_SCREENDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
case SCREENDATASIDEWARPTYPE: 	GET_SCREENDATA_BYTE_INDEX(sidewarptype, "SideWarpType", 3); break;	//b, 4 of these
case SCREENDATASIDEWARPOVFLAGS: 	GET_SCREENDATA_VAR_BYTE(sidewarpoverlayflags, "SideWarpOverlayFlags"); break;	//b
case SCREENDATAWARPARRIVALX: 	GET_SCREENDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
case SCREENDATAWARPARRIVALY: 	GET_SCREENDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
case SCREENDATAPATH: 		GET_SCREENDATA_BYTE_INDEX(path, "MazePath", 3); break;	//b, 4 of these
case SCREENDATASIDEWARPSC: 	GET_SCREENDATA_BYTE_INDEX(sidewarpscr, "SideWarpScreen", 3); break;	//b, 4 of these
case SCREENDATASIDEWARPDMAP: 	GET_SCREENDATA_VAR_INDEX32(sidewarpdmap, "SideWarpDMap", 3); break;	//w, 4 of these
case SCREENDATASIDEWARPINDEX: 	GET_SCREENDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
case SCREENDATAUNDERCOMBO: 	GET_SCREENDATA_VAR_INT32(undercombo, "Undercombo"); break;	//w
case SCREENDATAUNDERCSET:	 	GET_SCREENDATA_VAR_BYTE(undercset,	"UnderCSet"); break; //b
case SCREENDATACATCHALL:	 	GET_SCREENDATA_VAR_INT32(catchall,	"Catchall"); break; //W

case SCREENDATACSENSITIVE: 	GET_SCREENDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
case SCREENDATANORESET: 		GET_SCREENDATA_VAR_INT32(noreset, "NoReset"); break;	//W
case SCREENDATANOCARRY: 		GET_SCREENDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
case SCREENDATALAYERMAP:	 	GET_SCREENDATA_LAYER_INDEX(layermap, "LayerMap", 5); break;	//B, 6 OF THESE
case SCREENDATALAYERSCREEN: 	GET_SCREENDATA_LAYER_INDEX(layerscreen, "LayerScreen", 5); break;	//B, 6 OF THESE
case SCREENDATALAYEROPACITY: 	GET_SCREENDATA_LAYER_INDEX(layeropacity, "LayerOpacity", 5); break;	//B, 6 OF THESE
case SCREENDATATIMEDWARPTICS: 	GET_SCREENDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
case SCREENDATANEXTMAP: 		GET_SCREENDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
case SCREENDATANEXTSCREEN: 	GET_SCREENDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
case SCREENDATASECRETCOMBO: 	GET_SCREENDATA_VAR_INDEX32(secretcombo, "SecretCombo", 127); break;	//W, 128 OF THESE
case SCREENDATASECRETCSET: 	GET_SCREENDATA_BYTE_INDEX(secretcset, "SecretCSet", 127); break;	//B, 128 OF THESE
case SCREENDATASECRETFLAG: 	GET_SCREENDATA_BYTE_INDEX(secretflag, "SecretFlags", 127); break;	//B, 128 OF THESE
case SCREENDATAVIEWX: 		GET_SCREENDATA_VAR_INT32(viewX, "ViewX"); break;	//W
case SCREENDATAVIEWY: 		GET_SCREENDATA_VAR_INT32(viewY, "ViewY"); break; //W
case SCREENDATASCREENWIDTH: 	GET_SCREENDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
case SCREENDATASCREENHEIGHT: 	GET_SCREENDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
case SCREENDATAENTRYX: 		GET_SCREENDATA_VAR_BYTE(entry_x, "EntryX"); break;	//B
case SCREENDATAENTRYY: 		GET_SCREENDATA_VAR_BYTE(entry_y, "EntryY"); break;	//B
case SCREENDATANUMFF: 		GET_SCREENDATA_VAR_INT16(numff, "NumFFCs"); break;	//INT16

	//inita	//INT32, 32 OF THESE, EACH WITH 2
case SCREENDATAFFINITIALISED: 	GET_SCREENDATA_BOOL_INDEX(initialized, "FFCRunning", 31); break;	//BOOL, 32 OF THESE
case SCREENDATASCRIPTENTRY: 	GET_SCREENDATA_VAR_INT32(script_entry, "ScriptEntry"); break;	//W
case SCREENDATASCRIPTOCCUPANCY: 	GET_SCREENDATA_VAR_INT32(script_occupancy,	"ScriptOccupancy");  break;//W
case SCREENDATASCRIPTEXIT: 	GET_SCREENDATA_VAR_INT32(script_exit, "ExitScript"); break;	//W
case SCREENDATAOCEANSFX:	 	GET_SCREENDATA_VAR_BYTE(oceansfx, "OceanSFX"); break;	//B
case SCREENDATABOSSSFX: 		GET_SCREENDATA_VAR_BYTE(bosssfx, "BossSFX"); break;	//B
case SCREENDATASECRETSFX:	 	GET_SCREENDATA_VAR_BYTE(secretsfx, "SecretSFX"); break;	//B
case SCREENDATAHOLDUPSFX:	 	GET_SCREENDATA_VAR_BYTE(holdupsfx,	"ItemSFX"); break; //B
case SCREENDATASCREENMIDI: 	GET_SCREENDATA_VAR_INT16(screen_midi, "MIDI"); break;	//SHORT, OLD QUESTS ONLY?
case SCREENDATALENSLAYER:	 	GET_SCREENDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
	

case SCREENDATAFLAGS: 
{
	int flagid = (ri->d[0])/10000;
	//bool valtrue = ( value ? 10000 : 0);
	switch(flagid)
	{
		case 0: ret = (tmpscr->flags * 10000); break;
		case 1: ret = (tmpscr->flags2 * 10000); break;
		case 2: ret = (tmpscr->flags3 * 10000); break;
		case 3: ret = (tmpscr->flags4 * 10000); break;
		case 4: ret = (tmpscr->flags5 * 10000); break;
		case 5: ret = (tmpscr->flags6 * 10000); break;
		case 6: ret = (tmpscr->flags7 * 10000); break;
		case 7: ret = (tmpscr->flags8 * 10000); break;
		case 8: ret = (tmpscr->flags9 * 10000); break;
		case 9: ret = (tmpscr->flags10 * 10000); break;
		default:
		{
			Z_scripterrlog("Invalid index passed to mapdata->flags[]: %d\n", flagid); 
			ret = -10000;
			break;
			
		}
	}
	break;
	//GET_SCREENDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10
}

    case SDD:
    {
        int di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
        ret=FFScript::get_screen_d(di, ri->d[0]/10000);
    }
    break;
    
    case SDDD:
        ret=FFScript::get_screen_d((ri->d[0])/10000 + ((get_currdmap())<<7), ri->d[1] / 10000);
        break;
        
    case SDDDD:
        ret=FFScript::get_screen_d(ri->d[1] / 10000 + ((ri->d[0]/10000)<<7), ri->d[2] / 10000);
        break;
        
    case SCRDOORD:
        ret=tmpscr->door[ri->d[0]/10000]*10000;
        break;
    
    
    //These use the same method as GetScreenD -Z
    case SCREENWIDTH:
        ret=FFScript::get_screenWidth(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENHEIGHT:
        ret=FFScript::get_screenHeight(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENVIEWX:
        ret=get_screenViewX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENVIEWY:
        ret=get_screenViewY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENGUY:
        ret=get_screenGuy(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENSTRING:
        ret=get_screenString(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENROOM:
        ret=get_screenRoomtype(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENENTX:
        ret=get_screenEntryX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENENTY:
        ret=get_screenEntryY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENITEM:
        ret=get_screenitem(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENUNDCMB:
        ret=get_screenundercombo(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENUNDCST:
        ret=get_screenundercset(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case SCREENCATCH:
        ret=get_screenatchall(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;
 
        
    case LIT:
        ret= darkroom ? 0 : 10000;
        break;
        
    case WAVY:
        ret = wavy*10000;
        break;
        
    case QUAKE:
        ret = quakeclk*10000;
        break;
        
    case SCREENFLAGSD:
        ret = get_screenflags(tmpscr,vbound(ri->d[0] / 10000,0,9));
        break;
        
    case SCREENEFLAGSD:
        ret = get_screeneflags(tmpscr,vbound(ri->d[0] / 10000,0,2));
        break;
        
    case NPCCOUNT:
        ret = guys.Count()*10000;
        break;
        
    case ROOMDATA:
        ret = tmpscr->catchall*10000;
        break;
        
    case ROOMTYPE:
        ret = tmpscr->room*10000;
        break;
        
    case PUSHBLOCKX:
        ret = blockmoving ? int(mblock2.x)*10000 : -10000;
        break;
        
    case PUSHBLOCKY:
        ret = blockmoving ? int(mblock2.y)*10000 : -10000;
        break;
        
    case PUSHBLOCKCOMBO:
        ret = mblock2.bcombo*10000;
        break;
        
    case PUSHBLOCKCSET:
        ret = mblock2.cs*10000;
        break;
        
    case UNDERCOMBO:
        ret = tmpscr->undercombo*10000;
        break;
        
    case UNDERCSET:
        ret = tmpscr->undercset*10000;
        break;
    
    //Creates an lweapon using an iemdata struct values to generate its properties.
	//Useful in conjunction with the new weapon editor. 
        case CREATELWPNDX:
	{
		//Z_message("Trying to get Link->SetExtend().\n");
		long ID = (ri->d[0] / 10000);
		int itemid = (ri->d[1]/10000);
		itemid = vbound(itemid,0,(MAXITEMS-1));
		
		//Z_scripterrlog("GetLinkExtend rid->[2] is (%i), trying to use for '%s'\n", ri->d[2], "ri->d[2]");
	    //Z_scripterrlog("GetLinkExtend rid->[1] is (%i), trying to use for '%s'\n", state, "state");
	    //Z_scripterrlog("GetLinkExtend rid->[0] is (%i), trying to use for '%s'\n", dir, "dir");
	
		Lwpns.add(new weapon((fix)0,(fix)0,(fix)0,ID,0,0,0,itemid,Link.getUID()));
		ri->lwpn = Lwpns.spr(Lwpns.Count() - 1)->getUID();
		
		/* Z_scripterrlog("CreateLWeaponDx ri->d[0] is (%i), trying to use for '%s'\n", ID, "ID");
	    Z_scripterrlog("CreateLWeaponDx ri->d[1] is (%i), trying to use for '%s'\n", itemid, "itemid");
	Z_scripterrlog("CreateLWeaponDx ri->lwpn is (%i), trying to use for '%s'\n", ri->lwpn, "ri->lwpn"); */
		
		ret = ri->lwpn; 
	}
	break;
	
	/*
	//Screen->CollisionDx(int enemy_index, int lweapon_type, int power, int x, int y, int weapon_dir, int parent_item);
	//Handles collision between an npc at screen index 'enemy_index', and an arbitrary lweapon that may or may not exist.
	//Does damage, and returns true a value collision is valid. Returns a different value otherwise. 
	
	// take damage or ignore it
	// -1: damage (if any) dealt
	// 1: blocked
	// 0: weapon passes through unhindered
	// 2: heal enemy? -ZoriaRPG
	
	//THis function should automatically check for shields blocking the weapon, and calculate enemy defences.
	//In theory, scritped swords, and other melee weapons could use this, as coudl any weapon, and it may simplify
	//collision routines. 
	case COLLISIONDX:
	{
		//Z_message("Trying to get Link->SetExtend().\n");
		int index = (ri->d[0] / 10000);
		long lweapon_type = (ri->d[1] / 10000);
		int power = (ri->d[2]/10000);
		
		int wpnx = ri->4[3]/10000, wpny = ri->d[4]/10000;
		int dir = ri->d[5]/10000;
		int parentitem = (ri->d[6]/10000);
		lweapon_type = vbound(lweapon_type,0,40); //Are we at 40, or higher now>
		parentitem = vbound(itemid,0,255);
		
		//Log the stack events:
		Z_scripterrlog("CollisionDx ri->d[0] is (%i), trying to use for '%s'\n", index, "index");
		Z_scripterrlog("CollisionDx ri->d[1] is (%i), trying to use for '%s'\n", lweapon_type, "lweapon_type");
		Z_scripterrlog("CollisionDx ri->d[2] is (%i), trying to use for '%s'\n", power, "power");
		Z_scripterrlog("CollisionDx ri->d[3] is (%i), trying to use for '%s'\n", wpnx, "wpnx");
		Z_scripterrlog("CollisionDx ri->d[4] is (%i), trying to use for '%s'\n", wpny, "wpny");
		Z_scripterrlog("CollisionDx ri->d[5] is (%i), trying to use for '%s'\n", dir, "dir");
		Z_scripterrlog("CollisionDx ri->d[0] is (%i), trying to use for '%s'\n", parentitem, "parentitem");
		
		weapon *w = new weapon((fix)wpnx,(fix)wpny,(fix)0,lweapon_type,0,power,dir,parentitem,-1,false);
		int retval = ((enemy*)guys.spr(index))->takehit(w); 
			//TakeHit, as I recall, applies damage and returns the hit status. Gold.
		delete w;
		ret = retval;
	}
	break;
	
	*/
    case DEBUGREFFFC:
    {
	int r = -1;
	if ( ri->ffcref ) r = ri->ffcref;
        ret = r * 10000;
    }
        break;
        
    case DEBUGREFITEM:
    {
	int r = -1;
	if ( ri->itemref ) r = ri->itemref;
        ret = r * 10000;
    }
        break;
        
    case DEBUGREFITEMDATA:
    {
	int r = -1;
	if ( ri->idata ) r = ri->idata;
        ret = r * 10000;
    }
        break;
        
    case DEBUGREFLWEAPON:
    {
	int r = -1;
	if ( ri->lwpn ) r = ri->lwpn;
        ret = r * 10000;
    }
        break;
        
    case DEBUGREFEWEAPON:
    {
	int r = -1;
	if ( ri->ewpn ) r = ri->ewpn;
        ret = r * 10000;
    }
        break;
        
    case DEBUGREFNPC:
    {
	int r = -1;
	if ( ri->guyref ) r = ri->guyref;
        ret = r * 10000;
    }    
        break;
        
    case DEBUGSP:
    {
	int r = -1;
	if ( ri->sp ) r = ri->sp;
        ret = r * 10000;
    }   
        break;
    case DEBUGGDR:
    {
	int a = vbound(ri->d[0]/10000,0,15);
	int r = -1;
	if ( game->global_d[a] ) r = game->global_d[a];
        ret = r * 10000;
    }
        break;
    
///----------------------------------------------------------------------------------------------------//
//New Datatype Variables
    
///----------------------------------------------------------------------------------------------------//
//spritedata sp-> Variables
    
case SPRITEDATATILE: GET_SPRITEDATA_VAR_INT(newtile, "Tile") break;
case SPRITEDATAMISC: GET_SPRITEDATA_VAR_INT(misc, "Misc") break;
case SPRITEDATACSETS: GET_SPRITEDATA_VAR_INT(csets, "CSet") break;
case SPRITEDATAFRAMES: GET_SPRITEDATA_VAR_INT(frames, "Frames") break;
case SPRITEDATASPEED: GET_SPRITEDATA_VAR_INT(speed, "Speed") break;
case SPRITEDATATYPE: GET_SPRITEDATA_VAR_INT(type, "Type") break;
    
///----------------------------------------------------------------------------------------------------//
//mapdata m-> variables
    #define	GET_MAPDATA_VAR_INT32(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		ret = (m->member *10000); \
	} \

	#define	GET_MAPDATA_VAR_INT16(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		ret = (m->member *10000); \
	} \

	#define	GET_MAPDATA_VAR_BYTE(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		ret = (m->member *10000); \
	} \
	
	#define GET_MAPDATA_VAR_INDEX32(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			ret = -10000; \
		} \
		else \
		{ \
			mapscr *m = &TheMaps[ri->mapsref]; \
			ret = (m->member[indx] *10000); \
		} \
	} \
	
	#define GET_MAPDATA_VAR_INDEX16(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			ret = -10000; \
		} \
		else \
		{ \
			mapscr *m = &TheMaps[ri->mapsref]; \
			ret = (m->member[indx] *10000); \
		} \
	} \
	
	#define GET_MAPDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			ret = -10000; \
		} \
		else \
		{ \
			mapscr *m = &TheMaps[ri->mapsref]; \
			ret = (m->member[indx] *10000); \
		} \
	} \
	
	/*
	#define GET_MAPDATA_LAYER_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		mapscr *m = &TheMaps[ri->mapsref]; \
		if ( indx == 0 ) \
		{ \
			\
		} \
		else \
		{ \
			ret = (m->member[indx-1] *10000); \
		} \
	} \
	*/
	
	#define GET_MAPDATA_LAYER_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			ret = -10000; \
		} \
		else \
		{ \
			mapscr *m = &TheMaps[ri->mapsref]; \
			ret = (m->member[indx] *10000); \
		} \
	} \
	
	#define GET_MAPDATA_BOOL_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			ret = -10000; \
		} \
		else \
		{ \
			mapscr *m = &TheMaps[ri->mapsref]; \
			ret = (m->member[indx]?10000:0); \
		} \
	} \
	
	#define GET_MAPDATA_FLAG(member, str) \
	{ \
		long flag =  (value/10000);  \
		mapscr *m = &TheMaps[ri->mapsref]; \
		ret = (m->member&flag) ? 10000 : 0); \
	} \
	
	#define GET_SCREENDATA_COMBO_VAR(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		int pos = ri->d[0] / 10000; \
		if(BC::checkComboPos(pos, str) != SH::_NoError) \
		ret = -10000; \
		else \
			ret = m->member[pos]*10000; \
	}

	#define GET_MAPDATA_COMBO_VAR_BUF(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		int pos = ri->d[0] / 10000; \
		if(BC::checkComboPos(pos, str) != SH::_NoError) \
			ret = -10000; \
		else \
			ret = combobuf[m->data[pos]].member * 10000; \
	}

case LOADMAPDATA:
        ret=FFScript::loadMapData();
        break;


case MAPDATAVALID:		GET_MAPDATA_VAR_BYTE(valid, "Valid"); break;		//b
case MAPDATAGUY: 		GET_MAPDATA_VAR_BYTE(guy, "Guy"); break;		//b
case MAPDATASTRING:		GET_MAPDATA_VAR_INT32(str, "String"); break;		//w
case MAPDATAROOM: 		GET_MAPDATA_VAR_BYTE(room, "RoomType");	break;		//b
case MAPDATAITEM: 		GET_MAPDATA_VAR_BYTE(item, "Item"); break;		//b
case MAPDATAHASITEM: 		GET_MAPDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
case MAPDATATILEWARPTYPE: 	GET_MAPDATA_BYTE_INDEX(tilewarptype, "TileWarpType", 3); break;	//b, 4 of these
case MAPDATATILEWARPOVFLAGS: 	GET_MAPDATA_VAR_BYTE(tilewarpoverlayflags, "TileWarpOverlayFlags"); break;	//b, tilewarpoverlayflags
case MAPDATADOORCOMBOSET: 	GET_MAPDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
case MAPDATAWARPRETX:	 	GET_MAPDATA_BYTE_INDEX(warpreturnx, "WarpReturnX", 3); break;	//b, 4 of these
case MAPDATAWARPRETY:	 	GET_MAPDATA_BYTE_INDEX(warpreturny, "WarpReturnY", 3); break;	//b, 4 of these
case MAPDATAWARPRETURNC: 	GET_MAPDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
case MAPDATASTAIRX: 		GET_MAPDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
case MAPDATASTAIRY: 		GET_MAPDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
case MAPDATAITEMX:		GET_MAPDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
case MAPDATAITEMY:		GET_MAPDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
case MAPDATACOLOUR: 		GET_MAPDATA_VAR_INT32(color, "CSet"); break;	//w
case MAPDATAENEMYFLAGS: 	GET_MAPDATA_VAR_BYTE(enemyflags, "EnemyFlags");	break;	//b
case MAPDATADOOR: 		GET_MAPDATA_BYTE_INDEX(door, "Door", 3); break;	//b, 4 of these
case MAPDATATILEWARPDMAP: 	GET_MAPDATA_VAR_INDEX32(tilewarpdmap, "TileWarpDMap", 3); break;	//w, 4 of these
case MAPDATATILEWARPSCREEN: 	GET_MAPDATA_BYTE_INDEX(tilewarpscr, "TileWarpScreen", 3); break;	//b, 4 of these
case MAPDATAEXITDIR: 		GET_MAPDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
case MAPDATAENEMY: 		GET_MAPDATA_VAR_INDEX32(enemy, "Enemy", 9); break;	//w, 10 of these
case MAPDATAPATTERN: 		GET_MAPDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
case MAPDATASIDEWARPTYPE: 	GET_MAPDATA_BYTE_INDEX(sidewarptype, "SideWarpType", 3); break;	//b, 4 of these
case MAPDATASIDEWARPOVFLAGS: 	GET_MAPDATA_VAR_BYTE(sidewarpoverlayflags, "SideWarpOverlayFlags"); break;	//b
case MAPDATAWARPARRIVALX: 	GET_MAPDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
case MAPDATAWARPARRIVALY: 	GET_MAPDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
case MAPDATAPATH: 		GET_MAPDATA_BYTE_INDEX(path, "MazePath", 3); break;	//b, 4 of these
case MAPDATASIDEWARPSC: 	GET_MAPDATA_BYTE_INDEX(sidewarpscr, "SideWarpScreen", 3); break;	//b, 4 of these
case MAPDATASIDEWARPDMAP: 	GET_MAPDATA_VAR_INDEX32(sidewarpdmap, "SideWarpDMap", 3); break;	//w, 4 of these
case MAPDATASIDEWARPINDEX: 	GET_MAPDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
case MAPDATAUNDERCOMBO: 	GET_MAPDATA_VAR_INT32(undercombo, "Undercombo"); break;	//w
case MAPDATAUNDERCSET:	 	GET_MAPDATA_VAR_BYTE(undercset,	"UnderCSet"); break; //b
case MAPDATACATCHALL:	 	GET_MAPDATA_VAR_INT32(catchall,	"Catchall"); break; //W

case MAPDATACSENSITIVE: 	GET_MAPDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
case MAPDATANORESET: 		GET_MAPDATA_VAR_INT32(noreset, "NoReset"); break;	//W
case MAPDATANOCARRY: 		GET_MAPDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
case MAPDATALAYERMAP:	 	GET_MAPDATA_LAYER_INDEX(layermap, "LayerMap", 5); break;	//B, 6 OF THESE
case MAPDATALAYERSCREEN: 	GET_MAPDATA_LAYER_INDEX(layerscreen, "LayerScreen", 5); break;	//B, 6 OF THESE
case MAPDATALAYEROPACITY: 	GET_MAPDATA_LAYER_INDEX(layeropacity, "LayerOpacity", 5); break;	//B, 6 OF THESE
case MAPDATATIMEDWARPTICS: 	GET_MAPDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
case MAPDATANEXTMAP: 		GET_MAPDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
case MAPDATANEXTSCREEN: 	GET_MAPDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
case MAPDATASECRETCOMBO: 	GET_MAPDATA_VAR_INDEX32(secretcombo, "SecretCombo", 127); break;	//W, 128 OF THESE
case MAPDATASECRETCSET: 	GET_MAPDATA_BYTE_INDEX(secretcset, "SecretCSet", 127); break;	//B, 128 OF THESE
case MAPDATASECRETFLAG: 	GET_MAPDATA_BYTE_INDEX(secretflag, "SecretFlags", 127); break;	//B, 128 OF THESE
case MAPDATAVIEWX: 		GET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
case MAPDATAVIEWY: 		GET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
case MAPDATASCREENWIDTH: 	GET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
case MAPDATASCREENHEIGHT: 	GET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
case MAPDATAENTRYX: 		GET_MAPDATA_VAR_BYTE(entry_x, "EntryX"); break;	//B
case MAPDATAENTRYY: 		GET_MAPDATA_VAR_BYTE(entry_y, "EntryY"); break;	//B
case MAPDATANUMFF: 		GET_MAPDATA_VAR_INT16(numff, "NumFFCs"); break;	//INT16
case MAPDATAFFDATA: 		GET_MAPDATA_VAR_INDEX32(ffdata,	"FFCData", 31); break;	//W, 32 OF THESE
case MAPDATAFFCSET: 		GET_MAPDATA_BYTE_INDEX(ffcset,	"FFCCSet", 31); break;	//B, 32
case MAPDATAFFDELAY: 		GET_MAPDATA_VAR_INDEX32(ffdelay, "FFCDelay", 31); break;	//W, 32
case MAPDATAFFX: 		GET_MAPDATA_VAR_INDEX32(ffx, "FFCX", 31); break;	//INT32, 32 OF THESE
case MAPDATAFFY: 		GET_MAPDATA_VAR_INDEX32(ffy, "FFCY", 31); break;	//..
case MAPDATAFFXDELTA:	 	GET_MAPDATA_VAR_INDEX32(ffxdelta, "FFCVx", 31); break;	//..
case MAPDATAFFYDELTA: 		GET_MAPDATA_VAR_INDEX32(ffydelta, "FFCVy", 31); break;	//..
case MAPDATAFFXDELTA2:	 	GET_MAPDATA_VAR_INDEX32(ffxdelta2, "FFCAx", 31); break;	//..
case MAPDATAFFYDELTA2:	 	GET_MAPDATA_VAR_INDEX32(ffydelta2, "FFCAy", 31); break;	//..
case MAPDATAFFFLAGS: 		GET_MAPDATA_VAR_INDEX16(ffflags, "FFCFlags", 31); break;	//INT16, 32 OF THESE
//Height and With are Or'd together, and need to be separate:
/*
 //TileWidth ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref] & ~63) | (((value/10000)-1)&63);
*/
case MAPDATAFFWIDTH: 		
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCTileWidth[]: %d\n", indx);
		ret = -10000;
		break;
	}
	ret = ((m->ffwidth[indx]&0x3F)+1)*10000;
	break;
}	


//GET_MAPDATA_BYTE_INDEX(ffwidth, "FFCTileWidth");	//B, 32 OF THESE
case MAPDATAFFHEIGHT:	 	
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCTileHeight[]: %d\n", indx);
		ret = -10000;
		break;
	}
	ret=((m->ffheight[indx]&0x3F)+1)*10000;
	break;
	
}

//EffectWidth tmpscr->ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref]&63) | ((((value/10000)-1)&3)<<6);

//GET_MAPDATA_BYTE_INDEX(ffheight, "FFCTileHeight"	//B, 32 OF THESE
case MAPDATAFFEFFECTWIDTH: 		
{ 
	mapscr *m = &TheMaps[ri->mapsref]; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectWidth[]: %d\n", indx);
		ret = -10000;
		break;
	}
	ret=((m->ffwidth[indx]>>6)+1)*10000;
	break;
}


//GET_MAPDATA_BYTE_INDEX(ffwidth, "FFCEffectWidth");	//B, 32 OF THESE
case MAPDATAFFEFFECTHEIGHT:
{ 
	mapscr *m = &TheMaps[ri->mapsref];
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectHeight[]: %d\n", indx);
		ret = -10000;
		break;
	}
	ret=((m->ffheight[indx]>>6)+1)*10000;
	break;
}	
	
//GET_MAPDATA_BYTE_INDEX(ffheight, "FFCEffectHeight"	//B, 32 OF THESE	

case MAPDATAFFLINK: 		GET_MAPDATA_BYTE_INDEX(fflink, "FFCLink", 31); break;	//B, 32 OF THESE
case MAPDATAFFSCRIPT:	 	GET_MAPDATA_VAR_INDEX32(ffscript, "FFCScript", 31); break;	//W, 32 OF THESE

case MAPDATAINTID: 	 //Same form as SetScreenD()
	//SetFFCInitD(ffindex, d, value)
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	//int v = (value/10000);
	ret = (m->initd[(ri->d[0]/10000)][(ri->d[1]/10000)]) * 10000;
	break;
}	
	

//initd	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 

case MAPDATAINITA: 		
	//same form as SetScreenD
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	//int v = (value/10000);
	ret = (m->inita[(ri->d[0]/10000)][(ri->d[1]/10000)]) * 10000;
	break;
}	

	//inita	//INT32, 32 OF THESE, EACH WITH 2
case MAPDATAFFINITIALISED: 	GET_MAPDATA_BOOL_INDEX(initialized, "FFCRunning", 31); break;	//BOOL, 32 OF THESE
case MAPDATASCRIPTENTRY: 	GET_MAPDATA_VAR_INT32(script_entry, "ScriptEntry"); break;	//W
case MAPDATASCRIPTOCCUPANCY: 	GET_MAPDATA_VAR_INT32(script_occupancy,	"ScriptOccupancy");  break;//W
case MAPDATASCRIPTEXIT: 	GET_MAPDATA_VAR_INT32(script_exit, "ExitScript"); break;	//W
case MAPDATAOCEANSFX:	 	GET_MAPDATA_VAR_BYTE(oceansfx, "OceanSFX"); break;	//B
case MAPDATABOSSSFX: 		GET_MAPDATA_VAR_BYTE(bosssfx, "BossSFX"); break;	//B
case MAPDATASECRETSFX:	 	GET_MAPDATA_VAR_BYTE(secretsfx, "SecretSFX"); break;	//B
case MAPDATAHOLDUPSFX:	 	GET_MAPDATA_VAR_BYTE(holdupsfx,	"ItemSFX"); break; //B
case MAPDATASCREENMIDI: 	GET_MAPDATA_VAR_INT16(screen_midi, "MIDI"); break;	//SHORT, OLD QUESTS ONLY?
case MAPDATALENSLAYER:	 	GET_MAPDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
	

case MAPDATAFLAGS: 
{
	int flagid = (ri->d[0])/10000;
	mapscr *m = &TheMaps[ri->mapsref]; 
	//bool valtrue = ( value ? 10000 : 0);
	switch(flagid)
	{
		case 0: ret = (m->flags * 10000); break;
		case 1: ret = (m->flags2 * 10000); break;
		case 2: ret = (m->flags3 * 10000); break;
		case 3: ret = (m->flags4 * 10000); break;
		case 4: ret = (m->flags5 * 10000); break;
		case 5: ret = (m->flags6 * 10000); break;
		case 6: ret = (m->flags7 * 10000); break;
		case 7: ret = (m->flags8 * 10000); break;
		case 8: ret = (m->flags9 * 10000); break;
		case 9: ret = (m->flags10 * 10000); break;
		default:
		{
			Z_scripterrlog("Invalid index passed to mapdata->flags[]: %d\n", flagid); 
			ret = -10000;
			break;
			
		}
	}
	break;
	//GET_MAPDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10
}

case MAPDATAMISCD:
{
	int indx = (ri->d[0])/10000;
	
	if(indx < 0 || indx > 9)
	{
		Z_scripterrlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", indx);
		ret = -10000;
		break;
	}
	else 
	{
	    ret = (game->screen_d[ri->mapsref][indx]) * 10000;
	    break;
	}
}

    case MAPDATACOMBODD:
        GET_SCREENDATA_COMBO_VAR(data,  "mapdata->ComboD") break;
        
    case MAPDATACOMBOCD:
        GET_SCREENDATA_COMBO_VAR(cset,  "mapdata->ComboC") break;
        
    case MAPDATACOMBOFD:
        GET_SCREENDATA_COMBO_VAR(sflag, "mapdata->ComboF") break;
        

        
    case MAPDATACOMBOTD:
        GET_MAPDATA_COMBO_VAR_BUF(type, "mapdata->ComboT") break;
        
    case MAPDATACOMBOID:
        GET_MAPDATA_COMBO_VAR_BUF(flag, "mapdata->ComboI") break;
        
    case MAPDATACOMBOSD:
    {
        int pos = ri->d[0] / 10000;
	mapscr *m = &TheMaps[ri->mapsref];
        
        if(BC::checkComboPos(pos, "mapdata->ComboS") != SH::_NoError)
            ret = -10000;
        else
            ret = (combobuf[m->data[pos]].walk & 0xF) * 10000;
    }
    break;
    
    case MAPDATASCREENSTATED:
    {
        ret=((game->maps[ri->mapsref]>>((ri->d[0]/10000)))&1)?10000:0;
    }
    break;
    case MAPDATASCREENFLAGSD:
    {
	mapscr *m = &TheMaps[ri->mapsref];
        ret = get_screenflags(m,vbound(ri->d[0] / 10000,0,9));
        break;
    }
        
    case MAPDATASCREENEFLAGSD:
    {
	mapscr *m = &TheMaps[ri->mapsref];
        ret = get_screeneflags(m,vbound(ri->d[0] / 10000,0,2));
        break;
    }

///----------------------------------------------------------------------------------------------------//
//shopdata sd-> variables
  
	
	case SHOPDATAITEM:
	{ 
		
		int ref = ri->shopsref; 
		bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
		int indx = ri->d[0] / 10000; 
		if ( indx < 0 || indx > 2 ) 
		{ 
			Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "Item");
			ret = -10000;
		} 
		else 
		{ 
			if ( isInfo ) 
			{ 
				Z_scripterrlog("Attempted to load an 'item' from an infoshop, using shop ID: %d\n", ri->shopsref); 
				ret = -10000;
			} 
			else 
			{ 
				ret = ((int)(QMisc.shop[ref].item[indx]) * 10000); 
			} 
		} 
	break;
	} 
	
	
		// GET_SHOPDATA_VAR_INDEX(item, "Item", 2); break;
	case SHOPDATAHASITEM: 
	{ 
		
		int ref = ri->shopsref; 
		bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
		int indx = ri->d[0] / 10000; 
		if ( indx < 0 || indx > 2 ) 
		{ 
			Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "HasItem"); 
			ret = -10000;
		} 
		else 
		{ 
			if ( isInfo ) 
			{ 
				Z_scripterrlog("Attempted to load 'hasitem' from an infoshop, using shop ID: %d\n", ri->shopsref); 
				ret = -10000;
			} 
			else 
			{ 
				ret = ((int)(QMisc.shop[ref].hasitem[indx]) * 10000); 
			} 
		} 
	break;
	} 
	
		
	//GET_SHOPDATA_VAR_INDEX(hasitem, "HasItem", 2); break;
	case SHOPDATAPRICE: 
	{ 
		
		int ref = ri->shopsref; 
		bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
		int indx = ri->d[0] / 10000; 
		if ( indx < 0 || indx > 2 ) 
		{ 
			Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "Price"); 
			ret = -10000;
		} 
		else 
		{ 
			if ( isInfo ) 
			{ 
				ret = ((int)(QMisc.info[ref].price[indx]) * 10000); 
			} 
			else 
			{ 
				ret = ((int)(QMisc.shop[ref].price[indx]) * 10000); 
			} 
		} 
	break;
	} 
	
		
	//GET_SHOPDATA_VAR_INDEX(price, "Price", 2); break;
	//Pay for info
	case SHOPDATASTRING:
	{
		if ( ri->shopsref < NUMSHOPS || ri->shopsref > NUMINFOSHOPS )
		{
			Z_scripterrlog("Invalid Info Shop ID passed to shopdata->String[]: %d\n", ri->shopsref); 
			ret = -10000;
		}
		else 
		{
			int ref = ri->shopsref; 
			bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
			int indx = ri->d[0] / 10000; 
			if ( indx < 0 || indx > 2 ) 
			{ 
				Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "HasItem"); 
			} 
			else 
			{ 
				if ( isInfo ) 
				{ 
					ret = ((int)(QMisc.info[ref].str[indx]) * 10000); 
				} 
				else 
				{ 
					ret = ((int)(QMisc.shop[ref].str[indx]) * 10000); 
				} 
			} 
	
	
		}
			
		//GET_SHOPDATA_VAR_INDEX(str, String, 2); break;
	break;
	}
   

///----------------------------------------------------------------------------------------------------//
//dmapdata dmd-> variables

//getter
case DMAPDATAMAP: 	//byte
{
	ret = ((byte)DMaps[ri->dmapsref].map) * 10000; break;
}
case DMAPDATALEVEL:	//word
{
	ret = ((word)DMaps[ri->dmapsref].level) * 10000; break;
}
case DMAPDATAOFFSET:	//char
{
	ret = ((char)DMaps[ri->dmapsref].xoff) * 10000; break;
}
case DMAPDATACOMPASS:	//byte
{
	ret = ((byte)DMaps[ri->dmapsref].compass) * 10000; break;
}
case DMAPDATAPALETTE:	//word
{
	ret = ((word)DMaps[ri->dmapsref].color) * 10000; break;
}
case DMAPDATAMIDI:	//byte
{
	ret = ((byte)DMaps[ri->dmapsref].midi) * 10000; break;
}
case DMAPDATACONTINUE:	//byte
{
	ret = ((byte)DMaps[ri->dmapsref].cont) * 10000; break;
}
case DMAPDATATYPE:	//byte
{
	ret = ((byte)DMaps[ri->dmapsref].type) * 10000; break;
}
case DMAPDATASIDEVIEW:	//byte
{
	ret = (((byte)DMaps[ri->dmapsref].sideview)!=0) ? 10000 : 0; break;
}
case DMAPDATAGRID:	//byte[8] --array
{
	int indx = ri->d[0] / 10000;
	if ( indx < 0 || indx > 7 ) 
	{
		Z_scripterrlog("Invalid index supplied to dmapdata->Grid[]: %d\n", indx);
		ret = -10000;
	}
	else
	{
		ret = ((byte)DMaps[ri->dmapsref].grid[indx]) * 10000;  break;
	}
}
case DMAPDATAMINIMAPTILE:	//word - two of these, so let's do MinimapTile[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { ret = ((word)DMaps[ri->dmapsref].minimap_1_tile) * 10000; break; }
		case 1: { ret = ((word)DMaps[ri->dmapsref].minimap_2_tile) * 10000; break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->MiniMapTile[]: %d\n", indx);
			ret = -10000;
			break;
		}
	}
}
case DMAPDATAMINIMAPCSET:	//byte - two of these, so let's do MinimapCSet[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { ret = ((byte)DMaps[ri->dmapsref].minimap_1_cset) * 10000; break; }
		case 1: { ret = ((byte)DMaps[ri->dmapsref].minimap_2_cset) * 10000; break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->MiniMapCSet[]: %d\n", indx);
			ret = -10000;
			break;
		}
	}
}
case DMAPDATALARGEMAPTILE:	//word -- two of these, so let's to LargemapTile[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { ret = ((word)DMaps[ri->dmapsref].largemap_1_tile) * 10000; break; }
		case 1: { ret = ((word)DMaps[ri->dmapsref].largemap_2_tile) * 10000; break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->LargeMapTile[]: %d\n", indx);
			ret = -10000;
			break;
		}
	}
}
case DMAPDATALARGEMAPCSET:	//word -- two of these, so let's to LargemaCSet[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { ret = ((byte)DMaps[ri->dmapsref].largemap_1_cset) * 10000; break; }
		case 1: { ret = ((byte)DMaps[ri->dmapsref].largemap_2_cset) * 10000; break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->LargeMapCSet[]: %d\n", indx);
			ret = -10000;
			break;
		}
	}
}
case DMAPDATAMUISCTRACK:	//byte
{
	ret = ((byte)DMaps[ri->dmapsref].tmusictrack) * 10000; break;
}
case DMAPDATASUBSCRA:	 //byte, active subscreen
{
	ret = ((byte)DMaps[ri->dmapsref].active_subscreen) * 10000; break;
}
case DMAPDATASUBSCRP:	 //byte, passive subscreen
{
	ret = ((byte)DMaps[ri->dmapsref].passive_subscreen) * 10000; break;
}
case DMAPDATADISABLEDITEMS:	 //byte[iMax]
{
	int indx = ri->d[0] / 10000;
	if ( indx < 0 || indx > (iMax-1) ) 
	{
		Z_scripterrlog("Invalid index supplied to dmapdata->Grid[]: %d\n", indx);
		ret = -10000;
		break;
	}
	else
	{
		ret = ((byte)DMaps[ri->dmapsref].disableditems[indx]) * 10000; break;
	}
}
case DMAPDATAFLAGS:	 //long
{
	ret = (DMaps[ri->dmapsref].flags) * 10000; break;
}
//case DMAPDATAGRAVITY:	 //unimplemented
//case DMAPDATAJUMPLAYER:	 //unimplemented
	
///----------------------------------------------------------------------------------------------------//
//messagedata msgd-> variables
case MESSAGEDATANEXT: //W
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Next") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].nextstring) * 10000;
	break;
}	

case MESSAGEDATATILE: //W
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Tile") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].tile) * 10000;
	break;
}	

case MESSAGEDATACSET: //b
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->CSet") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].cset) * 10000;
	break;
}	
case MESSAGEDATATRANS: //BOOL
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Transparent") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((MsgStrings[ID].trans)?10000:0);
	break;
}	
case MESSAGEDATAFONT: //B
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Font") != SH::_NoError)
		ret = -10000;
	else 
		ret = (int)MsgStrings[ID].font * 10000;
	break;
}	
case MESSAGEDATAX: //SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->X") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].x) * 10000;
	break;
}	
case MESSAGEDATAY: //SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Y") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].y) * 10000;
	break;
}	
case MESSAGEDATAW: //UNSIGNED SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Width") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].w) * 10000;
	break;
}	
case MESSAGEDATAH: //UNSIGNED SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Height") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].h) * 10000;
	break;
}	
case MESSAGEDATASFX: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Sound") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].sfx) * 10000;
	break;
}	
case MESSAGEDATALISTPOS: //WORD
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->ListPosition") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].listpos) * 10000;
	break;
}	
case MESSAGEDATAVSPACE: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->VSpace") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].vspace) * 10000;
	break;
}	
case MESSAGEDATAHSPACE: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->HSpace") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].hspace) * 10000;
	break;
}	
case MESSAGEDATAFLAGS: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Flags") != SH::_NoError)
		ret = -10000;
	else 
		ret = ((int)MsgStrings[ID].stringflags) * 10000;
	break;
}


///----------------------------------------------------------------------------------------------------//
//combodata cd-> Getter variables
	#define	GET_COMBO_VAR_INT(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combobuf[ri->combosref].member *10000); \
		} \
	} \

	#define	GET_COMBO_VAR_BYTE(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combobuf[ri->combosref].member *10000); \
		} \
	} \
	
	#define	GET_COMBO_VAR_DWORD(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combobuf[ri->combosref].member *10000); \
		} \
	} \
	
	#define GET_COMBO_VAR_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
				ret = -10000; \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combobuf[ri->combosref].member[indx] * 10000); \
			} \
	}

	#define GET_COMBO_BYTE_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
				ret = -10000; \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combobuf[ri->combosref].member[indx] * 100000); \
			} \
	}
	
	#define GET_COMBO_FLAG(member, str, indexbound) \
	{ \
		long flag =  (value/10000);  \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			ret = (combobuf[ri->combosref].member&flag) ? 10000 : 0); \
		} \
	} \

//comboclass macros

#define	GET_COMBOCLASS_VAR_INT(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combo_class_buf[combobuf[ri->combosref].type].member *10000); \
		} \
	} \

	#define	GET_COMBOCLASS_VAR_BYTE(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combo_class_buf[combobuf[ri->combosref].type].member *10000); \
		} \
	} \
	
	#define	GET_COMBOCLASS_VAR_DWORD(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combo_class_buf[combobuf[ri->combosref].type].member *10000); \
		} \
	} \
	
	#define GET_COMBOCLASS_VAR_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
				ret = -10000; \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combo_class_buf[combobuf[ri->combosref].type].member[indx] * 10000); \
			} \
	}

	#define GET_COMBOCLASS_BYTE_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
				ret = -10000; \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (combo_class_buf[combobuf[ri->combosref].type].member[indx] * 100000); \
			} \
	}
	
	#define GET_COMBOCLASS_FLAG(member, str, indexbound) \
	{ \
		long flag =  (value/10000);  \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			ret = (combo_class_buf[combobuf[ri->combosref].type].member&flag) ? 10000 : 0); \
		} \
	} \




//NEWCOMBO STRUCT
case COMBODTILE:		GET_COMBO_VAR_DWORD(tile, "Tile"); break;					//word
case COMBODFLIP:		GET_COMBO_VAR_BYTE(flip, "Flip"); break;					//char
case COMBODWALK:		GET_COMBO_VAR_BYTE(walk, "Walk"); break;					//char
case COMBODTYPE:		GET_COMBO_VAR_BYTE(type, "Type"); break;					//char
case COMBODCSET:		GET_COMBO_VAR_BYTE(csets, "CSet"); break;					//C
case COMBODFOO:			GET_COMBO_VAR_DWORD(foo, "Foo"); break;						//W
case COMBODFRAMES:		GET_COMBO_VAR_BYTE(frames, "Frames"); break;					//C
case COMBODNEXTD:		GET_COMBO_VAR_DWORD(speed, "NextData"); break;					//W
case COMBODNEXTC:		GET_COMBO_VAR_BYTE(nextcombo, "NextCSet"); break;				//C
case COMBODFLAG:		GET_COMBO_VAR_BYTE(nextcset, "Flag"); break;					//C
case COMBODSKIPANIM:		GET_COMBO_VAR_BYTE(skipanim, "SkipAnim"); break;				//C
case COMBODNEXTTIMER:		GET_COMBO_VAR_DWORD(nexttimer, "NextTimer"); break;				//W
case COMBODAKIMANIMY:		GET_COMBO_VAR_BYTE(skipanimy, "SkipAnimY"); break;				//C
case COMBODANIMFLAGS:		GET_COMBO_VAR_BYTE(animflags, "AnimFlags"); break;				//C
case COMBODEXPANSION:		GET_COMBO_BYTE_INDEX(expansion, "Expansion[]", 6);				//C , 6 INDICES
case COMBODATTRIBUTES: 		GET_COMBO_VAR_INDEX(attributes,	"Attributes[]", 4); 				//LONG, 4 INDICES, INDIVIDUAL VALUES
case COMBODUSRFLAGS:		GET_COMBO_VAR_INT(usrflags, "UserFlags"); break;				//LONG
case COMBODTRIGGERFLAGS:	GET_COMBO_VAR_INDEX(triggerflags, "TriggerFlags[]", 3);				//LONG 3 INDICES AS FLAGSETS
case COMBODTRIGGERLEVEL:	GET_COMBO_VAR_INT(triggerlevel, "TriggerLevel"); break;				//LONG

//COMBOCLASS STRUCT
//case COMBODNAME:		//CHAR[64], STRING
case COMBODBLOCKNPC:		GET_COMBOCLASS_VAR_BYTE(block_enemies, "BlockNPC"); break;			//C
case COMBODBLOCKHOLE:		GET_COMBOCLASS_VAR_BYTE(block_hole, "BlockHole"); break;			//C
case COMBODBLOCKTRIG:		GET_COMBOCLASS_VAR_BYTE(block_trigger,	"BlockTrigger"); break; 		//C
case COMBODBLOCKWEAPON:		GET_COMBOCLASS_BYTE_INDEX(block_weapon,	"BlockWeapon[]", 32); 			//C, 32 INDICES
case COMBODCONVXSPEED:		GET_COMBOCLASS_VAR_DWORD(conveyor_x_speed, "ConveyorSpeedX"); break;		//SHORT
case COMBODCONVYSPEED:		GET_COMBOCLASS_VAR_DWORD(conveyor_y_speed, "ConveyorSpeedY"); break;		//SHORT
case COMBODSPAWNNPC:		GET_COMBOCLASS_VAR_DWORD(create_enemy, "SpawnNPC"); break;			//W
case COMBODSPAWNNPCWHEN:	GET_COMBOCLASS_VAR_BYTE(create_enemy_when, "SpawnNPCWhen"); break;		//C
case COMBODSPAWNNPCCHANGE:	GET_COMBOCLASS_VAR_INT(create_enemy_change, "SpawnNPCChange"); break;		//LONG
case COMBODDIRCHANGETYPE:	GET_COMBOCLASS_VAR_BYTE(directional_change_type, "DirChange"); break;		//C
case COMBODDISTANCECHANGETILES:	GET_COMBOCLASS_VAR_INT(distance_change_tiles, "DistanceChangeTiles"); break; 	//LONG
case COMBODDIVEITEM:		GET_COMBOCLASS_VAR_DWORD(dive_item, "DiveItem"); break;				//SHORT
case COMBODDOCK:		GET_COMBOCLASS_VAR_BYTE(dock, "Dock"); break;					//C
case COMBODFAIRY:		GET_COMBOCLASS_VAR_BYTE(fairy, "Fairy"); break;					//C
case COMBODFFATTRCHANGE:	GET_COMBOCLASS_VAR_BYTE(ff_combo_attr_change, "FFCAttributeChange"); break;	//C
case COMBODFOORDECOTILE:	GET_COMBOCLASS_VAR_INT(foot_decorations_tile, "DecorationTile"); break;		//LONG
case COMBODFOORDECOTYPE:	GET_COMBOCLASS_VAR_BYTE(foot_decorations_type, "DecorationType"); break;	//C
case COMBODHOOKSHOTPOINT:	GET_COMBOCLASS_VAR_BYTE(hookshot_grab_point, "Hookshot"); break;		//C
case COMBODLADDERPASS:		GET_COMBOCLASS_VAR_BYTE(ladder_pass, "Ladder"); break;				//C
case COMBODLOCKBLOCK:		GET_COMBOCLASS_VAR_BYTE(lock_block_type, "LockBlock"); break;			//C
case COMBODLOCKBLOCKCHANGE:	GET_COMBOCLASS_VAR_INT(lock_block_change, "LockBlockChange"); break;		//LONG
case COMBODMAGICMIRROR:		GET_COMBOCLASS_VAR_BYTE(magic_mirror_type, "Mirror"); break;			//C
case COMBODMODHPAMOUNT:		GET_COMBOCLASS_VAR_DWORD(modify_hp_amount, "DamageAmount"); break;		//SHORT
case COMBODMODHPDELAY:		GET_COMBOCLASS_VAR_BYTE(modify_hp_delay, "DamageDelay"); break;			//C
case COMBODMODHPTYPE:		GET_COMBOCLASS_VAR_BYTE(modify_hp_type,	"DamageType"); break; 			//C
case COMBODNMODMPAMOUNT:	GET_COMBOCLASS_VAR_DWORD(modify_mp_amount, "MagicAmount"); break;		//SHORT
case COMBODMODMPDELAY:		GET_COMBOCLASS_VAR_BYTE(modify_mp_delay, "MagicDelay"); break;			//C
case COMBODMODMPTYPE:		GET_COMBOCLASS_VAR_BYTE(modify_mp_type,	"MagicType");				//C
case COMBODNOPUSHBLOCK:		GET_COMBOCLASS_VAR_BYTE(no_push_blocks, "NoPushBlocks"); break;			//C
case COMBODOVERHEAD:		GET_COMBOCLASS_VAR_BYTE(overhead, "Overhead"); break;				//C
case COMBODPLACENPC:		GET_COMBOCLASS_VAR_BYTE(place_enemy, "PlaceNPC"); break;			//C
case COMBODPUSHDIR:		GET_COMBOCLASS_VAR_BYTE(push_direction,	"PushDir"); break; 			//C
case COMBODPUSHWAIT:		GET_COMBOCLASS_VAR_BYTE(push_wait, "PushDelay"); break;				//C
case COMBODPUSHHEAVY:		GET_COMBOCLASS_VAR_BYTE(push_weight, "PushHeavy");				//C
case COMBODPUSHED:		GET_COMBOCLASS_VAR_BYTE(pushed, "Pushed"); break;				//C
case COMBODRAFT:		GET_COMBOCLASS_VAR_BYTE(raft, "Raft"); break;					//C
case COMBODRESETROOM:		GET_COMBOCLASS_VAR_BYTE(reset_room, "ResetRoom"); break;			//C
case COMBODSAVEPOINTTYPE:	GET_COMBOCLASS_VAR_BYTE(save_point_type, "SavePoint"); break;			//C
case COMBODSCREENFREEZETYPE:	GET_COMBOCLASS_VAR_BYTE(screen_freeze_type, "FreezeScreen"); break;		//C
case COMBODSECRETCOMBO:		GET_COMBOCLASS_VAR_BYTE(secret_combo, "SecretCombo"); break;			//C
case COMBODSINGULAR:		GET_COMBOCLASS_VAR_BYTE(singular, "Singular"); break;				//C
case COMBODSLOWWALK:		GET_COMBOCLASS_VAR_BYTE(slow_movement, "SlowWalk"); break;			//C
case COMBODSTATUETYPE:		GET_COMBOCLASS_VAR_BYTE(statue_type, "Statue"); break;				//C
case COMBODSTEPTYPE:		GET_COMBOCLASS_VAR_BYTE(step_type, "Step"); break;				//C
case COMBODSTEPCHANGEINTO:	GET_COMBOCLASS_VAR_INT(step_change_to, "StepChange"); break;			//LONG
case COMBODSTRIKEWEAPONS:	GET_COMBOCLASS_BYTE_INDEX(strike_weapons, "Strike[]", 32);			//BYTE, 32 INDICES. 
case COMBODSTRIKEREMNANTS:	GET_COMBOCLASS_VAR_INT(strike_remnants,	"StrikeRemnants"); break;		//LONG
case COMBODSTRIKEREMNANTSTYPE:	GET_COMBOCLASS_VAR_BYTE(strike_remnants_type, "StrikeRemnantsType"); break;	//C
case COMBODSTRIKECHANGE:	GET_COMBOCLASS_VAR_INT(strike_change, "StrikeChange"); break;			//LONG
case COMBODSTRIKEITEM:		GET_COMBOCLASS_VAR_DWORD(strike_item, "StrikeItem"); break;			//SHORT
case COMBODTOUCHITEM:		GET_COMBOCLASS_VAR_DWORD(touch_item, "TouchItem"); break;			//SHORT
case COMBODTOUCHSTAIRS:		GET_COMBOCLASS_VAR_BYTE(touch_stairs, "TouchStairs"); break;			//C
case COMBODTRIGGERTYPE:		GET_COMBOCLASS_VAR_BYTE(trigger_type, "TriggerType"); break;			//C
case COMBODTRIGGERSENS:		GET_COMBOCLASS_VAR_BYTE(trigger_sensitive, "TriggerSensitivity"); break;	//C
case COMBODWARPTYPE:		GET_COMBOCLASS_VAR_BYTE(warp_type, "Warp"); break;				//C
case COMBODWARPSENS:		GET_COMBOCLASS_VAR_BYTE(warp_sensitive,	"WarpSensitivity"); break; 		//C
case COMBODWARPDIRECT:		GET_COMBOCLASS_VAR_BYTE(warp_direct, "WarpDirect"); break;			//C
case COMBODWARPLOCATION:	GET_COMBOCLASS_VAR_BYTE(warp_location, "WarpLocation"); break;			//C
case COMBODWATER:		GET_COMBOCLASS_VAR_BYTE(water, "Water"); break;					//C
case COMBODWHISTLE:		GET_COMBOCLASS_VAR_BYTE(whistle, "Whistle"); break;				//C
case COMBODWINGAME:		GET_COMBOCLASS_VAR_BYTE(win_game, "WinGame"); break; 				//C
case COMBODBLOCKWPNLEVEL:	GET_COMBOCLASS_VAR_BYTE(block_weapon_lvl, "BlockWeaponLevel"); break;		//C


	
///----------------------------------------------------------------------------------------------------//
//npcdata nd-> variables
    
//npcdata nd->member variable
	#define	GET_NPCDATA_VAR_INT32(member, str) \
	{ \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (guysbuf[ri->npcdataref].member *10000); \
		} \
	} \

	#define	GET_NPCDATA_VAR_BYTE(member, str) \
	{ \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (guysbuf[ri->npcdataref].member *10000); \
		} \
	} \
	
	#define	GET_NPCDATA_VAR_INT16(member, str) \
	{ \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (guysbuf[ri->npcdataref].member *10000); \
		} \
	} \
	
	#define GET_NPCDATA_VAR_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
				ret = -10000; \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to npcdata->%s: %d\n", indx, str); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (guysbuf[ri->npcdataref].member[indx] * 10000); \
			} \
	}

	#define GET_NPCDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
				ret = -10000; \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to npcdata->%s: %d\n", indx, str); \
				ret = -10000; \
			} \
			else \
			{ \
				ret = (guysbuf[ri->npcdataref].member[indx] * 100000); \
			} \
	}
	
	#define GET_NPCDATA_FLAG(member, str, indexbound) \
	{ \
		long flag =  (value/10000);  \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
		} \
		else \
		{ \
			ret = (guysbuf[ID].member&flag) ? 10000 : 0); \
		} \
	} \
    
case NPCDATATILE: GET_NPCDATA_VAR_BYTE(tile, "Tile"); break;
case NPCDATAWIDTH: GET_NPCDATA_VAR_BYTE(width, "Width"); break;
case NPCDATAHEIGHT: GET_NPCDATA_VAR_BYTE(height, "Height"); break;
case NPCDATAFLAGS: GET_NPCDATA_VAR_INT16(flags, "Flags"); break; //16 b its
case NPCDATAFLAGS2: GET_NPCDATA_VAR_INT16(flags2, "Flags2"); break; //16 bits
case NPCDATASTILE: GET_NPCDATA_VAR_BYTE(s_tile, "STile"); break;
case NPCDATASWIDTH: GET_NPCDATA_VAR_BYTE(s_width, "SWidth"); break;
case NPCDATASHEIGHT: GET_NPCDATA_VAR_BYTE(s_height, "SHeight"); break;
case NPCDATAETILE: GET_NPCDATA_VAR_INT32(e_tile, "ExTile"); break;
case NPCDATAEWIDTH: GET_NPCDATA_VAR_BYTE(e_width, "ExWidth"); break;
case NPCDATAEHEIGHT: GET_NPCDATA_VAR_BYTE(e_height, "ExHeight"); break;
case NPCDATAHP: GET_NPCDATA_VAR_INT16(hp, "HP"); break;
case NPCDATAFAMILY: GET_NPCDATA_VAR_INT16(family, "Family"); break;
case NPCDATACSET: GET_NPCDATA_VAR_INT16(cset, "CSet"); break;
case NPCDATAANIM: GET_NPCDATA_VAR_INT16(anim, "Anim"); break;
case NPCDATAEANIM: GET_NPCDATA_VAR_INT16(e_anim, "ExAnim"); break;
case NPCDATAFRAMERATE: GET_NPCDATA_VAR_INT16(frate, "Framerate"); break;
case NPCDATAEFRAMERATE: GET_NPCDATA_VAR_INT16(e_frate, "ExFramerate"); break;
case NPCDATATOUCHDAMAGE: GET_NPCDATA_VAR_INT16(dp, "TouchDamage"); break;
case NPCDATAWEAPONDAMAGE: GET_NPCDATA_VAR_INT16(wdp, "WeaponDamage"); break;
case NPCDATAWEAPON: GET_NPCDATA_VAR_INT16(weapon, "Weapon"); break;
case NPCDATARANDOM: GET_NPCDATA_VAR_INT16(rate, "Random"); break;
case NPCDATAHALT: GET_NPCDATA_VAR_INT16(hrate, "Haltrate"); break;
case NPCDATASTEP: GET_NPCDATA_VAR_INT16(step, "Step"); break;
case NPCDATAHOMING: GET_NPCDATA_VAR_INT16(homing, "Homing"); break;
case NPCDATAHUNGER: GET_NPCDATA_VAR_INT16(grumble, "Hunger"); break;
case NPCDATADROPSET: GET_NPCDATA_VAR_INT16(item_set, "Dropset"); break;
case NPCDATABGSFX: GET_NPCDATA_VAR_INT16(bgsfx, "BGSFX"); break;
case NPCDATADEATHSFX: GET_NPCDATA_VAR_BYTE(deadsfx, "DeathSFX"); break;
case NPCDATAHITSFX: GET_NPCDATA_VAR_BYTE(hitsfx, "HitSFX"); break;
case NPCDATAXOFS: GET_NPCDATA_VAR_INT32(xofs, "DrawXOffset"); break;
case NPCDATAYOFS: GET_NPCDATA_VAR_INT32(yofs, "DrawYOffset"); break;
case NPCDATAZOFS: GET_NPCDATA_VAR_INT32(zofs, "DrawZOffset"); break;
case NPCDATAHXOFS: GET_NPCDATA_VAR_INT32(hxofs, "HitXOffset"); break;
case NPCDATAHYOFS: GET_NPCDATA_VAR_INT32(hyofs, "HitYOffset"); break;
case NPCDATAHITWIDTH: GET_NPCDATA_VAR_INT32(hxsz, "HitWidth"); break;
case NPCDATAHITHEIGHT: GET_NPCDATA_VAR_INT32(hysz, "HitHeight"); break;
case NPCDATAHITZ: GET_NPCDATA_VAR_INT32(hzsz, "HitZHeight"); break;
case NPCDATATILEWIDTH: GET_NPCDATA_VAR_INT32(txsz, "TileWidth"); break;
case NPCDATATILEHEIGHT: GET_NPCDATA_VAR_INT32(tysz, "TileHeight"); break;
case NPCDATAWPNSPRITE: GET_NPCDATA_VAR_INT32(wpnsprite, "WeaponSprite"); break;
case NPCDATADEFENSE: GET_NPCDATA_VAR_INDEX(defense, "Defense", 42); break;
case NPCDATASIZEFLAG: GET_NPCDATA_VAR_INT32(SIZEflags, "SizeFlags"); break;

case NPCDATAFROZENTILE: GET_NPCDATA_VAR_INT32(frozentile, "FrozenTile"); break;
case NPCDATAFROZENCSET: GET_NPCDATA_VAR_INT32(frozencset, "FrozenCSet"); break;


case NPCDATAATTRIBUTE: 
{
	int indx = ri->d[0] / 10000; 
	if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) 
	{
		Z_scripterrlog("Invalid Sprite ID passed to npcdata->Attributes[]: %d\n", (ri->npcdataref*10000)); 
		ret = -10000;
	}
	else if ( indx < 0 || indx > MAX_NPC_ATRIBUTES )
	{ 
		Z_scripterrlog("Invalid Array Index passed to npcdata->Attributes[]: %d\n", (ri->npcdataref*10000)); 
		ret = -10000; 
	} 
	else 
	{ 
		switch(indx)
		{
			case 0: ret = (guysbuf[ri->npcdataref].misc1 * 10000); break;
			case 1: ret = (guysbuf[ri->npcdataref].misc2 * 10000); break;
			case 2: ret = (guysbuf[ri->npcdataref].misc3 * 10000); break;
			case 3: ret = (guysbuf[ri->npcdataref].misc4 * 10000); break;
			case 4: ret = (guysbuf[ri->npcdataref].misc5 * 10000); break;
			case 5: ret = (guysbuf[ri->npcdataref].misc6 * 10000); break;
			case 6: ret = (guysbuf[ri->npcdataref].misc7 * 10000); break;
			case 7: ret = (guysbuf[ri->npcdataref].misc8 * 10000); break;
			case 8: ret = (guysbuf[ri->npcdataref].misc9 * 10000); break;
			case 9: ret = (guysbuf[ri->npcdataref].misc10 * 10000); break;
			case 10: ret = (guysbuf[ri->npcdataref].misc11 * 10000); break;
			case 11: ret = (guysbuf[ri->npcdataref].misc12 * 10000); break;
			case 12: ret = (guysbuf[ri->npcdataref].misc13 * 10000); break;
			case 13: ret = (guysbuf[ri->npcdataref].misc14 * 10000); break;
			case 14: ret = (guysbuf[ri->npcdataref].misc15 * 10000); break;
			case 15: ret = (guysbuf[ri->npcdataref].misc16 * 10000); break;
			case 16: ret = (guysbuf[ri->npcdataref].misc17 * 10000); break;
			case 17: ret = (guysbuf[ri->npcdataref].misc18* 10000); break;
			case 18: ret = (guysbuf[ri->npcdataref].misc19 * 10000); break;
			case 19: ret = (guysbuf[ri->npcdataref].misc20 * 10000); break;
			case 20: ret = (guysbuf[ri->npcdataref].misc21 * 10000); break;
			case 21: ret = (guysbuf[ri->npcdataref].misc22 * 10000); break;
			case 22: ret = (guysbuf[ri->npcdataref].misc23 * 10000); break;
			case 23: ret = (guysbuf[ri->npcdataref].misc24 * 10000); break;
			case 24: ret = (guysbuf[ri->npcdataref].misc25 * 10000); break;
			case 25: ret = (guysbuf[ri->npcdataref].misc26 * 10000); break;
			case 26: ret = (guysbuf[ri->npcdataref].misc27 * 10000); break;
			case 27: ret = (guysbuf[ri->npcdataref].misc28 * 10000); break;
			case 28: ret = (guysbuf[ri->npcdataref].misc29 * 10000); break;
			case 29: ret = (guysbuf[ri->npcdataref].misc30 * 10000); break;
			case 30: ret = (guysbuf[ri->npcdataref].misc31 * 10000); break;
			case 31: ret = (guysbuf[ri->npcdataref].misc32 * 10000); break;
			
			
			
			default: 
			{
				Z_scripterrlog("Invalid Array Index passed to npcdata->Attributes[]: %d\n", (ri->npcdataref*10000)); 
				ret = -10000;
				break;
			}
		}
			
	} 
	break;
}

case NPCDATASHIELD:
{
	int indx = ri->d[0] / 10000; 
	if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) 
	{ 
		Z_scripterrlog("Invalid NPC ID passed to npcdata->Shield[]: %d\n", (ri->npcdataref*10000)); 
		ret = -10000; 
	} 
	else 
	{ 
		switch(indx)
		{
			case 0:
			{
				ret = ((guysbuf[ri->npcdataref].flags&inv_front) ? 10000 : 0);
				break;
			}
			case 1:
			{
				ret = ((guysbuf[ri->npcdataref].flags&inv_left) ? 10000 : 0);
				break;
			}
			case 2:
			{
				ret = ((guysbuf[ri->npcdataref].flags&inv_right) ? 10000 : 0);
				break;
			}
			case 3:
			{
				ret = ((guysbuf[ri->npcdataref].flags&inv_back) ? 10000 : 0);
				break;
			}
			case 4:
			{
				ret = ((guysbuf[ri->npcdataref].flags&guy_bkshield) ? 10000 : 0);
				break;
			}
			default:
			{
				Z_scripterrlog("Invalid Array Index passed to npcdata->Shield[]: %d\n", indx); 
				ret = -10000; 
				break;
			}
		}
	} 
}
    
    
///----------------------------------------------------------------------------------------------------//
//Audio Variables

case AUDIOVOLUME:
{
	int indx = ri->d[0] / 10000;
	int r;
	switch(indx)
	{
		
		case 0: //midi volume
		{
			r = FFScript::do_getMIDI_volume() * 10000;
			break;
		}
		case 1: //digi volume
		{
			r = FFScript::do_getDIGI_volume() * 10000;
			break;
		}
		case 2: //emh music volume
		{
			r = FFScript::do_getMusic_volume() * 10000;
			break;
		}
		case 3: //sfx volume
		{
			r = FFScript::do_getSFX_volume() * 10000;
			break;
		}
		default:
		{
			Z_scripterrlog("Attempted to access an invalid index of Audio->Volume[]", indx); 
			ret = -10000;
			break;
		}
	}
	
	ret = r; break;
}

case AUDIOPAN:
{
	ret = FFScript::do_getSFX_pan() * 10000;
	break;
}


///----------------------------------------------------------------------------------------------------//
//Misc./Internal
    case REFFFC:
        ret = ri->ffcref * 10000;
        break;
        
    case REFITEM:
        ret = ri->itemref;
        break;
        
    case REFITEMCLASS:
        ret = ri->idata;
        break;
        
    case REFLWPN:
        ret = ri->lwpn;
        break;
        
    case REFEWPN:
        ret = ri->ewpn;
        break;
        
    case REFNPC:
        ret = ri->guyref;
        break;
    
    case REFMAPDATA: ret = ri->mapsref; break;
    case REFSCREENDATA: ret = ri->screenref; break;
    case REFCOMBODATA: ret = ri->combosref; break;
    case REFSPRITEDATA: ret = ri->spritesref; break;
    case REFBITMAP: ret = ri->bitmapref; break;
    case REFNPCCLASS: ret = ri->npcdataref; break;
    
    
    case REFDMAPDATA: ret = ri->dmapsref; break;
    case REFSHOPDATA: ret = ri->shopsref; break;
    case REFMSGDATA: ret = ri->zmsgref; break;
    case REFUNTYPED: ret = ri->untypedref; break;
    
    case REFDROPS: ret = ri->dropsetref; break;
    case REFPONDS: ret = ri->pondref; break;
    case REFWARPRINGS: ret = ri->warpringref; break;
    case REFDOORS: ret = ri->doorsref; break;
    case REFUICOLOURS: ret = ri->zcoloursref; break;
    case REFRGB: ret = ri->rgbref; break;
    case REFPALETTE: ret = ri->paletteref; break;
    case REFTUNES: ret = ri->tunesref; break;
    case REFPALCYCLE: ret = ri->palcycleref; break;
    case REFGAMEDATA: ret = ri->gamedataref; break;
    case REFCHEATS: ret = ri->cheatsref; break;
   
        
    case SP:
        ret = ri->sp * 10000;
        break;
        
    case SCRIPTRAM:
    case GLOBALRAM:
        ret = ArrayH::getElement(ri->d[0] / 10000, ri->d[1] / 10000);
        break;
        
    case SCRIPTRAMD:
    case GLOBALRAMD:
        ret = ArrayH::getElement(ri->d[0] / 10000, 0);
        break;
        
    case GDD://Doesn't work like this =(
        ret = game->global_d[ri->d[0] / 10000];
        break;
        
///----------------------------------------------------------------------------------------------------//
//Most of this is deprecated I believe ~Joe123
    default:
    {
        if(arg >= D(0) && arg <= D(7))			ret = ri->d[arg - D(0)];
        else if(arg >= A(0) && arg <= A(1))		ret = ri->a[arg - A(0)];
        else if(arg >= GD(0) && arg <= GD(255))	ret = game->global_d[arg - GD(0)];
        
        break;
    }
    }
    
    return ret;
}

//Setter Instructions


void set_register(const long arg, const long value)
{
	//Macros
	
	#define	SET_SPRITEDATA_VAR_INT(member, str) \
	{ \
		if(ri->spritesref < 0 || ri->spritesref > (MAXWPNS-1) ) \
		{ \
			Z_scripterrlog("Invalid Sprite ID passed to spritedata->%s: %d\n", (ri->spritesref*10000), str); \
		} \
		else \
		{ \
			wpnsbuf[ri->spritesref].member = vbound((value / 10000),0,214747); \
		} \
	} \

	#define	SET_SPRITEDATA_VAR_BYTE(member, str) \
	{ \
		if(ri->spritesref < 0 || ri->spritesref > (MAXWPNS-1) ) \
		{ \
			Z_scripterrlog("Invalid Sprite ID passed to spritedata->%s: %d\n", (ri->spritesref*10000), str); \
		} \
		else \
		{ \
			wpnsbuf[ri->spritesref].member = vbound((value / 10000),0,255); \
		} \
	} \
	
    switch(arg)
    {
	    
	    //debug ri->d[]
    case DEBUGD:
    {
	int a = vbound((ri->d[0] / 10000), 0, 255);
	ri->d[a] = value/10000;
	break;
    }    
    
///----------------------------------------------------------------------------------------------------//
//FFC Variables
    case DATA:
        tmpscr->ffdata[ri->ffcref] = vbound(value/10000,0,MAXCOMBOS-1);
        break;
        
     case CHANGEFFSCRIPTR:
        FFScript::do_changeffcscript(false);
        break;
    
    case CHANGEFFSCRIPTV:
        FFScript::do_changeffcscript(true);
        break;
    
   case FFSCRIPT:
        for(long i = 1; i < MAX_ZCARRAY_SIZE; i++)
        {
            if(arrayOwner[i]==ri->ffcref)
                deallocateArray(i);
        }
        
        tmpscr->ffscript[ri->ffcref] = vbound(value/10000, 0, NUMSCRIPTFFC-1);
        
        for(int i=0; i<16; i++)
            ffmisc[ri->ffcref][i] = 0;
            
        for(int i=0; i<2; i++)
            tmpscr->inita[ri->ffcref][i] = 0;
            
        for(int i=0; i<8; i++)
            tmpscr->initd[ri->ffcref][i] = 0;
            
        ffcScriptData[ri->ffcref].Clear();
        tmpscr->initialized[ri->ffcref] = false;
        break;
        
        
    case FCSET:
        tmpscr->ffcset[ri->ffcref] = (value/10000)&15;
        break;
        
    case DELAY:
        tmpscr->ffdelay[ri->ffcref] = value/10000;
        break;
        
    case FX:
        tmpscr->ffx[ri->ffcref] = value;
        break;
        
    case FY:
        tmpscr->ffy[ri->ffcref]=value;
        break;
        
    case XD:
        tmpscr->ffxdelta[ri->ffcref]=value;
        break;
        
    case YD:
        tmpscr->ffydelta[ri->ffcref]=value;
        break;
    
    case FFCID:
        ri->ffcref = vbound((value-10000)/10000, 0, 31);
        break;
        
    case XD2:
        tmpscr->ffxdelta2[ri->ffcref]=value;
        break;
        
    case YD2:
        tmpscr->ffydelta2[ri->ffcref]=value;
        break;
        
    case FFFLAGSD:
        value ? tmpscr->ffflags[ri->ffcref] |=   1<<((ri->d[0])/10000)
                : tmpscr->ffflags[ri->ffcref] &= ~(1<<((ri->d[0])/10000));
        break;
        
    case FFCWIDTH:
        tmpscr->ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref] & ~63) | (((value/10000)-1)&63);
        break;
        
    case FFCHEIGHT:
        tmpscr->ffheight[ri->ffcref]= (tmpscr->ffheight[ri->ffcref] & ~63) | (((value/10000)-1)&63);
        break;
        
    case FFTWIDTH:
        tmpscr->ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref]&63) | ((((value/10000)-1)&3)<<6);
        break;
        
    case FFTHEIGHT:
        tmpscr->ffheight[ri->ffcref]=(tmpscr->ffheight[ri->ffcref]&63) | ((((value/10000)-1)&3)<<6);
        break;
        
    case FFLINK:
        (tmpscr->fflink[ri->ffcref])=vbound(value/10000, 0, 32); // Allow "ffc->Link = 0" to unlink ffc.
	//0 is none, setting this before made it impssible to clear it. -Z
        break;
        
    case FFMISCD:
    {
        int a = vbound(ri->d[0]/10000,0,15);
        ffmisc[ri->ffcref][a]=value;
        break;
    }
    
    case FFINITDD:
        (tmpscr->initd[ri->ffcref][vbound(ri->d[0]/10000,0,7)])=value;
        break;
    
        
        
///----------------------------------------------------------------------------------------------------//
//Link's Variables
    case LINKX:
        Link.setX(value/10000);
        break;
        
    case LINKY:
        Link.setY(value/10000);
        break;
        
    case LINKZ:
        Link.setZ(value/10000);
        break;
        
    case LINKJUMP:
        Link.setFall(fix((-value * (100.0)) / 10000.0));
        break;
        
    case LINKDIR:
    {
        //Link->setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
        if(Link.getAction() == attacking) Link.dir = (value/10000);
        else Link.setDir(value/10000);
        
        break;
    }
    
    case LINKHITDIR:
        Link.setHitDir(value / 10000);
        break;
        
    case LINKHP:
        game->set_life(zc_max(0, zc_min(value/10000,game->get_maxlife())));
        break;
        
    case LINKMP:
        game->set_magic(zc_max(0, zc_min(value/10000,game->get_maxmagic())));
        break;
        
    case LINKMAXHP:
        game->set_maxlife(value/10000);
        break;
        
    case LINKMAXMP:
        game->set_maxmagic(value/10000);
        break;
        
    case LINKACTION:
    {
	int act = value / 10000;
	if ( act < 25 ) {
		Link.setAction((actiontype)(act));
	}
	else FFCore.setLinkAction(act); //Protect from writing illegal actions to Link's raw variable. 
	//in the future, we can move all scripted actions that are not possible
	//to set in ZC into this mechanic. -Z
        break;
    }
        
    case LINKHELD:
        Link.setHeldItem(vbound(value/10000,0,MAXITEMS-1));
        break;
        
    case LINKITEMD:
        {
            int itemID=vbound(ri->d[0]/10000,0,MAXITEMS-1);
            
            // If the Cane of Byrna is being removed, cancel its effect.
            if(value==0 && itemID==current_item_id(itype_cbyrna))
                stopCaneOfByrna();
            
            bool settrue = ( value != 0 );
		    
	    //Sanity check to prevent setting the item if the value would be the same. -Z
	    if ( game->item[itemID] != settrue ) game->set_item(itemID,(value != 0));
                    
            //resetItems(game); - Is this really necessary? ~Joe123
            if((get_bit(quest_rules,qr_OVERWORLDTUNIC) != 0) || (currscr<128 || dlevel)) 
	    {
		    ringcolor(false);
		    //refreshpal=true;
	    }
        }
        break;
        
         case LINKEQUIP:
          {
        
		  //int seta = (value/10000) >> 8; int setb = value/10000) & 0xFF;
        	int setb = ((value/10000)&0xFF00)>>8, seta = (value/10000)&0xFF;
        	if(seta && get_bit(quest_rules,qr_SELECTAWPN) && game->item[seta]){
			Awpn = value/10000;
        	}
        	if(setb && game->item[setb]){
			Bwpn = value/10000;
        	}
          }
         break;
	  
	  case SETITEMSLOT:
	{
		//ri->d[1] = 1st arg
		//ri->d[0] = 2nd arg
		//value = third arg
		//int item, int slot, int force
		int itm = ri->d[0]/10000;
		
		int slot = ri->d[1]/10000;
		int force = ri->d[2]/10000;
		
	    Z_scripterrlog("SetItemSlot rid->[0] is (%i), trying to use for '%s'\n", itm, "itm");
	    Z_scripterrlog("SetItemSlot rid->[1] is (%i), trying to use for '%s'\n", slot, "slot");
		Z_scripterrlog("SetItemSlot rid->[2] is (%i), trying to use for '%s'\n", force, "force");
		
		//If we add more item buttons, slot should be an int
		//and force shuld be an int
		
		/*
			For zScript, 
				const int ITM_REQUIRE_NONE = 0
				const int ITM_REQUIRE_INVENTORY = 1
				const int ITM_REQUIRE_A_SLOT_RULE = 2
				//Combine as flags
		
		
		*/
		if ( force == 0 ) {
			if ( slot == 1 ) {
				Awpn = itm;
			}
			else Bwpn = itm;
		}
		if ( force == 1 ) {
			if(slot == 1 && game->item[itm]){
				Awpn = itm;
			}
			else { 
				if ( game->item[itm] ) Bwpn = itm;
			}
		}

		if ( force == 2 ) {
			if(slot == 1 && get_bit(quest_rules,qr_SELECTAWPN) ){
				Awpn = itm;
			}
			else { 
				Bwpn = itm;
			}
		}
		
		if ( force == 3 ) { //Flag ITM_REQUIRE_INVENTORY + ITM_REQUIRE_SLOT_A_RULE
			if(slot == 1 && get_bit(quest_rules,qr_SELECTAWPN) && game->item[itm]){
				Awpn = itm;
			}
			else { 
				if ( game->item[itm] ) Bwpn = itm;
			}
		}
	}
	break;
	  
    case LINKINVIS:
        Link.setDontDraw((value ? 2 : 0));
        break;
        
    case LINKINVINC:
        Link.scriptcoldet=(value/10000);
        break;
        
    case LINKSWORDJINX:
        Link.setSwordClk(value/10000);
        break;
        
    case LINKITEMJINX:
        Link.setItemClk(value/10000);
        break;
        
    case LINKDRUNK:
        Link.setDrunkClock(value/10000);
        break;
        
    case LINKMISCD:
        Link.miscellaneous[vbound(ri->d[0]/10000,0,31)] = value; 
        break;
    
    case LINKHITBY:
    {
	int indx = ri->d[0]/10000;
	switch(indx)
	{
		//screen index objects
		case 0:
		case 1:
		case 2:
		case 3:
		{
			Link.sethitLinkUID(indx, vbound((value/10000), 0, 255)); //Why the Flidd did I vbound this? UIDs are LONGs, with a starting value of 0.0001. Why did I allow it, in fact? -Z
			break;
		}
		//UIDs
		case 4:
		case 5:
		case 6:
		case 7:
		{
			Link.sethitLinkUID(indx, vbound((value), 0, 255)); //Why the Flidd did I vbound this? UIDs are LONGs, with a starting value of 0.0001. Why did I allow it, in fact? -Z
			break;
		}
		default: { al_trace("Invalid index passed to Link->HitBy[%d] /n", indx); break; }
	}
	break;
    }
    
    case LINKDEFENCE:
	Link.set_defence(vbound(ri->d[0]/10000,0,255), ((char)vbound((value/10000), 0, 255)));
        break;
        
    case LINKHXOFS:
        (Link.hxofs)=(fix)(value/10000);
        break;
        
    case LINKHYOFS:
        (Link.hyofs)=(fix)(value/10000);
        break;
        
    case LINKXOFS:
        (Link.xofs)=(fix)(value/10000);
        break;
        
    case LINKYOFS:
        (Link.yofs)=(fix)(value/10000)+playing_field_offset;
        break;
        
    case LINKZOFS:
        (Link.zofs)=(fix)(value/10000);
        break;
        
    case LINKHXSZ:
        (Link.hxsz)=(fix)(value/10000);
        break;
        
    case LINKHYSZ:
        (Link.hysz)=(fix)(value/10000);
        break;
        
    case LINKHZSZ:
        (Link.hzsz)=(fix)(value/10000);
        break;
        
    case LINKTXSZ:
        (Link.txsz)=(fix)(value/10000);
        break;
        
    case LINKTYSZ:
        (Link.tysz)=(fix)(value/10000);
        break;
        
    case LINKTILE:
        (Link.tile)=(fix)(value/10000);
        break;
        
    case LINKFLIP:
        (Link.flip)=(fix)(value/10000);
        break;
    
    
    
    case LINKINVFRAME:
	Link.setHClk( (int)vbound((value/10000), 0, 214747) );
	break;
    
    case LINKCANFLICKER:
	Link.setCanLinkFlicker((value/10000)?1:0);
	break;
    
    case LINKHURTSFX:
	Link.setHurtSFX( (int)vbound((value/10000), 0, 255) );
	break;
        
    /*
     case LINKITEMB:
	    //Link->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
    
	//Link->directItem = vbound((value/10000),0,(MAXITEMS-1));
	//Link->directItemB = Link->directItem;
	//Bwpn = vbound((value/10000),0,(MAXITEMS-1));
    
	Link.setDirectItem((int)vbound((value/10000),0,(MAXITEMS-1)));
	Link.setDirectItemB(Link->getDirectItem());
	Bwpn = (int)vbound((value/10000),0,(MAXITEMS-1));
	game->bwpn = (int)vbound((value/10000),0,(MAXITEMS-1));
	break;
    
    
    case LINKITEMA:
	    //Link->setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
	Link.setDirectItem((int)vbound((value/10000),0,(MAXITEMS-1)));
	Link.setDirectItemA(Link->getDirectItem());
	Awpn = (int)vbound((value/10000),0,(MAXITEMS-1));
	game->awpn = (int)vbound((value/10000),0,(MAXITEMS-1));
	break;
*/

      case LINKEATEN:
	Link.setEaten(value/10000);
	break;
      case LINKSTUN:
	Link.setStunClock(value/10000);
	break;
      case LINKSCRIPTTILE:
	script_link_sprite=(value/10000);
	break;
      case LINKSCRIPFLIP:
	script_link_flip=(value/10000);
	break;
    
      case GAMESETA:
	{
		//int state   = (ri->d[1]/10000);
		//int extend = (ri->d[1]/10000);
		//int dir = (ri->d[0]/10000);
		Z_message("Trying to force-set the A-button item().\n");
		Link.setAButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
	}
	break;
	
	case GAMESETB:
	{
		//int state   = (ri->d[1]/10000);
		//int extend = (ri->d[1]/10000);
		//int dir = (ri->d[0]/10000);
		Z_message("Trying to force-set the A-button item().\n");
		Link.setBButtonItem(vbound((value/10000),0,(MAXITEMS-1)));
	}
	break;
	
	//Set Link Diagonal
        case LINKDIAG:
	    Link.setDiagMove((value/10000)?1:0);
	break;
    
    //Set Link Big Hitbox
        case LINKBIGHITBOX:
	    Link.setBigHitbox((value/10000)?1:0);
	break;
///----------------------------------------------------------------------------------------------------//
//Input States
    case INPUTSTART:
        control_state[6]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTMAP:
        control_state[9]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTUP:
        control_state[0]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTDOWN:
        control_state[1]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTLEFT:
        control_state[2]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTRIGHT:
        control_state[3]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTA:
        control_state[4]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTB:
        control_state[5]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTL:
        control_state[7]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTR:
        control_state[8]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTEX1:
        control_state[10]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTEX2:
        control_state[11]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTEX3:
        control_state[12]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTEX4:
        control_state[13]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTAXISUP:
        control_state[14]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTAXISDOWN:
        control_state[15]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTAXISLEFT:
        control_state[16]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTAXISRIGHT:
        control_state[17]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSSTART:
        button_press[6]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSMAP:
        button_press[9]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSUP:
        button_press[0]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSDOWN:
        button_press[1]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSLEFT:
        button_press[2]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSRIGHT:
        button_press[3]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSA:
        button_press[4]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSB:
        button_press[5]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSL:
        button_press[7]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSR:
        button_press[8]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSEX1:
        button_press[10]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSEX2:
        button_press[11]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSEX3:
        button_press[12]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSEX4:
        button_press[13]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSAXISUP:
        button_press[14]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSAXISDOWN:
        button_press[15]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSAXISLEFT:
        button_press[16]=((value/10000)!=0)?true:false;
        break;
        
    case INPUTPRESSAXISRIGHT:
        button_press[17]=((value/10000)!=0)?true:false;
        break;
        
     case INPUTMOUSEX:
    {
        int leftOffset=(resx/2)-(128*screen_scale);
        position_mouse((value/10000)*screen_scale+leftOffset, gui_mouse_y());
        break;
    }
    
    case INPUTMOUSEY:
    {
        int topOffset=(resy/2)-((112-playing_field_offset)*screen_scale);
        position_mouse(gui_mouse_x(), (value/10000)*screen_scale+topOffset);
        break;
    }
    
    case INPUTMOUSEZ:
        position_mouse_z(value/10000);
        break;
    
    case BUTTONPRESS:
		// DUkey, DDkey, DLkey, DRkey, Akey, Bkey, Skey, Lkey, Rkey, Pkey, Exkey1, Exkey2, Exkey3, Exkey4 };
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		button_press[button]=((value/10000)!=0)?true:false;
		
	}
	break;

	case BUTTONINPUT:
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		control_state[button]=((value/10000)!=0)?true:false;
		
	}
	break;

	case BUTTONHELD:
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		button_hold[button]=((value/10000)!=0)?true:false;
	}
	break;

	case KEYPRESS:
	{	//Game->KeyPressed[], read-only
		//if ( !keypressed() ) break; //Don;t return values set by setting Link->Input/Press
		//hmm...no, this won;t return properly for modifier keys. 
		int keyid = ri->d[0]/10000;
		//key = vbound(key,0,n);
		key[keyid]=((value/10000)!=0)?true:false; //It isn't possible to set keys true, because polling occurs before they are set?
		//but they *can* be set false; ??? -Z
	}
	break;
	
	case MOUSEARR:
	{	
		int indx = (ri->d[0]/10000);
		switch (indx)
		{
			case 0: //MouseX
			{
				int leftOffset=(resx/2)-(128*screen_scale);
				position_mouse((value/10000)*screen_scale+leftOffset, gui_mouse_y());
				break;	
			}
			case 1: //MouseY
			{
				int topOffset=(resy/2)-((112-playing_field_offset)*screen_scale);
				position_mouse(gui_mouse_x(), (value/10000)*screen_scale+topOffset);
				break;
				
			}
			case 2: //MouseZ
			{
				position_mouse_z(value/10000);
				break;
			}
			case 3: //Left Click
			{
				int tmp = gui_mouse_b();
				tmp |= 0x01;
				mouse_b = tmp;
				//rv=((gui_mouse_b()&0x1))*10000;
				break;
			}
			case 4: //Right Click
			{
				int tmp = gui_mouse_b();
				tmp |= 0x02;
				mouse_b = tmp;
				//rv=((gui_mouse_b()&0x2))*10000;
				break;
			}
			case 5: //Middle Click
			{
				int tmp = gui_mouse_b();
				tmp |= 0x04;
				mouse_b = tmp;
				break;
			}
			default:
			{
				Z_scripterrlog("Invalid index passed to Input->Mouse[]: %d\n", indx);
			}
		}
			
	}
	break;
        
        
///----------------------------------------------------------------------------------------------------//
//Item Variables
    case ITEMX:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (s->x)=(fix)(value/10000);
            
            // Move the Fairy enemy as well.
            if(itemsbuf[((item*)(s))->id].family==itype_fairy && itemsbuf[((item*)(s))->id].misc3)
                movefairy2(((item*)(s))->x,((item*)(s))->y,((item*)(s))->misc);
        }
        
        break;
        
    case ITEMY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (s->y)=(fix)(value/10000);
            
            // Move the Fairy enemy as well.
            if(itemsbuf[((item*)(s))->id].family==itype_fairy && itemsbuf[((item*)(s))->id].misc3)
                movefairy2(((item*)(s))->x,((item*)(s))->y,((item*)(s))->misc);
        }
        
        break;
        
    case ITEMZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (s->z)=(fix)(value/10000);
            
            if(s->z < 0)
                s->z = 0;
        }
        
        break;
        
    case ITEMJUMP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->fall)=-value*100.0/10000.0;
        }
        
        break;
        
    case ITEMDRAWTYPE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->drawstyle)=value/10000;
        }
        
        break;
        
    case ITEMID:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->id)=value/10000;
            flushItemCache();
        }
        
        break;
        
    case ITEMTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->tile)=vbound(value/10000,0,NEWMAXTILES-1);
        }
        
        break;
	
    case ITEMPSTRING:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->pstring)=vbound(value/10000,0,(msg_count-1));
        }
        
        break;
	
    case ITEMPSTRINGFLAGS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->pickup_string_flags)=vbound(value/10000,0,(msg_count-1));
        }
        
        break;
	
    case ITEMOVERRIDEFLAGS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->overrideFLAGS)=vbound(value/10000,0,(msg_count-1));
        }
        
        break;
        
    case ITEMOTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_tile)=vbound(value/10000,0,NEWMAXTILES-1);
        }
        
        break;
        
    case ITEMCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_cset) = (((item *)s)->o_cset & ~15) | ((value/10000)&15);
            (((item *)s)->cs) = (((item *)s)->o_cset & 15);
        }
        
        break;
        
    case ITEMFLASHCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_cset) = ((value/10000)<<4) | (((item *)s)->o_cset & 15);
        }
        
        break;
        
    case ITEMFRAMES:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->frames)=value/10000;
        }
        
        break;
        
    case ITEMFRAME:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->aframe)=value/10000;
        }
        
        break;
        
    case ITEMASPEED:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_speed)=value/10000;
        }
        
        break;
	
     case ITEMACLK:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->aclk)=value/10000;
        }
        
        break;
	
    case ITEMDELAY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_delay)=value/10000;
        }
        
        break;
        
    case ITEMFLIP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->flip)=value/10000;
        }
        
        break;
        
    case ITEMFLASH:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->flash)= (value/10000)?1:0;
        }
        
        break;
        
    case ITEMEXTEND:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->extend)=value/10000;
        }
        
        break;
        
    case ITEMHXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hxofs=value/10000;
        }
        
        break;
        
    case ITEMHYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hyofs=value/10000;
        }
        
        break;
        
    case ITEMXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->xofs=(fix)(value/10000);
        }
        
        break;
        
    case ITEMYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->yofs=(fix)(value/10000)+playing_field_offset;
        }
        
        break;
        
    case ITEMZOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->zofs=(fix)(value/10000);
        }
        
        break;
        
    case ITEMHXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hxsz=value/10000;
        }
        
        break;
        
    case ITEMHYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hysz=value/10000;
        }
        
        break;
        
    case ITEMHZSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hzsz=value/10000;
        }
        
        break;
        
    case ITEMTXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->txsz=vbound((value/10000),1,20);
        }
        
        break;
        
    case ITEMTYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->tysz=vbound((value/10000),1,20);
        }
        
        break;
        
    case ITEMPICKUP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            int newpickup = value/10000;
            // Values that the questmaker should not use, ever
            newpickup &= ~(ipBIGRANGE | ipCHECK | ipMONEY | ipBIGTRI | ipNODRAW | ipFADE);
            
            // If making an item timeout, set its timer
            if(newpickup & ipFADE)
            {
                (((item*)(s))->clk2) = 512;
            }
            
            // If making it a carried item,
            // alter hasitem and set an itemguy.
            if((((item*)(s))->pickup & ipENEMY) < (newpickup & ipENEMY))
            {
                hasitem |= 2;
                bool hasitemguy = false;
                
                for(int i=0; i<guys.Count(); i++)
                {
                    if(((enemy*)guys.spr(i))->itemguy)
                    {
                        hasitemguy = true;
                    }
                }
                
                if(!hasitemguy && guys.Count()>0)
                {
                    ((enemy*)guys.spr(guys.Count()-1))->itemguy = true;
                }
            }
            // If unmaking it a carried item,
            // alter hasitem if there are no more carried items.
            else if((((item*)(s))->pickup & ipENEMY) > (newpickup & ipENEMY))
            {
                // Move it back onscreen!
                if(get_bit(quest_rules,qr_HIDECARRIEDITEMS))
                {
                    for(int i=0; i<guys.Count(); i++)
                    {
                        if(((enemy*)guys.spr(i))->itemguy)
                        {
                            ((item*)(s))->x = ((enemy*)guys.spr(i))->x;
                            ((item*)(s))->y = ((enemy*)guys.spr(i))->y;
                            ((item*)(s))->z = ((enemy*)guys.spr(i))->z;
                            break;
                        }
                    }
                }
                
                if(more_carried_items()<=1)  // 1 includes this own item.
                {
                    hasitem &= ~2;
                }
            }
            
            ((item*)(s))->pickup=value/10000;
        }
        
        break;
        
    case ITEMMISCD:
        if(0!=(s=checkItem(ri->itemref)))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            (((item*)(s))->miscellaneous[a])=value;
        }
        
        break;
        
///----------------------------------------------------------------------------------------------------//
//Itemdata Variables
	//not mine, but let;s guard some of them all the same -Z
	//item class
    case IDATAFAMILY:
        (itemsbuf[ri->idata].family)=vbound(value/10000,0, 254);
        flushItemCache();
        break;
    
    case IDATAUSEWPN:
        (itemsbuf[ri->idata].useweapon)=vbound(value/10000, 0, 255);
        break;
    case IDATAUSEDEF:
        (itemsbuf[ri->idata].usedefence)=vbound(value/10000, 0, 255);
        break;
    case IDATAWRANGE:
        (itemsbuf[ri->idata].weaprange)=vbound(value/10000, 0, 255);
        break;
    case IDATAMAGICTIMER:
        (itemsbuf[ri->idata].magiccosttimer)=vbound(value/10000, 0, 214747);
        break;
     case IDATADURATION:
        (itemsbuf[ri->idata].weapduration)=vbound(value/10000, 0, 255);
        break;
     
     case IDATADUPLICATES:
	(itemsbuf[ri->idata].duplicates)=vbound(value/10000, 0, 255);
        break;
    case IDATADRAWLAYER:
	(itemsbuf[ri->idata].drawlayer)=vbound(value/10000, 0, 7);
        break;
    case IDATACOLLECTFLAGS:
	//int a = ri->d[0] / 10000;
        (itemsbuf[ri->idata].collectflags)=vbound(value/10000, 0, 214747);
        break;
    case IDATAWEAPONSCRIPT:
	(itemsbuf[ri->idata].weaponscript)=vbound(value/10000, 0, 255);
        break;
    case IDATAMISCD:
    {
	    
	int a = vbound((ri->d[0] / 10000),0,31);
	(itemsbuf[ri->idata].wpn_misc_d[a])=(value/10000);
    }
    break;
    case IDATAWPNINITD:
    {
	    
	int a = vbound((ri->d[0] / 10000),0,7);
	(itemsbuf[ri->idata].weap_initiald[a])=(value/10000);
    }
    break;
    case IDATAWEAPHXOFS:
	(itemsbuf[ri->idata].weap_hxofs)=(value/10000);
        break;
    case IDATAWEAPHYOFS:
	(itemsbuf[ri->idata].weap_hyofs)=(value/10000);
        break;
    case IDATAWEAPHXSZ:
	(itemsbuf[ri->idata].weap_hxsz)=(value/10000);
        break;
    case IDATAWEAPHYSZ:
	(itemsbuf[ri->idata].weap_hysz)=(value/10000);
        break;
    case IDATAWEAPHZSZ:
	(itemsbuf[ri->idata].weap_hzsz)=(value/10000);
        break;
    case IDATAWEAPXOFS:
	(itemsbuf[ri->idata].weap_xofs)=(value/10000);
        break;
    case IDATAWEAPYOFS:
	(itemsbuf[ri->idata].weap_yofs)=(value/10000);
        break;

    
    case IDATAHXOFS:
        (itemsbuf[ri->idata].hxofs)=(value/10000);
        break;
    case IDATAHYOFS:
        (itemsbuf[ri->idata].hyofs)=(value/10000);
        break;
    case IDATAHXSZ:
        (itemsbuf[ri->idata].hxsz)=(value/10000);
        break;
    case IDATAHYSZ:
        (itemsbuf[ri->idata].hysz)=(value/10000);
        break;
    case IDATAHZSZ:
        (itemsbuf[ri->idata].hzsz)=(value/10000);
        break;
    case IDATADXOFS:
        (itemsbuf[ri->idata].xofs)=(value/10000);
        break;
    case IDATADYOFS:
        (itemsbuf[ri->idata].yofs)=(value/10000);
        break;
    case IDATATILEW:
        (itemsbuf[ri->idata].tilew)=(value/10000);
        break;
    case IDATATILEH:
        (itemsbuf[ri->idata].tileh)=(value/10000);
        break;
    case IDATAPICKUP:
        (itemsbuf[ri->idata].pickup)=(value/10000);
        break;
    case IDATAOVERRIDEFL:
        (itemsbuf[ri->idata].overrideFLAGS)=(value/10000);
        break;

    case IDATATILEWWEAP:
        (itemsbuf[ri->idata].weap_tilew)=(value/10000);
        break;
    case IDATATILEHWEAP:
        (itemsbuf[ri->idata].weap_tileh)=(value/10000);
        break;
    case IDATAOVERRIDEFLWEAP:
        (itemsbuf[ri->idata].weapoverrideFLAGS)=(value/10000);
        break;
    
    case IDATAUSEMVT:
	{
	    long a = vbound((ri->d[0] / 10000),0,(ITEM_MOVEMENT_PATTERNS-1));
	    (itemsbuf[ri->idata].weap_pattern[a])=vbound(value/10000, 0, 255);
	}
        break;
    
    
    //item level
    case IDATALEVEL:
        (itemsbuf[ri->idata].fam_type)=vbound(value/10000, 0, 512);
        flushItemCache();
        break;
        //bool keep
    case IDATAKEEP:
        (itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_GAMEDATA:0;
        break;
        //Need the legal range -Z
    case IDATAAMOUNT:
        (itemsbuf[ri->idata].amount)=value/10000;
        break;
        
    case IDATASETMAX:
        (itemsbuf[ri->idata].setmax)=value/10000;
        break;
        
    case IDATAMAX:
        (itemsbuf[ri->idata].max)=value/10000;
        break;
        
    case IDATAPOWER:
        (itemsbuf[ri->idata].power)=value/10000;
        break;
        
    case IDATACOUNTER:
        (itemsbuf[ri->idata].count)=vbound(value/10000,0,31);
        break;
        
    case IDATAUSESOUND:
        (itemsbuf[ri->idata].usesound)=vbound(value/10000, 0, 255);
        break;
    
    //2.54
    //My additions begin here. -Z
    //Stack item to gain next level
    case IDATACOMBINE:
		(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_COMBINE:0; 
		break;
    //using a level of an item downgrades to a lower one
	case IDATADOWNGRADE:
	      (itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_DOWNGRADE:0; 
		break;

	//Flags[5]
	case IDATAFLAGS: {
	    int index = vbound(ri->d[0]/10000,0,4);
		switch(index){
		    case 0:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG1:0; 
		    break;
		    case 1:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG2:0; 
		    case 2:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG3:0; 
		    case 3:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG4:0; 
		    case 4:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG5:0; 
		    case 5:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG6:0; 
		    case 6:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG7:0; 
		    case 7:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG8:0; 
		    case 8:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG9:0; 
		    case 9:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG10:0; 
		    case 10:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG11:0; 
		    case 11:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG12:0; 
		    case 12:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG13:0; 
		    case 13:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG14:0; 
		    case 14:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG15:0; 
		    case 15:
			(itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_FLAG16:0; 
		    
		    
		    default: 
			    break;
		}
		   
		break;
	}
	//Keep Old in editor
	case IDATAKEEPOLD:
	      (itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_KEEPOLD:0; 
		break;
	//Ruppes for magic
	case IDATARUPEECOST:
	      (itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_RUPEE_MAGIC:0; 
		break;
	//can be eaten
	case IDATAEDIBLE:
	      (itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_EDIBLE:0; 
		break;
	//Reserving this for item editor stuff. 
	case IDATAFLAGUNUSED:
	      (itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_UNUSED:0; 
		break;
	//gain lower level items
	case IDATAGAINLOWER:
	      (itemsbuf[ri->idata].flags)|=(value/10000)?ITEM_GAINOLD:0; 
		break;
	//Set the action script
	case IDATASCRIPT:
        itemsbuf[ri->idata].script=vbound(value/10000,1,255);
        break;
    
      /*
      case ITEMMISCD:
        if(0!=(s=checkItem(ri->itemref)))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            (((item*)(s))->miscellaneous[a])=value;
        }
        
        break;*/
	//Attributes[10]
	case IDATAATTRIB: {
	    int index = vbound(ri->d[0]/10000,0,9);
		switch(index){
		    case 0:
			itemsbuf[ri->idata].misc1=value/10000;
		    break;
		    case 1:
			itemsbuf[ri->idata].misc2=value/10000; break;
		    case 2:
			itemsbuf[ri->idata].misc3=value/10000; break;
		    case 3:
			itemsbuf[ri->idata].misc4=value/10000; break;
		    case 4:
			itemsbuf[ri->idata].misc5=value/10000; break;
		    case 5:
			itemsbuf[ri->idata].misc6=value/10000; break;
		    case 6:
			itemsbuf[ri->idata].misc7=value/10000; break;
		    case 7:
			itemsbuf[ri->idata].misc8=value/10000; break;
		    case 8:
			itemsbuf[ri->idata].misc9=value/10000; break;
		    case 9:
			itemsbuf[ri->idata].misc10=value/10000; break;

		    default: 
			    break;
		}
		   
		break;
	}
	//SpriteSprites[10]
	case IDATASPRITE: {
	    int index = vbound(ri->d[0]/10000,0,9);
		switch(index){
		    case 0:
			itemsbuf[ri->idata].wpn=vbound(value/10000, 0, 255);
		    break;
		    case 1:
			itemsbuf[ri->idata].wpn2=(value/10000, 0, 255); break;
		    case 2:
			itemsbuf[ri->idata].wpn3=(value/10000, 0, 255); break;
		    case 3:
			itemsbuf[ri->idata].wpn4=(value/10000, 0, 255); break;
		    case 4:
			itemsbuf[ri->idata].wpn5=(value/10000, 0, 255); break;
		    case 5:
			itemsbuf[ri->idata].wpn6=(value/10000, 0, 255); break;
		    case 6:
			itemsbuf[ri->idata].wpn7=(value/10000, 0, 255); break;
		    case 7:
			itemsbuf[ri->idata].wpn8=(value/10000, 0, 255); break;
		    case 8:
			itemsbuf[ri->idata].wpn9=(value/10000, 0, 255); break;
		    case 9:
			itemsbuf[ri->idata].wpn10=(value/10000, 0, 255); break;
		    
		    default: 
			    break;
		}
		   
		break;
	}
	//Link tile modifier. 
	case IDATALTM:
        itemsbuf[ri->idata].ltm=value/10000;
        break;
	//Pickup script
    case IDATAPSCRIPT:
        itemsbuf[ri->idata].collect_script=(value/10000, 1, 255);
        break;
    //pickup string
    case IDATAPSTRING:
        itemsbuf[ri->idata].pstring=(value/10000, 1, 255);
        break;
    //magic cost
     case IDATAMAGCOST:
        itemsbuf[ri->idata].magic=value/10000;
        break;
     //cost counter ref
     case IDATACOSTCOUNTER:
        itemsbuf[ri->idata].cost_counter=(vbound(value/10000,0,255));
        break;
     //min hearts to pick up
     case IDATAMINHEARTS:
        itemsbuf[ri->idata].pickup_hearts=vbound(value/10000, 0, 214748);
        break;
     //item tile
     case IDATATILE:
        itemsbuf[ri->idata].tile=vbound(value/10000, 0, 65519);
        break;
     //flash
     case IDATAMISC:
        itemsbuf[ri->idata].misc=value/10000;
        break;
     //cset
     case IDATACSET:
        itemsbuf[ri->idata].csets=vbound(value/10000,0,13);
        break;
     /*
     case IDATAFRAME:
        itemsbuf[ri->idata].frame=value/10000;
        break;
     */
     //A.Frames
     case IDATAFRAMES:
	(itemsbuf[ri->idata].frames)=vbound(value/10000, 0, 214748);
        break;
	//A.speed
     case IDATAASPEED:
        itemsbuf[ri->idata].speed=vbound(value/10000, 0, 214748);
        break;
     //Anim delay
     case IDATADELAY:
        itemsbuf[ri->idata].delay=vbound(value/10000, 0, 214748);
        break;
     
        //not one of mine. 
    case IDATAINITDD:
    {
        int a = ri->d[0] / 10000;
        
        if(BC::checkBounds(a, 0, 7, "itemdata->InitD") == SH::_NoError)
            itemsbuf[ri->idata].initiald[a] = value;
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//LWeapon Variables
    case LWPNX:
        if(0!=(s=checkLWpn(ri->lwpn,"X")))
            ((weapon*)s)->x=(fix)(value/10000);
            
        break;
        
    case LWPNY:
        if(0!=(s=checkLWpn(ri->lwpn,"Y")))
            ((weapon*)s)->y=(fix)(value/10000);
            
        break;
        
    case LWPNZ:
        if(0!=(s=checkLWpn(ri->lwpn,"Z")))
            ((weapon*)s)->z=zc_max((fix)(value/10000),(fix)0);
            
        break;
        
    case LWPNJUMP:
        if(0!=(s=checkLWpn(ri->lwpn,"Jump")))
            ((weapon*)s)->fall=((-value*100.0)/10000.0);
            
        break;
        
    case LWPNDIR:
        if(0!=(s=checkLWpn(ri->lwpn,"Dir")))
            ((weapon*)s)->dir=(value/10000);
            
        break;
        
    case LWPNSTEP:
        if(0!=(s=checkLWpn(ri->lwpn,"Step")))
            ((weapon*)s)->step=(value/10000)/100.0;
            
        break;
        
    case LWPNANGLE:
        if(0!=(s=checkLWpn(ri->lwpn,"Angle")))
            ((weapon*)s)->angle=(double)(value/10000.0);
            
        break;
        
    case LWPNANGULAR:
        if(0!=(s=checkLWpn(ri->lwpn,"Angular")))
            ((weapon*)s)->angular=(value/10000) != 0;
            
        break;
        
    case LWPNBEHIND:
        if(0!=(s=checkLWpn(ri->lwpn,"Behind")))
            ((weapon*)s)->behind=(value/10000) != 0;
            
        break;
        
    case LWPNDRAWTYPE:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawStyle")))
            ((weapon*)s)->drawstyle=(value/10000);
            
        break;
        
    case LWPNPOWER:
        if(0!=(s=checkLWpn(ri->lwpn,"Damage")))
            ((weapon*)s)->power=(value/10000);
            
        break;
/*
    case LWPNRANGE:
        if(0!=(s=checkLWpn(ri->lwpn,"Range")))
		((weapon*)s)->scriptrange=vbound((value/10000),0,512); //Allow it to move off-screen. -Z           
        break;
*/        
    case LWPNDEAD:
        if(0!=(s=checkLWpn(ri->lwpn,"DeadState")))
            ((weapon*)s)->dead=(value/10000);
            
        break;
        
    case LWPNID:
        if(0!=(s=checkLWpn(ri->lwpn,"ID")))
            ((weapon*)s)->id=(value/10000);
            
        break;
        
    case LWPNTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"Tile")))
            ((weapon*)s)->tile=(value/10000);
            
        break;
        
    case LWPNCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"CSet")))
            ((weapon*)s)->cs=(value/10000)&15;
            
        break;
        
    case LWPNFLASHCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"FlashCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)<<4;
            
        break;
        
    case LWPNFRAMES:
        if(0!=(s=checkLWpn(ri->lwpn,"NumFrames")))
            ((weapon*)s)->frames=(value/10000);
            
        break;
        
    case LWPNFRAME:
        if(0!=(s=checkLWpn(ri->lwpn,"Frame")))
            ((weapon*)s)->aframe=(value/10000);
            
        break;
        
    case LWPNASPEED:
        if(0!=(s=checkLWpn(ri->lwpn,"ASpeed")))
            ((weapon*)s)->o_speed=(value/10000);
            
        break;
        
    case LWPNFLASH:
        if(0!=(s=checkLWpn(ri->lwpn,"Flash")))
            ((weapon*)s)->flash=(value/10000);
            
        break;
        
    case LWPNFLIP:
        if(0!=(s=checkLWpn(ri->lwpn,"Flip")))
            ((weapon*)s)->flip=(value/10000);
            
        break;
        
    case LWPNEXTEND:
        if(0!=(s=checkLWpn(ri->lwpn,"Extend")))
            ((weapon*)s)->extend=(value/10000);
            
        break;
        
    case LWPNOTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalTile")))
            ((weapon*)s)->o_tile=(value/10000);
            
        break;
        
    case LWPNOCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)&15;
            
        break;
        
    case LWPNHXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitXOffset")))
            (((weapon*)s)->hxofs)=(value/10000);
            
        break;
        
    case LWPNHYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitYOffset")))
            (((weapon*)s)->hyofs)=(value/10000);
            
        break;
        
    case LWPNXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawXOffset")))
            (((weapon*)s)->xofs)=(fix)(value/10000);
            
        break;
        
    case LWPNYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawYOffset")))
            (((weapon*)s)->yofs)=(fix)(value/10000)+playing_field_offset;
            
        break;
        
    case LWPNZOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawZOffset")))
            (((weapon*)s)->zofs)=(fix)(value/10000);
            
        break;
        
    case LWPNHXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitWidth")))
            (((weapon*)s)->hxsz)=(value/10000);
            
        break;
        
    case LWPNHYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitHeight")))
            (((weapon*)s)->hysz)=(value/10000);
            
        break;
        
    case LWPNHZSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitZHeight")))
            (((weapon*)s)->hzsz)=(value/10000);
            
        break;
        
    case LWPNTXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileWidth")))
            (((weapon*)s)->txsz)=vbound((value/10000),1,20);
            
        break;
        
    case LWPNTYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileHeight")))
            (((weapon*)s)->tysz)=vbound((value/10000),1,20);
            
        break;
        
    case LWPNMISCD:
        if(0!=(s=checkLWpn(ri->lwpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            (((weapon*)(s))->miscellaneous[a])=value;
        }
        
        break;
        
    case LWPNCOLLDET:
        if(0!=(s=checkLWpn(ri->lwpn,"CollDetection")))
            (((weapon*)(s))->scriptcoldet)=value/10000;
            
        break;
	
    case LWPNPARENT:
    {
	    //int pitm = (vbound(value/10000,1,(MAXITEMS-1)));
	    //Z_scripterrlog("Attempting to set ParentItem to: %d\n", pitm); 
				
        if(0!=(s=checkLWpn(ri->lwpn,"Parent")))
            (((weapon*)(s))->parentitem)=(vbound(value/10000,1,(MAXITEMS-1)));
    }
        break;

    case LWPNLEVEL:
        if(0!=(s=checkLWpn(ri->lwpn,"Level")))
            (((weapon*)(s))->type)=value/10000;
            
        break;
        
///----------------------------------------------------------------------------------------------------//
//EWeapon Variables
    case EWPNX:
        if(0!=(s=checkEWpn(ri->ewpn,"X")))
            ((weapon*)s)->x=(fix)(value/10000);
            
        break;
        
    case EWPNY:
        if(0!=(s=checkEWpn(ri->ewpn,"Y")))
            ((weapon*)s)->y=(fix)(value/10000);
            
        break;
        
    case EWPNZ:
        if(0!=(s=checkEWpn(ri->ewpn,"Z")))
            ((weapon*)s)->z=zc_max((fix)(value/10000),(fix)0);
            
        break;
        
    case EWPNJUMP:
        if(0!=(s=checkEWpn(ri->ewpn,"Jump")))
            ((weapon*)s)->fall=(-value*100.0/10000.0);
            
        break;
        
    case EWPNDIR:
        if(0!=(s=checkEWpn(ri->ewpn,"Dir")))
            ((weapon*)s)->dir=(value/10000);
            
        break;
        
    case EWPNSTEP:
        if(0!=(s=checkEWpn(ri->ewpn,"Step")))
            ((weapon*)s)->step=(value/10000)/100.0;
            
        break;
        
    case EWPNANGLE:
        if(0!=(s=checkEWpn(ri->ewpn,"Angle")))
            ((weapon*)s)->angle=(double)(value/10000.0);
            
        break;
        
    case EWPNANGULAR:
        if(0!=(s=checkEWpn(ri->ewpn,"Angular")))
            ((weapon*)s)->angular=(value/10000) != 0;
            
        break;
        
    case EWPNBEHIND:
        if(0!=(s=checkEWpn(ri->ewpn,"Behind")))
            ((weapon*)s)->behind=(value/10000) != 0;
            
        break;
        
    case EWPNDRAWTYPE:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawStyle")))
            ((weapon*)s)->drawstyle=(value/10000);
            
        break;
        
    case EWPNPOWER:
        if(0!=(s=checkEWpn(ri->ewpn,"Damage")))
            ((weapon*)s)->power=(value/10000);
            
        break;
        
    case EWPNDEAD:
        if(0!=(s=checkEWpn(ri->ewpn,"DeadState")))
            ((weapon*)s)->dead=(value/10000);
            
        break;
        
    case EWPNID:
        if(0!=(s=checkEWpn(ri->ewpn,"ID")))
            ((weapon*)s)->id=(value/10000);
            
        break;
        
    case EWPNTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"Tile")))
            ((weapon*)s)->tile=(value/10000);
            
        break;
        
    case EWPNCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"CSet")))
            ((weapon*)s)->cs=(value/10000)&15;
            
        break;
        
    case EWPNFLASHCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"FlashCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)<<4;
            
        break;
        
    case EWPNFRAMES:
        if(0!=(s=checkEWpn(ri->ewpn,"NumFrames")))
            ((weapon*)s)->frames=(value/10000);
            
        break;
        
    case EWPNFRAME:
        if(0!=(s=checkEWpn(ri->ewpn,"Frame")))
            ((weapon*)s)->aframe=(value/10000);
            
        break;
        
    case EWPNASPEED:
        if(0!=(s=checkEWpn(ri->ewpn,"ASpeed")))
            ((weapon*)s)->o_speed=(value/10000);
            
        break;
        
    case EWPNFLASH:
        if(0!=(s=checkEWpn(ri->ewpn,"Flash")))
            ((weapon*)s)->flash=(value/10000);
            
        break;
        
    case EWPNFLIP:
        if(0!=(s=checkEWpn(ri->ewpn,"Flip")))
            ((weapon*)s)->flip=(value/10000);
            
        break;
        
    case EWPNEXTEND:
        if(0!=(s=checkEWpn(ri->ewpn,"Extend")))
            ((weapon*)s)->extend=(value/10000);
            
        break;
        
    case EWPNOTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalTile")))
            ((weapon*)s)->o_tile=(value/10000);
            
        break;
        
    case EWPNOCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)&15;
            
        break;
        
    case EWPNHXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitXOffset")))
            (((weapon*)s)->hxofs)=(value/10000);
            
        break;
        
    case EWPNHYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitYOffset")))
            (((weapon*)s)->hyofs)=(value/10000);
            
        break;
        
    case EWPNXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawXOffset")))
            (((weapon*)s)->xofs)=(fix)(value/10000);
            
        break;
        
    case EWPNYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawYOffset")))
            (((weapon*)s)->yofs)=(fix)(value/10000)+playing_field_offset;
            
        break;
        
    case EWPNZOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawZOffset")))
            (((weapon*)s)->zofs)=(fix)(value/10000);
            
        break;
        
    case EWPNHXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitWidth")))
            (((weapon*)s)->hxsz)=(value/10000);
            
        break;
        
    case EWPNHYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitHeight")))
            (((weapon*)s)->hysz)=(value/10000);
            
        break;
        
    case EWPNHZSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitZHeight")))
            (((weapon*)s)->hzsz)=(value/10000);
            
        break;
        
    case EWPNTXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileWidth")))
            (((weapon*)s)->txsz)=vbound((value/10000),1,20);
            
        break;
        
    case EWPNTYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileHeight")))
            (((weapon*)s)->tysz)=vbound((value/10000),1,20);
            
        break;
        
    case EWPNMISCD:
        if(0!=(s=checkEWpn(ri->ewpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            (((weapon*)(s))->miscellaneous[a])=value;
        }
        
        break;
        
    case EWPNCOLLDET:
        if(0!=(s=checkEWpn(ri->ewpn,"CollDetection")))
            (((weapon*)(s))->scriptcoldet)=value/10000;
            
        break;
	
   case EWPNPARENT:
        if(0!=(s=checkEWpn(ri->ewpn, "Parent")))
            (((weapon*)(s))->parentid)=vbound((value/10000),0,511);
            
        break;
        
///----------------------------------------------------------------------------------------------------//
//NPC Variables

//Fixs are all a bit different
    case NPCX:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->X") == SH::_NoError)
        {
            GuyH::getNPC()->x = fix(value / 10000);
            
            if(GuyH::hasLink())
                Link.setX(fix(value / 10000));
        }
    }
    break;
    
    case NPCY:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Y") == SH::_NoError)
        {
            fix oldy = GuyH::getNPC()->y;
            GuyH::getNPC()->y = fix(value / 10000);
            GuyH::getNPC()->floor_y += ((value / 10000) - oldy);
            
            if(GuyH::hasLink())
                Link.setY(fix(value / 10000));
        }
    }
    break;
    
    case NPCZ:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Z") == SH::_NoError)
        {
            if(!never_in_air(GuyH::getNPC()->id))
            {
                if(value < 0)
                    GuyH::getNPC()->z = fix(0);
                else
                    GuyH::getNPC()->z = fix(value / 10000);
                    
                if(GuyH::hasLink())
                    Link.setZ(fix(value / 10000));
            }
        }
    }
    break;
    
    case NPCJUMP:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Jump") == SH::_NoError)
        {
            if(canfall(GuyH::getNPC()->id))
                GuyH::getNPC()->fall = -fix(value * 100.0 / 10000.0);
                
            if(GuyH::hasLink())
                Link.setFall(value / fix(10000.0));
        }
    }
    break;
    
    case NPCSTEP:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Step") == SH::_NoError)
            GuyH::getNPC()->step = fix(value / 10000) / fix(100.0);
    }
    break;
    
    case NPCXOFS:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->DrawXOffset") == SH::_NoError)
            GuyH::getNPC()->xofs = fix(value / 10000);
    }
    break;
    
    case NPCYOFS:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->DrawYOffset") == SH::_NoError)
            GuyH::getNPC()->yofs = fix(value / 10000) + playing_field_offset;
    }
    break;
    
    case NPCZOFS:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->DrawZOffset") == SH::_NoError)
            GuyH::getNPC()->zofs = fix(value / 10000);
    }
    break;
    
#define SET_NPC_VAR_INT(member, str) \
{ \
if(GuyH::loadNPC(ri->guyref, str) == SH::_NoError) \
    GuyH::getNPC()->member = value / 10000; \
}
    
    
    case NPCISCORE:
        if(GuyH::loadNPC(ri->guyref, "npc->isCore") == SH::_NoError)
	    GuyH::getNPC()->isCore = ( (value / 10000) ? true : false );
        break;
	
	
    case NPCDIR:
        SET_NPC_VAR_INT(dir, "npc->Dir") break;
        
    case NPCRATE:
        SET_NPC_VAR_INT(rate, "npc->Rate") break;
        
    case NPCHOMING:
        SET_NPC_VAR_INT(homing, "npc->Homing") break;
        
    case NPCFRAMERATE:
        SET_NPC_VAR_INT(frate, "npc->ASpeed") break;
        
    case NPCHALTRATE:
        SET_NPC_VAR_INT(hrate, "npc->HaltRate") break;
        
    case NPCDRAWTYPE:
        SET_NPC_VAR_INT(drawstyle, "npc->DrawStyle") break;
        
    case NPCHP:
        SET_NPC_VAR_INT(hp, "npc->HP") break;
        
        //case NPCID:        SET_NPC_VAR_INT(id, "npc->ID") break; ~Disallowed
    case NPCDP:
        SET_NPC_VAR_INT(dp, "npc->Damage") break;
        
    case NPCWDP:
        SET_NPC_VAR_INT(wdp, "npc->WeaponDamage") break;
        
    case NPCITEMSET:
        SET_NPC_VAR_INT(item_set, "npc->ItemSet") break;
        
    case NPCBOSSPAL:
        SET_NPC_VAR_INT(bosspal, "npc->BossPal") break;
        
    case NPCBGSFX:
        if(GuyH::loadNPC(ri->guyref, "npc->SFX") == SH::_NoError)
        {
            enemy *en=GuyH::getNPC();
            int newSFX = value / 10000;
            
            // Stop the old sound and start the new one
            if(en->bgsfx != newSFX)
            {
                en->stop_bgsfx(GuyH::getNPCIndex(ri->guyref));
                cont_sfx(newSFX);
                en->bgsfx = newSFX;
            }
        }
        
        break;
        
    case NPCEXTEND:
        SET_NPC_VAR_INT(extend, "npc->Extend") break;
        
    case NPCHXOFS:
        SET_NPC_VAR_INT(hxofs, "npc->HitXOffset") break;
        
    case NPCHYOFS:
        SET_NPC_VAR_INT(hyofs, "npc->HitYOffset") break;
        
    case NPCHXSZ:
        SET_NPC_VAR_INT(hxsz, "npc->HitWidth") break;
        
    case NPCHYSZ:
        SET_NPC_VAR_INT(hysz, "npc->HitHeight") break;
        
    case NPCHZSZ:
        SET_NPC_VAR_INT(hzsz, "npc->HitZHeight") break;
        
    case NPCCOLLDET:
        SET_NPC_VAR_INT(scriptcoldet, "npc->CollDetection") break;
        
    case NPCSTUN:
        SET_NPC_VAR_INT(stunclk, "npc->Stun") break;
        
    case NPCHUNGER:
        SET_NPC_VAR_INT(grumble, "npc->Hunger") break;
    
    case NPCWEAPSPRITE:
        SET_NPC_VAR_INT(wpnsprite, "npc->WeaponSprite") break;
       
    case NPCCSET:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->CSet") == SH::_NoError)
            GuyH::getNPC()->cs = (value / 10000) & 0xF;
    }
    break;
    
//Bounds on value
    case NPCTXSZ:
    {
        long height = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->TileWidth") == SH::_NoError &&
                BC::checkBounds(height, 0, 20, "npc->TileWidth") == SH::_NoError)
            GuyH::getNPC()->txsz = height;
    }
    break;
    
    case NPCTYSZ:
    {
        long width = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->TileHeight") == SH::_NoError &&
                BC::checkBounds(width, 0, 20, "npc->TileHeight") == SH::_NoError)
            GuyH::getNPC()->tysz = width;
    }
    break;
    
    case NPCOTILE:
    {
        long tile = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->OriginalTile") == SH::_NoError &&
                BC::checkTile(tile, "npc->OriginalTile") == SH::_NoError)
            GuyH::getNPC()->o_tile = tile;
    }
    break;
    
    case NPCTILE:
    {
        long tile = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Tile") == SH::_NoError &&
                BC::checkTile(tile, "npc->Tile") == SH::_NoError)
            GuyH::getNPC()->tile = tile;
    }
    break;
    
    case NPCWEAPON:
    {
        long weapon = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Weapon") == SH::_NoError &&
                BC::checkBounds(weapon, 0, MAXWPNS-1, "npc->Weapon") == SH::_NoError)
	{
            GuyH::getNPC()->wpn = weapon;
	
		//al_trace("Correct weapon sprite is: %d /n", FFCore.GetDefaultWeaponSprite(weapon));
		if ( get_bit(extra_rules, er_SETENEMYWEAPONSPRITESONWPNCHANGE) ) //this should probably just be an extra_rule
		{
			GuyH::getNPC()->wpnsprite = FFCore.GetDefaultWeaponSprite(weapon);
		}
		//else GuyH::getNPC()->wpnsprite = FFCore.GetDefaultWeaponSprite(weapon); //just to test that this works. 
	}
    }
    break;
    
//Indexed
    case NPCDEFENSED:
    {
        long a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Defense") == SH::_NoError &&
                BC::checkBounds(a, 0, (edefLAST255), "npc->Defense") == SH::_NoError)
            GuyH::getNPC()->defense[a] = vbound((value / 10000),0,255);
    }
    break;
    
    case NPCHITBY:
    {
	long indx = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->HitBy[]") == SH::_NoError)
	{
		switch(indx)
		{
			//screen index objects
			case 0:
			case 1:
			case 2:
			case 3:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			{
				GuyH::getNPC()->hitby[indx] = vbound((value / 10000),0,255); //Once again, why did I vbound this, and why did I allow it to be written? UIDs are LONGs, with a starting value of 0.0001. -Z
    				break;
			}
			//UIDs
			case 4:
			case 5:
			case 6:
			case 7:
			{
				GuyH::getNPC()->hitby[indx] = value; //Once again, why did I vbound this, and why did I allow it to be written? UIDs are LONGs, with a starting value of 0.0001. -Z
    				break;
			}
			default: al_trace("Invalid index used with npc->hitBy[%d]. /n", indx); break;
		}
	}
	break;
    }
    
    //2.future compat. -Z
    
    
     case NPCSCRDEFENSED:
    {
        long a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->ScriptDefense") == SH::_NoError &&
                BC::checkBounds(a, 0, edefSCRIPTDEFS_MAX, "npc->ScriptDefense") == SH::_NoError)
            GuyH::getNPC()->defense[a+edefSCRIPT01] = value / 10000;
    }
    break;
    
    case NPCMISCD:
    {
        long a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Misc") == SH::_NoError &&
                BC::checkMisc32(a, "npc->Misc") == SH::_NoError)
            GuyH::getNPC()->miscellaneous[a] = value;
            
    }
    
    break;
    
    //npc->Attributes[] setter -Z
    case NPCDD:
    {
        long a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Attributes") == SH::_NoError &&
                BC::checkBounds(a, 0, 15, "npc->Attributes") == SH::_NoError)
	
	switch(a){
		case 0: GuyH::getNPC()->dmisc1 = value / 10000; break;
		case 1: GuyH::getNPC()->dmisc2 = value / 10000; break;
		case 2: GuyH::getNPC()->dmisc3 = value / 10000; break;
		case 3: GuyH::getNPC()->dmisc4 = value / 10000; break;
		case 4: GuyH::getNPC()->dmisc5 = value / 10000; break;
		case 5: GuyH::getNPC()->dmisc6 = value / 10000; break;
		case 6: GuyH::getNPC()->dmisc7 = value / 10000; break;
		case 7: GuyH::getNPC()->dmisc8 = value / 10000; break;
		case 8: GuyH::getNPC()->dmisc9 = value / 10000; break;
		case 9: GuyH::getNPC()->dmisc10 = value / 10000; break;
		case 10: GuyH::getNPC()->dmisc11 = value / 10000; break;
		case 11: GuyH::getNPC()->dmisc12 = value / 10000; break;
		case 12: GuyH::getNPC()->dmisc13 = value / 10000; break;
		case 13: GuyH::getNPC()->dmisc14 = value / 10000; break;
		case 14: GuyH::getNPC()->dmisc15 = value / 10000; break;
		case 15: GuyH::getNPC()->dmisc16 = value / 10000; break;
		case 16: GuyH::getNPC()->dmisc17 = value / 10000; break;
		case 17: GuyH::getNPC()->dmisc18 = value / 10000; break;
		case 18: GuyH::getNPC()->dmisc19 = value / 10000; break;
		case 19: GuyH::getNPC()->dmisc20 = value / 10000; break;
		case 20: GuyH::getNPC()->dmisc21 = value / 10000; break;
		case 21: GuyH::getNPC()->dmisc22 = value / 10000; break;
		case 22: GuyH::getNPC()->dmisc23 = value / 10000; break;
		case 23: GuyH::getNPC()->dmisc24 = value / 10000; break;
		case 24: GuyH::getNPC()->dmisc25 = value / 10000; break;
		case 25: GuyH::getNPC()->dmisc26 = value / 10000; break;
		case 26: GuyH::getNPC()->dmisc27 = value / 10000; break;
		case 27: GuyH::getNPC()->dmisc28 = value / 10000; break;
		case 28: GuyH::getNPC()->dmisc28 = value / 10000; break;
		case 29: GuyH::getNPC()->dmisc30 = value / 10000; break;
		case 30: GuyH::getNPC()->dmisc31 = value / 10000; break;
		case 31: GuyH::getNPC()->dmisc32 = value / 10000; break;
		default: break;
	}

    }
    break;
    
        
    case NPCINVINC:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->InvFrames") == SH::_NoError)
            GuyH::getNPC()->hclk = (int)value/10000;
    }
    break;
    
    case NPCSUPERMAN:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Invincible") == SH::_NoError)
            GuyH::getNPC()->superman = (int)value/10000;
    }
    break;
    
    case NPCHASITEM:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->HasItem") == SH::_NoError)
            GuyH::getNPC()->itemguy = (value/10000)?1:0;
    }
    break;
    
    case NPCRINGLEAD:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Ringleader") == SH::_NoError)
            GuyH::getNPC()->leader = (value/10000)?1:0;
    }
    break;
    
	case NPCSHIELD:
	{
		int indx = ri->d[0];
		if(GuyH::loadNPC(ri->guyref, "npc->Shield[]") == SH::_NoError)
		{
			switch(indx)
			{
				case 0:
				{
					(ri->d[1])? (GuyH::getNPC()->flags |= inv_front) : (GuyH::getNPC()->flags &= ~inv_front);
					break;
				}
				case 1:
				{
					(ri->d[1])? (GuyH::getNPC()->flags |= inv_left) : (GuyH::getNPC()->flags &= ~inv_left);
					break;
				}
				case 2:
				{
					(ri->d[1])? (GuyH::getNPC()->flags |= inv_right) : (GuyH::getNPC()->flags &= ~inv_right);
					break;
				}
				case 3:
				{
					(ri->d[1])? (GuyH::getNPC()->flags |= inv_back) : (GuyH::getNPC()->flags &= ~inv_back);
					break;
				}
				case 4: //shield can be broken
				{
					(ri->d[1])? (GuyH::getNPC()->flags |= guy_bkshield) : (GuyH::getNPC()->flags &= ~guy_bkshield);
					break;
				}
				default:
				{
					Z_scripterrlog("Invalid Array Index passed to npc->Shield[]: %d\n", indx); 
					break;
				}
			}
		}
	}
	
	case NPCFROZENTILE:
        SET_NPC_VAR_INT(frozentile, "npc->FrozenTile") break;
	case NPCFROZENCSET:
        SET_NPC_VAR_INT(frozencset, "npc->FrozenCSet") break;
	case NPCFROZEN:
        SET_NPC_VAR_INT(frozenclock, "npc->Frozen") break;
    
///----------------------------------------------------------------------------------------------------//
//Game Information
    
    case GAMESUBSCHEIGHT:
    {
	int v = vbound(value,0,256);
	passive_subscreen_height = (v/10000);   
    }
    break;
    
    case GAMEPLAYFIELDOFS:
    {
	int v = vbound(value,-256, 256);
	playing_field_offset = (v/10000);
    }
    break;
    
    case PASSSUBOFS:
    {
	int v = vbound(value,-256, 256);
	passive_subscreen_offset = (v/10000);
    }
    break;

    
    case GAMEDEATHS:
        game->set_deaths(value/10000);
        break;
        
    case GAMECHEAT:
        game->set_cheat(value/10000);
        cheat=(value/10000);
        break;
        
    case GAMETIME:
        game->set_time(value);
        break; // Can't multiply by 10000 or the maximum result is too big
        
    case GAMETIMEVALID:
        game->set_timevalid((value/10000)?1:0);
        break;
        
    case GAMEHASPLAYED:
        game->set_hasplayed((value/10000)?1:0);
        break;
    
    case TYPINGMODE:
        FFCore.kb_typing_mode = ((value/10000)?true:false);
        break;
        
    case GAMEGUYCOUNT:
    {
        int mi2 = (currmap*MAPSCRSNORMAL)+(ri->d[0]/10000);
        game->guys[mi2]=value/10000;
    }
    break;
    
    case GAMECONTSCR:
        game->set_continue_scrn(value/10000);
        break;
        
    case GAMECONTDMAP:
        game->set_continue_dmap(value/10000);
        break;
        
    case GAMEENTRSCR:
        lastentrance=value/10000;
        break;
        
    case GAMEENTRDMAP:
        lastentrance_dmap=value/10000;
        break;
        
    case GAMECOUNTERD:
        game->set_counter(value/10000, (ri->d[0])/10000);
        break;
        
    case GAMEMCOUNTERD:
        game->set_maxcounter(value/10000, (ri->d[0])/10000);
        break;
        
    case GAMEDCOUNTERD:
        game->set_dcounter(value/10000, (ri->d[0])/10000);
        break;
        
    case GAMEGENERICD:
        game->set_generic(value/10000, (ri->d[0])/10000);
        break;
    case GAMEMISC:
    {
	int indx = ri->d[0]/10000;
	if ( indx < 0 || indx > 31 )
	{
		Z_scripterrlog("Invalud index used to access Game->Misc: %d\n", indx);
	}
	else 
	{
		QMisc.questmisc[indx] = value;
	}
	break;
    }
    case GAMEITEMSD:
        game->set_item((ri->d[0])/10000,(value!=0));
        break;
    
    case DISABLEDITEM:
	game->items_off[(ri->d[0])/10000]=value/10000;
	break;
        
    case GAMELITEMSD:
        game->lvlitems[(ri->d[0])/10000]=value/10000;
        break;
        
    case GAMELKEYSD:
        game->lvlkeys[(ri->d[0])/10000]=value/10000;
        break;
        
    case SCREENSTATED:
    {
        int mi2 = (currmap*MAPSCRSNORMAL)+currscr;
        (value)?setmapflag(mi2, 1<<((ri->d[0])/10000)) : unsetmapflag(mi2, 1 << ((ri->d[0]) / 10000));
    }
    break;
    
    case SCREENSTATEDD:
    {
        int mi2 = ri->d[0]/10000;
        mi2 -= 8*(mi2/MAPSCRS);
        
        if(BC::checkMapID(mi2>>7, "Game->SetScreenState") == SH::_NoError)
            (value)?setmapflag(mi2, 1<<(ri->d[1]/10000)) : unsetmapflag(mi2, 1 << (ri->d[1] / 10000), true);
    }
    break;
    
    case GAMEGUYCOUNTD:
        game->guys[(currmap*MAPSCRSNORMAL)+(ri->d[0]/10000)] = value / 10000;
        break;
        
    case GAMECLICKFREEZE:
        disableClickToFreeze=value==0;
        break;
    
    
    case NOACTIVESUBSC:
	Link.stopSubscreenFalling((value/10000)?1:0);
	break;
        
///----------------------------------------------------------------------------------------------------//
//DMap Information

#define SET_DMAP_VAR(member, str) \
{ \
    int ID = ri->d[0] / 10000; \
    if(BC::checkDMapID(ID, str) == SH::_NoError) \
        DMaps[ID].member = value / 10000; \
}

    case DMAPFLAGSD:
        SET_DMAP_VAR(flags, "Game->DMapFlags") break;
        
    case DMAPLEVELD:
        SET_DMAP_VAR(level, "Game->DMapLevel") break;
        
    case DMAPCOMPASSD:
        SET_DMAP_VAR(compass, "Game->DMapCompass") break;
        
    case DMAPCONTINUED:
        SET_DMAP_VAR(cont, "Game->DMapContinue") break;
        
     case DMAPLEVELPAL:
   {
	int ID = ri->d[0] / 10000; 
	int pal = value/10000;
	pal = vbound(pal, 0, 0x1FF);
	   
	if(BC::checkDMapID(ID, "Game->DMapPalette") == SH::_NoError) 
        DMaps[ID].color = pal;

	loadlvlpal(DMaps[(ri->d[0] / 10000)].color);
	break;
   }
   
    case DMAPMIDID:
    {
        int ID = ri->d[0] / 10000;
        
        if(BC::checkDMapID(ID, "Game->DMapMIDI") == SH::_NoError)
        {
            // Based on play_DmapMusic
            switch(value / 10000)
            {
            case -6:
                DMaps[ID].midi = 2;
                break; // Dungeon
                
            case -3:
                DMaps[ID].midi = 3;
                break; // Level 9
                
            case -2:
                DMaps[ID].midi = 1;
                break; // Overworld
                
            case 0:
                DMaps[ID].midi = 0;
                break; // None
                
            default:
                DMaps[ID].midi = value / 10000 + 3;
            }
        }
        
        break;
    }
    
///----------------------------------------------------------------------------------------------------//
//Screen->ComboX
    case COMBODD:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
        {
            screen_combo_modify_preroutine(tmpscr,pos);
            tmpscr->data[pos]=(value/10000);
            screen_combo_modify_postroutine(tmpscr,pos);
        }
    }
    break;
    
    case COMBOCD:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
        {
            screen_combo_modify_preroutine(tmpscr,pos);
            tmpscr->cset[pos]=(value/10000)&15;
            screen_combo_modify_postroutine(tmpscr,pos);
        }
    }
    break;
    
    case COMBOFD:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
            tmpscr->sflag[pos]=(value/10000);
    }
    break;
    
    case COMBOTD:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
        {
            // Preprocess each instance of the combo on the screen
            for(int i = 0; i < 176; i++)
            {
                if(tmpscr->data[i] == tmpscr->data[pos])
                {
                    screen_combo_modify_preroutine(tmpscr,i);
                }
            }
            
            combobuf[tmpscr->data[pos]].type=value/10000;
            
            for(int i = 0; i < 176; i++)
            {
                if(tmpscr->data[i] == tmpscr->data[pos])
                {
                    screen_combo_modify_postroutine(tmpscr,i);
                }
            }
        }
    }
    break;
    
    case COMBOID:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
            combobuf[tmpscr->data[pos]].flag=value/10000;
    }
    break;
    
    case COMBOSD:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
            combobuf[tmpscr->data[pos]].walk=(value/10000)&15;
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//Game->SetComboX
case COMBODDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        
        if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;
        
        if(scr==(currmap*MAPSCRS+currscr))
            screen_combo_modify_preroutine(tmpscr,pos);
            
        TheMaps[scr].data[pos]=value/10000;
        
        if(scr==(currmap*MAPSCRS+currscr))
        {
            tmpscr->data[pos] = value/10000;
            screen_combo_modify_postroutine(tmpscr,pos);
        }
        
        int layr = whichlayer(scr);
        
        if(layr>-1)
        {
            //if (layr==(currmap*MAPSCRS+currscr))
            //  screen_combo_modify_preroutine(tmpscr,pos);
            tmpscr2[layr].data[pos]=value/10000;
            //if (layr==(currmap*MAPSCRS+currscr))
            //  screen_combo_modify_postroutine(tmpscr,pos);
        }
    }
    break;
    
    case COMBOCDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        
        if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;
        
        TheMaps[scr].cset[pos]=(value/10000)&15;
        
        if(scr==(currmap*MAPSCRS+currscr))
            tmpscr->cset[pos] = value/10000;
            
        int layr = whichlayer(scr);
        
        if(layr>-1)
            tmpscr2[layr].cset[pos]=(value/10000)&15;
    }
    break;
    
    case COMBOFDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        
        if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count)) break;
        
        TheMaps[scr].sflag[pos]=value/10000;
        
        if(scr==(currmap*MAPSCRS+currscr))
            tmpscr->sflag[pos] = value/10000;
            
        int layr = whichlayer(scr);
        
        if(layr>-1)
            tmpscr2[layr].sflag[pos]=value/10000;
    }
    break;
    
    case COMBOTDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        
        if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count))
            break;
            
        int cdata = TheMaps[scr].data[pos];
        
        // Preprocess the screen's combos in case the combo changed is present on the screen. -L
        for(int i = 0; i < 176; i++)
        {
            if(tmpscr->data[i] == cdata)
            {
                screen_combo_modify_preroutine(tmpscr,i);
            }
        }
        
        combobuf[cdata].type=value/10000;
        
        for(int i = 0; i < 176; i++)
        {
            if(tmpscr->data[i] == cdata)
            {
                screen_combo_modify_postroutine(tmpscr,i);
            }
        }
    }
    break;
    
    case COMBOIDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = zc_max((ri->d[1]/10000)-1,0);
        long scr = zc_max(m*MAPSCRS+sc,0);
        
        if(!(pos >= 0 && pos < 176 && scr >= 0 && sc < MAPSCRS && m < map_count))
            break;
            
        combobuf[TheMaps[scr].data[pos]].flag=value/10000;
    }
    break;
    
    case COMBOSDM:
    {
	    //This is how it was in 2.50.1-2
		int pos = (ri->d[0])/10000;
		long scr = (ri->d[1]/10000)*MAPSCRS+(ri->d[2]/10000);
		//This (below) us the precise code from 2.50.1 (?)
		//long scr = zc_max((ri->d[1]/10000)*MAPSCRS+(ri->d[2]/10000),0); //Not below 0. 

		if(pos < 0 || pos >= 176 || scr < 0) break;

		combobuf[TheMaps[scr].data[pos]].walk=(value/10000)&15;	    
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//Screen Variables
    
    	#define	SET_SCREENDATA_VAR_INT32(member, str) \
	{ \
		tmpscr->member = vbound((value / 10000),-214747,214747); \
	} \
	
	#define	SET_SCREENDATA_VAR_INT16(member, str) \
	{ \
		tmpscr->member = vbound((value / 10000),0,32767); \
	} \

	#define	SET_SCREENDATA_VAR_BYTE(member, str) \
	{ \
		tmpscr->member = vbound((value / 10000),0,255); \
	} \
	
	#define SET_SCREENDATA_VAR_INDEX32(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		tmpscr->member[indx] = vbound((value / 10000),-214747,214747); \
	} \
	
	#define SET_SCREENDATA_VAR_INDEX16(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		tmpscr->member[indx] = vbound((value / 10000),-32767,32767); \
	} \

	#define SET_SCREENDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		tmpscr->member[indx] = vbound((value / 10000),0,255); \
	}
	#define SET_SCREENDATA_LAYER_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		tmpscr->member[indx] = vbound((value / 10000),0,255); \
	}
	
	#define SET_SCREENDATA_FLAG(member, str) \
	{ \
		long flag =  (value/10000);  \
		if ( flag != 0 ) \
		{ \
			tmpscr->member|=flag; \
		} \
		else tmpscr->.member|= ~flag; \
	} \
	
	#define SET_SCREENDATA_BOOL_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to Screen->%s[]: %d\n", (indx), str); \
			break; \
		} \
		tmpscr->member[indx] =( (value/10000) ? 1 : 0 ); \
	}
    

case SCREENDATAVALID:		SET_SCREENDATA_VAR_BYTE(valid, "Valid"); break;		//b
case SCREENDATAGUY: 		SET_SCREENDATA_VAR_BYTE(guy, "Guy"); break;		//b
case SCREENDATASTRING:		SET_SCREENDATA_VAR_INT32(str, "String"); break;		//w
case SCREENDATAROOM: 		SET_SCREENDATA_VAR_BYTE(room, "RoomType");	break;		//b
case SCREENDATAITEM: 		SET_SCREENDATA_VAR_BYTE(item, "Item"); break;		//b
case SCREENDATAHASITEM: 		SET_SCREENDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
case SCREENDATATILEWARPTYPE: 	SET_SCREENDATA_BYTE_INDEX(tilewarptype, "TileWarpType", 3); break;	//b, 4 of these
case SCREENDATATILEWARPOVFLAGS: 	SET_SCREENDATA_VAR_BYTE(tilewarpoverlayflags, "TileWarpOverlayFlags"); break;	//b, tilewarpoverlayflags
case SCREENDATADOORCOMBOSET: 	SET_SCREENDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
case SCREENDATAWARPRETX:	 	SET_SCREENDATA_BYTE_INDEX(warpreturnx, "WarpReturnX", 3); break;	//b, 4 of these
case SCREENDATAWARPRETY:	 	SET_SCREENDATA_BYTE_INDEX(warpreturny, "WarpReturnY", 3); break;	//b, 4 of these
case SCREENDATAWARPRETURNC: 	SET_SCREENDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
case SCREENDATASTAIRX: 		SET_SCREENDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
case SCREENDATASTAIRY: 		SET_SCREENDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
case SCREENDATAITEMX:		SET_SCREENDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
case SCREENDATAITEMY:		SET_SCREENDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
case SCREENDATACOLOUR: 		SET_SCREENDATA_VAR_INT32(color, "CSet"); break;	//w
case SCREENDATAENEMYFLAGS: 	SET_SCREENDATA_VAR_BYTE(enemyflags, "EnemyFlags");	break;	//b
case SCREENDATADOOR: 		SET_SCREENDATA_BYTE_INDEX(door, "Door", 3); break;	//b, 4 of these
case SCREENDATATILEWARPDMAP: 	SET_SCREENDATA_VAR_INDEX32(tilewarpdmap, "TileWarpDMap", 3); break;	//w, 4 of these
case SCREENDATATILEWARPSCREEN: 	SET_SCREENDATA_BYTE_INDEX(tilewarpscr, "TileWarpScreen", 3); break;	//b, 4 of these
case SCREENDATAEXITDIR: 		SET_SCREENDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
case SCREENDATAENEMY: 		SET_SCREENDATA_VAR_INDEX32(enemy, "Enemy", 9); break;	//w, 10 of these
case SCREENDATAPATTERN: 		SET_SCREENDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
case SCREENDATASIDEWARPTYPE: 	SET_SCREENDATA_BYTE_INDEX(sidewarptype, "SideWarpType", 3); break;	//b, 4 of these
case SCREENDATASIDEWARPOVFLAGS: 	SET_SCREENDATA_VAR_BYTE(sidewarpoverlayflags, "SideWarpOverlayFlags"); break;	//b
case SCREENDATAWARPARRIVALX: 	SET_SCREENDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
case SCREENDATAWARPARRIVALY: 	SET_SCREENDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
case SCREENDATAPATH: 		SET_SCREENDATA_BYTE_INDEX(path, "MazePath", 3); break;	//b, 4 of these
case SCREENDATASIDEWARPSC: 	SET_SCREENDATA_BYTE_INDEX(sidewarpscr, "SideWarpScreen", 3); break;	//b, 4 of these
case SCREENDATASIDEWARPDMAP: 	SET_SCREENDATA_VAR_INDEX32(sidewarpdmap, "SideWarpDMap", 3); break;	//w, 4 of these
case SCREENDATASIDEWARPINDEX: 	SET_SCREENDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
case SCREENDATAUNDERCOMBO: 	SET_SCREENDATA_VAR_INT32(undercombo, "Undercombo"); break;	//w
case SCREENDATAUNDERCSET:	 	SET_SCREENDATA_VAR_BYTE(undercset,	"UnderCSet"); break; //b
case SCREENDATACATCHALL:	 	SET_SCREENDATA_VAR_INT32(catchall,	"Catchall"); break; //W

case SCREENDATACSENSITIVE: 	SET_SCREENDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
case SCREENDATANORESET: 		SET_SCREENDATA_VAR_INT32(noreset, "NoReset"); break;	//W
case SCREENDATANOCARRY: 		SET_SCREENDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
case SCREENDATALAYERMAP:	 	SET_SCREENDATA_LAYER_INDEX(layermap, "LayerMap", 5); break;	//B, 6 OF THESE
case SCREENDATALAYERSCREEN: 	SET_SCREENDATA_LAYER_INDEX(layerscreen, "LayerScreen", 5); break;	//B, 6 OF THESE
case SCREENDATALAYEROPACITY: 	SET_SCREENDATA_LAYER_INDEX(layeropacity, "LayerOpacity", 5); break;	//B, 6 OF THESE
case SCREENDATATIMEDWARPTICS: 	SET_SCREENDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
case SCREENDATANEXTMAP: 		SET_SCREENDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
case SCREENDATANEXTSCREEN: 	SET_SCREENDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
case SCREENDATASECRETCOMBO: 	SET_SCREENDATA_VAR_INDEX32(secretcombo, "SecretCombo", 127); break;	//W, 128 OF THESE
case SCREENDATASECRETCSET: 	SET_SCREENDATA_BYTE_INDEX(secretcset, "SecretCSet", 127); break;	//B, 128 OF THESE
case SCREENDATASECRETFLAG: 	SET_SCREENDATA_BYTE_INDEX(secretflag, "SecretFlags", 127); break;	//B, 128 OF THESE
case SCREENDATAVIEWX: 		SET_SCREENDATA_VAR_INT32(viewX, "ViewX"); break;	//W
case SCREENDATAVIEWY: 		SET_SCREENDATA_VAR_INT32(viewY, "ViewY"); break; //W
case SCREENDATASCREENWIDTH: 	SET_SCREENDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
case SCREENDATASCREENHEIGHT: 	SET_SCREENDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
case SCREENDATAENTRYX: 		SET_SCREENDATA_VAR_BYTE(entry_x, "EntryX"); break;	//B
case SCREENDATAENTRYY: 		SET_SCREENDATA_VAR_BYTE(entry_y, "EntryY"); break;	//B
case SCREENDATANUMFF: 		SET_SCREENDATA_VAR_INT16(numff, "NumFFCs"); break;	//INT16

	//inita	//INT32, 32 OF THESE, EACH WITH 2
case SCREENDATAFFINITIALISED: 	SET_SCREENDATA_BOOL_INDEX(initialized, "FFCRunning", 31); break;	//BOOL, 32 OF THESE
case SCREENDATASCRIPTENTRY: 	SET_SCREENDATA_VAR_INT32(script_entry, "ScriptEntry"); break;	//W
case SCREENDATASCRIPTOCCUPANCY: 	SET_SCREENDATA_VAR_INT32(script_occupancy,	"ScriptOccupancy");  break;//W
case SCREENDATASCRIPTEXIT: 	SET_SCREENDATA_VAR_INT32(script_exit, "ExitScript"); break;	//W
case SCREENDATAOCEANSFX:	 	SET_SCREENDATA_VAR_BYTE(oceansfx, "OceanSFX"); break;	//B
case SCREENDATABOSSSFX: 		SET_SCREENDATA_VAR_BYTE(bosssfx, "BossSFX"); break;	//B
case SCREENDATASECRETSFX:	 	SET_SCREENDATA_VAR_BYTE(secretsfx, "SecretSFX"); break;	//B
case SCREENDATAHOLDUPSFX:	 	SET_SCREENDATA_VAR_BYTE(holdupsfx,	"ItemSFX"); break; //B
case SCREENDATASCREENMIDI: 	SET_SCREENDATA_VAR_INT16(screen_midi, "MIDI"); break;	//SHORT, OLD QUESTS ONLY?
case SCREENDATALENSLAYER:	 	SET_SCREENDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
	

case SCREENDATAFLAGS: 
{
	int flagid = (ri->d[0])/10000;
	//bool valtrue = ( value ? 10000 : 0);
	switch(flagid)
	{
		case 0: tmpscr->flags = (value / 10000); break;
		case 1: tmpscr->flags2 = (value / 10000); break;
		case 2: tmpscr->flags3 = (value / 10000); break;
		case 3: tmpscr->flags4 = (value / 10000); break;
		case 4: tmpscr->flags5 = (value / 10000); break;
		case 5: tmpscr->flags6 = (value / 10000); break;
		case 6: tmpscr->flags7 = (value / 10000); break;
		case 7: tmpscr->flags8 = (value / 10000); break;
		case 8: tmpscr->flags9 = (value / 10000); break;
		case 9: tmpscr->flags10 = (value / 10000); break;
		default:
		{
			Z_scripterrlog("Invalid index passed to mapdata->flags[]: %d\n", flagid); 
			break;
			
		}
	}
	break;
	//GET_SCREENDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10
}


    //These use the same method as SetScreenD
    case SCREENWIDTH:
	FFScript::set_screenWidth(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENHEIGHT:
	FFScript::set_screenHeight(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENVIEWX:
	FFScript::set_screenViewX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENVIEWY:
	FFScript::set_screenViewY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENGUY:
	FFScript::set_screenGuy(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENSTRING:
    {
	FFScript::set_screenString(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	    //should this be either
	    //set_screenString(&TheMaps[((ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)])-1), value/10000);
	    //or
	    //set_screenString(&TheMaps[((ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)])-+1), value/10000);
	    Z_message("Map ref is: %d\n",((ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)));
    }
	break;

    case SCREENROOM:
	FFScript::set_screenRoomtype(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENENTX:
	FFScript::set_screenEntryX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENENTY:
	FFScript::set_screenEntryY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENITEM:
	FFScript::set_screenitem(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENUNDCMB:
	FFScript::set_screenundercombo(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENUNDCST:
	FFScript::set_screenundercset(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case SCREENCATCH:
	FFScript::set_screenatchall(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

//These use the method of SetScreenEnemy


//SetScreenLayerOpacity(int map, int scr, int layer, int v)
case SETSCREENLAYOP:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenLayerOpacity(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenLayerOpacity(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenLayerOpacity(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screenlayeropacity(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENSECCMB:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenSecretCombo(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenSecretCombo(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenSecretCombo(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screensecretcombo(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENSECCST:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenSecretCSet(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenSecretCSet(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenSecretCSet(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screensecretcset(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENSECFLG:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenSecretFlag(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenSecretFlag(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenSecretFlag(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screensecretflag(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENLAYMAP:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenLayerMap(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenLayerMap(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenLayerMap(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screenlayermap(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENLAYSCR:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenLayerScreen(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenLayerScreen(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenLayerScreen(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screenlayerscreen(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENPATH:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenPath(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenPath(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenPath(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screenpath(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENWARPRX:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenWarpReturnX(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenWarpReturnX(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenWarpReturnX(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screenwarpReturnX(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

case SETSCREENWARPRY:
{ 
	long map     = (ri->d[1] / 10000) - 1; //Should this be +1? -Z
	long scrn  = ri->d[2] / 10000; 
	long index = ri->d[0] / 10000; 
	int nn = ri->d[3]/10000; 

	if(BC::checkMapID(map, "Game->SetScreenWarpReturnY(...map...)") != SH::_NoError ||
		BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenWarpReturnY(...screen...)") != SH::_NoError ||
		BC::checkBounds(index, 0, 9, "Game->SetScreenWarpReturnY(...index...)") != SH::_NoError)
		return;
		
	FFScript::set_screenwarpReturnY(&TheMaps[map * MAPSCRS + scrn], index, nn); 
}
break;

    case SDD:
    {
        {
            int di2 = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
            FFScript::set_screen_d(di2, ri->d[0]/10000, value);
            break;
        }
    }
    
    case GDD:
        al_trace("GDD");
        game->global_d[ri->d[0]/10000]=value;
        break;
        
    case SDDD:
        FFScript::set_screen_d((ri->d[0])/10000 + ((get_currdmap())<<7), ri->d[1]/10000, value);
        break;
        
    case SDDDD:
        FFScript::set_screen_d(ri->d[1]/10000 + ((ri->d[0]/10000)<<7), ri->d[2]/10000, value);
        break;
        
    case SCRDOORD:
        tmpscr->door[ri->d[0]/10000]=value/10000;
        putdoor(scrollbuf,0,ri->d[0]/10000,value/10000,true,true);
        break;
        
    case LIT:
        naturaldark = !value;
        lighting(false, false);
        break;
        
    case WAVY:
        wavy=value/10000;
        break;
        
    case QUAKE:
        quakeclk=value/10000;
        break;
        
    case ROOMTYPE:
        tmpscr->room=value/10000; break; //this probably doesn't work too well...
    
    case ROOMDATA:
        tmpscr->catchall=value/10000;
        break;
        
    case PUSHBLOCKCOMBO:
        mblock2.bcombo=value/10000;
        break;
        
    case PUSHBLOCKCSET:
        mblock2.cs=value/10000;
        mblock2.oldcset=value/10000;
        break;
        
    case UNDERCOMBO:
        tmpscr->undercombo=value/10000;
        break;
        
    case UNDERCSET:
        tmpscr->undercset=value/10000;
        break;
    
    
    case DEBUGGDR:
    {
	int a = vbound(ri->d[0]/10000,0,15);
        game->global_d[a] = value / 10000;;
    }
        break;
    
    case DEBUGSP:
        ri->sp = vbound((value / 10000),0,256);
        break;
        
    case DEBUGREFFFC:
        ri->ffcref = vbound((value / 10000),1,31);
        break;
        
    case DEBUGREFITEM:
        ri->itemref = vbound((value / 10000),0,255);
        break;
        
    case DEBUGREFITEMDATA:
        ri->idata = vbound((value / 10000),0,255);
        break;
        
    case DEBUGREFLWEAPON:
        ri->lwpn = vbound((value / 10000),0,255);
        break;
        
    case DEBUGREFEWEAPON:
        ri->ewpn = vbound((value / 10000),0,255);
        break;
        
    case DEBUGREFNPC:
        ri->guyref = vbound((value / 10000),0,255);
        break;
        
    
    //Game Over Screen
    case SETGAMEOVERELEMENT:
    {
	long colour = value/10000;
	int index = ri->d[0]/10000;
	index = vbound(index,0,11);
	al_trace("GameOverScreen Index: %d/n",index);
	    al_trace("GameOverScreen Value: %d/n",colour);
	SetSaveScreenSetting(index,colour);
    }
        break;

	case SETGAMEOVERSTRING:
	{
		long arrayptr = value/10000;
		int index = ri->d[0]/10000;
		string filename_str;
		ArrayH::getString(arrayptr, filename_str, 73);
		ChangeSubscreenText(index,filename_str.c_str());
		
	}
        break;

///----------------------------------------------------------------------------------------------------//
//New Datatype Variables
    
///----------------------------------------------------------------------------------------------------//
//spritedata sp-> Variables
case SPRITEDATATILE: SET_SPRITEDATA_VAR_INT(newtile, "Tile"); break;
case SPRITEDATAMISC: SET_SPRITEDATA_VAR_BYTE(misc, "Misc"); break;
case SPRITEDATACSETS: SET_SPRITEDATA_VAR_BYTE(csets, "CSet"); break;
case SPRITEDATAFRAMES: SET_SPRITEDATA_VAR_BYTE(frames, "Frames"); break;
case SPRITEDATASPEED: SET_SPRITEDATA_VAR_BYTE(speed, "Speed"); break;
case SPRITEDATATYPE: SET_SPRITEDATA_VAR_BYTE(type, "Type"); break;
    
///----------------------------------------------------------------------------------------------------//
//mapdata m-> Variables
	//mapdata m-> Variables
	#define	SET_MAPDATA_VAR_INT32(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		m->member = vbound((value / 10000),-214747,214747); \
	} \
	
	#define	SET_MAPDATA_VAR_INT16(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		m->member = vbound((value / 10000),0,32767); \
	} \

	#define	SET_MAPDATA_VAR_BYTE(member, str) \
	{ \
		mapscr *m = &TheMaps[ri->mapsref]; \
		m->member = vbound((value / 10000),0,255); \
	} \
	
	#define SET_MAPDATA_VAR_INDEX32(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			break; \
		} \
		mapscr *m = &TheMaps[ri->mapsref]; \
		m->member[indx] = vbound((value / 10000),-214747,214747); \
	} \
	
	#define SET_MAPDATA_VAR_INDEX16(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			break; \
		} \
		mapscr *m = &TheMaps[ri->mapsref]; \
		m->member[indx] = vbound((value / 10000),-32767,32767); \
	} \

	#define SET_MAPDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			break; \
		} \
		mapscr *m = &TheMaps[ri->mapsref]; \
		m->member[indx] = vbound((value / 10000),0,255); \
	}
	
	#define SET_MAPDATA_BOOL_INDEX(member, str, indexbound) \
	{ \
		int indx = ri->d[0] / 10000; \
		if(indx < 0 || indx > indexbound ) \
		{ \
			Z_scripterrlog("Invalid Index passed to mapdata->%s[]: %d\n", (indx), str); \
			break; \
		} \
		mapscr *m = &TheMaps[ri->mapsref]; \
		m->member[indx] =( (value/10000) ? 1 : 0 ); \
	}
	
	#define SET_MAPDATA_FLAG(member, str) \
	{ \
		long flag =  (value/10000);  \
		mapscr *m = &TheMaps[ri->mapsref]; \
		if ( flag != 0 ) \
		{ \
			m->member|=flag; \
		} \
		else m->.member|= ~flag; \
	} \

case MAPDATAVALID:		SET_MAPDATA_VAR_BYTE(valid, "Valid"); break;		//b
case MAPDATAGUY: 		SET_MAPDATA_VAR_BYTE(guy, "Guy"); break;		//b
case MAPDATASTRING:		SET_MAPDATA_VAR_INT32(str, "String"); break;		//w
case MAPDATAROOM: 		SET_MAPDATA_VAR_BYTE(room, "RoomType");	break;		//b
case MAPDATAITEM: 		SET_MAPDATA_VAR_BYTE(item, "Item"); break;		//b
case MAPDATAHASITEM: 		SET_MAPDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
case MAPDATATILEWARPTYPE: 	SET_MAPDATA_BYTE_INDEX(tilewarptype, "TileWarpType", 3); break;	//b, 4 of these
case MAPDATATILEWARPOVFLAGS: 	SET_MAPDATA_VAR_BYTE(tilewarpoverlayflags, "TileWarpOverlayFlags"); break;	//b, tilewarpoverlayflags
case MAPDATADOORCOMBOSET: 	SET_MAPDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
case MAPDATAWARPRETX:	 	SET_MAPDATA_BYTE_INDEX(warpreturnx, "WarpReturnX", 3); break;	//b, 4 of these
case MAPDATAWARPRETY:	 	SET_MAPDATA_BYTE_INDEX(warpreturny, "WarpReturnY", 3); break;	//b, 4 of these
case MAPDATAWARPRETURNC: 	SET_MAPDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
case MAPDATASTAIRX: 		SET_MAPDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
case MAPDATASTAIRY: 		SET_MAPDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
case MAPDATAITEMX:		SET_MAPDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
case MAPDATAITEMY:		SET_MAPDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
case MAPDATACOLOUR: 		SET_MAPDATA_VAR_INT32(color, "CSet"); break;	//w
case MAPDATAENEMYFLAGS: 	SET_MAPDATA_VAR_BYTE(enemyflags, "EnemyFlags");	break;	//b
case MAPDATADOOR: 		SET_MAPDATA_BYTE_INDEX(door, "Door", 3); break;	//b, 4 of these
case MAPDATATILEWARPDMAP: 	SET_MAPDATA_VAR_INDEX32(tilewarpdmap, "TileWarpDMap", 3); break;	//w, 4 of these
case MAPDATATILEWARPSCREEN: 	SET_MAPDATA_BYTE_INDEX(tilewarpscr, "TileWarpScreen", 3); break;	//b, 4 of these
case MAPDATAEXITDIR: 		SET_MAPDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
case MAPDATAENEMY: 		SET_MAPDATA_VAR_INDEX32(enemy, "Enemy", 9); break;	//w, 10 of these
case MAPDATAPATTERN: 		SET_MAPDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
case MAPDATASIDEWARPTYPE: 	SET_MAPDATA_BYTE_INDEX(sidewarptype, "SideWarpType", 3); break;	//b, 4 of these
case MAPDATASIDEWARPOVFLAGS: 	SET_MAPDATA_VAR_BYTE(sidewarpoverlayflags, "SideWarpOverlayFlags"); break;	//b
case MAPDATAWARPARRIVALX: 	SET_MAPDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
case MAPDATAWARPARRIVALY: 	SET_MAPDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
case MAPDATAPATH: 		SET_MAPDATA_BYTE_INDEX(path, "MazePath", 3); break;	//b, 4 of these
case MAPDATASIDEWARPSC: 	SET_MAPDATA_BYTE_INDEX(sidewarpscr, "SideWarpScreen", 3); break;	//b, 4 of these
case MAPDATASIDEWARPDMAP: 	SET_MAPDATA_VAR_INDEX32(sidewarpdmap, "SideWarpDMap", 3); break;	//w, 4 of these
case MAPDATASIDEWARPINDEX: 	SET_MAPDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
case MAPDATAUNDERCOMBO: 	SET_MAPDATA_VAR_INT32(undercombo, "Undercombo"); break;	//w
case MAPDATAUNDERCSET:	 	SET_MAPDATA_VAR_BYTE(undercset,	"UnderCSet"); break; //b
case MAPDATACATCHALL:	 	SET_MAPDATA_VAR_INT32(catchall,	"Catchall"); break; //W

case MAPDATACSENSITIVE: 	SET_MAPDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
case MAPDATANORESET: 		SET_MAPDATA_VAR_INT32(noreset, "NoReset"); break;	//W
case MAPDATANOCARRY: 		SET_MAPDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
//! Layer arrays should be a size of 7, and return the current screen / map / and OP_OPAQUE 
//! if you try to read 0, so that they correspond to actual layer IDs. 
//! 
case MAPDATALAYERMAP:	 	SET_MAPDATA_BYTE_INDEX(layermap, "LayerMap", 5); break;	//B, 6 OF THESE
case MAPDATALAYERSCREEN: 	SET_MAPDATA_BYTE_INDEX(layerscreen, "LayerScreen", 5); break;	//B, 6 OF THESE
case MAPDATALAYEROPACITY: 	SET_MAPDATA_BYTE_INDEX(layeropacity, "LayerOpacity", 5); break;	//B, 6 OF THESE
case MAPDATATIMEDWARPTICS: 	SET_MAPDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
case MAPDATANEXTMAP: 		SET_MAPDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
case MAPDATANEXTSCREEN: 	SET_MAPDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
case MAPDATASECRETCOMBO: 	SET_MAPDATA_VAR_INDEX32(secretcombo, "SecretCombo", 127); break;	//W, 128 OF THESE
case MAPDATASECRETCSET: 	SET_MAPDATA_BYTE_INDEX(secretcset, "SecretCSet", 127); break;	//B, 128 OF THESE
case MAPDATASECRETFLAG: 	SET_MAPDATA_BYTE_INDEX(secretflag, "SecretFlags", 127); break;	//B, 128 OF THESE
case MAPDATAVIEWX: 		SET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
case MAPDATAVIEWY: 		SET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
case MAPDATASCREENWIDTH: 	SET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
case MAPDATASCREENHEIGHT: 	SET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
case MAPDATAENTRYX: 		SET_MAPDATA_VAR_BYTE(entry_x, "EntryX"); break;	//B
case MAPDATAENTRYY: 		SET_MAPDATA_VAR_BYTE(entry_y, "EntryY"); break;	//B
case MAPDATANUMFF: 		SET_MAPDATA_VAR_INT16(numff, "NumFFCs"); break;	//INT16
case MAPDATAFFDATA: 		SET_MAPDATA_VAR_INDEX32(ffdata,	"FFCData", 31); break;	//W, 32 OF THESE
case MAPDATAFFCSET: 		SET_MAPDATA_BYTE_INDEX(ffcset,	"FFCCSet", 31); break;	//B, 32
case MAPDATAFFDELAY: 		SET_MAPDATA_VAR_INDEX32(ffdelay, "FFCDelay", 31); break;	//W, 32
case MAPDATAFFX: 		SET_MAPDATA_VAR_INDEX32(ffx, "FFCX", 31); break;	//INT32, 32 OF THESE
case MAPDATAFFY: 		SET_MAPDATA_VAR_INDEX32(ffy, "FFCY", 31); break;	//..
case MAPDATAFFXDELTA:	 	SET_MAPDATA_VAR_INDEX32(ffxdelta, "FFCVx", 31); break;	//..
case MAPDATAFFYDELTA: 		SET_MAPDATA_VAR_INDEX32(ffydelta, "FFCVy", 31); break;	//..
case MAPDATAFFXDELTA2:	 	SET_MAPDATA_VAR_INDEX32(ffxdelta2, "FFCAx", 31); break;	//..
case MAPDATAFFYDELTA2:	 	SET_MAPDATA_VAR_INDEX32(ffydelta2, "FFCAy", 31); break;	//..
case MAPDATAFFFLAGS: 		SET_MAPDATA_VAR_INDEX16(ffflags, "FFCFlags", 31); break;	//INT16, 32 OF THESE
//Height and With are Or'd together, and need to be separate:
/*
 //TileWidth ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref] & ~63) | (((value/10000)-1)&63);
*/
case MAPDATAFFWIDTH: 		
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCTileWidth[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 || (value/10000) > 4 ) 
	{
		Z_scripterrlog("Invalid WIDTH value passed to MapData->FFCTileWidth[]: %d\n", value);
		break;
	}
	m->ffwidth[indx] = ( m->ffwidth[indx]&~63) | vbound( (((value/10000)-1)&63), (0&63), (214747&63) ); 
	break;
}	


//SET_MAPDATA_BYTE_INDEX(ffwidth, "FFCTileWidth");	//B, 32 OF THESE
case MAPDATAFFHEIGHT:	 	
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCTileHeight[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 || (value/10000) > 4 ) 
	{
		Z_scripterrlog("Invalid WIDTH value passed to MapData->FFCTileHeight[]: %d\n", value);
		break;
	}
	m->ffheight[indx] = ( m->ffheight[indx]&~63) | vbound( (((value/10000)-1)&63), (0&63), (4&63) ); 
	break;
	
}

//EffectWidth tmpscr->ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref]&63) | ((((value/10000)-1)&3)<<6);

//SET_MAPDATA_BYTE_INDEX(ffheight, "FFCTileHeight"	//B, 32 OF THESE
case MAPDATAFFEFFECTWIDTH: 		
{ 
	mapscr *m = &TheMaps[ri->mapsref]; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectWidth[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 ) 
	{
		Z_scripterrlog("Invalid WIDTH value passed to MapData->FFCEffectWidth[]: %d\n", value);
		break;
	}
	m->ffwidth[indx] = ( m->ffwidth[indx]&63) | vbound( ((((value/10000)-1)&3)<<6), ((((0)-1)&3)<<6), ((((214747)&3)<<6)) );
	break;
}


//SET_MAPDATA_BYTE_INDEX(ffwidth, "FFCEffectWidth");	//B, 32 OF THESE
case MAPDATAFFEFFECTHEIGHT:
{ 
	mapscr *m = &TheMaps[ri->mapsref]; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectHeight[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 ) 
	{
		Z_scripterrlog("Invalid HEIGHT value passed to MapData->FFCEffectHeight[]: %d\n", value);
		break;
	}
	m->ffheight[indx] = ( m->ffheight[indx]&63) | vbound( ((((value/10000)-1)&3)<<6), ((((0)-1)&3)<<6), ((((214747)&3)<<6)) );
	break;
}	
	
//SET_MAPDATA_BYTE_INDEX(ffheight, "FFCEffectHeight"	//B, 32 OF THESE	

case MAPDATAFFLINK: 		SET_MAPDATA_BYTE_INDEX(fflink, "FFCLink", 31); break;	//B, 32 OF THESE
case MAPDATAFFSCRIPT:	 	SET_MAPDATA_VAR_INDEX32(ffscript, "FFCScript", 31); break;	//W, 32 OF THESE

case MAPDATAINTID: 	 //Same form as SetScreenD()
	//SetFFCInitD(ffindex, d, value)
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	//int v = (value/10000);
	m->initd[(ri->d[0]/10000)][(ri->d[1]/10000)] = (value/10000);
	break;
}	
	

//initd	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 


case MAPDATAINITA: 		
	//same form as SetScreenD
{
	mapscr *m = &TheMaps[ri->mapsref]; 
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	//int v = (value/10000);
	m->inita[(ri->d[0]/10000)][(ri->d[1]/10000)] = (value/10000);
	break;
}	
	
case MAPDATAFFINITIALISED: 	SET_MAPDATA_BOOL_INDEX(initialized, "FFCRunning", 31); break;	//BOOL, 32 OF THESE
case MAPDATASCRIPTENTRY: 	SET_MAPDATA_VAR_INT32(script_entry, "ScriptEntry"); break;	//W
case MAPDATASCRIPTOCCUPANCY: 	SET_MAPDATA_VAR_INT32(script_occupancy,	"ScriptOccupancy");  break;//W
case MAPDATASCRIPTEXIT: 	SET_MAPDATA_VAR_INT32(script_exit, "ExitScript"); break;	//W
case MAPDATAOCEANSFX:	 	SET_MAPDATA_VAR_BYTE(oceansfx, "OceanSFX"); break;	//B
case MAPDATABOSSSFX: 		SET_MAPDATA_VAR_BYTE(bosssfx, "BossSFX"); break;	//B
case MAPDATASECRETSFX:	 	SET_MAPDATA_VAR_BYTE(secretsfx, "SecretSFX"); break;	//B
case MAPDATAHOLDUPSFX:	 	SET_MAPDATA_VAR_BYTE(holdupsfx,	"ItemSFX"); break; //B
case MAPDATASCREENMIDI: 	SET_MAPDATA_VAR_INT16(screen_midi, "MIDI"); break;	//SHORT, OLD QUESTS ONLY?
case MAPDATALENSLAYER:	 	SET_MAPDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
	

case MAPDATAFLAGS: 
{
	int flagid = (ri->d[0])/10000;
	mapscr *m = &TheMaps[ri->mapsref]; 
	//bool valtrue = ( value ? 10000 : 0);
	switch(flagid)
	{
		case 0: m->flags = (value / 10000); break;
		case 1: m->flags2 = (value / 10000); break;
		case 2: m->flags3 = (value / 10000); break;
		case 3: m->flags4 = (value / 10000); break;
		case 4: m->flags5 = (value / 10000); break;
		case 5: m->flags6 = (value / 10000); break;
		case 6: m->flags7 = (value / 10000); break;
		case 7: m->flags8 = (value / 10000); break;
		case 8: m->flags9 = (value / 10000); break;
		case 9: m->flags10 = (value / 10000); break;
		default:
		{
			Z_scripterrlog("Invalid index passed to mapdata->flags[]: %d\n", flagid); 
			break;
			
		}
	}
	break;
	//SET_MAPDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10
}

case MAPDATAMISCD:
{
	int indx = (ri->d[0])/10000;
	if(indx < 0 || indx > 7)
	{
		Z_scripterrlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", indx);
		break;
	}
	else 
	{
		game->screen_d[ri->mapsref][indx] = value/10000;
		break;
	}
}


  case MAPDATACOMBODD:
    {
        int pos = (ri->d[0])/10000;
        mapscr *m = &TheMaps[ri->mapsref];
        if(pos >= 0 && pos < 176)
        {
            screen_combo_modify_preroutine(m,pos);
            m->data[pos]=(value/10000);
            screen_combo_modify_postroutine(m,pos);
        }
    }
    break;
    
    case MAPDATACOMBOCD:
    {
        int pos = (ri->d[0])/10000;
        mapscr *m = &TheMaps[ri->mapsref];
        if(pos >= 0 && pos < 176)
        {
            screen_combo_modify_preroutine(m,pos);
            m->cset[pos]=(value/10000)&15;
            screen_combo_modify_postroutine(m,pos);
        }
    }
    break;
    
    case MAPDATACOMBOFD:
    {
        int pos = (ri->d[0])/10000;
        mapscr *m = &TheMaps[ri->mapsref];
        if(pos >= 0 && pos < 176)
            m->sflag[pos]=(value/10000);
    }
    break;
    
    case MAPDATACOMBOTD:
    {
        int pos = (ri->d[0])/10000;
        mapscr *m = &TheMaps[ri->mapsref];
        if(pos >= 0 && pos < 176)
        {
            // Preprocess each instance of the combo on the screen
            for(int i = 0; i < 176; i++)
            {
                if(m->data[i] == m->data[pos])
                {
                    screen_combo_modify_preroutine(m,i);
                }
            }
            
            combobuf[m->data[pos]].type=value/10000;
            
            for(int i = 0; i < 176; i++)
            {
                if(m->data[i] == m->data[pos])
                {
                    screen_combo_modify_postroutine(m,i);
                }
            }
        }
    }
    break;
    
    case MAPDATACOMBOID:
    {
        int pos = (ri->d[0])/10000;
        mapscr *m = &TheMaps[ri->mapsref];
        if(pos >= 0 && pos < 176)
            combobuf[m->data[pos]].flag=value/10000;
    }
    break;
    
    case MAPDATACOMBOSD:
    {
        int pos = (ri->d[0])/10000;
        mapscr *m = &TheMaps[ri->mapsref];
        if(pos >= 0 && pos < 176)
            combobuf[m->data[pos]].walk=(value/10000)&15;
    }
    break;

    case MAPDATASCREENSTATED:
    {
        (value)?setmapflag(ri->mapsref, 1<<((ri->d[0])/10000)) : unsetmapflag(ri->mapsref, 1 << ((ri->d[0]) / 10000));
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//shopdata sd-> Variables
	
	case SHOPDATAITEM: 
	{ 
		
		int ref = ri->shopsref; 
		bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
		int indx = ri->d[0] / 10000; 
		if ( indx < 0 || indx > 2 ) 
		{ 
			Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "Item"); 
		} 
		else 
		{ 
			if ( isInfo ) 
			{ 
				Z_scripterrlog("Attempted to write an 'item' to an infoshop, using shop ID: %d\n", ri->shopsref); 
			} 
			else 
			{ 
				QMisc.shop[ref].item[indx] = (byte)(vbound((value/10000), 0, 255)); 
			} 
		} 
	} 
	break;
//SET_SHOPDATA_VAR_INDEX(item, "Item", 2); break;
	case SHOPDATAHASITEM: 
	{ 
		
		int ref = ri->shopsref; 
		bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
		int indx = ri->d[0] / 10000; 
		if ( indx < 0 || indx > 2 ) 
		{ 
			Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "HasItem"); 
		} 
		else 
		{ 
			if ( isInfo ) 
			{ 
				Z_scripterrlog("Attempted to write 'hasitem' to an infoshop, using shop ID: %d\n", ri->shopsref); 
			} 
			else 
			{ 
				QMisc.shop[ref].hasitem[indx] = (byte)(vbound((value/10000), 0, 255)); 
			} 
		} 
	} 
	break;
//SET_SHOPDATA_VAR_INDEX(hasitem, "HasItem", 2); break;
	case SHOPDATAPRICE: 
	{ 
		
		int ref = ri->shopsref; 
		bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
		int indx = ri->d[0] / 10000; 
		if ( indx < 0 || indx > 2 ) 
		{ 
			Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "Price"); 
		} 
		else 
		{ 
			if ( isInfo ) 
			{ 
				QMisc.shop[ref].price[indx] = (byte)(vbound((value/10000), 0, 214747));
			} 
			else 
			{ 
				QMisc.shop[ref].price[indx] = (byte)(vbound((value/10000), 0, 214747));
			} 
		} 
	} 
//SET_SHOPDATA_VAR_INDEX(price, "Price", 2); break;
//Pay for info
case SHOPDATASTRING:
{
	{
		if ( ri->shopsref < NUMSHOPS || ri->shopsref > NUMINFOSHOPS )
		{
			Z_scripterrlog("Invalid Info Shop ID passed to shopdata->String[]: %d\n", ri->shopsref); 
			break;
		}
		else 
		{
			int ref = ri->shopsref; 
			bool isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); 
			int indx = ri->d[0] / 10000; 
			if ( indx < 0 || indx > 2 ) 
			{ 
				Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, "HasItem"); 
			} 
			else 
			{ 
				if ( isInfo ) 
				{ 
					QMisc.info[ref].str[indx] = (word)(vbound((value/10000), 0, 32767));
				} 
				else 
				{ 
					QMisc.shop[ref].str[indx] = (word)(vbound((value/10000), 0, 32767));
				} 
			} 
	
	
		}
			
		//GET_SHOPDATA_VAR_INDEX(str, String, 2); break;
	} break;
}

///----------------------------------------------------------------------------------------------------//
//dmapdata dmd-> Variables
case DMAPDATAMAP: 	//byte
{
	DMaps[ri->dmapsref].map = ((byte)(value / 10000)); break;
}
case DMAPDATALEVEL:	//word
{
	DMaps[ri->dmapsref].level = ((word)(value / 10000)); break;
}
case DMAPDATAOFFSET:	//char
{
	DMaps[ri->dmapsref].xoff = ((char)(value / 10000)); break;
}
case DMAPDATACOMPASS:	//byte
{
	DMaps[ri->dmapsref].compass = ((byte)(value / 10000)); break;
}
case DMAPDATAPALETTE:	//word
{
	DMaps[ri->dmapsref].color= ((word)(value / 10000)); break;
}
case DMAPDATAMIDI:	//byte
{
	DMaps[ri->dmapsref].midi = ((byte)(value / 10000)); break;
}
case DMAPDATACONTINUE:	//byte
{
	DMaps[ri->dmapsref].cont = ((byte)(value / 10000)); break;
}
case DMAPDATATYPE:	//byte
{
	DMaps[ri->dmapsref].type = ((byte)(value / 10000)); break;
}
case DMAPDATASIDEVIEW:	//byte, treat as bool
{
	DMaps[ri->dmapsref].type = ((byte)((value / 10000)!=0 ? 1 : 0)); break;
}
case DMAPDATAGRID:	//byte[8] --array
{
	int indx = ri->d[0] / 10000;
	if ( indx < 0 || indx > 7 ) 
	{
		Z_scripterrlog("Invalid index supplied to dmapdata->Grid[]: %d\n", indx); break;
	}
	else
	{
		DMaps[ri->dmapsref].grid[indx] = ((byte)(value / 10000)); break;
	}
}
case DMAPDATAMINIMAPTILE:	//word - two of these, so let's do MinimapTile[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { DMaps[ri->dmapsref].minimap_1_tile = ((word)(value / 10000)); break; }
		case 1: { DMaps[ri->dmapsref].minimap_2_tile = ((word)(value / 10000)); break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->MiniMapTile[]: %d\n", indx);
			break;
		}
	}
}
case DMAPDATAMINIMAPCSET:	//byte - two of these, so let's do MinimapCSet[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { DMaps[ri->dmapsref].minimap_1_cset= ((byte)(value / 10000)); break; }
		case 1: { DMaps[ri->dmapsref].minimap_2_cset= ((byte)(value / 10000)); break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->MiniMapCSet[]: %d\n", indx);
			break;
		}
	}
}
case DMAPDATALARGEMAPTILE:	//word -- two of these, so let's to LargemapTile[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { DMaps[ri->dmapsref].largemap_1_tile = ((word)(value / 10000)); break; }
		case 1: { DMaps[ri->dmapsref].largemap_2_tile = ((word)(value / 10000)); break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->LargeMapTile[]: %d\n", indx);
			break;
		}
	}
}
case DMAPDATALARGEMAPCSET:	//word -- two of these, so let's to LargemaCSet[2]
{
	int indx = ri->d[0] / 10000;
	switch(indx)
	{
		case 0: { DMaps[ri->dmapsref].largemap_1_cset= ((byte)(value / 10000)); break; }
		case 1: { DMaps[ri->dmapsref].largemap_2_cset= ((byte)(value / 10000)); break; }
		default: 
		{
			Z_scripterrlog("Invalid index supplied to dmapdata->LargeMapCSet[]: %d\n", indx);
			break;
		}
	}
}
case DMAPDATAMUISCTRACK:	//byte
{
	DMaps[ri->dmapsref].tmusictrack= ((byte)(value / 10000)); break;
}
case DMAPDATASUBSCRA:	 //byte, active subscreen
{
	DMaps[ri->dmapsref].active_subscreen= ((byte)(value / 10000)); break;
}
case DMAPDATASUBSCRP:	 //byte, passive subscreen
{
	DMaps[ri->dmapsref].passive_subscreen= ((byte)(value / 10000)); break;
}
case DMAPDATADISABLEDITEMS:	 //byte[iMax]
{
	int indx = ri->d[0] / 10000;
	if ( indx < 0 || indx > (iMax-1) ) 
	{
		Z_scripterrlog("Invalid index supplied to dmapdata->Grid[]: %d\n", indx); break;
	}
	else
	{
		DMaps[ri->dmapsref].disableditems[indx] = ((byte)(value / 10000)); break;
	}
}
case DMAPDATAFLAGS:	 //long
{
	DMaps[ri->dmapsref].flags= ((byte)(value / 10000)); break;
}
//case DMAPDATAGRAVITY:	 //unimplemented
//case DMAPDATAJUMPLAYER:	 //unimplemented

///----------------------------------------------------------------------------------------------------//
//messagedata msgd-> Variables
 

case MESSAGEDATANEXT: //W
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Next") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].nextstring = vbound((value/10000), 0, (msg_count-1));
	break;
}	

case MESSAGEDATATILE: //W
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Tile") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].tile = vbound((value/10000), 0, (NEWMAXTILES));
	break;
}	

case MESSAGEDATACSET: //b
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->CSet") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].cset = ((byte)vbound((value/10000), 0, 255));
	break;
}	
case MESSAGEDATATRANS: //BOOL
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Transparent") != SH::_NoError)
		break;
	else 
		(MsgStrings[ID].trans) = ((value)?true:false);
	break;
}	
case MESSAGEDATAFONT: //B
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Font") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].font = ((byte)vbound((value/10000), 0, 255));
	break;
}	
case MESSAGEDATAX: //SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->X") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].x = ((short)vbound((value/10000), SHRT_MIN, SHRT_MAX));
	break;
}	
case MESSAGEDATAY: //SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Y") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].y = ((short)vbound((value/10000), SHRT_MIN, SHRT_MAX));
	break;
}	
case MESSAGEDATAW: //UNSIGNED SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Width") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].w = ((unsigned short)vbound((value/10000), 0, USHRT_MAX));
	break;
}	
case MESSAGEDATAH: //UNSIGNED SHORT
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Height") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].h = ((unsigned short)vbound((value/10000), 0, USHRT_MAX));
	break;
}	
case MESSAGEDATASFX: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Sound") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].sfx = ((byte)vbound((value/10000), 0, 255));
	break;
}	
case MESSAGEDATALISTPOS: //WORD
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->ListPosition") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].listpos = vbound((value/10000), 1, (msg_count-1));
	break;
}	
case MESSAGEDATAVSPACE: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->VSpace") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].vspace = ((byte)vbound((value/10000), 0, 255));
	break;
}	
case MESSAGEDATAHSPACE: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->HSpace") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].hspace = ((byte)vbound((value/10000), 0, 255));
	break;
}	
case MESSAGEDATAFLAGS: //BYTE
{
	long ID = ri->zmsgref;	

	if(BC::checkMessage(ID, "messagedata->Flags") != SH::_NoError)
		break;
	else 
		MsgStrings[ID].stringflags = ((byte)vbound((value/10000), 0, 255));
	break;
}	


///----------------------------------------------------------------------------------------------------//
//combodata cd-> Setter Variables
//newcombo	
#define	SET_COMBO_VAR_INT(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			combobuf[ri->combosref].member = vbound((value / 10000),0,214747); \
		} \
	} \
	
	#define	SET_COMBO_VAR_DWORD(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			combobuf[ri->combosref].member = vbound((value / 10000),0,32767); \
		} \
	} \

	#define	SET_COMBO_VAR_BYTE(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			combobuf[ri->combosref].member = vbound((value / 10000),0,255); \
		} \
	} \
	
	#define SET_COMBO_VAR_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				combobuf[ri->combosref].member[indx] = vbound((value / 10000),0,214747); \
			} \
	}

	#define SET_COMBO_BYTE_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				combobuf[ri->combosref].member[indx] = vbound((value / 10000),0,255); \
			} \
	}
	
	#define SET_COMBO_FLAG(member, str) \
	{ \
		long flag =  (value/10000);  \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			if ( flag != 0 ) \
			{ \
				combobuf[ri->combosref].member|=flag; \
			} \
			else combobuf[ri->combosref].member|= ~flag; \
		} \
	} \
	
	//comboclass
#define	SET_COMBOCLASS_VAR_INT(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			combo_class_buf[combobuf[ri->combosref].type].member = vbound((value / 10000),0,214747); \
		} \
	} \
	
	#define	SET_COMBOCLASS_VAR_DWORD(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			combo_class_buf[combobuf[ri->combosref].type].member = vbound((value / 10000),0,32767); \
		} \
	} \

	#define	SET_COMBOCLASS_VAR_BYTE(member, str) \
	{ \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			combo_class_buf[combobuf[ri->combosref].type].member = vbound((value / 10000),0,255); \
		} \
	} \
	
	#define SET_COMBOCLASS_VAR_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				combo_class_buf[combobuf[ri->combosref].type].member[indx] = vbound((value / 10000),0,214747); \
			} \
	}

	#define SET_COMBOCLASS_BYTE_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
			{ \
				Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to combodata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				combo_class_buf[combobuf[ri->combosref].type].member[indx] = vbound((value / 10000),0,255); \
			} \
	}
	
	#define SET_COMBOCLASS_FLAG(member, str) \
	{ \
		long flag =  (value/10000);  \
		if(ri->combosref < 0 || ri->combosref > (MAXCOMBOS-1) ) \
		{ \
			Z_scripterrlog("Invalid Combo ID passed to combodata->%s: %d\n", (ri->combosref*10000), str); \
		} \
		else \
		{ \
			if ( flag != 0 ) \
			{ \
				combo_class_buf[combobuf[ri->combosref].type].member|=flag; \
			} \
			else combo_class_buf[combobuf[ri->combosref].type].member|= ~flag; \
		} \
	} \
	
//NEWCOMBO STRUCT
case COMBODTILE:	SET_COMBO_VAR_DWORD(tile, "Tile"); break;						//word
case COMBODFLIP:	SET_COMBO_VAR_BYTE(flip, "Flip"); break;						//char
case COMBODWALK:	SET_COMBO_VAR_BYTE(walk, "Walk"); break;						//char
case COMBODTYPE:	SET_COMBO_VAR_BYTE(type, "Type"); break;						//char
case COMBODCSET:	SET_COMBO_VAR_BYTE(csets, "CSet"); break;						//C
case COMBODFOO:		SET_COMBO_VAR_DWORD(foo, "Foo"); break;							//W
case COMBODFRAMES:	SET_COMBO_VAR_BYTE(frames, "Frames"); break;						//C
case COMBODNEXTD:	SET_COMBO_VAR_DWORD(speed, "NextData"); break;						//W
case COMBODNEXTC:	SET_COMBO_VAR_BYTE(nextcombo, "NextCSet"); break;					//C
case COMBODFLAG:	SET_COMBO_VAR_BYTE(nextcset, "Flag"); break;						//C
case COMBODSKIPANIM:	SET_COMBO_VAR_BYTE(skipanim, "SkipAnim"); break;					//C
case COMBODNEXTTIMER:	SET_COMBO_VAR_DWORD(nexttimer, "NextTimer"); break;					//W
case COMBODAKIMANIMY:	SET_COMBO_VAR_BYTE(skipanimy, "SkipAnimY"); break;					//C
case COMBODANIMFLAGS:	SET_COMBO_VAR_BYTE(animflags, "AnimFlags"); break;					//C
case COMBODEXPANSION:	SET_COMBO_BYTE_INDEX(expansion, "Expansion[]", 6);					//C , 6 INDICES
case COMBODATTRIBUTES: 	SET_COMBO_VAR_INDEX(attributes,	"Attributes[]", 4); 					//LONG, 4 INDICES, INDIVIDUAL VALUES
case COMBODUSRFLAGS:	SET_COMBO_VAR_INT(usrflags, "UserFlags"); break;					//LONG
case COMBODTRIGGERFLAGS:	SET_COMBO_VAR_INDEX(triggerflags, "TriggerFlags[]", 3);				//LONG 3 INDICES AS FLAGSETS
case COMBODTRIGGERLEVEL:	SET_COMBO_VAR_INT(triggerlevel, "TriggerLevel"); break;				//LONG

//COMBOCLASS STRUCT
//case COMBODNAME:		//CHAR[64], STRING
case COMBODBLOCKNPC:		SET_COMBOCLASS_VAR_BYTE(block_enemies, "BlockNPC"); break;			//C
case COMBODBLOCKHOLE:		SET_COMBOCLASS_VAR_BYTE(block_hole, "BlockHole"); break;			//C
case COMBODBLOCKTRIG:		SET_COMBOCLASS_VAR_BYTE(block_trigger,	"BlockTrigger"); break; 		//C
case COMBODBLOCKWEAPON:		SET_COMBOCLASS_BYTE_INDEX(block_weapon,	"BlockWeapon[]", 32); 			//C, 32 INDICES
case COMBODCONVXSPEED:		SET_COMBOCLASS_VAR_DWORD(conveyor_x_speed, "ConveyorSpeedX"); break;		//SHORT
case COMBODCONVYSPEED:		SET_COMBOCLASS_VAR_DWORD(conveyor_y_speed, "ConveyorSpeedY"); break;		//SHORT
case COMBODSPAWNNPC:		SET_COMBOCLASS_VAR_DWORD(create_enemy, "SpawnNPC"); break;			//W
case COMBODSPAWNNPCWHEN:	SET_COMBOCLASS_VAR_BYTE(create_enemy_when, "SpawnNPCWhen"); break;		//C
case COMBODSPAWNNPCCHANGE:	SET_COMBOCLASS_VAR_INT(create_enemy_change, "SpawnNPCChange"); break;		//LONG
case COMBODDIRCHANGETYPE:	SET_COMBOCLASS_VAR_BYTE(directional_change_type, "DirChange"); break;		//C
case COMBODDISTANCECHANGETILES:	SET_COMBOCLASS_VAR_INT(distance_change_tiles, "DistanceChangeTiles"); break; 	//LONG
case COMBODDIVEITEM:		SET_COMBOCLASS_VAR_DWORD(dive_item, "DiveItem"); break;				//SHORT
case COMBODDOCK:		SET_COMBOCLASS_VAR_BYTE(dock, "Dock"); break;					//C
case COMBODFAIRY:		SET_COMBOCLASS_VAR_BYTE(fairy, "Fairy"); break;					//C
case COMBODFFATTRCHANGE:	SET_COMBOCLASS_VAR_BYTE(ff_combo_attr_change, "FFCAttributeChange"); break;	//C
case COMBODFOORDECOTILE:	SET_COMBOCLASS_VAR_INT(foot_decorations_tile, "DecorationTile"); break;		//LONG
case COMBODFOORDECOTYPE:	SET_COMBOCLASS_VAR_BYTE(foot_decorations_type, "DecorationType"); break;	//C
case COMBODHOOKSHOTPOINT:	SET_COMBOCLASS_VAR_BYTE(hookshot_grab_point, "Hookshot"); break;		//C
case COMBODLADDERPASS:		SET_COMBOCLASS_VAR_BYTE(ladder_pass, "Ladder"); break;				//C
case COMBODLOCKBLOCK:		SET_COMBOCLASS_VAR_BYTE(lock_block_type, "LockBlock"); break;			//C
case COMBODLOCKBLOCKCHANGE:	SET_COMBOCLASS_VAR_INT(lock_block_change, "LockBlockChange"); break;		//LONG
case COMBODMAGICMIRROR:		SET_COMBOCLASS_VAR_BYTE(magic_mirror_type, "Mirror"); break;			//C
case COMBODMODHPAMOUNT:		SET_COMBOCLASS_VAR_DWORD(modify_hp_amount, "DamageAmount"); break;		//SHORT
case COMBODMODHPDELAY:		SET_COMBOCLASS_VAR_BYTE(modify_hp_delay, "DamageDelay"); break;			//C
case COMBODMODHPTYPE:		SET_COMBOCLASS_VAR_BYTE(modify_hp_type,	"DamageType"); break; 			//C
case COMBODNMODMPAMOUNT:	SET_COMBOCLASS_VAR_DWORD(modify_mp_amount, "MagicAmount"); break;		//SHORT
case COMBODMODMPDELAY:		SET_COMBOCLASS_VAR_BYTE(modify_mp_delay, "MagicDelay"); break;			//C
case COMBODMODMPTYPE:		SET_COMBOCLASS_VAR_BYTE(modify_mp_type,	"MagicType"); 				//C
case COMBODNOPUSHBLOCK:		SET_COMBOCLASS_VAR_BYTE(no_push_blocks, "NoPushBlocks"); break;			//C
case COMBODOVERHEAD:		SET_COMBOCLASS_VAR_BYTE(overhead, "Overhead"); break;				//C
case COMBODPLACENPC:		SET_COMBOCLASS_VAR_BYTE(place_enemy, "PlaceNPC"); break;			//C
case COMBODPUSHDIR:		SET_COMBOCLASS_VAR_BYTE(push_direction,	"PushDir"); break; 			//C
case COMBODPUSHWAIT:		SET_COMBOCLASS_VAR_BYTE(push_wait, "PushDelay"); break;				//C
case COMBODPUSHHEAVY:		SET_COMBOCLASS_VAR_BYTE(push_weight, "PushHeavy");				//C
case COMBODPUSHED:		SET_COMBOCLASS_VAR_BYTE(pushed, "Pushed"); break;				//C
case COMBODRAFT:		SET_COMBOCLASS_VAR_BYTE(raft, "Raft"); break;					//C
case COMBODRESETROOM:		SET_COMBOCLASS_VAR_BYTE(reset_room, "ResetRoom"); break;			//C
case COMBODSAVEPOINTTYPE:	SET_COMBOCLASS_VAR_BYTE(save_point_type, "SavePoint"); break;			//C
case COMBODSCREENFREEZETYPE:	SET_COMBOCLASS_VAR_BYTE(screen_freeze_type, "FreezeScreen"); break;		//C
case COMBODSECRETCOMBO:		SET_COMBOCLASS_VAR_BYTE(secret_combo, "SecretCombo"); break;			//C
case COMBODSINGULAR:		SET_COMBOCLASS_VAR_BYTE(singular, "Singular"); break;				//C
case COMBODSLOWWALK:		SET_COMBOCLASS_VAR_BYTE(slow_movement, "SlowWalk"); break;			//C
case COMBODSTATUETYPE:		SET_COMBOCLASS_VAR_BYTE(statue_type, "Statue"); break;				//C
case COMBODSTEPTYPE:		SET_COMBOCLASS_VAR_BYTE(step_type, "Step"); break;				//C
case COMBODSTEPCHANGEINTO:	SET_COMBOCLASS_VAR_INT(step_change_to, "StepChange"); break;			//LONG
case COMBODSTRIKEWEAPONS:	SET_COMBOCLASS_BYTE_INDEX(strike_weapons, "Strike[]", 32);			//BYTE, 32 INDICES. 
case COMBODSTRIKEREMNANTS:	SET_COMBOCLASS_VAR_INT(strike_remnants,	"StrikeRemnants"); break;		//LONG
case COMBODSTRIKEREMNANTSTYPE:	SET_COMBOCLASS_VAR_BYTE(strike_remnants_type, "StrikeRemnantsType"); break;	//C
case COMBODSTRIKECHANGE:	SET_COMBOCLASS_VAR_INT(strike_change, "StrikeChange"); break;			//LONG
case COMBODSTRIKEITEM:		SET_COMBOCLASS_VAR_DWORD(strike_item, "StrikeItem"); break;			//SHORT
case COMBODTOUCHITEM:		SET_COMBOCLASS_VAR_DWORD(touch_item, "TouchItem"); break;			//SHORT
case COMBODTOUCHSTAIRS:		SET_COMBOCLASS_VAR_BYTE(touch_stairs, "TouchStairs"); break;			//C
case COMBODTRIGGERTYPE:		SET_COMBOCLASS_VAR_BYTE(trigger_type, "TriggerType"); break;			//C
case COMBODTRIGGERSENS:		SET_COMBOCLASS_VAR_BYTE(trigger_sensitive, "TriggerSensitivity"); break;	//C
case COMBODWARPTYPE:		SET_COMBOCLASS_VAR_BYTE(warp_type, "Warp"); break;				//C
case COMBODWARPSENS:		SET_COMBOCLASS_VAR_BYTE(warp_sensitive,	"WarpSensitivity"); break; 		//C
case COMBODWARPDIRECT:		SET_COMBOCLASS_VAR_BYTE(warp_direct, "WarpDirect"); break;			//C
case COMBODWARPLOCATION:	SET_COMBOCLASS_VAR_BYTE(warp_location, "WarpLocation"); break;			//C
case COMBODWATER:		SET_COMBOCLASS_VAR_BYTE(water, "Water"); break;					//C
case COMBODWHISTLE:		SET_COMBOCLASS_VAR_BYTE(whistle, "Whistle"); break;				//C
case COMBODWINGAME:		SET_COMBOCLASS_VAR_BYTE(win_game, "WinGame"); break; 				//C
case COMBODBLOCKWPNLEVEL:	SET_COMBOCLASS_VAR_BYTE(block_weapon_lvl, "BlockWeaponLevel"); break;		//C



///----------------------------------------------------------------------------------------------------//
//npcdata nd-> Variables
	
#define	SET_NPCDATA_VAR_INT(member, str) \
	{ \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
		} \
		else \
		{ \
			guysbuf[ri->npcdataref].member = vbound((value / 10000),0,214747); \
		} \
	} \
	
	#define	SET_NPCDATA_VAR_DWORD(member, str) \
	{ \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
		} \
		else \
		{ \
			guysbuf[ri->npcdataref].member = vbound((value / 10000),0,32767); \
		} \
	} \

	#define	SET_NPCDATA_VAR_BYTE(member, str) \
	{ \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
		} \
		else \
		{ \
			guysbuf[ri->npcdataref].member = vbound((value / 10000),0,255); \
		} \
	} \
	
	#define SET_NPCDATA_VAR_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to npcdata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				guysbuf[ri->npcdataref].member[indx] = vbound((value / 10000),0,214747); \
			} \
	}

	#define SET_NPCDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
			int indx = ri->d[0] / 10000; \
			if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
			{ \
				Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
			} \
			else if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to npcdata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				guysbuf[ri->npcdataref].member[indx] = vbound((value / 10000),0,255); \
			} \
	}
	
	#define SET_NPCDATA_FLAG(member, str) \
	{ \
		long flag =  (value/10000);  \
		if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) \
		{ \
			Z_scripterrlog("Invalid NPC ID passed to npcdata->%s: %d\n", (ri->npcdataref*10000), str); \
		} \
		else \
		{ \
			if ( flag != 0 ) \
			{ \
				guysbuf[ri->npcdataref].member|=flag; \
			} \
			else guysbuf[ri->npcdataref].member|= ~flag; \
		} \
	} \
	
case NPCDATATILE: SET_NPCDATA_VAR_BYTE(tile, "Tile"); break;
case NPCDATAWIDTH: SET_NPCDATA_VAR_BYTE(width, "Width"); break;
case NPCDATAHEIGHT: SET_NPCDATA_VAR_BYTE(height, "Height"); break;
case NPCDATAFLAGS: SET_NPCDATA_VAR_DWORD(flags, "Flags"); break; //16 b its
case NPCDATAFLAGS2: SET_NPCDATA_VAR_DWORD(flags2, "Flags2"); break; //16 bits
case NPCDATASTILE: SET_NPCDATA_VAR_BYTE(s_tile, "STile"); break;
case NPCDATASWIDTH: SET_NPCDATA_VAR_BYTE(s_width, "SWidth"); break;
case NPCDATASHEIGHT: SET_NPCDATA_VAR_BYTE(s_height, "SHeight"); break;
case NPCDATAETILE: SET_NPCDATA_VAR_INT(e_tile, "ExTile"); break;
case NPCDATAEWIDTH: SET_NPCDATA_VAR_BYTE(e_width, "ExWidth"); break;
case NPCDATAEHEIGHT: SET_NPCDATA_VAR_BYTE(e_height, "ExHeight"); break;
case NPCDATAHP: SET_NPCDATA_VAR_DWORD(hp, "HP"); break;
case NPCDATAFAMILY: SET_NPCDATA_VAR_DWORD(family, "Family"); break;
case NPCDATACSET: SET_NPCDATA_VAR_DWORD(cset, "CSet"); break;
case NPCDATAANIM: SET_NPCDATA_VAR_DWORD(anim, "Anim"); break;
case NPCDATAEANIM: SET_NPCDATA_VAR_DWORD(e_anim, "ExAnim"); break;
case NPCDATAFRAMERATE: SET_NPCDATA_VAR_DWORD(frate, "Framerate"); break;
case NPCDATAEFRAMERATE: SET_NPCDATA_VAR_DWORD(e_frate, "ExFramerate"); break;
case NPCDATATOUCHDAMAGE: SET_NPCDATA_VAR_DWORD(dp, "TouchDamage"); break;
case NPCDATAWEAPONDAMAGE: SET_NPCDATA_VAR_DWORD(wdp, "WeaponDamage"); break;
case NPCDATAWEAPON: SET_NPCDATA_VAR_DWORD(weapon, "Weapon"); break;
case NPCDATARANDOM: SET_NPCDATA_VAR_DWORD(rate, "Random"); break;
case NPCDATAHALT: SET_NPCDATA_VAR_DWORD(hrate, "Haltrate"); break;
case NPCDATASTEP: SET_NPCDATA_VAR_DWORD(step, "Step"); break;
case NPCDATAHOMING: SET_NPCDATA_VAR_DWORD(homing, "Homing"); break;
case NPCDATAHUNGER: SET_NPCDATA_VAR_DWORD(grumble, "Hunger"); break;
case NPCDATADROPSET: SET_NPCDATA_VAR_DWORD(item_set, "Dropset"); break;
case NPCDATABGSFX: SET_NPCDATA_VAR_DWORD(bgsfx, "BGSFX"); break;
case NPCDATADEATHSFX: SET_NPCDATA_VAR_BYTE(deadsfx, "DeathSFX"); break;
case NPCDATAHITSFX: SET_NPCDATA_VAR_BYTE(hitsfx, "HitSFX"); break;
case NPCDATAXOFS: SET_NPCDATA_VAR_INT(xofs, "DrawXOffset"); break;
case NPCDATAYOFS: SET_NPCDATA_VAR_INT(yofs, "DrawYOffset"); break;
case NPCDATAZOFS: SET_NPCDATA_VAR_INT(zofs, "DrawZOffset"); break;
case NPCDATAHXOFS: SET_NPCDATA_VAR_INT(hxofs, "HitXOffset"); break;
case NPCDATAHYOFS: SET_NPCDATA_VAR_INT(hyofs, "HitYOffset"); break;
case NPCDATAHITWIDTH: SET_NPCDATA_VAR_INT(hxsz, "HitWidth"); break;
case NPCDATAHITHEIGHT: SET_NPCDATA_VAR_INT(hysz, "HitHeight"); break;
case NPCDATAHITZ: SET_NPCDATA_VAR_INT(hzsz, "HitZHeight"); break;
case NPCDATATILEWIDTH: SET_NPCDATA_VAR_INT(txsz, "TileWidth"); break;
case NPCDATATILEHEIGHT: SET_NPCDATA_VAR_INT(tysz, "TileHeight"); break;
case NPCDATAWPNSPRITE: SET_NPCDATA_VAR_INT(wpnsprite, "WeaponSprite"); break;
case NPCDATADEFENSE: SET_NPCDATA_VAR_INDEX(defense, "Defense", 42); break;
case NPCDATASIZEFLAG: SET_NPCDATA_VAR_INT(SIZEflags, "SizeFlags"); break;

case NPCDATAFROZENTILE: SET_NPCDATA_VAR_INT(frozentile, "FrozenTile"); break;
case NPCDATAFROZENCSET: SET_NPCDATA_VAR_INT(frozencset, "FrozenCSet"); break;

case NPCDATAATTRIBUTE: 
{
	int indx = ri->d[0] / 10000; 
	if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) 
	{
		Z_scripterrlog("Invalid Sprite ID passed to npcdata->Attributes[]: %d\n", (ri->npcdataref*10000)); 
	}
	else if ( indx < 0 || indx > MAX_NPC_ATRIBUTES )
	{ 
		Z_scripterrlog("Invalid Array Index passed to npcdata->Attributes[]: %d\n", (ri->npcdataref*10000)); 
	} 
	else 
	{ 
		switch(indx)
		{
			case 0: guysbuf[ri->npcdataref].misc1 = (value / 10000); break;
			case 1: guysbuf[ri->npcdataref].misc2 = (value / 10000); break;
			case 2: guysbuf[ri->npcdataref].misc3 = (value / 10000); break;
			case 3: guysbuf[ri->npcdataref].misc4 = (value / 10000); break;
			case 4: guysbuf[ri->npcdataref].misc5 = (value / 10000); break;
			case 5: guysbuf[ri->npcdataref].misc6 = (value / 10000); break;
			case 6: guysbuf[ri->npcdataref].misc7 = (value / 10000); break;
			case 7: guysbuf[ri->npcdataref].misc8 = (value / 10000); break;
			case 8: guysbuf[ri->npcdataref].misc9 = (value / 10000); break;
			case 9: guysbuf[ri->npcdataref].misc10 = (value / 10000); break;
			case 10: guysbuf[ri->npcdataref].misc11 = (value / 10000); break;
			case 11: guysbuf[ri->npcdataref].misc12 = (value / 10000); break;
			case 12: guysbuf[ri->npcdataref].misc13 = (value / 10000); break;
			case 13: guysbuf[ri->npcdataref].misc14 = (value / 10000); break;
			case 14: guysbuf[ri->npcdataref].misc15 = (value / 10000); break;
			
			case 15: guysbuf[ri->npcdataref].misc16 = value / 10000; break;
			case 16: guysbuf[ri->npcdataref].misc17 = value / 10000; break;
			case 17: guysbuf[ri->npcdataref].misc18 = value / 10000; break;
			case 18: guysbuf[ri->npcdataref].misc19 = value / 10000; break;
			case 19: guysbuf[ri->npcdataref].misc20 = value / 10000; break;
			case 20: guysbuf[ri->npcdataref].misc21 = value / 10000; break;
			case 21: guysbuf[ri->npcdataref].misc22 = value / 10000; break;
			case 22: guysbuf[ri->npcdataref].misc23 = value / 10000; break;
			case 23: guysbuf[ri->npcdataref].misc24 = value / 10000; break;
			case 24: guysbuf[ri->npcdataref].misc25 = value / 10000; break;
			case 25: guysbuf[ri->npcdataref].misc26 = value / 10000; break;
			case 26: guysbuf[ri->npcdataref].misc27 = value / 10000; break;
			case 27: guysbuf[ri->npcdataref].misc28 = value / 10000; break;
			case 28: guysbuf[ri->npcdataref].misc29 = value / 10000; break;
			case 29: guysbuf[ri->npcdataref].misc30 = value / 10000; break;
			case 30: guysbuf[ri->npcdataref].misc31 = value / 10000; break;
			case 31: guysbuf[ri->npcdataref].misc32 = value / 10000; break;
			
				default: 
			{
				Z_scripterrlog("Invalid Array Index passed to npcdata->Attributes[]: %d\n", (ri->npcdataref*10000)); 
				break;
			}
		}
			
	} 
	break;
}

case NPCDATASHIELD:
{
	int indx = ri->d[0] / 10000; 
	if(ri->npcdataref < 0 || ri->npcdataref > (MAXNPCS-1) ) 
	{ 
		Z_scripterrlog("Invalid NPC ID passed to npcdata->Shield[]: %d\n", (ri->npcdataref*10000));
	} 
	else 
	{ 
		switch(indx)
		{
			case 0:
			{
				(ri->d[1])? (guysbuf[ri->npcdataref].flags |= inv_front) : (guysbuf[ri->npcdataref].flags &= ~inv_front);
				break;
			}
			case 1:
			{
				(ri->d[1])? (guysbuf[ri->npcdataref].flags |= inv_left) : (guysbuf[ri->npcdataref].flags &= ~inv_left);
				break;
			}
			case 2:
			{
				(ri->d[1])? (guysbuf[ri->npcdataref].flags |= inv_right) : (guysbuf[ri->npcdataref].flags &= ~inv_right);
				break;
			}
			case 3:
			{
				(ri->d[1])? (guysbuf[ri->npcdataref].flags |= inv_back) : (guysbuf[ri->npcdataref].flags &= ~inv_back);
				break;
			}
			case 4:
			{
				(ri->d[1])? (guysbuf[ri->npcdataref].flags |= guy_bkshield) : (guysbuf[ri->npcdataref].flags &= ~guy_bkshield);
				break;
			}
			default:
			{
				Z_scripterrlog("Invalid Array Index passed to npcdata->Shield[]: %d\n", indx); 
				break;
			}
		}
	} 
}

	
///----------------------------------------------------------------------------------------------------//
//Audio Variables

case AUDIOVOLUME:
{
	int indx = ri->d[0] / 10000;
	//Z_scripterrlog("Volume[index] is: %d", indx);
	//int vol = value / 10000;
	//Z_scripterrlog("Attempted to change volume to: %d", vol);
	switch(indx)
	{
		
		case 0: //midi volume
		{
			if ( !(FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME) ) 
			{
				FFCore.usr_midi_volume = FFScript::do_getMIDI_volume();
				FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_MIDI_VOLUME,true);
			}
			FFScript::do_setMIDI_volume(value / 10000);
			break;
		}
		case 1: //digi volume
		{
			if ( !(FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME) ) 
			{
				FFCore.usr_digi_volume = FFScript::do_getDIGI_volume();
				FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_DIGI_VOLUME,true);
			}
			FFScript::do_setDIGI_volume(value / 10000);
			break;
		}
		case 2: //emh music volume
		{
			if ( !(FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME) ) 
			{
				FFCore.usr_music_volume = FFScript::do_getMusic_volume();
				FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_MUSIC_VOLUME,true);
			}
			FFScript::do_setMusic_volume(value / 10000);
			break;
		}
		case 3: //sfx volume
		{
			if ( !(FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME) ) 
			{
				FFCore.usr_sfx_volume = FFScript::do_getSFX_volume();
				FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_SFX_VOLUME,true);
			}
			FFScript::do_setSFX_volume(value / 10000);
			break;
		}
		default:
		{
			Z_scripterrlog("Attempted to access an invalid index of Audio->Volume[]", indx); 
			break;
		}
	}
	break;
}

case AUDIOPAN:
{
	if ( !(FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE) ) 
	{
		FFCore.usr_panstyle = FFScript::do_getSFX_pan();
		FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_PANSTYLE,true);
	}
	FFScript::do_setSFX_pan(value/10000);
	break;
}

///----------------------------------------------------------------------------------------------------//
//Misc./Internal
    case SP:
        ri->sp = value / 10000;
        break;
        
    case SCRIPTRAM:
    case GLOBALRAM:
        ArrayH::setElement(ri->d[0] / 10000, ri->d[1] / 10000, value);
        break;
        
    case SCRIPTRAMD:
    case GLOBALRAMD:
        ArrayH::setElement(ri->d[0] / 10000, 0, value);
        break;
        
    case REFFFC:
        ri->ffcref = value / 10000;
        break;
        
    case REFITEM:
        ri->itemref = value;
        break;
        
    case REFITEMCLASS:
        ri->idata = value;
        break;
        
    case REFLWPN:
        ri->lwpn = value;
        break;
        
    case REFEWPN:
        ri->ewpn = value;
        break;
        
    case REFNPC:
        ri->guyref = value;
        break;
    
    case REFMAPDATA: ri->mapsref = value; break;
    case REFSCREENDATA: ri->screenref = value; break;
    case REFCOMBODATA: ri->combosref = value; break;
    case REFSPRITEDATA: ri->spritesref = value; break;
    case REFBITMAP: ri->bitmapref = value; break;
    case REFNPCCLASS: ri->npcdataref = value; break;
    
    case REFDMAPDATA: ri->dmapsref = value; break;
    case REFSHOPDATA: ri->shopsref = value; break;
    case REFMSGDATA: ri->zmsgref = value; break;
    case REFUNTYPED: ri->untypedref = value; break;
    
    
    case REFDROPS:  ri->dropsetref = value; break;
    case REFPONDS:  ri->pondref = value; break;
    case REFWARPRINGS:  ri->warpringref = value; break;
    case REFDOORS:  ri->doorsref = value; break;
    case REFUICOLOURS:  ri->zcoloursref = value; break;
    case REFRGB:  ri->rgbref = value; break;
    case REFPALETTE:  ri->paletteref = value; break;
    case REFTUNES:  ri->tunesref = value; break;
    case REFPALCYCLE:  ri->palcycleref = value; break;
    case REFGAMEDATA:  ri->gamedataref = value; break;
    case REFCHEATS:  ri->cheatsref = value; break;
        
    default:
    {
        if(arg >= D(0) && arg <= D(7))			ri->d[arg - D(0)] = value;
        else if(arg >= A(0) && arg <= A(1))		ri->a[arg - A(0)] = value;
        else if(arg >= GD(0) && arg <= GD(255))	game->global_d[arg-GD(0)] = value;
        
        break;
    }
    }
} //end set_register

///----------------------------------------------------------------------------------------------------//
//                                       ASM Functions                                                 //
///----------------------------------------------------------------------------------------------------//


///----------------------------------------------------------------------------------------------------//
//Internal (to ZScript)

void do_set(const bool v, byte whichFFC)
{
    // Trying to change the current script?
    if(sarg1==FFSCRIPT && ri->ffcref==whichFFC)
        return;
        
    long temp = SH::get_arg(sarg2, v);
    set_register(sarg1, temp);
}

void do_push(const bool v)
{
    const long value = SH::get_arg(sarg1, v);
    ri->sp--;
    SH::write_stack(ri->sp, value);
}

void do_pop()
{
    const long value = SH::read_stack(ri->sp);
    ri->sp++;
    set_register(sarg1, value);
}

void do_loadi()
{
    const long stackoffset = get_register(sarg2) / 10000;
    const long value = SH::read_stack(stackoffset);
    set_register(sarg1, value);
}

void do_storei()
{
    const long stackoffset = get_register(sarg2) / 10000;
    const long value = get_register(sarg1);
    SH::write_stack(stackoffset, value);
}

void do_enqueue(const bool)
{
}
void do_dequeue(const bool)
{
}

void do_comp(const bool v)
{
    long temp = SH::get_arg(sarg2, v);
    long temp2 = get_register(sarg1);
    
    if(temp2 >= temp)   ri->scriptflag |= MOREFLAG;
    else                ri->scriptflag &= ~MOREFLAG;
    
    if(temp2 == temp)   ri->scriptflag |= TRUEFLAG;
    else                ri->scriptflag &= ~TRUEFLAG;
}

void do_allocatemem(const bool v, const bool local, const byte i)
{
    const long size = SH::get_arg(sarg2, v) / 10000;
    dword ptrval;
    
    if(size <= 0)
    {
        Z_scripterrlog("Array initialized to invalid size of %d\n", size);
        set_register(sarg1, 0); //Pass back NULL
        return;
    }
    
    if(local)
    {
        //localRAM[0] is used as an invalid container, so 0 can be the NULL pointer in ZScript
        for(ptrval = 1; localRAM[ptrval].Size() != 0; ptrval++) ;
        
        if(ptrval >= MAX_ZCARRAY_SIZE)
        {
            Z_scripterrlog("%d local arrays already in use, no more can be allocated\n", MAX_ZCARRAY_SIZE-1);
            ptrval = 0;
        }
        else
        {
            ZScriptArray &a = localRAM[ptrval]; //marginally faster for large arrays if we use a reference
            
            a.Resize(size);
            
            for(dword j = 0; j < (dword)size; j++)
                a[j] = 0; //initialize array
                
            // Keep track of which FFC created the array so we know which to deallocate when changing screens
            arrayOwner[ptrval]=i;
        }
    }
    else
    {
        //Globals are only allocated here at first play, otherwise in init_game
        for(ptrval = 0; game->globalRAM[ptrval].Size() != 0; ptrval++) ;
        
        if(ptrval >= game->globalRAM.size())
        {
            al_trace("Invalid pointer value of %ld passed to global allocate\n", ptrval);
            //this shouldn't happen, unless people are putting ALLOCATEGMEM in their ZASM scripts where they shouldn't be
        }
        
        ZScriptArray &a = game->globalRAM[ptrval];
        
        a.Resize(size);
        
        for(dword j = 0; j < (dword)size; j++)
            a[j] = 0;
            
        ptrval += MAX_ZCARRAY_SIZE; //so each pointer has a unique value
    }
    
    
    set_register(sarg1, ptrval * 10000);
    
    // If this happens once per frame, it can drown out every other message. -L
    /*Z_eventlog("Allocated %s array of size %d, pointer address %ld\n",
                local ? "local": "global", size, ptrval);*/
}

void do_deallocatemem()
{
    const long ptrval = get_register(sarg1) / 10000;
    
    FFScript::deallocateZScriptArray(ptrval);
}

void do_loada(const byte a)
{
    if(ri->a[a] == 0)
    {
        Z_eventlog("Global scripts currently have no A registers\n");
        return;
    }
    
    long ffcref = (ri->a[a] / 10000) - 1; //FFC 2
    
    if(BC::checkFFC(ffcref, "LOAD%i") != SH::_NoError)
        return;
        
    long reg = get_register(sarg2); //Register in FFC 2
    
    if(reg >= D(0) || reg <= D(7))
        set_register(sarg1, ffcScriptData[ffcref].d[reg - D(0)]); //get back the info into *sarg1
    else if(reg == A(0) || reg == A(1))
        set_register(sarg1, ffcScriptData[ffcref].a[reg - A(0)]);
    else if(reg == SP)
        set_register(sarg1, ffcScriptData[ffcref].sp * 10000);
        
    //Can get everything else using REFFFC
}

void do_seta(const byte a)
{
    if(ri->a[a] == 0)
    {
        Z_eventlog("Global scripts currently have no A registers\n");
        return;
    }
    
    long ffcref = (ri->a[a] / 10000) - 1; //FFC 2
    
    if(BC::checkFFC(ffcref, "SETA%i") != SH::_NoError)
        return;
        
    long reg = get_register(sarg2); //Register in FFC 2
    
    if(reg >= D(0) || reg <= D(7))
        ffcScriptData[ffcref].d[reg - D(0)] = get_register(sarg1); //Set it to *sarg1
    else if(reg == A(0) || reg == A(1))
        ffcScriptData[ffcref].a[reg - A(0)] = get_register(sarg1);
    else if(reg == SP)
        ffcScriptData[ffcref].sp = get_register(sarg1) / 10000;
}

///----------------------------------------------------------------------------------------------------//
//Mathematical

void do_add(const bool v)
{
    long temp = SH::get_arg(sarg2, v);
    long temp2 = get_register(sarg1);
    
    set_register(sarg1, temp2 + temp);
}

void do_sub(const bool v)
{
    long temp = SH::get_arg(sarg2, v);
    long temp2 = get_register(sarg1);
    
    set_register(sarg1, temp2 - temp);
}

void do_mult(const bool v)
{
    long long temp = SH::get_arg(sarg2, v);
    long temp2 = get_register(sarg1);
    
    set_register(sarg1, long((temp * temp2) / 10000));
}

void do_div(const bool v)
{
    long long temp = SH::get_arg(sarg2, v);
    long long temp2 = get_register(sarg1);
    
    if(temp == 0)
    {
        Z_scripterrlog("Script attempted to divide %ld by zero!\n", temp2);
        set_register(sarg1, long(sign(temp2) * LONG_MAX));
    }
    else
    {
        set_register(sarg1, long((temp2 * 10000) / temp));
    }
}

void do_mod(const bool v)
{
    long temp = SH::get_arg(sarg2, v);
    long temp2 = get_register(sarg1);
    
    if(temp == 0)
    {
        Z_scripterrlog("Script attempted to modulo %ld by zero!\n",temp2);
        temp = 1;
    }
    
    set_register(sarg1, temp2 % temp);
}

void do_trig(const bool v, const byte type)
{
    double rangle = (SH::get_arg(sarg2, v) / 10000.0) * PI / 180.0;
    
    switch(type)
    {
    case 0:
        set_register(sarg1, long(sin(rangle) * 10000.0));
        break;
        
    case 1:
        set_register(sarg1, long(cos(rangle) * 10000.0));
        break;
        
    case 2:
        set_register(sarg1, long(tan(rangle) * 10000.0));
        break;
    }
}

void do_asin(const bool v)
{
    double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
    
    if(temp >= -1 && temp <= 1)
        set_register(sarg1, long(asin(temp) * 10000.0));
    else
    {
        Z_scripterrlog("Script attempted to pass %ld into ArcSin!\n",temp);
        set_register(sarg1, -10000);
    }
}

void do_acos(const bool v)
{
    double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
    
    if(temp >= -1 && temp <= 1)
        set_register(sarg1, long(acos(temp) * 10000.0));
    else
    {
        Z_scripterrlog("Script attempted to pass %ld into ArcCos!\n",temp);
        set_register(sarg1, -10000);
    }
}

void do_arctan()
{
    double xpos = ri->d[0] / 10000.0;
    double ypos = ri->d[1] / 10000.0;
    
    set_register(sarg1, long(atan2(ypos, xpos) * 10000.0));
}

void do_abs(const bool v)
{
    long temp = SH::get_arg(sarg1, v);
    set_register(sarg1, abs(temp));
}

void do_log10(const bool v)
{
    double temp = double(SH::get_arg(sarg1, v)) / 10000.0;
    
    if(temp > 0)
        set_register(sarg1, long(log10(temp) * 10000.0));
    else if(temp == 0)
    {
        Z_eventlog("Script tried to calculate log of 0\n");
        set_register(sarg1, -LONG_MAX);
    }
    else
    {
        Z_eventlog("Script tried to calculate log of %f\n", temp / 10000.0);
        set_register(sarg1, 0);
    }
}

void do_naturallog(const bool v)
{
    double temp = double(SH::get_arg(sarg1, v)) / 10000.0;
    
    if(temp > 0)
        set_register(sarg1, long(log(temp) * 10000.0));
    else if(temp == 0)
    {
        Z_eventlog("Script tried to calculate ln of 0\n");
        set_register(sarg1, -LONG_MAX);
    }
    else
    {
        Z_eventlog("Script tried to calculate ln of %f\n", temp / 10000.0);
        set_register(sarg1, 0);
    }
}

void do_min(const bool v)
{
    long temp = SH::get_arg(sarg2, v);
    long temp2 = get_register(sarg1);
    set_register(sarg1, zc_min(temp2, temp));
}

void do_max(const bool v)
{
    long temp = SH::get_arg(sarg2, v);
    long temp2 = get_register(sarg1);
    
    set_register(sarg1, zc_max(temp2, temp));
}


void do_rnd(const bool v)
{
	long temp = SH::get_arg(sarg2, v) / 10000;

	if(temp > 0)
		set_register(sarg1, (rand() % temp) * 10000);
	else if(temp < 0)
		set_register(sarg1, (rand() % (-temp)) * -10000);
	else
		set_register(sarg1, 0); // Just return 0. (Do not log an error)
}

void do_factorial(const bool v)
{
    long temp;
    
    if(v)
        return;  //must factorial a register, not a value (why is this exactly? ~Joe123)
    else
    {
        temp = get_register(sarg1) / 10000;
        
        if(temp < 2)
        {
            set_register(sarg1, temp >= 0 ? 10000 : 00000);
            return;
        }
    }
    
    long temp2 = 1;
    
    for(long temp3 = temp; temp > 1; temp--)
        temp2 *= temp3;
        
    set_register(sarg1, temp2 * 10000);
}

void do_power(const bool v)
{
    double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
    double temp2 = double(get_register(sarg1)) / 10000.0;
    
    if(temp == 0 && temp2 == 0)
    {
        Z_scripterrlog("Script attempted to calculate 0 to the power 0!\n");
        set_register(sarg1, 1);
        return;
    }
    
    set_register(sarg1, long(pow(temp2, temp) * 10000.0));
}

void do_ipower(const bool v)
{
    double temp = 10000.0 / double(SH::get_arg(sarg2, v));
    double temp2 = double(get_register(sarg1)) / 10000.0;
    
    if(temp == 0 && temp2 == 0)
    {
        Z_scripterrlog("Script attempted to calculate 0 to the power 0!\n");
        set_register(sarg1, 1);
        return;
    }
    
    set_register(sarg1, long(pow(temp2, temp) * 10000.0));
}

void do_sqroot(const bool v)
{
    double temp = double(SH::get_arg(sarg2, v)) / 10000.0;
    
    if(temp < 0)
    {
        Z_scripterrlog("Script attempted to calculate square root of %ld!\n", temp);
        set_register(sarg1, -10000);
        return;
    }
    
    set_register(sarg1, long(sqrt(temp) * 10000.0));
}

///----------------------------------------------------------------------------------------------------//
//Bitwise

void do_and(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (temp2 & temp) * 10000);
}

void do_or(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (temp2 | temp) * 10000);
}

void do_xor(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (temp2 ^ temp) * 10000);
}

void do_nand(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (~(temp2 & temp)) * 10000);
}

void do_nor(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (~(temp2 | temp)) * 10000);
}

void do_xnor(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (~(temp2 ^ temp)) * 10000);
}

void do_not(const bool v)
{
    long temp = SH::get_arg(sarg2, v);
    set_register(sarg1, !temp);
}

void do_bitwisenot(const bool v)
{
    long temp = SH::get_arg(sarg1, v) / 10000;
    set_register(sarg1, (~temp) * 10000);
}

void do_lshift(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (temp2 << temp) * 10000);
}

void do_rshift(const bool v)
{
    long temp = SH::get_arg(sarg2, v) / 10000;
    long temp2 = get_register(sarg1) / 10000;
    set_register(sarg1, (temp2 >> temp) * 10000);
}

///----------------------------------------------------------------------------------------------------//
//Gameplay functions

void do_warp(bool v)
{
    int dmap=SH::get_arg(sarg1, v) / 10000;
    if(dmap<0 || dmap>=MAXDMAPS)
        return;
    int screen=SH::get_arg(sarg2, v) / 10000;
    if(screen<0 || screen>=MAPSCRS) // Should this be MAPSCRSNORMAL?
        return;
    // A shifted DMap can still go past the end of the maps, so check that
    if(DMaps[dmap].map*MAPSCRS+DMaps[dmap].xoff+screen>= (int)TheMaps.size())
        return;
    
    tmpscr->sidewarpdmap[0] = dmap;
    tmpscr->sidewarpscr[0]  = screen;
    tmpscr->sidewarptype[0] = wtIWARP;
    Link.ffwarp = true;
}

void do_pitwarp(bool v)
{
    int dmap=SH::get_arg(sarg1, v) / 10000;
    if(dmap<0 || dmap>=MAXDMAPS)
        return;
    int screen=SH::get_arg(sarg2, v) / 10000;
    if(screen<0 || screen>=MAPSCRS)
        return;
    if(DMaps[dmap].map*MAPSCRS+DMaps[dmap].xoff+screen>= (int)TheMaps.size())
        return;
    tmpscr->sidewarpdmap[0] = dmap;
    tmpscr->sidewarpscr[0]  = screen;
    tmpscr->sidewarptype[0] = wtIWARP;
    Link.ffwarp = true;
    Link.ffpit = true;
}

void do_breakshield()
{
    long UID = get_register(sarg1);
    
    for(int j = 0; j < guys.Count(); j++)
        if(guys.spr(j)->getUID() == UID)
        {
            ((enemy*)guys.spr(j))->break_shield();
            return;
        }
}

void do_showsavescreen()
{
    bool saved = save_game(false, 0);
    set_register(sarg1, saved ? 10000 : 0);
}

void do_selectweapon(bool v, bool Abtn)
{
    if(Abtn && !get_bit(quest_rules,qr_SELECTAWPN))
        return;
        
    byte dir=(byte)(SH::get_arg(sarg1, v)/10000);
    
    // Selection directions don't match the normal ones...
    switch(dir)
    {
    case 0:
        dir=SEL_UP;
        break;
        
    case 1:
        dir=SEL_DOWN;
        break;
        
    case 2:
        dir=SEL_LEFT;
        break;
        
    case 3:
        dir=SEL_RIGHT;
        break;
        
    default:
        return;
    }
    
    if(Abtn)
        selectNextAWpn(dir);
    else
        selectNextBWpn(dir);
}

///----------------------------------------------------------------------------------------------------//
//Screen Information

void do_issolid()
{
    int x = int(ri->d[0] / 10000);
    int y = int(ri->d[1] / 10000);
    
    set_register(sarg1, (_walkflag(x, y, 1) ? 10000 : 0));
}

void do_setsidewarp()
{
    long warp   = SH::read_stack(ri->sp + 3) / 10000;
    long scrn = SH::read_stack(ri->sp + 2) / 10000;
    long dmap   = SH::read_stack(ri->sp + 1) / 10000;
    long type   = SH::read_stack(ri->sp + 0) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->SetSideWarp") != SH::_NoError ||
            BC::checkBounds(scrn, -1, 0x87, "Screen->SetSideWarp") != SH::_NoError ||
            BC::checkBounds(dmap, -1, MAXDMAPS - 1, "Screen->SetSideWarp") != SH::_NoError ||
            BC::checkBounds(type, -1, wtMAX - 1, "Screen->SetSideWarp") != SH::_NoError)
        return;
        
    if(scrn > -1)
        tmpscr->sidewarpscr[warp] = scrn;
        
    if(dmap > -1)
        tmpscr->sidewarpdmap[warp] = dmap;
        
    if(type > -1)
        tmpscr->sidewarptype[warp] = type;
}

void do_settilewarp()
{
    long warp   = SH::read_stack(ri->sp + 3) / 10000;
    long scrn = SH::read_stack(ri->sp + 2) / 10000;
    long dmap   = SH::read_stack(ri->sp + 1) / 10000;
    long type   = SH::read_stack(ri->sp + 0) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->SetTileWarp") != SH::_NoError ||
            BC::checkBounds(scrn, -1, 0x87, "Screen->SetTileWarp") != SH::_NoError ||
            BC::checkBounds(dmap, -1, MAXDMAPS - 1, "Screen->SetTileWarp") != SH::_NoError ||
            BC::checkBounds(type, -1, wtMAX - 1, "Screen->SetTileWarp") != SH::_NoError)
        return;
        
    if(scrn > -1)
        tmpscr->tilewarpscr[warp] = scrn;
        
    if(dmap > -1)
        tmpscr->tilewarpdmap[warp] = dmap;
        
    if(type > -1)
        tmpscr->tilewarptype[warp] = type;
}

void do_getsidewarpdmap(const bool v)
{
    long warp = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->GetSideWarpDMap") != SH::_NoError)
    {
        set_register(sarg1, -10000);
        return;
    }
    
    set_register(sarg1, tmpscr->sidewarpdmap[warp]*10000);
}

void do_getsidewarpscr(const bool v)
{
    long warp = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->GetSideWarpScreen") != SH::_NoError)
    {
        set_register(sarg1, -10000);
        return;
    }
    
    set_register(sarg1, tmpscr->sidewarpscr[warp]*10000);
}

void do_getsidewarptype(const bool v)
{
    long warp = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->GetSideWarpType") != SH::_NoError)
    {
        set_register(sarg1, -10000);
        return;
    }
    
    set_register(sarg1, tmpscr->sidewarptype[warp]*10000);
}

void do_gettilewarpdmap(const bool v)
{
    long warp = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->GetTileWarpDMap") != SH::_NoError)
    {
        set_register(sarg1, -10000);
        return;
    }
    
    set_register(sarg1, tmpscr->tilewarpdmap[warp]*10000);
}

void do_gettilewarpscr(const bool v)
{
    long warp = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->GetTileWarpScreen") != SH::_NoError)
    {
        set_register(sarg1, -10000);
        return;
    }
    
    set_register(sarg1, tmpscr->tilewarpscr[warp]*10000);
}

void do_gettilewarptype(const bool v)
{
    long warp = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkBounds(warp, -1, 3, "Screen->GetTileWarpType") != SH::_NoError)
    {
        set_register(sarg1, -10000);
        return;
    }
    
    set_register(sarg1, tmpscr->tilewarptype[warp]*10000);
}

void do_layerscreen()
{
    long layer = (get_register(sarg2) / 10000) - 1;
    
    if(BC::checkBounds(layer, 0, 5, "Screen->LayerScreen") != SH::_NoError ||
            tmpscr->layermap[layer] == 0)
        set_register(sarg1, -10000);
    else
        set_register(sarg1, tmpscr->layerscreen[layer] * 10000);
}

void do_layermap()
{
    long layer = (get_register(sarg2) / 10000) - 1;
    
    if(BC::checkBounds(layer, 0, 5, "Screen->LayerMap") != SH::_NoError ||
            tmpscr->layermap[layer] == 0)
        set_register(sarg1, -10000);
    else
        set_register(sarg1, tmpscr->layermap[layer] * 10000);
}


	

void do_triggersecrets()
{
    hidden_entrance(0, true, false, -4);
	//hidden_entrance(0,true,single16,scombo); 
	//bool findentrance(int x, int y, int flag, bool setflag)
	//We need a variation on these that triggers any combos with a given flag. -Z
}




void do_getscreenflags()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long flagset = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenFlags") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenFlags") != SH::_NoError ||
            BC::checkBounds(flagset, 0, 9, "Game->GetScreenFlags") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenflags(&TheMaps[map * MAPSCRS + scrn], flagset));
}

void do_getscreeneflags()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long flagset = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenEFlags") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenEFlags") != SH::_NoError ||
            BC::checkBounds(flagset, 0, 9, "Game->GetScreenEFlags") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screeneflags(&TheMaps[map * MAPSCRS + scrn], flagset));
}

//Some of these need to be reduced to two inputs. -Z

long get_screendoor(mapscr *m, int d)
{
    int f = m->door[d];
    return f*10000;
}



long get_screenlayeropacity(mapscr *m, int d)
{
    int f = m->layeropacity[d]; //6 of these
    return f*10000;
}

long get_screensecretcombo(mapscr *m, int d)
{
    int f = m->secretcombo[d]; //128 of these
    return f*10000;
}

long get_screensecretcset(mapscr *m, int d)
{
    int f = m->secretcset[d]; //128 of these
    return f*10000;
}

long get_screensecretflag(mapscr *m, int d)
{
    int f = m->secretflag[d]; //128 of these
    return f*10000;
}

long get_screenlayermap(mapscr *m, int d)
{
    int f = m->layermap[d]; //6 of these
    return f*10000;
}

long get_screenlayerscreen(mapscr *m, int d)
{
    int f = m->layerscreen[d]; //6 of these
    return f*10000;
}

long get_screenpath(mapscr *m, int d)
{
    int f = m->path[d]; //4 of these
    return f*10000;
}

long get_screenwarpReturnX(mapscr *m, int d)
{
    int f = m->warpreturnx[d]; //4 of these
    return f*10000;
}

long get_screenwarpReturnY(mapscr *m, int d)
{
    int f = m->warpreturny[d]; //4 of these
    return f*10000;
}
//One too many inputs here. -Z
long get_screenViewX(mapscr *m)
{
    int f = m->viewX;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenGuy(mapscr *m)
{
    int f = m->guy;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenString(mapscr *m)
{
    int f = m->str;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenRoomtype(mapscr *m)
{
    int f = m->room;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenViewY(mapscr *m)
{
    int f = m->viewY;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenEntryX(mapscr *m)
{
    int f = m->entry_x;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenEntryY(mapscr *m)
{
    int f = m->entry_y;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenitem(mapscr *m)
{
    int f = m->item;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenundercombo(mapscr *m)
{
    int f = m->undercombo;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenundercset(mapscr *m)
{
    int f = m->undercset;
    return f*10000;
}
//One too many inputs here. -Z
long get_screenatchall(mapscr *m)
{
    int f = m->catchall;
    return f*10000;
}
void do_getscreenLayerOpacity()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetLayerOpacity(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetLayerOpacity(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 6, "Game->GetLayerOpacity(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenlayeropacity(&TheMaps[map * MAPSCRS + scrn], d));
}
void do_getscreenSecretCombo()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetSecretCombo(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetSecretCombo(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 127, "Game->GetSecretCombo(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screensecretcombo(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenSecretCSet()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetSecretCSet(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetSecretCSet(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 127, "Game->GetSecretCSet(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screensecretcset(&TheMaps[map * MAPSCRS + scrn], d));
}

void do_getscreenSecretFlag()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetSecretFlag(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetSecretFlag(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0,127, "Game->GetSecretFlag(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screensecretflag(&TheMaps[map * MAPSCRS + scrn], d));
}
void do_getscreenLayerMap()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetSreenLayerMap(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetSreenLayerMap(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 6, "Game->GetSreenLayerMap(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenlayermap(&TheMaps[map * MAPSCRS + scrn], d));
}
void do_getscreenLayerscreen()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetSreenLayerScreen(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetSreenLayerScreen(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 6, "Game->GetSreenLayerScreen(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenlayerscreen(&TheMaps[map * MAPSCRS + scrn], d));
}
void do_getscreenPath()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetSreenPath(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetSreenPath(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 3, "Game->GetSreenPath(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenpath(&TheMaps[map * MAPSCRS + scrn], d));
}
void do_getscreenWarpReturnX()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenWarpReturnX(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenWarpReturnX(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 3, "Game->GetScreenWarpReturnX(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenwarpReturnX(&TheMaps[map * MAPSCRS + scrn], d));
}
void do_getscreenWarpReturnY()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenWarpReturnY(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenWarpReturnY(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 3, "Game->GetScreenWarpReturnY(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenwarpReturnY(&TheMaps[map * MAPSCRS + scrn], d));
}

/*
//One too many inputs here. -Z
void do_getscreenatchall()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenCatchall(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenCatchall(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 6, "Game->GetScreenCatchall(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenatchall(&TheMaps[map * MAPSCRS + scrn], d));
}


//One too many inputs here. -Z
void do_getscreenUndercombo()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetcreenUndercombo(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetcreenUndercombo(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 6, "Game->GetcreenUndercombo(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenundercombo(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenUnderCSet()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GeScreenUnderCSet(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GeScreenUnderCSet(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 6, "Game->GeScreenUnderCSet(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenundercset(&TheMaps[map * MAPSCRS + scrn], d));
}

//One too many inputs here. -Z
void do_getscreenWidth()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenWidth(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenWidth(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenWidth(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenWidth(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenHeight()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenHeight(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenHeight(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenHeight(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenHeight(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenViewX()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenViewX(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenViewX(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenViewX(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenViewX(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenViewY()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenViewY(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenViewY(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenViewY(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenViewY(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenGuy()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenGuy(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenGuy(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenGuy(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenGuy(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenString()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenString(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenString(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenString(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenString(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenRoomType()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenRoomType(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenRoomType(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenRoomType(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenRoomtype(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenEntryX()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenEntryX(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenEntryX(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenEntryX(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenEntryX(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenEntryY()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenEntryY(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenEntryY(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 214747, "Game->GetScreenEntryY(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenEntryY(&TheMaps[map * MAPSCRS + scrn], d));
}
//One too many inputs here. -Z
void do_getscreenItem()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long d = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenItem(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenItem(...screen...)") != SH::_NoError ||
            BC::checkBounds(d, 0, 255, "Game->GetScreenItem(...val...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screenitem(&TheMaps[map * MAPSCRS + scrn], d));
}
*/
void do_getscreendoor()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long door = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenDoor(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenDoor(...screen...)") != SH::_NoError ||
            BC::checkBounds(door, 0, 3, "Game->GetScreenDoor(...doorindex...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screendoor(&TheMaps[map * MAPSCRS + scrn], door));
}

long get_screennpc(mapscr *m, int index)
{
    int f = m->enemy[index];
    return f*10000;
}


void do_getscreennpc()
{
    long map     = (ri->d[2] / 10000) - 1;
    long scrn  = ri->d[1] / 10000;
    long enemy = ri->d[0] / 10000;
    
    if(BC::checkMapID(map, "Game->GetScreenEnemy(...map...)") != SH::_NoError ||
            BC::checkBounds(scrn, 0, 0x87, "Game->GetScreenEnemy(...screen...)") != SH::_NoError ||
            BC::checkBounds(enemy, 0, 9, "Game->GetScreenEnemy(...enemy...)") != SH::_NoError)
        return;
        
    set_register(sarg1, get_screennpc(&TheMaps[map * MAPSCRS + scrn], enemy));
}


///----------------------------------------------------------------------------------------------------//
//Pointer handling

void do_isvaliditem()
{
    long IID = get_register(sarg1);
    //int ct = items.Count();
  
    //for ( int j = items.Count()-1; j >= 0; --j )
    for(int j = 0; j < items.Count(); j++)
    //for(int j = 0; j < ct; j++)
        if(items.spr(j)->getUID() == IID)
        {
            set_register(sarg1, 10000);
            return;
        }
        
    set_register(sarg1, 0);
}

void do_isvalidnpc()
{
    long UID = get_register(sarg1);
    //for ( int j = guys.Count()-1; j >= 0; --j )
    //int ct = guys.Count(); 
   
    for(int j = 0; j < guys.Count(); j++)
    //for(int j = 0; j < ct; j++)
        if(guys.spr(j)->getUID() == UID)
        {
            set_register(sarg1, 10000);
            return;
        }
        
    set_register(sarg1, 0);
}

void do_isvalidlwpn()
{
    long WID = get_register(sarg1);
	//int ct = Lwpns.Count();
    
    //for ( int j = Lwpns.Count()-1; j >= 0; --j )
    for(int j = 0; j < Lwpns.Count(); j++)
    //for(int j = 0; j < ct; j++)
        if(Lwpns.spr(j)->getUID() == WID)
        {
            set_register(sarg1, 10000);
            return;
        }
        
    set_register(sarg1, 0);
}

void do_isvalidewpn()
{
    long WID = get_register(sarg1);
   // int ct = Ewpns.Count();
   
   // for ( int j = Ewpns.Count()-1; j >= 0; --j )
    for(int j = 0; j < Ewpns.Count(); j++)
    //for(int j = 0; j < ct; j++)
        if(Ewpns.spr(j)->getUID() == WID)
        {
            set_register(sarg1, 10000);
            return;
        }
        
    set_register(sarg1, 0);
}

void do_lwpnusesprite(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkWeaponMiscSprite(ID, "lweapon->UseSprite") != SH::_NoError)
        return;
        
    if(LwpnH::loadWeapon(ri->lwpn, "lweapon->UseSprite") == SH::_NoError)
        LwpnH::getWeapon()->LOADGFX(ID);
}

void do_ewpnusesprite(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkWeaponMiscSprite(ID, "eweapon->UseSprite") != SH::_NoError)
        return;
        
    if(EwpnH::loadWeapon(ri->ewpn, "eweapon->UseSprite") == SH::_NoError)
        EwpnH::getWeapon()->LOADGFX(ID);
}

void do_clearsprites(const bool v)
{
    long spritelist = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkBounds(spritelist, 0, 5, "Screen->ClearSprites") != SH::_NoError)
        return;
        
    switch(spritelist)
    {
    case 0:
        guys.clear();
        break;
        
    case 1:
        items.clear();
        break;
        
    case 2:
        Ewpns.clear();
        break;
        
    case 3:
        Lwpns.clear();
        Link.reset_hookshot();
        break;
        
    case 4:
        decorations.clear();
        break;
        
    case 5:
        particles.clear();
        break;
    }
}

void do_loadlweapon(const bool v)
{
    long index = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkLWeaponIndex(index, "Screen->LoadLWeapon") != SH::_NoError)
        ri->lwpn = LONG_MAX;
    else
    {
        ri->lwpn = Lwpns.spr(index)->getUID();
        // This is too trivial to log. -L
        //Z_eventlog("Script loaded lweapon with UID = %ld\n", ri->lwpn);
    }
}

void do_loadeweapon(const bool v)
{
    long index = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkEWeaponIndex(index, "Screen->LoadEWeapon") != SH::_NoError)
        ri->ewpn = LONG_MAX;
    else
    {
        ri->ewpn = Ewpns.spr(index)->getUID();
        //Z_eventlog("Script loaded eweapon with UID = %ld\n", ri->ewpn);
    }
}

void do_loaditem(const bool v)
{
    long index = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkItemIndex(index, "Screen->LoadItem") != SH::_NoError)
        ri->itemref = LONG_MAX;
    else
    {
        ri->itemref = items.spr(index)->getUID();
        //Z_eventlog("Script loaded item with UID = %ld\n", ri->itemref);
    }
}


void do_loaditemdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    //I *think* this is the right check ~Joe
    if(BC::checkItemID(ID, "Game->LoadItemData") != SH::_NoError)
        return;
        
    ri->idata = ID;
    //Z_eventlog("Script loaded itemdata with ID = %ld\n", ri->idata);
}

void do_loadnpc(const bool v)
{
    long index = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkGuyIndex(index, "Screen->LoadNPC") != SH::_NoError)
        ri->guyref = LONG_MAX;
    else
    {
        ri->guyref = guys.spr(index)->getUID();
        //Z_eventlog("Script loaded NPC with UID = %ld\n", ri->guyref);
    }
}

/* script_UID is not yet part of sprite class. 

void FFScript::do_loaditem_by_script_uid(const bool v)
{
	long sUID = SH::get_arg(sarg1, v) / 10000; //script UID
	for(int j = 0; j < items.Count(); j++)
        if(items.spr(j)->script_UID == sUID)
        {
            ri->itemref = items.spr(j)->getUID();
	    return;
        }
	ri->itemref = LONG_MAX;
	//error here.
	//Z_eventlog("Script loaded NPC with UID = %ld\n", ri->guyref);
}

void FFScript::do_loadnpc_by_script_uid(const bool v)
{
	long sUID = SH::get_arg(sarg1, v) / 10000; //script UID
	//int ct = guys.Count();
	for(int j = 0; j < guys.Count(); j++)
        if(guys.spr(j)->script_UID == sUID)
        {
            ri->guyref = guys.spr(j)->getUID();
	    return;
        }
	ri->guyref = LONG_MAX;
	//error here.
	//Z_eventlog("Script loaded NPC with UID = %ld\n", ri->guyref);
}

*/

void FFScript::do_loaddmapdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > 511 )
    {
	Z_scripterrlog("Invalid DMap ID passed to Game->LoadDMapData(): %d\n", ID);
	ri->dmapsref = LONG_MAX;
    }
        
    else ri->dmapsref = ID;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}

void FFScript::do_getDMapData_dmapname(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "dmapdata->GetName()") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'dmapdata->GetName()' not large enough\n");
}

void FFScript::do_setDMapData_dmapname(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;

    string filename_str;
    
    if(BC::checkDMapID(ID, "dmapdata->SetName()") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(DMaps[ID].name, filename_str.c_str(), 72);
    DMaps[ID].name[72]='\0';
}

void FFScript::do_getDMapData_dmaptitle(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "dmapdata->GetIntro()") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].title)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'dmapdata->GetIntro()' not large enough\n");
}

void FFScript::do_setDMapData_dmaptitle(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    string filename_str;
    
    if(BC::checkDMapID(ID, "dmapdata->SetTitle()") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 21);
    strncpy(DMaps[ID].title, filename_str.c_str(), 20);
    DMaps[ID].title[20]='\0';
}

void FFScript::do_getDMapData_dmapintro(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "dmapdata->GetIntro()") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'dmapdata->GetIntro()' not large enough\n");
}

void FFScript::do_setDMapData_dmapintro(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    string filename_str;
    
    if(BC::checkDMapID(ID, "dmapdata->SetIntro()") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(DMaps[ID].intro, filename_str.c_str(), 72);
    DMaps[ID].intro[72]='\0';
}

void FFScript::do_getDMapData_music(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "dmapdata->GetMusic()") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].tmusic)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'dmapdata->GetMusic()' not large enough\n");
}

void FFScript::do_setDMapData_music(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    string filename_str;
    
    if(BC::checkDMapID(ID, "dmapdata->SetMusic()") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 56);
    strncpy(DMaps[ID].tmusic, filename_str.c_str(), 55);
    DMaps[ID].tmusic[55]='\0';
}

void FFScript::do_loadnpcdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 1 || ID > (MAXGUYS-1) )
    {
	Z_scripterrlog("Invalid NPC ID passed to Game->LoadNPCData: %d\n", ID);
	ri->npcdataref = LONG_MAX;
    }
        
    else ri->npcdataref = ID;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}
void FFScript::do_loadmessagedata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 1 || ID > (msg_count-1) )
    {
	Z_scripterrlog("Invalid Message ID passed to Game->LoadMessageData: %d\n", ID);
	ri->zmsgref = LONG_MAX;
    }
        
    else ri->zmsgref = ID;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}
//same syntax as loadmessage data
//the input is an array
void FFScript::do_messagedata_setstring(const bool v)
{

    long arrayptr = get_register(sarg1) / 10000;
    long ID = ri->zmsgref;
	if(BC::checkMessage(ID, "messagesata->Set()") != SH::_NoError)
        return;
	
	string filename_str;

        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(MsgStrings[ID].s, filename_str.c_str(), 72);
    MsgStrings[ID].s[72]='\0';
}
void FFScript::do_messagedata_getstring(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg1) / 10000;
    
    if(BC::checkMessage(ID, "messagedata->Get()") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(MsgStrings[ID].s)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'messagedata->Get()' not large enough\n");
}

void FFScript::do_loadcombodata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > (MAXCOMBOS-1) )
    {
	Z_scripterrlog("Invalid Combo ID passed to Game->LoadComboData: %d\n", ID);
	ri->combosref = 0;
    }

    else ri->combosref = ID;
    //Z_eventlog("Script loaded mapdata with ID = %ld\n", ri->idata);
}

void FFScript::do_loadmapdata(const bool v)
{
    long _map = SH::get_arg(sarg1, v) / 10000;
	
    long _scr = SH::get_arg(sarg2, v) / 10000;
	Z_scripterrlog("LoadMapData Map Value: %d\n", _map);
	Z_scripterrlog("LoadMapData Screen Value: %d\n", _scr);
    int indx = (_map * MAPSCRS + _scr);
	Z_scripterrlog("LoadMapData Indx Value: %d\n", indx);
    if ( _map < 1 || _map > (map_count-1) )
    {
	Z_scripterrlog("Invalid Map ID passed to Game->LoadMapData: %d\n", _map);
	ri->mapsref = 0;
	
    }
    else if ( _scr < 0 || _scr > 129 ) //0x00 to 0x81 -Z
    {
	Z_scripterrlog("Invalid Screen ID passed to Game->LoadMapData: %d\n", _scr);
	ri->mapsref = 0;
    }
    else ri->mapsref = indx;
    Z_scripterrlog("LoadMapData Screen set ri->mapsref to: %d\n", ri->mapsref);
    //Z_eventlog("Script loaded mapdata with ID = %ld\n", ri->idata);
}

	
void FFScript::do_loadshopdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > 255 )
    {
	Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
	ri->shopsref = 0;
    }
        
    else ri->shopsref = ID;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}


void FFScript::do_loadinfoshopdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > 255 )
    {
	Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
	ri->shopsref = 0;
    }
        
    else ri->shopsref = ID+NUMSHOPS;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}

/*
void FFScript::do_loadmapdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > (map_count-1) )
    {
	Z_scripterrlog("Invalid Map ID passed to Game->LoadMapData: %d\n", ID);
	ri->mapsref = LONG_MAX;
    }

    else ri->mapsref = ID;
    //Z_eventlog("Script loaded mapdata with ID = %ld\n", ri->idata);
}
*/

/*

void FFScript::do_loadmapdata(const bool v)
{
    long ID = get_register(sarg2) / 10000; 
    
    if ( ID < 0 || ID > (map_count-1) )
    {
	Z_scripterrlog("Invalid Map ID passed to Game->LoadMapData: %d\n", ID);
	return;
    }

    ri->mapsref = ID;
    set_register(sarg1, ri->mapsref); 
    //Z_eventlog("Script loaded mapdata with ID = %ld\n", ri->idata);
}

*/

void FFScript::do_loadspritedata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > (MAXWPNS-1) )
    {
	Z_scripterrlog("Invalid Sprite ID passed to Game->LoadSpriteData: %d\n", ID);
	ri->spritesref = 0; 
    }

    else ri->spritesref = ID;
    //Z_eventlog("Script loaded mapdata with ID = %ld\n", ri->idata);
}


void FFScript::do_loadscreendata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > (MAXSCREENS-1) )
    {
	Z_scripterrlog("Invalid Map ID passed to Game->LoadScreenData: %d\n", ID);
	ri->screenref = 0; 
    }

    else ri->screenref = ID;
    //Z_eventlog("Script loaded mapdata with ID = %ld\n", ri->idata);
}

void FFScript::do_loadbitmapid(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < MIN_INTERNAL_BITMAP || ID > MAX_INTERNAL_BITMAP )
    {
	Z_scripterrlog("Invalid Bitmap ID passed to Game->Load BitmapID: %d\n", ID);
	ri->bitmapref = 0; 
    }

    else ri->bitmapref = ID;
    //Z_eventlog("Script loaded mapdata with ID = %ld\n", ri->idata);
}

void do_createlweapon(const bool v)
{
    const long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkWeaponID(ID, "Screen->CreateLWeapon") != SH::_NoError)
        return;
        
    addLwpn(0, 0, 0, ID, 0, 0, 0, Link.getUID());
    
    if(Lwpns.Count() < 1)
    {
        ri->lwpn = LONG_MAX;
        Z_scripterrlog("Couldn't create lweapon %ld, screen lweapon limit reached\n", ID);
    }
    else
    {
        ri->lwpn = Lwpns.spr(Lwpns.Count() - 1)->getUID();
        Z_eventlog("Script created lweapon %ld with UID = %ld\n", ID, ri->lwpn);
    }
}

void do_createeweapon(const bool v)
{
    const long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkWeaponID(ID, "Screen->CreateEWeapon") != SH::_NoError)
        return;
        
    addEwpn(0, 0, 0, ID, 0, 0, 0, -1);
    
    if(Ewpns.Count() < 1)
    {
        ri->ewpn = LONG_MAX;
        Z_scripterrlog("Couldn't create eweapon %ld, screen eweapon limit reached\n", ID);
    }
    else
    {
        ri->ewpn = Ewpns.spr(Ewpns.Count() - 1)->getUID();
        Z_eventlog("Script created eweapon %ld with UID = %ld\n", ID, ri->ewpn);
    }
}

void do_createitem(const bool v)
{
    const long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkItemID(ID, "Screen->CreateItem") != SH::_NoError)
        return;
        
    additem(0, (get_bit(quest_rules, qr_NOITEMOFFSET) ? 1: 0), ID, ipBIGRANGE);
    
    if(items.Count() < 1)
    {
        ri->itemref = LONG_MAX;
        Z_scripterrlog("Couldn't create item \"%s\", screen item limit reached\n", item_string[ID]);
    }
    else
    {
        ri->itemref = items.spr(items.Count() - 1)->getUID();
        Z_eventlog("Script created item \"%s\" with UID = %ld\n", item_string[ID], ri->itemref);
    }
}

void do_createnpc(const bool v)
{
    const long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkGuyID(ID, "Screen->CreateNPC") != SH::_NoError)
        return;
        
    //If we make a segmented enemy there'll be more than one sprite created
    word numcreated = addenemy(0, 0, ID, -10);
    
    if(numcreated == 0)
    {
        ri->guyref = LONG_MAX;
        Z_scripterrlog("Couldn't create NPC \"%s\", screen NPC limit reached\n", guy_string[ID]);
    }
    else
    {
        word index = guys.Count() - numcreated; //Get the main enemy, not a segment
        ri->guyref = guys.spr(index)->getUID();
        
        for(; index<guys.Count(); index++)
            ((enemy*)guys.spr(index))->script_spawned=true;
            
        Z_eventlog("Script created NPC \"%s\" with UID = %ld\n", guy_string[ID], ri->guyref);
    }
}

///----------------------------------------------------------------------------------------------------//
//Drawing & Sound

void do_message(const bool v)
{
    const long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkMessage(ID, "Screen->Message") != SH::_NoError)
        return;
        
    if(ID == 0)
    {
        dismissmsg();
        msgfont = zfont;
        blockpath = false;
        Link.finishedmsg();
    }
    else
        donewmsg(ID);
}

INLINE void set_drawing_command_args(const int j, const word numargs)
{
    for(int k = 1; k <= numargs; k++)
        script_drawing_commands[j][k] = SH::read_stack(ri->sp + (numargs - k));
}

void do_drawing_command(const int script_command)
{
    int j = script_drawing_commands.GetNext();
    
    if(j == -1)  //out of drawing command space
    {
        Z_scripterrlog("Max draw primitive limit reached\n");
        return;
    }
    
    script_drawing_commands[j][0] = script_command;
    script_drawing_commands[j][18] = zscriptDrawingRenderTarget->GetCurrentRenderTarget(); // no fixed bs.
    
    switch(script_command)
    {
    case RECTR:
        set_drawing_command_args(j, 12);
        break;
        
    case CIRCLER:
        set_drawing_command_args(j, 11);
        break;
        
    case ARCR:
        set_drawing_command_args(j, 14);
        break;
        
    case ELLIPSER:
        set_drawing_command_args(j, 12);
        break;
        
    case LINER:
        set_drawing_command_args(j, 11);
        break;
        
    case PUTPIXELR:
        set_drawing_command_args(j, 8);
        break;
        
    case DRAWTILER:
        set_drawing_command_args(j, 15);
        break;
        
    case DRAWCOMBOR:
        set_drawing_command_args(j, 16);
        break;
        
    case FASTTILER:
        set_drawing_command_args(j, 6);
        break;
        
    case FASTCOMBOR:
        set_drawing_command_args(j, 6);
        break;
        
    case DRAWCHARR:
        set_drawing_command_args(j, 10);
        break;
        
    case DRAWINTR:
        set_drawing_command_args(j, 11);
        break;
        
    case SPLINER:
        set_drawing_command_args(j, 11);
        break;
        
    case QUADR:
        set_drawing_command_args(j, 15);
        break;
        
    case TRIANGLER:
        set_drawing_command_args(j, 13);
        break;
        
    case BITMAPR:
        set_drawing_command_args(j, 12);
        break;
    
    case BITMAPEXR:
        set_drawing_command_args(j, 16);
        break;
        
    case DRAWLAYERR:
        set_drawing_command_args(j, 8);
        break;
        
    case DRAWSCREENR:
        set_drawing_command_args(j, 6);
        break;
        
    case QUAD3DR:
    {
        std::vector<long> *v = script_drawing_commands.GetVector();
        v->resize(26, 0);
        
        long* pos = &v->at(0);
        long* uv = &v->at(12);
        long* col = &v->at(20);
        long* size = &v->at(24);
        
        set_drawing_command_args(j, 8);
        ArrayH::getValues(script_drawing_commands[j][2] / 10000, pos, 12);
        ArrayH::getValues(script_drawing_commands[j][3] / 10000, uv, 8);
        ArrayH::getValues(script_drawing_commands[j][4] / 10000, col, 4);
        ArrayH::getValues(script_drawing_commands[j][5] / 10000, size, 2);
        
        script_drawing_commands[j].SetVector(v);
    }
    break;
    
    case TRIANGLE3DR:
    {
        std::vector<long> *v = script_drawing_commands.GetVector();
        v->resize(20, 0);
        
        long* pos = &v->at(0);
        long* uv = &v->at(9);
        long* col = &v->at(15);
        long* size = &v->at(18);
        
        set_drawing_command_args(j, 8);
        ArrayH::getValues(script_drawing_commands[j][2] / 10000, pos, 8);
        ArrayH::getValues(script_drawing_commands[j][3] / 10000, uv, 6);
        ArrayH::getValues(script_drawing_commands[j][4] / 10000, col, 3);
        ArrayH::getValues(script_drawing_commands[j][5] / 10000, size, 2);
        
        script_drawing_commands[j].SetVector(v);
    }
    break;
    
    case DRAWSTRINGR:
    {
        set_drawing_command_args(j, 9);
        // Unused
        //const int index = script_drawing_commands[j][19] = j;
        
        string *str = script_drawing_commands.GetString();
        ArrayH::getString(script_drawing_commands[j][8] / 10000, *str);
        script_drawing_commands[j].SetString(str);
    }
    break;
    }
}

void do_set_rendertarget(bool)
{
    int target = int(SH::read_stack(ri->sp) / 10000);
    zscriptDrawingRenderTarget->SetCurrentRenderTarget(target);
}

void do_sfx(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkSFXID(ID, "Game->PlaySound") != SH::_NoError)
        return;
        
    sfx(ID);
}

int FFScript::do_get_internal_uid_npc(int index)
{
	return ((int)guys.spr(index)->getUID());
}
int FFScript::do_get_internal_uid_item(int index)
{
	return ((int)items.spr(index)->getUID());
}
int FFScript::do_get_internal_uid_lweapon(int index)
{
	return ((int)Lwpns.spr(index)->getUID());
}
int FFScript::do_get_internal_uid_eweapon(int index)
{
	return ((int)Ewpns.spr(index)->getUID());
}

void FFScript::do_adjustvolume(const bool v)
{
	long perc = SH::get_arg(sarg1, v) / 10000;
	int temp_midi;
	int temp_digi;
	int temp_mus;
	if ( !(coreflags&FFCORE_SCRIPTED_MIDI_VOLUME) ) 
	{
		temp_midi = do_getMIDI_volume();
		usr_midi_volume = temp_midi;
		SetFFEngineFlag(FFCORE_SCRIPTED_MIDI_VOLUME,true);
	}
	else 
	{
		temp_midi = usr_midi_volume;
	}
	if ( !(coreflags&FFCORE_SCRIPTED_DIGI_VOLUME) ) 
	{
		temp_digi = do_getDIGI_volume();
		usr_digi_volume = temp_digi;
		SetFFEngineFlag(FFCORE_SCRIPTED_DIGI_VOLUME,true);
	}
	else
	{
		temp_digi = usr_digi_volume;
	}
	if ( !(coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME) ) 
	{
		temp_mus = do_getMusic_volume();
		usr_music_volume = temp_mus;
		SetFFEngineFlag(FFCORE_SCRIPTED_MUSIC_VOLUME,true);
	}
	else
	{
		temp_mus = usr_music_volume;
	}
	temp_midi = ( (temp_midi / 100 ) * perc );
	temp_digi = ( (temp_digi / 100 ) * perc );
	temp_mus = ( (temp_mus / 100 ) * perc );
	do_setMIDI_volume(temp_midi);
	do_setDIGI_volume(temp_digi);
	do_setMusic_volume(temp_mus);

}

void FFScript::do_adjustsfxvolume(const bool v)
{
	long perc = SH::get_arg(sarg1, v) / 10000;
	int temp_sfx;
	if ( !(coreflags&FFCORE_SCRIPTED_SFX_VOLUME) ) 
	{
		temp_sfx = do_getSFX_volume();
		usr_sfx_volume = temp_sfx;
		SetFFEngineFlag(FFCORE_SCRIPTED_SFX_VOLUME,true);
	}
	else 
	{
		temp_sfx = usr_sfx_volume;
	}
	temp_sfx = ( (temp_sfx / 100 ) * perc );
	do_setSFX_volume(temp_sfx);
}
	

void do_midi(bool v)
{
    long MIDI = SH::get_arg(sarg1, v) / 10000;
    
    if(MIDI == 0)
        music_stop();
    else
        jukebox(MIDI + (ZC_MIDI_COUNT - 1));
}


void stop_sfx(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    int sfx = (int)ID;
    if(BC::checkSFXID(ID, "Game->EndSound") != SH::_NoError)
        return;
    stop_sfx(sfx);
}

void pause_sfx(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    int sfx = (int)ID;
    if(BC::checkSFXID(ID, "Game->PauseSound") != SH::_NoError)
        return;
    pause_sfx(sfx);
}

void resume_sfx(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    int sfx = (int)ID;
    if(BC::checkSFXID(ID, "Game->ResumeSound") != SH::_NoError)
        return;
    resume_sfx(sfx);
}



void do_enh_music(bool v)
{
    long arrayptr = SH::get_arg(sarg1, v) / 10000;
    long track = (SH::get_arg(sarg2, v) / 10000)-1;
    
    if(arrayptr == 0)
        music_stop();
    else // Pointer to a string..
    {
        string filename_str;
        char filename_char[256];
        bool ret;
        ArrayH::getString(arrayptr, filename_str, 256);
        strncpy(filename_char, filename_str.c_str(), 255);
        filename_char[255]='\0';
        ret=try_zcmusic(filename_char, track, -1000);
        set_register(sarg2, ret ? 10000 : 0);
    }
}

void do_get_enh_music_filename(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapMusicFilename") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].tmusic)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapMusicFilename' not large enough\n");
}

void do_get_enh_music_track(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapMusicTrack") != SH::_NoError)
        return;
        
    set_register(sarg1, (DMaps[ID].tmusictrack+1)*10000);
}

void do_set_dmap_enh_music(const bool v)
{
    long ID   = SH::read_stack(ri->sp + 2) / 10000;
    long arrayptr = SH::read_stack(ri->sp + 1) / 10000;
    long track = (SH::read_stack(ri->sp + 0) / 10000)-1;
    string filename_str;
    
    if(BC::checkDMapID(ID, "Game->SetDMapEnhancedMusic") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 56);
    strncpy(DMaps[ID].tmusic, filename_str.c_str(), 55);
    DMaps[ID].tmusic[55]='\0';
    DMaps[ID].tmusictrack=track;
}





///----------------------------------------------------------------------------------------------------//
//Tracing

void do_trace(bool v)
{
    long temp = SH::get_arg(sarg1, v);
    
    char tmp[100];
    sprintf(tmp, (temp < 0 ? "%06ld" : "%05ld"), temp);
    string s2(tmp);
    s2 = s2.substr(0, s2.size() - 4) + "." + s2.substr(s2.size() - 4, 4);
    al_trace("%s\n", s2.c_str());
    
    if(zconsole)
        printf("%s\n", s2.c_str());
}

void do_tracebool(const bool v)
{
    long temp = SH::get_arg(sarg1, v);
    
    al_trace("%s\n", temp ? "true": "false");
    
    if(zconsole)
        printf("%s\n", temp ? "true": "false");
}

void do_tracestring()
{
    long arrayptr = get_register(sarg1) / 10000;
    string str;
    ArrayH::getString(arrayptr, str, 512);
    al_trace("%s", str.c_str());
    
    if(zconsole)
        printf("%s", str.c_str());
}

void do_tracenl()
{
    al_trace("\n");
    
    if(zconsole)
        printf("\n");
}

void do_cleartrace()
{
    zc_trace_clear();
}

string inttobase(word base, long x, word mindigits)
{
    static const char coeff[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    string s2;
    word digits = zc_max(mindigits - 1, word(floor(log(double(x)) / log(double(base)))));
    
    for(int i = digits; i >= 0; i--)
    {
        s2 += coeff[word(floor(x / pow(double(base), i))) % base];
    }
    
    return s2;
}

void do_tracetobase()
{
    long x = SH::read_stack(ri->sp + 2) / 10000;
    unsigned long base = vbound(SH::read_stack(ri->sp + 1) / 10000, 2, 36);
    unsigned long mindigits = zc_max(1, SH::read_stack(ri->sp) / 10000);
    
    string s2 = x < 0 ? "-": "";
    
    switch(base)
    {
    case 8:
        s2 += '0';
        break;
        
    case 16:
        s2 += "0x";
        break;
    }
    
    s2 += inttobase(base, int(fabs(double(x))), mindigits);
    
    switch(base)
    {
    case 8:
    case 10:
    case 16:
        break;
        
    case 2:
        s2 += 'b';
        break;
        
    default:
        std::stringstream ss;
        ss << " (Base " << base << ')';
        s2 += ss.str();
        break;
    }
    
    al_trace("%s\n", s2.c_str());
    
    if(zconsole)
        printf("%s\n", s2.c_str());
}

///----------------------------------------------------------------------------------------------------//
//Array & string related

void do_arraysize()
{
    long arrayptr = get_register(sarg1) / 10000;
    set_register(sarg1, ArrayH::getSize(arrayptr) * 10000);
}

void do_getsavename()
{
    long arrayptr = get_register(sarg1) / 10000;
    
    if(ArrayH::setArray(arrayptr, string(game->get_name())) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetSaveName' not large enough\n");
}

void do_setsavename()
{
    long arrayptr = get_register(sarg1) / 10000;
    
    string str;
    ArrayH::getString(arrayptr, str);
    byte j;
    
    for(j = 0; str[j] != '\0'; j++)
    {
        if(j >= 8)
        {
            Z_scripterrlog("String supplied to 'Game->GetSaveName' too large\n");
            break;
        }
        
        game->get_name()[j] = str[j];
    }
    
    game->get_name()[j] = '\0';
}

void do_getmessage(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkMessage(ID, "Game->GetMessage") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(MsgStrings[ID].s)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetMessage' not large enough\n");
}



void do_setmessage(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
	if(BC::checkMessage(ID, "Game->SetMessage") != SH::_NoError)
        return;
	
	string filename_str;

        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(MsgStrings[ID].s, filename_str.c_str(), 72);
    MsgStrings[ID].s[72]='\0';
}


void do_getdmapname(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapName") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapName' not large enough\n");
}

void do_setdmapname(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    

    string filename_str;
    
    if(BC::checkDMapID(ID, "Game->Game->SetDMapName") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(DMaps[ID].name, filename_str.c_str(), 72);
    DMaps[ID].name[72]='\0';
}

void do_getdmaptitle(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapTitle") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].title)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapTitle' not large enough\n");
}


void do_setdmaptitle(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    string filename_str;
    
    if(BC::checkDMapID(ID, "Game->Game->SetDMapTitle") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 21);
    strncpy(DMaps[ID].title, filename_str.c_str(), 20);
    DMaps[ID].title[20]='\0';
}

void do_getdmapintro(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapIntro") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapIntro' not large enough\n");
}


void do_setdmapintro(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    string filename_str;
    
    if(BC::checkDMapID(ID, "Game->Game->SetDMapIntro") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(DMaps[ID].intro, filename_str.c_str(), 72);
    DMaps[ID].intro[72]='\0';
}

//Set npc and item names t.b.a. -Z

void do_getitemname()
{
    long arrayptr = get_register(sarg1) / 10000;
    
    if(ArrayH::setArray(arrayptr, item_string[ri->idata]) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'itemdata->GetName' not large enough\n");
}

void do_getnpcname()
{
    long arrayptr = get_register(sarg1) / 10000;
    
    if(GuyH::loadNPC(ri->guyref, "npc->GetName") != SH::_NoError)
        return;
        
    word ID = (GuyH::getNPC()->id & 0xFFF);
    
    if(ArrayH::setArray(arrayptr, guy_string[ID]) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'npc->GetName' not large enough\n");
}

void do_getffcscript()
{
    long arrayptr = get_register(sarg1) / 10000;
    string name;
    int num=-1;
    ArrayH::getString(arrayptr, name, 256); // What's the limit on name length?
    
    for(int i=0; i<512; i++)
    {
        if(strcmp(name.c_str(), ffcmap[i].second.c_str())==0)
        {
            num=i+1;
            break;
        }
    }
    
    set_register(sarg1, num * 10000);
}


void do_getitemscript()
{
	 long arrayptr = get_register(sarg1) / 10000;
    string name;
    int num=-1;
    ArrayH::getString(arrayptr, name, 256); // What's the limit on name length?
    
    for(int i=0; i<512; i++)
    {
        if(strcmp(name.c_str(), itemmap[i].second.c_str())==0)
        {
            num=i+1;
            break;
        }
    }
    
    set_register(sarg1, num * 10000);
}

///----------------------------------------------------------------------------------------------------//
//Tile Manipulation

void do_copytile(const bool v, const bool v2)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    long tile2 = SH::get_arg(sarg2, v2) / 10000;
    
    copy_tile(newtilebuf, tile, tile2, false);
}

void do_swaptile(const bool v, const bool v2)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    long tile2 = SH::get_arg(sarg2, v2) / 10000;
    
    copy_tile(newtilebuf, tile, tile2, true);
}

void do_overlaytile(const bool v, const bool v2)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    long tile2 = SH::get_arg(sarg2, v2) / 10000;
    
    if(BC::checkTile(tile, "OverlayTile") != SH::_NoError ||
            BC::checkTile(tile2, "OverlayTile") != SH::_NoError)
        return;
        
    //Could add an arg for the CSet or something instead of just passing 0, currently only 8-bit is supported
    overlay_tile(newtilebuf, tile, tile2, 0, false);
}

void do_fliprotatetile(const bool v, const bool v2)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    long tile2 = SH::get_arg(sarg2, v2) / 10000;
    
    if(BC::checkTile(tile, "FlipRotateTile") != SH::_NoError ||
            BC::checkTile(tile2, "FlipRotateTile") != SH::_NoError)
        return;
        
    //fliprotatetile
}

void do_settilepixel(const bool v)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkTile(tile, "SetTilePixel") != SH::_NoError)
        return;
        
    //settilepixel
}

void do_gettilepixel(const bool v)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkTile(tile, "GetTilePixel") != SH::_NoError)
        return;
        
    //gettilepixel
}

void do_shifttile(const bool v, const bool v2)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    long tile2 = SH::get_arg(sarg2, v2) / 10000;
    
    if(BC::checkTile(tile, "ShiftTile") != SH::_NoError ||
            BC::checkTile(tile2, "ShiftTile") != SH::_NoError)
        return;
        
    //shifttile
}

void do_cleartile(const bool v)
{
    long tile = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkTile(tile, "ClearTile") != SH::_NoError)
        return;
        
    reset_tile(newtilebuf, tile, newtilebuf[tile].format);
}

void do_combotile(const bool v)
{
    long combo = SH::get_arg(sarg2, v) / 10000;
    
    if(BC::checkCombo(combo, "Game->ComboTile") != SH::_NoError)
        return;
        
    set_register(sarg1, combobuf[combo].tile * 10000);
}


///----------------------------------------------------------------------------------------------------//
//                                       Run the script                                                //
///----------------------------------------------------------------------------------------------------//

// Let's do this
int run_script(const byte type, const word script, const byte i)
{
    if(Quit==qRESET || Quit==qEXIT) // In case an earlier script hung
        return 1;
    
    curScriptType=type;
    curScriptNum=script;
    numInstructions=0;
#ifdef _SCRIPT_COUNTER
    dword script_timer[NUMCOMMANDS];
    dword script_execount[NUMCOMMANDS];
    
    for(int j = 0; j < NUMCOMMANDS; j++)
    {
        script_timer[j]=0;
        script_execount[j]=0;
    }
    
    dword start_time, end_time;
    
    script_counter = 0;
#endif
    
    switch(type)
    {
    case SCRIPT_FFC:
    {
        ri = &(ffcScriptData[i]);
        
        curscript = ffscripts[script];
        stack = &(ffc_stack[i]);
        
        if(!tmpscr->initialized[i])
        {
            memcpy(ri->d, tmpscr->initd[i], 8 * sizeof(long));
            memcpy(ri->a, tmpscr->inita[i], 2 * sizeof(long));
        }
        
        ri->ffcref = i; //'this' pointer
    }
    break;
    
    case SCRIPT_ITEM:
    {
        ri = &itemScriptData;
        ri->Clear(); //Only runs for one frame so we just zero it out
	    //What would happen if we don't do this? -Z
        
        curscript = itemscripts[script];
        stack = &item_stack;
        memset(stack, 0, 256 * sizeof(long)); //zero here too //and don't do this? -Z
	    //If we can make item scripts capable of running for more than one frame, then we can
	    //copy the behaviour to npcs, weapons, and items.
	    //In theory, if we keep the screen caps on these, then we would have 256 or 512 stacks
	    //for each type at all times. That's an awful lot of wasted RAM.
	    //Suggestions? -Z
        
        memcpy(ri->d, itemsbuf[i].initiald, 8 * sizeof(long));
        memcpy(ri->a, itemsbuf[i].initiala, 2 * sizeof(long));
        
        ri->idata = i; //'this' pointer
        
    }
    break;
    //Only one global stack? -Z
    //No curscript? -Z
    //With two extra stacks, we could have dmap and screen scripts. -Z
    
    case SCRIPT_GLOBAL:
    {
        ri = &globalScriptData;
        
        curscript = globalscripts[script];
        stack = &global_stack;
	    //
    }
    break;
    
    default:
        al_trace("No other scripts are currently supported\n");
        return 1;
        break;
    }
    
    dword pc = ri->pc; //this is (marginally) quicker than dereferencing ri each time
    word scommand = curscript[pc].command;
    sarg1 = curscript[pc].arg1;
    sarg2 = curscript[pc].arg2;
    
    
#ifdef _FFDISSASSEMBLY
    
    if(scommand != 0xFFFF)
    {
#ifdef _FFONESCRIPTDISSASSEMBLY
        zc_trace_clear();
#endif
        
        switch(type)
        {
        case SCRIPT_FFC:
            al_trace("\nStart of FFC script %i processing on FFC %i:\n", script, i);
            break;
            
        case SCRIPT_ITEM:
            al_trace("\nStart of item script %i processing:\n", script);
            break;
            
        case SCRIPT_GLOBAL:
            al_trace("\nStart of global script %I processing:\n", script);
            break;
        }
    }
    
#endif
    
    bool increment = true;
    
    while(scommand != 0xFFFF && scommand != WAITFRAME && scommand != WAITDRAW)
    {
        numInstructions++;
        if(numInstructions==100000) // No need to check frequently
        {
            numInstructions=0;
            checkQuitKeys();
            if(Quit)
                scommand=0xFFFF;
        }
        
#ifdef _FFDEBUG
#ifdef _FFDISSASSEMBLY
        ffdebug::print_dissassembly(scommand);
#endif
#ifdef _SCRIPT_COUNTER
        start_time = script_counter;
#endif
#endif
        
        switch(scommand)
        {
        case QUIT:
            scommand = 0xFFFF;
            break;
            
        case GOTO:
            pc = sarg1;
            increment = false;
            break;
            
        case GOTOR:
        {
            pc = (get_register(sarg1) / 10000) - 1;
            increment = false;
        }
        break;
        
        case GOTOTRUE:
            if(ri->scriptflag & TRUEFLAG)
            {
                pc = sarg1;
                increment = false;
            }
            
            break;
            
        case GOTOFALSE:
            if(!(ri->scriptflag & TRUEFLAG))
            {
                pc = sarg1;
                increment = false;
            }
            
            break;
            
        case GOTOMORE:
            if(ri->scriptflag & MOREFLAG)
            {
                pc = sarg1;
                increment = false;
            }
            
            break;
            
        case GOTOLESS:
            if(!(ri->scriptflag & MOREFLAG) || (!get_bit(quest_rules,qr_GOTOLESSNOTEQUAL) && (ri->scriptflag & TRUEFLAG)))
            {
                pc = sarg1;
                increment = false;
            }
            
            break;
            
        case LOOP:
        {
            if(get_register(sarg2) > 0)
            {
                pc = sarg1;
                increment = false;
            }
            else
            {
                set_register(sarg1, sarg1 - 1);
            }
        }
        break;
        
        case SETTRUE:
            set_register(sarg1, (ri->scriptflag & TRUEFLAG) ? 1 : 0);
            break;
            
        case SETFALSE:
            set_register(sarg1, (ri->scriptflag & TRUEFLAG) ? 0 : 1);
            break;
            
        case SETMORE:
            set_register(sarg1, (ri->scriptflag & MOREFLAG) ? 1 : 0);
            break;
            
        case SETLESS:
            set_register(sarg1, (!(ri->scriptflag & MOREFLAG)
                                 || (ri->scriptflag & TRUEFLAG)) ? 1 : 0);
            break;
            
        case NOT:
            do_not(false);
            break;
            
        case COMPAREV:
            do_comp(true);
            break;
            
        case COMPARER:
            do_comp(false);
            break;
            
        case SETV:
            do_set(true, type==SCRIPT_FFC ? i : -1);
            break;
            
        case SETR:
            do_set(false, type==SCRIPT_FFC ? i : -1);
            break;
            
        case PUSHR:
            do_push(false);
            break;
            
        case PUSHV:
            do_push(true);
            break;
            
        case POP:
            do_pop();
            break;
            
        case LOADI:
            do_loadi();
            break;
            
        case STOREI:
            do_storei();
            break;
            
        case LOAD1:
            do_loada(0);
            break;
            
        case LOAD2:
            do_loada(1);
            break;
            
        case SETA1:
            do_seta(0);
            break;
            
        case SETA2:
            do_seta(1);
            break;
            
        case ALLOCATEGMEMR:
            if(type == SCRIPT_GLOBAL) do_allocatemem(false, false, type==SCRIPT_FFC?i:255);
            
            break;
            
        case ALLOCATEGMEMV:
            if(type == SCRIPT_GLOBAL) do_allocatemem(true, false, type==SCRIPT_FFC?i:255);
            
            break;
            
        case ALLOCATEMEMR:
            do_allocatemem(false, true, type==SCRIPT_FFC?i:255);
            break;
            
        case ALLOCATEMEMV:
            do_allocatemem(true, true, type==SCRIPT_FFC?i:255);
            break;
            
        case DEALLOCATEMEMR:
            do_deallocatemem();
            break;
            
        case ARRAYSIZE:
            do_arraysize();
            break;
         case ARRAYSIZEB:
            do_arraysize();
            break;
	case ARRAYSIZEF:
            do_arraysize();
            break;
	case ARRAYSIZEN:
            do_arraysize();
            break;
	case ARRAYSIZEI:
            do_arraysize();
            break;
	case ARRAYSIZEID:
            do_arraysize();
            break;
	case ARRAYSIZEL:
            do_arraysize();
            break;
	case ARRAYSIZEE:
            do_arraysize();
            break;
	
        case GETFFCSCRIPT:
            do_getffcscript();
            break;
	case GETITEMSCRIPT:
            do_getitemscript();
            break;
            
            
        case ADDV:
            do_add(true);
            break;
            
        case ADDR:
            do_add(false);
            break;
            
        case SUBV:
            do_sub(true);
            break;
            
        case SUBR:
            do_sub(false);
            break;
            
        case MULTV:
            do_mult(true);
            break;
            
        case MULTR:
            do_mult(false);
            break;
            
        case DIVV:
            do_div(true);
            break;
            
        case DIVR:
            do_div(false);
            break;
            
        case MODV:
            do_mod(true);
            break;
            
        case MODR:
            do_mod(false);
            break;
            
        case SINV:
            do_trig(true, 0);
            break;
            
        case SINR:
            do_trig(false, 0);
            break;
            
        case COSV:
            do_trig(true, 1);
            break;
            
        case COSR:
            do_trig(false, 1);
            break;
            
        case TANV:
            do_trig(true, 2);
            break;
            
        case TANR:
            do_trig(false, 2);
            break;
            
        case ARCSINR:
            do_asin(false);
            break;
            
        case ARCCOSR:
            do_acos(false);
            break;
            
        case ARCTANR:
            do_arctan();
            break;
            
        case ABSR:
            do_abs(false);
            break;
            
        case MINR:
            do_min(false);
            break;
            
        case MINV:
            do_min(true);
            break;
            
        case MAXR:
            do_max(false);
            break;
            
        case MAXV:
            do_max(true);
            break;
            
        case RNDR:
            do_rnd(false);
            break;
            
        case RNDV:
            do_rnd(true);
            break;
            
        case FACTORIAL:
            do_factorial(false);
            break;
            
        case SQROOTV:
            do_sqroot(true);
            break;
            
        case SQROOTR:
            do_sqroot(false);
            break;
            
        case POWERR:
            do_power(false);
            break;
            
        case POWERV:
            do_power(true);
            break;
            
        case IPOWERR:
            do_ipower(false);
            break;
            
        case IPOWERV:
            do_ipower(true);
            break;
            
        case LOG10:
            do_log10(false);
            break;
            
        case LOGE:
            do_naturallog(false);
            break;
            
        case ANDR:
            do_and(false);
            break;
            
        case ANDV:
            do_and(true);
            break;
            
        case ORR:
            do_or(false);
            break;
            
        case ORV:
            do_or(true);
            break;
            
        case XORR:
            do_xor(false);
            break;
            
        case XORV:
            do_xor(true);
            break;
            
        case NANDR:
            do_nand(false);
            break;
            
        case NANDV:
            do_nand(true);
            break;
            
        case NORR:
            do_nor(false);
            break;
            
        case NORV:
            do_nor(true);
            break;
            
        case XNORR:
            do_xnor(false);
            break;
            
        case XNORV:
            do_xnor(true);
            break;
            
        case BITNOT:
            do_bitwisenot(false);
            break;
            
        case LSHIFTR:
            do_lshift(false);
            break;
            
        case LSHIFTV:
            do_lshift(true);
            break;
            
        case RSHIFTR:
            do_rshift(false);
            break;
            
        case RSHIFTV:
            do_rshift(true);
            break;
            
        case TRACER:
            do_trace(false);
            break;
            
        case TRACEV:
            do_trace(true);
            break;
            
        case TRACE2R:
            do_tracebool(false);
            break;
	
	//Zap and Wavy Effects
	case FXWAVYR:
	    FFCore.do_fx_wavy(false);
	    break;
	case FXZAPR:
	    FFCore.do_fx_zap(false);
	    break;
	//Zap and Wavy Effects
	case FXWAVYV:
	    FFCore.do_fx_wavy(true);
	    break;
	case FXZAPV:
	    FFCore.do_fx_zap(true);
	    break;
	case GREYSCALER:
		FFCore.do_greyscale(false);
		break;
	case GREYSCALEV:
		FFCore.do_greyscale(true);
		break;
            
        case TRACE2V:
            do_tracebool(true);
            break;
            
        case TRACE3:
            do_tracenl();
            break;
            
        case TRACE4:
            do_cleartrace();
            break;
            
        case TRACE5:
            do_tracetobase();
            break;
            
        case TRACE6:
            do_tracestring();
            break;
            
        case WARP:
            do_warp(true);
            break;
            
        case WARPR:
            do_warp(false);
            break;
            
        case PITWARP:
            do_pitwarp(true);
            break;
            
        case PITWARPR:
            do_pitwarp(false);
            break;
            
        case BREAKSHIELD:
            do_breakshield();
            break;
            
        case SELECTAWPNV:
            do_selectweapon(true, true);
            break;
            
        case SELECTAWPNR:
            do_selectweapon(false, true);
            break;
            
        case SELECTBWPNV:
            do_selectweapon(true, false);
            break;
            
        case SELECTBWPNR:
            do_selectweapon(false, false);
            break;
            
        case PLAYSOUNDR:
            do_sfx(false);
            break;
            
        case PLAYSOUNDV:
            do_sfx(true);
            break;
	
	case ADJUSTSFXVOLUMER: FFCore.do_adjustsfxvolume(false); break;
	case ADJUSTSFXVOLUMEV: FFCore.do_adjustsfxvolume(true); break;	
	case ADJUSTVOLUMER: FFCore.do_adjustvolume(false); break;
	case ADJUSTVOLUMEV: FFCore.do_adjustvolume(true); break;
		
	case TRIGGERSECRETR:
            FFScript::do_triggersecret(false);
            break;
            
        case TRIGGERSECRETV:
            FFScript::do_triggersecret(true);
            break;
            
        case PLAYMIDIR:
            do_midi(false);
            break;
            
        case PLAYMIDIV:
            do_midi(true);
            break;
            
        case PLAYENHMUSIC:
            do_enh_music(false);
            break;
            
        case GETMUSICFILE:
            do_get_enh_music_filename(false);
            break;
            
        case GETMUSICTRACK:
            do_get_enh_music_track(false);
            break;
            
        case SETDMAPENHMUSIC:
            do_set_dmap_enh_music(false);
            break;
	
	// Audio->
	
	case ENDSOUNDR:
            stop_sfx(false);
            break;
            
        case ENDSOUNDV:
            stop_sfx(true);
            break;
	
	case PAUSESOUNDR:
            pause_sfx(false);
            break;
            
        case PAUSESOUNDV:
            pause_sfx(true);
            break;
	
	case RESUMESOUNDR:
            resume_sfx(false);
            break;
            
        case RESUMESOUNDV:
            resume_sfx(true);
            break;
	
	
	
	case PAUSESFX:
	{
		int sound = ri->d[0]/10000;
		pause_sfx(sound);
		
	}
	break;

	case RESUMESFX:
	{
		int sound = ri->d[0]/10000;
		resume_sfx(sound);
	}
	break;

	case ADJUSTSFX:
	{
		int sound = ri->d[2]/10000;
		int pan = ri->d[1];
		// control_state[6]=((value/10000)!=0)?true:false;
		bool loop = ((ri->d[0]/10000)!=0)?true:false;
		//SFXBackend.adjust_sfx(sound,pan,loop);
		
		//! adjust_sfx was not ported to the new back end!!! -Z
	}
	break;


	case CONTINUESFX:
	{
		int sound = ri->d[0]/10000;
		//Backend::sfx->cont_sfx(sound);
		
		//! cont_sfx was not ported to the new back end!!!
		// I believe this restarted the loop. 
		resume_sfx(sound);
		//What was the old instruction, again? Did it exist? -Z
		//continue_sfx(sound);
	}
	break;	

	
	/*
	case STOPITEMSOUND:
		void stop_item_sfx(int family)
	*/
	
	case PAUSEMUSIC:
		//What was the instruction prior to adding backends?
	//! The pauseAll() function pauses sfx, not music, so this instruction is not doing what I intended. -Z
		//Check AllOff() -Z
	//zcmusic_pause(ZCMUSIC* zcm, int pause); is in zcmusic.h
		midi_paused = true; 
		//pause_all_sfx();
	
		//Backend::sfx->pauseAll();
		break;
	case RESUMEMUSIC:
		//What was the instruction prior to adding backends?
		//Check AllOff() -Z
		//resume_all_sfx();
		midi_paused = false; 
		//Backend::sfx->resumeAll();
		break;
	
	//!!! typecasting
	case LWPNARRPTR:
	case EWPNARRPTR:
	case ITEMARRPTR:
	case IDATAARRPTR:
	case FFCARRPTR:
	case BOOLARRPTR:
	case NPCARRPTR:
		
	case LWPNARRPTR2:
	case EWPNARRPTR2:
	case ITEMARRPTR2:
	case IDATAARRPTR2:
	case FFCARRPTR2:
	case BOOLARRPTR2:
	case NPCARRPTR2:
	FFScript::do_typedpointer_typecast(false);
	break;
            
        case MSGSTRR:
            do_message(false);
            break;
            
        case MSGSTRV:
            do_message(true);
            break;
            
        case ITEMNAME:
            do_getitemname();
            break;
            
        case NPCNAME:
            do_getnpcname();
            break;
            
        case GETSAVENAME:
            do_getsavename();
            break;
            
        case SETSAVENAME:
            do_setsavename();
            break;
            
        case GETMESSAGE:
            do_getmessage(false);
            break;
	case SETMESSAGE:
            do_setmessage(false);
            break;
            
        case GETDMAPNAME:
            do_getdmapname(false);
            break;
            
        case GETDMAPTITLE:
            do_getdmaptitle(false);
            break;
            
        case GETDMAPINTRO:
            do_getdmapintro(false);
            break;
            
	case SETDMAPNAME:
            do_setdmapname(false);
            break;
            
        case SETDMAPTITLE:
            do_setdmaptitle(false);
            break;
	
        case SETDMAPINTRO:
            do_setdmapintro(false);
            break;
	
        case LOADLWEAPONR:
            do_loadlweapon(false);
            break;
            
        case LOADLWEAPONV:
            do_loadlweapon(true);
            break;
            
        case LOADEWEAPONR:
            do_loadeweapon(false);
            break;
            
        case LOADEWEAPONV:
            do_loadeweapon(true);
            break;
            
        case LOADITEMR:
            do_loaditem(false);
            break;
            
        case LOADITEMV:
            do_loaditem(true);
            break;
            
        case LOADITEMDATAR:
            do_loaditemdata(false);
            break;
	
	//New Datatypes
	case LOADSHOPR:
            FFScript::do_loadshopdata(false);
            break;
        case LOADSHOPV:
            FFScript::do_loadshopdata(true);
            break;
	
	case LOADINFOSHOPR:
            FFScript::do_loadinfoshopdata(false);
            break;
        case LOADINFOSHOPV:
            FFScript::do_loadinfoshopdata(true);
            break;
	case LOADNPCDATAR:
            FFScript::do_loadnpcdata(false);
            break;
        case LOADNPCDATAV:
            FFScript::do_loadnpcdata(true);
            break;
	
	case LOADCOMBODATAR:
            FFScript::do_loadcombodata(false);
            break;
        case LOADCOMBODATAV:
            FFScript::do_loadcombodata(true);
            break;
	
	case LOADMAPDATAR:
            FFScript::do_loadmapdata(false);
            break;
        case LOADMAPDATAV:
            FFScript::do_loadmapdata(true);
            break;
	
	case LOADSPRITEDATAR:
            FFScript::do_loadspritedata(false);
            break;
        case LOADSPRITEDATAV:
            FFScript::do_loadspritedata(true);
            break;
	
	case LOADSCREENDATAR:
            FFScript::do_loadscreendata(false);
            break;
        case LOADSCREENDATAV:
            FFScript::do_loadscreendata(true);
            break;
	
	case LOADBITMAPDATAR:
            FFScript::do_loadbitmapid(false);
            break;
        case LOADBITMAPDATAV:
            FFScript::do_loadbitmapid(true);
            break;
	
//functions
case LOADDMAPDATAR: //command
	FFScript::do_loaddmapdata(false); break;
case LOADDMAPDATAV: //command
	FFScript::do_loaddmapdata(true); break;


case DMAPDATAGETNAMER: //command
	FFScript::do_getDMapData_dmapname(false); break;
case DMAPDATAGETNAMEV: //command
	FFScript::do_getDMapData_dmapname(true); break;

case DMAPDATASETNAMER: //command
	FFScript::do_setDMapData_dmapname(false); break;
case DMAPDATASETNAMEV: //command
	FFScript::do_setDMapData_dmapname(true); break;



case DMAPDATAGETTITLER: //command
	FFScript::do_getDMapData_dmaptitle(false); break;
case DMAPDATAGETTITLEV: //command
	FFScript::do_getDMapData_dmaptitle(true); break;
case DMAPDATASETTITLER: //command
	FFScript::do_setDMapData_dmaptitle(false); break;
case DMAPDATASETTITLEV: //command
	FFScript::do_setDMapData_dmaptitle(true); break;


case DMAPDATAGETINTROR: //command
	FFScript::do_getDMapData_dmapintro(false); break;
case DMAPDATAGETINTROV: //command
	FFScript::do_getDMapData_dmapintro(true); break;
case DMAPDATANSETITROR: //command
	FFScript::do_setDMapData_dmapintro(false); break;
case DMAPDATASETINTROV: //command
	FFScript::do_setDMapData_dmapintro(true); break;


case DMAPDATAGETMUSICR: //command, string to load a music file
	FFScript::do_getDMapData_music(false); break;
case DMAPDATAGETMUSICV: //command, string to load a music file
	FFScript::do_getDMapData_music(true); break;
case DMAPDATASETMUSICR: //command, string to load a music file
	FFScript::do_setDMapData_music(false); break;
case DMAPDATASETMUSICV: //command, string to load a music file
	FFScript::do_setDMapData_music(true); break;

	case LOADMESSAGEDATAR: //COMMAND
	FFScript::do_loadmessagedata(false);
            break;
	case LOADMESSAGEDATAV: //COMMAND
	FFScript::do_loadmessagedata(false);
            break;
	

	case MESSAGEDATASETSTRINGR: //command
	FFScript::do_messagedata_setstring(false);
            break;
	case MESSAGEDATASETSTRINGV: //command
	FFScript::do_messagedata_setstring(false);
            break;
	
	case MESSAGEDATAGETSTRINGR: //command
	FFScript::do_messagedata_getstring(false);
            break;
	case MESSAGEDATAGETSTRINGV: //command
	FFScript::do_messagedata_getstring(false);
            break;	
        case LOADITEMDATAV:
            do_loaditemdata(true);
            break;
            
        case LOADNPCR:
            do_loadnpc(false);
            break;
            
        case LOADNPCV:
            do_loadnpc(true);
            break;
            
        case CREATELWEAPONR:
            do_createlweapon(false);
            break;
            
        case CREATELWEAPONV:
            do_createlweapon(true);
            break;
            
        case CREATEEWEAPONR:
            do_createeweapon(false);
            break;
            
        case CREATEEWEAPONV:
            do_createeweapon(true);
            break;
            
        case CREATEITEMR:
            do_createitem(false);
            break;
            
        case CREATEITEMV:
            do_createitem(true);
            break;
            
        case CREATENPCR:
            do_createnpc(false);
            break;
            
        case CREATENPCV:
            do_createnpc(true);
            break;
            
        case ISVALIDITEM:
            do_isvaliditem();
            break;
            
        case ISVALIDNPC:
            do_isvalidnpc();
            break;
            
        case ISVALIDLWPN:
            do_isvalidlwpn();
            break;
            
        case ISVALIDEWPN:
            do_isvalidewpn();
            break;
            
        case LWPNUSESPRITER:
            do_lwpnusesprite(false);
            break;
            
        case LWPNUSESPRITEV:
            do_lwpnusesprite(true);
            break;
            
        case EWPNUSESPRITER:
            do_ewpnusesprite(false);
            break;
            
        case EWPNUSESPRITEV:
            do_ewpnusesprite(true);
            break;
            
        case CLEARSPRITESR:
            do_clearsprites(false);
            break;
            
        case CLEARSPRITESV:
            do_clearsprites(true);
            break;
            
        case ISSOLID:
            do_issolid();
            break;
            
        case SETSIDEWARP:
            do_setsidewarp();
            break;
            
        case SETTILEWARP:
            do_settilewarp();
            break;
            
        case GETSIDEWARPDMAP:
            do_getsidewarpdmap(false);
            break;
            
        case GETSIDEWARPSCR:
            do_getsidewarpscr(false);
            break;
            
        case GETSIDEWARPTYPE:
            do_getsidewarptype(false);
            break;
            
        case GETTILEWARPDMAP:
            do_gettilewarpdmap(false);
            break;
            
        case GETTILEWARPSCR:
            do_gettilewarpscr(false);
            break;
            
        case GETTILEWARPTYPE:
            do_gettilewarptype(false);
            break;
            
        case LAYERSCREEN:
            do_layerscreen();
            break;
            
        case LAYERMAP:
            do_layermap();
            break;
            
        case SECRETS:
            do_triggersecrets();
            break;
            
        case GETSCREENFLAGS:
            do_getscreenflags();
            break;
            
        case GETSCREENEFLAGS:
            do_getscreeneflags();
            break;
	
	case GETSCREENDOOR:
            do_getscreendoor();
            break;
	
	case GETSCREENENEMY:
            do_getscreennpc();
            break;
	
	//screendata and mapdata
	    case SETSCREENENEMY:
	    { //void SetScreenEnemy(int map, int screen, int index, int value);
		
		    
			long map     = (ri->d[1] / 10000) - 1; 
			long scrn  = ri->d[2] / 10000; 
			long index = ri->d[0] / 10000; 
			int nn = ri->d[3]/10000; 
		
		   // int x;
		    
		   // Z_scripterrlog("rid->[3] is (%i), trying to use for '%s'\n", nn, "nn");
		   // Z_scripterrlog("rid->[2] is (%i), trying to use for '%s'\n", scrn, "scrn");
		   // Z_scripterrlog("rid->[1] is (%i), trying to use for '%s'\n", map, "map");
		   // Z_scripterrlog("rid->[0] is (%i), trying to use for '%s'\n", index, "index");
			
			if(BC::checkMapID(map, "Game->SetScreenEnemy(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenEnemy(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 9, "Game->SetScreenEnemy(...index...)") != SH::_NoError)
				return -10000;
			
		//	if ( BC::checkBounds(nn, 0, 2, "Game->SetScreenEnemy(...enemy...)") != SH::_NoError) x = 1;
		//	if ( BC::checkBounds(map, 20, 21, "Game->SetScreenEnemy(...map...)") != SH::_NoError) x = 2;
			FFScript::set_screenenemy(&TheMaps[map * MAPSCRS + scrn], index, nn); 
			
			
			
	    }
	    break;
	    
	    case SETSCREENDOOR:
	    { //void SetScreenDoor(int map, int screen, int index, int value);
		long map     = (ri->d[1] / 10000) - 1; 
		long scrn  = ri->d[2] / 10000; 
		long index = ri->d[0] / 10000; 
		int nn = ri->d[3]/10000; 
			
			if(BC::checkMapID(map, "Game->SetScreenDoor(...map...)") != SH::_NoError ||
				BC::checkBounds(scrn, 0, 0x87, "Game->SetScreenDoor(...screen...)") != SH::_NoError ||
				BC::checkBounds(index, 0, 3, "Game->SetScreenDoor(...doorindex...)") != SH::_NoError)
				return -10000;
	      
			FFScript::set_screendoor(&TheMaps[map * MAPSCRS + scrn], index, nn); 
			
	    }
            
	case GETSCREENLAYOP:
            do_getscreenLayerOpacity();
            break;
	case GETSCREENSECCMB:
	    do_getscreenSecretCombo();
            break;
	case GETSCREENSECCST:
	    do_getscreenSecretCSet();
            break;
	case GETSCREENSECFLG:
	    do_getscreenSecretFlag();
            break;
	case GETSCREENLAYMAP:
	    do_getscreenLayerMap();
            break;
	case GETSCREENLAYSCR:
	    do_getscreenLayerscreen();
            break;
	case GETSCREENPATH:
	    do_getscreenPath();
            break;
	case GETSCREENWARPRX:
	    do_getscreenWarpReturnX();
            break;
	case GETSCREENWARPRY:
	    do_getscreenWarpReturnY();
            break;

        case COMBOTILE:
            do_combotile(false);
            break;
            
        case RECTR:
        case CIRCLER:
        case ARCR:
        case ELLIPSER:
        case LINER:
        case PUTPIXELR:
        case DRAWTILER:
        case DRAWCOMBOR:
        case DRAWCHARR:
        case DRAWINTR:
        case QUADR:
        case TRIANGLER:
        case QUAD3DR:
        case TRIANGLE3DR:
        case FASTTILER:
        case FASTCOMBOR:
        case DRAWSTRINGR:
        case SPLINER:
        case BITMAPR:
	case BITMAPEXR:
        case DRAWLAYERR:
        case DRAWSCREENR:
            do_drawing_command(scommand);
            break;
            
        case COPYTILEVV:
            do_copytile(true, true);
            break;
            
        case COPYTILEVR:
            do_copytile(true, false);
            break;
            
        case COPYTILERV:
            do_copytile(false, true);
            break;
            
        case COPYTILERR:
            do_copytile(false, false);
            break;
            
        case SWAPTILEVV:
            do_swaptile(true, true);
            break;
            
        case SWAPTILEVR:
            do_swaptile(true, false);
            break;
            
        case SWAPTILERV:
            do_swaptile(false, true);
            break;
            
        case SWAPTILERR:
            do_swaptile(false, false);
            break;
            
        case CLEARTILEV:
            do_cleartile(true);
            break;
            
        case CLEARTILER:
            do_cleartile(false);
            break;
            
        case OVERLAYTILEVV:
            do_overlaytile(true, true);
            break;
            
        case OVERLAYTILEVR:
            do_overlaytile(true, false);
            break;
            
        case OVERLAYTILERV:
            do_overlaytile(false, true);
            break;
            
        case OVERLAYTILERR:
            do_overlaytile(false, false);
            break;
            
        case FLIPROTTILEVV:
            do_fliprotatetile(true, true);
            break;
            
        case FLIPROTTILEVR:
            do_fliprotatetile(true, false);
            break;
            
        case FLIPROTTILERV:
            do_fliprotatetile(false, true);
            break;
            
        case FLIPROTTILERR:
            do_fliprotatetile(false, false);
            break;
            
        case GETTILEPIXELV:
            do_gettilepixel(true);
            break;
            
        case GETTILEPIXELR:
            do_gettilepixel(false);
            break;
            
        case SETTILEPIXELV:
            do_settilepixel(true);
            break;
            
        case SETTILEPIXELR:
            do_settilepixel(false);
            break;
            
        case SHIFTTILEVV:
            do_shifttile(true, true);
            break;
            
        case SHIFTTILEVR:
            do_shifttile(true, false);
            break;
            
        case SHIFTTILERV:
            do_shifttile(false, true);
            break;
            
        case SHIFTTILERR:
            do_shifttile(false, false);
            break;
            
        case SETRENDERTARGET:
            do_set_rendertarget(true);
            break;
            
        case GAMEEND:
            Quit = qQUIT;
            skipcont = 1;
            scommand = 0xFFFF;
            break;
            
        case SAVE:
            if(scriptCanSave)
            {
                save_game(false);
                scriptCanSave=false;
            }
            break;
            
        case SAVESCREEN:
            do_showsavescreen();
            break;
            
        case SAVEQUITSCREEN:
            save_game(false, 1);
            break;
            
            //Not Implemented
        case ELLIPSE2:
        case FLOODFILL:
            break;
            
        case SETCOLORB:
        case SETDEPTHB:
        case GETCOLORB:
        case GETDEPTHB:
            break;
            
        case ENQUEUER:
            do_enqueue(false);
            break;
            
        case ENQUEUEV:
            do_enqueue(true);
            break;
            
        case DEQUEUE:
            do_dequeue(false);
            break;
	
	//Visual Effects
	case WAVYIN:
		FFScript::do_wavyin();
		break;
	case WAVYOUT:
		FFScript::do_wavyout();
		break;
	case ZAPIN:
		FFScript::do_zapin();
		break;
	case ZAPOUT:
		FFScript::do_zapout();
		break;
	case OPENWIPE:
		FFScript::do_openscreen();
		break;
	
	//Monochrome
	case GREYSCALEON:
		setMonochrome(true);
		break;
	case GREYSCALEOFF:
		setMonochrome(false);
		break;
	
	
	//case NPCData
	
	case 	GETNPCDATATILE: FFScript::getNPCData_tile(); break;
	case	GETNPCDATAEHEIGHT: FFScript::getNPCData_e_height(); break;
	case 	GETNPCDATAFLAGS: FFScript::getNPCData_flags(); break;
	case	GETNPCDATAFLAGS2: FFScript::getNPCData_flags2(); break;
	case	GETNPCDATAWIDTH: FFScript::getNPCData_flags2(); break;
	case	GETNPCDATAHEIGHT: FFScript::getNPCData_flags2(); break;
	case	GETNPCDATASTILE: FFScript::getNPCData_s_tile(); break;
	case	GETNPCDATASWIDTH: FFScript::getNPCData_s_width(); break;
	case	GETNPCDATASHEIGHT: FFScript::getNPCData_s_height(); break;
	case	GETNPCDATAETILE: FFScript::getNPCData_e_tile(); break;
	case	GETNPCDATAEWIDTH: FFScript::getNPCData_e_width(); break;
	case	GETNPCDATAHP: FFScript::getNPCData_hp(); break;
	case	GETNPCDATAFAMILY: FFScript::getNPCData_family(); break;
	case	GETNPCDATACSET: FFScript::getNPCData_cset(); break;
	case	GETNPCDATAANIM: FFScript::getNPCData_anim(); break;
	case	GETNPCDATAEANIM: FFScript::getNPCData_e_anim(); break;
	case	GETNPCDATAFRAMERATE: FFScript::getNPCData_frate(); break;
	case	GETNPCDATAEFRAMERATE: FFScript::getNPCData_e_frate(); break;
	case	GETNPCDATATOUCHDMG: FFScript::getNPCData_dp(); break;
	case	GETNPCDATAWPNDAMAGE: FFScript::getNPCData_wdp(); break;
	case	GETNPCDATAWEAPON: FFScript::getNPCData_wdp(); break;
	case	GETNPCDATARANDOM: FFScript::getNPCData_rate(); break;
	case	GETNPCDATAHALT: FFScript::getNPCData_hrate(); break;
	case	GETNPCDATASTEP: FFScript::getNPCData_step(); break;
	case	GETNPCDATAHOMING: FFScript::getNPCData_homing(); break;
	case	GETNPCDATAHUNGER: FFScript::getNPCData_grumble(); break;
	case	GETNPCDATADROPSET: FFScript::getNPCData_item_set(); break;
	case	GETNPCDATABGSFX: FFScript::getNPCData_bgsfx(); break;
	case	GETNPCDATADEATHSFX: FFScript::getNPCData_deadsfx(); break; 
	case	GETNPCDATAXOFS: FFScript::getNPCData_xofs(); break;
	case	GETNPCDATAYOFS: FFScript::getNPCData_yofs(); break;
	case	GETNPCDATAZOFS: FFScript::getNPCData_zofs(); break;
	case	GETNPCDATAHXOFS: FFScript::getNPCData_hxofs(); break;
	case	GETNPCDATAHYOFS: FFScript::getNPCData_hyofs(); break;
	case	GETNPCDATAHITWIDTH: FFScript::getNPCData_hxsz(); break;
	case	GETNPCDATAHITHEIGHT: FFScript::getNPCData_hysz(); break;
	case	GETNPCDATAHITZ: FFScript::getNPCData_hzsz(); break;
	case	GETNPCDATATILEWIDTH: FFScript::getNPCData_txsz(); break;
	case	GETNPCDATATILEHEIGHT: FFScript::getNPCData_tysz(); break;
	case	GETNPCDATAWPNSPRITE: FFScript::getNPCData_wpnsprite(); break;
	//case	GETNPCDATASCRIPTDEF: FFScript::getNPCData_scriptdefence(); break; //2.future cross-compat. 
	case	GETNPCDATADEFENSE: FFScript::getNPCData_defense(); break; 
	case	GETNPCDATASIZEFLAG: FFScript::getNPCData_SIZEflags(); break;
	case	GETNPCDATAATTRIBUTE: FFScript::getNPCData_misc(); break;
	case	GETNPCDATAHITSFX: FFScript::getNPCData_hitsfx(); break;
		
	case	SETNPCDATAFLAGS: FFScript::setNPCData_flags(); break;
	case	SETNPCDATAFLAGS2: FFScript::setNPCData_flags2(); break;
	case	SETNPCDATAWIDTH: FFScript::setNPCData_width(); break;
	case	SETNPCDATAHEIGHT: FFScript::setNPCData_height(); break;
	case	SETNPCDATASTILE: FFScript::setNPCData_s_tile(); break;
	case	SETNPCDATASWIDTH: FFScript::setNPCData_s_width(); break;
	case	SETNPCDATASHEIGHT: FFScript::setNPCData_s_height(); break;
	case	SETNPCDATAETILE: FFScript::setNPCData_e_tile(); break;
	case	SETNPCDATAEWIDTH: FFScript::setNPCData_e_width(); break;
	case	SETNPCDATAHP: FFScript::setNPCData_hp(); break;
	case	SETNPCDATAFAMILY: FFScript::setNPCData_family(); break;
	case	SETNPCDATACSET: FFScript::setNPCData_cset(); break;
	case	SETNPCDATAANIM: FFScript::setNPCData_anim(); break;
	case	SETNPCDATAEANIM: FFScript::setNPCData_e_anim(); break;
	case	SETNPCDATAFRAMERATE: FFScript::setNPCData_frate(); break;
	case	SETNPCDATAEFRAMERATE: FFScript::setNPCData_e_frate(); break;
	case	SETNPCDATATOUCHDMG: FFScript::setNPCData_dp(); break;
	case	SETNPCDATAWPNDAMAGE: FFScript::setNPCData_wdp(); break;
	case	SETNPCDATAWEAPON: FFScript::setNPCData_weapon(); break;
	case	SETNPCDATARANDOM: FFScript::setNPCData_rate(); break;
	case	SETNPCDATAHALT: FFScript::setNPCData_hrate(); break;
	case	SETNPCDATASTEP: FFScript::setNPCData_step(); break;
	case	SETNPCDATAHOMING: FFScript::setNPCData_homing(); break;
	case	SETNPCDATAHUNGER: FFScript::setNPCData_grumble(); break;
	case	SETNPCDATADROPSET: FFScript::setNPCData_item_set(); break;
	case	SETNPCDATABGSFX: FFScript::setNPCData_bgsfx(); break;
	case	SETNPCDATADEATHSFX: FFScript::setNPCData_hitsfx(); break;
	case	SETNPCDATAXOFS: FFScript::setNPCData_xofs(); break;
	case	SETNPCDATAYOFS: FFScript::setNPCData_yofs(); break;
	case	SETNPCDATAZOFS: FFScript::setNPCData_zofs(); break;
	case	SETNPCDATAHXOFS: FFScript::setNPCData_hxofs(); break;
	case	SETNPCDATAHYOFS: FFScript::setNPCData_hyofs(); break;
	case	SETNPCDATAHITWIDTH: FFScript::setNPCData_hxsz(); break;
	case	SETNPCDATAHITHEIGHT: FFScript::setNPCData_hysz(); break;
	case	SETNPCDATAHITZ: FFScript::setNPCData_hzsz(); break;
	case	SETNPCDATATILEWIDTH: FFScript::setNPCData_txsz(); break;
	case	SETNPCDATATILEHEIGHT: FFScript::setNPCData_tysz(); break;
	case	SETNPCDATAWPNSPRITE: FFScript::setNPCData_wpnsprite(); break;
	case	SETNPCDATAHITSFX: FFScript::setNPCData_hitsfx(); break;
	case	SETNPCDATATILE: FFScript::setNPCData_tile(); break;
	case	SETNPCDATAEHEIGHT: FFScript::setNPCData_e_height(); break;
	
	
	
	

		
//	case	SETNPCDATASCRIPTDEF  : FFScript::setNPCData_scriptdefence(); break;
	case 	SETNPCDATADEFENSE : FFScript::setNPCData_defense(ri->d[2]); break;
	case 	SETNPCDATASIZEFLAG : FFScript::setNPCData_SIZEflags(ri->d[2]); break;
	case 	SETNPCDATAATTRIBUTE : FFScript::setNPCData_misc(ri->d[2]); break;
	
	
	//ComboData
	
	case	GCDBLOCKENEM:  FFScript::getComboData_block_enemies(); break;
	case	GCDBLOCKHOLE:  FFScript::getComboData_block_hole(); break;
	case	GCDBLOCKTRIG:  FFScript::getComboData_block_trigger(); break;
	case	GCDCONVEYSPDX:  FFScript::getComboData_conveyor_x_speed(); break;
	case	GCDCONVEYSPDY:  FFScript::getComboData_conveyor_y_speed(); break;
	case	GCDCREATEENEM:  FFScript::getComboData_create_enemy(); break;
	case	GCDCREATEENEMWH:  FFScript::getComboData_create_enemy_when(); break;
	case	GCDCREATEENEMCH:  FFScript::getComboData_create_enemy_change(); break;
	case	GCDDIRCHTYPE:  FFScript::getComboData_directional_change_type(); break;
	case	GCDDISTCHTILES:  FFScript::getComboData_distance_change_tiles(); break;
	case	GCDDIVEITEM:  FFScript::getComboData_dive_item(); break;
	case	GCDDOCK:  FFScript::getComboData_dock(); break;
	case	GCDFAIRY:  FFScript::getComboData_fairy(); break;
	case	GCDFFCOMBOATTRIB:  FFScript::getComboData_ff_combo_attr_change(); break;
	case	GCDFOOTDECOTILE:  FFScript::getComboData_foot_decorations_tile(); break;
	case	GCDFOOTDECOTYPE:  FFScript::getComboData_foot_decorations_type(); break;
	case	GCDHOOKSHOTGRAB:  FFScript::getComboData_hookshot_grab_point(); break;
	case	GCDLADDERPASS:  FFScript::getComboData_ladder_pass(); break;
	case	GCDLOCKBLOCKTYPE:  FFScript::getComboData_lock_block_type(); break;
	case	GCDLOCKBLOCKCHANGE:  FFScript::getComboData_lock_block_change(); break;
	case	GCDMAGICMIRRORTYPE:  FFScript::getComboData_magic_mirror_type(); break;
	case	GCDMODIFYHPAMOUNT:  FFScript::getComboData_modify_hp_amount(); break;
	case	GCDMODIFYHPDELAY:  FFScript::getComboData_modify_hp_delay(); break;
	case	GCDMODIFYHPTYPE:  FFScript::getComboData_modify_hp_type(); break;
	case	GCDMODIFYMPAMOUNT:  FFScript::getComboData_modify_mp_amount(); break;
	case	GCDMODIFYMPDELAY:  FFScript::getComboData_modify_mp_delay(); break;
	case	GCDMODIFYMPTYPE:  FFScript::getComboData_modify_mp_type(); break;
	case	GCDNOPUSHBLOCKS:  FFScript::getComboData_no_push_blocks(); break;
	case	GCDOVERHEAD:  FFScript::getComboData_overhead(); break;
	case	GCDPLACEENEMY:  FFScript::getComboData_place_enemy(); break;
	case	GCDPUSHDIR:  FFScript::getComboData_push_direction(); break;
	case	GCDPUSHWEIGHT:  FFScript::getComboData_push_weight(); break;
	case	GCDPUSHWAIT:  FFScript::getComboData_push_wait(); break;
	case	GCDPUSHED:  FFScript::getComboData_pushed(); break;
	case	GCDRAFT:  FFScript::getComboData_raft(); break;
	case	GCDRESETROOM:  FFScript::getComboData_reset_room(); break;
	case	GCDSAVEPOINT:  FFScript::getComboData_save_point_type(); break;
	case	GCDSCREENFREEZE:  FFScript::getComboData_screen_freeze_type(); break;
	case	GCDSECRETCOMBO:  FFScript::getComboData_secret_combo(); break;
	case	GCDSINGULAR:  FFScript::getComboData_singular(); break;
	case	GCDSLOWMOVE:  FFScript::getComboData_slow_movement(); break;
	case	GCDSTATUE:  FFScript::getComboData_statue_type(); break;
	case	GCDSTEPTYPE:  FFScript::getComboData_step_type(); break;
	case	GCDSTEPCHANGETO:  FFScript::getComboData_step_change_to(); break;
	case	GCDSTRIKEREMNANTS:  FFScript::getComboData_strike_remnants(); break;
	case	GCDSTRIKEREMNANTSTYPE:  FFScript::getComboData_strike_remnants_type(); break;
	case	GCDSTRIKECHANGE:  FFScript::getComboData_strike_change(); break;
	case	GCDSTRIKECHANGEITEM:  FFScript::getComboData_strike_item(); break;
	case	GCDTOUCHITEM:  FFScript::getComboData_touch_item(); break;
	case	GCDTOUCHSTAIRS:  FFScript::getComboData_touch_stairs(); break;
	case	GCDTRIGGERTYPE:  FFScript::getComboData_trigger_type(); break;
	case	GCDTRIGGERSENS:  FFScript::getComboData_trigger_sensitive(); break;
	case	GCDWARPTYPE:  FFScript::getComboData_warp_type(); break;
	case	GCDWARPSENS:  FFScript::getComboData_warp_sensitive(); break;
	case	GCDWARPDIRECT:  FFScript::getComboData_warp_direct(); break;
	case	GCDWARPLOCATION:  FFScript::getComboData_warp_location(); break;
	case	GCDWATER:  FFScript::getComboData_water(); break;
	case	GCDWHISTLE:  FFScript::getComboData_whistle(); break;
	case	GCDWINGAME:  FFScript::getComboData_win_game(); break;
	case	GCDBLOCKWEAPLVL:  FFScript::getComboData_block_weapon_lvl(); break;
	case	GCDTILE:  FFScript::getComboData_tile(); break;
	case	GCDFLIP:  FFScript::getComboData_flip(); break;
	case	GCDWALK:  FFScript::getComboData_walk(); break;
	case	GCDTYPE:  FFScript::getComboData_type(); break;
	case	GCDCSETS:  FFScript::getComboData_csets(); break;
	case	GCDFOO:  FFScript::getComboData_foo(); break;
	case	GCDFRAMES:  FFScript::getComboData_frames(); break;
	case	GCDSPEED:  FFScript::getComboData_speed(); break;
	case	GCDNEXTCOMBO:  FFScript::getComboData_nextcombo(); break;
	case	GCDNEXTCSET:  FFScript::getComboData_nextcset(); break;
	case	GCDFLAG:  FFScript::getComboData_flag(); break;
	case	GCDSKIPANIM:  FFScript::getComboData_skipanim(); break;
	case	GCDNEXTTIMER:  FFScript::getComboData_nexttimer(); break;
	case	GCDSKIPANIMY:  FFScript::getComboData_skipanimy(); break;
	case	GCDANIMFLAGS:  FFScript::getComboData_animflags(); break;
	case	GCDBLOCKWEAPON:  FFScript::getComboData_block_weapon(); break;
	case	GCDEXPANSION:  FFScript::getComboData_expansion(); break;
	case	GCDSTRIKEWEAPONS:  FFScript::getComboData_strike_weapons(); break;
	case	SCDBLOCKENEM:  FFScript::setComboData_block_enemies(); break;
	case	SCDBLOCKHOLE:  FFScript::setComboData_block_hole(); break;
	case	SCDBLOCKTRIG:  FFScript::setComboData_block_trigger(); break;
	case	SCDCONVEYSPDX:  FFScript::setComboData_conveyor_x_speed(); break;
	case	SCDCONVEYSPDY:  FFScript::setComboData_conveyor_y_speed(); break;
	case	SCDCREATEENEM:  FFScript::setComboData_create_enemy(); break;
	case	SCDCREATEENEMWH:  FFScript::setComboData_create_enemy_when(); break;
	case	SCDCREATEENEMCH:  FFScript::setComboData_create_enemy_change(); break;
	case	SCDDIRCHTYPE:  FFScript::setComboData_directional_change_type(); break;
	case	SCDDISTCHTILES:  FFScript::setComboData_distance_change_tiles(); break;
	case	SCDDIVEITEM:  FFScript::setComboData_dive_item(); break;
	case	SCDDOCK:  FFScript::setComboData_dock(); break;
	case	SCDFAIRY:  FFScript::setComboData_fairy(); break;
	case	SCDFFCOMBOATTRIB:  FFScript::setComboData_ff_combo_attr_change(); break;
	case	SCDFOOTDECOTILE:  FFScript::setComboData_foot_decorations_tile(); break;
	case	SCDFOOTDECOTYPE:  FFScript::setComboData_foot_decorations_type(); break;
	case	SCDHOOKSHOTGRAB:  FFScript::setComboData_hookshot_grab_point(); break;
	case	SCDLADDERPASS:  FFScript::setComboData_ladder_pass(); break;
	case	SCDLOCKBLOCKTYPE:  FFScript::setComboData_lock_block_type(); break;
	case	SCDLOCKBLOCKCHANGE:  FFScript::setComboData_lock_block_change(); break;
	case	SCDMAGICMIRRORTYPE:  FFScript::setComboData_magic_mirror_type(); break;
	case	SCDMODIFYHPAMOUNT:  FFScript::setComboData_modify_hp_amount(); break;
	case	SCDMODIFYHPDELAY:  FFScript::setComboData_modify_hp_delay(); break;
	case	SCDMODIFYHPTYPE:  FFScript::setComboData_modify_hp_type(); break;
	case	SCDMODIFYMPAMOUNT:  FFScript::setComboData_modify_mp_amount(); break;
	case	SCDMODIFYMPDELAY:  FFScript::setComboData_modify_mp_delay(); break;
	case	SCDMODIFYMPTYPE:  FFScript::setComboData_modify_mp_type(); break;
	case	SCDNOPUSHBLOCKS:  FFScript::setComboData_no_push_blocks(); break;
	case	SCDOVERHEAD:  FFScript::setComboData_overhead(); break;
	case	SCDPLACEENEMY:  FFScript::setComboData_place_enemy(); break;
	case	SCDPUSHDIR:  FFScript::setComboData_push_direction(); break;
	case	SCDPUSHWEIGHT:  FFScript::setComboData_push_weight(); break;
	case	SCDPUSHWAIT:  FFScript::setComboData_push_wait(); break;
	case	SCDPUSHED:  FFScript::setComboData_pushed(); break;
	case	SCDRAFT:  FFScript::setComboData_raft(); break;
	case	SCDRESETROOM:  FFScript::setComboData_reset_room(); break;
	case	SCDSAVEPOINT:  FFScript::setComboData_save_point_type(); break;
	case	SCDSCREENFREEZE:  FFScript::setComboData_screen_freeze_type(); break;
	case	SCDSECRETCOMBO:  FFScript::setComboData_secret_combo(); break;
	case	SCDSINGULAR:  FFScript::setComboData_singular(); break;
	case	SCDSLOWMOVE:  FFScript::setComboData_slow_movement(); break;
	case	SCDSTATUE:  FFScript::setComboData_statue_type(); break;
	case	SCDSTEPTYPE:  FFScript::setComboData_step_type(); break;
	case	SCDSTEPCHANGETO:  FFScript::setComboData_step_change_to(); break;
	case	SCDSTRIKEREMNANTS:  FFScript::setComboData_strike_remnants(); break;
	case	SCDSTRIKEREMNANTSTYPE:  FFScript::setComboData_strike_remnants_type(); break;
	case	SCDSTRIKECHANGE:  FFScript::setComboData_strike_change(); break;
	case	SCDSTRIKECHANGEITEM:  FFScript::setComboData_strike_item(); break;
	case	SCDTOUCHITEM:  FFScript::setComboData_touch_item(); break;
	case	SCDTOUCHSTAIRS:  FFScript::setComboData_touch_stairs(); break;
	case	SCDTRIGGERTYPE:  FFScript::setComboData_trigger_type(); break;
	case	SCDTRIGGERSENS:  FFScript::setComboData_trigger_sensitive(); break;
	case	SCDWARPTYPE:  FFScript::setComboData_warp_type(); break;
	case	SCDWARPSENS:  FFScript::setComboData_warp_sensitive(); break;
	case	SCDWARPDIRECT:  FFScript::setComboData_warp_direct(); break;
	case	SCDWARPLOCATION:  FFScript::setComboData_warp_location(); break;
	case	SCDWATER:  FFScript::setComboData_water(); break;
	case	SCDWHISTLE:  FFScript::setComboData_whistle(); break;
	case	SCDWINGAME:  FFScript::setComboData_win_game(); break;
	case	SCDBLOCKWEAPLVL:  FFScript::setComboData_block_weapon_lvl(); break;
	case	SCDTILE:  FFScript::setComboData_tile(); break;
	case	SCDFLIP:  FFScript::setComboData_flip(); break;
	case	SCDWALK:  FFScript::setComboData_walk(); break;
	case	SCDTYPE:  FFScript::setComboData_type(); break;
	case	SCDCSETS:  FFScript::setComboData_csets(); break;
	case	SCDFOO:  FFScript::setComboData_foo(); break;
	case	SCDFRAMES:  FFScript::setComboData_frames(); break;
	case	SCDSPEED:  FFScript::setComboData_speed(); break;
	case	SCDNEXTCOMBO:  FFScript::setComboData_nextcombo(); break;
	case	SCDNEXTCSET:  FFScript::setComboData_nextcset(); break;
	case	SCDFLAG:  FFScript::setComboData_flag(); break;
	case	SCDSKIPANIM:  FFScript::setComboData_skipanim(); break;
	case	SCDNEXTTIMER:  FFScript::setComboData_nexttimer(); break;
	case	SCDSKIPANIMY:  FFScript::setComboData_skipanimy(); break;
	case	SCDANIMFLAGS:  FFScript::setComboData_animflags(); break;
	case	SCDBLOCKWEAPON:  FFScript::setComboData_block_weapon(ri->d[2]); break;
	case	SCDEXPANSION:  FFScript::setComboData_expansion(ri->d[2]); break;
	case	SCDSTRIKEWEAPONS:  FFScript::setComboData_strike_weapons(ri->d[2]); break;

	//SpriteData
	
	//case	GETSPRITEDATASTRING: 
	case	GETSPRITEDATATILE: FFScript::getSpriteDataTile();
	case	GETSPRITEDATAMISC: FFScript::getSpriteDataCSets();
	case	GETSPRITEDATACGETS: FFScript::getSpriteDataCSets();
	case	GETSPRITEDATAFRAMES: FFScript::getSpriteDataFrames();
	case	GETSPRITEDATASPEED: FFScript::getSpriteDataSpeed();
	case	GETSPRITEDATATYPE: FFScript::getSpriteDataType();

	//case	SETSPRITEDATASTRING:
	case	SETSPRITEDATATILE: FFScript::setSpriteDataTile();
	case	SETSPRITEDATAMISC: FFScript::setSpriteDataMisc();
	case	SETSPRITEDATACSETS: FFScript::setSpriteDataCSets();
	case	SETSPRITEDATAFRAMES: FFScript::setSpriteDataFrames();
	case	SETSPRITEDATASPEED: FFScript::setSpriteDataSpeed();
	case	SETSPRITEDATATYPE: FFScript::setSpriteDataType();
	
	//Game over Screen
	case	SETCONTINUESCREEN: FFScript::FFChangeSubscreenText();
	case	SETCONTINUESTRING: FFScript::FFSetSaveScreenSetting();
	
	
	
        default:
            Z_scripterrlog("Invalid ZASM command %ld reached\n", scommand);
            break;
        }
               
#ifdef _SCRIPT_COUNTER
        end_time = script_counter;
        script_timer[*command] += end_time - start_time;
        ++script_execount[*command];
#endif
        
        if(increment)	pc++;
        else			increment = true;
        
        if(scommand != 0xFFFF)
        {
            scommand = curscript[pc].command;
            sarg1 = curscript[pc].arg1;
            sarg2 = curscript[pc].arg2;
        }
    }
    
    if(!scriptCanSave)
        scriptCanSave=true;
    
    if(scommand == WAITDRAW)
    {
        switch(type)
        {
        case SCRIPT_GLOBAL:
            global_wait = true;
            break;
            
        default:
            Z_scripterrlog("Waitdraw can only be used in the active global script\n");
            break;
        }
    }
    
    if(scommand == 0xFFFF) //Quit/command list end reached/bad command
    {
        switch(type)
        {
        case SCRIPT_FFC:
            tmpscr->ffscript[i] = 0;
            break;
            
        case SCRIPT_GLOBAL:
            g_doscript = 0;
            break;
            
        case SCRIPT_ITEM:
            break; //item scripts aren't gonna go again anyway
        }
    }
    else
        pc++;
        
    ri->pc = pc; //Put it back where we got it from
    
#ifdef _SCRIPT_COUNTER
    
    for(int j = 0; j < NUMCOMMANDS; j++)
    {
        if(script_execount[j] != 0)
            al_trace("Command %s took %ld ticks in all to complete in %ld executions.\n",
                     command_list[j].name, script_timer[j], script_execount[j]);
    }
    
    remove_int(update_script_counter);
#endif
    
    return 0;
}

int ffscript_engine(const bool preload)
{
    for(byte i = 0; i < MAXFFCS; i++)
    {
        if(tmpscr->ffscript[i] == 0)
            continue;
            
        if(preload && !(tmpscr->ffflags[i]&ffPRELOAD))
            continue;
            
        if((tmpscr->ffflags[i]&ffIGNOREHOLDUP)==0 && Link.getHoldClk()>0)
            continue;
            
        ZScriptVersion::RunScript(SCRIPT_FFC, tmpscr->ffscript[i], i);
        tmpscr->initialized[i] = true;
    }
    
    return 0;
}



///----------------------------------------------------------------------------------------------------


void FFScript::set_screenwarpReturnY(mapscr *m, int d, int value)
{
    int y = vbound(value, 0, 255); //should be screen hight max, except that we may be able to move the subscreen.
    m->warpreturny[d] = y;
}

void FFScript::set_screendoor(mapscr *m, int d, int value)
{
    int dr = vbound(d,0,3);
    int doortype = vbound(value,0,14);
    m->door[dr] = doortype;
}


void FFScript::set_screenenemy(mapscr *m, int index, int value)
{
    int enem_indx = vbound(index,0,9);
    m->enemy[enem_indx] = vbound(value,0,511);
}
void FFScript::set_screenlayeropacity(mapscr *m, int d, int value)
{
    int layer = vbound(d,0,6); int op;
    if ( value <= 64 ) op = 64; 
    else op = 128;
    m->layeropacity[layer] = op;
}
void FFScript::set_screensecretcombo(mapscr *m, int d, int value)
{
    int indx = vbound(value,0,127);
    int cmb = vbound(value,0,MAXCOMBOS);
    m->secretcombo[indx] = cmb;
}
void FFScript::set_screensecretcset(mapscr *m, int d, int value)
{
    int indx = vbound(value,0,127);
    int cs = vbound(value,0,15);
    m->secretcset[indx] = cs;
}
void FFScript::set_screensecretflag(mapscr *m, int d, int value)
{
    int indx = vbound(d,0,127);
    int flag = vbound(value,0,MAX_FLAGS);
    m->secretflag[indx] = flag;
}
void FFScript::set_screenlayermap(mapscr *m, int d, int value)
{
    int layer = vbound(d, MIN_ZQ_LAYER, MAX_ZQ_LAYER);
    int mp = vbound(value,0, (map_count-1));
    m->layermap[layer] = mp;
}
void FFScript::set_screenlayerscreen(mapscr *m, int d, int value)
{
    int layer = vbound(d, MIN_ZQ_LAYER, MAX_ZQ_LAYER);
    int sc = vbound(value,0, 0x87);
    m->layerscreen[layer] = sc;
}
void FFScript::set_screenpath(mapscr *m, int d, int value)
{
    int indx = vbound(d,0,3);
    m->path[indx] = value;
}
void FFScript::set_screenwarpReturnX(mapscr *m, int d, int value)
{
    int x = vbound(value,0,255);
    m->warpreturnx[d] = x;
}


//Use as SetScreenD:
void FFScript::set_screenWidth(mapscr *m, int value)
{
    int w = vbound(value,0,255); //value is char
    m->scrWidth = w;
}
void FFScript::set_screenHeight(mapscr *m, int value)
{
    int h = vbound(value,0,255); //value is char
    m->scrHeight = h;
}
void FFScript::set_screenViewX(mapscr *m, int value)
{
    int x = vbound(value, 0, 255); //value is char
    m->viewX = x;
}
void FFScript::set_screenViewY(mapscr *m, int value)
{
    int y = vbound(value, 0, 255); //value is char
    m->viewY = y;
}
void FFScript::set_screenGuy(mapscr *m, int value)
{
    int bloke = vbound(value,0,9); 
    m->guy = bloke ;
}
void FFScript::set_screenString(mapscr *m, int value)
{
    int string = vbound(value, 0, msg_count-1); //Sanity check to keep it within the legal string IDs.
    m->str = string;
}
void FFScript::set_screenRoomtype(mapscr *m, int value)
{
    int r = vbound(value, rNONE, (rMAX-1)); 
    m->room = r;
}
void FFScript::set_screenEntryX(mapscr *m, int value)
{
    int x = vbound(value,0,255);
    m->entry_x = x;
}
void FFScript::set_screenEntryY(mapscr *m, int value)
{
    int y = vbound(value,0,255);
    m->entry_y = y;
}
void FFScript::set_screenitem(mapscr *m, int value)
{
    int itm = vbound(value,0,MAXITEMS);
    m->item = itm;
}
void FFScript::set_screenundercombo(mapscr *m, int value)
{
    int cmb = vbound(value,0,MAXCOMBOS);
    m->undercombo = cmb;
}
void FFScript::set_screenundercset(mapscr *m, int value)
{
    int cs = vbound(value,0,15);
    m->undercset = cs;
}
void FFScript::set_screenatchall(mapscr *m, int value)
{
    //What are ALL of the catchalls and their max (used) values?
    int ctch = vbound(value, 0, 65535); //It is a word type. 
    m->catchall = ctch;
}


//One too many inputs here. -Z
long FFScript::get_screenWidth(mapscr *m)
{
    long f = m->scrWidth;
    return f*10000;
}
//One too many inputs here. -Z
long FFScript::get_screenHeight(mapscr *m)
{
    int f = m->scrHeight;
    return f*10000;
}

int FFScript::GetQuestVersion()
{
	return ZCheader.zelda_version;
}
int FFScript::GetQuestBuild()
{
	return ZCheader.build;
}
int FFScript::GetQuestSectionVersion(int section)
{
	return ZCheader.zelda_version;
}

int FFScript::GetDefaultWeaponSprite(int wpn_id)
{
	switch (wpn_id)
	{
		case wNone:
			return 0; 
		
		case wSword:
		case wBeam:
		case wBrang:
		case wBomb:
		case wSBomb:
		case wLitBomb:
		case wLitSBomb:
		case wArrow:
		case wFire:
		case wWhistle:
		case wBait:
		case wWand:
		case wMagic:
		case wCatching:
		case wWind:
		case wRefMagic:
		case wRefFireball:
		case wRefRock:
		case wHammer:
		case wHookshot:
		case wHSHandle:
		case wHSChain:
		case wSSparkle:
		case wFSparkle:
		case wSmack:
		case wPhantom:
		case wCByrna:
		case wRefBeam:
		case wStomp:
		case lwMax:
		case wScript1:
		case wScript2:
		case wScript3:
		case wScript4:
		case wScript5:
		case wScript6:
		case wScript7:
		case wScript8:
		case wScript9:
		case wScript10:
		case wIce:
			//Cannot use any of these weapons yet. 
			return -1;
		
		case wEnemyWeapons:
		case ewFireball: return 17;
		
		case ewArrow: return 19; 
		case ewBrang: return 4; 
		case ewSword: return 20; 
		case ewRock: return 18; 
		case ewMagic: return 21; 
		case ewBomb: return 78; 
		case ewSBomb: return 79; 
		case ewLitBomb: return 76; 
		case ewLitSBomb: return 77; 
		case ewFireTrail: return 80; 
		case ewFlame: return 35; 
		case ewWind: return 36; 
		case ewFlame2: return 81; 
		case ewFlame2Trail: return 82; 
		case ewIce: return 83; 
		case ewFireball2: return 17;  //fireball (rising)
		
			
		default:  return -1; //No assign.
		
	}
}

//bitmap->GetPixel()


int FFScript::do_getpixel()
{

	int xoffset = 0, yoffset = 0;
	int xoff = 0; int yoff = 0; //IDR where these are normally read off-hand. 
	//Sticking this here to do initial tests. Will fix later. 
	//They're for the subscreen offsets. 
	const bool brokenOffset=get_bit(extra_rules, er_BITMAPOFFSET)!=0;
	//bool isTargetOffScreenBmp = false;
	//al_trace("Getpixel: The current bitmap ID is: %d /n",ri->bitmapref);
	//zscriptDrawingRenderTarget->SetCurrentRenderTarget(ri->bitmapref);
	BITMAP *bitty = zscriptDrawingRenderTarget->GetBitmapPtr(ri->bitmapref);
        //bmp = targetBitmap;
        if(!bitty)
        {
		bitty = scrollbuf;
		//al_trace("Getpixel: Loaded ScrollBuf into bitty /n");
		//return -10000;
	}
	// draw to screen with subscreen offset
	if(!brokenOffset && ri->bitmapref == -1 )
	{
                xoffset = xoff;
                yoffset = 56; //should this be -56?
	}
	else
        {
            xoffset = 0;
	    yoffset = 0;
        }
	//sdci[1]=x
	//sdci[2]=y
	//sdci[3]= return val?
	//al_trace("Getpixel: ri->d[0] is: %d /n",ri->d[0]);
	//al_trace("Getpixel: ri->d[1] is: %d /n",ri->d[1]);
	//int x1=ri->d[0]/10000;
	//int y1=ri->d[1]/10000;
	
	//al_trace("Getpixel: X is: %d /n",x1);
	//al_trace("Getpixel: Y is: %d /n",y1);
	//al_trace("Getpixel: X is: %d /n",ri->d[0]/10000);
	//al_trace("Getpixel: Y is: %d /n",ri->d[1]/10000);
	
	int yv = ri->d[1]/10000 + yoffset;
	//int ret =  getpixel(bitty, x1+xoffset, y1+yoffset); //This is a palette index value. 
	int ret =  getpixel(bitty, ri->d[0]/10000, yv	); //This is a palette index value. 
	
	//al_trace("Getpixel: Returning a palette index value of: %d /n",ret);
	//al_trace("I'm not yet sure if the PALETTE type will use a value div/mult by 10000. /n");
	
	return ret;

	
}


long FFScript::loadMapData()
{
	long _map = (ri->d[0] / 10000);
	long _scr = (ri->d[1]/10000);
	int indx = (zc_max((_map)-1,0) * MAPSCRS + _scr);
	//Z_scripterrlog("LoadMapData Map Value: %d\n", _map);
	//Z_scripterrlog("LoadMapData Screen Value: %d\n", _scr);
	//Z_scripterrlog("LoadMapData Indx Value: %d\n", indx);
     if ( _map < 1 || _map > (map_count-1) )
    {
	Z_scripterrlog("Invalid Map ID passed to Game->LoadMapData: %d\n", _map);
	ri->mapsref = LONG_MAX;
	
    }
    else if ( _scr < 0 || _scr > 129 ) //0x00 to 0x81 -Z
    {
	Z_scripterrlog("Invalid Screen ID passed to Game->LoadMapData: %d\n", _scr);
	ri->mapsref = LONG_MAX;
    }
    else ri->mapsref = indx;
    //Z_scripterrlog("LoadMapData Screen set ri->mapsref to: %d\n", ri->mapsref);
    return ri->mapsref;
}


// Called when leaving a screen; deallocate arrays created by FFCs that aren't carried over
void FFScript::deallocateZScriptArray(const long ptrval)
{
    if(ptrval<=0 || ptrval >= MAX_ZCARRAY_SIZE)
        Z_scripterrlog("Script tried to deallocate memory at invalid address %ld\n", ptrval);
    else
    {
        arrayOwner[ptrval] = 255;
        
        if(localRAM[ptrval].Size() == 0)
            Z_scripterrlog("Script tried to deallocate memory that was not allocated at address %ld\n", ptrval);
        else
        {
            word size = localRAM[ptrval].Size();
            localRAM[ptrval].Clear();
            
            // If this happens once per frame, it can drown out every other message. -L
            //Z_eventlog("Deallocated local array with address %ld, size %d\n", ptrval, size);
            size = size;
        }
    }
}

int FFScript::get_screen_d(long index1, long index2)
{
    if(index2 < 0 || index2 > 7)
    {
        Z_scripterrlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", index1);
        return 0;
    }
    
    return game->screen_d[index1][index2];
}

void FFScript::set_screen_d(long index1, long index2, int val)
{
    if(index2 < 0 || index2 > 7)
    {
        Z_scripterrlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", index1);
        return;
    }
    
    game->screen_d[index1][index2] = val;
}

// If scr is currently being used as a layer, return that layer no.
int FFScript::whichlayer(long scr)
{
    for(int i = 0; i < 6; i++)
    {
        if(scr == (tmpscr->layermap[i] - 1) * MAPSCRS + tmpscr->layerscreen[i])
            return i;
    }
    
    return -1;
}

void FFScript::clear_ffc_stack(const byte i)
{
    memset(ffc_stack[i], 0, 256 * sizeof(long));
}

void FFScript::clear_global_stack()
{
    memset(global_stack, 0, 256 * sizeof(long));
}

void FFScript::do_zapout()
{
	zapout();
}

void FFScript::do_zapin(){ zapin(); }

void FFScript::do_openscreen() { openscreen(); }
void FFScript::do_wavyin() { wavyin(); }
void FFScript::do_wavyout() { wavyout(false); }


void FFScript::do_triggersecret(const bool v)
{
    long ID = vbound((SH::get_arg(sarg1, v) / 10000), 0, 255);
    mapscr *s = tmpscr;
    int ft=0, checkflag; //Flag trigger, checked flag temp. 
    bool putit = true;  //Is set false with a mismatch (illegal value input).
    //Convert a flag type to a secret type. -Z
	switch(ID)
	{
		case mfBCANDLE:
		    ft=sBCANDLE;
		    break;
		    
		case mfRCANDLE:
		    ft=sRCANDLE;
		    break;
		    
		case mfWANDFIRE:
		    ft=sWANDFIRE;
		    break;
		    
		case mfDINSFIRE:
		    ft=sDINSFIRE;
		    break;
		    
		case mfARROW:
		    ft=sARROW;
		    break;
		    
		case mfSARROW:
		    ft=sSARROW;
		    break;
		    
		case mfGARROW:
		    ft=sGARROW;
		    break;
		    
		case mfSBOMB:
		    ft=sSBOMB;
		    break;
		    
		case mfBOMB:
		    ft=sBOMB;
		    break;
		    
		case mfBRANG:
		    ft=sBRANG;
		    break;
		    
		case mfMBRANG:
		    ft=sMBRANG;
		    break;
		    
		case mfFBRANG:
		    ft=sFBRANG;
		    break;
		    
		case mfWANDMAGIC:
		    ft=sWANDMAGIC;
		    break;
		    
		case mfREFMAGIC:
		    ft=sREFMAGIC;
		    break;
		    
		case mfREFFIREBALL:
		    ft=sREFFIREBALL;
		    break;
		    
		case mfSWORD:
		    ft=sSWORD;
		    break;
		    
		case mfWSWORD:
		    ft=sWSWORD;
		    break;
		    
		case mfMSWORD:
		    ft=sMSWORD;
		    break;
		    
		case mfXSWORD:
		    ft=sXSWORD;
		    break;
		    
		case mfSWORDBEAM:
		    ft=sSWORDBEAM;
		    break;
		    
		case mfWSWORDBEAM:
		    ft=sWSWORDBEAM;
		    break;
		    
		case mfMSWORDBEAM:
		    ft=sMSWORDBEAM;
		    break;
		    
		case mfXSWORDBEAM:
		    ft=sXSWORDBEAM;
		    break;
		    
		case mfHOOKSHOT:
		    ft=sHOOKSHOT;
		    break;
		    
		case mfWAND:
		    ft=sWAND;
		    break;
		    
		case mfHAMMER:
		    ft=sHAMMER;
		    break;
		    
		case mfSTRIKE:
		    ft=sSTRIKE;
		    break;
		    
		default:
		    putit = false;
		    break;
	}
	if ( putit ) {		
		for(int iter=0; iter<2; ++iter)
		{
			for ( int q = 0; q < 176; q++ ) 
			{		
				if(iter==1) checkflag=s->sflag[q]; //Placed
				else checkflag=combobuf[s->data[q]].flag; //Inherent
				Z_message("checkflag is: %d\n", checkflag);
				al_trace("checkflag is: %d\n", checkflag);
				
				Z_message("ID is: %d\n", ID);
				al_trace("ID is: %d\n", ID);
				//cmbx = COMBOX(q);
				////cmby = COMBOY(q);
				
				//Placed flags
				if ( iter == 1 ) {
					if ( s->sflag[q] == ID ) {
						screen_combo_modify_preroutine(s,q);
						s->data[q] = s->secretcombo[ft];
						s->cset[q] = s->secretcset[ft];
						s->sflag[q] = s->secretflag[ft];
					   // newflag = s->secretflag[ft];
						screen_combo_modify_postroutine(s,q);
					}
				}
				//Inherent flags
				else {
					if ( combobuf[s->data[q]].flag == ID ) {
						screen_combo_modify_preroutine(s,q);
						s->data[q] = s->secretcombo[ft];
						s->cset[q] = s->secretcset[ft];
						//s->sflag[q] = s->secretflag[ft];
						screen_combo_modify_postroutine(s,q);
					}
					
				}
			}
		}
	}
	
}

//Get rid of this pile of rubbish. it does not work! -Z
void FFScript::do_changeffcscript(const bool v){
	long ID = vbound((SH::get_arg(sarg1, v) / 10000), 0, 255);
	/*
	for(long i = 1; i < MAX_ZCARRAY_SIZE; i++)
	{
	    if(arrayOwner[i]==ri->ffcref)
		FFScript::deallocateZScriptArray(i);
	}
	
	tmpscr->ffscript[ri->ffcref] = vbound(ID/10000, 0, scripts.ffscripts.size()-1);
	
	for(int i=0; i<16; i++)
	    ffmisc[ri->ffcref][i] = 0;
	    
	for(int i=0; i<2; i++)
	    tmpscr->inita[ri->ffcref][i] = 0;
	    
	for(int i=0; i<8; i++)
	    tmpscr->initd[ri->ffcref][i] = 0;
	    
	ffcScriptData[ri->ffcref].Clear();
	tmpscr->initialized[ri->ffcref] = true;
	*/
}




//NPCData

//NPCData Getter Macros


	

//NPCData-> Function
#define GET_NPCDATA_FUNCTION_VAR_INT(member) \
{ \
	long ID = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		set_register(sarg1, guysbuf[ID].member * 10000); \
}

#define GET_NPCDATA_FUNCTION_VAR_INDEX(member, indexbound) \
{ \
	int ID = int(ri->d[0] / 10000);\
	int indx = vbound((ri->d[1] / 10000), 0, indexbound); \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		set_register(sarg1, guysbuf[ID].member[indx] * 10000); \
}

#define GET_NPCDATA_FUNCTION_VAR_FLAG(member) \
{ \
	int ID = int(ri->d[0] / 10000);\
	int flag = int(ri->d[1] / 10000);\
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		set_register(sarg1, (guysbuf[ID].member&flag) ? 10000 : 0); \
}

void FFScript::getNPCData_tile(){ GET_NPCDATA_FUNCTION_VAR_INT(tile); } //word
void FFScript::getNPCData_e_height(){ GET_NPCDATA_FUNCTION_VAR_INT(e_height); } 
void FFScript::getNPCData_flags(){ GET_NPCDATA_FUNCTION_VAR_INT(flags); } //word
void FFScript::getNPCData_flags2(){ GET_NPCDATA_FUNCTION_VAR_INT(flags2); } 
void FFScript::getNPCData_width(){ GET_NPCDATA_FUNCTION_VAR_INT(width); } 
void FFScript::getNPCData_height(){ GET_NPCDATA_FUNCTION_VAR_INT(height); } 
void FFScript::getNPCData_s_tile(){ GET_NPCDATA_FUNCTION_VAR_INT(s_tile); } 
void FFScript::getNPCData_s_width(){ GET_NPCDATA_FUNCTION_VAR_INT(s_width); } 
void FFScript::getNPCData_s_height(){ GET_NPCDATA_FUNCTION_VAR_INT(s_height); } 
void FFScript::getNPCData_e_tile(){ GET_NPCDATA_FUNCTION_VAR_INT(e_tile); } 
void FFScript::getNPCData_e_width(){ GET_NPCDATA_FUNCTION_VAR_INT(e_width); } 
void FFScript::getNPCData_hp(){ GET_NPCDATA_FUNCTION_VAR_INT(hp); } 
void FFScript::getNPCData_family(){ GET_NPCDATA_FUNCTION_VAR_INT(family); } 
void FFScript::getNPCData_cset(){ GET_NPCDATA_FUNCTION_VAR_INT(cset); } 
void FFScript::getNPCData_anim(){ GET_NPCDATA_FUNCTION_VAR_INT(anim); } 
void FFScript::getNPCData_e_anim(){ GET_NPCDATA_FUNCTION_VAR_INT(e_anim); } 
void FFScript::getNPCData_frate(){ GET_NPCDATA_FUNCTION_VAR_INT(frate); } 
void FFScript::getNPCData_e_frate(){ GET_NPCDATA_FUNCTION_VAR_INT(e_frate); } 
void FFScript::getNPCData_dp(){ GET_NPCDATA_FUNCTION_VAR_INT(dp); } 
void FFScript::getNPCData_wdp(){ GET_NPCDATA_FUNCTION_VAR_INT(wdp); } 
void FFScript::getNPCData_weapon(){ GET_NPCDATA_FUNCTION_VAR_INT(weapon); } 
void FFScript::getNPCData_rate(){ GET_NPCDATA_FUNCTION_VAR_INT(rate); } 
void FFScript::getNPCData_hrate(){ GET_NPCDATA_FUNCTION_VAR_INT(hrate); } 
void FFScript::getNPCData_step(){ GET_NPCDATA_FUNCTION_VAR_INT(step); } 
void FFScript::getNPCData_homing(){ GET_NPCDATA_FUNCTION_VAR_INT(homing); } 
void FFScript::getNPCData_grumble(){ GET_NPCDATA_FUNCTION_VAR_INT(grumble); } 
void FFScript::getNPCData_item_set(){ GET_NPCDATA_FUNCTION_VAR_INT(item_set); } 
void FFScript::getNPCData_bgsfx(){ GET_NPCDATA_FUNCTION_VAR_INT(bgsfx); } 
void FFScript::getNPCData_hitsfx(){ GET_NPCDATA_FUNCTION_VAR_INT(hitsfx); } 
void FFScript::getNPCData_deadsfx(){ GET_NPCDATA_FUNCTION_VAR_INT(deadsfx); } 
void FFScript::getNPCData_xofs(){ GET_NPCDATA_FUNCTION_VAR_INT(xofs); } 
void FFScript::getNPCData_yofs(){ GET_NPCDATA_FUNCTION_VAR_INT(yofs); } 
void FFScript::getNPCData_zofs(){ GET_NPCDATA_FUNCTION_VAR_INT(zofs); } 
void FFScript::getNPCData_hxofs(){ GET_NPCDATA_FUNCTION_VAR_INT(hxofs); } 
void FFScript::getNPCData_hyofs(){ GET_NPCDATA_FUNCTION_VAR_INT(hyofs); } 
void FFScript::getNPCData_hxsz(){ GET_NPCDATA_FUNCTION_VAR_INT(hxsz); } 
void FFScript::getNPCData_hysz(){ GET_NPCDATA_FUNCTION_VAR_INT(hysz); } 
void FFScript::getNPCData_hzsz(){ GET_NPCDATA_FUNCTION_VAR_INT(hzsz); } 
void FFScript::getNPCData_txsz(){ GET_NPCDATA_FUNCTION_VAR_INT(txsz); } 
void FFScript::getNPCData_tysz(){ GET_NPCDATA_FUNCTION_VAR_INT(tysz); } 
void FFScript::getNPCData_wpnsprite(){ GET_NPCDATA_FUNCTION_VAR_INT(wpnsprite); } 

//NPCData Getters, two inputs, one return, similar to ISSolid

/*

void do_issolid()
{
    int x = int(ri->d[0] / 10000);
    int y = int(ri->d[1] / 10000);
    
    set_register(sarg1, (_walkflag(x, y, 1) ? 10000 : 0));
}

*/






//void FFScript::getNPCData_scriptdefence(){GET_NPCDATA_FUNCTION_VAR_INDEX(scriptdefence)};


void FFScript::getNPCData_defense(){GET_NPCDATA_FUNCTION_VAR_INDEX(defense,(edefLAST255))};


void FFScript::getNPCData_SIZEflags(){GET_NPCDATA_FUNCTION_VAR_FLAG(SIZEflags);}


void FFScript::getNPCData_misc()
{
	int ID = int(ri->d[0] / 10000); //the enemy ID value
	int indx = int(ri->d[1] / 10000); //the misc index ID
	if ((ID < 1 || ID > 511) || ( indx < 0 || indx > 15 ))
		set_register(sarg1, -10000); 
	switch ( indx )
	{
		case 0: set_register(sarg1, guysbuf[ID].misc1 * 10000); break;
		case 1: set_register(sarg1, guysbuf[ID].misc2 * 10000); break;
		case 2: set_register(sarg1, guysbuf[ID].misc3 * 10000); break;
		case 3: set_register(sarg1, guysbuf[ID].misc4 * 10000); break;
		case 4: set_register(sarg1, guysbuf[ID].misc5 * 10000); break;
		case 5: set_register(sarg1, guysbuf[ID].misc6 * 10000); break;
		case 6: set_register(sarg1, guysbuf[ID].misc7 * 10000); break;
		case 7: set_register(sarg1, guysbuf[ID].misc8 * 10000); break;
		case 8: set_register(sarg1, guysbuf[ID].misc9 * 10000); break;
		case 9: set_register(sarg1, guysbuf[ID].misc10 * 10000); break;
		case 10: set_register(sarg1, guysbuf[ID].misc11 * 10000); break;
		case 11: set_register(sarg1, guysbuf[ID].misc12 * 10000); break;
		case 12: set_register(sarg1, guysbuf[ID].misc13 * 10000); break;
		case 13: set_register(sarg1, guysbuf[ID].misc14 * 10000); break;
		case 14: set_register(sarg1, guysbuf[ID].misc15 * 10000); break;
		default: set_register(sarg1, -10000); break;
	}
}

//NPCData Setters, two inputs, no return; similar to void GetDMapIntro(int DMap, int buffer[]);

/*

void do_getdmapintro(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapIntro") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapIntro' not large enough\n");
}

*/

//NPCData Setter Macros

//Variables for spritedata sp->member
	
	

//Functions for NPCData->

#define SET_NPCDATA_FUNCTION_VAR_INT(member, bound) \
{ \
	long ID = get_register(sarg1) / 10000; \
	long val = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		guysbuf[ID].member = vbound(val,0,bound); \
}



#define SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(member) \
{ \
	long ID = get_register(sarg1) / 10000; \
	long val = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		set_register(sarg1, -10000); \
	else \
		guysbuf[ID].member = val; \
}


//SET_NPC_VAR_INDEX(member,value)
#define SET_NPCDATA_FUNCTION_VAR_INDEX(member, val, bound, indexbound) \
{ \
	long ID = (ri->d[0]/10000);  \
	long indx =  vbound((ri->d[1]/10000),0,indexbound);  \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		return; \
	else \
		guysbuf[ID].member[indx] = vbound(val,0,bound); \
}

//Special case for flags, three inputs one return
#define SET_NPCDATA_FUNCTION_VAR_FLAG(member, val) \
{ \
	long ID = (ri->d[0]/10000);  \
	long flag =  (ri->d[1]/10000);  \
	if(ID < 1 || ID > (MAXGUYS-1)) \
		return; \
	else \
	{ \
		if ( val != 0 ) guysbuf[ID].member|=flag; \
		else guysbuf[ID].member|= ~flag; \
	}\
}

void FFScript::setNPCData_flags(){SET_NPCDATA_FUNCTION_VAR_INT(flags,ZS_DWORD);} //word
void FFScript::setNPCData_flags2(){SET_NPCDATA_FUNCTION_VAR_INT(flags2,ZS_DWORD);}
void FFScript::setNPCData_width(){SET_NPCDATA_FUNCTION_VAR_INT(width,ZS_BYTE);}
void FFScript::setNPCData_tile(){SET_NPCDATA_FUNCTION_VAR_INT(tile,ZS_WORD);}
void FFScript::setNPCData_e_height(){SET_NPCDATA_FUNCTION_VAR_INT(e_height,ZS_BYTE);}
void FFScript::setNPCData_height(){SET_NPCDATA_FUNCTION_VAR_INT(height,ZS_BYTE);}
void FFScript::setNPCData_s_tile(){SET_NPCDATA_FUNCTION_VAR_INT(s_tile,ZS_WORD);}
void FFScript::setNPCData_s_width(){SET_NPCDATA_FUNCTION_VAR_INT(s_width,ZS_BYTE);}
void FFScript::setNPCData_s_height(){SET_NPCDATA_FUNCTION_VAR_INT(s_height,ZS_BYTE);}
void FFScript::setNPCData_e_tile(){SET_NPCDATA_FUNCTION_VAR_INT(e_tile,ZS_WORD);}
void FFScript::setNPCData_e_width(){SET_NPCDATA_FUNCTION_VAR_INT(e_width,ZS_BYTE);}
void FFScript::setNPCData_hp(){SET_NPCDATA_FUNCTION_VAR_INT(hp,ZS_SHORT);}
void FFScript::setNPCData_family(){SET_NPCDATA_FUNCTION_VAR_INT(family,ZS_SHORT);}
void FFScript::setNPCData_cset(){SET_NPCDATA_FUNCTION_VAR_INT(cset,ZS_SHORT);}
void FFScript::setNPCData_anim(){SET_NPCDATA_FUNCTION_VAR_INT(anim,ZS_SHORT);}
void FFScript::setNPCData_e_anim(){SET_NPCDATA_FUNCTION_VAR_INT(e_anim,ZS_SHORT);}
void FFScript::setNPCData_frate(){SET_NPCDATA_FUNCTION_VAR_INT(frate,ZS_SHORT);}
void FFScript::setNPCData_e_frate(){SET_NPCDATA_FUNCTION_VAR_INT(e_frate,ZS_SHORT);}
void FFScript::setNPCData_dp(){SET_NPCDATA_FUNCTION_VAR_INT(dp,ZS_SHORT);}
void FFScript::setNPCData_wdp(){SET_NPCDATA_FUNCTION_VAR_INT(wdp,ZS_SHORT);}
void FFScript::setNPCData_weapon(){SET_NPCDATA_FUNCTION_VAR_INT(weapon,ZS_SHORT);}
void FFScript::setNPCData_rate(){SET_NPCDATA_FUNCTION_VAR_INT(rate,ZS_SHORT);}
void FFScript::setNPCData_hrate(){SET_NPCDATA_FUNCTION_VAR_INT(hrate,ZS_SHORT);}
void FFScript::setNPCData_step(){SET_NPCDATA_FUNCTION_VAR_INT(step,ZS_SHORT);}
void FFScript::setNPCData_homing(){SET_NPCDATA_FUNCTION_VAR_INT(homing,ZS_SHORT);}
void FFScript::setNPCData_grumble(){SET_NPCDATA_FUNCTION_VAR_INT(grumble,ZS_SHORT);}
void FFScript::setNPCData_item_set(){SET_NPCDATA_FUNCTION_VAR_INT(item_set,ZS_SHORT);}
void FFScript::setNPCData_bgsfx(){SET_NPCDATA_FUNCTION_VAR_INT(bgsfx,ZS_SHORT);}
void FFScript::setNPCData_hitsfx(){SET_NPCDATA_FUNCTION_VAR_INT(hitsfx,ZS_BYTE);}
void FFScript::setNPCData_deadsfx(){SET_NPCDATA_FUNCTION_VAR_INT(deadsfx,ZS_BYTE);}
void FFScript::setNPCData_xofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(xofs);}
void FFScript::setNPCData_yofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(yofs);}
void FFScript::setNPCData_zofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(zofs);}
void FFScript::setNPCData_hxofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hxofs);}
void FFScript::setNPCData_hyofs(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hyofs);}
void FFScript::setNPCData_hxsz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hxsz);}
void FFScript::setNPCData_hysz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hysz);}
void FFScript::setNPCData_hzsz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(hzsz);}
void FFScript::setNPCData_txsz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(txsz);}
void FFScript::setNPCData_tysz(){SET_NPCDATA_FUNCTION_VAR_INT_NOBOUND(tysz);}
void FFScript::setNPCData_wpnsprite(){SET_NPCDATA_FUNCTION_VAR_INT(wpnsprite,511);}

//NPCData Setters, three inputs, no return. works as SetDMapScreenD function







//void FFScript::setNPCData_scriptdefence(){SET_NPCDATA_FUNCTION_VAR_INDEX(scriptdefence);}
void FFScript::setNPCData_defense(int v){SET_NPCDATA_FUNCTION_VAR_INDEX(defense,v, ZS_INT, (edefLAST255) );}
void FFScript::setNPCData_SIZEflags(int v){SET_NPCDATA_FUNCTION_VAR_FLAG(SIZEflags,v);}
void FFScript::setNPCData_misc(int val)
{
	int ID = int(ri->d[0] / 10000); //the enemy ID value
	int indx = int(ri->d[1] / 10000); //the misc index ID
	if ((ID < 1 || ID > 511) || ( indx < 0 || indx > 15 )) return;
	switch ( indx )
	{
		case 0: guysbuf[ID].misc1 = val; break;
		case 1: guysbuf[ID].misc2 = val; break;
		case 2: guysbuf[ID].misc3 = val; break;
		case 3: guysbuf[ID].misc4 = val; break;
		case 4: guysbuf[ID].misc5 = val; break;
		case 5: guysbuf[ID].misc6 = val; break;
		case 6: guysbuf[ID].misc7 = val; break;
		case 7: guysbuf[ID].misc8 = val; break;
		case 8: guysbuf[ID].misc9 = val; break;
		case 9: guysbuf[ID].misc10 = val; break;
		case 10: guysbuf[ID].misc11 = val; break;
		case 11: guysbuf[ID].misc12 = val; break;
		case 12: guysbuf[ID].misc13 = val; break;
		case 13: guysbuf[ID].misc14 = val; break;
		case 14: guysbuf[ID].misc15 = val; break;
		default: break;
	}
	
};

//ComboData

//Macros

//Are these right? newcombo is *combo_class_buf and the others are *combobuf

//Getters for ComboData 'Type' submembers. 
#define GET_COMBODATA_TYPE_INT(member) \
{ \
	long ID = vbound((get_register(sarg2) / 10000),0,MAXCOMBOS);\
	set_register(sarg1, combo_class_buf[combobuf[ID].type].member * 10000); \
}

//this may need additional macros. 
//for combo_class_buf[ID].member ?
//I'm not sure which it needs to be at present. 

#define GET_COMBODATA_TYPE_INDEX(member, bound) \
{ \
	int ID = int(vbound((ri->d[0] / 10000),0,MAXCOMBOS));\
	int indx = int(vbound((ri->d[1] / 10000), 0, bound));\
	set_register(sarg1, combo_class_buf[combobuf[ID].type].member[indx] * 10000); \
}

#define GET_COMBODATA_TYPE_FLAG(member) \
{ \
	int ID = int(vbound(ri->d[0] / 10000),0,MAXCOMBOS);\
	int flag = int(ri->d[1] / 10000);\
	set_register(sarg1, (combo_class_buf[combobuf[ID].type].member&flag) ? 10000 : 0); \
}



//Getters for ComboData main members. 
#define GET_COMBODATA_VAR_INT(member) \
{ \
	long ID = vbound( (get_register(sarg2) / 10000), 0, MAXCOMBOS);\
	set_register(sarg1, combobuf[ID].member * 10000); \
}

#define GET_COMBODATA_VAR_INDEX(member, bound) \
{ \
	int ID = int( vbound( (ri->d[0] / 10000),0,MAXCOMBOS) );\
	int indx = int ( vbound( (ri->d[1] / 10000),0,bound) );\
	set_register(sarg1, combobuf[ID].member[indx] * 10000); \
}

#define GET_COMBODATA_VAR_FLAG(member) \
{ \
	int ID = int( vbound( ( ri->d[0] / 10000),0,MAXCOMBOS) );\
	int flag = int(ri->d[1] / 10000);\
	set_register(sarg1, (combobuf[ID].member&flag) ? 10000 : 0); \
}



//ComboData Setter Macros

//Setters for ComboData 'type' submembers.
#define SET_COMBODATA_TYPE_INT(member, bound) \
{ \
	long ID = get_register(sarg1) / 10000; \
	long val = vbound( (get_register(sarg2) / 10000), 0, bound); \
	if(ID < 1 || ID > 511) \
		set_register(sarg1, -10000); \
	else \
		combo_class_buf[combobuf[ID].type].member = val; \
}

#define SET_COMBODATA_TYPE_INDEX(member, val, bound, indexbound) \
{ \
	long ID = vbound((ri->d[0]/10000),0,MAXCOMBOS);  \
	long indx =  vbound((ri->d[1]/10000),0,indexbound);  \
	combo_class_buf[combobuf[ID].type].member[indx] = vbound(val,0,bound); \
}

#define SET_COMBODATA_TYPE_FLAG(member, val, bound) \
{ \
	long ID = vbound((ri->d[0]/10000),0,MAXCOMBOS);  \
	long flag =  (ri->d[1]/10000);  \
	combo_class_buf[combobuf[ID].type].member&flag = ((vbound(val,0,bound))!=0); \
 \


//Setters for ComboData main members
#define SET_COMBODATA_VAR_INT(member, bound) \
{ \
	long ID = vbound( (get_register(sarg1) / 10000), 0, MAXCOMBOS); \
	long val = vbound((get_register(sarg2) / 10000),0,bound); \
	combobuf[ID].member = val; \
}

//SET_NPC_VAR_INDEX(member,value)
#define SET_COMBODATA_VAR_INDEX(member, val, bound, indexbound) \
{ \
	long ID = vbound((ri->d[0]/10000),0,MAXCOMBOS);  \
	long indx =  vbound((ri->d[1]/10000),0,indexbound);  \
	combobuf[ID].member[indx] = vbound(val,0,bound); \
}

//Special case for flags, three inputs one return
#define SET_COMBODATA_VAR_FLAG(member, val, bound) \
{ \
	long ID = vbound((ri->d[0]/10000),0,MAXCOMBOS);  \
	long flag =  (ri->d[1]/10000);  \
	else \
	{ \
		combobuf[ID].member&flag = ((bvound(val,0,bound))!=0); \
	}\
}

//Getters

//one input, one return
void FFScript::getComboData_block_enemies(){ GET_COMBODATA_TYPE_INT(block_enemies); } //byte a
void FFScript::getComboData_block_hole(){ GET_COMBODATA_TYPE_INT(block_hole); } //byte b
void FFScript::getComboData_block_trigger(){ GET_COMBODATA_TYPE_INT(block_trigger); } //byte c
void FFScript::getComboData_conveyor_x_speed(){ GET_COMBODATA_TYPE_INT(conveyor_x_speed); } //short e
void FFScript::getComboData_conveyor_y_speed(){ GET_COMBODATA_TYPE_INT(conveyor_y_speed); } //short f
void FFScript::getComboData_create_enemy(){ GET_COMBODATA_TYPE_INT(create_enemy); } //word g
void FFScript::getComboData_create_enemy_when(){ GET_COMBODATA_TYPE_INT(create_enemy_when); } //byte h
void FFScript::getComboData_create_enemy_change(){ GET_COMBODATA_TYPE_INT(create_enemy_change); } //long i
void FFScript::getComboData_directional_change_type(){ GET_COMBODATA_TYPE_INT(directional_change_type); } //byte j
void FFScript::getComboData_distance_change_tiles(){ GET_COMBODATA_TYPE_INT(distance_change_tiles); } //long k
void FFScript::getComboData_dive_item(){ GET_COMBODATA_TYPE_INT(dive_item); } //short l
void FFScript::getComboData_dock(){ GET_COMBODATA_TYPE_INT(dock); } //byte m
void FFScript::getComboData_fairy(){ GET_COMBODATA_TYPE_INT(fairy); } //byte n
void FFScript::getComboData_ff_combo_attr_change(){ GET_COMBODATA_TYPE_INT(ff_combo_attr_change); } //byte o
void FFScript::getComboData_foot_decorations_tile(){ GET_COMBODATA_TYPE_INT(foot_decorations_tile); } //long p
void FFScript::getComboData_foot_decorations_type(){ GET_COMBODATA_TYPE_INT(foot_decorations_type); } //byte q
void FFScript::getComboData_hookshot_grab_point(){ GET_COMBODATA_TYPE_INT(hookshot_grab_point); } //byte r
void FFScript::getComboData_ladder_pass(){ GET_COMBODATA_TYPE_INT(ladder_pass); } //byte s
void FFScript::getComboData_lock_block_type(){ GET_COMBODATA_TYPE_INT(lock_block_type); } //byte t
void FFScript::getComboData_lock_block_change(){ GET_COMBODATA_TYPE_INT(lock_block_change); } //long u
void FFScript::getComboData_magic_mirror_type(){ GET_COMBODATA_TYPE_INT(magic_mirror_type); } //byte v
void FFScript::getComboData_modify_hp_amount(){ GET_COMBODATA_TYPE_INT(modify_hp_amount); } //short w
void FFScript::getComboData_modify_hp_delay(){ GET_COMBODATA_TYPE_INT(modify_hp_delay); } //byte x
void FFScript::getComboData_modify_hp_type(){ GET_COMBODATA_TYPE_INT(modify_hp_type); } //byte y
void FFScript::getComboData_modify_mp_amount(){ GET_COMBODATA_TYPE_INT(modify_mp_amount); } //short z
void FFScript::getComboData_modify_mp_delay(){ GET_COMBODATA_TYPE_INT(modify_mp_delay); } //byte aa
void FFScript::getComboData_modify_mp_type(){ GET_COMBODATA_TYPE_INT(modify_mp_type); } //byte ab
void FFScript::getComboData_no_push_blocks(){ GET_COMBODATA_TYPE_INT(no_push_blocks); } //byte ac
void FFScript::getComboData_overhead(){ GET_COMBODATA_TYPE_INT(overhead); } //byte ad
void FFScript::getComboData_place_enemy(){ GET_COMBODATA_TYPE_INT(place_enemy); } //byte ae
void FFScript::getComboData_push_direction(){ GET_COMBODATA_TYPE_INT(push_direction); } //byte af
void FFScript::getComboData_push_weight(){ GET_COMBODATA_TYPE_INT(push_weight); } //byte ag  heavy or not
void FFScript::getComboData_push_wait(){ GET_COMBODATA_TYPE_INT(push_wait); } //byte ah
void FFScript::getComboData_pushed(){ GET_COMBODATA_TYPE_INT(pushed); } //byte ai
void FFScript::getComboData_raft(){ GET_COMBODATA_TYPE_INT(raft); } //byte aj
void FFScript::getComboData_reset_room(){ GET_COMBODATA_TYPE_INT(reset_room); } //byte ak
void FFScript::getComboData_save_point_type(){ GET_COMBODATA_TYPE_INT(save_point_type); } //byte al
void FFScript::getComboData_screen_freeze_type(){ GET_COMBODATA_TYPE_INT(screen_freeze_type); } //byte am

void FFScript::getComboData_secret_combo(){ GET_COMBODATA_TYPE_INT(secret_combo); } //byte an
void FFScript::getComboData_singular(){ GET_COMBODATA_TYPE_INT(singular); } //byte ao
void FFScript::getComboData_slow_movement(){ GET_COMBODATA_TYPE_INT(slow_movement); } //byte ap
void FFScript::getComboData_statue_type(){ GET_COMBODATA_TYPE_INT(statue_type); } //byte aq
void FFScript::getComboData_step_type(){ GET_COMBODATA_TYPE_INT(step_type); } //byte ar
void FFScript::getComboData_step_change_to(){ GET_COMBODATA_TYPE_INT(step_change_to); } //long as
void FFScript::getComboData_strike_remnants(){ GET_COMBODATA_TYPE_INT(strike_remnants); } //long au
void FFScript::getComboData_strike_remnants_type(){ GET_COMBODATA_TYPE_INT(strike_remnants_type); } //byte av
void FFScript::getComboData_strike_change(){ GET_COMBODATA_TYPE_INT(strike_change); } //long aw
void FFScript::getComboData_strike_item(){ GET_COMBODATA_TYPE_INT(strike_item); } //short ax
void FFScript::getComboData_touch_item(){ GET_COMBODATA_TYPE_INT(touch_item); } //short ay
void FFScript::getComboData_touch_stairs(){ GET_COMBODATA_TYPE_INT(touch_stairs); } //byte az
void FFScript::getComboData_trigger_type(){ GET_COMBODATA_TYPE_INT(trigger_type); } //byte ba
void FFScript::getComboData_trigger_sensitive(){ GET_COMBODATA_TYPE_INT(trigger_sensitive); } //byte bb
void FFScript::getComboData_warp_type(){ GET_COMBODATA_TYPE_INT(warp_type); } //byte bc
void FFScript::getComboData_warp_sensitive(){ GET_COMBODATA_TYPE_INT(warp_sensitive); } //byte bd
void FFScript::getComboData_warp_direct(){ GET_COMBODATA_TYPE_INT(warp_direct); } //byte be
void FFScript::getComboData_warp_location(){ GET_COMBODATA_TYPE_INT(warp_location); } //byte bf
void FFScript::getComboData_water(){ GET_COMBODATA_TYPE_INT(water); } //byte bg
void FFScript::getComboData_whistle(){ GET_COMBODATA_TYPE_INT(whistle); } //byte bh
void FFScript::getComboData_win_game(){ GET_COMBODATA_TYPE_INT(win_game); } //byte bi
void FFScript::getComboData_block_weapon_lvl(){ GET_COMBODATA_TYPE_INT(block_weapon_lvl); } //byte bj - max level of weapon to block

void FFScript::getComboData_tile(){ GET_COMBODATA_VAR_INT(tile); } //newcombo, word
void FFScript::getComboData_flip(){ GET_COMBODATA_VAR_INT(flip); } //newcombo byte

void FFScript::getComboData_walk(){ GET_COMBODATA_VAR_INT(walk); } //newcombo byte
void FFScript::getComboData_type(){ GET_COMBODATA_VAR_INT(type); } //newcombo byte
void FFScript::getComboData_csets(){ GET_COMBODATA_VAR_INT(csets); } //newcombo byte
void FFScript::getComboData_foo(){ GET_COMBODATA_VAR_INT(foo); } //newcombo word
void FFScript::getComboData_frames(){ GET_COMBODATA_VAR_INT(frames); } //newcombo byte
void FFScript::getComboData_speed(){ GET_COMBODATA_VAR_INT(speed); } //newcombo byte
void FFScript::getComboData_nextcombo(){ GET_COMBODATA_VAR_INT(nextcombo); } //newcombo word
void FFScript::getComboData_nextcset(){ GET_COMBODATA_VAR_INT(nextcset); } //newcombo byte
void FFScript::getComboData_flag(){ GET_COMBODATA_VAR_INT(flag); } //newcombo byte
void FFScript::getComboData_skipanim(){ GET_COMBODATA_VAR_INT(skipanim); } //newcombo byte
void FFScript::getComboData_nexttimer(){ GET_COMBODATA_VAR_INT(nexttimer); } //newcombo word
void FFScript::getComboData_skipanimy(){ GET_COMBODATA_VAR_INT(skipanimy); } //newcombo byte
void FFScript::getComboData_animflags(){ GET_COMBODATA_VAR_INT(animflags); } //newcombo byte


//two inputs, one return
void FFScript::getComboData_block_weapon(){ GET_COMBODATA_TYPE_INDEX(block_weapon,32); } //byte array[32] d (ID of LWeapon)
void FFScript::getComboData_expansion(){ GET_COMBODATA_VAR_INDEX(expansion,6); } //newcombo byte, arr[6]
void FFScript::getComboData_strike_weapons(){ GET_COMBODATA_TYPE_INDEX(strike_weapons,32); } //byte at, arr[32]

//Setters, two inputs no returns

void FFScript::setComboData_block_enemies(){ SET_COMBODATA_TYPE_INT(block_enemies,ZS_BYTE); } //byte a
void FFScript::setComboData_block_hole(){ SET_COMBODATA_TYPE_INT(block_hole,ZS_BYTE); } //byte b
void FFScript::setComboData_block_trigger(){ SET_COMBODATA_TYPE_INT(block_trigger,ZS_BYTE); } //byte c
void FFScript::setComboData_conveyor_x_speed(){ SET_COMBODATA_TYPE_INT(conveyor_x_speed,ZS_SHORT); } //short e
void FFScript::setComboData_conveyor_y_speed(){ SET_COMBODATA_TYPE_INT(conveyor_y_speed,ZS_SHORT); } //short f
void FFScript::setComboData_create_enemy(){ SET_COMBODATA_TYPE_INT(create_enemy,ZS_WORD); } //word g
void FFScript::setComboData_create_enemy_when(){ SET_COMBODATA_TYPE_INT(create_enemy_when,ZS_BYTE); } //byte h
void FFScript::setComboData_create_enemy_change(){ SET_COMBODATA_TYPE_INT(create_enemy_change,ZS_LONG); } //long i
void FFScript::setComboData_directional_change_type(){ SET_COMBODATA_TYPE_INT(directional_change_type,ZS_BYTE); } //byte j
void FFScript::setComboData_distance_change_tiles(){ SET_COMBODATA_TYPE_INT(distance_change_tiles,ZS_LONG); } //long k
void FFScript::setComboData_dive_item(){ SET_COMBODATA_TYPE_INT(dive_item,ZS_SHORT); } //short l
void FFScript::setComboData_dock(){ SET_COMBODATA_TYPE_INT(dock,ZS_BYTE); } //byte m
void FFScript::setComboData_fairy(){ SET_COMBODATA_TYPE_INT(fairy,ZS_BYTE); } //byte n
void FFScript::setComboData_ff_combo_attr_change(){ SET_COMBODATA_TYPE_INT(ff_combo_attr_change,ZS_BYTE); } //byte o
void FFScript::setComboData_foot_decorations_tile(){ SET_COMBODATA_TYPE_INT(foot_decorations_tile,ZS_LONG); } //long p
void FFScript::setComboData_foot_decorations_type(){ SET_COMBODATA_TYPE_INT(foot_decorations_type,ZS_BYTE); } //byte q
void FFScript::setComboData_hookshot_grab_point(){ SET_COMBODATA_TYPE_INT(hookshot_grab_point,ZS_BYTE); } //byte r
void FFScript::setComboData_ladder_pass(){ SET_COMBODATA_TYPE_INT(ladder_pass,ZS_BYTE); } //byte s
void FFScript::setComboData_lock_block_type(){ SET_COMBODATA_TYPE_INT(lock_block_type,ZS_BYTE); } //byte t
void FFScript::setComboData_lock_block_change(){ SET_COMBODATA_TYPE_INT(lock_block_change,ZS_LONG); } //long u
void FFScript::setComboData_magic_mirror_type(){ SET_COMBODATA_TYPE_INT(magic_mirror_type,ZS_BYTE); } //byte v
void FFScript::setComboData_modify_hp_amount(){ SET_COMBODATA_TYPE_INT(modify_hp_amount,ZS_SHORT); } //short w
void FFScript::setComboData_modify_hp_delay(){ SET_COMBODATA_TYPE_INT(modify_hp_delay,ZS_BYTE); } //byte x
void FFScript::setComboData_modify_hp_type(){ SET_COMBODATA_TYPE_INT(modify_hp_type,ZS_BYTE); } //byte y
void FFScript::setComboData_modify_mp_amount(){ SET_COMBODATA_TYPE_INT(modify_mp_amount,ZS_SHORT); } //short z
void FFScript::setComboData_modify_mp_delay(){ SET_COMBODATA_TYPE_INT(modify_mp_delay,ZS_BYTE); } //byte aa
void FFScript::setComboData_modify_mp_type(){ SET_COMBODATA_TYPE_INT(modify_mp_type,ZS_BYTE); } //byte ab
void FFScript::setComboData_no_push_blocks(){ SET_COMBODATA_TYPE_INT(no_push_blocks,ZS_BYTE); } //byte ac
void FFScript::setComboData_overhead(){ SET_COMBODATA_TYPE_INT(overhead,ZS_BYTE); } //byte ad
void FFScript::setComboData_place_enemy(){ SET_COMBODATA_TYPE_INT(place_enemy,ZS_BYTE); } //byte ae
void FFScript::setComboData_push_direction(){ SET_COMBODATA_TYPE_INT(push_direction,ZS_BYTE); } //byte af
void FFScript::setComboData_push_weight(){ SET_COMBODATA_TYPE_INT(push_weight,ZS_BYTE); } //byte ag  heavy or not
void FFScript::setComboData_push_wait(){ SET_COMBODATA_TYPE_INT(push_wait,ZS_BYTE); } //byte ah
void FFScript::setComboData_pushed(){ SET_COMBODATA_TYPE_INT(pushed,ZS_BYTE); } //byte ai
void FFScript::setComboData_raft(){ SET_COMBODATA_TYPE_INT(raft,ZS_BYTE); } //byte aj
void FFScript::setComboData_reset_room(){ SET_COMBODATA_TYPE_INT(reset_room,ZS_BYTE); } //byte ak
void FFScript::setComboData_save_point_type(){ SET_COMBODATA_TYPE_INT(save_point_type,ZS_BYTE); } //byte al
void FFScript::setComboData_screen_freeze_type(){ SET_COMBODATA_TYPE_INT(screen_freeze_type,ZS_BYTE); } //byte am

void FFScript::setComboData_secret_combo(){ SET_COMBODATA_TYPE_INT(secret_combo,ZS_BYTE); } //byte an
void FFScript::setComboData_singular(){ SET_COMBODATA_TYPE_INT(singular,ZS_BYTE); } //byte ao
void FFScript::setComboData_slow_movement(){ SET_COMBODATA_TYPE_INT(slow_movement,ZS_BYTE); } //byte ap
void FFScript::setComboData_statue_type(){ SET_COMBODATA_TYPE_INT(statue_type,ZS_BYTE); } //byte aq
void FFScript::setComboData_step_type(){ SET_COMBODATA_TYPE_INT(step_type,ZS_BYTE); } //byte ar
void FFScript::setComboData_step_change_to(){ SET_COMBODATA_TYPE_INT(step_change_to,ZS_LONG); } //long as

void FFScript::setComboData_strike_remnants(){ SET_COMBODATA_TYPE_INT(strike_remnants,ZS_LONG); } //long au
void FFScript::setComboData_strike_remnants_type(){ SET_COMBODATA_TYPE_INT(strike_remnants_type,ZS_BYTE); } //byte av
void FFScript::setComboData_strike_change(){ SET_COMBODATA_TYPE_INT(strike_change,ZS_LONG); } //long aw
void FFScript::setComboData_strike_item(){ SET_COMBODATA_TYPE_INT(strike_item,ZS_SHORT); } //short ax
void FFScript::setComboData_touch_item(){ SET_COMBODATA_TYPE_INT(touch_item,ZS_SHORT); } //short ay
void FFScript::setComboData_touch_stairs(){ SET_COMBODATA_TYPE_INT(touch_stairs,ZS_BYTE); } //byte az
void FFScript::setComboData_trigger_type(){ SET_COMBODATA_TYPE_INT(trigger_type,ZS_BYTE); } //byte ba
void FFScript::setComboData_trigger_sensitive(){ SET_COMBODATA_TYPE_INT(trigger_sensitive,ZS_BYTE); } //byte bb
void FFScript::setComboData_warp_type(){ SET_COMBODATA_TYPE_INT(warp_type,ZS_BYTE); } //byte bc
void FFScript::setComboData_warp_sensitive(){ SET_COMBODATA_TYPE_INT(warp_sensitive,ZS_BYTE); } //byte bd
void FFScript::setComboData_warp_direct(){ SET_COMBODATA_TYPE_INT(warp_direct,ZS_BYTE); } //byte be
void FFScript::setComboData_warp_location(){ SET_COMBODATA_TYPE_INT(warp_location,ZS_BYTE); } //byte bf
void FFScript::setComboData_water(){ SET_COMBODATA_TYPE_INT(water,ZS_BYTE); } //byte bg
void FFScript::setComboData_whistle(){ SET_COMBODATA_TYPE_INT(whistle,ZS_BYTE); } //byte bh
void FFScript::setComboData_win_game(){ SET_COMBODATA_TYPE_INT(win_game,ZS_BYTE); } //byte bi
void FFScript::setComboData_block_weapon_lvl(){ SET_COMBODATA_TYPE_INT(block_weapon_lvl,ZS_BYTE); } //byte bj - max level of weapon to block

//combosbuf
void FFScript::setComboData_tile(){ SET_COMBODATA_VAR_INT(tile,ZS_WORD); } //newcombo, word
void FFScript::setComboData_flip(){ SET_COMBODATA_VAR_INT(flip,ZS_BYTE); } //newcombo byte

void FFScript::setComboData_walk(){ SET_COMBODATA_VAR_INT(walk,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_type(){ SET_COMBODATA_VAR_INT(type,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_csets(){ SET_COMBODATA_VAR_INT(csets,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_foo(){ SET_COMBODATA_VAR_INT(foo,ZS_WORD); } //newcombo word
void FFScript::setComboData_frames(){ SET_COMBODATA_VAR_INT(frames,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_speed(){ SET_COMBODATA_VAR_INT(speed,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_nextcombo(){ SET_COMBODATA_VAR_INT(nextcombo,ZS_WORD); } //newcombo word
void FFScript::setComboData_nextcset(){ SET_COMBODATA_VAR_INT(nextcset,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_flag(){ SET_COMBODATA_VAR_INT(flag,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_skipanim(){ SET_COMBODATA_VAR_INT(skipanim,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_nexttimer(){ SET_COMBODATA_VAR_INT(nexttimer,ZS_WORD); } //newcombo word
void FFScript::setComboData_skipanimy(){ SET_COMBODATA_VAR_INT(skipanimy,ZS_BYTE); } //newcombo byte
void FFScript::setComboData_animflags(){ SET_COMBODATA_VAR_INT(animflags,ZS_BYTE); } //newcombo byte

//three inputs, no returns
void FFScript::setComboData_block_weapon(int v){ SET_COMBODATA_TYPE_INDEX(block_weapon,v,ZS_BYTE,32); } //byte array[32] d (ID of LWeapon)
void FFScript::setComboData_strike_weapons(int v){ SET_COMBODATA_TYPE_INDEX(strike_weapons,v,ZS_BYTE,32); } //byte at, arr[32]
void FFScript::setComboData_expansion(int v){ SET_COMBODATA_VAR_INDEX(expansion,v,ZS_BYTE,6); } //newcombo byte, arr[6]

//SpriteData Macros
#define GET_SPRITEDATA_TYPE_INT(member) \
{ \
	long ID = vbound((get_register(sarg2) / 10000),0,255);\
	set_register(sarg1, wpnsbuf[ID].member * 10000); \
}

#define SET_SPRITEDATA_TYPE_INT(member, bound) \
{ \
	long ID = get_register(sarg1) / 10000; \
	long val = vbound( (get_register(sarg2) / 10000), 0, bound); \
	if(ID < 1 || ID > 255) \
		set_register(sarg1, -10000); \
	else \
		wpnsbuf[ID].member = val; \
}

#define SET_SPRITEDATA_TYPE_INT_NOBOUND(member) \
{ \
	long ID = get_register(sarg1) / 10000; \
	long val = get_register(sarg2) / 10000; \
	if(ID < 1 || ID > 255) \
		set_register(sarg1, -10000); \
	else \
		wpnsbuf[ID].member = val; \
}


void FFScript::getSpriteDataTile(){GET_SPRITEDATA_TYPE_INT(tile);}
void FFScript::getSpriteDataMisc(){GET_SPRITEDATA_TYPE_INT(misc);}
void FFScript::getSpriteDataCSets(){GET_SPRITEDATA_TYPE_INT(csets);}
void FFScript::getSpriteDataFrames(){GET_SPRITEDATA_TYPE_INT(frames);}
void FFScript::getSpriteDataSpeed(){GET_SPRITEDATA_TYPE_INT(speed);}
void FFScript::getSpriteDataType(){GET_SPRITEDATA_TYPE_INT(type);}
//void FFScript::getSpriteDataString();



void FFScript::setSpriteDataTile(){SET_SPRITEDATA_TYPE_INT(tile,ZS_WORD);}
void FFScript::setSpriteDataMisc(){SET_SPRITEDATA_TYPE_INT(misc,ZS_CHAR);}
void FFScript::setSpriteDataCSets(){SET_SPRITEDATA_TYPE_INT(csets,ZS_CHAR);}
void FFScript::setSpriteDataFrames(){SET_SPRITEDATA_TYPE_INT(frames,ZS_CHAR);}
void FFScript::setSpriteDataSpeed(){SET_SPRITEDATA_TYPE_INT(speed,ZS_CHAR);}
void FFScript::setSpriteDataType(){SET_SPRITEDATA_TYPE_INT(type,ZS_CHAR);}
//void FFScript::setSpriteDataString();


void FFScript::do_setMIDI_volume(int m)
{
	master_volume(-1,(vbound(m,0,255)));
}
void FFScript::do_setMusic_volume(int m)
{
	emusic_volume = vbound(m,0,255);
}
void FFScript::do_setDIGI_volume(int m)
{
	master_volume((vbound(m,0,255)),-1);
}
void FFScript::do_setSFX_volume(int m)
{
	sfx_volume = m;
}
void FFScript::do_setSFX_pan(int m)
{
	pan_style = vbound(m,0,3);
}
int FFScript::do_getMIDI_volume()
{
	return ((int)midi_volume);
}
int FFScript::do_getMusic_volume()
{
	return ((int)emusic_volume);
}
int FFScript::do_getDIGI_volume()
{
	return ((int)digi_volume);
}
int FFScript::do_getSFX_volume()
{
	return ((int)sfx_volume);
}
int FFScript::do_getSFX_pan()
{
	return ((int)pan_style);
}


//Change Game Over Screen Values
void FFScript::FFSetSaveScreenSetting() 
{
	
	long indx = get_register(sarg1) / 10000; 
	long value = get_register(sarg2) / 10000; //bounded in zelda.cpp 
	if(indx < 0 || indx > 11) 
		set_register(sarg1, -10000); 
	else 
		SetSaveScreenSetting(indx, value); 
}
	
	
	
void FFScript::FFChangeSubscreenText() 
{ 
	
    long index = get_register(sarg1) / 10000;
    long arrayptr = get_register(sarg2) / 10000;
    
	if ( index < 0 || index > 3 ) 
	{
		al_trace("The index supplied to Game->SetSubscreenText() is invalid. The index specified was: %d /n", index);
		return;
	}

	string filename_str;

        
        
	ArrayH::getString(arrayptr, filename_str, 73);
	ChangeSubscreenText(index,filename_str.c_str());
	
	//newtext[32]='\0';
	
    
	
}

void FFScript::do_typedpointer_typecast(const bool v)
{
     long ptr = SH::get_arg(sarg1, v);
     set_register(sarg1, ptr);
}


void FFScript::setFFRules()
{
	for ( int q = 0; q < QUESTRULES_SIZE; q++ )
	{
		FF_rules[q] = getQRBit(q);
	}
	for ( int q = QUESTRULES_SIZE; q < QUESTRULES_SIZE+EXTRARULES_SIZE; q++ ) 
	{
		FF_rules[q] = extra_rules[q-QUESTRULES_SIZE];
	}
	for ( int q = QUESTRULES_SIZE+EXTRARULES_SIZE; q < FFRULES_SIZE; q++ )
	{
		FF_rules[q] = 0; //wipe the rest.
	}
	for ( int q = 0; q < 2; q++ )
	{
		passive_subscreen_offsets[q] = 0;
	}
	active_subscreen_scrollspeed_adjustment = 0;
	//zinit.terminalv
	FF_gravity = zinit.gravity;
	FF_terminalv = zinit.terminalv;
	FF_msg_speed = zinit.msg_speed;
	FF_transition_type = zinit.transition_type; // Can't edit, yet.
	FF_jump_link_layer_threshold = zinit.jump_link_layer_threshold; // Link is drawn above layer 3 if z > this.
	FF_link_swim_speed = zinit.link_swim_speed;
	for ( int q = 0; q < MAXITEMS; q++ )
	{
		item_messages_played[q] = 0;
	}
}

void FFScript::SetItemMessagePlayed(int itm)
{
	item_messages_played[itm] = 1;
}
bool FFScript::GetItemMessagePlayed(int itm)
{
	return (( item_messages_played[itm] ) ? true : false);
}

void FFScript::setRule(int rule, bool s)
{
	FF_rules[rule] = ( s ? 1 : 0 );
}

bool FFScript::getRule(int rule)
{
	return ( FF_rules[rule] != 0 );
}

int FFScript::getQRBit(int rule)
{
	return ( get_bit(quest_rules,rule) ? 1 : 0 );
}

void FFScript::setLinkTile(int t)
{
	FF_link_tile = vbound(t, 0, NEWMAXTILES);
}

void FFScript::setLinkAction(int a)
{
	FF_link_action = vbound(a, 0, 255);
}

int FFScript::getLinkTile()
{
	return FF_link_tile;
}

int FFScript::getLinkAction()
{
	int special_action = Link.getAction2();
	if ( special_action != -1 ) return special_action; //spin, dive, charge
	else return FF_link_action; //everything else
}
//get_bit

int FFScript::GetScriptObjectUID(int type)
{
	++script_UIDs[type];
	return script_UIDs[type];
}

/*
FFScript::FFScript()
{
	init();
}
*/
void FFScript::init()
{
	coreflags = 0;
	for ( int q = 0; q < UID_TYPES; ++q ) { script_UIDs[q] = 0; }
	//for ( int q = 0; q < 512; q++ ) FF_rules[q] = 0;
	setFFRules(); //copy the quest rules over. 
	long usr_midi_volume = usr_digi_volume = usr_sfx_volume = usr_music_volume = usr_panstyle = 0;
	FF_link_tile = 0; FF_link_action = 0;
	for ( int q = 0; q < 4; q++ ) 
	{
		FF_screenbounds[q] = 0;
		FF_screen_dimensions[q] = 0;
		FF_subscreen_dimensions[q] = 0;
		FF_eweapon_removal_bounds[q] = 0; 
		FF_lweapon_removal_bounds[q] = 0;
	}
	for ( int q = 0; q < FFSCRIPTCLASS_CLOCKS; q++ )
	{
		FF_clocks[q] = 0;
	}
	for ( int q = 0; q < SCRIPT_DRAWING_RULES; q++ )
	{
		ScriptDrawingRules[q] = 0;
	}
	for ( int q = 0; q < NUM_USER_MIDI_OVERRIDES; q++ ) 
	{
		FF_UserMidis[q] = 0;
	}
	subscreen_scroll_speed = 0; //make a define for a default and read quest override! -Z
	kb_typing_mode = false;
	
}


void FFScript::SetFFEngineFlag(int flag, bool state)
{
	if ( state ) { coreflags |= flag; }
	else coreflags &= ~flag;
}

void FFScript::setSubscreenScrollSpeed(byte n)
{
	subscreen_scroll_speed = n;
}

int FFScript::getSubscreenScrollSpeed()
{
	return (int)subscreen_scroll_speed;
}

void FFScript::do_greyscale(const bool v)
{
    bool on = (SH::get_arg(sarg1, v)) != 0;
    setMonochrome(on);
}
void FFScript::do_fx_zap(const bool v)
{
    long out = SH::get_arg(sarg1, v);

    if ( out ) { FFScript::do_zapout(); } 
    else FFScript::do_zapin();
}

void FFScript::do_fx_wavy(const bool v)
{
    long out = SH::get_arg(sarg1, v);

    if ( out ) { FFScript::do_wavyout(); } 
    else FFScript::do_wavyin();
}
/*
void FFScript::init()
{
	for ( int q = 0; q < FFRULES_SIZE; q++ ) FF_rules[q] = 0;
	FF_link_tile = 0;
	FF_link_action = 0;
	for ( int q = 0; q < 4; q++ ) 
	{
		FF_screenbounds[q] = 0;
		FF_screen_dimensions[q] = 0;
		FF_subscreen_dimensions[q] = 0;
		FF_eweapon_removal_bounds[q] = 0;
		FF_lweapon_removal_bounds[q] = 0;
		
	}
	for ( int q = 0; q < FFSCRIPTCLASS_CLOCKS; q++ ) FF_clocks[q] = 0;
	for ( int q = 0; q < SCRIPT_DRAWING_RULES; q++ ) ScriptDrawingRules[q] = 0;
	for ( int q = 0; q < NUM_USER_MIDI_OVERRIDES; q++ ) FF_UserMidis[q] = 0;
}
*/
