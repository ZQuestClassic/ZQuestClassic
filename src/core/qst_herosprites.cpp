#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

namespace {

void setSprite(int32_t* arr, int32_t tile, int32_t flip, int32_t ext)
{
	arr[spr_tile] = tile;
	arr[spr_flip] = (flip > 3 ? 0 : flip);
	arr[spr_extend] = (ext > 2 ? 0 : ext);
}

//Used to read the player sprites as int32_t, not word.
int32_t readherosprites3(PACKFILE *f, int32_t v_herosprites)
{
	zinit.hero_swim_speed=67; //default
	setupherotiles(zinit.heroAnimationStyle);
	setupherodefenses();
	setupherooffsets();
	
	int32_t tile, tile2;
	byte flip, extend, dummy_byte;
	
	if(v_herosprites>=0)
	{
		
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetl(&tile,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&flip,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&extend,f))
			{
				return qe_invalid;
			}
			
			setSprite(walkspr[i], int32_t(tile), int32_t(flip), int32_t(extend));
		}
		
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetl(&tile,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&flip,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&extend,f))
			{
				return qe_invalid;
			}
			
			setSprite(stabspr[i], int32_t(tile), int32_t(flip), int32_t(extend));
		}
		
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetl(&tile,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&flip,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&extend,f))
			{
				return qe_invalid;
			}
			
			setSprite(slashspr[i], int32_t(tile), int32_t(flip), int32_t(extend));
		}
		
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetl(&tile,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&flip,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&extend,f))
			{
				return qe_invalid;
			}
			
			setSprite(floatspr[i], int32_t(tile), int32_t(flip), int32_t(extend));
		}
		
		if(v_herosprites>1)
		{
			for(int32_t i=0; i<4; i++)
			{
				if(!p_igetl(&tile,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&flip,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&extend,f))
				{
					return qe_invalid;
				}
				
				setSprite(swimspr[i], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetl(&tile,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&flip,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&extend,f))
			{
				return qe_invalid;
			}
			
			setSprite(divespr[i], int32_t(tile), int32_t(flip), int32_t(extend));
		}
		
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetl(&tile,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&flip,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&extend,f))
			{
				return qe_invalid;
			}
			
			setSprite(poundspr[i], int32_t(tile), int32_t(flip), int32_t(extend));
		}
		
		if(!p_igetl(&tile,f))
		{
			return qe_invalid;
		}
		
		flip=0;
		
		if(v_herosprites>0)
		{
			if(!p_getc(&flip,f))
			{
				return qe_invalid;
			}
		}
		
		if(!p_getc(&extend,f))
		{
			return qe_invalid;
		}
		
		setSprite(castingspr, int32_t(tile), int32_t(flip), int32_t(extend));
		
		if(v_herosprites>0)
		{
			int32_t num_holdsprs = (v_herosprites > 6 ? 3 : 2);
			for(int32_t i=0; i<2; i++)
			{
				for(int32_t j=0; j<num_holdsprs; j++)
				{
					if(!p_igetl(&tile,f))
					{
						return qe_invalid;
					}
					
					if(!p_getc(&flip,f))
					{
						return qe_invalid;
					}
					
					if(!p_getc(&extend,f))
					{
						return qe_invalid;
					}
					
					setSprite(holdspr[i][j], int32_t(tile), int32_t(flip), int32_t(extend));
				}
			}
		}
		else
		{
			for(int32_t i=0; i<2; i++)
			{
				if(!p_igetl(&tile,f))
				{
					return qe_invalid;
				}
				
				if(!p_igetl(&tile2,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&extend,f))
				{
					return qe_invalid;
				}
				
				setSprite(holdspr[i][spr_hold1], int32_t(tile), int32_t(flip), int32_t(extend));
				setSprite(holdspr[i][spr_hold2], int32_t(tile2), int32_t(flip), int32_t(extend));
			}
		}
		
		if(v_herosprites>2)
		{
			for(int32_t i=0; i<4; i++)
			{
				if(!p_igetl(&tile,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&flip,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&extend,f))
				{
					return qe_invalid;
				}
				
				setSprite(jumpspr[i], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		
		if(v_herosprites>3)
		{
			for(int32_t i=0; i<4; i++)
			{
				if(!p_igetl(&tile,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&flip,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&extend,f))
				{
					return qe_invalid;
				}
				
				setSprite(chargespr[i], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		
		if(v_herosprites>4)
		{
			if(!p_getc(&dummy_byte,f))
			{
				return qe_invalid;
			}
			
			zinit.hero_swim_speed=(byte)dummy_byte;
		}
		
		if(v_herosprites>6)
		{
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(frozenspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(frozen_waterspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(onfirespr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(onfire_waterspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(diggingspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(usingrodspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(usingcanespr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(pushingspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				byte frames = 0;
				if(v_herosprites > 15)
				{
					if(!p_getc(&frames,f))
						return qe_invalid;
				}
				
				setSprite(liftingspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
				liftingspr[q][spr_frames] = frames;
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(liftingwalkspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(stunnedspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(stunned_waterspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(drowningspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(drowning_lavaspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(fallingspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(shockedspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(shocked_waterspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(pullswordspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(readingspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(slash180spr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(slashZ4spr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(dashspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(bonkspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			
			for(int32_t q = 0; q < 3; ++q) //Not directions; number of medallion sprs
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(medallionsprs[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		else
		{
			memset(frozenspr, 0, sizeof(frozenspr));
			memset(frozen_waterspr, 0, sizeof(frozen_waterspr));
			memset(onfirespr, 0, sizeof(onfirespr));
			memset(onfire_waterspr, 0, sizeof(onfire_waterspr));
			memset(diggingspr, 0, sizeof(diggingspr));
			memset(usingrodspr, 0, sizeof(usingrodspr));
			memset(usingcanespr, 0, sizeof(usingcanespr));
			memset(pushingspr, 0, sizeof(pushingspr));
			memset(liftingspr, 0, sizeof(liftingspr));
			memset(liftingwalkspr, 0, sizeof(liftingwalkspr));
			memset(stunnedspr, 0, sizeof(stunnedspr));
			memset(stunned_waterspr, 0, sizeof(stunned_waterspr));
			memset(fallingspr, 0, sizeof(fallingspr));
			memset(shockedspr, 0, sizeof(shockedspr));
			memset(shocked_waterspr, 0, sizeof(shocked_waterspr));
			memset(pullswordspr, 0, sizeof(pullswordspr));
			memset(readingspr, 0, sizeof(readingspr));
			memset(slash180spr, 0, sizeof(slash180spr));
			memset(slashZ4spr, 0, sizeof(slashZ4spr));
			memset(dashspr, 0, sizeof(dashspr));
			memset(bonkspr, 0, sizeof(bonkspr));
			memset(medallionsprs, 0, sizeof(medallionsprs));
			memset(holdspr[0][2], 0, sizeof(holdspr[0][2])); //Sword hold (Land)
			memset(holdspr[1][2], 0, sizeof(holdspr[1][2])); //Sword hold (Water)
			for(int32_t q = 0; q < 4; ++q)
			{
				for(int32_t p = 0; p < 3; ++p)
				{
					drowningspr[q][p] = divespr[q][p];
					drowning_lavaspr[q][p] = divespr[q][p];
				}
			}
		}
		if (v_herosprites > 8)
		{
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(sideswimspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		else
		{
			memset(sideswimspr, 0, sizeof(sideswimspr));
		}
		if (v_herosprites > 9)
		{
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(sideswimslashspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(sideswimstabspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(sideswimpoundspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(sideswimchargespr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		else
		{
			memset(sideswimslashspr, 0, sizeof(sideswimslashspr));
			memset(sideswimstabspr, 0, sizeof(sideswimstabspr));
			memset(sideswimpoundspr, 0, sizeof(sideswimpoundspr));
			memset(sideswimchargespr, 0, sizeof(sideswimchargespr));
		}
		if (v_herosprites > 10)
		{
			for(int32_t q = 0; q < 4; ++q)
			{
				int32_t hmr;
				if(!p_igetl(&hmr,f))
					return qe_invalid;
				
				hammeroffsets[q] = hmr;
			}
		}
		else
		{
			for(int32_t q = 0; q < 4; ++q) hammeroffsets[q] = 0;
		}
		if (v_herosprites > 11)
		{
			for(int32_t q = 0; q < 3; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(sideswimholdspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		else
		{
			memset(sideswimholdspr, 0, sizeof(sideswimholdspr));
		}
		if (v_herosprites > 12)
		{
			if(!p_igetl(&tile,f))
				return qe_invalid;
			
			if(!p_getc(&flip,f))
				return qe_invalid;
			
			if(!p_getc(&extend,f))
				return qe_invalid;
			setSprite(sideswimcastingspr, int32_t(tile), int32_t(flip), int32_t(extend));
			
		}
		else
		{
			memset(sideswimcastingspr, 0, sizeof(sideswimcastingspr));
		}
		if (v_herosprites > 13)
		{
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(sidedrowningspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		else
		{
			memset(sidedrowningspr, 0, sizeof(sidedrowningspr));
		}
		if (v_herosprites > 14)
		{
			for(int32_t q = 0; q < 4; ++q)
			{
				if(!p_igetl(&tile,f))
					return qe_invalid;
				
				if(!p_getc(&flip,f))
					return qe_invalid;
				
				if(!p_getc(&extend,f))
					return qe_invalid;
				
				setSprite(revslashspr[q], int32_t(tile), int32_t(flip), int32_t(extend));
			}
		}
		else
		{
			memset(revslashspr, 0, sizeof(revslashspr));
		}
		if (v_herosprites > 7)
		{
			int32_t num_defense = wMax;
			byte def = 0;

			//Set num_defense accordingly if changes to enum require version upgrade - Jman
			/*if(v_herosprites > [x])
			* {
			*	 num_defense = 146 //value of wMax on version 8
			* }
			*/

			for (int32_t q = 0; q < num_defense; q++)
			{
				if (!p_getc(&def, f))
					return qe_invalid;

				hero_defenses[q] = def;
			}
		}
		else
		{
			int32_t num_defense = wMax;
			for (int32_t q = 0; q < num_defense; q++)
			{
				hero_defenses[q] = 0;
			}
		}
	}
	
	if(FFCore.quest_format[vInitData] < 34)
	{
		bool fastswim = zinit.hero_swim_speed > 60;
		// '2/3' or '1/2'
		zinit.hero_swim_mult = fastswim ? 2 : 1;
		zinit.hero_swim_div = fastswim ? 3 : 2;
	}
	return 0;
}

} // end namespace

int32_t readherosprites2(PACKFILE *f, int32_t v_herosprites)
{
    bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_herosprites);
    if (should_skip) return 0;

	assert(v_herosprites < 6);
	
	zinit.hero_swim_speed=67; //default
	setupherotiles(zinit.heroAnimationStyle);
	setupherodefenses();
	setupherooffsets();
    
    if(v_herosprites>=0)
    {
        word tile, tile2;
        byte flip, extend, dummy_byte;
        
        for(int32_t i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f))
            {
                return qe_invalid;
            }
            
			walkspr[i][spr_tile]=(int32_t)tile;
			walkspr[i][spr_flip]=(int32_t)flip;
			walkspr[i][spr_extend]=(int32_t)extend;
        }
        
        for(int32_t i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f))
            {
                return qe_invalid;
            }
            
			stabspr[i][spr_tile]=(int32_t)tile;
			stabspr[i][spr_flip]=(int32_t)flip;
			stabspr[i][spr_extend]=(int32_t)extend;
        }
        
        for(int32_t i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f))
            {
                return qe_invalid;
            }
            
			slashspr[i][spr_tile]=(int32_t)tile;
			slashspr[i][spr_flip]=(int32_t)flip;
			slashspr[i][spr_extend]=(int32_t)extend;
        }
        
        for(int32_t i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f))
            {
                return qe_invalid;
            }
            
			floatspr[i][spr_tile]=(int32_t)tile;
			floatspr[i][spr_flip]=(int32_t)flip;
			floatspr[i][spr_extend]=(int32_t)extend;
        }
        
        if(v_herosprites>1)
        {
            for(int32_t i=0; i<4; i++)
            {
                if(!p_igetw(&tile,f))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&flip,f))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f))
                {
                    return qe_invalid;
                }
                
				swimspr[i][spr_tile]=(int32_t)tile;
				swimspr[i][spr_flip]=(int32_t)flip;
				swimspr[i][spr_extend]=(int32_t)extend;
            }
        }
        
        for(int32_t i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f))
            {
                return qe_invalid;
            }
            
			divespr[i][spr_tile]=(int32_t)tile;
			divespr[i][spr_flip]=(int32_t)flip;
			divespr[i][spr_extend]=(int32_t)extend;
        }
        
        for(int32_t i=0; i<4; i++)
        {
            if(!p_igetw(&tile,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&flip,f))
            {
                return qe_invalid;
            }
            
            if(!p_getc(&extend,f))
            {
                return qe_invalid;
            }
            
			poundspr[i][spr_tile]=(int32_t)tile;
			poundspr[i][spr_flip]=(int32_t)flip;
			poundspr[i][spr_extend]=(int32_t)extend;
        }
        
        if(!p_igetw(&tile,f))
        {
            return qe_invalid;
        }
        
        flip=0;
        
        if(v_herosprites>0)
        {
            if(!p_getc(&flip,f))
            {
                return qe_invalid;
            }
        }
        
        if(!p_getc(&extend,f))
        {
            return qe_invalid;
        }
        
		castingspr[spr_tile]=(int32_t)tile;
		castingspr[spr_flip]=(int32_t)flip;
		castingspr[spr_extend]=(int32_t)extend;
        
        if(v_herosprites>0)
        {
			int32_t num_holdsprs = (v_herosprites > 6 ? 3 : 2);
            for(int32_t i=0; i<2; i++)
            {
                for(int32_t j=0; j<num_holdsprs; j++)
                {
                    if(!p_igetw(&tile,f))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&flip,f))
                    {
                        return qe_invalid;
                    }
                    
                    if(!p_getc(&extend,f))
                    {
                        return qe_invalid;
                    }
                    
					holdspr[i][j][spr_tile]=(int32_t)tile;
					holdspr[i][j][spr_flip]=(int32_t)flip;
					holdspr[i][j][spr_extend]=(int32_t)extend;
                }
            }
        }
        else
        {
            for(int32_t i=0; i<2; i++)
            {
                if(!p_igetw(&tile,f))
                {
                    return qe_invalid;
                }
                
                if(!p_igetw(&tile2,f))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f))
                {
                    return qe_invalid;
                }
                
				holdspr[i][spr_hold1][spr_tile]=(int32_t)tile;
				holdspr[i][spr_hold1][spr_flip]=(int32_t)flip;
				holdspr[i][spr_hold1][spr_extend]=(int32_t)extend;
				holdspr[i][spr_hold2][spr_tile]=(int32_t)tile2;
				holdspr[i][spr_hold2][spr_flip]=(int32_t)flip;
				holdspr[i][spr_hold2][spr_extend]=(int32_t)extend;
            }
        }
        
        if(v_herosprites>2)
        {
            for(int32_t i=0; i<4; i++)
            {
                if(!p_igetw(&tile,f))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&flip,f))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f))
                {
                    return qe_invalid;
                }
                
				jumpspr[i][spr_tile]=(int32_t)tile;
				jumpspr[i][spr_flip]=(int32_t)flip;
				jumpspr[i][spr_extend]=(int32_t)extend;
            }
        }
        
        if(v_herosprites>3)
        {
            for(int32_t i=0; i<4; i++)
            {
                if(!p_igetw(&tile,f))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&flip,f))
                {
                    return qe_invalid;
                }
                
                if(!p_getc(&extend,f))
                {
                    return qe_invalid;
                }
                
				chargespr[i][spr_tile]=(int32_t)tile;
				chargespr[i][spr_flip]=(int32_t)flip;
				chargespr[i][spr_extend]=(int32_t)extend;
            }
        }
        
        if(v_herosprites>4)
        {
            if(!p_getc(&dummy_byte,f))
            {
                return qe_invalid;
            }
            
			zinit.hero_swim_speed=(byte)dummy_byte;
        }
		
		memset(frozenspr, 0, sizeof(frozenspr));
		memset(frozen_waterspr, 0, sizeof(frozen_waterspr));
		memset(onfirespr, 0, sizeof(onfirespr));
		memset(onfire_waterspr, 0, sizeof(onfire_waterspr));
		memset(diggingspr, 0, sizeof(diggingspr));
		memset(usingrodspr, 0, sizeof(usingrodspr));
		memset(usingcanespr, 0, sizeof(usingcanespr));
		memset(pushingspr, 0, sizeof(pushingspr));
		memset(liftingspr, 0, sizeof(liftingspr));
		memset(liftingwalkspr, 0, sizeof(liftingwalkspr));
		memset(stunnedspr, 0, sizeof(stunnedspr));
		memset(stunned_waterspr, 0, sizeof(stunned_waterspr));
		memset(fallingspr, 0, sizeof(fallingspr));
		memset(shockedspr, 0, sizeof(shockedspr));
		memset(shocked_waterspr, 0, sizeof(shocked_waterspr));
		memset(pullswordspr, 0, sizeof(pullswordspr));
		memset(readingspr, 0, sizeof(readingspr));
		memset(slash180spr, 0, sizeof(slash180spr));
		memset(slashZ4spr, 0, sizeof(slashZ4spr));
		memset(dashspr, 0, sizeof(dashspr));
		memset(bonkspr, 0, sizeof(bonkspr));
		memset(medallionsprs, 0, sizeof(medallionsprs));
		memset(holdspr[0][2], 0, sizeof(holdspr[0][2])); //Sword hold (Land)
		memset(holdspr[1][2], 0, sizeof(holdspr[1][2])); //Sword hold (Water)
		for(int32_t q = 0; q < 4; ++q)
		{
			for(int32_t p = 0; p < 3; ++p)
			{
				drowningspr[q][p] = divespr[q][p];
				drowning_lavaspr[q][p] = divespr[q][p];
			}
		}
		memset(sideswimspr, 0, sizeof(sideswimspr));
		memset(sideswimslashspr, 0, sizeof(sideswimslashspr));
		memset(sideswimstabspr, 0, sizeof(sideswimstabspr));
		memset(sideswimpoundspr, 0, sizeof(sideswimpoundspr));
		memset(sideswimchargespr, 0, sizeof(sideswimchargespr));
		memset(sideswimholdspr, 0, sizeof(sideswimholdspr));
		memset(sidedrowningspr, 0, sizeof(sidedrowningspr));
    }
    
	if(FFCore.quest_format[vInitData] < 34)
	{
		bool fastswim = zinit.hero_swim_speed > 60;
		// '2/3' or '1/2'
		zinit.hero_swim_mult = fastswim ? 2 : 1;
		zinit.hero_swim_div = fastswim ? 3 : 2;
	}
    return 0;
}

int32_t readherosprites(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword dummy;
    word s_version=0;
    
    //section version info
    if(!p_igetw(&s_version,f))
    {
        return qe_invalid;
    }

	if (s_version > V_HEROSPRITES)
			return qe_version;
    
    FFCore.quest_format[vHeroSprites] = s_version;
    
    if(!read_deprecated_section_cversion(f))
    {
        return qe_invalid;
    }
    
    //section size
    if(!p_igetl(&dummy,f))
    {
        return qe_invalid;
    }
    if ( s_version >= 6 ) 
    {
	    return readherosprites3(f, s_version);
    }
    else return readherosprites2(f, s_version);
}
