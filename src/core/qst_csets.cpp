#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

namespace {

bool init_palnames()
{
    // if(palnames==NULL)
        // return false;
        
    for(int32_t x=0; x<MAXLEVELS; x++)
    {
        switch(x)
        {
        case 0:
            sprintf(palnames[x],"Overworld");
            break;
            
        case 10:
            sprintf(palnames[x],"Caves");
            break;
            
        case 11:
            sprintf(palnames[x],"Passageways");
            break;
            
        default:
            sprintf(palnames[x],"%c",0);
            break;
        }
    }
    
    return true;
}

} // end namespace

int32_t readcolordata(PACKFILE *f, miscQdata *Misc, word version, word build, word start_cset, word max_csets)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_colors);

	//these are here to bypass compiler warnings about unused arguments
	//THE *48 REFERS TO EACH CSET BEING 16 COLORS with 3 VALUES OF RGB (3*16 is 48)
	//Capitalized cause it'll save you a headache. -Deedee
	start_cset=start_cset;
	max_csets=max_csets;
	word s_version=0;
	
	miscQdata temp_misc;
	temp_misc = *Misc;
	
	byte temp_colordata[48];
	char temp_palname[PALNAMESIZE+1];
	
	int32_t dummy;
	word palcycles;
	
	if(version > 0x192)
	{
		//section version info
		if(!p_igetw(&s_version,f))
		{
			return qe_invalid;
		}

		if (s_version > V_CSETS)
			return qe_version;
	
		FFCore.quest_format[vCSets] = s_version;
		
		if(!p_igetw(&dummy,f))
		{
			return qe_invalid;
		}
		
		//section size
		if(!p_igetl(&dummy,f))
		{
			return qe_invalid;
		}
	}
	if (s_version < 5)
	{
		bool RealOldVerion = ((version < 0x192)||((version == 0x192)&&(build<73)));
		
		//finally...  section data
		int32_t q = 0;
		int32_t p = -15;
		for(int32_t i=0; i<oldpdTOTAL; ++i)
		{
			memset(temp_colordata, 0, 48);
			
			if(!pfread(temp_colordata,48,f))
			{
				return qe_invalid;
			}

			if (should_skip)
				continue;

			memcpy(&colordata[q*48], temp_colordata, 48);

			++q;
			if (p > 0 && (p%13)==12 && (i < oldpoSPRITE || !RealOldVerion)) //It's > 0 instead of >= 0 because it should append 
			{
				if (s_version < 5) //Bumping up the size of level palettes
				{
					memcpy(&colordata[(q)*48], &colordata[1*48], 48);
					memcpy(&colordata[(q+1)*48], &colordata[5*48], 48);
					memcpy(&colordata[(q+2)*48], &colordata[7*48], 48);
					memcpy(&colordata[(q+3)*48], &colordata[8*48], 48);
					q+=4;
				}
				else
				{
					for(int m = 0; m < 4; ++m)
					{
						memset(temp_colordata, 0, 48);
						if(!pfread(temp_colordata,48,f))
						{
							return qe_invalid;
						}
						memcpy(&colordata[q*48], temp_colordata, 48);
						++q;
					}
				}
			}
			++p;
		}
		
		if(RealOldVerion)
		{
			if (!should_skip)
			{
				memcpy(colordata+(poSPRITE255*48), colordata+((q-30)*48), 30*16*3);
				memset(colordata+((q-30)*48), 0, ((poSPRITE255-(q-30))*48));
				memcpy(colordata+((poSPRITE255+11)*48), colordata+((poSPRITE255+10)*48), 48);
				memcpy(colordata+((poSPRITE255+10)*48), colordata+((poSPRITE255+9)*48), 48);
				memcpy(colordata+((poSPRITE255+9)*48), colordata+((poSPRITE255+8)*48), 48);
				memset(colordata+((poSPRITE255+8)*48), 0, 48);
			}
		}
		else
		{
			memset(temp_colordata, 0, 48);
			
			for(int32_t i=0; i<newpdTOTAL-oldpdTOTAL; ++i)
			{
				if(!pfread(temp_colordata,48,f))
				{
					return qe_invalid;
				}

				if (should_skip)
					continue;

				memcpy(&colordata[q*48], temp_colordata, 48);

				++q;
				if (p > 0 && (p%13)==12 && (i < (newpoSPRITE-oldpdTOTAL) || (s_version >= 4))) //It's > 0 instead of >= 0 because it should append 
				{
					if (s_version < 5) //Bumping up the size of level palettes
					{
						memcpy(&colordata[(q)*48], &colordata[1*48], 48);
						memcpy(&colordata[(q+1)*48], &colordata[5*48], 48);
						memcpy(&colordata[(q+2)*48], &colordata[7*48], 48);
						memcpy(&colordata[(q+3)*48], &colordata[8*48], 48);
						q+=4;
					}
					else
					{
						for(int m = 0; m < 4; ++m)
						{
							memset(temp_colordata, 0, 48);
							if(!pfread(temp_colordata,48,f))
							{
								return qe_invalid;
							}
							memcpy(&colordata[q*48], temp_colordata, 48);
							++q;
						}
					}
				}
				++p;
			}
			
			if(s_version < 4)
			{
				if (!should_skip)
				{
					memcpy(colordata+(poSPRITE255*48), colordata+((q-30)*48), 30*16*3);
					memset(colordata+((q-30)*48), 0, ((poSPRITE255-(q-30))*48));
				}
			}
			else
			{
				for(int32_t i=0; i<newerpdTOTAL-newpdTOTAL; ++i)
				{
					if(!pfread(temp_colordata,48,f))
					{
						return qe_invalid;
					}

					if (should_skip)
						continue;
					
					memcpy(&colordata[q*48], temp_colordata, 48);
					++q;
					if (p > 0 && (p%13)==12 && i < newerpoSPRITE-newpdTOTAL) //It's > 0 instead of >= 0 because it should append 
					{
						if (s_version < 5) //Bumping up the size of level palettes
						{
							memcpy(&colordata[(q)*48], &colordata[1*48], 48);
							memcpy(&colordata[(q+1)*48], &colordata[5*48], 48);
							memcpy(&colordata[(q+2)*48], &colordata[7*48], 48);
							memcpy(&colordata[(q+3)*48], &colordata[8*48], 48);
							q+=4;
						}
						else
						{
							for(int m = 0; m < 4; ++m)
							{
								memset(temp_colordata, 0, 48);
								if(!pfread(temp_colordata,48,f))
								{
									return qe_invalid;
								}
								memcpy(&colordata[q*48], temp_colordata, 48);
								++q;
							}
						}
					}
					++p;
				}
				
				//By this point, q should be about equal to pdTOTAL255. If it isn't, I've fucked up. -Deedee
			}
		}
	}
	else
	{
		for(int32_t i=0; i<pdTOTAL255; ++i)
		{
			memset(temp_colordata, 0, 48);
			
			if(!pfread(temp_colordata,48,f))
			{
				return qe_invalid;
			}
			
			memcpy(&colordata[i*48], temp_colordata, 48);
		}
	}

	if (!should_skip && s_version < 6)
	{
		for (int i = 0; i < psTOTAL255; i++)
		{
			colordata[i] = _rgb_scale_6[colordata[i]];
		}
	}

	if((version < 0x192)||((version == 0x192)&&(build<76)))
	{
		if (!should_skip)
			init_palnames();
	}
	else
	{
		int32_t palnamestoread = 0;
		
		if(s_version < 3)
			palnamestoread = OLDMAXLEVELS;
		else
			palnamestoread = 512;
			
		for(int32_t i=0; i<palnamestoread; ++i)
		{
			if(!p_getstr(temp_palname,PALNAMESIZE,f))
			{
				return qe_invalid;
			}

			if (!should_skip)
				memcpy(palnames[i], temp_palname, PALNAMESIZE);
		}

		if (should_skip)
			return 0;
		
		for(int32_t i=palnamestoread; i<MAXLEVELS; i++)
		{
			memset(palnames[i], 0, PALNAMESIZE);
		}
	}
	
	if(version > 0x192)
	{
		for(int32_t i=0; i<256; i++)
		{
			for(int32_t j=0; j<3; j++)
			{
				temp_misc.cycles[i][j].first=0;
				temp_misc.cycles[i][j].count=0;
				temp_misc.cycles[i][j].speed=0;
			}
		}
		
		if(!p_igetw(&palcycles,f))
		{
			return qe_invalid;
		}

		if (palcycles > NUM_PAL_CYCLES)
		{
			return qe_invalid;
		}
		
		for(int32_t i=0; i<palcycles; i++)
		{
			for(int32_t j=0; j<3; j++)
			{
				if(!p_getc(&temp_misc.cycles[i][j].first,f))
				{
					return qe_invalid;
				}
			}
			
			for(int32_t j=0; j<3; j++)
			{
				if(!p_getc(&temp_misc.cycles[i][j].count,f))
				{
					return qe_invalid;
				}
			}
			
			for(int32_t j=0; j<3; j++)
			{
				if(!p_getc(&temp_misc.cycles[i][j].speed,f))
				{
					return qe_invalid;
				}
			}
		}
		
		*Misc = temp_misc;
	}
	
	return 0;
}
