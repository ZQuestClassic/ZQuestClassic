// Scripting arrays are declared by static array registrars that run at process start. For example:
//
//    static ArrayRegistrar GAMEBOTTLEST_registrar(GAMEBOTTLEST, []{
//        static ScriptingArray_ObjectMemberCArray<gamedata, &gamedata::bottleSlots> impl;
//        impl.setMul10000(true);
//        return &impl;
//    }());
//
// The above configures the GAMEBOTTLEST variable to map to the bottleSlots field of gamedata.
//
// static ArrayRegistrar IDATAWPNINITD_registrar(IDATAWPNINITD, []{
//     static ScriptingArray_ObjectSubMemberCArray<itemdata, &itemdata::weap_data, &weapon_data::initd> impl;
//     impl.compatSetDefaultValue(-10000);
//     impl.setMul10000(false);
//     return &impl;
// }());
//
// The above configures the IDATAWPNINITD variable to map to the initd field of weapon_data, inside the weap_data member of itemdata.
//
// Steps for defining new scripting arrays:
//
// 1. Locate the scripting/types files that corresponds to the bindings .zh file for the ZASM variable.
//
// 2. Copy the "static ArrayRegistrar" boilerplate from somewhere else, following the naming convention
//    of ZASM_VAR_registrar.
//
// 3. Determine which ScriptingArray interface fits best. These are all the options:
//
//	    ScriptingArray_GlobalCArray
//	    ScriptingArray_GlobalComputed
//	    ScriptingArray_ObjectMemberCArray
//	    ScriptingArray_ObjectSubMemberCArray
//	    ScriptingArray_ObjectMemberBitwiseFlags
//	    ScriptingArray_ObjectSubMemberBitwiseFlags
//	    ScriptingArray_ObjectMemberBitstring
//	    ScriptingArray_ObjectMemberContainer
//	    ScriptingArray_ObjectComputed
//
//   The "Global" interfaces pass the ref variable as an int. Many ZASM arrays are "global" in that they
//   do not need to resolve a context object (like `Game->FFRules[]`).
//
//   The "Object" interfaces take a type parameter for the scripting object that ref resolves for, and
//   handles getting the object and checking for its existence.
//
//   Some interfaces are very simple to use, such as: GlobalCArray, ObjectMemberCArray,
//   ObjectMemberBitwiseFlags, ObjectMemberBitstring, ObjectMemberContainer. All of these simply require a pointer to the struct
//   field as a type parameter (see `&gamedata::bottleSlots` in the first above example).
//
//   Some are similarly easy to use, such as: ObjectSubMemberCArray, ObjectSubMemberBitwiseFlags.
//   These require a pointer to the struct field, and a pointer to a field on THAT struct as well.
//   (see `&itemdata::weap_data` and `&weapon_data::initd` in the second above example)
//
//   The GlobalComputed and ObjectComputed interfaces are more involved. They take 3 lambdas each: one for
//   calculating the size, one for the getter, and one for the setter. For getters and setters, the index is
//   validated based on the size lambda, so no need to further validate in those lambas. For ObjectComputed,
//   the lambdas run only if the context object could be resolved, so no need to check it is valid. Lastly,
//   the values given to the getter/setter/setValueTransform lambdas have already been modified based on the
//   value of `impl.setMul10000(bool)` (see next section).
//
//   Sometimes the "Object" interfaces don't work for a given array, for example if the ref integer value is
//   needed; in these cases it's fine to use "GlobalComputed". Just be sure to validate the ref is valid in
//   each lambda.
//
// 4. Configure the array.
//
//    a) Configure the integer type (false for "long", true for "fixed-point int"): impl.setMul10000(false)
//
//    b) If the value is bounded or validated, use `impl.setValueTransform(...)` For example, to vbound between
//       0 and 255: `impl.setValueTransform(transforms::vboundByte)`. There's also `transforms::vbound<low, min>`
//       to use any bounds. To validate instead (disallowing values outside a range), use
//       `transforms::validate<low, max>`. Lastly, you can use any arbitary lambda function: return std::nullopt
//       to reject a value, and return an integer otherwise (optionally modifying it).
//
//    c) Configure `impl.setSideEffect(...)` to run some code after an array value is successfully set. Useful for an
//       array that otherwise fits the use case of the simpler interfaces (no need to reach for the more general
//       Global interface).
//
//    d) If read-only: `impl.setReadOnly()`

#include "zc/scripting/arrays.h"
#include "base/check.h"
#include "zc/ffscript.h"

#include <cstddef>

class ZasmArrayManager
{
public:
	void registerArray(int zasm_var, IScriptingArray* impl)
	{
		DCHECK(!m_arrays[zasm_var]);
		m_arrays[zasm_var] = impl;
	}

	bool isRegistered(int zasm_var)
	{
		return m_arrays[zasm_var];
	}

	size_t getSize(int zasm_var, int ref) const
	{
		if (auto impl = m_arrays[zasm_var])
			return impl->getSize(ref);

		return 0;
	}

	int getElement(int zasm_var, int ref, int index) const
	{
		if (auto impl = m_arrays[zasm_var])
			return impl->getElement(ref, index);

		return 0;
	}

	bool setElement(int zasm_var, int ref, int index, int value)
	{
		if (auto impl = m_arrays[zasm_var])
		{
			if (impl->readOnly())
			{
				scripting_log_error_with_context("Cannot modify read-only array");
				return false;
			}

			return impl->setElement(ref, index, value);
		}

		return false;
	}

private:
	IScriptingArray* m_arrays[NUMVARIABLES];
};

static ZasmArrayManager g_arrayManager;

ArrayRegistrar::ArrayRegistrar(int zasm_var, IScriptingArray* arrayImpl)
{
	g_arrayManager.registerArray(zasm_var, arrayImpl);
}

int zasm_array_size(int zasm_var, int ref)
{
	return g_arrayManager.getSize(zasm_var, ref);
}

int zasm_array_get(int zasm_var, int ref, int index)
{
	return g_arrayManager.getElement(zasm_var, ref, index);
}

bool zasm_array_set(int zasm_var, int ref, int index, int value)
{
	return g_arrayManager.setElement(zasm_var, ref, index, value);
}

bool zasm_array_supports(int zasm_var)
{
	return g_arrayManager.isRegistered(zasm_var);
}

void zasm_array_register(int zasm_var, IScriptingArray* impl)
{
	g_arrayManager.registerArray(zasm_var, impl);
}
