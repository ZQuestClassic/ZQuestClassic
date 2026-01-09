#include "zc/scripting/type_store.h"
#include "base/qst.h"
#include <string>

namespace {

enum class legacy_script_object_type
{
	none,
	bitmap,
	dir,
	file,
	paldata,
	rng,
	stack,
	websocket,
	class_object,
	array,
	untyped,
	last = untyped,
};

} // end namespace

TypeID type_store_lookup(const std::string& name)
{
	if (!zasm_debug_data.exists())
	{
		if (name == "array") return (int)legacy_script_object_type::array;
		if (name == "bitmap") return (int)legacy_script_object_type::bitmap;
		if (name == "dir") return (int)legacy_script_object_type::dir;
		if (name == "file") return (int)legacy_script_object_type::file;
		if (name == "object") return (int)legacy_script_object_type::class_object;
		if (name == "paldata") return (int)legacy_script_object_type::paldata;
		if (name == "rng") return (int)legacy_script_object_type::rng;
		if (name == "stack") return (int)legacy_script_object_type::stack;
		if (name == "untyped") return (int)legacy_script_object_type::untyped;
		if (name == "void") return (int)legacy_script_object_type::none;
		if (name == "websocket") return (int)legacy_script_object_type::websocket;

		return 0;
	}

	// for (const auto& type : zasm_debug_data.types)
	// {
	// 	if (type.)
	// }
	return 0;
}

bool type_store_is_array(TypeID type_id)
{
	return false; // TODO !
}

bool type_store_is_bitmap(TypeID type_id)
{
	return false; // TODO !
}

bool type_store_is_object(TypeID type_id)
{
	return false; // TODO !
}

bool type_store_is_restorable(TypeID type_id)
{
	if (!zasm_debug_data.exists())
		return type_id == (int)legacy_script_object_type::class_object || type_id == (int)legacy_script_object_type::array;

	// TODO !
	return false;
}

bool type_store_is_untyped(TypeID type_id)
{
	return false; // TODO !
}

bool type_store_is_valid(TypeID type_id)
{
	return false; // TODO !
}

