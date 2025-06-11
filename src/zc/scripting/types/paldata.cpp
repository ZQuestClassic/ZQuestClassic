#include "zc/scripting/arrays.h"

// paldata arrays.

static std::optional<int> value_transform_rgb(int value)
{
	if (value < 0 || value > scripting_max_color_val)
	{
		scripting_log_error_with_context("RGB value ({}) is out of range. RGB values range from 0 - {}.", value, scripting_max_color_val);
		return vbound(value, 0, scripting_max_color_val);
	}

	return value;
}

static ArrayRegistrar PALDATAR_registrar(PALDATAR, []{
	static ScriptingArray_ObjectComputed<user_paldata, int> impl(
		[](user_paldata* pd){ return PALDATA_NUM_COLORS; },
		[](user_paldata* pd, int index) -> int {
			return FFCore.do_paldata_getrgb(pd, index, 0);
		},
		[](user_paldata* pd, int index, int value){
			FFCore.do_paldata_setrgb(pd, index, value, 0);
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(value_transform_rgb);
	return &impl;
}());

static ArrayRegistrar PALDATAG_registrar(PALDATAG, []{
	static ScriptingArray_ObjectComputed<user_paldata, int> impl(
		[](user_paldata* pd){ return PALDATA_NUM_COLORS; },
		[](user_paldata* pd, int index) -> int {
			return FFCore.do_paldata_getrgb(pd, index, 1);
		},
		[](user_paldata* pd, int index, int value){
			FFCore.do_paldata_setrgb(pd, index, value, 1);
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(value_transform_rgb);
	return &impl;
}());

static ArrayRegistrar PALDATAB_registrar(PALDATAB, []{
	static ScriptingArray_ObjectComputed<user_paldata, int> impl(
		[](user_paldata* pd){ return PALDATA_NUM_COLORS; },
		[](user_paldata* pd, int index) -> int {
			return FFCore.do_paldata_getrgb(pd, index, 2);
		},
		[](user_paldata* pd, int index, int value){
			FFCore.do_paldata_setrgb(pd, index, value, 2);
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(value_transform_rgb);
	return &impl;
}());

static ArrayRegistrar PALDATACOLOR_registrar(PALDATACOLOR, []{
	static ScriptingArray_ObjectComputed<user_paldata, int> impl(
		[](user_paldata* pd){ return PALDATA_NUM_COLORS; },
		[](user_paldata* pd, int index) -> int {
			if (get_bit(pd->colors_used, index))
			{
				RGB c = pd->colors[index];
				return (c.r << 16) | (c.g << 8) | c.b;
			}
			else
			{
				return -10000; // INVALID_COLOR
			}
		},
		[](user_paldata* pd, int index, int value){
			int32_t clri = value;
			RGB c = _RGB((clri >> 16) & 0xFF, (clri >> 8) & 0xFF, clri & 0xFF);
			pd->set_color(index, c);
		}
	);
	impl.setDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());
