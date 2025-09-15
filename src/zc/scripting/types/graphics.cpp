#include "zc/scripting/arrays.h"

// Graphics arrays.

static ArrayRegistrar IS8BITTILE_registrar(IS8BITTILE, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return NEWMAXTILES; },
		[](int, int index) -> bool {
			return FFCore.Is8BitTile(index);
		},
		[](int, int, bool) {
			return false;
		}
	);
	impl.setMul10000(true);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar ISBLANKTILE_registrar(ISBLANKTILE, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return NEWMAXTILES; },
		[](int, int index) -> bool {
			return FFCore.IsBlankTile(index);
		},
		[](int, int, bool) {
			return false;
		}
	);
	impl.setMul10000(true);
	impl.readOnly();
	return &impl;
}());
