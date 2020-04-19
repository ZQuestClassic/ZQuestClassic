//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  ending.cc
//
//  Ending code for Zelda Classic.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>
#include <stdio.h>

#include "ending.h"
#include "zelda.h"
extern zcmodule moduledata;
extern ZModule zcm;
#include "zsys.h"
#include "sprite.h"
#include "items.h"
#include "pal.h"
#include "link.h"
#include "guys.h"
#include "title.h"
#include "subscr.h"
#include "init.h"
#include "gamedata.h"

#include <fstream>
 
extern byte linear_quest_loading; 
extern LinkClass   Link;
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;

//extern bool draw_screen_clip_rect_show_link;
//extern bool draw_screen_clip_rect_show_guys;

inline bool file_exists(const char *filename) 
{
	std::ifstream ifile(filename);
	return (bool)ifile;
}

namespace
{
    const int white = WHITE;
    const int red   = CSET(csBOSS)+4;
    const int blue  = CSET(csBOSS)+5;
    const int green = CSET(csBOSS)+6;
    
    struct EndingTextLine
    {
        const char* text;
        int xPos, yPos; // yPos determines when this line appears
        int color;
    };
    
    const int numCreditsLines = 12;
    const EndingTextLine credits[]={
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
    
    const int numQuest1EndLines = 4;
    const EndingTextLine quest1End[]={
        { "ANOTHER QUEST WILL START", 32, 656, white },
        { "FROM HERE.",               88, 672, white },
        { "PRESS THE START BUTTON.",  40, 696, white },
        { "\2731986 NINTENDO",        72, 760, white }
    };
    
    const int numQuest2EndLines = 4;
    const EndingTextLine quest2End[]={
        { "YOU ARE GREAT.",      72, 768, white },
        /* name - deaths */
        { "YOU HAVE AN AMAZING", 48, 816, white },
        { "WISDOM AND POWER.",   64, 832, white },
        { "\2731986 NINTENDO",   72, 912, blue  }
    };
    
    const int numQuest34EndLines = 6;
    const EndingTextLine quest34End[]={
        { "YOU ARE GREAT.",            72,  768, white },
        /* name - deaths */
        { "YOU HAVE AN AMAZING",       48,  816, white },
        { "WISDOM AND POWER.",         64,  832, white },
        { "END OF",                    104, 880, white },
        { "\"THE LEGEND OF ZELDA 1\"", 32,  896, white },
        { "\2731986 NINTENDO",         72,  912, blue  }
    };
    
    const int numCustomQuestEndLines = 6;
    const EndingTextLine customQuestEnd[]={
        { "Congratulations!", 64, 768, white },
        /* name - deaths */
        /* time */
        { "You finished a",   72, 816, white },
        { "custom quest.",    76, 832, white },
        { "ZELDA CLASSIC",    76, 880, white },
        { "\2741999-2019",    88, 896, white },
        { "Armageddon Games", 64, 912, blue }
    };
}

void noproc() {}

void put_triforce()
{
    if(get_bit(quest_rules,qr_HOLDITEMANIMATION))
    {
        putitem2(framebuf,120,113-(get_bit(quest_rules, qr_NOITEMOFFSET)),iTriforce,lens_hint_item[iTriforce][0],lens_hint_item[iTriforce][1], 0);
        putitem2(framebuf,136,113-(get_bit(quest_rules, qr_NOITEMOFFSET)),iTriforce,lens_hint_item[iTriforce][0],lens_hint_item[iTriforce][1], 0);
    }
    else
    {
        putitem(framebuf,120,113-(get_bit(quest_rules, qr_NOITEMOFFSET)),iTriforce);
        putitem(framebuf,136,113-(get_bit(quest_rules, qr_NOITEMOFFSET)),iTriforce);
    }
}

void putendmsg(const char *s,int x,int y,int speed,void(proc)())
{
    int i=0;
    int c=(int)strlen(s)*speed;
    
    for(int f=0; f<c && !Quit; f++)
    {
        if((f%speed)==0)
        {
            if(s[i]!=' ')
                sfx(WAV_MSG);
                
            textprintf_ex(framebuf,zfont,x+(i<<3),y,WHITE,0,"%c",s[i]);
            ++i;
        }
        
        proc();
        advanceframe(true);
    }
}

void brick(int x,int y)
{
    blit(scrollbuf,scrollbuf,256,0,x,y,8,8);
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
      0    LINK at ZELDA's side
      288  begin WIPE (8px per side per step, each 5 frames)
      363  WIPE complete, DOT out, A/B items out
      QMisc.colors.link_dot = 255;
      show_subscreen_items = false;
      365  first MESSAGE character in
      371  next character in
      407  last character in
      668  LINK out, ZELDA out
      669  LINK in (TRIFORCE overhead), ZELDA in (TRIFORCE overhead)
      733  BLUE flash bg
      734  RED
      735  GREEN
      736  BLACK
      ...
      860  BLACK, LINK out, ZELDA out
      861  LINK in, ZELDA in
      927  first MSG character in
      935  next character in
      1335 last character in
      1461 LINK out, ZELDA out
      1493 begin SCROLL
      */
    
    //get rid off all sprites but Link and Zelda
    items.clear();
    Ewpns.clear();
    Lwpns.clear();
    Sitems.clear();
    chainlinks.clear();
    decorations.clear();
    
    music_stop();
    kill_sfx();
    sfx(WAV_ZELDA);
    Quit=0;
    
    game->set_cheat(game->get_cheat() | (cheat>1)?1:0);
    
    draw_screen_clip_rect_x1=0;
    draw_screen_clip_rect_x2=255;
    draw_screen_clip_rect_y1=0;
    draw_screen_clip_rect_y2=223;
    //draw_screen_clip_rect_show_link=true;
    //draw_screen_clip_rect_show_guys=false;
    
    for(int f=0; f<365; f++)
    {
        if(f==363)
        {
            //363  WIPE complete, DOT out, A/B items out
            QMisc.colors.link_dot = 255;
            show_subscreen_items = false;
            
            for(int i = guys.Count() - 1; i >= 0; i--)
            {
                if(guys.spr(i)->id > gDUMMY9)
                {
                    guys.del(i);
                }
            }
            
            guys.draw(framebuf,true);
            Link.draw(framebuf);
        }
        
        if(f>=288 && ((f-288)%5 == 0))
        {
            //288  begin WIPE (8px per side per step, each 5 frames)
            //TODO::
            draw_screen_clip_rect_x1+=8;
            draw_screen_clip_rect_x2-=8;
            //draw_screen_clip_rect_show_guys=true;
        }
        
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(Quit) return;
    }
    
    clear_bitmap(msgdisplaybuf);
    draw_screen(tmpscr);
    advanceframe(true);
    
    draw_screen_clip_rect_x1=0;
    draw_screen_clip_rect_x2=255;
    //draw_screen_clip_rect_show_guys=false;
    
    char tmpmsg[6][25];
    
    for(int x=0; x<3; x++)
    {
        sprintf(tmpmsg[x], "%.24s", MsgStrings[QMisc.endstring].s+(24*x));
        //    sprintf(tmpmsg[x], "%.24s", MsgStrings[QMisc.endstring].s+(24*x));
        //    strncpy(tmpmsg[x], MsgStrings[QMisc.endstring].s+(24*x), 24);
    }
    
    for(int x=0; x<3; x++)
    {
        sprintf(tmpmsg[x+3], "%.24s", MsgStrings[QMisc.endstring+1].s+(24*x));
        //    strncpy(tmpmsg[x+3], MsgStrings[QMisc.endstring+1].s+(24*x), 24);
    }
    
    if(QMisc.endstring==0)
    {
        putendmsg("THANKS LINK,YOU'RE",32,96,6,noproc);
        putendmsg("THE HERO OF HYRULE.",32,112,6,noproc);
    }
    else
    {
        putendmsg(tmpmsg[0],32,80,6,noproc);
        putendmsg(tmpmsg[1],32,96,6,noproc);
        putendmsg(tmpmsg[2],32,112,6,noproc);
    }
    
    for(int f=408; f<927; f++)
    {
        /*
          668  LINK out, ZELDA out
          669  LINK in (TRIFORCE overhead), ZELDA in (TRIFORCE overhead)
          733  BLUE flash bg
          734  RED
          735  GREEN
          736  BLACK
          ...
          860  BLACK, LINK out, ZELDA out
          861  LINK in, ZELDA in
          */
        if(f==668)
        {
            rectfill(framebuf,120,127,152,145,0); //y1 == 129 was showing tiny bits left over from Link's sprite. 
            blit(framebuf, tmp_bmp, 120,113, 0,0, 32,32);
        }
        
        if(f==860)
        {
            blit(tmp_bmp, framebuf, 0,0, 120,113, 32,32);
        }
        
        if(f==669 || f==861)
        {
            overtile16(framebuf,36,120,129,6,0);//draw Zelda two-handed overhead
            overtile16(framebuf,BSZ?32:29,136,129,6,0);//draw Link two-handed overhead
        }
        
        if(f==733)
        {
            blit(framebuf,scrollbuf,0,playing_field_offset!=0?168:0,0,0,256,passive_subscreen_height);
            
            for(int y=0; y<224; y++)
            {
                for(int x=0; x<256; x++)
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
            blit(scrollbuf,framebuf,0,0,0,playing_field_offset!=0?168:0,256,passive_subscreen_height);
            try_zcmusic((char*)"zelda.nsf", 1, ZC_MIDI_ENDING);
            
            for(int y=0; y<224; y++)
            {
                for(int x=0; x<256; x++)
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
    
    for(int f=1336; f<1492; f++)
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
    
    clear_bitmap(scrollbuf);
    blit(framebuf,scrollbuf,0,0,0,0,256,224);
    endingpal();
    // draw the brick
    puttile16(scrollbuf,3,256,0,csBOSS,0);
    
    int len=600*2;
    
    if(game->get_quest()>1)
    {
        len=720*2;
    }
    
    int creditsLine=0;
    int endTextLine=0;
    
    const EndingTextLine* endText;
    int numEndTextLines=0;
    int deathsYPos=-1;
    int timeYPos=-1;
    
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
    
    for(int f=0; f<len; f++)
    {
        if(!(f&15))
        {
            int y=(f>>1)+224;
            
            if(y>240 && y<584)
            {
                brick(24,224);
                brick(224,224);
            }
            
            if(y==240 || y==584)
            {
                for(int x=24; x<=224; x+=8)
                {
                    brick(x,224);
                }
            }
            
            if(creditsLine<numCreditsLines)
            {
                if(y==credits[creditsLine].yPos)
                {
                    textout_ex(scrollbuf,
                               zfont,
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
                    textout_ex(scrollbuf,
                               zfont,
                               endText[endTextLine].text,
                               endText[endTextLine].xPos,
                               224,
                               endText[endTextLine].color,
                               0);
                    endTextLine++;
                }
                else if(y==deathsYPos)
                    textprintf_ex(scrollbuf, zfont, 72, 224, red, 0,
                                  "%-8s -%3d", game->get_name(), game->get_deaths());
                else if(y==timeYPos)
                {
                    if(game->get_timevalid() && !game->get_cheat())
                        textout_centre_ex(scrollbuf, zfont, time_str_med(game->get_time()), 128, 224, blue, 0);
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
            blit(scrollbuf,scrollbuf,0,1,0,0,256,232);
        }
        
        blit(scrollbuf,framebuf,0,0,0,0,256,224);
        advanceframe(true);
        
        if(Quit)
        {
            return;
        }
        
        load_control_state();
        rSbtn();
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
    while(!rSbtn());
    
    if(game->get_quest()>0 && game->get_quest()<=9)
    {
        inc_quest();
        removeItemsOfFamily(game, itemsbuf, itype_ring);
        int maxring = getHighestLevelOfFamily(&zinit,itemsbuf,itype_ring);
        
        if(maxring != -1)
        {
            getitem(maxring,true);
        }
        
        ringcolor(false);
    }
    //the ring colour and quest icon weren't advencing on custom quests.
    //if( game->get_quest()==0 )
    //{
//	ringcolor(false);    
    //}
    
    
    stop_midi();
    
    if(zcmusic != NULL)
    {
        zcmusic_stop(zcmusic);
        zcmusic_unload_file(zcmusic);
        zcmusic = NULL;
    }
    
//  setPackfilePassword(datapwd);
    load_quest(game);
//  setPackfilePassword(NULL);
    saves[currgame] = *game;
    load_game_icon_to_buffer(false,currgame);
    load_game_icon(game,false,currgame);
    
    game->set_continue_dmap(zinit.start_dmap);
    game->set_continue_scrn(0xFF);
    game->set_cont_hearts(zinit.cont_heart);
    show_saving(scrollbuf);
    save_savedgames();
}


void ending_scripted()
{
    
    
    items.clear();
    Ewpns.clear();
    Lwpns.clear();
    guys.clear();
    Sitems.clear();
    chainlinks.clear();
    decorations.clear();
    clear_bitmap(msgdisplaybuf);
    dismissmsg();
    ALLOFF(true, true);
    
    //music_stop();
    kill_sfx();
    //sfx(WAV_ZELDA);
    Quit=0;
    
    game->set_cheat(game->get_cheat() | (cheat>1)?1:0);
    
    draw_screen_clip_rect_x1=0;
    draw_screen_clip_rect_x2=255;
    draw_screen_clip_rect_y1=0;
    draw_screen_clip_rect_y2=223;
    //draw_screen_clip_rect_show_link=true;
    //draw_screen_clip_rect_show_guys=false;
   
    for(int f=0; f<365-288; f++)
    {
        
        if(f>=0 && ((f-0)%5 == 0))
        {
            //288  begin WIPE (8px per side per step, each 5 frames)
            //TODO::
            draw_screen_clip_rect_x1+=8;
            draw_screen_clip_rect_x2-=8;
            //draw_screen_clip_rect_show_guys=true;
        }
        
        draw_screen(tmpscr);
        advanceframe(true);
        
        if(Quit) return;
    }
    
    clear_bitmap(msgdisplaybuf);
    draw_screen(tmpscr);
    advanceframe(true);
    
    draw_screen_clip_rect_x1=0;
    draw_screen_clip_rect_x2=255;
    //draw_screen_clip_rect_show_guys=false;
    
    clear_bitmap(scrollbuf);
    blit(framebuf,scrollbuf,0,0,0,0,256,224);
    endingpal();
    
        inc_quest();
        removeItemsOfFamily(game, itemsbuf, itype_ring);
        int maxring = getHighestLevelOfFamily(&zinit,itemsbuf,itype_ring);
        
        if(maxring != -1)
        {
            getitem(maxring,true);
        }
        
        ringcolor(false);
    
    
    
    stop_midi();
    
    if(zcmusic != NULL)
    {
        zcmusic_stop(zcmusic);
        zcmusic_unload_file(zcmusic);
        zcmusic = NULL;
    }
    
//  setPackfilePassword(datapwd);
    load_quest(game);
    strcpy(game->title,QHeader.title);
//  setPackfilePassword(NULL);
    saves[currgame] = *game;
    load_game_icon_to_buffer(false,currgame);
    load_game_icon(game,false,currgame);
    
    game->set_continue_dmap(zinit.start_dmap);
    game->set_continue_scrn(0xFF);
    game->set_cont_hearts(zinit.cont_heart);
    game->set_hasplayed(false);
    show_saving(scrollbuf);
    save_savedgames();
}

void inc_quest()
{
	char name[9];
	strcpy(name,game->get_name());
	// Go to quest 3 if you got some heart containers,
	// or quest 4 if you got them all.
	int quest = 0; int deaths = game->get_deaths();
	
	int linear = get_config_int("zeldadx","linear_quest_progression",1);
	
	if ( linear_quest_loading || game->get_quest() >= 5 )
	{
		int curquest = game->get_quest();
		if ( curquest == 255 ) //255 is a custom quest. We set it to 0 to roll over to 1, later. 
		{
			curquest = 0;
		}
		/*switch(curquest)
		{
			
			case 1: quest = 2; break;
			case 2: quest = 3; break;
			case 3: quest = 4; break;
			case 4: quest = 5; break;
			case 5: 
			{
				if ( file_exists("6th.qst") )
				{
					quest = 6;
				}
				else quest = 5;
				break;
			}
		}
		*/
		if ( curquest + 1 < 11 ) 
		{
			quest = curquest + 1;
		}
		game->Clear();
	}
	else
	{
    
		if(game->get_quest()==2 && game->get_maxlife()>=HP_PER_HEART*16)
			quest = 4;
		else
			quest = zc_min(game->get_quest()+1,(file_exists("6th.qst")) ? 6 : 5);
	
		if(game->get_quest()==3 && game->get_maxlife()>=HP_PER_HEART*16)
			quest = 4;

		
		// If you beat the 3rd quest without dying skip over the easier 4th and play the 5th quest.
		if(game->get_quest()==3 && deaths == 0)
			quest = 5;
	
		//I can;t find a better solution. If we reach five with wacky progression, just move on. -Z
		
		if(game->get_quest()>=5 && game->get_quest() < 10 /*&& deaths > 0*/)
		{
			//if ( file_exists("6th.qst") )
			//{
			//	quest = 6;
			//}
			//else quest = 4;
			quest = game->get_quest()+1;
		}
		// Likewise, if you beat the 5th but died, go back to the 4th or move on to 6thif(game->get_quest()==5 && deaths > 0)
		//{
		//	if ( file_exists("6th.qst") )
		//	{
		//		quest = 6;
		//	}
		//	else quest = 4;
		//}
	
		/*
		// If you beat the 3rd quest without dying skip over the eas if it exists.
		ier 4th and play the 5th quest.
		if(game->get_quest()==3 && deaths == 0)
			quest = 5;
	
		if ( game->get_quest()== 4 && deaths > 0 )
		{
			if ( file_exists("6th.qst") )
			{
				quest = 6;
			}
			else quest = 5;
		}
		


		// Likewise, if you beat the 5th but died, go back to the 4th.
		if(game->get_quest()==5 && deaths > 0)
		{
			if ( file_exists("6th.qst") )
			{
				quest = 6;
			}
			else quest = 4;
		}
		
		else if ( game->get_quest()==5 && deaths == 0 && file_exists("6th.qst") )
		{
			quest = 6;
		}
		
		*/
	

		game->Clear();
	}
	game->set_name(name);
	game->set_quest(quest);
	game->set_deaths(deaths);
	game->set_maxlife(3*HP_PER_HEART);
	game->set_life(3*HP_PER_HEART);
	game->set_maxbombs(8);
	game->set_hasplayed(false); //Unless we NEED it true, custom ended quests probably want this false to start their init scripts and such.
	
	//now bound to modules
	game->set_continue_dmap(moduledata.startingdmap[quest-1]);
	game->set_continue_scrn(moduledata.startingscreen[quest-1]);
	
	//game->set_continue_dmap(zinit.start_dmap);
	//game->set_continue_scrn(0x77);
	resetItems(game,&zinit,true);
	load_quest(game);
	load_game_icon_to_buffer(false,currgame);
	load_game_icon(game,false,currgame);
}

