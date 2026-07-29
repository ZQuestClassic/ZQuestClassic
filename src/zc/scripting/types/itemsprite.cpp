#include "zc/scripting/types/itemsprite.h"

#include "core/qrs.h"
#include "core/zdefs.h"
#include "items.h"
#include "components/zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "zc/hero.h"
#include "zc/rendertarget.h"
#include "zc/scripting/arrays.h"
#include "zc/zelda.h"

#include <optional>

extern refInfo *ri;

namespace {

static item *tempitem = NULL;

bool do_itemsprite_delete()
{
	if (checkItem(GET_REF(itemref)))
	{
		auto ind = ItemH::getItemIndex(GET_REF(itemref));
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

int32_t itemsprite_get_register(int32_t reg)
{
	switch (reg)
	{
		case ITEMCOUNT:
			return items.Count() * 10000;
		case SPRITEMAXITEM:
			return items.getMax() * 10000;
	}

	int32_t ret = 0;
	item* s = checkItem(GET_REF(itemref));

	switch (reg)
	{
		case ITEMSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				ret = -1; break;
			}
			if (s)
			{
				ret=((int32_t)s->scale)*100.0;
			}
			break;
		
		case ITEMX:
		{
			if (s)
			{
				zfix x;
				auto const& fairy_item = itemsbuf.get(s->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
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
			if (s)
			{
				zfix y;
				auto const& fairy_item = itemsbuf.get(s->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
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
			if (s)
			{
				ret=((int32_t)s->scrconfig.script)*10000;
			}
			break;

		case ITEMTYPE:
			if (s)
			{
				ret=((int32_t)s->type)*10000;
			}
			break;
		
		case ITEMLEVEL:
			if (s)
			{
				ret=((int32_t)s->lvl)*10000;
			}
			break;
		
		case ITEMSCRIPTUID:
			if (s)
			{
				ret=((int32_t)s->getUID());
			}
			break;
		
			
		case ITEMZ:
			if (s)
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
			if (s)
			{
				ret = s->fall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
			break;
		
		case ITEMFAKEJUMP:
			if (s)
			{
				ret = s->fakefall.getZLong() / -100;
				if (get_qr(qr_SPRITE_JUMP_IS_TRUNCATED)) ret = trunc(ret / 10000) * 10000;
			}
			break;
			
		case ITEMDRAWTYPE:
			if (s)
			{
				ret=s->drawstyle*10000;
			}
			break;
		  
		case ITEMGRAVITY:
			if (s)
			{
				ret=((s->moveflags & move_obeys_grav) ? 10000 : 0);
			}
			break;
			
		case ITEMID:
			if (s)
			{
				ret=s->id*10000;
			}
			break;
			
		case ITEMTILE:
			if (s)
			{
				ret=s->tile*10000;
			}
			break;
			
		case ITEMSCRIPTTILE:
			if (s)
			{
				ret=s->scripttile*10000;
			}
			break;
			
		case ITEMSCRIPTFLIP:
			if (s)
			{
				ret=s->scriptflip*10000;
			}
			break;
		
		case ITEMPSTRING:
			if (s)
			{
				ret=s->pstring*10000;
			}
			break;
		case ITEMPSTRINGFLAGS:
			if (s)
			{
				ret=s->pickup_string_flags*10000;
			}
			break;
			
		case ITEMOTILE:
			if (s)
			{
				ret=s->o_tile*10000;
			}
			break;
			
		case ITEMCSET:
			if (s)
			{
				ret=(s->o_cset&15)*10000;
			}
			break;
			
		case ITEMFLASHCSET:
			if (s)
			{
				ret=(s->o_cset>>4)*10000;
			}
			break;
			
		case ITEMFRAMES:
			if (s)
			{
				ret=s->frames*10000;
			}
			break;
			
		case ITEMFRAME:
			if (s)
			{
				ret=s->aframe*10000;
			}
			break;
		
		case ITEMACLK:
			if (s)
			{
				ret=s->aclk*10000;
			}
			break;    
		
		case ITEMASPEED:
			if (s)
			{
				ret=s->o_speed*10000;
			}
			break;
			
		case ITEMDELAY:
			if (s)
			{
				ret=s->o_delay*10000;
			}
			break;
			
		case ITEMFLIP:
			if (s)
			{
				ret=s->flip*10000;
			}
			break;
			
		case ITEMFLASH:
			if (s)
			{
				ret=s->flash*10000;
			}
			break;
			
		case ITEMHXOFS:
			if (s)
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
			if (s)
			{
				ret=(s->rotation)*10000;
			}
			break;

		case ITEMHYOFS:
			if (s)
			{
				ret=(s->hyofs)*10000;
			}
			break;
			
		case ITEMXOFS:
			if (s)
			{
				ret=((int32_t)(s->xofs))*10000;
			}
			break;
			
		case ITEMYOFS:
			if (s)
			{
				ret=((int32_t)(s->yofs-(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset)))*10000;
			}
			break;
		
		case ITEMSHADOWXOFS:
			if (s)
			{
				ret=((int32_t)(s->shadowyofs))*10000;
			}
			break;
			
		case ITEMSHADOWYOFS:
			if (s)
			{
				ret=((int32_t)(s->shadowxofs))*10000;
			}
			break;
			
			
		case ITEMZOFS:
			if (s)
			{
				ret=((int32_t)(s->zofs))*10000;
			}
			break;
			
		case ITEMHXSZ:
			if (s)
			{
				ret=(s->hit_width)*10000;
			}
			break;
			
		case ITEMHYSZ:
			if (s)
			{
				ret=(s->hit_height)*10000;
			}
			break;
			
		case ITEMHZSZ:
			if (s)
			{
				ret=(s->hzsz)*10000;
			}
			break;
			
		case ITEMTXSZ:
			if (s)
			{
				ret=(s->txsz)*10000;
			}
			break;
			
		case ITEMTYSZ:
			if (s)
			{
				ret=(s->tysz)*10000;
			}
			break;
			
		case ITEMEXTEND:
			if (s)
			{
				ret=s->extend*10000;
			}
			break;
			
		case ITEMPICKUP:
			if (s)
			{
				ret=s->pickup*10000;
			}
			break;

		case ITEMFALLCLK:
			if (s)
			{
				ret = s->fallclk * 10000;
			}
			break;
		
		case ITEMFALLCMB:
			if (s)
			{
				ret = s->fallCombo * 10000;
			}
			break;
		
		case ITEMDROWNCLK:
			if (s)
			{
				ret = s->drownclk * 10000;
			}
			break;
		
		case ITEMDROWNCMB:
			if (s)
			{
				ret = s->drownCombo * 10000;
			}
			break;
		
		case ITEMFAKEZ:
			if (s)
			{
				if ( get_qr(qr_SPRITEXY_IS_FLOAT) )
				{
					ret=(s->fakez).getZLong();    
				}
				else 
					ret=((int32_t)s->fakez)*10000;
			}
			break;

		case ITEMGLOWRAD:
			if (s)
			{
				ret = s->glowRad * 10000;
			}
			break;
			
		case ITEMGLOWSHP:
			if (s)
			{
				ret = s->glowShape * 10000;
			}
			break;
			
		case ITEMDIR:
			if (s)
			{
				ret = s->dir * 10000;
			}
			break;
			
		case ITEMENGINEANIMATE:
			if (s)
			{
				ret = int32_t(s->do_animation) * 10000;
			}
			break;
			
		case ITEMSHADOWSPR:
			if (s)
			{
				ret = int32_t(s->spr_shadow) * 10000;
			}
			break;
		case ITEMDROPPEDBY:
			if (s)
			{
				ret = int32_t(s->from_dropset) * 10000;
			}
			break;
		case ITEMPICKUPEXSTATE:
			if (s)
			{
				ret = int32_t(s->pickupexstate) * 10000;
			}
			break;
		case ITMSWHOOKED:
			if (s)
			{
				ret = s->switch_hooked ? 10000 : 0;
			}
			break;
		case ITEMFORCEGRAB:
			if (s)
			{
				ret = s->get_forcegrab() ? 10000 : 0;
			}
			break;
			
		case ITEMNOSOUND:
			if (s)
			{
				ret=s->noSound ? 10000 : 0;
			}
			break;
			
		case ITEMNOHOLDSOUND:
			if (s)
			{
				ret=s->noHoldSound ? 10000 : 0;
			}
			break;

		default: NOTREACHED();
	}

	return ret;
}

void itemsprite_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case SPRITEMAXITEM:
			items.setMax(vbound((value/10000),1,MAX_ITEM_SPRITES));
			return;
	}

	item* s = checkItem(GET_REF(itemref));

	switch (reg)
	{
		case ITEMTYPE:
			if (s)
			{
				s->type=value/10000;
			}
			
			break;
		
		case ITEMLEVEL:
			if (s)
			{
				s->lvl=value/10000;
			}
			
			break;
		
		case ITEMX:
			if (s)
			{
				s->x = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				
				// Move the Fairy enemy as well.
				auto const& fairy_item = itemsbuf.get(s->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(s->fairyUID);
					if (fairy)
						fairy->x = s->x;
				}
			}
			break;

		case ITEMY:
			if (s)
			{
				s->y = get_qr(qr_SPRITEXY_IS_FLOAT) ? zslongToFix(value) : zfix(value/10000);
				
				// Move the Fairy enemy as well.
				auto const& fairy_item = itemsbuf.get(s->id);
				if (fairy_item.type == itype_fairy && fairy_item.misc3)
				{
					enemy* fairy = (enemy*) guys.getByUID(s->fairyUID);
					if (fairy)
						fairy->y = s->y;
				}
			}
			break;
		
		case ITEMSPRITESCRIPT:
			FFScript::deallocateAllScriptOwned(ScriptType::ItemSprite, GET_REF(itemref));
			if (s)
			{
				(s->scrconfig.script)=(value/10000);
				if (get_qr(qr_CLEARINITDONSCRIPTCHANGE))
					s->scrconfig.run_args.fill(0);
				s->scrconfig.inst_init.clear();
			}
			break;
		
		case ITEMSCALE:
			if ( get_qr(qr_OLDSPRITEDRAWS) ) 
			{
				scripting_log_error_with_context("To use this you must disable the quest rule 'Old (Faster) Sprite Drawing'.");
				break;
			}
			if (s)
			{
				(s->scale)=(zfix)(value/100.0);
			}
			
			break;
			
		case ITEMZ:
			if (s)
			{
				(s->z)=(zfix)(value/10000);
				
				if(s->z < 0)
					s->z = 0;
			}
			
			break;
			
		case ITEMJUMP:
			if (s)
			{
				(s->fall)=zslongToFix(value)*-100;
			}
			
			break;
		
		case ITEMFAKEJUMP:
			if (s)
			{
				(s->fakefall)=zslongToFix(value)*-100;
			}
			
			break;
			
		case ITEMDRAWTYPE:
			if (s)
			{
				(s->drawstyle)=value/10000;
			}
			
			break;
			
		case ITEMGRAVITY:
			if (s)
			{
				if(value)
					s->moveflags |= move_obeys_grav;
				else
					s->moveflags &= ~move_obeys_grav;
			}
			
			break;
			
		case ITEMID:
			if (s)
			{
				(s->id)=value/10000;
				flushItemCache();
			}
			
			break;
			
		case ITEMTILE:
			if (s)
			{
				(s->tile)=vbound(value/10000,0,NEWMAXTILES-1);
			}
			
			break;
			
		case ITEMSCRIPTTILE:
			if (s)
			{
				(s->scripttile)=vbound(value/10000,-1,NEWMAXTILES-1);
			}
			break;
			
		case ITEMSCRIPTFLIP:
			if (s)
			{
				(s->scriptflip)=vbound((value/10000),-1,127);
			}
			break;
		
		case ITEMPSTRING:
			if (s)
			{
				(s->pstring)=vbound(value/10000,0,(msg_count-1));
			}
			
			break;
		
		case ITEMPSTRINGFLAGS:
			if (s)
			{
				(s->pickup_string_flags)=vbound(value/10000, 0, 214748);
			}
			
			break;
			
		case ITEMOTILE:
			if (s)
			{
				(s->o_tile)=vbound(value/10000,0,NEWMAXTILES-1);
			}
			
			break;
			
		case ITEMCSET:
			if (s)
			{
				(s->o_cset) = (s->o_cset & ~15) | ((value/10000)&15);
				(s->cs) = (s->o_cset & 15);
			}
			
			break;
			
		case ITEMFLASHCSET:
			if (s)
			{
				(s->o_cset) = ((value/10000)<<4) | (s->o_cset & 15);
			}
			
			break;
			
		case ITEMFRAMES:
			if (s)
			{
				(s->frames)=value/10000;
			}
			
			break;
			
		case ITEMFRAME:
			if (s)
			{
				(s->aframe)=value/10000;
			}
			
			break;
			
		case ITEMASPEED:
			if (s)
			{
				(s->o_speed)=value/10000;
			}
			
			break;
		
		 case ITEMACLK:
			if (s)
			{
				(s->aclk)=value/10000;
			}
			
			break;
		
		case ITEMDELAY:
			if (s)
			{
				(s->o_delay)=value/10000;
			}
			
			break;
			
		case ITEMFLIP:
			if (s)
			{
				(s->flip)=value/10000;
			}
			
			break;
			
		case ITEMFLASH:
			if (s)
			{
				(s->flash)= (value/10000)?1:0;
			}
			
			break;
			
		case ITEMEXTEND:
			if (s)
			{
				(s->extend)=value/10000;
			}
			
			break;
			
		case ITEMHXOFS:
			if (s)
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
			if (s)
			{
				s->rotation=value/10000;
			}
			
			break;
			
		case ITEMHYOFS:
			if (s)
			{
				s->hyofs=value/10000;
			}
			
			break;
			
		case ITEMXOFS:
			if (s)
			{
				s->xofs=(zfix)(value/10000);
			}
			
			break;
			
		case ITEMYOFS:
			if (s)
			{
				s->yofs=(zfix)(value/10000)+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
			}
			
			break;
			
		case ITEMSHADOWXOFS:
			if (s)
			{
				s->shadowxofs=(zfix)(value/10000);
			}
			
			break;
		
		case ITEMSHADOWYOFS:
			if (s)
			{
				s->shadowyofs=(zfix)(value/10000);
			}
			
			break;
		
		case ITEMZOFS:
			if (s)
			{
				s->zofs=(zfix)(value/10000);
			}
			
			break;
			
		case ITEMHXSZ:
			if (s)
			{
				s->hit_width=value/10000;
			}
			
			break;
			
		case ITEMHYSZ:
			if (s)
			{
				s->hit_height=value/10000;
			}
			
			break;
			
		case ITEMHZSZ:
			if (s)
			{
				s->hzsz=value/10000;
			}
			
			break;
			
		case ITEMTXSZ:
			if (s)
			{
				s->txsz=vbound((value/10000),1,20);
			}
			
			break;
			
		case ITEMTYSZ:
			if (s)
			{
				s->tysz=vbound((value/10000),1,20);
			}
			
			break;
			
		case ITEMPICKUP:
			if (s)
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
				if(newpickup & ipFADE) // this isn't the right flag? bleh... Also doing the wrong thing?
				{
					(s->clk2) = game->get_item_timeout_dur();
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

		case ITEMFALLCLK:
			if (s)
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
			if (s)
			{
				s->fallCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case ITEMDROWNCLK:
			if (s)
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
			if (s)
			{
				s->drownCombo = vbound(value/10000,0,MAXCOMBOS-1);
			}
			break;
		case ITEMFAKEZ:
			if (s)
			{
				(s->fakez)=(zfix)(value/10000);
				
				if(s->fakez < 0)
					s->fakez = 0;
			}
			
			break;
		
		case ITEMGLOWRAD:
			if (s)
			{
				s->glowRad = vbound(value/10000,0,255);
			}
			break;
			
		case ITEMGLOWSHP:
			if (s)
			{
				s->glowShape = vbound(value/10000,0,255);
			}
			break;
			
		case ITEMDIR:
			if (s)
			{
				s->dir=(value/10000);
			}
			break;
			
		case ITEMENGINEANIMATE:
			if (s)
			{
				s->do_animation=value;
			}
			break;
			
		case ITEMSHADOWSPR:
			if (s)
			{
				s->spr_shadow=vbound(value/10000,0,MAXSPRITES-1);
			}
			break;
		case ITEMDROPPEDBY:
			if (s)
			{
				s->from_dropset=vbound(value/10000,-1,255);
			}
			break;
		case ITEMPICKUPEXSTATE:
			if (s)
			{
				s->pickupexstate = vbound(value / 10000, -1, 31);
			}
			break;
		case ITMSWHOOKED:
			break; //read-only
		case ITEMFORCEGRAB:
			if (s)
			{
				s->set_forcegrab(value!=0);
			}
			break;
		case ITEMNOSOUND:
			if (s)
			{
				s->noSound = (value!=0);
			}
			break;
		case ITEMNOHOLDSOUND:
			if (s)
			{
				s->noHoldSound = (value!=0);
			}
			break;

		default: NOTREACHED();
	}
}

std::optional<int32_t> itemsprite_run_command(word command)
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

static ArrayRegistrar ITEMMISCD_registrar(ITEMMISCD, []{
	static ScriptingArray_ObjectMemberCArray<item, &item::miscellaneous> impl;
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar ITEMMOVEFLAGS_registrar(ITEMMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<item, &item::moveflags, 11> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar ITEMSPRITEINITD_registrar(ITEMSPRITEINITD, []{
	static ScriptingArray_ObjectSubMemberContainer<item, &item::scrconfig, &script_config::run_args> impl;
	impl.setMul10000(false);
	return &impl;
}());
