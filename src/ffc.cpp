#include "base/zdefs.h"
#include "base/zsys.h"
#include "ffc.h"
#include "tiles.h"
#include "sprite.h"
#include "base/qrs.h"
#include "base/combo.h"
#include "zc/zc_ffc.h"

extern sprite_list Lwpns;

#ifdef IS_PLAYER
#include "zc/combos.h"
#include "zc/maps.h"
#include "zc/hero.h"
#include "base/mapscr.h"

extern int16_t lensclk;
extern HeroClass Hero;
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
	if (!data) return;
	if (flags&ffc_changer) return;
	#ifdef IS_PLAYER
	if ((flags&ffc_lensinvis) && lensclk) return; //If lens is active and ffc is invis to lens, don't draw
	if ((flags&ffc_lensvis) && !lensclk) return; //If FFC does not require lens, or lens is active, draw
	
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
	
	if(!(flags&ffc_overlay) == !overlay) //force cast both of these to boolean. They're both not, so same as if they weren't not.
	{
		int32_t tx = x + xofs;
		int32_t ty = y + yofs;
		
		if(flags&ffc_trans)
		{
			overcomboblocktranslucent(dest, tx, ty, data, cset, txsz, tysz,128);
		}
		else
		{
			overcomboblock(dest, tx, ty, data, cset, txsz, tysz);
		}
	}
}

bool ffcdata::setSolid(bool set) //exists so that ffcs can do special handling for whether to make something solid or not.
{
	bool actual = set && !(flags&ffc_changer) && loaded;
	bool ret = solid_object::setSolid(actual);
	solid = set;
	return ret;
}
void ffcdata::updateSolid()
{
	if(setSolid(flags&ffc_solid))
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
			Hero.movexy(dx, dy, false, false, false);
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
	if(loaded==set) return;
	loaded = set;
	updateSolid();
}
bool ffcdata::getLoaded() const
{
	return loaded;
}

void ffcdata::doContactDamage(int32_t hdir)
{
#ifdef IS_PLAYER
	if(flags & (ffc_changer | ffc_ethereal))
		return; //Changer or ethereal; has no type
	newcombo const& cmb = combobuf[data];
	if(data && isdamage_type(cmb.type))
	{
		int ffnum = -1;
		if(loaded)
		{
			int c = tmpscr->numFFC();
			for (word i = 0; i < c; i++)
			{
				if (this == &tmpscr->ffcs[i])
				{
					ffnum = i;
					break;
				}
			}
		}
		if(ffnum > -1)
		{
			trigger_damage_combo(data, ZSD_FFC, ffnum, hdir, true);
		}
		else trigger_damage_combo(data, ZSD_NONE, 0, hdir, true);
	}
#endif
}

