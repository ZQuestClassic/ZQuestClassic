#include "core/cpool.h"
#include "core/qrs.h"
#include "core/qst.h"
#include "defdata.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"
#include "zinfo.h"

extern const byte* legacy_skip_flags;
extern dword loading_tileset_flags;
extern zinfo* load_tmp_zi;

namespace {

void update_combo(newcombo& cmb, word section_version)
{
	if(section_version < 40)
	{
		switch(cmb.type)
		{
			case cWATER: case cSHALLOWWATER:
				cmb.c_attributes[8 + 6] = iwRipples;
				break;
			case cTALLGRASS: case cTALLGRASSNEXT: case cTALLGRASSTOUCHY:
				cmb.c_attributes[8 + 6] = iwTallGrass;
				break;
		}
	}
	if(section_version < 49)
	{
		if(cmb.type == cWATER || cmb.type == cSHALLOWWATER)
			cmb.sfx_landing = WAV_ZN1SPLASH;
	}
}

int32_t init_combo_classes()
{
	zinfo* zi = (load_tmp_zi ? load_tmp_zi : &ZI);
    for(int32_t i=0; i<cMAX; i++)
    {
        combo_class_buf[i] = default_combo_classes[i];
		if ( char const* nm = zi->getComboTypeName(i) )
		{
			size_t len = strlen(nm);
			for ( size_t q = 0; q < 64; q++ )
			{
				combo_class_buf[i].name[q] = (q<len ? nm[q] : 0);
			}
		}
    }
    
    return 0;
}

int32_t readcombos_old(word section_version, PACKFILE *f, zquestheader *Header, word version, word build, word start_combo, word max_combos)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_combos);
	byte tempbyte;
	if (!should_skip)
	{
		reset_all_combo_animations();
		init_combo_classes();
	}

	// combos
	word combos_used=0;
	int32_t dummy;
	byte padding;
	word wpadding;
	newcombo temp_combo;

	if (!should_skip)
	for(int32_t q = start_combo; q < start_combo+max_combos; ++q)
		combobuf[q].clear();

	if(version < 0x174)
	{
		combos_used=1024;
	}
	else if(version < 0x191)
	{
		combos_used=2048;
	}
	else
	{
		if(!p_igetw(&combos_used,f))
		{
			return qe_invalid;
		}
	}

	//finally...  section data
	for(int32_t i=0; i<combos_used; i++)
	{
		temp_combo.clear();
		combo_trigger& temp_trigger = temp_combo.triggers.emplace_back();
		int32_t temp_trigflags[6] = {0};
		
		if ( section_version >= 11 )
		{
			if(!p_igetl(&temp_combo.tile,f))
			{
				return qe_invalid;
			}
		}
		else
		{
			if(!p_igetw(&temp_combo.tile,f))
			{
				return qe_invalid;
			}
		}
		temp_combo.o_tile = temp_combo.tile;
		if(!p_getc(&temp_combo.flip,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_combo.walk,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_combo.type,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_combo.csets,f))
		{
			return qe_invalid;
		}
		
		if(version < 0x193)
		{
			if(!p_getc(&padding,f))
				return qe_invalid;
			
			if(!p_getc(&padding,f))
				return qe_invalid;
			
			if(version < 0x192)
			{
				if(version == 0x191)
				{
					for(int32_t tmpcounter=0; tmpcounter<16; tmpcounter++)
					{
						if(!p_getc(&padding,f))
							return qe_invalid;
					}
				}
			}
		}
		if(version >= 0x192)
		{
			if(!p_getc(&temp_combo.frames,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.speed,f))
				return qe_invalid;
			
			if(!p_igetw(&temp_combo.nextcombo,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.nextcset,f))
				return qe_invalid;
			
			//Base flag
			if(section_version>=3)
				if(!p_getc(&temp_combo.flag,f))
					return qe_invalid;
			
			if(section_version>=4)
			{
				if(!p_getc(&temp_combo.skipanim,f))
					return qe_invalid;
				
				if(!p_igetw(&temp_combo.nexttimer,f))
					return qe_invalid;
			}
			
			if(section_version>=5)
				if(!p_getc(&temp_combo.skipanimy,f))
					return qe_invalid;
			
			if(section_version>=6)
			{
				if(!p_getc(&temp_combo.animflags,f))
					return qe_invalid;
				
				if(section_version == 6)
					temp_combo.animflags = temp_combo.animflags ? AF_FRESH : 0;
			}
			
			if(section_version>=8) //combo Attributes[4] and userflags.
			{
				for ( int32_t q = 0; q < 4; q++ )
					if(!p_igetzf(&temp_combo.c_attributes[q],f))
						return qe_invalid;
				if(!p_igetl(&temp_combo.usrflags,f))
					return qe_invalid;
				if(section_version >= 20)
					if(!p_igetw(&temp_combo.genflags,f))
						return qe_invalid;
			}
			if(section_version>=10) //combo trigger flags
			{
				for ( int32_t q = 0; q < 3; q++ )
					if(!p_igetl(&temp_trigflags[q],f))
						return qe_invalid;
			}
			else if(section_version==9) //combo trigger flags, V9 only had two indices
			{
				for ( int32_t q = 0; q < 2; q++ )
					if(!p_igetl(&temp_trigflags[q],f))
						return qe_invalid;
			}
			if(section_version >= 9)
				if(!p_igetl(&temp_trigger.triggerlevel,f))
					return qe_invalid;
			if(section_version >= 22)
				if(!p_getc(&temp_trigger.triggerbtn,f))
					return qe_invalid;
			if(section_version >= 24)
			{
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_trigger.triggeritem = tempbyte;
				if(!p_getc(&tempbyte, f))
					return qe_invalid;
				temp_trigger.trigtimer = tempbyte;
			}
			if(section_version >= 25)
			{
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_trigger.trigsfx = tempbyte;
			}
			if(section_version >= 27)
				if(!p_igetl(&temp_trigger.trigchange,f))
					return qe_invalid;
			
			if(section_version >= 29)
			{
				if(!p_igetw(&temp_trigger.trigprox,f))
					return qe_invalid;
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_trigger.trigctr = tempbyte;
				if(!p_igetl(&temp_trigger.trigctramnt,f))
					return qe_invalid;
			}
			if(section_version >= 30)
				if(!p_getc(&temp_trigger.triglbeam,f))
					return qe_invalid;
			if(section_version >= 31)
			{
				if(!p_getc(&temp_trigger.trigcschange,f))
					return qe_invalid;
				if(!p_igetw(&temp_trigger.spawnitem,f))
					return qe_invalid;
				if(!p_igetw(&temp_trigger.spawnenemy,f))
					return qe_invalid;
				if(!p_getc(&temp_trigger.exstate,f))
					return qe_invalid;
				if(!p_igetl(&temp_trigger.spawnip,f))
					return qe_invalid;
				if(!p_getc(&temp_trigger.trigcopycat,f))
					return qe_invalid;
			}
			if(section_version >= 32)
				if(!p_getc(&temp_trigger.trigcooldown,f))
					return qe_invalid;
			
			if(section_version>=12) //combo label
			{
				char label[12];
				label[11] = '\0';
				for ( int32_t q = 0; q < 11; q++ )
					if(!p_getc(&label[q],f))
						return qe_invalid;
				temp_combo.label = label;
			}
			if(section_version >= 13 && section_version < 63) //attribytes[4]
			{
				for ( int32_t q = 0; q < 4; q++ )
				{
					if(!p_getc(&padding,f))
						return qe_invalid;
					temp_combo.c_attributes[8 + q] = padding;
				}
			}
			/* HIGHLY UNORTHODOX UPDATING THING, by Deedee
			* This fixes a poor implementation of a ->next flag bug thing.
			* Zoria didn't bump up the versions as liberally as he should have, but thankfully
			* there was a version bump a few weeks before a change that broke stuff.
			*/
			if (section_version >= 13 && section_version < 21)
			{
				set_qr(qr_BUGGY_BUGGY_SLASH_TRIGGERS,1);
			}
			//combo scripts
			if(section_version>=14) 
			{
				if(!p_igetw(&temp_combo.script,f))
					return qe_invalid;
				for ( int32_t q = 0; q < 2; q++ )
					if(!p_igetl(&temp_combo.initd[q],f))
						return qe_invalid;
			}
			if(section_version>=15)
			{
				if(!p_igetl(&temp_combo.o_tile,f)) return qe_invalid;
				if(!temp_combo.o_tile) temp_combo.o_tile = temp_combo.tile;
				if(!p_getc(&temp_combo.cur_frame,f)) return qe_invalid;
				if(!p_getc(&temp_combo.aclk,f)) return qe_invalid;
			}
			if(section_version >= 17 && section_version < 63) //attribytes[4]
			{
				for ( int32_t q = 4; q < 8; q++ ) //bump up attribytes...
				{
					if(!p_getc(&padding,f))
						return qe_invalid;
					temp_combo.c_attributes[8 + q] = padding;
				}
				for ( int32_t q = 0; q < 8; q++ ) //...and add attrishorts
				{
					if(!p_igetw(&wpadding,f))
						return qe_invalid;
					temp_combo.c_attributes[16 + q] = int16_t(wpadding);
				}
			}
			
			if(version < 0x193)
				for(int32_t q=0; q<11; q++)
					if(!p_getc(&dummy,f))
						return qe_invalid;
		}
		
		//Goriya tiles were flipped around in 2.11 build 7. Compensate for the flip here. -DD
		if((version < 0x211)||((version == 0x211)&&(build<7)))
		{
			if(!get_qr(qr_NEWENEMYTILES))
			{
				switch(temp_combo.tile)
				{
					case 130:
						temp_combo.tile = 132;
						break;
						
					case 131:
						temp_combo.tile = 133;
						break;
						
					case 132:
						temp_combo.tile = 130;
						break;
						
					case 133:
						temp_combo.tile = 131;
						break;
				}
			}
		}
		
		if(section_version < 15)
			temp_combo.o_tile = temp_combo.tile;
		
		if(section_version<18) //upper bits for .walk
			temp_combo.walk |= 0xF0;
			
		if(section_version < 19)
			for(int32_t q = 0; q < 4; ++q)
				temp_combo.c_attributes[q] *= 10000;
		
		if(section_version < 23)
		{
			switch(temp_combo.type) //TRIGFLAG_CMBTYPEFX now required for combotype-specific effects
			{
				case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
				case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
				case cTRIGGERGENERIC: case cCSWITCH:
					temp_trigflags[TRIGFLAG_CMBTYPEFX/32] |= 1<<(TRIGFLAG_CMBTYPEFX%32);
			}
		}
		if(section_version < 25)
		{
			switch(temp_combo.type)
			{
				case cLOCKBLOCK: case cBOSSLOCKBLOCK:
					if(!(temp_combo.usrflags & cflag3))
						temp_combo.c_attributes[8 + 3] = WAV_DOOR;
					temp_combo.usrflags &= ~cflag3;
					break;
			}
		}
		
		if(section_version < 26)
			if(temp_combo.type == cARMOS)
				if(temp_combo.usrflags & cflag1)
					temp_combo.usrflags |= cflag3;
		
		if(section_version < 27)
		{
			if(temp_trigflags[0] & 0x00040000) //'next'
				temp_trigger.trigchange = 1;
			else if(temp_trigflags[0] & 0x00080000) //'prev'
				temp_trigger.trigchange = -1;
			else temp_trigger.trigchange = 0;
			temp_trigflags[0] &= ~(0x00040000|0x00080000);
		}
		if(section_version < 28)
		{
			switch(temp_combo.type)
			{
				case cLOCKBLOCK: case cLOCKEDCHEST:
					if(temp_combo.usrflags & cflag7)
						temp_combo.usrflags |= cflag8;
					else temp_combo.usrflags &= ~cflag8;
					temp_combo.usrflags &= ~cflag7;
					break;
			}
			switch(temp_combo.type)
			{
				case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
					temp_combo.c_attributes[16 + 2] = -1;
					temp_combo.usrflags |= cflag7;
					break;
			}
		}
		if(section_version < 20)
		{
			temp_combo.genflags = 0;
			switch(temp_combo.type)
			{
				case cPUSH_WAIT: case cPUSH_HEAVY:
				case cPUSH_HW: case cL_STATUE:
				case cR_STATUE: case cPUSH_HEAVY2:
				case cPUSH_HW2: case cPOUND:
				case cC_STATUE: case cMIRROR:
				case cMIRRORSLASH: case cMIRRORBACKSLASH:
				case cMAGICPRISM: case cMAGICPRISM4:
				case cMAGICSPONGE: case cEYEBALL_A:
				case cEYEBALL_B: case cEYEBALL_4:
				case cBUSH: case cFLOWERS:
				case cLOCKBLOCK: case cLOCKBLOCK2:
				case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2:
				case cCHEST: case cCHEST2:
				case cLOCKEDCHEST: case cLOCKEDCHEST2:
				case cBOSSCHEST: case cBOSSCHEST2:
				case cBUSHNEXT: case cBUSHTOUCHY:
				case cFLOWERSTOUCHY: case cBUSHNEXTTOUCHY:
				case cSIGNPOST: case cCSWITCHBLOCK:
				case cTORCH: case cTRIGGERGENERIC:
					if(temp_combo.usrflags & cflag16)
					{
						temp_combo.genflags |= cflag1;
						temp_combo.usrflags &= ~cflag16;
					}
					break;
			}
		}
		
		temp_trigger.trigger_flags.clear();
		for(size_t q = 0; q < 32*6; ++q)
		{
			auto ind = q/32;
			auto bit = 1<<(q%32);
			if(temp_trigflags[ind] & bit)
				temp_trigger.trigger_flags.set(q, true);
		}
		
		if(temp_trigger.is_blank())
			temp_combo.triggers.clear();
		
		update_combo(temp_combo, section_version);
		if(section_version < 67 && !(Header && Header->version_major == 2 && Header->version_minor == 55 && Header->version_patch >= 14))
		{
			if(isCuttableType(temp_combo.type) && !temp_combo.triggers.empty())
				temp_combo.triggers.front().trigger_flags.set(TRIGFLAG_CMBTYPEFX, true);
		}
		
		if(i>=start_combo && !should_skip)
		{
			if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
			{
				temp_combo.script = 0;
				for(int q = 0; q < 8; ++q)
					temp_combo.initd[q] = 0;
			}
			combobuf[i] = temp_combo;
		}
	}

	if (should_skip)
		return 0;

	if((version < 0x192)|| ((version == 0x192)&&(build<185)))
	{
		for(int32_t tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
		{
			if(combobuf[tmpcounter].type==cHOOKSHOTONLY)
			{
				combobuf[tmpcounter].type=cLADDERHOOKSHOT;
			}
		}
	}
	
	//June 3 2012; ladder only is broken in 2.10 and allows the hookshot also. -Gleeok
	if(version == 0x210 && get_app_id() != App::zquest)
	{
		for(int32_t tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
			if(combobuf[tmpcounter].type == cLADDERONLY)
				combobuf[tmpcounter].type = cLADDERHOOKSHOT;
	}
	
	if(section_version<7)
	{
		for(int32_t tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
		{
			switch(combobuf[tmpcounter].type)
			{
			case cSLASH:
				combobuf[tmpcounter].type=cSLASHTOUCHY;
				break;
				
			case cSLASHITEM:
				combobuf[tmpcounter].type=cSLASHITEMTOUCHY;
				break;
				
			case cBUSH:
				combobuf[tmpcounter].type=cBUSHTOUCHY;
				break;
				
			case cFLOWERS:
				combobuf[tmpcounter].type=cFLOWERSTOUCHY;
				break;
				
			case cTALLGRASS:
				combobuf[tmpcounter].type=cTALLGRASSTOUCHY;
				break;
				
			case cSLASHNEXT:
				combobuf[tmpcounter].type=cSLASHNEXTTOUCHY;
				break;
				
			case cSLASHNEXTITEM:
				combobuf[tmpcounter].type=cSLASHNEXTITEMTOUCHY;
				break;
				
			case cBUSHNEXT:
				combobuf[tmpcounter].type=cBUSHNEXTTOUCHY;
				break;
			}
		}
	}
	if (section_version < 16)
	{
		for(int32_t tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
		{
			if (combobuf[tmpcounter].type == cWATER)
			{
				combobuf[tmpcounter].c_attributes[0] = 4;
			}
		}
	}
	if(!get_qr(qr_ALLOW_EDITING_COMBO_0))
	{
		combobuf[0].walk = 0xF0;
		combobuf[0].type = 0;
		combobuf[0].flag = 0;
	}

	//Now for the new combo alias reset
	if(section_version<2)
	{
		for(int32_t j=0; j<MAXCOMBOALIASES; j++)
		{
			combo_aliases[j].width = 0;
			combo_aliases[j].height = 0;
			combo_aliases[j].layermask = 0;
			combo_aliases[j].combos.clear();
			combo_aliases[j].csets.clear();
		}
	}


	setup_combo_animations();
	setup_combo_animations2();
	return 0;
}

int32_t readcombo_triggers_loop(PACKFILE* f, word s_version, combo_trigger& temp_trigger)
{
	byte tempbyte;
	word tempword;
	if(s_version >= 52)
		if(!p_getcstr(&temp_trigger.label,f))
			return qe_invalid;
	
	if(s_version < 57)
	{
		int32_t temp_trigflags[6] = {0};
		int numtrigs = s_version < 36 ? 3 : 6;
		for ( int32_t q = 0; q < numtrigs; q++ )
			if(!p_igetl(&temp_trigflags[q],f))
				return qe_invalid;
		temp_trigger.trigger_flags.clear();
		for(size_t q = 0; q < 32*numtrigs; ++q)
		{
			auto ind = q/32;
			auto bit = 1<<(q%32);
			if(temp_trigflags[ind] & bit)
				temp_trigger.trigger_flags.set(q, true);
		}
	}
	else if(!p_getbitstr(&temp_trigger.trigger_flags,f))
		return qe_invalid;
	if(!p_igetl(&temp_trigger.triggerlevel,f))
		return qe_invalid;
	if(!p_getc(&temp_trigger.triggerbtn,f))
		return qe_invalid;
	if (s_version >= 65)
	{
		if(!p_igetw(&temp_trigger.triggeritem,f))
			return qe_invalid;
	}
	else
	{
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_trigger.triggeritem = tempbyte;
	}
	if(s_version >= 53)
	{
		if(!p_igetw(&temp_trigger.trigtimer,f))
			return qe_invalid;
	}
	else
	{
		if(!p_getc(&tempbyte, f))
			return qe_invalid;
		temp_trigger.trigtimer = tempbyte;
	}
	if (s_version < 64)
	{
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_trigger.trigsfx = tempbyte;
	}
	else
	{
		if(!p_igetw(&temp_trigger.trigsfx,f))
			return qe_invalid;
		
	}
	if(!p_igetl(&temp_trigger.trigchange,f))
		return qe_invalid;
	if(!p_igetw(&temp_trigger.trigprox,f))
		return qe_invalid;
	if(s_version >= 53)
	{
		if(!p_igetw(&temp_trigger.trigctr,f))
			return qe_invalid;
	}
	else
	{
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_trigger.trigctr = tempbyte;
	}
	if(!p_igetl(&temp_trigger.trigctramnt,f))
		return qe_invalid;
	if(!p_getc(&temp_trigger.triglbeam,f))
		return qe_invalid;
	if(!p_getc(&temp_trigger.trigcschange,f))
		return qe_invalid;
	if (s_version >= 65)
	{
		if(!p_igetl(&temp_trigger.spawnitem,f))
			return qe_invalid;
	}
	else
	{
		if(!p_igetw(&tempword,f))
			return qe_invalid;
		temp_trigger.spawnitem = int16_t(tempword);
	}
	if(!p_igetw(&temp_trigger.spawnenemy,f))
		return qe_invalid;
	if(!p_getc(&temp_trigger.exstate,f))
		return qe_invalid;
	if(!p_igetl(&temp_trigger.spawnip,f))
		return qe_invalid;
	if(!p_getc(&temp_trigger.trigcopycat,f))
		return qe_invalid;
	if(!p_getc(&temp_trigger.trigcooldown,f))
		return qe_invalid;
	if(s_version >= 35)
	{
		if(!p_igetw(&temp_trigger.prompt_cid,f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.prompt_cs,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.prompt_x,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.prompt_y,f))
			return qe_invalid;
	}
	if(s_version >= 36)
	{
		if(!p_getc(&temp_trigger.trig_lstate,f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.trig_gstate,f))
			return qe_invalid;
		if(!p_igetl(&temp_trigger.trig_statetime,f))
			return qe_invalid;
	}
	if(s_version >= 37)
	{
		if(!p_igetw(&temp_trigger.trig_genscr,f))
			return qe_invalid;
	}
	if(s_version >= 38)
	{
		if(!p_getc(&temp_trigger.trig_group,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trig_group_val,f))
			return qe_invalid;
	}
	if(s_version >= 45)
	{
		if(!p_getc(&temp_trigger.exdoor_dir,f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.exdoor_ind,f))
			return qe_invalid;
	}
	if(s_version >= 46)
	{
		if (s_version >= 59)
		{
			if(!p_igetw(&temp_trigger.trig_levelitems,f))
				return qe_invalid;
		}
		else
		{
			if(!p_getc(&tempbyte,f))
				return qe_invalid;
			temp_trigger.trig_levelitems = word(tempbyte);
		}
		if(!p_igetw(&temp_trigger.trigdmlevel,f))
			return qe_invalid;
		if(s_version >= 48)
		{
			for(int q = 0; q < 3; ++q)
				if(!p_igetw(&temp_trigger.trigtint[q],f))
					return qe_invalid;
		}
		else
		{
			for(int q = 0; q < 3; ++q)
				if(!p_getc(&temp_trigger.trigtint[q],f))
					return qe_invalid;
			for(int q = 0; q < 3; ++q)
			{
				int v = temp_trigger.trigtint[q];
				int va = abs(v);
				temp_trigger.trigtint[q] = _rgb_scale_6[va] * sign(v);
			}
		}
		if(!p_igetw(&temp_trigger.triglvlpalette,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trigbosspalette,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trigquaketime,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trigwavytime,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trig_swjinxtime,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trig_itmjinxtime,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trig_stuntime,f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.trig_bunnytime,f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.trig_pushtime,f))
			return qe_invalid;
	}
	if(s_version >= 47)
	{
		if (!p_igetw(&temp_trigger.trig_shieldjinxtime, f))
			return qe_invalid;
	}
	if(s_version >= 53)
	{
		if(!p_igetl(&temp_trigger.req_level_state, f))
			return qe_invalid;
		if(!p_igetl(&temp_trigger.unreq_level_state, f))
			return qe_invalid;
		if(!p_getbitstr(&temp_trigger.req_global_state, f))
			return qe_invalid;
		if(!p_getbitstr(&temp_trigger.unreq_global_state, f))
			return qe_invalid;
		if(!p_igetw(&temp_trigger.fail_prompt_cid,f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.fail_prompt_cs,f))
			return qe_invalid;
		if(!p_igetl(&temp_trigger.trig_msgstr, f))
			return qe_invalid;
		if(!p_igetl(&temp_trigger.fail_msgstr, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.player_bounce, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.req_player_z, f))
			return qe_invalid;
	}
	else
	{
		temp_trigger.fail_prompt_cid = temp_trigger.prompt_cid;
		temp_trigger.fail_prompt_cs = temp_trigger.prompt_cs;
	}
	if(s_version >= 54)
	{
		if(!p_getc(&temp_trigger.req_player_dir,f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.dest_player_x, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.dest_player_y, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.dest_player_z, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.req_player_jump, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.req_player_x, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.req_player_y, f))
			return qe_invalid;
	}
	if(s_version >= 56)
	{
		if(!p_getc(&temp_trigger.dest_player_dir, f))
			return qe_invalid;
		if(!p_igetl(&temp_trigger.force_ice_combo, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.force_ice_vx, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.force_ice_vy, f))
			return qe_invalid;
	}
	if(s_version >= 58)
	{
		if(!p_igetzf(&temp_trigger.trig_gravity, f))
			return qe_invalid;
		if(!p_igetzf(&temp_trigger.trig_terminal_v, f))
			return qe_invalid;
	}
	if (s_version >= 61)
	{
		if(!p_getbitstr(&temp_trigger.req_screen_state, f))
			return qe_invalid;
		if(!p_getbitstr(&temp_trigger.unreq_screen_state, f))
			return qe_invalid;
		if(!p_getbitstr(&temp_trigger.req_screen_ex_state, f))
			return qe_invalid;
		if(!p_getbitstr(&temp_trigger.unreq_screen_ex_state, f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.trigstatemap, f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.trigstatescreen, f))
			return qe_invalid;
	}
	if (s_version >= 62)
	{
		if(!p_igetl(&temp_trigger.play_music, f))
			return qe_invalid;
		if(!p_getc(&temp_trigger.set_music_refresh, f))
			return qe_invalid;
	}
	return 0;
}

} // end namespace

bool isCuttableType(int32_t type)
{
	switch(type)
	{
		case cSLASH:
		case cSLASHITEM:
		case cBUSH:
		case cFLOWERS:
		case cTALLGRASS:
		case cTALLGRASSNEXT:
		case cSLASHNEXT:
		case cSLASHNEXTITEM:
		case cBUSHNEXT:
		
		case cSLASHTOUCHY:
		case cSLASHITEMTOUCHY:
		case cBUSHTOUCHY:
		case cFLOWERSTOUCHY:
		case cTALLGRASSTOUCHY:
		case cSLASHNEXTTOUCHY:
		case cSLASHNEXTITEMTOUCHY:
		case cBUSHNEXTTOUCHY:
			return true;
	}
	
	return false;
}

int32_t readcombo_loop(PACKFILE* f, word s_version, newcombo& temp_combo, zquestheader *Header)
{
	//WARNING: `Header` can be `nullptr` here from import/export code
	byte tempbyte;
	word tempword;
	word combo_has_flags;
	if(s_version < 55)
	{
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		combo_has_flags = tempbyte;
	}
	else
	{
		if(!p_igetw(&combo_has_flags,f))
			return qe_invalid;
	}
	
	temp_combo.clear();
	if(combo_has_flags)
	{
		if(combo_has_flags&CHAS_BASIC)
		{
			if(!p_igetl(&temp_combo.tile,f))
				return qe_invalid;
			temp_combo.o_tile = temp_combo.tile;
			
			if(!p_getc(&temp_combo.flip,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.walk,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.type,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.flag,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.csets,f))
				return qe_invalid;
		}
		if(combo_has_flags&CHAS_SCRIPT)
		{
			if (s_version>=41)
			{
				p_getcstr(&temp_combo.label, f);
			}
			else
			{
				char label[12];
				label[11] = '\0';
				for ( int32_t q = 0; q < 11; q++ )
					if(!p_getc(&label[q],f))
						return qe_invalid;
				temp_combo.label = label;
			}

			if(!p_igetw(&temp_combo.script,f)) return qe_invalid;
			auto initd_count = s_version >= 43 ? 8 : 2;
			for ( int32_t q = 0; q < initd_count; q++ )
				if(!p_igetl(&temp_combo.initd[q],f))
					return qe_invalid;
		}
		if(combo_has_flags&CHAS_ANIM)
		{
			if(!p_getc(&temp_combo.frames,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.speed,f))
				return qe_invalid;
			
			if(!p_igetw(&temp_combo.nextcombo,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.nextcset,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.skipanim,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.skipanimy,f))
				return qe_invalid;
			
			if(!p_getc(&temp_combo.animflags,f))
				return qe_invalid;
		}
		if(combo_has_flags&CHAS_ATTRIB)
		{
			if (s_version < 63)
			{
				for ( int32_t q = 0; q < 4; q++ )
					if(!p_igetzf(&temp_combo.c_attributes[q],f))
						return qe_invalid;
				for ( int32_t q = 0; q < 8; q++ )
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.c_attributes[8 + q] = tempbyte;
				}
				for ( int32_t q = 0; q < 8; q++ )
				{
					if(!p_igetw(&tempword,f))
						return qe_invalid;
					temp_combo.c_attributes[16 + q] = int16_t(tempword);
				}
			}
			else
			{
				word num_attributes;
				if (!p_igetw(&num_attributes, f))
					return qe_invalid;
				if (num_attributes > NUM_COMBO_ATTRIBUTES)
					return qe_invalid;
				for (size_t q = 0; q < num_attributes; ++q)
				{
					if (!p_igetzf(&temp_combo.c_attributes[q],f))
						return qe_invalid;
				}
			}
		}
		if(combo_has_flags&CHAS_FLAG)
		{
			if(!p_igetl(&temp_combo.usrflags,f))
				return qe_invalid;
			if(!p_igetw(&temp_combo.genflags,f))
				return qe_invalid;
		}
		if(combo_has_flags&CHAS_TRIG)
		{
			byte count = 1;
			if(s_version >= 52)
				if(!p_getc(&count, f))
					return qe_invalid;
			
			for(byte q = 0; q < count; ++q)
			{
				combo_trigger& temp_trigger = temp_combo.triggers.emplace_back();
				auto ret = readcombo_triggers_loop(f, s_version, temp_trigger);
				if(ret)
					return ret;
			}
			
			if(s_version < 52)
			{
				if(!temp_combo.triggers.empty())
					temp_combo.only_gentrig = (temp_combo.triggers[0].trigger_flags.get(TRIGFLAG_ONLYGENTRIG)) ? 1 : 0;
			}
			else if(!p_getc(&temp_combo.only_gentrig,f))
				return qe_invalid;
		}
		if(combo_has_flags&CHAS_LIFT)
		{
			if(!p_igetw(&temp_combo.liftcmb,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftcs,f))
				return qe_invalid;
			if(!p_igetw(&temp_combo.liftundercmb,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftundercs,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftdmg,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftlvl,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftitm,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftflags,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftgfx,f))
				return qe_invalid;
			if (s_version < 66)
			{
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_combo.liftsprite = tempbyte;
			}
			else
			{
				if(!p_igetw(&temp_combo.liftsprite,f))
					return qe_invalid;
			}
			if (s_version < 64)
			{
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_combo.liftsfx = tempbyte;
			}
			else
			{
				if(!p_igetw(&temp_combo.liftsfx,f))
					return qe_invalid;
			}
			if(!p_igetw(&temp_combo.liftbreaksprite,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.liftbreaksfx,f))
				return qe_invalid;
			if(s_version >= 34)
			{
				if(!p_getc(&temp_combo.lifthei,f))
					return qe_invalid;
				if(!p_getc(&temp_combo.lifttime,f))
					return qe_invalid;
			}
			if (s_version >= 65)
			{
				if(!p_igetw(&temp_combo.lift_parent_item,f))
					return qe_invalid;
			}
			else if(s_version >= 39)
			{
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_combo.lift_parent_item = tempbyte;
			}
			auto& weap_data = temp_combo.lift_weap_data;
			if(s_version >= 51 && s_version < 55)
			{
				if(!p_getc(&weap_data.light_rads[WPNSPR_BASE],f))
					return qe_invalid;
				if(!p_getc(&weap_data.glow_shape,f))
					return qe_invalid;
			}
			
			if(s_version >= 55)
			{
				if(auto ret = read_weap_data(weap_data, f))
					return ret;
			}
			else
			{
				auto const& pitm = get_item_data(temp_combo.lift_parent_item);
				auto weap_glow = weap_data.light_rads[WPNSPR_BASE];
				switch(pitm.type)
				{
					case itype_bomb:
					case itype_sbomb:
						weap_data = pitm.weap_data;
						if(!(weap_data.wflags & WFLAG_UPDATE_IGNITE_SPRITE))
						{
							weap_data.flags |= wdata_glow_rad;
							weap_data.light_rads[WPNSPR_BASE] = weap_glow;
						}
						break;
					default:
						weap_data.wflags |= WFLAG_BREAK_WHEN_LANDING;
						weap_data.flags |= wdata_glow_rad;
				}
				weap_data.moveflags |= move_obeys_grav|move_can_pitfall;
				if(temp_combo.liftflags & LF_BREAKONSOLID)
					weap_data.wflags |= WFLAG_BREAK_ON_SOLID;
			}
		}
		
		if(combo_has_flags&CHAS_GENERAL)
		{
			if(!p_getc(&temp_combo.speed_mult,f))
				return qe_invalid;
			if(!p_getc(&temp_combo.speed_div,f))
				return qe_invalid;
			if(!p_igetzf(&temp_combo.speed_add,f))
				return qe_invalid;
			if(s_version >= 42)
			{
				if (s_version < 64)
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_appear = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_disappear = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_loop = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_walking = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_standing = tempbyte;
				}
				else
				{
					if(!p_igetw(&temp_combo.sfx_appear,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.sfx_disappear,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.sfx_loop,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.sfx_walking,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.sfx_standing,f))
						return qe_invalid;
				}
				if (s_version < 66)
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.spr_appear = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.spr_disappear = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.spr_walking = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.spr_standing = tempbyte;
				}
				else
				{
					if(!p_igetw(&temp_combo.spr_appear,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.spr_disappear,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.spr_walking,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.spr_standing,f))
						return qe_invalid;
				}
			}
			if (s_version < 64)
			{
				if(s_version >= 44)
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_tap = tempbyte;
				}
				if(s_version >= 49)
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_landing = tempbyte;
				}
			}
			else
			{
				if(!p_igetw(&temp_combo.sfx_tap,f))
					return qe_invalid;
				if(!p_igetw(&temp_combo.sfx_landing,f))
					return qe_invalid;
			}
			if(s_version >= 50)
			{
				if (s_version < 66)
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.spr_falling = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.spr_drowning = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.spr_lava_drowning = tempbyte;
				}
				else
				{
					if(!p_igetw(&temp_combo.spr_falling,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.spr_drowning,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.spr_lava_drowning,f))
						return qe_invalid;
				}
				
				if (s_version < 64)
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_falling = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_drowning = tempbyte;
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					temp_combo.sfx_lava_drowning = tempbyte;
				}
				else
				{
					if(!p_igetw(&temp_combo.sfx_falling,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.sfx_drowning,f))
						return qe_invalid;
					if(!p_igetw(&temp_combo.sfx_lava_drowning,f))
						return qe_invalid;
				}
			}
			if(s_version >= 56)
			{
				if(!p_igetzf(&temp_combo.z_height,f))
					return qe_invalid;
				if(!p_igetzf(&temp_combo.z_step_height,f))
					return qe_invalid;
			}
			if(s_version >= 60)
			{
				if(!p_getc(&temp_combo.dive_under_level,f))
					return qe_invalid;
			}
		}
		if(combo_has_flags&CHAS_MISC_WEAP_DATA)
		{
			if(auto ret = read_weap_data(temp_combo.misc_weap_data, f))
				return ret;
		}
	}
	update_combo(temp_combo, s_version);
	if(s_version < 67 && !(Header && Header->version_major == 2 && Header->version_minor == 55 && Header->version_patch >= 14))
	{
		if(isCuttableType(temp_combo.type) && !temp_combo.triggers.empty())
			temp_combo.triggers.front().trigger_flags.set(TRIGFLAG_CMBTYPEFX, true);
	}
	return 0;
}

int32_t readcombos(PACKFILE *f, zquestheader *Header, word version, word build, word start_combo, word max_combos)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_combos);

	word section_version=0;
	word combos_used=0;
	int32_t dummy;
	byte padding;
	newcombo temp_combo;
	
	if (!should_skip)
	{
		reset_all_combo_animations();
		init_combo_classes();

		for(int32_t q = start_combo; q < start_combo+max_combos; ++q)
			combobuf[q].clear();
	}
	
	if(version > 0x192) //Version info
	{
		if(!p_igetw(&section_version,f))
		{
			return qe_invalid;
		}
		FFCore.quest_format[vCombos] = section_version;
		if(!read_deprecated_section_cversion(f))
		{
			return qe_invalid;
		}
		
		//section size
		if(!p_igetl(&dummy,f))
		{
			return qe_invalid;
		}
	}
	
	if(section_version > 32) //Cleanup time!
	{
		if(!p_igetw(&combos_used,f))
		{
			return qe_invalid;
		}
		for(int32_t i=0; i<combos_used; i++)
		{
			auto ret = readcombo_loop(f,section_version,temp_combo,Header);
			if(ret) return ret;
			if(i>=start_combo)
			{
				if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
				{
					temp_combo.script = 0;
					for(int q = 0; q < 8; ++q)
						temp_combo.initd[q] = 0;
				}
				combobuf[i] = temp_combo;
			}
		}
	}
	else //Call the old function for all old versions
	{
		auto ret = readcombos_old(section_version,f,Header,version,build,start_combo,max_combos);
		if(ret) return ret; //error, end read
	}

	if (should_skip)
		return 0;

	if(!get_qr(qr_ALLOW_EDITING_COMBO_0))
	{
		combobuf[0].walk = 0xF0;
		combobuf[0].type = 0;
		combobuf[0].flag = 0;
	}
	
	setup_combo_animations();
	setup_combo_animations2();
	return 0;
}
