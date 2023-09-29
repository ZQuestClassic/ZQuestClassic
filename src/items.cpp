#include "items.h"
#include "zc/guys.h"
#include "zc/maps.h"
#include "base/zdefs.h"
#include "base/qrs.h"
#include "base/combo.h"
#include "zc/ffscript.h"
#include <fmt/format.h>
#include "base/misctypes.h"

#include <queue>

char *item_string[MAXITEMS];

extern zinitdata zinit;
#ifndef IS_ZQUEST
	extern FFScript FFCore;
	extern ZModule zcm;
#endif

int32_t fairy_cnt=0;

item::~item()
{
	// TODO: we should have an item manager class in zc and manage lifetime explicitly, not via dtors.
#ifndef IS_ZQUEST
	if(itemsbuf[id].family==itype_fairy && itemsbuf[id].misc3>0 && misc>0 && !get_qr(qr_OLD_FAIRY_LIMIT))
		killfairynew(*this);
	FFCore.deallocateAllScriptOwned(ScriptType::ItemSprite, getUID());
#endif
}

bool item::animate(int32_t)
{
	if(switch_hooked)
	{
		solid_update(false);
		return false;
	}
	if(!screenIsScrolling()) // Because subscreen items are items, too. :p
	{
		if(fallclk > 0)
		{
			if(fallclk == PITFALL_FALL_FRAMES && fallCombo) sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
			if(!--fallclk) return true;
			
			wpndata& spr = wpnsbuf[QMisc.sprites[sprFALL]];
			cs = spr.csets & 0xF;
			int32_t fr = spr.frames ? spr.frames : 1;
			int32_t spd = spr.speed ? spr.speed : 1;
			int32_t animclk = (PITFALL_FALL_FRAMES-fallclk);
			tile = spr.tile + zc_min(animclk / spd, fr-1);
			
			solid_update(false);
			return false;
		}
		if(drownclk > 0)
		{
			//if(drownclk == WATER_DROWN_FRAMES && drownCombo) sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
			//!TODO: Drown SFX
			if(!--drownclk) return true;
			
			if (drownCombo && combobuf[drownCombo].usrflags&cflag1) 
			{
				wpndata &spr = wpnsbuf[QMisc.sprites[sprLAVADROWN]];
				cs = spr.csets & 0xF;
				int32_t fr = spr.frames ? spr.frames : 1;
				int32_t spd = spr.speed ? spr.speed : 1;
				int32_t animclk = (WATER_DROWN_FRAMES-drownclk);
				tile = spr.tile + zc_min(animclk / spd, fr-1);
			}
			else 
			{
				wpndata &spr = wpnsbuf[QMisc.sprites[sprDROWN]];
				cs = spr.csets & 0xF;
				int32_t fr = spr.frames ? spr.frames : 1;
				int32_t spd = spr.speed ? spr.speed : 1;
				int32_t animclk = (WATER_DROWN_FRAMES-drownclk);
				tile = spr.tile + zc_min(animclk / spd, fr-1);
			}
			
			solid_update(false);
			return false;
		}
#ifndef IS_ZQUEST
		if(isSideViewGravity())
		{
			if((
					(can_drop(x,y) && !(pickup & ipDUMMY) && !(pickup & ipCHECK))
					||
					(can_drop(x,y) && ipDUMMY && linked_parent == eeGANON ) //Ganon's dust pile
				) 
				&& 
				( moveflags & FLAG_OBEYS_GRAV ) //if the user set item->Gravity = false, let it float. -Z
			)
			{
				item_fall(x, y, fall);
			}
			else if(!can_drop(x,y) && !(pickup & ipDUMMY) && !(pickup & ipCHECK))
			{
				if(fall!=0)
				{
					y.doFloor();
					y-=int32_t(y)%8; //Fix coords
					fall = 0;
				}
				//fall = -fall/2; // LA key bounce. //Is this even working? Doesn't appear to be. -V
			}
		}
		else
		{
			if ( moveflags & FLAG_OBEYS_GRAV ) //if the user set item->Gravity = false, let it float. -Z
			{
				if (!(moveflags & FLAG_NO_FAKE_Z))
				{
					fakez-=fakefall/100;
				
					if(fakez<0)
					{
						fakez = 0;
						fakefall = -fakefall/2;
					}
					else if(fakez <= 1 && abs(fakefall) < (int32_t)(zinit.gravity2 / 100))
					{
						fakez=0;
						fakefall=0;
					}
					else if(fakefall <= (int32_t)zinit.terminalv)
					{
						fakefall += (zinit.gravity2 / 100);
					}
				}
				if (!(moveflags & FLAG_NO_REAL_Z))
				{
					z-=fall/100;
					
					if(z<0)
					{
						z = 0;
						fall = -fall/2;
					}
					else if(z <= 1 && abs(fall) < (int32_t)(zinit.gravity2 / 100))
					{
						z=0;
						fall=0;
					}
					else if(fall <= (int32_t)zinit.terminalv)
					{
						fall += (zinit.gravity2 / 100);
					}
				}
			}
			if ( moveflags & FLAG_CAN_PITFALL )
			{
				if(!subscreenItem && !force_grab && !is_dragged && z <= 0 && fakez <= 0 && !(pickup & ipDUMMY) && !(pickup & ipCHECK) && itemsbuf[id].family!=itype_fairy)
				{
					fallCombo = check_pits();
				}
			}
			if ( moveflags & FLAG_CAN_WATERDROWN )
			{
				if(!subscreenItem && !force_grab && !is_dragged && z <= 0 && fakez <= 0 && !(pickup & ipDUMMY) && !(pickup & ipCHECK) && itemsbuf[id].family!=itype_fairy)
				{
					drownCombo = check_water();
				}
			}
		}
#endif
	}
	
	// Maybe it fell off the bottom in sideview, or was moved by a script.
	if(y>352 || y<-176 || x<-256 || x > 512)
	{
		return true;
	}
	
	if((++clk)>=0x8000)
	{
		clk=0x7000;
	}
	
	itemdata const* itm = &itemsbuf[id];
	if(itm->family == itype_progressive_itm)
	{
		int32_t id2 = get_progressive_item(*itm);
		if(unsigned(id2) >= MAXITEMS)
			id2 = -1;
		if(id2 != linked_parent) //Update item
		{
			linked_parent = id2;
			if(id2 < 0)
				load_gfx(*itm);
			else load_gfx(itemsbuf[id2]);
		}
		if(id2 > -1)
			itm = &itemsbuf[id2];
	}
	
	if(flash)
	{
		cs = o_cset;
		
		if(frame&8)
		{
			cs >>= 4;
		}
		else
		{
			cs &= 15;
		}
	}
	
	if(do_animation && ((get_qr(qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES) ? (anim) : (frames>0)) || itm->family==itype_bottle))
	{
		int32_t spd = o_speed;
		
		if(aframe==0)
		{
			spd *= o_delay+1;
		}
		
		if(++aclk >= spd)
		{
			aclk=0;
			
			if(++aframe >= frames)
			{
				aframe=0;
			}
		}
		
		//tile = o_tile + aframe;
		if(extend > 2)
		{
			if(o_tile/TILES_PER_ROW==(o_tile+txsz*aframe)/TILES_PER_ROW)
				tile=o_tile+txsz*aframe;
			else
				tile=o_tile+(txsz*aframe)+((tysz-1)*TILES_PER_ROW)*(((o_tile+txsz*aframe)/TILES_PER_ROW)-(o_tile/TILES_PER_ROW));
		}
		else
			tile = o_tile + aframe;
#ifndef IS_ZQUEST
		//Bottles offset based on their slot's fill
		if(itm->family == itype_bottle)
		{
			int32_t slot = itm->misc1;
			size_t btype = game->get_bottle_slot(slot);
			int32_t offset = (frames ? frames : 1) * btype;
			if(extend > 2)
			{
				if(tile/TILES_PER_ROW==(tile+offset*txsz)/TILES_PER_ROW)
					offset *= txsz;
				else
					offset = (offset*txsz) + ((tysz-1)*TILES_PER_ROW)*(((tile+offset*txsz)/TILES_PER_ROW)-(tile/TILES_PER_ROW));
			}
			tile += offset;
		}
#endif
	}
	
#ifndef IS_ZQUEST
	if(itemsbuf[id].family == itype_fairy && itemsbuf[id].misc3)
	{
		movefairynew(x,y,*this);
	}
#endif
	
	if(fadeclk==0 && !subscreenItem)
	{
		return true;
	}
	
	if(pickup&ipTIMER)
	{
		if(++clk2 == 512)
		{
			return true;
		}
	}
	
	return false;
}

void item::draw(BITMAP *dest)
{
	if(pickup&ipNODRAW || tile==0 || force_grab)
		return;
		
	if ( (z > 0 || fakez > 0) && get_qr(qr_ITEMSHADOWS) )
	{
		shadowtile = wpnsbuf[spr_shadow].tile+aframe;
		sprite::drawshadow(dest,get_qr(qr_TRANSSHADOWS) != 0);
	}
	if(!(pickup&ipFADE) || fadeclk<0 || fadeclk&1 || fallclk || drownclk)
	{
		if(clk2>32 || (clk2&2)==0 || itemsbuf[id].family == itype_fairy || fallclk || drownclk)
		{
			sprite::draw(dest);
		}
	}
}

void item::draw_hitbox()
{
	if(subscreenItem) return;
	sprite::draw_hitbox();
}

item::item(zfix X,zfix Y,zfix Z,int32_t i,int32_t p,int32_t c, bool isDummy) : sprite()
{
	x=X;
	y=Y;
	z=Z;
	id=i;
	pickup=p;
	clk=c;
	misc=clk2=0;
	aframe=aclk=0;
	anim=flash=twohand=subscreenItem=false;
	dummy_int[0]=PriceIndex=-1;
	is_dragged=false;
	force_grab=false;
	itemdata const& itm = itemsbuf[id];
	from_dropset = -1;
	pickupexstate = -1;

	#ifndef IS_ZQUEST
	script_UID = FFCore.GetScriptObjectUID(UID_TYPE_ITEM); //This is used by child npcs. 
	//Sadly, this also stores UIDs for all dummy objects, including subscreen and other stuff. 
	//if ( !isDummy && ( pickup == 0x100 || pickup <= 0 || pickup == 0x002 || pickup == 0x004 && pickup == 0x800 ) ) script_UID = FFCore.GetScriptObjectUID(UID_TYPE_ITEM); //This is used by child npcs. 
	//if it is on the screen
	//if ( x > 0 && x < 256 && y > 56 && y < 256 && !isDummy && ( pickup == 0x100 || pickup == 0 || pickup == 0x002 || pickup == 0x004 && pickup == 0x800 ) ) script_UID = FFCore.GetScriptObjectUID(UID_TYPE_ITEM); //This is used by child npcs. 
	#endif
	
	if(id<0 || id>MAXITEMS) //>, not >= for dummy items such as the HC Piece display in the subscreen
		return;
		 
	o_tile = itm.tile;
	tile = itm.tile;
	cs = itm.csets&15;
	o_cset = itm.csets;
	o_speed = itm.speed;
	o_delay = itm.delay;
	frames = itm.frames;
	flip = itm.misc_flags>>2;
	family = itm.family;
	lvl = itm.fam_type;
	pstring = itm.pstring;
	pickup_string_flags = itm.pickup_string_flags;
	linked_parent = family == itype_progressive_itm ? -1 : 0;
	moveflags = FLAG_OBEYS_GRAV | FLAG_CAN_PITFALL;
	for ( int32_t q = 0; q < 8; q++ ) initD[q] = itm.initiald[q];
	
	//if ( itm.overrideFLAGS&itemdataOVERRIDE_PICKUP ) pickup = itm.pickup;
	switch (itm.pickupflag) 
	{
		case 1:
			pickup = itm.pickup;
			break;
		case 2:
			pickup |= itm.pickup;
			break;
		case 3:
			pickup &= ~itm.pickup;
			break;
		case 4:
			pickup &= itm.pickup;
			break;
		default:
			break;
	}
	
	if(itm.misc_flags&1)
		flash=true;
		
	if(itm.misc_flags&2)
		twohand=true;
		
	anim = itm.frames>0;
	
	if(pickup&ipBIGRANGE)
	{
		hxofs=-8;
		hit_width=17;
		hyofs=-4;
		hit_height=20;
	}
	else if(pickup&ipBIGTRI)
	{
		hxofs=-8;
		hit_width=28;
		hyofs=-4;
		hit_height=20;
	}
	else
	{
		hit_width=1;
		hyofs=4;
		hit_height=12;
	}
	
	if(!isDummy && itm.family == itype_fairy && itm.misc3)
	{
		misc = ++fairy_cnt;
#ifndef IS_ZQUEST
		if(addfairynew(x, y, itm.misc3, *this))
			sfx(itm.usesound);
#endif
	}
	
	/*for(int32_t j=0;j<8;j++)
	{
	  if(j<2) a[j]=itm.initiala[j]*10000;
	  d[j]=itm.initiald[j];
	}*/
	if ( itm.overrideFLAGS > 0 ) {
		extend = 3; 
		if ( itm.overrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { txsz = itm.tilew;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){  tysz = itm.tileh;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ){  hit_width = itm.hxsz;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {  hit_height = itm.hysz;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {  hzsz = itm.hzsz;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {  hxofs = itm.hxofs;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) { hyofs = itm.hyofs;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) { xofs = itm.xofs;}
		if ( itm.overrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {  yofs = itm.yofs+(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);} 
		/* yofs+playing_field_offset == yofs+56.
		It is needed for the passive subscreen offset.
		*/
	}
	//if ( itm.flags&itemdataOVERRIDE_DRAW_Z_OFFSET ) zofs = itm.zofs;
	script = itm.sprite_script;
}

// easy way to draw an item

void putitem(BITMAP *dest,int32_t x,int32_t y,int32_t item_id)
{
	item temp((zfix)x,(zfix)y,(zfix)0,item_id,0,0);
	temp.yofs=0;
	temp.hide_hitbox = true;
	
	if ( itemsbuf[item_id].overrideFLAGS > 0 ) {
		temp.extend = 3; 
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { temp.txsz = itemsbuf[item_id].tilew;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){temp.tysz = itemsbuf[item_id].tileh;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ) { temp.hit_width = itemsbuf[item_id].hxsz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {temp.hit_height = itemsbuf[item_id].hysz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) { temp.hzsz = itemsbuf[item_id].hzsz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) { temp.hxofs = itemsbuf[item_id].hxofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) {temp.hyofs = itemsbuf[item_id].hyofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) {temp.xofs = itemsbuf[item_id].xofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) { temp.yofs = itemsbuf[item_id].yofs; }
	}

	temp.animate(0);
	temp.draw(dest);
}

void item::load_gfx(itemdata const& itm)
{
	o_tile = itm.tile;
	cs = itm.csets&15;
	o_cset = itm.csets;
	o_speed = itm.speed;
	o_delay = itm.delay;
	frames = itm.frames;
	flip = itm.misc_flags>>2;
	anim = itm.frames>0;
	aframe = aclk = 0;
	if(do_animation && ((get_qr(qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES) ? (anim) : (frames>0))||itm.family==itype_bottle))
	{
		int32_t spd = o_speed;
		
		if(aframe==0)
		{
			spd *= o_delay+1;
		}
		
		if(aclk >= spd)
		{
			aclk=0;
			
			if(++aframe >= frames)
			{
				aframe=0;
			}
		}
		else if(aframe >= frames)
		{
			aframe=0;
		}
		
		//tile = o_tile + aframe;
		if(extend > 2)
		{
			if(o_tile/TILES_PER_ROW==(o_tile+txsz*aframe)/TILES_PER_ROW)
				tile=o_tile+txsz*aframe;
			else
				tile=o_tile+(txsz*aframe)+((tysz-1)*TILES_PER_ROW)*((o_tile+txsz*aframe)/TILES_PER_ROW)-(o_tile/TILES_PER_ROW);
		}
		else
			tile = o_tile + aframe;
#ifndef IS_ZQUEST
		//Bottles offset based on their slot's fill
		if(itm.family == itype_bottle)
		{
			int32_t slot = itm.misc1;
			size_t btype = game->get_bottle_slot(slot);
			int32_t offset = (frames ? frames : 1) * btype;
			tile += offset;
		}
#endif
	}
	else tile = o_tile;
}

int32_t get_progressive_item(itemdata const& itm, bool lastOwned)
{
#ifdef IS_ZQUEST
	return -1;
#else
	int32_t arr[] = {itm.misc1, itm.misc2, itm.misc3, itm.misc4, itm.misc5,
		itm.misc6, itm.misc7, itm.misc8, itm.misc9, itm.misc10};
	int32_t lastid = -1;
	for(auto id : arr)
	{
		if(unsigned(id) >= MAXITEMS)
			continue;
		lastid = id;
		
		//Skip items that are owned as 'Equipment Item's
		if(game->get_item(id))
			continue;
		itemdata const& targItem = itemsbuf[id];
		//Skip items that would increase a counter max by 0, due to 'Not Above...'
		if(targItem.setmax > 0) //Increases a counter
			if(game->get_maxcounter(targItem.count) >= targItem.max) //...but can't
				continue;
		if(targItem.family == itype_heartpiece)
		{
			int32_t hcid = heart_container_id();
			if(hcid < 0) continue;
			itemdata const& hcitem = itemsbuf[hcid];
			if(hcitem.setmax > 0)
				if(game->get_maxcounter(hcitem.count) >= hcitem.max)
					continue;
		}
		
		if(lastOwned) return lastid;
		return id;
	}
	return lastid;
#endif
}
// Linker issues because this is shared with ZQu4est. :( -Z
#ifndef IS_ZQUEST
int32_t item::getScriptUID() { return script_UID; }
void item::setScriptUID(int32_t new_id) { script_UID = new_id; }
#endif

void putitem2(BITMAP *dest,int32_t x,int32_t y,int32_t item_id, int32_t &aclk, int32_t &aframe, int32_t flash)
{
	item temp((zfix)x,(zfix)y,(zfix)0,item_id,0,0,true);
	temp.yofs=0;
	temp.aclk=aclk;
	temp.aframe=aframe;
	temp.hide_hitbox = true;
	
	if(flash)
	{
		temp.flash=(flash != 0);
	}
	if ( itemsbuf[item_id].overrideFLAGS > 0 ) {
		temp.extend = 3;
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) { temp.txsz = itemsbuf[item_id].tilew;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ){temp.tysz = itemsbuf[item_id].tileh;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ) { temp.hit_width = itemsbuf[item_id].hxsz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {temp.hit_height = itemsbuf[item_id].hysz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) { temp.hzsz = itemsbuf[item_id].hzsz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) { temp.hxofs = itemsbuf[item_id].hxofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) {temp.hyofs = itemsbuf[item_id].hyofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) {temp.xofs = itemsbuf[item_id].xofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) { temp.yofs = itemsbuf[item_id].yofs; }
	}	    
	
	temp.animate(0);
	temp.draw(dest);
	aclk=temp.aclk;
	aframe=temp.aframe;
	temp.unget_UID();
}

void dummyitem_animate(item* dummy, int32_t clk)
{
	int fr = dummy->frames, spd = dummy->o_speed, del = dummy->o_delay;
	if(fr < 1) fr = 1;
	if(spd < 1) spd = 1;
	int clkwid = spd*(fr+del);
	clk %= clkwid;
	
	//To counter needing to call animate, calculate one clk back
	clk = clk ? clk-1 : clkwid-1;
	
	dummy->aframe = clk < (del*spd) ? 0 : (clk-del*spd)/spd;
	dummy->aclk=dummy->aframe ? (clk-del*spd)%spd : clk;
	
	auto item_id = dummy->id;
	if ( itemsbuf[item_id].overrideFLAGS > 0 )
	{
		dummy->extend = 3;
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_TILEWIDTH ) {dummy->txsz = itemsbuf[item_id].tilew;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_TILEHEIGHT ) {dummy->tysz = itemsbuf[item_id].tileh;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_WIDTH ) {dummy->hit_width = itemsbuf[item_id].hxsz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_HEIGHT ) {dummy->hit_height = itemsbuf[item_id].hysz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_Z_HEIGHT ) {dummy->hzsz = itemsbuf[item_id].hzsz;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_X_OFFSET ) {dummy->hxofs = itemsbuf[item_id].hxofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_HIT_Y_OFFSET ) {dummy->hyofs = itemsbuf[item_id].hyofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_DRAW_X_OFFSET ) {dummy->xofs = itemsbuf[item_id].xofs;}
		if ( itemsbuf[item_id].overrideFLAGS&itemdataOVERRIDE_DRAW_Y_OFFSET ) {dummy->yofs = itemsbuf[item_id].yofs;}
	}	    
	
	dummy->animate(0);
}
void putitem3(BITMAP *dest,int32_t x,int32_t y,int32_t item_id, int32_t clk)
{
	item temp((zfix)x,(zfix)y,(zfix)0,item_id,0,0,true);
	temp.xofs=temp.yofs=0;
	temp.hide_hitbox = true;
	temp.subscreenItem = true;
	
	dummyitem_animate(&temp, clk);
	temp.draw(dest);
	temp.unget_UID();
}

//some methods for dealing with items
int32_t getItemFamily(itemdata* items, int32_t item)
{
	if(item < 0) return -1;
	return items[item&0xFF].family;
}

void removeItemsOfFamily(gamedata *g, itemdata *items, int32_t family)
{
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family)
		{
			g->set_item_no_flush(i,false);
			if ( game->forced_bwpn == i ) 
			{
				game->forced_bwpn = -1;
			} //not else if! -Z
			if ( game->forced_awpn == i ) 
			{
				game->forced_awpn = -1;
			}
			if ( game->forced_xwpn == i ) 
			{
				game->forced_xwpn = -1;
			}
			if ( game->forced_ywpn == i ) 
			{
				game->forced_ywpn = -1;
			}
		}
	}
	flushItemCache();
}

void removeLowerLevelItemsOfFamily(gamedata *g, itemdata *items, int32_t family, int32_t level)
{
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && items[i].fam_type < level)
		{
			g->set_item_no_flush(i, false);
			if ( game->forced_bwpn == i ) 
			{
				game->forced_bwpn = -1;
			} //not else if! -Z
			if ( game->forced_awpn == i ) 
			{
				game->forced_awpn = -1;
			}
			if ( game->forced_xwpn == i ) 
			{
				game->forced_xwpn = -1;
			}
			if ( game->forced_ywpn == i ) 
			{
				game->forced_ywpn = -1;
			}
		}
	}
	flushItemCache();
}

void removeItemsOfFamily(zinitdata *z, itemdata *items, int32_t family)
{
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family)
		{
			z->items[i]=false;
			if ( game->forced_bwpn == i ) 
			{
				game->forced_bwpn = -1;
			} //not else if! -Z
			if ( game->forced_awpn == i ) 
			{
				game->forced_awpn = -1;
			}
			if ( game->forced_xwpn == i ) 
			{
				game->forced_xwpn = -1;
			}
			if ( game->forced_ywpn == i ) 
			{
				game->forced_ywpn = -1;
			}
		}
	}
}

int32_t getHighestLevelOfFamily(zinitdata *source, itemdata *items, int32_t family)
{
	int32_t result = -1;
	int32_t highestlevel = -1;
	
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && source->items[i])
		{
			if(items[i].fam_type >= highestlevel)
			{
				highestlevel = items[i].fam_type;
				result=i;
			}
		}
	}
	
	return result;
}

int32_t getHighestLevelOfFamily(gamedata *source, itemdata *items, int32_t family, bool checkenabled)
{
	int32_t result = -1;
	int32_t highestlevel = -1;
	
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && source->get_item(i) && (checkenabled?(!(source->items_off[i])):1))
		{
			if(items[i].fam_type >= highestlevel)
			{
				highestlevel = items[i].fam_type;
				result=i;
			}
		}
	}
	
	return result;
}

int32_t getHighestLevelEvenUnowned(itemdata *items, int32_t family)
{
	int32_t result = -1;
	int32_t highestlevel = -1;
	
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family)
		{
			if(items[i].fam_type >= highestlevel)
			{
				highestlevel = items[i].fam_type;
				result=i;
			}
		}
	}
	
	return result;
}

int32_t getItemID(itemdata *items, int32_t family, int32_t level)
{
	if(level<0) return getCanonicalItemID(items, family);
	
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && items[i].fam_type == level)
			return i;
	}
	
	return -1;
}

int32_t getItemIDPower(itemdata *items, int32_t family, int32_t power)
{
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && items[i].power == power)
			return i;
	}
	
	return -1;
}

/* Retrieves the canonical item of a given item family, the item with least non-0 level */
int32_t getCanonicalItemID(itemdata *items, int32_t family)
{
	int32_t lowestid = -1;
	int32_t lowestlevel = -1;
	
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		if(items[i].family == family && (items[i].fam_type < lowestlevel || lowestlevel == -1))
		{
			lowestlevel = items[i].fam_type;
			lowestid = i;
		}
	}
	
	return lowestid;
}

void addOldStyleFamily(zinitdata *dest, itemdata *items, int32_t family, char levels)
{
	for(int32_t i=0; i<8; i++)
	{
		if(levels & (1<<i))
		{
			int32_t id = getItemID(items, family, i+1);
			
			if(id != -1)
				dest->items[id]=true;
		}
	}
}

const char *old_item_string[iLast] =
{
	"Rupee (1)", "Rupee (5)", "Heart", "Bomb (Normal)", "Clock",
	"Sword 1 (Wooden)", "Sword 2 (White)", "Sword 3 (Magic)", "Shield 2 (Magic)", "Key (Normal)",
	"Candle 1 (Blue)", "Candle 2 (Red)", "Letter", "Arrow 1 (Wooden)", "Arrow 2 (Silver)",
	"Bow 1 (Short)", "Bait", "Ring 1 (Blue)", "Ring 2 (Red)", "Bracelet 2",
	"Triforce (Fragment)", "Map", "Compass", "Boomerang 1 (Wooden)", "Boomerang 2 (Magic)",
	"Wand", "Raft", "Ladder 1", "Heart Container", "Potion 1 (Blue)",
	"Potion 2 (Red)", "Whistle", "Magic Book", "Key (Magic)", "Fairy (Moving)",
	"Boomerang 3 (Fire)", "Sword 4 (Master)", "Shield 3 (Mirror)", "Rupee (20)", "Rupee (50)",
	"Rupee (200)", "Wallet 1 (500)", "Wallet 2 (999)", "Dust Pile",
	"Triforce (Whole)", "Selection (1)", "Misc 1", "Misc 2", "Bomb (Super)","Heart Container Piece",
	"Amulet 1", "Flippers", "Hookshot 1 (Short)", "Lens of Truth", "Hammer", "Boots",
	"Bracelet 3", "Arrow 3 (Golden)", "Magic Container", "Magic Jar 1 (Small)",
	"Magic Jar 2 (Large)", "Ring 3 (Golden)", "Kill All Enemies", "Amulet 2",
	"Divine Fire", "Divine Escape", "Divine Protection", "Key (Boss)", "Bow 2 (Long)", "Fairy (Stationary)",
	"Arrow Ammunition (1)", "Arrow Ammunition (5)", "Arrow Ammunition (10)", "Arrow Ammunition (30)",
	"Quiver 1 (Small)", "Quiver 2 (Medium)", "Quiver 3 (Large)", "Bomb Ammunition (1)", "Bomb Ammunition (4)", "Bomb Ammunition (8)", "Bomb Ammunition (30)",
	"Bomb Bag 1 (Small)", "Bomb Bag 2 (Medium)", "Bomb Bag 3 (Large)", "Key (Level Specific)", "Selection (2)", "Rupee (10)", "Rupee (100)", "Cane of Byrna",
	"Hookshot 2 (Long)", "Letter (used)", "Roc's Feather", "Hover Boots","Shield 1 (Small)","Scroll: Spin Attack", "Scroll: Cross Beams", "Scroll: Quake Hammer",
	"Scroll: Super Quake", "Scroll: Hurricane Spin", "Whisp Ring 1", "Whisp Ring 2", "Charge Ring 1", "Charge Ring 2", "Scroll: Peril Beam", "Wallet 3 (Magic)",
	"Quiver 4 (Magic)", "Bomb Bag 4 (Magic)", "Bracelet 1 (Worn-out Glove)", "Ladder 2 (Four-Way)", "Wealth Medal 1 (75%)", "Wealth Medal 2 (50%)",
	"Wealth Medal 3 (25%)", "Heart Ring 1 (Slow)", "Heart Ring 2", "Heart Ring 3 (Fast)", "Magic Ring 1 (Slow)", "Magic Ring 2", "Magic Ring 3 (Fast)",
	"Magic Ring 4 (Light Force)", "Stone of Agony", "Stomp Boots", "Peril Ring", "Whimsical Ring",
	"Custom Item 01", "Custom Item 02", "Custom Item 03", "Custom Item 04", "Custom Item 05",
	"Custom Item 06", "Custom Item 07", "Custom Item 08", "Custom Item 09", "Custom Item 10",
	"Custom Item 11", "Custom Item 12", "Custom Item 13", "Custom Item 14", "Custom Item 15",
	"Custom Item 16", "Custom Item 17", "Custom Item 18", "Custom Item 19", "Custom Item 20"
};

const char *old_weapon_string[wLast] =
{
	"Sword 1 (Wooden)", "Sword 2 (White)", "Sword 3 (Magic)", "Sword 4 (Master)",
	"Boomerang 1 (Wooden)", "Boomerang 2 (Magic)", "Boomerang 3 (Fire)", "Bomb (Normal)",
	"Bomb (Super)", "Explosion (Normal)", "Arrow 1 (Wooden)", "Arrow 2 (Silver)", "Flame", "Whirlwind",
	"Bait", "Wand", "Magic (Normal)", "Fireball", "Rock", "Arrow (Enemy)", "Sword (Enemy)",
	"Magic (Enemy)", "MISC: Spawn", "MISC: Death", "MISC: <unused>", "Hammer",
	"Hookshot 1 (Short) Head", "Hookshot 1 (Short) Chain (Horizontal)", "Hookshot 1 (Short) Handle", "MISC: Arrow 2 (Silver) Sparkle",
	"MISC: Arrow 3 (Golden) Sparkle", "MISC: Boomerang 2 (Magic) Sparkle", "MISC: Boomerang 3 (Fire) Sparkle",
	"MISC: Hammer Impact", "Arrow 3 (Golden)", "Fire (Enemy)", "Whirlwind (Enemy)", "MISC: Magic Meter",
	"Divine Fire (Rising)", "Divine Fire (Falling)","Divine Fire Trail (Rising)", "Divine Fire Trail (Falling)", "Hookshot 1 (Short) Chain (Vertical)", "MISC: More...",
	"MISC: <unused>", "MISC: <unused>", "Sword 1 (Wooden) Slash",
	"Sword 2 (White) Slash", "Sword 3 (Magic) Slash", "Sword 4 (Master) Slash", "MISC: Shadow (Small)",
	"MISC: Shadow (Large)", "MISC: Bush Leaves", "MISC: Flower Clippings",
	"MISC: Grass Clippings", "MISC: Tall Grass", "MISC: Ripples", "MISC: <unused>",
	"Divine Protection (Left)", "Divine Protection (Left, Returning)","Divine Protection Trail (Left)", "Divine Protection Trail (Left, Returning)",
	"Divine Protection (Right)", "Divine Protection (Right, Returning)","Divine Protection Trail (Right)", "Divine Protection Trail (Right, Returning)",
	"Divine Protection Shield (Front)", "Divine Protection Shield (Back)", "MISC: Subscreen Vine", "Cane of Byrna", "Cane of Byrna (Slash)",
	"Hookshot 2 (Long) Head", "Hookshot 2 (Long) Chain (Horizontal)", "Hookshot 2 (Long) Handle", "Hookshot 2 (Long) Chain (Vertical)",
	"Explosion (Super)", "Bomb (Enemy, Normal)", "Bomb (Enemy, Super)", "Explosion (Enemy, Normal)",  "Explosion (Enemy, Super)",
	"Fire Trail (Enemy)", "Fire 2 (Enemy)", "Fire 2 Trail (Enemy) <Unused>", "Ice Magic (Enemy) <Unused>", "MISC: Hover Boots Glow", "Magic (Fire)", "MISC: Quarter Hearts", "Cane of Byrna (Beam)" /*, "MISC: Sideview Ladder", "MISC: Sideview Raft"*/
};

char *weapon_string[MAXWPNS];

ALLEGRO_COLOR item::hitboxColor(byte opacity) const
{
	return al_map_rgba(0,255,255,opacity);
}

std::string itemdata::get_name(bool init, bool plain) const
{
	std::string name;
	if(display_name[0])
	{
		name = display_name;
		size_t repl_pos = name.find("%s");
		if(repl_pos != std::string::npos)
		{
			std::string arg;
			switch(family)
			{
				case itype_bottle:
					if(init)
					{
						if(unsigned(misc1) < 256)
							arg = fmt::format("Slot {}",misc1);
						else arg = "Invalid Slot";
					}
					else
					{
#ifndef IS_ZQUEST
						arg = bottle_slot_name(misc1,"Empty");
#else
						arg = "Empty";
#endif
					}
					break;
			}
			name.replace(repl_pos,2,arg);
			//Anything with 2 args?
			//repl_pos = name.find("%s");
			replstr(name,"%s",""); //Clear any spare '%s'
		}
	}
	else
	{
		int id = -1;
		for(int q = 0; q < MAXITEMS; ++q)
		{
			if((itemsbuf+q) == this)
			{
				id = q;
				break;
			}
		}
		name = id > -1 ? item_string[id] : "";
		if(!plain)
		{
			std::string overname;
			switch(family)
			{
				case itype_arrow:
				{
					auto bowid = current_item_id(itype_bow,false);
					if(bowid>-1 && checkmagiccost(id))
						overname = itemsbuf[bowid].get_name() + " & " + name;
					break;
				}
				case itype_bottle:
#ifndef IS_ZQUEST
					overname = bottle_slot_name(misc1,"");
#endif
					break;
			}
			if(!overname.empty())
				return overname;
		}
	}
	return name;
}
