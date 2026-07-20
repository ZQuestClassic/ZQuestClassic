#include "zc/scripting/types/lweapon.h"

#include "base/check.h"
#include "base/zc_math.h"
#include "components/zasm/defines.h"
#include "core/qrs.h"
#include "core/zdefs.h"
#include "zc/ffscript.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

int32_t lweapon_get_register(int32_t reg)
{
	int32_t ret = 0;
	weapon* s = checkLWpn(GET_REF(lwpnref));
	if (!s)
		return 0;

	switch (reg)
	{
		case LWEAPONSCRIPTUID:
			ret=(s->getUID());
			break;
		case LWPNANGLE:
			ret=(int32_t)(s->angle*10000);
			break;
		case LWPNANGULAR:
			ret=s->angular*10000;
			break;
		case LWPNASPEED:
			ret=s->o_speed*10000;
			break;
		case LWPNAUTOROTATE:
			ret=s->autorotate*10000;
			break;
		case LWPNBEHIND:
			ret=s->behind*10000;
			break;
		case LWPNCOLLDET:
			ret = s->script_no_colldet ? 0 : 10000;
			break;
		case LWPNCSET:
			ret=s->cs*10000;
			break;
		case LWPNDEAD:
			ret=s->dead*10000;
			break;
		case LWPNDEATHDROPSET:
			ret = s->death_spawndropset * 10000;
			break;
		case LWPNDEATHIPICKUP:
			ret = s->death_item_pflags * 10000;
			break;
		case LWPNDEATHITEM:
			ret = s->death_spawnitem * 10000;
			break;
		case LWPNDEATHSFX:
			ret = s->death_sfx * 10000;
			break;
		case LWPNDEATHSPRITE:
			ret = s->death_sprite * 10000;
			break;
		case LWPNDEGANGLE:
			ret=(int32_t)(s->angle*(180.0 / PI)*10000);
			break;
		case LWPNDIR:
			ret=s->dir*10000;
			break;
		case LWPNDRAWTYPE:
			ret=s->drawstyle*10000;
			break;
		case LWPNDROWNCLK:
			ret = s->drownclk * 10000;
			break;
		case LWPNDROWNCMB:
			ret = s->drownCombo * 10000;
			break;
		case LWPNENGINEANIMATE:
			ret=(s->do_animation)*10000;
			break;
		case LWPNEXTEND:
			ret=s->extend*10000;
			break;
		case LWPNFAKEJUMP:
			ret = s->fakefall.getZLong() / -100;
			if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			break;
		case LWPNFAKEZ:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->fakez).getZLong();  
			}
			else 
				ret=((int32_t)s->fakez)*10000;
			break;
		case LWPNFALLCLK:
			ret = s->fallclk * 10000;
			break;
		case LWPNFALLCMB:
			ret = s->fallCombo * 10000;
			break;
		case LWPNFLASH:
			ret=s->flash*10000;
			break;
		case LWPNFLASHCSET:
			ret=(s->o_cset>>4)*10000;
			break;
		case LWPNFLIP:
			ret=s->flip*10000;
			break;
		case LWPNFRAME:
			ret=s->aframe*10000;
			break;
		case LWPNFRAMES:
			ret=s->frames*10000;
			break;
		case LWPNGLOWRAD:
			ret = s->glowRad * 10000;
			break;
		case LWPNGLOWSHP:
			ret = s->glowShape * 10000;
			break;
		case LWPNGRAVITY:
			ret= (s->moveflags & move_obeys_grav) ? 10000 : 0;
			break;
		case LWPNHXOFS:
			ret=(s->hxofs)*10000;
			break;
		case LWPNHXSZ:
			ret=(s->hit_width)*10000;
			break;
		case LWPNHYOFS:
			ret=(s->hyofs)*10000;
			break;
		case LWPNHYSZ:
			ret=(s->hit_height)*10000;
			break;
		case LWPNHZSZ:
			ret=(s->hzsz)*10000;
			break;
		case LWPNJUMP:
			ret = s->fall.getZLong() / -100;
			if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			break;
		case LWPNLEVEL:
			ret=(s->level)*10000;
			break;
		case LWPNLIFTHEIGHT:
			ret = s->lift_height.getZLong();
			break;
		case LWPNLIFTLEVEL:
			ret = s->lift_level * 10000;
			break;
		case LWPNLIFTTIME:
			ret = s->lift_time * 10000;
			break;
		case LWPNOCSET:
			ret=(s->o_cset&15)*10000;
			break;
		case LWPNOTILE:
			ret=s->o_tile*10000;
			break;
		case LWPNPARENT:
			ret=(s->parentitem)*10000;
			break;
		case LWPNPIERCE:
			ret = s->pierce_count * 10000;
			break;
		case LWPNPOWER:
			ret=s->power*10000;
			break;
		case LWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			ret=s->rotation*10000;
			break;
		case LWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			ret=((int32_t)s->scale)*100.0;
			break;
		case LWPNSCRIPT:
			ret=(s->scrconfig.script)*10000;
			break;
		case LWPNSCRIPTFLIP:
			ret=s->scriptflip*10000;
			break;
		case LWPNSCRIPTTILE:
			ret=s->scripttile*10000;
			break;
		case LWPNSHADOWSPR:
			ret = s->spr_shadow * 10000;
			break;
		case LWPNSHADOWXOFS:
			ret=((int32_t)(s->shadowxofs))*10000;
			break;
		case LWPNSHADOWYOFS:
			ret=((int32_t)(s->shadowyofs))*10000;
			break;
		case LWPNSPECIAL:
			ret=((int32_t)s->specialinfo)*10000;

			break;
		case LWPNSTEP:
			if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
			{
				ret=s->step.getZLong() * 100;
			}
			//old, buggy code replication, round two: Go! -Z
			//else ret = ( ( ( s->step ) * 100.0 ).getZLong() );

			//else 
			//{
				//old, buggy code replication, round THREE: Go! -Z
			//	double tmp = ( s->step.getFloat() ) * 1000000.0;
			//	ret = (int32_t)tmp;
			//}

			//old, buggy code replication, round FOUR: Go! -Z
			else ret = (int32_t)((float)s->step * 1000000.0);
			break;
		case LWPNTILE:
			ret=s->tile*10000;
			break;
		case LWPNTIMEOUT:
			if (s)
			{
				ret = s->weap_timeout * 10000;
			}
			break;
		case LWPNTOTALDYOFFS:
			ret = ((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))
					+ ((s->switch_hooked && Hero.switchhookstyle == swRISE)
						? -(8-(abs(Hero.switchhookclk-32)/4)) : 0)) * 10000;
			break;
		case LWPNTXSZ:
			ret=(s->txsz)*10000;
			break;
		case LWPNTYPE:
			ret=s->id*10000;
			break;
		case LWPNTYSZ:
			ret=(s->tysz)*10000;
			break;
		case LWPNUNBL:
			ret = s->unblockable * 10000;
			break;
		case LWPNUSEDEFENCE:
			ret=(s->usedefense)*10000;
			break;
		case LWPNUSEWEAPON:
			ret=(s->useweapon)*10000;
			break;
		case LWPNVX:
			if (s->angular)
				ret = int32_t(zc::math::Cos(s->angle)*10000.0*s->step);
			else
			{
				switch(NORMAL_DIR(s->dir))
				{
					case l_up:
					case l_down:
					case left:
						ret = int32_t(-10000.0*s->step);
						break;

					case r_down:
					case r_up:
					case right:
						ret = int32_t(10000.0*s->step);
						break;

					default:
						ret = 0;
						break;
				}
			}
			break;
		case LWPNVY:
			if (s->angular)
				ret = int32_t(zc::math::Sin(s->angle)*10000.0*s->step);
			else
			{
				switch(NORMAL_DIR(s->dir))
				{
					case l_up:
					case r_up:
					case up:
						ret = int32_t(-10000.0*s->step);
						break;
					case l_down:
					case r_down:
					case down:
						ret = int32_t(10000.0*s->step);
						break;

					default:
						ret = 0;
						break;
				}
			}
			break;
		case LWPNX:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->x).getZLong();  
			}
			else 
				ret=((int32_t)s->x)*10000;
			break;
		case LWPNXOFS:
			ret=((int32_t)(s->xofs))*10000;
			break;
		case LWPNY:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->y).getZLong();  
			}
			else 
				ret=((int32_t)s->y)*10000;
			break;
		case LWPNYOFS:
			ret=((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))*10000;
			break;
		case LWPNZ:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->z).getZLong();  
			}
			else 
				ret=((int32_t)s->z)*10000;
			break;
		case LWPNZOFS:
			ret=((int32_t)(s->zofs))*10000;
			break;
		case LWSWHOOKED:
			ret = s->switch_hooked ? 10000 : 0;
			break;
		case LWPN_BOUNCE_MULT:
			ret = s->bounce_mult.getZLong();
			break;
		case LWPN_BOUNCE_ADD:
			ret = s->bounce_add.getZLong();
			break;
		case LWPN_NO_TRIGGERS:
			ret = s->disable_triggers ? 10000 : 0;
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void lweapon_set_register(int32_t reg, int32_t value)
{
	weapon* s = checkLWpn(GET_REF(lwpnref));
	if (!s)
		return;

	switch (reg)
	{
		case LWPNANGLE:
			s->angle=(double)(value/10000.0);
			s->doAutoRotate();
			break;
		case LWPNANGULAR:
			s->angular=(value!=0);
			s->doAutoRotate(false, true);
			break;
		case LWPNASPEED:
			s->o_speed=(value/10000);
			break;
		case LWPNAUTOROTATE:
			s->autorotate=(value!=0);
			s->doAutoRotate(false, true);
			break;
		case LWPNBEHIND:
			s->behind=(value!=0);
			break;
		case LWPNCOLLDET:
			s->script_no_colldet = value ? 0 : -1;
			break;
		case LWPNCSET:
			s->cs=(value/10000)&15;
			break;
		case LWPNDEAD:
		{
			auto dead = value/10000;
			s->dead=dead;
			if(dead != 0) s->weapon_dying_frame = false;
			break;
		}
		case LWPNDEATHDROPSET:
			s->death_spawndropset = vbound(value/10000,-1,MAXITEMDROPSETS-1);
			break;
		case LWPNDEATHIPICKUP:
			s->death_item_pflags = value/10000;
			break;
		case LWPNDEATHITEM:
			s->death_spawnitem = vbound(value/10000,-1,MAXITEMS-1);
			break;
		case LWPNDEATHSFX:
			s->death_sfx = vbound(value/10000,0,MAX_SFX);
			break;
		case LWPNDEATHSPRITE:
			s->death_sprite = vbound(value/10000,-255,MAXSPRITES-1);
			break;
		case LWPNDEGANGLE:
		{
			double rangle = (value / 10000.0) * (PI / 180.0);
			s->angle=(double)(rangle);
			s->doAutoRotate();
			break;
		}
		case LWPNDIR:
			s->dir=(value/10000);
			s->doAutoRotate(true);
			break;
		case LWPNDRAWTYPE:
			s->drawstyle=(value/10000);
			break;
		case LWPNDROWNCLK:
			if(s->drownclk != 0 && value == 0)
			{
				s->cs = s->o_cset;
				s->tile = s->o_tile;
			}
			else if(s->drownclk == 0 && value != 0) s->o_cset = s->cs;
			s->drownclk = vbound(value/10000,0,70);
			break;
		case LWPNDROWNCMB:
			s->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case LWPNENGINEANIMATE:
			(s->do_animation)=value;
			break;
		case LWPNEXTEND:
			s->extend=(value/10000);
			break;
		case LWPNFAKEJUMP:
			s->fakefall=zslongToFix(value)*-100;
			break;
		case LWPNFAKEZ:
			s->fakez=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
			if(s->fakez < 0) s->fakez = 0_zf;
			break;
		case LWPNFALLCLK:
			if(s->fallclk != 0 && value == 0)
			{
				s->cs = s->o_cset;
				s->tile = s->o_tile;
			}
			else if(s->fallclk == 0 && value != 0) s->o_cset = s->cs;
			s->fallclk = vbound(value/10000,0,70);
			break;
		case LWPNFALLCMB:
			s->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case LWPNFLASH:
			s->flash=(value/10000);
			break;
		case LWPNFLASHCSET:
			(s->o_cset)|=(value/10000)<<4;
			break;
		case LWPNFLIP:
			s->flip=(value/10000);
			break;
		case LWPNFRAME:
			s->aframe=(value/10000);
			break;
		case LWPNFRAMES:
			s->frames=(value/10000);
			break;
		case LWPNGLOWRAD:
			s->glowRad = vbound(value/10000,0,255);
			break;
		case LWPNGLOWSHP:
			s->glowShape = vbound(value/10000,0,255);
			break;
		case LWPNGRAVITY:
			if(value)
				s->moveflags |= move_obeys_grav;
			else
				s->moveflags &= ~move_obeys_grav;
			break;
		case LWPNHXOFS:
			(s->hxofs)=(value/10000);
			break;
		case LWPNHXSZ:
			(s->hit_width)=(value/10000);
			break;
		case LWPNHYOFS:
			(s->hyofs)=(value/10000);
			break;
		case LWPNHYSZ:
			(s->hit_height)=(value/10000);
			break;
		case LWPNHZSZ:
			(s->hzsz)=(value/10000);
			break;
		case LWPNJUMP:
			s->fall=zslongToFix(value)*-100;
			break;
		case LWPNLEVEL:
			(s->level)=value/10000;
			break;
		case LWPNLIFTHEIGHT:
			s->lift_height = zslongToFix(value);
			break;
		case LWPNLIFTLEVEL:
			s->lift_level = vbound(value/10000,0,255);
			break;
		case LWPNLIFTTIME:
			s->lift_time = vbound(value/10000,0,255);
			break;
		case LWPNOCSET:
			(s->o_cset)|=(value/10000)&15;
			break;
		case LWPNOTILE:
			s->o_tile=(value/10000);
			s->ref_o_tile=(value/10000);
			//s->script_wrote_otile=1; //Removing this as of 26th October, 2019 -Z
			//if at some future point we WANT writing ->Tile to also overwrite ->OriginalTile,
			//then either the user will need to manually write tile, or we can add a QR and 
			// write ->tile here. 'script_wrote_otile' is out.
			break;
		case LWPNPARENT:
		{
			//int32_t pitm = (vbound(value/10000,1,(MAXITEMS-1)));
			(s->parentitem)=(vbound(value/10000,-1,(MAXITEMS-1)));
			break;
		}
		case LWPNPIERCE:
			s->pierce_count = vbound(value/10000,-1,32767);
			break;
		case LWPNPOWER:
			s->power=(value/10000);
			break;
		case LWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			s->rotation=(value/10000);
			break;
		case LWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			s->scale=(zfix)(value/100.0);
			break;
		case LWPNSCRIPT:
			(s->scrconfig.script)=vbound(value/10000,0,NUMSCRIPTWEAPONS-1);
			if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				s->scrconfig.run_args.fill(0);
			s->scrconfig.inst_init.clear();
			on_reassign_script_engine_data(ScriptType::Lwpn, ri->lwpnref);
			break;
		case LWPNSCRIPTFLIP:
			s->scriptflip=vbound((value/10000),-1,127);
			break;
		case LWPNSCRIPTTILE:
			s->scripttile=vbound((value/10000),-1,NEWMAXTILES-1);
			break;
		case LWPNSHADOWSPR:
			s->spr_shadow = vbound(value/10000, 0, MAXSPRITES-1);
			break;
		case LWPNSHADOWXOFS:
			(s->shadowxofs)=(zfix)(value/10000);
			break;
		case LWPNSHADOWYOFS:
			(s->shadowyofs)=(zfix)(value/10000);
			break;
		case LWPNSPECIAL:
			s->specialinfo=(value/10000);
			break;
		case LWPNSTEP:
		{
			// fp math is bad for replay, so always ignore this QR when replay is active.
			// TODO: can we just delete this QR? Would it actually break anything? For now,
			// just disable for replay and wait for more tests to be played with this QR
			// ignored.
			if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
			{
				s->step= zslongToFix(value / 100);
			}
			else
			{
				//old, buggy code replication, round two: Go! -Z
				//zfix val = zslongToFix(value);
				//val.doFloor();
				//s->step = ((val / 100.0).getFloat());

				//old, buggy code replication, round THREE: Go! -Z
				s->step = ((value/10000)/100.0);
			}
			break;
		}
		case LWPNTILE:
			s->tile=(value/10000);
			break;
		case LWPNTIMEOUT:
			if (s)
			{
				s->weap_timeout = vbound(value/10000,0,214748);
			}
			break;
		case LWPNTOTALDYOFFS:
			break; //READ-ONLY
		case LWPNTXSZ:
			(s->txsz)=vbound((value/10000),1,20);
			break;
		case LWPNTYPE:
			s->id=(value/10000);
			break;
		case LWPNTYSZ:
			(s->tysz)=vbound((value/10000),1,20);
			break;
		case LWPNUNBL:
			if (s)
			{
				s->unblockable = (value/10000)&WPNUNB_ALL;
			}
			break;
		case LWPNUSEDEFENCE:
			if (s)
			(s->usedefense)=vbound(value/10000,0,255);
			break;
		case LWPNUSEWEAPON:
			if (s)
			(s->useweapon)=vbound(value/10000,0,255);
			break;
		case LWPNVX:
		{
			double vy;
			double vx = (value / 10000.0);
			if (s->angular)
				vy = zc::math::Sin(s->angle)*s->step;
			else
			{
				switch(NORMAL_DIR(s->dir))
				{
					case l_up:
					case r_up:
					case up:
						vy = -1.0*s->step;
						break;
					case l_down:
					case r_down:
					case down:
						vy = s->step;
						break;

					default:
						vy = 0;
						break;
				}
			}
			s->angular = true;
			s->angle=zc::math::ArcTan2(vy, vx);
			s->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
			s->doAutoRotate();
			break;
		}
		case LWPNVY:
		{
			double vx;
			double vy = (value / 10000.0);
			if (s->angular)
				vx = zc::math::Cos(s->angle)*s->step;
			else
			{
				switch(NORMAL_DIR(s->dir))
				{
					case l_up:
					case l_down:
					case left:
						vx = -1.0*s->step;
						break;
					case r_down:
					case r_up:
					case right:
						vx = s->step;
						break;

					default:
						vx = 0;
						break;
				}
			}
			s->angular = true;
			s->angle=zc::math::ArcTan2(vy, vx);
			s->step=FFCore.Distance(0, 0, vx, vy)/10000.0;
			s->doAutoRotate();
			break;
		}
		case LWPNX:
			s->x=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
			break;
		case LWPNXOFS:
			(s->xofs)=(zfix)(value/10000);
			break;
		case LWPNY:
			s->y=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
			break;
		case LWPNYOFS:
			(s->yofs)=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			break;
		case LWPNZ:
			if (s)
			{
				s->z=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				if(s->z < 0) s->z = 0_zf;
			}
			break;
		case LWPNZOFS:
			(s->zofs)=(zfix)(value/10000);
			break;
		case LWSWHOOKED:
			break; //read-only
		case LWPN_BOUNCE_MULT:
			s->bounce_mult = zc_max(0_zf, zslongToFix(value));
			break;
		case LWPN_BOUNCE_ADD:
			s->bounce_add = zslongToFix(value);
			break;
		case LWPN_NO_TRIGGERS:
			s->disable_triggers = bool(value);
			break;
		default:
			NOTREACHED();
	}
}
