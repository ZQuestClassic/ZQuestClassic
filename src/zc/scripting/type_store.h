#ifndef ZC_SCRIPTING_TYPE_STORE_H_
#define ZC_SCRIPTING_TYPE_STORE_H_

#include "zasm/debug_data.h"
#include <string>

TypeID type_store_lookup(const std::string& name);

bool type_store_is_array(TypeID type_id);
bool type_store_is_bitmap(TypeID type_id);
bool type_store_is_object(TypeID type_id);
bool type_store_is_restorable(TypeID type_id);
bool type_store_is_untyped(TypeID type_id);
bool type_store_is_valid(TypeID type_id);

#endif
