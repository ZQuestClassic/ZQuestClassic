#include "zc/scripting/types/graphics.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t graphics_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case MAXDRAWS:
			ret = MAX_SCRIPT_DRAWING_COMMANDS * 10000;
			break;
		case NUMDRAWS:
			ret = script_drawing_commands.Count() * 10000;
			//ret = FFCore.numscriptdraws * 10000; // This isn't updated until end of frame, making it useless!
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void graphics_set_register(int32_t reg, [[maybe_unused]] int32_t value)
{
	switch (reg)
	{
		case MAXDRAWS: break;
		case NUMDRAWS: break;

		default:
			NOTREACHED();
	}
}

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
