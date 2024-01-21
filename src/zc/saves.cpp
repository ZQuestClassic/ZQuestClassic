#include "zc/saves.h"

#include "allegro/file.h"
#include "base/packfile.h"
#include "base/misctypes.h"
#include "base/fonts.h"
#include "base/dmap.h"
#include "base/qrs.h"
#include "base/util.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "base/zsys.h"
#include "base/mapscr.h"
#include "dialog/info.h"
#include "zc/zelda.h"
#include "zc/ffscript.h"
#include "zc/replay.h"
#include "pal.h"
#include "tiles.h"
#include "items.h"
#include "gui/jwin.h"
#include <cstddef>
#include <cstdio>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <fmt/format.h>

namespace fs = std::filesystem;

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

extern FFScript FFCore;

static const char *SAVE_HEADER = "ZQuest Classic Save File";
static const char *OLD_SAVE_HEADER = "Zelda Classic Save File";
static int currgame = -1;
static std::vector<save_t> saves;
static bool save_current_replay_games;

save_t::~save_t()
{
	if (this->game)
		delete this->game;
	else
		delete this->header;
}

static fs::path get_legacy_save_file_path()
{
	std::string save_file_name = zc_get_config("SAVEFILE", "save_filename", "zc.sav");
	return save_file_name;
}

static fs::path get_save_folder_path()
{
	return zc_get_config("zeldadx", "save_folder", "saves");
}

static fs::path get_save_order_path()
{
	return get_save_folder_path() / "order.txt";
}

static fs::path get_backup_folder_path()
{
	return get_save_folder_path() / "backup";
}

static fs::path get_deleted_folder_path()
{
	return get_save_folder_path() / "deleted";
}

static int move_to_folder(fs::path path, fs::path dir, std::string stem = "", bool force_suffix = false)
{
	if (!fs::exists(path))
		return 0;

	fs::create_directories(dir);
	auto dest = create_new_file_path(
		dir,
		stem.empty() ? path.stem().string() : stem,
		path.extension().string(),
		force_suffix);

	std::error_code err;
	fs::copy(path, dest, err);
	if (err)
		return err.value();

	fs::remove(path);
#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif
	return 0;
}

static fs::path create_path_for_new_save(gamedata_header* header)
{
    char timestamp[80];
	std::time_t rawtime;
    std::time(&rawtime);
    std::strftime(timestamp,80,"%Y-%m-%d",std::localtime(&rawtime));

	std::string title = header->title;
	if (title.empty())
		title = get_filename(header->qstpath.c_str());

	std::string filename_prefix = fmt::format("{}-{}-{}", timestamp, title, header->name);
	sanitize(filename_prefix);
	return create_new_file_path(get_save_folder_path(), filename_prefix, ".sav").string();
}

enum class ReadMode
{
	All,
	Header,
	Size,
};

static int32_t read_saves(ReadMode read_mode, PACKFILE* f, std::vector<save_t>& out_saves, int* out_count = nullptr)
{
	if (read_mode == ReadMode::Size)
		assert(out_count);

	FFCore.kb_typing_mode = false;
	FFCore.skip_ending_credits = 0;
	word count=0;
	char name[10];
	byte tempbyte = 0;
	int16_t tempshort = 0;
	word tempword = 0;
	word tempword2 = 0;
	word tempword3 = 0;
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
	
	if(!p_igetw(&count,f))
	{
		return 5;
	}

	if (out_count)
		*out_count = count;
	if (read_mode == ReadMode::Size)
		return 0;

	for (int32_t i = 0; i < count; i++)
	{
		auto& save = out_saves.emplace_back();
		save.game = new gamedata();
		save.header = &save.game->header;
		gamedata& game = *save.game;

		if(!p_getstr(name,9,f))
		{
			return 6;
		}
		
		game.set_name(name);
		
		if(!p_getc(&tempbyte,f))
		{
			return 7;
		}
		
		game.set_quest(tempbyte);
		
		if(section_version<3)
		{
			if(!p_igetw(&tempword,f))
			{
				return 8;
			}
			
			game.set_counter(tempword, 0);
			game.set_dcounter(tempword, 0);
			
			if(!p_igetw(&tempword,f))
			{
				return 9;
			}
			
			game.set_maxcounter(tempword, 0);
			
			if(!p_igetw(&tempshort,f))
			{
				return 10;
			}
			
			game.set_dcounter(tempshort, 1);
			
			if(!p_igetw(&tempword,f))
			{
				return 11;
			}
			
			game.set_counter(tempword, 1);
			
			if(!p_igetw(&tempword,f))
			{
				return 12;
			}
			
			game.set_counter(tempword, 3);
			game.set_dcounter(tempword, 3);
		}
		
		if(!p_igetw(&tempword,f))
		{
			return 13;
		}
		
		game.set_deaths(tempword);
		
		if(section_version<3)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 14;
			}
			
			game.set_counter(tempbyte, 5);
			game.set_dcounter(tempbyte, 5);
			
			if(!p_getc(&tempbyte,f))
			{
				return 15;
			}
			
			game.set_maxcounter(tempbyte, 2);
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 16;
			}
			
			game.set_wlevel(tempbyte);
		}
		
		if(!p_getc(&tempbyte,f))
		{
			return 17;
		}
		
		if (section_version < 24) tempbyte = (tempbyte ? DIDCHEAT_BIT : 0);
		game._cheat = tempbyte;
		
		char temp;
		
		for(int32_t j=0; j<MAXITEMS; j++)
		{
			if(!p_getc(&temp, f))
				return 18;
				
			game.set_item_no_flush(j, (temp != 0));
		}
		
		size_t versz = section_version<31 ? 9 : 16;
		if(!p_getstr(game.version,versz,f))
		{
			return 20;
		}

		if (!p_getstr(&game.header.title,65,f))
		{
			return 21;
		}
		
		if(!p_getc(&tempbyte,f))
		{
			return 22;
		}
		
		game.set_hasplayed(tempbyte);
		
		if(!p_igetl(&tempdword,f))
		{
			return 23;
		}
		
		game.set_time(tempdword);
		
		if(!p_getc(&tempbyte,f))
		{
			return 24;
		}
		
		game.set_timevalid(tempbyte);
		
		if(section_version >= 37)
		{
			if(!p_getbvec(&game.lvlitems,f))
				return 25;
		}
		else if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXLEVELS; ++j)
				if(!p_getc(&(game.lvlitems[j]),f))
					return 25;
		}
		else
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
				if(!p_getc(&(game.lvlitems[j]),f))
					return 25;
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 26;
			}
			
			game.set_HCpieces(tempbyte);
		}
		
		if(!p_getc(&tempbyte,f))
		{
			return 27;
		}
		
		game.set_continue_scrn(tempbyte);
		
		if(section_version <= 5)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 28;
			}
			
			game.set_continue_dmap(tempbyte);
		}
		else
		{
			if(!p_igetw(&tempword,f))
			{
				return 28;
			}
			
			game.set_continue_dmap(tempword);
		}
		
		if(section_version<3)
		{
			if(!p_igetw(&tempword,f))
			{
				return 29;
			}
			
			game.set_counter(tempword, 4);
			
			if(!p_igetw(&tempword,f))
			{
				return 30;
			}
			
			game.set_maxcounter(tempword, 4);
			
			if(!p_igetw(&tempshort,f))
			{
				return 31;
			}
			
			game.set_dcounter(tempshort, 4);
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f))
			{
				return 32;
			}
			
			game.set_magicdrainrate(tempbyte);
			
			if(!p_getc(&tempbyte,f))
			{
				return 33;
			}
			
			game.set_canslash(tempbyte);
		}
		
		if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXDMAPS; ++j)
			{
				if(!p_getc(&(game.visited[j]),f))
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
				game.bmaps.clear();
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
							game.bmaps[di] = game.visited[512+si]&0x8F; //Replicate bug; OOB indexes
							continue;
						}
						game.bmaps[di] = tempBMaps[si]&0x8F;
					}
				}
			}
		}
		else
		{
			for(int32_t j=0; j<MAXDMAPS; ++j)
			{
				if(!p_getc(&(game.visited[j]),f))
				{
					return 34;
				}
			}
			
			if(section_version < 17)
			{
				byte tempBMaps[MAXDMAPS*64] = {0};
				for(int32_t j=0; j<MAXDMAPS*64; ++j)
					if(!p_getc(&(tempBMaps[j]),f))
						return 35;
				game.bmaps.clear();
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
							game.bmaps[di] = game.visited[512+si]&0x8F; //Replicate bug; OOB indexes
							continue;
						}
						game.bmaps[di] = tempBMaps[si]&0x8F;
					}
				}
			}
			else if(section_version < 37)
			{
				for(int32_t j=0; j<MAXDMAPS*128; ++j)
					if(!p_getc(&(game.bmaps[j]),f))
						return 35;
			}
		}
		
		if(section_version < 37)
		{
			for(int32_t j=0; j<MAXSCRSNORMAL; j++)
				if(!p_igetw(&game.maps[j],f))
					return 36;
			for(int32_t j=0; j<MAXSCRSNORMAL; ++j)
				if(!p_getc(&(game.guys[j]),f))
					return 37;
		}
		else
		{
			if(!p_getbvec(&game.bmaps, f))
				return 35;
			if(!p_getbvec(&game.maps, f))
				return 36;
			if(!p_getbvec(&game.guys, f))
				return 37;
		}
		

		if (!p_getwstr(&game.header.qstpath, f))
			return 38;

		// Convert path separators so save files work across platforms (hopefully)
		regulate_path(game.header.qstpath);
		
		if(!pfread(game.header.icon,sizeof(game.header.icon),f))
		{
			return 40;
		}
		
		if(!pfread(game.header.pal,sizeof(game.header.pal),f))
		{
			return 41;
		}

		if(section_version < 37)
		{
			const int max = section_version <= 5 ? OLDMAXLEVELS : MAXLEVELS;
			for(int q = 0; q < max; ++q)
				if(!p_getc(&(game.lvlkeys[q]),f))
					return 42;
		}
		else
		{
			if(!p_getbvec(&game.lvlkeys, f))
				return 42;
		}
		
		if(section_version>1)
		{
			if(section_version >= 37)
			{
				if(!p_getbmap(&game.screen_d,f))
					return 43;
			}
			else
			{
				size_t num_mi = MAX_MI;
				if(section_version <= 5)
					num_mi = OLDMAXDMAPS*64;
				else if(section_version < 10)
					num_mi = MAXDMAPS*64;
				for(int32_t j=0; j<num_mi; j++)
					for(int32_t k=0; k<8; k++)
						if(!p_igetl(&game.screen_d[j][k],f))
							return 43;
			}
			if ( section_version >= 12 && FFCore.getQuestHeaderInfo(vZelda) >= 0x253 || section_version >= 16)
			/* 2.53.1 also have a v12 for this section. 
			I needed to path this to ensure that the s_v is specific to the build.
			I also skipped 13 to 15 so that 2.53.1 an use these if needed with the current patch. -Z
			*/
			{
				for(int32_t j=0; j<MAX_SCRIPT_REGISTERS; j++)
				{
				if(!p_igetl(&game.global_d[j],f))
				{
					return 45;
				}
				}
			}
			else
			{
				for(int32_t j=0; j<256; j++)
				{
					if(!p_igetl(&game.global_d[j],f))
					{
						return 45;
					}
				}
			}
		}
		
		if(section_version>2) //read counters
		{
			word num_ctr = 32;
			if(section_version > 32)
			{
				if(!p_igetw(&num_ctr,f))
					return 109;
			}
			for(int32_t j=0; j<num_ctr; j++)
			{
				if(!p_igetw(&tempword,f))
				{
					return 46;
				}
				
				game.set_counter(tempword, j);
				
				if(!p_igetw(&tempword,f))
				{
					return 47;
				}
				
				game.set_maxcounter(tempword, j);
				
				if(!p_igetw(&tempshort,f))
				{
					return 48;
				}
				
				game.set_dcounter(tempshort, j);
			}
			for(auto j = num_ctr; j < MAX_COUNTERS; ++j)
			{
				game.set_counter(0,j);
				game.set_maxcounter(0,j);
				game.set_dcounter(0,j);
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
				
				game.set_generic(templong, j);
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
				
				game.set_generic(tempbyte, j);
			}
		}
		if(section_version < 37)
		{
			if(!game.get_cont_percent())
				game.set_cont_hearts(game.get_cont_hearts()*game.get_hp_per_heart());
		}
		if (section_version < 38)
		{
			game.set_spriteflickerspeed(1);
		}

		game.header.life = game.get_life();
		game.header.maxlife = game.get_maxlife();
		game.header.hp_per_heart_container = game.get_hp_per_heart();

		if (read_mode == ReadMode::Header && count == 1)
			return 0;

		if(section_version >= 34)
		{
			if(!p_igetw(&game.awpn,f))
				return 109;
			if(!p_igetw(&game.bwpn,f))
				return 110;
		}
		else if(section_version>6)
		{
			if(!p_getc(&tempbyte, f))
				return 58;
			game.awpn = tempbyte<<8;
			if(!p_getc(&tempbyte,f))
				return 59;
			game.bwpn = tempbyte<<8;
		}
		else
		{
			game.awpn = 255;
			game.bwpn = 255;
		}
		
		//First we get the size of the vector
		if(!p_igetl(&tempdword, f))
			return 53;
			
		if(tempdword != 0) //Might not be any at all
		{
			//Then we allocate the vector
			game.globalRAM.resize(tempdword);
			
			for(dword j = 0; j < game.globalRAM.size(); j++)
			{
				ZScriptArray& a = game.globalRAM[j];
				
				//We get the size of each container
				if(!p_igetl(&tempdword, f))
					return 54;
					
				//We allocate the container
				a.Resize(tempdword);
				a.setValid(true); //should always be valid
				
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
			
			game.forced_awpn = tempword2;
			
			if(!p_igetw(&tempword3, f))
			{
				return 57;
			}
			
			game.forced_bwpn = tempword3;
		}
		else
		{
			game.forced_awpn = -1;
			game.forced_bwpn = -1;
		}
		if(section_version >= 34)
		{
			if(!p_igetw(&game.forced_xwpn,f))
				return 114;
			if(!p_igetw(&game.forced_ywpn,f))
				return 115;
			if(!p_igetw(&game.xwpn,f))
				return 111;
			if(!p_igetw(&game.ywpn,f))
				return 112;
		}
		else if (section_version > 17)
		{
			if(!p_getc(&tempbyte, f))
				return 58;
			game.xwpn = tempbyte<<8;
			if(!p_getc(&tempbyte,f))
				return 59;
			game.ywpn = tempbyte<<8;
			if(!p_igetw(&game.forced_xwpn, f))
				return 60;
			
			if(!p_igetw(&game.forced_ywpn, f))
				return 61;
		}
		else
		{
			game.xwpn = 0;
			game.ywpn = 0;
			game.forced_xwpn = -1;
			game.forced_ywpn = -1;
		}
		
		if(section_version >= 37)
		{
			if(!p_getbvec(&game.lvlswitches,f))
				return 62;
		}
		else if(section_version >= 19)
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
				if(!p_igetl(&(game.lvlswitches[j]),f))
					return 62;
		}
		
		if(section_version >= 21)
		{
			for(int32_t j=0; j<MAXITEMS; ++j)
			{
				if(!p_getc(&(game.item_messages_played[j]),f))
				{
					return 63;
				}
			}
		}
		else 
		{
			std::fill(game.item_messages_played, game.item_messages_played+MAXITEMS, 0);
		}
		if(section_version >= 22)
		{
			for(int32_t j=0; j<256; ++j)
			{
				if(!p_getc(&(game.bottleSlots[j]),f))
				{
					return 64;
				}
			}
		}
		else 
		{
			memset(game.bottleSlots, 0, sizeof(game.bottleSlots));
		}
		if(section_version >= 23)
		{
			if(!p_igetw(&(game.saved_mirror_portal.destdmap), f))
			{
				return 65;
			}
			if(!p_igetw(&(game.saved_mirror_portal.srcdmap), f))
			{
				return 66;
			}
			if(!p_getc(&(game.saved_mirror_portal.srcscr),f))
			{
				return 67;
			}
			if(section_version >= 32)
			{
				if(!p_getc(&(game.saved_mirror_portal.destscr),f))
				{
					return 67;
				}
			}
			else game.saved_mirror_portal.destscr = game.saved_mirror_portal.srcscr;
			if(!p_igetl(&(game.saved_mirror_portal.x), f))
			{
				return 68;
			}
			if(!p_igetl(&(game.saved_mirror_portal.y), f))
			{
				return 69;
			}
			if(!p_getc(&(game.saved_mirror_portal.sfx),f))
			{
				return 70;
			}
			if(!p_igetl(&(game.saved_mirror_portal.warpfx), f))
			{
				return 71;
			}
			if(!p_igetw(&(game.saved_mirror_portal.spr), f))
			{
				return 72;
			}
		}
		else game.saved_mirror_portal.clear();
		
		game.clear_genscript();
		
		word num_gen_scripts;
		if(section_version >= 25)
		{
			if(section_version < 37)
			{
				byte dummybyte;
				if(!p_igetw(&num_gen_scripts, f))
					return 73;
				for(size_t q=0; q<num_gen_scripts; q++)
				{
					if(!p_getc(&dummybyte,f))
						return 74;
					game.gen_doscript.set(q, dummybyte!=0);
					if(!p_igetw(&(game.gen_exitState[q]),f))
						return 75;
					if(!p_igetw(&(game.gen_reloadState[q]),f))
						return 76;
					for(size_t ind = 0; ind < 8; ++ind)
						if(!p_igetl(&(game.gen_initd[q][ind]),f))
							return 77;
					int32_t sz;
					if(!p_igetl(&sz,f))
						return 78;
					game.gen_data[q].resize(sz);
					for(auto ind = 0; ind < sz; ++ind)
						if(!p_igetl(&(game.gen_data[q][ind]),f))
							return 79;
				}
			}
			else
			{
				if(!p_getbitstr(&game.gen_doscript, f))
					return 73;
				if(!p_getbmap(&game.gen_exitState, f))
					return 73;
				if(!p_getbmap(&game.gen_reloadState, f))
					return 73;
				if(!p_getbmap(&game.gen_initd, f))
					return 73;
				if(!p_getbmap(&game.gen_eventstate, f))
					return 73;
				if(!p_getbmap(&game.gen_data, f))
					return 73;
			}
		}
		
		
		if(section_version >= 37)
		{
			if(!p_getbmap(&game.xstates,f))
				return 78;
		}
		else if(section_version >= 26)
		{
			for(int32_t j=0; j<MAXSCRSNORMAL; j++)
				if(!p_igetl(&(game.xstates[j]),f))
					return 78;
		}
		
		if(section_version >= 27 && section_version < 37)
		{
			for(size_t q=0; q<num_gen_scripts; q++)
			{
				if(!p_igetl(&game.gen_eventstate[q],f))
					return 78;
			}
		}
		if(section_version >= 37)
		{
			if(!p_getbmap(&game.gswitch_timers,f))
				return 79;
		}
		else if(section_version >= 28)
		{
			for(size_t q = 0; q < NUM_GSWITCHES; ++q)
				if(!p_igetl(&game.gswitch_timers[q],f))
					return 79;
		}
		
		if(section_version >= 29)
		{
			if (!p_getwstr(&game.header.replay_file, f))
				return 80;
		}
		else
		{
			game.header.replay_file = "";
		}
		if(section_version >= 30)
		{
			uint32_t sz;
			if(!p_igetl(&sz,f))
				return 83;
			for(uint32_t objind = 0; objind < sz; ++objind)
			{
				saved_user_object& s_ob = game.user_objects.emplace_back();
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
					zsarr.setValid(true); //should always be valid
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
				savedportal& p = game.user_portals.emplace_back();
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
		if(section_version >= 35)
		{
			if(section_version < 37)
			{
				for(int q = 0; q < itype_max; ++q)
					if(!p_igetw(&game.OverrideItems[q],f))
						return 113;
			}
			else if(!p_getbmap(&game.OverrideItems,f))
				return 113;
		}
		if(section_version == 36)
		{
			uint32_t num_used_mapscr_data;
			if(!p_igetl(&num_used_mapscr_data,f))
				return 114;
			vector<int32_t> tmpvec;
			for(size_t q = 0; q < num_used_mapscr_data; ++q)
			{
				if(!p_getlvec(&tmpvec,f))
					return 115;
				game.screen_data[q] = tmpvec;
			}
		}
		else if(section_version > 36)
			if(!p_getbmap(&game.screen_data, f))
				return 115;
		if(section_version > 38)
			if(!p_getbmap(&game.xdoors, f))
				return 116;
		if(section_version > 39)
		{
			if(!p_getc(&game.swim_mult,f))
				return 117;
			if(!p_getc(&game.swim_div,f))
				return 118;
		}
	}
	
	return 0;
}

static int32_t write_save(PACKFILE* f, save_t* save)
{
	gamedata& game = *save->game;

	int32_t section_id=ID_SAVEGAME;
	int32_t section_version=V_SAVEGAME;
	int32_t section_cversion=CV_SAVEGAME;
	int32_t section_size=0;
	game.normalize();
	
	//section id
	if(!p_mputl(section_id,f))
		return 1;
	
	//section version info
	if(!p_iputw(section_version,f))
		return 2;
	
	if(!p_iputw(section_cversion,f))
		return 3;
	
	//section size
	if(!p_iputl(section_size,f))
		return 4;
	
	word qstpath_len=0;

	// save count
	if(!p_iputw(1, f))
		return 5;

	qstpath_len=game.header.qstpath.length();
	
	if(!p_putstr(game.get_name(),9,f))
		return 6;
	
	if(!p_putc(game.get_quest(),f))
		return 7;
	
	if(!p_iputw(game.get_deaths(),f))
		return 13;
	
	if(!p_putc(game._cheat,f))
		return 17;
	
	for(int32_t j=0; j<MAXITEMS; j++)
		if(!p_putc(game.get_item(j) ? 1 : 0,f))
			return 18;
	
	if(!pfwrite(game.version,16,f))
		return 20;

	if(!pfwrite(game.header.title.c_str(),65,f))
		return 21;
	
	if(!p_putc(game.get_hasplayed(),f))
		return 22;
	
	if(!p_iputl(game.get_time(),f))
		return 23;
	
	if(!p_putc(game.get_timevalid(),f))
		return 24;
	
	if(!p_putbvec(game.lvlitems,f))
		return 25;
	
	if(!p_putc(game.get_continue_scrn(),f))
		return 27;
	
	if(!p_iputw(game.get_continue_dmap(),f))
		return 28;
	
	if(!pfwrite(game.visited,MAXDMAPS,f))
		return 34;
	
	if(!p_putbvec(game.bmaps,f))
		return 35;
	if(!p_putbvec(game.maps,f))
		return 36;
	if(!p_putbvec(game.guys,f))
		return 37;
	
	if(!p_iputw(qstpath_len,f))
		return 38;
	
	if(!pfwrite(game.header.qstpath.data(),qstpath_len,f))
		return 39;
	
	if(!pfwrite(game.header.icon,sizeof(game.header.icon),f))
		return 40;
	
	if(!pfwrite(game.header.pal,sizeof(game.header.pal),f))
		return 41;
	
	if(!p_putbvec(game.lvlkeys,f))
		return 42;
	
	if(!p_putbmap(game.screen_d,f))
		return 43;
	
	for(int32_t j=0; j<MAX_SCRIPT_REGISTERS; j++)
		if(!p_iputl(game.global_d[j],f))
			return 44;
	
	word num_ctr = 0;
	for(auto c = MAX_COUNTERS-1; c >= 0; --c)
	{
		//Find the last counter that isn't unused
		if(game.get_counter(c) || game.get_maxcounter(c) || game.get_dcounter(c))
		{
			num_ctr = c+1;
			break;
		}
	}
	if(!p_iputw(num_ctr, f))
		return 109;
	for(int32_t j=0; j<num_ctr; j++)
	{
		if(!p_iputw(game.get_counter(j), f))
			return 45;
		
		if(!p_iputw(game.get_maxcounter(j), f))
			return 46;
		
		if(!p_iputw(game.get_dcounter(j), f))
			return 47;
	}
	
	for(int32_t j=0; j<256; j++)
		if(!p_iputl(game.get_generic(j), f))
			return 48;
	
	if(!p_iputw(game.awpn, f))
		return 49;
	
	if(!p_iputw(game.bwpn, f))
		return 50;
	
	//First we put the size of the vector
	if(!p_iputl(game.globalRAM.size(), f))
		return 51;
		
	for(dword j = 0; j < game.globalRAM.size(); j++)
	{
		ZScriptArray& a = game.globalRAM[j];
		
		//Then we put the size of each container
		if(!p_iputl(a.Size(), f))
			return 52;
			
		//Followed by its contents
		for(dword k = 0; k < a.Size(); k++)
			if(!p_iputl(a[k], f))
				return 53;
	}
	if(!p_iputw(game.forced_awpn, f))
		return 54;
	
	if(!p_iputw(game.forced_bwpn, f))
		return 55;

	if(!p_iputw(game.forced_xwpn, f))
		return 56;
	
	if(!p_iputw(game.forced_ywpn, f))
		return 57;
	if(!p_iputw(game.xwpn, f))
		return 58;
	if(!p_iputw(game.ywpn, f))
		return 59;
	if(!p_putbvec(game.lvlswitches,f))
		return 60;
	if(!pfwrite(game.item_messages_played,MAXITEMS*sizeof(bool),f))
		return 61;
	if(!pfwrite(game.bottleSlots,256*sizeof(byte),f))
		return 62;
	if(!p_iputw(game.saved_mirror_portal.destdmap, f))
		return 63;
	if(!p_iputw(game.saved_mirror_portal.srcdmap, f))
		return 64;
	if(!p_putc(game.saved_mirror_portal.srcscr,f))
		return 65;
	if(!p_putc(game.saved_mirror_portal.destscr,f))
		return 109;
	if(!p_iputl(game.saved_mirror_portal.x, f))
		return 66;
	if(!p_iputl(game.saved_mirror_portal.y, f))
		return 67;
	if(!p_putc(game.saved_mirror_portal.sfx,f))
		return 68;
	if(!p_iputl(game.saved_mirror_portal.warpfx, f))
		return 69;
	if(!p_iputw(game.saved_mirror_portal.spr, f))
		return 70;
	
	save_genscript(game); //read the values into the save object
	if(!p_putbitstr(game.gen_doscript, f))
		return 72;
	if(!p_putbmap(game.gen_exitState, f))
		return 73;
	if(!p_putbmap(game.gen_reloadState, f))
		return 74;
	if(!p_putbmap(game.gen_initd, f))
		return 75;
	if(!p_putbmap(game.gen_eventstate, f))
		return 76;
	if(!p_putbmap(game.gen_data, f))
		return 77;
	
	if(!p_putbmap(game.xstates,f))
		return 78;
	if(!p_putbmap(game.gswitch_timers,f))
		return 79;
	if (!p_putwstr(game.header.replay_file, f))
		return 81;
	uint32_t sz = game.user_objects.size();
	if(!p_iputl(sz,f))
		return 83;
	for(saved_user_object const& s_ob : game.user_objects)
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
				if(!p_iputl(zsarr[ind],f))
					return 98;
		}
	}
	sz = game.user_portals.size();
	if(!p_iputl(sz,f))
		return 99;
	for(savedportal const& p : game.user_portals)
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
	if(!p_putbmap(game.OverrideItems,f))
		return 109;
	if(!p_putbmap(game.screen_data,f))
		return 110;
	if(!p_putbmap(game.xdoors,f))
		return 111;
	if(!p_putc(game.swim_mult,f))
		return 112;
	if(!p_putc(game.swim_div,f))
		return 113;
	return 0;
}

static int32_t write_save(save_t* save)
{
	if (save->path.empty() || !save->write_to_disk)
		return 0;

	std::string tmp_filename = util::create_temp_file_path(save->path.string());
	PACKFILE *f = pack_fopen(tmp_filename.c_str(), F_WRITE_PACKED);

	if (!f)
	{
		delete_file(tmp_filename.c_str());
		return 2;
	}

	if (write_save(f, save) != 0)
	{
		pack_fclose(f);
		delete_file(tmp_filename.c_str());
		return 4;
	}

	pack_fclose(f);

	// Move existing save to backup folder.
	auto backup_folder_path = get_backup_folder_path() / save->path.stem();
	move_to_folder(save->path, backup_folder_path, save->path.stem().string() + "-backup", true);

	int ret = 0;
	std::error_code ec;
	fs::rename(tmp_filename.c_str(), save->path, ec);
	if (ec)
	{
		al_trace("Error saving: %s\n", std::strerror(ec.value()));
		ret = 5;
	}

#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif

	Z_message("write save: %s\n", save->path.c_str());
	return ret;
}

// call once at startup
int32_t saves_init()
{
	saves.clear();
	currgame = -1;
	game = new gamedata();
	return 0;
}

static int load_from_save_file(ReadMode read_mode, fs::path filename, std::vector<save_t>& out_saves)
{
	const char* error;
	int32_t ret;
	PACKFILE *pf=NULL;
	char tmpfilename[L_tmpnam];
	temp_name(tmpfilename);

	auto filenameStr = filename.string();
	auto filenameCStr = filenameStr.c_str();

	// decode to temp file
#ifdef __EMSCRIPTEN__
    if (em_is_lazy_file(filenameStr))
    {
        em_fetch_file(filenameStr);
    }
#endif

	if (!exists(filename))
	{
		error = "not found";
		ret = 1;
		goto cantopen;
	}
	else
	{
		auto unencrypted_result = try_open_maybe_legacy_encoded_file(filenameCStr, SAVE_HEADER, OLD_SAVE_HEADER, "SVGM", nullptr);

		pf = unencrypted_result.decoded_pf;
		if (!pf)
		{
			ret = decode_file_007(filenameCStr, tmpfilename, SAVE_HEADER, ENC_METHOD_MAX-1, false, "");
			if (ret)
				ret = decode_file_007(filenameCStr, tmpfilename, OLD_SAVE_HEADER, ENC_METHOD_MAX-1, false, "");
			if (ret)
			{
				error = "can't decode";
				goto cantopen;
			}
			pf = pack_fopen(tmpfilename, F_READ_PACKED);
		}

		int count;
		ret = read_saves(read_mode, pf, out_saves, &count);
		if (ret)
		{
			error = "failed reading";
			goto cantopen;
		}

		if (count == 0)
		{
			error = "no save slots";
			ret = -1;
			goto cantopen;
		}

		if (read_mode == ReadMode::Header)
		{
			for (auto& save : out_saves)
			{
				save.header = new gamedata_header(save.game->header);
				delete save.game;
				save.game = nullptr;
			}
		}
		else if (read_mode == ReadMode::Size)
		{
			out_saves.resize(count);
		}
	}

	if (pf)
		pack_fclose(pf);

	return 0;

cantopen:
	{
		if (pf)
			pack_fclose(pf);
		out_saves.clear();

		enter_sys_pal();
		char buf[256];
		snprintf(buf, 256, "Couldn't open %s", filenameCStr);
		jwin_alert("Can't Open Saved Game File",
				   buf,
				   error,
				   to_string(ret).c_str(),
				   "OK",NULL,'o',0,get_zc_font(font_lfont));
		exit_sys_pal();
	}

	return ret;
}

static int load_from_save_file_expect_one(ReadMode read_mode, fs::path path, save_t& out_save)
{
	std::vector<save_t> saves;
	int ret = load_from_save_file(read_mode, path, saves);
	if (ret)
		return ret;

	if (saves.size() != 1)
	{
		return 1;
	}

	out_save = std::move(saves[0]);
	out_save.path = path;
	return 0;
}

static int get_save(save_t*& out_save, int index, bool full_data)
{
	assert(index >= 0 && index < saves.size());
	auto& save = saves[index];

	if (full_data)
	{
		if (save.game)
		{
			out_save = &save;
			return 0;
		}
	}
	else
	{
		if (save.header)
		{
			out_save = &save;
			return 0;
		}
	}

	if (index >= 0)
	{
		if (!save.game)
		{
			int ret = load_from_save_file_expect_one(full_data ? ReadMode::All : ReadMode::Header, save.path, save);
			if (ret)
			{
				out_save = nullptr;
				return ret;
			}
		}
	}

	out_save = &save;
	return 0;
}

static int maybe_split_save_file(fs::path filename)
{
	if (!exists(filename))
	{
		return 0;
	}

	std::vector<save_t> saves;
	int ret = load_from_save_file(ReadMode::Size, filename, saves);
	if (ret)
		return ret;

	if (saves.size() <= 1)
		return 0;

	// Load for real.
	saves.clear();
	ret = load_from_save_file(ReadMode::All, filename, saves);
	if (ret)
		return ret;

	bool errored = false;
	for (auto& save : saves)
	{
		if (save.header->quest == 0)
			continue;

		save.path = create_path_for_new_save(save.header);
		ret = write_save(&save);
		if (ret) errored = true;
	}

	if (errored)
		return ret;

	ret = move_to_folder(filename, get_backup_folder_path());
	if (ret)
		return ret;

	return 0;
}

static int split_up_saves()
{
	auto dir = get_save_folder_path();
	std::set<fs::path> paths;
	for (const auto & entry : fs::directory_iterator(dir))
	{
		if (!entry.is_regular_file())
			continue;

		auto path = entry.path();
		if (path.extension() == ".sav")
			paths.insert(path);
	}

	for (auto path : paths)
	{
		int ret = maybe_split_save_file(path);
		if (ret)
			return ret;
	}

	return 0;
}

static int move_legacy_save_file()
{
	auto save_file_path = get_legacy_save_file_path();
	auto save_folder_path = get_save_folder_path();
	if (!exists(save_file_path))
		return 0;

	std::error_code err;
	fs::copy(save_file_path, save_folder_path, err);
	if (err)
		return err.value();

	fs::remove(save_file_path);
	return 0;
}

static void do_save_order()
{
	if (standalone_mode)
		return;

	std::ofstream out(get_save_order_path(), std::ios::binary);
	for (auto& save : saves)
	{
		out << fs::proximate(save.path, get_save_folder_path()).string() << "\n";
	}
    out.close();

#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif
}

// Creates an empty save_t (no header or gamedata, just path) for every file in the
// save folder.
static int init_from_save_folder()
{
	auto dir = get_save_folder_path();
	for (const auto & entry : fs::directory_iterator(dir))
	{
		if (!entry.is_regular_file())
			continue;

		auto path = entry.path();
		if (path.extension() == ".sav")
		{
			auto& save = saves.emplace_back();
			save.path = path;
		}
	}

	if (standalone_mode)
		return 0;

	std::ifstream file(get_save_order_path());
	std::vector<std::string> lines;
	if (file.is_open())
	{
		std::string line;
		while (util::portable_get_line(file, line))
		{
			if (line.empty())
				continue;

			lines.push_back(line);
		}
	}

	if (!lines.empty())
	{
		std::sort(saves.begin(), saves.end(),
			[&] (const save_t& lhs, const save_t& rhs) {
				std::string lhs_str = fs::proximate(lhs.path, dir).string();
				std::string rhs_str = fs::proximate(rhs.path, dir).string();
				auto a = std::distance(lines.begin(), std::find(lines.begin(), lines.end(), lhs_str));
				auto b = std::distance(lines.begin(), std::find(lines.begin(), lines.end(), rhs_str));
				return a < b;
			});
	}
	else if (!saves.empty())
	{
		do_save_order();
	}

	return 0;
}

int32_t saves_load()
{
	FFCore.kb_typing_mode = false;
	FFCore.skip_ending_credits = 0;

	saves.clear();
	fs::create_directories(get_save_folder_path());

	if (standalone_mode)
	{
		assert(!standalone_save_path.empty());
		auto path = get_save_folder_path() / standalone_save_path;
		if (fs::exists(path))
		{
			auto& save = saves.emplace_back();
			save.path = path;
			return 0;
		}

		auto& save = saves.emplace_back();
		save.path = path;
		save.game = new gamedata();
		save.header = &save.game->header;
		save.header->qstpath = standalone_quest.string();
		save.header->name = get_filename(standalone_quest.string().c_str());
		saves_do_first_time_stuff(0);
		return 0;
	}

	int ret = move_legacy_save_file();
	if (ret)
		return ret;

	// First check which save files need to be split up into individual files.
	ret = split_up_saves();
	if (ret)
		return ret;

	// Should still be empty.
	assert(saves.empty());

	// Lastly, just set up the save vector to contain paths.
	// Each header will get loaded as requested from the title screen. gamedata is read only when needed.
	return init_from_save_folder();
}

static void update_icon(int index)
{
	save_t* save;
	int ret = get_save(save, index, true);
	if (ret)
		return;

	flushItemCache();
	int32_t maxringid = getHighestLevelOfFamily(save->game, itemsbuf, itype_ring);
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

	byte temp_tile[128];
	if (newtilebuf[tileind].format==tf4Bit)
	{
		byte *di = temp_tile;
		byte *src = newtilebuf[tileind].data;
		for (int32_t si=0; si<256; si+=2)
		{
			*di = (src[si]&15) + ((src[si+1]&15) << 4);
			++di;
		}
		si = temp_tile;
	}
	
	if(newtilebuf[tileind].format==tf8Bit)
	{
		for(int32_t j=0; j<128; j++)
		{
			save->game->header.icon[j] = 0;
		}
	}
	else
	{
		for(int32_t j=0; j<128; j++)
		{
			save->game->header.icon[j] = *(si++);
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
			save->game->header.pal[j] = 0;
		}
	}
	else
	{
		for(int32_t j=0; j<48; j++)
		{
			save->game->header.pal[j] = *(si++);
		}
	}
}

static int32_t do_save_games()
{
	// Not sure why this happens, but apparently it does...
	for (const auto& save : saves)
	{
		if (!save.header) continue;

		for(int32_t j=0; j<48; j++)
		{
			save.header->pal[j]&=63;
		}
	}

	if (currgame >= 0)
	{
		saves[currgame].game->Copy(*game);
		update_icon(currgame);
	}

	if (currgame >= 0 && save_current_replay_games)
	{
		auto dir = get_save_folder_path() / "current_replay";
		fs::create_directories(dir);
		saves[currgame].path = create_new_file_path(dir, "zc", ".sav", true);
		saves[currgame].write_to_disk = true;
		int ret = write_save(&saves[currgame]);
		if (ret)
			return ret;
	}

	if (disable_save_to_disk)
	{
		return 0;
	}

	fs::create_directories(get_save_folder_path());

	for (auto& save : saves)
	{
		if (!save.header || !save.game)
			continue;

		int ret = write_save(&save);
		if (ret)
			return ret;
	}

	do_save_order();
	return 0;
}

int32_t saves_write()
{
	Saving = true;
	if (!is_headless())
		render_zc();
	int32_t ret = do_save_games();
	Saving = false;
	// TODO: we should return error messages, not codes.
	if (ret)
	{
		enter_sys_pal();
		InfoDialog("Error writing saves", fmt::format("Error code: {}. Check allegro.log for more", ret)).show();
		exit_sys_pal();
	}
	return ret;
}

bool saves_select(int32_t index)
{
	currgame = index;
	if (index >= 0)
	{
		save_t* save;
		int ret = get_save(save, index, true);
		if (ret)
			return false;

		game->Copy(*save->game);
	}
	else
		game->Clear();

	// Unload any other games.
	for (int i = 0; i < saves.size(); i++)
	{
		if (i != index)
		{
			auto& save = saves[i];
			if (save.game)
			{
				save.header = new gamedata_header(save.game->header);
				delete save.game;
				save.game = nullptr;
			}
		}
	}

	return true;
}

void saves_unselect()
{
	if (currgame == -1)
		return;

	auto& save = saves[currgame];
	if (save.game)
	{
		save.header = new gamedata_header(save.game->header);
		delete save.game;
		save.game = nullptr;
	}
	currgame = -1;
}

int32_t saves_count()
{
	return saves.size();
}

int32_t saves_current_selection()
{
	return currgame;
}

bool saves_is_slot_loaded(int32_t index, bool full_data)
{
	if (saves.size() <= index)
		return false;
	if (full_data && saves[index].game == nullptr)
		return false;
	return saves[index].header != nullptr;
}

const save_t* saves_get_slot(int32_t index, bool full_data)
{
	save_t* save;
	int ret = get_save(save, index, full_data);
	if (ret)
		abort();

	return save;
}

const save_t* saves_get_current_slot()
{
	assert(currgame >= 0);
	return saves_get_slot(currgame, true);
}

void saves_delete(int32_t index)
{
	assert(index >= 0 && index < saves.size());
	auto& save = saves[index];

	int ret = move_to_folder(save.path, get_deleted_folder_path());
	if (ret)
		return;

	saves.erase(saves.begin() + index);
	do_save_order();

	if(listpos>saves_count()-1)
		listpos=zc_max(listpos-3,0);
}

void saves_copy(int32_t from_index)
{
	auto& new_save = saves.emplace_back();

	save_t* from_save_mut;
	int ret = get_save(from_save_mut, from_index, false);
	const save_t* from_save = from_save_mut;
	if (ret)
	{
		saves.pop_back();
		return;
	}

	new_save.path = create_path_for_new_save(from_save->header);
	std::error_code err;
	bool success = fs::copy_file(from_save->path, new_save.path, err);
	if (!success)
	{
		saves.pop_back();
		return;
	}

	new_save.header = new gamedata_header(*from_save->header);

	if (!from_save->header->replay_file.empty())
	{
		if (std::filesystem::exists(from_save->header->replay_file))
		{
			new_save.header->replay_file = create_replay_path_for_save(*new_save.header);
			std::filesystem::copy(from_save->header->replay_file, new_save.header->replay_file);
		}
		else
		{
			Z_error("Error copying replay file - %s not found", from_save->header->replay_file.c_str());
			new_save.header->replay_file = "";
		}
	}

	do_save_order();
}

bool saves_create_slot(gamedata* game, bool write_to_disk)
{
	auto& save = saves.emplace_back();
	save.game = game;
	save.header = &game->header;
	save.path = "";
	save.write_to_disk = write_to_disk;
	return true;
}

bool saves_create_slot(fs::path path, bool write_to_disk)
{
	if (!fs::exists(path))
		return false;

	auto& save = saves.emplace_back();
	save.path = path;
	save.write_to_disk = write_to_disk;
	return true;
}

int saves_do_first_time_stuff(int index)
{
	save_t* save;
	int ret = get_save(save, index, true);
	if (ret)
		return ret;

	if (!save->game->get_hasplayed())
	{
		if (save->write_to_disk)
		{
			// Save file may have already been set but now the player is changing the quest path.
			// Since this hasn't been played yet, this save file is useless, so just delete it.
			if (!save->path.empty() && fs::exists(save->path))
				fs::remove(save->path);
			save->path = create_path_for_new_save(save->header);
		}

		save->game->set_quest(save->game->header.qstpath.ends_with("classic_1st.qst") ? 1 : 0xFF);

		// Try to make relative to qstdir.
		// TODO: this is a weird place to do this.
		std::string rel_dir = (fs::current_path() / fs::path(qstdir)).string();
		auto maybe_rel_qstpath = util::is_subpath_of(rel_dir, save->game->header.qstpath) ?
			fs::relative(save->game->header.qstpath, rel_dir) :
			fs::path(save->game->header.qstpath);
		save->game->header.qstpath = maybe_rel_qstpath.string();

		ret = load_quest(save->game);
		if (ret)
			return ret;
		
		save->game->set_maxlife(zinit.mcounter[crLIFE]);
		save->game->set_life(zinit.mcounter[crLIFE]);
		save->game->set_hp_per_heart(zinit.hp_per_heart);
		save->game->header.life = save->game->get_life();
		save->game->header.maxlife = save->game->get_maxlife();
		save->game->header.hp_per_heart_container = save->game->get_hp_per_heart();

		if (standalone_mode)
		{
			// Why does the continue screen need set when
			// everything else gets set automatically?
			save->game->set_continue_dmap(0);
			save->game->set_continue_scrn(0xFF);
			save->game->set_hasplayed(false);
		}

		update_icon(index);
		if (save->path.empty())
			save->path = create_path_for_new_save(save->header);
		return saves_write();
	}

	return 0;
}

void saves_enable_save_current_replay()
{
	save_current_replay_games = true;
}

template<std::size_t N, class T>
constexpr std::size_t countof(T(&)[N]) { return N; }

template <typename T>
static bool gd_compare(T a, T b)
{
	return a == b;
}

static bool gd_compare(const char* a, const char* b)
{
	return strcmp(a, b) == 0;
}

bool saves_test()
{
	int fake_errno = 0;
	allegro_errno = &fake_errno;

	// For some reason MSVC hangs on compiling gamedata==
#ifndef _WIN32
	// First make sure there are not accidentally equalities that are impossible,
	// like fields that have pointers to objects.
	gamedata* g1 = new gamedata();
	gamedata* g2 = new gamedata();
	g1->saved_mirror_portal.clearUID();
	g2->saved_mirror_portal.clearUID();
	if (*g1 != *g2)
	{
		delete g1;
		delete g2;
		printf("failed: g1 == g2\n");
		return false;
	}
	delete g1;
	delete g2;
#endif

	gamedata* game = new gamedata();
	game->header.qstpath = "somegame.qst";
	game->header.title = "I am a test";
	game->header.name = "test";
	game->header.deaths = 10;
	game->header.has_played = true;
	game->header.life = game->get_life();
	game->header.maxlife = game->get_maxlife();
	game->header.hp_per_heart_container = game->get_hp_per_heart();
	game->OverrideItems[0] = 0;
	game->OverrideItems[511] = 511;
	// Does not persist.
	// game->header.did_cheat = true;

	save_t save;
	save.game = game;
	save.header = &game->header;
	save.path = "test.sav";
	save.write_to_disk = true;

	if (write_save(&save))
	{
		printf("failed: write_save\n");
		delete game;
		return false;
	}

	save.game = nullptr;
	save.header = nullptr;

	if (load_from_save_file_expect_one(ReadMode::All, save.path, save))
	{
		printf("failed: load_from_save_file_expect_one\n");
		delete game;
		return false;
	}

	#define SAVE_TEST_FIELD(field) if (!gd_compare(game->field, save.game->field)) {\
		printf("game->%s != save.game->%s\n", #field, #field);\
		printf("%s\n", fmt::format("{} != {}", game->field, save.game->field).c_str());\
		delete game;\
		return false;\
	}

	#define SAVE_TEST_FIELD_NOFMT(field) if (!gd_compare(game->field, save.game->field)) {\
		printf("game->%s != save.game->%s\n", #field, #field);\
		delete game;\
		return false;\
	}

	#define SAVE_TEST_VECTOR(field) for (int i = 0; i < game->field.size(); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			printf("%s\n", fmt::format("{} != {}", game->field[i], save.game->field[i]).c_str());\
			delete game;\
			return false;\
		}\
	}

	#define SAVE_TEST_VECTOR_NOFMT(field) for (int i = 0; i < game->field.size(); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			delete game;\
			return false;\
		}\
	}
	
	#define SAVE_TEST_VECTOR_2D(field) for (int i = 0; i < game->field.size(); ++i) {\
		for(int j = 0; j < game->field[i].size(); ++j) {\
			if (!gd_compare(game->field[i][j], save.game->field[i][j])) {\
				printf("game->%s[%d][%d] != save.game->%s[%d][%d]\n", #field, i, j, #field, i, j);\
				printf("%s\n", fmt::format("{} != {}", game->field[i][j], save.game->field[i][j]).c_str());\
				delete game;\
				return false;\
			}\
		}\
	}
	
	#define SAVE_TEST_BITSTRING(field) \
		SAVE_TEST_VECTOR(field.inner())

	#define SAVE_TEST_ARRAY(field) for (int i = 0; i < countof(game->field); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			printf("%s\n", fmt::format("{} != {}", game->field[i], save.game->field[i]).c_str());\
			delete game;\
			return false;\
		}\
	}

	#define SAVE_TEST_ARRAY_NOFMT(field) for (int i = 0; i < countof(game->field); i++) {\
		if (!gd_compare(game->field[i], save.game->field[i])) {\
			printf("game->%s[%d] != save.game->%s[%d]\n", #field, i, #field, i);\
			delete game;\
			return false;\
		}\
	}

	#define SAVE_TEST_ARRAY_2D(field) for (int i = 0; i < countof(game->field); i++) {\
		for (int j = 0; j < countof(game->field[0]); j++) {\
			if (!gd_compare(game->field[i][j], save.game->field[i][j])) {\
				printf("game->%s[%d][%d] != save.game->%s[%d][%d]\n", #field, i, j, #field, i, j);\
				printf("%s\n", fmt::format("{} != {}", game->field[i][j], save.game->field[i][j]).c_str());\
				delete game;\
				return false;\
			}\
		}\
	}

	#define SAVE_TEST_ARRAY_2D_NOFMT(field) for (int i = 0; i < countof(game->field); i++) {\
		for (int j = 0; j < countof(game->field[0]); j++) {\
			if (!gd_compare(game->field[i][j], save.game->field[i][j])) {\
				printf("game->%s[%d][%d] != save.game->%s[%d][%d]\n", #field, i, j, #field, i, j);\
				delete game;\
				return false;\
			}\
		}\
	}

	// Test some (but not all ... I got lazy) of the fields, in the order found in the save format.
	SAVE_TEST_FIELD(get_name());
	SAVE_TEST_FIELD(get_quest());
	SAVE_TEST_FIELD(get_deaths());
	SAVE_TEST_FIELD(_cheat);
	SAVE_TEST_ARRAY(item);
	SAVE_TEST_ARRAY(version);
	SAVE_TEST_FIELD(get_hasplayed());
	SAVE_TEST_FIELD(get_time());
	SAVE_TEST_FIELD(get_timevalid());
	SAVE_TEST_VECTOR(lvlitems);
	SAVE_TEST_FIELD(get_continue_scrn());
	SAVE_TEST_FIELD(get_continue_dmap());
	SAVE_TEST_ARRAY(visited);
	SAVE_TEST_VECTOR(bmaps);
	SAVE_TEST_VECTOR(maps);
	SAVE_TEST_VECTOR(guys);
	SAVE_TEST_VECTOR(lvlkeys);
	SAVE_TEST_VECTOR_2D(screen_d);
	SAVE_TEST_ARRAY(global_d);
	SAVE_TEST_ARRAY(_counter);
	SAVE_TEST_ARRAY(_maxcounter);
	SAVE_TEST_ARRAY(_dcounter);
	SAVE_TEST_FIELD(awpn);
	SAVE_TEST_FIELD(bwpn);
	SAVE_TEST_FIELD_NOFMT(globalRAM);
	SAVE_TEST_FIELD(forced_awpn);
	SAVE_TEST_FIELD(forced_bwpn);
	SAVE_TEST_FIELD(forced_xwpn);
	SAVE_TEST_FIELD(forced_ywpn);
	SAVE_TEST_FIELD(xwpn);
	SAVE_TEST_FIELD(ywpn);
	SAVE_TEST_VECTOR(lvlswitches);
	SAVE_TEST_ARRAY(item_messages_played);
	SAVE_TEST_ARRAY(bottleSlots);

	game->saved_mirror_portal.clearUID();
	save.game->saved_mirror_portal.clearUID();
	if (game->saved_mirror_portal != save.game->saved_mirror_portal)
	{
		printf("game->saved_mirror_portal != save.game->saved_mirror_portal\n");
		delete game;
		return false;
	}

	SAVE_TEST_BITSTRING(gen_doscript);
	SAVE_TEST_VECTOR(gen_exitState);
	SAVE_TEST_VECTOR(gen_reloadState);
	SAVE_TEST_VECTOR_2D(gen_initd);
	SAVE_TEST_VECTOR_2D(gen_data);
	SAVE_TEST_VECTOR_2D(screen_data);
	SAVE_TEST_VECTOR(xstates);
	SAVE_TEST_VECTOR_2D(xdoors);
	SAVE_TEST_VECTOR(gen_eventstate);
	SAVE_TEST_VECTOR(gswitch_timers);
	SAVE_TEST_VECTOR_NOFMT(user_objects);
	SAVE_TEST_VECTOR_NOFMT(user_portals);
	SAVE_TEST_VECTOR(OverrideItems);

	// Now do the header.
	if (game->header != save.game->header)
	{
		printf("game->header != save.game->header\n");
		delete game;
		return false;
	}

#ifndef _WIN32
	// Now do the entire thing.
	if (*game != *save.game)
	{
		printf("game != save.game\n");
		delete game;
		return false;
	}
#endif

	delete game;
	return true;
}

