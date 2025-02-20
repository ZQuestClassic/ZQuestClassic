#include "base/module.h"
#include "base/zdefs.h"
#include "base/zsys.h"
#include "zinfo.h"

extern zcmodule moduledata;

bool ZModule::init(bool d) //bool default
{
	memset(moduledata.module_name, 0, sizeof(moduledata.module_name));
	memset(moduledata.datafiles, 0, sizeof(moduledata.datafiles));
	memset(moduledata.roomtype_names, 0, sizeof(moduledata.roomtype_names));
	memset(moduledata.base_NSF_file, 0, sizeof(moduledata.base_NSF_file));
	memset(moduledata.copyright_strings, 0, sizeof(moduledata.copyright_strings));
	memset(moduledata.copyright_string_vars, 0, sizeof(moduledata.copyright_string_vars));
	memset(moduledata.delete_quest_data_on_wingame, 0, sizeof(moduledata.delete_quest_data_on_wingame));
	memset(moduledata.select_screen_tile_csets, 0, sizeof(moduledata.select_screen_tile_csets));
	memset(moduledata.select_screen_tiles, 0, sizeof(moduledata.select_screen_tiles));
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
		
		//datafiles
		strcpy(moduledata.datafiles[zelda_dat],zc_get_config_basic("DATAFILES","zcplayer_datafile","zelda.dat"));
		al_trace("Module zelda_dat set to %s\n",moduledata.datafiles[zelda_dat]);
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
		
		al_trace("Module Title: %s\n", moduledata.moduletitle);
		al_trace("Module Author: %s\n", moduledata.moduleauthor);
		al_trace("Module Info: \n%s\n%s\n%s\n%s\n%s\n", moduledata.moduleinfo0, moduledata.moduleinfo1, moduledata.moduleinfo2, moduledata.moduleinfo3, moduledata.moduleinfo4);
		
		al_trace("Module Version: %d.%d.%d.%d\n", moduledata.modver_1,moduledata.modver_2,moduledata.modver_3, moduledata.modver_4);
		al_trace("Module Build: %d, %s: %d\n", moduledata.modbuild, "Beta", moduledata.modbeta );
		
		al_trace("Module Date: %s %s, %d at @ %d:%d %s\n", dayextension(moduledata.modday).c_str(), 
			(char*)months[moduledata.modmonth], moduledata.modyear, moduledata.modhour, moduledata.modminute, moduledata.moduletimezone);
	}
	
	//shift back to the normal config file, when done
	zc_pop_config();
	return true;
}
