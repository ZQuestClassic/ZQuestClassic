#include "base/qrs.h"
#include "base/zdefs.h"
#include "items.h"
#include "zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "zc/hero.h"
#include "zc/rendertarget.h"
#include "zc/scripting/types/item.h"
#include "zc/zelda.h"

#include <optional>

extern refInfo *ri;

namespace {

static item *tempitem = NULL;

item *checkItem(int32_t iid)
{
	item *s = (item *)items.getByUID(iid);
	
	if(s == NULL)
	{
		Z_eventlog("Script attempted to reference a nonexistent item!\n");
		Z_eventlog("You were trying to reference an item with UID = %ld; Items on screen are UIDs ", iid);
		
		for(int32_t i=0; i<items.Count(); i++)
		{
			Z_eventlog("%ld ", items.spr(i)->getUID());
		}
		
		Z_eventlog("\n");
		return NULL;
	}
	
	return s;
}

bool do_itemsprite_delete()
{
	if (checkItem(ri->itemref))
	{
		auto ind = ItemH::getItemIndex(ri->itemref);
		if(!items.del(ind)) return false;
		for(int32_t i=0; i<Lwpns.Count(); i++)
		{
			weapon *w = (weapon*)Lwpns.spr(i);
			
			if(w->dragging==ind)
			{
				w->dragging=-1;
			}
			else if(w->dragging>ind)
			{
				w->dragging-=1;
			}
		}
		return true;
	}

	return false;
}

} // end namespace

int32_t ItemH::loadItem(int32_t uid)
{
	tempitem = ResolveItemSprite(uid);
	if (!tempitem) 
		return _InvalidSpriteUID;

	return _NoError;
}

int32_t ItemH::getItemIndex(int32_t uid)
{
	for(word i = 0; i < items.Count(); i++)
	{
		if(items.spr(i)->getUID() == uid)
			return i;
	}
	
	return -1;
}

item *ItemH::getItem()
{
	return tempitem;
}

void ItemH::clearTemp()
{
	tempitem = NULL;
}

std::optional<int32_t> item_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case ITEMSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)s->scale)*100.0;
			}
			break;
		
		case ITEMX:
		{
			if (auto s = checkItem(ri->itemref))
			{
				zfix x;
				bool is_fairy = itemsbuf[s->id].family==itype_fairy && itemsbuf[s->id].misc3;
				if (is_fairy)
				{
					enemy* fairy = (enemy*) guys.getByUID(s->fairyUID);
					x = fairy ? fairy->x : s->x;
				}
				else
				{
					x = s->x;
				}

				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=x.getZLong();
				}
				else ret = (int32_t)x * 10000;
			}
		}
		break;

		case ITEMY:
		{
			if (auto s = checkItem(ri->itemref))
			{
				zfix y;
				bool is_fairy = itemsbuf[s->id].family==itype_fairy && itemsbuf[s->id].misc3;
				if (is_fairy)
				{
					enemy* fairy = (enemy*) guys.getByUID(s->fairyUID);
					y = fairy ? fairy->y : s->y;
				}
				else
				{
					y = s->y;
				}

				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=y.getZLong();
				}
				else ret = (int32_t)y * 10000;
			}
		}
		break;

		case ITEMSPRITESCRIPT:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)s->script)*10000;
			}
			break;
		
		case ITEMSPRITEINITD:
			if (auto s = checkItem(ri->itemref))
			{
				int32_t a = vbound(ri->d[rINDEX]/10000,0,7);
				ret=((int32_t)s->initD[a]);
			}
			break;
		
		case ITEMFAMILY:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)s->family)*10000;
			}
			break;
		
		case ITEMLEVEL:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)s->lvl)*10000;
			}
			break;
			
		case SPRITEMAXITEM:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			ret = items.getMax() * 10000;
			break;
		}
		
		case ITEMSCRIPTUID:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)s->getUID());
			}
			break;
		
			
		case ITEMZ:
			if (auto s = checkItem(ri->itemref))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(s->z).getZLong();    
				}
				else 
					ret=((int32_t)s->z)*10000;
			}
			break;
			
		case ITEMJUMP:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
			break;
		
		case ITEMFAKEJUMP:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
			break;
			
		case ITEMDRAWTYPE:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->drawstyle*10000;
			}
			break;
		  
		case ITEMGRAVITY:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((s->moveflags & move_obeys_grav) ? 10000 : 0);
			}
			break;
			
		case ITEMID:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->id*10000;
			}
			break;
			
		case ITEMTILE:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->tile*10000;
			}
			break;
			
		case ITEMSCRIPTTILE:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->scripttile*10000;
			}
			break;
			
		case ITEMSCRIPTFLIP:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->scriptflip*10000;
			}
			break;
		
		case ITEMPSTRING:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->pstring*10000;
			}
			break;
		case ITEMPSTRINGFLAGS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->pickup_string_flags*10000;
			}
			break;
		case ITEMOVERRIDEFLAGS:
			ret=0;
			break;
			
		case ITEMOTILE:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->o_tile*10000;
			}
			break;
			
		case ITEMCSET:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->o_cset&15)*10000;
			}
			break;
			
		case ITEMFLASHCSET:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->o_cset>>4)*10000;
			}
			break;
			
		case ITEMFRAMES:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->frames*10000;
			}
			break;
			
		case ITEMFRAME:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->aframe*10000;
			}
			break;
		
		case ITEMACLK:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->aclk*10000;
			}
			break;    
		
		case ITEMASPEED:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->o_speed*10000;
			}
			break;
			
		case ITEMDELAY:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->o_delay*10000;
			}
			break;
			
		case ITEMFLIP:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->flip*10000;
			}
			break;
			
		case ITEMFLASH:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->flash*10000;
			}
			break;
			
		case ITEMHXOFS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->hxofs)*10000;
			}
			break;

		case ITEMROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->rotation)*10000;
			}
			break;

		case ITEMHYOFS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->hyofs)*10000;
			}
			break;
			
		case ITEMXOFS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)(s->xofs))*10000;
			}
			break;
			
		case ITEMYOFS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))*10000;
			}
			break;
		
		case ITEMSHADOWXOFS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)(s->shadowyofs))*10000;
			}
			break;
			
		case ITEMSHADOWYOFS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)(s->shadowxofs))*10000;
			}
			break;
			
			
		case ITEMZOFS:
			if (auto s = checkItem(ri->itemref))
			{
				ret=((int32_t)(s->zofs))*10000;
			}
			break;
			
		case ITEMHXSZ:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->hit_width)*10000;
			}
			break;
			
		case ITEMHYSZ:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->hit_height)*10000;
			}
			break;
			
		case ITEMHZSZ:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->hzsz)*10000;
			}
			break;
			
		case ITEMTXSZ:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->txsz)*10000;
			}
			break;
			
		case ITEMTYSZ:
			if (auto s = checkItem(ri->itemref))
			{
				ret=(s->tysz)*10000;
			}
			break;
			
		case ITEMCOUNT:
			ret=(items.Count())*10000;
			break;
		
		case GETRENDERTARGET:
			ret=(zscriptDrawingRenderTarget->GetCurrentRenderTarget())*10000;
			break;
			
		case ITEMEXTEND:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->extend*10000;
			}
			break;
			
		case ITEMPICKUP:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->pickup*10000;
			}
			break;
			
			
		case ITEMMISCD:
			if (auto s = checkItem(ri->itemref))
			{
				int32_t a = vbound(ri->d[rINDEX]/10000,0,31);
				ret=(s->miscellaneous[a]);
			}
			break;
		
		case ITEMFALLCLK:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->fallclk * 10000;
			}
			break;
		
		case ITEMFALLCMB:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->fallCombo * 10000;
			}
			break;
		
		case ITEMDROWNCLK:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->drownclk * 10000;
			}
			break;
		
		case ITEMDROWNCMB:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->drownCombo * 10000;
			}
			break;
		
		case ITEMFAKEZ:
			if (auto s = checkItem(ri->itemref))
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(s->fakez).getZLong();    
				}
				else 
					ret=((int32_t)s->fakez)*10000;
			}
			break;
			
		
		case ITEMMOVEFLAGS:
		{
			if (auto s = checkItem(ri->itemref))
			{
				int32_t indx = ri->d[rINDEX]/10000;
				if(BC::checkIndex(indx, 0, 10) != SH::_NoError)
					ret = 0; //false
				else
				{
					//All bits, in order, of a single byte; just use bitwise
					ret = (s->moveflags & (1<<indx)) ? 10000 : 0;
				}
			}
			break;
		}
		
		case ITEMGLOWRAD:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->glowRad * 10000;
			}
			break;
			
		case ITEMGLOWSHP:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->glowShape * 10000;
			}
			break;
			
		case ITEMDIR:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->dir * 10000;
			}
			break;
			
		case ITEMENGINEANIMATE:
			if (auto s = checkItem(ri->itemref))
			{
				ret = int32_t(s->do_animation) * 10000;
			}
			break;
			
		case ITEMSHADOWSPR:
			if (auto s = checkItem(ri->itemref))
			{
				ret = int32_t(s->spr_shadow) * 10000;
			}
			break;
		case ITEMDROPPEDBY:
			if (auto s = checkItem(ri->itemref))
			{
				ret = int32_t(s->from_dropset) * 10000;
			}
			break;
		case ITMSWHOOKED:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->switch_hooked ? 10000 : 0;
			}
			break;
		case ITEMFORCEGRAB:
			if (auto s = checkItem(ri->itemref))
			{
				ret = s->get_forcegrab() ? 10000 : 0;
			}
			break;
			
		case ITEMNOSOUND:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->noSound ? 10000 : 0;
			}
			break;
			
		case ITEMNOHOLDSOUND:
			if (auto s = checkItem(ri->itemref))
			{
				ret=s->noHoldSound ? 10000 : 0;
			}
			break;

		default: return std::nullopt;
	}

	return ret;
}

bool item_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case ITEMFAMILY:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->family)=value/10000;
			}
			
			break;
		
		case ITEMLEVEL:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->lvl)=value/10000;
			}
			
			break;
			
		case SPRITEMAXITEM:
		{
			//No bounds check, as this is a universal function and works from NULL pointers!
			items.setMax(vbound((value/10000),1,MAX_ITEM_SPRITES));
			break;
		}
		
		case ITEMX:
			if (auto s = checkItem(ri->itemref))
			{
				s->x = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				
				// Move the Fairy enemy as well.
				if(itemsbuf[s->id].family==itype_fairy && itemsbuf[s->id].misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(s->fairyUID);
					if (fairy)
						fairy->x = s->x;
				}
			}
			break;

		case ITEMY:
			if (auto s = checkItem(ri->itemref))
			{
				s->y = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				
				// Move the Fairy enemy as well.
				if(itemsbuf[s->id].family==itype_fairy && itemsbuf[s->id].misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(s->fairyUID);
					if (fairy)
						fairy->y = s->y;
				}
			}
			break;
		
		case ITEMSPRITESCRIPT:
			FFScript::deallocateAllScriptOwned(ScriptType::ItemSprite, ri->itemref);
			if (auto s = checkItem(ri->itemref))
			{
				(s->script)=(value/10000);
			}
			break;
		
		case ITEMSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if (auto s = checkItem(ri->itemref))
			{
				(s->scale)=(zfix)(value/100.0);
			}
			
			break;
			
		case ITEMZ:
			if (auto s = checkItem(ri->itemref))
			{
				(s->z)=(zfix)(value/10000);
				
				if(s->z < 0)
					s->z = 0;
			}
			
			break;
			
		case ITEMJUMP:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->fall)=zslongToFix(value)*-100;
			}
			
			break;
		
		case ITEMFAKEJUMP:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->fakefall)=zslongToFix(value)*-100;
			}
			
			break;
			
		case ITEMDRAWTYPE:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->drawstyle)=value/10000;
			}
			
			break;
			
		 case ITEMSPRITEINITD:
			if (auto s = checkItem(ri->itemref))
			{
				int32_t a = vbound(ri->d[rINDEX]/10000,0,7);
				(((item *)s)->initD[a])=value;
			}
			
			break;
			
		case ITEMGRAVITY:
			if (auto s = checkItem(ri->itemref))
			{
				if(value)
					((item *)s)->moveflags |= move_obeys_grav;
				else
					((item *)s)->moveflags &= ~move_obeys_grav;
			}
			
			break;
			
		case ITEMID:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->id)=value/10000;
				flushItemCache();
			}
			
			break;
			
		case ITEMTILE:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->tile)=vbound(value/10000,0,NEWMAXTILES-1);
			}
			
			break;
			
		case ITEMSCRIPTTILE:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->scripttile)=vbound(value/10000,-1,NEWMAXTILES-1);
			}
			break;
			
		case ITEMSCRIPTFLIP:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->scriptflip)=vbound((value/10000),-1,127);
			}
			break;
		
		case ITEMPSTRING:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->pstring)=vbound(value/10000,0,(msg_count-1));
			}
			
			break;
		
		case ITEMPSTRINGFLAGS:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->pickup_string_flags)=vbound(value/10000, 0, 214748);
			}
			
			break;
		
		case ITEMOVERRIDEFLAGS:
			break;
			
		case ITEMOTILE:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->o_tile)=vbound(value/10000,0,NEWMAXTILES-1);
			}
			
			break;
			
		case ITEMCSET:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->o_cset) = (((item *)s)->o_cset & ~15) | ((value/10000)&15);
				(((item *)s)->cs) = (((item *)s)->o_cset & 15);
			}
			
			break;
			
		case ITEMFLASHCSET:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->o_cset) = ((value/10000)<<4) | (((item *)s)->o_cset & 15);
			}
			
			break;
			
		case ITEMFRAMES:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->frames)=value/10000;
			}
			
			break;
			
		case ITEMFRAME:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->aframe)=value/10000;
			}
			
			break;
			
		case ITEMASPEED:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->o_speed)=value/10000;
			}
			
			break;
		
		 case ITEMACLK:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->aclk)=value/10000;
			}
			
			break;
		
		case ITEMDELAY:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->o_delay)=value/10000;
			}
			
			break;
			
		case ITEMFLIP:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->flip)=value/10000;
			}
			
			break;
			
		case ITEMFLASH:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->flash)= (value/10000)?1:0;
			}
			
			break;
			
		case ITEMEXTEND:
			if (auto s = checkItem(ri->itemref))
			{
				(((item *)s)->extend)=value/10000;
			}
			
			break;
			
		case ITEMHXOFS:
			if (auto s = checkItem(ri->itemref))
			{
				s->hxofs=value/10000;
			}
			
			break;
			
		case ITEMROTATION:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if (auto s = checkItem(ri->itemref))
			{
				s->rotation=value/10000;
			}
			
			break;
			
		case ITEMHYOFS:
			if (auto s = checkItem(ri->itemref))
			{
				s->hyofs=value/10000;
			}
			
			break;
			
		case ITEMXOFS:
			if (auto s = checkItem(ri->itemref))
			{
				s->xofs=(zfix)(value/10000);
			}
			
			break;
			
		case ITEMYOFS:
			if (auto s = checkItem(ri->itemref))
			{
				s->yofs=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			}
			
			break;
			
		case ITEMSHADOWXOFS:
			if (auto s = checkItem(ri->itemref))
			{
				s->shadowxofs=(zfix)(value/10000);
			}
			
			break;
		
		case ITEMSHADOWYOFS:
			if (auto s = checkItem(ri->itemref))
			{
				s->shadowyofs=(zfix)(value/10000);
			}
			
			break;
		
		case ITEMZOFS:
			if (auto s = checkItem(ri->itemref))
			{
				s->zofs=(zfix)(value/10000);
			}
			
			break;
			
		case ITEMHXSZ:
			if (auto s = checkItem(ri->itemref))
			{
				s->hit_width=value/10000;
			}
			
			break;
			
		case ITEMHYSZ:
			if (auto s = checkItem(ri->itemref))
			{
				s->hit_height=value/10000;
			}
			
			break;
			
		case ITEMHZSZ:
			if (auto s = checkItem(ri->itemref))
			{
				s->hzsz=value/10000;
			}
			
			break;
			
		case ITEMTXSZ:
			if (auto s = checkItem(ri->itemref))
			{
				s->txsz=vbound((value/10000),1,20);
			}
			
			break;
			
		case ITEMTYSZ:
			if (auto s = checkItem(ri->itemref))
			{
				s->tysz=vbound((value/10000),1,20);
			}
			
			break;
			
		case ITEMPICKUP:
			if (auto s = checkItem(ri->itemref))
			{
				int32_t newpickup = value/10000;
				// Values that the questmaker should not use, ever
				//Allowing it, for now, until something breaks. -Z 21-Jan-2020
				//newpickup &= ~(ipBIGRANGE | ipCHECK | ipMONEY | ipBIGTRI | ipNODRAW | ipFADE);
				//
				if (( FFCore.GetQuestVersion() == 0x250 && FFCore.GetQuestBuild() < 33 ) //this ishowit looks in 2.53.1, Beta 25
					|| ( FFCore.GetQuestVersion() < 0x250  ))
				{
					newpickup &= ~(ipBIGRANGE | ipCHECK | ipMONEY | ipBIGTRI | ipNODRAW | ipFADE);
				}
				
				// If making an item timeout, set its timer
				if(newpickup & ipFADE)
				{
					(s->clk2) = 512;
				}
				//else if(newpickup & ~ipFADE)
				//{
				//    (s->clk2) = 0;
				//}
				
				// If making it a carried item,
				// alter item state and set an itemguy.
				if((s->pickup & ipENEMY) < (newpickup & ipENEMY))
				{
					screen_item_set_state(s->screen_spawned, ScreenItemState::CarriedByEnemy);
					bool hasitemguy = false;
					
					for(int32_t i=0; i<guys.Count(); i++)
					{
						if(((enemy*)guys.spr(i))->itemguy)
						{
							hasitemguy = true;
						}
					}
					
					if(!hasitemguy && guys.Count()>0)
					{
						((enemy*)guys.spr(guys.Count()-1))->itemguy = true;
					}
				}
				// If unmaking it a carried item,
				// alter hasitem if there are no more carried items.
				else if((s->pickup & ipENEMY) > (newpickup & ipENEMY))
				{
					// Move it back onscreen!
					if(get_qr(qr_HIDECARRIEDITEMS))
					{
						for(int32_t i=0; i<guys.Count(); i++)
						{
							if(((enemy*)guys.spr(i))->itemguy)
							{
								if (!get_qr(qr_BROKEN_ITEM_CARRYING))
								{
									if (get_qr(qr_ENEMY_DROPS_USE_HITOFFSETS))
									{
										s->x = ((enemy*)guys.spr(i))->x+((enemy*)guys.spr(i))->hxofs+(((enemy*)guys.spr(i))->hit_width/2)-8;
										s->y = ((enemy*)guys.spr(i))->y+((enemy*)guys.spr(i))->hyofs+(((enemy*)guys.spr(i))->hit_height/2)-10;
										s->z = ((enemy*)guys.spr(i))->z;
									}
									else
									{
										if(((enemy*)guys.spr(i))->extend >= 3) 
										{
											s->x = ((enemy*)guys.spr(i))->x+(((enemy*)guys.spr(i))->txsz-1)*8;
											s->y = ((enemy*)guys.spr(i))->y-2+(((enemy*)guys.spr(i))->tysz-1)*8;
											s->z = ((enemy*)guys.spr(i))->z;
										}
										else 
										{
											s->x = ((enemy*)guys.spr(i))->x;
											s->y = ((enemy*)guys.spr(i))->y - 2;
											s->z = ((enemy*)guys.spr(i))->z;
										}
									}
								}
								else
								{
									s->x = ((enemy*)guys.spr(i))->x;
									s->y = ((enemy*)guys.spr(i))->y - 2;
									s->z = ((enemy*)guys.spr(i))->z;
								}
								break;
							}
						}
					}
					
					if(more_carried_items(s->screen_spawned)<=1)  // 1 includes this own item.
					{
						screen_item_set_state(s->screen_spawned, ScreenItemState::None);
					}
				}
				
				s->pickup=value/10000;
			}
			
			break;
			
		case ITEMMISCD:
			if (auto s = checkItem(ri->itemref))
			{
				int32_t a = vbound(ri->d[rINDEX]/10000,0,31);
				(s->miscellaneous[a])=value;
			}
			
			break;
		case ITEMFALLCLK:
			if (auto s = checkItem(ri->itemref))
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
		case ITEMFALLCMB:
			if (auto s = checkItem(ri->itemref))
			{
				s->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case ITEMDROWNCLK:
			if (auto s = checkItem(ri->itemref))
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
		case ITEMDROWNCMB:
			if (auto s = checkItem(ri->itemref))
			{
				s->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case ITEMFAKEZ:
			if (auto s = checkItem(ri->itemref))
			{
				(s->fakez)=(zfix)(value/10000);
				
				if(s->fakez < 0)
					s->fakez = 0;
			}
			
			break;
		
		case ITEMMOVEFLAGS:
		{
			if (auto s = checkItem(ri->itemref))
			{
				int32_t indx = ri->d[rINDEX]/10000;
				if(BC::checkIndex(indx, 0, 10) == SH::_NoError)
				{
					//All bits, in order, of a single byte; just use bitwise
					move_flags bit = (move_flags)(1<<indx);
					if(value)
						s->moveflags |= bit;
					else
						s->moveflags &= ~bit;
				}
			}
			break;
		}
		
		case ITEMGLOWRAD:
			if (auto s = checkItem(ri->itemref))
			{
				s->glowRad = vbound(value/10000,0,255);
			}
			break;
			
		case ITEMGLOWSHP:
			if (auto s = checkItem(ri->itemref))
			{
				s->glowShape = vbound(value/10000,0,255);
			}
			break;
			
		case ITEMDIR:
			if (auto s = checkItem(ri->itemref))
			{
				s->dir=(value/10000);
			}
			break;
			
		case ITEMENGINEANIMATE:
			if (auto s = checkItem(ri->itemref))
			{
				s->do_animation=value;
			}
			break;
			
		case ITEMSHADOWSPR:
			if (auto s = checkItem(ri->itemref))
			{
				s->spr_shadow=vbound(value/10000,0,255);
			}
			break;
		case ITEMDROPPEDBY:
			if (auto s = checkItem(ri->itemref))
			{
				s->from_dropset=vbound(value/10000,-1,255);
			}
			break;
		case ITMSWHOOKED:
			break; //read-only
		case ITEMFORCEGRAB:
			if (auto s = checkItem(ri->itemref))
			{
				s->set_forcegrab(value!=0);
			}
			break;
		case ITEMNOSOUND:
			if (auto s = checkItem(ri->itemref))
			{
				s->noSound = (value!=0);
			}
			break;
		case ITEMNOHOLDSOUND:
			if (auto s = checkItem(ri->itemref))
			{
				s->noHoldSound = (value!=0);
			}
			break;

		default: return false;
	}

	return true;
}

std::optional<int32_t> item_run_command(word command)
{
	extern int32_t sarg1;
	extern int32_t sarg2;
	extern int32_t sarg3;
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;

	switch (command)
	{
		case ITEMDEL:
		{
			FFScript::deallocateAllScriptOwned(ScriptType::ItemSprite, ri->itemref);
			if(type == ScriptType::ItemSprite && ri->itemref == i)
			{
				if(do_itemsprite_delete())
					return RUNSCRIPT_SELFDELETE;
			}
			else do_itemsprite_delete();
			break;
		}

		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}
