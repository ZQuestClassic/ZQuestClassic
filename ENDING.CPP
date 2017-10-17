//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  ending.cc
//
//  Ending code for Zelda Classic.
//
//--------------------------------------------------------

#include "ending.h"
#include "zelda.h"
#include "zsys.h"
#include "sprite.h"
#include "items.h"
#include "pal.h"
#include "link.h"
#include "guys.h"
#include "title.h"
#include "subscr.h"
#include <string.h>
#include <stdio.h>

extern LinkClass   Link;
extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;
extern int draw_screen_clip_rect_x1;
extern int draw_screen_clip_rect_x2;
extern int draw_screen_clip_rect_y1;
extern int draw_screen_clip_rect_y2;
extern bool draw_screen_clip_rect_show_link;
extern bool draw_screen_clip_rect_show_guys;

void noproc() {}

void put_triforce()
{
  if (get_bit(quest_rules,qr_HOLDITEMANIMATION))
  {
    putitem2(framebuf,120,113,iTriforce,lens_hint_item[iTriforce][0],lens_hint_item[iTriforce][1], 0);
    putitem2(framebuf,136,113,iTriforce,lens_hint_item[iTriforce][0],lens_hint_item[iTriforce][1], 0);
  }
  else
  {
    putitem(framebuf,120,113,iTriforce);
    putitem(framebuf,136,113,iTriforce);
  }
}

void putendmsg(char *s,int x,int y,int speed,void(proc)())
{
  int i=0;
  int c=strlen(s)*speed;

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
    advanceframe();
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
  const int white = WHITE;
  const int red   = CSET(csBOSS)+4;
  const int blue  = CSET(csBOSS)+5;
  const int green = CSET(csBOSS)+6;

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

  game.cheat |= (cheat>1)?1:0;

  draw_screen_clip_rect_x1=0;
  draw_screen_clip_rect_x2=255;
  draw_screen_clip_rect_y1=0;
  draw_screen_clip_rect_y2=223;
  draw_screen_clip_rect_show_link=true;
  draw_screen_clip_rect_show_guys=false;

  for(int f=0; f<365; f++)
  {
    if(f==363)
    {
    //363  WIPE complete, DOT out, A/B items out
      QMisc.colors.link_dot = 255;
      show_subscreen_items = false;
      for(int i = guys.Count() - 1; i >= 0; i--)
      {
        if (guys.spr(i)->id > gDABEI)
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
      draw_screen_clip_rect_show_guys=true;
    }
    draw_screen(tmpscr, 0, 0);
    advanceframe();
    if(Quit) return;
  }

  draw_screen_clip_rect_x1=0;
  draw_screen_clip_rect_x2=255;
  draw_screen_clip_rect_show_guys=false;

  char tmpmsg[6][25];

  for (int x=0; x<3; x++)
  {
    sprintf(tmpmsg[x], "%.24s", MsgStrings[QMisc.endstring].s+(24*x));
    //    sprintf(tmpmsg[x], "%.24s", MsgStrings[QMisc.endstring].s+(24*x));
    //    strncpy(tmpmsg[x], MsgStrings[QMisc.endstring].s+(24*x), 24);
  }
  for (int x=0; x<3; x++)
  {
    sprintf(tmpmsg[x+3], "%.24s", MsgStrings[QMisc.endstring+1].s+(24*x));
    //    strncpy(tmpmsg[x+3], MsgStrings[QMisc.endstring+1].s+(24*x), 24);
  }

  if (QMisc.endstring==0)
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
    if (f==668)
    {
       rectfill(framebuf,120,129,152,145,0);
       blit(framebuf, tmp_bmp, 120,113, 0,0, 32,32);
    }
    if (f==860)
    {
       blit(tmp_bmp, framebuf, 0,0, 120,113, 32,32);
    }
    if (f==669 || f==861)
    {
      overtile16(framebuf,36,120,129,6,0);//draw Zelda two-handed overhead
      overtile16(framebuf,BSZ?32:29,136,129,6,0);//draw Link two-handed overhead
    }

    if(f==733)
    {
      blit(framebuf,scrollbuf,0,0,0,0,256,56);
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
      blit(scrollbuf,framebuf,0,0,0,0,256,56);
      jukebox(MUSIC_ENDING);
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
    advanceframe();
    if(Quit)
    {
      return;
    }
  }

  if (QMisc.endstring==0)
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
    advanceframe();
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
  if(game.quest>1)
  {
    len=720*2;
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
      switch(y)
      {
        case 240: textout_ex(scrollbuf,zfont," STAFF ",104,224,white,0); break;
        case 272: textout_ex(scrollbuf,zfont,"EXECUTIVE",40,224,blue,0); break;
        case 280: textout_ex(scrollbuf,zfont,"PRODUCER... H.YAMAUCHI",40,224,blue,0); break;
        case 320: textout_ex(scrollbuf,zfont,"PRODUCER.... S.MIYAHON",40,224,green,0); break;
        case 360: textout_ex(scrollbuf,zfont,"DIRECTOR.... S.MIYAHON",40,224,red,0); break;
        case 384: textout_ex(scrollbuf,zfont,"        ...... TEN TEN",40,224,red,0); break;
        case 424: textout_ex(scrollbuf,zfont,"DESIGNER...... TEN TEN",40,224,blue,0); break;
        case 464: textout_ex(scrollbuf,zfont,"PROGRAMMER.. T.NAKAZOO",40,224,green,0); break;
        case 488: textout_ex(scrollbuf,zfont,"          ..... YACHAN",40,224,green,0); break;
        case 512: textout_ex(scrollbuf,zfont,"          ... MARUMARU",40,224,green,0); break;
        case 552: textout_ex(scrollbuf,zfont,"SOUND",40,224,red,0); break;
        case 560: textout_ex(scrollbuf,zfont,"COMPOSER...... KONCHAN",40,224,red,0); break;
      }

      switch(game.quest)
      {
        case 1:
          switch(y)
          {
            case 656: textout_ex(scrollbuf,zfont,"ANOTHER QUEST WILL START",32,224,white,0); break;
            case 672: textout_ex(scrollbuf,zfont,"FROM HERE.",88,224,white,0); break;
            case 696: textout_ex(scrollbuf,zfont,"PRESS THE START BUTTON.",40,224,white,0); break;
            case 760: textprintf_ex(scrollbuf,zfont,72,224,white,0,"%c1986 NINTENDO",0xBB); break;
          }
          break;

        case 2:
          switch(y)
          {
            case 768: textout_ex(scrollbuf,zfont,"YOU ARE GREAT.",72,224,white,0); break;
            case 792: textprintf_ex(scrollbuf,zfont,72,224,red,0,"%-8s -%3d",game.name,game.deaths); break;
            case 816: textout_ex(scrollbuf,zfont,"YOU HAVE AN AMAZING",48,224,white,0); break;
            case 832: textout_ex(scrollbuf,zfont,"WISDOM AND POWER.",64,224,white,0); break;
            /*
                    case 880: textout_ex(scrollbuf,zfont,"END OF",104,224,white); break;
                    case 896: textout_ex(scrollbuf,zfont,"\"THE LEGEND OF ZELDA 1\"",32,224,white); break;
            */
            case 912: textprintf_ex(scrollbuf,zfont,72,224,blue,0,"%c1986 NINTENDO",0xBB); break;
          }
          break;

        case 3:
          switch(y)
          {
            case 768: textout_ex(scrollbuf,zfont,"YOU ARE GREAT.",72,224,white,0); break;
            case 792: textprintf_ex(scrollbuf,zfont,72,224,red,0,"%-8s -%3d",game.name,game.deaths); break;
            case 816: textout_ex(scrollbuf,zfont,"YOU HAVE AN AMAZING",48,224,white,0); break;
            case 832: textout_ex(scrollbuf,zfont,"WISDOM AND POWER.",64,224,white,0); break;
            case 880: textout_ex(scrollbuf,zfont,"END OF",104,224,white,0); break;
            case 896: textout_ex(scrollbuf,zfont,"\"THE LEGEND OF ZELDA 1\"",32,224,white,0); break;
            case 912: textprintf_ex(scrollbuf,zfont,72,224,blue,0,"%c1986 NINTENDO",0xBB); break;
          }
          break;

        default:
          switch(y)
          {
            case 768: textout_centre_ex(scrollbuf,zfont,"Congratulations!",128,224,white,0); break;
            case 784: textprintf_ex(scrollbuf,zfont,72,224,red,0,"%-8s -%3d",game.name,game.deaths); break;
            case 800: if(game.timevalid && !game.cheat)
            textout_centre_ex(scrollbuf,zfont,time_str_med(game.time),128,224,blue,0); break;
            case 816: textout_centre_ex(scrollbuf,zfont,"You beat a",128,224,white,0); break;
            case 832: textout_centre_ex(scrollbuf,zfont,"custom quest.",128,224,white,0); break;
            case 880: textout_centre_ex(scrollbuf,zfont,"ZELDA CLASSIC",128,224,white,0); break;
            case 896: textout_centre_ex(scrollbuf,zfont,"1999-2007",128,224,white,0); break;
            case 912: textout_centre_ex(scrollbuf,zfont,"Armageddon Games",128,224,blue,0); break;
          }
          break;
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
    advanceframe();
    if(Quit)
    {
      return;
    }
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

    advanceframe();
    if(Quit)
    {
      return;
    }
  } while(!rSbtn());

  if(game.quest>0 && game.quest<255)
  {
    inc_quest();
  }

  ringcolor();
  packfile_password(datapwd);
  load_quest(&game);
  packfile_password(NULL);
  load_game_icon(&game);
  game.continue_dmap=zinit.start_dmap;
  game.continue_scrn=0xFF;
  saves[currgame] = game;
  save_savedgames();
}

void inc_quest()
{
  char name[9];
  strcpy(name,game.name);
  int quest = min(game.quest+1,QUEST_COUNT);
  int deaths = game.deaths;

  int *di = (int*)(&game);
  for(unsigned i=0; i<sizeof(gamedata)/sizeof(int); i++)
  {
    *(di++) = 0;
  }

  strcpy(game.name,name);
  game.quest = quest;
  game.deaths = deaths;
  game.maxlife = game.life = 3*HP_PER_HEART;
  game.maxbombs = 8;
  game.hasplayed = true;
}
