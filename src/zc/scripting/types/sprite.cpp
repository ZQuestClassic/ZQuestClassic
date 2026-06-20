#include "zc/scripting/types/sprite.h"

#include "base/check.h"
#include "base/general.h"
#include "core/qrs.h"
#include "core/zdefs.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "hero_tiles.h"
#include "zc/scripting/arrays.h"

#include <cstdint>

extern refInfo *ri;

static bool is_player(int ref)
{
	return ref == 1;
}

static bool is_player(sprite* s)
{
	return s->uid == 1;
}

static bool is_enemy(sprite* s)
{
	return dynamic_cast<enemy*>(s);
}

static bool enemy_has_hero(sprite* s)
{
	if (auto e = dynamic_cast<enemy*>(s); e)
	{
		if (e->type == eeWALLM || e->type == eeWALK)
			return e->hashero;
	}

	return false;
}

static sprite* get_sprite(int uid)
{
	return ResolveBaseSprite(uid);
}

static bool as_float(sprite* s)
{
	return get_qr(qr_SPRITEXY_IS_FLOAT) || dynamic_cast<ffcdata*>(s);
}

int32_t sprite_get_register(int32_t reg)
{
	sprite* s = get_sprite(GET_REF(spriteref));

	switch (reg)
	{
		case SPRITE_SPAWN_SCREEN:
		{
			if (is_player(GET_REF(spriteref)))
				return cur_screen * 10000;

			if (s)
				return s->screen_spawned * 10000;
			return 0;
		}
		case SPRITE_CURRENT_SCREEN:
		{
			if (s)
				return s->current_screen * 10000;
			return 0;
		}
		case SPRITE_X:
		{
			if (auto itm = dynamic_cast<item*>(s))
			{
				zfix x;
				auto const& fairy_item = itemsbuf.get(itm->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(itm->fairyUID);
					x = fairy ? fairy->x : itm->x;
				}
				else
				{
					x = itm->x;
				}

				return as_float(itm) ? x.getZLong() : int(x) * 10000;
			}

			if (s)
				return as_float(s) ? s->x.getZLong() : int(s->x) * 10000;
			return 0;
		}
		case SPRITE_Y:
		{
			if (auto itm = dynamic_cast<item*>(s))
			{
				zfix y;
				auto const& fairy_item = itemsbuf.get(itm->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(itm->fairyUID);
					y = fairy ? fairy->y : itm->y;
				}
				else
				{
					y = itm->y;
				}

				return as_float(itm) ? y.getZLong() : int(y) * 10000;
			}

			if (s)
				return as_float(s) ? s->y.getZLong() : int(s->y) * 10000;
			return 0;
		}
		case SPRITE_Z:
		{
			if (s)
				return as_float(s) ? s->z.getZLong() : int(s->z) * 10000;
			return 0;
		}
		case SPRITE_FAKE_Z:
		{
			if (s)
				return as_float(s) ? s->fakez.getZLong() : int(s->fakez) * 10000;
			return 0;
		}
		case SPRITE_X_OFFSET:
		{
			if (s)
				return s->xofs.getZLong();
			return 0;
		}
		case SPRITE_Y_OFFSET:
		{
			if (s)
				return int(s->yofs - (get_qr(qr_OLD_DRAWOFFSET) ? playing_field_offset : original_playing_field_offset)) * 10000;
			return 0;
		}
		case SPRITE_Z_OFFSET:
		{
			if (s)
				return s->zofs.getZLong();
			return 0;
		}
		case SPRITE_ROTATION:
		{
			if (get_qr(qr_OLDSPRITEDRAWS))
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				return 0;
			}

			if (s)
				return s->rotation * 10000;
			return 0;
		}
		case SPRITE_DIR:
		{
			if (s)
				return s->dir * 10000;
			return 0;
		}
		case SPRITE_INVIS_CLK:
			if (s)
				return s->invis_timer * 10000;
			return 0;
		case SPRITE_TILE:
		{
			if (s)
				return s->tile * 10000;
			return 0;
		}
		case SPRITE_SCRIPT_TILE:
		{
			// TODO: use sprite::scriptile
			if (is_player(GET_REF(spriteref)))
				return script_hero_sprite * 10000;

			if (s)
				return s->scripttile * 10000;
			return 0;
		}
		case SPRITE_TILE_W:
		{
			if (s)
				return s->txsz * 10000;
			return 0;
		}
		case SPRITE_TILE_H:
		{
			if (s)
				return s->tysz * 10000;
			return 0;
		}
		case SPRITE_CSET:
		{
			if (s)
			{
				if (auto ffc = dynamic_cast<ffcdata*>(s))
					return ffc->cset * 10000;

				if (auto i = dynamic_cast<item*>(s))
					return (i->o_cset & 0xF) * 10000;

				return (s->cs & 0xF) * 10000;
			}
			return 0;
		}
		case SPRITE_SCALE:
		{
			if (get_qr(qr_OLDSPRITEDRAWS))
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				return 0;
			}

			if (s)
				return s->scale * 100;
			return 0;
		}
		case SPRITE_DRAW_STYLE:
		{
			if (s)
				return s->drawstyle * 10000;
			return 0;
		}
		case SPRITE_JUMP:
		{
			if (is_player(GET_REF(spriteref)))
			{
				return Hero.getJump().getZLong();
			}

			if (s)
			{
				int32_t ret = s->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED) && !is_player(GET_REF(spriteref))) ret = trunc(ret / 10000) * 10000;
				return ret;
			}
			return 0;
		}
		case SPRITE_FAKE_JUMP:
		{
			if (is_player(GET_REF(spriteref)))
			{
				return Hero.getFakeJump().getZLong() / -100;
			}

			if (s)
			{
				int32_t ret = s->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED) && !is_player(GET_REF(spriteref))) ret = trunc(ret / 10000) * 10000;
				return ret;
			}
			return 0;
		}
		case SPRITE_GRAVITY:
		{
			if (s)
				return (s->moveflags & move_obeys_grav) ? 10000 : 0;
			return 0;
		}
		case SPRITE_GRAVITY_STRENGTH:
		{
			if (s)
				return s->get_gravity(true).getZLong();
			return 0;
		}
		case SPRITE_TERMINAL_VELOCITY:
		{
			if (s)
				return s->get_terminalv(true).getZLong();
			return 0;
		}
		case SPRITE_CUSTOM_GRAVITY_STRENGTH:
		{
			if (s)
				return s->custom_gravity.getZLong();
			return 0;
		}
		case SPRITE_CUSTOM_TERMINAL_VELOCITY:
		{
			if (s)
				return s->custom_terminal_v.getZLong();
			return 0;
		}
		case SPRITE_FLIP:
		{
			if (s)
				return s->flip * 10000;
			return 0;
		}
		case SPRITE_SCRIPT_FLIP:
		{
			// TODO: use sprite::scriptflip
			if (is_player(GET_REF(spriteref)))
				return script_hero_flip * 10000;
			if (s)
				return s->scriptflip * 10000;
			return 0;
		}
		case SPRITE_ENGINE_ANIMATE:
		{
			if (s)
				return s->do_animation * 10000;
			return 0;
		}
		case SPRITE_EXTEND:
		{
			if (s)
				return s->extend * 10000;
			return 0;
		}
		case SPRITE_HIT_WIDTH:
		{
			if (s)
				return s->hit_width * 10000;
			return 0;
		}
		case SPRITE_HIT_HEIGHT:
		{
			if (s)
				return s->hit_height * 10000;
			return 0;
		}
		case SPRITE_HIT_ZHEIGHT:
		{
			if (s)
				return s->hzsz * 10000;
			return 0;
		}
		case SPRITE_HIT_OFFSET_X:
		{
			if (s)
				return s->hxofs * 10000;
			return 0;
		}
		case SPRITE_HIT_OFFSET_Y:
		{
			if (s)
				return s->hyofs * 10000;
			return 0;
		}
		case SPRITE_FALL_CLK:
		{
			if (s)
				return s->fallclk * 10000;
			return 0;
		}
		case SPRITE_FALL_CMB:
		{
			if (s)
				return s->fallCombo * 10000;
			return 0;
		}
		case SPRITE_LIGHT_RADIUS:
		{
			if (s)
				return s->glowRad * 10000;
			return 0;
		}
		case SPRITE_LIGHT_SHAPE:
		{
			if (s)
				return s->glowShape * 10000;
			return 0;
		}
		case SPRITE_LIGHT_OFFSET:
		{
			if (s)
				return s->glowOffset * 10000;
			return 0;
		}
		case SPRITE_NOCOLL_CLK:
		{
			if (s)
				return s->script_no_colldet * 10000;
			break;
		}
		case SPRITE_SWHOOKED:
		{
			if (s)
				return s->switch_hooked * 10000;
			return 0;
		}
		case SPRITE_SHADOW_SPR:
		{
			if (s)
				return s->spr_shadow * 10000;
			return 0;
		}
		case SPRITE_DROWN_CLK:
		{
			if (s)
				return s->drownclk * 10000;
			return 0;
		}
		case SPRITE_DROWN_CMB:
		{
			if (s)
				return s->drownCombo * 10000;
			return 0;
		}
		case SPRITE_SHADOW_XOFS:
		{
			if (s)
				return s->shadowxofs * 10000;
			return 0;
		}
		case SPRITE_SHADOW_YOFS:
		{
			if (s)
				return s->shadowyofs * 10000;
			return 0;
		}
		case SPRITE_VIEWPORT_SUSPEND_RANGE:
		{
			if (s)
				return s->viewport_suspend_range * 10000;
			return 0;
		}
		case SPRITE_VIEWPORT_DESPAWN_RANGE:
		{
			if (s)
				return s->viewport_despawn_range * 10000;
			return 0;
		}

		default: NOTREACHED();
	}

	NOTREACHED();
}

void sprite_set_register(int32_t reg, int32_t value)
{
	sprite* s = get_sprite(GET_REF(spriteref));

	switch (reg)
	{
		case SPRITE_SPAWN_SCREEN:
		{
			if (is_player(GET_REF(spriteref)))
				break;
			if (dynamic_cast<ffcdata*>(s))
				break;

			if (s)
				s->screen_spawned = value / 10000;
			break;
		}
		case SPRITE_X:
		{
			if (is_player(GET_REF(spriteref)))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setXfix(zslongToFix(value));
				else
					Hero.setX(value / 10000);
				break;
			}

			if (s)
			{
				s->x = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (enemy_has_hero(s))
					Hero.setXfix(s->x);
			}

			// Move the Fairy enemy as well.
			if (auto itm = dynamic_cast<item*>(s))
			{
				auto const& fairy_item = itemsbuf.get(itm->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(itm->fairyUID);
					if (fairy)
						fairy->x = itm->x;
				}
			}
			break;
		}
		case SPRITE_Y:
		{
			if (is_player(GET_REF(spriteref)))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setYfix(zslongToFix(value));
				else
					Hero.setY(value / 10000);
				break;
			}

			if (s)
			{
				if (auto npc = dynamic_cast<enemy*>(s))
					npc->floor_y += ((get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000)) - s->y);
				s->y = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (enemy_has_hero(s))
					Hero.setYfix(s->y);
			}

			// Move the Fairy enemy as well.
			if (auto itm = dynamic_cast<item*>(s))
			{
				auto const& fairy_item = itemsbuf.get(itm->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(itm->fairyUID);
					if (fairy)
						fairy->y = itm->y;
				}
			}
			break;
		}
		case SPRITE_Z:
		{
			if (is_player(GET_REF(spriteref)))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setZfix(zslongToFix(value));
				else
					Hero.setZ(value / 10000);
				break;
			}

			if (is_enemy(s) && never_in_air(s->id))
			{
				break;
			}

			if (s)
			{
				s->z = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (s->z < 0)
					s->z = 0;
				if (enemy_has_hero(s))
					Hero.setZfix(s->z);
			}
			break;
		}
		case SPRITE_FAKE_Z:
		{
			if (is_player(GET_REF(spriteref)))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setFakeZfix(zslongToFix(value));
				else
					Hero.setFakeZ(value / 10000);
				break;
			}

			if (is_enemy(s) && never_in_air(s->id))
			{
				break;
			}

			if (s)
			{
				s->fakez = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (s->fakez < 0)
					s->fakez = 0;
				if (enemy_has_hero(s))
					Hero.setFakeZfix(s->fakez);
			}
			break;
		}
		case SPRITE_X_OFFSET:
		{
			if (s)
				s->xofs = value/10000;
			break;
		}
		case SPRITE_Y_OFFSET:
		{
			if (s)
				s->yofs = value/10000 + (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			break;
		}
		case SPRITE_Z_OFFSET:
		{
			if (s)
				s->zofs = value/10000;
			break;
		}
		case SPRITE_ROTATION:
		{
			if (get_qr(qr_OLDSPRITEDRAWS))
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}

			if (s)
				s->rotation = value / 10000;
			break;
		}
		case SPRITE_DIR:
		{
			if (is_player(GET_REF(spriteref)))
			{
				//Hero->setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
				if (Hero.getAction() == attacking || Hero.getAction() == sideswimattacking) Hero.dir = value / 10000;
				else Hero.setDir(value / 10000);
				break;
			}

			if (s)
			{
				s->dir = value / 10000;
				if (auto w = dynamic_cast<weapon*>(s))
					w->doAutoRotate(true);
			}
			break;
		}
		case SPRITE_INVIS_CLK:
			if (s)
				s->invis_timer = value / 10000;
			break;
		case SPRITE_TILE:
		{
			int tile = value / 10000;
			if (BC::checkTile(tile) != SH::_NoError)
				break;

			if (s)
				s->tile = tile;
			break;
		}
		case SPRITE_SCRIPT_TILE:
		{
			int tile = value / 10000;
			if (BC::checkBounds(tile, -1, NEWMAXTILES-1) != SH::_NoError)
				break;

			// TODO: use sprite::scriptile
			if (is_player(GET_REF(spriteref)))
			{
				script_hero_sprite = tile;
				break;
			}
			if (s)
				s->scripttile = tile;
			break;
		}
		case SPRITE_TILE_W:
		{
			if (s)
			{
				int width = value / 10000;
				if (dynamic_cast<ffcdata*>(s))
				{
					// TODO: don't vbound, check bounds.
					s->txsz = vbound(width, 1, 4);
					break;
				}

				if (BC::checkBounds(width, 0, 20) == SH::_NoError)
					s->txsz = width;
			}
			break;
		}
		case SPRITE_TILE_H:
		{
			if (s)
			{
				int height = value / 10000;
				if (dynamic_cast<ffcdata*>(s))
				{
					// TODO: don't vbound, check bounds.
					s->tysz = vbound(height, 1, 4);
					break;
				}

				if (BC::checkBounds(height, 0, 20) == SH::_NoError)
					s->tysz = height;
			}
			break;
		}
		case SPRITE_CSET:
		{
			if (s)
			{
				if (auto ffc = dynamic_cast<ffcdata*>(s))
				{
					ffc->cset = (value / 10000) & 0xF;
					break;
				}

				if (auto i = dynamic_cast<item*>(s))
				{
					i->o_cset = (i->o_cset & ~15) | ((value/10000)&15);
					i->cs = i->o_cset & 15;
					break;
				}

				s->cs = (value / 10000) & 0xF;
				if (auto e = dynamic_cast<enemy*>(s))
					if (e->type == eeLEV)
						e->dcset = e->cs;
			}
			break;
		}
		case SPRITE_SCALE:
		{
			if (get_qr(qr_OLDSPRITEDRAWS))
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}

			if (s)
				s->scale = value / 100;
			break;
		}
		case SPRITE_DRAW_STYLE:
		{
			if (s)
				s->drawstyle = value/10000;
			break;
		}
		case SPRITE_JUMP:
		{
			if (is_player(GET_REF(spriteref)))
			{
				Hero.setJump(zslongToFix(value));
				break;
			}

			if (s)
			{
				if (auto e = dynamic_cast<enemy*>(s))
				{
					if (canfall(e->id))
					{
						e->fall = -zslongToFix(value) * 100;
						if (enemy_has_hero(s))
							Hero.setFall(e->fall);
					}
					break;
				}

				s->fall = -zslongToFix(value) * 100;
			}
			break;
		}
		case SPRITE_FAKE_JUMP:
		{
			if (is_player(GET_REF(spriteref)))
			{
				Hero.setFakeFall(zslongToFix(value) * -100);
				break;
			}

			if (s)
			{
				if (auto e = dynamic_cast<enemy*>(s))
				{
					if (canfall(e->id))
					{
						e->fakefall = -zslongToFix(value) * 100;
						if (enemy_has_hero(s))
							Hero.setFall(e->fakefall);
					}
					break;
				}

				s->fakefall = -zslongToFix(value) * 100;
			}
			break;
		}
		case SPRITE_GRAVITY:
		{
			if (s)
				SETFLAG(s->moveflags, move_obeys_grav, value);
			break;
		}
		case SPRITE_CUSTOM_GRAVITY_STRENGTH:
		{
			if (s)
				s->custom_gravity = zslongToFix(value);
			break;
		}
		case SPRITE_CUSTOM_TERMINAL_VELOCITY:
		{
			if (s)
				s->custom_terminal_v = zslongToFix(value);
			break;
		}
		case SPRITE_FLIP:
		{
			if (s)
				s->flip = value / 10000;
			break;
		}
		case SPRITE_SCRIPT_FLIP:
		{
			// TODO: don't vbound, check bounds.
			// TODO: use sprite::scriptflip
			if (is_player(GET_REF(spriteref)))
			{
				script_hero_flip = vbound(value / 10000, -1, 256);
				break;
			}

			if (s)
				s->scriptflip = vbound(value / 10000, -1, 127);
			break;
		}
		case SPRITE_ENGINE_ANIMATE:
		{
			if (s)
				s->do_animation = value;
			break;
		}
		case SPRITE_EXTEND:
		{
			if (s)
				s->extend = value / 10000;
			break;
		}
		case SPRITE_HIT_WIDTH:
		{
			if (s)
				s->hit_width = value / 10000;
			break;
		}
		case SPRITE_HIT_HEIGHT:
		{
			if (s)
				s->hit_height = value / 10000;
			break;
		}
		case SPRITE_HIT_ZHEIGHT:
		{
			if (s)
				s->hzsz = value / 10000;
			break;
		}
		case SPRITE_HIT_OFFSET_X:
		{
			if (s)
				s->hxofs = value / 10000;
			break;
		}
		case SPRITE_HIT_OFFSET_Y:
		{
			if (s)
				s->hyofs = value / 10000;
			break;
		}
		case SPRITE_FALL_CLK:
		{
			if (is_player(GET_REF(spriteref)))
			{
				int val = vbound(value / 10000, 0, PITFALL_FALL_FRAMES);
				if (val)
					Hero.setAction(falling);
				else if (Hero.action == falling)
					Hero.setAction(none);
				Hero.fallclk = val;
				break;
			}

			if (s)
			{
				if (s->fallclk != 0 && value == 0)
				{
					s->cs = s->o_cset;
					s->tile = s->o_tile;
				}
				else if (s->fallclk == 0 && value != 0)
				{
					s->o_cset = s->cs;
				}
				s->fallclk = vbound(value / 10000, 0, PITFALL_FALL_FRAMES);
			}
			break;
		}
		case SPRITE_FALL_CMB:
		{
			if (s)
				s->fallCombo = vbound(value / 10000, 0, MAXCOMBOS - 1);
			break;
		}
		case SPRITE_LIGHT_RADIUS:
		{
			if (s)
				s->glowRad = vbound(value / 10000, 0, 255);
			break;
		}
		case SPRITE_LIGHT_SHAPE:
		{
			if (s)
				s->glowShape = vbound(value / 10000, 0, 255);
			break;
		}
		case SPRITE_LIGHT_OFFSET:
		{
			if (s)
				s->glowOffset = value / 10000;
			break;
		}
		case SPRITE_NOCOLL_CLK:
		{
			if (s)
				s->script_no_colldet = value / 10000;
			break;
		}
		case SPRITE_SWHOOKED:
		{
			// read-only
			break;
		}
		case SPRITE_SHADOW_SPR:
		{
			if (s)
				s->spr_shadow = vbound(value / 10000, 0, MAXSPRITES-1);
			break;
		}
		case SPRITE_DROWN_CLK:
		{
			if (is_player(GET_REF(spriteref)))
			{
				// TODO: this starts at 64, and we document the max as being 64, so... update to WATER_DROWN_FRAMES?
				int val = vbound(value / 10000, 0, 70);
				if (val)
				{
					if (Hero.action != lavadrowning) Hero.setAction(drowning);
				}
				else if (Hero.action == drowning || Hero.action == lavadrowning)
				{
					Hero.setAction(none);
				}
				Hero.drownclk = val;
				break;
			}

			if (s)
			{
				if (s->drownclk != 0 && value == 0)
				{
					s->cs = s->o_cset;
					s->tile = s->o_tile;
				}
				else if (s->drownclk == 0 && value != 0)
				{
					s->o_cset = s->cs;
				}
				// TODO: this starts at 64, and we document the max as being 64, so... update to WATER_DROWN_FRAMES?
				s->drownclk = vbound(value / 10000, 0, 70);
			}
			break;
		}
		case SPRITE_DROWN_CMB:
		{
			if (s)
				s->drownCombo = vbound(value / 10000, 0, MAXCOMBOS - 1);
			break;
		}
		case SPRITE_SHADOW_XOFS:
		{
			if (s)
				s->shadowxofs = value / 10000;
			break;
		}
		case SPRITE_SHADOW_YOFS:
		{
			if (s)
				s->shadowyofs = value / 10000;
			break;
		}
		case SPRITE_VIEWPORT_SUSPEND_RANGE:
		{
			if (s)
				s->viewport_suspend_range = value / 10000;
			break;
		}
		case SPRITE_VIEWPORT_DESPAWN_RANGE:
		{
			if (s)
				s->viewport_despawn_range = value / 10000;
			break;
		}

		default: NOTREACHED();
	}
}

static ArrayRegistrar SPRITE_MISCD_registrar(SPRITE_MISCD, []{
	static ScriptingArray_ObjectComputed<sprite, int32_t> impl(
		[](auto sprite){ return dynamic_cast<ffcdata*>(sprite) ? 16 : 32; },
		[](auto sprite, int index){
			// TODO: don't vbound for player.
			if (is_player(sprite))
				return Hero.miscellaneous[vbound(index,0,31)];

			return sprite->miscellaneous[index];
		},
		[](auto sprite, int index, int value){
			// TODO don't vbound for player.
			if (is_player(sprite))
			{
				Hero.miscellaneous[vbound(index,0,31)] = value;
				return;
			}

			sprite->miscellaneous[index] = value;
		}
	);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar SPRITE_MOVE_FLAGS_registrar(SPRITE_MOVE_FLAGS, []{
	static ScriptingArray_ObjectComputed<sprite, bool> impl(
		[](auto sprite){ return is_enemy(sprite) ? 16 : 11; },
		[](auto sprite, int index) -> bool {
			return sprite->moveflags & (1<<index);
		},
		[](auto sprite, int index, bool value){
			move_flags bit = (move_flags)(1<<index);
			SETFLAG(sprite->moveflags, bit, value);
		}
	);
	impl.setMul10000(true);
	return &impl;
}());
