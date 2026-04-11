#ifndef ZC_SCRIPTING_COMMON_H_
#define ZC_SCRIPTING_COMMON_H_

#include "allegro/base.h"
#include "base/expected.h"
#include "base/general.h"
#include "core/mapscr.h"
#include "zc/hero.h"
#include "zc/maps.h"
#include "zc/scripting/array_manager.h"

#include <string>

#define ZSCRIPT_MAX_STRING_CHARS 214748

// Avoid including ffscript.h b/c it causes circular includes, errors on windows.

int32_t get_register(int32_t arg);

// Avoid including ffscript.h b/c it causes circular includes, errors on windows.
void scripting_log_error_with_context(std::string text);

namespace {

template <typename... Args>
void _scripting_log_error_with_context(fmt::format_string<Args...> s, Args&&... args)
{
	std::string text = fmt::format(s, std::forward<Args>(args)...);
	scripting_log_error_with_context(text);
}

}

class SH
{

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

	static void write_stack(const uint32_t stackoffset, const int32_t value);
	static int32_t read_stack(const uint32_t stackoffset);
	static INLINE int32_t get_arg(int32_t arg, bool v)
	{
		return v ? arg : get_register(arg);
	}
};

class ArrayH : public SH
{
public:
	static size_t getSize(const int32_t ptr);
	
	//Can't you get the std::string and then check its length?
	static int32_t strlen(const int32_t ptr);
	
	//Returns values of a zscript array as an std::string.
	static void getString(const int32_t ptr, string &str, dword num_chars = ZSCRIPT_MAX_STRING_CHARS, dword offset = 0);
	
	//Used for issues where reading the ZScript array floods the console with errors 'Accessing array index [12] size of 12.
	//Happens with Quad3D and some other functions, and I have no clue why. -Z ( 28th April, 2019 )
	//Like getString but for an array of longs instead of chars. *(arrayPtr is not checked for validity)
	static void getValues2(const int32_t ptr, int32_t* arrayPtr, dword num_values, dword offset = 0);
	
	//Like getString but for an array of longs instead of chars. *(arrayPtr is not checked for validity)
	static void getValues(const int32_t ptr, int32_t* arrayPtr, dword num_values, dword offset = 0);
	
	static void copyValues(const int32_t ptr, const int32_t ptr2);

	//Get element from array
	static int32_t getElement(const int32_t ptr, int32_t offset, const bool neg = false);
	
	//Set element in array
	static void setElement(const int32_t ptr, int32_t offset, const int32_t value, const bool neg = false, const script_object_type type = script_object_type::none);
	
	//Puts values of a zscript array into a client <type> array. returns 0 on success. Overloaded
	template <typename T>
	static int32_t getArray(const int32_t ptr, T *refArray)
	{
		return getArray(ptr, getSize(ptr), 0, 0, 0, refArray);
	}
	
	template <typename T>
	static int32_t getArray(const int32_t ptr, const size_t size, T *refArray)
	{
		return getArray(ptr, size, 0, 0, 0, refArray);
	}
	
	template <typename T>
	static int32_t getArray(const int32_t ptr, const size_t size, size_t userOffset, const size_t userStride, const size_t refArrayOffset, T *refArray);
	
	static int32_t setArray(const int32_t ptr, string const& s2, bool resize = false);

	//Puts values of a client <type> array into a zscript array. returns 0 on success. Overloaded
	template <typename T>
	static int32_t setArray(const int32_t ptr, const size_t size, T *refArray, bool x10k = true, bool resize = false)
	{
		return setArray(ptr, size, 0, 0, 0, refArray, x10k, resize);
	}

	static INLINE int32_t checkUserArrayIndex(const int32_t index, const dword size, const bool neg = false)
	{
		if(index < (neg ? -int32_t(size) : 0) || index >= int32_t(size))
		{
			_scripting_log_error_with_context("Invalid index: {}, array size: {}", index, size);
			return _OutOfBounds;
		}
		
		return _NoError;
	}

	template <typename T>
	static int32_t setArray(const int32_t ptr, const size_t size, word userOffset, const word userStride, const word refArrayOffset, T *refArray, bool x10k = true, bool resize = false)
	{
		ArrayManager am(ptr);
		
		if (am.invalid())
			return _InvalidPointer;
		
		if(am.can_resize() && resize)
			am.resize_min((userStride+1)*size);
			
		word j = 0, k = userStride;
		size_t sz = am.size();
		for(word i = 0; j < size; i++)
		{
			if(i >= sz)
				return _Overflow; //Resize?
				
			if (userOffset > 0)
			{
				--userOffset;
				continue;
			}
				
			if(k > 0)
				k--;
			else if(checkUserArrayIndex(i, sz) == _NoError)
			{
				am.set(i,int32_t(refArray[j + refArrayOffset]) * (x10k ? 10000 : 1));
				k = userStride;
				j++;
			}
		}
		
		return _NoError;
	}
};

class BC : public SH
{
public:

	static INLINE int32_t checkMapID(const int32_t ID)
	{
		return checkBounds(ID, 0, map_count-1);
	}
	
	static INLINE int32_t checkDMapID(const int32_t ID)
	{
		return checkBounds(ID, 0, MAXDMAPS-1);
	}
	
	static INLINE int32_t checkComboPos(const int32_t pos)
	{
		return checkBoundsPos(pos, 0, 175);
	}

	static INLINE int32_t checkComboRpos(const rpos_t rpos)
	{
		return checkBoundsRpos(rpos, (rpos_t)0, region_max_rpos);
	}

	static INLINE int32_t checkTile(const int32_t pos)
	{
		return checkBounds(pos, 0, NEWMAXTILES-1);
	}
	
	static INLINE int32_t checkCombo(const int32_t pos)
	{
		return checkBounds(pos, 0, MAXCOMBOS-1);
	}
	
	static INLINE int32_t checkMessage(const int32_t ID)
	{
		return checkBounds(ID, 0, msg_strings_size-1);
	}
	
	static INLINE int32_t checkLayer(const int32_t layer)
	{
		return checkBounds(layer, 0, 6);
	}
	
	static INLINE int32_t checkFFC(ffc_id_t id)
	{
		return checkBoundsOneIndexed(id, 0, MAX_FFCID);
	}

	static INLINE int32_t checkMapdataFFC(int index)
	{
		return checkBounds(index, 0, MAXFFCS-1);
	}
	
	static INLINE int32_t checkGuyIndex(const int32_t index)
	{
		return checkBoundsOneIndexed(index, 0, guys.Count()-1);
	}
	
	static INLINE int32_t checkItemIndex(const int32_t index)
	{
		return checkBoundsOneIndexed(index, 0, items.Count()-1);
	}
	
	static INLINE int32_t checkEWeaponIndex(const int32_t index)
	{
		return checkBoundsOneIndexed(index, 0, Ewpns.Count()-1);
	}
	
	static INLINE int32_t checkLWeaponIndex(const int32_t index)
	{
		return checkBoundsOneIndexed(index, 0, Lwpns.Count()-1);
	}
	
	static INLINE int32_t checkGuyID(const int32_t ID)
	{
		//return checkBounds(ID, 0, MAXGUYS-1); //Can't create NPC ID 0
		return checkBounds(ID, 1, MAXGUYS-1);
	}
	
	static INLINE int32_t checkItemID(const int32_t ID)
	{
		return checkBounds(ID, 0, MAXITEMS-1);
	}
	
	static INLINE int32_t checkWeaponID(const int32_t ID)
	{
		return checkBounds(ID, 0, wMax-1);
	}
	
	static INLINE int32_t checkWeaponMiscSprite(const int32_t ID)
	{
		return checkBounds(ID, 0, MAXSPRITES-1);
	}
	
	static INLINE int32_t checkSFXID(const int32_t ID)
	{
		return checkBounds(ID, 0, MAX_SFX); // could check `quest_sounds.size()`, but want no error unless value is out of MAX bounds.
	}
	
	static INLINE int32_t checkBounds(const int32_t n, const int32_t boundlow, const int32_t boundup, const char* term = "value")
	{
		if(n < boundlow || n > boundup)
		{
			_scripting_log_error_with_context("Invalid {}: {} - must be >= {} and <= {}", term, n, boundlow, boundup);
			return _OutOfBounds;
		}
		
		return _NoError;
	}

	static INLINE int32_t checkIndex(const int32_t n, const int32_t boundlow, const int32_t boundup)
	{
		return checkBounds(n, boundlow, boundup, "index");
	}

	// Typical array indexing: >= 0 and < len.
	// TODO: use this in all index bound checks.
	static INLINE int32_t checkIndex2(int32_t n, int32_t len)
	{
		if(n < 0 || n >= len)
		{
			_scripting_log_error_with_context("Invalid index: {} - must be >= 0 and < {}", n, len);
			return _OutOfBounds;
		}

		return _NoError;
	}

	static INLINE int32_t checkIndex2OneIndex(int32_t n, int32_t len)
	{
		if(n <= 0 || n > len)
		{
			_scripting_log_error_with_context("Invalid index: {} - must be > 0 and <= {}", n, len);
			return _OutOfBounds;
		}

		return _NoError;
	}
	
	static INLINE int32_t checkBoundsPos(const int32_t n, const int32_t boundlow, const int32_t boundup)
	{
		return checkBounds(n, boundlow, boundup, "position");
	}

	static INLINE int32_t checkBoundsRpos(const rpos_t n, const rpos_t boundlow, const rpos_t boundup)
	{
		if(n < boundlow || n > boundup)
		{
			_scripting_log_error_with_context("Invalid position: {} - must be >= {} and <= {}", (int)n, (int)boundlow, (int)boundup);
			return _OutOfBounds;
		}
        
		return _NoError;
	}
	
	static INLINE int32_t checkBoundsOneIndexed(const int32_t n, const int32_t boundlow, const int32_t boundup)
	{
		if (boundup < 0)
		{
			_scripting_log_error_with_context("Invalid index: {} (empty)", n + 1);
			return _OutOfBounds;
		}

		if(n < boundlow || n > boundup)
		{
			_scripting_log_error_with_context("Invalid index: {} - must be >= {} and <= {}", n + 1, boundlow + 1, boundup + 1);
			return _OutOfBounds;
		}
		
		return _NoError;
	}
	
	static INLINE int32_t checkUserArrayIndex(const int32_t index, const dword size, const bool neg = false)
	{
		if(index < (neg ? -int32_t(size) : 0) || index >= int32_t(size))
		{
			_scripting_log_error_with_context("Invalid index: {}, array size: {}", index, size);
			return _OutOfBounds;
		}
		
		return _NoError;
	}
};

expected<std::string, std::string> parse_user_path(const std::string& user_path, bool is_file);

#endif
