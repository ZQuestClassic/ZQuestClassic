#include "zc/scripting/types/eweapon.h"

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

int32_t eweapon_get_register(int32_t reg)
{
	if (reg == SPRITEMAXEWPN)
	{
		//No bounds check, as this is a universal function and works from NULL pointers!
		return Ewpns.getMax() * 10000;
	}

	int32_t ret = 0;
	weapon* s = checkEWpn(GET_REF(ewpnref));
	if (!s)
		return ret;

	switch (reg)
	{
		case EWEAPONSCRIPTUID:
			ret=(s->getUID());
			break;
		case EWPNANGLE:
			ret=(int32_t)(s->angle*10000);
			break;
		case EWPNANGULAR:
			ret=s->angular*10000;
			break;
		case EWPNASPEED:
			ret=s->o_speed*10000;
			break;
		case EWPNAUTOROTATE:
			ret=s->autorotate*10000;
			break;
		case EWPNBEHIND:
			ret=s->behind*10000;
			break;
		case EWPNCOLLDET:
			ret = s->script_no_colldet ? 0 : 10000;
			break;
		case EWPNCSET:
			ret=s->cs*10000;
			break;
		case EWPNDEAD:
			ret=s->dead*10000;
			break;
		case EWPNDEATHDROPSET:
			ret = s->death_spawndropset * 10000;
			break;
		case EWPNDEATHIPICKUP:
			ret = s->death_item_pflags * 10000;
			break;
		case EWPNDEATHITEM:
			ret = s->death_spawnitem * 10000;
			break;
		case EWPNDEATHSFX:
			ret = s->death_sfx * 10000;
			break;
		case EWPNDEATHSPRITE:
			ret = s->death_sprite * 10000;
			break;
		case EWPNDEGANGLE:
			if (s)
			{
				ret=(int32_t)(s->angle*(180.0 / PI)*10000);
			}
			break;
		case EWPNDIR:
			ret=s->dir*10000;
			break;
		case EWPNDRAWTYPE:
			ret=s->drawstyle*10000;
			break;
		case EWPNDROWNCLK:
			ret = s->drownclk * 10000;
			break;
		case EWPNDROWNCMB:
			ret = s->drownCombo * 10000;
			break;
		case EWPNENGINEANIMATE:
			ret=(s->do_animation)*10000;
			break;
		case EWPNEXTEND:
			ret=s->extend*10000;
			break;
		case EWPNFAKEJUMP:
			ret = s->fakefall.getZLong() / -100;
			if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			break;
		case EWPNFAKEZ:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->fakez).getZLong();
			}
			else 
				ret=((int32_t)s->fakez)*10000;
			break;
		case EWPNFALLCLK:
			ret = s->fallclk * 10000;
			break;
		case EWPNFALLCMB:
			ret = s->fallCombo * 10000;
			break;
		case EWPNFLASH:
			ret=s->flash*10000;
			break;
		case EWPNFLASHCSET:
			ret=(s->o_cset>>4)*10000;
			break;
		case EWPNFLIP:
			ret=s->flip*10000;
			break;
		case EWPNFRAME:
			ret=s->aframe*10000;
			break;
		case EWPNFRAMES:
			ret=s->frames*10000;
			break;
		case EWPNGLOWRAD:
			ret = s->glowRad * 10000;
			break;
		case EWPNGLOWSHP:
			ret = s->glowShape * 10000;
			break;
		case EWPNGRAVITY:
			ret=((s->moveflags & move_obeys_grav) ? 10000 : 0);
			break;
		case EWPNHXOFS:
			ret=(s->hxofs)*10000;
			break;
		case EWPNHXSZ:
			ret=(s->hit_width)*10000;
			break;
		case EWPNHYOFS:
			ret=(s->hyofs)*10000;
			break;
		case EWPNHYSZ:
			ret=(s->hit_height)*10000;
			break;
		case EWPNHZSZ:
			ret=(s->hzsz)*10000;
			break;
		case EWPNJUMP:
			ret = s->fall.getZLong() / -100;
			if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			break;
		case EWPNLEVEL:
			ret=s->level*10000;
			break;
		case EWPNLIFTHEIGHT:
			ret = s->lift_height.getZLong();
			break;
		case EWPNLIFTLEVEL:
			ret = s->lift_level * 10000;
			break;
		case EWPNLIFTTIME:
			ret = s->lift_time * 10000;
			break;
		case EWPNOCSET:
			ret=(s->o_cset&15)*10000;
			break;
		case EWPNOTILE:
			ret=s->o_tile*10000;
			break;
		case EWPNPARENT:
			ret= ((get_qr(qr_OLDEWPNPARENT)) ? (s->parentid)*10000 : (s->parentid));
			break;
		case EWPNPARENTUID:
			ret = s->parent ? s->parent->getUID() : 0;
			break;
		case EWPNPIERCE:
			ret = s->pierce_count * 10000;
			break;
		case EWPNPOWER:
			ret=s->power*10000;
			break;
		case EWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'");
				break;
			}
			ret=s->rotation*10000;
			break;
		case EWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			ret=((int32_t)s->scale)*100.0;
			break;
		case EWPNSCRIPT:
			ret=(s->scrconfig.script)*10000;
			break;
		case EWPNSCRIPTFLIP:
			ret=s->scriptflip*10000;
			break;
		case EWPNSCRIPTTILE:
			ret=s->scripttile*10000;
			break;
		case EWPNSHADOWSPR:
			ret = s->spr_shadow * 10000;
			break;
		case EWPNSHADOWXOFS:
			ret=((int32_t)(s->shadowxofs))*10000;
			break;
		case EWPNSHADOWYOFS:
			ret=((int32_t)(s->shadowyofs))*10000;
			break;
		case EWPNSTEP:
			if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
			{
				ret=s->step.getZLong() * 100;
			}
			else ret = (int32_t)((float)s->step * 1000000.0);
			break;
		case EWPNTILE:
			ret=s->tile*10000;
			break;
		case EWPNTIMEOUT:
			ret = s->weap_timeout * 10000;
			break;
		case EWPNTOTALDYOFFS:
			ret = ((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))
					+ ((s->switch_hooked && Hero.switchhookstyle == swRISE)
						? -(8-(abs(Hero.switchhookclk-32)/4)) : 0) * 10000);
			break;
		case EWPNTXSZ:
			ret=(s->txsz)*10000;
			break;
		case EWPNTYPE:
			ret=s->id*10000;
			break;
		case EWPNTYSZ:
			ret=(s->tysz)*10000;
			break;
		case EWPNUNBL:
			ret = s->unblockable * 10000;
			break;
		case EWPNVX:
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
					case r_up:
					case r_down:
					case right:
						ret = int32_t(10000.0*s->step);
						break;

					default:
						ret = 0;
						break;
				}
			}
			break;
		case EWPNVY:
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
		case EWPNX:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->x).getZLong();
			}
			else 
				ret=((int32_t)s->x)*10000;
			break;
		case EWPNXOFS:
			ret=((int32_t)(s->xofs))*10000;
			break;
		case EWPNY:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->y).getZLong();
			}
			else 
				 ret=((int32_t)s->y)*10000;
			break;
		case EWPNYOFS:
			ret=((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))*10000;
			break;
		case EWPNZ:
			if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
			{
				ret=(s->z).getZLong();
			}
			else 
				ret=((int32_t)s->z)*10000;
			break;
		case EWPNZOFS:
			ret=((int32_t)(s->zofs))*10000;
			break;
		case EWSWHOOKED:
			ret = s->switch_hooked ? 10000 : 0;
			break;
		case EWPN_BOUNCE_MULT:
			ret = s->bounce_mult.getZLong();
			break;
		case EWPN_BOUNCE_ADD:
			ret = s->bounce_add.getZLong();
			break;
		case EWPN_NO_TRIGGERS:
			ret = s->disable_triggers ? 10000 : 0;
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void eweapon_set_register(int32_t reg, int32_t value)
{
	if (reg == SPRITEMAXEWPN)
	{
		//No bounds check, as this is a universal function and works from NULL pointers!
		Ewpns.setMax(vbound((value/10000),1,MAX_EWPN_SPRITES));
		return;
	}

	weapon* s = checkEWpn(GET_REF(ewpnref));
	if (!s)
		return;

	switch (reg)
	{
		case EWPNANGLE:
			s->angle=(double)(value/10000.0);
			s->doAutoRotate();
			break;
		case EWPNANGULAR:
			s->angular=(value!=0);
			s->doAutoRotate(false, true);
			break;
		case EWPNASPEED:
			s->o_speed=(value/10000);
			break;
		case EWPNAUTOROTATE:
			s->autorotate=(value!=0);
			s->doAutoRotate(false, true);
			break;
		case EWPNBEHIND:
			s->behind=(value!=0);
			break;
		case EWPNCOLLDET:
			s->script_no_colldet = value ? 0 : -1;
			break;
		case EWPNCSET:
			s->cs=(value/10000)&15;
			break;
		case EWPNDEAD:
		{
			auto dead = value/10000;
			s->dead=dead;
			if(dead != 0) s->weapon_dying_frame = false;
			break;
		}
		case EWPNDEATHDROPSET:
			s->death_spawndropset = vbound(value/10000,-1,MAXITEMDROPSETS-1);
			break;
		case EWPNDEATHIPICKUP:
			s->death_item_pflags = value/10000;
			break;
		case EWPNDEATHITEM:
			s->death_spawnitem = vbound(value/10000,-1,MAXITEMS-1);
			break;
		case EWPNDEATHSFX:
			s->death_sfx = vbound(value/10000,0,MAX_SFX);
			break;
		case EWPNDEATHSPRITE:
			s->death_sprite = vbound(value/10000,-255,MAXSPRITES-1);
			break;
		case EWPNDEGANGLE:
		{
			double rangle = (value / 10000.0) * (PI / 180.0);
			s->angle=(double)(rangle);
			s->doAutoRotate();
			break;
		}
		case EWPNDIR:
			s->dir=(value/10000);
			s->doAutoRotate(true);
			break;
		case EWPNDRAWTYPE:
			s->drawstyle=(value/10000);
			break;
		case EWPNDROWNCLK:
			if(s->drownclk != 0 && value == 0)
			{
				s->cs = s->o_cset;
				s->tile = s->o_tile;
			}
			else if(s->drownclk == 0 && value != 0) s->o_cset = s->cs;
			s->drownclk = vbound(value/10000,0,70);
			break;
		case EWPNDROWNCMB:
			s->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case EWPNENGINEANIMATE:
			(s->do_animation)=value;
			break;
		case EWPNEXTEND:
			s->extend=(value/10000);
			break;
		case EWPNFAKEJUMP:
			s->fakefall=zslongToFix(value)*-100;
			break;
		case EWPNFAKEZ:
			s->fakez=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
			if(s->fakez < 0) s->fakez = 0_zf;
			break;
		case EWPNFALLCLK:
			if(s->fallclk != 0 && value == 0)
			{
				s->cs = s->o_cset;
				s->tile = s->o_tile;
			}
			else if(s->fallclk == 0 && value != 0) s->o_cset = s->cs;
			s->fallclk = vbound(value/10000,0,70);
			break;
		case EWPNFALLCMB:
			s->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			break;
		case EWPNFLASH:
			s->flash=(value/10000);
			break;
		case EWPNFLASHCSET:
			(s->o_cset)|=(value/10000)<<4;
			break;
		case EWPNFLIP:
			s->flip=(value/10000);
			break;
		case EWPNFRAME:
			s->aframe=(value/10000);
			break;
		case EWPNFRAMES:
			s->frames=(value/10000);
			break;
		case EWPNGLOWRAD:
			s->glowRad = vbound(value/10000,0,255);
			break;
		case EWPNGLOWSHP:
			s->glowShape = vbound(value/10000,0,255);
			break;
		case EWPNGRAVITY:
			if(value)
				s->moveflags |= move_obeys_grav;
			else
				s->moveflags &= ~move_obeys_grav;
			break;
		case EWPNHXOFS:
			(s->hxofs)=(value/10000);
			break;
		case EWPNHXSZ:
			(s->hit_width)=(value/10000);
			break;
		case EWPNHYOFS:
			(s->hyofs)=(value/10000);
			break;
		case EWPNHYSZ:
			(s->hit_height)=(value/10000);
			break;
		case EWPNHZSZ:
			(s->hzsz)=(value/10000);
			break;
		case EWPNJUMP:
			s->fall=zslongToFix(value)*-100;
			break;
		case EWPNLEVEL:
			s->level=(value/10000);
			break;
		case EWPNLIFTHEIGHT:
			s->lift_height = zslongToFix(value);
			break;
		case EWPNLIFTLEVEL:
			s->lift_level = vbound(value/10000,0,255);
			break;
		case EWPNLIFTTIME:
			s->lift_time = vbound(value/10000,0,255);
			break;
		case EWPNOCSET:
			(s->o_cset)|=(value/10000)&15;
			break;
		case EWPNOTILE:
			s->o_tile=(value/10000);
			s->ref_o_tile=(value/10000);
			break;
		case EWPNPARENT:
			(s->parentid)= ( (get_qr(qr_OLDEWPNPARENT)) ? value / 10000 : value );
			break;
		case EWPNPARENTUID:
			s->setParent(sprite::getByUID(value));
			break;
		case EWPNPIERCE:
			s->pierce_count = vbound(value/10000,-1,32767);
			break;
		case EWPNPOWER:
			s->power=(value/10000);
			break;
		case EWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'");
				break;
			}
			s->rotation=(value/10000);
			break;
		case EWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			s->scale=(zfix)(value/100.0);
			break;
		case EWPNSCRIPT:
			if (s)
			{
				(s->scrconfig.script)=vbound(value/10000,0,NUMSCRIPTWEAPONS-1);
				if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
					s->scrconfig.run_args.fill(0);
				s->scrconfig.inst_init.clear();
				on_reassign_script_engine_data(ScriptType::Ewpn, ri->ewpnref);
			}
			break;
		case EWPNSCRIPTFLIP:
			s->scriptflip=vbound((value/10000),-1, 127);
			break;
		case EWPNSCRIPTTILE:
			s->scripttile=vbound((value/10000),-1, NEWMAXTILES-1);
			break;
		case EWPNSHADOWSPR:
			if (s)
			{
				s->spr_shadow = vbound(value/10000, 0, MAXSPRITES-1);
			}
			break;
		case EWPNSHADOWXOFS:
			(s->shadowxofs)=(zfix)(value/10000);
			break;
		case EWPNSHADOWYOFS:
			(s->shadowyofs)=(zfix)(value/10000);
			break;
		case EWPNSTEP:
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
		case EWPNTILE:
			s->tile=(value/10000);
			break;
		case EWPNTIMEOUT:
			if (s)
			{
				s->weap_timeout = vbound(value/10000,0,214748);
			}
			break;
		case EWPNTXSZ:
			(s->txsz)=vbound((value/10000),1,20);
			break;
		case EWPNTYPE:
			s->id=(value/10000);
			break;
		case EWPNTYSZ:
			(s->tysz)=vbound((value/10000),1,20);
			break;
		case EWPNUNBL:
			s->unblockable = (value/10000)&WPNUNB_ALL;
			break;
		case EWPNVX:
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
			s->step=FFCore.Distance(0, 0, vx, vy)/10000;
			s->doAutoRotate();
			break;
		}
		case EWPNVY:
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
			s->step=FFCore.Distance(0, 0, vx, vy)/10000;
			s->doAutoRotate();
			break;
		}
		case EWPNX:
			s->x = (get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
			break;
		case EWPNXOFS:
			(s->xofs)=(zfix)(value/10000);
			break;
		case EWPNY:
			s->y = (get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));
			break;
		case EWPNYOFS:
			(s->yofs)=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			break;
		case EWPNZ:
			s->z=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
			if(s->z < 0) s->z = 0_zf;
			break;
		case EWPNZOFS:
			(s->zofs)=(zfix)(value/10000);
			break;
		case EWSWHOOKED:
			break; //read-only
		case EWPN_BOUNCE_MULT:
			s->bounce_mult = zc_max(0_zf, zslongToFix(value));
			break;
		case EWPN_BOUNCE_ADD:
			s->bounce_add = zslongToFix(value);
			break;
		case EWPN_NO_TRIGGERS:
			s->disable_triggers = bool(value);
			break;

		default:
			NOTREACHED();
	}
}
