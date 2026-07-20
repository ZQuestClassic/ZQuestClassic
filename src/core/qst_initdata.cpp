#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern dword loading_tileset_flags;
extern byte subscr_mode;
extern byte deprecated_rules[QUESTRULES_NEW_SIZE];
extern bool fixpolsvoice;

namespace {

static int32_t readinitdata_old(PACKFILE *f, zquestheader *Header, word s_version, zinitdata& temp_zinit, message_icon& msg_more)
{
	byte padding, tempbyte;
	
	// Legacy item properties (now integrated into itemdata)
	byte sword_hearts[4];
	byte beam_hearts[4];
	byte beam_percent=0;
	word beam_power[4];
	byte hookshot_length=99;
	byte hookshot_links=100;
	byte longshot_length=99;
	byte longshot_links=100;
	byte moving_fairy_hearts=3;
	byte moving_fairy_heart_percent=0;
	byte stationary_fairy_hearts=3;
	byte stationary_fairy_heart_percent=0;
	byte moving_fairy_magic=0;
	byte moving_fairy_magic_percent=0;
	byte stationary_fairy_magic=0;
	byte stationary_fairy_magic_percent=0;
	byte blue_potion_hearts=100;
	byte blue_potion_heart_percent=1;
	byte red_potion_hearts=100;
	byte red_potion_heart_percent=1;
	byte blue_potion_magic=100;
	byte blue_potion_magic_percent=1;
	byte red_potion_magic=100;
	byte red_potion_magic_percent=1;
	
	byte bomb_ratio = 4;
	
	subscr_mode = 0;
	
	/* HIGHLY UNORTHODOX UPDATING THING, by L
	 * This fixes quests made before revision 277 (such as the 'Lost Isle Build'),
	 * where the speed of Pols Voice changed. It also coincided with V_INITDATA
	 * changing from 13 to 14.
	 */
	if(s_version < 14)
		fixpolsvoice=true;
		
	/* End highly unorthodox updating thing */
	
	if(s_version >= 15 && get_bit(deprecated_rules, 27)) // The int16_t-lived rule, qr_JUMPHEROLAYER3
		temp_zinit.jump_hero_layer_threshold=0;
		
	if(s_version >= 10)
	{
		char temp;
		
		//new-style items
		for(int32_t j=0; j<256; j++)
		{
			if(!p_getc(&temp,f))
				return qe_invalid;
				
			temp_zinit.set_item(j, temp != 0);
		}
	}
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>26)))
	{
		char temp;
		
		//finally...  section data
		if((Header->zelda_version > 0x192)||
				//new only
				((Header->zelda_version == 0x192)&&(Header->build>173)))
		{
			//OLD-style items... sigh
			if(s_version < 10)
			{
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				temp_zinit.set_item(iRaft, temp != 0);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				temp_zinit.set_item(iLadder, temp != 0);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				temp_zinit.set_item(iBook, temp != 0);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				temp_zinit.set_item(iMKey, temp != 0);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				temp_zinit.set_item(iFlippers, temp != 0);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				temp_zinit.set_item(iBoots, temp != 0);
			}
		}
		
		if(s_version < 10)
		{
			char tempring, tempsword, tempshield, tempwallet, tempbracelet, tempamulet, tempbow;
			
			if(!p_getc(&tempring,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempsword,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempshield,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempwallet,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempbracelet,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempamulet,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempbow,f))
			{
				return qe_invalid;
			}
			
			//old only
			if((Header->zelda_version == 0x192)&&(Header->build<174))
			{
				tempring=(tempring)?(1<<(tempring-1)):0;
				tempsword=(tempsword)?(1<<(tempsword-1)):0;
				tempshield=(tempshield)?(1<<(tempshield-1)):0;
				tempwallet=(tempwallet)?(1<<(tempwallet-1)):0;
				tempbracelet=(tempbracelet)?(1<<(tempbracelet-1)):0;
				tempamulet=(tempamulet)?(1<<(tempamulet-1)):0;
				tempbow=(tempbow)?(1<<(tempbow-1)):0;
			}
			
			//rings start at level 2... wtf
			//account for this -DD
			tempring <<= 1;
			addOldStyleFamily(&temp_zinit, itype_ring, tempring);
			addOldStyleFamily(&temp_zinit, itype_sword, tempsword);
			addOldStyleFamily(&temp_zinit, itype_shield, tempshield);
			addOldStyleFamily(&temp_zinit, itype_wallet, tempwallet);
			//bracelet ALSO starts at level 2 :-( -DD
			tempbracelet<<=1;
			addOldStyleFamily(&temp_zinit, itype_bracelet, tempbracelet);
			addOldStyleFamily(&temp_zinit, itype_amulet, tempamulet);
			addOldStyleFamily(&temp_zinit, itype_bow, tempbow);
			
			//new only
			if((Header->zelda_version == 0x192)&&(Header->build>173))
			{
				for(int32_t q=0; q<32; q++)
				{
					if(!p_getc(&padding,f))
					{
						return qe_invalid;
					}
				}
			}
			
			char tempcandle, tempboomerang, temparrow, tempwhistle;
			
			if(!p_getc(&tempcandle,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempboomerang,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&temparrow,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&temp,f))
			{
				return qe_invalid;
			}
			
			addOldStyleFamily(&temp_zinit, itype_potion, temp);
			
			if(!p_getc(&tempwhistle,f))
			{
				return qe_invalid;
			}
			
			//old only
			if((Header->zelda_version == 0x192)&&(Header->build<174))
			{
				tempcandle=(tempcandle)?(1<<(tempcandle-1)):0;
				tempboomerang=(tempboomerang)?(1<<(tempboomerang-1)):0;
				temparrow=(temparrow)?(1<<(temparrow-1)):0;
				tempwhistle=(tempwhistle)?(1<<(tempwhistle-1)):0;
			}
			
			addOldStyleFamily(&temp_zinit, itype_candle, tempcandle);
			addOldStyleFamily(&temp_zinit, itype_brang, tempboomerang);
			addOldStyleFamily(&temp_zinit, itype_arrow, temparrow);
			addOldStyleFamily(&temp_zinit, itype_whistle, tempwhistle);
			//What about the potion...?
			
		}
		
		if(s_version < 29)
		{
			//Oh sure, stick these IN THE MIDDLE OF THE ITEMS, just to make me want
			//to jab out my eye...
			if(!p_getc(&padding,f))
				return qe_invalid;
			temp_zinit.counter[crBOMBS] = padding;
			
			if(!p_getc(&padding,f))
				return qe_invalid;
			temp_zinit.counter[crSBOMBS] = padding;
		}
		
		//Back to more OLD item code
		if(s_version < 10)
		{
			if((Header->zelda_version > 0x192)||
					//new only
					((Header->zelda_version == 0x192)&&(Header->build>173)))
			{
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_wand, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_letter, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_lens, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_hookshot, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_bait, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_hammer, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_divinefire, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_divineescape, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				addOldStyleFamily(&temp_zinit, itype_divineprotection, temp);
				
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
				
				if(Header->zelda_version == 0x192)
				{
					for(int32_t q=0; q<32; q++)
					{
						if(!p_getc(&padding,f))
						{
							return qe_invalid;
						}
					}
				}
			}
		}
		
		//old only
		if((Header->zelda_version == 0x192)&&(Header->build<174))
		{
			byte equipment, tmpitm;                                //bit flags
			
			if(!p_getc(&equipment,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.set_item(iRaft, get_bit(&equipment, idE_RAFT)!=0);
			temp_zinit.set_item(iLadder, get_bit(&equipment, idE_LADDER)!=0);
			temp_zinit.set_item(iBook, get_bit(&equipment, idE_BOOK)!=0);
			temp_zinit.set_item(iMKey, get_bit(&equipment, idE_KEY)!=0);
			temp_zinit.set_item(iFlippers, get_bit(&equipment, idE_FLIPPERS)!=0);
			temp_zinit.set_item(iBoots, get_bit(&equipment, idE_BOOTS)!=0);
			
			
			if(!p_getc(&tmpitm,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.set_item(iWand, get_bit(&tmpitm, idI_WAND)!=0);
			temp_zinit.set_item(iLetter, get_bit(&tmpitm, idI_LETTER)!=0);
			temp_zinit.set_item(iLens, get_bit(&tmpitm, idI_LENS)!=0);
			temp_zinit.set_item(iHookshot, get_bit(&tmpitm, idI_HOOKSHOT)!=0);
			temp_zinit.set_item(iBait, get_bit(&tmpitm, idI_BAIT)!=0);
			temp_zinit.set_item(iHammer, get_bit(&tmpitm, idI_HAMMER)!=0);
		}
		
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_zinit.mcounter[crLIFE] = tempbyte;
		
		
		if(s_version < 14)
		{
			byte temphp;
			
			if(!p_getc(&temphp,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.counter[crLIFE]=temphp;
			
			if(!p_getc(&temphp,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.cont_heart=temphp;
		}
		else
		{
			if(!p_igetw(&temp_zinit.counter[crLIFE],f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.cont_heart,f))
			{
				return qe_invalid;
			}
		}
		
		if(!p_getc(&temp_zinit.hcp,f))
		{
			return qe_invalid;
		}
		
		if(s_version >= 14)
		{
			if(!p_getc(&temp_zinit.hcp_per_hc,f))
			{
				return qe_invalid;
			}
			
			if(s_version<16)  // July 2007
			{
				if(get_qr(qr_BRANGPICKUP+1))
					temp_zinit.hcp_per_hc = 0xFF;
					
				//Dispose of legacy rule
				set_qr(qr_BRANGPICKUP+1, 0);
			}
		}
		
		if(s_version < 29)
		{
			if(!p_getc(&padding,f))
				return qe_invalid;
			temp_zinit.mcounter[crBOMBS] = padding;
		}
		
		if(!p_getc(&temp_zinit.counter[crKEYS],f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_zinit.counter[crMONEY],f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		for(int q = 0; q < 8; ++q)
			SETFLAG(temp_zinit.litems[q+1], (1 << li_mcguffin), get_bitl(tempbyte, q));
		
		int level_count = 32;
		if(s_version>12 || (Header->zelda_version == 0x211 && Header->build == 18))
			level_count = 64;
		byte tmp_map[64];
		byte tmp_compass[64];
		for(int32_t i=0; i<level_count; i++)
			if(!p_getc(&tmp_map[i],f))
				return qe_invalid;
		for(int32_t i=0; i<level_count; i++)
			if(!p_getc(&tmp_compass[i],f))
				return qe_invalid;
		for(int q = 0; q < level_count*8; ++q)
		{
			SETFLAG(temp_zinit.litems[q], (1 << li_map), get_bit(tmp_map, q));
			SETFLAG(temp_zinit.litems[q], (1 << li_compass), get_bit(tmp_compass, q));
		}
		
		if((Header->zelda_version > 0x192)||
				//new only
				((Header->zelda_version == 0x192)&&(Header->build>173)))
		{
			byte tmp_boss_key[64];
			for(int32_t i=0; i<level_count; i++)
			{
				if(!p_getc(&tmp_boss_key[i],f))
				{
					return qe_invalid;
				}
			}
			for(int q = 0; q < level_count*8; ++q)
			{
				SETFLAG(temp_zinit.litems[q], (1 << li_boss_key), get_bit(tmp_boss_key, q));
			}
		}
		
		byte tmpmisc[16];
		for(int32_t i=0; i<16; i++)
			if(!p_getc(&tmpmisc[i],f))
				return qe_invalid;
		temp_zinit.flags.set(INIT_FL_CONTPERCENT,get_bit(tmpmisc,0));
		temp_zinit.magicdrainrate = get_bit(tmpmisc,1) ? 1 : 2; //Double Magic flag
		temp_zinit.flags.set(INIT_FL_CANSLASH,get_bit(tmpmisc,2));
		
		if(s_version < 15) for(int32_t i=0; i<4; i++)
			if(!p_getc(&sword_hearts[i],f))
				return qe_invalid;
			
		if(!p_getc(&temp_zinit.last_map,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_zinit.last_screen,f))
		{
			return qe_invalid;
		}
		
		if(s_version < 14)
		{
			byte tempmp;
			
			if(!p_getc(&tempmp,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.mcounter[crMAGIC]=tempmp;
			
			if(!p_getc(&tempmp,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.counter[crMAGIC]=tempmp;
		}
		else
		{
			if(!p_igetw(&temp_zinit.mcounter[crMAGIC],f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.counter[crMAGIC],f))
			{
				return qe_invalid;
			}
		}
		
		
		if(s_version < 15)
		{
			if(s_version < 12)
			{
				temp_zinit.mcounter[crMAGIC]*=32;
				temp_zinit.counter[crMAGIC]*=32;
			}
			
			for(int32_t i=0; i<4; i++)
			{
				if(!p_getc(&beam_hearts[i],f))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getc(&beam_percent,f))
			{
				return qe_invalid;
			}
		}
		else
		{
			if(!p_getc(&temp_zinit.bomb_ratio,f))
				return qe_invalid;
			if(temp_zinit.bomb_ratio < 1)
				temp_zinit.bomb_ratio = 1;
			else bomb_ratio = temp_zinit.bomb_ratio; //jank
		}
		
		if(s_version < 15)
		{
			byte tempbp;
			
			for(int32_t i=0; i<4; i++)
			{
				if(!(s_version < 14 ? p_getc(&tempbp,f) : p_igetw(&tempbp,f)))
				{
					return qe_invalid;
				}
				
				beam_power[i]=tempbp;
			}
			
			if(!p_getc(&hookshot_links,f))
			{
				return qe_invalid;
			}
			
			if(s_version>6)
			{
				if(!p_getc(&hookshot_length,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&longshot_links,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&longshot_length,f))
				{
					return qe_invalid;
				}
			}
		}
		
		if(!p_getc(&tempbyte, f))
			return qe_invalid;
		msg_more.x = tempbyte;
		
		if(!p_getc(&tempbyte, f))
			return qe_invalid;
		msg_more.y = zc_min(160, tempbyte);
		
		if(!p_getc(&subscr_mode,f))
			return qe_invalid;
		
		//old only
		if((Header->zelda_version == 0x192)&&(Header->build<174))
		{
			byte tmp_boss_key[32];
			for(int32_t i=0; i<32; i++)
			{
				if(!p_getc(&tmp_boss_key[i],f))
				{
					return qe_invalid;
				}
			}
			for(int q = 0; q < 32*8; ++q)
			{
				SETFLAG(temp_zinit.litems[q], (1 << li_boss_key), get_bit(tmp_boss_key, q));
			}
		}
		
		if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>173)))  //new only
		{
			if(s_version <= 10)
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
				
				temp_zinit.start_dmap = (word)tempbyte;
			}
			else
			{
				if(!p_igetw(&temp_zinit.start_dmap,f))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getc(&temp_zinit.heroAnimationStyle,f))
			{
				return qe_invalid;
			}
		}
		
		if(s_version>1 && s_version < 29)
		{
			if(!p_getc(&padding,f))
				return qe_invalid;
			temp_zinit.counter[crARROWS] = padding;
			
			if(!p_getc(&padding,f))
				return qe_invalid;
			temp_zinit.mcounter[crARROWS] = padding;
		}
		
		if(s_version>2)
		{
			if(s_version <= 10)
			{
				for(int32_t i=0; i<OLDMAXLEVELS; i++)
				{
					if(!p_getc(&(temp_zinit.level_keys[i]),f))
					{
						return qe_invalid;
					}
				}
			}
			else
			{
				for(int32_t i=0; i<MAXLEVELS; i++)
				{
					if(!p_getc(&(temp_zinit.level_keys[i]),f))
					{
						return qe_invalid;
					}
				}
			}
		}
		
		if(s_version>3)
		{
			if(!p_igetw(&temp_zinit.ss_grid_x,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.ss_grid_y,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.ss_grid_xofs,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.ss_grid_yofs,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.ss_grid_color,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.ss_bbox_1_color,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.ss_bbox_2_color,f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.ss_flags,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.ss_grid_x=zc_max(temp_zinit.ss_grid_x,1);
			temp_zinit.ss_grid_y=zc_max(temp_zinit.ss_grid_y,1);
		}
		
		if(s_version>4 && s_version<15)
		{
			if(!p_getc(&moving_fairy_hearts,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&moving_fairy_heart_percent,f))
			{
				return qe_invalid;
			}
		}
		
		if(s_version>5 && s_version < 10)
		{
			if(!p_getc(&temp,f))
			{
				return qe_invalid;
			}
			
			addOldStyleFamily(&temp_zinit, itype_quiver, temp);
		}
		
		if(s_version>6 && s_version<15)
		{
			if(!p_getc(&stationary_fairy_hearts,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&stationary_fairy_heart_percent,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&moving_fairy_magic,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&moving_fairy_magic_percent,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&stationary_fairy_magic,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&stationary_fairy_magic_percent,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&blue_potion_hearts,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&blue_potion_heart_percent,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&red_potion_hearts,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&red_potion_heart_percent,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&blue_potion_magic,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&blue_potion_magic_percent,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&red_potion_magic,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&red_potion_magic_percent,f))
			{
				return qe_invalid;
			}
		}
		
		if(s_version>6)
			if(!p_getc(&padding,f))
				return qe_invalid;
		
		if(s_version>7)
		{
			if(!p_getc(&padding,f))
			{
				return qe_invalid;
			}
		}
		
		if(s_version>8)
		{
			if(!p_igetw(&temp_zinit.mcounter[crMONEY],f))
			{
				return qe_invalid;
			}
			
			if(!p_igetw(&temp_zinit.mcounter[crKEYS],f))
			{
				return qe_invalid;
			}
		}
		
		if(s_version>16)
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
			temp_zinit.gravity = tempbyte*100;
			if(!p_igetw(&temp_zinit.terminalv,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&temp_zinit.msg_speed,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&padding,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&temp_zinit.jump_hero_layer_threshold,f))
			{
				return qe_invalid;
			}
		}
		else if (replay_version_check(0, 13))
			temp_zinit.msg_speed = 0;
		
		if(s_version>17)
		{
			if(!p_getc(&tempbyte,f))
				return qe_invalid;
			if (tempbyte)
				msg_more.anchor = message_anchor::screen_y_offset;
		}
		
		//expaned init data for larger values in 2.55
		if ( s_version >= 19 ) //expand init data bombs, sbombs, and arrows to 0xFFFF
		{
			if(!p_igetw(&temp_zinit.counter[crBOMBS],f))
			{
				return qe_invalid;
			}
			if(!p_igetw(&temp_zinit.counter[crSBOMBS],f))
			{
				return qe_invalid;
			}
			if(!p_igetw(&temp_zinit.mcounter[crBOMBS],f))
			{
				return qe_invalid;
			}
			if(!p_igetw(&temp_zinit.mcounter[crSBOMBS],f))
			{
				return qe_invalid;
			}
			if(!p_igetw(&temp_zinit.counter[crARROWS],f))
			{
				return qe_invalid;
			}
			if(!p_igetw(&temp_zinit.mcounter[crARROWS],f))
			{
				return qe_invalid;
			}
			
		}
		if ( s_version >= 20 )
		{
			if(!p_igetw(&temp_zinit.heroStep,f))
			{
				return qe_invalid;
			}
		}
		else
		{	
			temp_zinit.heroStep = 150; //1.5 pixels per frame
		}
		if ( s_version >= 21 )
		{
			if(!p_igetw(&temp_zinit.subscrSpeed,f))
			{
				return qe_invalid;
			}
		}
		else
		{	
			temp_zinit.subscrSpeed = 1; //3 pixels per frame
		}
		//old only
		if((Header->zelda_version == 0x192)&&(Header->build<174))
		{
			byte items2;
			
			if(!p_getc(&items2,f))
			{
				return qe_invalid;
			}
			
			temp_zinit.set_item(iDivineFire, get_bit(&items2, idI_DFIRE)!=0);
			temp_zinit.set_item(iDivineEscape, get_bit(&items2, idI_FWIND)!=0);
			temp_zinit.set_item(iDivineProtection, get_bit(&items2, idI_NLOVE)!=0);
		}
		
		if(Header->zelda_version < 0x193)
		{
			for(int32_t q=0; q<96; q++)
			{
				if(!p_getc(&padding,f))
				{
					return qe_invalid;
				}
			}
			
			//new only
			if((Header->zelda_version == 0x192)&&(Header->build>173))
			{
				if(!p_getc(&padding,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&padding,f))
				{
					return qe_invalid;
				}
			}
		}
	}
	
	if((Header->zelda_version < 0x211)||((Header->zelda_version == 0x211)&&(Header->build<15)))
	{
		//temp_zinit.shield=i_smallshield;
		int32_t sshieldid = getItemID(itype_shield, i_smallshield);
		
		if(sshieldid != -1)
			temp_zinit.set_item(sshieldid, true);
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<27)))
	{
		temp_zinit.mcounter[crLIFE]=3;
		temp_zinit.counter[crLIFE]=3;
		temp_zinit.cont_heart=3;
		temp_zinit.mcounter[crBOMBS]=8;
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<50)))
	{
		sword_hearts[0]=0;
		sword_hearts[1]=5;
		sword_hearts[2]=12;
		sword_hearts[3]=21;
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<51)))
	{
		temp_zinit.last_map=0;
		temp_zinit.last_screen=0;
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<68)))
	{
		temp_zinit.mcounter[crMAGIC]=0;
		temp_zinit.counter[crMAGIC]=0;
		temp_zinit.magicdrainrate = 2;
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<129)))
	{
	
		for(int32_t x=0; x<4; x++)
		{
			beam_hearts[x]=100;
		}
		
		for(int32_t i=0; i<idBP_MAX; i++)
		{
			set_bit(&beam_percent,i,!get_qr(qr_LENSHINTS+i));
			set_qr(qr_LENSHINTS+i,0);
		}
		
		for(int32_t x=0; x<4; x++)
		{
			beam_power[x]=get_qr(qr_HIDECARRIEDITEMS)?50:100;
		}
		
		set_qr(qr_HIDECARRIEDITEMS,0);
		hookshot_links=100;
		msg_more.x = 224;
		msg_more.y = 64;
	}
	
	// Okay,  let's put these legacy values into itemsbuf.
	if(s_version < 15)
	{
		itemsbuf[iFairyStill].misc1 = stationary_fairy_hearts;
		itemsbuf[iFairyStill].misc2 = stationary_fairy_magic;
		itemsbuf[iFairyStill].misc3 = 0;
		if (stationary_fairy_heart_percent) itemsbuf[iFairyStill].flags |= item_flag1;
		if (stationary_fairy_magic_percent) itemsbuf[iFairyStill].flags |= item_flag2;
		
		itemsbuf[iFairyMoving].misc1 = moving_fairy_hearts;
		itemsbuf[iFairyMoving].misc2 = moving_fairy_magic;
		itemsbuf[iFairyMoving].misc3 = 50;
		if (moving_fairy_heart_percent) itemsbuf[iFairyMoving].flags |= item_flag1;
		if (moving_fairy_magic_percent) itemsbuf[iFairyMoving].flags |= item_flag2;
		
		itemsbuf[iRPotion].misc1 = red_potion_hearts;
		itemsbuf[iRPotion].misc2 = red_potion_magic;
		if (red_potion_heart_percent) itemsbuf[iRPotion].flags |= item_flag1;
		if (red_potion_magic_percent) itemsbuf[iRPotion].flags |= item_flag2;
		
		itemsbuf[iBPotion].misc1 = blue_potion_hearts;
		itemsbuf[iBPotion].misc2 = blue_potion_magic;
		if (blue_potion_heart_percent) itemsbuf[iBPotion].flags |= item_flag1;
		if (blue_potion_magic_percent) itemsbuf[iBPotion].flags |= item_flag2;
		
		itemsbuf[iSword].pickup_hearts = sword_hearts[0];
		itemsbuf[iSword].misc1 = beam_hearts[0];
		itemsbuf[iSword].misc2 = beam_power[0];
		// It seems that item_flag1 was already added by reset_itembuf()...
		itemsbuf[iSword].flags &= (!get_bit(&beam_percent,0)) ? ~item_flag1 : ~item_none;
		
		itemsbuf[iWSword].pickup_hearts = sword_hearts[1];
		itemsbuf[iWSword].misc1 = beam_hearts[1];
		itemsbuf[iWSword].misc2 = beam_power[1];
		itemsbuf[iWSword].flags &= (!get_bit(&beam_percent,1)) ? ~item_flag1 : ~item_none;
		
		itemsbuf[iMSword].pickup_hearts = sword_hearts[2];
		itemsbuf[iMSword].misc1 = beam_hearts[2];
		itemsbuf[iMSword].misc2 = beam_power[2];
		itemsbuf[iMSword].flags &= (!get_bit(&beam_percent,2)) ? ~item_flag1 : ~item_none;
		
		itemsbuf[iXSword].pickup_hearts = sword_hearts[3];
		itemsbuf[iXSword].misc1 = beam_hearts[3];
		itemsbuf[iXSword].misc2 = beam_power[3];
		itemsbuf[iXSword].flags &= (!get_bit(&beam_percent,3)) ? ~item_flag1 : ~item_none;
		
		itemsbuf[iHookshot].misc1 = hookshot_length;
		itemsbuf[iHookshot].misc2 = hookshot_links;
		
		itemsbuf[iLongshot].misc1 = longshot_length;
		itemsbuf[iLongshot].misc2 = longshot_links;
	}
		
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<168)))
	{
		//was new subscreen rule
		subscr_mode=get_qr(qr_FREEFORM)?1:0;
		set_qr(qr_FREEFORM,0);
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<185)))
	{
		temp_zinit.start_dmap=0;
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<186)))
	{
		temp_zinit.heroAnimationStyle=get_qr(qr_BSZELDA)?1:0;
	}
	
	if(s_version < 16 && get_bit(deprecated_rules, qr_COOLSCROLL+1))
	{
		//addOldStyleFamily(&temp_zinit, itype_wallet, 4);   //is this needed?
		temp_zinit.mcounter[crMONEY]=999;
		//temp_zinit.counter[crMONEY]=999; //This rule only gave you an invisible max wallet; it did not give you max rupies.
	}
	if(Header->zelda_version < 0x190) //1.84 bugfix. -Z
	{
		//temp_zinit.items[iBombBag] = true; //No, this is 30 max bombs!
		temp_zinit.mcounter[crBOMBS] = 8;
	}
	// al_trace("About to copy over new init data values for quest made in: %x\n", Header->zelda_version);
	//time to ensure that we port all new values properly:
	if(Header->zelda_version < 0x250)
	{
		temp_zinit.mcounter[crSBOMBS] = bomb_ratio > 0 ? ( temp_zinit.mcounter[crBOMBS]/temp_zinit.bomb_ratio ) : (temp_zinit.mcounter[crBOMBS]/4);
	}
	
	if(s_version > 21)
	{
		if(!p_getc(&temp_zinit.hp_per_heart,f))
		{
			return qe_invalid;
		}
		if(!p_getc(&temp_zinit.magic_per_block,f))
		{
			return qe_invalid;
		}
		if(!p_getc(&temp_zinit.hero_damage_multiplier,f))
		{
			return qe_invalid;
		}
		if(!p_getc(&temp_zinit.ene_damage_multiplier,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_zinit.hp_per_heart = 16; //HP_PER_HEART, previously hardcoded
		temp_zinit.magic_per_block = 32; //MAGICPERBLOCK, previously hardcoded
		temp_zinit.hero_damage_multiplier = 2; //DAMAGE_MULTIPLIER, previously hardcoded
		temp_zinit.ene_damage_multiplier = 4; //(HP_PER_HEART/4), previously hardcoded
	}
	
	if(s_version > 22)
	{
		for(int32_t q = crCUSTOM1; q <= crCUSTOM25; ++q)
			if(!p_igetw(&temp_zinit.counter[q],f))
				return qe_invalid;
		for(int32_t q = crCUSTOM1; q <= crCUSTOM25; ++q)
			if(!p_igetw(&temp_zinit.mcounter[q],f))
				return qe_invalid;
	}
	
	
	if(s_version > 23)
	{
		if(!p_getc(&temp_zinit.dither_type,f))
		{
			return qe_invalid;
		}
		if(!p_getc(&temp_zinit.dither_arg,f))
		{
			return qe_invalid;
		}
		if(!p_getc(&temp_zinit.dither_percent,f))
		{
			return qe_invalid;
		}
		if(!p_getc(&temp_zinit.def_lightrad,f))
		{
			return qe_invalid;
		}
		if(!p_getc(&temp_zinit.transdark_percent,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_zinit.dither_type = 0;
		temp_zinit.dither_arg = 0;
		temp_zinit.dither_percent = 20;
		temp_zinit.def_lightrad = 24;
		temp_zinit.transdark_percent = 0;
	}
	
	if(s_version > 24)
	{
		if(!p_getc(&temp_zinit.darkcol,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_zinit.darkcol = BLACK;
	}
	
	if(s_version > 25)
	{
		if(!p_igetl(&temp_zinit.gravity,f))
		{
			return qe_invalid;
		}
		if(!p_igetl(&temp_zinit.swimgravity,f))
		{
			return qe_invalid;
		}
	}
	
	
	if(s_version > 26)
	{
		if(!p_igetw(&temp_zinit.heroSideswimUpStep,f))
		{
			return qe_invalid;
		}
		if(!p_igetw(&temp_zinit.heroSideswimSideStep,f))
		{
			return qe_invalid;
		}
		if(!p_igetw(&temp_zinit.heroSideswimDownStep,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_zinit.heroSideswimUpStep = 150;
		temp_zinit.heroSideswimSideStep = 100;
		temp_zinit.heroSideswimDownStep = 75;
	}
	
	if(s_version > 27)
	{
		if(!p_igetl(&temp_zinit.exitWaterJump,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_zinit.exitWaterJump = 0;
	}
	
	if(s_version > 29)
	{
		if(!p_igetl(&temp_zinit.bunny_ltm,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_zinit.bunny_ltm = 0;
	}
	
	if(s_version > 30)
	{
		if(!p_getc(&temp_zinit.switchhookstyle,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_zinit.switchhookstyle = 1;
	}
	
	if(s_version > 31)
	{
		if(!p_getc(&temp_zinit.magicdrainrate,f))
		{
			return qe_invalid;
		}
	}
	
	temp_zinit.clear_genscript();
	if(s_version > 32)
	{
		word numgenscript = 0;
		if(!p_igetw(&numgenscript,f))
			return qe_invalid;
		if (numgenscript > NUMSCRIPTSGENERIC)
			return qe_invalid;
		for(auto q = 1; q < numgenscript; ++q)
		{
			if(!p_getc(&tempbyte,f))
				return qe_invalid;
			if(!(tempbyte&2))
				continue;
			temp_zinit.gen_doscript.set(q, tempbyte&1);
			if(!p_igetw(&temp_zinit.gen_exitState[q],f))
				return qe_invalid;
			if(!p_igetw(&temp_zinit.gen_reloadState[q],f))
				return qe_invalid;
			for(auto p = 0; p < 8; ++p)
				if(!p_igetl(&temp_zinit.gen_initd[q][p],f))
					return qe_invalid;
			dword sz;
			if(!p_igetl(&sz,f))
				return qe_invalid;
			temp_zinit.gen_data[q].resize(sz);
			std::vector<int32_t> dummy;
			if(!p_getlvec(&dummy,f))
				return qe_invalid;
			temp_zinit.gen_data[q] = dummy;
			if(!p_igetl(&temp_zinit.gen_eventstate[q],f))
				return qe_invalid;
		}
	}
	if(s_version > 33)
	{
		if(!p_getc(&temp_zinit.hero_swim_mult,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hero_swim_div,f))
			return qe_invalid;
	}
	if(s_version > 34)
	{
		uint32_t num_used_mapscr_data;
		if(!p_igetl(&num_used_mapscr_data,f))
			return qe_invalid;
		for(uint32_t q = 0; q < num_used_mapscr_data; ++q)
		{
			uint32_t sz;
			if(!p_igetl(&sz,f))
				return qe_invalid;
			temp_zinit.screen_data[q].resize(sz);
			if(sz)
			{
				std::vector<int32_t> dummy;
				if(!p_getlvec(&dummy,f))
					return qe_invalid;
				temp_zinit.screen_data[q] = dummy;
			}
		}
	}
	if (s_version > 35)
		if(!p_igetzf(&temp_zinit.shove_offset,f))
			return qe_invalid;
	
	temp_zinit.counter[crLIFE] *= temp_zinit.hp_per_heart;
	temp_zinit.mcounter[crLIFE] *= temp_zinit.hp_per_heart;
	if(!temp_zinit.flags.get(INIT_FL_CONTPERCENT))
		temp_zinit.cont_heart *= temp_zinit.hp_per_heart;
	
	return 0;
}

} // end namespace

int32_t readinitdata(PACKFILE *f, zquestheader *Header)
{
	zinitdata temp_zinit = {};
	
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_initdata);
	
	int32_t dummy;
	word s_version=0;
	byte padding;
	
	if(Header->zelda_version > 0x192)
	{
		if(!p_igetw(&s_version,f))
			return qe_invalid;
		if (s_version > V_INITDATA)
			return qe_version;
		FFCore.quest_format[vInitData] = s_version;
		
		if(!read_deprecated_section_cversion(f))
			return qe_invalid;
		
		//section size
		if(!p_igetl(&dummy,f))
			return qe_invalid;
	}
	
	message_icon msg_more{};
	msg_more.sprite = iwMore;
	msg_more.anchor = message_anchor::screen;
	if(s_version < 37)
	{
		if(auto ret = readinitdata_old(f,Header,s_version,temp_zinit,msg_more))
			return ret;
	}
	else
	{
		subscr_mode = ssdtMAX;
		if (s_version >= 47)
		{
			if (!p_getbitstr(&temp_zinit.items, f))
				return qe_invalid;
		}
		else
		{
			for(int q = 0; q < 256/8; ++q)
			{
				if(!p_getc(&padding, f))
					return qe_invalid;
				temp_zinit.items.inner()[q] = padding;
			}
		}
		if(s_version >= 42)
		{
			for(int q = 0; q < MAXLEVELS; ++q)
			{
				if (s_version >= 44)
				{
					if(!p_igetw(&temp_zinit.litems[q], f))
						return qe_invalid;
				}
				else
				{
					if(!p_getc(&padding, f))
						return qe_invalid;
					temp_zinit.litems[q] = word(padding);
				}
			}
		}
		else
		{
			byte tmp_map[MAXLEVELS/8];
			byte tmp_compass[MAXLEVELS/8];
			byte tmp_boss_key[MAXLEVELS/8];
			byte tmp_mcguffin[MAXLEVELS/8];
			for(int q = 0; q < MAXLEVELS/8; ++q)
			{
				if(!p_getc(&tmp_map[q], f))
					return qe_invalid;
				if(!p_getc(&tmp_compass[q], f))
					return qe_invalid;
				if(!p_getc(&tmp_boss_key[q], f))
					return qe_invalid;
				if(!p_getc(&tmp_mcguffin[q], f))
					return qe_invalid;
			}
			for(int q = 0; q < MAXLEVELS; ++q)
			{
				SETFLAG(temp_zinit.litems[q], (1 << li_map), get_bit(tmp_map, q));
				SETFLAG(temp_zinit.litems[q], (1 << li_compass), get_bit(tmp_compass, q));
				SETFLAG(temp_zinit.litems[q], (1 << li_boss_key), get_bit(tmp_boss_key, q));
				SETFLAG(temp_zinit.litems[q], (1 << li_mcguffin), get_bit(tmp_mcguffin, q));
			}
		}
		if(!p_getbvec(&temp_zinit.level_keys, f))
			return qe_invalid;
		byte num_counters;
		if(!p_getc(&num_counters,f))
			return qe_invalid;
		if(num_counters > MAX_COUNTERS) // file-controlled; guard against OOB write into counter/mcounter
			return qe_invalid;
		for(int q = 0; q < num_counters; ++q)
			if(!p_igetw(&temp_zinit.counter[q],f))
				return qe_invalid;
		for(int q = 0; q < num_counters; ++q)
			if(!p_igetw(&temp_zinit.mcounter[q],f))
				return qe_invalid;
		if(!p_getc(&temp_zinit.bomb_ratio,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hcp,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hcp_per_hc,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.cont_heart,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hp_per_heart,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.magic_per_block,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hero_damage_multiplier,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.ene_damage_multiplier,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.dither_type,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.dither_arg,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.dither_percent,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.def_lightrad,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.transdark_percent,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.darkcol,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_grid_x,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_grid_y,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_grid_xofs,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_grid_yofs,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_grid_color,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_bbox_1_color,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_bbox_2_color,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.ss_flags,f))
			return qe_invalid;
		if(!p_getbitstr(&temp_zinit.flags,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.last_map,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.last_screen,f))
			return qe_invalid;
		if (s_version < 49)
		{
			if(!p_getc(&padding,f))
				return qe_invalid;
			msg_more.x = padding;
			if(!p_getc(&padding,f))
				return qe_invalid;
			msg_more.y = zc_min(160, padding);
			if(!p_getc(&padding,f))
				return qe_invalid;
			if (padding)
				msg_more.anchor = message_anchor::screen_y_offset;
		}
		if(!p_getc(&temp_zinit.msg_speed,f))
			return qe_invalid;
		if (s_version >= 49)
		{
			if(!p_getc(&temp_zinit.msg_advance_delay,f))
				return qe_invalid;
		}
		if(!p_igetl(&temp_zinit.gravity,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.swimgravity,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.terminalv,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hero_swim_speed,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hero_swim_mult,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.hero_swim_div,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.heroSideswimUpStep,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.heroSideswimSideStep,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.heroSideswimDownStep,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.exitWaterJump,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.heroStep,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.heroAnimationStyle,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.jump_hero_layer_threshold,f))
			return qe_invalid;
		if(!p_igetl(&temp_zinit.bunny_ltm,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.start_dmap,f))
			return qe_invalid;
		if(!p_igetw(&temp_zinit.subscrSpeed,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.switchhookstyle,f))
			return qe_invalid;
		if(!p_getc(&temp_zinit.magicdrainrate,f))
			return qe_invalid;
		if(!p_igetzf(&temp_zinit.shove_offset,f))
			return qe_invalid;
		if(!p_getbitstr(&temp_zinit.gen_doscript, f))
			return qe_invalid;
		if(!p_getbmap(&temp_zinit.gen_exitState, f))
			return qe_invalid;
		if(!p_getbmap(&temp_zinit.gen_reloadState, f))
			return qe_invalid;
		if(!p_getbmap(&temp_zinit.gen_initd, f))
			return qe_invalid;
		if(!p_getbmap(&temp_zinit.gen_eventstate, f))
			return qe_invalid;
		if(!p_getbmap(&temp_zinit.gen_data, f))
			return qe_invalid;
		if(!p_getbmap(&temp_zinit.screen_data, f))
			return qe_invalid;
		if (s_version >= 38)
		{
			if (!p_getc(&temp_zinit.spriteflickerspeed, f))
				return qe_invalid;
			if (!p_getc(&temp_zinit.spriteflickercolor, f))
				return qe_invalid;
			if (!p_getc(&temp_zinit.spriteflickertransp, f))
				return qe_invalid;
		}
		if(s_version >= 39)
			if(!p_igetzf(&temp_zinit.air_drag, f))
				return qe_invalid;
		
		// TODO: this first branch can likely be removed, as it only fixes an issues
		// that existed for a handful of temporary z3 builds (and active users of that
		// fork would have been updating often, beyond s_version 40).
		if (Header->is_z3 && s_version == 40)
		{
			if(!p_getc(&temp_zinit.region_mapping, f))
				return qe_invalid;
		}
		else
		{
			if(s_version >= 40)
			{
				if(!p_igetw(&temp_zinit.light_wave_rate, f))
					return qe_invalid;
				if(!p_igetw(&temp_zinit.light_wave_size, f))
					return qe_invalid;
			}

			if(s_version >= 41)
			{
				if(!p_getc(&temp_zinit.region_mapping, f))
					return qe_invalid;
			}
		}
		
		if(s_version >= 43)
			for(uint q = 0; q < NUM_BOTTLE_SLOTS; ++q)
				if (!p_getc(&temp_zinit.bottle_slot[q], f))
					return qe_invalid;
		
		if(s_version >= 44)
			if (!p_getbvec(&temp_zinit.lvlswitches, f))
				return qe_invalid;
		
		if (s_version >= 45)
		{
			if (!p_igetw(&temp_zinit.item_spawn_flicker, f))
				return qe_invalid;
			if (!p_igetw(&temp_zinit.item_timeout_dur, f))
				return qe_invalid;
			if (!p_igetw(&temp_zinit.item_timeout_flicker, f))
				return qe_invalid;
			if (!p_getc(&temp_zinit.item_flicker_speed, f))
				return qe_invalid;
		}
		if(s_version >= 46)
		{
			for(int q = 0; q < SPRITE_THRESHOLD_MAX; ++q)
				if(!p_igetw(&temp_zinit.sprite_z_thresholds[q], f))
					return qe_invalid;
		}
		if (s_version >= 48)
		{
			if (!p_getc(&temp_zinit.hero_itembox_xofs, f))
				return qe_invalid;
			if (!p_getc(&temp_zinit.hero_itembox_yofs, f))
				return qe_invalid;
			if (!p_getc(&temp_zinit.hero_itembox_width, f))
				return qe_invalid;
			if (!p_getc(&temp_zinit.hero_itembox_height, f))
				return qe_invalid;
		}
		else if (get_qr(qr_LTTPCOLLISION))
		{
			// the default values match qr_LTTPCOLLISION being off
			// just modify the y/height to match
			// Not actually needed for compat in any way, but seems like a convenient default to match
			temp_zinit.hero_itembox_yofs = 0;
			temp_zinit.hero_itembox_height = 16;
		}
		if (s_version >= 50)
		{
			if (!p_getbmap(&temp_zinit.gen_inst_init, f))
				return qe_invalid;
			if (!p_getarr(&temp_zinit.global_scrconfig, f))
				return qe_invalid;
			if (!p_getarr(&temp_zinit.hero_scrconfig, f))
				return qe_invalid;
		}
	}
	
	if(s_version < 46)
	{
		temp_zinit.sprite_z_thresholds[SPRITE_THRESHOLD_GROUND] = temp_zinit.jump_hero_layer_threshold;
		temp_zinit.sprite_z_thresholds[SPRITE_THRESHOLD_3] = temp_zinit.jump_hero_layer_threshold;
		temp_zinit.sprite_z_thresholds[SPRITE_THRESHOLD_4] = temp_zinit.jump_hero_layer_threshold;
		temp_zinit.sprite_z_thresholds[SPRITE_THRESHOLD_OVERHEAD] = word(-1);
		temp_zinit.sprite_z_thresholds[SPRITE_THRESHOLD_5] = word(-1);
	}
	
	if (should_skip)
		return 0;
	if (s_version < 49)
	{
		for (int q = 0; q < msg_count; ++q)
			MsgStrings[q].icon_more = msg_more;
	}

	if(loading_tileset_flags & TILESET_CLEARMAPS)
	{
		temp_zinit.last_map = 0;
		temp_zinit.last_screen = 0;
		temp_zinit.screen_data.clear();
	}
	temp_zinit.normalize();
	zinit = temp_zinit;
	
	if(zinit.heroAnimationStyle==las_zelda3slow)
	{
		hero_animation_speed=2;
	}
	else
	{
		hero_animation_speed=1;
	}
	
	return 0;
}
