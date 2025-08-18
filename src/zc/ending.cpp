#include <cstring>
#include <stdio.h>

#include "base/qrs.h"
#include "base/msgstr.h"
#include "zc/ending.h"
#include "zc/zelda.h"
#include "base/zsys.h"
#include "play_midi.h"
#include "sprite.h"
#include "items.h"
#include "pal.h"
#include "zc/hero.h"
#include "zc/guys.h"
#include "zc/title.h"
#include "subscr.h"
#include "init.h"
#include "gamedata.h"
#include "zc/ffscript.h"
#include "zc/saves.h"
#include "zinfo.h"
#include "base/misctypes.h"
#include "music_playback.h"
#include "zscriptversion.h"

extern sprite_list  guys, items, Ewpns, Lwpns, chainlinks, decorations;

namespace
{
	const int32_t white = WHITE;
	const int32_t red   = CSET(csBOSS)+4;
	const int32_t blue  = CSET(csBOSS)+5;
	const int32_t green = CSET(csBOSS)+6;
	
	struct EndingTextLine
	{
		const char* text;
		int32_t xPos, yPos; // yPos determines when this line appears
		int32_t color;
	};
	
	const int32_t numCreditsLines = 12;
	const EndingTextLine credits[]=
	{
		{ " STAFF ",                104, 240, white },
		{ "EXECUTIVE",              40,  272, blue  },
		{ "PRODUCER... H.YAMAUCHI", 40,  280, blue  },
		{ "PRODUCER.... S.MIYAHON", 40,  320, green },
		{ "DIRECTOR.... S.MIYAHON", 40,  360, red   },
		{ "        ...... TEN TEN", 40,  384, red   },
		{ "DESIGNER...... TEN TEN", 40,  424, blue  },
		{ "PROGRAMMER.. T.NAKAZOO", 40,  464, green },
		{ "          ..... YACHAN", 40,  488, green },
		{ "          ... MARUMARU", 40,  512, green },
		{ "SOUND",                  40,  552, red   },
		{ "COMPOSER...... KONCHAN", 40,  560, red   }
	};
	
	const int32_t numQuest1EndLines = 4;
	const EndingTextLine quest1End[]=
	{
		{ "ANOTHER QUEST WILL START", 32, 656, white },
		{ "FROM HERE.",               88, 672, white },
		{ "PRESS THE START BUTTON.",  40, 696, white },
		{ "\2731986 NINTENDO",        72, 760, white }
	};
	
	const int32_t numQuest2EndLines = 4;
	const EndingTextLine quest2End[]=
	{
		{ "YOU ARE GREAT.",      72, 768, white },
		/* name - deaths */
		{ "YOU HAVE AN AMAZING", 48, 816, white },
		{ "WISDOM AND POWER.",   64, 832, white },
		{ "\2731986 NINTENDO",   72, 912, blue  }
	};
	
	const int32_t numQuest34EndLines = 6;
	const EndingTextLine quest34End[]=
	{
		{ "YOU ARE GREAT.",            72,  768, white },
		/* name - deaths */
		{ "YOU HAVE AN AMAZING",       48,  816, white },
		{ "WISDOM AND POWER.",         64,  832, white },
		{ "END OF",                    104, 880, white },
		{ "\"THE LEGEND OF ZELDA 1\"", 32,  896, white },
		{ "\2731986 NINTENDO",         72,  912, blue  }
	};
	
	const int32_t numCustomQuestEndLines = 4;
	const EndingTextLine customQuestEnd[]=
	{
		{ "Congratulations!", 64, 768, white },
		/* name - deaths */
		/* time */
		{ "You finished a",   72, 816, white },
		{ "custom quest.",    76, 832, white },
		{ "ZQUEST CLASSIC",   76, 880, white }
	};
}

void noproc() {}

void put_triforce()
{
	if(get_qr(qr_HOLDITEMANIMATION))
	{
		putitem2(framebuf,120,113-(get_qr(qr_NOITEMOFFSET)),iTriforce,lens_hint_item[iTriforce][0],lens_hint_item[iTriforce][1], 0);
		putitem2(framebuf,136,113-(get_qr(qr_NOITEMOFFSET)),iTriforce,lens_hint_item[iTriforce][0],lens_hint_item[iTriforce][1], 0);
	}
	else
	{
		putitem(framebuf,120,113-(get_qr(qr_NOITEMOFFSET)),iTriforce);
		putitem(framebuf,136,113-(get_qr(qr_NOITEMOFFSET)),iTriforce);
	}
}

void putendmsg(const char *s,int32_t x,int32_t y,int32_t speed,void(proc)())
{
	int32_t i=0;
	int32_t c=(int32_t)strlen(s)*speed;
	
	for(int32_t f=0; f<c && !Quit; f++)
	{
		if((f%speed)==0)
		{
			if(s[i]!=' ')
				sfx(WAV_MSG);
			
			textprintf_ex(framebuf,get_zc_font(font_zfont),x+(i<<3),y,WHITE,0,"%c",s[i]);
			++i;
		}
		
		proc();
		advanceframe(true);
	}
}

void brick(int32_t x,int32_t y)
{
	blit(scrollbuf_old,scrollbuf_old,256,0,x,y,8,8);
}

void endingpal()
{
	byte pal[16*3] =
	{
		0, 0, 0,                                                // clear
		63,63,63,                                               // white
		31,31,31,                                               // gray
		0, 0, 0,                                                // black
		63,14, 0,                                               // red
		26,34,63,                                               // blue
		22,54,21                                                // green
	};
	for (int i = 0; i < 16*3; i++)
		pal[i] = _rgb_scale_6[pal[i]];
	byte *hold = colordata;
	colordata = pal;
	loadpalset(csBOSS,0);
	colordata = hold;
	refreshpal = true;
}

void ending()
{
	
	/*
	  *************************
	  * WIN THE GAME SEQUENCE *
	  *************************
	  0    HERO at ZELDA's side
	  288  begin WIPE (8px per side per step, each 5 frames)
	  363  WIPE complete, DOT out, A/B items out
	  QMisc.colors.hero_dot = 255;
	  show_subscreen_items = false;
	  365  first MESSAGE character in
	  371  next character in
	  407  last character in
	  668  HERO out, ZELDA out
	  669  HERO in (TRIFORCE overhead), ZELDA in (TRIFORCE overhead)
	  733  BLUE flash bg
	  734  RED
	  735  GREEN
	  736  BLACK
	  ...
	  860  BLACK, HERO out, ZELDA out
	  861  HERO in, ZELDA in
	  927  first MSG character in
	  935  next character in
	  1335 last character in
	  1461 HERO out, ZELDA out
	  1493 begin SCROLL
	  */
	
	//get rid off all sprites but Hero and Zelda
	items.clear();
	Ewpns.clear();
	Lwpns.clear();
	chainlinks.clear();
	decorations.clear();
	kill_subscr_items();
	
	set_uformat(U_ASCII);
	kill_sfx();
	
	Quit=0;
	
	draw_screen_clip_rect_x1=0;
	draw_screen_clip_rect_x2=255;
	draw_screen_clip_rect_y1=0;
	draw_screen_clip_rect_y2=231;
	
	for(int32_t f=0; f<365; f++)
	{
	script_drawing_commands.Clear();
	if ( FFCore.doscript(ScriptType::Hero) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255  ) 
	{
		ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_WIN);
		--f; load_control_state(); goto adv;
	}
	if ( f == 0 ) { sfx(WAV_ZELDA); music_stop(); }
		if(f==363)
		{
			//363  WIPE complete, DOT out, A/B items out
			QMisc.colors.hero_dot = 255;
			show_subscreen_items = false;
			
			for(int32_t i = guys.Count() - 1; i >= 0; i--)
			{
				if(guys.spr(i)->id > gDUMMY9)
				{
					guys.del(i);
				}
			}
			
			guys.draw(framebuf,true);
			Hero.draw(framebuf);
		}
		
		if(f>=288 && ((f-288)%5 == 0))
		{
			//288  begin WIPE (8px per side per step, each 5 frames)
			//TODO::
			draw_screen_clip_rect_x1+=8;
			draw_screen_clip_rect_x2-=8;
		}
		adv:
		draw_screen();
		advanceframe(true,true,false);
		
		if(Quit) return;
	}
	
	clear_bitmap(msg_txt_display_buf);
	clear_bitmap(msg_bg_display_buf);
	draw_screen();
	advanceframe(true);
	
	const EndingTextLine* endText;
	int32_t numEndTextLines=0;
	int32_t deathsYPos=-1;
	int32_t timeYPos=-1;
	int32_t len=600*2;
	int32_t creditsLine=0;
	int32_t endTextLine=0;
	
	
	if ( FFCore.skip_ending_credits ) goto credits_skip;
	
	draw_screen_clip_rect_x1=0;
	draw_screen_clip_rect_x2=255;
	
	char tmpmsg[6][25];
	
	for(int32_t x=0; x<3; x++)
	{
		sprintf(tmpmsg[x], "%.24s", MsgStrings[QMisc.endstring].s.c_str()+(24*x));
	}
	
	for(int32_t x=0; x<3; x++)
	{
		sprintf(tmpmsg[x+3], "%.24s", MsgStrings[QMisc.endstring+1].s.c_str()+(24*x));
	}
	
	if(QMisc.endstring==0)
	{
		putendmsg("THANKS HERO,YOU'RE",32,96,6,noproc);
		putendmsg("THE HERO OF HYRULE.",32,112,6,noproc);
	}
	else
	{
		putendmsg(tmpmsg[0],32,80,6,noproc);
		putendmsg(tmpmsg[1],32,96,6,noproc);
		putendmsg(tmpmsg[2],32,112,6,noproc);
	}
	
	for(int32_t f=408; f<927; f++)
	{
		/*
		  668  HERO out, ZELDA out
		  669  HERO in (TRIFORCE overhead), ZELDA in (TRIFORCE overhead)
		  733  BLUE flash bg
		  734  RED
		  735  GREEN
		  736  BLACK
		  ...
		  860  BLACK, HERO out, ZELDA out
		  861  HERO in, ZELDA in
		  */
		if(f==668)
		{
			rectfill(framebuf,120,127,152,145,0); //y1 == 129 was showing tiny bits left over from Hero's sprite. 
			blit(framebuf, tmp_bmp, 120,113, 0,0, 32,32);
		}
		
		if(f==860)
		{
			blit(tmp_bmp, framebuf, 0,0, 120,113, 32,32);
		}
		
		if(f==669 || f==861)
		{
			overtile16(framebuf,36,120,129,6,0);//draw Zelda two-handed overhead
			overtile16(framebuf,BSZ?32:29,136,129,6,0);//draw Hero two-handed overhead
		}
		
		if(f==733)
		{
			blit(framebuf,scrollbuf_old,0,playing_field_offset!=0?176:0,0,0,256,passive_subscreen_height);
			
			for(int32_t y=0; y<224; y++)
			{
				for(int32_t x=0; x<256; x++)
				{
					if(!(framebuf->line[y][x]&15))
					{
						framebuf->line[y][x]=16;
					}
				}
			}
		}
		
		if(f>=733 && f<861)
		{
			static byte flash[4] = {0x12,0x16,0x2A,0x0F};
			RAMpal[16] = NESpal(flash[(f-733)&3]);
			refreshpal=true;
		}
		
		if(f==861)
		{
			blit(scrollbuf,framebuf,0,0,0,playing_field_offset!=0?176:0,256,passive_subscreen_height);
			try_zcmusic("zelda.nsf", qstpath, 1, ZC_MIDI_ENDING, get_emusic_volume());
			
			for(int32_t y=0; y<224; y++)
			{
				for(int32_t x=0; x<256; x++)
				{
					if(framebuf->line[y][x]==16)
					{
						framebuf->line[y][x]=0;
					}
				}
			}
		}
		
		if(f>668 && f!=860)
		{
			put_triforce();
		}
		
		advanceframe(true);
		
		if(Quit)
		{
			return;
		}
	}
	
	if(QMisc.endstring==0)
	{
		putendmsg("FINALLY,",96,160,8,put_triforce);
		putendmsg("PEACE RETURNS TO HYRULE.",32,176,8,put_triforce);
		putendmsg("THIS ENDS THE STORY.",48,200,8,put_triforce);
	}
	else
	{
		putendmsg(tmpmsg[3],32,160,6,noproc);
		putendmsg(tmpmsg[4],32,176,6,noproc);
		putendmsg(tmpmsg[5],32,200,6,noproc);
	}
	
	for(int32_t f=1336; f<1492; f++)
	{
		if(f<1461)
		{
			put_triforce();
		}
		
		if(f==1461)
		{
			blit(tmp_bmp,framebuf,0,0,120,113,32,32);
		}
		
		advanceframe(true);
		
		if(Quit)
		{
			return;
		}
	}
	
	clear_bitmap(scrollbuf_old);
	blit(framebuf,scrollbuf_old,0,0,0,0,framebuf->w,framebuf->h);
	endingpal();
	// draw the brick
	puttile16(scrollbuf_old,3,256,0,csBOSS,0);
	
	if(game->get_quest()>1)
	{
		len=720*2;
	}
	
	
	switch(game->get_quest())
	{
	case 1:
		endText=quest1End;
		numEndTextLines=numQuest1EndLines;
		break;
		
	case 2:
		endText=quest2End;
		numEndTextLines=numQuest2EndLines;
		deathsYPos=792;
		break;
		
	case 3:
	case 4:
		endText=quest34End;
		numEndTextLines=numQuest34EndLines;
		deathsYPos=792;
		break;
		
	default:
		endText=customQuestEnd;
		numEndTextLines=numCustomQuestEndLines;
		deathsYPos=784;
		timeYPos=800;
		break;
	}
	
	for(int32_t f=0; f<len; f++)
	{
		if(!(f&15))
		{
			int32_t y=(f>>1)+224;
			
			if(y>240 && y<584)
			{
				brick(24,224);
				brick(224,224);
			}
			
			if(y==240 || y==584)
			{
				for(int32_t x=24; x<=224; x+=8)
				{
					brick(x,224);
				}
			}
			
			if(creditsLine<numCreditsLines)
			{
				if(y==credits[creditsLine].yPos)
				{
					textout_ex(scrollbuf_old,
					           get_zc_font(font_zfont),
					           credits[creditsLine].text,
					           credits[creditsLine].xPos,
					           224,
					           credits[creditsLine].color,
					           0);
					creditsLine++;
				}
			}
			else
			{
				if(endTextLine<numEndTextLines &&
				   y==endText[endTextLine].yPos)
				{
					textout_ex(scrollbuf_old,
					           get_zc_font(font_zfont),
					           endText[endTextLine].text,
					           endText[endTextLine].xPos,
					           224,
					           endText[endTextLine].color,
					           0);
					endTextLine++;
				}
				else if(y==deathsYPos)
					textprintf_ex(scrollbuf_old, get_zc_font(font_zfont), 72, 224, red, 0,
								  "%-8s -%3d", game->get_name(), game->get_deaths());
				else if(y==timeYPos)
				{
					if(game->get_timevalid())
						textout_centre_ex(scrollbuf_old, get_zc_font(font_zfont), time_str_med(game->get_time()), 128, 224, blue, 0);
				}
			}
		}
		
		if(f==112)                                              // after subscreen has scrolled away
		{
			init_NES_mode();
			loadpalset(9,pSprite(spPILE));
			endingpal();
		}
		
		if(f&1)
		{
			blit(scrollbuf_old,scrollbuf_old,0,1,0,0,256,232);
		}
		
		blit(scrollbuf_old,framebuf,0,0,0,0,framebuf->w,framebuf->h);
		advanceframe(true);
		
		if(Quit)
		{
			return;
		}
		
		load_control_state();
		getInput(btnS, INPUT_PRESS | INPUT_IGNORE_DISABLE);
	}
	
	do
	{
		if(frame&1)
		{
			overtile16(framebuf,176,120,129,9,0);
		}
		
		overtile16(framebuf,175,120,129,((frame&8)>>3)+7,0);
		
		if(!(frame&1))
		{
			overtile16(framebuf,176,120,129,9,0);
		}
		
		advanceframe(true);
		
		if(Quit)
		{
			return;
		}
		
		load_control_state();
	}
	while (!getInput(btnS, INPUT_PRESS | INPUT_IGNORE_DISABLE));
	advanceframe(true);
	
	credits_skip:
	
	zc_stop_midi();
	if (get_qr(qr_OLD_SCRIPT_VOLUME))
	{
		//restore user volume if it was changed by script
		if (FFCore.coreflags & FFCORE_SCRIPTED_MIDI_VOLUME)
		{
			Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
			midi_volume = FFCore.usr_midi_volume;
			//		master_volume(-1,FFCore.usr_midi_volume);
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
	if (FFCore.coreflags & FFCORE_SCRIPTED_PANSTYLE)
	{
		pan_style = (int32_t)FFCore.usr_panstyle;
	}

	if(zcmusic != NULL)
	{
		zcmusic_stop(zcmusic);
		zcmusic_unload_file(zcmusic);
		zcmusic = NULL;
		zcmixer->newtrack = NULL;
	}
	FFCore.skip_ending_credits = 0;
	
	//  setPackfilePassword(datapwd);
	load_quest(game);
	//  setPackfilePassword(NULL);
	game->save_objects(ZScriptVersion::gc_arrays());
	game->set_continue_dmap(zinit.start_dmap);
	game->set_continue_scrn(0xFF);
	game->set_cont_hearts(zinit.cont_heart);
	saves_write();
	if (replay_get_mode() == ReplayMode::Record) replay_save();
}


void ending_scripted()
{
	items.clear();
	Ewpns.clear();
	Lwpns.clear();
	guys.clear();
	chainlinks.clear();
	decorations.clear();
	kill_subscr_items();
	clear_bitmap(msg_txt_display_buf);
	clear_bitmap(msg_bg_display_buf);
	dismissmsg();
	ALLOFF(true, true);
    
	//music_stop();
	kill_sfx();
	//sfx(WAV_ZELDA);
	Quit=0;
    
	draw_screen_clip_rect_x1=0;
	draw_screen_clip_rect_x2=255;
	draw_screen_clip_rect_y1=0;
	draw_screen_clip_rect_y2=231;
   
	for(int32_t f=0; f<77; f++)
	{
        
		if(f>=0 && ((f-0)%5 == 0))
		{
			draw_screen_clip_rect_x1+=8;
			draw_screen_clip_rect_x2-=8;
		}
        
		draw_screen();
		advanceframe(true);
        
		if(Quit) return;
	}
    
	clear_bitmap(msg_txt_display_buf);
	clear_bitmap(msg_bg_display_buf);
	draw_screen();
	advanceframe(true);
    
	draw_screen_clip_rect_x1=0;
	draw_screen_clip_rect_x2=255;
    
	clear_bitmap(scrollbuf_old);
	blit(framebuf,scrollbuf_old,0,0,0,0,framebuf->w,framebuf->h);
	endingpal();
    
        removeItemsOfFamily(game, itemsbuf, itype_ring);
        int32_t maxring = getHighestLevelOfFamily(&zinit,itemsbuf,itype_ring);
        
        if(maxring != -1)
        {
            getitem(maxring,true);
        }
        
        ringcolor(false);
	zc_stop_midi();
	if (get_qr(qr_OLD_SCRIPT_VOLUME))
	{
		//restore user volume if it was changed by script
		if (FFCore.coreflags & FFCORE_SCRIPTED_MIDI_VOLUME)
		{
			Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
			midi_volume = FFCore.usr_midi_volume;
			//		master_volume(-1,FFCore.usr_midi_volume);
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
	if(zcmusic != NULL)
	{
		zcmusic_stop(zcmusic);
		zcmusic_unload_file(zcmusic);
		zcmusic = NULL;
		zcmixer->newtrack = NULL;
	}
	FFCore.skip_ending_credits = 0;
    
	//  setPackfilePassword(datapwd);
	load_quest(game);
	game->header.title = QHeader.title;
	//  setPackfilePassword(NULL);
	game->save_objects(ZScriptVersion::gc_arrays());
    
	game->set_continue_dmap(zinit.start_dmap);
	game->set_continue_scrn(0xFF);
	game->set_cont_hearts(zinit.cont_heart);
	game->set_hasplayed(false);
	saves_write();
	if (replay_get_mode() == ReplayMode::Record) replay_save();
}
