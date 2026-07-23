#include "core/dmap.h"
#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern byte deprecated_rules[QUESTRULES_NEW_SIZE];

// State for fixup_old_dmap_subscreen_indices, recorded by read_old_subscreens.
static bool has_old_subscreens;
static int old_subscreen_reach[2]; // per sstACTIVE/sstPASSIVE: count reachable by the old engine
static int old_subscreen_fallback_type = -1;
static int old_subscreen_fallback_index = -1;

void reset_old_subscreen_fixup_state()
{
	has_old_subscreens = false;
	old_subscreen_reach[sstACTIVE] = old_subscreen_reach[sstPASSIVE] = 0;
	old_subscreen_fallback_type = old_subscreen_fallback_index = -1;
}

namespace {

enum { ssiBOMB, ssiSWORD, ssiSHIELD, ssiCANDLE, ssiLETTER, ssiPOTION, ssiLETTERPOTION, ssiBOW, ssiARROW, ssiBOWANDARROW, ssiBAIT, ssiRING, ssiBRACELET, ssiMAP,
       ssiCOMPASS, ssiBOSSKEY, ssiMAGICKEY, ssiBRANG, ssiWAND, ssiRAFT, ssiLADDER, ssiWHISTLE, ssiBOOK, ssiWALLET, ssiSBOMB, ssiHCPIECE, ssiAMULET, ssiFLIPPERS,
       ssiHOOKSHOT, ssiLENS, ssiHAMMER, ssiBOOTS, ssiDIVINEFIRE, ssiDIVINEESCAPE, ssiDIVINEPROTECTION, ssiQUIVER, ssiBOMBBAG, ssiCBYRNA, ssiROCS, ssiHOVERBOOTS,
       ssiSPINSCROLL, ssiCROSSSCROLL, ssiQUAKESCROLL, ssiWHISPRING, ssiCHARGERING, ssiPERILSCROLL, ssiWEALTHMEDAL, ssiHEARTRING, ssiMAGICRING, ssiSPINSCROLL2,
       ssiQUAKESCROLL2, ssiAGONY, ssiSTOMPBOOTS, ssiWHIMSICALRING, ssiPERILRING, ssiMAX
     };

int32_t read_old_subscreens(PACKFILE *f, word s_version)
{
	subscreens_active.clear();
	subscreens_passive.clear();
	subscreens_overlay.clear();
	subscreens_map.clear();
	bool hit_empty = false;
	for(int32_t i=0; i<OLD_MAXCUSTOMSUBSCREENS; i++)
	{
		subscreen_group g;
		memset(&g,0,sizeof(subscreen_group));
		int32_t ret = read_one_old_subscreen(f, &g, s_version);
		if(ret!=0)
			return ret;
		if(g.objects[0].type == ssoNULL)
		{
			hit_empty = true;
			continue;
		}
		bool passive = g.ss_type == sstPASSIVE;
		auto& vec = passive ? subscreens_passive : subscreens_active;
		ZCSubscreen& sub = vec.emplace_back();
		sub.load_old(g);
		// The old engine resolved dmap subscreen indices by scanning this
		// combined array, stopping at the first empty slot. Track what it
		// could reach, for fixup_old_dmap_subscreen_indices.
		if(!hit_empty)
		{
			old_subscreen_reach[passive ? sstPASSIVE : sstACTIVE] = vec.size();
			old_subscreen_fallback_type = passive ? sstPASSIVE : sstACTIVE;
			old_subscreen_fallback_index = vec.size()-1;
			has_old_subscreens = true;
		}
	}

	return 0;
}

} // end namespace

int32_t read_one_old_subscreen(PACKFILE *f, subscreen_group* g, word s_version)
{
	int32_t numsub=0;
	byte temp_ss=0;
	subscreen_object temp_sub_stack;
	subscreen_object *temp_sub = &temp_sub_stack;
	
	char tempname[64];

	// FWIW I never saw anything bigger than 20.
	#define MAX_DP1_LEN 1024
	char tempdp1[MAX_DP1_LEN];
	
	if(!pfread(tempname,64,f))
	{
		return qe_invalid;
	}
	
	if(s_version > 1)
	{
		if(!p_getc(&temp_ss,f))
		{
			return qe_invalid;
		}
	}
	
	if(s_version < 4)
	{
		uint8_t tmp=0;
		
		if(!p_getc(&tmp,f))
		{
			return qe_invalid;
		}
		
		numsub = (int32_t)tmp;
	}
	else
	{
		word tmp;
		
		if(!p_igetw(&tmp, f))
		{
			return qe_invalid;
		}
		
		numsub = (int32_t)tmp;
	}
	
	int32_t j;
	
	for(j=0; (j<OLD_MAXSUBSCREENITEMS&&j<numsub); j++)
	{
		memset(temp_sub,0,sizeof(subscreen_object));
		
		switch(g->objects[j].type)
		{
			case ssoTEXT:
			case ssoTEXTBOX:
			case ssoCURRENTITEMTEXT:
			case ssoCURRENTITEMCLASSTEXT:
				if(g->objects[j].dp1 != NULL) delete [](char *)g->objects[j].dp1;
				
				//fall through
			default:
				memset(&g->objects[j],0,sizeof(subscreen_object));
				break;
		}
		
		if(!p_getc(&(temp_sub->type),f))
			return qe_invalid;
		
		if(!p_getc(&(temp_sub->pos),f))
			return qe_invalid;
		
		if(s_version < 5)
		{
			switch(temp_sub->pos)
			{
				case 0:
					temp_sub->pos = sspUP | sspDOWN | sspSCROLLING;
					break;
					
				case 1:
					temp_sub->pos = sspUP;
					break;
					
				case 2:
					temp_sub->pos = sspDOWN;
					break;
					
				default:
					temp_sub->pos = 0;
			}
		}
		
		if(!p_igetw(&(temp_sub->x),f))
			return qe_invalid;
		
		if(!p_igetw(&(temp_sub->y),f))
			return qe_invalid;
		
		if(!p_igetw(&(temp_sub->w),f))
			return qe_invalid;
		
		if(!p_igetw(&(temp_sub->h),f))
			return qe_invalid;
		
		if(!p_getc(&(temp_sub->colortype1),f))
			return qe_invalid;
		
		if(!p_igetw(&(temp_sub->color1),f))
			return qe_invalid;
		
		if(!p_getc(&(temp_sub->colortype2),f))
			return qe_invalid;
		
		if(!p_igetw(&(temp_sub->color2),f))
			return qe_invalid;
		
		if(!p_getc(&(temp_sub->colortype3),f))
			return qe_invalid;
		
		if(!p_igetw(&(temp_sub->color3),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d1),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d2),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d3),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d4),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d5),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d6),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d7),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d8),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d9),f))
			return qe_invalid;
		
		if(!p_igetd(&(temp_sub->d10),f))
			return qe_invalid;
		
		if(s_version < 2)
		{
			if(!p_igetl(&(temp_sub->speed),f))
				return qe_invalid;
			
			if(!p_igetl(&(temp_sub->delay),f))
				return qe_invalid;
			
			if(!p_igetl(&(temp_sub->frame),f))
				return qe_invalid;
		}
		else
		{
			if(!p_getc(&(temp_sub->speed),f))
				return qe_invalid;
			
			if(!p_getc(&(temp_sub->delay),f))
				return qe_invalid;
			
			if(!p_igetw(&(temp_sub->frame),f))
				return qe_invalid;
		}
		
		int32_t temp_size=0;
		
		// bool deletets = false;
		switch(temp_sub->type)
		{
		case ssoTEXT:
		case ssoTEXTBOX:
		case ssoCURRENTITEMTEXT:
		case ssoCURRENTITEMCLASSTEXT:
		{
			word temptempsize;
			
			if(!p_igetw(&temptempsize,f))
			{
				return qe_invalid;
			}
			
			//temptempsize = temp1 + (temp2 << 8);
			temp_size = (int32_t)temptempsize;
			uint32_t char_length = temp_size+2;
			if (char_length > MAX_DP1_LEN)
			{
				return qe_invalid;
			}
			tempdp1[char_length - 1] = '\0';
			
			if(temp_size)
				if(!pfread(tempdp1,temp_size+1,f))
					return qe_invalid;
			break;
		}
			
		case ssoLIFEMETER:
			if(get_bit(deprecated_rules, 12) != 0) // qr_24HC
				temp_sub->d3 = 1;
			
			if(!p_getc(&(temp_sub->dp1),f))
				return qe_invalid;
			
			break;
			
			
		case ssoCURRENTITEM:
		
			if(s_version < 6)
			{
				switch(temp_sub->d1)
				{
				case ssiBOMB:
					temp_sub->d1 = itype_bomb;
					break;
					
				case ssiSWORD:
					temp_sub->d1 = itype_sword;
					break;
					
				case ssiSHIELD:
					temp_sub->d1 = itype_shield;
					break;
					
				case ssiCANDLE:
					temp_sub->d1 = itype_candle;
					break;
					
				case ssiLETTER:
					temp_sub->d1 = itype_letter;
					break;
					
				case ssiPOTION:
					temp_sub->d1 = itype_potion;
					break;
					
				case ssiLETTERPOTION:
					temp_sub->d1 = itype_letterpotion;
					break;
					
				case ssiBOW:
					temp_sub->d1 = itype_bow;
					break;
					
				case ssiARROW:
					temp_sub->d1 = itype_arrow;
					break;
					
				case ssiBOWANDARROW:
					temp_sub->d1 = itype_bowandarrow;
					break;
					
				case ssiBAIT:
					temp_sub->d1 = itype_bait;
					break;
					
				case ssiRING:
					temp_sub->d1 = itype_ring;
					break;
					
				case ssiBRACELET:
					temp_sub->d1 = itype_bracelet;
					break;
					
				case ssiMAP:
					temp_sub->d1 = itype_map;
					break;
					
				case ssiCOMPASS:
					temp_sub->d1 = itype_compass;
					break;
					
				case ssiBOSSKEY:
					temp_sub->d1 = itype_bosskey;
					break;
					
				case ssiMAGICKEY:
					temp_sub->d1 = itype_magickey;
					break;
					
				case ssiBRANG:
					temp_sub->d1 = itype_brang;
					break;
					
				case ssiWAND:
					temp_sub->d1 = itype_wand;
					break;
					
				case ssiRAFT:
					temp_sub->d1 = itype_raft;
					break;
					
				case ssiLADDER:
					temp_sub->d1 = itype_ladder;
					break;
					
				case ssiWHISTLE:
					temp_sub->d1 = itype_whistle;
					break;
					
				case ssiBOOK:
					temp_sub->d1 = itype_book;
					break;
					
				case ssiWALLET:
					temp_sub->d1 = itype_wallet;
					break;
					
				case ssiSBOMB:
					temp_sub->d1 = itype_sbomb;
					break;
					
				case ssiHCPIECE:
					temp_sub->d1 = itype_heartpiece;
					break;
					
				case ssiAMULET:
					temp_sub->d1 = itype_amulet;
					break;
					
				case ssiFLIPPERS:
					temp_sub->d1 = itype_flippers;
					break;
					
				case ssiHOOKSHOT:
					temp_sub->d1 = itype_hookshot;
					break;
					
				case ssiLENS:
					temp_sub->d1 = itype_lens;
					break;
					
				case ssiHAMMER:
					temp_sub->d1 = itype_hammer;
					break;
					
				case ssiBOOTS:
					temp_sub->d1 = itype_boots;
					break;
					
				case ssiDIVINEFIRE:
					temp_sub->d1 = itype_divinefire;
					break;
					
				case ssiDIVINEESCAPE:
					temp_sub->d1 = itype_divineescape;
					break;
					
				case ssiDIVINEPROTECTION:
					temp_sub->d1 = itype_divineprotection;
					break;
					
				case ssiQUIVER:
					temp_sub->d1 = itype_quiver;
					break;
					
				case ssiBOMBBAG:
					temp_sub->d1 = itype_bombbag;
					break;
					
				case ssiCBYRNA:
					temp_sub->d1 = itype_cbyrna;
					break;
					
				case ssiROCS:
					temp_sub->d1 = itype_rocs;
					break;
					
				case ssiHOVERBOOTS:
					temp_sub->d1 = itype_hoverboots;
					break;
					
				case ssiSPINSCROLL:
					temp_sub->d1 = itype_spinscroll;
					break;
					
				case ssiCROSSSCROLL:
					temp_sub->d1 = itype_crossscroll;
					break;
					
				case ssiQUAKESCROLL:
					temp_sub->d1 = itype_quakescroll;
					break;
					
				case ssiWHISPRING:
					temp_sub->d1 = itype_whispring;
					break;
					
				case ssiCHARGERING:
					temp_sub->d1 = itype_chargering;
					break;
					
				case ssiPERILSCROLL:
					temp_sub->d1 = itype_perilscroll;
					break;
					
				case ssiWEALTHMEDAL:
					temp_sub->d1 = itype_wealthmedal;
					break;
					
				case ssiHEARTRING:
					temp_sub->d1 = itype_heartring;
					break;
					
				case ssiMAGICRING:
					temp_sub->d1 = itype_magicring;
					break;
					
				case ssiSPINSCROLL2:
					temp_sub->d1 = itype_spinscroll2;
					break;
					
				case ssiQUAKESCROLL2:
					temp_sub->d1 = itype_quakescroll2;
					break;
					
				case ssiAGONY:
					temp_sub->d1 = itype_agony;
					break;
					
				case ssiSTOMPBOOTS:
					temp_sub->d1 = itype_stompboots;
					break;
					
				case ssiWHIMSICALRING:
					temp_sub->d1 = itype_whimsicalring;
					break;
					
				case ssiPERILRING:
					temp_sub->d1 = itype_perilring;
					break;
					
				default:
					temp_sub->d1 += itype_custom1 - ssiMAX;
				}
			}
			
			//fall-through
		default:
			if(!p_getc(&(temp_sub->dp1),f))
				return qe_invalid;
			
			break;
		}
		
		if(s_version < 7)
		{
			switch(temp_sub->type)
			{
				case ssoMAGICGAUGE:
				{
					if(!temp_sub->d9)
						temp_sub->d9 = -1; //-1 now represents 'always'
					break;
				}
				case ssoLIFEGAUGE:
					temp_sub->d9 = 0; //Unused, doesn't do anything? Clear it...
					break;
			}
		}
		
		switch(temp_sub->type)
		{
		case ssoTEXT:
		case ssoTEXTBOX:
		case ssoCURRENTITEMTEXT:
		case ssoCURRENTITEMCLASSTEXT:
			if(g->objects[j].dp1 != NULL) delete[](char *)g->objects[j].dp1;
			
			memcpy(&g->objects[j],temp_sub,sizeof(subscreen_object));
			g->objects[j].dp1 = new char[temp_size+2];
			strcpy((char*)g->objects[j].dp1,tempdp1);
			break;
			
		case ssoCOUNTER:
			if(s_version<3)
			{
				temp_sub->d6=(temp_sub->d6?1:0)+(temp_sub->d8?2:0);
				temp_sub->d8=0;
			}
			
		default:
			memcpy(&g->objects[j],temp_sub,sizeof(subscreen_object));
			break;
		}
		
		g->name[0] = '\0';
		strncat(g->name, tempname, 64 - 1);
		g->ss_type = temp_ss;
	}
	
	for(j=numsub; j<OLD_MAXSUBSCREENITEMS; j++)
	{
		//clear all unused object in this subscreen -DD
		switch(g->objects[j].type)
		{
		case ssoTEXT:
		case ssoTEXTBOX:
		case ssoCURRENTITEMTEXT:
		case ssoCURRENTITEMCLASSTEXT:
			if(g->objects[j].dp1 != NULL) delete [](char *)g->objects[j].dp1;
			
			//fall through
		default:
			memset(&g->objects[j],0,sizeof(subscreen_object));
			break;
		}
	}
	
	return 0;
}

int32_t readsubscreens(PACKFILE *f)
{
	word s_version;
	dword dummy;
	if(!p_igetw(&s_version,f))
		return qe_invalid;
	if (s_version > V_SUBSCREEN)
		return qe_version;
	FFCore.quest_format[vSubscreen] = s_version;
	if(!read_deprecated_section_cversion(f))
		return qe_invalid;
	if(!p_igetl(&dummy,f)) //section size
		return qe_invalid;

	reset_old_subscreen_fixup_state();
	if(s_version < 8)
		return read_old_subscreens(f,s_version);
	
	subscreens_active.clear();
	subscreens_passive.clear();
	subscreens_overlay.clear();
	subscreens_map.clear();
	
	byte sz;
	if(!p_getc(&sz,f))
		return qe_invalid;
	for(byte q = 0; q < sz; ++q)
	{
		ZCSubscreen& tmp = subscreens_active.emplace_back();
		if (auto ret = tmp.read(f, s_version))
			return ret;
	}
	if(!p_getc(&sz,f))
		return qe_invalid;
	for(byte q = 0; q < sz; ++q)
	{
		ZCSubscreen& tmp = subscreens_passive.emplace_back();
		if (auto ret = tmp.read(f, s_version))
			return ret;
	}
	if(!p_getc(&sz,f))
		return qe_invalid;
	for(byte q = 0; q < sz; ++q)
	{
		ZCSubscreen& tmp = subscreens_overlay.emplace_back();
		if (auto ret = tmp.read(f, s_version))
			return ret;
	}
	if (s_version >= 16)
	{
		if(!p_getc(&sz,f))
			return qe_invalid;
		for(byte q = 0; q < sz; ++q)
		{
			ZCSubscreen& tmp = subscreens_map.emplace_back();
			if (auto ret = tmp.read(f, s_version))
				return ret;
		}
	}
	return 0;
}

// The old engine (pre new-subscreen-system) resolved a dmap's active/passive
// subscreen index by scanning the combined subscreen array in order, counting
// only entries of the wanted type and stopping early at the first empty slot.
// If the index was out of range, it landed on the last group before the first
// empty slot, regardless of that group's type. "Link's Grand Adventure 2" (the
// only quest known to hit this) has dmaps saying 'passive #1' when only one
// passive subscreen exists, and displayed fine only because of that fallback -
// so emulate it here.
void fixup_old_dmap_subscreen_indices()
{
	if(!has_old_subscreens || old_subscreen_fallback_type < 0)
		return;

	int fallback_index[2] = {-1,-1};
	fallback_index[old_subscreen_fallback_type] = old_subscreen_fallback_index;
	auto fallback = [&](int type) -> int
	{
		if(fallback_index[type] < 0)
		{
			// The old engine happily drew a group of the wrong type; clone it
			// into the requested list to match.
			auto& src_vec = old_subscreen_fallback_type == sstPASSIVE ? subscreens_passive : subscreens_active;
			auto& dest_vec = type == sstPASSIVE ? subscreens_passive : subscreens_active;
			ZCSubscreen& copy = dest_vec.emplace_back(src_vec[old_subscreen_fallback_index]);
			copy.sub_type = type;
			fallback_index[type] = dest_vec.size()-1;
		}
		return fallback_index[type];
	};

	for(int i=0; i<MAXDMAPS; ++i)
	{
		if(DMaps[i].active_subscreen >= old_subscreen_reach[sstACTIVE])
			DMaps[i].active_subscreen = fallback(sstACTIVE);
		if(DMaps[i].passive_subscreen >= old_subscreen_reach[sstPASSIVE])
			DMaps[i].passive_subscreen = fallback(sstPASSIVE);
	}
}
