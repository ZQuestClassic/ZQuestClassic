#include "zc/scripting/array_manager.h"
#include "base/general.h"
#include "base/qrs.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/script_object.h"
#include "zscriptversion.h"
#include <cstdint>

extern bool can_neg_array;

namespace {

int32_t legacy_get_int_arr(const int32_t ptr, int32_t indx)
{
	switch(ptr)
	{
		case INTARR_SCREEN_NPC:
		{
			current_zasm_context = "Screen->NPCs[]";
			if(BC::checkGuyIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return guys.spr(indx)->getUID();
		}
		case INTARR_SCREEN_ITEMSPR:
		{
			current_zasm_context = "Screen->Items[]";
			if(BC::checkItemIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return items.spr(indx)->getUID();
		}
		case INTARR_SCREEN_LWPN:
		{
			current_zasm_context = "Screen->LWeapons[]";
			if(BC::checkLWeaponIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return Lwpns.spr(indx)->getUID();
		}
		case INTARR_SCREEN_EWPN:
		{
			current_zasm_context = "Screen->EWeapons[]";
			if(BC::checkEWeaponIndex(indx) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return Ewpns.spr(indx)->getUID();
		}
		case INTARR_SCREEN_FFC:
		{
			current_zasm_context = "Screen->FFCs[]";
			if (auto ffc = ResolveFFCWithID(indx))
			{
				current_zasm_context = "";

				if (ZScriptVersion::ffcRefIsSpriteId())
					return ffc->getUID();

				return indx * 10000;
			}

			return 0;
		}
		case INTARR_SCREEN_PORTALS:
		{
			current_zasm_context = "Screen->Portals[]";
			if(BC::checkBoundsOneIndexed(indx, 0, portals.Count()-1) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return portals.spr(indx)->getUID();
		}
		case INTARR_SAVPRTL:
		{
			current_zasm_context = "Game->SavedPortals[]";
			if(BC::checkBoundsOneIndexed(indx, 0, game->user_portals.size()-1) != SH::_NoError)
				return 0;
			current_zasm_context = "";

			return (indx+1)*10000;
		}
		default:
		{
			scripting_log_error_with_context("Unknown internal array '{}' read from!", ptr);
			return 0;
		}
	}
}
void legacy_set_int_arr(const int32_t ptr, int32_t indx, int32_t val)
{
	switch(ptr)
	{
		case INTARR_SCREEN_NPC:
			scripting_log_error_with_context("Read-only array 'Screen->NPCs' cannot be written to!");
			return;
		case INTARR_SCREEN_ITEMSPR:
			scripting_log_error_with_context("Read-only array 'Screen->Items' cannot be written to!");
			return;
		case INTARR_SCREEN_LWPN:
			scripting_log_error_with_context("Read-only array 'Screen->LWeapons' cannot be written to!");
			return;
		case INTARR_SCREEN_EWPN:
			scripting_log_error_with_context("Read-only array 'Screen->EWeapons' cannot be written to!");
			return;
		case INTARR_SCREEN_FFC:
			scripting_log_error_with_context("Read-only array 'Screen->FFCs' cannot be written to!");
			return;
		case INTARR_SCREEN_PORTALS:
			scripting_log_error_with_context("Read-only array 'Screen->Portals' cannot be written to!");
			return;
		case INTARR_SAVPRTL:
			scripting_log_error_with_context("Read-only array 'Game->SavedPortals' cannot be written to!");
			return;
		
		default:
		{
			scripting_log_error_with_context("Unknown internal array '{}' written to!", ptr);
			return;
		}
	}
}

int32_t legacy_sz_int_arr(const int32_t ptr)
{
	switch(ptr)
	{
		case INTARR_SCREEN_NPC:
		{
			return guys.Count();
		}
		case INTARR_SCREEN_ITEMSPR:
		{
			return items.Count();
		}
		case INTARR_SCREEN_LWPN:
		{
			return Lwpns.Count();
		}
		case INTARR_SCREEN_EWPN:
		{
			return Ewpns.Count();
		}
		case INTARR_SCREEN_FFC:
		{
			return MAXFFCS;
		}
		case INTARR_SCREEN_PORTALS:
		{
			return portals.Count();
		}
		case INTARR_SAVPRTL:
		{
			return game->user_portals.size();
		}
		default:
		{
			scripting_log_error_with_context("Unknown internal array '{}' size read!", ptr);
			return -1;
		}
	}
}

}; // namespace

ArrayManager::ArrayManager(int32_t ptr, bool neg) : negAccess(neg)
{
	_invalid = false;
	internal_array_id = {};
	legacy_internal_id = 0;
	script_array_object = nullptr;

	if (ZScriptVersion::gc_arrays())
	{
		if (auto* array = checkArray(ptr))
		{
			script_array_object = array;

			if (array->internal_id.has_value())
			{
				if (array->internal_expired)
				{
					current_zasm_extra_context = scripting_get_zasm_register_context_string(array->internal_id->zasm_var);
					scripting_log_error_with_context("Invalid internal array: the object this array refers to has expired");
					_invalid = true;
					return;
				}

				aptr = nullptr;
				internal_array_id = array->internal_id.value();
				if (!zasm_array_supports(internal_array_id.zasm_var))
				{
					scripting_log_error_with_context("Invalid internal array id: {}", internal_array_id.zasm_var);
					_invalid = true;
					return;
				}
			}
			else
			{
				aptr = &array->arr;
			}
		}
		else
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}

		if (_invalid)
			scripting_log_error_with_context("Invalid pointer used as array: {}", ptr);
		return;
	}

	ptr /= 10000;

	if(ptr >= INTARR_OFFS)
	{
		aptr = nullptr;
		legacy_internal_id = ptr;
		if(legacy_sz_int_arr(ptr) < 0)
			_invalid = true;
	}
	else if(ptr == 0)
	{
		aptr = &INVALIDARRAY;
		_invalid = true;
	}
	else if(ptr < 0) //An object array?
	{
		int32_t objptr = -ptr;
		auto it = objectRAM.find(objptr);
		if(it == objectRAM.end())
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}
		else aptr = &(it->second);
	}
	else if(ptr >= NUM_ZSCRIPT_ARRAYS) //Then it's a global
	{
		dword gptr = ptr - NUM_ZSCRIPT_ARRAYS;
		
		if(gptr > game->globalRAM.size())
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}
		else aptr = &(game->globalRAM[gptr]);
	}
	else
	{
		if(!localRAM[ptr].Valid())
		{
			aptr = &INVALIDARRAY;
			_invalid = true;
		}
		else aptr = &(localRAM[ptr]);
	}
	if (_invalid)
	{
		scripting_log_error_with_context("Invalid pointer used as array: {}", ptr);
	}
}

ArrayManager::ArrayManager(int32_t ptr) : ArrayManager(ptr,can_neg_array){}

int32_t ArrayManager::get(int32_t indx) const
{
	if(_invalid)
		return get_qr(qr_OLD_SCRIPTS_ARRAYS_NON_ZERO_DEFAULT_VALUE) ? -10000 : 0;
	if(aptr)
	{
		int32_t sz = size();
		if(BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em
			return (*aptr)[indx];
		}
	}
	else //internal special array
	{
		if (ZScriptVersion::gc_arrays())
			return zasm_array_get(internal_array_id.zasm_var, internal_array_id.ref, indx);

		int32_t sz = size();
		if(sz >= 0 && BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em
			return legacy_get_int_arr(legacy_internal_id, indx);
		}
	}

	return get_qr(qr_OLD_SCRIPTS_ARRAYS_NON_ZERO_DEFAULT_VALUE) ? -10000 : 0;
}

void ArrayManager::set(int32_t indx, int32_t val, script_object_type type)
{
	if(_invalid) return;

	if(aptr)
	{
		int32_t sz = size();
		if(BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em

			bool is_object = type != script_object_type::none;
			if (is_object || aptr->HoldsObjects())
				script_object_ref_inc(val);

			if (holds_object(indx))
			{
				int id = (*aptr)[indx];
				script_object_ref_dec(id);
			}

			(*aptr)[indx] = val;

			if (aptr->MaybeHoldsObjects())
				script_array_object->set_type_in_untyped_array(indx, type);
		}
	}
	else //internal special array
	{
		if (ZScriptVersion::gc_arrays())
		{
			zasm_array_set(internal_array_id.zasm_var, internal_array_id.ref, indx, val, type);
			return;
		}

		int32_t sz = size();
		if(sz >= 0 && BC::checkUserArrayIndex(indx, sz, negAccess) == SH::_NoError)
		{
			if(indx < 0)
				indx += sz; //[-1] becomes [size-1] -Em
			legacy_set_int_arr(legacy_internal_id, indx, val);
		}
	}
}

int32_t ArrayManager::size() const
{
	if(_invalid) return -1;
	if(aptr)
		return aptr->Size();
	else // Internal special
	{
		if (ZScriptVersion::gc_arrays())
			return zasm_array_size(internal_array_id.zasm_var, internal_array_id.ref);

		int32_t sz = legacy_sz_int_arr(legacy_internal_id);
		if(sz < 0)
			return -1;
		return sz;
	}
}

bool ArrayManager::resize(size_t newsize)
{
	if(_invalid) return false;
	if(!aptr)
	{
		log_invalid_operation();
		return false;
	}
	if (aptr->HoldsObjects())
	{
		for (int i = newsize; i < aptr->Size(); i++)
		{
			auto id = (*aptr)[i];
			script_object_ref_dec(id);
		}
	}
	aptr->Resize(newsize);
	return true;
}

bool ArrayManager::resize_min(size_t newsize)
{
	if(size() >= newsize)
		return true;
	return resize(newsize);
}

bool ArrayManager::holds_object(int indx)
{
	if (_invalid) return false;

	if (aptr && script_array_object)
		return aptr->HoldsObjects() || (aptr->MaybeHoldsObjects() && script_array_object->holds_untyped_object(indx));

	// For compat.
	if (aptr)
		return aptr->HoldsObjects();

	if (script_array_object && script_array_object->internal_id.has_value())
		return script_array_object->arr.HoldsObjects() || (script_array_object->arr.MaybeHoldsObjects() && script_array_object->holds_untyped_object(indx));

	return false;
}

bool ArrayManager::can_resize()
{
	if(_invalid || !aptr)
		return false;
	return true;
}

bool ArrayManager::push(int32_t val, int indx)
{
	if(_invalid) return false;
	if(!aptr)
	{
		log_invalid_operation();
		return false;
	}
	if(aptr->Size() == MAX_ZC_ARRAY_SIZE)
		return false;
	aptr->Push(val,indx);
	if (aptr->HoldsObjects())
		script_object_ref_inc(val);
	return true;
}

int32_t ArrayManager::pop(int indx)
{
	if(_invalid) return -10000;
	if(!aptr)
	{
		log_invalid_operation();
		return -10000;
	}

	if (aptr->Empty())
	{
		scripting_log_error_with_context("Array had nothing to Pop!");
		return -10000;
	}

	int32_t val = aptr->Pop(indx);
	if (aptr->HoldsObjects())
		script_object_ref_dec(val);
	return val;
}

std::string ArrayManager::asString(std::function<char const*(int32_t)> formatter, const size_t& limit) const
{
	if(_invalid) return "{ INVALID ARRAY }";
	std::ostringstream oss;
	oss << "{ ";
	size_t s = size();
	bool overflow = limit < s;
	if(overflow)
		s = limit;
	
	for(auto q = 0; q < s; ++q)
	{
		oss << formatter(get(q));
		if (q + 1 < s)
			oss << ", ";
	}
	if (overflow)
		oss << ", ...";
	oss << " }";
	return oss.str();
}

void ArrayManager::log_invalid_operation() const
{
	if (internal_array_id.zasm_var)
	{
		current_zasm_extra_context = scripting_get_zasm_register_context_string(internal_array_id.zasm_var);
		scripting_log_error_with_context("Internal array not valid for this operation");
	}
	else
	{
		scripting_log_error_with_context("Internal array '{}' not valid for this operation", legacy_internal_id);
	}
}
