//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include "zelda.h"
#include "subscr.h"
#include "zc_subscr.h"
#include "link.h"
#include "gamedata.h"
#include "guys.h"
#include <string.h>
extern LinkClass   Link;

//DIALOG *sso_properties_dlg;

void put_active_subscr(miscQdata *misc, int y, bool pulled_down)
{
  //  load_Sitems();
  Sitems.animate();
  bool showtime = game->get_timevalid() && !game->get_cheat() && get_bit(quest_rules,qr_TIME);
  show_custom_subscreen(framebuf, misc, current_subscreen_active, 0, 6-y, showtime, pulled_down);
}

void dosubscr_new(miscQdata *misc)
{
  PALETTE temppal;
  if (tmpscr->flags3&fNOSUBSCR)
  {
    return;
  }

  if (usebombpal)
  {
    memcpy(temppal, RAMpal, PAL_SIZE*sizeof(RGB));
    memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
    refreshpal=true;
  }

  int miny;
  bool showtime = game->get_timevalid() && !game->get_cheat() && get_bit(quest_rules,qr_TIME);
  load_Sitems(misc);

  pause_sfx(WAV_BRANG);
  adjust_sfx(WAV_ER,128,false);
  adjust_sfx(WAV_MSG,128,false);

/*
  blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
  blit(framebuf,scrollbuf,0,playing_field_offset,0,230,256,176);
  miny = 6;
  blit(scrollbuf,framebuf,0,230,0,playing_field_offset,256,176);
  for(int y=174; y>=6; y-=3)
  {
    domoney();
    Link.refill();
    put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
    blit(scrollbuf,framebuf,0,y,0,0,256,COOLSCROLL?230-y:224);
    put_active_subscr(misc,y,false);
    advanceframe();
    if(Quit)
      return;
  }
*/


  blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
  blit(framebuf,scrollbuf,0,playing_field_offset,0,230,256,176);
  miny = 6;
  blit(scrollbuf,framebuf,0,230,0,playing_field_offset,256,176);
  for(int y=174; y>=6; y-=3)
  {
    domoney();
    Link.refill();
    //put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
    //blit(scrollbuf,framebuf,0,y,0,0,256,COOLSCROLL?230-y:224);
    blit(scrollbuf,framebuf,0,230,0,playing_field_offset,256,176);
    put_active_subscr(misc,-44-y,false);
    advanceframe();
    if(Quit)
      return;
  }


  bool done=false;

//  put_passive_subscr(scrollbuf,misc,0,174,showtime,true);
  do
  {
    load_control_state();
    int pos = Bpos;
    
    if(rUp())         selectWpn(0, -1, true);
    else if(rDown())  selectWpn(0, 1, true);
    else if(rLeft())  selectWpn(-1, 0, true);
    else if(rRight()) selectWpn(1, 0, true);
    else if(rLbtn())  selectWpn(-1, 0, true);
    else if(rRbtn())  selectWpn(1, 0, true);

    if (get_bit(quest_rules,qr_SELECTAWPN))
    {
      Bwpn = Bweapon(Bpos);
      Awpn = current_item(itype_sword) >=4 ? iXSword : current_item(itype_sword) - 1 + iSword;
    }
    if(pos!=Bpos)
      sfx(WAV_CHIME);

    domoney();
    Link.refill();
//    put_passive_subscr(framebuf,misc,0,174-miny,showtime,true);
    put_passive_subscr(framebuf,misc,0,passive_subscreen_offset,showtime,true);

//    blit(scrollbuf,framebuf,0,6,0,6-miny,256,168);
//    put_active_subscr(misc,miny,true);
    blit(scrollbuf,framebuf,0,230,0,playing_field_offset,256,176);
    put_active_subscr(misc,-playing_field_offset+miny,true);

    advanceframe();
    if(NESquit && Up() && cAbtn() && cBbtn())
    {
      Udown=true;
      Quit=qQUIT;
    }
    if(Quit)
      return;
    if(rSbtn())
      done=true;
  } while(!done);

  for(int y=6; y<=174; y+=3)
  {
    domoney();
    Link.refill();
    //put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
    blit(scrollbuf,framebuf,0,230,0,playing_field_offset,256,176);
    put_active_subscr(misc,-44-y,false);
    advanceframe();
    if(Quit)
      return;
  }


  //  Sitems.clear();
  if (usebombpal)
  {
    memcpy(RAMpal, temppal, PAL_SIZE*sizeof(RGB));
  }
  resume_sfx(WAV_BRANG);
}

void dosubscr(miscQdata *misc)
{
  PALETTE temppal;
  if (tmpscr->flags3&fNOSUBSCR)
  {
    return;
  }

  if (usebombpal)
  {
    memcpy(temppal, RAMpal, PAL_SIZE*sizeof(RGB));
    memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
    refreshpal=true;
  }

  int miny;
  bool showtime = game->get_timevalid() && !game->get_cheat() && get_bit(quest_rules,qr_TIME);
  load_Sitems(misc);

  pause_sfx(WAV_BRANG);
  adjust_sfx(WAV_ER,128,false);
  adjust_sfx(WAV_MSG,128,false);

  //make a copy of the blank playing field on the right side of scrollbuf
  blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
  //make a copy of the complete playing field on the bottom of scrollbuf
  blit(framebuf,scrollbuf,0,playing_field_offset,0,176,256,176);
  miny = 6;
  for(int y=176-2; y>=6; y-=3)
  {
    domoney();
    Link.refill();
    //fill in the screen with black to prevent the hall of mirrors effect
    rectfill(framebuf, 0, 0, 255, 223, 0);
    if(COOLSCROLL)
    {
      //copy the playing field back onto the screen
      blit(scrollbuf,framebuf,0,176,0,passive_subscreen_height,256,176);
    }
    else
    {
      //scroll the playing field (copy the copy we made)
      blit(scrollbuf,framebuf,0,176,0,176-2-y+passive_subscreen_height,256,y);
    }
    //throw the passive subscreen onto the screen
    put_passive_subscr(framebuf,misc,0,176-2-y,showtime,false);
    //put the active subscreen above the passive subscreen
    put_active_subscr(misc,y,false);
    advanceframe();
    if(Quit)
      return;
  }

  bool done=false;

  do
  {
    load_control_state();
    int pos = Bpos;

    if(rUp())         selectWpn(0, -1, true);
    else if(rDown())  selectWpn(0, 1, true);
    else if(rLeft())  selectWpn(-1, 0, true);
    else if(rRight()) selectWpn(1, 0, true);
    else if(rLbtn())  selectWpn(-1, 0, true);
    else if(rRbtn())  selectWpn(1, 0, true);

    if (get_bit(quest_rules,qr_SELECTAWPN))
    {
      if(rBbtn())
      {
        if (Awpn == Bweapon(Bpos))
	  Awpn = Bwpn;
        Bwpn = Bweapon(Bpos);
        sfx(WAV_PLACE);
      }
      else if(rAbtn())
      {
        if (Bwpn == Bweapon(Bpos))
	  Bwpn = Awpn;
        Awpn = Bweapon(Bpos);
        sfx(WAV_PLACE);
      }
    }
    if(pos!=Bpos)
      sfx(WAV_CHIME);

    domoney();
    Link.refill();

    //put_passive_subscr(framebuf,misc,0,174-miny,showtime,true);
    //blit(scrollbuf,framebuf,0,6,0,6-miny,256,168);
    //put_active_subscr(misc,miny,true);

    //fill in the screen with black to prevent the hall of mirrors effect
    rectfill(framebuf, 0, 0, 255, 223, 0);
    if(COOLSCROLL)
    {
      //copy the playing field back onto the screen
      blit(scrollbuf,framebuf,0,176,0,passive_subscreen_height,256,176);
    }
    else
    {
      //nothing to do; the playing field has scrolled off the screen
    }
    //throw the passive subscreen onto the screen
    put_passive_subscr(framebuf,misc,0,176-2-miny,showtime,false);
    //put the active subscreen above the passive subscreen
    put_active_subscr(misc,miny,false);

    advanceframe();
    if(NESquit && Up() && cAbtn() && cBbtn())
    {
      Udown=true;
      Quit=qQUIT;
    }
    if(Quit)
      return;
    if(rSbtn())
      done=true;
  } while(!done);

  for(int y=6; y<=174; y+=3)
  {
    domoney();
    Link.refill();
    //fill in the screen with black to prevent the hall of mirrors effect
    rectfill(framebuf, 0, 0, 255, 223, 0);
    if(COOLSCROLL)
    {
      //copy the playing field back onto the screen
      blit(scrollbuf,framebuf,0,176,0,passive_subscreen_height,256,176);
    }
    else
    {
      //scroll the playing field (copy the copy we made)
      blit(scrollbuf,framebuf,0,176,0,176-2-y+passive_subscreen_height,256,y);
    }
    //throw the passive subscreen onto the screen
    put_passive_subscr(framebuf,misc,0,176-2-y,showtime,false);
    //put the active subscreen above the passive subscreen
    put_active_subscr(misc,y,false);
    advanceframe();
    if(Quit)
      return;
  }

  //  Sitems.clear();
  if (usebombpal)
  {
    memcpy(RAMpal, temppal, PAL_SIZE*sizeof(RGB));
  }
  resume_sfx(WAV_BRANG);
}

void markBmap(int dir)
{
  if((DMaps[get_currdmap()].type&dmfTYPE)==dmOVERW)
    return;
  if(get_currscr()>=128)
    return;

  byte drow = DMaps[get_currdmap()].grid[get_currscr()>>4];
  byte mask = 1 << (7-((get_currscr()&15)-DMaps[get_currdmap()].xoff));
  int di = ((get_currdmap()-1)<<6) + ((get_currscr()>>4)<<3) + ((get_currscr()&15)-DMaps[get_currdmap()].xoff);
  int code = 0;

  switch((DMaps[get_currdmap()].type&dmfTYPE))
  {
    case dmDNGN:
    // check dmap
    if((drow&mask)==0)
      return;
    // calculate code
    for(int i=3; i>=0; i--)
    {
      code <<= 1;
      code += tmpscr->door[i]&1;
    }
    // mark the map
    game->bmaps[di] = code|128;
    break;

    default:
    game->bmaps[di] |= 128;
    if(dir>=0)
      game->bmaps[di] |= (1<<dir);
    break;
  }
}



/*** end of subscr.cc ***/
