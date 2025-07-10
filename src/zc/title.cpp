#include <cmath>
#include <filesystem>
#include <stdio.h>
#include <ctype.h>
#include <cstring>
#include <memory>
#include "base/files.h"
#include "base/fonts.h"
#include "base/render.h"
#include "base/zapp.h"
#include "base/zc_alleg.h"
#include "base/qrs.h"
#include "base/packfile.h"
#include "base/dmap.h"
#include "base/misctypes.h"

#include "base/zdefs.h"
#include "music_playback.h"
#include "sound/zcmusic.h"
#include "subscr.h"
#include "zc/replay.h"
#include "zc/replay_upload.h"
#include "zc/zc_sys.h"
#include "zc/zelda.h"
#include "base/zsys.h"
#include "base/qst.h"
#include "tiles.h"
#include "pal.h"
#include "base/gui.h"
#include "zc/title.h"
#include "zc/ffscript.h"
#include "zc/saves.h"
#include "zc/render.h"
#include "dialog/info.h"
#include "zinfo.h"
#include "zscriptversion.h"
#include <fmt/format.h>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

#ifdef _MSC_VER
#define strupr _strupr
#define stricmp _stricmp
#endif

extern int32_t loadlast;
extern int32_t skipcont;

static void select_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"NEW FILE",48,152,1,0);
	textout_ex(scrollbuf,get_zc_font(font_zfont),"COPY FILE",48,168,1,0);
	textout_ex(scrollbuf,get_zc_font(font_zfont),"DELETE FILE",48,184,1,0);
}

static void register_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"NEW FILE",48,152,CSET(2)+3,0);
}

static void copy_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"COPY FILE",48,168,CSET(2)+3,0);
}

static void delete_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"DELETE FILE",48,184,CSET(2)+3,0);
}

static void framerect(BITMAP* dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t c)
{
	BITMAP* temp = create_bitmap_ex(8, dest->w, dest->h);
	clear_bitmap(temp);
	rect(temp, x, y, x+w-1, y+h-1,c);
	rect(temp, x+1, y+1, x+w-2, y+h-2,c);
	temp->line[y][x] = 0;
	temp->line[y][x+w-1] = 0;
	temp->line[y+h-1][x] = 0;
	temp->line[y+h-1][x+w-1] = 0;
	masked_blit(temp, dest, x, y, x, y, w, h);
	destroy_bitmap(temp);
}

static RenderTreeItem* get_logo()
{
	static std::string logo_path;
	if (logo_path.empty())
	{
		if (exists("assets/logo.png"))
			logo_path = "assets/logo.png";
		else
			logo_path = "assets/zc/ZC_Logo.png";
	}

	static RenderTreeItem rti_logo("logo");
	static ALLEGRO_BITMAP* logo_bitmap = al_load_bitmap(logo_path.c_str());

	if (logo_bitmap)
	{
		rti_logo.bitmap = logo_bitmap;
		rti_logo.freeze = true;
		rti_game.add_child(&rti_logo);
	}

	return &rti_logo;
}

static void selectscreen()
{
	FFCore.kb_typing_mode = false;
	if (get_qr(qr_OLD_SCRIPT_VOLUME))
	{
		if (FFCore.coreflags & FFCORE_SCRIPTED_MIDI_VOLUME)
		{
			Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
			midi_volume = FFCore.usr_midi_volume;
			//master_volume(-1,FFCore.usr_midi_volume);
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_DIGI_VOLUME)
		{
			digi_volume = FFCore.usr_digi_volume;
			//master_volume((int32_t)(FFCore.usr_digi_volume),1);
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_MUSIC_VOLUME)
		{
			emusic_volume = (int32_t)FFCore.usr_music_volume;
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_SFX_VOLUME)
		{
			sfx_volume = (int32_t)FFCore.usr_sfx_volume;
		}
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
		pan_style = (int32_t)FFCore.usr_panstyle;
	}
	FFCore.skip_ending_credits = 0;
	init_NES_mode();
	loadfullpal();
	loadlvlpal(1);
	Bwpn = -1, Awpn = -1, Xwpn = -1, Ywpn = -1; //the subsreen values
	clear_bitmap(scrollbuf);
	
	framerect(scrollbuf, 27, 51, 26*8-6, 20*8-6,0x03);
	
	textout_ex(scrollbuf,get_zc_font(font_zfont)," NAME ",80,48,1,0);
	textout_ex(scrollbuf,get_zc_font(font_zfont)," LIFE ",152,48,1,0);

	auto logo = get_logo();
	if (logo->bitmap)
	{
		static float aspect_ratio = (float)al_get_bitmap_width(logo->bitmap) / al_get_bitmap_height(logo->bitmap);

		int target_height = 46;
		int target_width = target_height * aspect_ratio;
		float scale = (float)target_height / al_get_bitmap_height(logo->bitmap);
		int x = (al_get_bitmap_width(rti_game.bitmap) - target_width) / 2;
		logo->set_transform({.x = x, .y = -5, .xscale = scale, .yscale = scale});
	}
	else
	{
		textout_ex(scrollbuf,get_zc_font(font_zfont),"- S E L E C T -",64,24,1,0);
	}

	select_mode();
	RAMpal[CSET(9)+1]=NESpal(0x15);
	RAMpal[CSET(9)+2]=NESpal(0x27);
	RAMpal[CSET(9)+3]=NESpal(0x30);
	RAMpal[CSET(13)+1]=NESpal(0x30);
}

static byte left_arrow_str[] = {132,0};
static byte right_arrow_str[] = {133,0};

static int sels_herotile = 237;
static int sels_hero_cset = 0;

static void list_save(gamedata_header* header, int32_t save_num, int32_t ypos)
{
	bool r = refreshpal;
	byte quest = header->quest;

	game->set_maxlife(header->maxlife);
	game->set_life(header->maxlife);
	game->set_hp_per_heart(header->hp_per_heart_container);

	lifemeter(framebuf,144,ypos+((game->get_maxlife()>16*(header->hp_per_heart_container))?8:0),8,0);
	textout_ex(framebuf,get_zc_font(font_zfont),header->name.c_str(),72,ypos+16,1,0);
	
	if(quest)
		textprintf_ex(framebuf,get_zc_font(font_zfont),72,ypos+24,1,0,"%5d",header->deaths);

	textprintf_ex(framebuf,get_zc_font(font_zfont),72,ypos+16,1,0,"%s",header->name.c_str());

	byte temp_bytes[256];
	{
		byte *si = header->icon + 128;
		byte *di = temp_bytes + 256;
		
		for(int i=127; i>=0; --i)
		{
			(*(--di)) = (*(--si)) >> 4;
			(*(--di)) = (*si) & 15;
		}
	}

	byte *hold = newtilebuf[0].data;
	byte holdformat=newtilebuf[0].format;
	newtilebuf[0].format=tf4Bit;
	newtilebuf[0].data = temp_bytes;
	overtile16(framebuf,(sels_herotile > 1 && quest > 0 && quest < 255 ) ? sels_herotile : 0,48,ypos+17,
	((unsigned)sels_hero_cset < 15 && quest > 0 && quest < 255 ) ? (unsigned)sels_hero_cset < 15 :
	(save_num%3)+10,0);               //hero
	newtilebuf[0].format=holdformat;
	newtilebuf[0].data = hold;
	
	hold = colordata;
	colordata = header->pal;
	loadpalset((save_num%3)+10,0);
	colordata = hold;
	
	textout_ex(framebuf,get_zc_font(font_zfont),"-",136,ypos+16,1,0);
	
	refreshpal = r;
}

static void list_saves()
{
	// Fourth Quest turns the menu red.
	bool red = false;

	// First, make sure the presently listed quests are loaded.
	int savecnt = saves_count();
	for(int32_t i=0; i<3; i++)
	{
		int j = listpos+i;
		if (j >= savecnt)
			continue;

		if (saves_has_error(j))
			continue;

		if (auto r = saves_get_slot(j); !r)
		{
			enter_sys_pal();
			InfoDialog("Error loading save file", r.error()).show();
			exit_sys_pal();
		}
	}

	// Check if any loaded quests is the 4th quest.
	for(int32_t i=0; i<savecnt; i++)
	{
		if (!saves_is_slot_loaded(i))
			continue;

		if (auto r = saves_get_slot(i); r && r.value()->header->quest == 4)
		{
			red = true;
			break;
		}
	}

	loadpalset(0,red ? pSprite(spPILE) : 0);
	
	for(int32_t i=0; i<3; i++)
	{
		int slot = listpos+i;
		if (!saves_is_slot_loaded(slot))
			continue;

		save_t* save;
		if (auto r = saves_get_slot(slot); !r)
			continue;
		else save = r.value();

		list_save(save->header, slot, i*24+56);
	}
	
	// Draw the arrows above the lifemeter!
	if(savecnt>3)
	{
		if(listpos>=3)
			textout_ex(framebuf,get_zc_font(font_zfont),(char *)left_arrow_str,96,60,3,0);
			
		if(listpos+3<savecnt)
			textout_ex(framebuf,get_zc_font(font_zfont),(char *)right_arrow_str,176,60,3,0);
			
		textprintf_ex(framebuf,get_zc_font(font_zfont),112,60,3,0,"%2d - %-2d",listpos+1,listpos+3);
	}
}

static void draw_cursor(int32_t pos,int32_t mode)
{
	int32_t cs = (mode==3)?13:9;
	if(pos<3)
		overtile8(framebuf,0,40,pos*24+77,cs,0);
	else
		overtile8(framebuf,0,40,(pos-3)*16+153,cs,0);
}

static bool register_name()
{
	int s = saves_count();

	int32_t NameEntryMode2=NameEntryMode;

	gamedata* new_game = new gamedata();
	new_game->set_maxlife(3*16);
	new_game->set_life(3*16);
	new_game->set_maxbombs(8);
	new_game->set_continue_dmap(0);
	new_game->set_continue_scrn(0xFF);

	listpos=((saves_count())/3)*3;

//  clear_bitmap(framebuf);
	rectfill(framebuf,32,56,223,151,0);
	list_saves();
	blit(framebuf,scrollbuf,0,0,0,0,framebuf->w,framebuf->h);
	
	int32_t pos=s%3;
	int32_t y=((NameEntryMode2>0)?0:(pos*24))+72;
	int32_t x=0;
	int32_t spos=0;
	char name[9];
	
	memset(name,0,9);
	register_mode();
	clear_keybuf();
	SystemKeys=(NameEntryMode2>0);
	refreshpal=true;
	bool done=false;
	bool cancel=false;

	if (!load_qstpath.empty()) {
		new_game->header.qstpath = load_qstpath;

		std::string filename = get_filename(load_qstpath.c_str());
		filename.erase(remove(filename.begin(), filename.end(), ' '), filename.end());
		auto len = filename.find(".qst", 0);
		len = zc_min(len, 8);
		strcpy(name, filename.substr(0, len).c_str());
		x = strlen(name);
	}
	else if (!only_qstpath.empty())
	{
		new_game->header.qstpath = only_qstpath;
	}
	
	int32_t letter_grid_x=(NameEntryMode2==2)?34:44;
	int32_t letter_grid_y=120;
	int32_t letter_grid_offset=(NameEntryMode2==2)?10:8;
	int32_t letter_grid_width=(NameEntryMode2==2)?16:11;
	int32_t letter_grid_height=(NameEntryMode2==2)?6:4;
	int32_t letter_grid_spacing=(NameEntryMode2==2)?12:16;
	
	const char *simple_grid="ABCDEFGHIJKLMNOPQRSTUVWXYZ-.,!'&.0123456789 ";
	const char *complete_grid=" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";
	
	if(NameEntryMode2>0)
	{
		//int32_t pos=file%3;
		BITMAP *info = create_bitmap_ex(8,168,32);
		clear_bitmap(info);
		blit(framebuf,info,40,pos*24+70,0,0,168,26);
		rectfill(info,40,0,168,1,0);
		rectfill(info,0,24,39,25,0);
		rectfill(info,0,0,7,15,0);
		rectfill(framebuf,40,64,216,192,0);
		rectfill(framebuf,96,60,183,67,0);
		
		int32_t i=pos*24+70;
		
		do
		{
			blit(info,framebuf,0,0,40,i,168,32);
			advanceframe(true);
			i-=pos+pos;
		}
		while(pos && i>=70);
		
		clear_bitmap(framebuf);
		framerect(framebuf, 27, 51, 26*8-6, 8*8-6,0x03);
		textout_ex(framebuf,get_zc_font(font_zfont)," NAME ",80,48,1,0);
		textout_ex(framebuf,get_zc_font(font_zfont)," LIFE ",152,48,1,0);
		
		blit(info,framebuf,0,0,40,70,168,32);
		destroy_bitmap(info);
		
		framerect(framebuf, letter_grid_x-letter_grid_offset+3, letter_grid_y-letter_grid_offset+3,
			((NameEntryMode2==2)?26:23)*8-6, ((NameEntryMode2==2)?11:9)*8-6,0x03);
		
		if(NameEntryMode2==1)
		{
			textout_ex(framebuf,get_zc_font(font_zfont),"A B C D E F G H I J K",letter_grid_x,letter_grid_y,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"L M N O P Q R S T U V",letter_grid_x,letter_grid_y+16,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"W X Y Z - . , ! ' & .",letter_grid_x,letter_grid_y+32,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"0 1 2 3 4 5 6 7 8 9  ",letter_grid_x,letter_grid_y+48,1,-1);
		}
		else
		{
			textout_ex(framebuf,get_zc_font(font_zfont),"   \"  $  &  (  *  ,  .",letter_grid_x,   letter_grid_y,   1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "!  #  %  '  )  +  -  /",letter_grid_x+12,letter_grid_y,   1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"0  2  4  6  8  :  <  >", letter_grid_x,   letter_grid_y+12,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "1  3  5  7  9  ;  =  ?",letter_grid_x+12,letter_grid_y+12,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"@  B  D  F  H  J  L  N", letter_grid_x,   letter_grid_y+24,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "A  C  E  G  I  K  M  O",letter_grid_x+12,letter_grid_y+24,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"P  R  T  V  X  Z  \\  ^",letter_grid_x,   letter_grid_y+36,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "Q  S  U  W  Y  [  ]  _",letter_grid_x+12,letter_grid_y+36,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"`  b  d  f  h  j  l  n", letter_grid_x,   letter_grid_y+48,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "a  c  e  g  i  k  m  o",letter_grid_x+12,letter_grid_y+48,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"p  r  t  v  x  z  |  ~", letter_grid_x,   letter_grid_y+60,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "q  s  u  w  y  {  }",   letter_grid_x+12,letter_grid_y+60,1,-1);
		}
		
		advanceframe(true);
		blit(framebuf,scrollbuf,0,0,0,0,framebuf->w,framebuf->h);
		
	}
	
	int32_t grid_x=0;
	int32_t grid_y=0;
	
	
	do
	{
		if(NameEntryMode2>0)
		{
			spos = grid_y*letter_grid_width+grid_x;
			load_control_state();
			
			if(rLeft())
			{
				--grid_x;
				
				if(grid_x<0)
				{
					grid_x=letter_grid_width-1;
					--grid_y;
					
					if(grid_y<0)
					{
						grid_y=letter_grid_height-1;
					}
				}
				
				sfx(WAV_CHIME);
			}
			else if(rRight())
			{
				++grid_x;
				
				if(grid_x>=letter_grid_width)
				{
					grid_x=0;
					++grid_y;
					
					if(grid_y>=letter_grid_height)
					{
						grid_y=0;
					}
				}
				
				sfx(WAV_CHIME);
			}
			else if(rUp())
			{
				--grid_y;
				
				if(grid_y<0)
				{
					grid_y=letter_grid_height-1;
				}
				
				sfx(WAV_CHIME);
			}
			else if(rDown())
			{
				++grid_y;
				
				if(grid_y>=letter_grid_height)
				{
					grid_y=0;
				}
				
				sfx(WAV_CHIME);
			}
			else if(rBbtn())
			{
				if(x<8 && name[zc_min(x,7)])
				{
					++x;
					sfx(WAV_CHIME);
				}
				
				if(x>=8)
				{
					x=0;
				}
			}
			else if(rAbtn())
			{
				name[zc_min(x,7)]=(NameEntryMode2==2)?complete_grid[spos]:simple_grid[spos];
				++x;
				
				if(x>=8)
				{
					x=0;
				}
				
				sfx(WAV_PLACE);
			}
			else if(rSbtn())
			{
				done=true;
				int32_t ltrs=0;
				
				for(int32_t i=0; i<8; i++)
				{
					if(name[i]!=' ' && name[i]!=0)
					{
						++ltrs;
					}
				}
				
				if(!ltrs)
				{
					cancel=true;
				}
			}
			
		}
		else
		{
#ifdef __EMSCRIPTEN__
			// Allow gamepad to submit name.
			poll_joystick();
			load_control_state();
			if(rSbtn())
			{
				done = true;
				break;
			}
#endif

			if(keypressed())
			{
				int32_t k=readkey();
				
				if(isprint(k&255))
				{
					name[zc_min(x,7)]=k&0xFF;
					
					if(x<8)
					{
						++x;
					}
					
					sfx(WAV_PLACE);
				}
				else
				{
					switch(k>>8)
					{
					case KEY_LEFT:
						if(x>0)
						{
							if(x==8)
							{
								x=6;
							}
							else
							{
								--x;
							}
							
							sfx(WAV_CHIME);
						}
						
						break;
						
					case KEY_RIGHT:
						if(x<8 && name[zc_min(x,7)])
						{
							++x;
							sfx(WAV_CHIME);
						}
						
						break;
						
					case KEY_ENTER:
					case KEY_ENTER_PAD:
					{
						done=true;
						int32_t ltrs=0;
						
						for(int32_t i=0; i<8; i++)
						{
							if(name[i]!=' ' && name[i]!=0)
							{
								++ltrs;
							}
						}
						
						if(!ltrs)
						{
							cancel=true;
						}
					}
					break;
					
					case KEY_BACKSPACE:
						if(x>0)
						{
							--x;
							
							for(int32_t i=zc_min(x,7); i<8; i++)
							{
								name[i]=name[i+1];
							}
							
							sfx(WAV_OUCH);
						}
						
						break;
						
					case KEY_DEL:
						for(int32_t i=zc_min(x,7); i<8; i++)
						{
							name[i]=name[i+1];
						}
						
						sfx(WAV_OUCH);
						break;
						
					case KEY_ESC:
						x=-1;
						done=true;
						
						while(key[KEY_ESC])
						{
							poll_keyboard();
							/* do nothing */
							rest(1);
						}
						
						break;
					}
				}
			}
		}
		
		new_game->set_name(name);
		blit(scrollbuf,framebuf,0,0,0,0,framebuf->w,framebuf->h);
		list_save(&new_game->header, s, 56+((NameEntryMode2>0)?0:(pos*24)));
		
		int32_t x2=letter_grid_x + grid_x*letter_grid_spacing;
		int32_t y2=letter_grid_y + grid_y*letter_grid_spacing;
		
		if(frame&8)
		{
			int32_t tx=(zc_min(x,7)<<3)+72;
			
			for(int32_t dy=0; dy<8; dy++)
			{
				for(int32_t dx=0; dx<8; dx++)
				{
					if(framebuf->line[y+dy][tx+dx]==0)
					{
						framebuf->line[y+dy][tx+dx]=CSET(9)+1;
					}
					
					if(NameEntryMode2>0)
					{
						if(framebuf->line[y2+dy][x2+dx]==0)
						{
							framebuf->line[y2+dy][x2+dx]=CSET(9)+1;
						}
					}
				}
			}
		}
		
		draw_cursor((NameEntryMode2>0)?0:pos,0);
		advanceframe(true);
		/*
		  if(rBbtn())
		  {
		  x=-1;
		  done=true;
		  }
		  */
	}
	while(!done && !Quit);
	
	if(x<0 || cancel)
	{
		done=false;
	}
	
	if(done)
	{
		new_game->set_quest(0);
		new_game->set_timevalid(1);
		game->header.qstpath.clear();

		if (auto r = saves_create_slot(new_game); !r)
		{
			cancel = true;
			ringcolor(false);

			// Could have been canceled in the file dialog, in which case there is no error message.
			if (!r.error().empty())
			{
				enter_sys_pal();
				InfoDialog("Error creating save file", r.error()).show();
				exit_sys_pal();
			}
		}
	}
	else
	{
		delete new_game;
		new_game = nullptr;
	}

	listpos=((saves_count()-1)/3)*3;
	
	SystemKeys=true;
	selectscreen();
	list_saves();
	select_mode();
	return done && new_game != nullptr;
}

static bool copy_file(int32_t file)
{
	int savecnt = saves_count();
	if (file >= savecnt)
		return false;

	std::string err;
	if (!saves_copy(file, err))
	{
		enter_sys_pal();
		InfoDialog("Error copying save", err).show();
		exit_sys_pal();
		return false;
	}

	listpos=((saves_count()-1)/3)*3;
	sfx(WAV_SCALE);
	select_mode();
	return true;
}

static bool delete_save(int32_t file)
{
	int savecnt = saves_count();

	if (file < savecnt)
	{
		std::string err;
		if (!saves_delete(file, err))
		{
			enter_sys_pal();
			InfoDialog("Error deleting save", err).show();
			exit_sys_pal();
			return false;
		}

		--savecnt;
		if(listpos>savecnt-1)
			listpos=zc_max(listpos-3,0);

		sfx(WAV_OUCH);
		select_mode();
		return true;
	}

	return false;
}

/** game mode stuff **/

DIALOG gamemode_dlg[] =
{
	// (dialog proc)      (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)     (d1)           (d2)     (dp)                            (dp2)  (dp3)
	{ jwin_win_proc,       40,   44,  240,  180,   0,       0,       0,       D_EXIT,     0,             0, (void *) "Select Custom Quest", NULL,  NULL },
	// 1
	{ jwin_button_proc,   205,   76,   61,   21,   0,       0,       'b',     D_EXIT,     0,             0, (void *) "&Browse",             NULL,  NULL },
	{ jwin_textbox_proc,   55,   78,  140,   16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_text_proc,      76,  106,   64,    8,   vc(0),   vc(11),  0,       0,          0,             0, (void *) "Info:",               NULL,  NULL },
	{ jwin_textbox_proc,   76,  118,  168,   60,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	// 5
	{ jwin_button_proc,    90,  191,   61,   21,   0,       0,       'k',     D_EXIT,     0,             0, (void *) "O&K",                 NULL,  NULL },
	{ jwin_button_proc,   170,  191,   61,   21,   0,       0,       27,      D_EXIT,     0,             0, (void *) "Cancel",              NULL,  NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


static int32_t get_quest_info(zquestheader *header,char *str)
{
	if(strlen(get_filename(qstpath)) == 0)
	{
		str[0]=0;
		return 0;
	}

	int32_t error;
	PACKFILE* f = open_quest_file(&error, qstpath, false);

	if (!f)
	{
		strcpy(str,"Error: Unable to open file");
		return 0;
	}

	int32_t ret = readheader(f, header);
	pack_fclose(f);
	clear_quest_tmpfile();

	switch(ret)
	{
		case 0:
			break;
			
		case qe_invalid:
			strcpy(str,"Error: Invalid quest file");
			return 0;
			
		case qe_version:
			strcpy(str,"Error: Invalid version");
			return 0;
			
		case qe_obsolete:
			strcpy(str,"Error: Obsolete version");
			return 0;
			
		case qe_silenterr:
			return 0;
	}
	
	strcpy(str,"Title:\n");
	strcat(str,header->title);
	strcat(str,"\n\nAuthor:\n");
	strcat(str,header->author);

	return 1;
}

bool prompt_for_quest_path(std::string current_qstpath)
{
	if (!only_qstpath.empty())
	{
		strcpy(qstpath, only_qstpath.c_str());
		return true;
	}

	zquestheader h;
	char infostr[200];
	infostr[0] = '\0';
	int32_t ret=0; 
	int32_t focus_obj = 1; //Fixes the issue where the button tied to the enter key is stuck on 'browse'.

	bool jump_to_file_select = false;
	if (current_qstpath.empty())
	{
		jump_to_file_select = true;
	}
	else if (is_relative_filename(current_qstpath.c_str()))
	{
		// TODO: make `qstpath` use type fs::path
		auto qstpath_fs = fs::path(qstdir) / fs::path(current_qstpath);
		sprintf(qstpath, "%s", qstpath_fs.string().c_str());
	}
	else
	{
		sprintf(qstpath, "%s", current_qstpath.c_str());
	}
	char relpath[2048];
	relativize_path(relpath, qstpath);
	
	gamemode_dlg[0].dp2 = get_zc_font(font_lfont);
	gamemode_dlg[2].dp = relpath;
	
	if (!current_qstpath.empty() && get_quest_info(&h,infostr)==0)
	{
		gamemode_dlg[4].dp = infostr;
		gamemode_dlg[5].flags = D_DISABLED;
	}
	else
	{
		gamemode_dlg[4].dp = infostr;
		gamemode_dlg[5].flags = D_EXIT;
	}
	
	gamemode_dlg[2].d1 = gamemode_dlg[4].d1 = 0;
	gamemode_dlg[2].d2 = gamemode_dlg[4].d2 = 0;
	enter_sys_pal();
	
	clear_keybuf();
	
	large_dialog(gamemode_dlg);
   
	bool chose_quest = false;
	while(jump_to_file_select || (ret=do_zqdialog(gamemode_dlg,focus_obj))==1)
	{
		jump_to_file_select = false;
		blit(screen,tmp_scr,scrx,scry,0,0,320,240);
		
		int32_t  sel=0;
		static EXT_LIST list[] =
		{
			{ (char *)"ZC Quests (*.qst)", (char *)"qst" },
			{ NULL,                        NULL }
		};
		
		if (auto result = prompt_for_existing_file("Load Quest", "", list, qstpath))
		{
			std::string path = *result;
			chose_quest = true;
			replace_extension(qstpath,path.data(),"qst",2047);
			gamemode_dlg[2].dp = get_filename(qstpath);
			
			if(get_quest_info(&h,infostr)==0)
			{
				gamemode_dlg[4].dp = infostr;
				gamemode_dlg[5].flags = D_DISABLED;
				focus_obj = 1;
			}
			else
			{
				gamemode_dlg[4].dp = infostr;
				gamemode_dlg[5].flags = D_EXIT;
				focus_obj = 5;
			}
			
			gamemode_dlg[2].d1 = gamemode_dlg[4].d1 = 0;
			gamemode_dlg[2].d2 = gamemode_dlg[4].d2 = 0;
		}
		else if (current_qstpath.empty())
		{
			break;
		}
		
		blit(tmp_scr,screen,0,0,scrx,scry,320,240);
	}

	exit_sys_pal();
	key[KEY_ESC]=0;

	chose_quest = ret == 5 && chose_quest;
	if (chose_quest)
	{
		// Try to make relative to qstdir.
		// TODO: this is copied from saves_do_first_time_stuff
		std::string rel_dir = (fs::current_path() / fs::path(qstdir)).string();
		auto maybe_rel_qstpath = util::is_subpath_of(rel_dir, qstpath) ? fs::relative(qstpath, rel_dir) : qstpath;
		strcpy(qstpath, maybe_rel_qstpath.string().c_str());
	}

	return chose_quest;
}

static int32_t game_details(save_t* save)
{
	if (!save)
		return 0;

	const auto header = save->header;

	if (header->quest==0)
		return 0;

	int32_t pos=save->index%3;
	BITMAP *info = create_bitmap_ex(8,168,32);
	clear_bitmap(info);
	blit(framebuf,info,40,pos*24+70,0,0,168,26);
	rectfill(info,40,0,168,1,0);
	rectfill(info,0,24,39,25,0);
	rectfill(info,0,0,7,15,0);
	rectfill(framebuf,40,64,216,192,0);
	rectfill(framebuf,96,60,183,67,0);
	
	int32_t i=pos*24+70;
	
	do
	{
		blit(info,framebuf,0,0,40,i,168,32);
		advanceframe(true);
		i-=pos+pos;
	}
	while(pos && i>=70);
	
	destroy_bitmap(info);
	
	textout_ex(framebuf,get_zc_font(font_zfont),"GAME TYPE",40,104,3,0);
	textout_ex(framebuf,get_zc_font(font_zfont),"QUEST",40,112,3,0);
	textout_ex(framebuf,get_zc_font(font_zfont),"STATUS",40,120,3,0);

	if (!save->path.empty())
		textout_centre_ex(framebuf,get_zc_font(font_z3smallfont),save->path.filename().string().c_str(),125,132,1,0);

	if(header->quest<0xFF)
	{
		textout_ex(framebuf,get_zc_font(font_zfont),"Normal Game",120,104,1,0);
		textprintf_ex(framebuf,get_zc_font(font_zfont),120,112,1,0,"%s Quest",
					  ordinal(header->quest));
	}
	else
	{
		textout_ex(framebuf,get_zc_font(font_zfont),"Custom Quest",120,104,1,0);
		textprintf_ex(framebuf,get_zc_font(font_zfont),120,112,1,0,"%s",
					  get_filename(header->qstpath.c_str()));
	}
	
	if(!header->has_played)
		textout_ex(framebuf,get_zc_font(font_zfont),"Not Started",120,120,1,0);
	else if(!header->time_valid)
		textout_ex(framebuf,get_zc_font(font_zfont),"Time Unknown",120,120,1,0);
	else
		textout_ex(framebuf,get_zc_font(font_zfont),time_str_med(header->time),120,120,1,0);
		
	if(header->did_cheat)
		textout_ex(framebuf,get_zc_font(font_zfont),"Used Cheats",120,128,1,0);
		
	textout_ex(framebuf,get_zc_font(font_zfont),"START: PLAY GAME",56,152,1,0);
	textout_ex(framebuf,get_zc_font(font_zfont),"    B: CANCEL",56,168,1,0);
	
	if(!header->has_played)
		textout_ex(framebuf,get_zc_font(font_zfont),"    A: CUSTOM QUEST",56,184,1,0);
		
	while(!Quit)
	{
		advanceframe(true);
		load_control_state();
		
		if(rBbtn())
		{
			blit(scrollbuf,framebuf,0,0,0,0,framebuf->w,framebuf->h);
			return 0;
		}
		
		if(rSbtn())
		{
			blit(framebuf,scrollbuf,0,0,0,0,framebuf->w,framebuf->h);
			return 1;
		}

		// TODO: consider allowing qst file to be reconfigured, in case it is moved.
		if(rAbtn() && !header->has_played)
		{
			if (prompt_for_quest_path(save->header->qstpath))
			{
				if (auto r = saves_update_slot(save, qstpath); !r)
				{
					enter_sys_pal();
					InfoDialog("Error updating save", r.error()).show();
					exit_sys_pal();
				}
				selectscreen();
				break;
			}
		}
	}
	
	return 0;
}

// The save slot that the title screen cursor is currently on.
static int32_t saveslot = -1;

save_t* get_unset_save_slot()
{
	if (auto r = saves_get_slot(saveslot); r && r.value()->header->quest && r.value()->header->has_played)
		return r.value();

	return nullptr;
}

static int last_slot_pos;
static void select_game(bool skip = false)
{
	if(standalone_mode || skip)
		return;

	int32_t pos = last_slot_pos;
	int32_t mode = 0;
	saves_unselect();
	
	//kill_sfx();
	
	//  text_mode(0);
	selectscreen();
	
	if (saves_count() == 0)
		pos=3;

	saveslot = pos + listpos;

	bool done=false;
	refreshpal=true;
	do
	{
		if (keypressed())
		{
			int32_t k=readkey()>>8;
			if (k == KEY_ESC && mode)
			{
				mode = 0;
				select_mode();
				while(key[KEY_ESC])
				{
					poll_keyboard();
					/* do nothing */
					rest(1);
				}
			}
		}
		disabledKeys[KEY_ESC] = mode != 0;

		sfxdat=1;
		blit(scrollbuf,framebuf,0,0,0,0,framebuf->w,framebuf->h);
		list_saves();
		draw_cursor(pos,mode);
		advanceframe(true);
		load_control_state();

		saveslot = pos + listpos;

		if(!load_qstpath.empty())
		{
			if (register_name())
			{
				saveslot = saves_count()-1;
				if (auto r = saves_select(saveslot); !r)
				{
					enter_sys_pal();
					InfoDialog("Error loading save", r.error()).show();
					exit_sys_pal();
					continue;
				}
				else
				{
					loadlast = saves_current_selection() + 1;
				}

				break;
			}
			else
			{
				load_qstpath = "";
			}
		}
		
		if(rSbtn())
			switch(pos)
			{
			case 3:
				if(!register_name())
				{
					// canceled.
					pos = 3;
				}
				else
				{
					// new save slot was created, and has a qstpath set.
					pos = (saves_count()-1)%3;
				}
				refreshpal=true;
				break;
				
			case 4:
			{
				int savecnt = saves_count();
				if (savecnt)
				{
					mode=2;
					pos=0;
					copy_mode();
				}
				
				refreshpal=true;
			}
			break;
				
			case 5:
				if(saves_count())
				{
					mode=3;
					pos=0;
					delete_mode();
				}
				
				refreshpal=true;
				break;
				
			default:
				if (saveslot < saves_count()) switch(mode)
				{
				case 0:
					if (auto r = saves_select(saveslot); !r)
					{
						enter_sys_pal();
						InfoDialog("Error loading save file", r.error()).show();
						exit_sys_pal();
					}
					else
					{
						loadlast = saves_current_selection() + 1;
						if (r.value()->header->quest)
							done=true;
					}
					break;

				case 2:
					if(copy_file(saveslot))
					{
						mode=0;
						pos=(saves_count()-1)%3;
						refreshpal=true;
					}
					
					break;
					
				case 3:
					if(delete_save(saveslot))
					{
						mode=0;
						refreshpal=true;
					}

					pos = 5;
					break;
				}
			}
			
		if(rUp())
		{
			--pos;
			
			if(pos<0)
				pos=(mode)?2:5;
				
			sfx(WAV_CHIME);
		}
		
		if(rDown())
		{
			++pos;
			
			if(pos>((mode)?2:5))
				pos=0;
				
			sfx(WAV_CHIME);
		}
		
		if(rLeft() && listpos>2)
		{
			listpos-=3;
			sfx(WAV_CHIME);
			refreshpal=true;
		}
		
		if(rRight() && listpos+3<saves_count())
		{
			listpos+=3;
			sfx(WAV_CHIME);
			refreshpal=true;
		}
		
		if(rBbtn() && mode)
		{
			if(mode==2) pos=4;
			
			if(mode==3) pos=5;
			
			mode=0;
			select_mode();
		}
		
		if (rAbtn() && !mode && pos<3 && saveslot < saves_count())
		{
			if (auto r = saves_get_slot(saveslot); !r)
			{
				enter_sys_pal();
				InfoDialog("Error loading save", r.error()).show();
				exit_sys_pal();
			}
			else
			{
				save_t* save = r.value();
				if (game_details(save) && saves_select(save))
				{
					loadlast = saves_current_selection() + 1;
					if (save->header->quest)
						done=true;
				}
			}
		}
	}
	while(!Quit && !done);

	disabledKeys[KEY_ESC] = false;
	saveslot = -1;
}

static void actual_titlescreen()
{
	if (is_headless())
		return;

	int starting_volume = 0;
	if (exists("assets/title_music.mp3"))
	{
		try_zcmusic("assets/title_music.mp3", "", 0, 0, starting_volume = get_emusic_volume());
	}
	else
	{
		try_zcmusic("assets/zc/ZC_Forever_HD.mp3", "", 0, 0, starting_volume = get_emusic_volume() * 0.7);
	}

	init_NES_mode();
	loadfullpal();
	loadlvlpal(1);

	auto logo = get_logo();
	if (logo->bitmap)
	{
		static float aspect_ratio = (float)al_get_bitmap_width(logo->bitmap) / al_get_bitmap_height(logo->bitmap);
		int target_height = 46;
		int target_width = target_height * aspect_ratio;
		float scale = (float)target_height / al_get_bitmap_height(logo->bitmap);
		int x = (al_get_bitmap_width(rti_game.bitmap) - target_width) / 2;
		logo->set_transform({.x = x, .y = -5, .xscale = scale, .yscale = scale});
	}

	int starting_y = rti_game.height + 46;
	int final_y = (rti_game.height - 46) / 2;

	bool show_text = false;
	int duration = 150;
	int counter = 0;
	while (!Quit)
	{
		if (keypressed() || button_pressed())
		{
			sfx(WAV_CHIME);
			break;
		}

		double ratio = (double)counter / duration;

		auto t = logo->get_transform();
		t.y = starting_y + (final_y - starting_y) * ratio;
		logo->set_transform(t);

		if (!show_text)
		{
			int pos = zcmusic_get_curpos(zcmusic);
			if (pos >= 0)
			{
				show_text = pos >= 59000;
			}
			else
			{
				// For if music is not playing.
				show_text = counter == duration;
			}
		}

		clear_bitmap(framebuf);
		if (show_text)
		{
			int w = text_length(get_zc_font(font_zfont), "PRESS ANYTHING TO START");
			textout_ex(framebuf,get_zc_font(font_zfont),"PRESS ANYTHING TO START",(framebuf->w - w)/2,framebuf->h - 20,WHITE,0);
		}

		if (counter < duration)
			counter++;
		advanceframe(true);
	}

	clear_bitmap(framebuf);

	starting_y = logo->get_transform().y;
	final_y = -5;
	duration = 30;
	counter = 0;
	while (!Quit && counter <= duration)
	{
		double ratio = (double)counter / duration;
		zcmusic_set_volume(zcmusic, starting_volume - starting_volume * std::sqrt(ratio));

		auto t = logo->get_transform();
		t.y = starting_y + (final_y - starting_y) * ratio;
		logo->set_transform(t);

		counter++;
		advanceframe(true);
	}

	zcmusic_stop(zcmusic);
}

static void prompt_for_uploading_replays()
{
#ifdef HAS_CURL
	bool replay_upload_prompt = zc_get_config("zeldadx", "replay_upload_prompt", 0);
	if (!replay_upload_auto_enabled() && !replay_upload_prompt)
	{
		enter_sys_pal();
		if(jwin_alert3(
			"Upload replays", 
			"Would you like to periodically upload replays of your gameplay to the developers?",
			"This helps development by preventing bugs and simplifying bug reports.",
			NULL,
		"&Yes", 
		"&No", 
		NULL, 
		'y', 
		'n', 
		0, 
		get_zc_font(font_lfont)) == 1)
		{
			zc_set_config("zeldadx", "replay_new_saves", true);
			zc_set_config("zeldadx", "replay_upload", true);
		}
		exit_sys_pal();
	}
	if (!replay_upload_prompt)
		zc_set_config("zeldadx", "replay_upload_prompt", 1);
#endif
}

void titlescreen(int32_t lsave)
{
	int32_t q=Quit;
	
	Quit=0;
	Playing=Paused=GameLoaded=false;
	FFCore.kb_typing_mode = false;
	FFCore.skip_ending_credits = 0;

	clear_keybuf();

	if(q==qCONT)
	{
		cont_game();
		return;
	}

	if (q == qRELOAD)
	{
		zcmusic_stop(zcmusic);
		init_game();
		return;
	}

	if (saves_current_selection() != -1)
	{
		last_slot_pos = saves_current_selection() % 3;
		saves_unload(saves_current_selection());
	}

	if (replay_is_active())
		replay_quit();

	updateShowBottomPixels();

	if (!SkipTitle && load_qstpath.empty() && lsave == 0 && !q)
	{
		actual_titlescreen();
	}

	if(!Quit)
	{
		if(lsave<1)
		{
			if(slot_arg)
			{
				int slot = slot_arg2 - 1;
				if (auto r = saves_select(slot); !r)
					Z_error_fatal("Cannot load slot %d, error %s:", slot, r.error().c_str());
				slot_arg = 0;
			}
			else
			{
				select_game(q==qRELOAD);
			}
		}
		else
		{
			int slot = lsave - 1;
			if (auto r = saves_select(slot); !r)
				Z_error_fatal("Cannot load slot %d, error %s:", slot, r.error().c_str());
		}
	}

	if(!Quit)
	{
		if (!game->get_hasplayed())
			prompt_for_uploading_replays();
		zcmusic_stop(zcmusic);
		init_game();
	}
}

void game_over(int32_t type)
{
	FFCore.kb_typing_mode = false; 
	FFCore.skip_ending_credits = 0;
	kill_sfx();
	music_stop();
	clear_bitmap(screen);
	clear_info_bmp();
	loadfullpal();
	
	
	if(Quit==qGAMEOVER)
		jukebox(SaveScreenSettings[SAVESC_MIDI] + (ZC_MIDI_COUNT - 1));
		
	Quit=0;
	
	clear_to_color(framebuf,SaveScreenSettings[SAVESC_BACKGROUND]);
	
	//Setting the colour via the array isn't working. Perhaps misc colours need to be assigned to the array in init.
	textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_CONTINUE],88,72,( SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] : QMisc.colors.msgtext) ,-1);
	//WTF! Setting this in zq Init() didn't work?! -Z
	if(!type)
	{
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE],88,96,( SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] : QMisc.colors.msgtext),-1);
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,120,( SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext),-1);
	}
	else
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,96,( SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext),-1);
		
	int32_t pos = 0;
	int32_t f=-1;
	int32_t htile = SaveScreenSettings[SAVESC_USETILE];
	int32_t curcset = SaveScreenSettings[SAVESC_CURSOR_CSET];
	bool done=false;

	if (replay_version_check(7))
	{
		zc_readrawkey(Skey, true);
	}
	else
	{
		do {
			load_control_state();
		}
		while(getInput(btnS, true, false, true));//rSbtn
	}
	
	do
	{
		load_control_state();
		
		if(f==-1)
		{
			if(getInput(btnUp, true, false, true))//rUp
			{
				sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
				pos=(pos==0)?2:pos-1;
				
				if(type)
				{
					if(pos==1) pos--;
				}
			}
			
			if(getInput(btnDown, true, false, true))//rDown
			{
				sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
				pos=(pos+1)%3;
				
				if(type)
				{
					if(pos==1) pos++;
				}
			}
			
			if(getInput(btnS, true, false, true)) ++f;//rSbtn
		}
		
		if(f>=0)
		{
			if(++f == 65)
				done=true;
				
			if(!(f&3))
			{
				bool flash = (f&4)!=0;
				
				switch(pos)
				{
				case 0:
					textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_CONTINUE],88,72,(flash ? ( SaveScreenSettings[SAVESC_TEXT_CONTINUE_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_CONTINUE_FLASH]
						: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] > 0 ? 
							SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] : QMisc.colors.msgtext)),-1);
					break;
					
				case 1:
					textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE],88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_SAVE_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE_FLASH]
						: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] > 0 ? 
							SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] : QMisc.colors.msgtext)),-1);
					break;
					
				case 2:
					if(!type)
						textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,120,(flash ? ( SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH]
							: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? 
								SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext)),-1);
					else textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH]
						: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? 
							SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext)),-1);
					
					break;
				}
			}
		}
		
		rectfill(framebuf,72,72,79,127,SaveScreenSettings[SAVESC_BACKGROUND]);
		overtile8(framebuf,htile,72,pos*(type?12:24)+72,curcset,SaveScreenSettings[SAVESC_CUR_FLIP]);
		advanceframe(true);
	}
	while(!Quit && !done);

	if (replay_is_debug())
		replay_step_comment("game_over selection made");
	
	reset_all_combo_animations();
	clear_bitmap(framebuf);
	advanceframe(true);
	
	if(done)
	{
		// This is always the last step before a game save replay is stopped. On replay_continue,
		// the frame_count is set to this step's frame + 1 to continue the recording–so this comment
		// is super important.
		replay_step_comment("game_over");

		if(pos)
		{
			Quit=qQUIT;
		}
		else
		{
			Quit=qCONT;
		}
		
		// Save and quit.
		if (pos == 1)
		{
			//run save scripts
			FFCore.runOnSaveEngine();
			// reset palette before drawing the file select.
			doClearTint();
			
			game->save_objects(ZScriptVersion::gc_arrays());
			saves_write();
			replay_step_comment("save game");
		}

		// Anything but Continue.
		if (pos != 0)
		{
			if (replay_get_mode() == ReplayMode::Record) replay_save();
		}
	}
}

void save_game(bool savepoint)
{
	//run save scripts
	FFCore.runOnSaveEngine();
	if(savepoint)
	{
		game->set_continue_scrn(home_screen);
		game->set_continue_dmap(cur_dmap);
		lastentrance_dmap = cur_dmap;
		lastentrance = game->get_continue_scrn();
	}
	
	game->save_objects(ZScriptVersion::gc_arrays());
	
	flushItemCache();
	
	saves_write();
	if (replay_get_mode() == ReplayMode::Record) replay_save();
}

bool save_game(bool savepoint, int32_t type)
{
	kill_sfx();
	//music_stop();
	clear_bitmap(screen);
	clear_info_bmp();
	loadfullpal();
	
	int32_t htile = SaveScreenSettings[SAVESC_USETILE];
	int32_t curcset = SaveScreenSettings[SAVESC_CURSOR_CSET];
	bool done=false;
	bool didsaved=false;
	FFCore.kb_typing_mode = false;
	do
	{
		int32_t pos = 0;
		int32_t f=-1;
		bool done2=false;
		clear_to_color(framebuf,SaveScreenSettings[SAVESC_BACKGROUND]);
		
		if(type)
		{
		//Migrate this to use SaveScreenColours[SAVESC_TEXT] and set that to a default
		//of QMisc.colors.msgtext when loading the quest in the loadquest function
		//for quests with a version < 0x254! -Z
			textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVEQUIT],88,72,( SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] : QMisc.colors.msgtext),-1);
		}
		else
		{
			textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE2],88,72,( SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] : QMisc.colors.msgtext),-1);
		}
		
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_DONTSAVE],88,96,( SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] : QMisc.colors.msgtext),-1);
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_QUIT],88,120,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
		
		rUp(); rDown(); rSbtn(); //eat inputs
		do
		{
			load_control_state();
			
			if(f==-1)
			{
				if(getInput(btnUp, true, false, true))//rUp
				{
					sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
					pos=(pos==0)?2:pos-1;
				}
				
				if(getInput(btnDown, true, false, true))//rDown
				{
					sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
					pos=(pos+1)%3;
				}
				
				if(getInput(btnS, true, false, true)) ++f;//rSbtn
			}
			
			if(f>=0)
			{
				if(++f == 65)
					done2=true;
					
				if(!(f&3))
				{
					bool flash = (f&4)!=0;
					
					switch(pos)
					{
					case 0:
						if(type)
							textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVEQUIT],88,72,(flash ? ( SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_FLASH]
								: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] > 0 ? 
									SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] : QMisc.colors.msgtext)),-1);
						else textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE2],88,72,(flash ? ( SaveScreenSettings[SAVESC_TEXT_SAVE2_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE2_FLASH]
								: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] > 0 ? 
									SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] : QMisc.colors.msgtext)),-1);
						
						break;
						
					case 1:
						textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_DONTSAVE],88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_DONTSAVE_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_DONTSAVE_FLASH]
							: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] > 0 ? 
								SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] : QMisc.colors.msgtext)),-1);
						break;
						
					case 2:
						textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_QUIT],88,120,(flash ? ( SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH]
							: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? 
								SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext)),-1);
						break;
					}
				}
			}
			
			rectfill(framebuf,72,72,79,127,SaveScreenSettings[SAVESC_BACKGROUND]);
			overtile8(framebuf,htile,72,pos*24+72,curcset,SaveScreenSettings[SAVESC_CUR_FLIP]);
			advanceframe(true);
		}
		while(!Quit && !done2);

		clear_bitmap(framebuf);
		
		if(done2)
		{
			//Quit = (pos==2) ? qQUIT : 0;
			if(pos==1||pos==0) done=true;
			
			if(pos==0)
			{
				//run save scripts
				FFCore.runOnSaveEngine();
				if(savepoint)
				{
					game->set_continue_scrn(home_screen);
					game->set_continue_dmap(cur_dmap);
					lastentrance_dmap = cur_dmap;
					lastentrance = game->get_continue_scrn();
				}
				
				game->save_objects(ZScriptVersion::gc_arrays());
				saves_write();
				if (replay_get_mode() == ReplayMode::Record) replay_save();
				didsaved=true;
				
				if(type)
				{
					Quit = qQUIT;
					done=true;
					skipcont=1;
				}
			}
			
			if(pos==2)
			{
				clear_to_color(framebuf,SaveScreenSettings[SAVESC_BACKGROUND]);
				textout_ex(framebuf,get_zc_font(font_zfont),"ARE YOU SURE?",88,72,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
				textout_ex(framebuf,get_zc_font(font_zfont),"YES",88,96,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
				textout_ex(framebuf,get_zc_font(font_zfont),"NO",88,120,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
				int32_t pos2=0;
				int32_t g=-1;
				bool done3=false;
				
				rUp(); rDown(); rSbtn(); //eat inputs
				do
				{
					load_control_state();
					
					if(g==-1)
					{
						if(getInput(btnUp, true, false, true))//rUp
						{
							sfx(WAV_CHINK);
							pos2=(pos2==0)?1:pos2-1;
						}
						
						if(getInput(btnDown, true, false, true))//rDown
						{
							sfx(WAV_CHINK);
							pos2=(pos2+1)%2;
						}
						
						if(getInput(btnS, true, false, true)) ++g;//rSbtn
					}
					
					if(g>=0)
					{
						if(++g == 65)
							done3=true;
							
						if(!(g&3))
						{
							bool flash = (g&4)!=0;
							
							switch(pos2)
							{
							case 0:
								textout_ex(framebuf,get_zc_font(font_zfont),"YES",88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH]
									: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? 
										SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext)),-1);
								break;
								
							case 1:
								textout_ex(framebuf,get_zc_font(font_zfont),"NO",88,120,(flash ? ( SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH]
									: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? 
										SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext)),-1);
								break;
								//case 2: textout_ex(framebuf,get_zc_font(font_zfont),"QUIT",88,120,c,-1);   break;
							}
						}
					}
					
					
					rectfill(framebuf,72,72,79,127,0);
					puttile8(framebuf,htile,72,pos2*24+96,1,0);
					advanceframe(true);
				}
				while(!Quit && !done3);
				
				clear_bitmap(framebuf);
				
				if(pos2==0)
				{
					Quit = qQUIT;
					done=true;
					skipcont=1;
				}
			}
		}
	}
	while(!Quit && !done);
	
	ringcolor(false);
	loadlvlpal(DMaps[cur_dmap].color);
	
	if(darkroom)
	{
		if(get_qr(qr_FADE))
		{
			interpolatedfade();
		}
		else
		{
			loadfadepal((DMaps[cur_dmap].color)*pdLEVEL+poFADE3);
		}
	}
	
	return didsaved;
}
