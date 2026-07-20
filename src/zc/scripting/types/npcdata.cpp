#include "zc/scripting/types/npcdata.h"

#include "base/check.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

namespace {

// TODO: replace with checkNPCData.
bool checkNPCDataRef()
{
	if( (unsigned) GET_REF(npcdataref) > (MAXNPCS-1) )
	{
		scripting_log_error_with_context("Invalid npcdata ID: {}", GET_REF(npcdataref));
		return false;
	}

	return true;
}

} // end namespace

int32_t npcdata_get_register(int32_t reg)
{
	int32_t ret = 0;
	guydata* nd = checkNPCDataRef() ? &guysbuf[GET_REF(npcdataref)] : nullptr;

	#define	GET_NPCDATA_VAR_INT32(member, str) \
	{ \
		if( !nd ) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (nd->member *10000); \
		} \
	} \

	#define	GET_NPCDATA_VAR_BYTE(member, str) \
	{ \
		if( !nd ) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (nd->member *10000); \
		} \
	} \
	
	#define	GET_NPCDATA_VAR_INT16(member, str) \
	{ \
		if( !nd ) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (nd->member *10000); \
		} \
	} \

	#define GET_NPCDATA_FLAG(member, str, indexbound) \
	{ \
		int32_t flag =  (value/10000);  \
		if( !nd ) \
		{ \
		} \
		else \
		{ \
			ret = (guysbuf[ID].member&flag) ? 10000 : 0); \
		} \
	} \

	switch (reg)
	{
		case NPCDATAANIM: GET_NPCDATA_VAR_INT16(anim, "Anim"); break;
		case NPCDATABGSFX: GET_NPCDATA_VAR_INT16(bgsfx, "BGSFX"); break;
		case NPCDATACSET: GET_NPCDATA_VAR_INT16(cset, "CSet"); break;
		case NPCDATADEATHSFX: GET_NPCDATA_VAR_INT16(deadsfx, "DeathSFX"); break;
		case NPCDATADROPSET: GET_NPCDATA_VAR_INT16(item_set, "Dropset"); break;
		case NPCDATAEANIM: GET_NPCDATA_VAR_INT16(e_anim, "ExAnim"); break;
		case NPCDATAEFRAMERATE: GET_NPCDATA_VAR_INT16(e_frate, "ExFramerate"); break;
		case NPCDATAEHEIGHT: GET_NPCDATA_VAR_BYTE(e_height, "ExHeight"); break;
		case NPCDATAETILE: GET_NPCDATA_VAR_INT32(e_tile, "ExTile"); break;
		case NPCDATAEWIDTH: GET_NPCDATA_VAR_BYTE(e_width, "ExWidth"); break;
		case NPCDATAFIRESFX: GET_NPCDATA_VAR_BYTE(firesfx, "WeaponSFX"); break;
		case NPCDATAFLAGS1:
		{
			if( !nd )
			{
				ret = -10000;
			}
			else
			{
				uint32_t value = nd->flags & 0xFFFFFFFFLL;
				ret = value * 10000;
			}
		}
		break;
		case NPCDATAFLAGS2:
		{
			if( !nd )
			{
				ret = -10000;
			}
			else
			{
				uint32_t value = (nd->flags >> 32) & 0xFFFFFFFFLL;
				ret = value * 10000;
			}
		}
		break;
		case NPCDATAFRAMERATE: GET_NPCDATA_VAR_INT16(frate, "Framerate"); break;
		case NPCDATAFROZENCSET: GET_NPCDATA_VAR_INT32(frozencset, "FrozenCSet"); break;
		case NPCDATAFROZENTILE: GET_NPCDATA_VAR_INT32(frozentile, "FrozenTile"); break;
		case NPCDATAHALT: GET_NPCDATA_VAR_INT16(hrate, "Haltrate"); break;
		case NPCDATAHEIGHT: GET_NPCDATA_VAR_BYTE(height, "Height"); break;
		case NPCDATAHITHEIGHT: GET_NPCDATA_VAR_INT32(hysz, "HitHeight"); break;
		case NPCDATAHITSFX: GET_NPCDATA_VAR_INT16(hitsfx, "HitSFX"); break;
		case NPCDATAHITWIDTH: GET_NPCDATA_VAR_INT32(hxsz, "HitWidth"); break;
		case NPCDATAHITZ: GET_NPCDATA_VAR_INT32(hzsz, "HitZHeight"); break;
		case NPCDATAHOMING: GET_NPCDATA_VAR_INT16(homing, "Homing"); break;
		case NPCDATAHP: GET_NPCDATA_VAR_INT16(hp, "HP"); break;
		case NPCDATAHUNGER: GET_NPCDATA_VAR_INT16(grumble, "Hunger"); break;
		case NPCDATAHXOFS: GET_NPCDATA_VAR_INT32(hxofs, "HitXOffset"); break;
		case NPCDATAHYOFS: GET_NPCDATA_VAR_INT32(hyofs, "HitYOffset"); break;
		case NPCDATARANDOM: GET_NPCDATA_VAR_INT16(rate, "Random"); break;
		case NPCDATASCRIPT: GET_NPCDATA_VAR_INT32(scrconfig.script, "Script"); break;
		case NPCDATASHEIGHT: GET_NPCDATA_VAR_BYTE(s_height, "SHeight"); break;
		case NPCDATASIZEFLAG: GET_NPCDATA_VAR_INT32(SIZEflags, "SizeFlags"); break;
		case NPCDATASTEP: GET_NPCDATA_VAR_INT16(step, "Step"); break;
		case NPCDATASTILE: GET_NPCDATA_VAR_BYTE(s_tile, "STile"); break;
		case NPCDATASWIDTH: GET_NPCDATA_VAR_BYTE(s_width, "SWidth"); break;
		case NPCDATATILE: GET_NPCDATA_VAR_BYTE(tile, "Tile"); break;
		case NPCDATATILEHEIGHT: GET_NPCDATA_VAR_INT32(tysz, "TileHeight"); break;
		case NPCDATATILEWIDTH: GET_NPCDATA_VAR_INT32(txsz, "TileWidth"); break;
		case NPCDATATOUCHDAMAGE: GET_NPCDATA_VAR_INT16(dp, "TouchDamage"); break;
		case NPCDATATYPE: GET_NPCDATA_VAR_INT16(type, "Family"); break;
		case NPCDATAWEAPON: GET_NPCDATA_VAR_INT16(weapon, "Weapon"); break;
		case NPCDATAWEAPONDAMAGE: GET_NPCDATA_VAR_INT16(wdp, "WeaponDamage"); break;
		case NPCDATAWEAPONSCRIPT: 
		{
			if( (unsigned) ri->npcdataref > (MAXNPCS-1) )
			{
				Z_scripterrlog("Invalid NPC ID passed to npcdata->WeaponScript: %d\n", ri->npcdataref);
				ret = -10000;
			}
			else ret = (nd->weap_data.scrconfig.script *10000);
			break;
		}
		case NPCDATAWIDTH: GET_NPCDATA_VAR_BYTE(width, "Width"); break;
		case NPCDATAWPNSPRITE: GET_NPCDATA_VAR_INT32(wpnsprite, "WeaponSprite"); break;
		case NPCDATAXOFS: GET_NPCDATA_VAR_INT32(xofs, "DrawXOffset"); break;
		case NPCDATAYOFS: GET_NPCDATA_VAR_INT32(yofs, "DrawYOffset"); break;
		case NPCDATAZOFS: GET_NPCDATA_VAR_INT32(zofs, "DrawZOffset"); break;
		case NPCDDEATHSPR:
		{
			if(!nd) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->DeathSprite: %d\n", ri->npcdataref);
				ret = -10000; 
			} 
			else 
			{
				ret = nd->spr_death * 10000;
			} 
			break;
		}
		case NPCDSHADOWSPR:
		{
			if(!nd) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->ShadowSprite: %d\n", ri->npcdataref);
				ret = -10000; 
			} 
			else 
			{
				ret = nd->spr_shadow * 10000;
			} 
			break;
		}
		case NPCDSPAWNSPR:
		{
			if(!nd) 
			{ 
				Z_scripterrlog("Invalid NPC ID passed to npcdata->SpawnSprite: %d\n", ri->npcdataref);
				ret = -10000; 
			} 
			else 
			{
				ret = nd->spr_spawn * 10000;
			} 
			break;
		}
		case NPCDATA_WEAPONDATA:
		{
			if(!nd)
			{
				Z_scripterrlog("Invalid NPC ID passed to npcdata->WeaponData: %d\n", ri->npcdataref);
				ret = -10000;
				break;
			}
			ret = FFCore.get_new_weapondata(wdata_type::npcdata, GET_REF(npcdataref));
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void npcdata_set_register(int32_t reg, int32_t value)
{
	guydata* nd = checkNPCData(GET_REF(npcdataref));
	if (!nd)
		return;

	#define	SET_NPCDATA_VAR_INT(member, str) \
	{ \
		nd->member = vbound((value / 10000),0,214747); \
	} \

	#define	SET_NPCDATA_VAR_WORD(member, str) \
	{ \
		nd->member = vbound((value / 10000),0,32767); \
	} \

	#define	SET_NPCDATA_VAR_ENUM(member, str) \
	{ \
		nd->member = (decltype(nd->member))vbound((value / 10000),0,32767); \
	} \

	#define	SET_NPCDATA_VAR_BYTE(member, str) \
	{ \
		nd->member = vbound((value / 10000),0,255); \
	} \

	#define SET_NPCDATA_FLAG(member, str) \
	{ \
		int32_t flag =  (value/10000);  \
		if ( flag ) \
		{ \
			nd->member|=flag; \
		} \
		else nd->member|= ~flag; \
	} \

	switch (reg)
	{
		case NPCDATAANIM: SET_NPCDATA_VAR_WORD(anim, "Anim"); break;
		case NPCDATABGSFX: SET_NPCDATA_VAR_WORD(bgsfx, "BGSFX"); break;
		case NPCDATACSET: SET_NPCDATA_VAR_WORD(cset, "CSet"); break;
		case NPCDATADEATHSFX: SET_NPCDATA_VAR_WORD(deadsfx, "DeathSFX"); break;
		case NPCDATADROPSET: SET_NPCDATA_VAR_WORD(item_set, "Dropset"); break;
		case NPCDATAEANIM: SET_NPCDATA_VAR_WORD(e_anim, "ExAnim"); break;
		case NPCDATAEFRAMERATE: SET_NPCDATA_VAR_WORD(e_frate, "ExFramerate"); break;
		case NPCDATAEHEIGHT: SET_NPCDATA_VAR_BYTE(e_height, "ExHeight"); break;
		case NPCDATAETILE: SET_NPCDATA_VAR_INT(e_tile, "ExTile"); break;
		case NPCDATAEWIDTH: SET_NPCDATA_VAR_BYTE(e_width, "ExWidth"); break;
		case NPCDATAFIRESFX: SET_NPCDATA_VAR_BYTE(firesfx, "WeaponSFX"); break;
		case NPCDATAFLAGS1: SET_NPCDATA_VAR_ENUM(flags, "Flags (deprecated)"); break;
		case NPCDATAFLAGS2: SET_NPCDATA_VAR_ENUM(flags, "Flags2 (deprecated)"); break;
		case NPCDATAFRAMERATE: SET_NPCDATA_VAR_WORD(frate, "Framerate"); break;
		case NPCDATAFROZENCSET: SET_NPCDATA_VAR_INT(frozencset, "FrozenCSet"); break;
		case NPCDATAFROZENTILE: SET_NPCDATA_VAR_INT(frozentile, "FrozenTile"); break;
		case NPCDATAHALT: SET_NPCDATA_VAR_WORD(hrate, "Haltrate"); break;
		case NPCDATAHEIGHT: SET_NPCDATA_VAR_BYTE(height, "Height"); break;
		case NPCDATAHITHEIGHT: SET_NPCDATA_VAR_INT(hysz, "HitHeight"); break;
		case NPCDATAHITSFX: SET_NPCDATA_VAR_WORD(hitsfx, "HitSFX"); break;
		case NPCDATAHITWIDTH: SET_NPCDATA_VAR_INT(hxsz, "HitWidth"); break;
		case NPCDATAHITZ: SET_NPCDATA_VAR_INT(hzsz, "HitZHeight"); break;
		case NPCDATAHOMING: SET_NPCDATA_VAR_WORD(homing, "Homing"); break;
		case NPCDATAHP: SET_NPCDATA_VAR_WORD(hp, "HP"); break;
		case NPCDATAHUNGER: SET_NPCDATA_VAR_WORD(grumble, "Hunger"); break;
		case NPCDATAHXOFS: SET_NPCDATA_VAR_INT(hxofs, "HitXOffset"); break;
		case NPCDATAHYOFS: SET_NPCDATA_VAR_INT(hyofs, "HitYOffset"); break;
		case NPCDATARANDOM: SET_NPCDATA_VAR_WORD(rate, "Random"); break;
		case NPCDATASCRIPT:
			nd->scrconfig.script = vbound((value / 10000),0,NUMSCRIPTGUYS-1);
			if ( get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				nd->scrconfig.run_args.fill(0);
			nd->scrconfig.inst_init.clear();
			break;
		case NPCDATASHEIGHT: SET_NPCDATA_VAR_BYTE(s_height, "SHeight"); break;
		case NPCDATASIZEFLAG: SET_NPCDATA_VAR_INT(SIZEflags, "SizeFlags"); break;
		case NPCDATASTEP: SET_NPCDATA_VAR_WORD(step, "Step"); break;
		case NPCDATASTILE: SET_NPCDATA_VAR_BYTE(s_tile, "STile"); break;
		case NPCDATASWIDTH: SET_NPCDATA_VAR_BYTE(s_width, "SWidth"); break;
		case NPCDATATILE: SET_NPCDATA_VAR_BYTE(tile, "Tile"); break;
		case NPCDATATILEHEIGHT: SET_NPCDATA_VAR_INT(tysz, "TileHeight"); break;
		case NPCDATATILEWIDTH: SET_NPCDATA_VAR_INT(txsz, "TileWidth"); break;
		case NPCDATATOUCHDAMAGE: SET_NPCDATA_VAR_WORD(dp, "TouchDamage"); break;
		case NPCDATATYPE: SET_NPCDATA_VAR_WORD(type, "Family"); break;
		case NPCDATAWEAPON: SET_NPCDATA_VAR_WORD(weapon, "Weapon"); break;
		case NPCDATAWEAPONDAMAGE: SET_NPCDATA_VAR_WORD(wdp, "WeaponDamage"); break;
		case NPCDATAWEAPONSCRIPT: 
		{
			nd->weap_data.scrconfig.script = vbound((value / 10000),0,NUMSCRIPTWEAPONS-1);
			if ( get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				nd->weap_data.scrconfig.run_args.fill(0);
			nd->weap_data.scrconfig.inst_init.clear();
			break;
		}
		case NPCDATAWIDTH: SET_NPCDATA_VAR_BYTE(width, "Width"); break;
		case NPCDATAWPNSPRITE: SET_NPCDATA_VAR_INT(wpnsprite, "WeaponSprite"); break;
		case NPCDATAXOFS: SET_NPCDATA_VAR_INT(xofs, "DrawXOffset"); break;
		case NPCDATAYOFS: SET_NPCDATA_VAR_INT(yofs, "DrawYOffset"); break;
		case NPCDATAZOFS: SET_NPCDATA_VAR_INT(zofs, "DrawZOffset"); break;
		case NPCDDEATHSPR:
		{
			nd->spr_death = vbound(value/10000, 0, MAXSPRITES-1);
			break;
		}
		case NPCDSHADOWSPR:
		{
			nd->spr_shadow = vbound(value/10000, 0, MAXSPRITES-1);
			break;
		}
		case NPCDSPAWNSPR:
		{
			nd->spr_spawn = vbound(value/10000, 0, MAXSPRITES-1);
			break;
		}
		case NPCDATA_WEAPONDATA:
			break;

		default:
			NOTREACHED();
	}
}

// npcdata arrays.

static ArrayRegistrar NPCDATAATTRIBUTE_registrar(NPCDATAATTRIBUTE, []{
	static ScriptingArray_ObjectMemberCArray<guydata, &guydata::attributes> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATADEFENSE_registrar(NPCDATADEFENSE, []{
	static ScriptingArray_ObjectMemberCArray<guydata, &guydata::defense> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar NPCDATAINITD_registrar(NPCDATAINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<guydata, &guydata::scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar NPCDATAWEAPONINITD_registrar(NPCDATAWEAPONINITD, []{
	static ScriptingArray_ObjectSubSubMemberContainer<guydata, &guydata::weap_data, &weapon_data::scrconfig, &script_config::run_args> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar NPCDATAFLAG_registrar(NPCDATAFLAG, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<guydata, &guydata::flags, MAX_NPC_FLAGS> impl;
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATABEHAVIOUR_registrar(NPCDATABEHAVIOUR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<guydata, &guydata::editorflags, 16> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATAMOVEFLAGS_registrar(NPCDATAMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<guydata, &guydata::moveflags, 16> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATAWMOVEFLAGS_registrar(NPCDATAWMOVEFLAGS, []{
	static ScriptingArray_ObjectSubMemberBitwiseFlags<guydata, &guydata::weap_data, &weapon_data::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar NPCDATASHIELD_registrar(NPCDATASHIELD, []{
	static ScriptingArray_ObjectComputed<guydata, bool> impl(
		[](guydata*){ return 5; },
		[](guydata* guy, int index) -> bool {
			switch (index)
			{
				case 0: return guy->flags & guy_shield_front;
				case 1: return guy->flags & guy_shield_left;
				case 2: return guy->flags & guy_shield_right;
				case 3: return guy->flags & guy_shield_back;
				case 4: return guy->flags & guy_bkshield;
				default: NOTREACHED();
			}
		},
		[](guydata* guy, int index, bool value){
			switch (index)
			{
				case 0: SETFLAG(guy->flags, guy_shield_front, value); break;
				case 1: SETFLAG(guy->flags, guy_shield_left, value); break;
				case 2: SETFLAG(guy->flags, guy_shield_right, value); break;
				case 3: SETFLAG(guy->flags, guy_shield_back, value); break;
				case 4: SETFLAG(guy->flags, guy_bkshield, value); break;
				default: NOTREACHED();
			}
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());
