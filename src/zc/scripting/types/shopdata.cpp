#include "zc/scripting/types/shopdata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/zasm_check.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t shopdata_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case SHOPDATATYPE:
		{
			int32_t ref = ri->shopdataref(); 
			if ( ref > NUMINFOSHOPS || ref < 0 ) ret = 0;
			else ret = ( ( ref <= NUMSHOPS ) ? 10000 : 20000 ); 
			break;
		}

		default:
			ZASM_NOTREACHED();
	}

	return ret;
}

void shopdata_set_register([[maybe_unused]] int32_t reg, [[maybe_unused]] int32_t value)
{
	ZASM_NOTREACHED();
}

// shopdata arrays.

// A shopdata pointer is a shop (0 to NUMSHOPS-1) or an info shop (NUMSHOPS to NUMINFOSHOPS). Its
// arrays have no elements for any other value, so their getters and setters never see one.
static size_t shopdata_array_size(int ref)
{
	if (ref < 0 || ref > NUMINFOSHOPS)
	{
		scripting_log_error_with_context("Invalid shopdata pointer: {}", ref);
		return 0;
	}
	return 3;
}

static ArrayRegistrar SHOPDATAHASITEM_registrar(SHOPDATAHASITEM, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		shopdata_array_size,
		[](int ref, int index) -> bool {
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS ); 
			if (isInfo)
			{ 
				scripting_log_error_with_context("Attempted to load 'hasitem' from an infoshop, using shop ID: {}", ref); 
				return -1;
			}
			else 
				return QMisc.shop[ref].hasitem[index];
		},
		[](int ref, int index, bool value){
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS );
			if (isInfo)
			{
				scripting_log_error_with_context("Attempted to write 'hasitem' to an infoshop, using shop ID: {}", ref);
				return false;
			}
			else 
				QMisc.shop[ref].hasitem[index] = value;

			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SHOPDATAITEM_registrar(SHOPDATAITEM, []{
	static ScriptingArray_GlobalComputed<int> impl(
		shopdata_array_size,
		[](int ref, int index) -> int {
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS ); 
			if (isInfo)
			{ 
				scripting_log_error_with_context("Attempted to load 'item' from an infoshop, using shop ID: {}", ref); 
				return -1;
			}
			else 
				return (int)QMisc.shop[ref].item[index];
		},
		[](int ref, int index, int value){
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS );
			if (isInfo)
			{
				scripting_log_error_with_context("Attempted to write 'item' to an infoshop, using shop ID: {}", ref);
				return false;
			}
			else 
				QMisc.shop[ref].item[index] = value;

			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SHOPDATAPRICE_registrar(SHOPDATAPRICE, []{
	static ScriptingArray_GlobalComputed<int> impl(
		shopdata_array_size,
		[](int ref, int index) -> int {
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS ); 
			if (isInfo)
				return QMisc.info[ref - NUMSHOPS].price[index];
			else 
				return QMisc.shop[ref].price[index];
		},
		[](int ref, int index, int value){
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS );
			if (isInfo)
				QMisc.info[ref - NUMSHOPS].price[index] = value;
			else 
				QMisc.shop[ref].price[index] = value;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar SHOPDATASTRING_registrar(SHOPDATASTRING, []{
	static ScriptingArray_GlobalComputed<int> impl(
		shopdata_array_size,
		[](int ref, int index) -> int {
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS ); 
			if (isInfo)
				return QMisc.info[ref - NUMSHOPS].str[index];
			else 
				return QMisc.shop[ref].str[index];
		},
		[](int ref, int index, int value){
			bool isInfo = ( ref >= NUMSHOPS && ref <= NUMINFOSHOPS );
			if (isInfo)
				QMisc.info[ref - NUMSHOPS].str[index] = value;
			else 
				QMisc.shop[ref].str[index] = value;
			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 32767>);
	return &impl;
}());
