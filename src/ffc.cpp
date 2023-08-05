#include "base/zdefs.h"
#include "base/zsys.h"
#include "ffc.h"
#include "tiles.h"
#include "sprite.h"
#include "base/qrs.h"
#include "base/combo.h"

extern sprite_list Lwpns;

#ifdef IS_PLAYER
#include "zc/maps.h"
#include "zc/hero.h"
#include "zc/combos.h"
#include "base/mapscr.h"
#include "iter.h"

extern mapscr* tmpscr;
extern int16_t lensclk;
extern HeroClass Hero;
void screen_ffc_modify_postroutine(const ffc_handle_t& ffc_handle);
#endif

ffcdata::ffcdata(ffcdata const& other)
{
	*this = other;
	setData(data);
	updateSolid();
}

void ffcdata::clear()
{
	*this = ffcdata();
	setData(0);
	updateSolid();
}

void ffcdata::changerCopy(ffcdata& other, int32_t i, int32_t j)
{
	// TODO: could use `*this = other;` to replace most of this.
#ifdef IS_PLAYER
	if(other.flags&ffCHANGETHIS)
	{
		setData(other.data);
		cset = other.cset;
	}
	
	if(other.flags&ffCHANGENEXT)
		incData(1);
	
	if(other.flags&ffCHANGEPREV)
		incData(-1);
	
	delay=other.delay;
	x=other.x;
	y=other.y;
	vx=other.vx;
	vy=other.vy;
	ax=other.ax;
	ay=other.ay;
	link=other.link;
	hit_width=other.hit_width;
	hit_height=other.hit_height;
	txsz=other.txsz;
	tysz=other.tysz;
	
	if(flags&ffCARRYOVER)
		flags=other.flags|ffCARRYOVER;
	else
		flags=other.flags;
	
	flags&=~ffCHANGER;
	
	if(combobuf[other.data].flag>15 && combobuf[other.data].flag<32)
		other.setData(tmpscr->secretcombo[combobuf[other.data].flag-16+4]);
	
	if(i > -1 && j > -1)
	{
		changer_x = other.x.getInt();
		changer_y = other.y.getInt();
		if((other.flags&ffSWAPNEXT)||(other.flags&ffSWAPPREV))
		{
			int32_t k=0;
			
			if(other.flags&ffSWAPNEXT)
				k=j<(MAXFFCS-1)?j+1:0;
				
			if(other.flags&ffSWAPPREV)
				k=j>0?j-1:(MAXFFCS-1);
			ffcdata& ffck = tmpscr->ffcs[k];
			auto w = ffck.data;
			ffck.setData(other.data);
			other.setData(w);
			zc_swap(other.cset,ffck.cset);
			zc_swap(other.delay,ffck.delay);
			zc_swap(other.vx,ffck.vx);
			zc_swap(other.vy,ffck.vy);
			zc_swap(other.ax,ffck.ax);
			zc_swap(other.ay,ffck.ay);
			zc_swap(other.link,ffck.link);
			zc_swap(other.hit_width,ffck.hit_width);
			zc_swap(other.hit_height,ffck.hit_height);
			zc_swap(other.txsz,ffck.txsz);
			zc_swap(other.tysz,ffck.tysz);
			zc_swap(other.flags,ffck.flags);
		}
	}
	updateSolid();
	solid_update(false);
#endif
}

void ffcdata::setData(word newdata)
{
	data = newdata;

#if IS_PLAYER
	mapscr* screen = get_scr_no_load(currmap, screen_index);
	if (!screen) return;

	uint8_t screen_index_offset = get_region_screen_index_offset(screen_index);
	for (uint8_t i = 0; i < MAXFFCS; i++)
	{
		if (this == &screen->ffcs[i])
		{
			uint16_t id = screen_index_offset * MAXFFCS + i;
			screen_ffc_modify_postroutine({screen, screen_index, id, i, this});
			return;
		}
	}
#endif
}
void ffcdata::incData(int32_t inc)
{
	setData(data+inc);
}

void ffcdata::draw(BITMAP* dest, int32_t xofs, int32_t yofs, bool overlay)
{
	if (flags&ffCHANGER) return;
	#ifdef IS_PLAYER
	if ((flags&ffLENSINVIS) && lensclk) return; //If lens is active and ffc is invis to lens, don't draw
	if ((flags&ffLENSVIS) && !lensclk) return; //If FFC does not require lens, or lens is active, draw
	
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
	
	if(!(flags&ffOVERLAY) == !overlay) //force cast both of these to boolean. They're both not, so same as if they weren't not.
	{
		int32_t tx = x + xofs - viewport.x;
		int32_t ty = y + yofs - viewport.y;
		
		if(flags&ffTRANS)
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
	bool actual = set && !(flags&ffCHANGER) && loaded;
	bool ret = solid_object::setSolid(actual);
	solid = set;
	return ret;
}
void ffcdata::updateSolid()
{
	if(setSolid(flags&ffSOLID))
		solid_update(false);
}

void ffcdata::solid_update(bool push)
{
#ifdef IS_PLAYER
	if (hooked && push)
	{
		if (Lwpns.idFirst(wHookshot) > -1)
		{
			zfix dx = (x - old_x);
			zfix dy = (y - old_y);
			if (dx) 
				Hero.setXfix(Hero.getX() + dx);
			if (dy)
				Hero.setYfix(Hero.getY() + dy);
		}
		else
			hooked = false;
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
	if(flags & (ffCHANGER | ffETHEREAL))
		return; //Changer or ethereal; has no type
	newcombo const& cmb = combobuf[data];
	if(data && isdamage_type(cmb.type))
	{
		int ffnum = -1;
		if(loaded)
		{
			auto ffc_handle = find_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
				return this == ffc_handle.ffc;
			});
			if (ffc_handle)
			{
				ffnum = ffc_handle->id;
			}
		}
		if(ffnum > -1)
		{
			trigger_damage_combo(get_scr(currmap, screen_index), data, ZSD_FFC, ffnum, hdir, true);
		}
		else trigger_damage_combo(get_scr(currmap, screen_index), data, ZSD_NONE, 0, hdir, true);
	}
#endif
}
