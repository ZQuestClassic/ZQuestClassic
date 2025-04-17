#include "zc/scripting/types/sprite.h"

#include "base/general.h"
#include "base/qrs.h"
#include "base/zdefs.h"
#include "zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "hero_tiles.h"

#include <cstdint>
#include <optional>

extern refInfo *ri;

static bool is_player()
{
	return ri->spriteref == 1;
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

static sprite* get_sprite(int uid, const char* context)
{
	if (!uid)
	{
		Z_scripterrlog("The pointer is NULL or uninitialised (sprite->%s).\n", context);
		return nullptr;
	}

	sprite* s = sprite::getByUID(uid);
	if (!s)
	{
		Z_scripterrlog("Invalid pointer for UID %ld (sprite->%s).\n", uid, context);
		return nullptr;
	}

	return s;
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
			if (is_player())
				return cur_screen * 10000;

			if (auto s = get_sprite(ri->spriteref, "SpawnScreen"))
				return s->screen_spawned * 10000;
			break;
		}
		case SPRITE_X:
		{
			if (auto s = get_sprite(ri->spriteref, "X"))
				return as_float(s) ? s->x.getZLong() : int(s->x) * 10000;
			return 0;
		}
		case SPRITE_Y:
		{
			if (auto s = get_sprite(ri->spriteref, "Y"))
				return as_float(s) ? s->y.getZLong() : int(s->y) * 10000;
			return 0;
		}
		case SPRITE_Z:
		{
			if (auto s = get_sprite(ri->spriteref, "Z"))
				return as_float(s) ? s->z.getZLong() : int(s->z) * 10000;
			return 0;
		}
		case SPRITE_FAKE_Z:
		{
			if (auto s = get_sprite(ri->spriteref, "FakeZ"))
				return as_float(s) ? s->fakez.getZLong() : int(s->fakez) * 10000;
			return 0;
		}
		case SPRITE_X_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref, "DrawXOffset"))
				return s->xofs * 10000;
			return 0;
		}
		case SPRITE_Y_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref, "DrawYOffset"))
				return (s->yofs - (get_qr(qr_OLD_DRAWOFFSET) ? playing_field_offset : original_playing_field_offset)) * 10000;
			return 0;
		}
		case SPRITE_Z_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref, "DrawZOffset"))
				return s->zofs * 10000;
			return 0;
		}
		case SPRITE_ROTATION:
		{
			if (get_qr(qr_OLDSPRITEDRAWS))
			{
				Z_scripterrlog("To use %s you must disable the quest rule 'Old (Faster) Sprite Drawing'.\n", "sprite->Rotation");
				return 0;
			}

			if (auto s = get_sprite(ri->spriteref, "Rotation"))
				return s->rotation * 10000;
			return 0;
		}
		case SPRITE_DIR:
		{
			if (auto s = get_sprite(ri->spriteref, "Dir"))
				return s->dir * 10000;
			return 0;
		}
		case SPRITE_TILE:
		{
			if (auto s = get_sprite(ri->spriteref, "Tile"))
				return s->tile * 10000;
			return 0;
		}
		case SPRITE_SCRIPT_TILE:
		{
			// TODO: use sprite::scriptile
			if (is_player())
				return script_hero_sprite * 10000;

			if (auto s = get_sprite(ri->spriteref, "ScriptTile"))
				return s->scripttile * 10000;
			return 0;
		}
		case SPRITE_TILE_W:
		{
			if (auto s = get_sprite(ri->spriteref, "TileWidth"))
				return s->txsz * 10000;
			return 0;
		}
		case SPRITE_TILE_H:
		{
			if (auto s = get_sprite(ri->spriteref, "TileHeight"))
				return s->tysz * 10000;
			return 0;
		}
		case SPRITE_CSET:
		{
			if (auto s = get_sprite(ri->spriteref, "CSet"))
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
				Z_scripterrlog("To use %s you must disable the quest rule 'Old (Faster) Sprite Drawing'.\n", "sprite->Scale");
				return 0;
			}

			if (auto s = get_sprite(ri->spriteref, "Scale"))
				return s->scale * 100;
			return 0;
		}
		case SPRITE_DRAW_STYLE:
		{
			if (auto s = get_sprite(ri->spriteref, "DrawStyle"))
				return s->drawstyle * 10000;
			return 0;
		}
		case SPRITE_JUMP:
		{
			if (auto s = get_sprite(ri->spriteref, "Jump"))
			{
				int32_t ret = s->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED) && !is_player()) ret = trunc(ret / 10000) * 10000;
				return ret;
			}
			return 0;
		}
		case SPRITE_FAKE_JUMP:
		{
			if (auto s = get_sprite(ri->spriteref, "FakeJump"))
			{
				int32_t ret = s->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED) && !is_player()) ret = trunc(ret / 10000) * 10000;
				return ret;
			}
			return 0;
		}
		case SPRITE_GRAVITY:
		{
			if (auto s = get_sprite(ri->spriteref, "Gravity"))
				return (s->moveflags & move_obeys_grav) ? 10000 : 0;
			return 0;
		}
		case SPRITE_FLIP:
		{
			if (auto s = get_sprite(ri->spriteref, "Flip"))
				return s->flip * 10000;
			return 0;
		}
		case SPRITE_SCRIPT_FLIP:
		{
			// TODO: use sprite::scriptflip
			if (is_player())
				return script_hero_flip * 10000;
			if (auto s = get_sprite(ri->spriteref, "ScriptFlip"))
				return s->scriptflip * 10000;
			return 0;
		}
		case SPRITE_ENGINE_ANIMATE:
		{
			if (auto s = get_sprite(ri->spriteref, "Animation"))
				return s->do_animation * 10000;
			return 0;
		}
		case SPRITE_EXTEND:
		{
			if (auto s = get_sprite(ri->spriteref, "Extend"))
				return s->extend * 10000;
			return 0;
		}
		case SPRITE_HIT_WIDTH:
		{
			if (auto s = get_sprite(ri->spriteref, "HitWidth"))
				return s->hit_width * 10000;
			return 0;
		}
		case SPRITE_HIT_HEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref, "HitHeight"))
				return s->hit_height * 10000;
			return 0;
		}
		case SPRITE_HIT_ZHEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref, "HitZHeight"))
				return s->hzsz * 10000;
			return 0;
		}
		case SPRITE_HIT_OFFSET_X:
		{
			if (auto s = get_sprite(ri->spriteref, "HitXOffset"))
				return s->hxofs * 10000;
			return 0;
		}
		case SPRITE_HIT_OFFSET_Y:
		{
			if (auto s = get_sprite(ri->spriteref, "HitYOffset"))
				return s->hyofs * 10000;
			return 0;
		}
		case SPRITE_FALL_CLK:
		{
			if (auto s = get_sprite(ri->spriteref, "Falling"))
				return s->fallclk * 10000;
			return 0;
		}
		case SPRITE_FALL_CMB:
		{
			if (auto s = get_sprite(ri->spriteref, "FallCombo"))
				return s->fallCombo * 10000;
			return 0;
		}
		case SPRITE_MOVE_FLAGS:
		{
			if (auto s = get_sprite(ri->spriteref, "MoveFlags[]"))
			{
				int index = ri->d[rINDEX] / 10000;
				int limit = is_enemy(s) ? 15 : 10;
				if (BC::checkBounds(index, 0, limit, "sprite->MoveFlags[]") != SH::_NoError)
					return 0;

				return (s->moveflags & (1<<index)) ? 10000 : 0;
			}
			return 0;
		}
		case SPRITE_LIGHT_RADIUS:
		{
			if (auto s = get_sprite(ri->spriteref, "LightRadius"))
				return s->glowRad * 10000;
			return 0;
		}
		case SPRITE_LIGHT_SHAPE:
		{
			if (auto s = get_sprite(ri->spriteref, "LightShape"))
				return s->glowShape * 10000;
			return 0;
		}
		case SPRITE_SWHOOKED:
		{
			if (auto s = get_sprite(ri->spriteref, "SwitchHooked"))
				return s->switch_hooked * 10000;
			return 0;
		}
		case SPRITE_SHADOW_SPR:
		{
			if (auto s = get_sprite(ri->spriteref, "ShadowSprite"))
				return s->spr_shadow * 10000;
			return 0;
		}
		case SPRITE_DROWN_CLK:
		{
			if (auto s = get_sprite(ri->spriteref, "Drowning"))
				return s->drownclk * 10000;
			return 0;
		}
		case SPRITE_DROWN_CMB:
		{
			if (auto s = get_sprite(ri->spriteref, "DrownCombo"))
				return s->drownCombo * 10000;
			return 0;
		}
		case SPRITE_SHADOW_XOFS:
		{
			if (auto s = get_sprite(ri->spriteref, "ShadowXOffset"))
				return s->shadowxofs * 10000;
			return 0;
		}
		case SPRITE_SHADOW_YOFS:
		{
			if (auto s = get_sprite(ri->spriteref, "ShadowYOffset"))
				return s->shadowyofs * 10000;
			return 0;
		}
		case SPRITE_MISCD:
		{
			// TODO: don't vbound for player. check bounds, like the others.
			if (is_player())
				return Hero.miscellaneous[vbound(ri->d[rINDEX]/10000,0,31)];

			if (auto s = get_sprite(ri->spriteref, "Misc[]"))
			{
				int index = ri->d[rINDEX] / 10000;
				if (auto f = dynamic_cast<ffcdata*>(s))
				{
					if (BC::checkMisc(index, "sprite->Misc[]") != SH::_NoError)
						return 0;
				}
				else if (BC::checkMisc32(index, "sprite->Misc[]") != SH::_NoError)
					break;

				return s->miscellaneous[index];
			}

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
			if (is_player())
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setXfix(zslongToFix(value));
				else
					Hero.setX(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "X"))
			{
				s->x = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (enemy_has_hero(s))
					Hero.setXfix(s->x);
			}
			break;
		}
		case SPRITE_Y:
		{
			if (is_player())
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setYfix(zslongToFix(value));
				else
					Hero.setY(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Y"))
			{
				s->y = as_float(s) ? zslongToFix(value) : zfix(value/10000);
				if (enemy_has_hero(s))
					Hero.setXfix(s->y);
			}
			break;
		}
		case SPRITE_Z:
		{
			if (is_player())
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setZfix(zslongToFix(value));
				else
					Hero.setZ(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Z"))
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
			if (is_player())
			{
				if (get_qr(qr_SPRITEXY_IS_FLOAT))
					Hero.setFakeZfix(zslongToFix(value));
				else
					Hero.setFakeZ(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "FakeZ"))
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
			if (auto s = get_sprite(ri->spriteref, "DrawXOffset"))
				s->xofs = value/10000;
			break;
		}
		case SPRITE_Y_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref, "DrawYOffset"))
				s->yofs = value/10000 + (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			break;
		}
		case SPRITE_Z_OFFSET:
		{
			if (auto s = get_sprite(ri->spriteref, "DrawZOffset"))
				s->zofs = value/10000;
			break;
		}
		case SPRITE_ROTATION:
		{
			if (get_qr(qr_OLDSPRITEDRAWS))
			{
				Z_scripterrlog("To use %s you must disable the quest rule 'Old (Faster) Sprite Drawing'.\n", "sprite->Rotation");
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Rotation"))
				s->rotation = value / 10000;
			break;
		}
		case SPRITE_DIR:
		{
			if (is_player())
			{
				//Hero->setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
				if (Hero.getAction() == attacking || Hero.getAction() == sideswimattacking) Hero.dir = value / 10000;
				else Hero.setDir(value / 10000);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Dir"))
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
			if (BC::checkTile(tile, "sprite->Tile") != SH::_NoError)
				break;

			if (auto s = get_sprite(ri->spriteref, "Tile"))
				s->tile = tile;
			break;
		}
		case SPRITE_SCRIPT_TILE:
		{
			int tile = value / 10000;
			if (BC::checkBounds(tile, -1, NEWMAXTILES-1, "sprite->ScriptTile") != SH::_NoError)
				break;

			// TODO: use sprite::scriptile
			if (is_player())
			{
				script_hero_sprite = tile;
				break;
			}
			if (auto s = get_sprite(ri->spriteref, "ScriptTile"))
				s->scripttile = tile;
			break;
		}
		case SPRITE_TILE_W:
		{
			if (auto s = get_sprite(ri->spriteref, "TileWidth"))
			{
				int height = value / 10000;
				if (dynamic_cast<ffcdata*>(s))
				{
					// TODO: don't vbound, check bounds.
					s->txsz = vbound(height, 1, 4);
					break;
				}

				if (BC::checkBounds(height, 0, 20, "sprite->TileWidth") == SH::_NoError)
					s->txsz = height;
			}
			break;
		}
		case SPRITE_TILE_H:
		{
			if (auto s = get_sprite(ri->spriteref, "TileHeight"))
			{
				int height = value / 10000;
				if (dynamic_cast<ffcdata*>(s))
				{
					// TODO: don't vbound, check bounds.
					s->tysz = vbound(height, 1, 4);
					break;
				}

				if (BC::checkBounds(height, 0, 20, "sprite->TileHeight") == SH::_NoError)
					s->tysz = height;
			}
			break;
		}
		case SPRITE_CSET:
		{
			if (auto s = get_sprite(ri->spriteref, "CSet"))
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
				Z_scripterrlog("To use %s you must disable the quest rule 'Old (Faster) Sprite Drawing'.\n", "sprite->Scale");
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Scale"))
				s->scale = value / 100;
			break;
		}
		case SPRITE_DRAW_STYLE:
		{
			if (auto s = get_sprite(ri->spriteref, "DrawStyle"))
				s->drawstyle = value/10000;
			break;
		}
		case SPRITE_JUMP:
		{
			if (is_player())
			{
				Hero.setFall(-zslongToFix(value) * 100);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Jump"))
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
			if (is_player())
			{
				Hero.setFakeFall(zslongToFix(value) * -100);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "FakeJump"))
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
			if (auto s = get_sprite(ri->spriteref, "Gravity"))
				SETFLAG(s->moveflags, move_obeys_grav, value);
			break;
		}
		case SPRITE_FLIP:
		{
			if (auto s = get_sprite(ri->spriteref, "Flip"))
				s->flip = value / 10000;
			break;
		}
		case SPRITE_SCRIPT_FLIP:
		{
			// TODO: don't vbound, check bounds.
			// TODO: use sprite::scriptflip
			if (is_player())
			{
				script_hero_flip = vbound(value / 10000, -1, 256);
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "ScriptFlip"))
				s->scriptflip = vbound(value / 10000, -1, 127);
			break;
		}
		case SPRITE_ENGINE_ANIMATE:
		{
			if (auto s = get_sprite(ri->spriteref, "Animation"))
				s->do_animation = value;
			break;
		}
		case SPRITE_EXTEND:
		{
			if (auto s = get_sprite(ri->spriteref, "Extend"))
				s->extend = value / 10000;
			break;
		}
		case SPRITE_HIT_WIDTH:
		{
			if (auto s = get_sprite(ri->spriteref, "HitWidth"))
				s->hit_width = value / 10000;
			break;
		}
		case SPRITE_HIT_HEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref, "HitHeight"))
				s->hit_height = value / 10000;
			break;
		}
		case SPRITE_HIT_ZHEIGHT:
		{
			if (auto s = get_sprite(ri->spriteref, "HitZHeight"))
				s->hzsz = value / 10000;
			break;
		}
		case SPRITE_HIT_OFFSET_X:
		{
			if (auto s = get_sprite(ri->spriteref, "HitXOffset"))
				s->hxofs = value / 10000;
			break;
		}
		case SPRITE_HIT_OFFSET_Y:
		{
			if (auto s = get_sprite(ri->spriteref, "HitYOffset"))
				s->hyofs = value / 10000;
			break;
		}
		case SPRITE_FALL_CLK:
		{
			if (is_player())
			{
				int val = vbound(value / 10000, 0, PITFALL_FALL_FRAMES);
				if (val)
					Hero.setAction(falling);
				else if (Hero.action == falling)
					Hero.setAction(none);
				Hero.fallclk = val;
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Falling"))
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
			if (auto s = get_sprite(ri->spriteref, "FallCombo"))
				s->fallCombo = vbound(value / 10000, 0, MAXCOMBOS - 1);
			break;
		}
		case SPRITE_MOVE_FLAGS:
		{
			if (auto s = get_sprite(ri->spriteref, "MoveFlags[]"))
			{
				int index = ri->d[rINDEX] / 10000;
				int limit = is_enemy(s) ? 15 : 10;
				if (BC::checkBounds(index, 0, limit, "sprite->MoveFlags[]") != SH::_NoError)
					break;

				move_flags bit = (move_flags)(1<<index);
				SETFLAG(s->moveflags, bit, value);
			}
			break;
		}
		case SPRITE_LIGHT_RADIUS:
		{
			if (auto s = get_sprite(ri->spriteref, "LightRadius"))
				s->glowRad = vbound(value / 10000, 0, 255);
			break;
		}
		case SPRITE_LIGHT_SHAPE:
		{
			if (auto s = get_sprite(ri->spriteref, "LightShape"))
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
			if (auto s = get_sprite(ri->spriteref, "ShadowSprite"))
				s->spr_shadow = vbound(value / 10000, 0, 255);
			break;
		}
		case SPRITE_DROWN_CLK:
		{
			if (is_player())
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

			if (auto s = get_sprite(ri->spriteref, "Drowning"))
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
			if (auto s = get_sprite(ri->spriteref, "DrownCombo"))
				s->drownCombo = vbound(value / 10000, 0, MAXCOMBOS - 1);
			break;
		}
		case SPRITE_SHADOW_XOFS:
		{
			if (auto s = get_sprite(ri->spriteref, "ShadowXOffset"))
				s->shadowxofs = value / 10000;
			break;
		}
		case SPRITE_SHADOW_YOFS:
		{
			if (auto s = get_sprite(ri->spriteref, "ShadowYOffset"))
				s->shadowyofs = value / 10000;
			break;
		}
		case SPRITE_MISCD:
		{
			// TODO don't vbound for player. check bounds, like the others.
			if (is_player())
			{
				Hero.miscellaneous[vbound(ri->d[rINDEX]/10000,0,31)] = value;
				break;
			}

			if (auto s = get_sprite(ri->spriteref, "Misc[]"))
			{
				int index = ri->d[rINDEX] / 10000;
				if (auto f = dynamic_cast<ffcdata*>(s))
				{
					// TODO: allow ffcs to use all 32
					if (BC::checkMisc(index, "sprite->Misc[]") != SH::_NoError)
						break;
				}
				else if (BC::checkMisc32(index, "sprite->Misc[]") != SH::_NoError)
					break;

				s->miscellaneous[index] = value;
			}
			break;
		}

		default: return false;
	}

	return true;
}
