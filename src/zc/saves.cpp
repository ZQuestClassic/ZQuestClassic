#include "zc/saves.h"

#include "base/packfile.h"
#include "base/misctypes.h"
#include "base/fonts.h"
#include "base/dmap.h"
#include "zc/zelda.h"
#include "zc/ffscript.h"
#include "pal.h"
#include "tiles.h"
#include "items.h"
#include "jwin.h"
#include <vector>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

extern FFScript FFCore;

static const char *SAVE_HEADER = "Zelda Classic Save File";
static const char *OLD_SAVE_HEADER = "ZQuest Classic Save File";
static int currgame;
static std::vector<gamedata> saves;

static std::string get_save_file_path()
{
	std::string save_file_name = zc_get_config("SAVEFILE", "save_filename", "zc.sav");
#ifdef __EMSCRIPTEN__
		// There was a bug that causes browser zc.cfg files to use the wrong value for the save file.
		if (save_file_name == "zc.sav")
			save_file_name = "/local/zc.sav";
#endif
	return save_file_name;
}

static int32_t read_saves(PACKFILE *f)
{
	FFCore.kb_typing_mode = false;
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
		Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
		midi_volume = FFCore.usr_midi_volume;
		//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
		digi_volume = FFCore.usr_digi_volume;
		//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
		emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
		sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
		pan_style = (int32_t)FFCore.usr_panstyle;
	}
	FFCore.skip_ending_credits = 0;
	//word item_count;
	word qstpath_len=0;
	word save_count=0;
	char name[9]={0};
	byte tempbyte = 0;
	int16_t tempshort = 0;
	//  int32_t templong;
	word tempword = 0;
	word tempword2 = 0;
	word tempword3 = 0;
	word tempword4 = 0;
	word tempword5 = 0;
	dword tempdword = 0;
	int32_t templong = 0;
	int32_t section_id=0;
	word section_version=0;
	word section_cversion=0;
	dword section_size = 0;
	
	//section id
	if(!p_mgetl(&section_id,f))
	{
		return 1;
	}
	
	//section version info
	if(!p_igetw(&section_version,f))
	{
		return 2;
	}
	
	if(!p_igetw(&section_cversion,f))
	{
		return 3;
	}
	
	if(section_version < 11) //Sorry!
	{
		//Currently unsupported
		return 1;
	}
	
	//section size
	if(!p_igetl(&section_size,f))
	{
		return 4;
	}
	
	if(!p_igetw(&save_count,f))
	{
		return 5;
	}
	
	// Excess saves would get deleted, so...
	if(standalone_mode && save_count>1)
	{
		enter_sys_pal();
		jwin_alert("Invalid save file",
				   "This save file cannot be",
				   "used in standalone mode.",
				   "",
				   "OK",NULL,'o',0,get_zc_font(font_lfont));
		exit(0);
	}

	saves.clear();
	saves.resize(save_count);

	for(int32_t i=0; i<save_count; i++)
	{
		gamedata& save = saves[i];

		if(!pfread(name,9,f))
		{
			return 6;
		}
		
		save.set_name(name);
		
		if(!p_getc(&tempbyte,f))
		{
			return 7;
		}
		
		save.set_quest(tempbyte);
		
		if(section_version<3)
		{
			if(!p_igetw(&tempword,f))
			{
				return 8;
			}
			
			save.set_counter(tempword, 0);
			save.set_dcounter(tempword, 0);
			
			if(!p_igetw(&tempword,f))
			{
				return 9;
			}
			
			save.set_maxcounter(tempword, 0);
			
			if(!p_igetw(&tempshort,f))
			{
				return 10;
			}
			
			save.set_dcounter(tempshort, 1);
			
			if(!p_igetw(&tempword,f))
			{
				return 11;
			}
			
			save.set_counter(tempword, 1);
			
			if(!p_igetw(&tempword,f))
			{
				return 12;
			}
			
			save.set_counter(tempword, 3);
			save.set_dcounter(tempword, 3);
		}
		
		if(!p_igetw(&tempword,f))
		{
			return 13;
		}
		
		save.set_deaths(tempword);
		
		if(section_version<3)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 14;
			}
			
			save.set_counter(tempbyte, 5);
			save.set_dcounter(tempbyte, 5);
			
			if(!p_getc(&tempbyte,f))
			{
				return 15;
			}
			
			save.set_maxcounter(tempbyte, 2);
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 16;
			}
			
			save.set_wlevel(tempbyte);
		}
		
		if(!p_getc(&tempbyte,f))
		{
			return 17;
		}
		
		if (section_version < 24) tempbyte = (tempbyte ? DIDCHEAT_BIT : 0);
		save._cheat = tempbyte;
		
		char temp;
		
		for(int32_t j=0; j<MAXITEMS; j++) // why not MAXITEMS ?
		{
			if(!p_getc(&temp, f))
				return 18;
				
			save.set_item_no_flush(j, (temp != 0));
		}
		
		size_t versz = section_version<31 ? 9 : 16;
		if(!pfread(save.version,versz,f))
		{
			return 20;
		}
		
		if(!pfread(save.title,sizeof(save.title),f))
		{
			return 21;
		}
		
		if(!p_getc(&tempbyte,f))
		{
			return 22;
		}
		
		save.set_hasplayed(tempbyte);
		
		if(!p_igetl(&tempdword,f))
		{
			return 23;
		}
		
		save.set_time(tempdword);
		
		if(!p_getc(&tempbyte,f))
		{
			return 24;
		}
		
		save.set_timevalid(tempbyte);
		
		if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXLEVELS; ++j)
			{
				if(!p_getc(&(save.lvlitems[j]),f))
				{
					return 25;
				}
			}
		}
		else
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
			{
				if(!p_getc(&(save.lvlitems[j]),f))
				{
					return 25;
				}
			}
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 26;
			}
			
			save.set_HCpieces(tempbyte);
		}
		
		if(!p_getc(&tempbyte,f))
		{
			return 27;
		}
		
		save.set_continue_scrn(tempbyte);
		
		if(section_version <= 5)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 28;
			}
			
			save.set_continue_dmap(tempbyte);
		}
		else
		{
			if(!p_igetw(&tempword,f))
			{
				return 28;
			}
			
			save.set_continue_dmap(tempword);
		}
		
		if(section_version<3)
		{
			if(!p_igetw(&tempword,f))
			{
				return 29;
			}
			
			save.set_counter(tempword, 4);
			
			if(!p_igetw(&tempword,f))
			{
				return 30;
			}
			
			save.set_maxcounter(tempword, 4);
			
			if(!p_igetw(&tempshort,f))
			{
				return 31;
			}
			
			save.set_dcounter(tempshort, 4);
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 32;
			}
			
			save.set_magicdrainrate(tempbyte);
			
			if(!p_getc(&tempbyte,f))
			{
				return 33;
			}
			
			save.set_canslash(tempbyte);
		}
		
		if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXDMAPS; ++j)
			{
				if(!p_getc(&(save.visited[j]),f))
				{
					return 34;
				}
			}
			
			for(int32_t j=0; j<OLDMAXDMAPS*64; ++j)
			{
				byte tempBMaps[OLDMAXDMAPS*64] = {0};
				for(int32_t j=0; j<OLDMAXDMAPS*64; ++j)
				{
					if(!p_getc(&(tempBMaps[j]),f))
					{
						return 35;
					}
				}
				std::fill(save.bmaps, save.bmaps + MAXDMAPS*128, 0);
				for(int32_t dm = 0; dm < OLDMAXDMAPS; ++dm)
				{
					for(int32_t scr = 0; scr < 128; ++scr)
					{
						int32_t di = (dm<<7) + (scr & 0x70) + (scr&15)-(DMaps[dm].type==dmOVERW ? 0 : DMaps[dm].xoff); //New Calculation
						if(((unsigned)((scr&15)-DMaps[dm].xoff)) > 7) 
							continue;
						int32_t si = ((dm-1)<<6) + ((scr>>4)<<3) + ((scr&15)-DMaps[dm].xoff); //Old Calculation
						if(si < 0)
						{
							save.bmaps[di] = save.visited[512+si]&0x8F; //Replicate bug; OOB indexes
							continue;
						}
						save.bmaps[di] = tempBMaps[si]&0x8F;
					}
				}
			}
		}
		else
		{
			for(int32_t j=0; j<MAXDMAPS; ++j)
			{
				if(!p_getc(&(save.visited[j]),f))
				{
					return 34;
				}
			}
			
			if(section_version < 17)
			{
				byte tempBMaps[MAXDMAPS*64] = {0};
				for(int32_t j=0; j<MAXDMAPS*64; ++j)
				{
					if(!p_getc(&(tempBMaps[j]),f))
					{
						return 35;
					}
				}
				std::fill(save.bmaps, save.bmaps + MAXDMAPS*128, 0);
				for(int32_t dm = 0; dm < MAXDMAPS; ++dm)
				{
					for(int32_t scr = 0; scr < 128; ++scr)
					{
						int32_t di = (dm<<7) + (scr & 0x70) + (scr&15)-(DMaps[dm].type==dmOVERW ? 0 : DMaps[dm].xoff); //New Calculation
						if(((unsigned)((scr&15)-DMaps[dm].xoff)) > 7) 
							continue;
						int32_t si = ((dm-1)<<6) + ((scr>>4)<<3) + ((scr&15)-DMaps[dm].xoff); //Old Calculation
						if(si < 0)
						{
							save.bmaps[di] = save.visited[512+si]&0x8F; //Replicate bug; OOB indexes
							continue;
						}
						save.bmaps[di] = tempBMaps[si]&0x8F;
					}
				}
			}
			else
			{
				for(int32_t j=0; j<MAXDMAPS*128; ++j)
				{
					if(!p_getc(&(save.bmaps[j]),f))
					{
						return 35;
					}
				}
			}
		}
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
		{
			if(!p_igetw(&save.maps[j],f))
			{
				return 36;
			}
		}
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; ++j)
		{
			if(!p_getc(&(save.guys[j]),f))
			{
				return 37;
			}
		}
		
		if(!p_igetw(&qstpath_len,f))
		{
			return 38;
		}
		
		if(!pfread(save.qstpath,qstpath_len,f))
		{
			return 39;
		}

		// TODO: this breaks even if the file is the same but absolute/relative path fails a
		// strict string compare. should resolve both of these to be absolute ... or match
		// how load_quest works ...
		if(standalone_mode && strcmp(save.qstpath, standalone_quest)!=0)
		{
			enter_sys_pal();
			jwin_alert("Invalid save file",
					   "This save file is for",
					   "a different quest.",
					   "",
					   "OK",NULL,'o',0,get_zc_font(font_lfont));
			exit(0);
		}
		
		// Convert path separators so save files work across platforms (hopefully)
		regulate_path(save.qstpath);
		
		save.qstpath[qstpath_len]=0;
		
		if(!pfread(save.icon,sizeof(save.icon),f))
		{
			return 40;
		}
		
		if(!pfread(save.pal,sizeof(save.pal),f))
		{
			return 41;
		}
		
		if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXLEVELS; ++j)
			{
				if(!p_getc(&(save.lvlkeys[j]),f))
				{
					return 42;
				}
			}
		}
		else
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
			{
				if(!p_getc(&(save.lvlkeys[j]),f))
				{
					return 42;
				}
			}
		}
		
		if(section_version>1)
		{
			if(section_version <= 5)
			{
				for(int32_t j=0; j<OLDMAXDMAPS*64; j++)
				{
					for(int32_t k=0; k<8; k++)
					{
						if(!p_igetl(&save.screen_d[j][k],f))
						{
							return 43;
						}
					}
				}
			}
			else if(section_version < 10)
			{
				for(int32_t j=0; j<MAXDMAPS*64; j++)
				{
					for(int32_t k=0; k<8; k++)
					{
						if(!p_igetl(&save.screen_d[j][k],f))
						{
							return 43;
						}
					}
				}
			}
			else
			{
				for(int32_t j=0; j<MAX_MI; j++)
				{
					for(int32_t k=0; k<8; k++)
					{
						if(!p_igetl(&save.screen_d[j][k],f))
						{
							return 43;
						}
					}
				}
			}
			if ( section_version >= 12 && FFCore.getQuestHeaderInfo(vZelda) >= 0x253 || section_version >= 16)
			/* 2.53.1 also have a v12 for this section. 
			I needed to path this to ensure that the s_v is specific to the build.
			I also skipped 13 to 15 so that 2.53.1 an use these if needed with the current patch. -Z
			*/
			{
				for(int32_t j=0; j<MAX_SCRIPT_REGISTERS; j++)
				{
				if(!p_igetl(&save.global_d[j],f))
				{
					return 45;
				}
				}
			}
			else
			{
				for(int32_t j=0; j<256; j++)
				{
					if(!p_igetl(&save.global_d[j],f))
					{
						return 45;
					}
				}
			}
		}
		
		if(section_version>2)
		{
			for(int32_t j=0; j<32; j++)
			{
				if(!p_igetw(&tempword,f))
				{
					return 46;
				}
				
				save.set_counter(tempword, j);
				
				if(!p_igetw(&tempword,f))
				{
					return 47;
				}
				
				save.set_maxcounter(tempword, j);
				
				if(!p_igetw(&tempshort,f))
				{
					return 48;
				}
				
				save.set_dcounter(tempshort, j);
			}
		}
		
		if(section_version>19)
		{
			for(int32_t j=0; j<256; j++)
			{
				if(!p_igetl(&templong,f))
				{
					return 49;
				}
				
				save.set_generic(templong, j);
			}
		}
		else if(section_version>3)
		{
			for(int32_t j=0; j<256; j++)
			{
				if(!p_getc(&tempbyte,f))
				{
					return 49;
				}
				
				save.set_generic(tempbyte, j);
			}
		}
		
		if(section_version>6)
		{
			if(!p_getc(&tempbyte, f))
			{
				return 50;
			}
			
			save.awpn = tempbyte;
			
			if(!p_getc(&tempbyte, f))
			{
				return 51;
			}
			
			save.bwpn = tempbyte;
		}
		else
		{
			save.awpn = 0;
			save.bwpn = 0;
		}
		
		//First we get the size of the vector
		if(!p_igetl(&tempdword, f))
			return 53;
			
		if(tempdword != 0) //Might not be any at all
		{
			//Then we allocate the vector
			save.globalRAM.resize(tempdword);
			
			for(dword j = 0; j < save.globalRAM.size(); j++)
			{
				ZScriptArray& a = save.globalRAM[j];
				
				//We get the size of each container
				if(!p_igetl(&tempdword, f))
					return 54;
					
				//We allocate the container
				a.Resize(tempdword);
				
				//And then fill in the contents
				for(dword k = 0; k < a.Size(); k++)
					if(!p_igetl(&(a[k]), f))
						return 55;
			}
		}
		if((section_version > 11 && FFCore.getQuestHeaderInfo(vZelda) < 0x255) || (section_version > 15 && FFCore.getQuestHeaderInfo(vZelda) >= 0x255))
		{
			if(!p_igetw(&tempword2, f))
			{
				return 56;
			}
			
			save.forced_awpn = tempword2;
			
			if(!p_igetw(&tempword3, f))
			{
				return 57;
			}
			
			save.forced_bwpn = tempword3;
		}
		else
		{
			save.forced_awpn = -1;
			save.forced_bwpn = -1;
		}
		if (section_version > 17)
		{
			
			if(!p_getc(&tempbyte, f))
			{
				return 58;
			}
			
			save.xwpn = tempbyte;
			
			if(!p_getc(&tempbyte, f))
			{
				return 59;
			}
			
			save.ywpn = tempbyte;
		
			
			if(!p_igetw(&tempword3, f))
			{
				return 60;
			}
			
			save.forced_xwpn = tempword3;
			
			if(!p_igetw(&tempword4, f))
			{
				return 61;
			}
			
			save.forced_ywpn = tempword4;
		}
		else
		{
			save.xwpn = 0;
			save.ywpn = 0;
			save.forced_xwpn = -1;
			save.forced_ywpn = -1;
		}
		
		if(section_version >= 19)
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
			{
				if(!p_igetl(&(save.lvlswitches[j]),f))
				{
					return 62;
				}
			}
		}
		else
		{
			std::fill(save.lvlswitches, save.lvlswitches+MAXLEVELS, 0);
		}
		if(section_version >= 21)
		{
			for(int32_t j=0; j<MAXITEMS; ++j)
			{
				if(!p_getc(&(save.item_messages_played[j]),f))
				{
					return 63;
				}
			}
		}
		else 
		{
			std::fill(save.item_messages_played, save.item_messages_played+MAXITEMS, 0);
		}
		if(section_version >= 22)
		{
			for(int32_t j=0; j<256; ++j)
			{
				if(!p_getc(&(save.bottleSlots[j]),f))
				{
					return 64;
				}
			}
		}
		else 
		{
			memset(save.bottleSlots, 0, sizeof(save.bottleSlots));
		}
		if(section_version >= 23)
		{
			if(!p_igetw(&(save.saved_mirror_portal.destdmap), f))
			{
				return 65;
			}
			if(!p_igetw(&(save.saved_mirror_portal.srcdmap), f))
			{
				return 66;
			}
			if(!p_getc(&(save.saved_mirror_portal.srcscr),f))
			{
				return 67;
			}
			if(section_version >= 32)
			{
				if(!p_getc(&(save.saved_mirror_portal.destscr),f))
				{
					return 67;
				}
			}
			else save.saved_mirror_portal.destscr = save.saved_mirror_portal.srcscr;
			if(!p_igetl(&(save.saved_mirror_portal.x), f))
			{
				return 68;
			}
			if(!p_igetl(&(save.saved_mirror_portal.y), f))
			{
				return 69;
			}
			if(!p_getc(&(save.saved_mirror_portal.sfx),f))
			{
				return 70;
			}
			if(!p_igetl(&(save.saved_mirror_portal.warpfx), f))
			{
				return 71;
			}
			if(!p_igetw(&(save.saved_mirror_portal.spr), f))
			{
				return 72;
			}
		}
		else save.saved_mirror_portal.clear();
		
		save.clear_genscript();
		
		word num_gen_scripts;
		if(section_version >= 25)
		{
			byte dummybyte;
			if(!p_igetw(&num_gen_scripts, f))
			{
				return 73;
			}
			for(size_t q=0; q<num_gen_scripts; q++)
			{
				if(!p_getc(&dummybyte,f))
					return 74;
				save.gen_doscript[q] = dummybyte!=0;
				if(!p_igetw(&(save.gen_exitState[q]),f))
					return 75;
				if(!p_igetw(&(save.gen_reloadState[q]),f))
					return 76;
				for(size_t ind = 0; ind < 8; ++ind)
					if(!p_igetl(&(save.gen_initd[q][ind]),f))
						return 77;
				int32_t sz;
				if(!p_igetl(&sz,f))
					return 78;
				save.gen_dataSize[q] = sz;
				save.gen_data[q].resize(sz, 0);
				for(auto ind = 0; ind < sz; ++ind)
					if(!p_igetl(&(save.gen_data[q][ind]),f))
						return 79;
			}
		}
		
		if(section_version >= 26)
		{
			for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
			{
				if(!p_igetl(&(save.xstates[j]),f))
				{
					return 78;
				}
			}
		}
		else
		{
			std::fill(save.xstates, save.xstates+(MAXMAPS2*MAPSCRSNORMAL), 0);
		}
		
		std::fill(save.gen_eventstate, save.gen_eventstate+NUMSCRIPTSGENERIC, 0);
		if(section_version >= 27)
		{
			for(size_t q=0; q<num_gen_scripts; q++)
			{
				if(!p_igetl(&save.gen_eventstate[q],f))
					return 78;
			}
		}
		if(section_version >= 28)
		{
			for(size_t q = 0; q < NUM_GSWITCHES; ++q)
			{
				if(!p_igetl(&save.gswitch_timers[q],f))
					return 79;
			}
		}
		else
		{
			std::fill(save.gswitch_timers, save.gswitch_timers+NUM_GSWITCHES, 0);
		}
		if(section_version >= 29)
		{
			word replay_path_len;
			if(!p_igetw(&replay_path_len, f))
				return 80;

			auto buf = std::make_unique<char[]>(replay_path_len + 1);
			buf[replay_path_len] = '\0';
			if (!pfread(buf.get(), replay_path_len, f))
				return 81;
			save.replay_file = buf.get();

			// TODO why doesn't this work?
			// save.replay_file.reserve(replay_path_len + 1);
			// if (!pfread(save.replay_file.data(), replay_path_len, f))
			// 	return 81;
		}
		else
		{
			save.replay_file = "";
		}
		if(section_version >= 30)
		{
			uint32_t sz;
			if(!p_igetl(&sz,f))
				return 83;
			for(uint32_t objind = 0; objind < sz; ++objind)
			{
				saved_user_object& s_ob = save.user_objects.emplace_back();
				if(!p_igetl(&s_ob.object_index,f))
					return 84;
				//user_object
				user_object& obj = s_ob.obj;
				if(!p_getc(&tempbyte,f))
					return 85;
				obj.reserved = tempbyte!=0;
				//Don't need to save owned_type,owned_i?
				uint32_t datsz;
				if(!p_igetl(&datsz,f))
					return 86;
				for(uint32_t q = 0; q < datsz; ++q)
				{
					if(!p_igetl(&templong,f))
						return 87;
					obj.data.push_back(templong);
				}
				if(!p_igetl(&obj.owned_vars,f))
					return 88;
				//scr_func_exec
				scr_func_exec& exec = obj.destruct;
				if(!p_igetl(&exec.pc,f))
					return 89;
				if(!p_igetl(&exec.thiskey,f))
					return 90;
				if(!p_igetl(&exec.type,f))
					return 91;
				if(!p_igetl(&exec.i,f))
					return 92;
				if(!p_igetw(&exec.script,f))
					return 93;
				if(!p_getwstr(&exec.name,f))
					return 94;
				//array data map
				auto& map = s_ob.held_arrays;
				uint32_t arrcount;
				if(!p_igetl(&arrcount,f))
					return 95;
				for(uint32_t ind = 0; ind < arrcount; ++ind)
				{
					int32_t arr_index;
					if(!p_igetl(&arr_index,f))
						return 96;
					
					uint32_t arrsz;
					if(!p_igetl(&arrsz,f))
						return 97;
					ZScriptArray zsarr;
					zsarr.Resize(arrsz);
					for(uint32_t q = 0; q < arrsz; ++q)
					{
						if(!p_igetl(&templong,f))
							return 98;
						zsarr[q] = templong;
					}
					map[arr_index] = zsarr;
				}
			}
		}
		if(section_version >= 32)
		{
			uint32_t sz;
			if(!p_igetl(&sz,f))
				return 99;
			for(uint32_t q = 0; q < sz; ++q)
			{
				savedportal& p = save.user_portals.emplace_back();
				if(!p_igetw(&(p.destdmap), f))
					return 100;
				if(!p_igetw(&(p.srcdmap), f))
					return 101;
				if(!p_getc(&(p.srcscr),f))
					return 102;
				if(!p_getc(&(p.destscr),f))
					return 103;
				if(!p_igetl(&(p.x), f))
					return 104;
				if(!p_igetl(&(p.y), f))
					return 105;
				if(!p_getc(&(p.sfx),f))
					return 106;
				if(!p_igetl(&(p.warpfx), f))
					return 107;
				if(!p_igetw(&(p.spr), f))
					return 108;
			}
		}
	}
	
	
	return 0;
}

static int32_t write_saves(PACKFILE *f)
{
	int32_t section_id=ID_SAVEGAME;
	int32_t section_version=V_SAVEGAME;
	int32_t section_cversion=CV_SAVEGAME;
	int32_t section_size=0;
	
	//section id
	if(!p_mputl(section_id,f))
	{
		return 1;
	}
	
	//section version info
	if(!p_iputw(section_version,f))
	{
		return 2;
	}
	
	if(!p_iputw(section_cversion,f))
	{
		return 3;
	}
	
	//section size
	if(!p_iputl(section_size,f))
	{
		return 4;
	}
	
	//word item_count=MAXITEMS;
	word qstpath_len=0;

	word count = saves_count();
	
	if(!p_iputw(count,f))
	{
		return 5;
	}
	
	for(int32_t i=0; i < count; i++)
	{
		gamedata& save = saves[i];

		qstpath_len=(word)strlen(save.qstpath);
		
		if(!pfwrite(save.get_name_mutable(),9,f))
		{
			return 6;
		}
		
		if(!p_putc(save.get_quest(),f))
		{
			return 7;
		}
		
		if(!p_iputw(save.get_deaths(),f))
		{
			return 13;
		}
		
		if(!p_putc(save._cheat,f))
		{
			return 17;
		}
		
		for(int32_t j=0; j<MAXITEMS; j++)
		{
			if(!p_putc(save.get_item(j) ? 1 : 0,f))
				return 18;
		}
		
		if(!pfwrite(save.version,16,f))
		{
			return 20;
		}
		
		if(!pfwrite(save.title,sizeof(save.title),f))
		{
			return 21;
		}
		
		if(!p_putc(save.get_hasplayed(),f))
		{
			return 22;
		}
		
		if(!p_iputl(save.get_time(),f))
		{
			return 23;
		}
		
		if(!p_putc(save.get_timevalid(),f))
		{
			return 24;
		}
		
		if(!pfwrite(save.lvlitems,MAXLEVELS,f))
		{
			return 25;
		}
		
		if(!p_putc(save.get_continue_scrn(),f))
		{
			return 27;
		}
		
		if(!p_iputw(save.get_continue_dmap(),f))
		{
			return 28;
		}
		
		if(!pfwrite(save.visited,MAXDMAPS,f))
		{
			return 34;
		}
		
		if(!pfwrite(save.bmaps,MAXDMAPS*128,f))
		{
			return 35;
		}
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
		{
			if(!p_iputw(save.maps[j],f))
			{
				return 36;
			}
		}
		
		if(!pfwrite(save.guys,MAXMAPS2*MAPSCRSNORMAL,f))
		{
			return 37;
		}
		
		if(!p_iputw(qstpath_len,f))
		{
			return 38;
		}
		
		if(!pfwrite(save.qstpath,qstpath_len,f))
		{
			return 39;
		}
		
		if(!pfwrite(save.icon,sizeof(save.icon),f))
		{
			return 40;
		}
		
		if(!pfwrite(save.pal,sizeof(save.pal),f))
		{
			return 41;
		}
		
		if(!pfwrite(save.lvlkeys,MAXLEVELS,f))
		{
			return 42;
		}
		
		for(int32_t j=0; j<MAX_MI; j++)
		{
			for(int32_t k=0; k<8; k++)
			{
				if(!p_iputl(save.screen_d[j][k],f))
				{
					return 43;
				}
			}
		}
		
		for(int32_t j=0; j<MAX_SCRIPT_REGISTERS; j++)
		{
			if(!p_iputl(save.global_d[j],f))
			{
				return 44;
			}
		}
		
		for(int32_t j=0; j<32; j++)
		{
			if(!p_iputw(save.get_counter(j), f))
			{
				return 45;
			}
			
			if(!p_iputw(save.get_maxcounter(j), f))
			{
				return 46;
			}
			
			if(!p_iputw(save.get_dcounter(j), f))
			{
				return 47;
			}
		}
		
		for(int32_t j=0; j<256; j++)
		{
			if(!p_iputl(save.get_generic(j), f))
			{
				return 48;
			}
		}
		
		if(!p_putc(save.awpn, f))
		{
			return 49;
		}
		
		if(!p_putc(save.bwpn, f))
		{
			return 50;
		}
		
		//First we put the size of the vector
		if(!p_iputl(save.globalRAM.size(), f))
			return 51;
			
		for(dword j = 0; j < save.globalRAM.size(); j++)
		{
			ZScriptArray& a = save.globalRAM[j];
			
			//Then we put the size of each container
			if(!p_iputl(a.Size(), f))
				return 52;
				
			//Followed by its contents
			for(dword k = 0; k < a.Size(); k++)
				if(!p_iputl(a[k], f))
					return 53;
		}
		if(!p_iputw(save.forced_awpn, f))
		{
			return 54;
		}
		
		if(!p_iputw(save.forced_bwpn, f))
		{
			return 55;
		}
	
		if(!p_iputw(save.forced_xwpn, f))
		{
			return 56;
		}
		
		if(!p_iputw(save.forced_ywpn, f))
		{
			return 57;
		}
		if(!p_putc(save.xwpn, f))
		{
			return 58;
		}
		if(!p_putc(save.ywpn, f))
		{
			return 59;
		}
		if(!pfwrite(save.lvlswitches,MAXLEVELS*sizeof(int32_t),f))
		{
			return 60;
		}
		if(!pfwrite(save.item_messages_played,MAXITEMS*sizeof(bool),f))
		{
			return 61;
		}
		if(!pfwrite(save.bottleSlots,256*sizeof(byte),f))
		{
			return 62;
		}
		if(!p_iputw(save.saved_mirror_portal.destdmap, f))
		{
			return 63;
		}
		if(!p_iputw(save.saved_mirror_portal.srcdmap, f))
		{
			return 64;
		}
		if(!p_putc(save.saved_mirror_portal.srcscr,f))
		{
			return 65;
		}
		if(!p_putc(save.saved_mirror_portal.destscr,f))
		{
			return 109;
		}
		if(!p_iputl(save.saved_mirror_portal.x, f))
		{
			return 66;
		}
		if(!p_iputl(save.saved_mirror_portal.y, f))
		{
			return 67;
		}
		if(!p_putc(save.saved_mirror_portal.sfx,f))
		{
			return 68;
		}
		if(!p_iputl(save.saved_mirror_portal.warpfx, f))
		{
			return 69;
		}
		if(!p_iputw(save.saved_mirror_portal.spr, f))
		{
			return 70;
		}
		
		if(!p_iputw(NUMSCRIPTSGENERIC,f))
		{
			new_return(71);
		}
		save_genscript(save); //read the values into the save object
		for(size_t q=0; q<NUMSCRIPTSGENERIC; q++)
        {
			if(!p_putc(save.gen_doscript[q] ? 1 : 0,f))
				return 72;
			if(!p_iputw(save.gen_exitState[q],f))
				return 73;
			if(!p_iputw(save.gen_reloadState[q],f))
				return 74;
			for(size_t ind = 0; ind < 8; ++ind)
				if(!p_iputl(save.gen_initd[q][ind],f))
					return 75;
			int32_t sz = save.gen_dataSize[q];
			if(!p_iputl(sz,f))
				return 76;
			for(auto ind = 0; ind < sz; ++ind)
				if(!p_iputl(save.gen_data[q][ind],f))
					return 77;
        }
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
		{
			if(!p_iputl(save.xstates[j],f))
			{
				return 78;
			}
		}
		
		for(auto q = 0; q < NUMSCRIPTSGENERIC; ++q)
		{
			if(!p_iputl(save.gen_eventstate[q],f))
			{
				return 79;
			}
		}
		if(!pfwrite(save.gswitch_timers,NUM_GSWITCHES*sizeof(int32_t),f))
		{
			return 80;
		}
		if (!p_iputw(save.replay_file.length(), f))
			return 81;
		if (!pfwrite((void*)save.replay_file.c_str(), save.replay_file.length(), f))
			return 82;
		uint32_t sz = save.user_objects.size();
		if(!p_iputl(sz,f))
			return 83;
		for(saved_user_object const& s_ob : save.user_objects)
		{
			if(!p_iputl(s_ob.object_index,f))
				return 84;
			//user_object
			user_object const& obj = s_ob.obj;
			if(!p_putc(obj.reserved?1:0,f))
				return 85;
			//Don't need to save owned_type,owned_i?
			uint32_t datsz = obj.data.size();
			if(!p_iputl(datsz,f))
				return 86;
			for(uint32_t q = 0; q < datsz; ++q)
				if(!p_iputl(obj.data.at(q),f))
					return 87;
			if(!p_iputl(obj.owned_vars,f))
				return 88;
			//scr_func_exec
			scr_func_exec const& exec = obj.destruct;
			if(!p_iputl(exec.pc,f))
				return 89;
			if(!p_iputl(exec.thiskey,f))
				return 90;
			if(!p_iputl((int)exec.type,f))
				return 91;
			if(!p_iputl(exec.i,f))
				return 92;
			if(!p_iputw(exec.script,f))
				return 93;
			if(!p_putwstr(exec.name,f))
				return 94;
			auto& map = s_ob.held_arrays;
			uint32_t arrcount = map.size();
			if(!p_iputl(arrcount,f))
				return 95;
			for(auto it = map.begin(); it != map.end(); ++it)
			{
				auto& pair = *it;
				if(!p_iputl(pair.first,f))
					return 96;
				auto& zsarr = pair.second;
				uint32_t arrsz = zsarr.Size();
				if(!p_iputl(arrsz,f))
					return 97;
				for(uint32_t ind = 0; ind < arrsz; ++ind)
				{
					if(!p_iputl(zsarr[ind],f))
						return 98;
				}
			}
		}
		sz = save.user_portals.size();
		if(!p_iputl(sz,f))
			return 99;
		for(savedportal const& p : save.user_portals)
		{
			if(!p_iputw(p.destdmap, f))
				return 100;
			if(!p_iputw(p.srcdmap, f))
				return 101;
			if(!p_putc(p.srcscr,f))
				return 102;
			if(!p_putc(p.destscr,f))
				return 103;
			if(!p_iputl(p.x, f))
				return 104;
			if(!p_iputl(p.y, f))
				return 105;
			if(!p_putc(p.sfx,f))
				return 106;
			if(!p_iputl(p.warpfx, f))
				return 107;
			if(!p_iputw(p.spr, f))
				return 108;
		}
	}
	
	return 0;
}

// call once at startup
int32_t saves_init()
{
	saves.clear();
	currgame = -1;
	game = new gamedata();
	return 0;
}

int32_t saves_load()
{
	std::string save_file_name = get_save_file_path();
	const char *fname = save_file_name.c_str();

	FFCore.kb_typing_mode = false;
	FFCore.skip_ending_credits = 0;
	int32_t ret;
	PACKFILE *f=NULL;
	char tmpfilename[L_tmpnam];
	temp_name(tmpfilename);
	
	// see if it's there
	if(!exists(fname))
	{
		goto newdata;
	}
	
	if(file_size_ex_password(fname, "") == 0)
	{
		if(errno==0) // No error, file's empty
		{
			goto init;
		}
		else // Error...
		{
			goto cantopen;
		}
	}
	
	// decode to temp file
#ifdef __EMSCRIPTEN__
    if (em_is_lazy_file(fname))
    {
        em_fetch_file(fname);
    }
#endif
	ret = decode_file_007(fname, tmpfilename, SAVE_HEADER, ENC_METHOD_MAX-1, strstr(fname, ".dat#")!=NULL, "");
	if(ret) ret = decode_file_007(fname, tmpfilename, OLD_SAVE_HEADER, ENC_METHOD_MAX-1, strstr(fname, ".dat#")!=NULL, "");
	
	if(ret)
	{
		goto cantopen;
	}
	
	fname = tmpfilename;
	
	// load the games
	f = pack_fopen_password(fname, F_READ_PACKED, "");
	
	if(!f)
		goto cantopen;
		
	if(read_saves(f)!=0)
		goto reset;
	
	pack_fclose(f);
	delete_file(tmpfilename);
	return 0;
	
newdata:
	if(standalone_mode)
		goto init;

	system_pal();

	#ifdef __EMSCRIPTEN__
		goto init;
	#endif

	if(jwin_alert("Can't Find Saved Game File",
				  "The save file could not be found.",
				  "Create a new file from scratch?",
				  "Warning: Doing so will erase any previous saved games!",
				  "OK","Cancel",13,27,get_zc_font(font_lfont))!=1)
	{
		exit(1);
	}
	
	game_pal();
	Z_message("Save file not found.  Creating new save file.");
	goto init;
	
cantopen:
	{
		system_pal();
		char buf[256];
		snprintf(buf, 256, "still can't be opened, you'll need to delete %s.", fname);
		jwin_alert("Can't Open Saved Game File",
				   "The save file was found, but could not be opened. Wait a moment",
				   "and try again. If this problem persists, reboot. If the file",
				   buf,
				   "OK",NULL,'o',0,get_zc_font(font_lfont));
	}
	exit(1);
	
reset:
	system_pal();
	
	if(jwin_alert3("Can't Open Saved Game File",
				   "Unable to read the save file.",
				   "Create a new file from scratch?",
				   "Warning: Doing so will erase any previous saved games!",
				   "No","Yes",NULL,'n','y',0,get_zc_font(font_lfont))!=2)
	{
		exit(1);
	}
	
	game_pal();
	
	if(f)
		pack_fclose(f);
		
	delete_file(tmpfilename);
	Z_message("Format error.  Resetting game data... ");
	
init:
	saves.clear();

	if (standalone_mode)
	{
		char *fn=get_filename(standalone_quest);
		saves.emplace_back().set_name(fn);

		qstpath=(char*)malloc(2048);
		strncpy(qstpath, standalone_quest, 2047);
		qstpath[2047]='\0';

		saves_do_first_time_stuff(0);
	}

	return 0;
}

void saves_update_icon(int index)
{
	gamedata* g = saves_get_data_mutable(index);

	flushItemCache();
	int32_t maxringid = getHighestLevelOfFamily(g, itemsbuf, itype_ring);
	int32_t ring = 0;
	if (maxringid != -1)
	{
		ring = itemsbuf[maxringid].fam_type;
	}
	if (ring > 0) --ring;
	int32_t i = ring;

	int32_t t = QMisc.icons[i];
		
	if(t<0 || t>=NEWMAXTILES)
	{
		t=0;
	}
	
	int32_t tileind = t ? t : 28;
	
	byte *si = newtilebuf[tileind].data;
	
	if(newtilebuf[tileind].format==tf8Bit)
	{
		for(int32_t j=0; j<128; j++)
		{
			g->icon[j] = 0;
		}
	}
	else
	{
		for(int32_t j=0; j<128; j++)
		{
			g->icon[j] = *(si++);
		}
	}
	
	if(t)
	{
		si = colordata + CSET(pSprite(i+spICON1))*3;
	}
	else
	{
		if (i)
		{
			si = colordata + CSET(pSprite(i-1+spBLUE))*3;
		}
		else
		{
			si = colordata + CSET(6)*3;
		}
	}
	
	if(newtilebuf[tileind].format==tf8Bit)
	{
		for(int32_t j=0; j<48; j++)
		{
			g->pal[j] = 0;
		}
	}
	else
	{
		for(int32_t j=0; j<48; j++)
		{
			g->pal[j] = *(si++);
		}
	}
}

static int32_t do_save_games(const char* path)
{
	// Not sure why this happens, but apparently it does...
	for(int32_t i=0; i<saves_count(); i++)
	{
		for(int32_t j=0; j<48; j++)
		{
			saves[i].pal[j]&=63;
		}
	}

	if (currgame >= 0)
	{
		saves[currgame] = *game;
		saves_update_icon(currgame);
	}

	if (disable_save_to_disk)
	{
		return 1;
	}
	
	char tmpfilename[L_tmpnam];
	temp_name(tmpfilename);
	
	PACKFILE *f = pack_fopen_password(tmpfilename, F_WRITE_PACKED, "");
	
	if(!f)
	{
		delete_file(tmpfilename);
		return 2;
	}
	
	if(write_saves(f)!=0)
	{
		pack_fclose(f);
		delete_file(tmpfilename);
		return 4;
	}
	
	pack_fclose(f);
	int32_t ret = encode_file_007(tmpfilename, path, 0x413F0000 + (frame&0xffff), SAVE_HEADER, ENC_METHOD_MAX-1);
	
	if(ret)
		ret += 100;
		
	delete_file(tmpfilename);

#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif

	return ret;
}

int32_t saves_write()
{
	Saving = true;
	render_zc();
	std::string save_file_name = get_save_file_path();
	int32_t result = do_save_games(save_file_name.c_str());
	Saving = false;
	return result;
}

void saves_select(int32_t index)
{
	currgame = index;
	if (index >= 0)
		game->Copy(saves[index]);
	else
		game->Clear();
}

int32_t saves_count()
{
	return saves.size();
}

int32_t saves_current_selection()
{
	return currgame;
}

const gamedata* saves_get_data(int32_t index)
{
	if (index < 0)
		abort();
	return &saves[index];
}

void saves_delete(int32_t index)
{
	if (index < saves_count())
	{
		saves.erase(saves.begin() + index);
		if(listpos>saves_count()-1)
			listpos=zc_max(listpos-3,0);
	}
}

void saves_copy(int32_t from_index)
{
	int savecnt = saves_count();
	if (from_index >= savecnt || savecnt >= MAXSAVES)
		abort();

	saves.reserve(saves.size() + 1);
	const gamedata& from_save = saves[from_index];
	gamedata& to_save = saves.emplace_back(from_save);

	if (!to_save.replay_file.empty())
	{
		if (std::filesystem::exists(from_save.replay_file))
		{
			std::string new_replay_path = create_replay_path_for_save(&to_save);
			to_save.replay_file = new_replay_path;
			std::filesystem::copy(from_save.replay_file, new_replay_path);
		}
		else
		{
			Z_error("Error copying replay file - %s not found", from_save.replay_file.c_str());
			to_save.replay_file = "";
		}
	}
}

gamedata* saves_create()
{
	return &saves.emplace_back();
}

gamedata* saves_get_data_mutable(int32_t index)
{
	if (index < 0)
		abort();
	return &saves[index];
}

const gamedata* saves_get_data()
{
	if (currgame < 0)
		abort();
	return &saves[currgame];
}

void saves_do_first_time_stuff(int index)
{
	gamedata* save = saves_get_data_mutable(index);

	if (!save->get_hasplayed())
	{
		clear_to_color(screen,BLACK);
		save->set_quest(0xFF);
		char temppath[2048];
		memset(temppath, 0, 2048);
		zc_make_relative_filename(temppath, qstdir, qstpath, 2047);
		
		if(temppath[0]==0)  //can't make relative, go absolute
		{
			sprintf(save->qstpath, "%s", qstpath);
		}
		else
		{
			sprintf(save->qstpath, "%s", temppath);
		}
		
		load_quest(save);
		
		save->set_maxlife(zinit.hc*zinit.hp_per_heart);
		save->set_life(zinit.hc*zinit.hp_per_heart);
		save->set_hp_per_heart(zinit.hp_per_heart);

		if (standalone_mode)
		{
			// Why does the continue screen need set when
			// everything else gets set automatically?
			save->set_continue_dmap(0);
			save->set_continue_scrn(0xFF);
			save->set_hasplayed(false);
		}
		
		rest(200); // Formerly 1000 -L
		saves_update_icon(index);
	}
}
