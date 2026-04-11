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

	switch (reg)
	{
		case CMBTRIGBOSSPAL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigbosspalette * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGBUNNY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_bunnytime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCHANGECMB:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigchange * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCOOLDOWN:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigcooldown * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCOPYCAT:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigcopycat * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCSETCHANGE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigcschange * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigctr * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGCTRAMNT:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigctramnt * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGDMAPLVL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigdmlevel * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXDOORDIR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->exdoor_dir * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXDOORIND:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->exdoor_ind * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGEXSTATE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->exstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGENSCRIPT:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_genscr * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->dest_player_x;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->dest_player_y;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERDESTHEROZ:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->dest_player_z;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->fail_prompt_cid * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->fail_prompt_cs * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFAILSTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->fail_msgstr * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERFORCEPLAYERDIR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->dest_player_dir * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICECOMBO:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->force_ice_combo * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICEVX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->force_ice_vx;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERICEVY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->force_ice_vy;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPLAYERBOUNCE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->player_bounce;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->prompt_cid * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->prompt_cs * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->prompt_x * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERPROMPTY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->prompt_y * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERJUMP:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->req_player_jump;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->req_player_x;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->req_player_y;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERREQPLAYERZ:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->req_player_z;
			else ret = -10000;
			break;
		}
		case CMBTRIGGERTRIGSTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->trig_msgstr * 10000;
			else ret = -10000;
			break;
		}
		case CMBTRIGGER_GRAVITY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->trig_gravity;
			else ret = -10000;
			break;
			break;
		}
		case CMBTRIGGER_TERMINAL_VELOCITY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->trig_terminal_v;
			else ret = -10000;
			break;
		}
		
		case CMBTRIGGROUP:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_group * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGROUPVAL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_group_val * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGSTATE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_gstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGGTIMER:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_statetime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGITEMJINX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_itmjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGITEMPICKUP:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->spawnip * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLIGHTBEAM:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->triglbeam * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLITEMS:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_levelitems * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLSTATE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_lstate * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGLVLPAL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->triglvlpalette * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGPROX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigprox * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGPUSHTIME:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_pushtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGQUAKETIME:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigquaketime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGREQITEM:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->triggeritem * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGREQSTATEMAP:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigstatemap * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGREQSTATESCREEN:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigstatescreen * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSFX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigsfx * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSHIELDJINX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_shieldjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSPAWNENEMY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->spawnenemy * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSPAWNITEM:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->spawnitem * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSTUN:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_stuntime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGSWORDJINX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trig_swjinxtime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTIMER:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigtimer * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTB:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigtint[2] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTG:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigtint[1] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGTINTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigtint[0] * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGWAVYTIME:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->trigwavytime * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIGWPNLEVEL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				ret = trig->triggerlevel * 10000;
			}
			else ret = -10000;
			break;
		}
		case CMBTRIG_MUSIC_REFRESH:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				ret = trig->set_music_refresh * 10000;
			break;
		}
		case CMBTRIG_PLAY_MUSIC:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
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
	switch (reg)
	{
		case CMBTRIGBOSSPAL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigbosspalette = vbound(value/10000, -1, 29);
			}
			break;
		}
		case CMBTRIGBUNNY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_bunnytime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGCHANGECMB:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigchange = value/10000;
			}
			break;
		}
		case CMBTRIGCOOLDOWN:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigcooldown = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGCOPYCAT:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigcopycat = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGCSETCHANGE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigcschange = vbound(value/10000, -128, 127);
			}
			break;
		}
		case CMBTRIGCTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigctr = vbound(value/10000, sscMIN, MAX_COUNTERS-1);
			}
			break;
		}
		case CMBTRIGCTRAMNT:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigctramnt = vbound(value/10000, -65535, 65535);
			}
			break;
		}
		case CMBTRIGDMAPLVL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigdmlevel = vbound(value/10000, -1, MAXLEVELS-1);
			}
			break;
		}
		case CMBTRIGEXDOORDIR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->exdoor_dir = vbound(value/10000, -1, 3);
			}
			break;
		}
		case CMBTRIGEXDOORIND:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->exdoor_ind = vbound(value/10000, 0, 7);
			}
			break;
		}
		case CMBTRIGEXSTATE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->exstate = vbound(value/10000, -1, 31);
			}
			break;
		}
		case CMBTRIGGENSCRIPT:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_genscr = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGGERDESTHEROX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->dest_player_x = zslongToFix(value);
			break;
		}
		case CMBTRIGGERDESTHEROY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->dest_player_y = zslongToFix(value);
			break;
		}
		case CMBTRIGGERDESTHEROZ:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->dest_player_z = zslongToFix(value);
			break;
		}
		case CMBTRIGGERFAILPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->fail_prompt_cid = vbound(value/10000, 0, MAXCOMBOS-1);
			break;
		}
		case CMBTRIGGERFAILPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->fail_prompt_cs = (value/10000)&15;
			break;
		}
		case CMBTRIGGERFAILSTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->fail_msgstr = vbound(value/10000, 0, msg_count-1);
			break;
		}
		case CMBTRIGGERFORCEPLAYERDIR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->dest_player_dir = vbound(value/10000, 3, -1);
			break;
		}
		case CMBTRIGGERICECOMBO:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->force_ice_combo = vbound(value/10000, MAXCOMBOS-1, -1);
			break;
		}
		case CMBTRIGGERICEVX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->force_ice_vx = zslongToFix(value);
			break;
		}
		case CMBTRIGGERICEVY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->force_ice_vy = zslongToFix(value);
			break;
		}
		case CMBTRIGGERPLAYERBOUNCE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->player_bounce = zslongToFix(value);
			break;
		}
		case CMBTRIGGERPROMPTCID:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->prompt_cid = vbound(value/10000, 0, MAXCOMBOS-1);
			break;
		}
		case CMBTRIGGERPROMPTCS:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->prompt_cs = (value/10000)&15;
			break;
		}
		case CMBTRIGGERPROMPTX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->prompt_x = vbound(value/10000, -32768, 32767);
			break;
		}
		case CMBTRIGGERPROMPTY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->prompt_y = vbound(value/10000, -32768, 32767);
			break;
		}
		case CMBTRIGGERREQPLAYERJUMP:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->req_player_jump = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->req_player_x = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->req_player_y = zslongToFix(value);
			break;
		}
		case CMBTRIGGERREQPLAYERZ:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->req_player_z = zslongToFix(value);
			break;
		}
		case CMBTRIGGERTRIGSTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->trig_msgstr = vbound(value/10000, 0, msg_count-1);
			break;
		}
		case CMBTRIGGER_GRAVITY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->trig_gravity = zslongToFix(value);
			break;
		}
		case CMBTRIGGER_TERMINAL_VELOCITY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				trig->trig_terminal_v = zslongToFix(value);
			break;
		}
		case CMBTRIGGROUP:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_group = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGGROUPVAL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_group_val = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGGSTATE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_gstate = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGGTIMER:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_statetime = vbound(value/10000, 0, 214748);
			}
			break;
		}
		case CMBTRIGITEMJINX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_itmjinxtime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGITEMPICKUP:
		{
			const int32_t allowed_pflags = ipHOLDUP | ipTIMER | ipSECRETS | ipCANGRAB;
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->spawnip = (value/10000)&allowed_pflags;
			}
			break;
		}
		case CMBTRIGLIGHTBEAM:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->triglbeam = vbound(value/10000,0,32);
			}
			break;
		}
		case CMBTRIGLITEMS:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_levelitems = (value/10000) & LI_ALL;
			}
			break;
		}
		case CMBTRIGLSTATE:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_lstate = vbound(value/10000, 0, 31);
			}
			break;
		}
		case CMBTRIGLVLPAL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->triglvlpalette = vbound(value/10000, -1, 512);
			}
			break;
		}
		case CMBTRIGPROX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigprox = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGPUSHTIME:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_pushtime = vbound(value/10000, 0, 255);
			}
			break;
		}
		case CMBTRIGQUAKETIME:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigquaketime = zc_max(value/10000, -1);
			}
			break;
		}
		case CMBTRIGREQITEM:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->triggeritem = vbound(value/10000, 0, MAXITEMS-1);
			}
			break;
		}
		case CMBTRIGREQSTATEMAP:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigstatemap = vbound(value/10000, 0, map_count);
			}
			break;
		}
		case CMBTRIGREQSTATESCREEN:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigstatescreen = vbound(value/10000, 0, MAPSCRSNORMAL-1);
			}
			break;
		}
		case CMBTRIGSFX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigsfx = vbound(value/10000, 0, MAX_SFX);
			}
			break;
		}
		case CMBTRIGSHIELDJINX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_shieldjinxtime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGSPAWNENEMY:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->spawnenemy = vbound(value/10000, 0, 511);
			}
			break;
		}
		case CMBTRIGSPAWNITEM:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->spawnitem = vbound(value/10000, -(MAXITEMDROPSETS-1), MAXITEMS-1);
			}
			break;
		}
		case CMBTRIGSTUN:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_stuntime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGSWORDJINX:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trig_swjinxtime = zc_max(value/10000, -2);
			}
			break;
		}
		case CMBTRIGTIMER:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigtimer = vbound(value/10000, 0, 65535);
			}
			break;
		}
		case CMBTRIGTINTB:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigtint[2] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			}
			break;
		}
		case CMBTRIGTINTG:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigtint[1] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			}
			break;
		}
		case CMBTRIGTINTR:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigtint[0] = scripting_write_pal_color(vbound(value/10000, -scripting_max_color_val, scripting_max_color_val));
			}
			break;
		}
		case CMBTRIGWAVYTIME:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->trigwavytime = zc_max(value/10000, -1);
			}
			break;
		}
		case CMBTRIGWPNLEVEL:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
			{
				trig->triggerlevel = vbound(value/10000, 0, 214748);
			}
			break;
		}
		case CMBTRIG_MUSIC_REFRESH:
		{
			auto v = value / 10000;
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
				if (v >= -1 && v <= MUSIC_UPDATE_REGION)
					trig->set_music_refresh = v;
			break;
		}
		case CMBTRIG_PLAY_MUSIC:
		{
			if(auto* trig = get_combo_trigger(GET_REF(combotriggerref)))
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
