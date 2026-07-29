#include "zc/scripting/types/combodata.h"

#include "base/check.h"
#include "base/general.h"
#include "components/zasm/defines.h"
#include "core/qrs.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "zc/scripting/arrays.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;
extern ScriptType curScriptType;
extern script_data* curscript;

namespace {

// TODO: replace with checkCombo.
bool checkComboRef()
{
	if (GET_REF(combodataref) < 0 || GET_REF(combodataref) > (MAXCOMBOS-1))
	{
		scripting_log_error_with_context("Invalid combodata ID: {}", GET_REF(combodataref));
		return false;
	}

	return true;
}

} // end namespace

int32_t combodata_get_register(int32_t reg)
{
	int32_t ret = 0;

	#define	GET_COMBO_VAR_INT(member) \
	{ \
		if(!checkComboRef()) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combobuf[GET_REF(combodataref)].member *10000); \
		} \
	} \

	#define	GET_COMBO_VAR_BYTE(member) \
	{ \
		if(!checkComboRef()) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combobuf[GET_REF(combodataref)].member *10000); \
		} \
	} \

	#define	GET_COMBO_VAR_DWORD(member) \
	{ \
		if(!checkComboRef()) \
		{ \
			ret = -10000; \
		} \
		else \
		{ \
			ret = (combobuf[GET_REF(combodataref)].member *10000); \
		} \
	} \

	switch (reg)
	{
		case COMBODACLK:		GET_COMBO_VAR_BYTE(aclk); break;				//char
		case COMBODAKIMANIMY:		GET_COMBO_VAR_BYTE(skipanimy); break;				//C
		case COMBODANIMFLAGS:		GET_COMBO_VAR_BYTE(animflags); break;				//C
		case COMBODASPEED:		GET_COMBO_VAR_BYTE(speed); break;					//char
		case COMBODATAID: 		ret = (GET_REF(combodataref)*10000); break;
		case COMBODATASCRIPT:			GET_COMBO_VAR_DWORD(scrconfig.script); break;						//W
		case COMBODCSET:
		{
			if(!checkComboRef())
			{
				ret = -10000;
			}
			else
			{
				bool neg = combobuf[GET_REF(combodataref)].csets&0x8;
				ret = ((combobuf[GET_REF(combodataref)].csets&0x7) * (neg ? -10000 : 10000));
			}
			break;
		}
		case COMBODCSET2FLAGS:
		{
			if(checkComboRef())
			{
				ret = ((combobuf[GET_REF(combodataref)].csets & 0xF0) >> 4) * 10000;
			}
			break;
		}
		case COMBODEFFECT:
		{
			if(!checkComboRef())
			{
				ret = -10000;
			}
			else
			{
				ret = (((combobuf[GET_REF(combodataref)].walk&0xF0)>>4) *10000);
			}
			break;
		}
		case COMBODFLAG:		GET_COMBO_VAR_BYTE(flag); break;					//C
		case COMBODFLIP:		GET_COMBO_VAR_BYTE(flip); break;					//char
		case COMBODFRAME:		GET_COMBO_VAR_BYTE(cur_frame); break;				//char
		case COMBODFRAMES:		GET_COMBO_VAR_BYTE(frames); break;					//C
		case COMBODLIFTBREAKSFX:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftbreaksfx) * 10000;
			break;
		}
		case COMBODLIFTBREAKSPRITE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftbreaksprite) * 10000;
			break;
		}
		case COMBODLIFTDAMAGE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftdmg) * 10000;
			break;
		}
		case COMBODLIFTGFXCCSET:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftcs) * 10000;
			break;
		}
		case COMBODLIFTGFXCOMBO:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftcmb) * 10000;
			break;
		}
		case COMBODLIFTGFXSPRITE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftsprite) * 10000;
			break;
		}
		case COMBODLIFTGFXTYPE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftgfx) * 10000;
			break;
		}
		case COMBODLIFTHEIGHT:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].lifthei) * 10000;
			break;
		}
		case COMBODLIFTITEM:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftitm) * 10000;
			break;
		}
		case COMBODLIFTLEVEL:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftlvl) * 10000;
			break;
		}
		case COMBODLIFTLIGHTRAD:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].lift_weap_data.light_rads[WPNSPR_BASE]) * 10000;
			break;
		}
		case COMBODLIFTLIGHTSHAPE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].lift_weap_data.glow_shape) * 10000;
			break;
		}
		case COMBODLIFTSFX:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftsfx) * 10000;
			break;
		}
		case COMBODLIFTTIME:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].lifttime) * 10000;
			break;
		}
		case COMBODLIFTUNDERCMB:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftundercmb) * 10000;
			break;
		}
		case COMBODLIFTUNDERCS:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].liftundercs) * 10000;
			break;
		}
		case COMBODLIFTWEAPONITEM:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = (combobuf[GET_REF(combodataref)].lift_parent_item) * 10000;
			break;
		}
		case COMBODNEXTC:		GET_COMBO_VAR_BYTE(nextcset); break;				//C
		case COMBODNEXTD:		GET_COMBO_VAR_INT(nextcombo); break;					//W
		case COMBODNEXTTIMER:		GET_COMBO_VAR_DWORD(nexttimer); break;				//W
		case COMBODNUMTRIGGERS:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			ret = combobuf[GET_REF(combodataref)].triggers.size() * 10000;
			break;
		}
		case COMBODONLYGEN:
		{
			ret = 0;
			if (!checkComboRef()) break;

			ret = combobuf[GET_REF(combodataref)].only_gentrig ? 10000 : 0;
			break;
		}
		case COMBODOTILE:		GET_COMBO_VAR_DWORD(o_tile); break;			//word
		case COMBODSKIPANIM:		GET_COMBO_VAR_BYTE(skipanim); break;				//C
		case COMBODTILE:		GET_COMBO_VAR_DWORD(tile); break;					//word
		case COMBODTRIGBOSSPAL:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigbosspalette * 10000;
			break;
		}
		case COMBODTRIGBUNNY:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_bunnytime * 10000;
			break;
		}
		case COMBODTRIGCSETCHANGE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigcschange * 10000;
			break;
		}
		case COMBODTRIGDMAPLVL:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigdmlevel * 10000;
			break;
		}
		case COMBODTRIGEXDOORDIR:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->exdoor_dir * 10000;
			break;
		}
		case COMBODTRIGEXDOORIND:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->exdoor_ind * 10000;
			break;
		}
		case COMBODTRIGEXSTATE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->exstate * 10000;
			break;
		}
		case COMBODTRIGGERCHANGECMB:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigchange * 10000;
			break;
		}
		case COMBODTRIGGERCOOLDOWN:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigcooldown * 10000;
			break;
		}
		case COMBODTRIGGERCOPYCAT:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigcopycat * 10000;
			break;
		}
		case COMBODTRIGGERCTR:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigctr * 10000;
			break;
		}
		case COMBODTRIGGERCTRAMNT:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigctramnt * 10000;
			break;
		}
		case COMBODTRIGGERGENSCRIPT:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_genscr * 10000;
			break;
		}
		case COMBODTRIGGERGROUP:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_group * 10000;
			break;
		}
		case COMBODTRIGGERGROUPVAL:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_group_val * 10000;
			break;
		}
		case COMBODTRIGGERGSTATE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_gstate * 10000;
			break;
		}
		case COMBODTRIGGERGTIMER:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_statetime * 10000;
			break;
		}
		case COMBODTRIGGERITEM:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->triggeritem * 10000;
			break;
		}
		case COMBODTRIGGERLEVEL:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->triggerlevel * 10000;
			break;
		}
		case COMBODTRIGGERLIGHTBEAM:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->triglbeam * 10000;
			break;
		}
		case COMBODTRIGGERLSTATE:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_lstate * 10000;
			break;
		}
		case COMBODTRIGGERPROX:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigprox * 10000;
			break;
		}
		case COMBODTRIGGERSFX:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigsfx * 10000;
			break;
		}
		case COMBODTRIGGERTIMER:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtimer * 10000;
			break;
		}
		case COMBODTRIGITEMJINX:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_itmjinxtime * 10000;
			break;
		}
		case COMBODTRIGITEMPICKUP:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->spawnip * 10000;
			break;
		}
		case COMBODTRIGLITEMS:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_levelitems * 10000;
			break;
		}
		case COMBODTRIGLVLPAL:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->triglvlpalette * 10000;
			break;
		}
		case COMBODTRIGPUSHTIME:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_pushtime * 10000;
			break;
		}
		case COMBODTRIGQUAKETIME:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigquaketime * 10000;
			break;
		}
		case COMBODTRIGSHIELDJINX:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_shieldjinxtime * 10000;
			break;
		}
		case COMBODTRIGSPAWNENEMY:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->spawnenemy * 10000;
			break;
		}
		case COMBODTRIGSPAWNITEM:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->spawnitem * 10000;
			break;
		}
		case COMBODTRIGSTUN:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_stuntime * 10000;
			break;
		}
		case COMBODTRIGSWORDJINX:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trig_swjinxtime * 10000;
			break;
		}
		case COMBODTRIGTINTB:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtint[2] * 10000;
			break;
		}
		case COMBODTRIGTINTG:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtint[1] * 10000;
			break;
		}
		case COMBODTRIGTINTR:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigtint[0] * 10000;
			break;
		}
		case COMBODTRIGWAVYTIME:
		{
			ret = -10000;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				ret = trig->trigwavytime * 10000;
			break;
		}
		case COMBODTYPE:		GET_COMBO_VAR_BYTE(type); break;					//char
		case COMBODUSRFLAGS:		GET_COMBO_VAR_INT(usrflags); break;				//LONG
		case COMBODWALK:
		{
			if(!checkComboRef())
			{
				ret = -10000;
			}
			else
			{
				ret = ((combobuf[GET_REF(combodataref)].walk&0x0F) *10000);
			}
			break;
		}
		case COMBOD_DIVE_UNDER_LEVEL:
		{
			ret = 0;
			if (!checkComboRef()) break;

			ret = combobuf[GET_REF(combodataref)].dive_under_level * 10000;
			break;
		}
		case COMBOD_Z_HEIGHT:
		{
			ret = 0;
			if (!checkComboRef()) break;

			ret = combobuf[GET_REF(combodataref)].z_height.getZLong();
			break;
		}
		case COMBOD_Z_STEP_HEIGHT:
		{
			ret = 0;
			if (!checkComboRef()) break;

			ret = combobuf[GET_REF(combodataref)].z_step_height.getZLong();
			break;
		}
		case COMBOLAYERR:
		{
			if ( curScriptType == ScriptType::Combo )
			{
				int32_t layer = combopos_ref_to_layer(GET_REF(comboposref));
				ret = layer * 10000;
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, name: {}", ScriptTypeToString(curScriptType), curscript->name());
				ret = -10000;
			}
			break;
		}
		case COMBOPOSR:
		{
			if ( curScriptType == ScriptType::Combo )
			{
				rpos_t rpos = combopos_ref_to_rpos(GET_REF(comboposref));
				ret = (int)rpos * 10000;
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, name: {}", ScriptTypeToString(curScriptType), curscript->name());
				ret = -10000;
			}
			break;
		}
		case COMBOXR:
		{
			if ( curScriptType == ScriptType::Combo )
			{
				rpos_t rpos = combopos_ref_to_rpos(GET_REF(comboposref));
				ret = (( COMBOX_REGION((rpos)) ) * 10000);
				//this may be wrong...may need a special new var for this, storing the exact combopos
				//i is the current script number
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, name: {}", ScriptTypeToString(curScriptType), curscript->name());
				ret = -10000;
			}
			break;
		}
		case COMBOYR:
		{
			if ( curScriptType == ScriptType::Combo )
			{
				rpos_t rpos = combopos_ref_to_rpos(GET_REF(comboposref));
				ret = (( COMBOY_REGION((rpos)) ) * 10000);
			}
			else
			{
				scripting_log_error_with_context("Can only be called by combodata scripts, but you tried to use it from script type {}, name: {}", ScriptTypeToString(curScriptType), curscript->name());
				ret = -10000;
			}
			break;
		}
		case COMBODATA_MISC_WEAPONDATA:
		{
			if (checkCombo(GET_REF(combodataref), false))
				ret = FFCore.get_new_weapondata(wdata_type::combodata_misc, GET_REF(combodataref));
			break;
		}
		case COMBODATA_LIFT_WEAPONDATA:
		{
			if (checkCombo(GET_REF(combodataref), false))
				ret = FFCore.get_new_weapondata(wdata_type::combodata_lift, GET_REF(combodataref));
			break;
		}
		
		default:
			NOTREACHED();
	}

	return ret;
}

void combodata_set_register(int32_t reg, int32_t value)
{
	#define	SET_COMBO_VAR_INT(member) \
	{ \
		if(checkComboRef()) \
		{ \
			screen_combo_modify_pre(GET_REF(combodataref)); \
			combobuf[GET_REF(combodataref)].member = vbound((value / 10000),0,214747); \
			screen_combo_modify_post(GET_REF(combodataref)); \
			\
		} \
	} \

	#define	SET_COMBO_VAR_DWORD(member) \
	{ \
		if(checkComboRef()) \
		{ \
			screen_combo_modify_pre(GET_REF(combodataref)); \
			combobuf[GET_REF(combodataref)].member = vbound((value / 10000),0,32767); \
			screen_combo_modify_post(GET_REF(combodataref)); \
		} \
	} \

	#define	SET_COMBO_VAR_BYTE(member) \
	{ \
		if(checkComboRef()) \
		{ \
			screen_combo_modify_pre(GET_REF(combodataref)); \
			combobuf[GET_REF(combodataref)].member = vbound((value / 10000),0,255); \
			screen_combo_modify_post(GET_REF(combodataref)); \
		} \
	} \

	switch (reg)
	{
		case COMBODACLK:		SET_COMBO_VAR_BYTE(aclk); break;				//char
		case COMBODAKIMANIMY:	SET_COMBO_VAR_BYTE(skipanimy); break;					//C
		case COMBODANIMFLAGS:	SET_COMBO_VAR_BYTE(animflags); break;					//C
		case COMBODASPEED:	SET_COMBO_VAR_BYTE(speed); break;						//char
		case COMBODATASCRIPT:	SET_COMBO_VAR_DWORD(scrconfig.script); break;						//word
		case COMBODCSET:
		{
			if (!checkComboRef()) break;

			screen_combo_modify_pre(GET_REF(combodataref));
			int8_t v = vbound(value, -8, 7);
			combobuf[GET_REF(combodataref)].csets &= ~0xF;
			combobuf[GET_REF(combodataref)].csets |= v;
			screen_combo_modify_post(GET_REF(combodataref));
			break;
		}
		case COMBODCSET2FLAGS:
		{
			if (!checkComboRef()) break;

			screen_combo_modify_pre(GET_REF(combodataref));
			combobuf[GET_REF(combodataref)].csets &= 0xF;
			combobuf[GET_REF(combodataref)].csets |= (value&0xF)<<4;
			screen_combo_modify_post(GET_REF(combodataref));
			break;
		}
		case COMBODEFFECT:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].walk &= ~0xF0;
			combobuf[GET_REF(combodataref)].walk |= ((value / 10000)&0x0F)<<4;
			break;
		}
		case COMBODFLAG:	SET_COMBO_VAR_BYTE(flag); break;						//C
		case COMBODFLIP:	SET_COMBO_VAR_BYTE(flip); break;						//char
		case COMBODFRAME:		SET_COMBO_VAR_BYTE(cur_frame); break;				//char
		case COMBODFRAMES:	SET_COMBO_VAR_BYTE(frames); break;						//C
		case COMBODLIFTBREAKSFX:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftbreaksfx = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTBREAKSPRITE:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftbreaksprite = vbound(value/10000, -4, MAXSPRITES-1);
			break;
		}
		case COMBODLIFTDAMAGE:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftdmg = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTGFXCCSET:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftcs = vbound(value/10000, 0, 13);
			break;
		}
		case COMBODLIFTGFXCOMBO:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftcmb = vbound(value/10000, 0, MAXCOMBOS);
			break;
		}
		case COMBODLIFTGFXSPRITE:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftsprite = vbound(value/10000, 0, MAXSPRITES-1);
			break;
		}
		case COMBODLIFTGFXTYPE:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftgfx = vbound(value/10000, 0, 2);
			break;
		}
		case COMBODLIFTHEIGHT:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].lifthei = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTITEM:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftitm = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTLEVEL:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftlvl = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTLIGHTRAD:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].lift_weap_data.light_rads[WPNSPR_BASE] = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTLIGHTSHAPE:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].lift_weap_data.glow_shape = vbound(value/10000, 0, 2);
			break;
		}
		case COMBODLIFTSFX:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftsfx = vbound(value/10000, 0, MAX_SFX);
			break;
		}
		case COMBODLIFTTIME:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].lifttime = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODLIFTUNDERCMB:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftundercmb = vbound(value/10000, 0, MAXCOMBOS);
			break;
		}
		case COMBODLIFTUNDERCS:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].liftundercs = vbound(value/10000, 0, 13);
			break;
		}
		case COMBODLIFTWEAPONITEM:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].lift_parent_item = vbound(value/10000, 0, MAXITEMS-1);
			break;
		}
		case COMBODNEXTC:	SET_COMBO_VAR_BYTE(nextcset); break;					//C
		case COMBODNEXTD:	SET_COMBO_VAR_INT(nextcombo); break;						//W
		case COMBODNEXTTIMER:	SET_COMBO_VAR_DWORD(nexttimer); break;					//W
		case COMBODNUMTRIGGERS:
		{
			if(checkComboRef())
				combobuf[GET_REF(combodataref)].triggers.resize(vbound(value / 10000, 0, MAX_COMBO_TRIGGERS));
			break;
		}
		case COMBODONLYGEN:
		{
			if(checkComboRef())
				combobuf[GET_REF(combodataref)].only_gentrig = value != 0 ? 1 : 0;
			break;
		}
		case COMBODOTILE:
		{
			if (!checkComboRef()) break;

			newcombo& cdata = combobuf[GET_REF(combodataref)];
			cdata.o_tile = vbound((value / 10000),0,NEWMAXTILES);
			if(get_qr(qr_NEW_COMBO_ANIMATION))
			{
				cdata.tile = cdata.o_tile + ((1+cdata.skipanim)*cdata.cur_frame);
				if(int32_t rowoffset = TILEROW(cdata.tile)-TILEROW(cdata.o_tile))
				{
					cdata.tile += cdata.skipanimy * rowoffset * TILES_PER_ROW;
				}
				combo_caches::drawing.refresh(GET_REF(combodataref));
			}
			break;
		}
		case COMBODSKIPANIM:	SET_COMBO_VAR_BYTE(skipanim); break;					//C
		case COMBODTILE:	SET_COMBO_VAR_INT(tile); break;						//word
		case COMBODTRIGBOSSPAL:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigbosspalette = vbound(value/10000, -1, 29);
			break;
		}
		case COMBODTRIGBUNNY:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_bunnytime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGCSETCHANGE:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigcschange = vbound(value/10000, -15, 15);
			break;
		}
		case COMBODTRIGDMAPLVL:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigdmlevel = vbound(value/10000, -1, MAXLEVELS-1);
			break;
		}
		case COMBODTRIGEXDOORDIR:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->exdoor_dir = vbound(value/10000, -1, 3);
			break;
		}
		case COMBODTRIGEXDOORIND:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->exdoor_ind = vbound(value/10000, 0, 7);
			break;
		}
		case COMBODTRIGEXSTATE:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->exstate = vbound(value/10000, -1, 31);
			break;
		}
		case COMBODTRIGGERCHANGECMB:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigchange = vbound(value/10000,-65535,65535);
			break;
		}
		case COMBODTRIGGERCOOLDOWN:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigcooldown = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERCOPYCAT:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigcopycat = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERCTR:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigctr = vbound(value/10000, sscMIN, MAX_COUNTERS-1);
			break;
		}
		case COMBODTRIGGERCTRAMNT:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigctramnt = vbound(value/10000, -65535, 65535);
			break;
		}
		case COMBODTRIGGERGENSCRIPT:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_genscr = vbound(value/10000, 0, 65535);
			break;
		}
		case COMBODTRIGGERGROUP:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_group = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERGROUPVAL:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_group_val = vbound(value/10000, 0, 65535);
			break;
		}
		case COMBODTRIGGERGSTATE:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_gstate = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGGERGTIMER:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_statetime = vbound(value/10000, 0, 214748);
			break;
		}
		case COMBODTRIGGERITEM:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->triggeritem = vbound(value/10000, 0, MAXITEMS-1);
			break;
		}
		case COMBODTRIGGERLEVEL:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->triggerlevel = vbound(value/10000, 0, 214747);
			break;
		}
		case COMBODTRIGGERLIGHTBEAM:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->triglbeam = vbound(value/10000,0,32);
			break;
		}
		case COMBODTRIGGERLSTATE:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_lstate = vbound(value/10000, 0, 31);
			break;
		}
		case COMBODTRIGGERPROX:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigprox = vbound(value/10000,0,65535);
			break;
		}
		case COMBODTRIGGERSFX:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigsfx = vbound(value/10000, 0, MAX_SFX);
			break;
		}
		case COMBODTRIGGERTIMER:
		{
			if (!checkComboRef()) break;

			screen_combo_modify_pre(GET_REF(combodataref));
			if(auto* trig = get_first_combo_trigger())
				trig->trigtimer = vbound(value/10000,0,65535);
			screen_combo_modify_post(GET_REF(combodataref));
			break;
		}
		case COMBODTRIGITEMJINX:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_itmjinxtime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGITEMPICKUP:
		{
			const int32_t allowed_pflags = ipHOLDUP | ipTIMER | ipSECRETS | ipCANGRAB;
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->spawnip = (value/10000)&allowed_pflags;
			break;
		}
		case COMBODTRIGLITEMS:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_levelitems = (value/10000)&LI_ALL;
			break;
		}
		case COMBODTRIGLVLPAL:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->triglvlpalette = vbound(value/10000, -1, 512);
			break;
		}
		case COMBODTRIGPUSHTIME:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_pushtime = vbound(value/10000, 0, 255);
			break;
		}
		case COMBODTRIGQUAKETIME:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigquaketime = zc_max(value/10000, -1);
			break;
		}
		case COMBODTRIGSHIELDJINX:
		{
			if (!checkComboRef()) break;

			else if(auto* trig = get_first_combo_trigger())
				trig->trig_shieldjinxtime = zc_max(value / 10000, -2);
			break;
		}
		case COMBODTRIGSPAWNENEMY:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->spawnenemy = vbound(value/10000, 0, 511);
			break;
		}
		case COMBODTRIGSPAWNITEM:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->spawnitem = vbound(value/10000, -(MAXITEMDROPSETS-1), MAXITEMS-1);
			break;
		}
		case COMBODTRIGSTUN:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_stuntime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGSWORDJINX:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trig_swjinxtime = zc_max(value/10000, -2);
			break;
		}
		case COMBODTRIGTINTB:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigtint[2] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case COMBODTRIGTINTG:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigtint[1] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case COMBODTRIGTINTR:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigtint[0] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case COMBODTRIGWAVYTIME:
		{
			if (!checkComboRef()) break;

			if(auto* trig = get_first_combo_trigger())
				trig->trigwavytime = zc_max(value/10000, -1);
			break;
		}
		case COMBODTYPE:
		{
			if (!checkComboRef()) break;

			screen_combo_modify_pre(GET_REF(combodataref));
			combobuf[GET_REF(combodataref)].type = vbound((value / 10000),0,255);
			screen_combo_modify_post(GET_REF(combodataref));
			break;
		}
		case COMBODUSRFLAGS:	SET_COMBO_VAR_INT(usrflags); break;					//LONG
		case COMBODWALK:
		{
			if (!checkComboRef()) break;

			combobuf[GET_REF(combodataref)].walk &= ~0x0F;
			combobuf[GET_REF(combodataref)].walk |= (value / 10000)&0x0F;
			break;
		}
		case COMBOD_DIVE_UNDER_LEVEL:
		{
			if(checkComboRef())
				combobuf[GET_REF(combodataref)].dive_under_level = (byte)vbound(value / 10000, 0, 255);
			break;
		}
		case COMBOD_Z_HEIGHT:
		{
			if(checkComboRef())
				combobuf[GET_REF(combodataref)].z_height = zslongToFix(value);
			break;
		}
		case COMBOD_Z_STEP_HEIGHT:
		{
			if(checkComboRef())
				combobuf[GET_REF(combodataref)].z_step_height = zslongToFix(zc_max(0,value));
			break;
		}
		case COMBODATA_MISC_WEAPONDATA:
			break;
		case COMBODATA_LIFT_WEAPONDATA:
			break;

		default:
			NOTREACHED();
	}
}

// combodata arrays.

static ArrayRegistrar COMBODATTRIBUTES_registrar(COMBODATTRIBUTES, []{
	static ScriptingArray_ObjectComputed<newcombo, int> impl(
		[](newcombo*) {
			return NUM_COMBO_ATTRIBUTES;
		},
		[](newcombo* cmb, int index) -> int {
			return cmb->c_attributes[index].getZLong();
		},
		[](newcombo* cmb, int index, int value) {
			cmb->c_attributes[index] = zslongToFix(value);
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar COMBODATTRISHORTS_registrar(COMBODATTRISHORTS, []{
	static ScriptingArray_ObjectComputed<newcombo, int> impl(
		[](newcombo*){
			return 8;
		},
		[](newcombo* cmb, int index) -> int {
			return cmb->c_attributes[16 + index].getTrunc();
		},
		[](newcombo* cmb, int index, int value){
			cmb->c_attributes[16 + index] = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<-32768, 32767>);
	return &impl;
}());

static ArrayRegistrar COMBODATTRIBYTES_registrar(COMBODATTRIBYTES, []{
	static ScriptingArray_ObjectComputed<newcombo, int> impl(
		[](newcombo*){
			return 8;
		},
		[](newcombo* cmb, int index) -> int {
			return cmb->c_attributes[8 + index].getTrunc();
		},
		[](newcombo* cmb, int index, int value){
			cmb->c_attributes[8 + index] = value;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());

static ArrayRegistrar COMBODGENFLAGARR_registrar(COMBODGENFLAGARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<newcombo, &newcombo::genflags, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBOD_LARGECOMBO_DIRS_registrar(COMBOD_LARGECOMBO_DIRS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<newcombo, &newcombo::large_combo_dirs, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODLIFTFLAGS_registrar(COMBODLIFTFLAGS, []{
	static ScriptingArray_ObjectComputed<newcombo, int> impl(
		[](newcombo*){
			return 8;
		},
		[](newcombo* cmb, int index) -> int {
			auto bit = 1 << index;
			if(bit == LF_BREAKONSOLID)
				return cmb->lift_weap_data.wflags & WFLAG_BREAK_ON_SOLID;
			return cmb->liftflags & (1 << index);
		},
		[](newcombo* cmb, int index, int value){
			auto bit = lift_flags(1 << index);
			if(bit == LF_BREAKONSOLID)
				SETFLAG(cmb->lift_weap_data.wflags, WFLAG_BREAK_ON_SOLID, value);
			else
				SETFLAG(cmb->liftflags, bit, value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODUSRFLAGARR_registrar(COMBODUSRFLAGARR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<newcombo, &newcombo::usrflags, 16> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODATAINITD_registrar(COMBODATAINITD, []{
	static ScriptingArray_ObjectComputed<newcombo, int> impl(
		[](newcombo* cmb){
			return cmb->scrconfig.run_args.size();
		},
		[](newcombo* cmb, int index) -> int {
			return cmb->scrconfig.run_args[index] * (get_qr(qr_COMBODATA_INITD_MULT_TENK) ? 10000 : 1);
		},
		[](newcombo* cmb, int index, int value){
			// TODO: Not sure why this compat qr multiplies by 10000, should probably divide?
			cmb->scrconfig.run_args[index] = value * ( get_qr(qr_COMBODATA_INITD_MULT_TENK) ? 10000 : 1);
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERBUTTON_registrar(COMBODTRIGGERBUTTON, []{
	static ScriptingArray_ObjectComputed<newcombo, bool> impl(
		[](newcombo*){
			return 8;
		},
		[](newcombo*, int index) -> bool {
			if (auto* trig = get_first_combo_trigger())
				return trig->triggerbtn & (1<<index);
			return 0;
		},
		[](newcombo*, int index, bool value){
			if (auto* trig = get_first_combo_trigger())
				SETFLAG(trig->triggerbtn, 1<<index, value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERFLAGS2_registrar(COMBODTRIGGERFLAGS2, []{
	static ScriptingArray_ObjectComputed<newcombo, bool> impl(
		[](newcombo*){
			return 32*6;
		},
		[](newcombo* cmb, int index) -> bool {
			if (auto* trig = get_first_combo_trigger())
			{
				if (index == TRIGFLAG_ONLYGENTRIG)
					return cmb->only_gentrig;
				else
					return (trig->trigger_flags.get(index));
			}

			return 0;
		},
		[](newcombo* cmb, int index, bool value){
			if (auto* trig = get_first_combo_trigger())
			{
				trig->trigger_flags.set(index, value);
				if (index == TRIGFLAG_ONLYGENTRIG)
					cmb->only_gentrig = value;
			}
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERS_registrar(COMBODTRIGGERS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) -> int {
			if (auto cmb = checkCombo(ref))
				return zc_min(cmb->triggers.size(), MAX_COMBO_TRIGGERS);

			return 0;
		},
		[](int ref, int index) -> int {
			return dword(ref) | dword(index) << 24;
		},
		[](int, int, int){
			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());

static ArrayRegistrar COMBODTRIGGERFLAGS_registrar(COMBODTRIGGERFLAGS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int ref) -> int {
			if (auto cmb = checkCombo(ref))
			{
				if (cmb->triggers.empty())
					cmb->triggers.emplace_back();
				return 6; // deprecated accessor
			}

			return 0;
		},
		[](int ref, int index) -> int {
			if (auto cmb = checkCombo(ref))
			{
				auto& trig = cmb->triggers[0];
				int32_t ret = 0;
				for(size_t q = 0; q < 32; ++q)
					if(trig.trigger_flags.get(index*32 + q))
						ret |= 1<<q;
				if (index == 0)
					SETFLAG(ret, 1 << (TRIGFLAG_ONLYGENTRIG%32), cmb->only_gentrig);
				return ret;
			}

			return -1;
		},
		[](int ref, int index, int value){
			if (auto cmb = checkCombo(ref))
			{
				auto& trig = cmb->triggers[0];
				screen_combo_modify_pre(ref);
				for(size_t q = 0; q < 32; ++q)
					trig.trigger_flags.set(index*32 + q, value & (1<<q));
				if (index == 0)
					cmb->only_gentrig = trig.trigger_flags.get(TRIGFLAG_ONLYGENTRIG);
				screen_combo_modify_post(ref);
				return true;
			}

			return false;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	impl.setValueTransform(transforms::vbound<0, 214747>);
	return &impl;
}());
