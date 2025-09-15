#include "zc/scripting/arrays.h"

// spritedata arrays.

static ArrayRegistrar SPRITEDATAFLAGS_registrar(SPRITEDATAFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<wpndata, &wpndata::misc, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());
