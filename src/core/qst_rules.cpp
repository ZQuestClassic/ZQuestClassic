#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern byte deprecated_rules[QUESTRULES_NEW_SIZE];

int32_t readrules(PACKFILE *f, zquestheader *Header)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_rules);
	if (should_skip)
		return 0;

	int32_t dummy;
	zquestheader tempheader = *Header;
	word s_version = 0;
	dword compatrule_version = 0;
	dword compile_setting_version = 0;
	
	qst_compiler_settings.clear();
	
	if(tempheader.zelda_version >= 0x193)
	{
		//section version info
		if(!p_igetw(&s_version,f))
		{
			return qe_invalid;
		}

		if (s_version > V_RULES)
			return qe_version;
	
		FFCore.quest_format[vRules] = s_version;
		
		if(!p_igetw(&dummy,f))
		{
			return qe_invalid;
		}
		
		if(s_version > 16)
		{
			if(!p_igetl(&compatrule_version,f))
			{
				return qe_invalid;
			}
		}
		FFCore.quest_format[vCompatRule] = compatrule_version;
		
		//section size
		if(!p_igetl(&dummy,f))
		{
			return qe_invalid;
		}
		
		if ( s_version < 15 )
		{
			//finally...  section data
			if(!pfread(quest_rules,QUESTRULES_SIZE,f))
			{
				return qe_invalid;
			}
		}
		else
		{
			
			if(!pfread(quest_rules,QUESTRULES_NEW_SIZE,f))
			{
				return qe_invalid;
			}
			
		}
		
		if (s_version > 17)
			if (!p_igetl(&compile_setting_version, f))
				return qe_invalid;
		FFCore.quest_format[vCompileSetting] = compile_setting_version;
		
		if (compile_setting_version > 0)
		{
			word sz;
			if (!p_igetw(&sz, f))
				return qe_invalid;
			word id;
			int val;
			for (word q = 0; q < sz; ++q)
			{
				if (!p_igetw(&id, f))
					return qe_invalid;
				if (!p_igetl(&val, f))
					return qe_invalid;
				qst_compiler_settings[(QSTCompilerSetting)id] = val;
			}
		}
	}

	//{ bunch of compat stuff
	memcpy(deprecated_rules, quest_rules, QUESTRULES_NEW_SIZE);

	unpack_qrs();
	
	if(s_version<2)
	{
		set_qr(14,0);
		set_qr(27,0);
		set_qr(28,0);
		set_qr(29,0);
		set_qr(30,0);
		set_qr(32,0);
		set_qr(36,0);
		set_qr(49,0);
		set_qr(50,0);
		set_qr(51,0);
		set_qr(68,0);
		set_qr(75,0);
		set_qr(76,0);
		set_qr(98,0);
		set_qr(110,0);
		set_qr(113,0);
		set_qr(116,0);
		set_qr(102,0);
		set_qr(132,0);
	}
	
	//Now, do any updates...
	if((tempheader.zelda_version < 0x211)||((tempheader.zelda_version == 0x211)&&(tempheader.build<18)))
	{
		set_qr(qr_SMOOTHVERTICALSCROLLING,1);
		set_qr(qr_REPLACEOPENDOORS, 1);
		set_qr(qr_OLDLENSORDER, 1);
		set_qr(qr_NOFAIRYGUYFIRES, 1);
		set_qr(qr_TRIGGERSREPEAT, 1);
	}
	
	if((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<3)))
	{
		set_qr(qr_WALLFLIERS,1);
	}
	
	if((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<4)))
	{
		set_qr(qr_NOBOMBPALFLASH,1);
	}
	
	if((tempheader.zelda_version < 0x193)||((tempheader.zelda_version == 0x193)&&(tempheader.build<3)))
	{
		set_qr(qr_NOSCROLLCONTINUE,1);
	}
	
	if(tempheader.zelda_version <= 0x210)
	{
		set_qr(qr_ARROWCLIP,1);
	}
	
	if(tempheader.zelda_version == 0x210)
	{
		set_qr(qr_NOSCROLLCONTINUE, get_qr(qr_CMBCYCLELAYERS));
		set_qr(qr_CMBCYCLELAYERS, 0);
		set_qr(qr_CONT_SWORD_TRIGGERS, 1);
	}
	
	if(tempheader.zelda_version <= 0x210)
	{
		set_qr(qr_OLDSTYLEWARP,1);
		set_qr(qr_210_WARPRETURN,1);
	}
	
	//might not be correct
	if(tempheader.zelda_version < 0x210)
	{
		set_bit(deprecated_rules, qr_OLDTRIBBLES_DEP,1);
		set_qr(qr_OLDTRIBBLES_DEP,1);
		set_qr(qr_OLDHOOKSHOTGRAB,1);
	}
	
	if(tempheader.zelda_version < 0x211)
	{
		set_qr(qr_WRONG_BRANG_TRAIL_DIR,1);
	}
	
	if((tempheader.zelda_version == 0x192 && tempheader.build <= 163) || tempheader.zelda_version < 0x192)
	{
		set_qr(qr_192b163_WARP,1);
	}
	
	if(tempheader.zelda_version == 0x210)
	{
		set_bit(deprecated_rules, qr_OLDTRIBBLES_DEP, get_qr(qr_DMGCOMBOPRI));
		set_qr(qr_OLDTRIBBLES_DEP, get_qr(qr_DMGCOMBOPRI));
		set_qr(qr_DMGCOMBOPRI, 0);
	}
	
	if(tempheader.zelda_version < 0x211 || (tempheader.zelda_version == 0x211 && tempheader.build<15))
	{
		set_qr(qr_OLDPICKUP,1);
	}
	
	if(tempheader.zelda_version < 0x211 || (tempheader.zelda_version == 0x211 && tempheader.build < 18))
	{
		set_qr(qr_NOSOLIDDAMAGECOMBOS, 1);
		set_qr(qr_ITEMPICKUPSETSBELOW, 1); // broke around build 400
	}
	
	if(tempheader.zelda_version < 0x250) // version<0x250 checks for beta 18; build was set to 18 prematurely
	{
		set_qr(qr_HOOKSHOTDOWNBUG, 1);
	}
	
	if(tempheader.zelda_version == 0x250 && tempheader.build == 24) // Annoying...
	{
		set_qr(qr_PEAHATCLOCKVULN, 1);
	}
	
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build < 22)) //22 is 2.50.0 RC4. Gotta set the door repair QR... -Dimi
	{
		set_qr(qr_OLD_DOORREPAIR, 1);
	}
	
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build < 20)) //20 is 2.50.0 RC1 and RC2 (cause it didn't get bumped). Okay I'm gonna be honest I have no idea if any 2.50 build was available before RC1, but gonna try and cover my ass here -Dimi
	{
		set_qr(qr_OLD_SECRETMONEY, 1);
	}
	
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build < 28)) //28 is 2.50.1 final. Potion bug might have been used, I dunno. -Dimi
	{
		set_qr(qr_OLD_POTION_OR_HC, 1);
	}
	
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<28))
	{
		set_qr(qr_OFFSCREENWEAPONS, 1);
	}
	
	//Bombchu fix.
	if(tempheader.zelda_version == 0x250)
	{
		if ( tempheader.build == 24 ) //2.50.0
		{
			set_qr(qr_BOMBCHUSUPERBOMB, 1);
		}
		if ( tempheader.build == 28 ) //2.50.1
		{
			set_qr(qr_BOMBCHUSUPERBOMB, 1);
		}
		if ( tempheader.build == 29 ) //2.50.2
		{
			set_qr(qr_BOMBCHUSUPERBOMB, 0);
		}
		if ( tempheader.build == 30 ) //2.50.3RC1
		{
			set_qr(qr_BOMBCHUSUPERBOMB, 0);
		}
	}
	
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<29))
	{
		// qr_OFFSETEWPNCOLLISIONFIX
		// All 'official' quests need this disabled.
		// All 2.10 and lower quests need this enabled to preseve compatability.
		// All 2.11 - 2.5.1 quests should have it set also, due to a bug in about half of all the betas.

		//~Gleeok
		set_qr(qr_OFFSETEWPNCOLLISIONFIX, 1); //This has to be set!!!!
		
		// Broke in build 695
		if(tempheader.zelda_version>=0x211 && tempheader.build>=18)
			set_qr(qr_BROKENSTATUES, 1);
	}
	if (tempheader.zelda_version <= 0x190 || (tempheader.zelda_version == 0x192 && std::string(tempheader.title).starts_with("Zelda 3000\0")))
	{
		set_qr(qr_COPIED_SWIM_SPRITES, 1);
	}
	if ( (tempheader.zelda_version == 0x250 && tempheader.build < 33) || tempheader.zelda_version == 0x254 || tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x255 && tempheader.build < 50) )
	{
		set_qr(qr_OLD_SLASHNEXT_SECRETS, 1);
	}
	
	if ( (tempheader.zelda_version < 0x211) ) //2.10 water and ladder interaction
	{
		set_qr(qr_OLD_210_WATER, 1);
	}
	
	if ( (tempheader.zelda_version < 0x255 ) || (tempheader.zelda_version == 0x255 &&  tempheader.build < 51 ) ) //2.10 water and ladder interaction
	{
		set_qr(qr_STEP_IS_FLOAT,0);
	}
	
	if ( tempheader.zelda_version < 0x250 ) 
	{
		set_qr(qr_8WAY_SHOT_SFX_DEP, 1);		
	}
	
	if(s_version < 3)
	{
		set_qr(qr_HOLDNOSTOPMUSIC, 1);
		set_qr(qr_CAVEEXITNOSTOPMUSIC, 1);
	}
	
	if(s_version<4)
	{
		set_qr(10,0);
	}
	
	if(s_version<5)
	{
		set_qr(27,0);
	}
	
	if(s_version<6)
	{
		set_qr(46,0);
	}
	
	if(s_version<7) // January 2008
	{
		set_qr(qr_HEARTSREQUIREDFIX,0);
		set_qr(qr_PUSHBLOCKCSETFIX,1);
	}
	
	if(s_version<8)
	{
		set_qr(12, 0);
	}
	else
	{
		set_bit(deprecated_rules, 12, 0);
	}
	
	if(s_version<9) // October 2008
	{
		set_qr(qr_NOROPE2FLASH_DEP,0);
		set_qr(qr_NOBUBBLEFLASH_DEP,0);
		set_qr(qr_GHINI2BLINK_DEP,0);
		set_qr(qr_PHANTOMGHINI2_DEP,0);
	}
	
	if(s_version<10) // December 2008
	{
		set_qr(qr_NOCLOCKS_DEP,0);
		set_qr(qr_ALLOW10RUPEEDROPS_DEP,0);
	}
	
	if(s_version<11) // April 2009
	{
		set_qr(qr_SLOWENEMYANIM_DEP,0);
	}
	
	// This served no purpose.
	// if(s_version<12)  // December 2009
	// {
	// 	set_qr(qr_BRKBLSHLDS_DEP,0);
	// 	set_qr(qr_OLDTRIBBLES_DEP,0);
	// }
	
	//if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build < 24))
	if(s_version < 13)
	{
		set_qr(qr_SHOPCHEAT, 1);
	}
	
	// Not entirely sure this is the best place for this...
	//2.50.2 bitmap offset fix
	memset(extra_rules, 0, EXTRARULES_SIZE);
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<29))
	{
		set_er(er_BITMAPOFFSET, 1);
		set_qr(qr_BITMAPOFFSETFIX, 1);
	}
	//required because quest templates also used this bit, although
	//it never did anything, before. -Z
	if ( tempheader.zelda_version == 0x250 )
	{
		if( tempheader.build == 29 || tempheader.build == 30 || tempheader.build == 31 )
		{
			set_er(er_BITMAPOFFSET, 0);
			set_qr(qr_BITMAPOFFSETFIX, 0);    
		}
	}
	if ( tempheader.zelda_version == 0x254 )
	{
		set_er(er_BITMAPOFFSET, 0);
		set_qr(qr_BITMAPOFFSETFIX, 0);    
	}
	if ( tempheader.zelda_version == 0x255 && tempheader.build < 42 ) //QR was added to 255 in this build.
	{
		set_er(er_BITMAPOFFSET, 0);
		set_qr(qr_BITMAPOFFSETFIX, 0);    
	}
	//optimise fast drawing for older versions.
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 42) )
	{
		set_qr(qr_OLDSPRITEDRAWS, 1);    
	}
	//Old eweapon->Parent (was added in 2.54, Alpha 19)
	//The change was made in build 43, but I'm setting this to < 42, because quests made in 42 would benefit from this change, and
	//older quests can set the rule by hand. We need a new qst.dat again.
	if ( tempheader.zelda_version == 0x254 || (tempheader.zelda_version == 0x255 && tempheader.build < 42) )
	{
		set_qr(qr_OLDEWPNPARENT, 1);    
	}
	if ( tempheader.zelda_version == 0x254 || (tempheader.zelda_version == 0x255 && tempheader.build < 44) )
	{
		set_qr(qr_OLDCREATEBITMAP_ARGS, 1);    
	}
	if ( tempheader.zelda_version == 0x254 || (tempheader.zelda_version == 0x255 && tempheader.build < 45) )
	{
		set_qr(qr_OLDQUESTMISC, 1);    
	}
	if ( tempheader.zelda_version < 0x254 )
	{
		set_qr(qr_OLDCREATEBITMAP_ARGS, 0);  
		set_qr(qr_OLDEWPNPARENT, 0); 	    
		set_qr(qr_OLDQUESTMISC, 0); 	    
	}
	
	//item scripts continue to run
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 44) )
	{
		set_qr(qr_ITEMSCRIPTSKEEPRUNNING, 0);  	    
		set_qr(qr_SCRIPTSRUNINHEROSTEPFORWARD, 0);  	    
		set_qr(qr_FIXSCRIPTSDURINGSCROLLING, 0);
		set_qr(qr_SCRIPTDRAWSINWARPS, 0);  	    
		set_qr(qr_DYINGENEMYESDONTHURTHERO, 0);  	    
		set_qr(qr_OUTOFBOUNDSENEMIES, 0);  
		set_qr(qr_SPRITEXY_IS_FLOAT, 0);
	}
	
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 46) )
	{
		set_qr(qr_CLEARINITDONSCRIPTCHANGE, 1);  	    
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 46) )
	{
		set_qr(qr_TRACESCRIPTIDS, 0);      
		set_qr(qr_SCRIPT_FRIENDLY_ENEMY_TYPES, 1);      
		set_qr(qr_PARSER_BOOL_TRUE_DECIMAL, 1); // Deprecated.
		set_qr(qr_PARSER_250DIVISION,1); // Deprecated.
		set_qr(qr_PARSER_TRUE_INT_SIZE,0); // Deprecated.
		set_qr(qr_PARSER_FORCE_INLINE,0); // Deprecated.
		set_qr(qr_PARSER_BINARY_32BIT,0); // Deprecated.
		if ( get_qr(qr_SELECTAWPN) ) 
		{
			set_qr(qr_NO_L_R_BUTTON_INVENTORY_SWAP,1); 
			//In < 2.55a27, if you had an A+B subscreen, L and R didn't shift through inventory.
			//Now they **do**, unless you disable that behaviour.
			//For the sake of compatibility, old quests with the A+B subscreen rule enabed
			//now enable the disable L/R item swap on load.
		}
			
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 47) )
	{
		//Compatibility: Setting the hero's action to rafting was previously disallowed, though legal for scripts to attempt.
		set_qr(qr_DISALLOW_SETTING_RAFTING, 1);
		//Compatibility: The calculation for when to loop an animation did not factor in ASkipY correctly, resulting in
		//animations ending earlier than they should.
		set_qr(qr_BROKEN_ASKIP_Y_FRAMES, 1);
		//Enemies would ignore solidity on the top half of combos
		set_qr(qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY, 1);
		//Ceiling collison was a bit wonky, including hitting your head before you are near the ceiling or clipping into it slightly.
		set_qr(qr_OLD_SIDEVIEW_CEILING_COLLISON, 1);
		//If an itemdata had a 'frames' of 0, items created of that data would ignore all changes to 'frames'
		set_qr(qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES, 1);
		//Collision used some odd calculations before, and enemies could not be hit back into the top row or left column
		set_qr(qr_OLD_ENEMY_KNOCKBACK_COLLISION, 1);
	}
	if ( tempheader.zelda_version < 0x255 )
	{
		set_qr(qr_NOFFCWAITDRAW, 1);  
		set_qr(qr_NOITEMWAITDRAW, 1);  
		set_qr(qr_SETENEMYWEAPONSPRITESONWPNCHANGE, 1); 
		set_qr(qr_OLD_INIT_SCRIPT_TIMING, 1);
		//set_qr(qr_DO_NOT_DEALLOCATE_INIT_AND_SAVELOAD_ARRAYS, 1);
	}
	if ( tempheader.zelda_version < 0x255 || ( tempheader.zelda_version == 0x255 && tempheader.build < 48 ) )
	{
		set_qr(qr_SETENEMYWEAPONSPRITESONWPNCHANGE, 1);  
	}
	if( tempheader.zelda_version < 0x255 || ( tempheader.zelda_version == 0x255 && tempheader.build < 52 ) )
	{
		set_qr(qr_OLD_PRINTF_ARGS, 1);
	}
	
	
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 54) )
	{
		set_qr(qr_BROKEN_RING_POWER, 1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 56) )
	{
		set_qr(qr_NO_OVERWORLD_MAP_CHARTING, 1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 57) )
	{
		set_qr(qr_DUNGEONS_USE_CLASSIC_CHARTING, 1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 58) )
	{
		//Rule used to be 'qr_SETXYBUTTONITEMS', now split.
		if(get_qr(qr_SET_XBUTTON_ITEMS))
			set_qr(qr_SET_YBUTTON_ITEMS,1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 59) )
	{
		set_qr(qr_ALLOW_EDITING_COMBO_0,1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 60) )
	{
		set_qr(qr_OLD_CHEST_COLLISION,1);
	}
	
	if ( tempheader.zelda_version < 0x254 )
	{
		set_qr(qr_250WRITEEDEFSCRIPT, 1);  
	}
	//Sideview spikes in 2.50.0
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<27)) //2.50.1RC3
	{
		set_qr(qr_OLDSIDEVIEWSPIKES, 1);
	}
	//more 2.50 fixes -Z
	if(tempheader.zelda_version < 0x250 || (tempheader.zelda_version == 0x250 && tempheader.build<31))
	{
		set_qr(qr_MELEEMAGICCOST, 0);
		set_qr(qr_GANONINTRO, 0); //This will get flipped later on in the compatrule 11 check. That's why it's turning it off.
		set_qr(qr_OLDMIRRORCOMBOS, 1);
		set_qr(qr_BROKENBOOKCOST, 1);
		set_qr(qr_BROKENCHARINTDRAWING, 1);
	
	}
	if(tempheader.zelda_version == 0x254 && tempheader.build<41)
	{
		//set_qr(qr_MELEEMAGICCOST, get_er(er_MAGICCOSTSWORD));
		set_qr(qr_MELEEMAGICCOST, 1);
	}
	
	if(tempheader.zelda_version < 0x193)
	{
		set_qr(qr_SHORTDGNWALK, 1);
	}
	
	if(tempheader.zelda_version < 0x255)
	{
		set_qr(qr_OLDINFMAGIC, 1);
	}
	
	if((tempheader.zelda_version < 0x250)) //2.10 and earlier allowed the triforce to Warp Hero out of Item Cellars in Dungeons. -Z (15th March, 2019 )
	{
		set_qr(qr_SIDEVIEWTRIFORCECELLAR,1);
	}
	
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 47) )
	{
		set_qr(qr_OLD_F6,1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 49) )
	{
		set_qr(qr_NO_OVERWRITING_HOPPING,1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 50) )
	{
		set_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT,1);
	}
	if ( tempheader.zelda_version < 0x255 || (tempheader.zelda_version == 0x255 && tempheader.build < 53) )
	{
		set_qr(qr_BROKEN_OVERWORLD_MINIMAP,1);
	}
	//}
	
	if(compatrule_version < 1) //Enemies->Secret only affects flag 16-31
		set_qr(qr_ENEMIES_SECRET_ONLY_16_31,1);
	
	if(compatrule_version < 2) //Old CSet2 Handling
		set_qr(qr_OLDCS2,1);
	
	if(compatrule_version < 3) //Hardcoded Shadow/Spawn/Death anim frames
		set_qr(qr_HARDCODED_ENEMY_ANIMS,1);
	
	if(compatrule_version < 4) //Hardcoded Shadow/Spawn/Death anim frames
		set_qr(qr_OLD_ITEMDATA_SCRIPT_TIMING,1);
	
	if(compatrule_version < 5 && tempheader.zelda_version >= 0x250) //Hardcoded Shadow/Spawn/Death anim frames
		set_qr(qr_NO_LANMOLA_RINGLEADER,1);
	
	if(compatrule_version < 6) //Step->Secret (Temp) only affects flag 16-31
		set_qr(qr_STEPTEMP_SECRET_ONLY_16_31,1);
	
	if(compatrule_version < 7) //'Hit All Triggers->Perm Secret' doesn't trigger temp secrets
		set_qr(qr_ALLTRIG_PERMSEC_NO_TEMP,1);
	
	if(compatrule_version < 8) //Hardcoded LItem/Bomb/Clock/Magic Tile Mods
		set_qr(qr_HARDCODED_LITEM_LTMS,1);
	
	if(compatrule_version < 9)
	{
		//Hardcoded BS Patras
		set_qr(qr_HARDCODED_BS_PATRA,1);
		//Hardcoded Patra Inner Eye offsets
		set_qr(qr_PATRAS_USE_HARDCODED_OFFSETS,1);
		//Broken 'Big enemy' animation style
		set_qr(qr_BROKEN_BIG_ENEMY_ANIMATION,1);
		//Broken Attribute 31/32
		set_qr(qr_BROKEN_ATTRIBUTE_31_32,1);
	}
	
	if(compatrule_version < 10) //Shared candle use limits
		set_qr(qr_CANDLES_SHARED_LIMIT,1);
	
	if(compatrule_version < 11) //No cross-screen return points
		set_qr(qr_OLD_RESPAWN_POINTS,1);

	if(compatrule_version < 12)
	{
		//Old fire trail duration
		set_qr(qr_OLD_FLAMETRAIL_DURATION,1);
		//Old Intro String in Ganon Room Behavior
		if(get_qr(qr_GANONINTRO)) set_qr(qr_GANONINTRO,0);
		else set_qr(qr_GANONINTRO,1);
	}
	
	if(compatrule_version < 13 && tempheader.zelda_version >= 0x255) //ANone doesn't reset to originaltile
		set_qr(qr_ANONE_NOANIM,1);
	
	if(compatrule_version < 14) //Old Bridge Combo Behavior
		set_qr(qr_OLD_BRIDGE_COMBOS,1);
	
	if(compatrule_version < 15) //Broken Z3 Animation
		set_qr(qr_BROKEN_Z3_ANIMATION,1);
	
	if(compatrule_version < 16) //Old Enemy Tile Behavior with Animation (None) Enemies
		set_qr(qr_OLD_TILE_INITIALIZATION,1);
	
	if(compatrule_version < 17)
	{
		//Old Quake/DrawYOffset behavior
		//set_qr(qr_OLD_DRAWOFFSET,1);
		//I'm leaving this commented cause I doubt it'll break anything and I think the bugfix might be appreciated in older versions.
		//On the offchance that it *does* break old quests, fixing it is as simple as uncommenting the set_bit above.
	}
	
	if(compatrule_version < 18)
	{
		//Broken DrawScreen Derivative Functions
		set_qr(qr_BROKEN_DRAWSCREEN_FUNCTIONS,1);
		//Scrolling Doesn't Cancel Charge
		set_qr(qr_SCROLLING_KILLS_CHARGE,1);
	}
	
	if(compatrule_version < 19) //Broken Enemy Item Carrying with Large Enemies
		set_qr(qr_BROKEN_ITEM_CARRYING,1);
	
	if(compatrule_version < 20)
		set_qr(qr_CUSTOMWEAPON_IGNORE_COST,1);
	
	if(compatrule_version < 21)
	{
		set_qr(qr_LEEVERS_DONT_OBEY_STUN,1);
		set_qr(qr_GANON_CANT_SPAWN_ON_CONTINUE,1);
		set_qr(qr_WIZZROBES_DONT_OBEY_STUN,1);
		set_qr(qr_OLD_BUG_NET,1);
		set_qr(qr_MANHANDLA_BLOCK_SFX,1);
	}
	
	if(compatrule_version < 22)
		set_qr(qr_BROKEN_KEEPOLD_FLAG,1);
	
	if(compatrule_version < 23)
		set_qr(qr_OLD_HALF_MAGIC,1);
	
	if(compatrule_version < 24)
	{
		set_qr(qr_WARPS_RESTART_DMAPSCRIPT,1);
		set_qr(qr_DMAP_0_CONTINUE_BUG,1);
	}
	
	if(compatrule_version < 25)
	{
		if (get_qr(qr_OLD_FAIRY_LIMIT)) set_qr(qr_OLD_FAIRY_LIMIT,0);
		else set_qr(qr_OLD_FAIRY_LIMIT,1);
		set_qr(qr_OLD_SCRIPTED_KNOCKBACK,1);
	}
	if(compatrule_version < 26)
	{
		set_qr(qr_OLD_KEESE_Z_AXIS,1);
		set_qr(qr_POLVIRE_NO_SHADOW,1);
		set_qr(qr_SUBSCR_OLD_SELECTOR,1);
	}
	if(compatrule_version < 27) //Noticed some junk data in the QR array...
	{
		for(auto q = qr_POLVIRE_NO_SHADOW+1; q < qr_PARSER_250DIVISION; ++q)
			set_qr(q,0);
		for(auto q = qr_COMBODATA_INITD_MULT_TENK+1; q < qr_MAX; ++q)
			set_qr(q,0);
		//This should nuke any remaining junk data... not sure if it affected anything previous. -Em
	}
	if(compatrule_version < 28)
		set_qr(qr_SUBSCR_BACKWARDS_ID_ORDER,1);
	if(compatrule_version < 29)
		set_qr(qr_OLD_LOCKBLOCK_COLLISION,1);
	if(compatrule_version < 30)
	{
		set_qr(qr_DECO_2_YOFFSET,1);
		set_qr(qr_SCREENSTATE_80s_BUG,1);
	}
	if(compatrule_version < 31)
	{
		set_qr(qr_GOHMA_UNDAMAGED_BUG,1);
		set_qr(qr_FFCPRELOAD_BUGGED_LOAD,1);
	}
	if(compatrule_version < 32)
		set_qr(qr_BROKEN_GETPIXEL_VALUE,1);
	if(compatrule_version < 33)
		set_qr(qr_NO_LIFT_SPRITE,1);
	if(compatrule_version < 34)
	{
		set_qr(qr_OLD_SIDEVIEW_LANDING_CODE,1);
		set_qr(qr_OLD_FFC_SPEED_CAP,1);
		set_qr(qr_OLD_FFC_FUNCTIONALITY,1);
		set_qr(qr_OLD_WIZZROBE_SUBMERGING,1);
	}
	if(compatrule_version < 35)
	{
		set_qr(qr_ZS_NO_NEG_ARRAY,1);
		set_qr(qr_BROKEN_INPUT_DOWN_STATE,1);
	}
	if(compatrule_version < 36)
		set_qr(qr_OLD_SHALLOW_SFX,1);
	if(compatrule_version < 37)
		set_qr(qr_SPARKLES_INHERIT_PROPERTIES,1);
	if(compatrule_version < 38)
		set_qr(qr_BUGGED_LAYERED_FLAGS,1);
	if(compatrule_version < 39)
		set_qr(qr_HARDCODED_FFC_BUSH_DROPS,1);
	if(compatrule_version < 40)
		set_qr(qr_MOVINGBLOCK_FAKE_SOLID,1);
	if(compatrule_version < 41)
		set_qr(qr_BROKENHITBY,1);
	if(compatrule_version < 42)
		set_qr(qr_BROKEN_MOVING_BOMBS,1);
	if(compatrule_version < 43)
		set_qr(qr_OLD_BOMB_HITBOXES,1);
	if(compatrule_version < 44)
		set_qr(qr_SCROLLWARP_NO_RESET_FRAME,1);
	if(compatrule_version < 45)
		set_qr(qr_ENEMIES_DONT_SCRIPT_FIRST_FRAME,1);
	if(compatrule_version < 46)
		set_qr(qr_BROKEN_RAFT_SCROLL,1);
	if(compatrule_version < 47)
	{
		set_qr(qr_SENSITIVE_SOLID_DAMAGE,1);
		set_qr(qr_OLD_CONVEYOR_COLLISION,1);
	}
	if(compatrule_version < 48)
		set_qr(qr_OLD_GUY_HANDLING,1);
	if(compatrule_version < 49)
		set_qr(qr_FAIRY_FLAG_COMPAT,1);
	if(compatrule_version < 50)
		set_qr(qr_OLD_LENS_LAYEREFFECT,1);
	if(compatrule_version < 51)
		set_qr(qr_PUSHBLOCK_SPRITE_LAYER,1);
	if (compatrule_version < 52)
		set_qr(qr_OLD_SCRIPT_VOLUME, 1);
	if(compatrule_version < 53)
	{
		set_qr(qr_OLD_SUBSCR,1);
		set_qr(qr_ITM_0_INVIS_ON_BTNS,1);
		set_qr(qr_OLD_GAUGE_TILE_LAYOUT,1);
	}
	if(compatrule_version < 54)
		set_qr(qr_WALKTHROUGHWALL_NO_DOORSTATE,1);
	if(compatrule_version < 55)
		set_qr(qr_SPOTLIGHT_IGNR_SOLIDOBJ,1);
	if(compatrule_version < 56)
		set_qr(qr_BROKEN_LIGHTBEAM_HITBOX,1);
	if(compatrule_version < 57)
		set_qr(qr_BROKEN_SWORD_SPIN_TRIGGERS,1);
	if(compatrule_version < 58)
		set_qr(qr_OLD_DMAP_INTRO_STRINGS,1);
	if(compatrule_version < 59)
		set_qr(qr_SCRIPT_CONTHP_IS_HEARTS,1);
	if(compatrule_version < 60)
		set_qr(qr_SEPARATE_BOMBABLE_TAPPING_SFX,1);
	if(compatrule_version < 61)
		set_qr(qr_BROKEN_BOMB_AMMO_COSTS,1);
	if(compatrule_version < 62)
		set_qr(qr_OLD_BROKEN_WARPEX_MUSIC,1);
	if(compatrule_version < 63)
	{
		set_qr(qr_BROKEN_LIFTSWIM,1);
		set_qr(qr_BROKEN_GENERIC_PUSHBLOCK_LOCKING,1);
	}
	if(compatrule_version < 64)
		set_qr(qr_BROKEN_FLAME_ARROW_REFLECTING,1);
	if(compatrule_version < 65)
		set_qr(qr_BROKEN_SIDEVIEW_SPRITE_JUMP,1);
	if(compatrule_version < 67)
		set_qr(qr_OLD_HERO_WARP_RETSQUARE,1);
	if(compatrule_version < 68)
		set_qr(qr_SCRIPTS_6_BIT_COLOR,1);
	if(compatrule_version < 69)
		set_qr(qr_SETENEMYWEAPONSOUNDSONWPNCHANGE, 1);
	if (compatrule_version < 70)
		set_qr(qr_BROKEN_CONVEYORS, 1);
	if (compatrule_version < 71)
		set_qr(qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS, 1);
	if (compatrule_version < 72)
		set_qr(qr_BROKEN_PUSHBLOCK_FLAG_CLONING, 1);
	if (compatrule_version < 73)
	{
		set_qr(qr_OLD_LANDING_SFX, 1);
		set_qr(qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS, 1);
	}
	if (compatrule_version < 76)
		set_qr(qr_INVERTED_DARK_COMBO_TRIGGERS, 1);
	if (compatrule_version < 77)
		set_qr(qr_BROKEN_ICY_FLOOR_SIDEVIEW, 1);
	if (compatrule_version < 78)
		set_qr(qr_SCRIPTS_SCREEN_DRAW_LIGHT_NO_OFFSET, 1);
	if (compatrule_version < 79)
		set_qr(qr_BROKEN_SYSTEM_COLORS, 1);
	if (compatrule_version < 80)
		set_qr(qr_ZS_OLD_SUSPEND_FFC, 1);
	if (compatrule_version < 82)
	{
		set_qr(qr_EW_ROCKS_HARDCODED_BREAK_ON_SOLID, 1);
		set_qr(qr_IMPRECISE_WEAPON_SOLIDITY_CHECKS, 1);
	}
	if (compatrule_version < 85)
		set_qr(qr_OLD_WEAPON_REFLECTION, 1);
	if (compatrule_version < 87)
		set_qr(qr_OLD_TERMINAL_VELOCITY, 1);
	if (compatrule_version < 88)
		set_qr(qr_OLD_SCRIPT_LEVEL_GLOBAL_STATES, 1);
	if (compatrule_version < 89)
		set_qr(qr_BROKEN_ARMOS_GRAVE_BIGHITBOX_COLLISION, 1);
	
	if (compatrule_version < 90)
	{
		set_qr(qr_CLASSIC_DRAWING_ORDER, 1);
		set_qr(qr_OLD_WEAPON_DRAW_ANIMATE_TIMING, 1);
	}

	if (compatrule_version < 91)
	{
		set_qr(qr_OLD_SCRIPTS_INTERNAL_ARRAYS_BOUND_INDEX, 1);
		set_qr(qr_OLD_SCRIPTS_ARRAYS_NON_ZERO_DEFAULT_VALUE, 1);
	}

	if (compatrule_version < 94)
		set_qr(qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING, 1);
	if (compatrule_version < 95)
		set_qr(qr_ONLY_MARK_SCREENS_VISITED_IF_MAP_VIEWABLE, 1);
	
	if (compatrule_version < 101)
		set_qr(qr_BROKEN_WATER_PASSIVE_DAMAGE, 1);
	if (compatrule_version < 103)
		set_qr(qr_OLD_ITEM_HITBOXES, 1);
	if (compatrule_version < 104)
		set_qr(qr_SUBSCR_SELECTOR_USES_ITEM_HITBOX, 1);
	if (compatrule_version < 108)
	{
		set_qr(qr_BROKEN_SV_SOLID_CONVEYORS, 1);
		set_qr(qr_ITEM_CONVEYORS_IGNORE_HITBOX, 1);
		set_qr(qr_ITEMS_HARDCODED_SIDEVIEW_HITBOX, 1);
		set_qr(qr_WEAPONS_HARDCODED_SIDEVIEW_HITBOX, 1);
	}
	if (compatrule_version < 110)
	{
		set_qr(qr_OLD_LAYER_DRAW_ORDER, get_qr(qr_CLASSIC_DRAWING_ORDER));
		set_qr(qr_MAP_SPOILS_DARKROOMS, 1);
	}
	if (compatrule_version < 113)
	{
		set_qr(qr_OLD_BRIDGE_COMBO_COVER, 1);
		set_qr(qr_MINIMAP_VISITED_CHARTED, 1);
		set_qr(qr_QUAKE_HAMMER_IN_AIR, 1);
	}
	if (compatrule_version < 114)
	{
		set_qr(qr_NO_FLASHING_MSG_MORE, 1);
		set_qr(qr_BROKEN_LENS_HINTS_SECRETS, 1);
	}
	if (compatrule_version < 115)
		set_qr(qr_CHESTS_SIGNS_LOCKBLOCKS_ABOVE_2, 1);
	if (compatrule_version < 116)
		set_qr(qr_OLD_SUMMONER_COUNTS, 1);
	if (compatrule_version < 117)
		set_qr(qr_BROKEN_LARGE_ARMOS_ANIM, 1);
	if (compatrule_version < 119)
		set_qr(qr_OLD_LONG_SCC_ARGS, 1);
	if (compatrule_version < 120)
		set_qr(qr_STRINGS_DONT_SCROLL, 1);
	if (compatrule_version < 121)
		set_qr(qr_SCRIPTS_SHARE_INITD, 1);

	set_qr(qr_ANIMATECUSTOMWEAPONS,0);
	if (s_version < 16)
		set_qr(qr_BROKEN_HORIZONTAL_WEAPON_ANIM,1);

	// These QRs are for changes that were cherry-picked to the 2.55 branch. We can't bump the
	// compatrule version there anymore, so instead we check the header version.
	if (tempheader.version_major >= 3)
	{
		if(compatrule_version < 66)
			set_qr(qr_NEWDARK_TRANS_STACKING, 1);
		if (compatrule_version < 74)
			set_qr(qr_BROKEN_SCRIPTS_SCROLLING_HERO_POSITION, 1);
		if (compatrule_version < 75)
			set_qr(qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN, 1);
		if (compatrule_version < 81)
		{
			set_qr(qr_BROKEN_SCROLL_INSTEAD_OF_DROWN_FALL, 1);
			set_qr(qr_ROPE_ENEMIES_SPEED_NOT_CONFIGURABLE, 1);
		}
		if (compatrule_version < 83)
			set_qr(qr_BROKEN_BLOCKHOLE_PITFALLS, 1);
		if (compatrule_version < 84)
			set_qr(qr_CUSTOM_WEAPON_BROKEN_SIZE, 1);
		if (compatrule_version < 86)
			set_qr(qr_OLD_SPRITE_FALL_DROWN, 1);
		if (compatrule_version < 92)
			set_qr(qr_OLD_PIT_SENSITIVITY, 1);
		if (compatrule_version < 93)
			set_qr(qr_ACTIVE_SHIELD_PASSIVE_ROC_NO_SCRIPT, 1);
		if (compatrule_version < 96)
			set_qr(qr_ACTIVE_SUB_IGNORE_8PX, 1);
		if (compatrule_version < 97)
		{
			set_qr(qr_DRAWCOMBO_IGNORES_FRAME, 1);
			set_qr(qr_DRAWTILE_TALL_DRAWS_WRAP_POORLY, 1);
		}
		if (compatrule_version < 98)
			set_qr(qr_BUGGY_MULTIPLE_WIND, 1);
		if (compatrule_version < 99)
			set_qr(qr_NO_DEATH_EVENTS_FOR_SEGMENTED_ENEMY_CORES, 1);
		if (compatrule_version < 100)
			set_qr(qr_BROKEN_COPYCAT_SELF_TRIGGER, 1);
		if (compatrule_version < 102)
			set_qr(qr_BROKEN_SELF_TRIGGERING_TRIGGERS, 1);
		if (compatrule_version < 105)
			set_qr(qr_BROKEN_0_DAMAGE_BRANG_HSHOT, 1);
		if (compatrule_version < 106)
			set_qr(qr_BROKEN_SPINTILE_COMBO_CHANGE, 1);
		if (compatrule_version < 107)
			set_qr(qr_LIGHT_RADIUS_IGNORE_HIT_OFFSETS, 1);
		if (compatrule_version < 109)
			set_qr(qr_BROKEN_PUSHBLOCK_TRIGGER_GROUPS, 1);
		if (compatrule_version < 110)
			set_qr(qr_BS_OW_IGNORES_MAP_ITEM, 1);
		if (compatrule_version < 111)
			set_qr(qr_BROKEN_ENEMY_SIDEVIEW_PLATFORM_HITBOX, 1);
		if (compatrule_version < 112)
			set_qr(qr_BROKEN_PLAYER_SOLIDPUSH, 1);
		if (compatrule_version < 115)
		{
			set_qr(qr_KNOCKBACK_THROUGH_PLATFORMS, 1);
			set_qr(qr_BROKEN_SIDEVIEW_SOLID_FFC_COLLISION, 1);
		}
		if (compatrule_version < 118)
			set_qr(qr_BROKEN_SCC_MAP_ARGS, 1);
	}
	else
	{
		// Older than 2.55.6?
		if (tempheader.compareVer(2, 55, 6) < 0)
			set_qr(qr_ROPE_ENEMIES_SPEED_NOT_CONFIGURABLE, 1);
	
		// Older than 2.55.9?
		if (tempheader.compareVer(2, 55, 9) < 0)
		{
			set_qr(qr_NEWDARK_TRANS_STACKING,1);
			set_qr(qr_BROKEN_SCRIPTS_SCROLLING_HERO_POSITION, 1);
			// Note: in 3.0, this is only sometimes enabled for quests prior to 2.55.9.
			// So purposefully skipped here. See compat_qr_hide_bottom_pixels.
			// set_qr(qr_HIDE_BOTTOM_8_PIXELS, 1);
		}
	
		// Older than 2.55.10?
		if (tempheader.compareVer(2, 55, 10) < 0)
		{
			set_qr(qr_BROKEN_SCRIPTS_BITMAP_DRAW_ORIGIN, 1);
			set_qr(qr_LAYER6_STRINGS_OVER_SUBSCREEN, 1);
			set_qr(qr_BROKEN_SCROLL_INSTEAD_OF_DROWN_FALL, 1);
		}
	
		// Older than 2.55.11?
		if (tempheader.compareVer(2, 55, 11) < 0)
		{
			set_qr(qr_BROKEN_BLOCKHOLE_PITFALLS, 1);
			set_qr(qr_CUSTOM_WEAPON_BROKEN_SIZE, 1);
			set_qr(qr_OLD_SPRITE_FALL_DROWN, 1);
			set_qr(qr_OLD_PIT_SENSITIVITY, 1);
		}
	
		// Older than 2.55.12?
		if (tempheader.compareVer(2, 55, 12) < 0)
		{
			set_qr(qr_ACTIVE_SHIELD_PASSIVE_ROC_NO_SCRIPT, 1);
			set_qr(qr_ACTIVE_SUB_IGNORE_8PX, 1);
		}
	
		// Older than 2.55.13?
		if (tempheader.compareVer(2, 55, 13) < 0)
		{
			set_qr(qr_DRAWCOMBO_IGNORES_FRAME, 1);
			set_qr(qr_DRAWTILE_TALL_DRAWS_WRAP_POORLY, 1);
			set_qr(qr_BUGGY_MULTIPLE_WIND, 1);
			set_qr(qr_NO_DEATH_EVENTS_FOR_SEGMENTED_ENEMY_CORES, 1);
			set_qr(qr_BROKEN_COPYCAT_SELF_TRIGGER, 1);
			set_qr(qr_BROKEN_SELF_TRIGGERING_TRIGGERS, 1);
		}

		// Older than 2.55.14?
		if (tempheader.compareVer(2, 55, 14) < 0)
		{
			set_qr(qr_BROKEN_0_DAMAGE_BRANG_HSHOT, 1);
			set_qr(qr_BROKEN_SPINTILE_COMBO_CHANGE, 1);
			set_qr(qr_LIGHT_RADIUS_IGNORE_HIT_OFFSETS, 1);
		}

		// Older than 2.55.15?
		if (tempheader.compareVer(2, 55, 15) < 0)
		{
			set_qr(qr_BROKEN_PUSHBLOCK_TRIGGER_GROUPS, 1);
			set_qr(qr_BS_OW_IGNORES_MAP_ITEM, 1);
			set_qr(qr_BROKEN_ENEMY_SIDEVIEW_PLATFORM_HITBOX, 1);
			set_qr(qr_BROKEN_PLAYER_SOLIDPUSH, 1);
			set_qr(qr_KNOCKBACK_THROUGH_PLATFORMS, 1);
			set_qr(qr_BROKEN_SIDEVIEW_SOLID_FFC_COLLISION, 1);
			set_qr(qr_BROKEN_SCC_MAP_ARGS, 1);
		}
	}

	*Header = tempheader;

	return 0;
}
