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
	int32_t ret = 0;

	switch (reg)
	{
		case EWEAPONSCRIPTUID:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->getUID());

			break;
		case EWPNANGLE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(int32_t)(s->angle*10000);

			break;
		case EWPNANGULAR:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->angular*10000;

			break;
		case EWPNASPEED:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->o_speed*10000;

			break;
		case EWPNAUTOROTATE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->autorotate*10000;

			break;
		case EWPNBEHIND:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->behind*10000;

			break;
		case EWPNCOLLDET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->scriptcoldet)*10000;

			break;
		case EWPNCSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->cs*10000;

			break;
		case EWPNDEAD:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->dead*10000;

			break;
		case EWPNDEATHDROPSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->death_spawndropset * 10000;
			}
			break;
		case EWPNDEATHIPICKUP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->death_item_pflags * 10000;
			}
			break;
		case EWPNDEATHITEM:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->death_spawnitem * 10000;
			}
			break;
		case EWPNDEATHSFX:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->death_sfx * 10000;
			}
			break;
		case EWPNDEATHSPRITE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->death_sprite * 10000;
			}
			break;
		case EWPNDEGANGLE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret=(int32_t)(s->angle*(180.0 / PI)*10000);
			}

			break;
		case EWPNDIR:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->dir*10000;

			break;
		case EWPNDRAWTYPE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->drawstyle*10000;

			break;
		case EWPNDROWNCLK:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->drownclk * 10000;
			}
			break;
		case EWPNDROWNCMB:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->drownCombo * 10000;
			}
			break;
		case EWPNENGINEANIMATE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->do_animation)*10000;

			break;
		case EWPNEXTEND:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->extend*10000;

			break;
		case EWPNFAKEJUMP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}

			break;
		case EWPNFAKEZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(s->fakez).getZLong();
				}
				else 
					ret=((int32_t)s->fakez)*10000;
			}
			break;
		case EWPNFALLCLK:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->fallclk * 10000;
			}
			break;
		case EWPNFALLCMB:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->fallCombo * 10000;
			}
			break;
		case EWPNFLASH:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->flash*10000;

			break;
		case EWPNFLASHCSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->o_cset>>4)*10000;

			break;
		case EWPNFLIP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->flip*10000;

			break;
		case EWPNFRAME:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->aframe*10000;

			break;
		case EWPNFRAMES:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->frames*10000;

			break;
		case EWPNGLOWRAD:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->glowRad * 10000;
			}
			break;
		case EWPNGLOWSHP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->glowShape * 10000;
			}
			break;
		case EWPNGRAVITY:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=((s->moveflags & move_obeys_grav) ? 10000 : 0);

			break;
		case EWPNHXOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->hxofs)*10000;

			break;
		case EWPNHXSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->hit_width)*10000;

			break;
		case EWPNHYOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->hyofs)*10000;

			break;
		case EWPNHYSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->hit_height)*10000;

			break;
		case EWPNHZSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->hzsz)*10000;

			break;
		case EWPNJUMP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}

			break;
		case EWPNLEVEL:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->level*10000;

			break;
		case EWPNLIFTHEIGHT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->lift_height.getZLong();
			}
			break;
		case EWPNLIFTLEVEL:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->lift_level * 10000;
			}
			break;
		case EWPNLIFTTIME:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->lift_time * 10000;
			}
			break;
		case EWPNOCSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->o_cset&15)*10000;

			break;
		case EWPNOTILE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->o_tile*10000;

			break;
		case EWPNPARENT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret= ((get_qr(qr_OLDEWPNPARENT)) ? (s->parentid)*10000 : (s->parentid));

			break;
		case EWPNPARENTUID:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret = s->parent ? s->parent->getUID() : 0;

			break;
		case EWPNPIERCE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->pierce_count * 10000;
			}
			break;
		case EWPNPOWER:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->power*10000;

			break;
		case EWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'");
				break;
			}
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->rotation*10000;

			break;
		case EWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=((int32_t)s->scale)*100.0;

			break;
		case EWPNSCRIPT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->script)*10000;

			break;
		case EWPNSCRIPTFLIP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->scriptflip*10000;

			break;
		case EWPNSCRIPTTILE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->scripttile*10000;

			break;
		case EWPNSHADOWSPR:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->spr_shadow * 10000;
			}
			break;
		case EWPNSHADOWXOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=((int32_t)(s->shadowxofs))*10000;

			break;
		case EWPNSHADOWYOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=((int32_t)(s->shadowyofs))*10000;

			break;
		case EWPNSTEP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if ( get_qr(qr_STEP_IS_FLOAT) || replay_is_active() )
				{
					ret=s->step.getZLong() * 100;
				}
				//old, buggy code replication, round two: Go! -Z
				//else ret = ( ( ( s->step ) * 100.0 ).getZLong() );
				//old, buggy code replication, round FOUR: Go! -Z
				else ret = (int32_t)((float)s->step * 1000000.0);
			}
			//else 
			//{
				//old, buggy code replication, round THREE: Go! -Z
			//	double tmp = ( s->step.getFloat() ) * 1000000.0;
			//	ret = int32_t(tmp);
			//}
			break;
		case EWPNTILE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->tile*10000;

			break;
		case EWPNTIMEOUT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->weap_timeout * 10000;
			}
			break;
		case EWPNTOTALDYOFFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret = ((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset))
					+ ((s->switch_hooked && Hero.switchhookstyle == swRISE)
						? -(8-(abs(Hero.switchhookclk-32)/4)) : 0) * 10000);
			break;
		case EWPNTXSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->txsz)*10000;

			break;
		case EWPNTYPE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=s->id*10000;

			break;
		case EWPNTYSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=(s->tysz)*10000;

			break;
		case EWPNUNBL:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->unblockable * 10000;
			}
			break;
		case EWPNVX:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
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
			}

			break;
		case EWPNVY:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
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
			}

			break;
		case EWPNX:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(s->x).getZLong();
				}
				else 
					ret=((int32_t)s->x)*10000;
			}
			break;
		case EWPNXOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=((int32_t)(s->xofs))*10000;

			break;
		case EWPNY:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(s->y).getZLong();
				}
				else 
					 ret=((int32_t)s->y)*10000;
			}
			break;
		case EWPNYOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))*10000;

			break;
		case EWPNZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(s->z).getZLong();
				}
				else 
					ret=((int32_t)s->z)*10000;
			}
			break;
		case EWPNZOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				ret=((int32_t)(s->zofs))*10000;

			break;
		case EWSWHOOKED:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				ret = s->switch_hooked ? 10000 : 0;
			}
			break;
		case SPRITEMAXEWPN:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			ret = Ewpns.getMax() * 10000;
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void eweapon_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case EWPNANGLE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->angle=(double)(value/10000.0);
				s->doAutoRotate();
			}

			break;
		case EWPNANGULAR:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->angular=(value!=0);
				s->doAutoRotate(false, true);
			}

			break;
		case EWPNASPEED:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->o_speed=(value/10000);

			break;
		case EWPNAUTOROTATE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->autorotate=(value!=0);
				s->doAutoRotate(false, true);
			}

			break;
		case EWPNBEHIND:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->behind=(value!=0);

			break;
		case EWPNCOLLDET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->scriptcoldet)=value;

			break;
		case EWPNCSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->cs=(value/10000)&15;

			break;
		case EWPNDEAD:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				auto dead = value/10000;
				s->dead=dead;
				if(dead != 0) s->weapon_dying_frame = false;
			}

			break;
		case EWPNDEATHDROPSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->death_spawndropset = vbound(value/10000,-1,MAXITEMDROPSETS-1);
			}
			break;
		case EWPNDEATHIPICKUP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->death_item_pflags = value/10000;
			}
			break;
		case EWPNDEATHITEM:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->death_spawnitem = vbound(value/10000,-1,MAXITEMS-1);
			}
			break;
		case EWPNDEATHSFX:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->death_sfx = vbound(value/10000,0,MAX_SFX);
			}
			break;
		case EWPNDEATHSPRITE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->death_sprite = vbound(value/10000,-255,MAXSPRITES-1);
			}
			break;
		case EWPNDEGANGLE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				double rangle = (value / 10000.0) * (PI / 180.0);
				s->angle=(double)(rangle);
				s->doAutoRotate();
			}

			break;
		case EWPNDIR:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->dir=(value/10000);
				s->doAutoRotate(true);
			}

			break;
		case EWPNDRAWTYPE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->drawstyle=(value/10000);

			break;
		case EWPNDROWNCLK:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if(s->drownclk != 0 && value == 0)
				{
					s->cs = s->o_cset;
					s->tile = s->o_tile;
				}
				else if(s->drownclk == 0 && value != 0) s->o_cset = s->cs;
				s->drownclk = vbound(value/10000,0,70);
			}
			break;
		case EWPNDROWNCMB:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case EWPNENGINEANIMATE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->do_animation)=value;

			break;
		case EWPNEXTEND:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->extend=(value/10000);

			break;
		case EWPNFAKEJUMP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->fakefall=zslongToFix(value)*-100;

			break;
		case EWPNFAKEZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->fakez=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				if(s->fakez < 0) s->fakez = 0_zf;
			}

			break;
		case EWPNFALLCLK:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if(s->fallclk != 0 && value == 0)
				{
					s->cs = s->o_cset;
					s->tile = s->o_tile;
				}
				else if(s->fallclk == 0 && value != 0) s->o_cset = s->cs;
				s->fallclk = vbound(value/10000,0,70);
			}
			break;
		case EWPNFALLCMB:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case EWPNFLASH:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->flash=(value/10000);

			break;
		case EWPNFLASHCSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->o_cset)|=(value/10000)<<4;

			break;
		case EWPNFLIP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->flip=(value/10000);

			break;
		case EWPNFRAME:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->aframe=(value/10000);

			break;
		case EWPNFRAMES:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->frames=(value/10000);

			break;
		case EWPNGLOWRAD:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->glowRad = vbound(value/10000,0,255);
			}
			break;
		case EWPNGLOWSHP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->glowShape = vbound(value/10000,0,255);
			}
			break;
		case EWPNGRAVITY:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				if(value)
					s->moveflags |= move_obeys_grav;
				else
					s->moveflags &= ~move_obeys_grav;
			}
			break;
		case EWPNHXOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->hxofs)=(value/10000);

			break;
		case EWPNHXSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->hit_width)=(value/10000);

			break;
		case EWPNHYOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->hyofs)=(value/10000);

			break;
		case EWPNHYSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->hit_height)=(value/10000);

			break;
		case EWPNHZSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->hzsz)=(value/10000);

			break;
		case EWPNJUMP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->fall=zslongToFix(value)*-100;

			break;
		case EWPNLEVEL:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->level=(value/10000);

			break;
		case EWPNLIFTHEIGHT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->lift_height = zslongToFix(value);
			}
			break;
		case EWPNLIFTLEVEL:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->lift_level = vbound(value/10000,0,255);
			}
			break;
		case EWPNLIFTTIME:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->lift_time = vbound(value/10000,0,255);
			}
			break;
		case EWPNOCSET:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->o_cset)|=(value/10000)&15;

			break;
		case EWPNOTILE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->o_tile=(value/10000);
				s->ref_o_tile=(value/10000);
			}

			break;
		case EWPNPARENT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->parentid)= ( (get_qr(qr_OLDEWPNPARENT)) ? value / 10000 : value );

			break;
		case EWPNPARENTUID:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->setParent(sprite::getByUID(value));
			break;
		case EWPNPIERCE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->pierce_count = vbound(value/10000,-1,32767);
			}
			break;
		case EWPNPOWER:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->power=(value/10000);

			break;
		case EWPNROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'");
				break;
			}
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->rotation=(value/10000);

			break;
		case EWPNSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->scale=(zfix)(value/100.0);

			break;
		case EWPNSCRIPT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				(s->script)=vbound(value/10000,0,NUMSCRIPTWEAPONS-1);
				if ( get_qr(qr_CLEARINITDONSCRIPTCHANGE))
				{
					for(int32_t q=0; q<8; q++)
						(s->initD[q]) = 0;
				}
				on_reassign_script_engine_data(ScriptType::Ewpn, ri->ewpnref);
			}
			break;
		case EWPNSCRIPTFLIP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->scriptflip=vbound((value/10000),-1, 127);

			break;
		case EWPNSCRIPTTILE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->scripttile=vbound((value/10000),-1, NEWMAXTILES-1);

			break;
		case EWPNSHADOWSPR:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->spr_shadow = vbound(value/10000, 0, MAXSPRITES-1);
			}
			break;
		case EWPNSHADOWXOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->shadowxofs)=(zfix)(value/10000);

			break;
		case EWPNSHADOWYOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->shadowyofs)=(zfix)(value/10000);

			break;
		case EWPNSTEP:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
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
			}

			break;
		case EWPNTILE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->tile=(value/10000);

			break;
		case EWPNTIMEOUT:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->weap_timeout = vbound(value/10000,0,214748);
			}
			break;
		case EWPNTXSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->txsz)=vbound((value/10000),1,20);

			break;
		case EWPNTYPE:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->id=(value/10000);

			break;
		case EWPNTYSZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->tysz)=vbound((value/10000),1,20);

			break;
		case EWPNUNBL:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->unblockable = (value/10000)&WPNUNB_ALL;
			}
			break;
		case EWPNVX:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
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
				s->angle=atan2(vy, vx);
				s->step=FFCore.Distance(0, 0, vx, vy)/10000;
				s->doAutoRotate();
			}

			break;
		case EWPNVY:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
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
				s->angle=atan2(vy, vx);
				s->step=FFCore.Distance(0, 0, vx, vy)/10000;
				s->doAutoRotate();
			}

			break;
		case EWPNX:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->x = (get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));

			break;
		case EWPNXOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->xofs)=(zfix)(value/10000);

			break;
		case EWPNY:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				s->y = (get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000));

			break;
		case EWPNYOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->yofs)=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);

			break;
		case EWPNZ:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
			{
				s->z=get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				if(s->z < 0) s->z = 0_zf;
			}

			break;
		case EWPNZOFS:
			if(auto s=checkEWpn(GET_REF(ewpnref)))
				(s->zofs)=(zfix)(value/10000);

			break;
		case EWSWHOOKED:
			break; //read-only
		case SPRITEMAXEWPN:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			Ewpns.setMax(vbound((value/10000),1,MAX_EWPN_SPRITES));
			break;
		}

		default:
			NOTREACHED();
	}
}
