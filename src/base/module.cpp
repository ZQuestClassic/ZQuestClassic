#include "base/module.h"
#include "base/zdefs.h"
#include "base/zsys.h"
#include "zinfo.h"

extern zcmodule moduledata;

bool ZModule::init(bool d) //bool default
{
	memset(moduledata.module_name, 0, sizeof(moduledata.module_name));
	memset(moduledata.quests, 0, sizeof(moduledata.quests));
	memset(moduledata.skipnames, 0, sizeof(moduledata.skipnames));
	memset(moduledata.datafiles, 0, sizeof(moduledata.datafiles));
	memset(moduledata.enem_type_names, 0, sizeof(moduledata.enem_type_names));
	memset(moduledata.enem_anim_type_names, 0, sizeof(moduledata.enem_anim_type_names));
	
	memset(moduledata.roomtype_names, 0, sizeof(moduledata.roomtype_names));
	memset(moduledata.walkmisc7_names, 0, sizeof(moduledata.walkmisc7_names));
	memset(moduledata.walkmisc9_names, 0, sizeof(moduledata.walkmisc9_names));
	memset(moduledata.guy_type_names, 0, sizeof(moduledata.guy_type_names));
	memset(moduledata.enemy_weapon_names, 0, sizeof(moduledata.enemy_weapon_names));
	memset(moduledata.enemy_weapon_names, 0, sizeof(moduledata.enemy_scriptweaponweapon_names)); 
	memset(moduledata.player_weapon_names, 0, sizeof(moduledata.player_weapon_names));
	memset(moduledata.base_NSF_file, 0, sizeof(moduledata.base_NSF_file));
	memset(moduledata.copyright_strings, 0, sizeof(moduledata.copyright_strings));
	memset(moduledata.copyright_string_vars, 0, sizeof(moduledata.copyright_string_vars));
	memset(moduledata.delete_quest_data_on_wingame, 0, sizeof(moduledata.delete_quest_data_on_wingame));
	memset(moduledata.select_screen_tile_csets, 0, sizeof(moduledata.select_screen_tile_csets));
	memset(moduledata.select_screen_tiles, 0, sizeof(moduledata.select_screen_tiles));
	moduledata.old_quest_serial_flow = 0;
	memset(moduledata.startingdmap, 0, sizeof(moduledata.startingdmap));
	memset(moduledata.startingscreen, 0, sizeof(moduledata.startingscreen));
	moduledata.max_quest_files = 0;
	moduledata.animate_NES_title = 0;
	moduledata.title_track = moduledata.tf_track = moduledata.gameover_track = moduledata.ending_track = moduledata.dungeon_track = moduledata.overworld_track = moduledata.lastlevel_track = 0;
	moduledata.refresh_title_screen = 0;
	
	memset(moduledata.moduletitle, 0, sizeof(moduledata.moduletitle));
	memset(moduledata.moduleauthor, 0, sizeof(moduledata.moduleauthor));
	memset(moduledata.moduleinfo0, 0, sizeof(moduledata.moduleinfo0));
	memset(moduledata.moduleinfo1, 0, sizeof(moduledata.moduleinfo1));	
	memset(moduledata.moduleinfo2, 0, sizeof(moduledata.moduleinfo2));
	memset(moduledata.moduleinfo3, 0, sizeof(moduledata.moduleinfo3));
	memset(moduledata.moduleinfo4, 0, sizeof(moduledata.moduleinfo4));
	memset(moduledata.moduletimezone, 0, sizeof(moduledata.moduletimezone));
	//memset(moduledata.module_base_nsf, 0, sizeof(moduledata.module_base_nsf));
	
	moduledata.modver_1 = 0;
	moduledata.modver_2 = 0;	
	moduledata.modver_3 = 0;
	moduledata.modver_4 = 0;
	moduledata.modbuild = 0;
	moduledata.modbeta = 0;
	moduledata.modmonth = 0;
	moduledata.modday = 0;
	moduledata.modyear = 0;
	moduledata.modhour = 0;
	moduledata.modminute = 0;
	
	//strcpy(moduledata.module_name,"default.zmod");
	//al_trace("Module name set to %s\n",moduledata.module_name);
	//We load the current module name from zc.cfg or zquest.cfg!
	//Otherwise, we don't know what file to access to load the module vars! 
	strcpy(moduledata.module_name,zc_get_config("ZCMODULE","current_module","modules/classic.zmod"));
	al_trace("The Current ZQuest Classic Module is: %s\n",moduledata.module_name); 
	if(!fileexists((char*)moduledata.module_name))
	{
		Z_error_fatal("ZQuest Classic I/O Error:\nNo module definitions found.\nZQuest Classic cannot run without these definitions,\nand is now exiting.\nPlease check your settings in %s.cfg.\n","zcl");
		return false;
	}
	if(!d) return true;
	zc_push_config();
	//Switch to the module to load its config properties.
	zc_config_file(moduledata.module_name);
	
	{
		//zcm path
		//al_trace("Module name set to %s\n",moduledata.module_name);
		
		//Metadata
		strcpy(moduledata.moduletitle,zc_get_config_basic("METADATA","title",""));
		strcpy(moduledata.moduleauthor,zc_get_config_basic("METADATA","author",""));
		strcpy(moduledata.moduleinfo0,zc_get_config_basic("METADATA","info_0",""));
		strcpy(moduledata.moduleinfo1,zc_get_config_basic("METADATA","info_1",""));
		strcpy(moduledata.moduleinfo2,zc_get_config_basic("METADATA","info_2",""));
		strcpy(moduledata.moduleinfo3,zc_get_config_basic("METADATA","info_3",""));
		strcpy(moduledata.moduleinfo4,zc_get_config_basic("METADATA","info_4",""));
		strcpy(moduledata.moduletimezone,zc_get_config_basic("METADATA","timezone","GMT"));
		//strcpy(moduledata.module_base_nsf,zc_get_config_basic("METADATA","nsf",""));
		moduledata.modver_1 = zc_get_config_basic("METADATA","version_first",0);
		moduledata.modver_2 = zc_get_config_basic("METADATA","version_second",0);	
		moduledata.modver_3 = zc_get_config_basic("METADATA","version_third",0);
		moduledata.modver_4 = zc_get_config_basic("METADATA","version_fourth",0);
		moduledata.modbuild = zc_get_config_basic("METADATA","version_build",0);
		moduledata.modbeta = zc_get_config_basic("METADATA","version_beta",0);
		moduledata.modmonth = zc_get_config_basic("METADATA","version_month",0);
		moduledata.modday = zc_get_config_basic("METADATA","version_day",0);
		moduledata.modyear = zc_get_config_basic("METADATA","version_year",0);
		moduledata.modhour = zc_get_config_basic("METADATA","version_hour",0);
		moduledata.modminute = zc_get_config_basic("METADATA","version_minute",0); 
		
		//quests
		moduledata.old_quest_serial_flow = zc_get_config_basic("QUESTS","quest_flow",1);
		moduledata.max_quest_files = vbound(zc_get_config_basic("QUESTS","num_quest_files",0),0,10);
		strcpy(moduledata.quests[0],zc_get_config_basic("QUESTS","first_qst","-"));
		strcpy(moduledata.quests[1],zc_get_config_basic("QUESTS","second_qst","-"));
		strcpy(moduledata.quests[2],zc_get_config_basic("QUESTS","third_qst","-"));
		strcpy(moduledata.quests[3],zc_get_config_basic("QUESTS","fourth_qst","-"));
		strcpy(moduledata.quests[4],zc_get_config_basic("QUESTS","fifth_qst","-"));
		strcpy(moduledata.quests[5],zc_get_config_basic("QUESTS","sixth_qst","-"));
		strcpy(moduledata.quests[6],zc_get_config_basic("QUESTS","seventh_qst","-"));
		strcpy(moduledata.quests[7],zc_get_config_basic("QUESTS","eighth_qst","-"));
		strcpy(moduledata.quests[8],zc_get_config_basic("QUESTS","ninth_qst","-"));
		strcpy(moduledata.quests[9],zc_get_config_basic("QUESTS","tenth_qst","-"));

#ifdef __EMSCRIPTEN__
		strcpy(moduledata.quests[0],"purezc/773/1st-mirrored-vertical-and-horizontal.qst");
		strcpy(moduledata.quests[1],"purezc/773/1st-mirrored-vertical-and-horizontal.qst");
#endif

		for ( int32_t q = 0; q < 10; q++ )
		{
			if ( moduledata.quests[q][0] == '-' ) strcpy(moduledata.quests[q],"");
		}
		
		//quest skip names
		moduledata.skipnames[0][0] = 0;
		strcpy(moduledata.skipnames[1],zc_get_config_basic("NAMEENTRY","second_qst_skip","-"));
		strcpy(moduledata.skipnames[2],zc_get_config_basic("NAMEENTRY","third_qst_skip","-"));
		strcpy(moduledata.skipnames[3],zc_get_config_basic("NAMEENTRY","fourth_qst_skip","-"));
		strcpy(moduledata.skipnames[4],zc_get_config_basic("NAMEENTRY","fifth_qst_skip","-"));
		strcpy(moduledata.skipnames[5],zc_get_config_basic("NAMEENTRY","sixth_qst_skip","-"));
		strcpy(moduledata.skipnames[6],zc_get_config_basic("NAMEENTRY","seventh_qst_skip","-"));
		strcpy(moduledata.skipnames[7],zc_get_config_basic("NAMEENTRY","eighth_qst_skip","-"));
		strcpy(moduledata.skipnames[8],zc_get_config_basic("NAMEENTRY","ninth_qst_skip","-"));
		strcpy(moduledata.skipnames[9],zc_get_config_basic("NAMEENTRY","tenth_qst_skip","-"));
		
		//datafiles
		strcpy(moduledata.datafiles[zelda_dat],zc_get_config_basic("DATAFILES","zcplayer_datafile","zelda.dat"));
		al_trace("Module zelda_dat set to %s\n",moduledata.datafiles[zelda_dat]);
		strcpy(moduledata.datafiles[zquest_dat],zc_get_config_basic("DATAFILES","zquest_datafile","zquest.dat"));
		al_trace("Module zquest_dat set to %s\n",moduledata.datafiles[zquest_dat]);
		strcpy(moduledata.datafiles[fonts_dat],zc_get_config_basic("DATAFILES","fonts_datafile","fonts.dat"));
		al_trace("Module fonts_dat set to %s\n",moduledata.datafiles[fonts_dat]);
		strcpy(moduledata.datafiles[sfx_dat],zc_get_config_basic("DATAFILES","sounds_datafile","sfx.dat"));
		al_trace("Module sfx_dat set to %s\n",moduledata.datafiles[sfx_dat]);
		
		
		strcpy(moduledata.base_NSF_file,zc_get_config_basic("DATAFILES","base_NSF_file","zelda.nsf"));
		al_trace("Base NSF file: %s\n", moduledata.base_NSF_file);
		
		moduledata.title_track = zc_get_config_basic("DATAFILES","title_track",0);
		moduledata.ending_track = zc_get_config_basic("DATAFILES","ending_track",1);
		moduledata.tf_track = zc_get_config_basic("DATAFILES","tf_track",5);
		moduledata.gameover_track = zc_get_config_basic("DATAFILES","gameover_track",0);
		moduledata.dungeon_track = zc_get_config_basic("DATAFILES","dungeon_track",0);
		moduledata.overworld_track = zc_get_config_basic("DATAFILES","overworld_track",0);
		moduledata.lastlevel_track = zc_get_config_basic("DATAFILES","lastlevel_track",0);
		
		const char enemy_family_strings[eeMAX][255] =
		{
			"ee_family_guy","ee_family_walk","ee_family_shoot","ee_family_tek","ee_family_lev",
			"ee_family_pea","ee_family_zor","ee_family_rock","ee_family_gh","ee_family_arm",
			//10
			"ee_family_ke","ee_family_ge","ee_family_zl","ee_family_rp","ee_family_gor",
			"ee_family_trap","ee_family_wm","ee_family_jinx","ee_family_vir","ee_family_rike",
			//20
			"ee_family_pol","ee_family_wiz","ee_family_aqu","ee_family_mold","ee_family_dod",
			"ee_family_mhd","ee_family_glk","ee_family_dig","ee_family_goh","ee_family_lan",
			//30
			"ee_family_pat","ee_family_gan","ee_family_proj","ee_family_gtrib","ee_family_ztrib",
			"ee_family_vitrib","ee_family_ketrib","ee_family_spintile","ee_family_none","ee_family_faerie",
			//40
			"ee_family_otherflt","ee_family_other", "max250",
			"Custom_01", "Custom_02", "Custom_03", "Custom_04", "Custom_05",
			"Custom_06", "Custom_07", "Custom_08", "Custom_09", "Custom_10",
			"Custom_11", "Custom_12", "Custom_13", "Custom_14", "Custom_15",
			"Custom_16", "Custom_17", "Custom_18", "Custom_19", "Custom_20",
			"Friendly_NPC_01", "Friendly_NPC_02", "Friendly_NPC_03", "Friendly_NPC_04",
			"Friendly_NPC_05", "Friendly_NPC_06", "Friendly_NPC_07",
			"Friendly_NPC_08", "Friendly_NPC_09", "Friendly_NPC_10"
		};
		
		const char default_enemy_types[eeMAX][255] =
		{
			"-Guy","Walking Enemy","-Unused","Tektite","Leever",
			"Peahat","Zora","Rock","Ghini","-Unused",
			"Keese","-Unused","-Unused","-Unused","-Unused",//goriya
			"Trap","Wall Master","-Unused","-Unused","-Unused",//likelike
			"-Unused","Wizzrobe","Aquamentus","Moldorm","Dodongo",
			"Manhandla","Gleeok","Digdogger","Gohma","Lanmola",
			"Patra","Ganon","Projectile Shooter","-Unused","-Unused",//zol trib
			"-Unused","-Unused","Spin Tile","(None)","-Fairy","Other (Floating)","Other",
			"-max250",
			"Custom 01", "Custom 02", "Custom 03", "Custom 04", "Custom 05",
			"Custom 06", "Custom 07", "Custom 08", "Custom 09", "Custom 10",
			"Custom 11", "Custom 12", "Custom 13", "Custom 14", "Custom 15",
			"Custom 16", "Custom 17", "Custom 18", "Custom 19", "Custom 20",
			"Friendly NPC 01", "Friendly NPC 02", "Friendly NPC 03", "Friendly NPC 04",
			"Friendly NPC 05", "Friendly NPC 06", "Friendly NPC 07",
			"Friendly NPC 08", "Friendly NPC 09", "Friendly NPC 10"
		};
		for ( int32_t q = 0; q < eeMAX; q++ )
		{
			strcpy(moduledata.enem_type_names[q],zc_get_config_basic("ENEMIES",enemy_family_strings[q],default_enemy_types[q]));
			//al_trace("Enemy family ID %d is: %s\n", q, moduledata.enem_type_names[q]);
		}
		const char default_enemy_anims[aMAX][255] =
		{
			"(None)","Flip","-Unused","2-Frame","-Unused",
			"Octorok (NES)","Tektite (NES)","Leever (NES)","Walker","Zora (NES)",
			"Zora (4-Frame)","Ghini","Armos (NES)","Rope","Wall Master (NES)",
			"Wall Master (4-Frame)","Darknut (NES)","Vire","3-Frame","Wizzrobe (NES)",
			"Aquamentus","Dodongo (NES)","Manhandla","Gleeok","Digdogger",
			"Gohma","Lanmola","2-Frame Flying","4-Frame 4-Dir + Tracking","4-Frame 8-Dir + Tracking",
			"4-Frame 4-Dir + Firing","4-Frame 4-Dir","4-Frame 8-Dir + Firing","Armos (4-Frame)","4-Frame Flying 4-Dir",
			"4-Frame Flying 8-Dir","-Unused","4-Frame 8-Dir Big","Tektite (4-Frame)","3-Frame 4-Dir",
			"2-Frame 4-Dir","Leever (4-Frame)","2-Frame 4-Dir + Tracking","Wizzrobe (4-Frame)","Dodongo (4-Frame)",
			"Dodongo BS (4-Frame)","4-Frame Flying 8-Dir + Firing","4-Frame Flying 4-Dir + Firing","4-Frame","Ganon",
			"2-Frame Big", "4-Frame 8-Dir Big + Tracking", "4-Frame 4-Dir Big + Tracking", "4-Frame 8-Dir Big + Firing",
			"4-Frame 4-Dir Big", "4-Frame 4-Dir Big + Firing"
		};
		const char enemy_anim_strings[aMAX][255] =
		{
			"ea_none","ea_flip","ea_unused1","ea_2frame","ea_unused2",
			"ea_oct","ea_tek","ea_lev","ea_walk","ea_zor",
			"ea_zor4","ea_gh","ea_arm","ea_rp","ea_wm",
			"ea_wm4","ea_dkn","ea_vir", "ea_3f","ea_wiz",
			"ea_aqu","ea_dod","ea_mhn","ea_gkl","ea_dig",
			"ea_goh","ea_lan","ea_fly2","ea_4f4dT","ea_4f8dT",
			"ea_4f4dF","ea_4f4d","ea_4f8dF","ea_arm","ea_fly_4f4d",
			"ea_fly4f8d","ea_unused3","ea_4f8dLG","ea_tek4","ea_3f4d",
			"ea_2f4d","ea_lev4","ea_2f4dT","ea_wiz4","ea_dod4",
			"ea_bsdod","ea_fly4f4dT","ea_fly_4f4dF","ea_4f","ea_gan",
			"ea_2fLG", "ea_4f8dLGT", "ea_4f4dLGT", "ea_4f8dLGF",
			"ea_4f4dLG", "ea_4f4dLGF"
		};
		for ( int32_t q = 0; q < aMAX; q++ )
		{
			strcpy(moduledata.enem_anim_type_names[q],zc_get_config_basic("ENEMIES",enemy_anim_strings[q],default_enemy_anims[q]));
			//al_trace("Enemy animation type ID %d is: %s\n", q, moduledata.enem_anim_type_names[q]);
		}
		
		const char roomtype_cats[rMAX][256] =
		{
			"rNONE","rSP_ITEM","rINFO","rMONEY","rGAMBLE","rREPAIR","rRP_HC","rGRUMBLE",
			"rQUESTOBJ","rP_SHOP","rSHOP","rBOMBS","rSWINDLE","r10RUPIES","rWARP","rMAINBOSS","rWINGAME",
			"rITEMPOND","rMUPGRADE","rLEARNSLASH","rARROWS","rTAKEONE","rBOTTLESHOP"
		};
		const char roomtype_defaults[rMAX][255] =
		{
			"(None)","Special Item","Pay for Info","Secret Money","Gamble",
			"Door Repair","Red Potion or Heart Container","Feed the Goriya","Triforce Check",
			"Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupees",
			"3-Stair Warp","Ganon","Zelda", "-<item pond>", "1/2 Magic Upgrade", "Learn Slash",
			"More Arrows","Take One Item","Bottle Shop"
		};
		for ( int32_t q = 0; q < rMAX; q++ )
		{
			strcpy(moduledata.roomtype_names[q],zc_get_config_basic("ROOMTYPES",roomtype_cats[q],roomtype_defaults[q]));
			//al_trace("Map Flag ID %d is: %s\n", q, moduledata.roomtype_names[q]);
		}
		
		const char enemy_walk_type_defaults[e9tARMOS+1][255] =
		{
			"Normal", "Rope", "Vire", "Pols Voice", "Armos"
		};
		
		const char enemy_walk_style_cats[e9tARMOS+1][255]=
		{
			"wsNormal","wsCharge","wsHopSplit","wsHop","wsStatue"
		};
		for ( int32_t q = 0; q < e9tARMOS+1; q++ )
		{
			strcpy(moduledata.walkmisc9_names[q],zc_get_config_basic("ENEMYWALKSTYLE",enemy_walk_style_cats[q],enemy_walk_type_defaults[q]));
			//al_trace("Map Flag ID %d is: %s\n", q, moduledata.walkmisc9_names[q]);
		}
		const char guy_types[gDUMMY1][255]=
		{
			"gNONE", "gOLDMAN", "gOLDWOMAN", "gDUDE", "gORC",
			"gFIRE", "gFAIRY", "gGRUMBLE", "gPRINCESS", "gOLDMAN2",
			"gEMPTY"
		};
		
		const char guy_default_names[gDUMMY1][255]=
		{
			"(None)","Abei","Ama","Merchant","Moblin","Fire",
			"Fairy","Goriya","Zelda","Abei 2","Empty"
		};
		for ( int32_t q = 0; q < gDUMMY1; q++ )
		{
			strcpy(moduledata.guy_type_names[q],zc_get_config_basic("GUYS",guy_types[q],guy_default_names[q]));
			//al_trace("Map Flag ID %d is: %s\n", q, moduledata.guy_type_names[q]);
		}
		
		const char enemy_weapon_cats[wMax-wEnemyWeapons][255]=
		{
			"ewNone",
			"ewFireball",
			"ewArrow",
			"ewBrang",
			"ewSword",
			"ewRock",
			"ewMagic",
			"ewBomb",
			"ewSBomb",
			"ewLitBomb",
			"ewLitSBomb",
			"ewFireTrail",
			"ewFlame",
			"ewWind",
			"ewFlame2",
			"ewFlame2Trail",
			"ewIce",
			"ewFireball2"
		};
		
		const char enemy_weapon_default_names[wMax-wEnemyWeapons][255]=
		{
			"(None)",
			"Fireball",
			"Arrow",
			"Boomerang",
			"Sword",
			"Rock",
			"Magic",
			"Bomb Blast",
			"Super Bomb Blast",
			"Lit Bomb",
			"Lit Super Bomb",
			"Fire Trail",
			"Flame",
			"Wind",
			"Flame 2",
			"-Flame 2 Trail <unused>",
			"-Ice <unused>",
			"Fireball (Rising)"
		};
		
		for ( int32_t q = 0; q < sizeof(enemy_weapon_default_names)/255; q++ )
		{
			strcpy(moduledata.enemy_weapon_names[q],zc_get_config_basic("EWEAPONS",enemy_weapon_cats[q],enemy_weapon_default_names[q]));
			//al_trace("EWeapon ID %d is: %s\n", q, moduledata.enemy_weapon_names[q]);
		}
		
		
		strcpy(moduledata.enemy_scriptweaponweapon_names[0],zc_get_config_basic("EWEAPONS","Custom_1","Custom 01"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[1],zc_get_config_basic("EWEAPONS","Custom_2","Custom 02"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[2],zc_get_config_basic("EWEAPONS","Custom_3","Custom 03"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[3],zc_get_config_basic("EWEAPONS","Custom_4","Custom 04"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[4],zc_get_config_basic("EWEAPONS","Custom_5","Custom 05"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[5],zc_get_config_basic("EWEAPONS","Custom_6","Custom 06"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[6],zc_get_config_basic("EWEAPONS","Custom_7","Custom 07"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[7],zc_get_config_basic("EWEAPONS","Custom_8","Custom 08"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[8],zc_get_config_basic("EWEAPONS","Custom_9","Custom 09"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[9],zc_get_config_basic("EWEAPONS","Custom_10","Custom 10"));
		
		const char lweapon_cats[wRefFire2+1][255]=
		{
			"lwNone","lwSword","lwBeam","lwBrang","lwBomb","lwSBomb","lwLitBomb",
			"lwLitSBomb","lwArrow","lwFire","lwWhistle","lwMeat","lwWand","lwMagic","lwCatching",
			"lwWind","lwRefMagic","lwRefFireball","lwRefRock", "lwHammer","lwGrapple", "lwHSHandle", 
			"lwHSChain", "lwSSparkle","lwFSparkle", "lwSmack", "lwPhantom", 
			"lwCane","lwRefBeam", "lwStomp","","lwScript1", "lwScript2", "lwScript3", 
			"lwScript4","lwScript5", "lwScript6", "lwScript7", "lwScript8","lwScript9", "lwScript10", "lwIce",
			"-wFlame", "-wSound", "-wThrown", "-wPot", "-wLit", "-wBombos", "-wEther", "-wQuake",
			"-wSword180", "-wSwordLA", "-wBugNet", "lwRefArrow", "lwRefFire", "lwRefFire2"
		};
		const char lweapon_default_names[wRefFire2+1][255]=
		{
			"(None)","Sword","Sword Beam","Boomerang","Bomb","Super Bomb","Lit Bomb",
			"Lit Super Bomb","Arrow","Fire","Whistle","Bait","Wand","Magic","-Catching",
			"Wind","Reflected Magic","Reflected Fireball","Reflected Rock", "Hammer","Hookshot", "-HSHandle", 
			"-HSChain", "Sparkle","-FSparkle", "-Smack", "-Phantom", 
			"Cane of Byrna","Reflected Sword Beam", "-Stomp","-lwmax","Script1", "Script2", "Script3", 
			"Script4","Script5", "Script6", "Script7", "Script8","Script9", "Script10", "Ice",
			"-wFlame", "-wSound", "-wThrown", "-wPot", "-wLit", "-wBombos", "-wEther", "-wQuake",
			"-wSword180", "-wSwordLA", "-wBugNet", "Reflected Arrow", "Reflected Fire", "Reflected Fire 2"
		};
		for ( int32_t q = 0; q < wRefFire2+1; q++ )
		{
			if(lweapon_cats[q][0] != '-')
				strcpy(moduledata.player_weapon_names[q],(lweapon_cats[q][0] ? zc_get_config_basic("LWEAPONS",lweapon_cats[q],lweapon_default_names[q]) : lweapon_default_names[q]));
			//al_trace("LWeapon ID %d is: %s\n", q, moduledata.player_weapon_names[q]);
		}
		
		al_trace("Module Title: %s\n", moduledata.moduletitle);
		al_trace("Module Author: %s\n", moduledata.moduleauthor);
		al_trace("Module Info: \n%s\n%s\n%s\n%s\n%s\n", moduledata.moduleinfo0, moduledata.moduleinfo1, moduledata.moduleinfo2, moduledata.moduleinfo3, moduledata.moduleinfo4);
		//al_trace("Module Base NSF: %s\n", moduledata.module_base_nsf);
		
		al_trace("Module Version: %d.%d.%d.%d\n", moduledata.modver_1,moduledata.modver_2,moduledata.modver_3, moduledata.modver_4);
		al_trace("Module Build: %d, %s: %d\n", moduledata.modbuild, (moduledata.modbeta<0) ? "Alpha" : "Beta", moduledata.modbeta );
		
		//al_trace("Build Day: %s\n",dayextension(moduledata.modday).c_str());
		//al_trace("Build Month: %s\n",(char*)months[moduledata.modmonth]);
		//al_trace("Build Year: %d\n",moduledata.modyear);
		al_trace("Module Date: %s %s, %d at @ %d:%d %s\n", dayextension(moduledata.modday).c_str(), 
			(char*)months[moduledata.modmonth], moduledata.modyear, moduledata.modhour, moduledata.modminute, moduledata.moduletimezone);
	}
	
	//shift back to the normal config file, when done
	zc_pop_config();
	return true;
}

