#include "zc/scripting/types/combotrigger.h"

#include "base/check.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/types/musicdata.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t combotrigger_get_register(int32_t reg)
{
	int32_t ret = 0;
	combo_trigger* trig = get_combo_trigger(GET_REF(combotriggerref));

	switch (reg)
	{
		case CMBTRIGBOSSPAL:
		{
			if (trig)
			{
				ret = trig->trigbosspalette * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGBUNNY:
		{
			if (trig)
			{
				ret = trig->trig_bunnytime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCHANGECMB:
		{
			if (trig)
			{
				ret = trig->trigchange * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCOOLDOWN:
		{
			if (trig)
			{
				ret = trig->trigcooldown * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCOPYCAT:
		{
			if (trig)
			{
				ret = trig->trigcopycat * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCSETCHANGE:
		{
			if (trig)
			{
				ret = trig->trigcschange * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCTR:
		{
			if (trig)
			{
				ret = trig->trigctr * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCTRAMNT:
		{
			if (trig)
			{
				ret = trig->trigctramnt * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGDMAPLVL:
		{
			if (trig)
			{
				ret = trig->trigdmlevel * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXDOORDIR:
		{
			if (trig)
			{
				ret = trig->exdoor_dir * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXDOORIND:
		{
			if (trig)
			{
				ret = trig->exdoor_ind * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXSTATE:
		{
			if (trig)
			{
				ret = trig->exstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGENSCRIPT:
		{
			if (trig)
			{
				ret = trig->trig_genscr * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROX:
		{
			if (trig)
				ret = trig->dest_player_x;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROY:
		{
			if (trig)
				ret = trig->dest_player_y;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROZ:
		{
			if (trig)
				ret = trig->dest_player_z;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILPROMPTCID:
		{
			if (trig)
				ret = trig->fail_prompt_cid * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILPROMPTCS:
		{
			if (trig)
				ret = trig->fail_prompt_cs * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILSTR:
		{
			if (trig)
				ret = trig->fail_msgstr * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFORCEPLAYERDIR:
		{
			if (trig)
				ret = trig->dest_player_dir * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICECOMBO:
		{
			if (trig)
				ret = trig->force_ice_combo * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICEVX:
		{
			if (trig)
				ret = trig->force_ice_vx;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICEVY:
		{
			if (trig)
				ret = trig->force_ice_vy;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPLAYERBOUNCE:
		{
			if (trig)
				ret = trig->player_bounce;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTCID:
		{
			if (trig)
				ret = trig->prompt_cid * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTCS:
		{
			if (trig)
				ret = trig->prompt_cs * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTX:
		{
			if (trig)
				ret = trig->prompt_x * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTY:
		{
			if (trig)
				ret = trig->prompt_y * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERJUMP:
		{
			if (trig)
				ret = trig->req_player_jump;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERX:
		{
			if (trig)
				ret = trig->req_player_x;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERY:
		{
			if (trig)
				ret = trig->req_player_y;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERZ:
		{
			if (trig)
				ret = trig->req_player_z;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERTRIGSTR:
		{
			if (trig)
				ret = trig->trig_msgstr * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGER_GRAVITY:
		{
			if (trig)
				ret = trig->trig_gravity;
			else ret = -10000;
			break;
		}
		case CMBTRIGGER_TERMINAL_VELOCITY:
		{
			if (trig)
				ret = trig->trig_terminal_v;
			else ret = -10000;
			break;
		}
		case CMBTRIGGER_VIEWPORT_RANGE:
		{
			if (trig)
				ret = trig->viewport_cond_range * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGER_CHANCE_NUMERATOR:
		{
			if (trig)
				ret = trig->chance_numerator * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGER_CHANCE_DENOMINATOR:
		{
			if (trig)
				ret = trig->chance_denominator * 10000;
			else ret = -10000;
			break;
		}
		
		case CMBTRIGGROUP:
		{
			if (trig)
			{
				ret = trig->trig_group * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGROUPVAL:
		{
			if (trig)
			{
				ret = trig->trig_group_val * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGSTATE:
		{
			if (trig)
			{
				ret = trig->trig_gstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGTIMER:
		{
			if (trig)
			{
				ret = trig->trig_statetime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGITEMJINX:
		{
			if (trig)
			{
				ret = trig->trig_itmjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGITEMPICKUP:
		{
			if (trig)
			{
				ret = trig->spawnip * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLIGHTBEAM:
		{
			if (trig)
			{
				ret = trig->triglbeam * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLITEMS:
		{
			if (trig)
			{
				ret = trig->trig_levelitems * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLSTATE:
		{
			if (trig)
			{
				ret = trig->trig_lstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLVLPAL:
		{
			if (trig)
			{
				ret = trig->triglvlpalette * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGPROX:
		{
			if (trig)
			{
				ret = trig->trigprox * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGPUSHTIME:
		{
			if (trig)
			{
				ret = trig->trig_pushtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGQUAKETIME:
		{
			if (trig)
			{
				ret = trig->trigquaketime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGREQITEM:
		{
			if (trig)
			{
				ret = trig->triggeritem * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGREQSTATEMAP:
		{
			if (trig)
			{
				ret = trig->trigstatemap * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGREQSTATESCREEN:
		{
			if (trig)
			{
				ret = trig->trigstatescreen * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSFX:
		{
			if (trig)
			{
				ret = trig->trigsfx * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSHIELDJINX:
		{
			if (trig)
			{
				ret = trig->trig_shieldjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSPAWNENEMY:
		{
			if (trig)
			{
				ret = trig->spawnenemy * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSPAWNITEM:
		{
			if (trig)
			{
				ret = trig->spawnitem * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSTUN:
		{
			if (trig)
			{
				ret = trig->trig_stuntime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSWORDJINX:
		{
			if (trig)
			{
				ret = trig->trig_swjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTIMER:
		{
			if (trig)
			{
				ret = trig->trigtimer * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTB:
		{
			if (trig)
			{
				ret = trig->trigtint[2] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTG:
		{
			if (trig)
			{
				ret = trig->trigtint[1] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTR:
		{
			if (trig)
			{
				ret = trig->trigtint[0] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGWAVYTIME:
		{
			if (trig)
			{
				ret = trig->trigwavytime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGWPNLEVEL:
		{
			if (trig)
			{
				ret = trig->triggerlevel * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIG_MUSIC_REFRESH:
		{
			if (trig)
				ret = trig->set_music_refresh * 10000;
			break;
		}
		case CMBTRIG_PLAY_MUSIC:
		{
			if (trig)
				ret = trig->play_music;
			else ret = -2;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void combotrigger_set_register(int32_t reg, int32_t value)
{
	combo_trigger* trig = get_combo_trigger(GET_REF(combotriggerref));
	if (!trig)
		return;

	switch (reg)
	{
		case CMBTRIGBOSSPAL:
		{
			trig->trigbosspalette = vbound(value/10000, -1, 29);
			break;
		}
		case CMBTRIGBUNNY:
		{
			trig->trig_bunnytime = zc_max(value/10000, -2);
			break;
		}
		case CMBTRIGCHANGECMB:
		{
			trig->trigchange = value/10000;
			break;
		}
		case CMBTRIGCOOLDOWN:
		{
			trig->trigcooldown = vbound(value/10000, 0, 255);
			break;
		}
		case CMBTRIGCOPYCAT:
		{
			trig->trigcopycat = vbound(value/10000, 0, 255);
			break;
		}
		case CMBTRIGCSETCHANGE:
		{
			trig->trigcschange = vbound(value/10000, -128, 127);
			break;
		}
		case CMBTRIGCTR:
		{
			trig->trigctr = vbound(value/10000, sscMIN, MAX_COUNTERS-1);
			break;
		}
		case CMBTRIGCTRAMNT:
		{
			trig->trigctramnt = vbound(value/10000, -65535, 65535);
			break;
		}
		case CMBTRIGDMAPLVL:
		{
			trig->trigdmlevel = vbound(value/10000, -1, MAXLEVELS-1);
			break;
		}
		case CMBTRIGEXDOORDIR:
		{
			trig->exdoor_dir = vbound(value/10000, -1, 3);
			break;
		}
		case CMBTRIGEXDOORIND:
		{
			trig->exdoor_ind = vbound(value/10000, 0, 7);
			break;
		}
		case CMBTRIGEXSTATE:
		{
			trig->exstate = vbound(value/10000, -1, 31);
			break;
		}
		case CMBTRIGGENSCRIPT:
		{
			trig->trig_genscr = vbound(value/10000, 0, 65535);
			break;
		}
		case CMBTRIGGERDESTHEROX:
		{
			trig->dest_player_x = zslongToFix(value);
			break;
		}
		case CMBTRIGGERDESTHEROY:
		{
			trig->dest_player_y = zslongToFix(value);
			break;
		}
		case CMBTRIGGERDESTHEROZ:
		{
			trig->dest_player_z = zslongToFix(value);
			break;
		}
		case CMBTRIGGERFAILPROMPTCID:
		{
			trig->fail_prompt_cid = vbound(value/10000, 0, MAXCOMBOS-1);
			break;
		}
		case CMBTRIGGERFAILPROMPTCS:
		{
			trig->fail_prompt_cs = (value/10000)&15;
			break;
		}
		case CMBTRIGGERFAILSTR:
		{
			trig->fail_msgstr = vbound(value/10000, 0, msg_count-1);
			break;
		}
		case CMBTRIGGERFORCEPLAYERDIR:
		{
			trig->dest_player_dir = vbound(value/10000, 3, -1);
			break;
		}
		case CMBTRIGGERICECOMBO:
		{
			trig->force_ice_combo = vbound(value/10000, MAXCOMBOS-1, -1);
			break;
		}
		case CMBTRIGGERICEVX:
		{
			trig->force_ice_vx = zslongToFix(value);
			break;
		}
		case CMBTRIGGERICEVY:
		{
			trig->force_ice_vy = zslongToFix(value);
			break;
		}
		case CMBTRIGGERPLAYERBOUNCE:
		{
			trig->player_bounce = zslongToFix(value);
			break;
		}
		case CMBTRIGGERPROMPTCID:
		{
			trig->prompt_cid = vbound(value/10000, 0, MAXCOMBOS-1);
			break;
		}
		case CMBTRIGGERPROMPTCS:
		{
			trig->prompt_cs = (value/10000)&15;
			break;
		}
		case CMBTRIGGERPROMPTX:
		{
			trig->prompt_x = vbound(value/10000, -32768, 32767);
			break;
		}
		case CMBTRIGGERPROMPTY:
		{
			trig->prompt_y = vbound(value/10000, -32768, 32767);
			break;
		}
		case CMBTRIGGERREQPLAYERJUMP:
		{
			trig->req_player_jump = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERX:
		{
			trig->req_player_x = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERY:
		{
			trig->req_player_y = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERZ:
		{
			trig->req_player_z = zslongToFix(value);
			break;
		}
		case CMBTRIGGERTRIGSTR:
		{
			trig->trig_msgstr = vbound(value/10000, 0, msg_count-1);
			break;
		}
		case CMBTRIGGER_GRAVITY:
		{
			trig->trig_gravity = zslongToFix(value);
			break;
		}
		case CMBTRIGGER_TERMINAL_VELOCITY:
		{
			trig->trig_terminal_v = zslongToFix(value);
			break;
		}
		case CMBTRIGGER_VIEWPORT_RANGE:
		{
			trig->viewport_cond_range = vbound(value/10000, -32768, 32767);
			break;
		}
		case CMBTRIGGER_CHANCE_NUMERATOR:
		{
			trig->chance_numerator = zc_max(0, value/10000);
			break;
		}
		case CMBTRIGGER_CHANCE_DENOMINATOR:
		{
			trig->chance_denominator = zc_max(1, value/10000);
			break;
		}
		
		case CMBTRIGGROUP:
		{
			trig->trig_group = vbound(value/10000, 0, 255);
			break;
		}
		case CMBTRIGGROUPVAL:
		{
			trig->trig_group_val = vbound(value/10000, 0, 65535);
			break;
		}
		case CMBTRIGGSTATE:
		{
			trig->trig_gstate = vbound(value/10000, 0, 255);
			break;
		}
		case CMBTRIGGTIMER:
		{
			trig->trig_statetime = vbound(value/10000, 0, 214748);
			break;
		}
		case CMBTRIGITEMJINX:
		{
			trig->trig_itmjinxtime = zc_max(value/10000, -2);
			break;
		}
		case CMBTRIGITEMPICKUP:
		{
			const int32_t allowed_pflags = ipHOLDUP | ipTIMER | ipSECRETS | ipCANGRAB;
			trig->spawnip = (value/10000)&allowed_pflags;
			break;
		}
		case CMBTRIGLIGHTBEAM:
		{
			trig->triglbeam = vbound(value/10000,0,32);
			break;
		}
		case CMBTRIGLITEMS:
		{
			trig->trig_levelitems = (value/10000) & LI_ALL;
			break;
		}
		case CMBTRIGLSTATE:
		{
			trig->trig_lstate = vbound(value/10000, 0, 31);
			break;
		}
		case CMBTRIGLVLPAL:
		{
			trig->triglvlpalette = vbound(value/10000, -1, 512);
			break;
		}
		case CMBTRIGPROX:
		{
			trig->trigprox = vbound(value/10000, 0, 65535);
			break;
		}
		case CMBTRIGPUSHTIME:
		{
			trig->trig_pushtime = vbound(value/10000, 0, 255);
			break;
		}
		case CMBTRIGQUAKETIME:
		{
			trig->trigquaketime = zc_max(value/10000, -1);
			break;
		}
		case CMBTRIGREQITEM:
		{
			trig->triggeritem = vbound(value/10000, 0, MAXITEMS-1);
			break;
		}
		case CMBTRIGREQSTATEMAP:
		{
			trig->trigstatemap = vbound(value/10000, 0, map_count);
			break;
		}
		case CMBTRIGREQSTATESCREEN:
		{
			trig->trigstatescreen = vbound(value/10000, 0, MAPSCRSNORMAL-1);
			break;
		}
		case CMBTRIGSFX:
		{
			trig->trigsfx = vbound(value/10000, 0, MAX_SFX);
			break;
		}
		case CMBTRIGSHIELDJINX:
		{
			trig->trig_shieldjinxtime = zc_max(value/10000, -2);
			break;
		}
		case CMBTRIGSPAWNENEMY:
		{
			trig->spawnenemy = vbound(value/10000, 0, 511);
			break;
		}
		case CMBTRIGSPAWNITEM:
		{
			trig->spawnitem = vbound(value/10000, -(MAXITEMDROPSETS-1), MAXITEMS-1);
			break;
		}
		case CMBTRIGSTUN:
		{
			trig->trig_stuntime = zc_max(value/10000, -2);
			break;
		}
		case CMBTRIGSWORDJINX:
		{
			trig->trig_swjinxtime = zc_max(value/10000, -2);
			break;
		}
		case CMBTRIGTIMER:
		{
			trig->trigtimer = vbound(value/10000, 0, 65535);
			break;
		}
		case CMBTRIGTINTB:
		{
			trig->trigtint[2] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case CMBTRIGTINTG:
		{
			trig->trigtint[1] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case CMBTRIGTINTR:
		{
			trig->trigtint[0] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			break;
		}
		case CMBTRIGWAVYTIME:
		{
			trig->trigwavytime = zc_max(value/10000, -1);
			break;
		}
		case CMBTRIGWPNLEVEL:
		{
			trig->triggerlevel = vbound(value/10000, 0, 214748);
			break;
		}
		case CMBTRIG_MUSIC_REFRESH:
		{
			auto v = value / 10000;
			if (v >= -1 && v <= MUSIC_UPDATE_REGION)
				trig->set_music_refresh = v;
			break;
		}
		case CMBTRIG_PLAY_MUSIC:
		{
			if (value == -2 || value == -1 || value == 0 || checkMusic(value))
				trig->play_music = value;
			break;
		}

		default:
			NOTREACHED();
	}
}

// combotrigger arrays.

static ArrayRegistrar CMBTRIGBUTTON_registrar(CMBTRIGBUTTON, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::triggerbtn, 8> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGREQLVLSTATE_registrar(CMBTRIGREQLVLSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::req_level_state, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGUNREQLVLSTATE_registrar(CMBTRIGUNREQLVLSTATE, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::unreq_level_state, 32> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGREQGLOBALSTATE_registrar(CMBTRIGREQGLOBALSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::req_global_state, 256> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGUNREQGLOBALSTATE_registrar(CMBTRIGUNREQGLOBALSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::unreq_global_state, 256> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGREQSCRSTATE_registrar(CMBTRIGREQSCRSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::req_screen_state, 256> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGUNREQSCRSTATE_registrar(CMBTRIGUNREQSCRSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::unreq_screen_state, 256> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGREQSCREXSTATE_registrar(CMBTRIGREQSCREXSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::req_screen_ex_state, 256> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGUNREQSCREXSTATE_registrar(CMBTRIGUNREQSCREXSTATE, []{
	static ScriptingArray_ObjectMemberBitstring<combo_trigger, &combo_trigger::unreq_screen_ex_state, 256> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGGERREQPLAYERDIR_registrar(CMBTRIGGERREQPLAYERDIR, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<combo_trigger, &combo_trigger::req_player_dir, 4> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar CMBTRIGFLAGS_registrar(CMBTRIGFLAGS, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int ref) -> int {
			if (checkComboTrigger(ref))
				return TRIGFLAG_MAX;

			return 0;
		},
		[](int ref, int index) -> bool {
			if (auto* trig = checkComboTrigger(ref))
			{
				if (index == TRIGFLAG_ONLYGENTRIG)
				{
					auto cmb = checkComboFromTriggerRef(ref);
					return cmb ? cmb->only_gentrig : 0;
				}
				else
					return (trig->trigger_flags.get(index));
			}

			return -1;
		},
		[](int ref, int index, bool value){
			if (auto* trig = checkComboTrigger(ref))
			{
				trig->trigger_flags.set(index, value);
				if (index == TRIGFLAG_ONLYGENTRIG)
				{
					if (auto cmb = checkComboFromTriggerRef(ref))
						cmb->only_gentrig = value;
				}
				return true;
			}

			return false;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());
