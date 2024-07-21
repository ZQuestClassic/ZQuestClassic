#include "zc/ffscript.h"
#include "base/module.h"
#include "zinfo.h"

extern zcmodule moduledata;

const char * select_screen_tile_cats[sels_tile_LAST] =
{
	"sels_tile_frame", "sels_tile_questicon_1A", "sels_tile_questicon_1B", "sels_tile_questicon_2A",
	"sels_tile_questicon_2B", "sels_tile_questicon_3A", "sels_tile_questicon_3B", "sels_tile_questicon_4A",
	"sels_tile_questicon_4B", "sels_tile_questicon_5A", "sels_tile_questicon_5B", "sels_tile_questicon_6A",
	"sels_tile_questicon_6B", "sels_tile_questicon_7A", "sels_tile_questicon_7B", "sels_tile_questicon_8A",
	"sels_tile_questicon_8B", "sels_tile_questicon_9A", "sels_tile_questicon_9B", "sels_tile_questicon_10A",
	"sels_tile_questicon_10B",
		//x positions
	"sels_tile_questicon_1A_X", "sels_tile_questicon_1B_X", "sels_tile_questicon_2A_X", "sels_tile_questicon_2B_X",
	"sels_tile_questicon_3A_X", "sels_tile_questicon_3B_X", "sels_tile_questicon_4A_X", "sels_tile_questicon_4B_X",
	"sels_tile_questicon_5A_X", "sels_tile_questicon_5B_X", "sels_tile_questicon_6A_X", "sels_tile_questicon_6B_X",
	"sels_tile_questicon_7A_X", "sels_tile_questicon_7B_X", "sels_tile_questicon_8A_X", "sels_tile_questicon_8B_X",
	"sels_tile_questicon_9A_X", "sels_tile_questicon_9B_X", "sels_tile_questicon_10A_X", "sels_tile_questicon_10B_X",
	"sels_cursor_tile", "sels_heart_tile", "sels_linktile", "draw_link_first"
};

const char * select_screen_tile_cset_cats[sels_tile_LAST] =
{
	"sels_tile_frame_cset", "sels_tile_questicon_1A_cset", "sels_tile_questicon_1B_cset", "sels_tile_questicon_2A_cset",
	"sels_tile_questicon_2B_cset", "sels_tile_questicon_3A_cset", "sels_tile_questicon_3B_cset", "sels_tile_questicon_4A_cset",
	"sels_tile_questicon_4B_cset", "sels_tile_questicon_5A_cset", "sels_tile_questicon_5B_cset", "sels_tile_questicon_6A_cset",
	"sels_tile_questicon_6B_cset", "sels_tile_questicon_7A_cset", "sels_tile_questicon_7B_cset", "sels_tile_questicon_8A_cset",
	"sels_tile_questicon_8B_cset", "sels_tile_questicon_9A_cset", "sels_tile_questicon_9B_cset", "sels_tile_questicon_10A_cset",
	"sels_tile_questicon_10B_cset", "change_cset_on_quest_3", 
	"sels_cusror_cset", "sels_heart_tilettile_cset", "sels_link_cset"
};

bool ZModule::init(bool d) //bool default
{
	memset(moduledata.module_name, 0, sizeof(moduledata.module_name));
	memset(moduledata.quests, 0, sizeof(moduledata.quests));
	memset(moduledata.skipnames, 0, sizeof(moduledata.skipnames));
	memset(moduledata.datafiles, 0, sizeof(moduledata.datafiles));
	memset(moduledata.enem_type_names, 0, sizeof(moduledata.enem_type_names));
	memset(moduledata.enem_anim_type_names, 0, sizeof(moduledata.enem_anim_type_names));
	
	memset(moduledata.startingdmap, 0, sizeof(moduledata.startingdmap));
	memset(moduledata.startingscreen, 0, sizeof(moduledata.startingscreen));
	
	memset(moduledata.roomtype_names, 0, sizeof(moduledata.roomtype_names));
	memset(moduledata.walkmisc7_names, 0, sizeof(moduledata.walkmisc7_names));
	memset(moduledata.walkmisc9_names, 0, sizeof(moduledata.walkmisc9_names));
	memset(moduledata.guy_type_names, 0, sizeof(moduledata.guy_type_names));
	memset(moduledata.enemy_weapon_names, 0, sizeof(moduledata.enemy_weapon_names));
	memset(moduledata.enemy_weapon_names, 0, sizeof(moduledata.enemy_scriptweaponweapon_names));
	memset(moduledata.player_weapon_names, 0, sizeof(moduledata.player_weapon_names));
	memset(moduledata.delete_quest_data_on_wingame, 0, sizeof(moduledata.delete_quest_data_on_wingame));
	memset(moduledata.base_NSF_file, 0, sizeof(moduledata.base_NSF_file));
	memset(moduledata.copyright_strings, 0, sizeof(moduledata.copyright_strings));
	memset(moduledata.copyright_string_vars, 0, sizeof(moduledata.copyright_string_vars));
	memset(moduledata.select_screen_tiles, 0, sizeof(moduledata.select_screen_tiles));
	memset(moduledata.select_screen_tile_csets, 0, sizeof(moduledata.select_screen_tile_csets));
	moduledata.old_quest_serial_flow = 0;
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

	//We load the current module name from zc.cfg or zquest.cfg!
	//Otherwise, we don't know what file to access to load the module vars! 
	strcpy(moduledata.module_name,zc_get_config("ZCMODULE","current_module","modules/classic.zmod"));
	al_trace("The Current ZQuest Player Module is: %s\n",moduledata.module_name); 
	if(!fileexists((char*)moduledata.module_name))
	{
		Z_error("ZC Player I/O Error: No module definitions found. Please check your settings in %s.cfg.\n", "zc");
		return false;
	}
	if(!d) return true;
	//Switch to the module to load its config properties.
	zc_push_config();
	zc_config_file(moduledata.module_name);
	
	{
		
		//zcm path
		
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

		for ( auto q = 0; q < 10; q++ )
		{
			if ( moduledata.quests[q][0] == '-' )
				strcpy(moduledata.quests[q],"");
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
		for ( auto q = 0; q < 10; q++ )
		{
			if ( moduledata.skipnames[q][0] == '-' )
				strcpy(moduledata.skipnames[q],"");
		}
		
		//Quest starting screens and DMaps
	
		//dmaps
		moduledata.startingdmap[0] = zc_get_config_basic("QUESTS","first_startdmap",0);
		moduledata.startingdmap[1] = zc_get_config_basic("QUESTS","second_startdmap",0);
		moduledata.startingdmap[2] = zc_get_config_basic("QUESTS","third_startdmap",0);
		moduledata.startingdmap[3] = zc_get_config_basic("QUESTS","fourth_startdmap",0);
		moduledata.startingdmap[4] = zc_get_config_basic("QUESTS","fifth_startdmap",0);
		moduledata.startingdmap[5] = zc_get_config_basic("QUESTS","sixth_startdmap",0);
		moduledata.startingdmap[6] = zc_get_config_basic("QUESTS","seventh_startdmap",0);
		moduledata.startingdmap[7] = zc_get_config_basic("QUESTS","eighth_startdmap",0);
		moduledata.startingdmap[8] = zc_get_config_basic("QUESTS","ninth_startdmap",0);
		moduledata.startingdmap[9] = zc_get_config_basic("QUESTS","tenth_startdmap",0);
		//screens
		moduledata.startingscreen[0] = zc_get_config_basic("QUESTS","first_startscreen",0x77);
		moduledata.startingscreen[1] = zc_get_config_basic("QUESTS","second_startscreen",0x77);
		moduledata.startingscreen[2] = zc_get_config_basic("QUESTS","third_startscreen",0x77);
		moduledata.startingscreen[3] = zc_get_config_basic("QUESTS","fourth_startscreen",0x77);
		moduledata.startingscreen[4] = zc_get_config_basic("QUESTS","fifth_startscreen",0x77);
		moduledata.startingscreen[5] = zc_get_config_basic("QUESTS","sixth_startscreen",0x77);
		moduledata.startingscreen[6] = zc_get_config_basic("QUESTS","seventh_startscreen",0x77);
		moduledata.startingscreen[7] = zc_get_config_basic("QUESTS","eighth_startscreen",0x77);
		moduledata.startingscreen[8] = zc_get_config_basic("QUESTS","ninth_startscreen",0x77);
		moduledata.startingscreen[9] = zc_get_config_basic("QUESTS","tenth_startscreen",0x77);
	
		//name entry icons, tiles, and csets
		for ( int32_t q = 0; q < sels_tile_LAST; q++ ) 
		{
			moduledata.select_screen_tiles[q] = zc_get_config_basic("NAMEENTRY",select_screen_tile_cats[q],0);
		}
		for ( int32_t q = 0; q < sels_tile_cset_LAST; q++ ) 
		{
			moduledata.select_screen_tile_csets[q] = zc_get_config_basic("NAMEENTRY",select_screen_tile_cset_cats[q],( ( q == 0 || q == sels_heart_tilettile_cset || q == sels_cusror_cset )  ? 0 : 9 )); //the player icon csets are 9 by default, and the tile frame is 0
		}
		
		//datafiles
		//Hardcoded sfx.dat, fix quest sounds bug
		strcpy(moduledata.datafiles[sfx_dat],"sfx.dat");
		al_trace("Module sfx_dat set to %s\n",moduledata.datafiles[sfx_dat]);
		
		
		strcpy(moduledata.datafiles[zelda_dat],zc_get_config_basic("DATAFILES","zcplayer_datafile","zelda.dat"));
		al_trace("Module zelda_dat set to %s\n",moduledata.datafiles[zelda_dat]);
		strcpy(moduledata.datafiles[zquest_dat],zc_get_config_basic("DATAFILES","zquest_datafile","zquest.dat"));
		al_trace("Module zquest_dat set to %s\n",moduledata.datafiles[zquest_dat]);
		strcpy(moduledata.datafiles[fonts_dat],zc_get_config_basic("DATAFILES","fonts_datafile","fonts.dat"));
		al_trace("Module fonts_dat set to %s\n",moduledata.datafiles[fonts_dat]);
		
		strcpy(moduledata.base_NSF_file,zc_get_config_basic("DATAFILES","base_NSF_file","zelda.nsf"));
		al_trace("Base NSF file: %s\n", moduledata.base_NSF_file);
		
		moduledata.title_track = zc_get_config_basic("DATAFILES","title_track",0);
		moduledata.ending_track = zc_get_config_basic("DATAFILES","ending_track",1);
		moduledata.tf_track = zc_get_config_basic("DATAFILES","tf_track",5);
		moduledata.gameover_track = zc_get_config_basic("DATAFILES","gameover_track",0);
		moduledata.dungeon_track = zc_get_config_basic("DATAFILES","dungeon_track",0);
		moduledata.overworld_track = zc_get_config_basic("DATAFILES","overworld_track",0);
		moduledata.lastlevel_track = zc_get_config_basic("DATAFILES","lastlevel_track",0);
		
		strcpy(moduledata.copyright_strings[0],zc_get_config_basic("DATAFILES","copy_string_0","1986 NINTENDO"));
		if( moduledata.copyright_strings[0][0] == '-' ) strcpy(moduledata.copyright_strings[0],"");
		strcpy(moduledata.copyright_strings[1],zc_get_config_basic("DATAFILES","copy_string_1"," AG"));
		if( moduledata.copyright_strings[1][0] == '-' ) strcpy(moduledata.copyright_strings[1],"");
		//year
		strcpy(moduledata.copyright_strings[2],zc_get_config_basic("DATAFILES","copy_string_year",COPYRIGHT_YEAR));
		if( moduledata.copyright_strings[2][0] == '-' ) strcpy(moduledata.copyright_strings[1],"");
		
		
		moduledata.copyright_string_vars[titleScreen250+0] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_font",0);
		moduledata.copyright_string_vars[titleScreen250+1] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_x",80);
		moduledata.copyright_string_vars[titleScreen250+2] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_y",134);
		moduledata.copyright_string_vars[titleScreen250+3] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_col",255);
		moduledata.copyright_string_vars[titleScreen250+4] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_sz",-1);
		
		moduledata.copyright_string_vars[titleScreen250+5] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_font2",0);
		moduledata.copyright_string_vars[titleScreen250+6] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_x2",80);
		moduledata.copyright_string_vars[titleScreen250+7] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_y2",142);
		moduledata.copyright_string_vars[titleScreen250+8] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_col2",255);
		moduledata.copyright_string_vars[titleScreen250+9] = zc_get_config_basic("DATAFILES","cpystr_5frame_var_sz2",-1);
		
		moduledata.copyright_string_vars[titleScreen210+0] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_font",0);
		moduledata.copyright_string_vars[titleScreen210+1] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_x",46);
		moduledata.copyright_string_vars[titleScreen210+2] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_y",138);
		moduledata.copyright_string_vars[titleScreen210+3] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_col",255);
		moduledata.copyright_string_vars[titleScreen210+4] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_sz",-1);
		
		moduledata.copyright_string_vars[titleScreen210+5] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_font2",0);
		moduledata.copyright_string_vars[titleScreen210+6] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_x2",46);
		moduledata.copyright_string_vars[titleScreen210+7] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_y2",146);
		moduledata.copyright_string_vars[titleScreen210+8] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_col2",255);
		moduledata.copyright_string_vars[titleScreen210+9] = zc_get_config_basic("DATAFILES","cpystr_4frame_var_sz2",-1);
		
		moduledata.copyright_string_vars[titleScreenMAIN+0] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_font",0);
		moduledata.copyright_string_vars[titleScreenMAIN+1] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_x",86);
		moduledata.copyright_string_vars[titleScreenMAIN+2] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_y",128);
		moduledata.copyright_string_vars[titleScreenMAIN+3] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_col",13);
		moduledata.copyright_string_vars[titleScreenMAIN+4] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_sz",-1);
		
		moduledata.copyright_string_vars[titleScreenMAIN+5] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_font2",0);
		moduledata.copyright_string_vars[titleScreenMAIN+6] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_x2",86);
		moduledata.copyright_string_vars[titleScreenMAIN+7] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_y2",136);
		moduledata.copyright_string_vars[titleScreenMAIN+8] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_col2",13);
		moduledata.copyright_string_vars[titleScreenMAIN+9] = zc_get_config_basic("DATAFILES","cpystr_1frame_var_sz2",-1);
		
		moduledata.animate_NES_title =  zc_get_config_basic("DATAFILES","disable_title_NES_animation",0);
		
		static const char roomtype_cats[rMAX][256] =
		{
			"rNONE","rSP_ITEM","rINFO","rMONEY","rGAMBLE","rREPAIR","rRP_HC","rGRUMBLE",
			"rQUESTOBJ","rP_SHOP","rSHOP","rBOMBS","rSWINDLE","r10RUPIES","rWARP","rMAINBOSS","rWINGAME",
			"rITEMPOND","rMUPGRADE","rLEARNSLASH","rARROWS","rTAKEONE","rBOTTLESHOP"
		};
		static const char roomtype_defaults[rMAX][255] =
		{
			"(None)","Special Item","Pay for Info","Secret Money","Gamble",
			"Door Repair","Red Potion or Heart Container","Feed the Goriya","Level 9 Entrance",
			"Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupees",
			"3-Stair Warp","Ganon","Zelda", "-<item pond>", "1/2 Magic Upgrade", "Learn Slash",
			"More Arrows","Take One Item","Bottle Shop"
		};
		for ( int32_t q = 0; q < rMAX; q++ )
		{
			strcpy(moduledata.roomtype_names[q],zc_get_config_basic("ROOMTYPES",roomtype_cats[q],roomtype_defaults[q]));
		}
		static const char lweapon_cats[wRefFire2+1][255]=
		{
			"lwNone","lwSword","lwBeam","lwBrang","lwBomb","lwSBomb","lwLitBomb",
			"lwLitSBomb","lwArrow","lwFire","lwWhistle","lwMeat","lwWand","lwMagic","lwCatching",
			"lwWind","lwRefMagic","lwRefFireball","lwRefRock", "lwHammer","lwGrapple", "lwHSHandle", 
			"lwHSChain", "lwSSparkle","lwFSparkle", "lwSmack", "lwPhantom", 
			"lwCane","lwRefBeam", "lwStomp","lwScript1", "lwScript2", "lwScript3", 
			"lwScript4","lwScript5", "lwScript6", "lwScript7", "lwScript8","lwScript9", "lwScript10", "lwIce",
			"-wFlame", "-wSound", "-wThrown", "-wPot", "-wLit", "-wBombos", "-wEther", "-wQuake",
			"-wSword180", "-wSwordLA", "-wBugNet", "wRefArrow", "wRefFire", "wRefFire2"
		};
		static const char lweapon_default_names[wRefFire2+1][255]=
		{
			"(None)","Sword","Sword Beam","Boomerang","Bomb","Super Bomb","Lit Bomb",
			"Lit Super Bomb","Arrow","Fire","Whistle","Bait","Wand","Magic","-Catching",
			"Wind","Reflected Magic","Reflected Fireball","Reflected Rock", "Hammer","Hookshit", "-HSHandle", 
			"-HSChain", "Sparkle","-FSparkle", "-Smack", "-Phantom", 
			"Cane of Byrna","Reflected Sword Beam", "-Stomp","Script1", "Script2", "Script3", 
			"Script4","Script5", "Script6", "Script7", "Script8","Script9", "Script10", "Ice",
			"-wFlame", "-wSound", "-wThrown", "-wPot", "-wLit", "-wBombos", "-wEther", "-wQuake",
			"-wSword180", "-wSwordLA", "-wBugNet", "Reflected Arrow", "Reflected Fire", "Reflected Fire 2"
		};
		for ( int32_t q = 0; q < wRefFire2+1; q++ )
		{
			if(lweapon_cats[q][0] != '-')
				strcpy(moduledata.player_weapon_names[q],zc_get_config_basic("LWEAPONS",lweapon_cats[q],lweapon_default_names[q]));
		}
		static const char counter_cats[33][255]=
		{
			"crNONE","crLIFE","crMONEY","crBOMBS","crARROWS","crMAGIC","crKEYS",
			"crSBOMBS","crCUSTOM1","crCUSTOM2","crCUSTOM3","crCUSTOM4","crCUSTOM5","crCUSTOM6",
			"crCUSTOM7","crCUSTOM8","crCUSTOM9","crCUSTOM10","crCUSTOM11","crCUSTOM12","crCUSTOM13",
			"crCUSTOM14","crCUSTOM15","crCUSTOM16","crCUSTOM17","crCUSTOM18","crCUSTOM19",
			"crCUSTOM20","crCUSTOM21","crCUSTOM22","crCUSTOM23","crCUSTOM24","crCUSTOM25"
		};
		
		al_trace("Module Title: %s\n", moduledata.moduletitle);
		al_trace("Module Author: %s\n", moduledata.moduleauthor);
		al_trace("Module Info: \n%s\n%s\n%s\n%s\n%s\n", moduledata.moduleinfo0, moduledata.moduleinfo1, moduledata.moduleinfo2, moduledata.moduleinfo3, moduledata.moduleinfo4);
		
		al_trace("Module Version: %d.%d.%d.%d\n", moduledata.modver_1,moduledata.modver_2,moduledata.modver_3, moduledata.modver_4);
		al_trace("Module Build: %d, %s: %d\n", moduledata.modbuild, (moduledata.modbeta<0) ? "Alpha" : "Beta", moduledata.modbeta );
		
		al_trace("Build Date: %s %s, %d at @ %d:%d %s\n", dayextension(moduledata.modday).c_str(), 
			(char*)months[moduledata.modmonth], moduledata.modyear, moduledata.modhour, moduledata.modminute, moduledata.moduletimezone);
		
	}
	
	//shift back to the normal config file, when done
	zc_pop_config();
	return true;
}
