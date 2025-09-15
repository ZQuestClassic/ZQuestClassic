#include "zc/scripting/arrays.h"

// subscreenpage arrays.

static ArrayRegistrar SUBPGWIDGETS_registrar(SUBPGWIDGETS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref){
			if (SubscrPage* pg = checkSubPage(ref))
				return (int)pg->size();
			return 0;
		},
		[](int ref, int index){
			if (checkSubPage(ref))
			{
				auto [sb, ty, pg, _ind] = from_subref(ref);
				return (int)get_subref(sb, ty, pg, index);
			}

			return 0;
		},
		[](int, int index, int value){ return false; }
	);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());
