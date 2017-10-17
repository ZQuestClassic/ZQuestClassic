//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include "subscr.h"
//#include "guys.h"
#include "zelda.h"
#include "tiles.h"
#include "zsys.h"
#include "guys.h"
#include "link.h"
#include <stdio.h>
#include <string.h>

bool show_subscreen_dmap_dots=true;
bool show_subscreen_numbers=true;
bool show_subscreen_items=true;
bool show_subscreen_life=true;

extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;
extern LinkClass   Link;

void blueframe(BITMAP *dest,int x,int y,int xsize,int ysize)
{
  /*
     /- -- -- -\ 
     |         |
     |         |
     \_ __ __ _/

      0  1  4  5
      2  3  6  7
     80 81 84 85
     82 83 86 87
  */

  int t = QMisc.colors.blueframe_tile;
  int c = QMisc.colors.blueframe_cset;

  int t8 = t<<2;

  for(int dx=0; dx<xsize; dx++)
  {
    int top    = (dx >= (xsize>>1)) ? ((dx==xsize-1) ? t8+5  : t8+4 ) : ((dx==0) ? t8    : t8+1 );
    int bottom = (dx >= (xsize>>1)) ? ((dx==xsize-1) ? t8+87 : t8+86) : ((dx==0) ? t8+82 : t8+83);

    overtile8(dest,top,(dx<<3)+x,y,c,0);
    overtile8(dest,bottom,(dx<<3)+x,((ysize-1)<<3)+y,c,0);
  }

  for(int dy=1; dy<ysize-1; dy++)
  {
    int left  = (dy >= (ysize>>1)) ? t8+80 : t8+2;
    int right = (dy >= (ysize>>1)) ? t8+85 : t8+7;

    overtile8(dest,left,x,(dy<<3)+y,c,0);
    overtile8(dest,right,((xsize-1)<<3)+x,(dy<<3)+y,c,0);

    for(int dx=1; dx<xsize-1; dx++)
    {
      int fill = (dy >= (ysize>>1)) ? ((dx >= (xsize>>1)) ? t8+84 : t8+81) : ((dx >= (xsize>>1)) ? t8+6 : t8+3);
      overtile8(dest,fill,(dx<<3)+x,(dy<<3)+y,c,0);
    }
  }
}

void drawgrid(BITMAP *dest,int c1,int c2)
{
  int si=0;
  for(int y=16; y<48; y+=4)
  {
    byte dl = DMaps[currdmap].grid[si];
    for(int x=16; x<76; x+=8)
    {
      if(c2==-1)
      {
        if(dl&0x80)
          rectfill(dest,x,y,x+6,y+2,c1);
      }
      else
      {
        rectfill(dest,x,y,x+6,y+2,c2);
        if(dl&0x80)
          rectfill(dest,x+2,y,x+4,y+2,c1);
      }
      dl<<=1;
    }
    ++si;
  }
}

void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h)
{
  for(int j=0; j<h; j++)
    for(int i=0; i<w; i++)
      overtile16(dest,tile+j*20+i,x+(i<<4),y+(j<<4),cset,0);
}

void drawdmap(BITMAP *dest)
{
  zcolors c=QMisc.colors;

  switch((DMaps[currdmap].type&dmfTYPE))
  {
    case dmOVERW:
    case dmBSOVERW:

      if (DMaps[currdmap].minimap_1_tile)
      {
        draw_block(dest,8,8,DMaps[currdmap].minimap_1_tile,DMaps[currdmap].minimap_1_cset,5,3);
      }
      else if (c.overworld_map_tile)
      {
        draw_block(dest,8,8,c.overworld_map_tile,c.overworld_map_cset,5,3);
      }
      else
      {
        rectfill(dest,16,16,79,47,c.overw_bg);
      }

      if (!DMaps[currdmap].minimap_1_tile &&
        ((DMaps[currdmap].type&dmfTYPE) == dmBSOVERW))
      {
        drawgrid(dest,c.bs_goal,c.bs_dk);
      }
      break;

    case dmDNGN:
    case dmCAVE:

      int maptile=(game.lvlitems[dlevel]&liMAP)?
        DMaps[currdmap].minimap_2_tile:
      DMaps[currdmap].minimap_1_tile;
      int mapcset=(game.lvlitems[dlevel]&liMAP)?
        DMaps[currdmap].minimap_2_cset:
      DMaps[currdmap].minimap_1_cset;

      if (maptile)
      {
        draw_block(dest,8,8,maptile,mapcset,5,3);
      }
      else if (c.dungeon_map_tile)
      {
        draw_block(dest,8,8,c.dungeon_map_tile,c.dungeon_map_cset,5,3);
      }
      else
      {
        rectfill(dest,16,16,79,47,c.dngn_bg);
      }

      if (!DMaps[currdmap].minimap_2_tile &&
        game.lvlitems[dlevel]&liMAP)
      {
        if ((DMaps[currdmap].type&dmfTYPE) == dmCAVE)
        {
          drawgrid(dest,c.cave_fg,-1);
        }
        else
        {
          drawgrid(dest,c.dngn_fg,-1);
        }
      }
      break;
  }
}

void lifemeter(BITMAP *dest,int x,int y)
{
  if (!show_subscreen_life)
  {
    return;
  }
  for(int i=0; i<game.maxlife; i+=HP_PER_HEART)
  {
    int tile=0;
    if(i+(HP_PER_HEART/2)>=game.life) tile=1;
    if(i>=game.life) tile=4;
    overtile8(dest,tile,x,y,1,0);
    x+=8;
    if(((i>>4)&7)==7)
      {x-=64; y-=8;}
  }
}

void magicmeter(BITMAP *dest,int x,int y)
{
  if (!get_bit(quest_rules,qr_ENABLEMAGIC)) return;
  if (game.maxmagic==0) return;
  int tile;
  int mmtile=wpnsbuf[iwMMeter].tile;
  int mmcset=wpnsbuf[iwMMeter].csets&15;
  overtile8(dest,(mmtile*4)+2,x-8,y,mmcset,0);
  if (game.magicdrainrate==1)
  {
    overtile8(dest,(mmtile*4)+1,x-10,y,mmcset,0);
  }
  for(int i=0; i<game.maxmagic; i+=MAGICPERBLOCK)
  {
    if (game.magic>=i+MAGICPERBLOCK)
    {
      tile=mmtile*4;                                        //full block
    }
    else
    {
      if (i>game.magic)
      {
        tile=((mmtile+1)*4);                                //empty block
      }
      else
      {
        tile=((mmtile+1)*4)+((game.magic-i)%MAGICPERBLOCK);
      }
    }
    overtile8(dest,tile,x,y,mmcset,0);
    x+=8;
  }
  overtile8(dest,(mmtile*4)+3,x,y,mmcset,0);

}

void putxnum(BITMAP *dest,int x,int y,int num)
{
  if (!show_subscreen_numbers)
  {
    return;
  }
  if(num>=100)
  {
    textprintf_ex(dest,zfont,x,y,QMisc.colors.text,-1,"%d",num);
  }
  else
  {
    textprintf_ex(dest,zfont,x,y,QMisc.colors.text,-1,"X%d",num);
  }
}

/*
inline void putdot(BITMAP *dest,int x,int y,int c)
{
  rectfill(dest,x,y,x+2,y+2,c);
}
*/

/****  Subscr items code  ****/

item *Bitem = NULL, *Aitem = NULL;
int   Bid = 0, Aid = 0;

void reset_subscr_items()
{
  if(Aitem)
  {
    delete Aitem;
    Aitem = NULL;
  }
  if(Bitem)
  {
    delete Bitem;
    Bitem = NULL;
  }
  Aid = Bid = 0;
}

void update_subscr_items()
{
  if(Bid != Bwpn)
  {
    Bid = 0;
    if(Bitem)
    {
      delete Bitem;
      Bitem = NULL;
    }

    if(Bwpn > 0)
    {
      Bitem = new item((fix)124, (fix)24, Bwpn, 0, 0);
      if(Bitem != NULL)
      {
        Bid = Bwpn;
        Bitem->yofs = 0;
      }
    }
  }

  if(Aid != Awpn)
  {
    Aid = 0;
    if(Aitem)
    {
      delete Aitem;
      Aitem = NULL;
    }

    if(Awpn > 0)
    {
      Aitem = new item((fix)148, (fix)24, Awpn, 0, 0);
      if(Aitem != NULL)
      {
        Aid = Awpn;
        Aitem->yofs = 0;
      }
    }
  }

  if(Bitem)
    Bitem->animate(0);
  if(Aitem)
    Aitem->animate(0);
}

/****/
int stripspaces(char *source, char *target, int stop)
{
  int begin, end;
  for (begin=0; ((begin<stop)&&(source[begin]==' ')); begin++);
  if (begin==stop)
  {
    return 0;
  }
  for (end=stop-1; ((source[end]==' ')); end--);
  sprintf(target, "%.*s", (end-begin+1), source+begin);
  return (end-begin+1);
}

void putsubscr(BITMAP *dest,int x,int y,bool showtime)
{
  int type = (DMaps[currdmap].type&dmfTYPE);
  BITMAP *subscr = create_sub_bitmap(dest,x,y,256,56);
  if (tmpscr->flags3&fNOSUBSCR)
  {
    clear_to_color(subscr,0);
    return;
  }
  clear_to_color(subscr,QMisc.colors.subscr_bg);
  drawdmap(subscr);

  if(type==dmDNGN || type==dmCAVE)
  {
    if(show_subscreen_dmap_dots&&game.lvlitems[dlevel]&liCOMPASS)
    {
      int c = QMisc.colors.compass_dk;
      if(((game.lvlitems[dlevel]&liTRIFORCE)==0) && (frame&16))
        c = QMisc.colors.compass_lt;
      int cx = ((DMaps[currdmap].compass&15)<<3)+18;
      int cy = ((DMaps[currdmap].compass&0xF0)>>2)+16;
      putdot(subscr,cx,cy,c);
    }
  }

  char dmaptitlesource[2][11];
  char dmaptitle[2][11];
  sprintf(dmaptitlesource[0], "%.10s", DMaps[currdmap].title);
  sprintf(dmaptitlesource[1], "%.10s", DMaps[currdmap].title+10);

  int title_len1=stripspaces(dmaptitlesource[0], dmaptitle[0], 10);
  int title_len2=stripspaces(dmaptitlesource[1], dmaptitle[1], 10);
  //  text_mode(0);

  //  textprintf(subscr,zfont, COOLSCROLL?20:16, COOLSCROLL?6:8,QMisc.colors.text,"LEVEL-%d",dlevel);

  if ((title_len1>0)||(title_len2>0))
  {
    if ((title_len1>0)&&(title_len2>0))
    {
      textprintf_centre_ex(subscr,zfont, 48, 0,QMisc.colors.text,0,dmaptitle[0]);
      textprintf_centre_ex(subscr,zfont, 48, 8,QMisc.colors.text,0,dmaptitle[1]);
    }
    else
    {
      if (title_len1>0)
      {
        textprintf_centre_ex(subscr,zfont, 48, 4,QMisc.colors.text,0,dmaptitle[0]);
      }
      else
      {
        textprintf_centre_ex(subscr,zfont, 48, 4,QMisc.colors.text,0,dmaptitle[1]);
      }
    }
  }

  /*
    textprintf_centre(subscr,zfont, 48, 0,QMisc.colors.text,dmaptitlesource[0]);
    textprintf_centre(subscr,zfont, 48, 8,QMisc.colors.text,dmaptitlesource[1]);
  */
  //  text_mode(-1);
  if(show_subscreen_dmap_dots && QMisc.colors.link_dot != 255)
  {
    if(type==dmOVERW)
    {
      putdot(subscr,((homescr&15)<<2)+17,((homescr&0xF0)>>2)+16,QMisc.colors.link_dot);
    }
    else if(type==dmBSOVERW || ((type==dmDNGN || type==dmCAVE) && currscr<128))
    {
      putdot(subscr,(((homescr&15)-DMaps[currdmap].xoff)<<3)+18,((homescr&0xF0)>>2)+16,QMisc.colors.link_dot);
    }
  }

  int yofs = can_use_item(itype_sbomb,1) ? 8 : 0;

  overtile8(subscr,5,88,16,1,0);
  overtile8(subscr,8,88,32-yofs,1,0);
  overtile8(subscr,9,88,40-yofs,0,0);

  putxnum(subscr,96,16,game.rupies);
  if(current_item(itype_magickey,true)>=i_magickey)
  {
    if (show_subscreen_numbers)
    {
      textout_ex(subscr,zfont,"XA",96,32-yofs,QMisc.colors.text,-1);
    }
  }
  else
  {
    putxnum(subscr,96,32-yofs,game.keys);
  }
  putxnum(subscr,96,40-yofs,current_item(itype_bomb, true));

  if(can_use_item(itype_sbomb,1))
  {
    overtile8(subscr,13,88,40,1,0);
    putxnum(subscr,96,40,current_item(itype_sbomb, true));
  }

  if(showtime)
  {
    char *ts = time_str_med(game.time);
    textout_ex(subscr,zfont,ts,240-(strlen(ts)<<3),0,QMisc.colors.text,-1);
  }

  if (!get_bit(quest_rules,qr_ENABLEMAGIC))
  {
    textout_ex(subscr,zfont,"-LIFE-",184,16,QMisc.colors.caption,-1);
  }
  else
  {
    textout_ex(subscr,zfont,"-LIFE-",184,12,QMisc.colors.caption,-1);
  }
  if (!get_bit(quest_rules,qr_ENABLEMAGIC))
  {
    lifemeter(subscr,176,40);
  }
  else
  {
    lifemeter(subscr,176,36);
  }
  magicmeter(subscr,176,44);
  blueframe(subscr,120,16,3,4);
  blueframe(subscr,144,16,3,4);

  update_subscr_items();
  if(Bitem&&show_subscreen_items)
    Bitem->draw(subscr);
  if(Aitem&&show_subscreen_items)
    Aitem->draw(subscr);

  //  text_mode(QMisc.colors.subscr_bg);
  textout_ex(subscr,zfont,"B",128,16,QMisc.colors.text,QMisc.colors.subscr_bg);
  textout_ex(subscr,zfont,"A",152,16,QMisc.colors.text,QMisc.colors.subscr_bg);
  destroy_bitmap(subscr);
}

/*
const byte tripiece[8*3] =
{
 112,112,0, 128,112,1, 96,128,0, 144,128,1,
 112,128,2, 112,128,1, 128,128,3, 128,128,0
};
*/

void puttriframe(BITMAP *dest)
{
  if(QMisc.colors.triframe_tile)
  {
    draw_block(dest,80,104,QMisc.colors.triframe_tile,QMisc.colors.triframe_cset,6,3);
  }
  else
  {
    int cs = QMisc.colors.triframe_color;
    line(dest,127,104,81,150,cs);
    line(dest,128,104,174,150,cs);
    hline(dest,80,151,175,cs);
    line(dest,127,111,95,143,cs);
    line(dest,128,111,160,143,cs);
    hline(dest,95,144,160,cs);
  }

  for(int i=0; i<8; i++)
  {
    int lvl = i+1;
    if(get_bit(quest_rules,qr_4TRI) && lvl>4)
      lvl -= 4;

    if(game.lvlitems[lvl]&liTRIFORCE)
    {
      int tp=0;
      for( ; tp<8; tp++)
        if(QMisc.triforce[tp] == i+1)
          break;

      overtile16(dest,QMisc.colors.triforce_tile,tripiece[tp*3],tripiece[tp*3+1],QMisc.colors.triforce_cset,tripiece[tp*3+2]);
    }
  }
}

void markBmap(int dir)
{
  if((DMaps[currdmap].type&dmfTYPE)==dmOVERW)
    return;
  if(currscr>=128)
    return;

  byte drow = DMaps[currdmap].grid[currscr>>4];
  byte mask = 1 << (7-((currscr&15)-DMaps[currdmap].xoff));
  int di = ((currdmap-1)<<6) + ((currscr>>4)<<3) + ((currscr&15)-DMaps[currdmap].xoff);
  int code = 0;

  switch((DMaps[currdmap].type&dmfTYPE))
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
      game.bmaps[di] = code|128;
      break;

    default:
      game.bmaps[di] |= 128;
      if(dir>=0)
        game.bmaps[di] |= (1<<dir);
      break;
  }
}

/*
const byte bmap[16*2] = {
0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,
0,3,0,0,0,3,0,0,0,0,3,0,0,0,3,0 };

const int fringe[8] = { 6,2,4,7,6,8,7,5 };

const byte bmaptiles[8*5] = {
 0, 1, 2, 3, 2, 3, 3, 4,
20,21,22,23,22,23,23,24,
20,21,22,23,22,23,23,24,
20,21,22,23,22,23,23,24,
40,41,42,43,42,43,43,44 };
*/

void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h);
void putBmap(BITMAP *dest)
{
  int si=0;

  int maptile=(game.lvlitems[dlevel]&liMAP)?
    DMaps[currdmap].largemap_2_tile:
  DMaps[currdmap].largemap_1_tile;
  int mapcset=(game.lvlitems[dlevel]&liMAP)?
    DMaps[currdmap].largemap_2_cset:
  DMaps[currdmap].largemap_1_cset;

  if (maptile)
  {
    draw_block(dest,96,88,maptile,mapcset,9,5);
  }
  else if(QMisc.colors.dungeon_map_tile)
  {
    for(int y=0; y<5; y++)
      for(int x=0; x<8; x++)
    {
      overtile16(dest,QMisc.colors.dungeon_map_tile+bmaptiles[si],
        96+(x<<4),88+(y<<4),QMisc.colors.dungeon_map_cset,0);
      ++si;
    }
  }
  else
  {
    BITMAP *bmp = create_bitmap_ex(8,8,8);
    if(!bmp)
      return;

    clear_bitmap(bmp);
    for(int x=0; x<8; x++)
      for(int y=8-fringe[x]; y<8; y++)
        putpixel(bmp,x,y,QMisc.colors.bmap_bg);

    rectfill(dest,96,88,223,167,QMisc.colors.bmap_bg);
    for(int y=88; y<=88+72; y+=72)
      for(int x=96; x<96+128; x+=8)
    {
      if(bmap[si])
      {
        rectfill(dest,x,y,x+7,y+7,QMisc.colors.subscr_bg);
        switch(bmap[si])
        {
          case 3:  draw_sprite_v_flip(dest,bmp,x,y); break;
          case 1:
          default: draw_sprite(dest,bmp,x,y); break;
        }
      }
      ++si;
    }
    destroy_bitmap(bmp);
  }

  int c = QMisc.colors.bmap_fg;

  si=(currdmap-1)<<6;
  for(int y=96; y<160; y+=8)
    for(int x=128+(maptile?8:0); x<192+(maptile?8:0); x+=8)
  {
    if(game.bmaps[si])
    {
      rectfill(dest,x+1,y+1,x+6,y+6,c);
      if(game.bmaps[si]&1) hline(dest,x+3,y,x+4,c);
      if(game.bmaps[si]&2) hline(dest,x+3,y+7,x+4,c);
      if(game.bmaps[si]&4) vline(dest,x,y+3,y+4,c);
      if(game.bmaps[si]&8) vline(dest,x+7,y+3,y+4,c);
    }
    ++si;
  }
  if(currscr<128)
    putdot(dest,(((homescr&15)-DMaps[currdmap].xoff)<<3)+130+(maptile?8:0),((homescr&0xF0)>>1)+99,QMisc.colors.link_dot);
}

void load_Sitems()
{
  int ofs=0;                                                // = NEWSUBSCR ? 108 : 0;
  int y=0;                                                  //   = NEWSUBSCR ? 48 : 24;
  int x[6];                                                 // = {128,148,160,176,192,204};

  switch(zinit.subscreen)
  {
    case 0:                                                 //original
      ofs=0;
      y=24;
      x[0]=128; x[1]=148; x[2]=160;
      x[3]=176; x[4]=192; x[5]=204;
      if(BSZ)
        x[1]=146;
      break;
    case 1:                                                 //revision 1 ("New Subscreen")
      ofs=108;
      y=48;
      for(int i=0; i<6; i++)
      {
        x[i] = (i<<4)+128;
      }
      break;
    case 2:                                                 //revision 2 (1.92 beta 168)
      ofs=108;
      y=48;
      for(int i=0; i<6; i++)
      {
        x[i] = (i<<4)+122;
      }
      break;
  }
  /*
   switch(zinit.subscreen) {
     case 0: //original
       break;
     case 1: //revision 1 ("New Subscreen")
       break;
     case 2: //revision 2 (1.92 beta 168)
       break;
   }
  */

  Sitems.clear();

  if(current_item(itype_raft, true)>=i_raft)     Sitems.add(new item((fix)(x[0]-ofs),(fix)y,iRaft,0,0));
  if(current_item(itype_book, true)>=i_book)     Sitems.add(new item((fix)(x[1]-ofs),(fix)y,iBook,0,0));
  if(current_item(itype_ring,true))
  {
    if (current_item(itype_ring,true)>=3) Sitems.add(new item((fix)(x[2]-ofs),(fix)y,iGRing,0,0));
    else Sitems.add(new item((fix)(x[2]-ofs),(fix)y,current_item(itype_ring,true)-1+iBRing,0,0));
  }
  if(current_item(itype_ladder, true)>=i_ladder)   Sitems.add(new item((fix)(x[3]-ofs),(fix)y,iLadder,0,0));
  if(current_item(itype_magickey, true)>=i_magickey)     Sitems.add(new item((fix)(x[4]-ofs),(fix)y,iMKey,0,0));
  if(current_item(itype_bracelet,true)>=i_bracelet2)    Sitems.add(new item((fix)(x[5]-ofs),(fix)y,iL2Bracelet,0,0));
  else if(current_item(itype_bracelet,true)>=i_bracelet1)  Sitems.add(new item((fix)(x[5]-ofs),(fix)y,iBracelet,0,0));

  if (zinit.subscreen>0)
    //  if(NEWSUBSCR)
  {
    y = 64;

    if(current_item(itype_shield,true)>=i_mirrorshield)      Sitems.add(new item((fix)(x[0]-ofs),(fix)y,iMShield,0,0));
    else if(current_item(itype_shield,true)>=i_largeshield)  Sitems.add(new item((fix)(x[0]-ofs),(fix)y,iShield,0,0));
    if(current_item(itype_wallet,true)) Sitems.add(new item((fix)(x[1]-ofs),(fix)y,iWallet500-1+current_item(itype_wallet,true),0,0));
    //    if(game.misc2&iCROSS)       Sitems.add(new item(x[2]-ofs,y,iCross,0,0));
    if(current_item(itype_amulet,true)>=i_amulet2)
    {
      Sitems.add(new item((fix)(x[2]-ofs),(fix)y,iL2Amulet,0,0));
    }
    else if (current_item(itype_amulet,true)>=i_amulet1)
    {
      Sitems.add(new item((fix)(x[2]-ofs),(fix)y,iAmulet,0,0));
    }
    if(current_item(itype_flippers,true)>=i_flippers)    Sitems.add(new item((fix)(x[3]-ofs),(fix)y,iFlippers,0,0));
    if(current_item(itype_boots,true)>=i_boots)       Sitems.add(new item((fix)(x[4]-ofs),(fix)y,iBoots,0,0));

    // HC Pieces
    if(QMisc.colors.HCpieces_tile)
    {
      item *HCP = new item((fix)(x[5]-ofs),(fix)y,iMax,0,0);
      if(HCP)
      {
        HCP->tile = QMisc.colors.HCpieces_tile + vbound(game.HCpieces,0,3);
        HCP->cs   = QMisc.colors.HCpieces_cset;
        Sitems.add(HCP);
      }
    }
  }

  
  //first row
  switch(zinit.subscreen)
  {
    case 0:                                                 //original
      ofs=0;
      y=48;
      break;
    case 1:                                                 //revision 1 ("New Subscreen")
      ofs=16;
      y=32;
      break;
    case 2:                                                 //revision 2 (1.92 beta 168)
      ofs=-2;
//      y=16;
      y=32;
      break;
  }
  //  ofs = NEWSUBSCR ? 16 : 0;
  //  y = NEWSUBSCR ? 32 : 48;

  if(current_item(itype_brang,true))
  {
    if (current_item(itype_brang,true)<i_fbrang)
    {
      Sitems.add(new item((fix)(128+ofs),(fix)y,current_item(itype_brang,true)-1+iBrang,0,0));
    }
    else
    {
      Sitems.add(new item((fix)(128+ofs),(fix)y,iFBrang,0,0));
    }
  }
  if(current_item(itype_bomb,true))          Sitems.add(new item((fix)(152+ofs),(fix)y,iBombs,0,0));
  if(current_item(itype_arrow,true))
  {
    if (current_item(itype_arrow,true)<i_garrow)
    {
      Sitems.add(new item((fix)(172+ofs),(fix)y,current_item(itype_arrow,true)-1+iArrow,0,0));
    }
    else
    {
      Sitems.add(new item((fix)(172+ofs),(fix)y,iGArrow,0,0));
    }
  }
  if(current_item(itype_bow,true))
  {
    if(current_item(itype_bow,true)==2)
    {
      Sitems.add(new item((fix)(180+ofs),(fix)y,iBow2,0,0));
    }
    else
    {
      Sitems.add(new item((fix)(180+ofs),(fix)y,iBow,0,0));
    }
  }
  if(current_item(itype_candle,true))         Sitems.add(new item((fix)(200+ofs),(fix)y,current_item(itype_candle,true)-1+iBCandle,0,0));

  switch(zinit.subscreen)
  {
    case 0:                                                 //original
      y=64;
      break;
    case 1:                                                 //revision 1 ("New Subscreen")
      y=48;
      break;
    case 2:                                                 //revision 2 (1.92 beta 168)
//      y=32;
      y=48;
      break;
  }
  //  y = NEWSUBSCR ? 48 : 64;

  if(current_item(itype_whistle,true))        Sitems.add(new item((fix)(128+ofs),(fix)y,iWhistle,0,0));
  if(current_item(itype_bait,true))           Sitems.add(new item((fix)(152+ofs),(fix)y,iBait,0,0));
  if(current_item(itype_potion,true))         Sitems.add(new item((fix)(176+ofs),(fix)y,current_item(itype_potion,true)-1+iBPotion,0,0));
  else if(current_item(itype_letter,true))    Sitems.add(new item((fix)(176+ofs),(fix)y,iLetter,0,0));
  if(current_item(itype_wand,true))           Sitems.add(new item((fix)(200+ofs),(fix)y,iWand,0,0));

  //  if(NEWSUBSCR)
  switch(zinit.subscreen)
  {
    case 0:                                                 //original
      //nothing.  already drew all the items
      break;
    case 1:                                                 //revision 1 ("New Subscreen")
    case 2:                                                 //revision 2 (1.92 beta 168)
      y = 64;
      if (zinit.subscreen==2)
      {
//        y=48;
      }
      if(current_item(itype_hookshot,true))   Sitems.add(new item((fix)(128+ofs),(fix)y,iHookshot,0,0));
      if(current_item(itype_sbomb,true))            Sitems.add(new item((fix)(152+ofs),(fix)y,iSBomb,0,0));
      if(current_item(itype_lens,true))       Sitems.add(new item((fix)(176+ofs),(fix)y,iLens,0,0));
      if(current_item(itype_hammer,true))     Sitems.add(new item((fix)(200+ofs),(fix)y,iHammer,0,0));
      if (zinit.subscreen==2)
      {
        if(current_item(itype_dinsfire,true))        Sitems.add(new item((fix)(224+ofs),(fix)(y-32),iDinsFire,0,0));
        if(current_item(itype_faroreswind,true))     Sitems.add(new item((fix)(224+ofs),(fix)(y-16),iFaroresWind,0,0));
        if(current_item(itype_nayruslove,true))      Sitems.add(new item((fix)(224+ofs),(fix)y,iNayrusLove,0,0));
      }
      break;
  }

  int type = (DMaps[currdmap].type&dmfTYPE);

  switch (zinit.subscreen)
  {
    case 0:                                                 //original
    case 1:                                                 //new subscreen
      if(type==dmDNGN || type==dmCAVE)
      {
        if(game.lvlitems[dlevel]&liMAP)     Sitems.add(new item((fix)44,(fix)112,iMap,0,0));
        if(game.lvlitems[dlevel]&liCOMPASS) Sitems.add(new item((fix)44,(fix)152,iCompass,0,0));
      }
      break;
    case 2:                                                 //revision 2
      if(type==dmDNGN || type==dmCAVE)
      {
        if(game.lvlitems[dlevel]&liMAP)     Sitems.add(new item((fix)24,(fix)112,iMap,0,0));
        if(game.lvlitems[dlevel]&liCOMPASS) Sitems.add(new item((fix)44,(fix)112,iCompass,0,0));
        if(game.lvlitems[dlevel]&liBOSSKEY) Sitems.add(new item((fix)64,(fix)112,iBossKey,0,0));
      }
      break;
  }
}

void put_topsubscr()
{
  BITMAP *subscr = scrollbuf;
  int type = (DMaps[currdmap].type&dmfTYPE);

  rectfill(subscr,0,0,255,173,QMisc.colors.subscr_bg);

  //  text_mode(-1);

  //  if(NEWSUBSCR)
  switch(zinit.subscreen)
  {
    case 0:                                                 //original
      blueframe(subscr,56,40,4,4);
      textout_ex(subscr,zfont,"INVENTORY",32,24,QMisc.colors.caption,-1);
      textout_ex(subscr,zfont,"USE B BUTTON",16,72,QMisc.colors.text,-1);
      textout_ex(subscr,zfont,"FOR THIS",32,80,QMisc.colors.text,-1);
      break;
    case 1:                                                 //revision 1 ("New Subscreen")
      textout_ex(subscr,zfont,"INVENTORY",32,24,QMisc.colors.caption,-1);
      blueframe(subscr,12,40,14,6);
      break;
    case 2:                                                 //revision 2 (1.92 beta 168)
      textout_ex(subscr,zfont,"INVENTORY",26,24,QMisc.colors.caption,-1);
      blueframe(subscr,6,40,14,6);
      break;
  }

  if(type == dmOVERW || type == dmBSOVERW)
  {
    textout_ex(subscr,zfont,"TRIFORCE",96,160,QMisc.colors.caption,-1);
    puttriframe(subscr);
  }
  else
  {
    if (zinit.subscreen<2)
    {
      textout_ex(subscr,zfont,"MAP",40,96,QMisc.colors.caption,-1);
      textout_ex(subscr,zfont,"COMPASS",24,136,QMisc.colors.caption,-1);
    }
    putBmap(subscr);
  }

  switch(zinit.subscreen)
  {
    case 0:                                                 //original
      blueframe(subscr, 120, 40, 13, 6);
      break;
    case 1:                                                 //revision 1 ("New Subscreen")
      blueframe(subscr, 136, 24, 13, 8);
      break;
    case 2:                                                 //revision 2 (1.92 beta 168)
      if(type==dmDNGN || type==dmCAVE)
      {
        blueframe(subscr, 16, 104, 9, 4);
      }
//      blueframe(subscr, 118, 8, 16, 10);
      blueframe(subscr, 118, 24, 16, 8);
      break;
  }

  //  blueframe(subscr, NEWSUBSCR ? 136:120, NEWSUBSCR ? 24:40, 13, NEWSUBSCR ? 8:6);

  //  textprintf(subscr, zfont, 96, 168, QMisc.colors.text, "%x", game.misc2);
}

void update_topsubscr(int y)
{
  //  if(!NEWSUBSCR && (Bitem))
  if((zinit.subscreen==0) && (Bitem))
  {
    Bitem->x = 64;
    Bitem->y = 48-y;
    Bitem->draw(framebuf);
    Bitem->x = 124;
    Bitem->y = 24;
  }

  for(int i=0; i<Sitems.Count(); i++)
    Sitems.spr(i)->yofs = -y;
  Sitems.animate();
  Sitems.draw(framebuf,false);
}

void dosubscr()
{
  if (tmpscr->flags3&fNOSUBSCR)
  {
    return;
  }
  int miny;
  bool showtime = game.timevalid && !game.cheat && get_bit(quest_rules,qr_TIME);

  load_Sitems();

  pause_sfx(WAV_BRANG);
  adjust_sfx(WAV_ER,128,false);
  adjust_sfx(WAV_MSG,128,false);

  if(COOLSCROLL)
  {
    blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
    blit(framebuf,scrollbuf,0,56,0,230,256,176);
    put_topsubscr();
    miny = 6;
    blit(scrollbuf,framebuf,0,230,0,56,256,176);
    for(int y=174; y>=6; y-=3)
    {
      domoney();
      Link.refill();
      putsubscr(scrollbuf,0,174,showtime);
      blit(scrollbuf,framebuf,0,y,0,0,256,230-y);
      update_topsubscr(y);
      advanceframe();
      if(Quit)
        return;
    }
  }
  else
  {
    blit(scrollbuf,scrollbuf,0,0,256,0,256,176);
    blit(framebuf,scrollbuf,0,56,0,230,256,176);
    put_topsubscr();
    miny = 0;
    blit(scrollbuf,framebuf,0,230,0,56,256,176);
    /* old code
        blit(scrollbuf,scrollbuf,0,0,0,230,256,176);
        put_topsubscr();
        miny = 0;
    */
    for(int y=174; y>=0; y-=3)
    {
      domoney();
      Link.refill();
      putsubscr(scrollbuf,0,174,showtime);
      blit(scrollbuf,framebuf,0,y,0,0,256,224);
      update_topsubscr(y);
      advanceframe();
      if(Quit)
        return;
    }
  }

  bool done=false;
  item sel((fix)0,(fix)0,iSelect,0,0);
  sel.yofs = 0;

  do
  {
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
    putsubscr(framebuf,0,174-miny,showtime);
    //    blit(scrollbuf,framebuf,0,32,0,32-miny,256,64);
//    blit(scrollbuf,framebuf,0,6,0,6-miny,256,100);
    blit(scrollbuf,framebuf,0,6,0,6-miny,256,168);
    //    blit(scrollbuf,framebuf,0,6,0,6-miny,256,64);
    update_topsubscr(miny);

    //    int sx = ;
    //    int sy = ;

    sel.animate(0);
    switch(zinit.subscreen)
    {
      case 0:                                               //original
        sel.x = (Bpos%4)*24 + 128;
        sel.y = (Bpos/4)*16 + 48-miny;
        break;
      case 1:                                               //revision 1 ("New Subscreen")
        sel.x = (Bpos%4)*24 + 144;
        sel.y = (Bpos/4)*16 + 32-miny;
        break;
      case 2:                                               //revision 2 (1.92 beta 168)
        sel.x = (Bpos%5)*24 + 126;
//        sel.y = (Bpos/5)*16 + 16-miny;
        sel.y = (Bpos/5)*16 + 32-miny;
        break;
    }
    sel.draw(framebuf);

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
      putsubscr(scrollbuf,0,174,showtime);
      blit(scrollbuf,framebuf,0,168+230+6-y,0,230-y,256,y-6);
      blit(scrollbuf,framebuf,0,y,0,0,256,230-y);
      update_topsubscr(y);
      advanceframe();
      if(Quit)
        return;
    }
    blit(scrollbuf,scrollbuf,256,0,0,0,256,176);
  }
  else
  {
    for(int y=0; y<=174; y+=3)
    {
      domoney();
      Link.refill();
      putsubscr(scrollbuf,0,174,showtime);
      blit(scrollbuf,framebuf,0,y,0,0,256,224);
      update_topsubscr(y);
      advanceframe();
      if(Quit)
        return;
    }
    blit(scrollbuf,scrollbuf,0,230,0,0,256,176);
  }

  Sitems.clear();
  resume_sfx(WAV_BRANG);
}

/*** end of subscr.cc ***/
