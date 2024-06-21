#include "zc/zc_ffc.h"
#include "base/combo.h"
#include "base/general.h"
#include "base/mapscr.h"
#include "zc/guys.h"
#include "zc/maps.h"
#include "zc/zelda.h"

void zc_ffc_set(ffcdata& ffc, word data)
{
	ffc.data = data;

	for (word i = 0; i < MAXFFCS; i++)
	{
		if (&ffc == &tmpscr->ffcs[i])
		{
			screen_ffc_modify_postroutine(i);
			break;
		}
	}
}

void zc_ffc_update(ffcdata& ffc, word data)
{
	if(ffc.data != data)
		zc_ffc_set(ffc,data);
}

void zc_ffc_modify(ffcdata& ffc, int32_t amount)
{
	if(amount)
		zc_ffc_set(ffc, ffc.data + amount);
}

void zc_ffc_changer(ffcdata& ffc, ffcdata& other, int32_t i, int32_t j)
{
	// TODO: could use `ffc = other;` to replace most of this.
	if(other.flags&ffc_changethis)
	{
		zc_ffc_set(ffc, other.data);
		ffc.cset = other.cset;
	}
	
	if(other.flags&ffc_changenext)
		zc_ffc_modify(ffc, 1);
	
	if(other.flags&ffc_changeprev)
		zc_ffc_modify(ffc, -1);
	
	ffc.delay=other.delay;
	ffc.x=other.x;
	ffc.y=other.y;
	ffc.vx=other.vx;
	ffc.vy=other.vy;
	ffc.ax=other.ax;
	ffc.ay=other.ay;
	ffc.link=other.link;
	ffc.hit_width=other.hit_width;
	ffc.hit_height=other.hit_height;
	ffc.txsz=other.txsz;
	ffc.tysz=other.tysz;
	
	if(ffc.flags&ffc_carryover)
		ffc.flags=other.flags|ffc_carryover;
	else
		ffc.flags=other.flags;
	
	ffc.flags&=~ffc_changer;
	
	if(combobuf[other.data].flag>15 && combobuf[other.data].flag<32)
		zc_ffc_set(other, tmpscr->secretcombo[combobuf[other.data].flag-16+4]);
	
	if(i > -1 && j > -1)
	{
		ffposx[i]=(other.x.getInt());
		ffposy[i]=(other.y.getInt());
		if((other.flags&ffc_swapnext)||(other.flags&ffc_swapprev))
		{
			int32_t k=0;
			
			if(other.flags&ffc_swapnext)
				k=j<(MAXFFCS-1)?j+1:0;
				
			if(other.flags&ffc_swapprev)
				k=j>0?j-1:(MAXFFCS-1);
			ffcdata& ffck = tmpscr->ffcs[k];
			auto w = ffck.data;
			zc_ffc_set(ffck, other.data);
			zc_ffc_set(other, w);

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
	ffc.updateSolid();
	ffc.solid_update(false);
}

