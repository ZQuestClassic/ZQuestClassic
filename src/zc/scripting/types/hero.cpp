#include "zc/scripting/types/hero.h"

#include "base/check.h"
#include "base/zc_math.h"
#include "components/zasm/defines.h"
#include "core/qrs.h"
#include "zc/ffscript.h"
#include "zc/render.h"
#include "zc/scripting/arrays.h"

#include <cstdint>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;
extern ScriptType curScriptType;
extern word curScriptNum;
extern int32_t curScriptIndex;
extern int32_t earlyretval;
extern int32_t directItemA;
extern int32_t directItemB;
extern int32_t directItemX;
extern int32_t directItemY;

int32_t hero_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case CLOCKACTIVE:
			ret=watch?10000:0;
			break;
		case CLOCKCLK:
			ret=clockclk*10000;
			break;
		case HEROBUNNY:
			ret = Hero.BunnyClock()*10000;
			break;
		case HEROCOYOTETIME:
		{
			ret = Hero.coyotetime*10000;
			break;
		}
		case HERODROWNCLK:
			ret = Hero.drownclk * 10000;
			break;
		case HERODROWNCMB:
			ret = Hero.drownCombo * 10000;
			break;
		case HEROFAKEJUMP:
			ret = Hero.getFakeJump().getZLong() / -100;
			break;
		case HEROFAKEZ:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getFakeZ().getZLong();
			}
			else ret = int32_t(Hero.getFakeZ()) * 10000;

			break;
		} 
		case HEROFALLCLK:
			ret = Hero.fallclk * 10000;
			break;
		case HEROFALLCMB:
			ret = Hero.fallCombo * 10000;
			break;
		case HEROFLASHINGCSET:
			ret = (int32_t)(Hero.getFlashingCSet()) * 10000; break;
		case HEROFLICKERCOLOR:
			ret = (int32_t)(Hero.flickercolor) * 10000; break;
		case HEROFLICKERTRANSP:
			ret = (int32_t)(Hero.flickertransp) * 10000; break;
		case HEROHAMMERSTATE:
		{
			ret = Hero.getHammerState() * 10000;
			break;
		}
		case HEROHEALTHBEEP:
		{
			ret = heart_beep ? ( heart_beep_timer * 10000 ) : 0;
			break;
		}
		case HEROICECMB:
			ret = Hero.ice_combo*10000; break;
		case HEROICEENTRYFRAMES:
			ret = Hero.ice_entry_count*10000; break;
		case HEROICEENTRYMAXFRAMES:
			ret = Hero.ice_entry_mcount*10000; break;
		case HEROICEVX:
			ret = Hero.ice_vx.getZLong(); break;
		case HEROICEVY:
			ret = Hero.ice_vy.getZLong(); break;
		case HEROIMMORTAL:
		{
			ret = Hero.immortal * 10000;
			break;
		}
		case HEROISWARPING:
			ret = Hero.is_warping ? 10000L : 0L;
			break;
		case HEROJUMPCOUNT:
			ret = Hero.extra_jump_count * 10000;
			break;
		case HEROLIFTEDWPN:
		{
			ret = Hero.lift_wpn ? Hero.lift_wpn->getUID() : 0;
			break;
		}
		case HEROLIFTHEIGHT:
		{
			ret = Hero.liftheight.getZLong();
			break;
		}
		case HEROLIFTMAXTIMER:
		{
			ret = Hero.tliftclk * 10000;
			break;
		}
		case HEROLIFTTIMER:
		{
			ret = Hero.liftclk * 10000;
			break;
		}
		case HERONOSTEPFORWARD:
			ret = ( (FFCore.nostepforward) ? 10000 : 0 );
			break;
		case HEROPULLCLK:
			ret = Hero.pit_pullclk * 10000;
			break;
		case HEROPULLDIR:
			ret = Hero.pit_pulldir * 10000;
			break;
		case HERORESPAWNDMAP:
		{
			ret = Hero.respawn_dmap * 10000;
			break;
		}
		case HERORESPAWNSCR:
		{
			ret = Hero.respawn_scr * 10000;
			break;
		}
		case HERORESPAWNX:
		{
			ret = Hero.respawn_x.getZLong();
			break;
		}
		case HERORESPAWNY:
		{
			ret = Hero.respawn_y.getZLong();
			break;
		}
		case HEROFORCEDRESPAWN:
			ret = Hero.forced_respawn_point ? 10000 : 0;
			break;
		case HEROSCRICECMB:
			ret = Hero.script_ice_combo*10000; break;
		case HEROSCRIPTCSET:
			ret=script_hero_cset*10000;
			break;
		case HEROSHADOWXOFS:
			ret = (int32_t)(Hero.shadowxofs)*10000;
			break;
		case HEROSHADOWYOFS:
			ret = (int32_t)(Hero.shadowyofs)*10000;
			break;
		case HEROSHIELDJINX:
			ret = Hero.shieldjinxclk * 10000;
			break;
		case HEROSHOVEOFFSET:
			ret = Hero.shove_offset.getZLong();
			break;
		case HEROSLIDING:
			ret = Hero.sliding*10000; break;
		case HEROSTANDING:
		{
			ret = Hero.isStanding(true) ? 10000 : 0;
			break;
		}
		case HEROSTEPRATE:
			ret = Hero.getStepRate() * 10000;
			break;
		case HERO_TMP_STEPBOOST:
			ret = Hero.tmp_step_boost * 10000;
			break;
		case HEROSWITCHMAXTIMER:
		{
			ret = Hero.switchhookmaxtime * 10000;
			break;
		}
		case HEROSWITCHTIMER:
		{
			ret = Hero.switchhookclk * 10000;
			break;
		}
		case HEROTOTALDYOFFS:
			ret = 10000*(((int32_t)(Hero.yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))
				+ ((Hero.switch_hooked && Hero.switchhookstyle == swRISE)
					? -(8-(abs(Hero.switchhookclk-32)/4)) : 0));
			break;
		case HERO_ITEMBOX_HEIGHT:
			ret = Hero.itembox_height * 10000; break;
		case HERO_ITEMBOX_WIDTH:
			ret = Hero.itembox_width * 10000; break;
		case HERO_ITEMBOX_XOFF:
			ret = Hero.itembox_xofs * 10000; break;
		case HERO_ITEMBOX_YOFF:
			ret = Hero.itembox_yofs * 10000; break;
		case HERO_HAMMER_POS:
			ret = Hero.getHammerState() * 10000; break;
		case INPUTA:
			ret=control_state[4]?10000:0;
			break;
		case INPUTAXISDOWN:
			ret=control_state[15]?10000:0;
			break;
		case INPUTAXISLEFT:
			ret=control_state[16]?10000:0;
			break;
		case INPUTAXISRIGHT:
			ret=control_state[17]?10000:0;
			break;
		case INPUTAXISUP:
			ret=control_state[14]?10000:0;
			break;
		case INPUTB:
			ret=control_state[5]?10000:0;
			break;
		case INPUTDOWN:
			ret=control_state[1]?10000:0;
			break;
		case INPUTEX1:
			ret=control_state[10]?10000:0;
			break;
		case INPUTEX2:
			ret=control_state[11]?10000:0;
			break;
		case INPUTEX3:
			ret=control_state[12]?10000:0;
			break;
		case INPUTEX4:
			ret=control_state[13]?10000:0;
			break;
		case INPUTL:
			ret=control_state[7]?10000:0;
			break;
		case INPUTLEFT:
			ret=control_state[2]?10000:0;
			break;
		case INPUTMAP:
			ret=control_state[9]?10000:0;
			break;
		case INPUTMOUSEB:
			ret=(get_mouse_state(3))*10000;
			break;
		case INPUTMOUSEX:
		{
			ret=get_mouse_state(0)*10000;
			break;
		}
		case INPUTMOUSEY:
		{
			int32_t mousequakeoffset = 56+((int32_t)(zc::math::Sin((double)(quakeclk*int64_t(2)-frame))*4));
			int32_t tempoffset = (quakeclk > 0) ? mousequakeoffset : (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			ret=((get_mouse_state(1)-tempoffset))*10000;
			break;
		}
		case INPUTMOUSEZ:
			ret=(get_mouse_state(2))*10000;
			break;
		case INPUTPRESSA:
			ret=button_press[4]?10000:0;
			break;
		case INPUTPRESSB:
			ret=button_press[5]?10000:0;
			break;
		case INPUTPRESSDOWN:
			ret=button_press[1]?10000:0;
			break;
		case INPUTPRESSEX1:
			ret=button_press[10]?10000:0;
			break;
		case INPUTPRESSEX2:
			ret=button_press[11]?10000:0;
			break;
		case INPUTPRESSEX3:
			ret=button_press[12]?10000:0;
			break;
		case INPUTPRESSEX4:
			ret=button_press[13]?10000:0;
			break;
		case INPUTPRESSL:
			ret=button_press[7]?10000:0;
			break;
		case INPUTPRESSLEFT:
			ret=button_press[2]?10000:0;
			break;
		case INPUTPRESSMAP:
			ret=button_press[9]?10000:0;
			break;
		case INPUTPRESSR:
			ret=button_press[8]?10000:0;
			break;
		case INPUTPRESSRIGHT:
			ret=button_press[3]?10000:0;
			break;
		case INPUTPRESSSTART:
			ret=button_press[6]?10000:0;
			break;
		case INPUTPRESSUP:
			ret=button_press[0]?10000:0;
			break;
		case INPUTR:
			ret=control_state[8]?10000:0;
			break;
		case INPUTRIGHT:
			ret=control_state[3]?10000:0;
			break;
		case INPUTSTART:
			ret=control_state[6]?10000:0;
			break;
		case INPUTUP:
			ret=control_state[0]?10000:0;
			break;
		case LINKACTION:
		{
			ret = FFCore.getHeroAction() * 10000;
			break;
		}
		case LINKBIGHITBOX:
			ret=Hero.getBigHitbox()?10000:0;
			break;
		case LINKCANFLICKER:
			ret= Hero.getCanFlicker()?10000:0;
			break;
		case LINKCLIMBING:
			ret = Hero.getOnSideviewLadder()?10000:0;
			break;
		case LINKCSET:
		{
			ret = Hero.cs * 10000;
			break;
		}		
		case LINKDIAG:
			ret=Hero.getDiagMove()?10000:0;
			break;
		case LINKDIR:
			ret=(int32_t)(Hero.dir)*10000;
			break;
		case LINKDRUNK:
			ret = (int32_t)(Hero.DrunkClock())*10000;
			break;
		case LINKEATEN:
			ret=(int32_t)Hero.getEaten()*10000;
			break;
		case LINKENGINEANIMATE:
			ret = (int32_t)(Hero.do_animation)*10000;
			break;
		case LINKEQUIP:
			ret = ((Awpn.id&0xFF)|((Bwpn.id&0xFF)<<8))*10000;
			break;
		case LINKFLIP:
			ret = (int32_t)(Hero.flip)*10000;
			break;
		case LINKGRABBED:
			ret = Hero.inwallm ? 10000 : 0;
			break;
		case LINKGRAVITY:
			ret = ( (Hero.moveflags & move_obeys_grav) ? 10000 : 0 );
			break;
		case LINKHELD:
			ret = (int32_t)(Hero.getHeldItem())*10000;
			break;
		case LINKHITDIR:
			ret=(int32_t)(Hero.getHitDir())*10000;
			break;
		case LINKHP:
			ret=(int32_t)(game->get_life())*10000;
			break;
		case LINKHURTSFX:
			ret = (int32_t)Hero.getHurtSFX()*10000;
			break;
		case LINKX:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getX().getZLong();
			}
			else ret = int32_t(Hero.getX()) * 10000;

			break;
		}
		case LINKXOFS:
			ret = (int32_t)(Hero.xofs)*10000;
			break;
		case LINKY:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getY().getZLong();
			}
			else ret = int32_t(Hero.getY()) * 10000;

			break;
		}	
		case LINKYOFS:
			ret = (int32_t)(Hero.yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))*10000;
			break;
		case LINKZ:
		{
			if (get_qr(qr_SPRITEXY_IS_FLOAT))
			{
				ret = Hero.getZ().getZLong();
			}
			else ret = int32_t(Hero.getZ()) * 10000;

			break;
		} 
		case LINKZOFS:
			ret = (int32_t)(Hero.zofs)*10000;
			break;	
		case LINKMAXHP:
			ret=(int32_t)(game->get_maxlife())*10000;
			break;
		case LINKMAXMP:
			ret=(int32_t)(game->get_maxmagic())*10000;
			break;
		case LINKJUMP:
			ret = Hero.getJump().getZLong();
			break;
		case LINKMP:
			ret=(int32_t)(game->get_magic())*10000;
			break;
		case LINKINVIS:
			ret = (((int32_t)(Hero.getDontDraw())) ? 10000 : 0);
			break;
		case LINKINVINC:
			ret = (int32_t)(Hero.scriptcoldet)*10000;
			break;
		case LINKLADDERX:
			ret=(int32_t)(Hero.getLadderX())*10000;
			break;
		case LINKLADDERY:
			ret=(int32_t)(Hero.getLadderY())*10000;
			break;
		case LINKSWORDJINX:
			ret = (int32_t)(Hero.getSwordClk())*10000;
			break;
		case LINKITEMJINX:
			ret = (int32_t)(Hero.getItemClk())*10000;
			break;
		case LINKROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			ret = (int32_t)(Hero.rotation)*10000;
			break;
		case LINKSCALE:
		{
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			ret = (int32_t)(Hero.scale*100.0);
			break;
		}
		case LINKPUSH:
			ret=(int32_t)Hero.getPushing()*10000;
			break;
		case LINKSTUN:
			ret=(int32_t)Hero.StunClock()*10000;
			break;
		case LINKSCRIPTTILE:
			ret=script_hero_sprite*10000;
			break;
		case LINKSCRIPFLIP:
			ret=script_hero_flip*10000;
			break;
		case LINKITEMB:
			ret = Bwpn.id * 10000;
			break;
		case LINKITEMA:
			ret = Awpn.id * 10000;
			break;
		case LINKITEMX:
			ret = Xwpn.id * 10000;
			break;
		case LINKITEMY:
			ret = Ywpn.id * 10000;
			break;
		case LINKTILEMOD:
			ret = Hero.getTileModifier() * 10000;
			break;
		case LINKHXOFS:
			ret = (int32_t)(Hero.hxofs)*10000;
			break;
		case LINKHYOFS:
			ret = (int32_t)(Hero.hyofs)*10000;
			break;
		case LINKHXSZ:
			ret = (int32_t)(Hero.hit_width)*10000;
			break;
		case LINKHYSZ:
			ret = (int32_t)(Hero.hit_height)*10000;
			break;
		case LINKHZSZ:
			ret = (int32_t)(Hero.hzsz)*10000;
			break;
		case LINKTXSZ:
			ret = (int32_t)(Hero.txsz)*10000;
			break;
		case LINKTYSZ:
			ret = (int32_t)(Hero.tysz)*10000;
			break;
		case LINKTILE:
			ret = (int32_t)(Hero.tile)*10000;
			break;
		case LINKINVFRAME:
			ret = (int32_t)Hero.getHClk()*10000;
			break;
		case PRESSAXISDOWN:
			ret=button_press[15]?10000:0;
			break;
		case PRESSAXISLEFT:
			ret=button_press[16]?10000:0;
			break;
		case PRESSAXISRIGHT:
			ret=button_press[17]?10000:0;
			break;
		case PRESSAXISUP:
			ret=button_press[14]?10000:0;
			break;
		case LINKOTILE:
			ret=FFCore.getHeroOTile(GET_D(rINDEX)/10000, GET_D(rINDEX2) / 10000);
			break;
		case SETITEMSLOT:
			scripting_log_error_with_context("Not supported.");
			ret = 0;
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void hero_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case CLOCKACTIVE:
		{
			Hero.setClock(watch=(value?true:false));
			break;
		}
		case CLOCKCLK:
			clockclk = vbound((value/10000), 0, 214748);
			break;
		case HEROBUNNY:
			Hero.setBunnyClock(value/10000);
			break;
		case HEROCOYOTETIME:
		{
			auto v = value/10000;
			if(v < 0 || v > 65535) v = 65535;
			Hero.coyotetime = word(v);
			break;
		}
		case HERODROWNCLK:
		{
			int32_t val = vbound(value/10000,0,70);
			if(val)
			{
				if (Hero.action != lavadrowning) Hero.setAction(drowning);
			}
			else if(Hero.action == drowning || Hero.action == lavadrowning)
			{
				Hero.setAction(none);
			}
			Hero.drownclk = val;
			break;
		}
		case HERODROWNCMB:
			Hero.drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case HEROFAKEJUMP:
			Hero.setFakeFall(zslongToFix(value) * -100);
			break;
		case HEROFAKEZ:
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					Hero.setFakeZfix(zslongToFix(value));
				}
				else
				{
					Hero.setFakeZ(value/10000);
				}
			}
			break;
		case HEROFALLCLK:
		{
			int32_t val = vbound(value/10000,0,70);
			if(val)
				Hero.setAction(falling);
			else if(Hero.action == falling)
			{
				Hero.setAction(none);
			}
			Hero.fallclk = val;
			break;
		}
		case HEROFALLCMB:
			Hero.fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case HEROFLICKERCOLOR:
		{
			Hero.flickercolor = value/10000;
			break;
		}
		case HEROFLICKERTRANSP:
		{
			Hero.flickertransp = value / 10000;
			break;
		}
		case HEROHAMMERSTATE:
		{
			//readonly
			break;
		}
		case HEROHEALTHBEEP:
		{
			int32_t beep = vbound((value/10000),-4, 255); 
			//-2 suspends system control of stopping the sound
			//-3 suspends system control of stopping the sound AND suspends
			// system control over starting or playing it.
			heart_beep_timer = beep;
			if ( heart_beep_timer > -1 )
			{
				cont_sfx(QMisc.miscsfx[sfxLOWHEART]);
			}
			else
			{
				stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
			}
			break;
		}
		case HEROICEENTRYFRAMES:
			Hero.ice_entry_count = vbound(value/10000,0,255); break;
		case HEROICEENTRYMAXFRAMES:
			Hero.ice_entry_mcount = vbound(value/10000,0,255); break;
		case HEROICEVX:
			Hero.ice_vx = zslongToFix(value); break;
		case HEROICEVY:
			Hero.ice_vy = zslongToFix(value); break;
		case HEROIMMORTAL:
		{
			Hero.setImmortal(value/10000);
			break;
		}
		case HEROJUMPCOUNT:
			Hero.extra_jump_count = value/10000;
			break;
		case HEROLIFTEDWPN:
		{
			if(Hero.lift_wpn)
			{
				delete Hero.lift_wpn;
				Hero.lift_wpn = nullptr;
			}
			if(value)
			{
				if(weapon* wpn = checkLWpn(value))
				{
					if(wpn == Hero.lift_wpn) break;
					Hero.lift_wpn = wpn;
					if(Lwpns.find(wpn) > -1)
						Lwpns.remove(wpn);
					if(curScriptType == ScriptType::Lwpn && value == curScriptIndex)
						earlyretval = RUNSCRIPT_SELFREMOVE;
				}
			} 
			break;
		}
		case HEROLIFTHEIGHT:
		{
			Hero.liftheight = zslongToFix(value);
			break;
		}
		case HEROLIFTMAXTIMER:
		{
			Hero.tliftclk = value/10000;
			break;
		}
		case HEROLIFTTIMER:
		{
			Hero.liftclk = value/10000;
			break;
		}
		case HERONOSTEPFORWARD:
			FFCore.nostepforward = ( (value) ? 1 : 0 ); 
			break;
		case HEROPULLCLK:
			Hero.pit_pullclk = value/10000;
			break;
		case HERORESPAWNDMAP:
		{
			Hero.respawn_dmap = vbound(value/10000, 0, MAXDMAPS-1);
			break;
		}
		case HERORESPAWNSCR:
		{
			Hero.respawn_scr = vbound(value/10000, 0, 0x7F);
			break;
		}
		case HERORESPAWNX:
		{
			Hero.respawn_x = zslongToFix(value);
			break;
		}
		case HERORESPAWNY:
		{
			zfix zy = zslongToFix(value);
			Hero.respawn_y = zslongToFix(value);
			break;
		}
		case HEROFORCEDRESPAWN:
			Hero.forced_respawn_point = bool(value);
			break;
		case HEROSCRICECMB:
			Hero.script_ice_combo = vbound(value/10000,-1,MAXCOMBOS); break;
		case HEROSCRIPTCSET:
			script_hero_cset=vbound((value/10000), -1, 0xF);
			break;
		case HEROSHADOWXOFS:
			(Hero.shadowxofs)=(zfix)(value/10000);
			break;
		case HEROSHADOWYOFS:
			(Hero.shadowyofs)=(zfix)(value/10000);
			break;
		case HEROSHIELDJINX:
		{
			Hero.shieldjinxclk = value / 10000;
			break;
		}
		case HEROSHOVEOFFSET:
			if(!get_qr(qr_NEW_HERO_MOVEMENT2))
				Z_scripterrlog("To use 'Hero->ShoveOffset', you must enable the quest rule 'Newer Hero Movement'.\n");
			Hero.shove_offset = vbound(zslongToFix(value),16_zf,0_zf);
			if(!get_qr(qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER))
				zinit.shove_offset = Hero.shove_offset;
			break;
		case HEROSTEPRATE:
			if(!(get_qr(qr_NEW_HERO_MOVEMENT) || get_qr(qr_NEW_HERO_MOVEMENT2)))
				scripting_log_error_with_context("To use this you must enable New Hero Movement or Newer Hero Movement!");
			Hero.setStepRate(zc_max(value/10000,0));
			if(!get_qr(qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER))
				zinit.heroStep = Hero.getStepRate();
			break;
		case HERO_TMP_STEPBOOST:
			if (!(get_qr(qr_NEW_HERO_MOVEMENT) || get_qr(qr_NEW_HERO_MOVEMENT2)))
				scripting_log_error_with_context("To use this you must enable New Hero Movement or Newer Hero Movement!");
			Hero.tmp_step_boost = value / 10000;
			break;
		case HEROSWITCHMAXTIMER:
		case HEROSWITCHTIMER:
			break; //read-only
		case HEROTOTALDYOFFS:
			break; //READ-ONLY
		case HERO_ITEMBOX_HEIGHT:
			Hero.itembox_height = vbound(value/10000,0,255); break;
		case HERO_ITEMBOX_WIDTH:
			Hero.itembox_width = vbound(value/10000,0,255); break;
		case HERO_ITEMBOX_XOFF:
			Hero.itembox_xofs = vbound(value/10000,-128,127); break;
		case HERO_ITEMBOX_YOFF:
			Hero.itembox_yofs = vbound(value/10000,-128,127); break;
		case INPUTA:
		{
			control_state[4]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[4]=false;
			break;
		}
		case INPUTAXISDOWN:
			control_state[15]=(value?true:false);
			break;
		case INPUTAXISLEFT:
			control_state[16]=(value?true:false);
			break;
		case INPUTAXISRIGHT:
			control_state[17]=(value?true:false);
			break;
		case INPUTAXISUP:
			control_state[14]=(value?true:false);
			break;
		case INPUTB:
		{
			control_state[5]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[5]=false;
			break;
		}
		case INPUTDOWN:
		{
			control_state[1]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) 
				drunk_toggle_state[1]=false;
			break;
		}
		case INPUTEX1:
		{
			control_state[10]=(value?true:false);
			break;
		}
		case INPUTEX2:
			control_state[11]=(value?true:false);
			break;
		case INPUTEX3:
			control_state[12]=(value?true:false);
			break;
		case INPUTEX4:
			control_state[13]=(value?true:false);
			break;
		case INPUTL:
		{
			control_state[7]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[7]=false;
			break;
		}
		case INPUTLEFT:
		{
			control_state[2]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[2]=false;
			break;
		}
		case INPUTMAP:
		{
			control_state[9]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) 
				drunk_toggle_state[9]=false;
			break;
		}
		case INPUTMOUSEX:
		{
			auto [x, y] = rti_game.local_to_world(value/10000, mouse_y);
			position_mouse(x, y);
			break;
		}
		case INPUTMOUSEY:
		{
			int32_t mousequakeoffset = 56+((int32_t)(zc::math::Sin((double)(quakeclk*int64_t(2)-frame))*4));
			int32_t tempoffset = (quakeclk > 0) ? mousequakeoffset : (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			auto [x, y] = rti_game.local_to_world(mouse_x, value/10000 + tempoffset);
			position_mouse(x, y);
			break;
		}
		case INPUTMOUSEZ:
			position_mouse_z(value/10000);
			break;
		case INPUTPRESSA:
			button_press[4]=(value?true:false);
			break;
		case INPUTPRESSB:
			button_press[5]=(value?true:false);
			break;
		case INPUTPRESSDOWN:
			button_press[1]=(value?true:false);
			break;
		case INPUTPRESSEX1:
			button_press[10]=(value?true:false);
			break;
		case INPUTPRESSEX2:
			button_press[11]=(value?true:false);
			break;
		case INPUTPRESSEX3:
			button_press[12]=(value?true:false);
			break;
		case INPUTPRESSEX4:
			button_press[13]=(value?true:false);
			break;
		case INPUTPRESSL:
			button_press[7]=(value?true:false);
			break;
		case INPUTPRESSLEFT:
			button_press[2]=(value?true:false);
			break;
		case INPUTPRESSMAP:
			button_press[9]=(value?true:false);
			break;
		case INPUTPRESSR:
			button_press[8]=(value?true:false);
			break;
		case INPUTPRESSRIGHT:
			button_press[3]=(value?true:false);
			break;
		case INPUTPRESSSTART:
			button_press[6]=(value?true:false);
			break;
		case INPUTPRESSUP:
			button_press[0]=(value?true:false);
			break;
		case INPUTR:
		{
			control_state[8]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[8]=false;
			break;
		}
		case INPUTRIGHT:
		{
			control_state[3]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[3]=false;
			break;
		}
		case INPUTSTART:
		{
			control_state[6]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[6]=false;
			break;
		}
		case INPUTUP:
		{
			control_state[0]=(value?true:false);
			if ( get_qr(qr_FIXDRUNKINPUTS) ) drunk_toggle_state[0]=false;
			break;
		}
		case LINKACTION:
		{
			int32_t act = value / 10000;
			switch(act)
			{
				case hookshotout:
				case stunned:
				case ispushing:
					FFCore.setHeroAction(act);
					break;
				default:
					Hero.setAction((actiontype)(act));
			}
			//Protect from writing illegal actions to Hero's raw variable. 
			//in the future, we can move all scripted actions that are not possible
			//to set in ZC into this mechanic. -Z
			break;
		}
		case LINKBIGHITBOX:
			Hero.setBigHitbox((value/10000)?1:0);
			set_qr(qr_LTTPCOLLISION, (value/10000)?1:0);
			break;
		case LINKCANFLICKER:
			Hero.setCanFlicker((value/10000)?1:0);
			break;
		case LINKCLIMBING:
			Hero.setOnSideviewLadder(value!=0?true:false);
			break;
		case LINKCSET:
		{
			Hero.cs = value/10000;
			break;
		}
		case LINKDIAG:
			Hero.setDiagMove(value?1:0);
			set_qr(qr_LTTPWALK, value?1:0);
			break;
		case LINKDIR:
		{
			//Hero->setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
			if(Hero.getAction() == attacking || Hero.getAction() == sideswimattacking) Hero.dir = (value/10000);
			else Hero.setDir(value/10000);

			break;
		}
		case LINKDRUNK:
			Hero.setDrunkClock(value/10000);
			break;
		case LINKEATEN:
			Hero.setEaten(value/10000);
			break;
		case LINKENGINEANIMATE:
			Hero.do_animation=value;
			break;
		case LINKEQUIP:
		{
			if ( FFCore.getQuestHeaderInfo(vZelda) == 0x250 && FFCore.getQuestHeaderInfo(vBuild) < 33 )
			{
				break;
			}
			//int32_t seta = (value/10000) >> 8; int32_t setb = value/10000) & 0xFF;
			int32_t setb = ((value/10000)&0xFF00)>>8, seta = (value/10000)&0xFF;
			seta = vbound(seta,-1,255);
			setb = vbound(setb,-1,255);

			Awpn = {seta};
			game->awpn = 255;
			game->forced_awpn = seta;
			if (seta > -1)
				game->items_off.set(seta, false);
			directItemA = seta;

			Bwpn = {setb};
			game->bwpn = 255;
			game->forced_bwpn = setb;
			if (setb > -1)
				game->items_off.set(setb, false);
			directItemB = setb;
			break;
		}
		case LINKFLIP:
			(Hero.flip)=(zfix)(value/10000);
			break;
		case LINKGRABBED:
			Hero.inwallm = value != 0;
			break;
		case LINKGRAVITY:
			if(value)
				Hero.moveflags |= move_obeys_grav;
			else
				Hero.moveflags &= ~move_obeys_grav;
			break;
		case LINKHELD:
			Hero.setHeldItem(vbound(value/10000,0,MAXITEMS-1));
			break;
		case LINKHITDIR:
			Hero.setHitDir(value / 10000);
			break;
		case LINKHP:
			game->set_life(zc_max(0, zc_min(value/10000,game->get_maxlife())));
			break;
		case LINKHURTSFX:
			Hero.setHurtSFX( (int32_t)vbound((value/10000), 0, 255) );
			break;
		case LINKHXOFS:
			(Hero.hxofs)=(zfix)(value/10000);
			break;
		case LINKHXSZ:
			(Hero.hit_width)=(zfix)(value/10000);
			break;
		case LINKHYOFS:
			(Hero.hyofs)=(zfix)(value/10000);
			break;
		case LINKHYSZ:
			(Hero.hit_height)=(zfix)(value/10000);
			break;
		case LINKHZSZ:
			(Hero.hzsz)=(zfix)(value/10000);
			break;
		case LINKINVFRAME:
			Hero.setHClk( (int32_t)vbound((value/10000), 0, 214747) );
			break;
		case LINKINVINC:
			Hero.scriptcoldet=(value/10000);
			break;
		case LINKINVIS:
			Hero.setDontDraw((value ? 2 : 0));
			break;
		case LINKITEMA:
		{
			auto val = value / 10000;
			if (val != -1 && invalid_item_id(val))
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemA: %d\n",val);
				break;
			}
			if (Awpn.id != (val))
			{
				Awpn = {val};
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Awpn, game->awpn);
				if (val > -1)
					game->items_off.set(val, false);
				game->forced_awpn = val;
			}
			directItemA = val;
			break;
		}
		case LINKITEMB:
		{
			auto val = value / 10000;
			if (val != -1 && invalid_item_id(val))
			{
				al_trace("Tried to write an invalid item ID to Hero->ItemB: %d\n",val);
				break;
			}
			if (Bwpn.id != (val))
			{
				Bwpn = {val};
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Bwpn, game->bwpn);
				if (val > -1)
					game->items_off.set(val, false);
				game->forced_bwpn = val;
			}
			directItemB = val;
			break;
		}
		case LINKITEMJINX:
			Hero.setItemClk(value/10000);
			break;
		case LINKITEMX:
		{
			auto val = value / 10000;
			if (val != -1 && invalid_item_id(val))
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemX: %d\n",val);
				break;
			}
			if (Xwpn.id != (val))
			{
				Xwpn = {val};
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Xwpn, game->xwpn);
				if (val > -1)
					game->items_off.set(val, false);
				game->forced_xwpn = val;
			}
			directItemX = val;
			break;
		}
		case LINKITEMY:
		{
			auto val = value / 10000;
			if (val != -1 && invalid_item_id(val))
			{
				Z_scripterrlog("Tried to write an invalid item ID to Hero->ItemY: %d\n",val);
				break;
			}
			if (Ywpn.id != (val))
			{
				Ywpn = {val};
				if(new_subscreen_active)
					new_subscreen_active->get_page_pos(Ywpn, game->ywpn);
				if (val > -1)
					game->items_off.set(val, false);
				game->forced_ywpn = val;
			}
			directItemY = val;
			break;
		}
		case LINKJUMP:
			Hero.setJump(zslongToFix(value));
			break;
		case LINKMAXHP:
			game->set_maxlife(value/10000);
			break;
		case LINKMAXMP:
			game->set_maxmagic(value/10000);
			break;
		case LINKMP:
			game->set_magic(zc_max(0, zc_min(value/10000,game->get_maxmagic())));
			break;
		case LINKPUSH:
			Hero.pushing = zc_max((value/10000),0);
			break;
		case LINKROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			(Hero.rotation)=(value/10000);
			break;
		case LINKSCALE:
		{
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			(Hero.scale)=(value/100.0);
			break;
		}
		case LINKSCRIPFLIP:
			script_hero_flip=vbound((value/10000),-1,256);
			break;
		case LINKSCRIPTTILE:
			script_hero_sprite=vbound((value/10000), -1, NEWMAXTILES-1);
			break;
		case LINKSTUN:
			Hero.setStunClock(value/10000);
			break;
		case LINKSWORDJINX:
			Hero.setSwordClk(value/10000);
			break;
		case LINKTILE:
			(Hero.tile)=(zfix)(value/10000);
			break;
		case LINKTXSZ:
			(Hero.txsz)=(zfix)(value/10000);
			break;
		case LINKTYSZ:
			(Hero.tysz)=(zfix)(value/10000);
			break;
		case LINKX:
		{
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				Hero.setXfix(zslongToFix(value));
			}
			else
			{
				Hero.setX(value/10000);
			}
		}
		break;
		case LINKXOFS:
			(Hero.xofs)=(zfix)(value/10000);
			break;
		case LINKY:
		{
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				Hero.setYfix(zslongToFix(value));
			}
			else
			{
				Hero.setY(value/10000);
			}
		}
		break;
		case LINKYOFS:
			(Hero.yofs)=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			break;
		case LINKZ:
		{
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				Hero.setZfix(zslongToFix(value));
			}
			else
			{
				Hero.setZ(value/10000);
			}
		}
		break;
		case LINKZOFS:
			(Hero.zofs)=(zfix)(value/10000);
			break;
		case PRESSAXISDOWN:
			button_press[15]=(value?true:false);
			break;
		case PRESSAXISLEFT:
			button_press[16]=(value?true:false);
			break;
		case PRESSAXISRIGHT:
			button_press[17]=(value?true:false);
			break;
		case PRESSAXISUP:
			button_press[14]=(value?true:false);
			break;
		case SETITEMSLOT:
		{
			//value = third arg
			//int32_t item, int32_t slot, int32_t force
			int32_t itm = GET_D(rINDEX)/10000;
			itm = vbound(itm, -1, MAXITEMS-1);

			int32_t slot = GET_D(rINDEX2)/10000;
			int32_t force = GET_D(rEXP1)/10000;

			//If we add more item buttons, slot should be an int32_t
			//and force shuld be an int32_t

			/*
				For zScript, 
					const int32_t ITM_REQUIRE_NONE = 0
					const int32_t ITM_REQUIRE_INVENTORY = 1
					const int32_t ITM_REQUIRE_A_SLOT_RULE = 2
					//Combine as flags
			*/
			if ( force == 0 )
			{
				switch(slot)
				{
					case 0: //b
						Bwpn = {itm};
						if (itm > -1)
							game->items_off.set(itm, false);
						game->bwpn = 255;
						game->forced_bwpn = itm;
						directItemB = itm;
						break;

					case 1: //a
						Awpn = {itm};
						if (itm > -1)
							game->items_off.set(itm, false);
						game->awpn = 255;
						game->forced_awpn = itm;
						directItemA = itm;
						break;

					case 2: //x
						Xwpn = {itm};
						if (itm > -1)
							game->items_off.set(itm, false);
						game->xwpn = 255;
						game->forced_xwpn = itm;
						directItemX = itm;
						break;

					case 3: //y
						Ywpn = {itm};
						if (itm > -1)
							game->items_off.set(itm, false);
						game->ywpn = 255;
						game->forced_ywpn = itm;
						directItemX = itm;
						break;
				}
			}
			else if ( force == 1 )
			{
				if (game->get_item(itm))
				{
					switch(slot)
					{
						case 0: //b
							Bwpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->bwpn = 255;
							game->forced_bwpn = itm;
							directItemB = itm;
							break;

						case 1: //a
							Awpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->awpn = 255;
							game->forced_awpn = itm;
							directItemA = itm;
							break;

						case 2: //x
							Xwpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->xwpn = 255;
							game->forced_xwpn = itm;
							directItemX = itm;
							break;

						case 3: //y
							Ywpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->ywpn = 255;
							game->forced_ywpn = itm;
							directItemY = itm;
							break;
					}
				}
			}
			else if ( force == 2 )
			{
				switch(slot)
				{
					case 0: //b
						Bwpn = {itm};
						if (itm > -1)
							game->items_off.set(itm, false);
						game->bwpn = 255;
						game->forced_bwpn = itm;
						directItemB = itm;
						break;

					case 1: //a
						if (get_qr(qr_SELECTAWPN))
						{
							Awpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->awpn = 255;
							game->forced_awpn = itm;
							directItemA = itm;
						}
						break;

					case 2:  //x
						Xwpn = {itm};
						if (itm > -1)
							game->items_off.set(itm, false);
						game->xwpn = 255;
						game->forced_xwpn = itm;
						directItemX = itm;
						break;

					case 3: //y
						Ywpn = {itm};
						if (itm > -1)
							game->items_off.set(itm, false);
						game->ywpn = 255;
						game->forced_ywpn = itm;
						directItemY = itm;
						break;
				}
			}
			else if ( force == 3 ) //Flag ITM_REQUIRE_INVENTORY + ITM_REQUIRE_SLOT_A_RULE
			{
				if ( game->get_item(itm) )
				{
					switch(slot)
					{
						case 0: //b
							Bwpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->bwpn = 255;
							game->forced_bwpn = itm;
							directItemB = itm;
							break;

						case 1: //a
							if (get_qr(qr_SELECTAWPN))
							{
								Awpn = {itm};
								if (itm > -1)
									game->items_off.set(itm, false);
								game->awpn = 255;
								game->forced_awpn = itm;
								directItemA = itm;
							}
							break;

						case 2: //x
							Xwpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->xwpn = 255;
							game->forced_xwpn = itm;
							directItemX = itm;
							break;

						case 3: //y
							Ywpn = {itm};
							if (itm > -1)
								game->items_off.set(itm, false);
							game->ywpn = 255;
							game->forced_ywpn = itm;
							directItemY = itm;
							break;
					}
				}
			}
		}
		break;

		default:
			NOTREACHED();
	}
}

// hero arrays.

static ArrayRegistrar LINKMISCD_registrar(LINKMISCD, []{
	static ScriptingArray_ObjectMemberCArray<HeroClass, &HeroClass::miscellaneous> impl;
	impl.setMul10000(false);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar LINKDEFENCE_registrar(LINKDEFENCE, []{
	static ScriptingArray_ObjectMemberCArray<HeroClass, &HeroClass::defence> impl;
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar HEROLIFTFLAGS_registrar(HEROLIFTFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<HeroClass, &HeroClass::liftflags, NUM_LIFTFL> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar HEROMOVEFLAGS_registrar(HEROMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<HeroClass, &HeroClass::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar HEROSTEPS_registrar(HEROSTEPS, []{
	static ScriptingArray_GlobalCArray impl(lsteps, comptime_array_size(lsteps));
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar LINKITEMD_registrar(LINKITEMD, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return MAXITEMS; },
		[](int, int index) {
			return game->get_item(index);
		},
		[](int, int index, bool value) {
			int itemID = index;

			// If the Cane of Byrna is being removed, cancel its effect.
			if (!value && itemID==current_item_id(itype_cbyrna))
				stopCaneOfByrna();
			
			auto& data = get_item_script_engine_data(itemID);
		
			//Stop current script if set false.
			if ( !value && data.doscript )
			{
				data.doscript = 4; //Val of 4 means 'clear stack and quit'
				//itemScriptData[itemID].Clear(); //Don't clear here, causes crash if is current item!
			}
			else if ( value && !data.doscript )
			{
				//Clear the item refInfo and stack for use.
				data.clear_ref();
				if ( (itemsbuf.get(itemID).flags&item_passive_script) ) data.doscript = 1;
			}
			else if ( value && data.doscript == 4 ) 
			{
				// Arbitrary event number 49326: Writing the item false, then true, in the same frame. -Z
				if ( (itemsbuf.get(itemID).flags&item_passive_script) ) data.doscript = 1;
			}
			
			//Sanity check to prevent setting the item if the value would be the same. -Z
			if ( game->get_item(itemID) != value )
			{
				game->set_item(itemID, value);
			}
					
			if((get_qr(qr_OVERWORLDTUNIC) != 0) || (cur_screen<128 || dlevel)) 
			{
				ringcolor(false);
				//refreshpal=true;
			}
			if (!value) //setting the item false clears the state of forced ->Equipment writes.
			{
				if (game->forced_bwpn == itemID)
					game->forced_bwpn = -1;
				if (game->forced_awpn == itemID)
					game->forced_awpn = -1;
				if (game->forced_xwpn == itemID)
					game->forced_xwpn = -1;
				if (game->forced_ywpn == itemID)
					game->forced_ywpn = -1;
			}

			return true;
		}
	);
	impl.setMul10000(true);
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar HEROITEMCOOLDOWN_registrar(HEROITEMCOOLDOWN, []{
	static ScriptingArray_ObjectMemberContainer<HeroClass, &HeroClass::item_cooldown> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar LINKHITBY_registrar(LINKHITBY, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return NUM_HIT_TYPES_USED; },
		[](int, int index) -> int {
			switch (index)
			{
				//screen indices of objects
				case 0:
				case 1:
				case 2:
				case 3:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 16:
					return (Hero.gethitHeroUID(index)) * 10000;

				//uids of objects
				case 4:
				case 5:
				case 6:
				case 7:
				case 13:
				case 14:
				case 15:
					return Hero.gethitHeroUID(index); //do not multiply by 10000! UIDs are not *10000!

				default: NOTREACHED();
			}
		},
		[](int, int, int) {
			return false;
		}
	);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());