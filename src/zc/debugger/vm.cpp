#include "zc/debugger/vm.h"
#include "base/expected.h"
#include "base/qst.h"
#include "base/zdefs.h"
#include "user_object.h"
#include "zasm/debug_data.h"
#include "zasm/defines.h"
#include "zasm/pc.h"
#include "zasm/table.h"
#include "zc/ffscript.h"
#include "zc/scripting/script_object.h"
#include <cstdint>
#include <optional>

namespace {

const DebugType* getDebugTypeOfUntypedArrayElement(script_object_type engine_type, int raw_value, const DebugType* source_array_type)
{
	if (engine_type == script_object_type::none)
		return &BasicTypes[TYPE_UNTYPED];

	auto script_obj = get_script_object(raw_value);
	if (!script_obj)
		return &BasicTypes[TYPE_UNTYPED];

	if (engine_type == script_object_type::class_object)
	{
		if (auto obj = dynamic_cast<user_object*>(script_obj))
		{
			const DebugScope* scope = zasm_debug_data.resolveClassScope(obj->ctor_pc);
			return zasm_debug_data.getType(scope);
		}
	}
	else if (engine_type == script_object_type::array)
	{
		// TODO: this returns the type for untyped[]. Could probably do better.
		return source_array_type;
	}
	else if (engine_type == script_object_type::bitmap)
	{
		std::string name;
		switch (engine_type)
		{
			case script_object_type::bitmap: name = "bitmap"; break;
			case script_object_type::dir: name = "directory"; break;
			case script_object_type::file: name = "file"; break;
			case script_object_type::paldata: name = "paldata"; break;
			case script_object_type::rng: name = "randgen"; break;
			case script_object_type::stack: name = "stack"; break;
			case script_object_type::websocket: name = "websocket"; break;
			default: return &BasicTypes[TYPE_UNTYPED];
		}

		for (auto& type : zasm_debug_data.types)
		{
			if (type.tag != TYPE_CLASS)
				continue;

			if (zasm_debug_data.getTypeName(&type) == name)
				return &type;
		}
	}

	return &BasicTypes[TYPE_UNTYPED];
}

const script_object_type getEngineTypeForDebugType(const DebugType* type)
{
	if (type->isArray(zasm_debug_data))
		return script_object_type::array;

	std::string name = zasm_debug_data.getTypeName(type->asNonConst(zasm_debug_data));
	if (name == "bitmap") return script_object_type::bitmap;
	if (name == "directory") return script_object_type::dir;
	if (name == "file") return script_object_type::file;
	if (name == "paldata") return script_object_type::paldata;
	if (name == "randgen") return script_object_type::rng;
	if (name == "stack") return script_object_type::stack;
	if (name == "websocket") return script_object_type::websocket;

	return script_object_type::none;
}

} // end namespace

int32_t VM::readStack(int32_t offset)
{
	auto ri = &current_data->ref;
	int index = ri->debugger_stack_frames.size() - current_frame_index - 1;
	if (index < 0 || index >= ri->debugger_stack_frames.size())
	{
		DCHECK(false);
		return 0;
	}

	int sframe = ri->debugger_stack_frames.at(index).stack_frame_base;
	uint32_t sp = sframe + offset;
	if (sp >= MAX_STACK_SIZE)
		return -10000;

	return current_data->stack[sp];
}

int32_t VM::readGlobal(int32_t idx)
{
	return game->global_d[idx];
}

int32_t VM::readRegister(int32_t id)
{
	if (id == CLASS_THISKEY)
		return getThisPointer();

	script_is_within_debugger_vm = true;
	int result = get_register(id);
	script_is_within_debugger_vm = false;
	return result;
}

std::optional<DebugValue> VM::readObjectMember(DebugValue object, const DebugSymbol* sym)
{
	std::optional<int> raw_value;

	if (sym->storage == LOC_REGISTER)
	{
		auto type = zasm_debug_data.getType(sym->type_id)->asNonConst(zasm_debug_data);
		auto scope = zasm_debug_data.scopes[type->extra];
		if (type->isArray(zasm_debug_data))
		{
			raw_value = find_or_create_internal_script_array({sym->offset, object.raw_value})->id;
		}
		else
		{
			auto refvar = get_register_ref_dependency(sym->offset);
			if (refvar.has_value())
			{
				int prev_val = get_register(refvar.value());
				set_register(refvar.value(), object.raw_value);
				raw_value = get_register(sym->offset);
				set_register(refvar.value(), prev_val);
			}
			else
			{
				raw_value = get_register(sym->offset);
			}
		}
	}
	else if (sym->storage == LOC_CLASS)
	{
		if (auto obj = get_script_object(object.raw_value))
		{
			if (auto user_obj = dynamic_cast<user_object*>(obj))
			{
				if (sym->offset < user_obj->data.size())
					raw_value = user_obj->data[sym->offset];
			}
		}
	}

	if (!raw_value)
		return std::nullopt;

	DebugValue value{};
	value.is_lvalue = true;
	value.raw_value = raw_value.value();
	value.type = zasm_debug_data.getType(sym->type_id);
	return value;
}

std::optional<std::vector<DebugValue>> VM::readArray(DebugValue array)
{
	ArrayManager am(array.raw_value);
	if (am.invalid()) return std::nullopt;

	const DebugType* default_elem_type = zasm_debug_data.getType(array.type->extra);

	script_is_within_debugger_vm = true;
	std::vector<int32_t> data = am.data();
	script_is_within_debugger_vm = false;

	std::vector<DebugValue> result;
	for (int i = 0; i < data.size(); i++)
	{
		// Elements within untyped arrays have dynamic typing.
		if (default_elem_type->isUntyped(zasm_debug_data) && am.script_array())
		{
			script_object_type type = (script_object_type)am.script_array()->get_type_in_untyped_array(i);
			int id = data[i];
			const DebugType* elem_type = getDebugTypeOfUntypedArrayElement(type, id, array.type);
			// This type shouldn't ever be null, but just incase...
			if (elem_type)
			{
				result.push_back(DebugValue{id, elem_type});
				continue;
			}
		}

		result.push_back(DebugValue{data[i], default_elem_type});
	}

	return result;
}

std::optional<DebugValue> VM::readArrayElement(DebugValue array, int index)
{
	ArrayManager am(array.raw_value);
	if (am.invalid()) return std::nullopt;
	if (am.size() <= index) return std::nullopt;

	if (index < 0)
	{
		index = am.normalize_index(index);
		if (index < 0) return std::nullopt;
	}

	script_is_within_debugger_vm = true;
	int value = am.get(index);
	script_is_within_debugger_vm = false;

	// Elements within untyped arrays have dynamic typing.
	const DebugType* default_elem_type = zasm_debug_data.getType(array.type->extra);
	if (default_elem_type->isUntyped(zasm_debug_data) && am.script_array())
	{
		script_object_type type = (script_object_type)am.script_array()->get_type_in_untyped_array(index);
		int id = value;
		const DebugType* elem_type = getDebugTypeOfUntypedArrayElement(type, id, array.type);
		return DebugValue{id, elem_type};
	}

	return DebugValue{value, default_elem_type};
}

std::optional<std::string> VM::readString(int32_t string_ptr)
{
	script_is_within_debugger_vm = true;

	ArrayManager am(string_ptr);
	if (am.invalid())
	{
		script_is_within_debugger_vm = false;
		return std::nullopt;
	}

	std::string str;
	ArrayH::getString(string_ptr, str);
	script_is_within_debugger_vm = false;
	return str;
}

void VM::writeGlobal(int32_t offset, int32_t value)
{
	game->global_d[offset] = value;
}

void VM::writeStack(int32_t offset, int32_t value)
{
	auto ri = &current_data->ref;
	int index = ri->debugger_stack_frames.size() - current_frame_index - 1;
	if (index < 0 || index >= ri->debugger_stack_frames.size())
	{
		DCHECK(false);
		return;
	}

	int sframe = ri->debugger_stack_frames.at(index).stack_frame_base;
	uint32_t sp = sframe + offset;
	if (sp >= MAX_STACK_SIZE)
		return;

	current_data->stack[sp] = value;
}

void VM::writeRegister(int32_t offset, int32_t value)
{
	script_is_within_debugger_vm = true;
	set_register(offset, value);
	script_is_within_debugger_vm = false;
}

bool VM::writeObjectMember(DebugValue object, const DebugSymbol* sym, DebugValue value)
{
	if (sym->storage == LOC_REGISTER)
	{
		auto type = zasm_debug_data.getType(sym->type_id)->asNonConst(zasm_debug_data);
		auto scope = zasm_debug_data.scopes[type->extra];
		if (type->isArray(zasm_debug_data))
			return false;

		auto refvar = get_register_ref_dependency(sym->offset);
		if (refvar.has_value())
		{
			int prev_val = get_register(refvar.value());
			set_register(refvar.value(), object.raw_value);
			set_register(sym->offset, value.raw_value);
			set_register(refvar.value(), prev_val);
		}
		else
		{
			set_register(sym->offset, value.raw_value);
		}

		return true;
	}
	else if (sym->storage == LOC_CLASS)
	{
		if (auto obj = get_script_object(object.raw_value))
		{
			if (auto user_obj = dynamic_cast<user_object*>(obj))
			{
				if (sym->offset < user_obj->data.size())
				{
					user_obj->data[sym->offset] = value.raw_value;
					return true;
				}
			}
		}
	}

	return false;
}

bool VM::writeArrayElement(DebugValue array, int32_t index, DebugValue value)
{
	ArrayManager am(array.raw_value);
	if (am.invalid()) return false;
	if (am.size() <= index) return false;

	if (index < 0)
	{
		index = am.normalize_index(index);
		if (index < 0) return false;
	}

	script_is_within_debugger_vm = true;
	am.set(index, value.raw_value, getEngineTypeForDebugType(value.type));
	script_is_within_debugger_vm = false;

	return true;
}

// TODO: this only mostly works. Need to actually handle special cases like assigning a variable in
// an untyped array, or to an object member field.
void VM::decreaseObjectReference(DebugValue value, const DebugSymbol* sym)
{
	script_object_ref_dec(value.raw_value);
}

void VM::increaseObjectReference(DebugValue value, const DebugSymbol* sym)
{
	script_object_ref_inc(value.raw_value);
}

expected<int32_t, std::string> VM::executeSandboxed(pc_t start_pc, int this_zasm_var, int this_raw_value, const std::vector<int32_t>& args)
{
	extern refInfo *ri;
	extern int32_t(*stack)[MAX_STACK_SIZE];
	extern int32_t(*ret_stack)[MAX_CALL_FRAMES];

	refInfo* prev_ri = ri;
	int32_t(*prev_stack)[MAX_STACK_SIZE] = stack;
	int32_t(*prev_ret_stack)[MAX_CALL_FRAMES] = ret_stack;

	refInfo tmp_ri{};
	int32_t tmp_stack[MAX_STACK_SIZE] = {0};
	int32_t tmp_ret_stack[MAX_CALL_FRAMES] = {0};

	script_is_within_debugger_vm = true;

	tmp_ri.screenref = current_data->ref.screenref;

	ri = &tmp_ri;
	ri->pc = start_pc;
	ri->sp -= args.size();
	ri->retsp = 1;

	if (this_zasm_var)
		set_register(this_zasm_var, this_raw_value);

	stack = &tmp_stack;
	for (int i = 0; i < args.size(); i++)
		tmp_stack[MAX_STACK_SIZE - 1 - i] = args[i];

	ret_stack = &tmp_ret_stack;

	int script_engine_result = run_script_int();
	bool success = script_engine_result == RUNSCRIPT_OK;

	ri = prev_ri;
	stack = prev_stack;
	ret_stack = prev_ret_stack;
	script_is_within_debugger_vm = false;

	if (!success)
	{
		if (tmp_ri.waitframes)
			return make_unexpected("Cannot call Waitframe for expressions evaluated in the debugger");
		if (script_engine_result == RUNSCRIPT_INFINITE_LOOP)
			return make_unexpected("Detected possible infinite loop, aborting function call");
		if (tmp_ri.overflow)
			return make_unexpected("Function call overflowed");
		return make_unexpected(fmt::format("Function call failed for an unknown reason ({})", script_engine_result));
	}

	return tmp_ri.d[D(2)];
}

DebugValue VM::createArray(std::vector<int32_t> args, const DebugType* array_type)
{
	auto array = create_script_array();
	for (int arg : args)
		array->arr.Push(arg);
	return DebugValue{(int)array->id, array_type};
}

DebugValue VM::createString(const std::string& str)
{
	const DebugType* string_type = nullptr;
	for (auto& type : zasm_debug_data.types)
	{
		if (type.tag == TYPE_ARRAY && type.extra == TYPE_CHAR32)
		{
			string_type = &type;
			break;
		}
	}

	if (!string_type)
		return {};

	auto array = create_script_array();
	for (char arg : str)
		array->arr.Push(arg * 10000);
	array->arr.Push(0);

	return DebugValue{(int)array->id, string_type};
}

int32_t VM::getThisPointer()
{
	auto ri = &current_data->ref;
	int index = ri->debugger_stack_frames.size() - current_frame_index - 1;
	if (index < 0 || index >= ri->debugger_stack_frames.size())
	{
		DCHECK(false);
		return 0;
	}

	return ri->debugger_stack_frames.at(index).this_ptr;
}
