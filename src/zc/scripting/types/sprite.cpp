#include "zc/scripting/types/sprite.h"

#include "base/general.h"
#include "base/qrs.h"
#include "base/zdefs.h"
#include "zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "hero_tiles.h"
#include "zc/scripting/arrays.h"

#include <cstdint>
#include <optional>

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
		if (e->family == eeWALLM || e->family == eeWALK)
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

std::optional<int32_t> sprite_get_register(int32_t reg)
{
	switch (reg)
	{
		case SPRITE_SPAWN_SCREEN:
		{
			if (is_player(ri->spriteref))
				return cur_screen * 10000;

			if (auto s = get_sprite(ri->spriteref))
				return s->screen_spawned * 10000;
			break;
		}
		case SPRITE_X:
		{
			if (auto s = get_sprite(ri->spriteref))
				return as_float(s) ? s->x.getZLong() : int(s->x) * 10000;
			return 0;
		}
		case SPRITE_Y:
		{
			if (auto s = get_sprite(ri->spriteref))
				return as_float(s) ? s->y.getZLong() : int(s->y) * 10000;
			return 0;
		}
		case SPRITE_Z:
		{
			if (auto s = get_sprite(ri->spriteref))
				return as_float(s) ? s->z.getZLong() : int(s->z) * 10000;
			return 0;
		}
		case SPRITE_FAKE_Z:
		{
			if (auto s = get_sprite(ri->spriteref))
				return as_float(s) ? s->fakez.getZLong() : int(s->fakez) * 10000;
			return 0;
		}
		case SPRITE_X_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->xofs * 10000;
			return 0;
		}
		case SPRITE_Y_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref))
				return (s->yofs - (get_qr(qr_OLD_DRAWOFFSET) ? playing_field_offset : original_playing_field_offset)) * 10000;
			return 0;
		}
		case SPRITE_Z_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->zofs * 10000;
			return 0;
		}
		case SPRITE_ROTATION:
		{
			if (get_qr(qr_OLDSPRITEDRAWS))
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				return 0;
			}

			if (auto s = get_sprite(ri->spriteref))
				return s->rotation * 10000;
			return 0;
		}
		case SPRITE_DIR:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->dir * 10000;
			return 0;
		}
		case SPRITE_TILE:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->tile * 10000;
			return 0;
		}
		case SPRITE_SCRIPT_TILE:
		{
			// TODO: use sprite::scriptile
			if (is_player(ri->spriteref))
				return script_hero_sprite * 10000;

			if (auto s = get_sprite(ri->spriteref))
				return s->scripttile * 10000;
			return 0;
		}
		case SPRITE_TILE_W:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->txsz * 10000;
			return 0;
		}
		case SPRITE_TILE_H:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->tysz * 10000;
			return 0;
		}
		case SPRITE_CSET:
		{
			if (auto s = get_sprite(ri->spriteref))
			{
				if (auto ffc = dynamic_cast<ffcdata*>(s))
					return ffc->cset * 10000;

				if (auto i = dynamic_cast<item*>(s))
					return i->o_cset * 10000;

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

			if (auto s = get_sprite(ri->spriteref))
				return s->scale * 100;
			return 0;
		}
		case SPRITE_DRAW_STYLE:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->drawstyle * 10000;
			return 0;
		}
		case SPRITE_JUMP:
		{
			if (auto s = get_sprite(ri->spriteref))
			{
				int32_t ret = s->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED) && !is_player(ri->spriteref)) ret = trunc(ret / 10000) * 10000;
				return ret;
			}
			return 0;
		}
		case SPRITE_FAKE_JUMP:
		{
			if (auto s = get_sprite(ri->spriteref))
			{
				int32_t ret = s->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED) && !is_player(ri->spriteref)) ret = trunc(ret / 10000) * 10000;
				return ret;
			}
			return 0;
		}
		case SPRITE_GRAVITY:
		{
			if (auto s = get_sprite(ri->spriteref))
				return (s->moveflags & move_obeys_grav) ? 10000 : 0;
			return 0;
		}
		case SPRITE_FLIP:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->flip * 10000;
			return 0;
		}
		case SPRITE_SCRIPT_FLIP:
		{
			// TODO: use sprite::scriptflip
			if (is_player(ri->spriteref))
				return script_hero_flip * 10000;
			if (auto s = get_sprite(ri->spriteref))
				return s->scriptflip * 10000;
			return 0;
		}
		case SPRITE_ENGINE_ANIMATE:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->do_animation * 10000;
			return 0;
		}
		case SPRITE_EXTEND:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->extend * 10000;
			return 0;
		}
		case SPRITE_HIT_WIDTH:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->hit_width * 10000;
			return 0;
		}
		case SPRITE_HIT_HEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->hit_height * 10000;
			return 0;
		}
		case SPRITE_HIT_ZHEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->hzsz * 10000;
			return 0;
		}
		case SPRITE_HIT_OFFSET_X:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->hxofs * 10000;
			return 0;
		}
		case SPRITE_HIT_OFFSET_Y:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->hyofs * 10000;
			return 0;
		}
		case SPRITE_FALL_CLK:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->fallclk * 10000;
			return 0;
		}
		case SPRITE_FALL_CMB:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->fallCombo * 10000;
			return 0;
		}
		case SPRITE_LIGHT_RADIUS:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->glowRad * 10000;
			return 0;
		}
		case SPRITE_LIGHT_SHAPE:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->glowShape * 10000;
			return 0;
		}
		case SPRITE_SWHOOKED:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->switch_hooked * 10000;
			return 0;
		}
		case SPRITE_SHADOW_SPR:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->spr_shadow * 10000;
			return 0;
		}
		case SPRITE_DROWN_CLK:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->drownclk * 10000;
			return 0;
		}
		case SPRITE_DROWN_CMB:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->drownCombo * 10000;
			return 0;
		}
		case SPRITE_SHADOW_XOFS:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->shadowxofs * 10000;
			return 0;
		}
		case SPRITE_SHADOW_YOFS:
		{
			if (auto s = get_sprite(ri->spriteref))
				return s->shadowyofs * 10000;
			return 0;
		}
	}

	return std::nullopt;
}

bool sprite_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case SPRITE_X:
		{
			if (is_player(ri->spriteref))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setXfix(zslongToFix(value));
				else
					Hero.setX(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
			{
				s->x = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (enemy_has_hero(s))
					Hero.setXfix(s->x);
			}
			break;
		}
		case SPRITE_Y:
		{
			if (is_player(ri->spriteref))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setYfix(zslongToFix(value));
				else
					Hero.setY(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
			{
				s->y = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (enemy_has_hero(s))
					Hero.setXfix(s->y);
			}
			break;
		}
		case SPRITE_Z:
		{
			if (is_player(ri->spriteref))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setZfix(zslongToFix(value));
				else
					Hero.setZ(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
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
			if (is_player(ri->spriteref))
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setFakeZfix(zslongToFix(value));
				else
					Hero.setFakeZ(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
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
			if (auto s = get_sprite(ri->spriteref))
				s->xofs = value/10000;
			break;
		}
		case SPRITE_Y_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->yofs = value/10000 + (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			break;
		}
		case SPRITE_Z_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref))
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

			if (auto s = get_sprite(ri->spriteref))
				s->rotation = value / 10000;
			break;
		}
		case SPRITE_DIR:
		{
			if (is_player(ri->spriteref))
			{
				//Hero->setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
				if (Hero.getAction() == attacking || Hero.getAction() == sideswimattacking) Hero.dir = value / 10000;
				else Hero.setDir(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
			{
				s->dir = value / 10000;
				if (auto w = dynamic_cast<weapon*>(s))
					w->doAutoRotate(true);
			}
			break;
		}
		case SPRITE_TILE:
		{
			int tile = value / 10000;
			if (BC::checkTile(tile) != SH::_NoError)
				break;

			if (auto s = get_sprite(ri->spriteref))
				s->tile = tile;
			break;
		}
		case SPRITE_SCRIPT_TILE:
		{
			int tile = value / 10000;
			if (BC::checkBounds(tile, -1, NEWMAXTILES-1) != SH::_NoError)
				break;

			// TODO: use sprite::scriptile
			if (is_player(ri->spriteref))
			{
				script_hero_sprite = tile;
				break;
			}
			if (auto s = get_sprite(ri->spriteref))
				s->scripttile = tile;
			break;
		}
		case SPRITE_TILE_W:
		{
			if (auto s = get_sprite(ri->spriteref))
			{
				int height = value / 10000;
				if (dynamic_cast<ffcdata*>(s))
				{
					// TODO: don't vbound, check bounds.
					s->txsz = vbound(height, 1, 4);
					break;
				}

				if (BC::checkBounds(height, 0, 20) == SH::_NoError)
					s->txsz = height;
			}
			break;
		}
		case SPRITE_TILE_H:
		{
			if (auto s = get_sprite(ri->spriteref))
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
			if (auto s = get_sprite(ri->spriteref))
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
					if (e->family == eeLEV)
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

			if (auto s = get_sprite(ri->spriteref))
				s->scale = value / 100;
			break;
		}
		case SPRITE_DRAW_STYLE:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->drawstyle = value/10000;
			break;
		}
		case SPRITE_JUMP:
		{
			if (is_player(ri->spriteref))
			{
				Hero.setFall(-zslongToFix(value) * 100);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
			{
				if (auto e = dynamic_cast<enemy*>(s))
				{
					if (canfall(e->id))
						e->fall = -zslongToFix(value) * 100;
					if (enemy_has_hero(s))
						Hero.setFall(e->fall);
					break;
				}

				s->fall = -zslongToFix(value) * 100;
			}
			break;
		}
		case SPRITE_FAKE_JUMP:
		{
			if (is_player(ri->spriteref))
			{
				Hero.setFakeFall(zslongToFix(value) * -100);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
			{
				if (auto e = dynamic_cast<enemy*>(s))
				{
					if (canfall(e->id))
						e->fakefall = -zslongToFix(value) * 100;
					if (enemy_has_hero(s))
						Hero.setFall(e->fakefall);
					break;
				}

				s->fakefall = -zslongToFix(value) * 100;
			}
			break;
		}
		case SPRITE_GRAVITY:
		{
			if (auto s = get_sprite(ri->spriteref))
				SETFLAG(s->moveflags, move_obeys_grav, value);
			break;
		}
		case SPRITE_FLIP:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->flip = value / 10000;
			break;
		}
		case SPRITE_SCRIPT_FLIP:
		{
			// TODO: don't vbound, check bounds.
			// TODO: use sprite::scriptflip
			if (is_player(ri->spriteref))
			{
				script_hero_flip = vbound(value / 10000, -1, 256);
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
				s->scriptflip = vbound(value / 10000, -1, 127);
			break;
		}
		case SPRITE_ENGINE_ANIMATE:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->do_animation = value;
			break;
		}
		case SPRITE_EXTEND:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->extend = value / 10000;
			break;
		}
		case SPRITE_HIT_WIDTH:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->hit_width = value / 10000;
			break;
		}
		case SPRITE_HIT_HEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->hit_height = value / 10000;
			break;
		}
		case SPRITE_HIT_ZHEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->hzsz = value / 10000;
			break;
		}
		case SPRITE_HIT_OFFSET_X:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->hxofs = value / 10000;
			break;
		}
		case SPRITE_HIT_OFFSET_Y:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->hyofs = value / 10000;
			break;
		}
		case SPRITE_FALL_CLK:
		{
			if (is_player(ri->spriteref))
			{
				int val = vbound(value / 10000, 0, PITFALL_FALL_FRAMES);
				if (val)
					Hero.setAction(falling);
				else if (Hero.action == falling)
					Hero.setAction(none);
				Hero.fallclk = val;
				break;
			}

			if (auto s = get_sprite(ri->spriteref))
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
			if (auto s = get_sprite(ri->spriteref))
				s->fallCombo = vbound(value / 10000, 0, MAXCOMBOS - 1);
			break;
		}
		case SPRITE_LIGHT_RADIUS:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->glowRad = vbound(value / 10000, 0, 255);
			break;
		}
		case SPRITE_LIGHT_SHAPE:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->glowShape = vbound(value / 10000, 0, 255);
			break;
		}
		case SPRITE_SWHOOKED:
		{
			// read-only
			break;
		}
		case SPRITE_SHADOW_SPR:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->spr_shadow = vbound(value / 10000, 0, 255);
			break;
		}
		case SPRITE_DROWN_CLK:
		{
			if (is_player(ri->spriteref))
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

			if (auto s = get_sprite(ri->spriteref))
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
			if (auto s = get_sprite(ri->spriteref))
				s->drownCombo = vbound(value / 10000, 0, MAXCOMBOS - 1);
			break;
		}
		case SPRITE_SHADOW_XOFS:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->shadowxofs = value / 10000;
			break;
		}
		case SPRITE_SHADOW_YOFS:
		{
			if (auto s = get_sprite(ri->spriteref))
				s->shadowyofs = value / 10000;
			break;
		}

		default: return false;
	}

	return true;
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
	impl.setDefaultValue(0);
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
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());
