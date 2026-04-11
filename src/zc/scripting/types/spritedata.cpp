#include "zc/scripting/types/spritedata.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t spritedata_get_register(int32_t reg)
{
	int32_t ret = 0;

	#define GET_SPRITEDATA_VAR_INT(member) \
	{ \
		if(unsigned(GET_REF(spritedataref)) > (MAXSPRITES-1) )    \
		{ \
			ret = -10000; \
			scripting_log_error_with_context("Invalid Sprite ID: {}", GET_REF(spritedataref)*10000); \
		} \
		else \
			ret = (sprite_data_buf.get(GET_REF(spritedataref)).member * 10000); \
	}

	switch (reg)
	{
		case SPRITEDATACSETS:
		{
			if (auto sd = checkSpriteData(GET_REF(spritedataref)); !sd)
				ret = -10000;
			else
				ret = ((sd->csets & 0xF) * 10000);
			break;
		}
		case SPRITEDATAFLCSET:
		{
			if (auto sd = checkSpriteData(GET_REF(spritedataref)); !sd)
				ret = -10000;
			else
				ret = (((sd->csets & 0xF0)>>4) * 10000);
			break;
		}
		case SPRITEDATAFRAMES: GET_SPRITEDATA_VAR_INT(frames) break;
		case SPRITEDATAID:
		{
			if(unsigned(GET_REF(spritedataref)) > (MAXSPRITES-1) )
			{
				ret = -10000;
				Z_scripterrlog("Invalid Sprite ID passed to spritedata->ID: %d\n", GET_REF(spritedataref));
				break;
			}
			ret = ri->spritedataref*10000;
			break;
		}

		case SPRITEDATAMISC: GET_SPRITEDATA_VAR_INT(misc) break;
		case SPRITEDATASPEED: GET_SPRITEDATA_VAR_INT(speed) break;
		case SPRITEDATATILE: GET_SPRITEDATA_VAR_INT(tile) break;
		case SPRITEDATATYPE: GET_SPRITEDATA_VAR_INT(type) break;

		default:
			NOTREACHED();
	}

	return ret;
}

void spritedata_set_register(int32_t reg, int32_t value)
{
	#define	SET_SPRITEDATA_VAR_INT(member, str) \
	{ \
		if(unsigned(GET_REF(spritedataref)) > (MAXSPRITES-1) ) \
		{ \
			Z_scripterrlog("Invalid Sprite ID passed to spritedata->%s: %d\n", str, GET_REF(spritedataref)); \
		} \
		else \
		{ \
			sprite_data_buf[GET_REF(spritedataref)].member = vbound((value / 10000),0,214747); \
		} \
	} \

	#define	SET_SPRITEDATA_VAR_BYTE(member, str) \
	{ \
		if(unsigned(GET_REF(spritedataref)) > (MAXSPRITES-1) ) \
		{ \
			Z_scripterrlog("Invalid Sprite ID passed to spritedata->%s: %d\n", str, GET_REF(spritedataref)); \
		} \
		else \
		{ \
			sprite_data_buf[GET_REF(spritedataref)].member = vbound((value / 10000),0,255); \
		} \
	} \

	switch (reg)
	{
		case SPRITEDATACSETS:
		{
			if(unsigned(GET_REF(spritedataref)) > (MAXSPRITES-1) )
			{
				Z_scripterrlog("Invalid Sprite ID passed to spritedata->CSet: %d\n", GET_REF(spritedataref));
			}
			else
			{
				sprite_data_buf[GET_REF(spritedataref)].csets &= 0xF0;
				sprite_data_buf[GET_REF(spritedataref)].csets |= vbound((value / 10000),0,15);
			}
			break;
		}
		case SPRITEDATAFLCSET:
		{
			if(unsigned(GET_REF(spritedataref)) > (MAXSPRITES-1) )
			{
				Z_scripterrlog("Invalid Sprite ID passed to spritedata->FlashCSet: %d\n", GET_REF(spritedataref));
			}
			else
			{
				sprite_data_buf[GET_REF(spritedataref)].csets &= 0x0F;
				sprite_data_buf[GET_REF(spritedataref)].csets |= vbound((value / 10000),0,15)<<4;
			}
			break;
		}
		case SPRITEDATAFRAMES: SET_SPRITEDATA_VAR_BYTE(frames, "Frames"); break;
		case SPRITEDATAMISC: SET_SPRITEDATA_VAR_BYTE(misc, "Misc"); break;
		case SPRITEDATASPEED: SET_SPRITEDATA_VAR_BYTE(speed, "Speed"); break;
		case SPRITEDATATILE: SET_SPRITEDATA_VAR_INT(tile, "Tile"); break;
		case SPRITEDATATYPE: SET_SPRITEDATA_VAR_BYTE(type, "Type"); break;

		default:
			NOTREACHED();
	}
}

// spritedata arrays.

static ArrayRegistrar SPRITEDATAFLAGS_registrar(SPRITEDATAFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<sprite_data, &sprite_data::misc, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());
