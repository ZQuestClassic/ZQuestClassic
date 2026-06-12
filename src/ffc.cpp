#include "core/zdefs.h"
#include "zalleg/zsys.h"
#include "ffc.h"
#include "tiles.h"
#include "sprite.h"
#include "core/qrs.h"
#include "core/combo.h"
#include "zc/zc_ffc.h"

extern sprite_list Lwpns;

#ifdef IS_PLAYER
#include "zc/maps.h"
#include "zc/hero.h"
#include "zc/combos.h"
#include "core/mapscr.h"
#include "iter.h"

extern int16_t lensclk;
#endif

void ffcdata::clear()
{
	*this = ffcdata();
}

void ffcdata::draw(BITMAP* dest)
{
	draw_ffc(dest, 0, 0, false);
}

void ffcdata::draw_ffc(BITMAP* dest, int32_t xofs, int32_t yofs, bool overlay)
{
	(void)overlay;

	if (flags&ffc_changer) return;
	#ifdef IS_PLAYER
	if(combobuf[data].animflags & AF_EDITOR_ONLY) return;
	if ((flags&ffc_lensinvis) && lensclk) return; //If lens is active and ffc is invis to lens, don't draw
	if ((flags&ffc_lensvis) && !lensclk) return; //If FFC does not require lens, or lens is active, draw
	if(!(flags&ffc_overlay) != !overlay) return; //force cast both of these to boolean. They're both not, so same as if they weren't not.
	
	if (switch_hooked)
	{
		switch(Hero.switchhookstyle)
		{
			default: case swPOOF:
				break;
			case swFLICKER:
			{
				if (abs(Hero.switchhookclk-33)&0b1000)
					break;
				return;
			}
			case swRISE:
				yofs -= 8-(abs(Hero.switchhookclk-32)/4);
				break;
		}
	}
	#endif
	
#ifdef IS_PLAYER
	int32_t tx = x + xofs - viewport.x;
	int32_t ty = y + yofs - viewport.y;
#else
	int32_t tx = x + xofs;
	int32_t ty = y + yofs;
#endif

	if(flags&ffc_trans)
	{
		overcomboblocktranslucent(dest, tx, ty, data, cset, txsz, tysz,128);
	}
	else
	{
		overcomboblock(dest, tx, ty, data, cset, txsz, tysz);
	}
}

bool ffcdata::checkSolid() const
{
	if (flags & ffc_solid)
		return true;
	if (flags & ffc_cmb_solid)
	{
		auto const& cmb = combobuf[data];
		if (cmb.walk & 0xF)
			return true;
	}
	return false;
}

bool ffcdata::setSolid(bool new_solid) //exists so that ffcs can do special handling for whether to make something solid or not.
{
	if (!loaded || (flags&ffc_changer))
		new_solid = false;
	return solid_object::setSolid(new_solid);
}
void ffcdata::updateSolid()
{
	if(setSolid(checkSolid()))
		solid_update(false);
}

void ffcdata::solid_update(bool push)
{
#ifdef IS_PLAYER
	if (push) // if 'push' is false, do NOT move the Hero or anything else
	{
		zfix dx = (x - old_x);
		zfix dy = (y - old_y);
		if ((flags & ffc_platform) && Hero.on_ffc_platform(*this, true))
		{
			if (get_qr(qr_BROKEN_SIDEVIEW_SOLID_FFC_COLLISION))
				Hero.movexy(dx, dy, false, false, false);
			else
			{
				bool t = getTempNonsolid();
				setTempNonsolid(true);
				Hero.push_move(dx, dy);
				setTempNonsolid(t);
			}
		}
		else if (hooked)
		{
			if (Lwpns.idFirst(wHookshot) > -1)
			{
				if (dx)
					Hero.setXfix(Hero.getX() + dx);
				if (dy)
					Hero.setYfix(Hero.getY() + dy);
			}
			else
				hooked = false;
		}
	}
#endif
	solid_object::solid_update(push);
}

void ffcdata::setLoaded(bool set)
{
	loaded = set;
}
bool ffcdata::getLoaded() const
{
	return loaded;
}

void ffcdata::doContactDamage(int32_t hdir)
{
	(void)hdir;

#ifdef IS_PLAYER
	if(flags & (ffc_changer | ffc_ethereal))
		return; //Changer or ethereal; has no type
	newcombo const& cmb = combobuf[data];
	if(data && isdamage_type(cmb.type))
	{
		int ffnum = -1;
		if(loaded)
		{
			auto ffc_handle = find_ffc([&](const ffc_handle_t& ffc_handle) {
				return this == ffc_handle.ffc;
			});
			if (ffc_handle)
			{
				ffnum = ffc_handle->id;
			}
		}
		if(ffnum > -1)
		{
			trigger_damage_combo(get_scr(screen_spawned), data, ZSD_FFC, ffnum, hdir, true);
		}
		else trigger_damage_combo(get_scr(screen_spawned), data, ZSD_NONE, 0, hdir, true);
	}
#endif
}

