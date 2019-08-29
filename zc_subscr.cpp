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
  bool showtime = get_gamedata_timevalid() && !get_gamedata_cheat() && get_bit(quest_rules,qr_TIME);
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
  bool showtime = get_gamedata_timevalid(game) && !get_gamedata_cheat(game) && get_bit(quest_rules,qr_TIME);
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

    if(rUp())         selectBwpn(0, -1);
    else if(rDown())  selectBwpn(0, 1);
    else if(rLeft())  selectBwpn(-1, 0);
    else if(rRight()) selectBwpn(1, 0);
    else if(rLbtn())  selectBwpn(-1, 0);
    else if(rRbtn())  selectBwpn(1, 0);

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
/*
  if(COOLSCROLL)
  {
    for(int y=6; y<=174; y+=3)
    {
      domoney();
      Link.refill();
      put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
      blit(scrollbuf,framebuf,0,168+230+6-y,0,230-y,256,y-6);
      blit(scrollbuf,framebuf,0,y,0,0,256,230-y);
      put_active_subscr(misc,y,false);
      advanceframe();
      if(Quit)
        return;
    }
    blit(scrollbuf,scrollbuf,256,0,0,0,256,176);
  }
  else
  {
    for(int y=6; y<=174; y+=3)
    {
      domoney();
      Link.refill();
      put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
      blit(scrollbuf,framebuf,0,y,0,0,256,224);
      put_active_subscr(misc,y,false);
      advanceframe();
      if(Quit)
        return;
    }
    blit(scrollbuf,scrollbuf,0,230,0,0,256,176);
  }
*/


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
  bool showtime = get_gamedata_timevalid(game) && !get_gamedata_cheat(game) && get_bit(quest_rules,qr_TIME);
  load_Sitems(misc);

  pause_sfx(WAV_BRANG);
  adjust_sfx(WAV_ER,128,false);
  adjust_sfx(WAV_MSG,128,false);

  if(COOLSCROLL)
  {
    blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
    blit(framebuf,scrollbuf,0,56,0,230,256,176);
    miny = 6;
    blit(scrollbuf,framebuf,0,230,0,56,256,176);
    for(int y=174; y>=6; y-=3)
    {
      domoney();
      Link.refill();
      put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
      blit(scrollbuf,framebuf,0,y,0,0,256,230-y);
      put_active_subscr(misc,y,false);
      advanceframe();
      if(Quit)
        return;
    }
  }
  else
  {
    blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
    blit(framebuf,scrollbuf,0,56,0,230,256,176);
    miny = 6;
    blit(scrollbuf,framebuf,0,230,0,56,256,176);
    for(int y=174; y>=6; y-=3)
    {
      domoney();
      Link.refill();
      put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
      blit(scrollbuf,framebuf,0,y,0,0,256,224);
      put_active_subscr(misc,y,false);
      advanceframe();
      if(Quit)
        return;
    }
  }

  bool done=false;

  put_passive_subscr(scrollbuf,misc,0,174,showtime,true);
  do
  {
    load_control_state();
    int pos = Bpos;

    if(rUp())         selectBwpn(0, -1);
    else if(rDown())  selectBwpn(0, 1);
    else if(rLeft())  selectBwpn(-1, 0);
    else if(rRight()) selectBwpn(1, 0);
    else if(rLbtn())  selectBwpn(-1, 0);
    else if(rRbtn())  selectBwpn(1, 0);

    if(pos!=Bpos)
      sfx(WAV_CHIME);

    domoney();
    Link.refill();
    put_passive_subscr(framebuf,misc,0,174-miny,showtime,true);

    blit(scrollbuf,framebuf,0,6,0,6-miny,256,168);
    put_active_subscr(misc,miny,true);

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

  if(COOLSCROLL)
  {
    for(int y=6; y<=174; y+=3)
    {
      domoney();
      Link.refill();
      put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
      blit(scrollbuf,framebuf,0,168+230+6-y,0,230-y,256,y-6);
      blit(scrollbuf,framebuf,0,y,0,0,256,230-y);
      put_active_subscr(misc,y,false);
      advanceframe();
      if(Quit)
        return;
    }
    blit(scrollbuf,scrollbuf,256,0,0,0,256,176);
  }
  else
  {
    for(int y=6; y<=174; y+=3)
    {
      domoney();
      Link.refill();
      put_passive_subscr(scrollbuf,misc,0,174,showtime,false);
      blit(scrollbuf,framebuf,0,y,0,0,256,224);
      put_active_subscr(misc,y,false);
      advanceframe();
      if(Quit)
        return;
    }
    blit(scrollbuf,scrollbuf,0,230,0,0,256,176);
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
