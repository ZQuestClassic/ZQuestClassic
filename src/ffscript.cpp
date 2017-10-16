#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

//#include <deque>
//#include <algorithm>
#include <string>
#include <sstream>
#include <math.h>
#include <cstdio>
#include "scripting/ZASMdefs.h"
#include "scripting/bindings/IEnemy.h"
#include "scripting/bindings/IItem.h"
#include "scripting/bindings/IWeapon.h"
#include "ffasm.h"
#include "zc_sys.h"
#include "zc_math.h"
#include "zc_array.h"
#include "ffscript.h"
#include "zelda.h"
#include "link.h"
#include "guys.h"
#include "zc_init.h"
#include "zsys.h"
#include "title.h"
#include "mem_debug.h"
#include "rendertarget.h"
#include "backend/AllBackends.h"
#include "pal.h"
#include "zdefs.h"
#include "zq_class.h"

#ifdef _FFDEBUG
#include "ffdebug.h"
#endif

#include "debug.h"

#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))

#ifdef _MSC_VER
#pragma warning ( disable : 4800 ) //int to bool town. population: lots.
#endif

using std::string;
using namespace ZAsm;

extern sprite_list particles;
extern LinkClass *Link;
extern int skipcont;
extern std::map<int, std::pair<string,string> > ffcmap;
extern std::map<int, std::pair<string,string> > itemmap;
extern std::map<int, std::pair<string,string> > globalmap;
extern GameScripts scripts;

PALETTE tempgreypal; //Palettes go here. This is used for Greyscale() / Monochrome()

int virtualScreenScale();

//We gain some speed by not passing as arguments
long sarg1 = 0;
long sarg2 = 0;
refInfo *ri = NULL;
ZAsmScript *curscript = NULL;

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
long(*stack)[256] = NULL;
long ffc_stack[32][256];
long global_stack[256];
long item_stack[256];
long ffmisc[32][16];
refInfo ffcScriptData[32];




int FFScript::getrule(int rule){
	return -1;
}

void FFScript::setrule(int rule, bool state){
	
}

//ScriptHelper
class SH : public FFScript
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
        // TODO quest module scripting support
        return checkBounds(ID, 0, curQuest->getModule("CORE").enemyDefTable().getNumEnemyDefinitions()-1, str);
    }
    
    static INLINE int checkItemID(const long ID, const char * const str)
    {
        // TODO quest module scripting support
        return checkBounds(ID, 0, curQuest->getModule("CORE").itemDefTable().getNumItemDefinitions()-1, str);
    }
    
    static INLINE int checkWeaponID(const long ID, const char * const str)
    {
        // TODO quest module scripting support
        return checkBounds(ID, 0, curQuest->getModule("CORE").spriteDefTable().getNumSpriteDefinitions()-1, str);
    }
    
    static INLINE int checkWeaponMiscSprite(const long ID, const char * const str)
    {
        // TODO quest module scripting support
        return checkBounds(ID, 0, curQuest->getModule("CORE").spriteDefTable().getNumSpriteDefinitions()-1, str);
    }
    
    static INLINE int checkSFXID(const long ID, const char * const str)
    {
        return checkBounds(ID, 0, Backend::sfx->numSlots()-1, str);
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
		tempenemy = IEnemy(*pool, eid).getPtr();
        
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
    //Fized the size of this array. There are 15 total attribs, [0] to [14], not [0] to [9]. -Z
    static long getNPCDMisc(const byte a)
    {
        if (a <= 15)
        {
            return tempenemy->dmiscs[a];
        }
        else
        {
            Z_scripterrlog("Enemy misc index out of bounds: %d\n", a);
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
        tempitem = IItem(*pool, iid).getPtr();
        
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
        tempweapon = IWeapon(*pool, wid).getPtr();
        
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
        tempweapon = IWeapon(*pool, wid).getPtr();
        
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

//THis would be better if 't'were in a header. -Z

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

	// Non std::string version. Requires exact length, array must be valid.
	static void uncheckedGetCString(const long ptr, char* str, int stringLength)
	{
		ZScriptArray& a = getArray(ptr);

		int i = 0;
		if(a != INVALIDARRAY)
		{
			for( ; i < stringLength; ++i)
			{
				str[i] = char(a[i] / 10000);
			}
		}

		str[i] = 0; //null terminate.
	}
    
	//Why does this lose error status??
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



item *checkItem(long iid)
{
    item *s = IItem(*pool, iid).getPtr();
    
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
    weapon *s = IWeapon(*pool, eid).getPtr();
    
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
    weapon *s = IWeapon(*pool, eid).getPtr();
    
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





sprite *s;


long get_itemdata_register(long arg);
void set_itemdata_register(long arg, long value);


long get_register(const long arg)
{
    long ret = 0;
    
    switch (arg)
    {
///----------------------------------------------------------------------------------------------------//
//FFC Variables
    case VarId_DATA:
        ret = tmpscr->ffdata[ri->ffcref]*10000;
        break;
        
    case VarId_FFSCRIPT:
        ret = tmpscr->ffscript[ri->ffcref]*10000;
        break;
        
    case VarId_CSET:
        ret = tmpscr->ffcset[ri->ffcref]*10000;
        break;
        
    case VarId_DELAY:
        ret = tmpscr->ffdelay[ri->ffcref]*10000;
        break;
        
    case VarId_X:
        ret = tmpscr->ffx[ri->ffcref];
        break;
        
    case VarId_Y:
        ret = tmpscr->ffy[ri->ffcref];
        break;
        
    case VarId_XD:
        ret = tmpscr->ffxdelta[ri->ffcref];
        break;
        
    case VarId_YD:
        ret = tmpscr->ffydelta[ri->ffcref];
        break;
    case VarId_FFCID:
        ret=(ri->ffcref*10000)+10000;
        break;
        
    case VarId_XD2:
        ret = tmpscr->ffxdelta2[ri->ffcref];
        break;
        
    case VarId_YD2:
        ret = tmpscr->ffydelta2[ri->ffcref];
        break;
        
    case VarId_FFFLAGSD:
        ret=((tmpscr->ffflags[ri->ffcref] >> (ri->d[0] / 10000))&1) ? 10000 : 0;
        break;
        
    case VarId_FFCWIDTH:
        ret=((tmpscr->ffwidth[ri->ffcref]&0x3F)+1)*10000;
        break;
        
    case VarId_FFCHEIGHT:
        ret=((tmpscr->ffheight[ri->ffcref]&0x3F)+1)*10000;
        break;
        
    case VarId_FFTWIDTH:
        ret=((tmpscr->ffwidth[ri->ffcref]>>6)+1)*10000;
        break;
        
    case VarId_FFTHEIGHT:
        ret=((tmpscr->ffheight[ri->ffcref]>>6)+1)*10000;
        break;
        
    case VarId_FFLINK:
        ret=(tmpscr->fflink[ri->ffcref])*10000;
        break;
        
    case VarId_FFMISCD:
    {
        int a = ri->d[0] / 10000;
        
        if(BC::checkMisc(a, "ffc->Misc") != SH::_NoError)
            ret = -10000;
        else
            ret = ffmisc[ri->ffcref][a];
    }
    break;
    
    case VarId_FFINITDD:
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
    case VarId_LINKX:
        ret = long(Link->getX()) * 10000;
        break;
        
    case VarId_LINKY:
        ret = long(Link->getY()) * 10000;
        break;
        
    case VarId_LINKZ:
        ret = long(Link->getZ()) * 10000;
        break;
        
    case VarId_LINKJUMP:
        // -fall/100*10000, but doing it that way screwed up the result
        ret = long(-Link->getFall()) * 100;
        break;
        
    case VarId_LINKDIR:
        ret=(int)(Link->dir)*10000;
        break;
        
    case VarId_LINKHITDIR:
        ret=(int)(Link->getHitDir())*10000;
        break;
        
    case VarId_LINKHP:
        ret=(int)(game->get_life())*10000;
        break;
        
    case VarId_LINKMP:
        ret=(int)(game->get_magic())*10000;
        break;
        
    case VarId_LINKMAXHP:
        ret=(int)(game->get_maxlife())*10000;
        break;
        
    case VarId_LINKMAXMP:
        ret=(int)(game->get_maxmagic())*10000;
        break;
        
    case VarId_LINKACTION:
        ret=(int)(Link->getAction())*10000;
        break;
        
    case VarId_LINKHELD:
    {
        //TODO script support for modules
        ret = (int)(Link->getHeldItem().slot) * 10000;
        break;
    }
        
    case VarId_LINKITEMD:
    {
        ItemDefinitionRef ref("CORE", vbound(ri->d[0] / 10000, 0, curQuest->getModule("CORE").itemDefTable().getNumItemDefinitions() - 1));
        // TODO quest module support
        ret = game->get_item(ref) ? 10000 : 0;
        break;
    }
        
    case VarId_LINKEQUIP:
    {
        //TODO this is a pretty useless function, and should only be used in old quests
        // first of all, it doesn't work if Link is wielding an item with id > 255
        // second, module support is missing
        ret = ((Awpn.slot & 0xFF) | ((Bwpn.slot & 0xFF) << 8)) * 10000;
        break;
    }
        
    case VarId_LINKINVIS:
        ret = (int)(Link->getDontDraw())*10000;
        break;
        
    case VarId_LINKINVINC:
        ret = (int)(Link->scriptcoldet)*10000;
        break;
        
    case VarId_LINKLADDERX:
        ret=(int)(Link->getLadderX())*10000;
        break;
        
    case VarId_LINKLADDERY:
        ret=(int)(Link->getLadderY())*10000;
        break;
        
    case VarId_LINKSWORDJINX:
        ret = (int)(Link->getSwordClk())*10000;
        break;
        
    case VarId_LINKITEMJINX:
        ret = (int)(Link->getItemClk())*10000;
        break;
        
    case VarId_LINKDRUNK:
        ret = (int)(Link->DrunkClock())*10000;
        break;
        
    case VarId_LINKMISCD:
        ret = (int)(Link->miscellaneous[vbound(ri->d[0]/10000,0,31)]);
        break;
        
    case VarId_LINKHXOFS:
        ret = (int)(Link->hxofs)*10000;
        break;
        
    case VarId_LINKHYOFS:
        ret = (int)(Link->hyofs)*10000;
        break;
        
    case VarId_LINKXOFS:
        ret = (int)(Link->xofs)*10000;
        break;
        
    case VarId_LINKYOFS:
        ret = (int)(Link->yofs-playing_field_offset)*10000;
        break;
        
    case VarId_LINKZOFS:
        ret = (int)(Link->zofs)*10000;
        break;
        
    case VarId_LINKHXSZ:
        ret = (int)(Link->hxsz)*10000;
        break;
        
    case VarId_LINKHYSZ:
        ret = (int)(Link->hysz)*10000;
        break;
        
    case VarId_LINKHZSZ:
        ret = (int)(Link->hzsz)*10000;
        break;
        
    case VarId_LINKTXSZ:
        ret = (int)(Link->txsz)*10000;
        break;
        
    case VarId_LINKTYSZ:
        ret = (int)(Link->tysz)*10000;
        break;
        
    case VarId_LINKTILE:
        ret = (int)(Link->tile)*10000;
        break;
        
    case VarId_LINKFLIP:
        ret = (int)(Link->flip)*10000;
        break;
    
    case VarId_LINKINVFRAME:
	ret = (int)Link->getHClk()*10000;
	break;
    
    case VarId_LINKCANFLICKER:
        ret= Link->getCanLinkFlicker()?10000:0;
        break;
    case VarId_LINKHURTSFX:
	ret = (int)Link->getHurtSFX()*10000;
	break;
    
    case VarId_LINKEATEN:
	ret=(int)Link->getEaten()*10000;
	break;
        
    case VarId_LINKDIAG:
	ret=Link->getDiagMove()?10000:0;
	break;
    
    case VarId_LINKBIGHITBOX:
	ret=Link->getBigHitbox()?10000:0;
	break;
    
    
///----------------------------------------------------------------------------------------------------//
//Input States
    case VarId_INPUTSTART:
        ret=control_state[6]?10000:0;
        break;
        
    case VarId_INPUTMAP:
        ret=control_state[9]?10000:0;
        break;
        
    case VarId_INPUTUP:
        ret=control_state[0]?10000:0;
        break;
        
    case VarId_INPUTDOWN:
        ret=control_state[1]?10000:0;
        break;
        
    case VarId_INPUTLEFT:
        ret=control_state[2]?10000:0;
        break;
        
    case VarId_INPUTRIGHT:
        ret=control_state[3]?10000:0;
        break;
        
    case VarId_INPUTA:
        ret=control_state[4]?10000:0;
        break;
        
    case VarId_INPUTB:
        ret=control_state[5]?10000:0;
        break;
        
    case VarId_INPUTL:
        ret=control_state[7]?10000:0;
        break;
        
    case VarId_INPUTR:
        ret=control_state[8]?10000:0;
        break;
        
    case VarId_INPUTEX1:
        ret=control_state[10]?10000:0;
        break;
        
    case VarId_INPUTEX2:
        ret=control_state[11]?10000:0;
        break;
        
    case VarId_INPUTEX3:
        ret=control_state[12]?10000:0;
        break;
        
    case VarId_INPUTEX4:
        ret=control_state[13]?10000:0;
        break;
        
    case VarId_INPUTAXISUP:
        ret=control_state[14]?10000:0;
        break;
        
    case VarId_INPUTAXISDOWN:
        ret=control_state[15]?10000:0;
        break;
        
    case VarId_INPUTAXISLEFT:
        ret=control_state[16]?10000:0;
        break;
        
    case VarId_INPUTAXISRIGHT:
        ret=control_state[17]?10000:0;
        break;
        
    case VarId_INPUTMOUSEX:
    {
        int leftOffset=(Backend::graphics->virtualScreenW()/2)-(128* virtualScreenScale());
        ret=((Backend::mouse->getVirtualScreenX()-leftOffset)/ virtualScreenScale())*10000;
        break;
    }
    
    case VarId_INPUTMOUSEY:
    {
        int topOffset=(Backend::graphics->virtualScreenH()/2)-((112-playing_field_offset)*virtualScreenScale());
        ret=((Backend::mouse->getVirtualScreenY()-topOffset)/ virtualScreenScale())*10000;
        break;
    }
    
    case VarId_INPUTMOUSEZ:
        ret=(Backend::mouse->getWheelPosition())*10000;
        break;
        
    case VarId_INPUTMOUSEB:
        ret=(
			(Backend::mouse->leftButtonClicked() ? 1 : 0)
			+ (Backend::mouse->rightButtonClicked() ? 2 : 0)
			+ (Backend::mouse->middleButtonClicked() ? 4 : 0)
			)*10000;
        break;
        
    case VarId_INPUTPRESSSTART:
        ret=button_press[6]?10000:0;
        break;
        
    case VarId_INPUTPRESSMAP:
        ret=button_press[9]?10000:0;
        break;
        
    case VarId_INPUTPRESSUP:
        ret=button_press[0]?10000:0;
        break;
        
    case VarId_INPUTPRESSDOWN:
        ret=button_press[1]?10000:0;
        break;
        
    case VarId_INPUTPRESSLEFT:
        ret=button_press[2]?10000:0;
        break;
        
    case VarId_INPUTPRESSRIGHT:
        ret=button_press[3]?10000:0;
        break;
        
    case VarId_INPUTPRESSA:
        ret=button_press[4]?10000:0;
        break;
        
    case VarId_INPUTPRESSB:
        ret=button_press[5]?10000:0;
        break;
        
    case VarId_INPUTPRESSL:
        ret=button_press[7]?10000:0;
        break;
        
    case VarId_INPUTPRESSR:
        ret=button_press[8]?10000:0;
        break;
        
    case VarId_INPUTPRESSEX1:
        ret=button_press[10]?10000:0;
        break;
        
    case VarId_INPUTPRESSEX2:
        ret=button_press[11]?10000:0;
        break;
        
    case VarId_INPUTPRESSEX3:
        ret=button_press[12]?10000:0;
        break;
        
    case VarId_INPUTPRESSEX4:
        ret=button_press[13]?10000:0;
        break;
        
    case VarId_PRESSAXISUP:
        ret=button_press[14]?10000:0;
        break;
        
    case VarId_PRESSAXISDOWN:
        ret=button_press[15]?10000:0;
        break;
        
    case VarId_PRESSAXISLEFT:
        ret=button_press[16]?10000:0;
        break;
        
    case VarId_PRESSAXISRIGHT:
        ret=button_press[17]?10000:0;
        break;
        
    case VarId_BUTTONPRESS:
		// DUkey, DDkey, DLkey, DRkey, Akey, Bkey, Skey, Lkey, Rkey, Pkey, Exkey1, Exkey2, Exkey3, Exkey4 };
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		ret = button_press[button]?10000:0;
		
	}
	break;

	case VarId_BUTTONINPUT:
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		ret=control_state[button]?10000:0;
		
	}
	break;

	case VarId_BUTTONHELD:
	{
		//Read-only
		int button = vbound((ri->d[0]/10000),0,17);
		ret = button_hold[button]?10000:0;
	}
	break;

	case VarId_KEYPRESS:
	{	//Game->KeyPressed[], read-only
		//if ( !keypressed() ) break; //Don;t return values set by setting Link->Input/Press
		//hmm...no, this won;t return properly for modifier keys. 
		int keyid = ri->d[0]/10000;
		//key = vbound(key,0,n);
		bool pressed = key[keyid] != 0;
		ret = pressed?10000:0;
	}
	break;

	case VarId_READKEY:
	{
		//Game->ReadKey(int key), also clears it. 
		int keyid = ri->d[0]/10000;
		bool pressed = ReadKey(keyid);
		ret = pressed?10000:0;
	}
	break;

	case VarId_JOYPADPRESS:
	{
		//Checks if a press is from the joypad, not keyboard. 
		int button = ri->d[0]/10000;
		ret = joybtn(button)?10000:0;
	}
	break;
        
///----------------------------------------------------------------------------------------------------//
//Item Variables
    case VarId_ITEMX:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)((item*)(s))->x)*10000;
        }
        
        break;
        
    case VarId_ITEMY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)((item*)(s))->y)*10000;
        }
        
        break;
        
    case VarId_ITEMZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)((item*)(s))->z)*10000;
        }
        
        break;
        
    case VarId_ITEMJUMP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret = long((-((item*)(s))->fall / fix(100.0))  * 10000);
        }
        
        break;
        
    case VarId_ITEMDRAWTYPE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->drawstyle*10000;
        }
        
        break;
        
    case VarId_ITEMID:
    {
        //TODO script module support
        if (0 != (s = checkItem(ri->itemref)))
        {
            ret = ((item*)(s))->itemDefinition.slot * 10000;
        }
        break;
    }
    case VarId_ITEMTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->tile*10000;
        }
        
        break;
        
    case VarId_ITEMOTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->o_tile*10000;
        }
        
        break;
        
    case VarId_ITEMCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->o_cset&15)*10000;
        }
        
        break;
        
    case VarId_ITEMFLASHCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->o_cset>>4)*10000;
        }
        
        break;
        
    case VarId_ITEMFRAMES:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->frames*10000;
        }
        
        break;
        
    case VarId_ITEMFRAME:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->aframe*10000;
        }
        
        break;
        
    case VarId_ITEMASPEED:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->o_speed*10000;
        }
        
        break;
        
    case VarId_ITEMDELAY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->o_delay*10000;
        }
        
        break;
        
    case VarId_ITEMFLIP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->flip*10000;
        }
        
        break;
        
    case VarId_ITEMFLASH:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->flash*10000;
        }
        
        break;
        
    case VarId_ITEMHXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hxofs)*10000;
        }
        
        break;
        
    case VarId_ITEMHYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hyofs)*10000;
        }
        
        break;
        
    case VarId_ITEMXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)(((item*)(s))->xofs))*10000;
        }
        
        break;
        
    case VarId_ITEMYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)(((item*)(s))->yofs-playing_field_offset))*10000;
        }
        
        break;
        
    case VarId_ITEMZOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((int)(((item*)(s))->zofs))*10000;
        }
        
        break;
        
    case VarId_ITEMHXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hxsz)*10000;
        }
        
        break;
        
    case VarId_ITEMHYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hysz)*10000;
        }
        
        break;
        
    case VarId_ITEMHZSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->hzsz)*10000;
        }
        
        break;
        
    case VarId_ITEMTXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->txsz)*10000;
        }
        
        break;
        
    case VarId_ITEMTYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=(((item*)(s))->tysz)*10000;
        }
        
        break;
        
    case VarId_ITEMCOUNT:
        ret=(items.Count())*10000;
        break;
        
    case VarId_ITEMEXTEND:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->extend*10000;
        }
        
        break;
        
    case VarId_ITEMPICKUP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ret=((item*)(s))->pickup*10000;
        }
        
        break;
        
    case VarId_ITEMMISCD:
        if(0!=(s=checkItem(ri->itemref)))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            ret=(((item*)(s))->miscellaneous[a]);
            break;
        }
        
///----------------------------------------------------------------------------------------------------//
//Itemdata Variables	
	
//2.54
//Get the ID of an item.
    case VarId_IDATAID:
        ret=ri->idata*10000;
        break;

    case VarId_IDATAUSEWPN:
    case VarId_IDATAUSEDEF:
    case VarId_IDATAWRANGE:
    case VarId_IDATAUSEMVT:
    case VarId_IDATADURATION:
    case VarId_IDATADUPLICATES:
    case VarId_IDATADRAWLAYER:
    case VarId_IDATACOLLECTFLAGS:
    case VarId_IDATAWEAPONSCRIPT:
    case VarId_IDATAMISCD:
    case VarId_IDATAWPNINITD:
    case VarId_IDATAWEAPHXOFS:
    case VarId_IDATAWEAPHYOFS:
    case VarId_IDATAWEAPHXSZ:
    case VarId_IDATAWEAPHYSZ:
    case VarId_IDATAWEAPHZSZ:
    case VarId_IDATAWEAPXOFS:
    case VarId_IDATAWEAPYOFS:
    case VarId_IDATAFAMILY:
    case VarId_IDATALEVEL:
    case VarId_IDATAKEEP:
    case VarId_IDATAAMOUNT:
    case VarId_IDATASETMAX:
    case VarId_IDATAMAX:
    case VarId_IDATACOUNTER:
    case VarId_IDATAUSESOUND:
    case VarId_IDATAPOWER:
    case VarId_IDATASCRIPT:
    case VarId_IDATAATTRIB:
    case VarId_IDATASPRITE: 
    case VarId_IDATALTM:
    case VarId_IDATAPSCRIPT:
    case VarId_IDATAMAGCOST:
    case VarId_IDATAMINHEARTS:
    case VarId_IDATATILE:
    case VarId_IDATAMISC:
    case VarId_IDATACSET:
    case VarId_IDATAFRAMES:
    case VarId_IDATAASPEED:
    case VarId_IDATADELAY:
    case VarId_IDATACOMBINE:
    case VarId_IDATADOWNGRADE:
    case VarId_IDATAFLAGS: 
    case VarId_IDATAKEEPOLD:
    case VarId_IDATARUPEECOST:
    case VarId_IDATAEDIBLE:
    case VarId_IDATAFLAGUNUSED:
    case VarId_IDATAGAINLOWER:
    case VarId_IDATAINITDD:
    
        get_itemdata_register(arg);
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

    case VarId_NPCDIR:
        if(GuyH::loadNPC(ri->guyref, "npc->Dir") != SH::_NoError)
            ret = -10000;
        else
            ret = zc_max(GuyH::getNPC()->dir * 10000, 0);
            
        break;
        
    case VarId_NPCRATE:
        GET_NPC_VAR_INT(rate, "npc->Rate") break;
        
    case VarId_NPCHOMING:
        GET_NPC_VAR_INT(homing, "npc->Homing") break;
        
    case VarId_NPCFRAMERATE:
        GET_NPC_VAR_INT(frate, "npc->ASpeed") break;
        
    case VarId_NPCHALTRATE:
        GET_NPC_VAR_INT(hrate, "npc->HaltRate") break;
        
    case VarId_NPCDRAWTYPE:
        GET_NPC_VAR_INT(drawstyle, "npc->DrawStyle") break;
        
    case VarId_NPCHP:
        GET_NPC_VAR_INT(hp, "npc->HP") break;
        
    case VarId_NPCCOLLDET:
        GET_NPC_VAR_INT(scriptcoldet, "npc->ColDetection") break;
        
    case VarId_NPCSTUN:
        GET_NPC_VAR_INT(stunclk, "npc->Stun") break;
        
    case VarId_NPCHUNGER:
        GET_NPC_VAR_INT(grumble, "npc->Hunger") break;
    
    case VarId_NPCWEAPSPRITE:
    {
        //TODO module support for scripts
        if(GuyH::loadNPC(ri->guyref, "npc->WeaponSprite") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPC()->wpnsprite.slot * 10000;
        break;
    }
        
    case VarId_NPCTYPE:
        GET_NPC_VAR_INT(family, "npc->Type") break;
        
    case VarId_NPCDP:
        GET_NPC_VAR_INT(dp, "npc->Damage") break;
        
    case VarId_NPCWDP:
        GET_NPC_VAR_INT(wdp, "npc->WeaponDamage") break;
        
    case VarId_NPCOTILE:
        GET_NPC_VAR_INT(o_tile, "npc->OriginalTile") break;
        
    case VarId_NPCTILE:
        GET_NPC_VAR_INT(tile, "npc->Tile") break;
        
    case VarId_NPCWEAPON:
        GET_NPC_VAR_INT(wpn, "npc->Weapon") break;
        
    case VarId_NPCITEMSET:
        GET_NPC_VAR_INT(item_set, "npc->ItemSet") break;
        
    case VarId_NPCCSET:
        GET_NPC_VAR_INT(cs, "npc->CSet") break;
        
    case VarId_NPCBOSSPAL:
        GET_NPC_VAR_INT(bosspal, "npc->BossPal") break;
        
    case VarId_NPCBGSFX:
        GET_NPC_VAR_INT(bgsfx, "npc->SFX") break;
        
    case VarId_NPCEXTEND:
        GET_NPC_VAR_INT(extend, "npc->Extend") break;
        
    case VarId_NPCHXOFS:
        GET_NPC_VAR_INT(hxofs, "npc->HitXOffset") break;
        
    case VarId_NPCHYOFS:
        GET_NPC_VAR_INT(hyofs, "npc->HitYOffset") break;
        
    case VarId_NPCHXSZ:
        GET_NPC_VAR_INT(hxsz, "npc->HitWidth") break;
        
    case VarId_NPCHYSZ:
        GET_NPC_VAR_INT(hysz, "npc->HitHeight") break;
        
    case VarId_NPCHZSZ:
        GET_NPC_VAR_INT(hzsz, "npc->HitZHeight") break;
        
    case VarId_NPCTXSZ:
        GET_NPC_VAR_INT(txsz, "npc->TileWidth") break;
        
    case VarId_NPCTYSZ:
        GET_NPC_VAR_INT(tysz, "npc->TileHeight") break;
        
//And fix
#define GET_NPC_VAR_FIX(member, str) \
{ \
	if(GuyH::loadNPC(ri->guyref, str) != SH::_NoError) \
		ret = -10000; \
	else \
		ret = (long(GuyH::getNPC()->member) * 10000); \
}

    case VarId_NPCX:
        GET_NPC_VAR_FIX(x, "npc->X") break;
        
    case VarId_NPCY:
        GET_NPC_VAR_FIX(y, "npc->Y") break;
        
    case VarId_NPCZ:
        GET_NPC_VAR_FIX(z, "npc->Z") break;
        
    case VarId_NPCXOFS:
        GET_NPC_VAR_FIX(xofs, "npc->DrawXOffset") break;
        
    case VarId_NPCYOFS:
        GET_NPC_VAR_FIX(yofs, "npc->DrawYOffset") ret-=playing_field_offset*10000;
        break;
        
    case VarId_NPCZOFS:
        GET_NPC_VAR_FIX(zofs, "npc->DrawZOffset") break;
        
        //These variables are all different to the templates (casting for jump and step is slightly non-standard)
    case VarId_NPCJUMP:
        if(GuyH::loadNPC(ri->guyref, "npc->Jump") != SH::_NoError)
            ret = -10000;
        else
            ret = (long(-GuyH::getNPC()->fall / fix(100.0)) * 10000);
            
        break;
        
    case VarId_NPCSTEP:
        if(GuyH::loadNPC(ri->guyref, "npc->Step") != SH::_NoError)
            ret = -10000;
        else
            ret = long(GuyH::getNPC()->step * fix(100.0)) * 10000;
            
        break;
        
    case VarId_NPCID:
        //TODO script support for modules
        if(GuyH::loadNPC(ri->guyref, "npc->ID") != SH::_NoError)
            ret = -10000;
        else
            ret = (GuyH::getNPC()->enemyDefinition.slot) * 10000;
            
        break;
        
    case VarId_NPCMFLAGS:
        if(GuyH::loadNPC(ri->guyref, "npc->MiscFlags") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getMFlags() * 10000;
            
        break;
        
        //Indexed (two checks)
    case VarId_NPCDEFENSED:
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Defense") != SH::_NoError ||
                BC::checkBounds(a, 0, (edefLAST255-1), "npc->Defense") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPC()->defense[a] * 10000;
    }
    break;
    
    //2.fuure compat.
    
    case VarId_NPCSCRDEFENSED:
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->ScriptDefense") != SH::_NoError ||
                BC::checkBounds(a, 0, edefSCRIPTDEFS_MAX, "npc->ScriptDefense") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPC()->defense[a+edefSCRIPT01] * 10000;
    }
    break;
    
    
    case VarId_NPCMISCD:
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Misc") != SH::_NoError ||
                BC::checkMisc32(a, "npc->Misc") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPC()->miscellaneous[a];
    }
    break;
    
    case VarId_NPCDD: //Fized the size of this array. There are 15 total attribs, [0] to [14], not [0] to [9]. -Z
    {
        int a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Attributes") != SH::_NoError ||
                BC::checkBounds(a, 0, 15, "npc->Attributes") != SH::_NoError)
            ret = -10000;
        else
            ret = GuyH::getNPCDMisc(a) * 10000;
    }
    break;
    
        case VarId_NPCINVINC:
        if(GuyH::loadNPC(ri->guyref, "npc->InvFrames") != SH::_NoError)
            ret = -10000;
        else
	ret = (int)GuyH::getNPC()->hclk * 10000;
            
        break;
	
    case VarId_NPCHASITEM:
        if(GuyH::loadNPC(ri->guyref, "npc->HasItem") != SH::_NoError)
            ret = 0;
        else
	ret = GuyH::getNPC()->itemguy?10000:0;
            
        break;
	
    case VarId_NPCRINGLEAD:
        if(GuyH::loadNPC(ri->guyref, "npc->Ringleader") != SH::_NoError)
            ret = 0;
        else
	ret = GuyH::getNPC()->leader?10000:0;
            
        break;
	
	case VarId_NPCSUPERMAN:
        if(GuyH::loadNPC(ri->guyref, "npc->Invincible") != SH::_NoError)
            ret = -10000;
        else
	ret = (int)GuyH::getNPC()->superman * 10000;
            
        break;
    
    
    
///----------------------------------------------------------------------------------------------------//
//LWeapon Variables
    case VarId_LWPNX:
        if(0!=(s=checkLWpn(ri->lwpn,"X")))
            ret=((int)((weapon*)(s))->x)*10000;
            
        break;
        
    case VarId_LWPNY:
        if(0!=(s=checkLWpn(ri->lwpn,"Y")))
            ret=((int)((weapon*)(s))->y)*10000;
            
        break;
        
    case VarId_LWPNZ:
        if(0!=(s=checkLWpn(ri->lwpn,"Z")))
            ret=((int)((weapon*)(s))->z)*10000;
            
        break;
        
    case VarId_LWPNJUMP:
        if(0!=(s=checkLWpn(ri->lwpn,"Jump")))
            ret = long((-((weapon*)(s))->fall / fix(100.0)) * 100000);
            
        break;
        
    case VarId_LWPNDIR:
        if(0!=(s=checkLWpn(ri->lwpn,"Dir")))
            ret=((weapon*)(s))->dir*10000;
            
        break;
        
    case VarId_LWPNSTEP:
        if(0!=(s=checkLWpn(ri->lwpn,"Step")))
            ret=(int)((float)((weapon*)s)->step * 1000000.0);
            
        break;
        
    case VarId_LWPNANGLE:
        if(0!=(s=checkLWpn(ri->lwpn,"Angle")))
            ret=(int)(((weapon*)(s))->angle*10000);
            
        break;
        
    case VarId_LWPNANGULAR:
        if(0!=(s=checkLWpn(ri->lwpn,"Angular")))
            ret=((weapon*)(s))->angular*10000;
            
        break;
        
    case VarId_LWPNBEHIND:
        if(0!=(s=checkLWpn(ri->lwpn,"Behind")))
            ret=((weapon*)(s))->behind*10000;
            
        break;
        
    case VarId_LWPNDRAWTYPE:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawStyle")))
            ret=((weapon*)(s))->drawstyle*10000;
            
        break;
        
    case VarId_LWPNPOWER:
        if(0!=(s=checkLWpn(ri->lwpn,"Damage")))
            ret=((weapon*)(s))->power*10000;
            
        break;
	
    case VarId_LWPNRANGE:
        if(0!=(s=checkLWpn(ri->lwpn,"Range")))
            ret=((weapon*)(s))->scriptrange*10000;
            
        break;
        
    case VarId_LWPNDEAD:
        if(0!=(s=checkLWpn(ri->lwpn,"DeadState")))
            ret=((weapon*)(s))->dead*10000;
            
        break;
        
    case VarId_LWPNID:
        if(0!=(s=checkLWpn(ri->lwpn,"ID")))
            ret=((weapon*)(s))->id*10000;
            
        break;
        
    case VarId_LWPNTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"Tile")))
            ret=((weapon*)(s))->tile*10000;
            
        break;
        
    case VarId_LWPNCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"CSet")))
            ret=((weapon*)(s))->cs*10000;
            
        break;
        
    case VarId_LWPNFLASHCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"FlashCSet")))
            ret=(((weapon*)(s))->o_cset>>4)*10000;
            
        break;
        
    case VarId_LWPNFRAMES:
        if(0!=(s=checkLWpn(ri->lwpn,"NumFrames")))
            ret=((weapon*)(s))->frames*10000;
            
        break;
        
    case VarId_LWPNFRAME:
        if(0!=(s=checkLWpn(ri->lwpn,"Frame")))
            ret=((weapon*)(s))->aframe*10000;
            
        break;
        
    case VarId_LWPNASPEED:
        if(0!=(s=checkLWpn(ri->lwpn,"ASpeed")))
            ret=((weapon*)(s))->o_speed*10000;
            
        break;
        
    case VarId_LWPNFLASH:
        if(0!=(s=checkLWpn(ri->lwpn,"Flash")))
            ret=((weapon*)(s))->flash*10000;
            
        break;
        
    case VarId_LWPNFLIP:
        if(0!=(s=checkLWpn(ri->lwpn,"Flip")))
            ret=((weapon*)(s))->flip*10000;
            
        break;
        
    case VarId_LWPNCOUNT:
        ret=Lwpns.Count()*10000;
        break;
        
    case VarId_LWPNEXTEND:
        if(0!=(s=checkLWpn(ri->lwpn,"Extend")))
            ret=((weapon*)(s))->extend*10000;
            
        break;
        
    case VarId_LWPNOTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalTile")))
            ret=((weapon*)(s))->o_tile*10000;
            
        break;
        
    case VarId_LWPNOCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalCSet")))
            ret=(((weapon*)(s))->o_cset&15)*10000;
            
        break;
        
    case VarId_LWPNHXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitXOffset")))
            ret=(((weapon*)(s))->hxofs)*10000;
            
        break;
        
    case VarId_LWPNHYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitYOffset")))
            ret=(((weapon*)(s))->hyofs)*10000;
            
        break;
        
    case VarId_LWPNXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawXOffset")))
            ret=((int)(((weapon*)(s))->xofs))*10000;
            
        break;
        
    case VarId_LWPNYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawYOffset")))
            ret=((int)(((weapon*)(s))->yofs-playing_field_offset))*10000;
            
        break;
        
    case VarId_LWPNZOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawZOffset")))
            ret=((int)(((weapon*)(s))->zofs))*10000;
            
        break;
        
    case VarId_LWPNHXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitWidth")))
            ret=(((weapon*)(s))->hxsz)*10000;
            
        break;
        
    case VarId_LWPNHYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitHeight")))
            ret=(((weapon*)(s))->hysz)*10000;
            
        break;
        
    case VarId_LWPNHZSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitZHeight")))
            ret=(((weapon*)(s))->hzsz)*10000;
            
        break;
        
    case VarId_LWPNTXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileWidth")))
            ret=(((weapon*)(s))->txsz)*10000;
            
        break;
        
    case VarId_LWPNTYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileHeight")))
            ret=(((weapon*)(s))->tysz)*10000;
            
        break;
        
    case VarId_LWPNMISCD:
        if(0!=(s=checkLWpn(ri->lwpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            ret=(((weapon*)(s))->miscellaneous[a]);
        }
        
        break;
        
    case VarId_LWPNCOLLDET:
        if(0!=(s=checkLWpn(ri->lwpn,"CollDetection")))
            ret=(((weapon*)(s))->scriptcoldet)*10000;
            
        break;
        
///----------------------------------------------------------------------------------------------------//
//EWeapon Variables
    case VarId_EWPNX:
        if(0!=(s=checkEWpn(ri->ewpn, "X")))
            ret=((int)((weapon*)(s))->x)*10000;
            
        break;
        
    case VarId_EWPNY:
        if(0!=(s=checkEWpn(ri->ewpn, "Y")))
            ret=((int)((weapon*)(s))->y)*10000;
            
        break;
        
    case VarId_EWPNZ:
        if(0!=(s=checkEWpn(ri->ewpn, "Z")))
            ret=((int)((weapon*)(s))->z)*10000;
            
        break;
        
    case VarId_EWPNJUMP:
        if(0!=(s=checkEWpn(ri->ewpn, "Jump")))
            ret = long((-((weapon*)(s))->fall / fix(100.0)) * 100000);
            
        break;
        
    case VarId_EWPNDIR:
        if(0!=(s=checkEWpn(ri->ewpn, "Dir")))
            ret=((weapon*)(s))->dir*10000;
            
        break;
        
    case VarId_EWPNSTEP:
        if(0!=(s=checkEWpn(ri->ewpn, "Step")))
            ret=(int)((float)((weapon*)s)->step * 1000000.0);
            
        break;
        
    case VarId_EWPNANGLE:
        if(0!=(s=checkEWpn(ri->ewpn,"Angle")))
            ret=(int)(((weapon*)(s))->angle*10000);
            
        break;
        
    case VarId_EWPNANGULAR:
        if(0!=(s=checkEWpn(ri->ewpn,"Angular")))
            ret=((weapon*)(s))->angular*10000;
            
        break;
        
    case VarId_EWPNBEHIND:
        if(0!=(s=checkEWpn(ri->ewpn,"Behind")))
            ret=((weapon*)(s))->behind*10000;
            
        break;
        
    case VarId_EWPNDRAWTYPE:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawStyle")))
            ret=((weapon*)(s))->drawstyle*10000;
            
        break;
        
    case VarId_EWPNPOWER:
        if(0!=(s=checkEWpn(ri->ewpn,"Damage")))
            ret=((weapon*)(s))->power*10000;
            
        break;
        
    case VarId_EWPNDEAD:
        if(0!=(s=checkEWpn(ri->ewpn,"DeadState")))
            ret=((weapon*)(s))->dead*10000;
            
        break;
        
    case VarId_EWPNID:
        if(0!=(s=checkEWpn(ri->ewpn,"ID")))
            ret=((weapon*)(s))->id*10000;
            
        break;
        
    case VarId_EWPNTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"Tile")))
            ret=((weapon*)(s))->tile*10000;
            
        break;
        
    case VarId_EWPNCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"CSet")))
            ret=((weapon*)(s))->cs*10000;
            
        break;
        
    case VarId_EWPNFLASHCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"FlashCSet")))
            ret=(((weapon*)(s))->o_cset>>4)*10000;
            
        break;
        
    case VarId_EWPNFRAMES:
        if(0!=(s=checkEWpn(ri->ewpn,"NumFrames")))
            ret=((weapon*)(s))->frames*10000;
            
        break;
        
    case VarId_EWPNFRAME:
        if(0!=(s=checkEWpn(ri->ewpn,"Frame")))
            ret=((weapon*)(s))->aframe*10000;
            
        break;
        
    case VarId_EWPNASPEED:
        if(0!=(s=checkEWpn(ri->ewpn,"ASpeed")))
            ret=((weapon*)(s))->o_speed*10000;
            
        break;
        
    case VarId_EWPNFLASH:
        if(0!=(s=checkEWpn(ri->ewpn,"Flash")))
            ret=((weapon*)(s))->flash*10000;
            
        break;
        
    case VarId_EWPNFLIP:
        if(0!=(s=checkEWpn(ri->ewpn,"Flip")))
            ret=((weapon*)(s))->flip*10000;
            
        break;
        
    case VarId_EWPNCOUNT:
        ret=Ewpns.Count()*10000;
        break;
        
    case VarId_EWPNEXTEND:
        if(0!=(s=checkEWpn(ri->ewpn,"Extend")))
            ret=((weapon*)(s))->extend*10000;
            
        break;
        
    case VarId_EWPNOTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalTile")))
            ret=((weapon*)(s))->o_tile*10000;
            
        break;
        
    case VarId_EWPNOCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalCSet")))
            ret=(((weapon*)(s))->o_cset&15)*10000;
            
        break;
        
    case VarId_EWPNHXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitXOffset")))
            ret=(((weapon*)(s))->hxofs)*10000;
            
        break;
        
    case VarId_EWPNHYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitYOffset")))
            ret=(((weapon*)(s))->hyofs)*10000;
            
        break;
        
    case VarId_EWPNXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawXOffset")))
            ret=((int)(((weapon*)(s))->xofs))*10000;
            
        break;
        
    case VarId_EWPNYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawYOffset")))
            ret=((int)(((weapon*)(s))->yofs-playing_field_offset))*10000;
            
        break;
        
    case VarId_EWPNZOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawZOffset")))
            ret=((int)(((weapon*)(s))->zofs))*10000;
            
        break;
        
    case VarId_EWPNHXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitWidth")))
            ret=(((weapon*)(s))->hxsz)*10000;
            
        break;
        
    case VarId_EWPNHYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitHeight")))
            ret=(((weapon*)(s))->hysz)*10000;
            
        break;
        
    case VarId_EWPNHZSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitZHeight")))
            ret=(((weapon*)(s))->hzsz)*10000;
            
        break;
        
    case VarId_EWPNTXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileWidth")))
            ret=(((weapon*)(s))->txsz)*10000;
            
        break;
        
    case VarId_EWPNTYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileHeight")))
            ret=(((weapon*)(s))->tysz)*10000;
            
        break;
        
    case VarId_EWPNMISCD:
        if(0!=(s=checkEWpn(ri->ewpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            ret=(((weapon*)(s))->miscellaneous[a]);
        }
        
        break;
        
    case VarId_EWPNCOLLDET:
        if(0!=(s=checkEWpn(ri->ewpn,"CollDetection")))
            ret=(((weapon*)(s))->scriptcoldet)*10000;
            
        break;
        
///----------------------------------------------------------------------------------------------------//
//Game Info
	
	
    case VarId_GAMESUBSCHEIGHT:
    {
	ret = passive_subscreen_height*10000;    
    }
    break;
    
    case VarId_GAMEPLAYFIELDOFS:
	ret = playing_field_offset*10000;
    break;
    
    case VarId_PASSSUBOFS:
	ret = passive_subscreen_offset * 10000;
    break;

    
    case VarId_ZELDAVERSION:
        ret = ZC_VERSION; //Do *not* multiply by 10,000!
        break;
    case VarId_ZELDABUILD:
	ret = (int)VERSION_BUILD*10000;
	break;
    case VarId_ZELDABETA:
	ret = (int)VERSION_BETA*10000;
	break;
    case VarId_GAMEDEATHS:
        ret=game->get_deaths()*10000;
        break;
        
    case VarId_GAMECHEAT:
        ret=game->get_cheat()*10000;
        break;
        
    case VarId_GAMETIME:
        ret=game->get_time();
        break;// Can't multiply by 10000 or the maximum result is too big
        
    case VarId_GAMETIMEVALID:
        ret=game->get_timevalid()?10000:0;
        break;
        
    case VarId_GAMEHASPLAYED:
        ret=game->get_hasplayed()?10000:0;
        break;
        
    case VarId_GAMESTANDALONE:
        ret=standalone_mode?10000:0;
        break;
        
    case VarId_GAMEGUYCOUNT:
    {
        int mi = (currmap*MAPSCRSNORMAL)+(ri->d[0]/10000);
        ret=game->guys[mi]*10000;
    }
    break;
    
    case VarId_GAMECONTSCR:
        ret=game->get_continue_scrn()*10000;
        break;
        
    case VarId_GAMECONTDMAP:
        ret=game->get_continue_dmap()*10000;
        break;
        
    case VarId_GAMEENTRSCR:
        ret=lastentrance*10000;
        break;
        
    case VarId_GAMEENTRDMAP:
        ret=lastentrance_dmap*10000;
        break;
        
    case VarId_GAMECOUNTERD:
        ret=game->get_counter((ri->d[0])/10000)*10000;
        break;
        
    case VarId_GAMEMCOUNTERD:
        ret=game->get_maxcounter((ri->d[0])/10000)*10000;
        break;
        
    case VarId_GAMEDCOUNTERD:
        ret=game->get_dcounter((ri->d[0])/10000)*10000;
        break;
        
    case VarId_GAMEGENERICD:
        ret=game->get_generic((ri->d[0])/10000)*10000;
        break;
        
    case VarId_GAMEITEMSD:
    {
        //TODO script support for modules
        ItemDefinitionRef ref("CORE", ri->d[0] / 10000);
        ret = (game->get_item(ref) ? 10000 : 0);
        break;
    }
    case VarId_DISABLEDITEM:
    {
        //TODO script support for modules
        ItemDefinitionRef ref("CORE", ri->d[0] / 10000);
        ret = (game->get_disabled_item(ref) ? 10000 : 0);
        break;
    }
        
    case VarId_GAMELITEMSD:
        ret=game->lvlitems[(ri->d[0])/10000]*10000;
        break;
        
    case VarId_GAMELKEYSD:
        ret=game->lvlkeys[(ri->d[0])/10000]*10000;
        break;
        
    case VarId_SCREENSTATED:
    {
        int mi =(currmap*MAPSCRSNORMAL)+currscr;
        ret=((game->maps[mi]>>((ri->d[0]/10000)))&1)?10000:0;
    }
    break;
    
    case VarId_SCREENSTATEDD:
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
    
    case VarId_GAMEGUYCOUNTD:
        ret=game->guys[(currmap * MAPSCRSNORMAL) + (ri->d[0] / 10000)]*10000;
        break;
        
    case VarId_CURMAP:
        ret=(1+currmap)*10000;
        break;
        
    case VarId_CURSCR:
        ret=currscr*10000;
        break;
        
    case VarId_GETMIDI:
        ret=(currmidi-(ZC_MIDI_COUNT-1))*10000;
        break;
        
    case VarId_CURDSCR:
    {
        int di = (get_currscr()-DMaps[get_currdmap()].xoff);
        ret=(DMaps[get_currdmap()].type==dmOVERW ? currscr : di)*10000;
    }
    break;
    
    case VarId_GAMEMAXMAPS:
	ret = (map_count-1)*10000;
	break;
    case VarId_GAMENUMMESSAGES:
	ret = (msg_count-1) * 10000; 
	break;
    
    case VarId_CURDMAP:
        ret=currdmap*10000;
        break;
        
    case VarId_CURLEVEL:
        ret=DMaps[get_currdmap()].level*10000;
        break;
        
    case VarId_GAMECLICKFREEZE:
        ret=disableClickToFreeze?0:10000;
        break;
        
    
    case VarId_NOACTIVESUBSC:
	ret=Link->stopSubscreenFalling()?10000:0;
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

    case VarId_DMAPFLAGSD:
        GET_DMAP_VAR(flags,   "Game->DMapFlags")    break;
        
    case VarId_DMAPLEVELD:
        GET_DMAP_VAR(level,   "Game->DMapLevel")    break;
        
    case VarId_DMAPCOMPASSD:
        GET_DMAP_VAR(compass, "Game->DMapCompass")  break;
        
    case VarId_DMAPCONTINUED:
        GET_DMAP_VAR(cont,    "Game->DMapContinue") break;
    
    case VarId_DMAPLEVELPAL:
	GET_DMAP_VAR(color,   "Game->DMapPalette")    break; 
        
    case VarId_DMAPOFFSET:
        GET_DMAP_VAR(xoff,    "Game->DMapOffset")   break;
        
    case VarId_DMAPMAP:
    {
        int ID = ri->d[0] / 10000;
        
        if(BC::checkDMapID(ID, "Game->DMapMap") != SH::_NoError)
            ret = -10000;
        else
            ret = (DMaps[ID].map+1) * 10000;
            
        break;
    }
    
    case VarId_DMAPMIDID:
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

    case VarId_COMBODD:
        GET_COMBO_VAR(data,  "Screen->ComboD") break;
        
    case VarId_COMBOCD:
        GET_COMBO_VAR(cset,  "Screen->ComboC") break;
        
    case VarId_COMBOFD:
        GET_COMBO_VAR(sflag, "Screen->ComboF") break;
        
#define GET_COMBO_VAR_BUF(member, str) \
{ \
    int pos = ri->d[0] / 10000; \
    if(BC::checkComboPos(pos, str) != SH::_NoError) \
        ret = -10000; \
    else \
        ret = combobuf[tmpscr->data[pos]].member * 10000; \
}
        
    case VarId_COMBOTD:
        GET_COMBO_VAR_BUF(type, "Screen->ComboT") break;
        
    case VarId_COMBOID:
        GET_COMBO_VAR_BUF(flag, "Screen->ComboI") break;
        
    case VarId_COMBOSD:
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
    case VarId_COMBODDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Game->GetComboData: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->GetComboData: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->GetComboData: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		ret = -10000;
	}
        else 
	{
            long scr = m*MAPSCRS+sc;
            int layr = FFScript::whichlayer(scr);
            if(scr==(currmap*MAPSCRS+currscr))
                ret=tmpscr->data[pos]*10000;
            else if(layr>-1)
                ret=tmpscr2[layr].data[pos]*10000;
            else ret=TheMaps[scr].data[pos]*10000;
        }

    }
    break;
    
    case VarId_COMBOCDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Game->GetComboCSet: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->GetComboCSet: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->GetComboCSet: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		ret = -10000;
	}
        
	else
	{
            long scr = m*MAPSCRS+sc;
            int layr = FFScript::whichlayer(scr);
            if(scr==(currmap*MAPSCRS+currscr))
                ret=tmpscr->cset[pos]*10000;
            else if(layr>-1)
                ret=tmpscr2[layr].cset[pos]*10000;
            else ret=TheMaps[scr].cset[pos]*10000;
        }
    }
    break;
    
    case VarId_COMBOFDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Game->GetComboFlag: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->GetComboFlag: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->GetComboFlag: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		ret = -10000;
	}
        
	else
	{
            long scr = m*MAPSCRS+sc;
            int layr = FFScript::whichlayer(scr);
            if(scr==(currmap*MAPSCRS+currscr))
                ret=tmpscr->sflag[pos]*10000;
            else if(layr>-1)
                ret=tmpscr2[layr].sflag[pos]*10000;
            else ret=TheMaps[scr].sflag[pos]*10000;
        }
    }
    break;
    
    case VarId_COMBOTDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Game->GetComboType: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->GetComboType: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->GetComboype: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		ret = -10000;
	}
        
	else
	{
            long scr = m*MAPSCRS+sc;
            int layr = FFScript::whichlayer(scr);
            if(scr==(currmap*MAPSCRS+currscr))
                ret=combobuf[tmpscr->data[pos]].type*10000;
            else if(layr>-1)
                ret=combobuf[tmpscr2[layr].data[pos]].type*10000;
            else ret=combobuf[
                             TheMaps[scr].data[pos]].type*10000;
        }
    }
    break;
    
    case VarId_COMBOIDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Game->GetComboInherentFlag: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->GetComboInherentFlag: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->GetComboInherentFLag: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		ret = -10000;
	}
        
        else
        {
            long scr = m*MAPSCRS+sc;
            int layr = FFScript::whichlayer(scr);
            if(scr==(currmap*MAPSCRS+currscr))
                ret=combobuf[tmpscr->data[pos]].flag*10000;
            else if(layr>-1)
                ret=combobuf[tmpscr2[layr].data[pos]].flag*10000;
            else ret=combobuf[TheMaps[scr].data[pos]].flag*10000;
        }
    }
    break;
    
    case VarId_COMBOSDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);		
        int m = (ri->d[1]/10000)-1;
		if (curscript->version < 2)
			m++;
        
        bool err_input_found; 
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Game->GetComboSolid: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->GetComboSolid: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->GetComboSolid: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		ret = -10000;
	}
        
        else
        {
            long scr = m*MAPSCRS+sc;
            int layr = FFScript::whichlayer(scr);
            if(scr==(currmap*MAPSCRS+currscr))
                ret=(combobuf[tmpscr->data[pos]].walk&15)*10000;
            else if(layr>-1)
                ret=(combobuf[tmpscr2[layr].data[pos]].walk&15)*10000;
            else ret=(combobuf[TheMaps[scr].data[pos]].walk&15)*10000;
        }
    }
    break;
    
    
    
    case VarId_SETSCREENENEMY:
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
    
    case VarId_SETSCREENDOOR:
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
    
    
///----------------------------------------------------------------------------------------------------//
//Screen Information
    case VarId_SDD:
    {
        int di = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
        ret=FFScript::get_screen_d(di, ri->d[0]/10000);
    }
    break;
    
    case VarId_SDDD:
        ret=FFScript::get_screen_d((ri->d[0])/10000 + ((get_currdmap())<<7), ri->d[1] / 10000);
        break;
        
    case VarId_SDDDD:
        ret=FFScript::get_screen_d(ri->d[1] / 10000 + ((ri->d[0]/10000)<<7), ri->d[2] / 10000);
        break;
        
    case VarId_SCRDOORD:
        ret=tmpscr->door[ri->d[0]/10000]*10000;
        break;
    
    
    //These use the same method as GetScreenD -Z
    case VarId_SETSCREENWIDTH:
        ret=FFScript::get_screenWidth(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENHEIGHT:
        ret=FFScript::get_screenHeight(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENVIEWX:
        ret=get_screenViewX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENVIEWY:
        ret=get_screenViewY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENGUY:
        ret=get_screenGuy(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENSTRING:
        ret=get_screenString(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENROOM:
        ret=get_screenRoomtype(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENENTX:
        ret=get_screenEntryX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENENTY:
        ret=get_screenEntryY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENITEM:
        ret=get_screenitem(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENUNDCMB:
        ret=get_screenundercombo(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENUNDCST:
        ret=get_screenundercset(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;

case VarId_SETSCREENCATCH:
        ret=get_screenatchall(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)]);
        break;
 
        
    case VarId_LIT:
        ret= darkroom ? 0 : 10000;
        break;
        
    case VarId_WAVY:
        ret = wavy*10000;
        break;
        
    case VarId_QUAKE:
        ret = quakeclk*10000;
        break;
        
    case VarId_SCREENFLAGSD:
        ret = get_screenflags(tmpscr,vbound(ri->d[0] / 10000,0,9));
        break;
        
    case VarId_SCREENEFLAGSD:
        ret = get_screeneflags(tmpscr,vbound(ri->d[0] / 10000,0,2));
        break;
        
    case VarId_NPCCOUNT:
        ret = guys.Count()*10000;
        break;
        
    case VarId_ROOMDATA:
        ret = tmpscr->catchall*10000;
        break;
        
    case VarId_ROOMTYPE:
        ret = tmpscr->room*10000;
        break;
        
    case VarId_PUSHBLOCKX:
        ret = blockmoving ? int(mblock2->x)*10000 : -10000;
        break;
        
    case VarId_PUSHBLOCKY:
        ret = blockmoving ? int(mblock2->y)*10000 : -10000;
        break;
        
    case VarId_PUSHBLOCKCOMBO:
        ret = mblock2->bcombo*10000;
        break;
        
    case VarId_PUSHBLOCKCSET:
        ret = mblock2->cs*10000;
        break;
        
    case VarId_UNDERCOMBO:
        ret = tmpscr->undercombo*10000;
        break;
        
    case VarId_UNDERCSET:
        ret = tmpscr->undercset*10000;
        break;
    
    //Creates an lweapon using an iemdata struct values to generate its properties.
	//Useful in conjunction with the new weapon editor. 
    case VarId_CREATELWPNDX:
	{
		//Z_message("Trying to get Link->SetExtend().\n");
        long ID = (ri->d[0] / 10000);
        // TODO script support for modules
        ItemDefinitionRef itemid("CORE", ri->d[1]/10000);
        
		//Z_scripterrlog("GetLinkExtend rid->[2] is (%i), trying to use for '%s'\n", ri->d[2], "ri->d[2]");
	    //Z_scripterrlog("GetLinkExtend rid->[1] is (%i), trying to use for '%s'\n", state, "state");
	    //Z_scripterrlog("GetLinkExtend rid->[0] is (%i), trying to use for '%s'\n", dir, "dir");
	
		Lwpns.add(new weapon((fix)0,(fix)0,(fix)0,ID,0,0,0,itemid,Link->getUID()));
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
	case VarId_COLLISIONDX:
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
    case VarId_DEBUGREFFFC:
    {
	int r = -1;
	if ( ri->ffcref ) r = ri->ffcref;
        ret = r * 10000;
    }
        break;
        
    case VarId_DEBUGREFITEM:
    {
	int r = -1;
	if ( ri->itemref ) r = ri->itemref;
        ret = r * 10000;
    }
        break;
        
    case VarId_DEBUGREFITEMDATA:
    {
	int r = -1;
	if ( ri->idata ) r = ri->idata;
        ret = r * 10000;
    }
        break;
        
    case VarId_DEBUGREFLWEAPON:
    {
	int r = -1;
	if ( ri->lwpn ) r = ri->lwpn;
        ret = r * 10000;
    }
        break;
        
    case VarId_DEBUGREFEWEAPON:
    {
	int r = -1;
	if ( ri->ewpn ) r = ri->ewpn;
        ret = r * 10000;
    }
        break;
        
    case VarId_DEBUGREFNPC:
    {
	int r = -1;
	if ( ri->guyref ) r = ri->guyref;
        ret = r * 10000;
    }    
        break;
        
    case VarId_DEBUGSP:
    {
	int r = -1;
	if ( ri->sp ) r = ri->sp;
        ret = r * 10000;
    }   
        break;
    case VarId_DEBUGGDR:
    {
	int a = vbound(ri->d[0]/10000,0,15);
	int r = -1;
	if ( game->global_d[a] ) r = game->global_d[a];
        ret = r * 10000;
    }
        break;
    
///----------------------------------------------------------------------------------------------------//
//Misc./Internal
    case VarId_REFFFC:
        ret = ri->ffcref * 10000;
        break;
        
    case VarId_REFITEM:
        ret = ri->itemref;
        break;
        
    case VarId_REFITEMCLASS:
        ret = ri->idata;
        break;
        
    case VarId_REFLWPN:
        ret = ri->lwpn;
        break;
        
    case VarId_REFEWPN:
        ret = ri->ewpn;
        break;
        
    case VarId_REFNPC:
        ret = ri->guyref;
        break;
        
    case VarId_SP:
        ret = ri->sp * 10000;
        break;
        
    case VarId_SCRIPTRAM:
    case VarId_GLOBALRAM:
        ret = ArrayH::getElement(ri->d[0] / 10000, ri->d[1] / 10000);
        break;
        
    case VarId_SCRIPTRAMD:
    case VarId_GLOBALRAMD:
        ret = ArrayH::getElement(ri->d[0] / 10000, 0);
        break;
        
    case VarId_GDD://Doesn't work like this =(
        ret = game->global_d[ri->d[0] / 10000];
        break;
        
///----------------------------------------------------------------------------------------------------//
//Most of this is deprecated I believe ~Joe123
    default:
    {
        if (arg >= VAR_D(0) && arg <= VAR_D(7))
	        ret = ri->d[arg - VAR_D(0)];
        else if (arg >= VAR_A(0) && arg <= VAR_A(1))
	        ret = ri->a[arg - VAR_A(0)];
        else if (arg >= VAR_GD(0) && arg <= VAR_GD(255))
	        ret = game->global_d[arg - VAR_GD(0)];
        
        break;
    }
    }
    
    return ret;
}



void set_register(const long arg, const long value)
{

    switch (arg)
    {
        ///----------------------------------------------------------------------------------------------------//
        //FFC Variables
    case VarId_DATA:
        tmpscr->ffdata[ri->ffcref] = vbound(value / 10000, 0, MAXCOMBOS - 1);
        break;

        /* These are commands, not registers. -Gray
    case VarId_CHANGEFFSCRIPTR:
        FFScript::do_changeffcscript(false);
        break;

    case VarId_CHANGEFFSCRIPTV:
        FFScript::do_changeffcscript(true);
        break;
        */

    case VarId_FFSCRIPT:
        for (long i = 1; i < MAX_ZCARRAY_SIZE; i++)
        {
            if (arrayOwner[i] == ri->ffcref)
                FFScript::deallocateZScriptArray(i);
        }

        tmpscr->ffscript[ri->ffcref] = vbound(value / 10000, 0, scripts.ffscripts.size() - 1);

        for (int i = 0; i < 16; i++)
            ffmisc[ri->ffcref][i] = 0;

        for (int i = 0; i < 2; i++)
            tmpscr->inita[ri->ffcref][i] = 0;

        for (int i = 0; i < 8; i++)
            tmpscr->initd[ri->ffcref][i] = 0;

        ffcScriptData[ri->ffcref].Clear();
        tmpscr->initialized[ri->ffcref] = false;
        break;

    case VarId_CSET:
        tmpscr->ffcset[ri->ffcref] = (value / 10000) & 15;
        break;

    case VarId_DELAY:
        tmpscr->ffdelay[ri->ffcref] = value / 10000;
        break;

    case VarId_X:
        tmpscr->ffx[ri->ffcref] = value;
        break;

    case VarId_Y:
        tmpscr->ffy[ri->ffcref] = value;
        break;

    case VarId_XD:
        tmpscr->ffxdelta[ri->ffcref] = value;
        break;

    case VarId_YD:
        tmpscr->ffydelta[ri->ffcref] = value;
        break;

    case VarId_FFCID:
        ri->ffcref = vbound((value - 10000) / 10000, 0, 31);
        break;

    case VarId_XD2:
        tmpscr->ffxdelta2[ri->ffcref] = value;
        break;

    case VarId_YD2:
        tmpscr->ffydelta2[ri->ffcref] = value;
        break;

    case VarId_FFFLAGSD:
        value ? tmpscr->ffflags[ri->ffcref] |= 1 << ((ri->d[0]) / 10000)
            : tmpscr->ffflags[ri->ffcref] &= ~(1 << ((ri->d[0]) / 10000));
        break;

    case VarId_FFCWIDTH:
        tmpscr->ffwidth[ri->ffcref] = (tmpscr->ffwidth[ri->ffcref] & ~63) | (((value / 10000) - 1) & 63);
        break;

    case VarId_FFCHEIGHT:
        tmpscr->ffheight[ri->ffcref] = (tmpscr->ffheight[ri->ffcref] & ~63) | (((value / 10000) - 1) & 63);
        break;

    case VarId_FFTWIDTH:
        tmpscr->ffwidth[ri->ffcref] = (tmpscr->ffwidth[ri->ffcref] & 63) | ((((value / 10000) - 1) & 3) << 6);
        break;

    case VarId_FFTHEIGHT:
        tmpscr->ffheight[ri->ffcref] = (tmpscr->ffheight[ri->ffcref] & 63) | ((((value / 10000) - 1) & 3) << 6);
        break;

    case VarId_FFLINK:
        (tmpscr->fflink[ri->ffcref]) = vbound(value / 10000, 0, 32); // Allow "ffc->Link = 0" to unlink ffc.
    //0 is none, setting this before made it impssible to clear it. -Z
        break;

    case VarId_FFMISCD:
    {
        int a = vbound(ri->d[0] / 10000, 0, 15);
        ffmisc[ri->ffcref][a] = value;
        break;
    }

    case VarId_FFINITDD:
        (tmpscr->initd[ri->ffcref][vbound(ri->d[0] / 10000, 0, 7)]) = value;
        break;


        ///----------------------------------------------------------------------------------------------------//
        //Link's Variables
    case VarId_LINKX:
        Link->setX(value / 10000);
        break;

    case VarId_LINKY:
        Link->setY(value / 10000);
        break;

    case VarId_LINKZ:
        Link->setZ(value / 10000);
        break;

    case VarId_LINKJUMP:
        Link->setFall(fix((-value * (100.0)) / 10000.0));
        break;

    case VarId_LINKDIR:
    {
        //Link->setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
        if (Link->getAction() == attacking) Link->dir = (value / 10000);
        else Link->setDir(value / 10000);

        break;
    }

    case VarId_LINKHITDIR:
        Link->setHitDir(value / 10000);
        break;

    case VarId_LINKHP:
        game->set_life(zc_max(0, zc_min(value / 10000, game->get_maxlife())));
        break;

    case VarId_LINKMP:
        game->set_magic(zc_max(0, zc_min(value / 10000, game->get_maxmagic())));
        break;

    case VarId_LINKMAXHP:
        game->set_maxlife(value / 10000);
        break;

    case VarId_LINKMAXMP:
        game->set_maxmagic(value / 10000);
        break;

    case VarId_LINKACTION:
        Link->setAction((actiontype)(value / 10000));
        break;

    case VarId_LINKHELD:
    {
        //TODO script support for modules
        ItemDefinitionRef ref("CORE", value / 10000);
        if (curQuest->isValid(ref))
            Link->setHeldItem(ref);
        break;
    }

    case VarId_LINKITEMD:
    {
        //TODO script support for modules
        ItemDefinitionRef itemref("CORE", ri->d[0] / 10000);
        if (!curQuest->isValid(itemref))
        {
            Z_scripterrlog("Cannot give link item with invalid item id %ld\n", ri->d[0] / 10000);
            break;
        }
        // If the Cane of Byrna is being removed, cancel its effect.
        if (value == 0 && itemref == current_item_id(itype_cbyrna))
            stopCaneOfByrna();

        bool settrue = (value != 0);

        //Sanity check to prevent setting the item if the value would be the same. -Z
        if (game->get_item(itemref) != settrue) game->set_item(itemref, (value != 0));

        //resetItems(game); - Is this really necessary? ~Joe123
        if ((get_bit(quest_rules, qr_OVERWORLDTUNIC) != 0) || (currscr < 128 || dlevel)) ringcolor(false);

        break;
    }

    case VarId_LINKEQUIP:
    {
        //TODO only supports items up to 255, also need scripting support for modules
        int setb = ((value / 10000) & 0xFF00) >> 8, seta = (value / 10000) & 0xFF;

        ItemDefinitionRef refa("CORE", seta);
        ItemDefinitionRef refb("CORE", seta);

        if (seta && get_bit(quest_rules, qr_SELECTAWPN) && game->get_item(refa)) {
            Awpn = refa;
        }
        if (setb && game->get_item(refb)) {
            Bwpn = refb;
        }
        break;
    }

    case VarId_SETITEMSLOT:
    {
        //ri->d[1] = 1st arg
        //ri->d[0] = 2nd arg
        //value = third arg
        //int item, int slot, int force
        int itm = ri->d[0] / 10000;

        int slot = ri->d[1] / 10000;
        int force = ri->d[2] / 10000;

        Z_scripterrlog("SetItemSlot rid->[0] is (%i), trying to use for '%s'\n", itm, "itm");
        Z_scripterrlog("SetItemSlot rid->[1] is (%i), trying to use for '%s'\n", slot, "slot");
        Z_scripterrlog("SetItemSlot rid->[2] is (%i), trying to use for '%s'\n", force, "force");

        //TODO needs module support
        ItemDefinitionRef itmref("CORE", itm);

        if (!curQuest->isValid(itmref))
        {
            Z_scripterrlog("Cannot SetItemSlot with invalid item id %d\n", itm);
        }
        else
        {
            if (force == 0) {
                if (slot == 1) {
                    Awpn = itmref;
                }
                else Bwpn = itmref;
            }
            if (force == 1) {
                if (slot == 1 && game->get_item(itmref)) {
                    Awpn = itmref;
                }
                else {
                    if (game->get_item(itmref)) Bwpn = itmref;
                }
            }

            if (force == 2) {
                if (slot == 1 && get_bit(quest_rules, qr_SELECTAWPN)) {
                    Awpn = itmref;
                }
                else {
                    Bwpn = itmref;
                }
            }

            if (force == 3) { //Flag ITM_REQUIRE_INVENTORY + ITM_REQUIRE_SLOT_A_RULE
                if (slot == 1 && get_bit(quest_rules, qr_SELECTAWPN) && game->get_item(itmref)) {
                    Awpn = itmref;
                }
                else {
                    if (game->get_item(itmref)) Bwpn = itmref;
                }
            }
        }
    }
    break;

    case VarId_LINKINVIS:
        Link->setDontDraw(value / 10000);
        break;

    case VarId_LINKINVINC:
        Link->scriptcoldet = (value / 10000);
        break;

    case VarId_LINKSWORDJINX:
        Link->setSwordClk(value / 10000);
        break;

    case VarId_LINKITEMJINX:
        Link->setItemClk(value / 10000);
        break;

    case VarId_LINKDRUNK:
        Link->setDrunkClock(value / 10000);
        break;

    case VarId_LINKMISCD:
        Link->miscellaneous[vbound(ri->d[0] / 10000, 0, 31)] = value;
        break;

    case VarId_LINKHXOFS:
        (Link->hxofs) = (fix)(value / 10000);
        break;

    case VarId_LINKHYOFS:
        (Link->hyofs) = (fix)(value / 10000);
        break;

    case VarId_LINKXOFS:
        (Link->xofs) = (fix)(value / 10000);
        break;

    case VarId_LINKYOFS:
        (Link->yofs) = (fix)(value / 10000) + playing_field_offset;
        break;

    case VarId_LINKZOFS:
        (Link->zofs) = (fix)(value / 10000);
        break;

    case VarId_LINKHXSZ:
        (Link->hxsz) = (fix)(value / 10000);
        break;

    case VarId_LINKHYSZ:
        (Link->hysz) = (fix)(value / 10000);
        break;

    case VarId_LINKHZSZ:
        (Link->hzsz) = (fix)(value / 10000);
        break;

    case VarId_LINKTXSZ:
        (Link->txsz) = (fix)(value / 10000);
        break;

    case VarId_LINKTYSZ:
        (Link->tysz) = (fix)(value / 10000);
        break;

    case VarId_LINKTILE:
        (Link->tile) = (fix)(value / 10000);
        break;

    case VarId_LINKFLIP:
        (Link->flip) = (fix)(value / 10000);
        break;



    case VarId_LINKINVFRAME:
        Link->setHClk((int)vbound((value / 10000), 0, 214747));
        break;

    case VarId_LINKCANFLICKER:
        Link->setCanLinkFlicker((value / 10000) ? 1 : 0);
        break;

    case VarId_LINKHURTSFX:
        Link->setHurtSFX((int)vbound((value / 10000), 0, 255));
        break;

    case VarId_LINKEATEN:
    {
        Link->setEaten(value / 10000);
        break;
    }
    
    case VarId_GAMESETA:
	{
        int slot = value / 10000;
        Link->setAButtonItem(slot);
        verifyAWpn();
        break;
	}
	
	case VarId_GAMESETB:
	{
        int slot = value / 10000;
        Link->setBButtonItem(slot);
        verifyBWpn();
        break;
    }
	
	//Set Link Diagonal
        case VarId_LINKDIAG:
	    Link->setDiagMove((value/10000)?1:0);
	break;
    
    //Set Link Big Hitbox
        case VarId_LINKBIGHITBOX:
	    Link->setBigHitbox((value/10000)?1:0);
	break;
///----------------------------------------------------------------------------------------------------//
//Input States
    case VarId_INPUTSTART:
        control_state[6]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTMAP:
        control_state[9]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTUP:
        control_state[0]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTDOWN:
        control_state[1]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTLEFT:
        control_state[2]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTRIGHT:
        control_state[3]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTA:
        control_state[4]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTB:
        control_state[5]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTL:
        control_state[7]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTR:
        control_state[8]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTEX1:
        control_state[10]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTEX2:
        control_state[11]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTEX3:
        control_state[12]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTEX4:
        control_state[13]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTAXISUP:
        control_state[14]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTAXISDOWN:
        control_state[15]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTAXISLEFT:
        control_state[16]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTAXISRIGHT:
        control_state[17]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSSTART:
        button_press[6]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSMAP:
        button_press[9]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSUP:
        button_press[0]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSDOWN:
        button_press[1]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSLEFT:
        button_press[2]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSRIGHT:
        button_press[3]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSA:
        button_press[4]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSB:
        button_press[5]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSL:
        button_press[7]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSR:
        button_press[8]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSEX1:
        button_press[10]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSEX2:
        button_press[11]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSEX3:
        button_press[12]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTPRESSEX4:
        button_press[13]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_PRESSAXISUP:
        button_press[14]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_PRESSAXISDOWN:
        button_press[15]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_PRESSAXISLEFT:
        button_press[16]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_PRESSAXISRIGHT:
        button_press[17]=((value/10000)!=0)?true:false;
        break;
        
    case VarId_INPUTMOUSEX:
    {
        int leftOffset=(Backend::graphics->virtualScreenW()/2)-(128*virtualScreenScale());
        position_mouse((value/10000)*virtualScreenScale() +leftOffset, Backend::mouse->getVirtualScreenY());
        break;
    }
    
    case VarId_INPUTMOUSEY:
    {
        int topOffset=(Backend::graphics->virtualScreenH() /2)-((112-playing_field_offset)*virtualScreenScale());
        position_mouse(Backend::mouse->getVirtualScreenX(), (value/10000)*virtualScreenScale() +topOffset);
        break;
    }
    
    case VarId_INPUTMOUSEZ:
		Backend::mouse->setWheelPosition(value/10000);
        break;
        
///----------------------------------------------------------------------------------------------------//
//Item Variables
    case VarId_ITEMX:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (s->x)=(fix)(value/10000);
            
            // Move the Fairy enemy as well.
            if(curQuest->getItemDefinition(((item*)(s))->itemDefinition).family==itype_fairy && curQuest->getItemDefinition(((item*)(s))->itemDefinition).misc3)
                movefairy2(((item*)(s))->x,((item*)(s))->y,((item*)(s))->misc);
        }
        
        break;
        
    case VarId_ITEMY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (s->y)=(fix)(value/10000);
            
            // Move the Fairy enemy as well.
            if(curQuest->getItemDefinition(((item*)(s))->itemDefinition).family==itype_fairy && curQuest->getItemDefinition(((item*)(s))->itemDefinition).misc3)
                movefairy2(((item*)(s))->x,((item*)(s))->y,((item*)(s))->misc);
        }
        
        break;
        
    case VarId_ITEMZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (s->z)=(fix)(value/10000);
            
            if(s->z < 0)
                s->z = 0;
        }
        
        break;
        
    case VarId_ITEMJUMP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->fall)=-value*100.0/10000.0;
        }
        
        break;
        
    case VarId_ITEMDRAWTYPE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->drawstyle)=value/10000;
        }
        
        break;
        
    case VarId_ITEMID:
    {
        //TODO script module support        
        if (0 != (s = checkItem(ri->itemref)))
        {
            ItemDefinitionRef newref(((item *)s)->itemDefinition.module, value / 10000);
            if (!curQuest->isValid(newref))
            {
                Z_scripterrlog("Cannot set item to invalid item ID %ld", value / 10000);
            }
            else
            {
                ((item *)s)->itemDefinition = newref;
                flushItemCache();
            }
        }

        break;
    }

    case VarId_ITEMTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->tile)=vbound(value/10000,0,NEWMAXTILES-1);
        }
        
        break;
        
    case VarId_ITEMOTILE:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_tile)=vbound(value/10000,0,NEWMAXTILES-1);
        }
        
        break;
        
    case VarId_ITEMCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_cset) = (((item *)s)->o_cset & ~15) | ((value/10000)&15);
            (((item *)s)->cs) = (((item *)s)->o_cset & 15);
        }
        
        break;
        
    case VarId_ITEMFLASHCSET:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_cset) = ((value/10000)<<4) | (((item *)s)->o_cset & 15);
        }
        
        break;
        
    case VarId_ITEMFRAMES:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->frames)=value/10000;
        }
        
        break;
        
    case VarId_ITEMFRAME:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->aframe)=value/10000;
        }
        
        break;
        
    case VarId_ITEMASPEED:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_speed)=value/10000;
        }
        
        break;
        
    case VarId_ITEMDELAY:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->o_delay)=value/10000;
        }
        
        break;
        
    case VarId_ITEMFLIP:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->flip)=value/10000;
        }
        
        break;
        
    case VarId_ITEMFLASH:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->flash)= (value/10000)?1:0;
        }
        
        break;
        
    case VarId_ITEMEXTEND:
        if(0!=(s=checkItem(ri->itemref)))
        {
            (((item *)s)->extend)=value/10000;
        }
        
        break;
        
    case VarId_ITEMHXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hxofs=value/10000;
        }
        
        break;
        
    case VarId_ITEMHYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hyofs=value/10000;
        }
        
        break;
        
    case VarId_ITEMXOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->xofs=(fix)(value/10000);
        }
        
        break;
        
    case VarId_ITEMYOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->yofs=(fix)(value/10000)+playing_field_offset;
        }
        
        break;
        
    case VarId_ITEMZOFS:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->zofs=(fix)(value/10000);
        }
        
        break;
        
    case VarId_ITEMHXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hxsz=value/10000;
        }
        
        break;
        
    case VarId_ITEMHYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hysz=value/10000;
        }
        
        break;
        
    case VarId_ITEMHZSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->hzsz=value/10000;
        }
        
        break;
        
    case VarId_ITEMTXSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->txsz=vbound((value/10000),1,20);
        }
        
        break;
        
    case VarId_ITEMTYSZ:
        if(0!=(s=checkItem(ri->itemref)))
        {
            ((item*)(s))->tysz=vbound((value/10000),1,20);
        }
        
        break;
        
    case VarId_ITEMPICKUP:
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
        
    case VarId_ITEMMISCD:
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
    case VarId_IDATAFAMILY:
    case VarId_IDATAUSEWPN:
    case VarId_IDATAUSEDEF:
    case VarId_IDATAWRANGE:
     case VarId_IDATADURATION:
     case VarId_IDATADUPLICATES:
    case VarId_IDATADRAWLAYER:
    case VarId_IDATACOLLECTFLAGS:
    case VarId_IDATAWEAPONSCRIPT:
    case VarId_IDATAMISCD:
    case VarId_IDATAWPNINITD:
    case VarId_IDATAWEAPHXOFS:
    case VarId_IDATAWEAPHYOFS:
    case VarId_IDATAWEAPHXSZ:
    case VarId_IDATAWEAPHYSZ:
    case VarId_IDATAWEAPHZSZ:
    case VarId_IDATAWEAPXOFS:
    case VarId_IDATAWEAPYOFS:
    case VarId_IDATAUSEMVT:
    case VarId_IDATALEVEL:
    case VarId_IDATAKEEP:
    case VarId_IDATAAMOUNT:
    case VarId_IDATASETMAX:
    case VarId_IDATAMAX:
    case VarId_IDATAPOWER:
    case VarId_IDATACOUNTER:
    case VarId_IDATAUSESOUND:
    case VarId_IDATACOMBINE:
	case VarId_IDATADOWNGRADE:
	case VarId_IDATAFLAGS: 
	case VarId_IDATAKEEPOLD:
	case VarId_IDATARUPEECOST:
	case VarId_IDATAEDIBLE:
	case VarId_IDATAFLAGUNUSED:
	case VarId_IDATAGAINLOWER:
	case VarId_IDATASCRIPT:
	case VarId_IDATAATTRIB: 
	case VarId_IDATASPRITE: 
	case VarId_IDATALTM:
    case VarId_IDATAPSCRIPT:
     case VarId_IDATAMAGCOST:
     case VarId_IDATAMINHEARTS:
     case VarId_IDATATILE:
     case VarId_IDATAMISC:
     case VarId_IDATACSET:
     case VarId_IDATAFRAMES:
     case VarId_IDATAASPEED:
     case VarId_IDATADELAY:
    case VarId_IDATAINITDD:
        set_itemdata_register(arg, value);
    break;
    
///----------------------------------------------------------------------------------------------------//
//LWeapon Variables
    case VarId_LWPNX:
        if(0!=(s=checkLWpn(ri->lwpn,"X")))
            ((weapon*)s)->x=(fix)(value/10000);
            
        break;
        
    case VarId_LWPNY:
        if(0!=(s=checkLWpn(ri->lwpn,"Y")))
            ((weapon*)s)->y=(fix)(value/10000);
            
        break;
        
    case VarId_LWPNZ:
        if(0!=(s=checkLWpn(ri->lwpn,"Z")))
            ((weapon*)s)->z=zc_max((fix)(value/10000),(fix)0);
            
        break;
        
    case VarId_LWPNJUMP:
        if(0!=(s=checkLWpn(ri->lwpn,"Jump")))
            ((weapon*)s)->fall=((-value*100.0)/10000.0);
            
        break;
        
    case VarId_LWPNDIR:
        if(0!=(s=checkLWpn(ri->lwpn,"Dir")))
            ((weapon*)s)->dir=(value/10000);
            
        break;
        
    case VarId_LWPNSTEP:
        if(0!=(s=checkLWpn(ri->lwpn,"Step")))
            ((weapon*)s)->step=(value/10000)/100.0;
            
        break;
        
    case VarId_LWPNANGLE:
        if(0!=(s=checkLWpn(ri->lwpn,"Angle")))
            ((weapon*)s)->angle=(double)(value/10000.0);
            
        break;
        
    case VarId_LWPNANGULAR:
        if(0!=(s=checkLWpn(ri->lwpn,"Angular")))
            ((weapon*)s)->angular=(value/10000) != 0;
            
        break;
        
    case VarId_LWPNBEHIND:
        if(0!=(s=checkLWpn(ri->lwpn,"Behind")))
            ((weapon*)s)->behind=(value/10000) != 0;
            
        break;
        
    case VarId_LWPNDRAWTYPE:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawStyle")))
            ((weapon*)s)->drawstyle=(value/10000);
            
        break;
        
    case VarId_LWPNPOWER:
        if(0!=(s=checkLWpn(ri->lwpn,"Damage")))
            ((weapon*)s)->power=(value/10000);
            
        break;
	
    case VarId_LWPNRANGE:
        if(0!=(s=checkLWpn(ri->lwpn,"Range")))
		((weapon*)s)->scriptrange=vbound((value/10000),0,512); //Allow it to move off-screen. -Z           
        break;
        
    case VarId_LWPNDEAD:
        if(0!=(s=checkLWpn(ri->lwpn,"DeadState")))
            ((weapon*)s)->dead=(value/10000);
            
        break;
        
    case VarId_LWPNID:
        if(0!=(s=checkLWpn(ri->lwpn,"ID")))
            ((weapon*)s)->id=(value/10000);
            
        break;
        
    case VarId_LWPNTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"Tile")))
            ((weapon*)s)->tile=(value/10000);
            
        break;
        
    case VarId_LWPNCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"CSet")))
            ((weapon*)s)->cs=(value/10000)&15;
            
        break;
        
    case VarId_LWPNFLASHCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"FlashCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)<<4;
            
        break;
        
    case VarId_LWPNFRAMES:
        if(0!=(s=checkLWpn(ri->lwpn,"NumFrames")))
            ((weapon*)s)->frames=(value/10000);
            
        break;
        
    case VarId_LWPNFRAME:
        if(0!=(s=checkLWpn(ri->lwpn,"Frame")))
            ((weapon*)s)->aframe=(value/10000);
            
        break;
        
    case VarId_LWPNASPEED:
        if(0!=(s=checkLWpn(ri->lwpn,"ASpeed")))
            ((weapon*)s)->o_speed=(value/10000);
            
        break;
        
    case VarId_LWPNFLASH:
        if(0!=(s=checkLWpn(ri->lwpn,"Flash")))
            ((weapon*)s)->flash=(value/10000);
            
        break;
        
    case VarId_LWPNFLIP:
        if(0!=(s=checkLWpn(ri->lwpn,"Flip")))
            ((weapon*)s)->flip=(value/10000);
            
        break;
        
    case VarId_LWPNEXTEND:
        if(0!=(s=checkLWpn(ri->lwpn,"Extend")))
            ((weapon*)s)->extend=(value/10000);
            
        break;
        
    case VarId_LWPNOTILE:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalTile")))
            ((weapon*)s)->o_tile=(value/10000);
            
        break;
        
    case VarId_LWPNOCSET:
        if(0!=(s=checkLWpn(ri->lwpn,"OriginalCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)&15;
            
        break;
        
    case VarId_LWPNHXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitXOffset")))
            (((weapon*)s)->hxofs)=(value/10000);
            
        break;
        
    case VarId_LWPNHYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"HitYOffset")))
            (((weapon*)s)->hyofs)=(value/10000);
            
        break;
        
    case VarId_LWPNXOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawXOffset")))
            (((weapon*)s)->xofs)=(fix)(value/10000);
            
        break;
        
    case VarId_LWPNYOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawYOffset")))
            (((weapon*)s)->yofs)=(fix)(value/10000)+playing_field_offset;
            
        break;
        
    case VarId_LWPNZOFS:
        if(0!=(s=checkLWpn(ri->lwpn,"DrawZOffset")))
            (((weapon*)s)->zofs)=(fix)(value/10000);
            
        break;
        
    case VarId_LWPNHXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitWidth")))
            (((weapon*)s)->hxsz)=(value/10000);
            
        break;
        
    case VarId_LWPNHYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitHeight")))
            (((weapon*)s)->hysz)=(value/10000);
            
        break;
        
    case VarId_LWPNHZSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"HitZHeight")))
            (((weapon*)s)->hzsz)=(value/10000);
            
        break;
        
    case VarId_LWPNTXSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileWidth")))
            (((weapon*)s)->txsz)=vbound((value/10000),1,20);
            
        break;
        
    case VarId_LWPNTYSZ:
        if(0!=(s=checkLWpn(ri->lwpn,"TileHeight")))
            (((weapon*)s)->tysz)=vbound((value/10000),1,20);
            
        break;
        
    case VarId_LWPNMISCD:
        if(0!=(s=checkLWpn(ri->lwpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            (((weapon*)(s))->miscellaneous[a])=value;
        }
        
        break;
        
    case VarId_LWPNCOLLDET:
        if(0!=(s=checkLWpn(ri->lwpn,"CollDetection")))
            (((weapon*)(s))->scriptcoldet)=value/10000;
            
        break;
        
///----------------------------------------------------------------------------------------------------//
//EWeapon Variables
    case VarId_EWPNX:
        if(0!=(s=checkEWpn(ri->ewpn,"X")))
            ((weapon*)s)->x=(fix)(value/10000);
            
        break;
        
    case VarId_EWPNY:
        if(0!=(s=checkEWpn(ri->ewpn,"Y")))
            ((weapon*)s)->y=(fix)(value/10000);
            
        break;
        
    case VarId_EWPNZ:
        if(0!=(s=checkEWpn(ri->ewpn,"Z")))
            ((weapon*)s)->z=zc_max((fix)(value/10000),(fix)0);
            
        break;
        
    case VarId_EWPNJUMP:
        if(0!=(s=checkEWpn(ri->ewpn,"Jump")))
            ((weapon*)s)->fall=(-value*100.0/10000.0);
            
        break;
        
    case VarId_EWPNDIR:
        if(0!=(s=checkEWpn(ri->ewpn,"Dir")))
            ((weapon*)s)->dir=(value/10000);
            
        break;
        
    case VarId_EWPNSTEP:
        if(0!=(s=checkEWpn(ri->ewpn,"Step")))
            ((weapon*)s)->step=(value/10000)/100.0;
            
        break;
        
    case VarId_EWPNANGLE:
        if(0!=(s=checkEWpn(ri->ewpn,"Angle")))
            ((weapon*)s)->angle=(double)(value/10000.0);
            
        break;
        
    case VarId_EWPNANGULAR:
        if(0!=(s=checkEWpn(ri->ewpn,"Angular")))
            ((weapon*)s)->angular=(value/10000) != 0;
            
        break;
        
    case VarId_EWPNBEHIND:
        if(0!=(s=checkEWpn(ri->ewpn,"Behind")))
            ((weapon*)s)->behind=(value/10000) != 0;
            
        break;
        
    case VarId_EWPNDRAWTYPE:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawStyle")))
            ((weapon*)s)->drawstyle=(value/10000);
            
        break;
        
    case VarId_EWPNPOWER:
        if(0!=(s=checkEWpn(ri->ewpn,"Damage")))
            ((weapon*)s)->power=(value/10000);
            
        break;
        
    case VarId_EWPNDEAD:
        if(0!=(s=checkEWpn(ri->ewpn,"DeadState")))
            ((weapon*)s)->dead=(value/10000);
            
        break;
        
    case VarId_EWPNID:
        if(0!=(s=checkEWpn(ri->ewpn,"ID")))
            ((weapon*)s)->id=(value/10000);
            
        break;
        
    case VarId_EWPNTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"Tile")))
            ((weapon*)s)->tile=(value/10000);
            
        break;
        
    case VarId_EWPNCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"CSet")))
            ((weapon*)s)->cs=(value/10000)&15;
            
        break;
        
    case VarId_EWPNFLASHCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"FlashCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)<<4;
            
        break;
        
    case VarId_EWPNFRAMES:
        if(0!=(s=checkEWpn(ri->ewpn,"NumFrames")))
            ((weapon*)s)->frames=(value/10000);
            
        break;
        
    case VarId_EWPNFRAME:
        if(0!=(s=checkEWpn(ri->ewpn,"Frame")))
            ((weapon*)s)->aframe=(value/10000);
            
        break;
        
    case VarId_EWPNASPEED:
        if(0!=(s=checkEWpn(ri->ewpn,"ASpeed")))
            ((weapon*)s)->o_speed=(value/10000);
            
        break;
        
    case VarId_EWPNFLASH:
        if(0!=(s=checkEWpn(ri->ewpn,"Flash")))
            ((weapon*)s)->flash=(value/10000);
            
        break;
        
    case VarId_EWPNFLIP:
        if(0!=(s=checkEWpn(ri->ewpn,"Flip")))
            ((weapon*)s)->flip=(value/10000);
            
        break;
        
    case VarId_EWPNEXTEND:
        if(0!=(s=checkEWpn(ri->ewpn,"Extend")))
            ((weapon*)s)->extend=(value/10000);
            
        break;
        
    case VarId_EWPNOTILE:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalTile")))
            ((weapon*)s)->o_tile=(value/10000);
            
        break;
        
    case VarId_EWPNOCSET:
        if(0!=(s=checkEWpn(ri->ewpn,"OriginalCSet")))
            (((weapon*)s)->o_cset)|=(value/10000)&15;
            
        break;
        
    case VarId_EWPNHXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitXOffset")))
            (((weapon*)s)->hxofs)=(value/10000);
            
        break;
        
    case VarId_EWPNHYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"HitYOffset")))
            (((weapon*)s)->hyofs)=(value/10000);
            
        break;
        
    case VarId_EWPNXOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawXOffset")))
            (((weapon*)s)->xofs)=(fix)(value/10000);
            
        break;
        
    case VarId_EWPNYOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawYOffset")))
            (((weapon*)s)->yofs)=(fix)(value/10000)+playing_field_offset;
            
        break;
        
    case VarId_EWPNZOFS:
        if(0!=(s=checkEWpn(ri->ewpn,"DrawZOffset")))
            (((weapon*)s)->zofs)=(fix)(value/10000);
            
        break;
        
    case VarId_EWPNHXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitWidth")))
            (((weapon*)s)->hxsz)=(value/10000);
            
        break;
        
    case VarId_EWPNHYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitHeight")))
            (((weapon*)s)->hysz)=(value/10000);
            
        break;
        
    case VarId_EWPNHZSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"HitZHeight")))
            (((weapon*)s)->hzsz)=(value/10000);
            
        break;
        
    case VarId_EWPNTXSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileWidth")))
            (((weapon*)s)->txsz)=vbound((value/10000),1,20);
            
        break;
        
    case VarId_EWPNTYSZ:
        if(0!=(s=checkEWpn(ri->ewpn,"TileHeight")))
            (((weapon*)s)->tysz)=vbound((value/10000),1,20);
            
        break;
        
    case VarId_EWPNMISCD:
        if(0!=(s=checkEWpn(ri->ewpn,"Misc")))
        {
            int a = vbound(ri->d[0]/10000,0,31);
            (((weapon*)(s))->miscellaneous[a])=value;
        }
        
        break;
        
    case VarId_EWPNCOLLDET:
        if(0!=(s=checkEWpn(ri->ewpn,"CollDetection")))
            (((weapon*)(s))->scriptcoldet)=value/10000;
            
        break;
        
///----------------------------------------------------------------------------------------------------//
//NPC Variables

//Fixs are all a bit different
    case VarId_NPCX:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->X") == SH::_NoError)
        {
            GuyH::getNPC()->x = fix(value / 10000);
            
            if(GuyH::hasLink())
                Link->setX(fix(value / 10000));
        }
    }
    break;
    
    case VarId_NPCY:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Y") == SH::_NoError)
        {
            fix oldy = GuyH::getNPC()->y;
            GuyH::getNPC()->y = fix(value / 10000);
            GuyH::getNPC()->floor_y += ((value / 10000) - oldy);
            
            if(GuyH::hasLink())
                Link->setY(fix(value / 10000));
        }
    }
    break;
    
    case VarId_NPCZ:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Z") == SH::_NoError)
        {
            if(!never_in_air(GuyH::getNPC()->enemyDefinition))
            {
                if(value < 0)
                    GuyH::getNPC()->z = fix(0);
                else
                    GuyH::getNPC()->z = fix(value / 10000);
                    
                if(GuyH::hasLink())
                    Link->setZ(fix(value / 10000));
            }
        }
    }
    break;
    
    case VarId_NPCJUMP:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Jump") == SH::_NoError)
        {
            if(canfall(GuyH::getNPC()->enemyDefinition))
                GuyH::getNPC()->fall = -fix(value * 100.0 / 10000.0);
                
            if(GuyH::hasLink())
                Link->setFall(value / fix(10000.0));
        }
    }
    break;
    
    case VarId_NPCSTEP:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Step") == SH::_NoError)
            GuyH::getNPC()->step = fix(value / 10000) / fix(100.0);
    }
    break;
    
    case VarId_NPCXOFS:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->DrawXOffset") == SH::_NoError)
            GuyH::getNPC()->xofs = fix(value / 10000);
    }
    break;
    
    case VarId_NPCYOFS:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->DrawYOffset") == SH::_NoError)
            GuyH::getNPC()->yofs = fix(value / 10000) + playing_field_offset;
    }
    break;
    
    case VarId_NPCZOFS:
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
    
    case VarId_NPCDIR:
        SET_NPC_VAR_INT(dir, "npc->Dir") break;
        
    case VarId_NPCRATE:
        SET_NPC_VAR_INT(rate, "npc->Rate") break;
        
    case VarId_NPCHOMING:
        SET_NPC_VAR_INT(homing, "npc->Homing") break;
        
    case VarId_NPCFRAMERATE:
        SET_NPC_VAR_INT(frate, "npc->ASpeed") break;
        
    case VarId_NPCHALTRATE:
        SET_NPC_VAR_INT(hrate, "npc->HaltRate") break;
        
    case VarId_NPCDRAWTYPE:
        SET_NPC_VAR_INT(drawstyle, "npc->DrawStyle") break;
        
    case VarId_NPCHP:
        SET_NPC_VAR_INT(hp, "npc->HP") break;
        
        //case VarId_NPCID:        SET_NPC_VAR_INT(id, "npc->ID") break; ~Disallowed
    case VarId_NPCDP:
        SET_NPC_VAR_INT(dp, "npc->Damage") break;
        
    case VarId_NPCWDP:
        SET_NPC_VAR_INT(wdp, "npc->WeaponDamage") break;
        
    case VarId_NPCITEMSET:
        SET_NPC_VAR_INT(item_set, "npc->ItemSet") break;
        
    case VarId_NPCBOSSPAL:
        SET_NPC_VAR_INT(bosspal, "npc->BossPal") break;
        
    case VarId_NPCBGSFX:
        if(GuyH::loadNPC(ri->guyref, "npc->SFX") == SH::_NoError)
        {
            enemy *en=GuyH::getNPC();
            int newSFX = value / 10000;
            
            // Stop the old sound and start the new one
            if(en->bgsfx != newSFX)
            {
                en->stop_bgsfx(GuyH::getNPCIndex(ri->guyref));
                Backend::sfx->loop(newSFX,128);
                en->bgsfx = newSFX;
            }
        }
        
        break;
        
    case VarId_NPCEXTEND:
        SET_NPC_VAR_INT(extend, "npc->Extend") break;
        
    case VarId_NPCHXOFS:
        SET_NPC_VAR_INT(hxofs, "npc->HitXOffset") break;
        
    case VarId_NPCHYOFS:
        SET_NPC_VAR_INT(hyofs, "npc->HitYOffset") break;
        
    case VarId_NPCHXSZ:
        SET_NPC_VAR_INT(hxsz, "npc->HitWidth") break;
        
    case VarId_NPCHYSZ:
        SET_NPC_VAR_INT(hysz, "npc->HitHeight") break;
        
    case VarId_NPCHZSZ:
        SET_NPC_VAR_INT(hzsz, "npc->HitZHeight") break;
        
    case VarId_NPCCOLLDET:
        SET_NPC_VAR_INT(scriptcoldet, "npc->CollDetection") break;
        
    case VarId_NPCSTUN:
        SET_NPC_VAR_INT(stunclk, "npc->Stun") break;
        
    case VarId_NPCHUNGER:
        SET_NPC_VAR_INT(grumble, "npc->Hunger") break;
    
    case VarId_NPCWEAPSPRITE:
    {
        //TODO module support for scripts
        SpriteDefinitionRef ref("CORE", value / 10000);
        if(GuyH::loadNPC(ri->guyref, "npc->WeaponSprite") == SH::_NoError) \
            GuyH::getNPC()->wpnsprite = ref;
        break;
    }
        
    case VarId_NPCCSET:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->CSet") == SH::_NoError)
            GuyH::getNPC()->cs = (value / 10000) & 0xF;
    }
    break;
    
//Bounds on value
    case VarId_NPCTXSZ:
    {
        long height = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->TileWidth") == SH::_NoError &&
                BC::checkBounds(height, 0, 20, "npc->TileWidth") == SH::_NoError)
            GuyH::getNPC()->txsz = height;
    }
    break;
    
    case VarId_NPCTYSZ:
    {
        long width = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->TileHeight") == SH::_NoError &&
                BC::checkBounds(width, 0, 20, "npc->TileHeight") == SH::_NoError)
            GuyH::getNPC()->tysz = width;
    }
    break;
    
    case VarId_NPCOTILE:
    {
        long tile = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->OriginalTile") == SH::_NoError &&
                BC::checkTile(tile, "npc->OriginalTile") == SH::_NoError)
            GuyH::getNPC()->o_tile = tile;
    }
    break;
    
    case VarId_NPCTILE:
    {
        long tile = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Tile") == SH::_NoError &&
                BC::checkTile(tile, "npc->Tile") == SH::_NoError)
            GuyH::getNPC()->tile = tile;
    }
    break;
    
    case VarId_NPCWEAPON:
    {
        long weapon = value / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Weapon") == SH::_NoError &&
                BC::checkBounds(weapon, 0, wMax-1, "npc->Weapon") == SH::_NoError)
            GuyH::getNPC()->wpn = weapon;
    }
    break;
    
//Indexed
    case VarId_NPCDEFENSED:
    {
        long a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Defense") == SH::_NoError &&
                BC::checkBounds(a, 0, (edefLAST255-1), "npc->Defense") == SH::_NoError)
            GuyH::getNPC()->defense[a] = value / 10000;
    }
    break;
    
    //2.future compat. -Z
    
    
     case VarId_NPCSCRDEFENSED:
    {
        long a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->ScriptDefense") == SH::_NoError &&
                BC::checkBounds(a, 0, edefSCRIPTDEFS_MAX, "npc->ScriptDefense") == SH::_NoError)
            GuyH::getNPC()->defense[a+edefSCRIPT01] = value / 10000;
    }
    break;
    
    case VarId_NPCMISCD:
    {
        long a = ri->d[0] / 10000;
        
        if(GuyH::loadNPC(ri->guyref, "npc->Misc") == SH::_NoError &&
                BC::checkMisc32(a, "npc->Misc") == SH::_NoError)
            GuyH::getNPC()->miscellaneous[a] = value;
            
    }
    
    break;
    
    //npc->Attributes[] setter -Z
    case VarId_NPCDD:
    {
        long a = ri->d[0] / 10000;

        if (GuyH::loadNPC(ri->guyref, "npc->Attributes") == SH::_NoError &&
            BC::checkBounds(a, 0, 15, "npc->Attributes") == SH::_NoError)
            GuyH::getNPC()->dmiscs[a] = value / 10000;
        break;
    }
       
    
        
    case VarId_NPCINVINC:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->InvFrames") == SH::_NoError)
            GuyH::getNPC()->hclk = (int)value/10000;
    }
    break;
    
    case VarId_NPCSUPERMAN:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Invincible") == SH::_NoError)
            GuyH::getNPC()->superman = (int)value/10000;
    }
    break;
    
    case VarId_NPCHASITEM:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->HasItem") == SH::_NoError)
            GuyH::getNPC()->itemguy = (value/10000)?1:0;
    }
    break;
    
    case VarId_NPCRINGLEAD:
    {
        if(GuyH::loadNPC(ri->guyref, "npc->Ringleader") == SH::_NoError)
            GuyH::getNPC()->leader = (value/10000)?1:0;
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//Game Information
    
    case VarId_GAMESUBSCHEIGHT:
    {
	int v = vbound(value,0,256);
	passive_subscreen_height = (v/10000);   
    }
    break;
    
    case VarId_GAMEPLAYFIELDOFS:
    {
	int v = vbound(value,-256, 256);
	playing_field_offset = (v/10000);
    }
    break;
    
    case VarId_PASSSUBOFS:
    {
	int v = vbound(value,-256, 256);
	passive_subscreen_offset = (v/10000);
    }
    break;

    
    case VarId_GAMEDEATHS:
        game->set_deaths(value/10000);
        break;
        
    case VarId_GAMECHEAT:
        game->set_cheat(value/10000);
        cheat=(value/10000);
        break;
        
    case VarId_GAMETIME:
        game->set_time(value);
        break; // Can't multiply by 10000 or the maximum result is too big
        
    case VarId_GAMETIMEVALID:
        game->set_timevalid((value/10000)?1:0);
        break;
        
    case VarId_GAMEHASPLAYED:
        game->set_hasplayed((value/10000)?1:0);
        break;
        
    case VarId_GAMEGUYCOUNT:
    {
        int mi2 = (currmap*MAPSCRSNORMAL)+(ri->d[0]/10000);
        game->guys[mi2]=value/10000;
    }
    break;
    
    case VarId_GAMECONTSCR:
        game->set_continue_scrn(value/10000);
        break;
        
    case VarId_GAMECONTDMAP:
        game->set_continue_dmap(value/10000);
        break;
        
    case VarId_GAMEENTRSCR:
        lastentrance=value/10000;
        break;
        
    case VarId_GAMEENTRDMAP:
        lastentrance_dmap=value/10000;
        break;
        
    case VarId_GAMECOUNTERD:
        game->set_counter(value/10000, (ri->d[0])/10000);
        break;
        
    case VarId_GAMEMCOUNTERD:
        game->set_maxcounter(value/10000, (ri->d[0])/10000);
        break;
        
    case VarId_GAMEDCOUNTERD:
        game->set_dcounter(value/10000, (ri->d[0])/10000);
        break;
        
    case VarId_GAMEGENERICD:
        game->set_generic(value/10000, (ri->d[0])/10000);
        break;
        
    case VarId_GAMEITEMSD:
    {
        //TODO module support for scripts
        ItemDefinitionRef ref("CORE", ri->d[0] / 10000);
        game->set_item(ref, (value != 0));
        break;
    }
    case VarId_DISABLEDITEM:
    {
        //TODO module support for scripts
        ItemDefinitionRef ref("CORE", ri->d[0] / 10000);
        game->set_disabled_item(ref, value / 10000);
        break;
    }
        
    // LItems is a size of byte. -Z
    case VarId_GAMELITEMSD:
        game->lvlitems[(ri->d[0])/10000]=vbound(value/10000, 0, 255);
        break;
        
    // LKeys is a size of byte. -Z
    case VarId_GAMELKEYSD:
	game->lvlkeys[((ri->d[0])/10000)]=vbound(value/10000, 0, 255);
        break;
        
    case VarId_SCREENSTATED:
    {
        int mi2 = (currmap*MAPSCRSNORMAL)+currscr;
        (value)?setmapflag(mi2, 1<<((ri->d[0])/10000)) : unsetmapflag(mi2, 1 << ((ri->d[0]) / 10000));
    }
    break;
    
    case VarId_SCREENSTATEDD:
    {
        int mi2 = ri->d[0]/10000;
        mi2 -= 8*(mi2/MAPSCRS);
        
        if(BC::checkMapID(mi2>>7, "Game->SetScreenState") == SH::_NoError)
            (value)?setmapflag(mi2, 1<<(ri->d[1]/10000)) : unsetmapflag(mi2, 1 << (ri->d[1] / 10000), true);
    }
    break;
    
    case VarId_GAMEGUYCOUNTD:
        game->guys[(currmap*MAPSCRSNORMAL)+(ri->d[0]/10000)] = value / 10000;
        break;
        
    case VarId_GAMECLICKFREEZE:
        disableClickToFreeze=value==0;
        break;
    
    
    case VarId_NOACTIVESUBSC:
	Link->stopSubscreenFalling((value/10000)?1:0);
	break;
        
///----------------------------------------------------------------------------------------------------//
//DMap Information

#define SET_DMAP_VAR(member, str) \
{ \
    int ID = ri->d[0] / 10000; \
    if(BC::checkDMapID(ID, str) == SH::_NoError) \
        DMaps[ID].member = value / 10000; \
}

    case VarId_DMAPFLAGSD:
        SET_DMAP_VAR(flags, "Game->DMapFlags") break;
        
    case VarId_DMAPLEVELD:
        SET_DMAP_VAR(level, "Game->DMapLevel") break;
        
    case VarId_DMAPCOMPASSD:
        SET_DMAP_VAR(compass, "Game->DMapCompass") break;
        
    case VarId_DMAPCONTINUED:
        SET_DMAP_VAR(cont, "Game->DMapContinue") break;
        
     case VarId_DMAPLEVELPAL:
   {
	int ID = ri->d[0] / 10000; 
	int pal = value/10000;
	pal = vbound(pal, 0, 0x1FF);
	   
	if(BC::checkDMapID(ID, "Game->DMapPalette") == SH::_NoError) 
        DMaps[ID].color = pal;

	loadlvlpal(DMaps[(ri->d[0] / 10000)].color);
	break;
   }
   
    case VarId_DMAPMIDID:
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
    case VarId_COMBODD:
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
    
    case VarId_COMBOCD:
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
    
    case VarId_COMBOFD:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
            tmpscr->sflag[pos]=(value/10000);
    }
    break;
    
    case VarId_COMBOTD:
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
    
    case VarId_COMBOID:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
            combobuf[tmpscr->data[pos]].flag=value/10000;
    }
    break;
    
    case VarId_COMBOSD:
    {
        int pos = (ri->d[0])/10000;
        
        if(pos >= 0 && pos < 176)
            combobuf[tmpscr->data[pos]].walk=(value/10000)&15;
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//Game->SetComboX
    case VarId_COMBODDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Screen->SetComboData: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->SetComboData: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->SetComboData: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		break;
	}
        
        long scr = m*MAPSCRS+sc;
        if(scr==(currmap*MAPSCRS+currscr))
            screen_combo_modify_preroutine(tmpscr,pos);
            
        TheMaps[scr].data[pos]=value/10000;
        
        if(scr==(currmap*MAPSCRS+currscr))
        {
            tmpscr->data[pos] = value/10000;
            screen_combo_modify_postroutine(tmpscr,pos);
        }
        
        int layr = FFScript::whichlayer(scr);
        
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
    
    case VarId_COMBOCDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Screen->SetComboCSet: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->SetComboSet: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->SetComboSet: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		break;
	}
        
        long scr = m*MAPSCRS+sc;
        TheMaps[scr].cset[pos]=(value/10000)&15;
        
        if(scr==(currmap*MAPSCRS+currscr))
            tmpscr->cset[pos] = value/10000;
            
        int layr = FFScript::whichlayer(scr);
        
        if(layr>-1)
            tmpscr2[layr].cset[pos]=(value/10000)&15;
    }
    break;
    
    case VarId_COMBOFDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Screen->SetComboFlag: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->SetComboFlag: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->SetComboFlag: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		break;
	}
        
        long scr = m*MAPSCRS+sc;
        TheMaps[scr].sflag[pos]=value/10000;
        
        if(scr==(currmap*MAPSCRS+currscr))
            tmpscr->sflag[pos] = value/10000;
            
        int layr = FFScript::whichlayer(scr);
        
        if(layr>-1)
            tmpscr2[layr].sflag[pos]=value/10000;
    }
    break;
    
    case VarId_COMBOTDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Screen->SetComboInherentFlag: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->SetComboInherentFlag: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->SetComboInherentFlag: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		break;
	}
        
        long scr = m*MAPSCRS+sc;
        int cdata = TheMaps[scr].data[pos];
        
        // Preprocess the screen's combos in case VarId_the combo changed is present on the screen. -L
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
    
    case VarId_COMBOIDM:
    {
        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
        int m = (ri->d[1]/10000)-1;
        bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Screen->SetComboInherentFlag: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->SetComboInherentFlag: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->SetComboInherentFlag: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		break;
	}
        
        long scr = m*MAPSCRS+sc;
        combobuf[TheMaps[scr].data[pos]].flag=value/10000;
    }
    break;
    
    case VarId_COMBOSDM:
    {

        int pos = (ri->d[0])/10000;
        int sc = (ri->d[2]/10000);
		int m = (ri->d[1] / 10000) - 1;
		if (curscript->version < 2)
			m++;     

	bool err_input_found;
	    
	if ( m < 0 || m >= map_count ) {
		al_trace("Invalid Map Reference Passed to Screen->SetComboolid: \n %d", m);
		err_input_found = true;
	}
	
	if ( sc < 0 || sc >= MAPSCRS ) 
	{
		al_trace("Invalid Screen Reference Passed to Screen->SetComboolid: \n %d", sc);
		err_input_found = true;
	}
	
	if ( pos < 0 || pos > 175 ) 
	{
		al_trace("Invalid Position Reference Passed to Screen->SetComboolid: \n %d", pos);
		err_input_found = true;
	}
	
	if ( err_input_found ) { 
		break;
	}
        
        long scr = m*MAPSCRS+sc;
        combobuf[TheMaps[scr].data[pos]].walk=(value/10000)&15;
    }
    break;
    
///----------------------------------------------------------------------------------------------------//
//Screen Variables
    
    //These use the same method as SetScreenD
    case VarId_SETSCREENWIDTH:
	FFScript::set_screenWidth(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENHEIGHT:
	FFScript::set_screenHeight(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENVIEWX:
	FFScript::set_screenViewX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENVIEWY:
	FFScript::set_screenViewY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENGUY:
	FFScript::set_screenGuy(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENSTRING:
    {
	FFScript::set_screenString(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	    //should this be either
	    //set_screenString(&TheMaps[((ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)])-1), value/10000);
	    //or
	    //set_screenString(&TheMaps[((ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)])-+1), value/10000);
	    Z_message("Map ref is: %d\n",((ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)));
    }
	break;

    case VarId_SETSCREENROOM:
	FFScript::set_screenRoomtype(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENENTX:
	FFScript::set_screenEntryX(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENENTY:
	FFScript::set_screenEntryY(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENITEM:
	FFScript::set_screenitem(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENUNDCMB:
	FFScript::set_screenundercombo(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENUNDCST:
	FFScript::set_screenundercset(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

    case VarId_SETSCREENCATCH:
	FFScript::set_screenatchall(&TheMaps[(ri->d[1] / 10000) * MAPSCRS + (ri->d[0]/10000)], value/10000);
	break;

//These use the method of SetScreenEnemy


//SetScreenLayerOpacity(int map, int scr, int layer, int v)
case VarId_SETSCREENLAYOP:
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

case VarId_SETSCREENSECCMB:
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

case VarId_SETSCREENSECCST:
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

case VarId_SETSCREENSECFLG:
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

case VarId_SETSCREENLAYMAP:
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

case VarId_SETSCREENLAYSCR:
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

case VarId_SETSCREENPATH:
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

case VarId_SETSCREENWARPRX:
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

case VarId_SETSCREENWARPRY:
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

    case VarId_SDD:
    {
        {
            int di2 = ((get_currdmap())<<7) + get_currscr()-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
            FFScript::set_screen_d(di2, ri->d[0]/10000, value);
            break;
        }
    }
    
    case VarId_GDD:
        al_trace("GDD");
        game->global_d[ri->d[0]/10000]=value;
        break;
        
    case VarId_SDDD:
        FFScript::set_screen_d((ri->d[0])/10000 + ((get_currdmap())<<7), ri->d[1]/10000, value);
        break;
        
    case VarId_SDDDD:
        FFScript::set_screen_d(ri->d[1]/10000 + ((ri->d[0]/10000)<<7), ri->d[2]/10000, value);
        break;
        
    case VarId_SCRDOORD:
        tmpscr->door[ri->d[0]/10000]=value/10000;
        putdoor(scrollbuf,0,ri->d[0]/10000,value/10000,true,true);
        break;
        
    case VarId_LIT:
        naturaldark = !value;
        lighting(false, false);
        break;
        
    case VarId_WAVY:
        wavy=value/10000;
        break;
        
    case VarId_QUAKE:
        quakeclk=value/10000;
        break;
        
        //case VarId_ROOMTYPE:
        //  tmpscr->room=value/10000; break; //this probably doesn't work too well...
    case VarId_ROOMDATA:
        tmpscr->catchall=value/10000;
        break;
        
    case VarId_PUSHBLOCKCOMBO:
        mblock2->bcombo=value/10000;
        break;
        
    case VarId_PUSHBLOCKCSET:
        mblock2->cs=value/10000;
        mblock2->oldcset=value/10000;
        break;
        
    case VarId_UNDERCOMBO:
        tmpscr->undercombo=value/10000;
        break;
        
    case VarId_UNDERCSET:
        tmpscr->undercset=value/10000;
        break;
    
    
    case VarId_DEBUGGDR:
    {
	int a = vbound(ri->d[0]/10000,0,15);
        game->global_d[a] = value / 10000;;
    }
        break;
    
    case VarId_DEBUGSP:
        ri->sp = vbound((value / 10000),0,256);
        break;
        
    case VarId_DEBUGREFFFC:
        ri->ffcref = vbound((value / 10000),1,31);
        break;
        
    case VarId_DEBUGREFITEM:
        ri->itemref = vbound((value / 10000),0,255);
        break;
        
    case VarId_DEBUGREFITEMDATA:
        ri->idata = vbound((value / 10000),0,255);
        break;
        
    case VarId_DEBUGREFLWEAPON:
        ri->lwpn = vbound((value / 10000),0,255);
        break;
        
    case VarId_DEBUGREFEWEAPON:
        ri->ewpn = vbound((value / 10000),0,255);
        break;
        
    case VarId_DEBUGREFNPC:
        ri->guyref = vbound((value / 10000),0,255);
        break;
        
///----------------------------------------------------------------------------------------------------//
//Misc./Internal
    case VarId_SP:
        ri->sp = value / 10000;
        break;
        
    case VarId_SCRIPTRAM:
    case VarId_GLOBALRAM:
        ArrayH::setElement(ri->d[0] / 10000, ri->d[1] / 10000, value);
        break;
        
    case VarId_SCRIPTRAMD:
    case VarId_GLOBALRAMD:
        ArrayH::setElement(ri->d[0] / 10000, 0, value);
        break;
        
    case VarId_REFFFC:
        ri->ffcref = value / 10000;
        break;
        
    case VarId_REFITEM:
        ri->itemref = value;
        break;
        
    case VarId_REFITEMCLASS:
        ri->idata = value;
        break;
        
    case VarId_REFLWPN:
        ri->lwpn = value;
        break;
        
    case VarId_REFEWPN:
        ri->ewpn = value;
        break;
        
    case VarId_REFNPC:
        ri->guyref = value;
        break;
        
    default:
    {
        if (arg >= VAR_D(0) && arg <= VAR_D(7))
	        ri->d[arg - VAR_D(0)] = value;
        else if (arg >= VAR_A(0) && arg <= VAR_A(1))
	        ri->a[arg - VAR_A(0)] = value;
        else if (arg >= VAR_GD(0) && arg <= VAR_GD(255))
	        game->global_d[arg - VAR_GD(0)] = value;
        
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
    if (sarg1 == VarId_FFSCRIPT && ri->ffcref == whichFFC)
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
    
    if(reg >= VAR_D(0) || reg <= VAR_D(7))
        set_register(sarg1, ffcScriptData[ffcref].d[reg - VAR_D(0)]); //get back the info into *sarg1
    else if(reg == VAR_A(0) || reg == VAR_A(1))
        set_register(sarg1, ffcScriptData[ffcref].a[reg - VAR_A(0)]);
    else if(reg == VarId_SP)
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
    
    if(reg >= VAR_D(0) || reg <= VAR_D(7))
        ffcScriptData[ffcref].d[reg - VAR_D(0)] = get_register(sarg1); //Set it to *sarg1
    else if(reg == VAR_A(0) || reg == VAR_A(1))
        ffcScriptData[ffcref].a[reg - VAR_A(0)] = get_register(sarg1);
    else if(reg == VarId_SP)
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
    Link->ffwarp = true;
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
    Link->ffwarp = true;
    Link->ffpit = true;
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
    //TODO script module support
    int f = m->guy.slot;
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
    //TODO module support for scripts
    int f = m->screenItem.slot;
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
    //TODO module support for scripts

    EnemyDefinitionRef f = m->enemy[index];
    return f.slot*10000;
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
    
    for(int j = 0; j < items.Count(); j++)
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
    
    for(int j = 0; j < guys.Count(); j++)
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
    
    for(int j = 0; j < Lwpns.Count(); j++)
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
    
    for(int j = 0; j < Ewpns.Count(); j++)
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
        
    if (LwpnH::loadWeapon(ri->lwpn, "lweapon->UseSprite") == SH::_NoError)
    {
        //TODO module support for scripts
        SpriteDefinitionRef ref("CORE", ID);
        LwpnH::getWeapon()->LOADGFX(ref);
    }
}

void do_ewpnusesprite(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkWeaponMiscSprite(ID, "eweapon->UseSprite") != SH::_NoError)
        return;
        
    if (EwpnH::loadWeapon(ri->ewpn, "eweapon->UseSprite") == SH::_NoError)
    {
        //TODO module support for scripts
        SpriteDefinitionRef ref("CORE", ID);
        EwpnH::getWeapon()->LOADGFX(ref);
    }
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
        Link->reset_hookshot();
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

void do_createlweapon(const bool v)
{
    const long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkWeaponID(ID, "Screen->CreateLWeapon") != SH::_NoError)
        return;
        
    addLwpn(0, 0, 0, ID, 0, 0, 0, Link->getUID());
    
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

    //TODO module support for scripts
    ItemDefinitionRef ref("CORE", ri->idata);
    if (!curQuest->isValid(ref))
    {
        Z_scripterrlog("Cannot create invalid item %ld\n", ri->idata);
    }
    else
    {
        additem(0, (get_bit(quest_rules, qr_NOITEMOFFSET) ? 1 : 0), ref, ipBIGRANGE);
    }
    
    if(items.Count() < 1)
    {
        ri->itemref = LONG_MAX;
        Z_scripterrlog("Couldn't create item \"%ld\", screen item limit reached\n", ri->idata);
    }
    else
    {
        ri->itemref = items.spr(items.Count() - 1)->getUID();
        Z_eventlog("Script created item \"%ld\" with UID = %ld\n", ri->idata, ri->itemref);
    }
}

void do_createnpc(const bool v)
{
    const long ID = SH::get_arg(sarg1, v) / 10000;
    
    if(BC::checkGuyID(ID, "Screen->CreateNPC") != SH::_NoError)
        return;

    //TODO module support for scripts

    EnemyDefinitionRef ref("CORE", ID);

    if (!curQuest->isValid(ref))
    {
        Z_scripterrlog("Couldn't create NPC with invalid ID %ld\n", ID);
        return;
    }
        
    //If we make a segmented enemy there'll be more than one sprite created
    word numcreated = addenemy(0, 0, ref, -10);
    
    if(numcreated == 0)
    {
        ri->guyref = LONG_MAX;
        Z_scripterrlog("Couldn't create NPC \"%s\", screen NPC limit reached\n", curQuest->getEnemyDefinition(ref).name.c_str());
    }
    else
    {
        word index = guys.Count() - numcreated; //Get the main enemy, not a segment
        ri->guyref = guys.spr(index)->getUID();
        
        for(; index<guys.Count(); index++)
            ((enemy*)guys.spr(index))->script_spawned=true;
            
        Z_eventlog("Script created NPC \"%s\" with UID = %ld\n", curQuest->getEnemyDefinition(ref).name.c_str(), ri->guyref);
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
        Link->finishedmsg();
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
    case CmdId_RECTR:
        set_drawing_command_args(j, 12);
        break;
        
    case CmdId_CIRCLER:
        set_drawing_command_args(j, 11);
        break;
        
    case CmdId_ARCR:
        set_drawing_command_args(j, 14);
        break;
        
    case CmdId_ELLIPSER:
        set_drawing_command_args(j, 12);
        break;
        
    case CmdId_LINER:
        set_drawing_command_args(j, 11);
        break;
        
    case CmdId_PUTPIXELR:
        set_drawing_command_args(j, 8);
        break;
        
    case CmdId_DRAWTILER:
        set_drawing_command_args(j, 15);
        break;
        
    case CmdId_DRAWCOMBOR:
        set_drawing_command_args(j, 16);
        break;
        
    case CmdId_FASTTILER:
        set_drawing_command_args(j, 6);
        break;
        
    case CmdId_FASTCOMBOR:
        set_drawing_command_args(j, 6);
        break;
        
    case CmdId_DRAWCHARR:
        set_drawing_command_args(j, 10);
        break;
        
    case CmdId_DRAWINTR:
        set_drawing_command_args(j, 11);
        break;
        
    case CmdId_SPLINER:
        set_drawing_command_args(j, 11);
        break;
        
    case CmdId_QUADR:
        set_drawing_command_args(j, 15);
        break;
        
    case CmdId_TRIANGLER:
        set_drawing_command_args(j, 13);
        break;
        
    case CmdId_BITMAPR:
        set_drawing_command_args(j, 12);
        break;
    
    case CmdId_BITMAPEXR:
        set_drawing_command_args(j, 16);
        break;
        
    case CmdId_DRAWLAYERR:
        set_drawing_command_args(j, 8);
        break;
        
    case CmdId_DRAWSCREENR:
        set_drawing_command_args(j, 6);
        break;
        
    case CmdId_QUAD3DR:
    {
        long* v = script_drawing_commands[j].AllocateDrawBuffer(26 * sizeof(long));
        
        long* pos = v + 0;
        long* uv = v + 12;
        long* col = v + 20;
        long* size = v + 24;
        
        set_drawing_command_args(j, 8);
        ArrayH::getValues(script_drawing_commands[j][2] / 10000, pos, 12);
        ArrayH::getValues(script_drawing_commands[j][3] / 10000, uv, 8);
        ArrayH::getValues(script_drawing_commands[j][4] / 10000, col, 4);
        ArrayH::getValues(script_drawing_commands[j][5] / 10000, size, 2);
        
       // script_drawing_commands[j].SetPtr(v);
    }
    break;
    
    case CmdId_TRIANGLE3DR:
    {
        long *v = script_drawing_commands[j].AllocateDrawBuffer(20 * sizeof(long));
        
        long* pos = v + 0;
        long* uv = v + 9;
        long* col = v + 15;
        long* size = v + 18;
        
        set_drawing_command_args(j, 8);
        ArrayH::getValues(script_drawing_commands[j][2] / 10000, pos, 8);
        ArrayH::getValues(script_drawing_commands[j][3] / 10000, uv, 6);
        ArrayH::getValues(script_drawing_commands[j][4] / 10000, col, 3);
        ArrayH::getValues(script_drawing_commands[j][5] / 10000, size, 2);
        
        //script_drawing_commands[j].SetVector(v);
    }
    break;
    
    case CmdId_DRAWSTRINGR:
    {
        set_drawing_command_args(j, 9);

		long arrayID = script_drawing_commands[j][8] / 10000;
		int length = ArrayH::strlen(arrayID);
		if(length > 0)
		{
			char *str = (char*)script_drawing_commands[j].AllocateDrawBuffer(length + 1);
			ArrayH::uncheckedGetCString(arrayID, str, length);
		}
		else
		{
			script_drawing_commands.AbortDrawingCommand();
		}
    }
    break;


	//case CmdId_BITMAPEXR:
	//{
	//}
	//break;

	case CmdId_POLYGONR:
	{
		set_drawing_command_args(j, 6);
		int count = script_drawing_commands[j][2] / 10000;
		int user_array_sz = ArrayH::getSize(script_drawing_commands[j][3] / 10000);
		
		if(count < 6)
		{
			al_trace("The value passed to 'int vertices' for Polygon() is invalid!\n Vertices must be at least 6!\n The value passed was: %d\n", count);
			script_drawing_commands.AbortDrawingCommand();
			break;
		}
		
		if ( (count*2) != user_array_sz )
		{
			al_trace("WARNING! The array passed to Polygon() has a mismatched number of points for the number of vertices supplied!\n");
		}
		
		count = zc_min(count*2, user_array_sz); //Sanity check: Double the count, or use the array size, whichever is smaller. 
		
		

		long* p = (long*)script_drawing_commands[j].AllocateDrawBuffer(count * sizeof(long));
		ArrayH::getValues(script_drawing_commands[j][3] / 10000, p, count);
		
	}
	break;

	//case CmdId_PIXELARRAYR:
	//{
	//	set_drawing_command_args(j, 5);
	//	int count = script_drawing_commands[j][2] / 10000; //todo: errcheck

	//	long* ptr = (long*)zc_malloc(3 * count * sizeof(long));
	//	long* p = ptr;

	//	ArrayH::getValues(script_drawing_commands[j][3] / 10000, p, count); p += count;
	//	ArrayH::getValues(script_drawing_commands[j][4] / 10000, p, count); p += count;
	//	ArrayH::getValues(script_drawing_commands[j][5] / 10000, p, count);

	//	script_drawing_commands[j].SetPtr(ptr);
	//}
	//break;

	//case CmdId_TILEARRAYR:
	//{
	//	set_drawing_command_args(j, 6);
	//	int count = script_drawing_commands[j][2] / 10000; //todo: errcheck

	//	long* ptr = (long*)zc_malloc(3 * count * sizeof(long));
	//	long* p = ptr;

	//	ArrayH::getValues(script_drawing_commands[j][3] / 10000, p, count); p += count;
	//	ArrayH::getValues(script_drawing_commands[j][4] / 10000, p, count); p += count;
	//	ArrayH::getValues(script_drawing_commands[j][5] / 10000, p, count);

	//	script_drawing_commands[j].SetPtr(ptr);
	//}
	//break;

	//case CmdId_COMBOARRAYR:
	//{
	//	set_drawing_command_args(j, 6);
	//	int count = script_drawing_commands[j][2] / 10000; //todo: errcheck

	//	long* ptr = (long*)zc_malloc(3 * count * sizeof(long));
	//	long* p = ptr;

	//	ArrayH::getValues(script_drawing_commands[j][3] / 10000, p, count); p += count;
	//	ArrayH::getValues(script_drawing_commands[j][4] / 10000, p, count); p += count;
	//	ArrayH::getValues(script_drawing_commands[j][5] / 10000, p, count);

	//	script_drawing_commands[j].SetPtr(ptr);
	//}
	//break;

	} //switch
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
        
    Backend::sfx->play(ID,128);
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
    Backend::sfx->stop(sfx);
}

void pause_sfx(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    int sfx = (int)ID;
    if(BC::checkSFXID(ID, "Game->PauseSound") != SH::_NoError)
        return;
    Backend::sfx->pause(sfx);
}

void resume_sfx(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    int sfx = (int)ID;
    if(BC::checkSFXID(ID, "Game->ResumeSound") != SH::_NoError)
        return;
    Backend::sfx->resume(sfx);
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
    //TODO module support for scripts
    ItemDefinitionRef ref("CORE", ri->idata);
    if (!curQuest->isValid(ref))
    {
        Z_scripterrlog("Cannot get name of invalid item ref %ld\n", ri->idata);
    }
    else
    {
        if (ArrayH::setArray(arrayptr, curQuest->getItemDefinition(ref).name) == SH::_Overflow)
            Z_scripterrlog("Array supplied to 'itemdata->GetName' not large enough\n");
    }
}

void do_getnpcname()
{
    long arrayptr = get_register(sarg1) / 10000;
    
    if(GuyH::loadNPC(ri->guyref, "npc->GetName") != SH::_NoError)
        return;
        
    EnemyDefinitionRef ref = (GuyH::getNPC()->enemyDefinition);
    if (!curQuest->isValid(ref))
    {
        Z_scripterrlog("Invalid enemy ID: %ld\n", ri->guyref);
    }
    else if(ArrayH::setArray(arrayptr, curQuest->getEnemyDefinition(ref).name) == SH::_Overflow)
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
        
        curscript = &scripts.ffscripts[script];
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
        
        curscript = &scripts.itemscripts[script];
        stack = &item_stack;
        memset(stack, 0, 256 * sizeof(long)); //zero here too

        //TODO module support for scripts
        ItemDefinitionRef ref("CORE", i);
        
        memcpy(ri->d, curQuest->getItemDefinition(ref).initiald, 8 * sizeof(long));
        memcpy(ri->a, curQuest->getItemDefinition(ref).initiala, 2 * sizeof(long));
        
        ri->idata = i; //'this' pointer
        
    }
    break;
    
    case SCRIPT_GLOBAL:
    {
        ri = &globalScriptData;
        
        curscript = &scripts.globalscripts[script];
        stack = &global_stack;
    }
    break;
    
    default:
        al_trace("No other scripts are currently supported\n");
        return 1;
        break;
    }
    
    dword pc = ri->pc; //this is (marginally) quicker than dereferencing ri each time
    word scommand = curscript->commands[pc].command;
    sarg1 = curscript->commands[pc].arg1;
    sarg2 = curscript->commands[pc].arg2;
    
    
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
    
    while (scommand != 0xFFFF && scommand != CmdId_WAITFRAME
           && scommand != CmdId_WAITDRAW)
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
        // Doc Label: RUN_SCRIPT_GIANT_SWITCH
        switch (scommand)
        {
        case CmdId_QUIT:
	        scommand = 0xFFFF;
	        break;
            
        case CmdId_GOTO:
	        pc = sarg1;
	        increment = false;
	        break;
            
        case CmdId_GOTOR:
        {
	        pc = (get_register(sarg1) / 10000) - 1;
	        increment = false;
        }
        break;
        
        case CmdId_GOTOTRUE:
	        if(ri->scriptflag & TRUEFLAG)
	        {
		        pc = sarg1;
		        increment = false;
	        }
            
	        break;
            
        case CmdId_GOTOFALSE:
	        if(!(ri->scriptflag & TRUEFLAG))
	        {
		        pc = sarg1;
		        increment = false;
	        }
            
	        break;
            
        case CmdId_GOTOMORE:
	        if(ri->scriptflag & MOREFLAG)
	        {
		        pc = sarg1;
		        increment = false;
	        }
            
	        break;
            
        case CmdId_GOTOLESS:
	        if(!(ri->scriptflag & MOREFLAG) || (!get_bit(quest_rules,qr_GOTOLESSNOTEQUAL) && (ri->scriptflag & TRUEFLAG)))
	        {
		        pc = sarg1;
		        increment = false;
	        }
            
	        break;
            
        case CmdId_LOOP:
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
        
        case CmdId_SETTRUE:
	        set_register(sarg1, (ri->scriptflag & TRUEFLAG) ? 1 : 0);
	        break;
            
        case CmdId_SETFALSE:
	        set_register(sarg1, (ri->scriptflag & TRUEFLAG) ? 0 : 1);
	        break;
            
        case CmdId_SETMORE:
	        set_register(sarg1, (ri->scriptflag & MOREFLAG) ? 1 : 0);
	        break;
            
        case CmdId_SETLESS:
	        set_register(sarg1, (!(ri->scriptflag & MOREFLAG)
	                             || (ri->scriptflag & TRUEFLAG)) ? 1 : 0);
	        break;
            
        case CmdId_NOT:
	        do_not(false);
	        break;
            
        case CmdId_COMPAREV:
	        do_comp(true);
	        break;
            
        case CmdId_COMPARER:
	        do_comp(false);
	        break;
            
        case CmdId_SETV:
	        do_set(true, type==SCRIPT_FFC ? i : -1);
	        break;
            
        case CmdId_SETR:
	        do_set(false, type==SCRIPT_FFC ? i : -1);
	        break;
            
        case CmdId_PUSHR:
	        do_push(false);
	        break;
            
        case CmdId_PUSHV:
	        do_push(true);
	        break;
            
        case CmdId_POP:
	        do_pop();
	        break;
            
        case CmdId_LOADI:
	        do_loadi();
	        break;
            
        case CmdId_STOREI:
	        do_storei();
	        break;
            
        case CmdId_LOAD1:
	        do_loada(0);
	        break;
            
        case CmdId_LOAD2:
	        do_loada(1);
	        break;
            
        case CmdId_SETA1:
	        do_seta(0);
	        break;
            
        case CmdId_SETA2:
	        do_seta(1);
	        break;
            
        case CmdId_ALLOCATEGMEMR:
	        if(type == SCRIPT_GLOBAL) do_allocatemem(false, false, type==SCRIPT_FFC?i:255);
            
	        break;
            
        case CmdId_ALLOCATEGMEMV:
	        if(type == SCRIPT_GLOBAL) do_allocatemem(true, false, type==SCRIPT_FFC?i:255);
            
	        break;
            
        case CmdId_ALLOCATEMEMR:
	        do_allocatemem(false, true, type==SCRIPT_FFC?i:255);
	        break;
            
        case CmdId_ALLOCATEMEMV:
	        do_allocatemem(true, true, type==SCRIPT_FFC?i:255);
	        break;
            
        case CmdId_DEALLOCATEMEMR:
	        do_deallocatemem();
	        break;
            
        case CmdId_ARRAYSIZE:
	        do_arraysize();
	        break;
        case CmdId_ARRAYSIZEB:
	        do_arraysize();
	        break;
        case CmdId_ARRAYSIZEF:
	        do_arraysize();
	        break;
        case CmdId_ARRAYSIZEN:
	        do_arraysize();
	        break;
        case CmdId_ARRAYSIZEI:
	        do_arraysize();
	        break;
        case CmdId_ARRAYSIZEID:
	        do_arraysize();
	        break;
        case CmdId_ARRAYSIZEL:
	        do_arraysize();
	        break;
        case CmdId_ARRAYSIZEE:
	        do_arraysize();
	        break;
	
        case CmdId_GETFFCSCRIPT:
	        do_getffcscript();
	        break;
        case CmdId_GETITEMSCRIPT:
	        do_getitemscript();
	        break;
            
            
        case CmdId_ADDV:
	        do_add(true);
	        break;
            
        case CmdId_ADDR:
	        do_add(false);
	        break;
            
        case CmdId_SUBV:
	        do_sub(true);
	        break;
            
        case CmdId_SUBR:
	        do_sub(false);
	        break;
            
        case CmdId_MULTV:
	        do_mult(true);
	        break;
            
        case CmdId_MULTR:
	        do_mult(false);
	        break;
            
        case CmdId_DIVV:
	        do_div(true);
	        break;
            
        case CmdId_DIVR:
	        do_div(false);
	        break;
            
        case CmdId_MODV:
	        do_mod(true);
	        break;
            
        case CmdId_MODR:
	        do_mod(false);
	        break;
            
        case CmdId_SINV:
	        do_trig(true, 0);
	        break;
            
        case CmdId_SINR:
	        do_trig(false, 0);
	        break;
            
        case CmdId_COSV:
	        do_trig(true, 1);
	        break;
            
        case CmdId_COSR:
	        do_trig(false, 1);
	        break;
            
        case CmdId_TANV:
	        do_trig(true, 2);
	        break;
            
        case CmdId_TANR:
	        do_trig(false, 2);
	        break;
            
        case CmdId_ARCSINR:
	        do_asin(false);
	        break;
            
        case CmdId_ARCCOSR:
	        do_acos(false);
	        break;
            
        case CmdId_ARCTANR:
	        do_arctan();
	        break;
            
        case CmdId_ABS:
	        do_abs(false);
	        break;
            
        case CmdId_MINR:
	        do_min(false);
	        break;
            
        case CmdId_MINV:
	        do_min(true);
	        break;
            
        case CmdId_MAXR:
	        do_max(false);
	        break;
            
        case CmdId_MAXV:
	        do_max(true);
	        break;
            
        case CmdId_RNDR:
	        do_rnd(false);
	        break;
            
        case CmdId_RNDV:
	        do_rnd(true);
	        break;
            
        case CmdId_FACTORIAL:
	        do_factorial(false);
	        break;
            
        case CmdId_SQROOTV:
	        do_sqroot(true);
	        break;
            
        case CmdId_SQROOTR:
	        do_sqroot(false);
	        break;
            
        case CmdId_POWERR:
	        do_power(false);
	        break;
            
        case CmdId_POWERV:
	        do_power(true);
	        break;
            
        case CmdId_IPOWERR:
	        do_ipower(false);
	        break;
            
        case CmdId_IPOWERV:
	        do_ipower(true);
	        break;
            
        case CmdId_LOG10:
	        do_log10(false);
	        break;
            
        case CmdId_LOGE:
	        do_naturallog(false);
	        break;
            
        case CmdId_ANDR:
	        do_and(false);
	        break;
            
        case CmdId_ANDV:
	        do_and(true);
	        break;
            
        case CmdId_ORR:
	        do_or(false);
	        break;
            
        case CmdId_ORV:
	        do_or(true);
	        break;
            
        case CmdId_XORR:
	        do_xor(false);
	        break;
            
        case CmdId_XORV:
	        do_xor(true);
	        break;
            
        case CmdId_NANDR:
	        do_nand(false);
	        break;
            
        case CmdId_NANDV:
	        do_nand(true);
	        break;
            
        case CmdId_NORR:
	        do_nor(false);
	        break;
            
        case CmdId_NORV:
	        do_nor(true);
	        break;
            
        case CmdId_XNORR:
	        do_xnor(false);
	        break;
            
        case CmdId_XNORV:
	        do_xnor(true);
	        break;
            
        case CmdId_BITNOT:
	        do_bitwisenot(false);
	        break;
            
        case CmdId_LSHIFTR:
	        do_lshift(false);
	        break;
            
        case CmdId_LSHIFTV:
	        do_lshift(true);
	        break;
            
        case CmdId_RSHIFTR:
	        do_rshift(false);
	        break;
            
        case CmdId_RSHIFTV:
	        do_rshift(true);
	        break;
            
        case CmdId_TRACER:
	        do_trace(false);
	        break;
            
        case CmdId_TRACEV:
	        do_trace(true);
	        break;
            
        case CmdId_TRACE2R:
	        do_tracebool(false);
	        break;
            
        case CmdId_TRACE2V:
	        do_tracebool(true);
	        break;
            
        case CmdId_TRACE3:
	        do_tracenl();
	        break;
            
        case CmdId_TRACE4:
	        do_cleartrace();
	        break;
            
        case CmdId_TRACE5:
	        do_tracetobase();
	        break;
            
        case CmdId_TRACE6:
	        do_tracestring();
	        break;
            
        case CmdId_WARP:
	        do_warp(true);
	        break;
            
        case CmdId_WARPR:
	        do_warp(false);
	        break;
            
        case CmdId_PITWARP:
	        do_pitwarp(true);
	        break;
            
        case CmdId_PITWARPR:
	        do_pitwarp(false);
	        break;
            
        case CmdId_BREAKSHIELD:
	        do_breakshield();
	        break;
            
        case CmdId_SELECTAWPNV:
	        do_selectweapon(true, true);
	        break;
            
        case CmdId_SELECTAWPNR:
	        do_selectweapon(false, true);
	        break;
            
        case CmdId_SELECTBWPNV:
	        do_selectweapon(true, false);
	        break;
            
        case CmdId_SELECTBWPNR:
	        do_selectweapon(false, false);
	        break;
            
        case CmdId_PLAYSOUNDR:
	        do_sfx(false);
	        break;
            
        case CmdId_PLAYSOUNDV:
	        do_sfx(true);
	        break;
	
        case CmdId_TRIGGERSECRETR:
	        FFScript::do_triggersecret(false);
	        break;
            
        case CmdId_TRIGGERSECRETV:
	        FFScript::do_triggersecret(true);
	        break;
            
        case CmdId_PLAYMIDIR:
	        do_midi(false);
	        break;
            
        case CmdId_PLAYMIDIV:
	        do_midi(true);
	        break;
            
        case CmdId_PLAYENHMUSIC:
	        do_enh_music(false);
	        break;
            
        case CmdId_GETMUSICFILE:
	        do_get_enh_music_filename(false);
	        break;
            
        case CmdId_GETMUSICTRACK:
	        do_get_enh_music_track(false);
	        break;
            
        case CmdId_SETDMAPENHMUSIC:
	        do_set_dmap_enh_music(false);
	        break;
	
	        // Audio->
	
        case CmdId_ENDSOUNDR:
	        stop_sfx(false);
	        break;
            
        case CmdId_ENDSOUNDV:
	        stop_sfx(true);
	        break;
	
        case CmdId_PAUSESOUNDR:
	        pause_sfx(false);
	        break;
            
        case CmdId_PAUSESOUNDV:
	        pause_sfx(true);
	        break;
	
        case CmdId_RESUMESOUNDR:
	        resume_sfx(false);
	        break;
            
        case CmdId_RESUMESOUNDV:
	        resume_sfx(true);
	        break;
	
	
	
        case CmdId_PAUSESFX:
        {
	        int sound = ri->d[0]/10000;
	        Backend::sfx->pause(sound);
		
        }
        break;

        case CmdId_RESUMESFX:
        {
	        int sound = ri->d[0]/10000;
	        Backend::sfx->resume(sound);
        }
        break;

        case CmdId_ADJUSTSFX:
        {
	        int sound = ri->d[2]/10000;
	        int pan = ri->d[1];
	        // control_state[6]=((value/10000)!=0)?true:false;
	        bool loop = ((ri->d[0]/10000)!=0)?true:false;
	        //SFXBackend.adjust_sfx(sound,pan,loop);
		
	        //! adjust_sfx was not ported to the new back end!!! -Z
        }
        break;


        case CmdId_CONTINUESFX:
        {
	        int sound = ri->d[0]/10000;
	        //Backend::sfx->cont_sfx(sound);
		
	        //! cont_sfx was not ported to the new back end!!!
	        // I believe this restarted the loop. 
		
	        Backend::sfx->resume(sound);
        }
        break;	

	
        case CmdId_PAUSEMUSIC:
	        Backend::sfx->pauseAll();
	        break;
        case CmdId_RESUMEMUSIC:
	        Backend::sfx->resumeAll();
	        break;
            
            
        case CmdId_MSGSTRR:
	        do_message(false);
	        break;
            
        case CmdId_MSGSTRV:
	        do_message(true);
	        break;
            
        case CmdId_ITEMNAME:
	        do_getitemname();
	        break;
            
        case CmdId_NPCNAME:
	        do_getnpcname();
	        break;
            
        case CmdId_GETSAVENAME:
	        do_getsavename();
	        break;
            
        case CmdId_SETSAVENAME:
	        do_setsavename();
	        break;
            
        case CmdId_GETMESSAGE:
	        do_getmessage(false);
	        break;
        case CmdId_SETMESSAGE:
	        do_setmessage(false);
	        break;
            
        case CmdId_GETDMAPNAME:
	        do_getdmapname(false);
	        break;
            
        case CmdId_GETDMAPTITLE:
	        do_getdmaptitle(false);
	        break;
            
        case CmdId_GETDMAPINTRO:
	        do_getdmapintro(false);
	        break;
            
        case CmdId_SETDMAPNAME:
	        do_setdmapname(false);
	        break;
            
        case CmdId_SETDMAPTITLE:
	        do_setdmaptitle(false);
	        break;
	
        case CmdId_SETDMAPINTRO:
	        do_setdmapintro(false);
	        break;
	
        case CmdId_LOADLWEAPONR:
	        do_loadlweapon(false);
	        break;
            
        case CmdId_LOADLWEAPONV:
	        do_loadlweapon(true);
	        break;
            
        case CmdId_LOADEWEAPONR:
	        do_loadeweapon(false);
	        break;
            
        case CmdId_LOADEWEAPONV:
	        do_loadeweapon(true);
	        break;
            
        case CmdId_LOADITEMR:
	        do_loaditem(false);
	        break;
            
        case CmdId_LOADITEMV:
	        do_loaditem(true);
	        break;
            
        case CmdId_LOADITEMDATAR:
	        do_loaditemdata(false);
	        break;
            
        case CmdId_LOADITEMDATAV:
	        do_loaditemdata(true);
	        break;
            
        case CmdId_LOADNPCR:
	        do_loadnpc(false);
	        break;
            
        case CmdId_LOADNPCV:
	        do_loadnpc(true);
	        break;
            
        case CmdId_CREATELWEAPONR:
	        do_createlweapon(false);
	        break;
            
        case CmdId_CREATELWEAPONV:
	        do_createlweapon(true);
	        break;
            
        case CmdId_CREATEEWEAPONR:
	        do_createeweapon(false);
	        break;
            
        case CmdId_CREATEEWEAPONV:
	        do_createeweapon(true);
	        break;
            
        case CmdId_CREATEITEMR:
	        do_createitem(false);
	        break;
            
        case CmdId_CREATEITEMV:
	        do_createitem(true);
	        break;
            
        case CmdId_CREATENPCR:
	        do_createnpc(false);
	        break;
            
        case CmdId_CREATENPCV:
	        do_createnpc(true);
	        break;
            
        case CmdId_ISVALIDITEM:
	        do_isvaliditem();
	        break;
            
        case CmdId_ISVALIDNPC:
	        do_isvalidnpc();
	        break;
            
        case CmdId_ISVALIDLWPN:
	        do_isvalidlwpn();
	        break;
            
        case CmdId_ISVALIDEWPN:
	        do_isvalidewpn();
	        break;
            
        case CmdId_LWPNUSESPRITER:
	        do_lwpnusesprite(false);
	        break;
            
        case CmdId_LWPNUSESPRITEV:
	        do_lwpnusesprite(true);
	        break;
            
        case CmdId_EWPNUSESPRITER:
	        do_ewpnusesprite(false);
	        break;
            
        case CmdId_EWPNUSESPRITEV:
	        do_ewpnusesprite(true);
	        break;
            
        case CmdId_CLEARSPRITESR:
	        do_clearsprites(false);
	        break;
            
        case CmdId_CLEARSPRITESV:
	        do_clearsprites(true);
	        break;
            
        case CmdId_ISSOLID:
	        do_issolid();
	        break;
            
        case CmdId_SETSIDEWARP:
	        do_setsidewarp();
	        break;
            
        case CmdId_SETTILEWARP:
	        do_settilewarp();
	        break;
            
        case CmdId_GETSIDEWARPDMAP:
	        do_getsidewarpdmap(false);
	        break;
            
        case CmdId_GETSIDEWARPSCR:
	        do_getsidewarpscr(false);
	        break;
            
        case CmdId_GETSIDEWARPTYPE:
	        do_getsidewarptype(false);
	        break;
            
        case CmdId_GETTILEWARPDMAP:
	        do_gettilewarpdmap(false);
	        break;
            
        case CmdId_GETTILEWARPSCR:
	        do_gettilewarpscr(false);
	        break;
            
        case CmdId_GETTILEWARPTYPE:
	        do_gettilewarptype(false);
	        break;
            
        case CmdId_LAYERSCREEN:
	        do_layerscreen();
	        break;
            
        case CmdId_LAYERMAP:
	        do_layermap();
	        break;
            
        case CmdId_SECRETS:
	        do_triggersecrets();
	        break;
            
        case CmdId_GETSCREENFLAGS:
	        do_getscreenflags();
	        break;
            
        case CmdId_GETSCREENEFLAGS:
	        do_getscreeneflags();
	        break;
	
        case CmdId_GETSCREENDOOR:
	        do_getscreendoor();
	        break;
	
        case CmdId_GETSCREENENEMY:
	        do_getscreennpc();
	        break;
            
        case CmdId_GETSCREENLAYOP:
	        do_getscreenLayerOpacity();
	        break;
        case CmdId_GETSCREENSECCMB:
	        do_getscreenSecretCombo();
	        break;
        case CmdId_GETSCREENSECCST:
	        do_getscreenSecretCSet();
	        break;
        case CmdId_GETSCREENSECFLG:
	        do_getscreenSecretFlag();
	        break;
        case CmdId_GETSCREENLAYMAP:
	        do_getscreenLayerMap();
	        break;
        case CmdId_GETSCREENLAYSCR:
	        do_getscreenLayerscreen();
	        break;
        case CmdId_GETSCREENPATH:
	        do_getscreenPath();
	        break;
        case CmdId_GETSCREENWARPRX:
	        do_getscreenWarpReturnX();
	        break;
        case CmdId_GETSCREENWARPRY:
	        do_getscreenWarpReturnY();
	        break;

        case CmdId_COMBOTILE:
	        do_combotile(false);
	        break;
            
        case CmdId_RECTR:
        case CmdId_CIRCLER:
        case CmdId_ARCR:
        case CmdId_ELLIPSER:
        case CmdId_LINER:
        case CmdId_PUTPIXELR:
        case CmdId_DRAWTILER:
        case CmdId_DRAWCOMBOR:
        case CmdId_DRAWCHARR:
        case CmdId_DRAWINTR:
        case CmdId_QUADR:
        case CmdId_TRIANGLER:
        case CmdId_QUAD3DR:
        case CmdId_TRIANGLE3DR:
        case CmdId_FASTTILER:
        case CmdId_FASTCOMBOR:
        case CmdId_DRAWSTRINGR:
        case CmdId_SPLINER:
        case CmdId_BITMAPR:
        case CmdId_BITMAPEXR:
        case CmdId_DRAWLAYERR:
        case CmdId_DRAWSCREENR:
	        //case CmdId_BITMAPEXR:
	        //case CmdId_POLYGONR:
	        //case CmdId_PIXELARRAYR:
	        //case CmdId_TILEARRAYR:
	        //case CmdId_COMBOARRAYR:
	        do_drawing_command(scommand);
	        break;
            
        case CmdId_COPYTILEVV:
	        do_copytile(true, true);
	        break;
            
        case CmdId_COPYTILEVR:
	        do_copytile(true, false);
	        break;
            
        case CmdId_COPYTILERV:
	        do_copytile(false, true);
	        break;
            
        case CmdId_COPYTILERR:
	        do_copytile(false, false);
	        break;
            
        case CmdId_SWAPTILEVV:
	        do_swaptile(true, true);
	        break;
            
        case CmdId_SWAPTILEVR:
	        do_swaptile(true, false);
	        break;
            
        case CmdId_SWAPTILERV:
	        do_swaptile(false, true);
	        break;
            
        case CmdId_SWAPTILERR:
	        do_swaptile(false, false);
	        break;
            
        case CmdId_CLEARTILEV:
	        do_cleartile(true);
	        break;
            
        case CmdId_CLEARTILER:
	        do_cleartile(false);
	        break;
            
        case CmdId_OVERLAYTILEVV:
	        do_overlaytile(true, true);
	        break;
            
        case CmdId_OVERLAYTILEVR:
	        do_overlaytile(true, false);
	        break;
            
        case CmdId_OVERLAYTILERV:
	        do_overlaytile(false, true);
	        break;
            
        case CmdId_OVERLAYTILERR:
	        do_overlaytile(false, false);
	        break;
            
        case CmdId_FLIPROTTILEVV:
	        do_fliprotatetile(true, true);
	        break;
            
        case CmdId_FLIPROTTILEVR:
	        do_fliprotatetile(true, false);
	        break;
            
        case CmdId_FLIPROTTILERV:
	        do_fliprotatetile(false, true);
	        break;
            
        case CmdId_FLIPROTTILERR:
	        do_fliprotatetile(false, false);
	        break;
            
        case CmdId_GETTILEPIXELV:
	        do_gettilepixel(true);
	        break;
            
        case CmdId_GETTILEPIXELR:
	        do_gettilepixel(false);
	        break;
            
        case CmdId_SETTILEPIXELV:
	        do_settilepixel(true);
	        break;
            
        case CmdId_SETTILEPIXELR:
	        do_settilepixel(false);
	        break;
            
        case CmdId_SHIFTTILEVV:
	        do_shifttile(true, true);
	        break;
            
        case CmdId_SHIFTTILEVR:
	        do_shifttile(true, false);
	        break;
            
        case CmdId_SHIFTTILERV:
	        do_shifttile(false, true);
	        break;
            
        case CmdId_SHIFTTILERR:
	        do_shifttile(false, false);
	        break;
            
        case CmdId_SETRENDERTARGET:
	        do_set_rendertarget(true);
	        break;
            
        case CmdId_GAMEEND:
	        Quit = qQUIT;
	        skipcont = 1;
	        scommand = 0xFFFF;
	        break;
            
        case CmdId_SAVE:
	        if(scriptCanSave)
	        {
		        save_game(false);
		        scriptCanSave=false;
	        }
	        break;
            
        case CmdId_SAVESCREEN:
	        do_showsavescreen();
	        break;
            
        case CmdId_SAVEQUITSCREEN:
	        save_game(false, 1);
	        break;
            
	        //Not Implemented
        case CmdId_ELLIPSE2:
        case CmdId_FLOODFILL:
	        break;
            
        case CmdId_SETCOLORB:
        case CmdId_SETDEPTHB:
        case CmdId_GETCOLORB:
        case CmdId_GETDEPTHB:
	        break;
            
        case CmdId_ENQUEUER:
	        do_enqueue(false);
	        break;
            
        case CmdId_ENQUEUEV:
	        do_enqueue(true);
	        break;
            
        case CmdId_DEQUEUE:
	        do_dequeue(false);
	        break;
	
	        //Visual Effects
        case CmdId_WAVYIN:
	        FFScript::do_wavyin();
	        break;
        case CmdId_WAVYOUT:
	        FFScript::do_wavyout();
	        break;
        case CmdId_ZAPIN:
	        FFScript::do_zapin();
	        break;
        case CmdId_ZAPOUT:
	        FFScript::do_zapout();
	        break;
        case CmdId_OPENWIPE:
	        FFScript::do_openscreen();
	        break;
	
	        //Monochrome
        case CmdId_GREYSCALEON:
	        setMonochrome(true);
	        break;
        case CmdId_GREYSCALEOFF:
	        setMonochrome(false);
	        break;
	
	
	
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
            scommand = curscript->commands[pc].command;
            sarg1 = curscript->commands[pc].arg1;
            sarg2 = curscript->commands[pc].arg2;
        }
    }
    
    if(!scriptCanSave)
        scriptCanSave=true;
    
    if (scommand == CmdId_WAITDRAW)
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
            
        if((tmpscr->ffflags[i]&ffIGNOREHOLDUP)==0 && Link->getHoldClk()>0)
            continue;
            
		run_script(SCRIPT_FFC, tmpscr->ffscript[i], i);
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
    //TODO script support for modules
    EnemyDefinitionRef ref("CORE", value);
    m->enemy[enem_indx] = ref;
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
    // TODO script module support
    int bloke = vbound(value,0,9); 
    EnemyDefinitionRef guyref("CORE", bloke);
    m->guy = guyref;
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
    //TODO module support for scripts
    ItemDefinitionRef ref("CORE", value);
    if (!curQuest->isValid(ref))
    {
        Z_scripterrlog("Cannot set screen item to invalid item id %ld\n", ref.slot);
    }
    else
    {
        m->screenItem = ref;
    }
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

    //TODO module support for scripts
    // also, the fact that this function does so many different things (depending on room type) is not good.
    // should be split into one function per room type
    ItemDefinitionRef ref("CORE", ctch);
    m->catchallItem = ref;
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

void FFScript::do_changeffcscript(const bool v){
	long ID = vbound((SH::get_arg(sarg1, v) / 10000), 0, 255);
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
}

long get_itemdata_register(long arg)
{
    long ret = -1;
    ItemDefinitionRef ref("CORE", ri->idata);
    if (!curQuest->isValid(ref))
    {
        Z_scripterrlog("Invalid itemdata index = %ld\n", ri->idata);
        return -10000;
    }

    const itemdata &idata = curQuest->getItemDefinition(ref);

    switch (arg)
    {
    case VarId_IDATAUSEWPN:
        ret = (idata.useweapon) * 10000;
        break;

    case VarId_IDATAUSEDEF:
        ret = (idata.usedefence) * 10000;
        break;

    case VarId_IDATAWRANGE:
        ret = (idata.weaprange) * 10000;
        break;

    case VarId_IDATAUSEMVT:
    {
        long a = vbound((ri->d[0] / 10000), 0, (ITEM_MOVEMENT_PATTERNS - 1));
        ret = (idata.weap_pattern[a]) * 10000;
        break;
    }

    case VarId_IDATADURATION:
        ret = (idata.weapduration) * 10000;
        break;

    case VarId_IDATADUPLICATES:
        ret = (idata.duplicates) * 10000;
        break;

    case VarId_IDATADRAWLAYER:
        ret = (idata.drawlayer) * 10000;
        break;

    case VarId_IDATACOLLECTFLAGS:
        ret = (idata.collectflags) * 10000;
        break;

    case VarId_IDATAWEAPONSCRIPT:
        ret = (idata.weaponscript) * 10000;
        break;

    case VarId_IDATAMISCD:
    {
        int a = vbound((ri->d[0] / 10000), 0, 31);
        ret = (idata.wpn_misc_d[a]) * 10000;
        break;
    }

    case VarId_IDATAWPNINITD:
    {
        int a = vbound((ri->d[0] / 10000), 0, 7);
        ret = (idata.weap_initiald[a]) * 10000;
        break;
    }

    case VarId_IDATAWEAPHXOFS:
    {
        ret = (idata.weap_hxofs) * 10000;
        break;
    }

    case VarId_IDATAWEAPHYOFS:
    {
        ret = (idata.weap_yxofs) * 10000;
        break;
    }

    case VarId_IDATAWEAPHXSZ:
    {
        ret = (idata.weap_hxsz) * 10000;
        break;
    }

    case VarId_IDATAWEAPHYSZ:
    {
        ret = (idata.weap_hysz) * 10000;
        break;
    }

    case VarId_IDATAWEAPHZSZ:
    {
        ret = (idata.weap_hzsz) * 10000;
        break;
    }

    case VarId_IDATAWEAPXOFS:
    {
        ret = (idata.weap_xofs) * 10000;
        break;
    }

    case VarId_IDATAWEAPYOFS:
    {
        ret = (idata.weap_yofs) * 10000;
        break;
    }

    case VarId_IDATAFAMILY:
        ret = (idata.family) * 10000;
        break;

    case VarId_IDATALEVEL:
        ret = (idata.fam_type) * 10000;
        break;

    case VarId_IDATAKEEP:
        ret = (idata.flags & itemdata::IF_GAMEDATA) ? 10000 : 0;
        break;

    case VarId_IDATAAMOUNT:
        ret = (idata.amount) * 10000;
        break;

    case VarId_IDATASETMAX:
        ret = (idata.setmax) * 10000;
        break;

    case VarId_IDATAMAX:
        ret = (idata.max) * 10000;
        break;

    case VarId_IDATACOUNTER:
        ret = (idata.count) * 10000;
        break;

    case VarId_IDATAUSESOUND:
        ret = (idata.usesound) * 10000;
        break;

    case VarId_IDATAPOWER:
        ret = (idata.power) * 10000;
        break;

        //Get the script assigned to an item (active)
    case VarId_IDATASCRIPT:
        ret = (idata.script) * 10000;
        break;

        //Get the ->Attributes[] of an item
    case VarId_IDATAATTRIB:
    {
        int index = vbound(ri->d[0] / 10000, 0, 9);
        switch (index) {
        case 0:
            ret = (idata.misc1) * 10000;
            break;
        case 1:
            ret = (idata.misc2) * 10000; break;
        case 2:
            ret = (idata.misc3) * 10000; break;
        case 3:
            ret = (idata.misc4) * 10000; break;
        case 4:
            ret = (idata.misc5) * 10000; break;
        case 5:
            ret = (idata.misc6) * 10000; break;
        case 6:
            ret = (idata.misc7) * 10000; break;
        case 7:
            ret = (idata.misc8) * 10000; break;
        case 8:
            ret = (idata.misc9) * 10000; break;
        case 9:
            ret = (idata.misc10) * 10000; break;
        default:
            ret = -10000; break;
        }
        break;
    }

    //Get the ->Sprite[] of an item.
    case VarId_IDATASPRITE:
    {
        //TODO: script support for modules
        int index = vbound(ri->d[0] / 10000, 0, 9);
        ret = (idata.wpns[index]).slot * 10000;

        break;
    }
    //Link TIle modifier
    case VarId_IDATALTM:
        ret = (idata.ltm) * 10000;
        break;

        //Pickup script
    case VarId_IDATAPSCRIPT:
        ret = (idata.collect_script) * 10000;
        break;

        //Magic cost
    case VarId_IDATAMAGCOST:
        ret = (idata.magic) * 10000;
        break;

        //Min Hearts to Pick Up
    case VarId_IDATAMINHEARTS:
        ret = (idata.pickup_hearts) * 10000;
        break;

        //Tile used by the item
    case VarId_IDATATILE:
        ret = (idata.tile) * 10000;
        break;

        //itemdata->Flash
    case VarId_IDATAMISC:
        ret = (idata.misc) * 10000;
        break;

        //->CSet
    case VarId_IDATACSET:
        ret = (idata.csets) * 10000;
        break;

        //->A.Frames
    case VarId_IDATAFRAMES:
        ret = (idata.frames) * 10000;
        break;

        //->A.Speed
    case VarId_IDATAASPEED:
        ret = (idata.speed) * 10000;
        break;

        //->Delay
    case VarId_IDATADELAY:
        ret = (idata.delay) * 10000;
        break;

        // teo of this item upgrades
    case VarId_IDATACOMBINE:
        ret = (idata.flags & itemdata::IF_COMBINE) ? 10000 : 0;
        break;

        //Use item, and get the lower level one
    case VarId_IDATADOWNGRADE:
        ret = (idata.flags & itemdata::IF_DOWNGRADE) ? 10000 : 0;
        break;

        //->Flags[5]
    case VarId_IDATAFLAGS:
    {
        int index = vbound(ri->d[0] / 10000, 0, 4);
        switch (index) {
        case 0:
            ret = (idata.flags & itemdata::IF_FLAG1) ? 10000 : 0;
            break;
        case 1:
            ret = (idata.flags & itemdata::IF_FLAG2) ? 10000 : 0; break;
        case 2:
            ret = (idata.flags & itemdata::IF_FLAG3) ? 10000 : 0; break;
        case 3:
            ret = (idata.flags & itemdata::IF_FLAG4) ? 10000 : 0; break;
        case 4:
            ret = (idata.flags & itemdata::IF_FLAG5) ? 10000 : 0; break;

        default:
            ret = 0; break;
        }

        break;
    }

    //->Keep Old
    case VarId_IDATAKEEPOLD:
        ret = (idata.flags & itemdata::IF_KEEPOLD) ? 10000 : 0;
        break;

        //Use rupees instead of magic
    case VarId_IDATARUPEECOST:
        ret = (idata.flags & itemdata::IF_RUPEE_MAGIC) ? 10000 : 0;
        break;

        //Can be eaten
    case VarId_IDATAEDIBLE:
        ret = (idata.flags & itemdata::IF_EDIBLE) ? 10000 : 0;
        break;

        //Not int he editor, could become flags[6], but I'm reserving this one for other item uses. 
    case VarId_IDATAFLAGUNUSED:
        ret = (idata.flags & itemdata::IF_UNUSED) ? 10000 : 0;
        break;

        //Gain lower level items when collected
    case VarId_IDATAGAINLOWER:
        ret = (idata.flags & itemdata::IF_GAINOLD) ? 10000 : 0;
        break;

        //Unchanged from master
    case VarId_IDATAINITDD:
    {
        int a = ri->d[0] / 10000;

        if (BC::checkBounds(a, 0, 7, "itemdata->InitD") != SH::_NoError)
            ret = -10000;
        else
            ret = idata.initiald[a];

        break;
    }    
    }
    return ret;
}

void set_itemdata_register(long arg, long value)
{
    ItemDefinitionRef ref("CORE", ri->idata);
    if (!curQuest->isValid(ref))
    {
        Z_scripterrlog("Invalid itemdata index = %ld\n", ri->idata);
        return;
    }

    itemdata &idata = curQuest->getItemDefinition(ref);

    switch (arg)
    {
    case VarId_IDATAFAMILY:
        idata.family = vbound(value / 10000, 0, 254);
        flushItemCache();
        break;

    case VarId_IDATAUSEWPN:
        idata.useweapon = vbound(value / 10000, 0, 255);
        break;
    case VarId_IDATAUSEDEF:
        idata.usedefence = vbound(value / 10000, 0, 255);
        break;
    case VarId_IDATAWRANGE:
        idata.weaprange = vbound(value / 10000, 0, 255);
        break;
    case VarId_IDATADURATION:
        idata.weapduration = vbound(value / 10000, 0, 255);
        break;

    case VarId_IDATADUPLICATES:
        idata.duplicates = vbound(value / 10000, 0, 255);
        break;
    case VarId_IDATADRAWLAYER:
        idata.drawlayer = vbound(value / 10000, 0, 7);
        break;
    case VarId_IDATACOLLECTFLAGS:
        //int a = ri->d[0] / 10000;
        idata.collectflags = vbound(value / 10000, 0, 214747);
        break;
    case VarId_IDATAWEAPONSCRIPT:
        idata.weaponscript = vbound(value / 10000, 0, 255);
        break;
    case VarId_IDATAMISCD:
    {

        int a = vbound((ri->d[0] / 10000), 0, 31);
        idata.wpn_misc_d[a] = (value / 10000);
    }
    break;
    case VarId_IDATAWPNINITD:
    {

        int a = vbound((ri->d[0] / 10000), 0, 7);
        idata.weap_initiald[a] = (value / 10000);
    }
    break;
    case VarId_IDATAWEAPHXOFS:
        idata.weap_hxofs = (value / 10000);
        break;
    case VarId_IDATAWEAPHYOFS:
        idata.weap_yxofs = (value / 10000);
        break;
    case VarId_IDATAWEAPHXSZ:
        idata.weap_hxsz = (value / 10000);
        break;
    case VarId_IDATAWEAPHYSZ:
        idata.weap_hysz = (value / 10000);
        break;
    case VarId_IDATAWEAPHZSZ:
        idata.weap_hzsz = (value / 10000);
        break;
    case VarId_IDATAWEAPXOFS:
        idata.weap_xofs = (value / 10000);
        break;
    case VarId_IDATAWEAPYOFS:
        idata.weap_yofs = (value / 10000);
        break;


    case VarId_IDATAUSEMVT:
    {
        long a = vbound((ri->d[0] / 10000), 0, (ITEM_MOVEMENT_PATTERNS - 1));
        idata.weap_pattern[a] = vbound(value / 10000, 0, 255);
    }
    break;


    //item level
    case VarId_IDATALEVEL:
        idata.fam_type = vbound(value / 10000, 0, 512);
        flushItemCache();
        break;
        //bool keep
    case VarId_IDATAKEEP:
        idata.flags |= (value / 10000) ? itemdata::IF_GAMEDATA : 0;
        break;
        //Need the legal range -Z
    case VarId_IDATAAMOUNT:
        idata.amount = value / 10000;
        break;

    case VarId_IDATASETMAX:
        idata.setmax = value / 10000;
        break;

    case VarId_IDATAMAX:
        idata.max = value / 10000;
        break;

    case VarId_IDATAPOWER:
        idata.power = value / 10000;
        break;

    case VarId_IDATACOUNTER:
        idata.count = vbound(value / 10000, 0, 31);
        break;

    case VarId_IDATAUSESOUND:
        idata.usesound = vbound(value / 10000, 0, 255);
        break;

        //2.54
        //My additions begin here. -Z
        //Stack item to gain next level
    case VarId_IDATACOMBINE:
        idata.flags |= (value / 10000) ? itemdata::IF_COMBINE : 0;
        break;
        //using a level of an item downgrades to a lower one
    case VarId_IDATADOWNGRADE:
        idata.flags |= (value / 10000) ? itemdata::IF_DOWNGRADE : 0;
        break;

        //Flags[5]
    case VarId_IDATAFLAGS: {
        int index = vbound(ri->d[0] / 10000, 0, 4);
        switch (index) {
        case 0:
            idata.flags |= (value / 10000) ? itemdata::IF_FLAG1 : 0;
            break;
        case 1:
            idata.flags |= (value / 10000) ? itemdata::IF_FLAG2 : 0;
            break;
        case 2:
            idata.flags |= (value / 10000) ? itemdata::IF_FLAG3 : 0;
            break;
        case 3:
            idata.flags |= (value / 10000) ? itemdata::IF_FLAG4 : 0;
            break;
        case 4:
            idata.flags |= (value / 10000) ? itemdata::IF_FLAG5 : 0;
            break;


        default:
            break;
        }

        break;
    }
                     //Keep Old in editor
    case VarId_IDATAKEEPOLD:
        idata.flags |= (value / 10000) ? itemdata::IF_KEEPOLD : 0;
        break;
        //Ruppes for magic
    case VarId_IDATARUPEECOST:
        idata.flags |= (value / 10000) ? itemdata::IF_RUPEE_MAGIC : 0;
        break;
        //can be eaten
    case VarId_IDATAEDIBLE:
        idata.flags |= (value / 10000) ? itemdata::IF_EDIBLE : 0;
        break;
        //Reserving this for item editor stuff. 
    case VarId_IDATAFLAGUNUSED:
        idata.flags |= (value / 10000) ? itemdata::IF_UNUSED : 0;
        break;
        //gain lower level items
    case VarId_IDATAGAINLOWER:
        idata.flags |= (value / 10000) ? itemdata::IF_GAINOLD : 0;
        break;
        //Set the action script
    case VarId_IDATASCRIPT:
        idata.script = vbound(value / 10000, 1, 255);
        break;

        /*
        case VarId_ITEMMISCD:
        if(0!=(s=checkItem(ri->itemref)))
        {
        int a = vbound(ri->d[0]/10000,0,31);
        (((item*)(s))->miscellaneous[a])=value;
        }

        break;*/
        //Attributes[10]
    case VarId_IDATAATTRIB: {
        int index = vbound(ri->d[0] / 10000, 0, 9);
        switch (index) {
        case 0:
            idata.misc1 = value / 10000;
            break;
        case 1:
            idata.misc2 = value / 10000; 
            break;
        case 2:
            idata.misc3 = value / 10000; 
            break;
        case 3:
            idata.misc4 = value / 10000; 
            break;
        case 4:
            idata.misc5 = value / 10000; 
            break;
        case 5:
            idata.misc6 = value / 10000; 
            break;
        case 6:
            idata.misc7 = value / 10000;
            break;
        case 7:
            idata.misc8 = value / 10000;
            break;
        case 8:
            idata.misc9 = value / 10000;
            break;
        case 9:
            idata.misc10 = value / 10000; 
            break;

        default:
            break;
        }

        break;
    }
                      //Sprites[10]
    case VarId_IDATASPRITE: {
        int index = vbound(ri->d[0] / 10000, 0, 9);
        //TODO script module support
        SpriteDefinitionRef newref("CORE", value / 10000);
        if (!curQuest->isValid(newref))
        {
            Z_scripterrlog("Cannot set sprite to invalid slot %ld", value / 10000);
        }
        else
        {
            idata.wpns[index] = newref;
        }
        break;
    }
                      //Link tile modifier. 
    case VarId_IDATALTM:
        idata.ltm = value / 10000;
        break;
        //Pickup script
    case VarId_IDATAPSCRIPT:
        idata.collect_script = (value / 10000, 1, 255);
        break;
        //magic cost
    case VarId_IDATAMAGCOST:
        idata.magic = value / 10000;
        break;
        //min hearts to pick up
    case VarId_IDATAMINHEARTS:
        idata.pickup_hearts = vbound(value / 10000, 0, 214748);
        break;
        //item tile
    case VarId_IDATATILE:
        idata.tile = vbound(value / 10000, 0, 65519);
        break;
        //flash
    case VarId_IDATAMISC:
        idata.misc = value / 10000;
        break;
        //cset
    case VarId_IDATACSET:
        idata.csets = vbound(value / 10000, 0, 13);
        break;
        //A.Frames
    case VarId_IDATAFRAMES:
        idata.frames = vbound(value / 10000, 0, 214748);
        break;
        //A.speed
    case VarId_IDATAASPEED:
        idata.speed = vbound(value / 10000, 0, 214748);
        break;
        //Anim delay
    case VarId_IDATADELAY:
        idata.delay = vbound(value / 10000, 0, 214748);
        break;

        //not one of mine. 
    case VarId_IDATAINITDD:
    {
        int a = ri->d[0] / 10000;

        if (BC::checkBounds(a, 0, 7, "itemdata->InitD") == SH::_NoError)
            idata.initiald[a] = value;
    }
    break;

    default:
        assert(false);
    }
}
