#include "zc/scripting/types/weapondata.h"
#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t weapondata_get_register(int32_t reg)
{
	int32_t ret = 0;
	weapon_data* data = checkWeaponData(GET_REF(weapdataref), reg == WEAPDATA_VALID);

	switch (reg)
	{
		case WEAPDATA_LIGHT_SHAPE:
			if (data)
				ret = 10000 * data->glow_shape;
			break;
		case WEAPDATA_SIZE_BITFLAGS:
			if (data)
				ret = 10000 * data->override_flags;
			break;
		case WEAPDATA_HIT_OFFSET_X:
			if (data)
				ret = 10000 * data->hxofs;
			break;
		case WEAPDATA_HIT_OFFSET_Y:
			if (data)
				ret = 10000 * data->hyofs;
			break;
		case WEAPDATA_HIT_WIDTH:
			if (data)
				ret = 10000 * data->hxsz;
			break;
		case WEAPDATA_HIT_HEIGHT:
			if (data)
				ret = 10000 * data->hysz;
			break;
		case WEAPDATA_HIT_ZHEIGHT:
			if (data)
				ret = 10000 * data->hzsz;
			break;
		case WEAPDATA_X_OFFSET:
			if (data)
				ret = 10000 * data->xofs;
			break;
		case WEAPDATA_Y_OFFSET:
			if (data)
				ret = 10000 * data->yofs;
			break;
		case WEAPDATA_TILE_W:
			if (data)
				ret = 10000 * data->tilew;
			break;
		case WEAPDATA_TILE_H:
			if (data)
				ret = 10000 * data->tileh;
			break;
		case WEAPDATA_STEP:
			if (data)
				ret = data->step.getZLong() * 100;
			break;
		case WEAPDATA_BOUNCE_MULT:
			if (data)
				ret = data->bounce_mult.getZLong();
			break;
		case WEAPDATA_BOUNCE_ADD:
			if (data)
				ret = data->bounce_add.getZLong();
			break;
		case WEAPDATA_UNBLOCKABLE:
			if (data)
				ret = 10000 * data->unblockable;
			break;
		case WEAPDATA_TIMEOUT:
			if (data)
				ret = 10000 * data->timeout;
			break;
		case WEAPDATA_USEWEAPON:
			if (data)
				ret = 10000 * data->imitate_weapon;
			break;
		case WEAPDATA_USEDEFENCE:
			if (data)
				ret = 10000 * data->default_defense;
			break;
		case WEAPDATA_LIFTLEVEL:
			if (data)
				ret = 10000 * data->lift_level;
			break;
		case WEAPDATA_LIFTTIME:
			if (data)
				ret = 10000 * data->lift_time;
			break;
		case WEAPDATA_LIFTHEIGHT:
			if (data)
				ret = data->lift_height.getZLong();
			break;
		case WEAPDATA_SCRIPT:
			if (data)
				ret = 10000 * data->scrconfig.script;
			break;
		case WEAPDATA_PIERCE:
			if (data)
				ret = 10000 * data->pierce_count;
			break;
		case WEAPDATA_VIEWPORT_SUSPEND_RANGE:
			if (data)
				ret = 10000 * data->viewport_suspend_range;
			break;
		case WEAPDATA_VIEWPORT_DESPAWN_RANGE:
			if (data)
				ret = 10000 * data->viewport_despawn_range;
			break;
		case WEAPDATA_VALID:
			ret = data ? 10000 : 0;
			break;

		default: NOTREACHED();
	}

	return ret;
}

void weapondata_set_register(int32_t reg, int32_t value)
{
	weapon_data* data = checkWeaponData(GET_REF(weapdataref));

	switch (reg)
	{
		case WEAPDATA_LIGHT_SHAPE:
			if (data)
				data->glow_shape = vbound(value / 10000, 0, 255);
			break;
		case WEAPDATA_SIZE_BITFLAGS:
			if (data)
				data->override_flags = value / 10000;
			break;
		case WEAPDATA_HIT_OFFSET_X:
			if (data)
				data->hxofs = value / 10000;
			break;
		case WEAPDATA_HIT_OFFSET_Y:
			if (data)
				data->hyofs = value / 10000;
			break;
		case WEAPDATA_HIT_WIDTH:
			if (data)
				data->hxsz = value / 10000;
			break;
		case WEAPDATA_HIT_HEIGHT:
			if (data)
				data->hysz = value / 10000;
			break;
		case WEAPDATA_HIT_ZHEIGHT:
			if (data)
				data->hzsz = value / 10000;
			break;
		case WEAPDATA_X_OFFSET:
			if (data)
				data->xofs = value / 10000;
			break;
		case WEAPDATA_Y_OFFSET:
			if (data)
				data->yofs = value / 10000;
			break;
		case WEAPDATA_TILE_W:
			if (data)
			{
				int width = value / 10000;
				if (BC::checkBounds(width, 0, 20) == SH::_NoError)
					data->tilew = width;
			}
			break;
		case WEAPDATA_TILE_H:
			if (data)
			{
				int height = value / 10000;
				if (BC::checkBounds(height, 0, 20) == SH::_NoError)
					data->tileh = height;
			}
			break;
		case WEAPDATA_STEP:
			if (data)
				data->step = zslongToFix(value / 100);
			break;
		case WEAPDATA_BOUNCE_MULT:
			if (data)
				data->bounce_mult = zslongToFix(value);
			break;
		case WEAPDATA_BOUNCE_ADD:
			if (data)
				data->bounce_add = zslongToFix(value);
			break;
		case WEAPDATA_UNBLOCKABLE:
			if (data)
				data->unblockable = (value / 10000) & WPNUNB_ALL;
			break;
		case WEAPDATA_TIMEOUT:
			if (data)
				data->timeout = zc_max(value/10000, 0);
			break;
		case WEAPDATA_USEWEAPON:
			if (data)
				data->imitate_weapon = vbound(value / 10000, 0, 255);
			break;
		case WEAPDATA_USEDEFENCE:
			if (data)
				data->default_defense = vbound(value / 10000, 0, 255);
			break;
		case WEAPDATA_LIFTLEVEL:
			if (data)
				data->lift_level = vbound(value / 10000, 0, 255);
			break;
		case WEAPDATA_LIFTTIME:
			if (data)
				data->lift_time = vbound(value / 10000, 0, 255);
			break;
		case WEAPDATA_LIFTHEIGHT:
			if (data)
				data->lift_height = zslongToFix(value);
			break;
		case WEAPDATA_SCRIPT:
			if (data)
			{
				data->scrconfig.script = vbound(value/10000,0,NUMSCRIPTWEAPONS-1);
				if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
					data->scrconfig.run_args.fill(0);
				data->scrconfig.inst_init.clear();;
			}
			break;
		case WEAPDATA_PIERCE:
			if (data)
				data->pierce_count = vbound(value/10000,-1,32767);
			break;
		case WEAPDATA_VIEWPORT_SUSPEND_RANGE:
			if (data)
				data->viewport_suspend_range = value / 10000;
			break;
		case WEAPDATA_VIEWPORT_DESPAWN_RANGE:
			if (data)
				data->viewport_despawn_range = value / 10000;
			break;
		case WEAPDATA_VALID:
			break;
		
		default: NOTREACHED();
	}
}

std::optional<int32_t> weapondata_run_command(word command)
{
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	switch (command)
	{
		case CREATEWEAPDATA:
		{
			SET_D(rEXP1, FFCore.get_new_weapondata(wdata_type::script, 0));
			break;
		}
		case WEAPDATA_COPY:
		{
			if (weapon_data* src = checkWeaponData(GET_REF(weapdataref)))
				if (weapon_data* dest = checkWeaponData(get_register(sarg1)))
					*dest = *src;
			break;
		}
		case WEAPDATA_APPLY:
		{
			if (weapon_data* data = checkWeaponData(GET_REF(weapdataref)))
				if (weapon* w = checkWpn(get_register(sarg1)))
					w->load_weap_data(*data);
			break;
		}

		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}

// weapondata arrays.

static ArrayRegistrar WEAPDATA_MISC_FLAGS_registrar(WEAPDATA_MISC_FLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<weapon_data, &weapon_data::flags, 2> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar WEAPDATA_MOVE_FLAGS_registrar(WEAPDATA_MOVE_FLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<weapon_data, &weapon_data::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar WEAPDATA_WEAP_FLAGS_registrar(WEAPDATA_WEAP_FLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<weapon_data, &weapon_data::wflags, WFLAG_MAX> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar WEAPDATA_BURN_SPRITES_registrar(WEAPDATA_BURN_SPRITES, []{
	static ScriptingArray_ObjectMemberCArray<weapon_data, &weapon_data::burnsprs> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundWord);
	return &impl;
}());

static ArrayRegistrar WEAPDATA_BURN_LIGHTRAD_registrar(WEAPDATA_BURN_LIGHTRAD, []{
	static ScriptingArray_ObjectMemberCArray<weapon_data, &weapon_data::light_rads> impl;
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vboundByte);
	return &impl;
}());

static ArrayRegistrar WEAPDATA_BURN_LIGHTOFFSET_registrar(WEAPDATA_BURN_LIGHTOFFSET, []{
	static ScriptingArray_ObjectMemberCArray<weapon_data, &weapon_data::light_offsets> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar WEAPDATA_INITD_registrar(WEAPDATA_INITD, []{
	static ScriptingArray_ObjectSubMemberContainer<weapon_data, &weapon_data::scrconfig, &script_config::run_args> impl;
	impl.setMul10000(false);
	return &impl;
}());

